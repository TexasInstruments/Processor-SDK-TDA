/**
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \file csl_rawfe.c
 *
 *  \brief VISS RAW FE Csl file, setting up different modules of RAW FE
 *
 */


/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <soc/V1/csl_rawfe.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* WDR Merge register offsets, there are two instances of the WDR Merge block.
 * These offsets are same in both the modules and used for configuring
 * both the modules
 */
#define RFE_WDRMRG_CFG                          (CSL_RAWFE_CFG_WDRMRG1_CFG - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_GAIN                         (CSL_RAWFE_CFG_WDRMRG1_GAIN - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_LBLK12                       (CSL_RAWFE_CFG_WDRMRG1_LBLK12 -\
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_LBLK34                       (CSL_RAWFE_CFG_WDRMRG1_LBLK34 -\
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_SBLK12                       (CSL_RAWFE_CFG_WDRMRG1_SBLK12 -\
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_SBLK34                       (CSL_RAWFE_CFG_WDRMRG1_SBLK34 -\
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_LWB12                        (CSL_RAWFE_CFG_WDRMRG1_LWB12 - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_LWB34                        (CSL_RAWFE_CFG_WDRMRG1_LWB34 - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_SWB12                        (CSL_RAWFE_CFG_WDRMRG1_SWB12 - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_SWB34                        (CSL_RAWFE_CFG_WDRMRG1_SWB34 - \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_WDRTHR_BF                    (                              \
    CSL_RAWFE_CFG_WDRMRG1_WDRTHR_BF - CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_AF                           (CSL_RAWFE_CFG_WDRMRG1_AF -    \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_MA                           (CSL_RAWFE_CFG_WDRMRG1_MA -    \
    CSL_RAWFE_CFG_WDRMRG1_CFG)
#define RFE_WDRMRG_CLIP_SFT                                                    \
    (CSL_RAWFE_CFG_WDRMRG1_CLIP_SFT - CSL_RAWFE_CFG_WDRMRG1_CFG)

/* PWL register offsets, there are three instances of the PWL modules.
 * These offsets are same in both the modules and used for configuring
 * both the modules
 */
#define RFE_PWL_MASK_SH                         (CSL_RAWFE_CFG_PWL1_MASK_SH -  \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_EN                              (CSL_RAWFE_CFG_PWL1_EN -       \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_THRX12                          (CSL_RAWFE_CFG_PWL1_THRX12 -   \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_THRX3                           (CSL_RAWFE_CFG_PWL1_THRX3 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_THRY1                           (CSL_RAWFE_CFG_PWL1_THRY1 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_THRY2                           (CSL_RAWFE_CFG_PWL1_THRY2 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_THRY3                           (CSL_RAWFE_CFG_PWL1_THRY3 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_SLP12                           (CSL_RAWFE_CFG_PWL1_SLP12 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_SLP34                           (CSL_RAWFE_CFG_PWL1_SLP34 -    \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL_SLPSH_CLIP                                                     \
    (CSL_RAWFE_CFG_PWL1_SLPSH_CLIP - CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_OFF1                           (CSL_RAWFE_CFG_PWL1_OFF1 -     \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_OFF2                           (CSL_RAWFE_CFG_PWL1_OFF2 -     \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_OFF3                           (CSL_RAWFE_CFG_PWL1_OFF3 -     \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_OFF4                           (CSL_RAWFE_CFG_PWL1_OFF4 -     \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL23_OFF1                          (CSL_RAWFE_CFG_PWL2_OFF1 -     \
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL23_OFF2                          (CSL_RAWFE_CFG_PWL2_OFF2 -     \
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL23_OFF3                          (CSL_RAWFE_CFG_PWL2_OFF3 -     \
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL23_OFF4                          (CSL_RAWFE_CFG_PWL2_OFF4 -     \
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL1_WB12                           (CSL_RAWFE_CFG_PWL1_WB_GAIN12 -\
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_WB34                           (CSL_RAWFE_CFG_PWL1_WB_GAIN34 -\
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL23_WB12                          (CSL_RAWFE_CFG_PWL2_WB_GAIN12 -\
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL23_WB34                          (CSL_RAWFE_CFG_PWL2_WB_GAIN34 -\
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL1_LUT                            (CSL_RAWFE_CFG_PWL1_LUT -      \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL1_LUTCLIP                        (CSL_RAWFE_CFG_PWL1_LUTCLIP -  \
    CSL_RAWFE_CFG_PWL1_MASK_SH)
#define RFE_PWL23_LUT                           (CSL_RAWFE_CFG_PWL2_LUT -      \
    CSL_RAWFE_CFG_PWL2_MASK_SH)
#define RFE_PWL23_LUTCLIP                       (CSL_RAWFE_CFG_PWL2_LUTCLIP -  \
    CSL_RAWFE_CFG_PWL2_MASK_SH)

#define RAWFE_MAX_INPUT_WIDTH                                                  \
    (CSL_RAWFE_CFG_IMAGE_CFG_WIDTH_MASK >> CSL_RAWFE_CFG_IMAGE_CFG_WIDTH_SHIFT)
#define RAWFE_MAX_INPUT_HEIGHT                                                 \
    (CSL_RAWFE_CFG_IMAGE_CFG_HEIGHT_MASK >> CSL_RAWFE_CFG_IMAGE_CFG_HEIGHT_SHIFT)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Local function to set Companding Lut configuration in the register
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param lutAddr          MMR address of the companding lut.
 *                          This parameter should not be 0
 *                          if companding lut is enabled.
 *  \param config           Pointer to #Vhwa_LutConfig structure
 *                          containing Lut configuration.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
static void RfeSetMergeLut(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    const Vhwa_LutConfig *cfg);

/**
 *  \brief Local function to set decompanding Lut configuration in the register
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param lutAddr          MMR address of the decompading lut.
 *                          This parameter should not be 0
 *                          if decompanding lut is enabled.
 *  \param lutModule        Decompanding module to be enabled/disabled.
 *                          Valid values are RFE_MODULE_DECOMP_LUT1,
 *                          RFE_MODULE_DECOMP_LUT2, RFE_MODULE_DECOMP_LUT3
 *  \param config           Pointer to Vhwa_LutConfig structure
 *                          containing companding configuration.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
static void RfeSetPwlLut(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    uint32_t lutModule, const Vhwa_LutConfig *cfg);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */


/**
 *  Sets the common configuration to the RAW FE registers.
 */
int32_t CSL_rfeSetFrameConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const CslRfe_FrameConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    {
        if ((RAWFE_MAX_INPUT_HEIGHT < cfg->height) ||
            (RAWFE_MAX_INPUT_WIDTH < cfg->width))
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }

    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&rfeRegs->IMAGE_CFG);
        CSL_FINS(regVal, RAWFE_CFG_IMAGE_CFG_HEIGHT, cfg->height);
        CSL_FINS(regVal, RAWFE_CFG_IMAGE_CFG_WIDTH, cfg->width);
        CSL_REG32_WR(&rfeRegs->IMAGE_CFG, regVal);
    }

    return (status);
}

/* Sets WDR Merge Settings */
int32_t CSL_rfeSetWdrConfig(const CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t wdrModule, const Rfe_WdrConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    volatile uint32_t regVal;
    uint32_t wdrBaseAddr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs) ||
        ((RFE_MODULE_WDR_MERGE_MA1 != wdrModule) &&
            (RFE_MODULE_WDR_MERGE_MA2 != wdrModule)))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        if (RFE_MODULE_WDR_MERGE_MA1 == wdrModule)
        {
            wdrBaseAddr = (uint32_t)&rfeRegs->WDRMRG1_CFG;
        }
        else
        {
            wdrBaseAddr = (uint32_t)&rfeRegs->WDRMRG2_CFG;
        }

        if ((uint32_t)UTRUE == cfg->enable)
        {
            /* Set the WDR Merge Gains */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_GAIN);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_GAIN_GSHORT, cfg->gshort);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_GAIN_GLONG, cfg->glong);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_GAIN), regVal);

            /* Set Black Level for the long exposure */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_LBLK12);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LBLK12_LBK00, cfg->lbk[0U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LBLK12_LBK01, cfg->lbk[1U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_LBLK12), regVal);

            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_LBLK34);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LBLK34_LBK10, cfg->lbk[2U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LBLK34_LBK11, cfg->lbk[3U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_LBLK34), regVal);

            /* Set Black Level for the short exposure */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_SBLK12);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SBLK12_SBK00, cfg->sbk[0U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SBLK12_SBK01, cfg->sbk[1U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_SBLK12), regVal);

            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_SBLK34);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SBLK34_SBK10, cfg->sbk[2U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SBLK34_SBK11, cfg->sbk[3U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_SBLK34), regVal);

            /* WDR merge WB gain 1 and 2 for long frame */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_LWB12);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LWB12_WB00, cfg->lwb[0U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LWB12_WB01, cfg->lwb[1U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_LWB12), regVal);

            /* WDR merge WB gain 3 and 4 for long frame */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_LWB34);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LWB34_WB10, cfg->lwb[2U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_LWB34_WB11, cfg->lwb[3U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_LWB34), regVal);

            /* WDR merge WB gain 1 and 2 for short frame */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_SWB12);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SWB12_WB00, cfg->swb[0U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SWB12_WB01, cfg->swb[1U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_SWB12), regVal);

            /* WDR merge WB gain 3 and 4 for short frame */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_SWB34);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SWB34_WB10, cfg->swb[2U]);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_SWB34_WB11, cfg->swb[3U]);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_SWB34), regVal);

            /* Set the WDR merge parameter WDRTHR and BF */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_WDRTHR_BF);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_WDRTHR_BF_WDRTHR, cfg->wdrThr);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_WDRTHR_BF_BF, cfg->bf);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_WDRTHR_BF), regVal);

            /* Set the WDR merge parameter AF */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_AF);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_AF_AFM, cfg->afm);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_AF_AFE, cfg->afe);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_AF), regVal);

            /* Set WDR merge parameter MA */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_MA);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_MA_MAD, cfg->mad);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_MA_MAS, cfg->mas);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_MA), regVal);

            /* Set WDR merge clip value and shift before weight block */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_CLIP_SFT);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CLIP_SFT_CLIP, cfg->mergeClip);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CLIP_SFT_WTSFT, cfg->mergeShift);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_CLIP_SFT), regVal);
        }

        if ((uint32_t)UTRUE == cfg->enable)
        {
            /* Set WDR Merge configuration */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_CFG);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_SBIT, cfg->sbit);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_LBIT, cfg->lbit);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_DST, cfg->dst);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_WGT_SEL,
                cfg->useShortExpForWgtCalc);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_EN, 1U);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_CFG), regVal);
        }
        else
        {
            /* Set WDR Merge configuration */
            regVal = CSL_REG32_RD(wdrBaseAddr + RFE_WDRMRG_CFG);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_SBIT, cfg->sbit);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_LBIT, cfg->lbit);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_DST, cfg->dst);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_WGT_SEL,
                cfg->useShortExpForWgtCalc);
            CSL_FINS(regVal, RAWFE_CFG_WDRMRG1_CFG_CFG_EN, 0U);
            CSL_REG32_WR((wdrBaseAddr + RFE_WDRMRG_CFG), regVal);
            /* Disable/bypass the merge block */
            CSL_REG32_FINS((wdrBaseAddr + RFE_WDRMRG_CFG),
                RAWFE_CFG_WDRMRG1_CFG_CFG_EN, 0U);
        }
    }

    return (status);
}

