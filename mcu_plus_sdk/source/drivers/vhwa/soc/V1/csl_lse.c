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
 *  \file csl_lse.c
 *
 *  \brief File containing implementation of LSE HAL
 *
 */

/* TODO:
 * 1, Check if circular buffer size is required to be multiple of total
 *    size/height
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#include <src/csl/include/csl_lse.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define CSL_LSE_SRC_BUFF_CFG_OFFSET          (0x8U)

#if defined(VHWA_TEST_NEGATIVE)
#define TEST_VISS_WDTIMER_ERR_INTERRUPT
#define TEST_MSC_WDTIMER_ERR_INTERRUPT
#endif


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static void LseSetOutChConfig(CSL_lseRegs *lseRegs, uint32_t outCh,
    const CSL_LseOutChConfig *outCfg);
static void LseSetInChConfig(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseInChConfig *inCfg);
static void LseSetVportConfig(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseVportConfig *vpCfg);
static void LseSetH3aConfig(CSL_lseRegs *lseRegs,
    const CSL_LseH3aConfig *h3aCfg);
static int32_t LseCheckCfg(const CSL_LseConfig *cfg);
static void LseSetLpbkConfig(CSL_lseRegs *lseRegs,
    const CSL_LseLpbkConfig *lpbkCfg);
static void LseConfigInBuff(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseInChConfig *inCfg);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t CSL_lseCheckConfig(const CSL_LseConfig  *cfg)
{
    int32_t  status = FVID2_SOK;

    /* Check for NULL pointer */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == cfg)
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Check for Error */
        status = LseCheckCfg(cfg);
    }

    return (status);
}

/**
 *  \brief Sets the entire MSC configuration to the MSC registers.
 *
 *  \param mscBaseAddr      MSC Base Address
 *  \param config           Pointer to VpsHalVpacMsc_Config structure
 *                          containing the register configurations.
 *                          This parameter should be non-NULL.
 *  \param arg              Not used, should be NULL
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_lseSetConfig(CSL_lseRegs          *lseRegs,
                         const CSL_LseConfig  *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;
    volatile uint32_t regVal;

    /* Check for NULL pointer */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == lseRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* Check for Error */
        status = LseCheckCfg(cfg);
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
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
        regVal = CSL_REG32_RD(&lseRegs->CFG_LSE);
        if ((uint32_t)UTRUE == cfg->enablePsa)
        {
            CSL_FINS(regVal, LSE_CFG_LSE_PSA_EN, (uint32_t)1U);
        }
        else
        {
            CSL_FINS(regVal, LSE_CFG_LSE_PSA_EN, (uint32_t)0U);
        }
        if ((uint32_t)UTRUE == cfg->enableInChSyncOnFrame)
        {
            CSL_FINS(regVal, LSE_CFG_LSE_IN_CH_SYNC_MODE, (uint32_t)1U);
        }
        else
        {
            CSL_FINS(regVal, LSE_CFG_LSE_IN_CH_SYNC_MODE, (uint32_t)0U);
        }
        if ((uint32_t)UTRUE == cfg->enableFixedArbMode)
        {
            CSL_FINS(regVal, LSE_CFG_LSE_VM_ARB_FIXED_MODE, (uint32_t)1U);
        }
        else
        {
            CSL_FINS(regVal, LSE_CFG_LSE_VM_ARB_FIXED_MODE, (uint32_t)0U);
        }
        CSL_REG32_WR(&lseRegs->CFG_LSE, regVal);

        LseSetH3aConfig(lseRegs, &cfg->h3aCfg);
        LseSetVportConfig(lseRegs, 0U, &cfg->vpCfg);
        LseSetLpbkConfig(lseRegs, &cfg->lpbkCfg);
        for (cnt = 0U; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->inChCfg[cnt].bypass)
            {
                LseSetInChConfig(lseRegs, cnt, &cfg->inChCfg[cnt]);
            }
        }
        for (cnt = 0U; cnt < CSL_LSE_MAX_OUTPUT_CH; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->outChCfg[cnt].bypass)
            {
                LseSetOutChConfig(lseRegs, cnt, &cfg->outChCfg[cnt]);
            }
        }
    }

    return (status);
}

/**
 *  \brief Sets the Frame Size in the input and output channels.
 *
 *  \param mscBaseAddr      MSC Base Address
 *  \param config           Pointer to VpsHalVpacMsc_Config structure
 *                          containing the register configurations.
 *                          This parameter should be non-NULL.
 *  \param arg              Not used, should be NULL
 *  \return                 Returns 0 on success else returns error value
 */
