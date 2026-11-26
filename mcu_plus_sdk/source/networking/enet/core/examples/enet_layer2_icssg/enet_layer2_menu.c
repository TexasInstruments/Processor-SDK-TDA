/*
 * Copyright (C) 2024-2026 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <enet_apputils.h>
#include <enet.h>
#include <enet_cfg.h>
#include <include/core/enet_dma.h>
#include <include/per/icssg.h>
#include <include/phy/enetphy.h>
#include "ti_enet_config.h"
#include "enet_layer2_menu.h"

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
#include <priv/per/icssg_priv.h>
#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 0U)
/* Normal mode: CSL hardware path requires PRUICSS attrs for mdioBaseAddr */
#include <drivers/pruicss.h>
#include <hw_include/mdio/csl_mdio.h>
#endif
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Any value above the largest valid menu choice (3); see showSwitchConfigMenu */
#define ENETMP_MENU_CHOICE_INVALID               (0xFFFFFFFFU)

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
/* Advertisement bitmasks used by EnetLayer2_setLinkSpeed(). The SDK's
 * phy_common.h has no GIGCR/ANAR advertisement bit names, so these mirror
 * GIGCR_1000FD/GIGCR_1000HD and ANAR_100FD/... from the lwip example's
 * extPhyMgmt/generic_phy.h. Keep the values in sync with that file. */
#define PHY_1000FD_ADV      (0x0200U)   /* GIGCR bit 9 */
#define PHY_1000HD_ADV      (0x0100U)   /* GIGCR bit 8 */
#define PHY_ANAR_100FD      (0x0100U)   /* ANAR bit 8 */
#define PHY_ANAR_100HD      (0x0080U)   /* ANAR bit 7 */
#define PHY_ANAR_10FD       (0x0040U)   /* ANAR bit 6 */
#define PHY_ANAR_10HD       (0x0020U)   /* ANAR bit 5 */
#endif /* ENET_SYSCFG_ENABLE_EXTPHY == 0U */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static uint32_t EnetLayer2_promptUint(const char *prompt, uint32_t minVal, uint32_t maxVal);

static uint32_t EnetLayer2_selectInstance(uint32_t numPerCtxts);

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
static int32_t EnetLayer2_setLinkSpeed(Icssg_Handle hIcssg, uint32_t mdioBaseAddr,
                                        uint8_t phyPortNum, uint8_t linkSpeed);
#endif

static int32_t EnetLayer2_applyForwardModeAllQueues(Enet_Handle hEnet, uint32_t coreId,
                                                      uint32_t instIdx,
                                                      const Enet_MacPort *macPorts,
                                                      uint8_t numMacPorts,
                                                      Icssg_QueueForwardMode fwdMode);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Last-applied forwarding mode per instance/port/queue. Needed because the
 * underlying IOCTL always writes all 8 queues at once with no corresponding
 * read-back, so changing a single queue has to resubmit the other 7 unchanged. */
static Icssg_QueueForwardMode s_queueFwdMode[ENETMP_PER_MAX][ENETMP_PORT_MAX][ENET_PRI_NUM];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static uint32_t EnetLayer2_promptUint(const char *prompt, uint32_t minVal, uint32_t maxVal)
{
    uint32_t val;
    int32_t  status;

    EnetAppUtils_print("%s", prompt);
    do
    {
        /* DebugP_scanf() only reports the readline status, not whether the
         * text parsed, so pre-set an out-of-range value and treat anything
         * still out of range after the read as invalid input. */
        val = maxVal + 1U;
        status = DebugP_scanf("%u", &val);
        if ((status != ENET_SOK) || (val < minVal) || (val > maxVal))
        {
            EnetAppUtils_print("\r\nInvalid selection, enter a value between %u and %u: ",
                               minVal, maxVal);
            val = maxVal + 1U;
        }
    } while (val > maxVal);

    return val;
}

