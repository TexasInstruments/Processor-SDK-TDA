/*
 *  Copyright (c) Texas Instruments Incorporated 2020 - 2025
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

/*!
 * \file  dp83822.c
 *
 * \brief This file contains the implementation of the DP83822 PHY.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>

#include "../include/dp83822.h"
#include "dp83822_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DP83822_OUI                           (0x080028U)
#define DP83822_MODEL                         (0x24U)
#define DP83822_REV                           (0x00U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

static void Dp83822_enableAutoMdix(EthPhyDrv_Handle hPhy,
                                   bool enable);

static int32_t Dp83822_getSpeedDuplex (EthPhyDrv_Handle hPhy,
                                       Phy_Link_SpeedDuplex* pConfig);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Phy_DrvObj_t gEnetPhyDrvDp83822 =
{
    .fxn =
    {
        .name               = "DP83822",
        .bind               = Dp83822_bind,
        .isPhyDevSupported  = Dp83822_isPhyDevSupported,
        .isMacModeSupported = Dp83822_isMacModeSupported,
        .config             = Dp83822_config,
        .reset              = GenericPhy_reset,
        .isResetComplete    = GenericPhy_isResetComplete,
        .readExtReg         = GenericPhy_readExtReg,
        .writeExtReg        = GenericPhy_writeExtReg,
        .printRegs          = Dp83822_printRegs,
        .adjPtpFreq              = NULL,
        .adjPtpPhase             = NULL,
        .getPtpTime              = NULL,
        .setPtpTime              = NULL,
        .getPtpTxTime            = NULL,
        .getPtpRxTime            = NULL,
        .waitPtpTxTime           = NULL,
        .procStatusFrame         = NULL,
        .getStatusFrameEthHeader = NULL,
        .enablePtp               = NULL,
        .tickDriver              = NULL,
        .enableEventCapture      = NULL,
        .enableTriggerOutput     = NULL,
        .getEventTs              = NULL,
        .configMediaClock        = NULL,
        .nudgeCodecClock         = NULL,
        .getSpeedDuplex          = Dp83822_getSpeedDuplex,

    }
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Dp83822_initCfg(Dp83822_Cfg *cfg)
{
    /* No extended config parameters at the moment */
}

void Dp83822_bind(EthPhyDrv_Handle* hPhy,
                    uint8_t phyAddr,
                    Phy_RegAccessCb_t* pRegAccessCb)
{
    Phy_Obj_t* pObj = (Phy_Obj_t*) hPhy;
    pObj->phyAddr = phyAddr;
    pObj->regAccessApi = *pRegAccessCb;
}

bool Dp83822_isPhyDevSupported(EthPhyDrv_Handle hPhy,
                                const void *pVersion)
{
    const Phy_Version *version = (Phy_Version *)pVersion;

    bool supported = false;

    if ((version->oui == DP83822_OUI) &&
        (version->model == DP83822_MODEL) &&
        (version->revision == DP83822_REV))
    {
        supported = true;
    }

    return supported;
}

bool Dp83822_isMacModeSupported(EthPhyDrv_Handle hPhy,
                                Phy_Mii mii)
{
    bool supported;

    switch (mii)
    {
        case PHY_MAC_MII_RMII:
            supported = true;
            break;

        /* This driver doesn't support MII and RGMII interfaces,
         * but the DP83822 PHY does support them */
        case PHY_MAC_MII_MII:
        case PHY_MAC_MII_RGMII:
        default:
            supported = false;
            break;
    }

    return supported;
}

int32_t Dp83822_config(EthPhyDrv_Handle hPhy,
                        const void *pExtCfg,
                        const uint32_t extCfgSize,
                        Phy_Mii mii,
                        bool loopbackEn)
{
    uint8_t phyAddr = PhyPriv_getPhyAddr(hPhy);

    const Dp83822_Cfg *extendedCfg = (const Dp83822_Cfg *)pExtCfg;
    uint32_t extendedCfgSize = extCfgSize;
    bool enableAutoMdix;
    int32_t status = PHY_SOK;

    if ((extendedCfg == NULL) ||
        (extendedCfgSize != sizeof(*extendedCfg)))
    {
        PHYTRACE_ERR("PHY %u: invalid config params (cfg=%p, size=%u)\r\n",
                     phyAddr, extendedCfg, extendedCfgSize);
        status = PHY_EINVALIDPARAMS;
    }

    /* Auto-MDIX should be disabled in near-end loopback modes */
    enableAutoMdix = !loopbackEn;

    /* Enable Auto-MDIX and Robust Auto-MDIX */
    if (status == PHY_SOK)
    {
        Dp83822_enableAutoMdix(hPhy, enableAutoMdix);
    }

    return status;
}

