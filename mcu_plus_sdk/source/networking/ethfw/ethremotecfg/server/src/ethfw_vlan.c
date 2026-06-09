/*
 *  Copyright (c) Texas Instruments Incorporated 2023-2024
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
 * \file  ethfw_vlan.c
 *
 * \brief This file contains the implementation of VLAN helper functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x105


/* Enet LLD header files */
#include <enet.h>
#include <include/per/cpsw.h>

/* EthFw header files */
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <ethremotecfg/server/include/ethfw_virtport.h>
#include "ethfw_vlan_priv.h"

#if defined(ETHFW_VEPA_SUPPORT)
#include "ethfw_vepa_priv.h"
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*!
 * \brief Static VLAN info.
 */
typedef struct EthFwVlan_Vlan_s
{
    /* VLAN id */
    uint16_t vlanId;

    /* ALE member mask of hardware ports */
    uint32_t memberMask;

    /* ALE port mask where registered multicast packets will be flooded to */
    uint32_t regMcastFloodMask;

    /* ALE port mask where unregistered multicast packets will be flooded to */
    uint32_t unregMcastFloodMask;

    /* ALE port mask where VLAN tag must be removed on egress */
    uint32_t untagMask;

    /* Member mask of virtual ports */
    uint32_t virtMemberMask;

    /* Member mask of virtual ports that have joined the VLAN */
    uint32_t virtActiveMask;

    /* Whether this entry is free or not */
    bool isFree;

    /* Whether this entry is statically allocated */
    bool isStatic;
} EthFwVlan_Vlan;

/* ETHFW VLAN object */
typedef struct EthFwVlan_Obj_s
{
    /* VLAN configuration */
    EthFwVlan_Vlan vlan[ETHFWVLAN_VLANS_MAX];

    /* Number of valid VLAN configuration entries */
    uint32_t numVlans;

    /* Default port mask for all ports in switch mode */
    uint32_t defaultPortMask;

    /* Default virtual port mask for all ports in switch mode */
    uint32_t defaultVirtPortMask;

    /* Whether forwarding to all Switch ports for dynamic VLANs is enabled */
    bool dVlanSwtFwdEn;

    /*! Mutex handle used to protect VLAN configuration table */
    EthFwOsal_MutexHandle hMutex;
} EthFwVlan_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static inline uint32_t EthFwVlan_popcount(uint32_t v);

static EthFwVlan_Vlan *EthFwVlan_getVlan(uint16_t vlanId);

static int32_t EthFwVlan_getCfg(const EthFwVlan_Cfg *cfg);

static int32_t EthFwVlan_setupVlan(Enet_Handle hEnet,
                                   uint16_t vlanId,
                                   uint32_t memberMask,
                                   uint32_t regMcastFloodMask,
                                   uint32_t unregMcastFloodMask,
                                   uint32_t untagMask);

