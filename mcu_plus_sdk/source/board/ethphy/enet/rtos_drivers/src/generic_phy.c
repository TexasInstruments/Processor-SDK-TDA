/*
 *  Copyright (c) Texas Instruments Incorporated 2020-2025
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
 * \file  generic_phy.c
 *
 * \brief This file contains the implementation of the generic Ethernet PHY.
 *        It provides the basic functionality allowed with IEEE standard
 *        registers.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include "generic_phy.h"
#include "phy_common_priv.h"
#include "port.h"
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

void GenericPhy_bind(EthPhyDrv_Handle* hPhy, uint8_t phyAddr, Phy_RegAccessCb_t* pRegAccessCb);

static bool GenericPhy_isPhyDevSupported(EthPhyDrv_Handle hPhy,
                                            const void *pVersion);

static bool GenericPhy_isMacModeSupported(EthPhyDrv_Handle hPhy, Phy_Mii mii);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Phy_DrvObj_t gEnetPhyDrvGeneric =
{
     .fxn =
     {
        .name                             = "generic",
        .bind                             = GenericPhy_bind,
        .isPhyDevSupported                = GenericPhy_isPhyDevSupported,
        .isMacModeSupported               = GenericPhy_isMacModeSupported,
        .config                           = NULL,
        .reset                            = GenericPhy_reset,
        .isResetComplete                  = GenericPhy_isResetComplete,
        .readReg                          = GenericPhy_readReg,
        .writeReg                         = GenericPhy_writeReg,
        .readExtReg                       = GenericPhy_readExtReg,
        .writeExtReg                      = GenericPhy_writeExtReg,
        .printRegs                        = GenericPhy_printRegs,
        .getId                            = GenericPhy_getId,
        .isPowerDownActive                = GenericPhy_isPowerDownActive,
        .ctrlPowerDown                    = GenericPhy_ctrlPowerDown,
        .enableAdvertisement              = GenericPhy_enableAdvertisement,
        .disableAdvertisement             = GenericPhy_disableAdvertisement,
        .ctrlAutoNegotiation              = GenericPhy_ctrlAutoNegotiation,
        .isLinkPartnerAutoNegotiationAble = GenericPhy_isLinkPartnerAutoNegotiationAble,
        .isAutoNegotiationEnabled         = GenericPhy_isAutoNegotiationEnabled,
        .isAutoNegotiationComplete        = GenericPhy_isAutoNegotiationComplete,
        .isAutoNegotiationRestartComplete = GenericPhy_isAutoNegotiationRestartComplete,
        .isLinkUp                         = GenericPhy_isLinkUp,
        .setSpeedDuplex                   = GenericPhy_setSpeedDuplex,
        .getSpeedDuplex                   = NULL,
        .setMiiMode                       = NULL,
        .ctrlExtFD                        = NULL,
        .ctrlOddNibbleDetection           = NULL,
        .ctrlRxErrIdle                    = NULL,
     },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void GenericPhy_bind(EthPhyDrv_Handle* hPhy, uint8_t phyAddr, Phy_RegAccessCb_t* pRegAccessCb)
{
    Phy_Obj_t* pObj = (Phy_Obj_t*) hPhy;
    pObj->phyAddr = phyAddr;
    pObj->regAccessApi = *pRegAccessCb;
}

static bool GenericPhy_isPhyDevSupported(EthPhyDrv_Handle hPhy,
                                            const void *pVersion)
{
    /* All IEEE-standard PHY models are supported */
    return true;
}

static bool GenericPhy_isMacModeSupported(EthPhyDrv_Handle hPhy, Phy_Mii mii)
{
    /* All MAC modes are supported */
    return true;
}

void GenericPhy_reset(EthPhyDrv_Handle hPhy)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    PHYTRACE_DBG("PHY %u: reset\n", ((Phy_Obj_t*) hPhy)->phyAddr);

    /* Reset the PHY */
    pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, BMCR_RESET, BMCR_RESET);
}

bool GenericPhy_isResetComplete(EthPhyDrv_Handle hPhy)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    int32_t status;
    uint16_t val;
    bool complete = false;

    /* Reset is complete when RESET bit has self-cleared */
    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);
    if (status == PHY_SOK)
    {
        complete = ((val & BMCR_RESET) == 0U);
    }

    PHYTRACE_DBG("PHY %u: reset is %scomplete\n", ((Phy_Obj_t*) hPhy)->phyAddr, complete ? "" : "not");

    return complete;
}

int32_t GenericPhy_readReg(EthPhyDrv_Handle hPhy,
                           uint32_t reg,
                           uint16_t *pVal)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    int32_t status = PHY_EFAIL;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, reg, pVal);

    PHYTRACE_VERBOSE_IF(status == PHY_SOK,
                        "PHY %u: failed to read reg %u\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg);
    PHYTRACE_ERR_IF(status != PHY_SOK,
                    "PHY %u: read reg %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, *pVal);

    return status;
}

int32_t GenericPhy_writeReg(EthPhyDrv_Handle hPhy,
                            uint32_t reg,
                            uint16_t val)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    int32_t status = PHY_EFAIL;

    status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, reg, val);

    PHYTRACE_ERR_IF(status != PHY_SOK,
                    "PHY %u: failed to write reg %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, val);

    return status;
}

