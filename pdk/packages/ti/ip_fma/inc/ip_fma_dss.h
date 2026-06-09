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
 *  \defgroup IP_FMA_DSS Register readback for dss static registers
 */

/**
 *  \ingroup  IP_FMA_DSS
 *  \defgroup IP_FMA_DSS_INTERFACE Register readback interface.
 *
 *  @{
 */

/**
 *  \file     ip_fma_dss.h
 *
 *  \brief    Register readback interface for dss registers.
 *
 */

#ifndef IP_FMA_DSS
#define IP_FMA_DSS

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ip_fma_common.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \brief Dss Common M region registers.
 */
typedef struct
{
    uint32_t dssRevision;                    /**< DSS_REVISION */
    uint32_t dssCbaCfg;                      /**< DSS_CBA_CFG */
    uint32_t dssDispcIrqEnableSet;           /**< DSS_DISPC_IRQENABLE_SET */
    uint32_t dssVidIrqEnable0;               /**< DSS_VID_IRQENABLE0 */
    uint32_t dssVidIrqEnable1;               /**< DSS_VID_IRQENABLE1 */
    uint32_t dssVidIrqEnable2;               /**< DSS_VID_IRQENABLE2 */
    uint32_t dssVidIrqEnable3;               /**< DSS_VID_IRQENABLE3 */
    uint32_t dssVpIrqEnable0;                /**< DSS_VP_IRQENABLE0 */
    uint32_t dssVpIrqEnable1;                /**< DSS_VP_IRQENABLE1 */
    uint32_t dssVpIrqEnable2;                /**< DSS_VP_IRQENABLE2 */
    uint32_t dssVpIrqEnable3;                /**< DSS_VP_IRQENABLE3 */
    uint32_t dssWbIrqEnable;                 /**< DSS_WB_IRQENABLE */
    uint32_t dssDispcSecureDisable;          /**< DSS_DISPC_SECURE_DISABLE */
    uint32_t dssDispcGlobalMFlagAttribute;   /**< DSS_DISPC_GLOBAL_MFLAG_ATTRIBUTE */
    uint32_t dssDispcGlobalOutputEnable;     /**< DSS_DISPC_GLOBAL_OUTPUT_ENABLE */
    uint32_t dssDispcGlobalBuffer;           /**< DSS_DISPC_GLOBAL_BUFFER */
    uint32_t dssFbdcCommonControl;           /**< DSS_FBDC_COMMON_CONTROL */
    uint32_t dssFbdcConstantColor0;          /**< DSS_FBDC_CONSTANT_COLOR0 */
    uint32_t dssFbdcConstantColor1;          /**< DSS_FBDC_CONSTANT_COLOR1 */
    uint32_t dssDispcConnections;            /**< DSS_DISPC_CONNECTIONS */
    uint32_t dssDispcMssVp1;                 /**< DSS_DISPC_MSS_VP1 */
    uint32_t dssDispcMssVp3;                 /**< DSS_DISPC_MSS_VP3 */
    uint32_t dssGlobalDmaThreadSize;         /**< DSS_GLOBAL_DMA_THREAD_SIZE */
    uint32_t dssGlobalDmaThreadSizeStatus;   /**< DSS_GLOBAL_DMA_THREAD_SIZE_STATUS */
    uint32_t dssGlobalGobitMode;             /**< DSS_GLOBAL_GOBIT_MODE */
} IpFma_DssCommonMRegs;

/**
 *  \brief Dss Common S region registers.
 */
typedef struct
{
    uint32_t dssDispcIrqStatus;      /**< DSS_DISPC_IRQSTATUS */
    uint32_t dssWbIrqStatus;         /**< DSS_WB_IRQSTATUS */
    uint32_t dssVidIrqEnable0;       /**< DSS_VID_IRQENABLE0 */
    uint32_t dssVidIrqEnable1;       /**< DSS_VID_IRQENABLE1 */
    uint32_t dssVidIrqEnable2;       /**< DSS_VID_IRQENABLE2 */
    uint32_t dssVidIrqEnable3;       /**< DSS_VID_IRQENABLE3 */
    uint32_t dssVpIrqEnable0;        /**< DSS_VP_IRQENABLE0 */
    uint32_t dssVpIrqEnable1;        /**< DSS_VP_IRQENABLE1 */
    uint32_t dssVpIrqEnable2;        /**< DSS_VP_IRQENABLE2 */
    uint32_t dssVpIrqEnable3;        /**< DSS_VP_IRQENABLE3 */
    uint32_t dssWbIrqEnable;         /**< DSS_WB_IRQENABLE */
} IpFma_DssCommonSRegs;