static void Dp83822_enableAutoMdix(EthPhyDrv_Handle hPhy,
                                   bool enable)
{
    Phy_RegAccessCb_t* pRegAccessApi = PhyPriv_getRegAccessApi(hPhy);

    PHYTRACE_DBG("PHY %u: %s automatic cross-over\n",
                 PhyPriv_getPhyAddr(hPhy), enable ? "enable" : "disable");
                 pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, DP83822_PHYCR,
                 PHYCR_AUTOMDIX_ENABLE,
                 enable ? PHYCR_AUTOMDIX_ENABLE : 0);

    if (enable)
    {
        PHYTRACE_DBG("PHY %u: enable Robust Auto-MDIX\n",PhyPriv_getPhyAddr(hPhy));
        pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, DP83822_CR1,
                       CR1_ROBUSTAUTOMDIX,
                       CR1_ROBUSTAUTOMDIX);
    }
    else
    {
        pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, DP83822_PHYCR,
                       PHYCR_FORCEMDIX_MASK,
                       PHYCR_FORCEMDIX_MDI);
    }
}

void Dp83822_printRegs(EthPhyDrv_Handle hPhy)
{
    uint16_t val;

    const uint8_t phyAddr = PhyPriv_getPhyAddr(hPhy);
    Phy_RegAccessCb_t* pRegAccessApi = PhyPriv_getRegAccessApi(hPhy);

    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);
    printf("PHY %u: BMCR    = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);
    printf("PHY %u: BMSR    = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR1, &val);
    printf("PHY %u: PHYIDR1 = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR2, &val);
    printf("PHY %u: PHYIDR2 = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANAR, &val);
    printf("PHY %u: ANAR    = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANLPAR, &val);
    printf("PHY %u: ANLPAR  = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANER, &val);
    printf("PHY %u: ANER    = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANNPTR, &val);
    printf("PHY %u: ANNPTR  = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANNPRR, &val);
    printf("PHY %u: ANNPRR  = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, DP83822_CR1, &val);
    printf("PHY %u: CR1     = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_GIGSR, &val);
    printf("PHY %u: STS1    = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_GIGESR, &val);
    printf("PHY %u: 1KSCR   = 0x%04x\r\n",phyAddr, val);
    pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, DP83822_PHYCR, &val);
    printf("PHY %u: PHYCR   = 0x%04x\r\n",phyAddr, val);
}

int32_t Dp83822_getSpeedDuplex (EthPhyDrv_Handle hPhy, Phy_Link_SpeedDuplex* pConfig)
{
    int32_t  status;
    uint32_t speed;
    uint16_t tmp;
    uint16_t val;

    Phy_RegAccessCb_t* pRegAccessApi = PhyPriv_getRegAccessApi(hPhy);

    /* Restart is complete when RESET bit has self-cleared */
    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, DP83822_PHYSTS, &val);
    if (status == PHY_SOK)
    {
        if (val & DP83822_PHYSTS_LINK)
        {
            tmp = (val & PHYST_SPEEDSEL_MASK);

            switch(tmp)
            {
                case PHYST_SPEEDSEL_10_MBPS:
                    speed = 10;

                    *pConfig = PHY_LINK_HD10;
                    if (val & PHYST_DUPLEXMODEENV_FD)
                    {
                        *pConfig = PHY_LINK_FD10;
                    }
                    break;
                case PHYST_SPEEDSEL_100_MBPS:
                    speed = 100;

                    *pConfig = PHY_LINK_HD100;
                    if (val & PHYST_DUPLEXMODEENV_FD)
                    {
                        *pConfig = PHY_LINK_FD100;
                    }
                    break;
                default:
                    speed = 0;

                    *pConfig = PHY_LINK_INVALID;
                    break;
            }
        }
        else 
        {
            *pConfig = PHY_LINK_INVALID;
        }
    }

    PHYTRACE_DBG("PHY %u: selected speed is %d Mbps with %s-duplex\n", PhyPriv_getPhyAddr(hPhy), speed, (val & PHYST_DUPLEXMODEENV_FD) ? "full" : "half");

    (void)speed;

    return status;
}