int32_t CSL_lseSetUpdateConfig(CSL_lseRegs          *lseRegs,
                               CSL_LseConfig  *cfg)
{
    int32_t  status = FVID2_SOK;
    uint32_t cnt;
    CSL_LseInChConfig *inCfg = NULL;
    volatile uint32_t regVal;

    /* Check for NULL pointer */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL == lseRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK == status)
    /* LDRA_JUSTIFY_END */
    {
        for (cnt = 0U; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
        {
            inCfg = &cfg->inChCfg[cnt];
            if ((uint32_t)UTRUE == inCfg->enable)
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC[cnt].FRAME_SIZE);
                CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_WIDTH, inCfg->frameWidth);
                CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_HEIGHT, inCfg->frameHeight);
                CSL_REG32_WR(&lseRegs->SRC[cnt].FRAME_SIZE, regVal);
            }
        }
    }

    return (status);
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
<justification end> */
void CSL_lseGetCoreStatus(const CSL_lseRegs   *lseRegs,
                          CSL_LseStatus *stat)
{
    volatile uint32_t regVal;

    /* Check for NULL pointer */
    if ((NULL != lseRegs) && (NULL != stat))
    {
        regVal = CSL_REG32_RD(&lseRegs->STATUS_PARAM);
        stat->numLpbkCh = CSL_FEXT(regVal, LSE_STATUS_PARAM_BYPASS_CH);
        stat->isOutSkipEn = CSL_FEXT(regVal, LSE_STATUS_PARAM_OUT_SKIP_EN);
        stat->is2DSupported = CSL_FEXT(regVal, LSE_STATUS_PARAM_CORE_OUT_2D);
        stat->outDataWidth= CSL_FEXT(regVal, LSE_STATUS_PARAM_CORE_OUT_DW);
        stat->isLineSkipEn = CSL_FEXT(regVal, LSE_STATUS_PARAM_LINE_SKIP_EN);
        stat->isSrcNibOffsetEn = CSL_FEXT(regVal, LSE_STATUS_PARAM_BIT_AOFFSET);
        stat->isBlankInsertSupported = CSL_FEXT(regVal,
            LSE_STATUS_PARAM_HV_INSERT);
        stat->maxPixMatrixHeight = CSL_FEXT(regVal, LSE_STATUS_PARAM_PIX_MX_HT);
        stat->inputDataBusWidth = CSL_FEXT(regVal, LSE_STATUS_PARAM_CORE_DW);
        stat->numOutH3aCh = CSL_FEXT(regVal, LSE_STATUS_PARAM_SL2_OUT_H3A_CH);
        stat->numOutCh = CSL_FEXT(regVal, LSE_STATUS_PARAM_SL2_OUT_CH);
        stat->numInChPerThr = CSL_FEXT(regVal, LSE_STATUS_PARAM_SL2_IN_CH_THR);
        stat->numVportInCh = CSL_FEXT(regVal, LSE_STATUS_PARAM_VPORT_THR);
        stat->numThrsSupported = CSL_FEXT(regVal, LSE_STATUS_PARAM_NTHR);
    }
}
/* LDRA_JUSTIFY_END */

void CSL_lseGetPsaSign(const CSL_lseRegs *lseRegs,
                       const CSL_LseConfig *cfg,
                       uint32_t psaSign[CSL_LSE_MAX_OUTPUT_CH])
{
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != cfg) && (NULL != psaSign))
    /* LDRA_JUSTIFY_END */
    {
        for (cnt = 0u; cnt < cfg->numOutCh; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->outChCfg[cnt].enable)
            {
                psaSign[cnt] = CSL_REG32_RD(&lseRegs->PSA_SIGNATURE[cnt]);
            }
        }

        if ((uint32_t)UTRUE == cfg->h3aCfg.enable)
        {
            psaSign[cnt] = CSL_REG32_RD(&lseRegs->PSA_SIGNATURE[cnt]);
        }
    }
}

void CSL_lseStartChannels(CSL_lseRegs *lseRegs, const CSL_LseConfig *cfg)
{
    uint32_t                    cnt, bufCnt;
    volatile uint32_t           regVal;
    const CSL_LseOutChConfig   *outCfg;
    const CSL_LseInChConfig    *inCfg;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != cfg))
    {
    /* LDRA_JUSTIFY_END */
        for (cnt = 0U; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
        {
            inCfg = &cfg->inChCfg[cnt];
            if ((uint32_t)UFALSE == inCfg->bypass)
            {
                for (bufCnt = 0U; bufCnt < inCfg->numInBuff; bufCnt ++)
                {
                    regVal = CSL_REG32_RD(&lseRegs->SRC[cnt].BUF_BA[bufCnt]);
                    CSL_FINS(regVal, LSE_SRC_BUF_BA_ENABLE, (uint32_t)1U);
                    CSL_FINS(regVal, LSE_SRC_BUF_BA_ADDR,
                        (inCfg->bufAddr[bufCnt] >> CSL_LSE_SRC_BUF_BA_ADDR_SHIFT));
                    CSL_REG32_WR(&lseRegs->SRC[cnt].BUF_BA[bufCnt], regVal);
                }
            }
        }

        for (cnt = 0U; cnt < CSL_LSE_MAX_OUTPUT_CH; cnt ++)
        {
            outCfg = &cfg->outChCfg[cnt];
            if ((uint32_t)UFALSE == cfg->outChCfg[cnt].bypass)
            {
                regVal = CSL_REG32_RD(&lseRegs->DST[cnt].BUF_BA);
                CSL_FINS(regVal, LSE_DST_BUF_BA_ENABLE, (uint32_t)1U);
                CSL_FINS(regVal, LSE_DST_BUF_BA_ADDR,
                    (outCfg->bufAddr >> CSL_LSE_DST_BUF_BA_ADDR_SHIFT));
                CSL_REG32_WR(&lseRegs->DST[cnt].BUF_BA, regVal);
            }
        }
    }
}

