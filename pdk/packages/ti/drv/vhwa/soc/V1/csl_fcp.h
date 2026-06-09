/**
 *   Copyright (c) Texas Instruments Incorporated 2021-2022
 *   All rights reserved.
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
 *  \file csl_fcp.h
 *
 *  \brief CSL Header file for FCP
 *      This file defines the HAL APIs for VPAC Flex CC module
 *
 */

#ifndef CSL_FCP_H_
#define CSL_FCP_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <soc/vhwa_soc.h>
#include <include/fcp_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TODO, Questions:
 * 1, offsets for the FCP and FCC registers need to be changed as they
 *    are not from Flex Cp base address
 */


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Horizontal latency required for Flex CFA
 */
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
#define CSL_FLEX_CFA_HORZ_LATENCY               (41u)
#else
#define CSL_FLEX_CFA_HORZ_LATENCY               (45u)
#endif

/** \brief Horizontal latency required for Flex CC
 */
#define CSL_FLEX_CC_HORZ_LATENCY                (25u)

/** \brief Horizontal Blanking required for Flex CC
 */
#define CSL_FLEX_CC_HORZ_BLANKING               (20u)

/** \brief Horizontal latency required for Flex Edge Enhancer
 */
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
#define CSL_FLEX_EE_HORZ_LATENCY                (26u)
#else
#define CSL_FLEX_EE_HORZ_LATENCY                (29u)
#endif

/** \brief Vertical Blanking required for Flex CC
 */
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
#define CSL_FLEX_CC_VERT_BLANKING               (0u)
#else
#define CSL_FLEX_CC_VERT_BLANKING               (3u)
#endif

#if defined (VHWA_VPAC_IP_REV_VPAC3L)
/** \brief Vertical latency required for Flex CFA
 */
#define CSL_FLEX_CFA_VERT_LATENCY               (3u)

/** \brief Vertical blanking required for Flex CFA
 */
#define CSL_FLEX_CFA_VERT_BLANKING               (1u)

/** \brief Vertical latency required for Flex Edge Enhancer
 */
#define CSL_FLEX_EE_VERT_LATENCY                (2u)

/** \brief Vertical Latency required for Flex CC
 */
#define CSL_FLEX_CC_VERT_LATENCY               (1u)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  struct CslFcp_Config
 *  \brief VPAC FCP CSL configuration,
 *         Currently this structure is used to configure VISS input frame size.
 *
 *         The other modules of the RAWFE are configured using different APIs
 */
typedef struct
{
    uint32_t                        width;
    /**< Input frame width */
    uint32_t                        height;
    /**< Input frame height */
} CslFcp_FrameConfig;

/**
 *  \brief structure for selecting outputs on five output channels
 */
typedef struct
{
    uint32_t luma12OutSel;
    /**< Select output on Luma12b output channel,
     *   #Fcp_Luma12OutSelect */
    uint32_t chroma12OutSel;
    /**< Select output on Chroma12b output channel,
     *   #Fcp_Chroma12OutSelect */
    uint32_t luma8OutSel;
    /**< Select output on Luma8b output channel,
     *   #Fcp_Luma8OutSelect */
    uint32_t chroma8OutSel;
    /**< Select output on Chroma8b output channel,
     *   #Fcp_Chroma8OutSelect   */
    uint32_t sat8OutSel;
    /**< Select output on Saturation output channel,
     *   #Fcp_Sat8OutSelect      */
    uint32_t chromaMode;
    /**< Used to select 420 or 422 output,
     *   #Fcp_ChromaMode         */
} CslFcp_OutputSelect;


/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 *  \brief Sets the common configuration like frame size etc.
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to CslFcp_Config structure
 *                          currently contains just frame size.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetFrameConfig(CSL_flexcfaRegs *cfaRegs,
    CSL_flexcc_cfgRegs *fcpRegs, const CslFcp_FrameConfig *cfg);