static uint32_t EnetLayer2_selectInstance(uint32_t numPerCtxts)
{
    uint32_t instIdx = 0U;

    if (numPerCtxts > 1U)
    {
        instIdx = EnetLayer2_promptUint("\r\nSelect instance [0-N]: ", 0U, numPerCtxts - 1U);
    }

    return instIdx;
}

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
static int32_t EnetLayer2_setLinkSpeed(Icssg_Handle hIcssg, uint32_t mdioBaseAddr,
                                        uint8_t phyPortNum, uint8_t linkSpeed)
{
    uint16_t       regVal;
    uint32_t       phyAddr;
    EnetPhy_Handle hPhy;

    hPhy = hIcssg->hPhy[phyPortNum - 1U];
    if (hPhy == NULL)
    {
        EnetAppUtils_print("ERROR: NULL PHY handle for port %u\r\n", phyPortNum);
        return ENET_EFAIL;
    }
    phyAddr = hPhy->phyCfg.phyAddr;

    /* Full-duplex only; no half-duplex advertisement path is offered. */
    if (linkSpeed == 0U)
    {
        hPhy->reqLinkCaps = ENETPHY_LINK_CAP_FD1000;
#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 0U)
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_GIGCR, 0x0300U);
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_BMCR, &regVal);
        regVal |= PHY_BMCR_ANRESTART;
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_BMCR, regVal);
#else
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_GIGCR, 0x0300U, hPhy->mdioArgs);
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_BMCR, &regVal, hPhy->mdioArgs);
        regVal |= PHY_BMCR_ANRESTART;
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_BMCR, regVal, hPhy->mdioArgs);
#endif
    }
    else if (linkSpeed == 1U)
    {
        hPhy->reqLinkCaps = ENETPHY_LINK_CAP_FD100;
#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 0U)
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_GIGCR, &regVal);
        regVal &= ~(PHY_1000FD_ADV | PHY_1000HD_ADV);
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_GIGCR, regVal);
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_ANAR, &regVal);
        regVal &= ~(PHY_ANAR_100FD | PHY_ANAR_100HD | PHY_ANAR_10FD | PHY_ANAR_10HD);
        regVal |= PHY_ANAR_100FD;
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_ANAR, regVal);
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_BMCR, &regVal);
        regVal |= (PHY_BMCR_ANEN | PHY_BMCR_ANRESTART);
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_BMCR, regVal);
#else
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_GIGCR, &regVal, hPhy->mdioArgs);
        regVal &= ~(PHY_1000FD_ADV | PHY_1000HD_ADV);
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_GIGCR, regVal, hPhy->mdioArgs);
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_ANAR, &regVal, hPhy->mdioArgs);
        regVal &= ~(PHY_ANAR_100FD | PHY_ANAR_100HD | PHY_ANAR_10FD | PHY_ANAR_10HD);
        regVal |= PHY_ANAR_100FD;
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_ANAR, regVal, hPhy->mdioArgs);
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_BMCR, &regVal, hPhy->mdioArgs);
        regVal |= (PHY_BMCR_ANEN | PHY_BMCR_ANRESTART);
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_BMCR, regVal, hPhy->mdioArgs);
#endif
    }
    else if (linkSpeed == 2U)
    {
        hPhy->reqLinkCaps = ENETPHY_LINK_CAP_FD10;
#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 0U)
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_GIGCR, &regVal);
        regVal &= ~(PHY_1000FD_ADV | PHY_1000HD_ADV);
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_GIGCR, regVal);
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_ANAR, &regVal);
        regVal &= ~(PHY_ANAR_100FD | PHY_ANAR_100HD | PHY_ANAR_10FD | PHY_ANAR_10HD);
        regVal |= PHY_ANAR_10FD;
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_ANAR, regVal);
        CSL_MDIO_phyRegRead(mdioBaseAddr, phyAddr, PHY_BMCR, &regVal);
        regVal |= (PHY_BMCR_ANEN | PHY_BMCR_ANRESTART);
        CSL_MDIO_phyRegWrite(mdioBaseAddr, phyAddr, PHY_BMCR, regVal);
