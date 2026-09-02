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
 *
 */

/**
 *  \file vhwa_soc.c
 *
 *  \brief File containing the SOC related configuration functions for VHWA.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/hw_include/soc_config.h>
#include <drivers/hw_include/cslr_soc.h>
#include <soc/vhwa_soc.h>
#include <include/vhwa_common.h>
#include <soc/vhwa_vpac_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

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
#if defined (VHWA_VPAC_IP_REV_VPAC3L)
/* Global containing SoC information. */
static Msc_SocInfo gMsc_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_CP_INTD_CFG_INTD_CFG_BASE,
    (CSL_mscRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_MSC_CFG_VP_CFG_VP_BASE,
    (CSL_lseRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_MSC_CFG_VP_LSE_CFG_VP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_HTS_S_VBUSP_BASE,
    1u,
};

static Ldc_SocInfo gLdc_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_CP_INTD_CFG_INTD_CFG_BASE,
    (CSL_ldc_coreRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_LDC0_S_VBUSP_MMR_VBUSP_BASE,
    (CSL_lseRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_LDC0_S_VBUSP_VPAC_LDC_LSE_CFG_VP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_HTS_S_VBUSP_BASE,
    (uint32_t *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_LDC0_S_VBUSP_PIXWRINTF_DUALY_LUTCFG_DUALY_LUT_BASE,
    (uint32_t *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_LDC0_S_VBUSP_PIXWRINTF_DUALC_LUTCFG_DUALC_LUT_BASE
};

static Viss_SocInfo gViss_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_CP_INTD_CFG_INTD_CFG_BASE,
    (uint32_t)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_HTS_S_VBUSP_BASE,
    (CSL_pcidRegs *)CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_PAR_VPAC_VISS0_S_VBUSP_VISS_PCID_S_VBUSP_MMRCFG_PCID_BASE,
};
#else

/* Global containing SoC information. */
static Msc_SocInfo gMsc_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_CP_INTD_CFG_INTD_CFG_BASE,
    (CSL_mscRegs *)CSL_VPAC0_PAR_VPAC_MSC_CFG_VP_CFG_VP_BASE,
    (CSL_lseRegs *)CSL_VPAC0_PAR_VPAC_MSC_CFG_VP_LSE_CFG_VP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_HTS_S_VBUSP_BASE,
    1u
};

static Ldc_SocInfo gLdc_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_CP_INTD_CFG_INTD_CFG_BASE,
    (CSL_ldc_coreRegs *)CSL_VPAC0_PAR_VPAC_LDC0_S_VBUSP_MMR_VBUSP_BASE,
    (CSL_lseRegs *)CSL_VPAC0_PAR_VPAC_LDC0_S_VBUSP_VPAC_LDC_LSE_CFG_VP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_HTS_S_VBUSP_BASE,
    (uint32_t *)CSL_VPAC0_PAR_VPAC_LDC0_S_VBUSP_PIXWRINTF_DUALY_LUTCFG_DUALY_LUT_BASE,
    (uint32_t *)CSL_VPAC0_PAR_VPAC_LDC0_S_VBUSP_PIXWRINTF_DUALC_LUTCFG_DUALC_LUT_BASE
};

static Viss_SocInfo gViss_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_CP_INTD_CFG_INTD_CFG_BASE,
    (uint32_t)CSL_VPAC0_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_HTS_S_VBUSP_BASE
};