/* Sets PWL Decompanding Settings */
int32_t CSL_rfeSetPwlConfig(const CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t pwlModule, const Rfe_PwlConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    volatile uint32_t regVal, pwlBaseAddr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs) ||
        ((RFE_MODULE_PWL1 != pwlModule) &&
            (RFE_MODULE_PWL2 != pwlModule) &&
            (RFE_MODULE_PWL3 != pwlModule)))
    {
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Assuming
            PWL1 is for VS input
            PWL2 is for Short input
            PWL3 is for Long input
           TODO: Confirm this understanding
         */
        if (RFE_MODULE_PWL3 == pwlModule)
        {
            pwlBaseAddr = (uint32_t)&rfeRegs->PWL3_MASK_SH;
        }
        else if (RFE_MODULE_PWL2 == pwlModule)
        {
            pwlBaseAddr = (uint32_t)&rfeRegs->PWL2_MASK_SH;
        }
        else
        {
            pwlBaseAddr = (uint32_t)&rfeRegs->PWL1_MASK_SH;
        }

        /* Set Shift/Mask and DC Sub even if PWL is disabled */
        regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL_MASK_SH);
        CSL_FINS(regVal, RAWFE_CFG_PWL1_MASK_SH_MASK, cfg->mask);
        CSL_FINS(regVal, RAWFE_CFG_PWL1_MASK_SH_SHIFT, cfg->shift);
        CSL_REG32_WR((pwlBaseAddr + RFE_PWL_MASK_SH), regVal);

        if (RFE_MODULE_PWL1 == pwlModule)
        {
            /* White Balance Gains */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL1_WB12);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN12_WB_GAIN00, cfg->gain[0U]);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN12_WB_GAIN01, cfg->gain[1U]);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL1_WB12), regVal);
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL1_WB34);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN34_WB_GAIN10, cfg->gain[2U]);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN34_WB_GAIN11, cfg->gain[3U]);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL1_WB34), regVal);

            /* Set Black level in DC Sub register */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL1_OFF1),
                RAWFE_CFG_PWL1_OFF1_OFST00, cfg->offset[0U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL1_OFF2),
                RAWFE_CFG_PWL1_OFF2_OFST01, cfg->offset[1U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL1_OFF3),
                RAWFE_CFG_PWL1_OFF3_OFST10, cfg->offset[2U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL1_OFF4),
                RAWFE_CFG_PWL1_OFF4_OFST11, cfg->offset[3U]);
        }
        else
        {
            /* White Balance Gains */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL23_WB12);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN12_WB_GAIN00, cfg->gain[0U]);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN12_WB_GAIN01, cfg->gain[1U]);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL23_WB12), regVal);
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL23_WB34);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN34_WB_GAIN10, cfg->gain[2U]);
            CSL_FINS(regVal, RAWFE_CFG_PWL3_WB_GAIN34_WB_GAIN11, cfg->gain[3U]);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL23_WB34), regVal);

            /* Set Black level in DC Sub register */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL23_OFF1),
                RAWFE_CFG_PWL1_OFF1_OFST00, cfg->offset[0U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL23_OFF2),
                RAWFE_CFG_PWL1_OFF2_OFST01, cfg->offset[1U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL23_OFF3),
                RAWFE_CFG_PWL1_OFF3_OFST10, cfg->offset[2U]);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL23_OFF4),
                RAWFE_CFG_PWL1_OFF4_OFST11, cfg->offset[3U]);
        }

        {
            /* Set PWL threshold X1 and X2 - Unsigned */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL_THRX12);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_THRX12_THR_X1, cfg->xthr1);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_THRX12_THR_X2, cfg->xthr2);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL_THRX12), regVal);

            /* Set PWL threshold X3 - Unsigned */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_THRX3),
                RAWFE_CFG_PWL1_THRX3_THR_X3, cfg->xthr3);

            /* Set PWL threshold Y1, Y2 and Y3 - Unsigned */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_THRY1),
                RAWFE_CFG_PWL1_THRY1_THR_Y1, cfg->ythr1);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_THRY2),
                RAWFE_CFG_PWL1_THRY2_THR_Y2, cfg->ythr2);
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_THRY3),
                RAWFE_CFG_PWL1_THRY3_THR_Y3, cfg->ythr3);

            /* Set the PWL slope 1 and 2 - Unsigned */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL_SLP12);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLP12_SLOPE_1, cfg->slope1);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLP12_SLOPE_2, cfg->slope2);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL_SLP12), regVal);

            /* Set the PWL slope 3 and 4 - Unsigned */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL_SLP34);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLP34_SLOPE_3, cfg->slope3);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLP34_SLOPE_4, cfg->slope4);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL_SLP34), regVal);

            /* PWL slope shift and clip */
            regVal = CSL_REG32_RD(pwlBaseAddr + RFE_PWL_SLPSH_CLIP);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLPSH_CLIP_SLOPE_SHIFT,
                cfg->slopeShift);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_SLPSH_CLIP_CLIP, cfg->outClip);
            CSL_REG32_WR((pwlBaseAddr + RFE_PWL_SLPSH_CLIP), regVal);
        }

        if ((uint32_t)UTRUE == cfg->enable)
        {
            /* Enable PWL Module */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_EN), RAWFE_CFG_PWL1_EN_ENABLE,
                1U);
        }
        else
        {
            /* Disable/Bypass the module */
            CSL_REG32_FINS((pwlBaseAddr + RFE_PWL_EN), RAWFE_CFG_PWL1_EN_ENABLE,
                0U);
        }
    }

    return (status);
}

