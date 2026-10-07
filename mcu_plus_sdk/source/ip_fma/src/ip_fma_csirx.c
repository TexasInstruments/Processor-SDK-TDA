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
 *  \ingroup  IP_FMA_CSIRX
 *  \defgroup IP_FMA_CSIRX_IMPLEMENTATION CSIRX module Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_csirx.c
 *
 *  \brief    CSIRX-specific wrappers over IP_FMA Register Check.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ip_fma_csirx.h"
#include <drivers/csirx/v0/hw_include/cslr_csi_rx_if.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \brief DPHY configuration related macros: Register offsets
 */
#define DPHYRX_WRAP_REGS_LANE		                          ((uint32_t)0U)
#define	DPHYRX_PCS_TX_DIG_TBIT0                               ((uint32_t)0xB00U)
#define	DPHYRX_PCS_TX_DIG_TBIT2                               ((uint32_t)0xB08U)
#define	DPHYRX_PCS_TX_DIG_TBIT3                               ((uint32_t)0xB0CU)
#define	DPHYRX_CMN0_CMN_DIG_TBIT2                             ((uint32_t)0x20U)
#define	DPHYRX_ISO_PHY_ISO_CL_CNTRL_L                         ((uint32_t)0xC10U)
#define	DPHYRX_ISO_PHY_ISO_DL_CTRL_L0                         ((uint32_t)0xC14U)
#define	DPHYRX_ISO_PHY_ISO_DL_CTRL_L1                         ((uint32_t)0xC20U)
#define	DPHYRX_ISO_LDD_PHY_ISO_DL_CTRL_L2                     ((uint32_t)0xC30U)
#define	DPHYRX_ISO_LDD_PHY_ISO_DL_CTRL_L3                     ((uint32_t)0xC3CU)
#define	DPHYRX_CMN0_CMN_DIG_TBIT56                            ((uint32_t)0xF0U)
#define	DPHYRX_CMN0_CMN_DIG_TBIT35                            ((uint32_t)0xA4U)
#define	DPHYRX_CMN0_CMN_DIG_TBIT36                            ((uint32_t)0xA8U)
#define CNTX_CNTL_SIZE                                        ((uint32_t)32U)

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Csirx_GetDphyRegs(uintptr_t baseAddr, IpFma_CsirxDphyRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc registerDescriptorsArr[] =
        {
            { DPHYRX_PCS_TX_DIG_TBIT0,            0u, IPFMA_WIDTH_32 },
            { DPHYRX_PCS_TX_DIG_TBIT2,            0u, IPFMA_WIDTH_32 },
            { DPHYRX_PCS_TX_DIG_TBIT3,            0u, IPFMA_WIDTH_32 },
            { DPHYRX_CMN0_CMN_DIG_TBIT2,          0u, IPFMA_WIDTH_32 },
            { DPHYRX_ISO_PHY_ISO_CL_CNTRL_L,      0u, IPFMA_WIDTH_32 },
            { DPHYRX_ISO_PHY_ISO_DL_CTRL_L0,      0u, IPFMA_WIDTH_32 },
            { DPHYRX_ISO_PHY_ISO_DL_CTRL_L1,      0u, IPFMA_WIDTH_32 },
            { DPHYRX_ISO_LDD_PHY_ISO_DL_CTRL_L2,  0u, IPFMA_WIDTH_32 },
            { DPHYRX_ISO_LDD_PHY_ISO_DL_CTRL_L3,  0u, IPFMA_WIDTH_32 },
            { DPHYRX_CMN0_CMN_DIG_TBIT56,         0u, IPFMA_WIDTH_32 },
            { DPHYRX_CMN0_CMN_DIG_TBIT35,         0u, IPFMA_WIDTH_32 },
            { DPHYRX_CMN0_CMN_DIG_TBIT36,         0u, IPFMA_WIDTH_32 }
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));
        
        if (IPFMA_OK == status)
        {
            out->pcs_tx_dig_tbit0            = registerDescriptorsArr[0].value;
            out->pcs_tx_dig_tbit2            = registerDescriptorsArr[1].value;
            out->pcs_tx_dig_tbit3            = registerDescriptorsArr[2].value;
            out->cmn0_cmn_dig_tbit2          = registerDescriptorsArr[3].value;
            out->iso_phy_iso_cl_cntrl_l      = registerDescriptorsArr[4].value;
            out->iso_phy_iso_dl_ctrl_l0      = registerDescriptorsArr[5].value;
            out->iso_phy_iso_dl_ctrl_l1      = registerDescriptorsArr[6].value;
            out->iso_ldd_phy_iso_dl_ctrl_l2  = registerDescriptorsArr[7].value;
            out->iso_ldd_phy_iso_dl_ctrl_l3  = registerDescriptorsArr[8].value;
            out->cmn0_cmn_dig_tbit56         = registerDescriptorsArr[9].value;
            out->cmn0_cmn_dig_tbit35         = registerDescriptorsArr[10].value;
            out->cmn0_cmn_dig_tbit36         = registerDescriptorsArr[11].value;
        }
    }

    return status;
}