void CSL_lseStopChannels(CSL_lseRegs *lseRegs, const CSL_LseConfig *cfg)
{
    uint32_t cnt;
    volatile uint32_t           regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != cfg))
    {
    /* LDRA_JUSTIFY_END */
        for (cnt = 0U; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->inChCfg[cnt].bypass)
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC[cnt].BUF_BA[0]);
                CSL_FINS(regVal, LSE_SRC_BUF_BA_ENABLE, (uint32_t)0U);
                CSL_REG32_WR(&lseRegs->SRC[cnt].BUF_BA[0], regVal);
                regVal = CSL_REG32_RD(&lseRegs->SRC[cnt].BUF_BA[1]);
                CSL_FINS(regVal, LSE_SRC_BUF_BA_ENABLE, (uint32_t)0U);
                CSL_REG32_WR(&lseRegs->SRC[cnt].BUF_BA[1], regVal);
                regVal = CSL_REG32_RD(&lseRegs->SRC[cnt].BUF_BA[2]);
                CSL_FINS(regVal, LSE_SRC_BUF_BA_ENABLE, (uint32_t)0U);
                CSL_REG32_WR(&lseRegs->SRC[cnt].BUF_BA[2], regVal);
            }
        }
        for (cnt = 0U; cnt < CSL_LSE_MAX_OUTPUT_CH; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->outChCfg[cnt].bypass)
            {
                regVal = CSL_REG32_RD(&lseRegs->DST[cnt].BUF_BA);
                CSL_FINS(regVal, LSE_DST_BUF_BA_ENABLE, (uint32_t)0U);
                CSL_REG32_WR(&lseRegs->DST[cnt].BUF_BA, regVal);
            }
        }
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
<justification end> */
void CSL_lseUpdateWHOtfPrms(CSL_lseRegs *lseRegs, const CSL_LseConfig *cfg)
{
    uint32_t cnt;
    volatile uint32_t regVal;

    if ((NULL != lseRegs) && (NULL != cfg))
    {
        regVal = CSL_REG32_RD(&lseRegs->SRC[0].FRAME_SIZE);
        CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_WIDTH, cfg->inChCfg[0U].frameWidth);
        CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_HEIGHT, cfg->inChCfg[0U].frameHeight);
        CSL_REG32_WR(&lseRegs->SRC[0].FRAME_SIZE, regVal);

        CSL_REG32_FINS(&lseRegs->SRC[0].CFG,
                        LSE_SRC_CFG_VP_HBLNK_CNT,
                        cfg->inChCfg[0U].vpHBlankCnt);

        if ((uint32_t)UTRUE == cfg->vpCfg.enable)
        {
            CSL_REG32_FINS(&lseRegs->SRC[0].VPIN_CFG,
                LSE_SRC_VPIN_CFG_VPORT_EN, 1U);
        }
        else
        {
            CSL_REG32_FINS(&lseRegs->SRC[0].VPIN_CFG,
                LSE_SRC_VPIN_CFG_VPORT_EN, 0U);
        }

        for (cnt = 0U; cnt < CSL_LSE_MAX_OUTPUT_CH; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->outChCfg[cnt].bypass)
            {
                CSL_REG32_FINS(&lseRegs->DST[cnt].BUF_ATTR0,
                                LSE_DST_BUF_ATTR0_LOUT_SKIP_INIT,
                                cfg->outChCfg[cnt].numInitOutSkip);
            }
        }
    }
}
/* LDRA_JUSTIFY_END */

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static void LseSetH3aConfig(CSL_lseRegs *lseRegs,
    const CSL_LseH3aConfig *h3aCfg)
{
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != h3aCfg))
    {
    /* LDRA_JUSTIFY_END */
        if ((uint32_t)UTRUE == h3aCfg->enable)
        {
            regVal = CSL_REG32_RD(&lseRegs->H3A.BUF_ATTR);
            CSL_FINS(regVal, LSE_H3A_BUF_ATTR_CBUF_SIZE,
                h3aCfg->circBufSize);
            CSL_FINS(regVal, LSE_H3A_BUF_ATTR_H3A_LN_SIZE,
                (h3aCfg->lineOffset >> CSL_LSE_H3A_BUF_ATTR_H3A_LN_SIZE_SHIFT));
            CSL_REG32_WR(&lseRegs->H3A.BUF_ATTR, regVal);

            regVal = CSL_REG32_RD(&lseRegs->H3A.BUF_BA);
            CSL_FINS(regVal, LSE_H3A_BUF_BA_ADDR,
                (h3aCfg->bufAddr >> CSL_LSE_H3A_BUF_BA_ADDR_SHIFT));
            CSL_FINS(regVal, LSE_H3A_BUF_BA_ENABLE, (uint32_t)1U);
            CSL_REG32_WR(&lseRegs->H3A.BUF_BA, regVal);
        }
        else
        {
            CSL_REG32_FINS(&lseRegs->H3A.BUF_BA, LSE_H3A_BUF_BA_ENABLE, 0U);
        }
    }
}