/**
 *  \brief Sets Companding LUT Settings
 *
 *  \param rfeRegs          RFE Base Address,
 *                          This parameter should not be 0.
 *  \param lutModule        ID of the Lut module instance
 *                          Only valid values are
 *                          #RFE_MODULE_DECOMP_LUT_LONG,
 *                          #RFE_MODULE_DECOMP_LUT_SHORT_LONG or
 *                          #RFE_MODULE_DECOMP_LUT_VSHORT_SHORT
 *                          #RFE_MODULE_COMP_LUT
 *  \param cfg              Pointer to Vhwa_LutConfig structure
 *                          containing the RAWFE Lut configurations.
 *                          This parameter should not be NULL.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_rfeSetLutConfig(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    uint32_t lutModule, const Vhwa_LutConfig *cfg)
{
    int32_t  status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs) ||
        ((RFE_MODULE_DECOMP_LUT1 != lutModule) &&
            (RFE_MODULE_DECOMP_LUT2 != lutModule) &&
            (RFE_MODULE_DECOMP_LUT3 != lutModule) &&
            (RFE_MODULE_COMP_LUT != lutModule)))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> lutAddr is passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to lutAddr register, instObj is defined as a global gM2mVissInstObj.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
            if ((NULL == lutAddr) || (NULL == cfg->tableAddr))
            /* LDRA_JUSTIFY_END */
            {
                status = FVID2_EBADARGS;
            }
        }
    }

    if (FVID2_SOK == status)
    {
        if (RFE_MODULE_COMP_LUT == lutModule)
        {
            RfeSetMergeLut(rfeRegs, lutAddr, cfg);
        }
        else
        {
            RfeSetPwlLut(rfeRegs, lutAddr, lutModule, cfg);
        }
    }

    return (status);
}

