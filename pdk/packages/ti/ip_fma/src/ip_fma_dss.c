/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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
 *  \ingroup  IP_FMA_DSS
 *  \defgroup IP_FMA_DSS_IMPLEMENTATION Register readback implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_dss.c
 *
 *  \brief    Register readback implementation for dss static registers.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ip_fma_dss.h>
#include <ti/drv/dss/dss.h>

#include <ti/csl/src/ip/dss/V4/csl_dssTop.h>
#include <ti/drv/dss/soc/V2/dss_soc_priv.h>

#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

static const Dss_CommRegInfo gDss_commRegInfo[] = {
    {CSL_DSS_COMM_REG_ID_0, (void *)CSL_DSS0_DISPC_0_COMMON_M_BASE},
    {CSL_DSS_COMM_REG_ID_1, (void *)CSL_DSS0_DISPC_0_COMMON_S0_BASE},
    {CSL_DSS_COMM_REG_ID_2, (void *)CSL_DSS0_DISPC_0_COMMON_S1_BASE},
    {CSL_DSS_COMM_REG_ID_3, (void *)CSL_DSS0_DISPC_0_COMMON_S2_BASE}
};

static const Dss_PipeRegInfo gDss_pipeRegInfo[] = {
    {CSL_DSS_VID_PIPE_ID_VID1, (void *)CSL_DSS0_VID1_BASE},
    {CSL_DSS_VID_PIPE_ID_VIDL1, (void *)CSL_DSS0_VIDL1_BASE},
    {CSL_DSS_VID_PIPE_ID_VID2, (void *)CSL_DSS0_VID2_BASE},
    {CSL_DSS_VID_PIPE_ID_VIDL2, (void *)CSL_DSS0_VIDL2_BASE}
};

static const Dss_OverlayRegInfo gDss_overlayRegInfo[] = {
    {CSL_DSS_OVERLAY_ID_1, (void *)CSL_DSS0_OVR1_BASE},
    {CSL_DSS_OVERLAY_ID_2, (void *)CSL_DSS0_OVR2_BASE},
    {CSL_DSS_OVERLAY_ID_3, (void *)CSL_DSS0_OVR3_BASE},
    {CSL_DSS_OVERLAY_ID_4, (void *)CSL_DSS0_OVR4_BASE}
};

static const Dss_VpRegInfo gDss_vpRegInfo[] = {
    {CSL_DSS_VP_ID_1, (void *)CSL_DSS0_VP1_BASE},
    {CSL_DSS_VP_ID_2, (void *)CSL_DSS0_VP2_BASE},
    {CSL_DSS_VP_ID_3, (void *)CSL_DSS0_VP3_BASE},
    {CSL_DSS_VP_ID_4, (void *)CSL_DSS0_VP4_BASE}
};

