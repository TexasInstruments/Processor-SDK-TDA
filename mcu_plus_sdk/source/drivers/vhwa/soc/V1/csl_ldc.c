/* =============================================================================
 *   Copyright (c) Texas Instruments Incorporated 2021-2022
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
 *  \file csl_ldc.c
 *
 *  \brief LDC CSL file, contains implementation of API, configuring LDC module.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <src/csl/include/csl_ldc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \brief Macro to check width alignment by 8
 */
#define LDC_BLOCK_WTH_ALIGN_8           (8U)

/**
 *  \brief Macro for max LDC Lut block size
 */
#define LDC_MAX_LDC_LUT_BLK_SIZE        (5U*1024U)

/**
 *  \brief Macro for Max width supported by LDC
 *         Defined here as macro, because size of this field is
 *         14bits, ie 16383, but LDC supports at max 8192 width
 */
#define LDC_MAX_WIDTH                   (8192U)

/**
 *  \brief Macro for Max height supported by LDC
 *         Defined here as macro, because size of this field is
 *         14bits, ie 16383, but LDC supports at max 8192 height
 */
#define LDC_MAX_HEIGHT                  (8192U)


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Function to check configuration, checks all parameters of
 *          ldcconfig and returns status.
 *
 * \param   instInfo           Pointer to instance Information.
 * \param   cfg                Pointer to ldcConfig structure
 *
 * \return  FVID2_SOK on success else error code.
 *
 **/
static int32_t CSL_ldcCheckCfg(const Ldc_Config *cfg);

/**
 * \brief   Local Function to check affine transformation configuration.
 *
 * \param   coreRegs           LDC Register Overlay
 * \param   cfg                Perspective transformation configuration
 *
 **/
static void CSL_ldcSetAffineTransformCfg(
    CSL_ldc_coreRegs                  *coreRegs,
    const Ldc_PerspectiveTransformCfg *cfg);

/**
 * \brief   Local Function to set output configuration,
 *          ie size, block size position etc.
 *
 * \param   coreRegs           LDC Register Overlay
 * \param   cfg                LDC configuration, from which it
 *                             extracts output configuration
 *
 **/
static void CSL_ldcSetOutputParams(
    CSL_ldc_coreRegs *coreRegs,
    const Ldc_Config *cfg);

/**
 * \brief   Local Function to set input frame parameters,
 *          ie frame size, format etc.
 *
 * \param   coreRegs           LDC Register Overlay
 * \param   cfg                Pointer to CslLdc_Config structure
 *                             containing containing input frame format.
 *                             This parameter should be non-NULL.
 *
 **/
static void CSL_ldcSetInFrameParams(
    CSL_ldc_coreRegs    *coreRegs,
    const Ldc_Config *cfg);

/**
 * \brief   Local Function to set parameters for multi-regions.
 *
 * \param   coreRegs           LDC Register Overlay
 * \param   cfg                Multi-region configuration
 *
 **/
static void CSL_ldcSetMultiRegionCfg(
    CSL_ldc_coreRegs       *coreRegs,
    const Ldc_RegionConfig *cfg);

/**
 *  CSL_ldcSetLutConfig
 *  \brief API for setting LUT configuration.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *  \param lutCfg           Pointer to LUT configuration structure
 *
 */
static void CSL_ldcSetLutConfig(
    CSL_ldc_coreRegs *coreRegs,
    const Ldc_LutCfg *lutCfg);

/**
 *  CSL_ldcSetRemapLut
 *  \brief API for setting Remapping Lut.
 *
 *  \param lutAddr          SoC Address of Lut
 *  \param lutCfg           pointer to array containing user provided Lut
 *
 */