static void EthFwVlan_deleteVlan(Enet_Handle hEnet,
                                 uint16_t vlanId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ETHFW VLAN object */
static EthFwVlan_Obj gEthFwVlanObj;

/* Broadcast address */
#if defined(ETHFW_VEPA_SUPPORT)
struct eth_addr bcastAddr = {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwVlan_init(Enet_Handle hEnet,
                       const EthFwVlan_Cfg *cfg)
{
    EthFwVlan_Vlan *vlan;
    uint32_t i;
    int32_t status = ENET_SOK;

    gEthFwVlanObj.defaultPortMask     = cfg->defaultPortMask;
    gEthFwVlanObj.defaultVirtPortMask = cfg->defaultVirtPortMask;
    gEthFwVlanObj.dVlanSwtFwdEn       = cfg->dVlanSwtFwdEn;

    /* Create mutex to protect VLAN configuration table */
    gEthFwVlanObj.hMutex = EthFwOsal_createMutex();
    if (gEthFwVlanObj.hMutex == NULL)
    {
        status = ENET_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to create mutex");
    }

    /* Mark all entries in table as free and dynamic. */
    if (status == ETHFW_SOK)
    {
        for (i = 0U; i < ETHFWVLAN_VLANS_MAX; i++)
        {
            gEthFwVlanObj.vlan[i].isFree = BTRUE;
            gEthFwVlanObj.vlan[i].isStatic = BFALSE;
        }
    }

    /* Check config params and save VLAN configuration */
    if (status == ENET_SOK)
    {
        status = EthFwVlan_getCfg(cfg);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Incorrect static VLAN params");
    }

    /* Add VLAN entry for static VLANs in ALE */
    if (status == ENET_SOK)
    {
        for (i = 0U; i < gEthFwVlanObj.numVlans; i++)
        {
            vlan = &gEthFwVlanObj.vlan[i];

            if (vlan->isFree == BFALSE)
            {
                /* Exclude host post from the VLAN, will be added when virtual port(s) join */
                status = EthFwVlan_setupVlan(hEnet,
                                            vlan->vlanId,
                                            vlan->memberMask & ~CPSW_ALE_HOST_PORT_MASK,
                                            vlan->regMcastFloodMask & ~CPSW_ALE_HOST_PORT_MASK,
                                            vlan->unregMcastFloodMask & ~CPSW_ALE_HOST_PORT_MASK,
                                            vlan->untagMask & ~CPSW_ALE_HOST_PORT_MASK);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to setup VLANs in ALE");
            }
        }

        ETHFWTRACE_INFO_IF((status == ETHFW_SOK), "%u VLAN entries added in ALE table", i);
    }

    return status;
}

void EthFwVlan_deinit(Enet_Handle hEnet)
{
    EthFwVlan_Vlan *vlan;
    int32_t i;

    /* Delete all static VLANs */
    for (i = 0U; i < gEthFwVlanObj.numVlans; i++)
    {
        vlan = &gEthFwVlanObj.vlan[i];

        ETHFWTRACE_ERR_IF((vlan->virtActiveMask != 0U), ENET_EUNEXPECTED,
                           "Virtual ports still active 0x%x", vlan->virtActiveMask);

        EthFwVlan_deleteVlan(hEnet, vlan->vlanId);
    }

    /* Delete VLAN configuration table mutex */
    if (gEthFwVlanObj.hMutex != NULL)
    {
        EthFwOsal_deleteMutex(gEthFwVlanObj.hMutex);
        gEthFwVlanObj.hMutex = NULL;
    }
}

int32_t EthFwVlan_join(Enet_Handle hEnet,
                       EthRemoteCfg_VirtPort virtPort,
                       uint16_t vlanId,
                       const uint8_t *macAddr,
                       uint32_t flowIdx)
{
    EthFwVlan_Vlan *vlan = NULL;
    int32_t status = ENET_SOK;
    Enet_MacPort macPort = EthFwVirtPort_getMacPort(virtPort);
    bool isMacPort = EthFwVirtPort_isMacPort(virtPort);
    uint32_t i;
#if defined(ETHFW_VEPA_SUPPORT)
    uint32_t coreId;
#endif

    /* MAC address should be client's unicast address */
    if (EnetUtils_isMcastAddr(macAddr))
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Cannot join VLAN %u with mcast addr %02x:%02x:%02x:%02x:%02x:%02x",
                       vlanId,
                       macAddr[0U], macAddr[1U], macAddr[2U],
                       macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    EthFwOsal_lockMutex(gEthFwVlanObj.hMutex);

    /* Get VLAN info for the VLAN id that remote client is trying to join */
    if (status == ENET_SOK)
    {
        vlan = EthFwVlan_getVlan(vlanId);

        /* Found a matching entry. */
        if (vlan != NULL)
        {
            /* Check if the virtual port is allowed in the VLAN */
            if (!ENET_IS_BIT_SET(vlan->virtMemberMask, virtPort))
            {
                status = ENET_EPERM;
                ETHFWTRACE_ERR(status, "Virtual port %u is not allowed in VLAN %u", virtPort, vlanId);
            }

            /* Update VLAN and broadcast entries in ALE table with host port added */
            if (status == ENET_SOK)
            {
                status = EthFwVlan_setupVlan(hEnet,
                                            vlan->vlanId,
                                            vlan->memberMask,
                                            vlan->regMcastFloodMask,
                                            vlan->unregMcastFloodMask,
                                            vlan->untagMask);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to update VLAN %u in ALE", vlanId);
            }

            /* Mark virtual port as active in the VLAN */
            if (status == ENET_SOK)
            {
                vlan->virtActiveMask |= ENET_BIT(virtPort);
            }

#if defined(ETHFW_VEPA_SUPPORT)
            if (status == ENET_SOK)
            {
                coreId = EnetSoc_getCoreId();
                /* Add broadcast entry with vlanId in VEPA table */
                status = EthFwVepa_addAddr(&bcastAddr, vlanId, virtPort);
            }
#endif
        }
        else
        {
            /* Allocate a new entry in the table and set up the vlan. */
            for (i = 0U; i < ETHFWVLAN_VLANS_MAX; i++)
            {
                vlan = &gEthFwVlanObj.vlan[i];

                if (vlan->isFree == BTRUE)
                {
                    break;
                }
            }

            if ((vlan != NULL) && (vlan->isFree == BTRUE))
            {
                vlan->vlanId              = vlanId;
                vlan->isFree              = BFALSE;

                if (isMacPort)
                {
                    vlan->memberMask  = CPSW_ALE_HOST_PORT_MASK;
                    vlan->memberMask |= CPSW_ALE_MACPORT_TO_PORTMASK(macPort);
                    vlan->regMcastFloodMask   = vlan->memberMask;
                    vlan->unregMcastFloodMask = vlan->memberMask;
                    vlan->virtMemberMask      = virtPort;
                }
                else
                {
                    if (gEthFwVlanObj.dVlanSwtFwdEn == BTRUE)
                    {
                        vlan->memberMask          = gEthFwVlanObj.defaultPortMask;
                        vlan->virtMemberMask      = gEthFwVlanObj.defaultVirtPortMask;
                        vlan->regMcastFloodMask   = gEthFwVlanObj.defaultPortMask;
                        vlan->unregMcastFloodMask = gEthFwVlanObj.defaultPortMask;
                    }
                    else
                    {
                        vlan->memberMask          = CPSW_ALE_HOST_PORT_MASK;
                        vlan->virtMemberMask      = 0U;
                        vlan->regMcastFloodMask   = CPSW_ALE_HOST_PORT_MASK;
                        vlan->unregMcastFloodMask = CPSW_ALE_HOST_PORT_MASK;
                    }
                }
                vlan->untagMask           = 0U;
                vlan->virtActiveMask      = 0U;
                gEthFwVlanObj.numVlans++;

                status = EthFwVlan_setupVlan(hEnet,
                                            vlan->vlanId,
                                            vlan->memberMask,
                                            vlan->regMcastFloodMask,
                                            vlan->unregMcastFloodMask,
                                            vlan->untagMask);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to update VLAN %u in ALE", vlanId);

                /* Mark virtual port as active in the VLAN */
                if (status == ENET_SOK)
                {
                    vlan->virtActiveMask |= ENET_BIT(virtPort);
                }

#if defined(ETHFW_VEPA_SUPPORT)
                if (status == ENET_SOK)
                {
                    coreId = EnetSoc_getCoreId();
                    /* Add broadcast entry with vlanId in VEPA table */
                    status = EthFwVepa_addAddr(&bcastAddr, vlanId, virtPort);
                }
#endif
            }
            else
            {
                status = ENET_EALLOC;
                ETHFWTRACE_ERR(status, "Failed to allocate a vlan entry for vlan %u ", vlanId);
            }
        }
    }

    EthFwOsal_unlockMutex(gEthFwVlanObj.hMutex);

    return status;
}

int32_t EthFwVlan_leave(Enet_Handle hEnet,
                        EthRemoteCfg_VirtPort virtPort,
                        uint16_t vlanId,
                        const uint8_t *macAddr,
                        uint32_t flowIdx)
{
    EthFwVlan_Vlan *vlan = NULL;
    int32_t status = ENET_SOK;
#if defined(ETHFW_VEPA_SUPPORT)
    uint32_t coreId;
#endif

    /* MAC address should be client's unicast address */
    if (EnetUtils_isMcastAddr(macAddr))
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Cannot leave VLAN %u with mcast addr %02x:%02x:%02x:%02x:%02x:%02x",
                       vlanId,
                       macAddr[0U], macAddr[1U], macAddr[2U],
                       macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    EthFwOsal_lockMutex(gEthFwVlanObj.hMutex);

    /* Get VLAN info for the VLAN id that remote client is trying to join */
    vlan = EthFwVlan_getVlan(vlanId);
    if (vlan == NULL)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "VLAN %u is not registered, virtual port %u cannot leave", vlanId, virtPort);
    }

    /* Check if the virtual port is allowed in the VLAN */
    if (status == ENET_SOK)
    {
        if (!ENET_IS_BIT_SET(vlan->virtMemberMask, virtPort))
        {
            status = ENET_EPERM;
            ETHFWTRACE_ERR(status, "Virtual port %u is not allowed in VLAN %u", virtPort, vlanId);
        }
    }

    /* Check if the virtual port had joined the VLAN */
    if (status == ENET_SOK)
    {
        if (!ENET_IS_BIT_SET(vlan->virtActiveMask, virtPort))
        {
            status = ENET_EINVALIDPARAMS;
            ETHFWTRACE_ERR(status, "Virtual port %u had not joined VLAN %u", virtPort);
        }
    }

    /* Clear virtual port from VLAN active list */
    if (status == ENET_SOK)
    {
        vlan->virtActiveMask &= ~ENET_BIT(virtPort);

        /* Exclude host post from the VLAN until virtual port(s) join again */
        if (vlan->virtActiveMask == 0U)
        {
            status = EthFwVlan_setupVlan(hEnet,
                                         vlan->vlanId,
                                         vlan->memberMask & ~CPSW_ALE_HOST_PORT_MASK,
                                         vlan->regMcastFloodMask & ~CPSW_ALE_HOST_PORT_MASK,
                                         vlan->unregMcastFloodMask & ~CPSW_ALE_HOST_PORT_MASK,
                                         vlan->untagMask & ~CPSW_ALE_HOST_PORT_MASK);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to setup VLANs in ALE");

            /* Delete the dynamic vlan entries */
            if (vlan->isStatic == BFALSE)
            {
                EthFwVlan_deleteVlan(hEnet,
                                     vlan->vlanId);
            }
        }
    }

#if defined(ETHFW_VEPA_SUPPORT)
    if (status == ENET_SOK)
    {
        coreId = EnetSoc_getCoreId();
        /* Remove broadcast entry with vlanId from VEPA table */
        status = EthFwVepa_delAddr(&bcastAddr, vlanId, virtPort);
    }
#endif

    EthFwOsal_unlockMutex(gEthFwVlanObj.hMutex);

    return status;
}