static const Dss_WbPipeRegInfo gDss_wbRegInfo[] = {
    {CSL_DSS_WB_PIPE_ID_1, (void *)CSL_DSS0_WB_BASE},
};

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Dss_GetCommonMRegs(IpFma_DssCommonMRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc commonRegs[] =
        {
            { CSL_DSS_COMMON_M_DSS_REVISION,                  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DSS_CBA_CFG,                   0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_IRQENABLE_SET,           0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VID_IRQENABLE_0,               0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VID_IRQENABLE_1,               0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VID_IRQENABLE_2,               0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VID_IRQENABLE_3,               0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VP_IRQENABLE_0,                0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VP_IRQENABLE_1,                0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VP_IRQENABLE_2,                0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_VP_IRQENABLE_3,                0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_WB_IRQENABLE,                  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_SECURE_DISABLE,          0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_GLOBAL_MFLAG_ATTRIBUTE,  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_GLOBAL_OUTPUT_ENABLE,    0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_GLOBAL_BUFFER,           0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_FBDC_COMMON_CONTROL,           0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_FBDC_CONSTANT_COLOR_0,         0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_FBDC_CONSTANT_COLOR_1,         0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_CONNECTIONS,             0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_MSS_VP1,                 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_DISPC_MSS_VP3,                 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_GLOBAL_DMA_THREADSIZE,         0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_GLOBAL_DMA_THREADSIZESTATUS,   0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_M_GLOBAL_GOBITMODE,              0U, IPFMA_WIDTH_32 },
        };

        for (uint32_t index = 0U; index < CSL_DSS_COMM_REG_ID_MAX; index++)
        {
            if (gDss_commRegInfo[index].commRegId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_commRegInfo[index].commRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, commonRegs, (uint32_t)(sizeof(commonRegs)/sizeof(commonRegs[0])));

        if (IPFMA_OK == status)
        {
            out->dssRevision                    = (uint32_t)commonRegs[0].value;
            out->dssCbaCfg                      = (uint32_t)commonRegs[1].value;
            out->dssDispcIrqEnableSet           = (uint32_t)commonRegs[2].value;
            out->dssVidIrqEnable0               = (uint32_t)commonRegs[3].value;
            out->dssVidIrqEnable1               = (uint32_t)commonRegs[4].value;
            out->dssVidIrqEnable2               = (uint32_t)commonRegs[5].value;
            out->dssVidIrqEnable3               = (uint32_t)commonRegs[6].value;
            out->dssVpIrqEnable0                = (uint32_t)commonRegs[7].value;
            out->dssVpIrqEnable1                = (uint32_t)commonRegs[8].value;
            out->dssVpIrqEnable2                = (uint32_t)commonRegs[9].value;
            out->dssVpIrqEnable3                = (uint32_t)commonRegs[10].value;
            out->dssWbIrqEnable                 = (uint32_t)commonRegs[11].value;
            out->dssDispcSecureDisable          = (uint32_t)commonRegs[12].value;
            out->dssDispcGlobalMFlagAttribute   = (uint32_t)commonRegs[13].value;
            out->dssDispcGlobalOutputEnable     = (uint32_t)commonRegs[14].value;
            out->dssDispcGlobalBuffer           = (uint32_t)commonRegs[15].value;
            out->dssFbdcCommonControl           = (uint32_t)commonRegs[16].value;
            out->dssFbdcConstantColor0          = (uint32_t)commonRegs[17].value;
            out->dssFbdcConstantColor1          = (uint32_t)commonRegs[18].value;
            out->dssDispcConnections            = (uint32_t)commonRegs[19].value;
            out->dssDispcMssVp1                 = (uint32_t)commonRegs[20].value;
            out->dssDispcMssVp3                 = (uint32_t)commonRegs[21].value;
            out->dssGlobalDmaThreadSize         = (uint32_t)commonRegs[22].value;
            out->dssGlobalDmaThreadSizeStatus   = (uint32_t)commonRegs[23].value;
            out->dssGlobalGobitMode             = (uint32_t)commonRegs[24].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetCommonSRegs(IpFma_DssCommonSRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc commonRegs[] =
        {
            { CSL_DSS_COMMON_S0_DISPC_IRQSTATUS, 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_WB_IRQSTATUS,    0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VID_IRQENABLE_0, 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VID_IRQENABLE_1, 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VID_IRQENABLE_2, 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VID_IRQENABLE_3, 0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VP_IRQENABLE_0,  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VP_IRQENABLE_1,  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VP_IRQENABLE_2,  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_VP_IRQENABLE_3,  0U, IPFMA_WIDTH_32 },
            { CSL_DSS_COMMON_S0_WB_IRQENABLE,    0U, IPFMA_WIDTH_32 },
        };

        for (uint32_t index = 0U; index < CSL_DSS_COMM_REG_ID_MAX; index++)
        {
            if (gDss_commRegInfo[index].commRegId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_commRegInfo[index].commRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, commonRegs, (uint32_t)(sizeof(commonRegs)/sizeof(commonRegs[0])));

        if (IPFMA_OK == status)
        {
            out->dssDispcIrqStatus  = (uint32_t)commonRegs[0].value;
            out->dssWbIrqStatus     = (uint32_t)commonRegs[1].value;
            out->dssVidIrqEnable0   = (uint32_t)commonRegs[2].value;
            out->dssVidIrqEnable1   = (uint32_t)commonRegs[3].value;
            out->dssVidIrqEnable2   = (uint32_t)commonRegs[4].value;
            out->dssVidIrqEnable3   = (uint32_t)commonRegs[5].value;
            out->dssVpIrqEnable0    = (uint32_t)commonRegs[6].value;
            out->dssVpIrqEnable1    = (uint32_t)commonRegs[7].value;
            out->dssVpIrqEnable2    = (uint32_t)commonRegs[8].value;
            out->dssVpIrqEnable3    = (uint32_t)commonRegs[9].value;
            out->dssWbIrqEnable     = (uint32_t)commonRegs[10].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetVideoPipeRegs(IpFma_DssVideoPipeRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc videoPipeRegs[]=
        {
            { CSL_DSS_VID1_ATTRIBUTES,          0U, IPFMA_WIDTH_32 }, /* 0 */
            { CSL_DSS_VID1_ATTRIBUTES2,         0U, IPFMA_WIDTH_32 }, /* 1 */
            { CSL_DSS_VID1_BUF_THRESHOLD,       0U, IPFMA_WIDTH_32 }, /* 2 */
            { CSL_DSS_VID1_CSC_COEF0,           0U, IPFMA_WIDTH_32 }, /* 3 */
            { CSL_DSS_VID1_CSC_COEF1,           0U, IPFMA_WIDTH_32 }, /* 4 */
            { CSL_DSS_VID1_CSC_COEF2,           0U, IPFMA_WIDTH_32 }, /* 5 */
            { CSL_DSS_VID1_CSC_COEF3,           0U, IPFMA_WIDTH_32 }, /* 6 */
            { CSL_DSS_VID1_CSC_COEF4,           0U, IPFMA_WIDTH_32 }, /* 7 */
            { CSL_DSS_VID1_CSC_COEF5,           0U, IPFMA_WIDTH_32 }, /* 8 */
            { CSL_DSS_VID1_CSC_COEF6,           0U, IPFMA_WIDTH_32 }, /* 9 */
            { CSL_DSS_VID1_GLOBAL_ALPHA,        0U, IPFMA_WIDTH_32 }, /* 10 */
            { CSL_DSS_VID1_MFLAG_THRESHOLD,     0U, IPFMA_WIDTH_32 }, /* 11 */
            { CSL_DSS_VID1_CSC_COEF7,           0U, IPFMA_WIDTH_32 }, /* 12 */
            { CSL_DSS_VID1_FBDC_ATTRIBUTES,     0U, IPFMA_WIDTH_32 }, /* 13 */
            { CSL_DSS_VID1_FBDC_CLEAR_COLOR,    0U, IPFMA_WIDTH_32 }, /* 14 */
            { CSL_DSS_VID1_CLUT_0,              0U, IPFMA_WIDTH_32 }, /* 15 */
            { CSL_DSS_VID1_CLUT_1,              0U, IPFMA_WIDTH_32 }, /* 16 */
            { CSL_DSS_VID1_CLUT_2,              0U, IPFMA_WIDTH_32 }, /* 17 */
            { CSL_DSS_VID1_CLUT_3,              0U, IPFMA_WIDTH_32 }, /* 18 */
            { CSL_DSS_VID1_CLUT_4,              0U, IPFMA_WIDTH_32 }, /* 19 */
            { CSL_DSS_VID1_CLUT_5,              0U, IPFMA_WIDTH_32 }, /* 20 */
            { CSL_DSS_VID1_CLUT_6,              0U, IPFMA_WIDTH_32 }, /* 21 */
            { CSL_DSS_VID1_CLUT_7,              0U, IPFMA_WIDTH_32 }, /* 22 */
            { CSL_DSS_VID1_CLUT_8,              0U, IPFMA_WIDTH_32 }, /* 23 */
            { CSL_DSS_VID1_CLUT_9,              0U, IPFMA_WIDTH_32 }, /* 24 */
            { CSL_DSS_VID1_CLUT_10,             0U, IPFMA_WIDTH_32 }, /* 25 */
            { CSL_DSS_VID1_CLUT_11,             0U, IPFMA_WIDTH_32 }, /* 26 */
            { CSL_DSS_VID1_CLUT_12,             0U, IPFMA_WIDTH_32 }, /* 27 */
            { CSL_DSS_VID1_CLUT_13,             0U, IPFMA_WIDTH_32 }, /* 28 */
            { CSL_DSS_VID1_CLUT_14,             0U, IPFMA_WIDTH_32 }, /* 29 */
            { CSL_DSS_VID1_CLUT_15,             0U, IPFMA_WIDTH_32 }, /* 30 */
            { CSL_DSS_VID1_LUMAKEY,             0U, IPFMA_WIDTH_32 }, /* 31 */
            { CSL_DSS_VID1_DMA_BUFSIZE,         0U, IPFMA_WIDTH_32 }, /* 32 */
            { CSL_DSS_VID1_CROP,                0U, IPFMA_WIDTH_32 }, /* 33 */
            { CSL_DSS_VID1_SECURE,              0U, IPFMA_WIDTH_32 }, /* 34 */
            { CSL_DSS_VID1_PIPE_GO,             0U, IPFMA_WIDTH_32 }  /* 35 */
        };

        for (uint32_t index = 0U; index < CSL_DSS_VID_PIPE_ID_MAX; index++)
        {
            if (gDss_pipeRegInfo[index].pipeId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_pipeRegInfo[index].pipeRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, videoPipeRegs, (uint32_t)(sizeof(videoPipeRegs)/sizeof(videoPipeRegs[0])));

        if(IPFMA_OK == status)
        {
            out->dssAttributes          = (uint32_t)videoPipeRegs[0].value;
            out->dssAttributes2         = (uint32_t)videoPipeRegs[1].value;
            out->dssBufThreshold        = (uint32_t)videoPipeRegs[2].value;
            out->dssCscCoef0            = (uint32_t)videoPipeRegs[3].value;
            out->dssCscCoef1            = (uint32_t)videoPipeRegs[4].value;
            out->dssCscCoef2            = (uint32_t)videoPipeRegs[5].value;
            out->dssCscCoef3            = (uint32_t)videoPipeRegs[6].value;
            out->dssCscCoef4            = (uint32_t)videoPipeRegs[7].value;
            out->dssCscCoef5            = (uint32_t)videoPipeRegs[8].value;
            out->dssCscCoef6            = (uint32_t)videoPipeRegs[9].value;
            out->dssGlobalAlpha         = (uint32_t)videoPipeRegs[10].value;
            out->dssMflagThreshold      = (uint32_t)videoPipeRegs[11].value;
            out->dssCscCoef7            = (uint32_t)videoPipeRegs[12].value;
            out->dssFbdcAttributes      = (uint32_t)videoPipeRegs[13].value;
            out->dssFbdcClearColor      = (uint32_t)videoPipeRegs[14].value;
            out->dssClut0               = (uint32_t)videoPipeRegs[15].value;
            out->dssClut1               = (uint32_t)videoPipeRegs[16].value;
            out->dssClut2               = (uint32_t)videoPipeRegs[17].value;
            out->dssClut3               = (uint32_t)videoPipeRegs[18].value;
            out->dssClut4               = (uint32_t)videoPipeRegs[19].value;
            out->dssClut5               = (uint32_t)videoPipeRegs[20].value;
            out->dssClut6               = (uint32_t)videoPipeRegs[21].value;
            out->dssClut7               = (uint32_t)videoPipeRegs[22].value;
            out->dssClut8               = (uint32_t)videoPipeRegs[23].value;
            out->dssClut9               = (uint32_t)videoPipeRegs[24].value;
            out->dssClut10              = (uint32_t)videoPipeRegs[25].value;
            out->dssClut11              = (uint32_t)videoPipeRegs[26].value;
            out->dssClut12              = (uint32_t)videoPipeRegs[27].value;
            out->dssClut13              = (uint32_t)videoPipeRegs[28].value;
            out->dssClut14              = (uint32_t)videoPipeRegs[29].value;
            out->dssClut15              = (uint32_t)videoPipeRegs[30].value;
            out->dssLumaKey             = (uint32_t)videoPipeRegs[31].value;
            out->dssDmaBufSize          = (uint32_t)videoPipeRegs[32].value;
            out->dssCrop                = (uint32_t)videoPipeRegs[33].value;
            out->dssSecure              = (uint32_t)videoPipeRegs[34].value;
            out->dssPipeGo              = (uint32_t)videoPipeRegs[35].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetVideoPipeLayerRegs(IpFma_DssVideoPipeLayerRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc videoPipeLayer[] =
        {
            { CSL_DSS_VIDL1_BUF_SIZE_STATUS,     0U, IPFMA_WIDTH_32 }, /* 0 */
            { CSL_DSS_VIDL1_SAFETY_ATTRIBUTES,   0U, IPFMA_WIDTH_32 }, /* 1 */
            { CSL_DSS_VIDL1_ATTRIBUTES,          0U, IPFMA_WIDTH_32 }, /* 2 */
            { CSL_DSS_VIDL1_ATTRIBUTES2,         0U, IPFMA_WIDTH_32 }, /* 3 */
            { CSL_DSS_VIDL1_BUF_THRESHOLD,       0U, IPFMA_WIDTH_32 }, /* 4 */
            { CSL_DSS_VIDL1_CSC_COEF0,           0U, IPFMA_WIDTH_32 }, /* 5 */
            { CSL_DSS_VIDL1_CSC_COEF1,           0U, IPFMA_WIDTH_32 }, /* 6 */
            { CSL_DSS_VIDL1_CSC_COEF2,           0U, IPFMA_WIDTH_32 }, /* 7 */
            { CSL_DSS_VIDL1_CSC_COEF3,           0U, IPFMA_WIDTH_32 }, /* 8 */
            { CSL_DSS_VIDL1_CSC_COEF4,           0U, IPFMA_WIDTH_32 }, /* 9 */
            { CSL_DSS_VIDL1_CSC_COEF5,           0U, IPFMA_WIDTH_32 }, /* 10 */
            { CSL_DSS_VIDL1_CSC_COEF6,           0U, IPFMA_WIDTH_32 }, /* 11 */
            { CSL_DSS_VIDL1_GLOBAL_ALPHA,        0U, IPFMA_WIDTH_32 }, /* 12 */
            { CSL_DSS_VIDL1_MFLAG_THRESHOLD,     0U, IPFMA_WIDTH_32 }, /* 13 */
            { CSL_DSS_VIDL1_CSC_COEF7,           0U, IPFMA_WIDTH_32 }, /* 14 */
            { CSL_DSS_VIDL1_FBDC_ATTRIBUTES,     0U, IPFMA_WIDTH_32 }, /* 15 */
            { CSL_DSS_VIDL1_FBDC_CLEAR_COLOR,    0U, IPFMA_WIDTH_32 }, /* 16 */
            { CSL_DSS_VIDL1_CLUT_0,              0U, IPFMA_WIDTH_32 }, /* 17 */
            { CSL_DSS_VIDL1_CLUT_1,              0U, IPFMA_WIDTH_32 }, /* 18 */
            { CSL_DSS_VIDL1_CLUT_2,              0U, IPFMA_WIDTH_32 }, /* 19 */
            { CSL_DSS_VIDL1_CLUT_3,              0U, IPFMA_WIDTH_32 }, /* 20 */
            { CSL_DSS_VIDL1_CLUT_4,              0U, IPFMA_WIDTH_32 }, /* 21 */
            { CSL_DSS_VIDL1_CLUT_5,              0U, IPFMA_WIDTH_32 }, /* 22 */
            { CSL_DSS_VIDL1_CLUT_6,              0U, IPFMA_WIDTH_32 }, /* 23 */
            { CSL_DSS_VIDL1_CLUT_7,              0U, IPFMA_WIDTH_32 }, /* 24 */
            { CSL_DSS_VIDL1_CLUT_8,              0U, IPFMA_WIDTH_32 }, /* 25 */
            { CSL_DSS_VIDL1_CLUT_9,              0U, IPFMA_WIDTH_32 }, /* 26 */
            { CSL_DSS_VIDL1_CLUT_10,             0U, IPFMA_WIDTH_32 }, /* 27 */
            { CSL_DSS_VIDL1_CLUT_11,             0U, IPFMA_WIDTH_32 }, /* 28 */
            { CSL_DSS_VIDL1_CLUT_12,             0U, IPFMA_WIDTH_32 }, /* 29 */
            { CSL_DSS_VIDL1_CLUT_13,             0U, IPFMA_WIDTH_32 }, /* 30 */
            { CSL_DSS_VIDL1_CLUT_14,             0U, IPFMA_WIDTH_32 }, /* 31 */
            { CSL_DSS_VIDL1_CLUT_15,             0U, IPFMA_WIDTH_32 }, /* 32 */
            { CSL_DSS_VIDL1_LUMAKEY,             0U, IPFMA_WIDTH_32 }, /* 33 */
            { CSL_DSS_VIDL1_DMA_BUFSIZE,         0U, IPFMA_WIDTH_32 }, /* 34 */
            { CSL_DSS_VIDL1_CROP,                0U, IPFMA_WIDTH_32 }, /* 35 */
            { CSL_DSS_VIDL1_SECURE,              0U, IPFMA_WIDTH_32 }, /* 36 */
            { CSL_DSS_VIDL1_PIPE_GO,             0U, IPFMA_WIDTH_32 }  /* 37 */

        };

        for (uint32_t index = 0U; index < CSL_DSS_VID_PIPE_ID_MAX; index++)
        {
            if (gDss_pipeRegInfo[index].pipeId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_pipeRegInfo[index].pipeRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, videoPipeLayer, (uint32_t)(sizeof(videoPipeLayer)/sizeof(videoPipeLayer[0])));

        if (IPFMA_OK == status)
        {
            out->dssBufSizeStatus       = (uint32_t)videoPipeLayer[0].value;
            out->dssSafetyAttributes    = (uint32_t)videoPipeLayer[1].value;
            out->dssAttributes          = (uint32_t)videoPipeLayer[2].value;
            out->dssAttributes2         = (uint32_t)videoPipeLayer[3].value;
            out->dssBufThreshold        = (uint32_t)videoPipeLayer[4].value;
            out->dssCscCoef0            = (uint32_t)videoPipeLayer[5].value;
            out->dssCscCoef1            = (uint32_t)videoPipeLayer[6].value;
            out->dssCscCoef2            = (uint32_t)videoPipeLayer[7].value;
            out->dssCscCoef3            = (uint32_t)videoPipeLayer[8].value;
            out->dssCscCoef4            = (uint32_t)videoPipeLayer[9].value;
            out->dssCscCoef5            = (uint32_t)videoPipeLayer[10].value;
            out->dssCscCoef6            = (uint32_t)videoPipeLayer[11].value;
            out->dssGlobalAlpha         = (uint32_t)videoPipeLayer[12].value;
            out->dssMflagThreshold      = (uint32_t)videoPipeLayer[13].value;
            out->dssCscCoef7            = (uint32_t)videoPipeLayer[14].value;
            out->dssFbdcAttributes      = (uint32_t)videoPipeLayer[15].value;
            out->dssFbdcClearColor      = (uint32_t)videoPipeLayer[16].value;
            out->dssClut0               = (uint32_t)videoPipeLayer[17].value;
            out->dssClut1               = (uint32_t)videoPipeLayer[18].value;
            out->dssClut2               = (uint32_t)videoPipeLayer[19].value;
            out->dssClut3               = (uint32_t)videoPipeLayer[20].value;
            out->dssClut4               = (uint32_t)videoPipeLayer[21].value;
            out->dssClut5               = (uint32_t)videoPipeLayer[22].value;
            out->dssClut6               = (uint32_t)videoPipeLayer[23].value;
            out->dssClut7               = (uint32_t)videoPipeLayer[24].value;
            out->dssClut8               = (uint32_t)videoPipeLayer[25].value;
            out->dssClut9               = (uint32_t)videoPipeLayer[26].value;
            out->dssClut10              = (uint32_t)videoPipeLayer[27].value;
            out->dssClut11              = (uint32_t)videoPipeLayer[28].value;
            out->dssClut12              = (uint32_t)videoPipeLayer[29].value;
            out->dssClut13              = (uint32_t)videoPipeLayer[30].value;
            out->dssClut14              = (uint32_t)videoPipeLayer[31].value;
            out->dssClut15              = (uint32_t)videoPipeLayer[32].value;
            out->dssLumaKey             = (uint32_t)videoPipeLayer[33].value;
            out->dssDmaBufSize          = (uint32_t)videoPipeLayer[34].value;
            out->dssCrop                = (uint32_t)videoPipeLayer[35].value;
            out->dssSecure              = (uint32_t)videoPipeLayer[36].value;
            out->dssPipeGo              = (uint32_t)videoPipeLayer[37].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetOverlayRegs(IpFma_DssOverlayRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc overlayRegs[] = {
            { CSL_DSS_OVR1_CONFIG,             0U, IPFMA_WIDTH_32 }, /* 0 */
            { CSL_DSS_OVR1_VIRTUALVP,          0U, IPFMA_WIDTH_32 }, /* 1 */
            { CSL_DSS_OVR1_DEFAULT_COLOR,      0U, IPFMA_WIDTH_32 }, /* 2 */
            { CSL_DSS_OVR1_DEFAULT_COLOR2,     0U, IPFMA_WIDTH_32 }, /* 3 */
            { CSL_DSS_OVR1_TRANS_COLOR_MAX,    0U, IPFMA_WIDTH_32 }, /* 4 */
            { CSL_DSS_OVR1_TRANS_COLOR_MAX2,   0U, IPFMA_WIDTH_32 }, /* 5 */
            { CSL_DSS_OVR1_TRANS_COLOR_MIN,    0U, IPFMA_WIDTH_32 }, /* 6 */
            { CSL_DSS_OVR1_TRANS_COLOR_MIN2,   0U, IPFMA_WIDTH_32 }, /* 7 */
            { CSL_DSS_OVR1_SECURE,             0U, IPFMA_WIDTH_32 }  /* 8 */
        };

        for (uint32_t index = 0U; index < CSL_DSS_OVERLAY_ID_MAX; index++)
        {
            if (gDss_overlayRegInfo[index].overlayId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_overlayRegInfo[index].overlayRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, overlayRegs, (uint32_t)(sizeof(overlayRegs)/sizeof(overlayRegs[0])));

        if(IPFMA_OK == status)
        {
            out->dssConfig            = (uint32_t)overlayRegs[0].value;
            out->dssVirtualVp         = (uint32_t)overlayRegs[1].value;
            out->dssDefaultColor      = (uint32_t)overlayRegs[2].value;
            out->dssDefaultColor2     = (uint32_t)overlayRegs[3].value;
            out->dssTransColorMax     = (uint32_t)overlayRegs[4].value;
            out->dssTransColorMax2    = (uint32_t)overlayRegs[5].value;
            out->dssTransColorMin     = (uint32_t)overlayRegs[6].value;
            out->dssTransColorMin2    = (uint32_t)overlayRegs[7].value;
            out->dssSecure            = (uint32_t)overlayRegs[8].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetVideoPortRegs(IpFma_DssVideoPortRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc videoPortRegs[] = {
            
            { CSL_DSS_VP1_CONFIG,           0U, IPFMA_WIDTH_32 }, /* 0 */
            { CSL_DSS_VP1_CONTROL,          0U, IPFMA_WIDTH_32 }, /* 1 */
            { CSL_DSS_VP1_CSC_COEF0,        0U, IPFMA_WIDTH_32 }, /* 2 */
            { CSL_DSS_VP1_CSC_COEF1,        0U, IPFMA_WIDTH_32 }, /* 3 */
            { CSL_DSS_VP1_CSC_COEF2,        0U, IPFMA_WIDTH_32 }, /* 4 */
            { CSL_DSS_VP1_DATA_CYCLE_0,     0U, IPFMA_WIDTH_32 }, /* 5 */
            { CSL_DSS_VP1_DATA_CYCLE_1,     0U, IPFMA_WIDTH_32 }, /* 6 */
            { CSL_DSS_VP1_DATA_CYCLE_2,     0U, IPFMA_WIDTH_32 }, /* 7 */
            { CSL_DSS_VP1_LINE_NUMBER,      0U, IPFMA_WIDTH_32 }, /* 8 */
            { CSL_DSS_VP1_POL_FREQ,         0U, IPFMA_WIDTH_32 }, /* 9 */
            { CSL_DSS_VP1_SIZE_SCREEN,      0U, IPFMA_WIDTH_32 }, /* 10 */
            { CSL_DSS_VP1_TIMING_H,         0U, IPFMA_WIDTH_32 }, /* 11 */
            { CSL_DSS_VP1_TIMING_V,         0U, IPFMA_WIDTH_32 }, /* 12 */
            { CSL_DSS_VP1_CSC_COEF3,        0U, IPFMA_WIDTH_32 }, /* 13 */
            { CSL_DSS_VP1_CSC_COEF4,        0U, IPFMA_WIDTH_32 }, /* 14 */
            { CSL_DSS_VP1_CSC_COEF5,        0U, IPFMA_WIDTH_32 }, /* 15 */
            { CSL_DSS_VP1_CSC_COEF6,        0U, IPFMA_WIDTH_32 }, /* 16 */
            { CSL_DSS_VP1_CSC_COEF7,        0U, IPFMA_WIDTH_32 }, /* 17 */
            { CSL_DSS_VP1_GAMMA_TABLE_0,    0U, IPFMA_WIDTH_32 }, /* 18 */
            { CSL_DSS_VP1_GAMMA_TABLE_1,    0U, IPFMA_WIDTH_32 }, /* 19 */
            { CSL_DSS_VP1_GAMMA_TABLE_2,    0U, IPFMA_WIDTH_32 }, /* 20 */
            { CSL_DSS_VP1_GAMMA_TABLE_3,    0U, IPFMA_WIDTH_32 }, /* 21 */
            { CSL_DSS_VP1_GAMMA_TABLE_4,    0U, IPFMA_WIDTH_32 }, /* 22 */
            { CSL_DSS_VP1_GAMMA_TABLE_5,    0U, IPFMA_WIDTH_32 }, /* 23 */
            { CSL_DSS_VP1_GAMMA_TABLE_6,    0U, IPFMA_WIDTH_32 }, /* 24 */
            { CSL_DSS_VP1_GAMMA_TABLE_7,    0U, IPFMA_WIDTH_32 }, /* 25 */
            { CSL_DSS_VP1_GAMMA_TABLE_8,    0U, IPFMA_WIDTH_32 }, /* 26 */
            { CSL_DSS_VP1_GAMMA_TABLE_9,    0U, IPFMA_WIDTH_32 }, /* 27 */
            { CSL_DSS_VP1_GAMMA_TABLE_10,   0U, IPFMA_WIDTH_32 }, /* 28 */
            { CSL_DSS_VP1_GAMMA_TABLE_11,   0U, IPFMA_WIDTH_32 }, /* 29 */
            { CSL_DSS_VP1_GAMMA_TABLE_12,   0U, IPFMA_WIDTH_32 }, /* 30 */
            { CSL_DSS_VP1_GAMMA_TABLE_13,   0U, IPFMA_WIDTH_32 }, /* 31 */
            { CSL_DSS_VP1_GAMMA_TABLE_14,   0U, IPFMA_WIDTH_32 }, /* 32 */
            { CSL_DSS_VP1_GAMMA_TABLE_15,   0U, IPFMA_WIDTH_32 }, /* 33 */
            { CSL_DSS_VP1_DSS_OLDI_CFG,     0U, IPFMA_WIDTH_32 }, /* 34 */
            { CSL_DSS_VP1_DSS_OLDI_STATUS,  0U, IPFMA_WIDTH_32 }, /* 35 */
            { CSL_DSS_VP1_DSS_OLDI_LB,      0U, IPFMA_WIDTH_32 }, /* 36 */
            { CSL_DSS_VP1_SECURE,           0U, IPFMA_WIDTH_32 }  /* 37 */
        };

        for (uint32_t index = 0U; index < CSL_DSS_VP_ID_MAX; index++)
        {
            if (gDss_vpRegInfo[index].vpId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_vpRegInfo[index].vpRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, videoPortRegs, (uint32_t)(sizeof(videoPortRegs)/sizeof(videoPortRegs[0])));

        if (IPFMA_OK == status)
        {
            out->dssConfig        = (uint32_t)videoPortRegs[0].value;
            out->dssControl       = (uint32_t)videoPortRegs[1].value;
            out->dssCscCoef0      = (uint32_t)videoPortRegs[2].value;
            out->dssCscCoef1      = (uint32_t)videoPortRegs[3].value;
            out->dssCscCoef2      = (uint32_t)videoPortRegs[4].value;
            out->dssDataCycle0    = (uint32_t)videoPortRegs[5].value;
            out->dssDataCycle1    = (uint32_t)videoPortRegs[6].value;
            out->dssDataCycle2    = (uint32_t)videoPortRegs[7].value;
            out->dssLineNumber    = (uint32_t)videoPortRegs[8].value;
            out->dssPolFreq       = (uint32_t)videoPortRegs[9].value;
            out->dssSizeScreen    = (uint32_t)videoPortRegs[10].value;
            out->dssTimingH       = (uint32_t)videoPortRegs[11].value;
            out->dssTimingV       = (uint32_t)videoPortRegs[12].value;
            out->dssCscCoef3      = (uint32_t)videoPortRegs[13].value;
            out->dssCscCoef4      = (uint32_t)videoPortRegs[14].value;
            out->dssCscCoef5      = (uint32_t)videoPortRegs[15].value;
            out->dssCscCoef6      = (uint32_t)videoPortRegs[16].value;
            out->dssCscCoef7      = (uint32_t)videoPortRegs[17].value;
            out->dssGammaTable0   = (uint32_t)videoPortRegs[18].value;
            out->dssGammaTable1   = (uint32_t)videoPortRegs[19].value;
            out->dssGammaTable2   = (uint32_t)videoPortRegs[20].value;
            out->dssGammaTable3   = (uint32_t)videoPortRegs[21].value;
            out->dssGammaTable4   = (uint32_t)videoPortRegs[22].value;
            out->dssGammaTable5   = (uint32_t)videoPortRegs[23].value;
            out->dssGammaTable6   = (uint32_t)videoPortRegs[24].value;
            out->dssGammaTable7   = (uint32_t)videoPortRegs[25].value;
            out->dssGammaTable8   = (uint32_t)videoPortRegs[26].value;
            out->dssGammaTable9   = (uint32_t)videoPortRegs[27].value;
            out->dssGammaTable10  = (uint32_t)videoPortRegs[28].value;
            out->dssGammaTable11  = (uint32_t)videoPortRegs[29].value;
            out->dssGammaTable12  = (uint32_t)videoPortRegs[30].value;
            out->dssGammaTable13  = (uint32_t)videoPortRegs[31].value;
            out->dssGammaTable14  = (uint32_t)videoPortRegs[32].value;
            out->dssGammaTable15  = (uint32_t)videoPortRegs[33].value;
            out->dssOldiConfig    = (uint32_t)videoPortRegs[34].value;
            out->dssOldiStatus    = (uint32_t)videoPortRegs[35].value;
            out->dssOldiLb        = (uint32_t)videoPortRegs[36].value;
            out->dssSecure        = (uint32_t)videoPortRegs[37].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_GetWriteBackPipeRegs(IpFma_DssWriteBackPipeRegs* out, uint32_t instanceId)
{
    IpFma_Status status = IPFMA_OK;
    uintptr_t instanceBaseAddress = 0U;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc writeBackPipeRegs[] = {
            { CSL_DSS_WB_ATTRIBUTES,         0U, IPFMA_WIDTH_32 }, /* 0 */
            { CSL_DSS_WB_ATTRIBUTES2,        0U, IPFMA_WIDTH_32 }, /* 1 */
            { CSL_DSS_WB_BUF_THRESHOLD,      0U, IPFMA_WIDTH_32 }, /* 2 */
            { CSL_DSS_WB_CSC_COEF0,          0U, IPFMA_WIDTH_32 }, /* 3 */
            { CSL_DSS_WB_CSC_COEF1,          0U, IPFMA_WIDTH_32 }, /* 4 */
            { CSL_DSS_WB_CSC_COEF2,          0U, IPFMA_WIDTH_32 }, /* 5 */
            { CSL_DSS_WB_CSC_COEF3,          0U, IPFMA_WIDTH_32 }, /* 6 */
            { CSL_DSS_WB_CSC_COEF4,          0U, IPFMA_WIDTH_32 }, /* 7 */
            { CSL_DSS_WB_CSC_COEF5,          0U, IPFMA_WIDTH_32 }, /* 8 */
            { CSL_DSS_WB_CSC_COEF6,          0U, IPFMA_WIDTH_32 }, /* 9 */
            { CSL_DSS_WB_MFLAG_THRESHOLD,    0U, IPFMA_WIDTH_32 }, /* 10 */
            { CSL_DSS_WB_SECURE,             0U, IPFMA_WIDTH_32 }  /* 11 */
        };

        for (uint32_t index = 0U; index < CSL_DSS_WB_PIPE_ID_MAX; index++)
        {
            if (gDss_wbRegInfo[index].pipeId == instanceId)
            {
                instanceBaseAddress = (uintptr_t) gDss_wbRegInfo[index].wbPipeRegs;
            }
        }

        status = IpFma_GetRegsValues(instanceBaseAddress, writeBackPipeRegs, (uint32_t)(sizeof(writeBackPipeRegs)/sizeof(writeBackPipeRegs[0])));

        if (IPFMA_OK == status)
        {
            out->dssAttributes        = (uint32_t)writeBackPipeRegs[0].value;
            out->dssAttributes2       = (uint32_t)writeBackPipeRegs[1].value;
            out->dssBufThreshold      = (uint32_t)writeBackPipeRegs[2].value;
            out->dssCscCoef0          = (uint32_t)writeBackPipeRegs[3].value;
            out->dssCscCoef1          = (uint32_t)writeBackPipeRegs[4].value;
            out->dssCscCoef2          = (uint32_t)writeBackPipeRegs[5].value;
            out->dssCscCoef3          = (uint32_t)writeBackPipeRegs[6].value;
            out->dssCscCoef4          = (uint32_t)writeBackPipeRegs[7].value;
            out->dssCscCoef5          = (uint32_t)writeBackPipeRegs[8].value;
            out->dssCscCoef6          = (uint32_t)writeBackPipeRegs[9].value;
            out->dssMflagThreshold    = (uint32_t)writeBackPipeRegs[10].value;
            out->dssSecure            = (uint32_t)writeBackPipeRegs[11].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareCommonMRegs(const IpFma_DssCommonMRegs* expected, const IpFma_DssCommonMRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssCbaCfg != actual->dssCbaCfg)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssRevision != actual->dssRevision))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcIrqEnableSet != actual->dssDispcIrqEnableSet))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable0 != actual->dssVidIrqEnable0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable1 != actual->dssVidIrqEnable1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable2 != actual->dssVidIrqEnable2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable3 != actual->dssVidIrqEnable3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable0 != actual->dssVpIrqEnable0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable1 != actual->dssVpIrqEnable1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable2 != actual->dssVpIrqEnable2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable3 != actual->dssVpIrqEnable3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssWbIrqEnable != actual->dssWbIrqEnable))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcSecureDisable != actual->dssDispcSecureDisable))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcGlobalMFlagAttribute != actual->dssDispcGlobalMFlagAttribute))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcGlobalOutputEnable != actual->dssDispcGlobalOutputEnable))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcGlobalBuffer != actual->dssDispcGlobalBuffer))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcCommonControl != actual->dssFbdcCommonControl))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcConstantColor0 != actual->dssFbdcConstantColor0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcConstantColor1 != actual->dssFbdcConstantColor1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcConnections != actual->dssDispcConnections))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcMssVp1 != actual->dssDispcMssVp1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDispcMssVp3 != actual->dssDispcMssVp3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGlobalDmaThreadSize != actual->dssGlobalDmaThreadSize))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGlobalDmaThreadSizeStatus != actual->dssGlobalDmaThreadSizeStatus))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGlobalGobitMode != actual->dssGlobalGobitMode))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}


