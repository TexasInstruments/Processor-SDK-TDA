/**
 *   Copyright (c) Texas Instruments Incorporated 2019
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
 *  \file csl_rawfe.h
 *
 *  \brief CSL Header file for RAWFE
 *      This file defines the CSL APIs for RAW FE module
 *
 */

#ifndef CSL_RAWFE_H_
#define CSL_RAWFE_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <soc/vhwa_soc.h>
#include <include/rawfe_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TODO, Questions:
 * 1, Output of the MA-1 block can be 20bits, but merge block can accept
 *    only data upto 16bits?? How is this converted to 16bits?
 * 2, In figure-1 for two exposure input. short exposure should be read
 *    on VS input channel and long exposure should be read on short
 *    input channel.. correct?
 * 3, Specs says -  Accesses to non shadowed registers or rams during
 *    active window can cause corruption. During an active phase, non
 *    blanking region, any acceses to a ram, read or write, will corrupt
 *    the output data. H3a rams, lsc lut, dpc lut rams are exceptions
 *    Does it mean we could write to lsc/dpc at any point of time?
 * 4, What is H3a RAMs
 * 5, Still to add support for ECC aggregator for all LUTs
 * 6, Still need to add support for handling of error interrupts
 * 7, H3a Lut and H3a accumulator cfg?? DPC Line memory??
 * 8, Still need to add support Debug events,
 * 9, h3a_line/accm and dpc_line are for debug purposes, not use
 *    configurable rams
 * a, all the registers of RAW FE are shadowed, so can be applied almost
 *    any time
 */


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Horizontal latency required for RAW FE
 */
#define CSL_RAWFE_HORZ_LATENCY                  (50U)

/** \brief Vertical Blanking required for DPC
 */
#define CSL_RAWFE_DPC_VERT_BLANKING             (2u)


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  struct CslRfe_Config
 *  \brief VPAC RAWFE Csl configuration,
 *         Currently this structure is used to configure VISS input frame size.
 *
 *         The other modules of the RAWFE are configured using different APIs
 */
typedef struct
{
    uint32_t                    width;
    /**< Input frame width */
    uint32_t                    height;
    /**< Input frame height */
} CslRfe_FrameConfig;

/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 *  \brief Sets the common configuration to the VISS registers.
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param config           Pointer to CslRfe_Config structure
 *                          containing input frame size.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetFrameConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const CslRfe_FrameConfig *cfg);

/**
 *  \brief Sets WDR Merge Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param wdrModule        ID of the WDR Merge instance
 *                          Only valid values are
 *                          #RFE_MODULE_WDR_MERGE_MA1 and
 *                          #RFE_MODULE_WDR_MERGE_MA2
 *  \param cfg              Pointer to Rfe_WdrConfig structure
 *                          containing the RAWFE WDR Merge configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetWdrConfig(const CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t wdrModule, const Rfe_WdrConfig *cfg);

/**
 *  \brief Sets PWL Decompanding Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param pwlModule        ID of the PWL module instance
 *                          Only valid values are
 *                          #RFE_MODULE_PWL1,
 *                          #RFE_MODULE_PWL2 or
 *                          #RFE_MODULE_PWL3
 *  \param cfg              Pointer to Rfe_PwlConfig structure
 *                          containing the RAWFE PWL configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetPwlConfig(const CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t pwlModule, const Rfe_PwlConfig *cfg);

/**
 *  \brief Sets Companding LUT Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param lutAddr          MMR Address of the LUT, where given
 *                          loopup table will be set.
 *  \param lutModule        ID of the Lut module instance
 *                          Only valid values are
 *                          #RFE_MODULE_DECOMP_LUT1,
 *                          #RFE_MODULE_DECOMP_LUT2 or
 *                          #RFE_MODULE_DECOMP_LUT3
 *                          #RFE_MODULE_COMP_LUT
 *  \param cfg              Pointer to Vhwa_LutConfig structure
 *                          containing the RAWFE Lut configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetLutConfig(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    uint32_t lutModule, const Vhwa_LutConfig *cfg);

/**
 *  \brief Sets LSC Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param lscLutAddr       Address of the LSC Lut, in which given lut
 *                          from lscConfig will be copied.
 *                          Must not be 0 if LSC is enabled.
 *  \param cfg              Pointer to Rfe_LscConfig structure
 *                          containing the RAWFE LSC configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetLscConfig(CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t *lscLutAddr, const Rfe_LscConfig *cfg);

/**
 *  \brief Sets DPC Otf Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Rfe_DpcOtfConfig structure
 *                          containing the RAWFE DPC Otf configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetDpcOtfConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_DpcOtfConfig *cfg);

/**
 *  \brief Sets DPC Lut Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param dpcLutAddr       Address of the DPC Lut RAM,
 *                          Must not be 0 if DPC Lut is enabled.
 *  \param cfg              Pointer to Rfe_DpcLutConfig structure
 *                          containing the RAWFE DPC Lut configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetDpcLutConfig(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *dpcLutAddr,
    const Rfe_DpcLutConfig *cfg);

/**
 *  \brief Sets WB Gain Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param cfg              Pointer to Rfe_GainOfstConfig structure
 *                          containing the RAWFE White Balance configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetWbConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_GainOfstConfig *cfg);

/**
 *  \brief Sets H3A Configuration
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param h3aLutAddr       H3A LUT Address
 *
 *  \param cfg              Pointer to Rfe_H3aConfig structure
 *                          containing the H3A configurations in RAWFE.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetH3aConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_H3aInConfig *cfg);

/**
 *  \brief Sets H3A LUT Configuration
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param h3aLutAddr       H3A LUT Address
 *
 *  \param cfg              Pointer to Rfe_H3aConfig structure
 *                          containing the H3A configurations in RAWFE.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetH3aLutConfig(CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t *h3aLutAddr, const Vhwa_LutConfig *lCfg);

#ifdef __cplusplus
}
#endif

#endif  /* CSL_RAWFE_H_ */
