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
        .getLocalCaps                     = GenericPhy_getLocalCaps,
        .setAdvertisement                 = GenericPhy_setAdvertisement,
        .enableAdvertisement              = GenericPhy_enableAdvertisement,
        .disableAdvertisement             = GenericPhy_disableAdvertisement,
        .ctrlAutoNegotiation              = GenericPhy_ctrlAutoNegotiation,
        .isLinkPartnerAutoNegotiationAble = GenericPhy_isLinkPartnerAutoNegotiationAble,
        .isAutoNegotiationEnabled         = GenericPhy_isAutoNegotiationEnabled,
        .isAutoNegotiationComplete        = GenericPhy_isAutoNegotiationComplete,
        .isAutoNegotiationRestartComplete = GenericPhy_isAutoNegotiationRestartComplete,
        .isGigabitSupported               = GenericPhy_isGigabitSupported,
        .isLinkUp                         = GenericPhy_isLinkUp,
        .setSpeedDuplex                   = GenericPhy_setSpeedDuplex,
        .getSpeedDuplex                   = NULL,
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

int32_t GenericPhy_reset(EthPhyDrv_Handle hPhy)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    int32_t status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    PHYTRACE_DBG("PHY %u: reset\n", ((Phy_Obj_t*) hPhy)->phyAddr);

    /* Reset the PHY */
    status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, BMCR_RESET, BMCR_RESET);

laError:

    return status;
}

int32_t GenericPhy_isResetComplete(EthPhyDrv_Handle hPhy, bool *pCompleted)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    int32_t status = PHY_EFAIL;
    uint16_t val = 0;

    if ((NULL == hPhy) ||
        (NULL == pCompleted))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    *pCompleted = false;

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    /* Reset is complete when RESET bit has self-cleared */
    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (status == PHY_SOK)
    {
        *pCompleted = ((val & BMCR_RESET) == 0U);
    }

    PHYTRACE_DBG("PHY %u: reset is %scomplete\n", ((Phy_Obj_t*) hPhy)->phyAddr, *pCompleted ? "" : "not");

laError:

    return status;
}

int32_t GenericPhy_readReg(EthPhyDrv_Handle hPhy,
                           uint32_t reg,
                           uint16_t *pVal)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    int32_t status = PHY_EFAIL;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, reg, pVal);

    PHYTRACE_VERBOSE_IF(status != PHY_SOK,
                        "PHY %u: failed to read reg %u\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg);
    PHYTRACE_ERR_IF(status == PHY_SOK,
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
    int32_t status = PHY_EFAIL;

    status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_ADDR);

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, reg);
    }

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
    }

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_MMD_DR, val);
    }

    PHYTRACE_VERBOSE_IF(status != PHY_SOK,
                         "PHY %u: failed to read reg %u\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg);
    PHYTRACE_ERR_IF(status == PHY_SOK,
                     "PHY %u: read reg %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, *val);

    return status;
}

int32_t GenericPhy_writeExtReg(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t val)
{
    Phy_RegAccessCb_t* pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;
    uint16_t devad = MMD_CR_DEVADDR;
    int32_t status = PHY_EFAIL;

    PHYTRACE_VERBOSE("PHY %u: write %u val 0x%04x\n", ((Phy_Obj_t*) hPhy)->phyAddr, reg, val);

    status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_ADDR);
    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, reg);
    }

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
    }

    if (status == PHY_SOK)
    {
        status = pRegAccessApi->EnetPhy_writeReg(pRegAccessApi->pArgs, PHY_MMD_DR, val);
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
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pId))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR1, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pId = ((uint32_t) val) << 16;

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_PHYIDR2, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pId |= (uint32_t) val;

laError:

    return status;
}

int32_t GenericPhy_isPowerDownActive (EthPhyDrv_Handle hPhy,
                                      bool *pActive)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pActive))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pActive = false;

    if (BMCR_PWRDOWN == (val & BMCR_PWRDOWN))
    {
        *pActive = true;
    }