/* Sets LSC Settings */
int32_t CSL_rfeSetLscConfig(CSL_rawfe_cfgRegs *rfeRegs,
    uint32_t *lscLutAddr, const Rfe_LscConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt, val;
    volatile uint32_t regVal, *regAddr;
    uint32_t *lutAddr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
           if (cfg->numTblEntry > RFE_LSC_TBL_SIZE)
           {
               status = FVID2_EINVALID_PARAMS;
           }
           /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
           if (NULL == lscLutAddr)
           {
               status = FVID2_EBADARGS;
           }
           /* LDRA_JUSTIFY_END */
        }
    }

    if (FVID2_SOK == status)
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            regAddr = lscLutAddr;
            lutAddr = cfg->tableAddr;

            uint32_t table_entry = cfg->numTblEntry;

            #if defined (VHWA_VPAC_IP_REV_VPAC3L)/* For VPAC3L the table entries are to be packed if channel mode is not 0*/
            if(cfg->chn_mode == RFE_LSC_CHNMODE_4_LEGACY)
            {
                /* Do nothing */
            }
            else
            {
                table_entry = table_entry/4;
            }
            #endif
 
            for (cnt = 0U; cnt < table_entry; cnt ++)
            {
                #if defined (VHWA_VPAC_IP_REV_VPAC3L)/* For VPAC3L the table entries are to be packed if channel mode is not 0*/
                if(cfg->chn_mode == RFE_LSC_CHNMODE_4_LEGACY)
                {
                    val = *lutAddr;
                }
                else
                {
                    val = (*lutAddr) & 0xFFu;
                    lutAddr ++;
                    val |= ((*lutAddr) & 0xFFu) << 8u;
                    lutAddr ++;
                    val |= ((*lutAddr) & 0xFFu) << 16u;
                    lutAddr ++;
                    val |= ((*lutAddr) & 0xFFu) << 24u;
                }
                #else
                    val = *lutAddr;
                #endif
                lutAddr ++;
                CSL_REG32_WR(regAddr, val);

                regAddr ++;
            }

            regVal = CSL_REG32_RD(&rfeRegs->LSC_CFG);
            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
            CSL_FINS(regVal, RAWFE_CFG_LSC_CFG_CHN_MODE, (uint32_t)cfg->chn_mode);
            #endif
            CSL_FINS(regVal, RAWFE_CFG_LSC_CFG_GAIN_FORMAT, (uint32_t)cfg->gainFmt);
            CSL_FINS(regVal, RAWFE_CFG_LSC_CFG_MODE_M, (uint32_t)cfg->horzDsFactor);
            CSL_FINS(regVal, RAWFE_CFG_LSC_CFG_MODE_N, (uint32_t)cfg->vertDsFactor);
            CSL_FINS(regVal, RAWFE_CFG_LSC_CFG_EN, 1U);
            CSL_REG32_WR(&rfeRegs->LSC_CFG, regVal);