bool EthFwVlan_isInVlan(EthRemoteCfg_VirtPort virtPort,
                        uint16_t vlanId)
{
    EthFwVlan_Vlan *vlan = NULL;
    bool active = BFALSE;

    EthFwOsal_lockMutex(gEthFwVlanObj.hMutex);

    vlan = EthFwVlan_getVlan(vlanId);
    active = (vlan != NULL) && ENET_IS_BIT_SET(vlan->virtActiveMask, virtPort);

    EthFwOsal_unlockMutex(gEthFwVlanObj.hMutex);

    return active;
}

/* http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
static inline uint32_t EthFwVlan_popcount(uint32_t v)
{
    uint32_t c;

    for (c = 0U; v != 0U; c++)
    {
        v &= v - 1U;
    }

    return c;
}

static EthFwVlan_Vlan *EthFwVlan_getVlan(uint16_t vlanId)
{
    EthFwVlan_Vlan *vlan = NULL;
    uint32_t i;

    /* Find VLAN info for the given VLAN id */
    for (i = 0U; i < gEthFwVlanObj.numVlans; i++)
    {
        if (gEthFwVlanObj.vlan[i].vlanId == vlanId)
        {
            vlan = &gEthFwVlanObj.vlan[i];
        }
    }

    return vlan;
}

