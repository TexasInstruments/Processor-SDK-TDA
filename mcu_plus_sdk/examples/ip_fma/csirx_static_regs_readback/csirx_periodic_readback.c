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
 *  \file     csirx_periodic_readback.c
 *
 *  \brief    This example demonstrates periodic software readback of static
 *            CSIRX D-PHY and SHIM configuration registers and reports match
 *            or mismatch between expected and actual register values for
 *            validation and diagnostic purposes.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <drivers/hw_include/j722s/cslr_soc_baseaddress.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <ip_fma_csirx.h>
#include <drivers/sciclient/include/tisci/j722s/tisci_devices.h>
#include <drivers/soc/j722s/soc.h>

/* ========================================================================== */
/*                                Macros                                      */
/* ========================================================================== */

#define APP_CSIRX_STR           "CSIRX IP FMA Example"
/**< Number of periodic readback iterations */
#define PERIODIC_CHECK_NUM      ((uint8_t)10U)
/**< Spin-loop count used between readback iterations */
#define DELAY_LOOPS             ((uint32_t)1000U)

/**< CSIRX0 D-PHY wrapper base address */
#define APP_CSIRX0_DPHY_BASE    (CSL_DPHY_RX0_VBUS2APB_WRAP_VBUSP_K3_DPHY_RX_BASE)
/**< CSIRX0 RX SHIM base address */
#define APP_CSIRX0_SHIM_BASE    (CSL_CSI_RX_IF0_RX_SHIM_VBUSP_MMR_CSI2RXIF_BASE)
/**< CSIRX1 D-PHY wrapper base address */
#define APP_CSIRX1_DPHY_BASE    (CSL_DPHY_RX1_VBUS2APB_WRAP_VBUSP_K3_DPHY_RX_BASE)
/**< CSIRX1 RX SHIM base address */
#define APP_CSIRX1_SHIM_BASE    (CSL_CSI_RX_IF1_RX_SHIM_VBUSP_MMR_CSI2RXIF_BASE)
/**< CSIRX2 D-PHY wrapper base address */
#define APP_CSIRX2_DPHY_BASE    (CSL_DPHY_RX2_VBUS2APB_WRAP_VBUSP_K3_DPHY_RX_BASE)
/**< CSIRX2 RX SHIM base address */
#define APP_CSIRX2_SHIM_BASE    (CSL_CSI_RX_IF2_RX_SHIM_VBUSP_MMR_CSI2RXIF_BASE)
/**< CSIRX3 D-PHY wrapper base address */
#define APP_CSIRX3_DPHY_BASE    (CSL_DPHY_RX3_VBUS2APB_WRAP_VBUSP_K3_DPHY_RX_BASE)
/**< CSIRX3 RX SHIM base address */
#define APP_CSIRX3_SHIM_BASE    (CSL_CSI_RX_IF3_RX_SHIM_VBUSP_MMR_CSI2RXIF_BASE)
/**< CSIRX domain count */
#define CSI_RX_DOMAIN_CNT       (CSL_CSI_RX_IF_MAIN_CNT)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static const uint32_t TISCI_DEV_CSI_RX_DOMAIN_INDEX[CSI_RX_DOMAIN_CNT] = {
    TISCI_DEV_CSI_RX_IF0,
    TISCI_DEV_CSI_RX_IF1,
    TISCI_DEV_CSI_RX_IF2,
    TISCI_DEV_CSI_RX_IF3
};

static const uint32_t TISCI_DEV_DPHY_RX_DOMAIN_INDEX[CSI_RX_DOMAIN_CNT] = {
    TISCI_DEV_DPHY_RX0,
    TISCI_DEV_DPHY_RX1,
    TISCI_DEV_DPHY_RX2,
    TISCI_DEV_DPHY_RX3
};

static const uint32_t CSI_RX_DPHY_DOMAINS_BASE[CSI_RX_DOMAIN_CNT] = {
    APP_CSIRX0_DPHY_BASE,
    APP_CSIRX1_DPHY_BASE,
    APP_CSIRX2_DPHY_BASE,
    APP_CSIRX3_DPHY_BASE
};

static const uint32_t CSI_RX_SHIM_DOMAINS_BASE[CSI_RX_DOMAIN_CNT] = {
    APP_CSIRX0_SHIM_BASE,
    APP_CSIRX1_SHIM_BASE,
    APP_CSIRX2_SHIM_BASE,
    APP_CSIRX3_SHIM_BASE
};

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

static void CsirxApp_moduleClockConfig(uint8_t config_mode);
static void CsirxApp_Delay(uint32_t loops);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void CsirxApp_moduleClockEnable(void)
{
    CsirxApp_moduleClockConfig(1U);
}

void CsirxApp_moduleClockDisable(void)
{
    CsirxApp_moduleClockConfig(0U);
}