#else
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_GIGCR, &regVal, hPhy->mdioArgs);
        regVal &= ~(PHY_1000FD_ADV | PHY_1000HD_ADV);
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_GIGCR, regVal, hPhy->mdioArgs);
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_ANAR, &regVal, hPhy->mdioArgs);
        regVal &= ~(PHY_ANAR_100FD | PHY_ANAR_100HD | PHY_ANAR_10FD | PHY_ANAR_10HD);
        regVal |= PHY_ANAR_10FD;
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_ANAR, regVal, hPhy->mdioArgs);
        hPhy->hMdio->readC22(hPhy->group, phyAddr, PHY_BMCR, &regVal, hPhy->mdioArgs);
        regVal |= (PHY_BMCR_ANEN | PHY_BMCR_ANRESTART);
        hPhy->hMdio->writeC22(hPhy->group, phyAddr, PHY_BMCR, regVal, hPhy->mdioArgs);
#endif
    }

    ClockP_usleep(1000U);

    return ENET_SOK;
}
#endif /* ENET_SYSCFG_ENABLE_EXTPHY == 0U */

static int32_t EnetLayer2_applyForwardModeAllQueues(Enet_Handle hEnet, uint32_t coreId,
                                                      uint32_t instIdx,
                                                      const Enet_MacPort *macPorts,
                                                      uint8_t numMacPorts,
                                                      Icssg_QueueForwardMode fwdMode)
{
    IcssgMacPort_SetQueueCtPremptModeInArgs inArgs;
    Enet_IoctlPrms prms;
    int32_t status = ENET_SOK;
    int32_t portStatus;
    uint8_t portIdx;
    uint32_t q;

    for (portIdx = 0U; portIdx < numMacPorts; portIdx++)
    {
        inArgs.macPort = macPorts[portIdx];

        for (q = 0U; q < ENET_PRI_NUM; q++)
        {
            inArgs.queuePreemptMode[q] = ICSSG_QUEUE_PREEMPT_MODE_EXPRESS;
            inArgs.queueForwardMode[q] = fwdMode;
        }

        ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);
        ENET_IOCTL(hEnet, coreId, ICSSG_MACPORT_IOCTL_SET_QUEUE_CUT_THROUGH_PREEMPT_SELECT,
                   &prms, portStatus);
        if (portStatus != ENET_SOK)
        {
            EnetAppUtils_print("ERROR: IOCTL failed for MAC port %u: %d\r\n",
                               inArgs.macPort, portStatus);
            status = portStatus;
        }
        else
        {
            /* Keep the per-queue shadow state in step with what the IOCTL applied */
            for (q = 0U; q < ENET_PRI_NUM; q++)
            {
                s_queueFwdMode[instIdx][portIdx][q] = fwdMode;
            }
        }
    }

    return status;
}

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
static int32_t EnetLayer2_findPortInstance(const EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts,
                                           Enet_MacPort macPort, uint32_t *instIdx)
{
    uint32_t i;
    uint32_t j;

    for (i = 0U; i < numPerCtxts; i++)
    {
        for (j = 0U; j < perCtxt[i].macPortNum; j++)
        {
            if (perCtxt[i].macPort[j] == macPort)
            {
                *instIdx = i;
                return ENET_SOK;
            }
        }
    }

    return ENET_EFAIL;
}

static int32_t EnetLayer2_setPortLinkSpeed(EnetMp_PerCtxt *perCtxt, uint32_t instIdx,
                                           uint8_t phyPortNum, uint8_t linkSpeed)
{
    Enet_Handle  hEnet = perCtxt[instIdx].handleInfo.hEnet;
    Icssg_Handle hIcssg;
    uint32_t     mdioBaseAddr = 0U;

    if (hEnet == NULL)
    {
        EnetAppUtils_print("ERROR: NULL Enet handle for instance %u\r\n", instIdx);
        return ENET_EFAIL;
    }

    hIcssg = (Icssg_Handle)(hEnet->enetPer);
    if (hIcssg == NULL)
    {
        EnetAppUtils_print("ERROR: NULL ICSSG handle for instance %u\r\n", instIdx);
        return ENET_EFAIL;
    }

#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 0U)
    mdioBaseAddr = (uint32_t)((PRUICSS_HwAttrs *)(hIcssg->pruss->hPruss->hwAttrs))->miiMdioRegBase;
#endif

    return EnetLayer2_setLinkSpeed(hIcssg, mdioBaseAddr, phyPortNum, linkSpeed);
}
#endif /* ENET_SYSCFG_ENABLE_EXTPHY == 0U */