static void LseSetVportConfig(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseVportConfig *vpCfg)
{
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != vpCfg))
    /* LDRA_JUSTIFY_END */ 
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */
        if ((uint32_t)UTRUE == vpCfg->enable)
        {
            regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].VPIN_CFG);

            CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_EN, 1U);
            CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_TWO_PIXEL,
                vpCfg->numPixels);

            if (FVID2_VIFW_8BIT == vpCfg->vifw)
            {
                CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_PW, 0U);
            }
            else if (FVID2_VIFW_12BIT == vpCfg->vifw)
            {
                CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_PW, 1U);
            }
            else if (FVID2_VIFW_14BIT == vpCfg->vifw)
            {
                CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_PW, 2U);
            }
            else
            {
                CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VPORT_PW, 3U);
            }
            CSL_FINS(regVal, LSE_SRC_VPIN_CFG_VP_PROTOCOL_CHK,
                vpCfg->enableProtocolFix);

            CSL_REG32_WR(&lseRegs->SRC[inCh].VPIN_CFG, regVal);

            regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].FRAME_SIZE);
            CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_WIDTH, vpCfg->width);
            CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_HEIGHT, vpCfg->height);
            CSL_REG32_WR(&lseRegs->SRC[inCh].FRAME_SIZE, regVal);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_FINS(&lseRegs->SRC[inCh].VPIN_CFG,
                LSE_SRC_VPIN_CFG_VPORT_EN, 0U);
        }
    }
}

static void LseConfigInBuff(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseInChConfig *inCfg)
{
    uint32_t cnt;
    volatile uint32_t regVal;

    /* This configures the additional buffers */
    for (cnt = 0U; cnt < (CSL_LSE_MAX_INPUT_BUF - 1u); cnt ++)
    {
        if(UTRUE == inCfg->buffConfig[cnt].enable)
        {
            if(0u == inCh)
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC0_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET));

                CSL_REG32_FINS(&lseRegs->SRC0_FRAME_SIZE1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET),
                                    LSE_SRC0_FRAME_SIZE1_WIDTH,
                                    inCfg->buffConfig[cnt].width);
            }
            else
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC1_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET));

                CSL_REG32_FINS(&lseRegs->SRC1_FRAME_SIZE1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET),
                                    LSE_SRC0_FRAME_SIZE1_WIDTH,
                                    inCfg->buffConfig[cnt].width);
            }

            CSL_FINS(regVal, LSE_SRC0_CFG1_ENABLE_CHAN_SPECIFIC_PARAMS,
                        (uint32_t)1U);
            CSL_FINS(regVal, LSE_SRC0_CFG1_SKIP_SL2_READS,
                        inCfg->buffConfig[cnt].skipSl2Read);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            switch (inCfg->buffConfig[cnt].ccsf)
            {
            /* LDRA_JUSTIFY_END */
                case FVID2_CCSF_BITS8_PACKED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 0U);
                    break;
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The test framework and test apps cannot reach this portion.
                    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                    <justification end> */
                case FVID2_CCSF_BITS8_UNPACKED12:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED12_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                /* LDRA_JUSTIFY_END */
                case FVID2_CCSF_BITS12_PACKED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS12_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The test framework and test apps cannot reach this portion.
                    The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                    Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                    <justification end> */
                case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS14_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 2U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS14_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 2U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS16_PACKED:
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_PW, 3U);
                    CSL_FINS(regVal, LSE_SRC0_CFG1_PIX_FMT_CNTRSZ, 2U);
                    break;
                default:
                    /* Nothing to do here */
                    break;
                /* LDRA_JUSTIFY_END */
            }

            if(0u == inCh)
            {
                CSL_REG32_WR((&lseRegs->SRC0_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET)), regVal);
            }
            else
            {
                CSL_REG32_WR((&lseRegs->SRC1_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET)), regVal);
            }
        }
        else
        {
            if(0u == inCh)
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC0_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET));

                CSL_FINS(regVal, LSE_SRC0_CFG1_ENABLE_CHAN_SPECIFIC_PARAMS,
                            (uint32_t)0U);
                CSL_FINS(regVal, LSE_SRC0_CFG1_SKIP_SL2_READS,
                            (uint32_t)0U);

                CSL_REG32_WR((&lseRegs->SRC0_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET)), regVal);
            }
            else
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC1_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET));

                CSL_FINS(regVal, LSE_SRC0_CFG1_ENABLE_CHAN_SPECIFIC_PARAMS,
                            (uint32_t)0U);
                CSL_FINS(regVal, LSE_SRC0_CFG1_SKIP_SL2_READS,
                            (uint32_t)0U);

                CSL_REG32_WR((&lseRegs->SRC1_CFG1 +
                                (cnt * CSL_LSE_SRC_BUFF_CFG_OFFSET)), regVal);
            }
        }
    }
}