static void CSL_ldcSetRemapLut(volatile uint32_t *lutAddr,const uint16_t *tblAddr);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  CSL_ldcSetConfig
 *  \brief Sets the entire LDC configuration to the LDC registers.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *  \param cfg              Pointer to CslLdc_Config structure
 *                          containing the register configurations.
 *                          This parameter should be non-NULL.
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetConfig(
    CSL_ldc_coreRegs *coreRegs,
    const Ldc_Config *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == coreRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Check for Error */
        status = CSL_ldcCheckCfg(cfg);
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set Lut configuration */
        CSL_ldcSetLutConfig(coreRegs, &cfg->lutCfg);

        /* Set the output frame parameters */
        CSL_ldcSetOutputParams(coreRegs, cfg);

        /* Set the transformation configuration */
        CSL_ldcSetAffineTransformCfg(coreRegs, &cfg->perspTrnsformCfg);
        
        status = CSL_ldcSetBwLimitConfig(coreRegs, &cfg->rdBwLimitCfg);

        /* Set the input frame parameters */
        CSL_ldcSetInFrameParams(coreRegs, cfg);

        /* Set the Luma Interpolation */
        CSL_REG32_FINS(&coreRegs->CFG, LDC_CORE_CFG_YINT_TYP,
            cfg->lumaIntrType);

        /* Set Continuous Mode */
        CSL_REG32_FINS(&coreRegs->CFG, LDC_CORE_CFG_CNTU_MODE,
            cfg->continuousMode);

        /* Enable YUV422 to YUV420 conversion */
        if (((cfg->inFmt.dataFormat == FVID2_DF_YUV422I_UYVY) ||
             (cfg->inFmt.dataFormat == FVID2_DF_YUV422SP_UV)) &&
            (cfg->outFmt[0].dataFormat == FVID2_DF_YUV420SP_UV))
        {
            CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_OP_DATAMODE, 1u);
        }
        else
        {
            CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_OP_DATAMODE, 0u);
        }

        /* Enable Back Mapping */
        CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_LDMAPEN,
            (uint32_t)cfg->enableBackMapping);

        if ((uint32_t)UFALSE == cfg->enableMultiRegions)
        {
            for (cnt = 0u; cnt < LDC_MAX_REGIONS; cnt ++)
            {
                CSL_REG32_FINS(&coreRegs->REGION[cnt].CTRL,
                    LDC_CORE_REGION_CTRL_ENABLE, 0u);
            }

            /* Disable Multi Region Mode in CTRL register */
            CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_REGMODE_EN, 0u);
        }
        else
        {
            CSL_ldcSetMultiRegionCfg(coreRegs, &cfg->regCfg);

            /* Enable Multi Region Mode in CTRL register */
            CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_REGMODE_EN, 1u);
        }


        CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_IP_CIRCEN,
            (uint32_t)0u);
        CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_IP_HTS_ROWSYNC,
            (uint32_t)0u);

        if(0u != cfg->outputStartX)
        {
            CSL_REG32_FINS(&coreRegs->INITXY, LDC_CORE_INITXY_INITX,
                (uint32_t)cfg->outputStartX);
        }
        if(0u != cfg->outputStartY)
        {
            CSL_REG32_FINS(&coreRegs->INITXY, LDC_CORE_INITXY_INITY,
                (uint32_t)cfg->outputStartY);
        }
    }

    return (status);
}



/**
 *  CSL_ldcStart
 *  \brief LDC Api for enabling LDC module.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcStart(CSL_ldc_coreRegs *coreRegs)
{
    int32_t status = FVID2_SOK;

    /* Check for the input parameters */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == coreRegs)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_LDC_EN, 1U);
    }

    return (status);
}

/**
 *  CSL_ldcStop
 *  \brief LDC Api for disabling LDC module.
 *         This is required when continuous mode is enabled, as hardware
 *         will not automatically clear LDC_EN bit at end of frame.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcStop(CSL_ldc_coreRegs *coreRegs)
{
    int32_t status = FVID2_SOK;

    /* Check for the input parameters */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == coreRegs)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Clear LDC_EN bit to disable LDC and return to idle state */
        CSL_REG32_FINS(&coreRegs->CTRL, LDC_CORE_CTRL_LDC_EN, 0U);
    }

    return (status);
}

/**
 *  CSL_ldcSetBwLimitConfig
 *  \brief API for setting Bandwidth Limit config for Rd Dma.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *  \param lutCfg           Pointer to LUT configuration structure
 *
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_ldcSetBwLimitConfig(
    CSL_ldc_coreRegs          *coreRegs,
    const Ldc_RdBwLimitConfig *bwLimitCfg)
{
    int32_t status = FVID2_SOK;
    volatile uint32_t regVal;

    /* Check the input parameters */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == coreRegs) || (NULL == bwLimitCfg))
    {
        status = FVID2_EBADARGS;
    }
    else if (bwLimitCfg->rdMaxBurstLength >= VHWA_LDC_MAX_BURST_LENGTH_MAX)
    /* LDRA_JUSTIFY_END */
    {
        status = FVID2_EINVALID_PARAMS;
    }
    else
    {
        regVal = CSL_REG32_RD(&coreRegs->VBUSMR_CFG);
        CSL_FINS(regVal, LDC_CORE_VBUSMR_CFG_BW_CTRL, bwLimitCfg->rdBwLimit);
        CSL_FINS(regVal, LDC_CORE_VBUSMR_CFG_TAG_CNT, bwLimitCfg->rdTagCnt);
        CSL_FINS(regVal, LDC_CORE_VBUSMR_CFG_MAX_BURSTLEN,
            bwLimitCfg->rdMaxBurstLength);
        CSL_REG32_WR(&coreRegs->VBUSMR_CFG, regVal);
    }

    return (status);
}


/**
 *  CSL_ldcSoftReset
 *  \brief API for triggering soft resetting for LDC.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
void CSL_ldcSoftReset(CSL_ldc_coreRegs *coreRegs)
{
    volatile uint32_t regVal;

    if (NULL == coreRegs)
    {
        regVal = CSL_REG32_RD(&coreRegs->CFG);
        regVal = regVal | CSL_LDC_CORE_COREOUT_CHANCFG_RSRV_CH0_MASK;
        CSL_REG32_WR(&coreRegs->CFG, regVal);
    }
}
/* LDRA_JUSTIFY_END */
/**
 *  CSL_ldcIsResetDone
 *  \brief API to check if reset is done or not.
 *
 *  \param coreRegs         Pointer to a #CSL_ldc_coreRegs structure
 *                          containing the common configuration
 *
 *  \return                 Returns UTRUE if reset is done
 *                                  UFALSE otherwise
 */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