void EnetLayer2_showLinkSpeedMenu(EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts)
{
    uint32_t instIdx = 0U;
    uint8_t  phyPortNum;
    uint8_t  linkSpeed;
    uint8_t  portNum;
    int32_t  status = ENET_SOK;
    int32_t  portStatus;

    phyPortNum = (uint8_t)EnetLayer2_promptUint(
        "\r\nSelect PHY port number (1 or 2) or 3 for BOTH ports: ", 1U, 3U);
    linkSpeed  = (uint8_t)EnetLayer2_promptUint(
        "\r\nSelect link speed [0 : 1G(default)  1 : 100M   2 : 10M] : ", 0U, 2U);

#if (ENET_SYSCFG_ENABLE_EXTPHY == 0U)
    for (portNum = 1U; portNum <= 2U; portNum++)
    {
        if ((phyPortNum & portNum) == 0U)
        {
            continue;
        }

        if (EnetLayer2_findPortInstance(perCtxt, numPerCtxts, (Enet_MacPort)(portNum - 1U),
                                        &instIdx) != ENET_SOK)
        {
            EnetAppUtils_print("ERROR: no instance owns MAC port %u\r\n", portNum);
            status = ENET_EFAIL;
            continue;
        }

        portStatus = EnetLayer2_setPortLinkSpeed(perCtxt, instIdx, portNum, linkSpeed);
        if (portStatus != ENET_SOK)
        {
            EnetAppUtils_print("ERROR: failed to configure port %u: %d\r\n", portNum, portStatus);
            status = portStatus;
        }
    }

    if (status == ENET_SOK)
    {
        EnetAppUtils_print("Link speed configuration complete\r\n");
    }
#else
    EnetAppUtils_print("ERROR: link speed is not supported in EXTPHY builds\r\n");
#endif
}

void EnetLayer2_showForwardModeMenu(EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts, uint32_t coreId)
{
    uint32_t               instIdx;
    uint32_t               fwdSel;
    Icssg_QueueForwardMode fwdMode;
    Enet_Handle            hEnet;
    int32_t                status;

    instIdx = EnetLayer2_selectInstance(numPerCtxts);
    fwdSel  = EnetLayer2_promptUint(
        "\r\nSelect forwarding mode [0: Store-and-Forward (default)  1: Cut-Through]: ", 0U, 1U);
    fwdMode = (fwdSel == 1U) ? ICSSG_QUEUE_FORWARD_MODE_CUTTHROUGH : ICSSG_QUEUE_FORWARD_MODE_STOREANDFWD;
    EnetAppUtils_print("\r\n%s selected\r\n", (fwdSel == 1U) ? "Cut-Through" : "Store-and-Forward");

    hEnet = perCtxt[instIdx].handleInfo.hEnet;
    if (hEnet == NULL)
    {
        EnetAppUtils_print("ERROR: NULL Enet handle for instance %u\r\n", instIdx);
        return;
    }

    status = EnetLayer2_applyForwardModeAllQueues(hEnet, coreId, instIdx,
                                                   perCtxt[instIdx].macPort,
                                                   perCtxt[instIdx].macPortNum, fwdMode);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("ERROR: forwarding mode configuration failed: %d\r\n", status);
    }
    else
    {
        EnetAppUtils_print("Forwarding mode configuration complete\r\n");
    }
}