static void LseSetInChConfig(CSL_lseRegs *lseRegs, uint32_t inCh,
    const CSL_LseInChConfig *inCfg)
{
    volatile uint32_t regVal;
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != inCfg))
    {
    /* LDRA_JUSTIFY_END */ 
        if ((uint32_t)UTRUE == inCfg->enable)
        {
            regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].CFG);

            CSL_FINS(regVal, LSE_SRC_CFG_VP_HBLNK_CNT, inCfg->vpHBlankCnt);

            if(UTRUE == inCfg->enableMultiTapKnlConfig)
            {
                CSL_FINS(regVal, LSE_SRC_CFG_KERN_TPAD_SZ, inCfg->knTopPadding);
                CSL_FINS(regVal, LSE_SRC_CFG_KERN_BPAD_SZ,
                    inCfg->knBottomPadding);
                CSL_FINS(regVal, LSE_SRC_CFG_KERN_LN_OFFSET, inCfg->knLineOffset);
                CSL_FINS(regVal, LSE_SRC_CFG_KERN_SZ_HEIGHT, inCfg->knHeight);
            }
            CSL_FINS(regVal, LSE_SRC_CFG_SRC_LN_INC_2,
                inCfg->enableAddrIncrBy2);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
            An else block can added to catch and print the error status in a future release.
            <justification end> */
            switch (inCfg->ccsf)
            {
            /* LDRA_JUSTIFY_END */
                case FVID2_CCSF_BITS8_PACKED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 0U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED12:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED12_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS12_PACKED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS12_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS14_UNPACKED16:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 2U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS14_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 2U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS16_PACKED:
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_PW, 3U);
                    CSL_FINS(regVal, LSE_SRC_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	            An else block can added to catch and print the error status in a future release.
                <justification end> */
                default:
                    /* Nothing to do here */
                    break;
                /* LDRA_JUSTIFY_END */
            }
            #if defined(TEST_VISS_WDTIMER_ERR_INTERRUPT)
            regVal = regVal & 0x003FFFFF;
            #endif
            CSL_REG32_WR(&lseRegs->SRC[inCh].CFG, regVal);

            regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].FRAME_SIZE);
            CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_WIDTH, inCfg->frameWidth);
            CSL_FINS(regVal, LSE_SRC_FRAME_SIZE_HEIGHT, inCfg->frameHeight);
            CSL_REG32_WR(&lseRegs->SRC[inCh].FRAME_SIZE, regVal);

            regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].BUF_ATTR);
            CSL_FINS(regVal, LSE_SRC_BUF_ATTR_START_NIB_OFFSET,
                inCfg->startOffset);
            CSL_FINS(regVal, LSE_SRC_BUF_ATTR_CBUF_SIZE,
                inCfg->circBufSize);
            CSL_FINS(regVal, LSE_SRC_BUF_ATTR_BUF_STRIDE,
                (inCfg->lineOffset >> CSL_LSE_SRC_BUF_ATTR_BUF_STRIDE_SHIFT));
            CSL_REG32_WR(&lseRegs->SRC[inCh].BUF_ATTR, regVal);

            for (cnt = 0U; cnt < inCfg->numInBuff; cnt ++)
            {
                regVal = CSL_REG32_RD(&lseRegs->SRC[inCh].BUF_BA[cnt]);
                CSL_FINS(regVal, LSE_SRC_BUF_BA_ENABLE, (uint32_t)1U);
                CSL_FINS(regVal, LSE_SRC_BUF_BA_ADDR,
                    (inCfg->bufAddr[cnt] >> CSL_LSE_SRC_BUF_BA_ADDR_SHIFT));
                CSL_FINS(regVal, LSE_SRC_BUF_BA_SKIP_ALTERNATE_LINE_PROC,
                    (inCfg->skipAltLine[cnt]));
                CSL_FINS(regVal, LSE_SRC_BUF_BA_SKIP_ODD_LINE_PROC,
                    (inCfg->skipOddLine[cnt]));

                if ((uint32_t)UTRUE == inCfg->enableYuv422I)
                {
                    CSL_FINS(regVal,
                             LSE_SRC_BUF_BA_ENABLE_INTERLEAVED_PIXEL_EXTRACTION,
                             (uint32_t)1U);
                    if (((0u == cnt) &&
                        (FVID2_DF_YUV422I_YUYV == inCfg->yuv422DataFmt)) ||
                        ((1u == cnt) &&
                        (FVID2_DF_YUV422I_UYVY == inCfg->yuv422DataFmt)))
                    {
                        CSL_FINS(regVal,
                                 LSE_SRC_BUF_BA_EXTRACT_INTERLEAVED_ODD_PIXELS,
                                (uint32_t)1U);
                    }
                    else
                    {
                        CSL_FINS(regVal,
                                 LSE_SRC_BUF_BA_EXTRACT_INTERLEAVED_ODD_PIXELS,
                                (uint32_t)0U);
                    }
                }
                else
                {
                    CSL_FINS(regVal,
                             LSE_SRC_BUF_BA_ENABLE_INTERLEAVED_PIXEL_EXTRACTION,
                             (uint32_t)0U);
                    CSL_FINS(regVal,
                                 LSE_SRC_BUF_BA_EXTRACT_INTERLEAVED_ODD_PIXELS,
                                (uint32_t)0U);
                }

                CSL_REG32_WR(&lseRegs->SRC[inCh].BUF_BA[cnt], regVal);
            }

            LseConfigInBuff(lseRegs, inCh, inCfg);
        }
        else
        {
            CSL_REG32_FINS(&lseRegs->SRC[inCh].BUF_BA[0U],
                LSE_SRC_BUF_BA_ENABLE, 0U);
            CSL_REG32_FINS(&lseRegs->SRC[inCh].BUF_BA[1U],
                LSE_SRC_BUF_BA_ENABLE, 0U);
            CSL_REG32_FINS(&lseRegs->SRC[inCh].BUF_BA[2U],
                LSE_SRC_BUF_BA_ENABLE, 0U);
        }
    }
}