uint32_t CSL_ldcIsResetDone(const CSL_ldc_coreRegs *coreRegs)
{
    uint32_t status = UFALSE;
    volatile uint32_t regVal;

    if (NULL != coreRegs)
    {
        regVal = CSL_REG32_RD(&coreRegs->CFG);
        if ((regVal & CSL_LDC_CORE_COREOUT_CHANCFG_RSRV_CH0_MASK) == 0u)
        {
            status = UTRUE;
        }
    }

    return (status);
}
/* LDRA_JUSTIFY_END */
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
    uint64_t chromaAddr)
{
    uint32_t addr;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != coreRegs)
    /* LDRA_JUSTIFY_END */
    {
        /* Set the input frame address */
        addr = (uint32_t)(lumaAddr & CSL_LDC_CORE_RD_BASE_L_ADDR_MASK);
        CSL_REG32_FINS(&coreRegs->RD_BASE_L, LDC_CORE_RD_BASE_L_ADDR, addr);

        addr = (uint32_t)((lumaAddr >> 32U) & CSL_LDC_CORE_RD_BASE_H_ADDR_MASK);
        CSL_REG32_FINS(&coreRegs->RD_BASE_H, LDC_CORE_RD_BASE_H_ADDR, addr);

        addr = (uint32_t)(chromaAddr & CSL_LDC_CORE_RD_420C_BASE_L_ADDR_MASK);
        CSL_REG32_FINS(&coreRegs->RD_420C_BASE_L, LDC_CORE_RD_420C_BASE_L_ADDR,
            addr);

        addr = (uint32_t)((chromaAddr >> 32U) &
            CSL_LDC_CORE_RD_420C_BASE_H_ADDR_MASK);
        CSL_REG32_FINS(&coreRegs->RD_420C_BASE_H, LDC_CORE_RD_420C_BASE_H_ADDR,
            addr);
    }
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void CSL_ldcSetInBuffOffset.* <function end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
void CSL_ldcSetInBuffOffset(CSL_ldc_coreRegs *coreRegs,
                                uint32_t xOffset,
                                uint32_t yOffset)
{
    volatile uint32_t regVal = 0;

    if (NULL != coreRegs)
    {
        CSL_FINS(regVal, LDC_CORE_IBUF_PIX_START_STARTX, xOffset);
        CSL_FINS(regVal, LDC_CORE_IBUF_PIX_START_STARTY, yOffset);
        CSL_REG32_WR(&coreRegs->IBUF_PIX_START, regVal);
    }
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t CSL_ldcSetLumaToneMapLutCfg.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
int32_t CSL_ldcSetLumaToneMapLutCfg(CSL_ldc_coreRegs *coreRegs,
    volatile uint32_t *lutAddr, const Ldc_RemapLutCfg *lutCfg)
{
    int32_t status = FVID2_SOK;
    volatile uint32_t regVal;

    /* Check the input parameters */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == coreRegs)
    {
        status = FVID2_EBADARGS;
    }
    else if (NULL == lutCfg)
    {
        /* Disable LUT when config is NULL */
        CSL_REG32_FINS(&coreRegs->DUALOUT_CFG,
            LDC_CORE_DUALOUT_CFG_YLUT_EN, 0U);
    }
    else if ((uint32_t)UTRUE == lutCfg->enable)
    {
        /* Validate LUT configuration */
        if (NULL == lutAddr)
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else if ((lutCfg->inputBits < 8u) || (lutCfg->inputBits > 12u))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else if ((lutCfg->outputBits < 8u) || (lutCfg->outputBits > 12u))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else
        {
            /* All validations passed — configure LUT */
            regVal = CSL_REG32_RD(&coreRegs->DUALOUT_CFG);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_YIN_BITDPTH,
                lutCfg->inputBits);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_YOUT_BITDPTH,
                lutCfg->outputBits);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_YLUT_EN, 1u);
            CSL_REG32_WR(&coreRegs->DUALOUT_CFG, regVal);
            CSL_ldcSetRemapLut(lutAddr, lutCfg->tableAddr);
        }
    }
    else
    {
        CSL_REG32_FINS(&coreRegs->DUALOUT_CFG,
            LDC_CORE_DUALOUT_CFG_YLUT_EN, 0U);
    }

    return (status);
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t CSL_ldcSetChromaToneMapLutCfg.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
int32_t CSL_ldcSetChromaToneMapLutCfg(CSL_ldc_coreRegs *coreRegs,
    volatile uint32_t *lutAddr, const Ldc_RemapLutCfg *lutCfg)
{
    int32_t status = FVID2_SOK;
    volatile uint32_t regVal;

    
    /* Check the input parameters */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == coreRegs)
    {
        status = FVID2_EBADARGS;
    }
    else if (NULL == lutCfg)
    {
        /* Disable LUT when config is NULL */
        CSL_REG32_FINS(&coreRegs->DUALOUT_CFG,
            LDC_CORE_DUALOUT_CFG_CLUT_EN, 0U);
    }
    else if ((uint32_t)UTRUE == lutCfg->enable)
    {
        /* Validate LUT configuration */
        if (NULL == lutAddr)
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else if ((lutCfg->inputBits < 8u) || (lutCfg->inputBits > 12u))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else if ((lutCfg->outputBits < 8u) || (lutCfg->outputBits > 12u))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        else
        {
            /* All validations passed — configure LUT */
            regVal = CSL_REG32_RD(&coreRegs->DUALOUT_CFG);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_CIN_BITDPTH,
                lutCfg->inputBits);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_COUT_BITDPTH,
                lutCfg->outputBits);
            CSL_FINS(regVal, LDC_CORE_DUALOUT_CFG_CLUT_EN, (uint32_t)1U);
            CSL_REG32_WR(&coreRegs->DUALOUT_CFG, regVal);
            CSL_ldcSetRemapLut(lutAddr, lutCfg->tableAddr);
        }
    }
    else
    {
        CSL_REG32_FINS(&coreRegs->DUALOUT_CFG,
            LDC_CORE_DUALOUT_CFG_CLUT_EN, 0U);
    }

    return (status);
}

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static void CSL_ldcSetAffineTransformCfg(
    CSL_ldc_coreRegs                  *coreRegs,
    const Ldc_PerspectiveTransformCfg *cfg)
{
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end> 
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if ((NULL != coreRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        /* Set all perspective transformation parameters */
        regVal = CSL_REG32_RD(&coreRegs->AFF_AB);
        CSL_FINS(regVal, LDC_CORE_AFF_AB_A, (uint32_t)cfg->coeffA);
        CSL_FINS(regVal, LDC_CORE_AFF_AB_B, (uint32_t)cfg->coeffB);
        CSL_REG32_WR(&coreRegs->AFF_AB, regVal);

        regVal = CSL_REG32_RD(&coreRegs->AFF_CD);
        CSL_FINS(regVal, LDC_CORE_AFF_CD_C, (uint32_t)cfg->coeffC);
        CSL_FINS(regVal, LDC_CORE_AFF_CD_D, (uint32_t)cfg->coeffD);
        CSL_REG32_WR(&coreRegs->AFF_CD, regVal);

        regVal = CSL_REG32_RD(&coreRegs->AFF_EF);
        CSL_FINS(regVal, LDC_CORE_AFF_EF_E, (uint32_t)cfg->coeffE);
        CSL_FINS(regVal, LDC_CORE_AFF_EF_F, (uint32_t)cfg->coeffF);
        CSL_REG32_WR(&coreRegs->AFF_EF, regVal);

        regVal = CSL_REG32_RD(&coreRegs->PWARP_GH);
        CSL_FINS(regVal, LDC_CORE_PWARP_GH_G, (uint32_t)cfg->coeffG);
        CSL_FINS(regVal, LDC_CORE_PWARP_GH_H, (uint32_t)cfg->coeffH);
        CSL_REG32_WR(&coreRegs->PWARP_GH, regVal);

        regVal = CSL_REG32_RD(&coreRegs->CTRL);
        if(1U == cfg->enableWarp)
        {
            regVal |= CSL_LDC_CORE_CTRL_PWARPEN_MASK;
        }
        else
        {
            /* Disable Perspective transformation and
               set GH parameters to be 0 */
            regVal &= ~CSL_LDC_CORE_CTRL_PWARPEN_MASK;
            CSL_REG32_WR(&coreRegs->PWARP_GH, 0U);
        }
        CSL_REG32_WR(&coreRegs->CTRL, regVal);
    }
}

static void CSL_ldcSetOutputParams(
    CSL_ldc_coreRegs *coreRegs,
    const Ldc_Config *cfg)
{
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != coreRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        /* Set Block size and pixel pad */
        regVal  = CSL_REG32_RD(&coreRegs->OUT_BLKSZ);
        CSL_FINS(regVal, LDC_CORE_OUT_BLKSZ_PIXPAD, cfg->pixelPad);
        CSL_FINS(regVal, LDC_CORE_OUT_BLKSZ_OBW, cfg->outputBlockWidth);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required. 
        <justification end> */
        if (FVID2_DF_CHROMA_ONLY == cfg->inFmt.dataFormat)
        {
            CSL_FINS(regVal, LDC_CORE_OUT_BLKSZ_OBH,
                cfg->outputBlockHeight * 2u);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, LDC_CORE_OUT_BLKSZ_OBH,
                cfg->outputBlockHeight);
        }
        CSL_REG32_WR(&coreRegs->OUT_BLKSZ, regVal);

        /* Set the output position */
        regVal  = CSL_REG32_RD(&coreRegs->INITXY);
        CSL_FINS(regVal, LDC_CORE_INITXY_INITX, cfg->outputStartX);
        CSL_FINS(regVal, LDC_CORE_INITXY_INITY, cfg->outputStartY);
        CSL_REG32_WR(&coreRegs->INITXY, regVal);

        regVal = CSL_REG32_RD(&coreRegs->COMPUTE_FRSZ);
        CSL_FINS(regVal, LDC_CORE_COMPUTE_FRSZ_W,
            cfg->outputFrameWidth);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required. 
        <justification end> */
        if (FVID2_DF_CHROMA_ONLY == cfg->inFmt.dataFormat)
        {
            CSL_FINS(regVal, LDC_CORE_COMPUTE_FRSZ_H,
                cfg->outputFrameHeight*2u);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, LDC_CORE_COMPUTE_FRSZ_H,
                cfg->outputFrameHeight);
        }
        CSL_REG32_WR(&coreRegs->COMPUTE_FRSZ, regVal);

        regVal = CSL_REG32_RD(&coreRegs->COREOUT_CHANCFG);
        regVal &= ~(CSL_LDC_CORE_COREOUT_CHANCFG_CH2_EN_MASK |
            CSL_LDC_CORE_COREOUT_CHANCFG_CH3_EN_MASK);

        if (UTRUE == cfg->enableOutput[1])
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required. 
            <justification end> */
            if (FVID2_DF_YUV420SP_UV == cfg->outFmt[1].dataFormat)
            {
                regVal |= CSL_LDC_CORE_COREOUT_CHANCFG_CH2_EN_MASK |
                    CSL_LDC_CORE_COREOUT_CHANCFG_CH3_EN_MASK;
            }
            else if (FVID2_DF_YUV422I_UYVY == cfg->outFmt[1].dataFormat)
            {
                regVal |= CSL_LDC_CORE_COREOUT_CHANCFG_CH2_EN_MASK;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required. 
            <justification end> */
            else if (FVID2_DF_LUMA_ONLY == cfg->outFmt[1].dataFormat)
            /* LDRA_JUSTIFY_END */
            {
                regVal |= CSL_LDC_CORE_COREOUT_CHANCFG_CH2_EN_MASK;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */ 
            else
            {
                regVal |= CSL_LDC_CORE_COREOUT_CHANCFG_CH3_EN_MASK;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required. 
        <justification end> */
        if((FVID2_DF_RGI_B == cfg->outFmt[0].dataFormat) ||
           (FVID2_DF_RGB == cfg->outFmt[0].dataFormat))
        /* LDRA_JUSTIFY_END */
        {
            regVal |= CSL_LDC_CORE_COREOUT_CHANCFG_CH3_EN_MASK;
        }

        CSL_REG32_WR(&coreRegs->COREOUT_CHANCFG, regVal);

        /* Assuming single region, it sets the output frame size in region0 */
        CSL_REG32_FINS(&coreRegs->REGN_W12_SZ, LDC_CORE_REGN_W12_SZ_W1,
            cfg->outputFrameWidth);
        CSL_REG32_FINS(&coreRegs->REGN_H12_SZ, LDC_CORE_REGN_H12_SZ_H1,
            cfg->outputFrameHeight);
    }
}

static void CSL_ldcSetInFrameParams(
    CSL_ldc_coreRegs   *coreRegs,
    const Ldc_Config *cfg)
{
    volatile uint32_t regVal;
    const Fvid2_Format *fmt = NULL;

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != coreRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        fmt = &cfg->inFmt;

        /* Set the input frame size in */
        regVal = CSL_REG32_RD(&coreRegs->INPUT_FRSZ);
        CSL_FINS(regVal, LDC_CORE_INPUT_FRSZ_W, fmt->width);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_DF_CHROMA_ONLY == fmt->dataFormat)
        {
            CSL_FINS(regVal, LDC_CORE_INPUT_FRSZ_H, fmt->height*2u);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, LDC_CORE_INPUT_FRSZ_H, fmt->height);
        }
        CSL_REG32_WR(&coreRegs->INPUT_FRSZ, regVal);

        /* Set input frame format */
        regVal = CSL_REG32_RD(&coreRegs->CTRL);

        /* Set the pixel data format */
        regVal &= ~CSL_LDC_CORE_CTRL_IP_DATAMODE_MASK;
        if (FVID2_DF_YUV422I_UYVY == fmt->dataFormat)
        {
            regVal &= ~CSL_LDC_CORE_CTRL_IP_DATAMODE_MASK;
        }
        else if (FVID2_DF_YUV420SP_UV == fmt->dataFormat)
        {
            regVal |= ((uint32_t)0x2U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        else if (FVID2_DF_LUMA_ONLY == fmt->dataFormat)
        {
            regVal |= ((uint32_t)0x1U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        else if (FVID2_DF_YUV422SP_UV == fmt->dataFormat)
        {
            regVal |= ((uint32_t)0x4U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        else if (FVID2_DF_2PLANES == fmt->dataFormat)
        {
            regVal |= ((uint32_t)0x5U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (FVID2_DF_R_GBI == fmt->dataFormat)
        /* LDRA_JUSTIFY_END */
        {
            regVal |= ((uint32_t)0x6U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        else /* Chroma Only Mode */
        {
            regVal |= ((uint32_t)0x3U << CSL_LDC_CORE_CTRL_IP_DATAMODE_SHIFT);
        }
        /* LDRA_JUSTIFY_END */

        /* Set the storage format */
        regVal &= ~CSL_LDC_CORE_CTRL_ALIGN_12BIT_MASK;
        regVal &= ~CSL_LDC_CORE_CTRL_IP_DFMT_MASK;
        if (FVID2_CCSF_BITS12_UNPACKED16 == fmt->ccsFormat)
        {
            regVal |= ((uint32_t)0x2U << CSL_LDC_CORE_CTRL_IP_DFMT_SHIFT);
        }
        else if (FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED == fmt->ccsFormat)
        {
            regVal |= ((uint32_t)0x2U << CSL_LDC_CORE_CTRL_IP_DFMT_SHIFT);
            regVal |= ((uint32_t)0x1U << CSL_LDC_CORE_CTRL_ALIGN_12BIT_SHIFT);
        }
        else if (FVID2_CCSF_BITS12_PACKED == fmt->ccsFormat)
        {
            regVal |= ((uint32_t)0x1U << CSL_LDC_CORE_CTRL_IP_DFMT_SHIFT);
        }
        else /* 8bit mode */
        {
            regVal &= ~CSL_LDC_CORE_CTRL_IP_DFMT_MASK;
        }

        regVal &= ~CSL_LDC_CORE_CTRL_CH_CHANCTRL_EN_MASK;
        /* Configure Independent channel parameters */
        if (UTRUE == cfg->indChPrms.enable)
        {
            regVal &= ~CSL_LDC_CORE_CTRL_CH_IP_DFMT_MASK;

            regVal |= ((uint32_t)0x1U << CSL_LDC_CORE_CTRL_CH_CHANCTRL_EN_SHIFT);

            if ((FVID2_CCSF_BITS12_UNPACKED16 == cfg->indChPrms.ccsf) ||
                (FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED == cfg->indChPrms.ccsf))
            {
                regVal |= ((uint32_t)0x2U << CSL_LDC_CORE_CTRL_CH_IP_DFMT_SHIFT);
            }
            else if (FVID2_CCSF_BITS12_PACKED == cfg->indChPrms.ccsf)
            {
                regVal |= ((uint32_t)0x1U << CSL_LDC_CORE_CTRL_CH_IP_DFMT_SHIFT);
            }
            else /* 8bit mode */
            {
                regVal &= ~CSL_LDC_CORE_CTRL_CH_IP_DFMT_MASK;
            }
        }
        else
        {
            regVal &= ~CSL_LDC_CORE_CTRL_CH_CHANCTRL_EN_MASK;
        }

        CSL_REG32_WR(&coreRegs->CTRL, regVal);

        /* Set line offset for the input buffer */
        CSL_REG32_FINS(&coreRegs->RD_OFST, LDC_CORE_RD_OFST_OFST,
            fmt->pitch[0u]);

        /* Configure Independent channel parameters */
        if (UTRUE == cfg->indChPrms.enable)
        {
            /* Set line offset for the input buffer */
            CSL_REG32_FINS(&coreRegs->CH_RD_OFST, LDC_CORE_CH_RD_OFST_OFST,
                cfg->indChPrms.pitch);
        }
    }
}

static int32_t CSL_ldcCheckCfg(const Ldc_Config *cfg)
{
    int32_t status = FVID2_SOK;

    /* Assuming cfg pointer is not null */

    /* Output 0 must be enabled */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (UFALSE == cfg->enableOutput[0])
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* For YUV422 output, input must be YUV422 */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((cfg->outFmt[0].dataFormat == FVID2_DF_YUV422I_UYVY) ||
          (cfg->outFmt[0].dataFormat == FVID2_DF_YUV422SP_UV))&&
        ((cfg->inFmt.dataFormat != FVID2_DF_YUV422I_UYVY)) &&
         (cfg->inFmt.dataFormat != FVID2_DF_YUV422SP_UV))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */
    /* For YUV420 output, input must be either YUV420 or YUV422 */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((cfg->outFmt[0].dataFormat == FVID2_DF_YUV420SP_UV) &&
        ((cfg->inFmt.dataFormat != FVID2_DF_YUV422I_UYVY) &&
         (cfg->inFmt.dataFormat != FVID2_DF_YUV420SP_UV) &&
         (cfg->inFmt.dataFormat != FVID2_DF_YUV422SP_UV)))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    if (UTRUE == cfg->enableOutput[1u])
    {
        /* For the second output, dataformat cannot be different */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (cfg->outFmt[0].dataFormat != cfg->outFmt[1].dataFormat)
        {
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Except for YUV422 input and YUV420 output or
       R_GBI input and RGB/RGI_B input and output
     * data format must be same */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested. 
    <justification end> */
    if((cfg->inFmt.dataFormat == FVID2_DF_R_GBI) &&
       (UTRUE == cfg->enableOutput[1]))
    {
        status = FVID2_EINVALID_PARAMS;
    }

    if((cfg->inFmt.dataFormat == FVID2_DF_R_GBI) &&
       (UTRUE != cfg->indChPrms.enable))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* Output block height must not be 0, must be even and
       must be multiple of output height */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((0U == cfg->outputBlockHeight) ||
       (0U != (cfg->outputBlockHeight & 0x1U)))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* Output block width must not be 0, must be even and must be
       multiple of output width */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((0U == cfg->outputBlockWidth) ||
        (0U != (cfg->outputBlockWidth & 0x1u)) ||
        (cfg->outputBlockWidth < LDC_BLOCK_WTH_ALIGN_8))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Check for valid value of downscaling factor */
    if (cfg->lutCfg.dsFactor >= VHWA_LDC_LUT_DS_FACTOR_MAX)
    {
        status = FVID2_EINVALID_PARAMS;
    }

    /* Check for valid value of Luma Interpolation type */
    if (cfg->lumaIntrType >= VHWA_LDC_LUMA_INTRP_MAX)
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* For YUV420 and YUV420 mode, block size must be less than 5KB internal
       memory size */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(((cfg->outputBlockWidth /
         ((uint32_t)0x1U << cfg->lutCfg.dsFactor)) *
        (cfg->outputBlockHeight /
         ((uint32_t)0x1U << cfg->lutCfg.dsFactor)) *
        4U) > LDC_MAX_LDC_LUT_BLK_SIZE)
    {
        /* Block size is invalid for this down scaling factor */
        status = FVID2_EINVALID_PARAMS;
    }

    if ((LDC_MAX_WIDTH < cfg->inFmt.width) ||
        (LDC_MAX_HEIGHT < cfg->inFmt.height))
    {
        /* Input Frame size is out of supported range */
        status = FVID2_EINVALID_PARAMS;
    }

    if ((FVID2_DF_YUV422I_UYVY != cfg->inFmt.dataFormat) &&
        (FVID2_DF_YUV420SP_UV != cfg->inFmt.dataFormat) &&
        (FVID2_DF_YUV422SP_UV != cfg->inFmt.dataFormat) &&
        (FVID2_DF_2PLANES != cfg->inFmt.dataFormat) &&
        (FVID2_DF_R_GBI != cfg->inFmt.dataFormat) &&
        (FVID2_DF_LUMA_ONLY != cfg->inFmt.dataFormat) &&
        (FVID2_DF_CHROMA_ONLY != cfg->inFmt.dataFormat))
    {
        /* Input/Output Frame size is out of supported range */
        status = FVID2_EINVALID_PARAMS;
    }

    if ((FVID2_CCSF_BITS8_PACKED != cfg->inFmt.ccsFormat) &&
        (FVID2_CCSF_BITS12_PACKED != cfg->inFmt.ccsFormat) &&
        (FVID2_CCSF_BITS12_UNPACKED16 != cfg->inFmt.ccsFormat) &&
        (FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED != cfg->inFmt.ccsFormat))
    {
        /* Input/Output Frame format is out of supported range */
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    if ((uint32_t)UTRUE == cfg->enableBackMapping)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Address and line offset must not be zero if backmapping is enabled */
        if ((0u == cfg->lutCfg.address) || (0u == cfg->lutCfg.lineOffset))
        {
            status = FVID2_EINVALID_PARAMS;
        }

        /* Address and line offset must be 16byte aligned */
        if ((0u != ((LDC_MESH_BUF_ADDR_ALIGN - 1u) & cfg->lutCfg.address)) ||
            (0u != ((LDC_MESH_BUF_ADDR_ALIGN - 1u) & cfg->lutCfg.lineOffset)))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Address and line offset must be 16byte aligned */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((0u != ((LDC_MESH_BUF_ADDR_ALIGN - 1u) & cfg->lutCfg.address)) ||
        (0u != ((LDC_MESH_BUF_ADDR_ALIGN - 1u) & cfg->lutCfg.lineOffset)))
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static void CSL_ldcSetMultiRegionCfg(
    CSL_ldc_coreRegs   *coreRegs,
    const Ldc_RegionConfig *cfg)
{
    uint32_t hCnt, vCnt, cnt;
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != coreRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        cnt = 0u;
        for (vCnt = 0u; vCnt < LDC_MAX_VERT_REGIONS; vCnt ++)
        {
            for (hCnt = 0u; hCnt < LDC_MAX_HORZ_REGIONS; hCnt ++)
            {
                /* Enable this region */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((uint32_t)UTRUE == cfg->enable[vCnt][hCnt])
                /* LDRA_JUSTIFY_END */
                {
                    CSL_REG32_FINS(&coreRegs->REGION[cnt].CTRL,
                        LDC_CORE_REGION_CTRL_ENABLE, 1u);

                    /* Set pixel pad, block size parameter for this region */
                    regVal = CSL_REG32_RD(&coreRegs->REGION[cnt].OUT_BLKSZ);
                    CSL_FINS(regVal, LDC_CORE_REGION_OUT_BLKSZ_PIXPAD,
                        cfg->pixelPad[vCnt][hCnt]);
                    CSL_FINS(regVal, LDC_CORE_REGION_OUT_BLKSZ_OBH,
                        cfg->blockHeight[vCnt][hCnt]);
                    CSL_FINS(regVal, LDC_CORE_REGION_OUT_BLKSZ_OBW,
                        cfg->blockWidth[vCnt][hCnt]);
                    CSL_REG32_WR(&coreRegs->REGION[cnt].OUT_BLKSZ, regVal);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else /* Disable this region */
                {
                    CSL_REG32_FINS(&coreRegs->REGION[cnt].CTRL,
                        LDC_CORE_REGION_CTRL_ENABLE, 0u);
                }
                /* LDRA_JUSTIFY_END */

                cnt ++;
            }
        }

        /* Set region boundaries, common width */
        regVal = CSL_REG32_RD(&coreRegs->REGN_W12_SZ);
        CSL_FINS(regVal, LDC_CORE_REGN_W12_SZ_W1, cfg->width[0u]);
        CSL_FINS(regVal, LDC_CORE_REGN_W12_SZ_W2, cfg->width[1u]);
        CSL_REG32_WR(&coreRegs->REGN_W12_SZ, regVal);

        CSL_REG32_FINS(&coreRegs->REGN_W3_SZ, LDC_CORE_REGN_W3_SZ_W3,
            cfg->width[2u]);

        /* Set region boundaries, common height */
        regVal = CSL_REG32_RD(&coreRegs->REGN_H12_SZ);
        CSL_FINS(regVal, LDC_CORE_REGN_H12_SZ_H1, cfg->height[0u]);
        CSL_FINS(regVal, LDC_CORE_REGN_H12_SZ_H2, cfg->height[1u]);
        CSL_REG32_WR(&coreRegs->REGN_H12_SZ, regVal);

        CSL_REG32_FINS(&coreRegs->REGN_H3_SZ, LDC_CORE_REGN_H3_SZ_H3,
            cfg->height[2u]);
    }
}

static void CSL_ldcSetLutConfig(
    CSL_ldc_coreRegs *coreRegs,
    const Ldc_LutCfg *lutCfg)
{
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != coreRegs) && (NULL != lutCfg))
    /* LDRA_JUSTIFY_END */
    {
        /* Line Offset */
        CSL_REG32_FINS(&coreRegs->MESH_OFST, LDC_CORE_MESH_OFST_OFST,
            lutCfg->lineOffset);

        /* MeshTable Base Address */
        CSL_REG32_FINS(&coreRegs->MESH_BASE_L, LDC_CORE_MESH_BASE_L_ADDR,
            lutCfg->address & 0xFFFFFFFFU);

        CSL_REG32_FINS(&coreRegs->MESH_BASE_H, LDC_CORE_MESH_BASE_H_ADDR,
            (lutCfg->address >> 32U) & 0xFFFFFFFFU);

        /* MeshTable Down Scale Factor */
        CSL_REG32_FINS(&coreRegs->MESHTABLE_CFG, LDC_CORE_MESHTABLE_CFG_M,
            lutCfg->dsFactor);

        /* Mesh table Frame size */
        regVal = CSL_REG32_RD(&coreRegs->MESH_FRSZ);
        CSL_FINS(regVal, LDC_CORE_MESH_FRSZ_W, lutCfg->width);
        CSL_FINS(regVal, LDC_CORE_MESH_FRSZ_H, lutCfg->height);
        CSL_REG32_WR(&coreRegs->MESH_FRSZ, regVal);

    }
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void CSL_ldcSetRemapLut.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit
None;Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
static void CSL_ldcSetRemapLut(volatile uint32_t *lutAddr,const uint16_t *tblAddr)
{
    volatile uint32_t cnt;
    volatile uint32_t regVal;
    volatile uint32_t *lutAddrPtr=lutAddr;

    for (cnt = 0u; cnt < LDC_REMAP_LUT_SIZE; cnt += 2u)
    {
        regVal = 0u;

        CSL_FINS(regVal, LDC_DUALYLUT_LUT_LUT_0, (uint32_t)tblAddr[cnt]);
        if (cnt < LDC_REMAP_LUT_SIZE)
        {
            CSL_FINS(regVal, LDC_DUALYLUT_LUT_LUT_1, (uint32_t)tblAddr[cnt + 1u]);
        }
        CSL_REG32_WR(lutAddrPtr, regVal);

        lutAddrPtr++;
    }
}