void EnetLayer2_showQueueForwardModeMenu(EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts, uint32_t coreId)
{
    IcssgMacPort_SetQueueCtPremptModeInArgs inArgs;
    Enet_IoctlPrms         prms;
    uint32_t               instIdx;
    uint32_t               portSel;
    uint8_t                portIdx;
    uint32_t               queueIdx;
    uint32_t               fwdSel;
    Icssg_QueueForwardMode fwdMode;
    Enet_Handle            hEnet;
    int32_t                status;
    uint32_t               q;

    instIdx = EnetLayer2_selectInstance(numPerCtxts);
    if (perCtxt[instIdx].macPortNum == 0U)
    {
        EnetAppUtils_print("ERROR: No MAC ports available on instance %u\r\n", instIdx);
        return;
    }

    portSel  = EnetLayer2_promptUint("\r\nSelect MAC port number (1 or 2): ",
                                      1U, perCtxt[instIdx].macPortNum);
    portIdx  = (uint8_t)(portSel - 1U);

    queueIdx = EnetLayer2_promptUint("\r\nSelect queue number (0-7): ", 0U, ENET_PRI_NUM - 1U);
    fwdSel   = EnetLayer2_promptUint(
        "\r\nSelect forwarding mode [0: Store-and-Forward (default)  1: Cut-Through]: ", 0U, 1U);
    fwdMode  = (fwdSel == 1U) ? ICSSG_QUEUE_FORWARD_MODE_CUTTHROUGH : ICSSG_QUEUE_FORWARD_MODE_STOREANDFWD;

    hEnet = perCtxt[instIdx].handleInfo.hEnet;
    if (hEnet == NULL)
    {
        EnetAppUtils_print("ERROR: NULL Enet handle for instance %u\r\n", instIdx);
        return;
    }

    /* Build the request from the shadow state with only this queue changed,
     * and commit the shadow update only once the IOCTL has succeeded. */
    inArgs.macPort = perCtxt[instIdx].macPort[portIdx];
    for (q = 0U; q < ENET_PRI_NUM; q++)
    {
        inArgs.queuePreemptMode[q] = ICSSG_QUEUE_PREEMPT_MODE_EXPRESS;
        inArgs.queueForwardMode[q] = (q == queueIdx) ? fwdMode : s_queueFwdMode[instIdx][portIdx][q];
    }

    ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);
    ENET_IOCTL(hEnet, coreId, ICSSG_MACPORT_IOCTL_SET_QUEUE_CUT_THROUGH_PREEMPT_SELECT, &prms, status);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("ERROR: IOCTL failed for MAC port %u: %d\r\n", inArgs.macPort, status);
    }
    else
    {
        s_queueFwdMode[instIdx][portIdx][queueIdx] = fwdMode;
        EnetAppUtils_print("Queue %u set to %s\r\n", queueIdx,
                           (fwdSel == 1U) ? "Cut-Through" : "Store-and-Forward");
    }
}

bool EnetLayer2_isSwitchBuild(const EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts)
{
    uint32_t i;

    for (i = 0U; i < numPerCtxts; i++)
    {
        if (perCtxt[i].enetType == ENET_ICSSG_SWITCH)
        {
            return true;
        }
    }

    return false;
}

void EnetLayer2_showSwitchConfigMenu(EnetMp_PerCtxt *perCtxt, uint32_t numPerCtxts, uint32_t coreId)
{
    uint32_t choice;
    bool     isSwitch = EnetLayer2_isSwitchBuild(perCtxt, numPerCtxts);

    do
    {
        if (isSwitch)
        {
            EnetAppUtils_print("\r\nSwitch Configuration Menu:\r\n");
        }
        else
        {
            EnetAppUtils_print("\r\nDUAL-MAC Configuration Menu:\r\n");
        }
        EnetAppUtils_print(" 1 - Configure link speed\r\n");
        if (isSwitch)
        {
            EnetAppUtils_print(" 2 - Configure forwarding mode (all queues)\r\n");
            EnetAppUtils_print(" 3 - Configure forwarding mode (single queue)\r\n");
        }
        EnetAppUtils_print(" 0 - Back\r\n");
        EnetAppUtils_print("Enter choice: ");
        /* Pre-set to an invalid choice so a non-numeric entry reaches the
         * default branch instead of using an unset value. */
        choice = ENETMP_MENU_CHOICE_INVALID;
        DebugP_scanf("%u", &choice);

        switch (choice)
        {
            case 1U:
                EnetLayer2_showLinkSpeedMenu(perCtxt, numPerCtxts);
                break;
            case 2U:
                if (isSwitch)
                {
                    EnetLayer2_showForwardModeMenu(perCtxt, numPerCtxts, coreId);
                    break;
                }
                EnetAppUtils_print("Invalid choice\r\n");
                break;
            case 3U:
                if (isSwitch)
                {
                    EnetLayer2_showQueueForwardModeMenu(perCtxt, numPerCtxts, coreId);
                    break;
                }
                EnetAppUtils_print("Invalid choice\r\n");
                break;
            case 0U:
                break;
            default:
                EnetAppUtils_print("Invalid choice\r\n");
                break;
        }
    } while (choice != 0U);
}