int32_t GenericPhy_readExtReg(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t* val)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    uint16_t devad = MMD_CR_DEVADDR;
    int32_t status;

    status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_ADDR);

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, reg);
    }

    if (status == PHY_SOK)
    {
        pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
    }

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_MMD_DR, val);
    }

    PHYTRACE_VERBOSE_IF(status == PHY_SOK,
                         "PHY %u: failed to read reg %u\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg);
    PHYTRACE_ERR_IF(status != PHY_SOK,
                     "PHY %u: read reg %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, *val);

    return status;
}

int32_t GenericPhy_writeExtReg(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t val)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
	uint16_t devad = MMD_CR_DEVADDR;
    int32_t status;

    PHYTRACE_VERBOSE("PHY %u: write %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr;, reg, val);

    status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_ADDR);
    if (status == PHY_SOK)
    {
        pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, reg);
    }

    if (status == PHY_SOK)
    {
        pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
    }

    if (status == PHY_SOK)
    {
        pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, val);
    }

    PHYTRACE_ERR_IF(status != PHY_SOK,
                     "PHY %u: failed to write reg %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, val);

    return status;
}

void GenericPhy_printRegs(EthPhyDrv_Handle hPhy)
{
    uint32_t i;
    uint16_t val;
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    const uint8_t phyAddr = ((Phy_Obj_t*) hPhy)->phyAddr;

    for (i = PHY_BMCR; i <= PHY_GIGESR; i++)
    {
        pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, i, &val);
        printf("PHY %u: reg 0x%02x = 0x%04x\n", phyAddr, i, val);
    }
}

int32_t GenericPhy_getId (EthPhyDrv_Handle hPhy, 
                          uint32_t* pId)
{
    uint16_t val = 0;
    uint32_t res = 0;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR1, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    res = ((uint32_t) val) << 16;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR2, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    res |= (uint32_t) val;

    *pId = res;

laError:

    return status;
}

int32_t GenericPhy_isPowerDownActive (EthPhyDrv_Handle hPhy,
                                      bool *pActive)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (BMCR_PWRDOWN == (val & BMCR_PWRDOWN))
    {
        res = true;
    }

    *pActive = res;

laError:

    return status;
}

int32_t GenericPhy_ctrlPowerDown (EthPhyDrv_Handle hPhy,
                                  bool control)
{
    uint16_t val = 0;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if (control)
    {
        //Power Down mode
        val = BMCR_ISOLATE | BMCR_PWRDOWN;
    }

    return pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, BMCR_ISOLATE | BMCR_PWRDOWN, val);
}

int32_t GenericPhy_enableAdvertisement (EthPhyDrv_Handle hPhy,
                                        uint32_t advertisement)
{
    uint16_t val = 0;
    uint32_t tmpMask = 0;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if (0 == advertisement)
    {
        status = PHY_SOK;
        goto laError;
    }

    tmpMask = PHY_LINK_ADV_HD10 | PHY_LINK_ADV_FD10 | PHY_LINK_ADV_HD100 | PHY_LINK_ADV_FD100;

    if (0 != (advertisement & tmpMask))
    {
        val |= ANAR_802P3;

        if (0 != (advertisement & PHY_LINK_ADV_HD10))
        {
            val |= ANAR_10HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD10))
        {
            val |= ANAR_10FD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_HD100))
        {
            val |= ANAR_100HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD100))
        {
            val |= ANAR_100FD;
        }

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_ANAR, ANAR_802P3 | ANAR_10HD | ANAR_10FD | ANAR_100HD | ANAR_100FD, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

    val = 0;
    tmpMask = PHY_LINK_ADV_HD1000 | PHY_LINK_ADV_FD1000;

    if (0 != (advertisement & tmpMask))
    {
        if (0 != (advertisement & PHY_LINK_ADV_HD1000))
        {
            val |= GIGSR_1000HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD1000))
        {
            val |= GIGSR_1000FD;
        }

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_GIGCR, GIGSR_1000FD | GIGSR_1000HD, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

laError:

    return status;
}

