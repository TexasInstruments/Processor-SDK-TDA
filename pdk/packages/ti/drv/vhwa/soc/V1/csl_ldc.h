/* =============================================================================
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \file csl_ldc.h
 *
 *  \brief HAL Header file for LDC
 *      This file defines the HAL APIs for LDC module
 *
 */

#ifndef CSL_LDC_H_
#define CSL_LDC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <soc/vhwa_soc.h>
#include <include/ldc_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \brief Macro for the reset value of the PRIVATE_MEMSIZE status register
 *         for LDC. This value is used to compare the status register
 *         during VPAC-10 safety mechanism (status readback)
 */
#define VHWA_VPAC_LDC_PRIVATE_MEM_SIZE_RESET_VALUE   (0x000A2A2AU)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 *  CSL_ldcSetConfig
 *  \brief Sets the entire LDC configuration to the LDC registers.
 *
 *  \param coreRegs         LDC Base Address
 *  \param config           Pointer to VpsHal_issldcConfig structure
 *                          containing the register configurations.
 *                          This parameter should be non-NULL.
 *  \param arg              Not used, should be NULL
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetConfig(CSL_ldc_coreRegs *coreRegs,
                         const Ldc_Config *cfg);

/**
 *  CSL_ldcStart
 *  \brief LDC Api for enabling LDC module.
 *
 *  \param coreRegs         LDC Base Address
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcStart(CSL_ldc_coreRegs *coreRegs);

/**
 *  CSL_ldcStop
 *  \brief API for disabling LDC module.
 *         This is required when continuous mode is enabled, as hardware
 *         will not automatically clear LDC_EN bit at end of frame.
 *
 *  \param coreRegs         LDC Base Address
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcStop(CSL_ldc_coreRegs *coreRegs);

/**
 *  CSL_ldcSetBwLimitConfig
 *  \brief API for setting Bandwidth Limit config for Rd Dma.
 *
 *  \param coreRegs         LDC Base Address
 *  \param lutCfg           Pointer to LUT configuration structure
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetBwLimitConfig(
    CSL_ldc_coreRegs          *coreRegs,
    const Ldc_RdBwLimitConfig *bwLimitCfg);

/**
 *  CSL_ldcSoftReset
 *  \brief API for triggering soft resetting for LDC.
 *
 *  \param coreRegs         LDC Base Address
 *  \param arg              not used as of now
 */
void CSL_ldcSoftReset(CSL_ldc_coreRegs *coreRegs);

/**
 *  CSL_ldcIsResetDone
 *  \brief API to check if reset is done or not.
 *
 *  \param coreRegs         LDC Base Address
 *  \param arg              not used as of now
 *
 *  \return                 Returns UTRUE if reset is done
 *                                  UFALSE otherwise
 */
uint32_t CSL_ldcIsResetDone(const CSL_ldc_coreRegs *coreRegs);

/**
 *  CSL_ldcSetInAddress
 *  \brief API to set input Luma and Chroma address in LDC
 *
 *  \param coreRegs         LDC Base Address
 *  \param lumaAddr         Luma Buffer Address
 *  \param chromaAddr       Chroma Buffer Address
 *
 */
void CSL_ldcSetInAddress(CSL_ldc_coreRegs *coreRegs, uint64_t lumaAddr,
    uint64_t chromaAddr);

/**
 *  CSL_ldcSetInBuffOffset
 *  \brief API to set offset of Start co-ordinate of pixel data stored in input
 *         buffer
 *
 *  \param coreRegs         LDC Base Address
 *  \param xOffset          Horizontal pixel start position
 *  \param yOffset          Vertical pixel start position
 *
 */
void CSL_ldcSetInBuffOffset(CSL_ldc_coreRegs *coreRegs,
                                uint32_t xOffset,
                                uint32_t yOffset);
/*
 *  CSL_ldcSetLumaToneMapLutCfg
 *  \brief API to set ToneMapping configuration for Luma in LDC
 *
 *  \param coreRegs         LDC Base Address
 *  \param lutAddr          SoC Address of the Lut
 *  \param lutCfg           Luma Lut Configuration
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetLumaToneMapLutCfg(CSL_ldc_coreRegs *coreRegs,
    volatile uint32_t *lutAddr, const Ldc_RemapLutCfg *lutCfg);

/**
 *  CSL_ldcSetChromaToneMapLutCfg
 *  \brief API to set ToneMapping configuration for Chroma in LDC
 *
 *  \param coreRegs         LDC Base Address
 *  \param lutAddr          SoC Address of the Lut
 *  \param lutCfg           Chroma Lut Configuration
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetChromaToneMapLutCfg(CSL_ldc_coreRegs *coreRegs,
    volatile uint32_t *lutAddr, const Ldc_RemapLutCfg *lutCfg);


#ifdef __cplusplus
}
#endif

#endif  /* CSL_LDC_H_ */