static int32_t EthFwVlan_getCfg(const EthFwVlan_Cfg *cfg)
{
    const EthFwVlan_VlanCfg *vlanCfg = NULL;
    EthFwVlan_Vlan *vlan = NULL;
    uint32_t aleSwitchPortMask = 0U;
    uint32_t aleMacOnlyPortMask = 0U;
    uint32_t memberMask = 0U;
    uint16_t vlanId = 0U;
    uint32_t i;
    int32_t status = ENET_SOK;

    aleSwitchPortMask  = (cfg->switchPortMask << 1U);
    aleMacOnlyPortMask = (cfg->macOnlyPortMask << 1U);

    /* Check that number of VLAN has not been exceeded (this is a software limitation) */
    if (cfg->numStaticVlans > ETHFWVLAN_VLANS_MAX)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Too many VLANs (%u), max is %u", cfg->numStaticVlans, ETHFWVLAN_VLANS_MAX);
    }

    /* Check all VLAN config params and add them to local VLAN info table */
    if (status == ENET_SOK)
    {
        for (i = 0U; i < cfg->numStaticVlans; i++)
        {
            vlanCfg = &cfg->vlanCfg[i];

            vlanId = vlanCfg->vlanId;
            memberMask = vlanCfg->memberMask;

            /* VLANs cannot be same as default VLAN ids for switch and MAC-only ports */
            if ((vlanId == cfg->dfltVlanIdSwitchPorts) ||
                (vlanId == cfg->dfltVlanIdMacOnlyPorts))
            {
                status = ENET_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "VLAN id %u cannot be same as default VLAN ids (%u, %u)",
                               vlanId, cfg->dfltVlanIdSwitchPorts, cfg->dfltVlanIdMacOnlyPorts);
                break;
            }

            /* Cannot have MAC-only and switch ports in the same VLAN, else leaks */
            if (status == ENET_SOK)
            {
                if (ENET_NOT_ZERO(memberMask & aleSwitchPortMask) &&
                    ENET_NOT_ZERO(memberMask & aleMacOnlyPortMask))
                {
                    status = ENET_EINVALIDPARAMS;
                    ETHFWTRACE_ERR(status, "VLAN %u member list 0x%x cannot be both MAC-only and switch ports",
                                   vlanId, memberMask);
                    break;
                }
            }

            /* Cannot have more than one MAC-only port in the same VLAN, else leaks */
            if (status == ENET_SOK)
            {
                memberMask &= aleMacOnlyPortMask;
                if (EthFwVlan_popcount(memberMask) > 1U)
                {
                    status = ENET_EINVALIDPARAMS;
                    ETHFWTRACE_ERR(status, "VLAN %u member list %x cannot have multiple MAC-only ports",
                                   vlanId, memberMask);
                    break;
                }
            }

            /* All checks passed, save it */
            if (status == ENET_SOK)
            {
                vlan = &gEthFwVlanObj.vlan[i];

                /* Cap masks to member mask */
                vlan->vlanId              = vlanCfg->vlanId;
                vlan->isFree              = BFALSE;
                vlan->isStatic            = BTRUE;
                vlan->memberMask          = vlanCfg->memberMask;
                vlan->virtMemberMask      = vlanCfg->virtMemberMask;
                vlan->regMcastFloodMask   = vlanCfg->regMcastFloodMask & vlanCfg->memberMask;
                vlan->unregMcastFloodMask = vlanCfg->unregMcastFloodMask & vlanCfg->memberMask;
                vlan->untagMask           = vlanCfg->untagMask &vlanCfg->memberMask;
                vlan->virtActiveMask      = 0U;
                gEthFwVlanObj.numVlans++;

                ETHFWTRACE_INFO("VLAN %u member=0x%x virtMember=0x%x "
                                "regMcastFlood=0x%x unregMcastFlood=0x%x untag=0x%x",
                                vlan->vlanId,
                                vlan->memberMask,
                                vlan->virtMemberMask,
                                vlan->regMcastFloodMask,
                                vlan->unregMcastFloodMask,
                                vlan->untagMask);
            }
        }
    }

    return status;
}