static void LseSetOutChConfig(CSL_lseRegs *lseRegs, uint32_t outCh,
    const CSL_LseOutChConfig *outCfg)
{
    uint32_t cnt, idx;
    volatile uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != outCfg))
    {
    /* LDRA_JUSTIFY_END */
        if ((uint32_t)UTRUE == outCfg->enable)
        {
            regVal = CSL_REG32_RD(&lseRegs->DST[outCh].BUF_CFG);

            if ((uint32_t)UTRUE == outCfg->enableYuv422Out)
            {
                CSL_FINS(regVal, LSE_DST_BUF_CFG_YUV422_OUT_EN, (uint32_t)1U);
                if (FVID2_DF_YUV422I_UYVY == outCfg->yuv422DataFmt)
                {
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_YUV422_INTLV_ORDER,
                            (uint32_t)0U);
                }
                else
                {
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_YUV422_INTLV_ORDER,
                            (uint32_t)1U);
                }
            }
            else
            {
                CSL_FINS(regVal, LSE_DST_BUF_CFG_YUV422_OUT_EN, (uint32_t)0U);
                CSL_FINS(regVal, LSE_DST_BUF_CFG_YUV422_INTLV_ORDER,
                            (uint32_t)0U);
            }

            CSL_FINS(regVal, LSE_DST_BUF_CFG_ENABLE_OUTPUT_PIXEL_ROUNDING,
                (uint32_t)outCfg->enableRounding);

            CSL_FINS(regVal, LSE_DST_BUF_CFG_THREAD_MAP,
                (uint32_t)outCfg->thrId);
            CSL_FINS(regVal, LSE_DST_BUF_CFG_CHAN_THREAD_MAP,
                (uint32_t)outCfg->chThrId);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            switch (outCfg->ccsf)
            {
			/* LDRA_JUSTIFY_END */
                case FVID2_CCSF_BITS8_PACKED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 0U);
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The negative test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
                <justification end> */
                case FVID2_CCSF_BITS8_UNPACKED12:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED12_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS10_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS12_PACKED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 1U);
                    break;
                /* LDRA_JUSTIFY_END */
                case FVID2_CCSF_BITS12_UNPACKED16:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                case FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 1U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                <justification end> */
                case FVID2_CCSF_BITS16_PACKED:
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_ALIGN, 0U);

                /* Pixel width and container size should be set to 8bit for YUV422 merge enable */
                if ((uint32_t)TRUE == outCfg->enableYuv422Out)
                {
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 0U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 0U);
                }
                else
                {
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_PW, 3U);
                    CSL_FINS(regVal, LSE_DST_BUF_CFG_PIX_FMT_CNTRSZ, 2U);
                }
                    break;
                /* LDRA_JUSTIFY_END */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	            An else block can added to catch and print the error status in a future release.
                <justification end> */
                default:
                    /* Nothing to do here */
                    break;
                /* LDRA_JUSTIFY_END */
            }
            #if defined(TEST_MSC_WDTIMER_ERR_INTERRUPT)
            regVal = regVal | 0x00000100;
            #endif
            CSL_REG32_WR(&lseRegs->DST[outCh].BUF_CFG, regVal);

            regVal = CSL_REG32_RD(&lseRegs->DST[outCh].BUF_ATTR0);
            CSL_FINS(regVal, LSE_DST_BUF_ATTR0_LOUT_SKIP_INIT,
                outCfg->numInitOutSkip);
            CSL_FINS(regVal, LSE_DST_BUF_ATTR0_CBUF_SIZE,
                outCfg->circBufSize);
            CSL_FINS(regVal, LSE_DST_BUF_ATTR0_BUF_STRIDE,
                (outCfg->lineOffset >> CSL_LSE_DST_BUF_ATTR0_BUF_STRIDE_SHIFT));
            CSL_REG32_WR(&lseRegs->DST[outCh].BUF_ATTR0, regVal);

            regVal = CSL_REG32_RD(&lseRegs->DST[outCh].BUF_ATTR1);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if ((uint32_t)UTRUE == outCfg->enableVertWrap)
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_CBUF_VWRAP_EN, 1U);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_CBUF_VWRAP_EN, 0U);
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if ((uint32_t)UTRUE == outCfg->useMultiBprParams)
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_BPR_SEL_MODE, 1U);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_BPR_SEL_MODE, 0U);
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_CBUF_BPR_CHAN, outCfg->bpr);
            }

            if ((uint32_t)UTRUE == outCfg->enableTDoneEoBPR)
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_TDONE_GEN_MODE, 1U);
            }
            else
            {
                CSL_FINS(regVal, LSE_DST_BUF_ATTR1_TDONE_GEN_MODE, 0U);
            }

            CSL_REG32_WR(&lseRegs->DST[outCh].BUF_ATTR1, regVal);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if ((uint32_t)UTRUE == outCfg->useMultiBprParams)
            {
                idx = 0U;
                for (cnt = 0u; cnt < (CSL_LSE_BPR_MAX_REGIONS/3U); cnt ++)
                {
                    regVal = CSL_REG32_RD(&lseRegs->COMMON_CFG_[cnt].ROW);
                    CSL_FINS(regVal, LSE_COMMON_CFG__ROW_BPR0,
                        outCfg->regBpr[idx]);
                    CSL_FINS(regVal, LSE_COMMON_CFG__ROW_BPR1,
                        outCfg->regBpr[idx + 1U]);
                    CSL_FINS(regVal, LSE_COMMON_CFG__ROW_BPR2,
                        outCfg->regBpr[idx + 2U]);
                    CSL_REG32_WR(&lseRegs->COMMON_CFG_[cnt].ROW, regVal);

                    idx += CSL_LSE_BPR_MAX_REGIONS/3U;
                }
            }
            /* LDRA_JUSTIFY_END */

            regVal = CSL_REG32_RD(&lseRegs->DST[outCh].BUF_BA);
            CSL_FINS(regVal, LSE_DST_BUF_BA_ENABLE, (uint32_t)1U);
            CSL_FINS(regVal, LSE_DST_BUF_BA_ADDR,
                (outCfg->bufAddr >> CSL_LSE_DST_BUF_BA_ADDR_SHIFT));
            CSL_FINS(regVal, LSE_DST_BUF_BA_SKIP_ALTERNATE_LINE_PROC,
                    (outCfg->skipAltLine));
                CSL_FINS(regVal, LSE_DST_BUF_BA_SKIP_ODD_LINE_PROC,
                    (outCfg->skipOddLine));
            CSL_REG32_WR(&lseRegs->DST[outCh].BUF_BA, regVal);
        }
        else
        {
            CSL_REG32_FINS(&lseRegs->DST[outCh].BUF_BA,
                LSE_DST_BUF_BA_ENABLE, 0U);
        }
    }
}