/**
 *  \brief Video pipe region registers.
 */
typedef struct
{
    uint32_t    dssAttributes;        /**< DSS_VID_ATTRIBUTES */
    uint32_t    dssAttributes2;       /**< DSS_VID_ATTRIBUTES2 */
    uint32_t    dssBufThreshold;      /**< DSS_VID_BUF_THRESHOLD */
    uint32_t    dssCscCoef0;          /**< DSS_VID_CSC_COEF0 */
    uint32_t    dssCscCoef1;          /**< DSS_VID_CSC_COEF1 */
    uint32_t    dssCscCoef2;          /**< DSS_VID_CSC_COEF2 */
    uint32_t    dssCscCoef3;          /**< DSS_VID_CSC_COEF3 */
    uint32_t    dssCscCoef4;          /**< DSS_VID_CSC_COEF4 */
    uint32_t    dssCscCoef5;          /**< DSS_VID_CSC_COEF5 */
    uint32_t    dssCscCoef6;          /**< DSS_VID_CSC_COEF6 */
    uint32_t    dssGlobalAlpha;       /**< DSS_VID_GLOBAL_ALPHA */
    uint32_t    dssMflagThreshold;    /**< DSS_VID_MFLAG_THRESHOLD */
    uint32_t    dssCscCoef7;          /**< DSS_VID_CSC_COEF7 */
    uint32_t    dssFbdcAttributes;    /**< DSS_VID_FBDC_ATTRIBUTES */
    uint32_t    dssFbdcClearColor;    /**< DSS_VID_FBDC_CLEAR_COLOR */
    uint32_t    dssClut0;             /**< DSS_VID_CLUT_0 */
    uint32_t    dssClut1;             /**< DSS_VID_CLUT_1 */
    uint32_t    dssClut2;             /**< DSS_VID_CLUT_2 */
    uint32_t    dssClut3;             /**< DSS_VID_CLUT_3 */
    uint32_t    dssClut4;             /**< DSS_VID_CLUT_4 */
    uint32_t    dssClut5;             /**< DSS_VID_CLUT_5 */
    uint32_t    dssClut6;             /**< DSS_VID_CLUT_6 */
    uint32_t    dssClut7;             /**< DSS_VID_CLUT_7 */
    uint32_t    dssClut8;             /**< DSS_VID_CLUT_8 */
    uint32_t    dssClut9;             /**< DSS_VID_CLUT_9 */
    uint32_t    dssClut10;            /**< DSS_VID_CLUT_10 */
    uint32_t    dssClut11;            /**< DSS_VID_CLUT_11 */
    uint32_t    dssClut12;            /**< DSS_VID_CLUT_12 */
    uint32_t    dssClut13;            /**< DSS_VID_CLUT_13 */
    uint32_t    dssClut14;            /**< DSS_VID_CLUT_14 */
    uint32_t    dssClut15;            /**< DSS_VID_CLUT_15 */
    uint32_t    dssLumaKey;           /**< DSS_VID_LUMAKEY */
    uint32_t    dssDmaBufSize;        /**< DSS_VID_DMA_BUFSIZE */
    uint32_t    dssCrop;              /**< DSS_VID_CROP */
    uint32_t    dssSecure;            /**< DSS_VID_SECURE */
    uint32_t    dssPipeGo;            /**< DSS_VID_PIPE_GO */
} IpFma_DssVideoPipeRegs;

/**
 *  \brief Video pipe layer region registers.
 */