static int32_t EthFwVlan_setupVlan(Enet_Handle hEnet,
                                   uint16_t vlanId,
                                   uint32_t memberMask,
                                   uint32_t regMcastFloodMask,
                                   uint32_t unregMcastFloodMask,
                                   uint32_t untagMask)
{
    CpswAle_VlanEntryInfo vlanInArgs;
    Enet_IoctlPrms prms;
    uint32_t aleEntry;
    uint32_t coreId;
    int32_t status = ENET_SOK;

    coreId = EnetSoc_getCoreId();

    /* Add ALE VLAN entry */
    memset(&vlanInArgs, 0U, sizeof (CpswAle_VlanEntryInfo));
    vlanInArgs.vlanIdInfo.vlanId       = vlanId;
    vlanInArgs.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
    vlanInArgs.vlanMemberList          = memberMask;
    vlanInArgs.regMcastFloodMask       = regMcastFloodMask;
    vlanInArgs.unregMcastFloodMask     = unregMcastFloodMask;
    vlanInArgs.forceUntaggedEgressMask = untagMask;
    vlanInArgs.noLearnMask             = 0U;
    vlanInArgs.vidIngressCheck         = BTRUE;
    vlanInArgs.limitIPNxtHdr           = BFALSE;
    vlanInArgs.disallowIPFrag          = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &vlanInArgs, &aleEntry);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_ADD_VLAN, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add ALE VLAN %u entry", vlanId);

    return status;
}

static void EthFwVlan_deleteVlan(Enet_Handle hEnet,
                                 uint16_t vlanId)
{
    CpswAle_VlanIdInfo vlanInArgs;
    Enet_IoctlPrms prms;
    uint32_t coreId;
    int32_t status = ENET_SOK;

    coreId = EnetSoc_getCoreId();

    /* Delete VLAN entry */
    vlanInArgs.vlanId  = vlanId;
    vlanInArgs.tagType = ENET_VLAN_TAG_TYPE_INNER;

    ENET_IOCTL_SET_IN_ARGS(&prms, &vlanInArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_REMOVE_VLAN, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to delete ALE VLAN %u entry", vlanId);
}