IpFma_Status IpFma_Csirx_CompareDphyRegs(const IpFma_CsirxDphyRegs* expected,
                                         const IpFma_CsirxDphyRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (((NULL_PTR == expected)) || ((NULL_PTR == actual)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if ((expected->pcs_tx_dig_tbit0 != actual->pcs_tx_dig_tbit0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->pcs_tx_dig_tbit2 != actual->pcs_tx_dig_tbit2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->pcs_tx_dig_tbit3 != actual->pcs_tx_dig_tbit3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->cmn0_cmn_dig_tbit2 != actual->cmn0_cmn_dig_tbit2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->iso_phy_iso_cl_cntrl_l != actual->iso_phy_iso_cl_cntrl_l))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->iso_phy_iso_dl_ctrl_l0 != actual->iso_phy_iso_dl_ctrl_l0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->iso_phy_iso_dl_ctrl_l1 != actual->iso_phy_iso_dl_ctrl_l1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->iso_ldd_phy_iso_dl_ctrl_l2 != actual->iso_ldd_phy_iso_dl_ctrl_l2))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->iso_ldd_phy_iso_dl_ctrl_l3 != actual->iso_ldd_phy_iso_dl_ctrl_l3))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->cmn0_cmn_dig_tbit56 != actual->cmn0_cmn_dig_tbit56))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->cmn0_cmn_dig_tbit35 != actual->cmn0_cmn_dig_tbit35))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->cmn0_cmn_dig_tbit36 != actual->cmn0_cmn_dig_tbit36))
        {
            status = IPFMA_E_MISMATCH;
        }
    }

    return status;
}

IpFma_Status IpFma_Csirx_GetShimRegs(uintptr_t baseAddr, IpFma_CsirxShimRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_CSI_RX_IF_CSIRX_ID,                      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_VP0,                           0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_VP1,                           0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTL,                          0u, IPFMA_WIDTH_32 },

            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x0U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x2U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x3U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x4U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x5U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x6U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x7U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x8U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x9U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xAU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xBU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xCU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xDU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xEU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0xFU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x10U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x11U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x12U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x13U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x14U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x15U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x16U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x17U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x18U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x19U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1AU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1BU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1CU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1DU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1EU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_DMACNTX(0x1FU),      0u, IPFMA_WIDTH_32 },

            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x0U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x2U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x3U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x4U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x5U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x6U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x7U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x8U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x9U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xAU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xBU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xCU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xDU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xEU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0xFU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x10U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x11U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x12U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x13U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x14U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x15U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x16U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x17U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x18U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x19U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1AU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1BU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1CU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1DU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1EU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG0(0x1FU),      0u, IPFMA_WIDTH_32 },

            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x0U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x2U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x3U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x4U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x5U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x6U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x7U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x8U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x9U),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xAU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xBU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xCU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xDU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xEU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0xFU),       0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x10U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x11U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x12U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x13U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x14U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x15U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x16U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x17U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x18U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x19U),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1AU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1BU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1CU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1DU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1EU),      0u, IPFMA_WIDTH_32 },
            { CSL_CSI_RX_IF_CNTX_CNTL_PSI_CFG1(0x1FU),      0u, IPFMA_WIDTH_32 }
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            out->csirx_id   = registerDescriptorsArr[0].value;
            out->vp0        = registerDescriptorsArr[1].value;
            out->vp1        = registerDescriptorsArr[2].value;
            out->cntl       = registerDescriptorsArr[3].value;

            for (uint32_t i = 0; i < CNTX_CNTL_SIZE; i++)
            {
                out->dmacntx[i]  = registerDescriptorsArr[i+4].value;
                out->psi_cfg0[i] = registerDescriptorsArr[i+CNTX_CNTL_SIZE+4].value;
                out->psi_cfg1[i] = registerDescriptorsArr[i+2*CNTX_CNTL_SIZE+4].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Csirx_CompareShimRegs(const IpFma_CsirxShimRegs* expected,
                                         const IpFma_CsirxShimRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if (((NULL_PTR == expected)) || ((NULL_PTR == actual)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if ((expected->csirx_id != actual->csirx_id))
        {
            status = IPFMA_E_MISMATCH;
        }

        if ((IPFMA_OK == status) && (expected->vp0 != actual->vp0))
        {
            status = IPFMA_E_MISMATCH;
        }

        if ((IPFMA_OK == status) && (expected->vp1 != actual->vp1))
        {
            status = IPFMA_E_MISMATCH;
        }

        if ((IPFMA_OK == status) && (expected->cntl != actual->cntl))
        {
            status = IPFMA_E_MISMATCH;
        }

        if (IPFMA_OK == status)
        {
            for (uint32_t i = 0; i < CNTX_CNTL_SIZE; i++)
            {
                if ((expected->dmacntx[i]  != actual->dmacntx[i])  || 
                    (expected->psi_cfg0[i] != actual->psi_cfg0[i]) || 
                    (expected->psi_cfg1[i] != actual->psi_cfg1[i]) )
                {
                    status = IPFMA_E_MISMATCH;
                    break;
                }
            }
        }

    }

    return status;
}

/** @} */