#if defined (VHWA_VPAC_IP_REV_VPAC3L)
            /* Set LUT MAP values */
            regAddr = &rfeRegs->LSC_LUT_MAP07;
            for (cnt = 0U; cnt < RFE_LSC_LUT_MAP_SIZE/8; cnt ++)
            {
                regVal = 0;
                /* Using Mask and offset from MAP07 register as they are
                    the same for MAP815 */
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP0, cfg->lut_map[cnt*8 + 0]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP1, cfg->lut_map[cnt*8 + 1]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP2, cfg->lut_map[cnt*8 + 2]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP3, cfg->lut_map[cnt*8 + 3]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP4, cfg->lut_map[cnt*8 + 4]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP5, cfg->lut_map[cnt*8 + 5]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP6, cfg->lut_map[cnt*8 + 6]);
                CSL_FINS(regVal, RAWFE_CFG_LSC_LUT_MAP07_MAP7, cfg->lut_map[cnt*8 + 7]);
                CSL_REG32_WR(regAddr, regVal);

                regAddr++;
            }
#endif
        }
        else
        {
            CSL_REG32_FINS(&rfeRegs->LSC_CFG, RAWFE_CFG_LSC_CFG_EN, 0U);
        }
    }

    return (status);
}

/* Sets DPC Otf Settings */
int32_t CSL_rfeSetDpcOtfConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_DpcOtfConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;
    volatile uint32_t regVal, *regAddr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            regAddr = &rfeRegs->OTFDPC_THRSLP1;

            for (cnt = 0U; cnt < RFE_DPC_OTF_LUT_SIZE; cnt ++)
            {
                regVal = CSL_REG32_RD(regAddr);
                /* Using Mask and offset from THRSLP1 register as they are
                   same for all thresholds and slopes */
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_THRSLP1_THR1, cfg->threshold[cnt]);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_THRSLP1_SLP1, cfg->slope[cnt]);
                CSL_REG32_WR(regAddr, regVal);

                regAddr = regAddr + 1U;
            }