int32_t GenericPhy_disableAdvertisement (EthPhyDrv_Handle hPhy,
                                         uint32_t advertisement)
{
    uint16_t val = 0;
    uint32_t tmpMask = 0;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if (0 == advertisement)
    {
        status = PHY_SOK;
        goto laError;
    }

    tmpMask = PHY_LINK_ADV_HD10 | PHY_LINK_ADV_FD10 | PHY_LINK_ADV_HD100 | PHY_LINK_ADV_FD100;

    if (0 != (advertisement & tmpMask))
    {
        val = ANAR_802P3 | PHY_LINK_ADV_HD10 | PHY_LINK_ADV_FD10 | PHY_LINK_ADV_HD100 | PHY_LINK_ADV_FD100;

        if (0 != (advertisement & PHY_LINK_ADV_HD10))
        {
            val &= ~ANAR_10HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD10))
        {
            val &= ~ANAR_10FD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_HD100))
        {
            val &= ~ANAR_100HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD100))
        {
            val &= ~ANAR_100FD;
        }

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_ANAR, ANAR_802P3 | ANAR_10HD | ANAR_10FD | ANAR_100HD | ANAR_100FD, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

    tmpMask = PHY_LINK_ADV_HD1000 | PHY_LINK_ADV_FD1000;

    if (0 != (advertisement & tmpMask))
    {
        val = GIGSR_1000HD | GIGSR_1000FD;

        if (0 != (advertisement & PHY_LINK_ADV_HD1000))
        {
            val &= ~GIGSR_1000HD;
        }

        if (0 != (advertisement & PHY_LINK_ADV_FD1000))
        {
            val &= ~GIGSR_1000FD;
        }

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_GIGCR, GIGSR_1000FD | GIGSR_1000HD, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

laError:

    return status;
}

int32_t GenericPhy_ctrlAutoNegotiation(EthPhyDrv_Handle hPhy,
                                       uint32_t control)
{
    uint16_t val  = 0;
    uint16_t mask = 0;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    switch(control)
    {
        case PHY_AUTO_NEGOTIATION_CTRL_DISABLE:
            /* disable Auto Negotiation */
            mask = BMCR_ANEN;
            val  = (uint16_t) ~BMCR_ANEN;
            break;
        case PHY_AUTO_NEGOTIATION_CTRL_ENABLE:
            /* enable Auto Negotiation */
            mask = BMCR_ANEN;
            val  = BMCR_ANEN;
            break;
        case PHY_AUTO_NEGOTIATION_CTRL_RESTART:
            /* restart Auto Negotiation */
            mask = BMCR_ANRESTART;
            val  = BMCR_ANRESTART;
            break;
        case PHY_AUTO_NEGOTIATION_CTRL_ENABLE_AND_RESTART:
            /* enable and restart Auto Negotiation */
            mask = BMCR_ANEN | BMCR_ANRESTART;
            val  = BMCR_ANEN | BMCR_ANRESTART;
            break;
        default:
            /* unknown control command */
            break;
    }

    return pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, mask, val);
}

int32_t GenericPhy_isLinkPartnerAutoNegotiationAble (EthPhyDrv_Handle hPhy,
                                                     bool *pAble)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANER, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (ANER_LPISANABLE == (val & ANER_LPISANABLE))
    {
        res = true;
    }

    *pAble = res;

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationEnabled(EthPhyDrv_Handle hPhy,
                                            bool *pEnabled)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (BMCR_ANEN == (val & BMCR_ANEN))
    {
        res = true;
    }

    *pEnabled = res;

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationComplete (EthPhyDrv_Handle hPhy,
                                              bool *pCompleted)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (BMSR_ANCOMPLETE == (val & BMSR_ANCOMPLETE))
    {
        res = true;
    }

    *pCompleted = res;

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationRestartComplete (EthPhyDrv_Handle hPhy,
                                                     bool *pCompleted)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (0 == (val & BMCR_ANRESTART))
    {
        res = true;
    }

    *pCompleted = res;

laError:

    return status;
}

int32_t GenericPhy_setSpeedDuplex (EthPhyDrv_Handle hPhy,
                                   uint32_t settings)
{
    uint16_t val  = 0;
    uint16_t mask = 0;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    mask = BMCR_SPEED100 | BMCR_SPEED1000 | BMCR_FD;

    switch(settings)
    {
        case PHY_LINK_HD10:
            /* Select 10Mbps, Half-Duplex */
            val  = 0;
            break;
        case PHY_LINK_FD10:
            /* Select 10Mbps, Full-Duplex */
            val  = BMCR_FD;
            break;
        case PHY_LINK_HD100:
            /* Select 100Mbps, Half-Duplex */
            val  = BMCR_SPEED100;
            break;
        case PHY_LINK_FD100:
            /* Select 100Mbps, Full-Duplex */
            val  = BMCR_SPEED100 | BMCR_FD;
            break;
        case PHY_LINK_HD1000:
            /* Select 1000Mbps, Half-Duplex */
            val  = BMCR_SPEED1000;
            break;
        case PHY_LINK_FD1000:
            /* Select 1000Mbps, Full-Duplex */
            val  = BMCR_SPEED1000 | BMCR_FD;
            break;
        default:
            /* unknown control command */
            break;
    }

    return pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, mask, val);
}

int32_t GenericPhy_isLinkUp (EthPhyDrv_Handle hPhy,
                             bool *pLinkUp)
{
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if (0 != (val & BMSR_LINKSTS))
    {
        /* read second time due to latch low */
        status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

        if (PHY_SOK != status)
        {
            goto laError;
        }

        if (0 != (val & BMSR_LINKSTS))
        {
            res = true;
        }
    }

    *pLinkUp = res;

laError:

    return status;
}