/**
 *  \brief Sets CFA configuration
 *         Sets FIR coefficients, gradient threshold etc.
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_CfaConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetCfaConfig(CSL_flexcfaRegs *cfaRegs, const Fcp_CfaConfig *cfg);

/**
 *  \brief Sets Companding configuration
 *         This companding module converts GLBCE 16b output to 12b and
 *         give input to CFA
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param compLutAddr      Memory mapped address of the Lut
 *  \param cfg              Pointer to VpsVpac_LutConfig structure
 *                          Output clip is not used from this structure as
 *                          output is always 12b.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetCompConfig(CSL_flexcfaRegs *cfaRegs,
    const Vhwa_LutConfig *cfg);

/**
 *  \brief Sets color conversion configuration
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to VpsVpac_LutConfig structure
 *                          Output clip is not used from this structure as
 *                          output is always 12b.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetCcmConfig(CSL_flexcc_cfgRegs *fcpRegs,
    const Fcp_CcmConfig *cfg);

/**
 *  \brief Sets gamma correct/contrast stretch configuration
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_GammaConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CLS_fcpSetGammaConfig(CSL_flexcc_cfgRegs *fcpRegs, uint32_t lut1Addr[],
    uint32_t lut2Addr[], uint32_t lut3Addr[], const Fcp_GammaConfig *cfg);

/**
 *  \brief Sets RGB2HSV configuration
 *         Used for converting 12b RGB to 12b Saturation and grey scale
 *         Hue is not supported.
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_Rgb2HsvConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetRgb2HsvConfig(CSL_flexcc_cfgRegs *fcpRegs,
    const Fcp_Rgb2HsvConfig *cfg);

/**
 *  \brief Sets RGB2YUV configuration
 *         Used for converting 12b RGB to 12b YUV
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_Rgb2YuvConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CLS_fcpSetRgb2YuvConfig(CSL_flexcc_cfgRegs *fcpRegs,
    const Fcp_Rgb2YuvConfig *cfg);

/**
 *  \brief Sets histogram configuration
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_HistConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetHistogramConfig(CSL_flexcc_cfgRegs *fcpRegs,
    const Fcp_HistConfig *cfg);

/**
 *  \brief Reads current histogram from the register
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param hist             Histogram array of size #FCP_HISTOGRAM_SIZE
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpReadHistogram(const CSL_flexcc_histRegs *histRegs,
    uint32_t histData[]);

/**
 *  \brief Sets RGB Companding Lut
 *         This Lut is used for converting RGB 12b to 8b.
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param lut1Addr         Memory mapped base address of lut for color red
 *                          Should not be used if lut is enabled.
 *  \param lut2Addr         Memory mapped base address of lut for color green
 *                          Should not be used if lut is enabled.
 *  \param lut3Addr         Memory mapped base address of lut for color blue
 *                          Should not be used if lut is enabled.
 *  \param cfg              Pointer to Fcp_RgbLutConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetRgbLutConfig(CSL_flexcc_cfgRegs *fcpRegs, uint32_t lut1Addr[],
    uint32_t lut2Addr[], uint32_t lut3Addr[], const Fcp_RgbLutConfig *cfg);

/**
 *  \brief Sets YUV/Saturation Companding Lut
 *         This Lut is used for converting YUV/Saturation 12b to 8b.
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param lut1Addr         Memory mapped base address of lut for color Luma
 *                          Should not be used if lut is enabled.
 *  \param lut2Addr         Memory mapped base address of lut for color Chroma
 *                          Should not be used if lut is enabled.
 *  \param lut3Addr         Memory mapped base address of lut for color Saturation
 *                          Should not be used if lut is enabled.
 *  \param cfg              Pointer to Fcp_YuvSatLutConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetYuvSatLutConfig(CSL_flexcc_cfgRegs *fcpRegs,
    uint32_t lut1Addr[], uint32_t lut2Addr[], uint32_t lut3Addr[],
    const Fcp_YuvSatLutConfig *cfg);

/**
 *  \brief Sets output muxes
 *
 *  \param fcpBaseAddr      FCP Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_OutputSelect structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetOutputMux(CSL_flexcc_cfgRegs *fcpRegs,
    const CslFcp_OutputSelect *cfg);

/**
 *  \brief Sets CFA Comp LUT configuration
 *
 *  \param cfaRegs          CFA Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_comDecomLutConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetCfaCompLut(CSL_flexcfaRegs *cfaRegs,
                            const Fcp_comDecomLutConfig *cfg);

/**
 *  \brief Sets CFA DeComp LUT configuration
 *
 *  \param cfaRegs          CFA Base Address,
 *                          This parameter should not be 0.
 *  \param cfaDLutRegs      CFA DLUT Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Fcp_comDecomLutConfig structure
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpSetCfaDeCompLut(CSL_flexcfaRegs *cfaRegs,
                               CSL_flexcfa_dlutsRegs *cfaDLutRegs,
                               const Fcp_comDecomLutConfig *cfg);

/**
 *  \brief Changes the active histogram bank
 *
 *  \param fcpRegs          FCP Base Address,
 *                          This parameter should not be 0.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_fcpHistChangeBank(CSL_flexcc_cfgRegs *fcpRegs);

#ifdef __cplusplus
}
#endif

#endif  /* CSL_FCP_H_ */