#if defined (VHWA_VPAC_IP_REV_VPAC3L)
            /* RAWFE CFA CFG*/
            regAddr = &rfeRegs->CFA_CFG;
            regVal = CSL_REG32_RD(regAddr);
            CSL_FINS(regVal, RAWFE_CFG_CFA_CFG_PHASE, cfg->cfa_phase);
            CSL_FINS(regVal, RAWFE_CFG_CFA_CFG_MODE, cfg->cfa_mode);
            CSL_REG32_WR(regAddr, regVal);
            
            /* THRX */
            regAddr = &rfeRegs->OTFDPC_THRX12;
            for (cnt = 0U; cnt < RFE_DPC_OTF_LUT_SIZE/2; cnt ++)
            {
                regVal = CSL_REG32_RD(regAddr);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_THRX12_THRX1, cfg->thrx[cnt*2]);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_THRX12_THRX2, cfg->thrx[cnt*2 + 1]);
                CSL_REG32_WR(regAddr, regVal);
                
                regAddr = regAddr + 1U;
            }
            
            /* LUT2 threshold and slope */
            regAddr = &rfeRegs->OTFDPC_LUT2_THRSLP1;
            for (cnt = 0U; cnt < RFE_DPC_OTF_LUT_SIZE; cnt ++)
            {
                regVal = CSL_REG32_RD(regAddr);
                /* Using Mask and offset from THRSLP1 register as they are
                same for all thresholds and slopes */
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_LUT2_THRSLP1_THR1, cfg->lut2_threshold[cnt]);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_LUT2_THRSLP1_SLP1, cfg->lut2_slope[cnt]);
                CSL_REG32_WR(regAddr, regVal);
                
                regAddr = regAddr + 1U;
            }
            
            /* LUT2 THRX */
            regAddr = &rfeRegs->OTFDPC_LUT2_THRX12;
            for (cnt = 0U; cnt < RFE_DPC_OTF_LUT_SIZE/2; cnt ++)
            {
                regVal = CSL_REG32_RD(regAddr);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_LUT2_THRX12_THRX1, cfg->lut2_thrx[cnt*2]);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_LUT2_THRX12_THRX2, cfg->lut2_thrx[cnt*2 + 1]);
                CSL_REG32_WR(regAddr, regVal);

                regAddr = regAddr + 1U;
            }
#endif
            /* Enable DPC OTF */
            #if defined (VHWA_VPAC_IP_REV_VPAC3L)
                regAddr = &rfeRegs->OTFDPC_EN;
                regVal = CSL_REG32_RD(regAddr);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_EN_EN, 1U);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_EN_STATS_CFG, 1U);
                CSL_FINS(regVal, RAWFE_CFG_OTFDPC_EN_LUT_MAP, cfg->lut_map);
                CSL_REG32_WR(regAddr, regVal);
            #else
                CSL_REG32_FINS(&rfeRegs->OTFDPC_EN, RAWFE_CFG_OTFDPC_EN_EN, 1U);
            #endif

        }
        else
        {
            /* Disable DPC OTF */
            CSL_REG32_FINS(&rfeRegs->OTFDPC_EN, RAWFE_CFG_OTFDPC_EN_EN, 0U);
        }
    }

    return (status);
}

/* Sets DPC Lut Settings */
int32_t CSL_rfeSetDpcLutConfig(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *dpcLutAddr,
    const Rfe_DpcLutConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;
    uint32_t *pDpcLutAddr = NULL;
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            if ((cfg->maxDefectPixels > RFE_DPC_LUT_SIZE) ||
                (0U == cfg->maxDefectPixels))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == dpcLutAddr)
            {
                status = FVID2_EBADARGS;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                pDpcLutAddr = dpcLutAddr;
            }
        }
    }

    if (FVID2_SOK == status)
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            /* Set the defective pixels Lookup table */
            for (cnt = 0U; cnt < cfg->maxDefectPixels; cnt ++)
            {
                CSL_REG32_FINS(pDpcLutAddr, RAWFE_DPC_LUT_RAM_RAM1_DATA,
                    cfg->table[cnt]);

                /* Go to next Lut address */
                pDpcLutAddr = pDpcLutAddr + 1U;
            }

            /* Set the other configuration */
            regVal = CSL_REG32_RD(&rfeRegs->LUTDPC_CFG);
            CSL_FINS(regVal, RAWFE_CFG_LUTDPC_CFG_SIZE, cfg->maxDefectPixels - 1U);
            CSL_FINS(regVal, RAWFE_CFG_LUTDPC_CFG_SEL, cfg->isReplaceWhite);
            CSL_FINS(regVal, RAWFE_CFG_LUTDPC_CFG_EN, 1U);
            CSL_REG32_WR(&rfeRegs->LUTDPC_CFG, regVal);
        }
        else
        {
            /* Disable DPC Lut */
            CSL_REG32_FINS(&rfeRegs->LUTDPC_CFG, RAWFE_CFG_LUTDPC_CFG_EN, 0U);
        }
    }

    return (status);
}