laError:

    return status;
}

int32_t GenericPhy_ctrlPowerDown (EthPhyDrv_Handle hPhy,
                                  bool enable)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if (enable)
    {
        //Power Down mode
        val = BMCR_ISOLATE | BMCR_PWRDOWN;
    }

    status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, BMCR_ISOLATE | BMCR_PWRDOWN, val);

laError:

    return status;
}

int32_t GenericPhy_getLocalCaps (EthPhyDrv_Handle hPhy,
                                 uint32_t *pCapabilities)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val  = 0U;
    int32_t status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    /* Get 10/100 Mbps capabilities */
    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    if ((val & BMSR_100FD) != 0U)
    {
        *pCapabilities |= PHY_LINK_CAP_FD100;
    }

    if ((val & BMSR_100HD) != 0U)
    {
        *pCapabilities |= PHY_LINK_CAP_HD100;
    }

    if ((val & BMSR_10FD) != 0U)
    {
        *pCapabilities |= PHY_LINK_CAP_FD10;
    }

    if ((val & BMSR_10HD) != 0U)
    {
        *pCapabilities |= PHY_LINK_CAP_HD10;
    }

    /* Get extended (1 Gbps) capabilities if supported */
    if ((val & BMSR_GIGEXTSTS) != 0U)
    {
        status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_GIGESR, &val);

        if (PHY_SOK != status)
        {
            goto laError;
        }

        if ((val & GIGESR_1000FD) != 0U)
        {
            *pCapabilities |= PHY_LINK_CAP_FD1000;
        }

        if ((val & GIGESR_1000HD) != 0U)
        {
            *pCapabilities |= PHY_LINK_CAP_HD1000;
        }
    }

laError:

    return status;
}

int32_t GenericPhy_setAdvertisement (EthPhyDrv_Handle hPhy,
                                     uint32_t capabilities,
                                     uint32_t advertisement)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    uint32_t mask = 0;
    int32_t  status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if (0 == advertisement)
    {
        status = PHY_SOK;
        goto laError;
    }

    if (advertisement != (capabilities & advertisement))
    {
        status = PHY_EINVALIDPARAMS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if ((0 != (capabilities & PHY_LINK_ADV_10))  ||
        (0 != (capabilities & PHY_LINK_ADV_100)))
    {
        if (0 != (capabilities & PHY_LINK_ADV_HD10))
        {
            mask |= ANAR_10HD;

            if (0 != (advertisement & PHY_LINK_ADV_HD10))
            {
                val |= ANAR_10HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD10))
        {
            mask |= ANAR_10FD;

            if (0 != (advertisement & PHY_LINK_ADV_FD10))
            {
                val |= ANAR_10FD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_HD100))
        {
            mask |= ANAR_100HD;

            if (0 != (advertisement & PHY_LINK_ADV_HD100))
            {
                val |= ANAR_100HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD100))
        {
            mask |= ANAR_100FD;

            if (0 != (advertisement & PHY_LINK_ADV_FD100))
            {
                val |= ANAR_100FD;
            }
        }

        mask |= ANAR_802P3;
        val  |= ANAR_802P3;

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_ANAR, mask, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

    if (capabilities & PHY_LINK_ADV_1000)
    {
        mask = 0;
        val  = 0;

        if (0 != (capabilities & PHY_LINK_ADV_HD1000))
        {
            mask |= GIGCR_1000HD;

            if (0 != (advertisement & PHY_LINK_ADV_HD1000))
            {
                val |= GIGCR_1000HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD1000))
        {
            mask |= GIGCR_1000FD;

            if (0 != (advertisement & PHY_LINK_ADV_FD1000))
            {
                val |= GIGCR_1000FD;
            }
        }

        status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_GIGCR, mask, val);

        if (PHY_SOK != status)
        {
            goto laError;
        }
    }

laError:

    return status;
}