typedef struct
{
    uint32_t    dssBufSizeStatus;        /**< Dss buf size status register. */
    uint32_t    dssSafetyAttributes;     /**< Dss safety attributes.        */
    uint32_t    dssAttributes;           /**< DSS_VIDL_ATTRIBUTES */
    uint32_t    dssAttributes2;          /**< DSS_VIDL_ATTRIBUTES2 */
    uint32_t    dssBufThreshold;         /**< DSS_VIDL_BUF_THRESHOLD */
    uint32_t    dssCscCoef0;             /**< DSS_VIDL_CSC_COEF0 */
    uint32_t    dssCscCoef1;             /**< DSS_VIDL_CSC_COEF1 */
    uint32_t    dssCscCoef2;             /**< DSS_VIDL_CSC_COEF2 */
    uint32_t    dssCscCoef3;             /**< DSS_VIDL_CSC_COEF3 */
    uint32_t    dssCscCoef4;             /**< DSS_VIDL_CSC_COEF4 */
    uint32_t    dssCscCoef5;             /**< DSS_VIDL_CSC_COEF5 */
    uint32_t    dssCscCoef6;             /**< DSS_VIDL_CSC_COEF6 */
    uint32_t    dssGlobalAlpha;          /**< DSS_VIDL_GLOBAL_ALPHA */
    uint32_t    dssMflagThreshold;       /**< DSS_VIDL_MFLAG_THRESHOLD */
    uint32_t    dssCscCoef7;             /**< DSS_VIDL_CSC_COEF7 */
    uint32_t    dssFbdcAttributes;       /**< DSS_VIDL_FBDC_ATTRIBUTES */
    uint32_t    dssFbdcClearColor;       /**< DSS_VIDL_FBDC_CLEAR_COLOR */
    uint32_t    dssClut0;                /**< DSS_VIDL_CLUT_0 */
    uint32_t    dssClut1;                /**< DSS_VIDL_CLUT_1 */
    uint32_t    dssClut2;                /**< DSS_VIDL_CLUT_2 */
    uint32_t    dssClut3;                /**< DSS_VIDL_CLUT_3 */
    uint32_t    dssClut4;                /**< DSS_VIDL_CLUT_4 */
    uint32_t    dssClut5;                /**< DSS_VIDL_CLUT_5 */
    uint32_t    dssClut6;                /**< DSS_VIDL_CLUT_6 */
    uint32_t    dssClut7;                /**< DSS_VIDL_CLUT_7 */
    uint32_t    dssClut8;                /**< DSS_VIDL_CLUT_8 */
    uint32_t    dssClut9;                /**< DSS_VIDL_CLUT_9 */
    uint32_t    dssClut10;               /**< DSS_VIDL_CLUT_10 */
    uint32_t    dssClut11;               /**< DSS_VIDL_CLUT_11 */
    uint32_t    dssClut12;               /**< DSS_VIDL_CLUT_12 */
    uint32_t    dssClut13;               /**< DSS_VIDL_CLUT_13 */
    uint32_t    dssClut14;               /**< DSS_VIDL_CLUT_14 */
    uint32_t    dssClut15;               /**< DSS_VIDL_CLUT_15 */
    uint32_t    dssLumaKey;              /**< DSS_VIDL_LUMAKEY */
    uint32_t    dssDmaBufSize;           /**< DSS_VIDL_DMA_BUFSIZE */
    uint32_t    dssCrop;                 /**< DSS_VIDL_CROP */
    uint32_t    dssSecure;               /**< DSS_VIDL_SECURE */
    uint32_t    dssPipeGo;               /**< DSS_VIDL_PIPE_GO */
} IpFma_DssVideoPipeLayerRegs;

/**
 *  \brief Dss overlay region registers.
 */
typedef struct
{
    uint32_t dssConfig;              /**< DSS_OVR_CONFIG */
    uint32_t dssVirtualVp;           /**< DSS_OVR_VIRTUALVP */
    uint32_t dssDefaultColor;        /**< DSS_OVR_DEFAULT_COLOR */
    uint32_t dssDefaultColor2;       /**< DSS_OVR_DEFAULT_COLOR2 */
    uint32_t dssTransColorMax;       /**< DSS_OVR_TRANS_COLOR_MAX */
    uint32_t dssTransColorMax2;      /**< DSS_OVR_TRANS_COLOR_MAX2 */
    uint32_t dssTransColorMin;       /**< DSS_OVR_TRANS_COLOR_MIN */
    uint32_t dssTransColorMin2;      /**< DSS_OVR_TRANS_COLOR_MIN2 */
    uint32_t dssSecure;              /**< DSS_OVR_SECURE */
} IpFma_DssOverlayRegs;