void CsirxApp_PeriodicReadback(void)
{
    IpFma_CsirxDphyRegs dphyExpected, dphyActual;
    IpFma_CsirxShimRegs shimExpected, shimActual;
    IpFma_Status dphyStatus = IPFMA_OK;
    IpFma_Status shimStatus = IPFMA_OK;
    IpFma_Status dphyOverallStatus;
    IpFma_Status shimOverallStatus;
    IpFma_Status dphyDomainStatus;
    IpFma_Status shimDomainStatus;
    uint8_t csi_rx_domain;
    uint8_t i;

    DebugP_log("\r\n" APP_CSIRX_STR ": Start\r\n");

    /* Loop over each CSI RX domain instance */
    for (csi_rx_domain = 0U; csi_rx_domain < CSI_RX_DOMAIN_CNT; csi_rx_domain++)
    {
        /* Capture expected (baseline) register values */
        DebugP_log(APP_CSIRX_STR ": Loading expected register values...\r\n");

        dphyStatus = IpFma_Csirx_GetDphyRegs(CSI_RX_DPHY_DOMAINS_BASE[csi_rx_domain], &dphyExpected);

        if (IPFMA_OK != dphyStatus)
        {
            DebugP_log(APP_CSIRX_STR ": Error reading initial DPHY registers\r\n");
            return;
        }

        shimStatus = IpFma_Csirx_GetShimRegs(CSI_RX_SHIM_DOMAINS_BASE[csi_rx_domain], &shimExpected);
        if (IPFMA_OK != shimStatus)
        {
            DebugP_log(APP_CSIRX_STR ": Error reading initial SHIM registers\r\n");
            return;
        }

        DebugP_log(APP_CSIRX_STR ": Register check starts...\r\n");

        dphyOverallStatus = IPFMA_OK;
        shimOverallStatus = IPFMA_OK;

        /* Periodic readback loop */
        for (i = 0U; i < PERIODIC_CHECK_NUM; i++)
        {
            dphyDomainStatus = IPFMA_OK;
            shimDomainStatus = IPFMA_OK;

            CsirxApp_Delay(DELAY_LOOPS);

            dphyStatus = IpFma_Csirx_GetDphyRegs(CSI_RX_DPHY_DOMAINS_BASE[csi_rx_domain], &dphyActual);
            if (IPFMA_OK == dphyStatus)
            {
                dphyStatus = IpFma_Csirx_CompareDphyRegs(&dphyExpected, &dphyActual);
                DebugP_log(APP_CSIRX_STR ": Comparing DPHY registers... values %s\r\n",
                        (IPFMA_OK == dphyStatus) ? "MATCH!" : "MISMATCH!");
            }
            else
            {
                DebugP_log(APP_CSIRX_STR ": Error reading DPHY registers\r\n");
            }

            if (IPFMA_OK != dphyStatus)
            {
                dphyDomainStatus = dphyStatus;
            }

            shimStatus = IpFma_Csirx_GetShimRegs(CSI_RX_SHIM_DOMAINS_BASE[csi_rx_domain], &shimActual);
            if (IPFMA_OK == shimStatus)
            {
                shimStatus = IpFma_Csirx_CompareShimRegs(&shimExpected, &shimActual);
                DebugP_log(APP_CSIRX_STR ": Comparing SHIM registers... values %s\r\n",
                        (IPFMA_OK == shimStatus) ? "MATCH!" : "MISMATCH!");
            }
            else
            {
                DebugP_log(APP_CSIRX_STR ": Error reading SHIM registers\r\n");
            }

            if (IPFMA_OK != shimStatus)
            {
                shimDomainStatus = shimStatus;
            }
        }

        if ((IPFMA_OK == dphyDomainStatus) && (IPFMA_OK == shimDomainStatus))
        {
            DebugP_log(APP_CSIRX_STR ": CSIRX%d register check passed\r\n", csi_rx_domain);
        }
        else
        {
            dphyOverallStatus = dphyDomainStatus;
            shimOverallStatus = shimDomainStatus;
            DebugP_log(APP_CSIRX_STR ": CSIRX%d register check failed!!\r\n", csi_rx_domain);
        }
    }
    
    if ((IPFMA_OK == dphyOverallStatus) && (IPFMA_OK == shimOverallStatus))
    {
        DebugP_log(APP_CSIRX_STR ": All tests have passed. Completes!!!\r\n");
    }
    else
    {
        DebugP_log(APP_CSIRX_STR ": Tests failed !!\r\n");
    }
}

/* ========================================================================== */
/*                 Internal Function Definitions                              */
/* ========================================================================== */

static void CsirxApp_moduleClockConfig(uint8_t config_mode)
{
    int32_t status = SystemP_SUCCESS;
    uint8_t csi_rx_domain;

    for (csi_rx_domain = 0; csi_rx_domain < CSI_RX_DOMAIN_CNT; csi_rx_domain++)
    {
        status = SOC_moduleClockEnable(TISCI_DEV_CSI_RX_DOMAIN_INDEX[csi_rx_domain], config_mode);
        DebugP_assert(status == SystemP_SUCCESS);

        status = SOC_moduleClockEnable(TISCI_DEV_DPHY_RX_DOMAIN_INDEX[csi_rx_domain], config_mode);
        DebugP_assert(status == SystemP_SUCCESS);
    }
}

static void CsirxApp_Delay(uint32_t loops)
{
    while (loops--)
    {
        asm("   NOP");
    }
}

/********************************* End of file ******************************/