IpFma_Status IpFma_Dss_CompareCommonSRegs(const IpFma_DssCommonSRegs* expected, const IpFma_DssCommonSRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssDispcIrqStatus != actual->dssDispcIrqStatus)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssWbIrqStatus != actual->dssWbIrqStatus))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable0 != actual->dssVidIrqEnable0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable1 != actual->dssVidIrqEnable1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable2 != actual->dssVidIrqEnable2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVidIrqEnable3 != actual->dssVidIrqEnable3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable0 != actual->dssVpIrqEnable0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable1 != actual->dssVpIrqEnable1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable2 != actual->dssVpIrqEnable2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVpIrqEnable3 != actual->dssVpIrqEnable3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssWbIrqEnable != actual->dssWbIrqEnable))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareVideoPipeRegs(const IpFma_DssVideoPipeRegs* expected, const IpFma_DssVideoPipeRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssAttributes != actual->dssAttributes)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssAttributes2 != actual->dssAttributes2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssBufThreshold != actual->dssBufThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef0 != actual->dssCscCoef0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef1 != actual->dssCscCoef1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef2 != actual->dssCscCoef2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef3 != actual->dssCscCoef3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef4 != actual->dssCscCoef4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef5 != actual->dssCscCoef5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef6 != actual->dssCscCoef6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGlobalAlpha != actual->dssGlobalAlpha))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssMflagThreshold != actual->dssMflagThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef7 != actual->dssCscCoef7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcAttributes != actual->dssFbdcAttributes))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcClearColor != actual->dssFbdcClearColor))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut0  != actual->dssClut0))  
        {
            status = IPFMA_E_MISMATCH; 
        }
        if ((IPFMA_OK == status) && (expected->dssClut1 != actual->dssClut1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut2 != actual->dssClut2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut3 != actual->dssClut3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut4 != actual->dssClut4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut5 != actual->dssClut5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut6 != actual->dssClut6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut7 != actual->dssClut7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut8 != actual->dssClut8))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut9 != actual->dssClut9))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut10 != actual->dssClut10))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut11 != actual->dssClut11))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut12 != actual->dssClut12))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut13 != actual->dssClut13))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut14 != actual->dssClut14))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut15 != actual->dssClut15))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssLumaKey != actual->dssLumaKey))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDmaBufSize != actual->dssDmaBufSize))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCrop != actual->dssCrop))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSecure != actual->dssSecure))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssPipeGo != actual->dssPipeGo))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareVideoPipeLayerRegs(const IpFma_DssVideoPipeLayerRegs* expected, const IpFma_DssVideoPipeLayerRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssBufSizeStatus != actual->dssBufSizeStatus)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSafetyAttributes != actual->dssSafetyAttributes))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssAttributes != actual->dssAttributes))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssAttributes2 != actual->dssAttributes2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssBufThreshold != actual->dssBufThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef0 != actual->dssCscCoef0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef1 != actual->dssCscCoef1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef2 != actual->dssCscCoef2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef3 != actual->dssCscCoef3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef4 != actual->dssCscCoef4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef5 != actual->dssCscCoef5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef6 != actual->dssCscCoef6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGlobalAlpha != actual->dssGlobalAlpha))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssMflagThreshold != actual->dssMflagThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef7 != actual->dssCscCoef7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcAttributes != actual->dssFbdcAttributes))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssFbdcClearColor != actual->dssFbdcClearColor))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut0  != actual->dssClut0))
        {
            status = IPFMA_E_MISMATCH;
        }        
        if ((IPFMA_OK == status) && (expected->dssClut1 != actual->dssClut1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut2 != actual->dssClut2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut3 != actual->dssClut3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut4 != actual->dssClut4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut5 != actual->dssClut5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut6 != actual->dssClut6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut7 != actual->dssClut7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut8 != actual->dssClut8))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut9 != actual->dssClut9))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut10 != actual->dssClut10))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut11 != actual->dssClut11))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut12 != actual->dssClut12))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut13 != actual->dssClut13))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut14 != actual->dssClut14))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssClut15 != actual->dssClut15))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssLumaKey != actual->dssLumaKey))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDmaBufSize != actual->dssDmaBufSize))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCrop != actual->dssCrop))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSecure != actual->dssSecure))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssPipeGo != actual->dssPipeGo))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareOverlayRegs(const IpFma_DssOverlayRegs* expected, const IpFma_DssOverlayRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssConfig != actual->dssConfig)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssVirtualVp != actual->dssVirtualVp))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDefaultColor != actual->dssDefaultColor))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDefaultColor2 != actual->dssDefaultColor2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTransColorMax != actual->dssTransColorMax))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTransColorMax2 != actual->dssTransColorMax2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTransColorMin != actual->dssTransColorMin))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTransColorMin2 != actual->dssTransColorMin2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSecure != actual->dssSecure))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareVideoPortRegs(const IpFma_DssVideoPortRegs* expected, const IpFma_DssVideoPortRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssConfig != actual->dssConfig)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssControl != actual->dssControl))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef0 != actual->dssCscCoef0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef1 != actual->dssCscCoef1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef2 != actual->dssCscCoef2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDataCycle0 != actual->dssDataCycle0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDataCycle1 != actual->dssDataCycle1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssDataCycle2 != actual->dssDataCycle2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssLineNumber != actual->dssLineNumber))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssPolFreq != actual->dssPolFreq))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSizeScreen != actual->dssSizeScreen))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTimingH != actual->dssTimingH))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssTimingV != actual->dssTimingV))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef3 != actual->dssCscCoef3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef4 != actual->dssCscCoef4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef5 != actual->dssCscCoef5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef6 != actual->dssCscCoef6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef7 != actual->dssCscCoef7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable0  != actual->dssGammaTable0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable1  != actual->dssGammaTable1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable2  != actual->dssGammaTable2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable3  != actual->dssGammaTable3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable4  != actual->dssGammaTable4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable5  != actual->dssGammaTable5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable6  != actual->dssGammaTable6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable7  != actual->dssGammaTable7))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable8  != actual->dssGammaTable8))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable9  != actual->dssGammaTable9))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable10 != actual->dssGammaTable10))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable11 != actual->dssGammaTable11))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable12 != actual->dssGammaTable12))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable13 != actual->dssGammaTable13))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable14 != actual->dssGammaTable14))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssGammaTable15 != actual->dssGammaTable15))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssOldiConfig != actual->dssOldiConfig))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssOldiStatus != actual->dssOldiStatus))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssOldiLb != actual->dssOldiLb))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSecure != actual->dssSecure))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Dss_CompareWriteBackPipeRegs(const IpFma_DssWriteBackPipeRegs* expected, const IpFma_DssWriteBackPipeRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == expected) || (NULL_PTR == actual))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->dssAttributes != actual->dssAttributes)
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssAttributes2 != actual->dssAttributes2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssBufThreshold != actual->dssBufThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef0 != actual->dssCscCoef0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef1 != actual->dssCscCoef1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef2 != actual->dssCscCoef2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef3 != actual->dssCscCoef3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef4 != actual->dssCscCoef4))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef5 != actual->dssCscCoef5))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssCscCoef6 != actual->dssCscCoef6))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssMflagThreshold != actual->dssMflagThreshold))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dssSecure != actual->dssSecure))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/** @} */