/**
 *  \brief Dss video port region registers.
 */
typedef struct
{
    uint32_t dssConfig;              /**< DSS_VP_CONFIG */
    uint32_t dssControl;             /**< DSS_VP_CONTROL */
    uint32_t dssCscCoef0;            /**< DSS_VP_CSC_COEF0 */
    uint32_t dssCscCoef1;            /**< DSS_VP_CSC_COEF1 */
    uint32_t dssCscCoef2;            /**< DSS_VP_CSC_COEF2 */
    uint32_t dssDataCycle0;          /**< DSS_VP_DATA_CYCLE_0 */
    uint32_t dssDataCycle1;          /**< DSS_VP_DATA_CYCLE_1 */
    uint32_t dssDataCycle2;          /**< DSS_VP_DATA_CYCLE_2 */
    uint32_t dssLineNumber;          /**< DSS_VP_LINE_NUMBER */
    uint32_t dssPolFreq;             /**< DSS_VP_POL_FREQ */
    uint32_t dssSizeScreen;          /**< DSS_VP_SIZE_SCREEN */
    uint32_t dssTimingH;             /**< DSS_VP_TIMING_H */
    uint32_t dssTimingV;             /**< DSS_VP_TIMING_V */
    uint32_t dssCscCoef3;            /**< DSS_VP_CSC_COEF3 */
    uint32_t dssCscCoef4;            /**< DSS_VP_CSC_COEF4 */
    uint32_t dssCscCoef5;            /**< DSS_VP_CSC_COEF5 */
    uint32_t dssCscCoef6;            /**< DSS_VP_CSC_COEF6 */
    uint32_t dssCscCoef7;            /**< DSS_VP_CSC_COEF7 */
    uint32_t dssGammaTable0;         /**< DSS_VP_GAMMA_TABLE_0 */
    uint32_t dssGammaTable1;         /**< DSS_VP_GAMMA_TABLE_1 */
    uint32_t dssGammaTable2;         /**< DSS_VP_GAMMA_TABLE_2 */
    uint32_t dssGammaTable3;         /**< DSS_VP_GAMMA_TABLE_3 */
    uint32_t dssGammaTable4;         /**< DSS_VP_GAMMA_TABLE_4 */
    uint32_t dssGammaTable5;         /**< DSS_VP_GAMMA_TABLE_5 */
    uint32_t dssGammaTable6;         /**< DSS_VP_GAMMA_TABLE_6 */
    uint32_t dssGammaTable7;         /**< DSS_VP_GAMMA_TABLE_7 */
    uint32_t dssGammaTable8;         /**< DSS_VP_GAMMA_TABLE_8 */
    uint32_t dssGammaTable9;         /**< DSS_VP_GAMMA_TABLE_9 */
    uint32_t dssGammaTable10;        /**< DSS_VP_GAMMA_TABLE_10 */
    uint32_t dssGammaTable11;        /**< DSS_VP_GAMMA_TABLE_11 */
    uint32_t dssGammaTable12;        /**< DSS_VP_GAMMA_TABLE_12 */
    uint32_t dssGammaTable13;        /**< DSS_VP_GAMMA_TABLE_13 */
    uint32_t dssGammaTable14;        /**< DSS_VP_GAMMA_TABLE_14 */
    uint32_t dssGammaTable15;        /**< DSS_VP_GAMMA_TABLE_15 */
    uint32_t dssOldiConfig;          /**< DSS_VP_DSS_OLDI_CFG */
    uint32_t dssOldiStatus;          /**< DSS_VP_DSS_OLDI_STATUS */
    uint32_t dssOldiLb;              /**< DSS_VP_DSS_OLDI_LB */
    uint32_t dssSecure;              /**< DSS_VP_SECURE */
} IpFma_DssVideoPortRegs;

/**
 *  \brief Dss write-back pipe region registers.
 */