static void LseSetLpbkConfig(CSL_lseRegs *lseRegs,
    const CSL_LseLpbkConfig *lpbkCfg)
{
    uint32_t regVal;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
	Rationale: The component level negative test framework and test applications cannot reach this portion.
	The parameters are pre-validated by the caller before the control reaches here.
	Effect on this unit: If the control reaches here, our code base is NOT expected to accumulate the error status.
	An else block can be added to catch and print the error status in a future release. 
	<justification end> */
    if ((NULL != lseRegs) && (NULL != lpbkCfg))
    {
    /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == lpbkCfg->enable)
        {
            regVal = CSL_REG32_RD(&lseRegs->CFG_LSE);
            CSL_FINS(regVal, LSE_CFG_LSE_LOOPBACK_CORE_EN,
                (uint32_t)lpbkCfg->coreEnable);
            CSL_FINS(regVal, LSE_CFG_LSE_LOOPBACK_IN_CH_SEL,
                (uint32_t)lpbkCfg->inCh);
            CSL_FINS(regVal, LSE_CFG_LSE_LOOPBACK_EN, 1U);
            CSL_REG32_WR(&lseRegs->CFG_LSE, regVal);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_FINS(&lseRegs->CFG_LSE, LSE_CFG_LSE_LOOPBACK_EN, 0U);
        }
    }
}