/* Sets WB Gain Settings */
int32_t CSL_rfeSetWbConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_GainOfstConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Set offset 0 and 1 in WB register */
        regVal = CSL_REG32_RD(&rfeRegs->WB2_OFFSET12);
        CSL_FINS(regVal, RAWFE_CFG_WB2_OFFSET12_WB_OFST00, cfg->offset[0U]);
        CSL_FINS(regVal, RAWFE_CFG_WB2_OFFSET12_WB_OFST01, cfg->offset[1U]);
        CSL_REG32_WR(&rfeRegs->WB2_OFFSET12, regVal);

        /* Set offset 2 and 3 in WB register */
        regVal = CSL_REG32_RD(&rfeRegs->WB2_OFFSET34);
        CSL_FINS(regVal, RAWFE_CFG_WB2_OFFSET34_WB_OFST10, cfg->offset[2U]);
        CSL_FINS(regVal, RAWFE_CFG_WB2_OFFSET34_WB_OFST11, cfg->offset[3U]);
        CSL_REG32_WR(&rfeRegs->WB2_OFFSET34, regVal);

        /* Set gains 0 and 1 in WB register */
        regVal = CSL_REG32_RD(&rfeRegs->WB2_GAIN12);
        CSL_FINS(regVal, RAWFE_CFG_WB2_GAIN12_WB_GAIN00, cfg->gain[0U]);
        CSL_FINS(regVal, RAWFE_CFG_WB2_GAIN12_WB_GAIN01, cfg->gain[1U]);
        CSL_REG32_WR(&rfeRegs->WB2_GAIN12, regVal);

        /* Set gains 2 and 3 in WB register */
        regVal = CSL_REG32_RD(&rfeRegs->WB2_GAIN34);
        CSL_FINS(regVal, RAWFE_CFG_WB2_GAIN34_WB_GAIN10, cfg->gain[2U]);
        CSL_FINS(regVal, RAWFE_CFG_WB2_GAIN34_WB_GAIN11, cfg->gain[3U]);
        CSL_REG32_WR(&rfeRegs->WB2_GAIN34, regVal);
    }

    return (status);
}

int32_t CSL_rfeSetH3aConfig(CSL_rawfe_cfgRegs *rfeRegs,
    const Rfe_H3aInConfig *cfg)
{
    int32_t  status = FVID2_SOK;
    volatile uint32_t regVal = 0U;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == cfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(&rfeRegs->H3AMUX_CFG);
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
        if (cfg->inSel == RFE_H3A_IN_SEL_PCID_OUT_FRAME)
        {
            CSL_FINS(regVal, RAWFE_CFG_H3AMUX_CFG_PCIDSEL, 1U);
            CSL_FINS(regVal, RAWFE_CFG_H3AMUX_CFG_SEL, 0U);
        }
        else
        {
            CSL_FINS(regVal, RAWFE_CFG_H3AMUX_CFG_PCIDSEL, 0U);
#endif
            CSL_FINS(regVal, RAWFE_CFG_H3AMUX_CFG_SEL, (uint32_t)cfg->inSel);
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
        }
#endif
        CSL_FINS(regVal, RAWFE_CFG_H3AMUX_CFG_SHIFT, cfg->shift);
        CSL_REG32_WR(&rfeRegs->H3AMUX_CFG, regVal);
    }

    return (status);
}