static Nf_SocInfo gNf_SocInfo = {
    (CSL_vpac_cntlRegs *)CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE,
    (CSL_vpac_intd_cfgRegs *)CSL_VPAC0_CP_INTD_CFG_INTD_CFG_BASE,
    (CSL_vpac_nfRegs *)CSL_VPAC0_PAR_VPAC_NF_S_VBUSP_MMR_VBUSP_NF_CFG_BASE,
    (CSL_lseRegs *)CSL_VPAC0_PAR_VPAC_NF_S_VBUSP_VPAC_NF_LSE_CFG_VP_BASE,
    (CSL_htsRegs *)CSL_VPAC0_HTS_S_VBUSP_BASE
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Msc_getSocInfo(Msc_SocInfo *socInfo)
{
    if (NULL != socInfo)
    {
        socInfo->vpacCntlRegs = gMsc_SocInfo.vpacCntlRegs;
        socInfo->vpacIntdRegs = gMsc_SocInfo.vpacIntdRegs;
        socInfo->mscRegs      = gMsc_SocInfo.mscRegs;
        socInfo->lseRegs      = gMsc_SocInfo.lseRegs;
        socInfo->htsRegs      = gMsc_SocInfo.htsRegs;
        socInfo->isDualChannel = gMsc_SocInfo.isDualChannel;
    }
}

void Msc_getIrqInfo(Vhwa_IrqInfo irqInfo[])
{
    if (NULL != irqInfo)
    {
        irqInfo[0U].irqNum     = VHWA_M2M_MSC0_IRQ_NUM;
        irqInfo[0U].vhwaIrqNum = VHWA_M2M_IRQ_NUM_1;

        irqInfo[1U].irqNum     = VHWA_M2M_MSC1_IRQ_NUM;
        irqInfo[1U].vhwaIrqNum = VHWA_M2M_IRQ_NUM_2;
    }
}

void Ldc_getSocInfo(Ldc_SocInfo *socInfo)
{
    if (NULL != socInfo)
    {
        socInfo->vpacCntlRegs = gLdc_SocInfo.vpacCntlRegs;
        socInfo->vpacIntdRegs = gLdc_SocInfo.vpacIntdRegs;
        socInfo->ldcRegs      = gLdc_SocInfo.ldcRegs;
        socInfo->lseRegs      = gLdc_SocInfo.lseRegs;
        socInfo->htsRegs      = gLdc_SocInfo.htsRegs;
        socInfo->lumaLutBaseAddr = gLdc_SocInfo.lumaLutBaseAddr;
        socInfo->chromaLutBaseAddr = gLdc_SocInfo.chromaLutBaseAddr;
    }
}

void Ldc_getIrqInfo(Vhwa_IrqInfo *irqInfo)
{
    if (NULL != irqInfo)
    {
        irqInfo->irqNum     = VHWA_M2M_LDC_IRQ_NUM;
        irqInfo->vhwaIrqNum = VHWA_M2M_IRQ_NUM_3;
    }
}

void Viss_getSocInfo(Viss_SocInfo *socInfo)
{
    if (NULL != socInfo)
    {
        socInfo->vpacCntlRegs = gViss_SocInfo.vpacCntlRegs;
        socInfo->vpacIntdRegs = gViss_SocInfo.vpacIntdRegs;
        socInfo->vissBaseAddr = gViss_SocInfo.vissBaseAddr;
        socInfo->htsRegs      = gViss_SocInfo.htsRegs;
    }
}

void Viss_getIrqInfo(Vhwa_IrqInfo *irqInfo)
{
    if (NULL != irqInfo)
    {
        irqInfo->irqNum     = VHWA_M2M_VISS_IRQ_NUM;
        irqInfo->vhwaIrqNum = VHWA_M2M_IRQ_NUM_0;
    }
}
#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
void Nf_getSocInfo(Nf_SocInfo *socInfo)
{
    if (NULL != socInfo)
    {
        socInfo->vpacCntlRegs = gNf_SocInfo.vpacCntlRegs;
        socInfo->vpacIntdRegs = gNf_SocInfo.vpacIntdRegs;
        socInfo->nfRegs       = gNf_SocInfo.nfRegs;
        socInfo->lseRegs      = gNf_SocInfo.lseRegs;
        socInfo->htsRegs      = gNf_SocInfo.htsRegs;
    }
}

void Nf_getIrqInfo(Vhwa_IrqInfo *irqInfo)
{
    if (NULL != irqInfo)
    {
        irqInfo->irqNum     = VHWA_M2M_NF_IRQ_NUM;
        irqInfo->vhwaIrqNum = VHWA_M2M_IRQ_NUM_4;
    }
}
#endif

void Fc_getSocHtsInfo(CSL_htsRegs **htsRegs)
{
    if (NULL != htsRegs)
    {
        *htsRegs      = gViss_SocInfo.htsRegs;
    }
}