static int32_t LseCheckCfg(const CSL_LseConfig *cfg)
{
    int32_t                 status = FVID2_SOK;
    uint32_t                cnt;
    const CSL_LseOutChConfig   *outCfg;
    const CSL_LseInChConfig    *inCfg;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((uint32_t)UTRUE == cfg->lpbkCfg.enable)
    {
        if ((0U != cfg->lpbkCfg.inCh) && (1U != cfg->lpbkCfg.inCh) &&
            (2U != cfg->lpbkCfg.inCh))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        if (((uint32_t)UTRUE != cfg->lpbkCfg.coreEnable) &&
            ((uint32_t)UFALSE != cfg->lpbkCfg.coreEnable))
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }

    if ((uint32_t)UTRUE == cfg->vpCfg.enable)
    {
        if ((0U != cfg->vpCfg.numPixels) && (1U != cfg->vpCfg.numPixels))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        if ((FVID2_VIFW_8BIT != cfg->vpCfg.vifw) &&
            (FVID2_VIFW_12BIT != cfg->vpCfg.vifw) &&
            (FVID2_VIFW_14BIT != cfg->vpCfg.vifw) &&
            (FVID2_VIFW_16BIT != cfg->vpCfg.vifw))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        if (((uint32_t)UTRUE != cfg->vpCfg.enableProtocolFix) &&
            ((uint32_t)UFALSE != cfg->vpCfg.enableProtocolFix))
        {
            status = FVID2_EINVALID_PARAMS;
        }
    }
    /* LDRA_JUSTIFY_END */
    if ((uint32_t)UTRUE == cfg->h3aCfg.enable)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != (cfg->h3aCfg.lineOffset & CSL_LSE_ALIGN_MASK))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the caller before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != (cfg->h3aCfg.bufAddr & CSL_LSE_ALIGN_MASK))
        {
            status = FVID2_EINVALID_PARAMS;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (cfg->numInCh > CSL_LSE_MAX_INPUT_CH)
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (cfg->numOutCh > CSL_LSE_MAX_OUTPUT_CH)
    {
        status = FVID2_EINVALID_PARAMS;
    }
    /* LDRA_JUSTIFY_END */

    for (cnt = 0U; cnt < CSL_LSE_MAX_OUTPUT_CH; cnt ++)
    {
        if (FVID2_SOK != status)
        {
            break;
        }
        outCfg = &cfg->outChCfg[cnt];

        if ((uint32_t)UTRUE == outCfg->enable)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The negative test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: Unused feature, cannot be enabled, control cannot reach here.
            <justification end> */
            if ((FVID2_CCSF_BITS8_PACKED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED12 != outCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED16 != outCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED12_MSB_ALIGNED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS10_UNPACKED16 != outCfg->ccsf) &&
                (FVID2_CCSF_BITS10_UNPACKED16_MSB_ALIGNED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS12_PACKED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS12_UNPACKED16 != outCfg->ccsf) &&
                (FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED != outCfg->ccsf) &&
                (FVID2_CCSF_BITS16_PACKED != outCfg->ccsf))
             /* LDRA_JUSTIFY_END */
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
            if (0U != (outCfg->lineOffset & CSL_LSE_ALIGN_MASK))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the caller before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != (outCfg->bufAddr & CSL_LSE_ALIGN_MASK))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            if ((uint32_t)UTRUE == outCfg->enableYuv422Out)
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the caller before the control reaches here.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((FVID2_DF_YUV422I_UYVY != outCfg->yuv422DataFmt) &&
                    (FVID2_DF_YUV422I_YUYV != outCfg->yuv422DataFmt))
                {
                    status = FVID2_EINVALID_PARAMS;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }

    for (cnt = 0U; cnt < CSL_LSE_MAX_INPUT_CH; cnt ++)
    {
        if (FVID2_SOK != status)
        {
            break;
        }
        inCfg = &cfg->inChCfg[cnt];

        if ((uint32_t)UTRUE == inCfg->enable)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((inCfg->vpHBlankCnt > 1023U) || (inCfg->vpHBlankCnt == 1U))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((UTRUE == inCfg->enableMultiTapKnlConfig) &&
                ((inCfg->knTopPadding > 4U) ||
                 (inCfg->knBottomPadding > 4U) ||
                 (inCfg->knLineOffset > 4U)))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((UTRUE == inCfg->enableMultiTapKnlConfig) && ((inCfg->knHeight < 1U) || (inCfg->knHeight > 5U)))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (((uint32_t)UTRUE != inCfg->enableAddrIncrBy2) &&
                ((uint32_t)UFALSE != inCfg->enableAddrIncrBy2))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            if ((FVID2_CCSF_BITS8_PACKED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED12 != inCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED16 != inCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED12_MSB_ALIGNED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS10_UNPACKED16 != inCfg->ccsf) &&
                (FVID2_CCSF_BITS10_UNPACKED16_MSB_ALIGNED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS12_PACKED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS12_UNPACKED16 != inCfg->ccsf) &&
                (FVID2_CCSF_BITS14_UNPACKED16 != inCfg->ccsf) &&
                (FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS14_UNPACKED16_MSB_ALIGNED != inCfg->ccsf) &&
                (FVID2_CCSF_BITS16_PACKED != inCfg->ccsf))
            {
                status = FVID2_EINVALID_PARAMS;
            }

            if ((inCfg->frameWidth > CSL_LSE_SRC_FRAME_SIZE_WIDTH_MASK) ||
                (inCfg->frameHeight > (CSL_LSE_SRC_FRAME_SIZE_HEIGHT_MASK >>
                    CSL_LSE_SRC_FRAME_SIZE_HEIGHT_SHIFT)))
            {
                status = FVID2_EINVALID_PARAMS;
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != (inCfg->lineOffset & CSL_LSE_ALIGN_MASK))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((1U != inCfg->numInBuff) && (2U != inCfg->numInBuff) &&
                (3U != inCfg->numInBuff))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            /* LDRA_JUSTIFY_END */
            
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U != (inCfg->bufAddr[0U] & CSL_LSE_ALIGN_MASK))
            {
                status = FVID2_EINVALID_PARAMS;
            }
            if (3U == inCfg->numInBuff)
            {
                if ((0U != (inCfg->bufAddr[1U] & CSL_LSE_ALIGN_MASK)) ||
                    (0U != (inCfg->bufAddr[2U] & CSL_LSE_ALIGN_MASK)))
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
            if (2U == inCfg->numInBuff)
            {
                if (0U != (inCfg->bufAddr[1U] & CSL_LSE_ALIGN_MASK))
                {
                    status = FVID2_EINVALID_PARAMS;
                }
            }
            /* LDRA_JUSTIFY_END */
        }
    }
    return (status);
}