int32_t CSL_rfeSetH3aLutConfig(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *h3aLutAddr,
    const Vhwa_LutConfig *lCfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;
    volatile uint32_t regVal = 0U;
    volatile uint32_t *tempLutAddr = NULL;
    uint32_t          *pH3aLutAddr = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == lCfg) || (NULL == rfeRegs))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == lCfg->enable)
        {
            if (NULL == lCfg->tableAddr)
            {
                status = FVID2_EBADARGS;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == h3aLutAddr)
            {
                status = FVID2_EBADARGS;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                pH3aLutAddr = h3aLutAddr;
            }
        }
    }

    if (FVID2_SOK == status)
    {
        if ((uint32_t)UTRUE == lCfg->enable)
        {
            tempLutAddr = lCfg->tableAddr;
            for (cnt = 0U; cnt < RFE_H3A_COMP_LUT_SIZE; cnt += 2U)
            {
                regVal = 0U;

                CSL_FINS(regVal, RAWFE_H3A_LUT_RAM_RAM1_DATA_L,
                    tempLutAddr[cnt]);

                if ((cnt + 1U) < RFE_H3A_COMP_LUT_SIZE)
                {
                    CSL_FINS(regVal, RAWFE_H3A_LUT_RAM_RAM1_DATA_U,
                        tempLutAddr[cnt + 1U]);
                }

                CSL_REG32_WR(pH3aLutAddr, regVal);

                pH3aLutAddr ++;
            }

            regVal = CSL_REG32_RD(&rfeRegs->H3ALUT_CFG);
            CSL_FINS(regVal, RAWFE_CFG_H3ALUT_CFG_CLIP, lCfg->clip);
            CSL_FINS(regVal, RAWFE_CFG_H3ALUT_CFG_BITS, lCfg->inputBits);
            CSL_FINS(regVal, RAWFE_CFG_H3ALUT_CFG_EN, 1U);
            CSL_REG32_WR(&rfeRegs->H3ALUT_CFG, regVal);
        }
        else
        {
            CSL_REG32_FINS(&rfeRegs->H3ALUT_CFG, RAWFE_CFG_H3ALUT_CFG_EN, 0U);
        }
    }

    return (status);
}

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static void RfeSetMergeLut(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    const Vhwa_LutConfig *cfg)
{
    uint32_t              cnt;
    volatile uint32_t     regVal = 0U;
    volatile uint32_t     *tempLutAddr = NULL;
    uint32_t              *pLutAddr = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != rfeRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        if ((uint32_t)UTRUE == cfg->enable)
        {
            tempLutAddr = cfg->tableAddr;
            pLutAddr = lutAddr;
            for (cnt = 0U; cnt < RFE_COMP_DECOMP_LUT_SIZE; cnt += 2U)
            {
                regVal = 0U;

                CSL_FINS(regVal, RAWFE_WDR_LUT_RAM_RAM1_DATA_L,
                    tempLutAddr[cnt]);

                if ((cnt + 1U) < RFE_COMP_DECOMP_LUT_SIZE)
                {
                    CSL_FINS(regVal, RAWFE_WDR_LUT_RAM_RAM1_DATA_U,
                        tempLutAddr[cnt + 1U]);
                }

                CSL_REG32_WR(pLutAddr, regVal);

                pLutAddr ++;
            }

            regVal = CSL_REG32_RD(&rfeRegs->MRGLUT_CFG);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_CLIP, cfg->clip);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_BITS, cfg->inputBits);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_EN, 1U);
            CSL_REG32_WR(&rfeRegs->MRGLUT_CFG, regVal);
        }
        else
        {
            regVal = CSL_REG32_RD(&rfeRegs->MRGLUT_CFG);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_CLIP, cfg->clip);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_BITS, cfg->inputBits);
            CSL_FINS(regVal, RAWFE_CFG_MRGLUT_CFG_EN, 0U);
            CSL_REG32_WR(&rfeRegs->MRGLUT_CFG, regVal);
        }
    }
}

static void RfeSetPwlLut(CSL_rawfe_cfgRegs *rfeRegs, uint32_t *lutAddr,
    uint32_t lutModule, const Vhwa_LutConfig *cfg)
{
    uint32_t                cnt;
    volatile uint32_t       regVal, *regAddrLut, *regAddrLutClip;
    uint32_t                *tempLutAddr = NULL;
    uint32_t                *pLutAddr = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != rfeRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        /* Assuming
            PWL1 is for VS input
            PWL2 is for Short input
            PWL3 is for Long input
           TODO: Confirm this understanding
         */
        if (RFE_MODULE_DECOMP_LUT3 == lutModule)
        {
            regAddrLut = &rfeRegs->PWL3_LUT;
            regAddrLutClip = &rfeRegs->PWL3_LUTCLIP;
        }
        else if (RFE_MODULE_DECOMP_LUT2 == lutModule)
        {
            regAddrLut = &rfeRegs->PWL2_LUT;
            regAddrLutClip = &rfeRegs->PWL2_LUTCLIP;
        }
        else /* PWL for Very Short input */
        {
            regAddrLut = &rfeRegs->PWL1_LUT;
            regAddrLutClip = &rfeRegs->PWL1_LUTCLIP;
        }

        if ((uint32_t)UTRUE == cfg->enable)
        {
            tempLutAddr = cfg->tableAddr;
            pLutAddr = lutAddr;
            for (cnt = 0U; cnt < RFE_COMP_DECOMP_LUT_SIZE; cnt += 2U)
            {
                regVal = 0U;

                CSL_FINS(regVal, RAWFE_PWL_LUT1_RAM_RAM1_DATA_L,
                    tempLutAddr[cnt]);

                if ((cnt + 1U) < RFE_COMP_DECOMP_LUT_SIZE)
                {
                    CSL_FINS(regVal, RAWFE_PWL_LUT1_RAM_RAM1_DATA_U,
                        tempLutAddr[cnt + 1U]);
                }

                CSL_REG32_WR(pLutAddr, regVal);

                pLutAddr ++;
            }

            CSL_REG32_FINS(regAddrLutClip, RAWFE_CFG_PWL1_LUTCLIP_LUTCLIP,
                cfg->clip);

            regVal = CSL_REG32_RD(regAddrLut);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_LUT_LUT_BITS, cfg->inputBits);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_LUT_LUT_EN, 1U);
            CSL_REG32_WR(regAddrLut, regVal);
        }
        else
        {
            regVal = CSL_REG32_RD(regAddrLut);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_LUT_LUT_BITS, cfg->inputBits);
            CSL_FINS(regVal, RAWFE_CFG_PWL1_LUT_LUT_EN, 0U);
            CSL_REG32_WR(regAddrLut, regVal);
        }
    }
}