int32_t GenericPhy_enableAdvertisement (EthPhyDrv_Handle hPhy,
                                        uint32_t capabilities,
                                        uint32_t advertisement)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    uint32_t mask = 0;
    int32_t  status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if (0 == advertisement)
    {
        status = PHY_SOK;
        goto laError;
    }

    if (advertisement != (capabilities & advertisement))
    {
        status = PHY_EINVALIDPARAMS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if ((0 != (capabilities & PHY_LINK_ADV_10))  ||
        (0 != (capabilities & PHY_LINK_ADV_100)))
    {
        if (0 != (capabilities & PHY_LINK_ADV_HD10))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD10))
            {
                mask |= ANAR_10HD;
                val |= ANAR_10HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD10))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD10))
            {
                mask |= ANAR_10FD;
                val |= ANAR_10FD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_HD100))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD100))
            {
                mask |= ANAR_100HD;
                val |= ANAR_100HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD100))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD100))
            {
                mask |= ANAR_100FD;
                val |= ANAR_100FD;
            }
        }

        if (0 != mask)
        {
            status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_ANAR, mask, val);

            if (PHY_SOK != status)
            {
                goto laError;
            }
        }
    }

    if (capabilities & PHY_LINK_ADV_1000)
    {
        val  = 0;
        mask = 0;

        if (0 != (capabilities & PHY_LINK_ADV_HD1000))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD1000))
            {
                mask |= GIGCR_1000HD;
                val |= GIGCR_1000HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD1000))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD1000))
            {
                mask |= GIGCR_1000FD;
                val |= GIGCR_1000FD;
            }
        }

        if (0 != mask)
        {
            status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_GIGCR, mask, val);

           if (PHY_SOK != status)
           {
               goto laError;
           }
        }
    }

laError:

    return status;
}

int32_t GenericPhy_disableAdvertisement (EthPhyDrv_Handle hPhy,
                                         uint32_t capabilities,
                                         uint32_t advertisement)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint32_t mask = 0;
    int32_t  status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if (0 == advertisement)
    {
        status = PHY_SOK;
        goto laError;
    }

    if (advertisement != (capabilities & advertisement))
    {
        status = PHY_EINVALIDPARAMS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    if ((0 != (capabilities & PHY_LINK_ADV_10))  ||
        (0 != (capabilities & PHY_LINK_ADV_100)))
    {
        if (0 != (capabilities & PHY_LINK_ADV_HD10))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD10))
            {
                mask |= ANAR_10HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD10))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD10))
            {
                mask |= ANAR_10FD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_HD100))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD100))
            {
                mask |= ANAR_100HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD100))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD100))
            {
                mask |= ANAR_100FD;
            }
        }

        if (0 != mask)
        {
            status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_ANAR, mask, 0);

            if (PHY_SOK != status)
            {
                goto laError;
            }
        }
    }

    if (capabilities & PHY_LINK_ADV_1000)
    {
        mask = 0;

        if (0 != (capabilities & PHY_LINK_ADV_HD1000))
        {
            if (0 != (advertisement & PHY_LINK_ADV_HD1000))
            {
                mask |= GIGCR_1000HD;
            }
        }

        if (0 != (capabilities & PHY_LINK_ADV_FD1000))
        {
            if (0 != (advertisement & PHY_LINK_ADV_FD1000))
            {
                mask |= GIGCR_1000FD;
            }
        }

        if (0 != mask)
        {
            status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_GIGCR, mask, 0);

           if (PHY_SOK != status)
           {
               goto laError;
           }
        }
    }

laError:

    return status;
}

int32_t GenericPhy_ctrlAutoNegotiation(EthPhyDrv_Handle hPhy,
                                       uint32_t control)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val  = 0;
    uint16_t mask = 0;
    int32_t  status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

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
            status = PHY_EINVALIDPARAMS;
            break;
    }

    if (PHY_EINVALIDPARAMS == status)
    {
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, mask, val);

laError:

    return status;
}