typedef struct
{
    uint32_t dssAttributes;        /**< DSS_WB_ATTRIBUTES */
    uint32_t dssAttributes2;       /**< DSS_WB_ATTRIBUTES2 */
    uint32_t dssBufThreshold;      /**< DSS_WB_BUF_THRESHOLD */
    uint32_t dssCscCoef0;          /**< DSS_WB_CSC_COEF0 */
    uint32_t dssCscCoef1;          /**< DSS_WB_CSC_COEF1 */
    uint32_t dssCscCoef2;          /**< DSS_WB_CSC_COEF2 */
    uint32_t dssCscCoef3;          /**< DSS_WB_CSC_COEF3 */
    uint32_t dssCscCoef4;          /**< DSS_WB_CSC_COEF4 */
    uint32_t dssCscCoef5;          /**< DSS_WB_CSC_COEF5 */
    uint32_t dssCscCoef6;          /**< DSS_WB_CSC_COEF6 */
    uint32_t dssMflagThreshold;    /**< DSS_WB_MFLAG_THRESHOLD */
    uint32_t dssSecure;            /**< DSS_WB_SECURE */
} IpFma_DssWriteBackPipeRegs;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This function gets dss common M region registers.
 *
 * \param   out		        Structure containing dss common M region registers states
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetCommonMRegs(IpFma_DssCommonMRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets dss common S region registers.
 *
 * \param   out		        Structure containing dss common S region registers states
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetCommonSRegs(IpFma_DssCommonSRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets video pipe region registers.
 *
 * \param   out             Structure containing video pipe region registers
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetVideoPipeRegs(IpFma_DssVideoPipeRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets video pipe layer region registers.
 *
 * \param   out             Structure containing video pipe layer region registers
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetVideoPipeLayerRegs(IpFma_DssVideoPipeLayerRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets dss overlay region registers.
 *
 * \param   out             Structure containing dss overlay region registers states
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetOverlayRegs(IpFma_DssOverlayRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets dss video port region registers.
 *
 * \param   out             Structure containing dss video port region registers states
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetVideoPortRegs(IpFma_DssVideoPortRegs* out, uint32_t instanceId);

/**
 * \brief   This function gets dss write-back pipe region registers.
 *
 * \param   out             Structure containing dss write-back pipe region registers states
 * \param   instanceId      Id of the instance's MMR region
 *
 * \retval  \ref    IPFMA_OK 		    Registers states read successfully
 * 		    \ref    IPFMA_E_PARAM       Registers states read unsuccessfully - Invalid parameter(s)
 */
IpFma_Status IpFma_Dss_GetWriteBackPipeRegs(IpFma_DssWriteBackPipeRegs* out, uint32_t instanceId);

/**
 * \brief   This function is used to compare dss common M region registers
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareCommonMRegs(const IpFma_DssCommonMRegs* expected, const IpFma_DssCommonMRegs* actual);

/**
 * \brief   This function is used to compare dss common S region registers
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareCommonSRegs(const IpFma_DssCommonSRegs* expected, const IpFma_DssCommonSRegs* actual);

/**
 * \brief   This function is used to compare video pipe region registers.
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareVideoPipeRegs(const IpFma_DssVideoPipeRegs* expected, const IpFma_DssVideoPipeRegs* actual);

/**
 * \brief   This function is used to compare video pipe layer region registers.
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareVideoPipeLayerRegs(const IpFma_DssVideoPipeLayerRegs* expected, const IpFma_DssVideoPipeLayerRegs* actual);

/**
 * \brief   This function is used to compare dss overlay region registers.
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareOverlayRegs(const IpFma_DssOverlayRegs* expected, const IpFma_DssOverlayRegs* actual);

/**
 * \brief   This function is used to compare dss video port region registers.
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareVideoPortRegs(const IpFma_DssVideoPortRegs* expected, const IpFma_DssVideoPortRegs* actual);

/**
 * \brief   This function is used to compare dss write-back pipe region registers.
 *
 * \param   expected    Previous status of dss registers
 * \param   actual      Current status of dss registers
 *
 * \retval  \ref        IPFMA_OK 		    Registers value has not been modified
 * 		    \ref        IPFMA_E_MISMATCH    Registers value modified
 */
IpFma_Status IpFma_Dss_CompareWriteBackPipeRegs(const IpFma_DssWriteBackPipeRegs* expected, const IpFma_DssWriteBackPipeRegs* actual);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_DSS */

/** @} */