int32_t GenericPhy_isLinkPartnerAutoNegotiationAble (EthPhyDrv_Handle hPhy,
                                                     bool *pAble)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pAble))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_ANER, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pAble = false;

    if (ANER_LPISANABLE == (val & ANER_LPISANABLE))
    {
        *pAble = true;
    }

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationEnabled(EthPhyDrv_Handle hPhy,
                                            bool *pEnabled)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pEnabled))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMCR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pEnabled = false;

    if (BMCR_ANEN == (val & BMCR_ANEN))
    {
        *pEnabled = true;
    }

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationComplete (EthPhyDrv_Handle hPhy,
                                              bool *pCompleted)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pCompleted))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pCompleted = false;

    if (BMSR_ANCOMPLETE == (val & BMSR_ANCOMPLETE))
    {
        *pCompleted = true;
    }

laError:

    return status;
}

int32_t GenericPhy_isAutoNegotiationRestartComplete (EthPhyDrv_Handle hPhy,
                                                     bool *pCompleted)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    bool res = false;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pCompleted))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

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
                                   uint32_t capabilities,
                                   uint32_t settings)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    uint16_t mask = 0;
    int32_t status = PHY_EFAIL;

    if (NULL == hPhy)
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_rmwReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    mask = BMCR_SPEED100 | BMCR_SPEED1000 | BMCR_FD;

    switch(settings)
    {
        case PHY_LINK_HD10:
            /* Select 10Mbps, Half-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_HD10))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = 0;
            break;
        case PHY_LINK_FD10:
            /* Select 10Mbps, Full-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_FD10))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = BMCR_FD;
            break;
        case PHY_LINK_HD100:
            /* Select 100Mbps, Half-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_HD100))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = BMCR_SPEED100;
            break;
        case PHY_LINK_FD100:
            /* Select 100Mbps, Full-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_FD100))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = BMCR_SPEED100 | BMCR_FD;
            break;
        case PHY_LINK_HD1000:
            /* Select 1000Mbps, Half-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_HD1000))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = BMCR_SPEED1000;
            break;
        case PHY_LINK_FD1000:
            /* Select 1000Mbps, Full-Duplex */
            if (0 == (capabilities & PHY_LINK_CAP_FD1000))
            {
                status = PHY_EINVALIDPARAMS;
            }

            val  = BMCR_SPEED1000 | BMCR_FD;
            break;
        default:
            /* unknown control command */
            status = PHY_EINVALIDPARAMS;
            break;
    }

    if (PHY_EINVALIDPARAMS == status)
    {
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_rmwReg(pRegAccessApi->pArgs, PHY_BMCR, mask, val);

laError:

    return status;
}

int32_t GenericPhy_isLinkUp (EthPhyDrv_Handle hPhy,
                             bool *pLinkUp)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pLinkUp))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

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

        *pLinkUp = false;

        if (0 != (val & BMSR_LINKSTS))
        {
            *pLinkUp = true;
        }
    }

laError:

    return status;
}

int32_t GenericPhy_isGigabitSupported (EthPhyDrv_Handle hPhy,
                                       bool *pSupported)
{
    Phy_RegAccessCb_t* pRegAccessApi = NULL;
    uint16_t val = 0;
    int32_t status = PHY_EFAIL;

    if ((NULL == hPhy) ||
        (NULL == pSupported))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    pRegAccessApi = &((Phy_Obj_t*) hPhy)->regAccessApi;

    if ((NULL == pRegAccessApi->EnetPhy_readReg) ||
        (NULL == pRegAccessApi->pArgs))
    {
        status = PHY_EBADARGS;
        goto laError;
    }

    status = pRegAccessApi->EnetPhy_readReg(pRegAccessApi->pArgs, PHY_BMSR, &val);

    if (PHY_SOK != status)
    {
        goto laError;
    }

    *pSupported = false;

    if (0 != (val & BMSR_GIGEXTSTS))
    {
        *pSupported = true;
    }

laError:

    return status;
}