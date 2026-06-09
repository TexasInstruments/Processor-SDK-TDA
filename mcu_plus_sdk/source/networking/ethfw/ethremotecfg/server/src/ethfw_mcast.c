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
 * \file  ethfw_mcast.c
 *
 * \brief This file contains the implementation of multicast helper functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x103

/* Enet LLD header files */
#include <enet.h>
#include <include/per/cpsw.h>

/* EthFw header files */
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <ethremotecfg/server/include/ethfw_virtport.h>
#include "ethfw_mcast_priv.h"
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

/* Shared multicast info */
typedef struct EthFwMcast_SharedMcastInfo_s
{
    /* Multicast address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /* Hardware port mask */
    uint32_t portMask;

    /* Virtual port mask. */
    uint32_t virtPortMask;

    /* Reference count */
    uint32_t refCnt;
} EthFwMcast_SharedMcastInfo;

/* Shared multicast table */
typedef struct EthFwMcast_SharedMcastTable_s
{
    /* Shared multicast info table */
    EthFwMcast_SharedMcastInfo table[ETHFW_SHARED_MCAST_LIST_LEN];

    /* Table length */
    uint32_t len;
} EthFwMcast_SharedMcastTable;

/* Exclusive multicast info */
typedef struct EthFwMcast_ExclusiveMcastInfo_s
{
    /* Multicast address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /* Virtual port */
    uint32_t virtPort;

    /* Reference count */
    uint32_t refCnt;

    /* Whether this entry is free or not */
    bool isFree;
} EthFwMcast_ExclusiveMcastInfo;

/* Exclusive multicast table */
typedef struct EthFwMcast_ExclusiveMcastTable_s
{
    /* Exclusive multicast info table */
    EthFwMcast_ExclusiveMcastInfo table[ETHFW_EXCLUSIVE_MCAST_LIST_LEN];
} EthFwMcast_ExclusiveMcastTable;

/* Reserved multicast table */
typedef struct EthFwMcast_RsvdMcastTable_s
{
    /* Shared multicast info table */
    EthFwMcast_RsvdMcast table[ETHFW_RSVD_MCAST_LIST_LEN];

    /* Table length */
    uint32_t len;
} EthFwMcast_RsvdMcastTable;

/* ETHFW multicast object */
typedef struct EthFwMcast_Obj_s
{
    /* Shared multicast table */
    EthFwMcast_SharedMcastTable sharedMcastTable;

    /* Reserved multicast table */
    EthFwMcast_RsvdMcastTable rsvdMcastTable;

    /* Exclusive multicast table */
    EthFwMcast_ExclusiveMcastTable exclusiveMcastTable;

    /* Callback to notify addition of mcast address to filter */
    EthFwMcast_FilterAddMacSharedCb filterAddMacSharedCb;

    /* Callback to notify removal of mcast address from filter */
    EthFwMcast_FilterDelMacSharedCb filterDelMacSharedCb;

    /* ALE switch port mask */
    uint32_t switchPortMask;

    /* ALE MAC-only port mask */
    uint32_t macOnlyPortMask;
} EthFwMcast_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t EthFwMcast_getSharedMcastCfg(const EthFwMcast_SharedMcastCfg *cfg);

static int32_t EthFwMcast_getRsvdMcastCfg(const EthFwMcast_RsvdMcastCfg *cfg);

static bool EthFwMcast_isRsvdMcast(const uint8_t *macAddr,
                                   uint16_t vlanId);

static EthFwMcast_SharedMcastInfo *EthFwMcast_getSharedMcastInfo(const uint8_t *macAddr,
                                                                 uint16_t vlanId);

static int32_t EthFwMcast_filterAddMacShared(EthRemoteCfg_VirtPort virtPort,
                                             Enet_Handle hEnet,
                                             EthFwMcast_SharedMcastInfo *mcastInfo,
                                             const uint8_t *macAddr,
                                             uint8_t coreId);

static int32_t EthFwMcast_filterDelMacShared(EthRemoteCfg_VirtPort virtPort,
                                             Enet_Handle hEnet,
                                             EthFwMcast_SharedMcastInfo *mcastInfo,
                                             const uint8_t *macAddr,
                                             uint8_t coreId);

static int32_t EthFwMcast_filterAddMacExcl(EthRemoteCfg_VirtPort virtPort,
                                           Enet_Handle hEnet,
                                           const uint8_t *macAddr,
                                           uint32_t flowIdxOffset);

static int32_t EthFwMcast_filterDelMacExcl(EthRemoteCfg_VirtPort virtPort,
                                           Enet_Handle hEnet,
                                           const uint8_t *macAddr);

static EthFwMcast_ExclusiveMcastInfo *EthFwMcast_getExclusiveMcastInfo(const uint8_t *macAddr,
                                                                       uint16_t vlanId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ETHFW multicast object */
static EthFwMcast_Obj gEthFwMcastObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwMcast_init(const EthFwMcast_Cfg *cfg,
                        uint32_t switchPortMask,
                        uint32_t macOnlyPortMask)
{
    int32_t status = ETHFW_EFAIL;
    uint32_t i = 0U;

    memset(&gEthFwMcastObj, 0, sizeof(gEthFwMcastObj));

    /* Get shared multicast configuration */
    status = EthFwMcast_getSharedMcastCfg(&cfg->sharedMcastCfg);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to get shared mcast config");

    /* Get reserved multicast configuration */
    if (status == ETHFW_SOK)
    {
        status = EthFwMcast_getRsvdMcastCfg(&cfg->rsvdMcastCfg);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to get reserved mcast config");
    }

    if (status ==  ETHFW_SOK)
    {
        gEthFwMcastObj.switchPortMask = switchPortMask;
        gEthFwMcastObj.macOnlyPortMask = macOnlyPortMask;
    }

    for (i = 0U; i < ETHFW_EXCLUSIVE_MCAST_LIST_LEN; i++)
    {
        gEthFwMcastObj.exclusiveMcastTable.table[i].isFree = BTRUE;
    }

    return status;
}

void EthFwMcast_deinit(void)
{
    /* Nothing to do here */
}

int32_t EthFwMcast_filterAddMac(EthRemoteCfg_VirtPort virtPort,
                                Enet_Handle hEnet,
                                const uint8_t *macAddr,
                                uint16_t vlanId,
                                uint16_t hwVlanId,
                                uint32_t flowIdxOffset,
                                uint8_t hostId)
{
    EthFwMcast_SharedMcastInfo *mcastInfo;
    EthFwMcast_ExclusiveMcastInfo *exclusiveMcastInfo;
#if defined(ETHFW_VEPA_SUPPORT)
    struct eth_addr hwAddr;
#endif
    bool isRsvd;
    int32_t status = ETHFW_SOK;

    isRsvd = EthFwMcast_isRsvdMcast(macAddr, vlanId);
    if (!isRsvd)
    {
        mcastInfo = EthFwMcast_getSharedMcastInfo(macAddr, vlanId);
        if (mcastInfo != NULL)
        {
            status = EthFwMcast_filterAddMacShared(virtPort, hEnet, mcastInfo,
                                                   macAddr, hostId);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                               "Failed to add shared mcast address");

#if defined(ETHFW_VEPA_SUPPORT)
            if (status == ETHFW_SOK)
            {
                SMEMCPY(&hwAddr, macAddr, ETH_HWADDR_LEN);
                status = EthFwVepa_addAddr(&hwAddr, vlanId, virtPort);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                  "Failed to add shared mcast for core %u into VEPA table", hostId);
            }
#endif
        }
        else
        {
            exclusiveMcastInfo = EthFwMcast_getExclusiveMcastInfo(macAddr, vlanId);
            /* Some virtual port has registered with this exclusive multicast address */
            if (exclusiveMcastInfo != NULL)
            {
                if (exclusiveMcastInfo->virtPort == virtPort)
                {
                    exclusiveMcastInfo->refCnt++;
                }
                /* Exclusive multicast already registered by other client */
                else
                {
                    status = ETHFW_EFAIL;
                    ETHFWTRACE_ERR(status, "Exclusive multicast already registered by other client");
                }
            }
            else
            {
                status = EthFwMcast_filterAddMacExcl(virtPort, hEnet, macAddr, flowIdxOffset);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to add exclusive mcast address");   
            }
        }
    }
    else
    {
        status = ETHFW_EBADARGS;
    }

    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                      "Failed to add %smcast addr on vlan %u and hwVlanId %u %02x:%02x:%02x:%02x:%02x:%02x",
                      isRsvd ? "reserved " : "",
                      vlanId,
                      hwVlanId,
                      macAddr[0U], macAddr[1U], macAddr[2U],
                      macAddr[3U], macAddr[4U], macAddr[5U]);

    return status;
}

int32_t EthFwMcast_filterDelMac(EthRemoteCfg_VirtPort virtPort,
                                Enet_Handle hEnet,
                                uint8_t *macAddr,
                                uint16_t vlanId,
                                uint16_t hwVlanId,
                                uint8_t hostId)
{
    EthFwMcast_SharedMcastInfo *mcastInfo;
    EthFwMcast_ExclusiveMcastInfo *exclusiveMcastInfo;
#if defined(ETHFW_VEPA_SUPPORT)
    struct eth_addr hwAddr;
#endif
    bool isRsvd;
    int32_t status = ETHFW_SOK;

    isRsvd = EthFwMcast_isRsvdMcast(macAddr, vlanId);
    if (!isRsvd)
    {
        mcastInfo = EthFwMcast_getSharedMcastInfo(macAddr, vlanId);
        if (mcastInfo != NULL)
        {
#if defined(ETHFW_VEPA_SUPPORT)
            SMEMCPY(&hwAddr, mcastInfo->macAddr, ETH_HWADDR_LEN);

            status = EthFwVepa_delAddr(&hwAddr, vlanId, virtPort);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                              "Failed to delete shared mcast for core %u from VEPA table", hostId);
#endif
            if (status == ETHFW_SOK)
            {
                status = EthFwMcast_filterDelMacShared(virtPort, hEnet, mcastInfo,
                                                       macAddr, hostId);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                   "Failed to delete shared mcast address");
            }

        }
        else
        {
            exclusiveMcastInfo = EthFwMcast_getExclusiveMcastInfo(macAddr, vlanId);
            /* Some virtual port has registered with this exclusive multicast address */
            if ((exclusiveMcastInfo != NULL) && 
                (exclusiveMcastInfo->virtPort == virtPort))
            {
                if (exclusiveMcastInfo->refCnt > 1U)
                {
                    exclusiveMcastInfo->refCnt--;
                }
                else if (exclusiveMcastInfo->refCnt == 1U)
                {
                    exclusiveMcastInfo->refCnt--;
                    status = EthFwMcast_filterDelMacExcl(virtPort, hEnet, macAddr);
                    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                    "Failed to delete exclusive mcast address");
                    if (status == ETHFW_SOK)
                    {
                        exclusiveMcastInfo->isFree = BTRUE;
                        exclusiveMcastInfo->refCnt = 0U;
                    }
                    else
                    {
                        /* Failed to delete exclusive multicast address, hence putting back the ref count to 1 */
                        exclusiveMcastInfo->refCnt = 1U;
                    }
                }
                else
                {
                    ETHFWTRACE_ERR(status, "Negative reference count failed to delete exclusive mcast address");
                }
            }
            else
            {
                status = ETHFW_EFAIL;
                ETHFWTRACE_ERR(status, "Failed to delete exclusive mcast address, not found");
            }
        }
    }
    else
    {
        status = ETHFW_EBADARGS;
    }

    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                      "Failed to delete %smcast addr %02x:%02x:%02x:%02x:%02x:%02x",
                      isRsvd ? "reserved " : "",
                      macAddr[0U], macAddr[1U], macAddr[2U],
                      macAddr[3U], macAddr[4U], macAddr[5U]);

    return status;
}

static int32_t EthFwMcast_getSharedMcastCfg(const EthFwMcast_SharedMcastCfg *cfg)
{
    EthFwMcast_SharedMcastInfo *entry;
    const EthFwMcast_McastCfg *mcastCfg = NULL;
    const uint8_t *macAddr;
    uint32_t i;
    int32_t status = ETHFW_SOK;

    if (cfg->numMcast > ETHFW_SHARED_MCAST_LIST_LEN)
    {
        status = ETHFW_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Invalid number of multicast addresses (%u), must be <= %u",
                       cfg->numMcast, ETHFW_SHARED_MCAST_LIST_LEN);
    }

    if ((status == ETHFW_SOK) &&
        (cfg->numMcast > 0U))
    {
        ETHFWTRACE_INFO("Shared multicasts:");

        gEthFwMcastObj.sharedMcastTable.len = 0U;

        for (i = 0U; i < cfg->numMcast; i++)
        {
            entry = &gEthFwMcastObj.sharedMcastTable.table[i];
            mcastCfg = &cfg->mcastCfg[i];
            macAddr = &mcastCfg->macAddr[0U];

            if (EnetUtils_isMcastAddr(macAddr))
            {
                ETHFWTRACE_INFO("  %02x:%02x:%02x:%02x:%02x:%02x",
                                macAddr[0U], macAddr[1U], macAddr[2U],
                                macAddr[3U], macAddr[4U], macAddr[5U]);

                EnetUtils_copyMacAddr(&entry->macAddr[0U], macAddr);
                entry->portMask     = mcastCfg->portMask;
                entry->virtPortMask = mcastCfg->virtPortMask;
                entry->refCnt       = 0U;

                gEthFwMcastObj.sharedMcastTable.len++;
            }
            else
            {
                status = ETHFW_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "Addr %02x:%02x:%02x:%02x:%02x:%02x is not mcast",
                               macAddr[0U], macAddr[1U], macAddr[2U],
                               macAddr[3U], macAddr[4U], macAddr[5U]);
                break;
            }
        }
    }

    if (status == ETHFW_SOK)
    {
        gEthFwMcastObj.filterAddMacSharedCb = cfg->filterAddMacSharedCb;
        gEthFwMcastObj.filterDelMacSharedCb = cfg->filterDelMacSharedCb;
    }
    else
    {
        gEthFwMcastObj.sharedMcastTable.len = 0U;
    }

    return status;
}

static int32_t EthFwMcast_getRsvdMcastCfg(const EthFwMcast_RsvdMcastCfg *cfg)
{
    EthFwMcast_RsvdMcast *entry;
    const EthFwMcast_RsvdMcast *mcastCfg = NULL;
    const uint8_t *macAddr;
    uint32_t i;
    int32_t status = ETHFW_SOK;

    if (cfg->numMcast > ETHFW_RSVD_MCAST_LIST_LEN)
    {
        status = ETHFW_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Invalid number of multicast addresses (%u), must be <= %u",
                       cfg->numMcast, ETHFW_RSVD_MCAST_LIST_LEN);
    }

    if ((status == ETHFW_SOK) &&
        (cfg->numMcast > 0U))
    {
        ETHFWTRACE_INFO("Reserved multicasts:");

        gEthFwMcastObj.rsvdMcastTable.len = 0U;

        for (i = 0U; i < cfg->numMcast; i++)
        {
            entry = &gEthFwMcastObj.rsvdMcastTable.table[i];
            mcastCfg = &cfg->mcastCfg[i];
            macAddr = &mcastCfg->macAddr[0U];

            if (EnetUtils_isMcastAddr(macAddr))
            {
                ETHFWTRACE_INFO("  %02x:%02x:%02x:%02x:%02x:%02x",
                                macAddr[0U], macAddr[1U], macAddr[2U],
                                macAddr[3U], macAddr[4U], macAddr[5U]);

                EnetUtils_copyMacAddr(&entry->macAddr[0U], macAddr);
                entry->vlanId   = mcastCfg->vlanId;
                entry->portMask = mcastCfg->portMask;

                gEthFwMcastObj.rsvdMcastTable.len++;
            }
            else
            {
                status = ETHFW_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "Addr %02x:%02x:%02x:%02x:%02x:%02x is not mcast",
                               macAddr[0U], macAddr[1U], macAddr[2U],
                               macAddr[3U], macAddr[4U], macAddr[5U]);
                break;
            }
        }
    }

    if (status != ETHFW_SOK)
    {
        gEthFwMcastObj.rsvdMcastTable.len = 0U;
    }

    return status;
}

static bool EthFwMcast_isRsvdMcast(const uint8_t *macAddr,
                                   uint16_t vlanId)
{
    EthFwMcast_RsvdMcast *entry = NULL;
    bool isRsvd = BFALSE;
    uint32_t i;

    for (i = 0U; i < gEthFwMcastObj.rsvdMcastTable.len; i++)
    {
        entry = &gEthFwMcastObj.rsvdMcastTable.table[i];

        if ((entry->vlanId == vlanId) &&
            EnetUtils_cmpMacAddr(&entry->macAddr[0U], macAddr))
        {
            isRsvd = BTRUE;
            break;
        }
    }

    return isRsvd;
}

static EthFwMcast_SharedMcastInfo *EthFwMcast_getSharedMcastInfo(const uint8_t *macAddr,
                                                                 uint16_t vlanId)
{
    EthFwMcast_SharedMcastInfo *entry = NULL;
    bool found = BFALSE;
    uint32_t i;

    for (i = 0U; i < gEthFwMcastObj.sharedMcastTable.len; i++)
    {
        entry = &gEthFwMcastObj.sharedMcastTable.table[i];

        if (EnetUtils_cmpMacAddr(&entry->macAddr[0U], macAddr))
        {
            found = BTRUE;
            break;
        }
    }

    return (found ? entry : NULL);
}

static EthFwMcast_ExclusiveMcastInfo *EthFwMcast_getExclusiveMcastInfo(const uint8_t *macAddr,
                                                                       uint16_t vlanId)
{
    EthFwMcast_ExclusiveMcastInfo *entry = NULL;
    bool found = BFALSE;
    uint32_t i;

    for (i = 0U; i < ETHFW_EXCLUSIVE_MCAST_LIST_LEN; i++)
    {
        entry = &gEthFwMcastObj.exclusiveMcastTable.table[i];

        if ((entry->isFree == BFALSE) && 
            (EnetUtils_cmpMacAddr(&entry->macAddr[0U], macAddr)))
        {
            found = BTRUE;
            break;
        }
    }

    return (found ? entry : NULL);
}

static int32_t EthFwMcast_filterAddMacShared(EthRemoteCfg_VirtPort virtPort,
                                             Enet_Handle hEnet,
                                             EthFwMcast_SharedMcastInfo *mcastInfo,
                                             const uint8_t *macAddr,
                                             uint8_t coreId)
{
    Enet_IoctlPrms prms;
    CpswAle_SetMcastEntryInArgs mcastInArgs;
    uint32_t aleEntry;
    int32_t status = ETHFW_SOK;

    /* Add multicast entry in ALE table */
    if (mcastInfo->refCnt == 0U)
    {
        mcastInArgs.addr.vlanId = 0U;
        EnetUtils_copyMacAddr(&mcastInArgs.addr.addr[0], macAddr);
        mcastInArgs.info.super    = BFALSE;
        mcastInArgs.info.fwdState = CPSW_ALE_FWDSTLVL_FWD;
        mcastInArgs.info.portMask = mcastInfo->portMask;
        mcastInArgs.info.numIgnBits = 0U;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &mcastInArgs, &aleEntry);

        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_ADD_MCAST, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add shared mcast entry in ALE");

#if defined(ETHFW_VEPA_SUPPORT)
        if (status == ETHFW_SOK)
        {
            /* Multicast address (irrespective of VLAN) should come to pkt duplication flow */
            status = EthFwVepa_addMcastPolicer(hEnet, coreId,
                                               0U, macAddr,
                                               CPSW_ALE_POLICER_PARTITION_DEFAULT);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                               "Failed to add policer for shared mcast for core %u for packet duplication", coreId);
        }
#endif
    }

    /* Increase reference counter and call 'add' callback */
    if (status == ETHFW_SOK)
    {
        mcastInfo->refCnt++;
        if (gEthFwMcastObj.filterAddMacSharedCb != NULL)
        {
            gEthFwMcastObj.filterAddMacSharedCb(macAddr, coreId);
        }
    }

    return status;
}

static int32_t EthFwMcast_filterDelMacShared(EthRemoteCfg_VirtPort virtPort,
                                             Enet_Handle hEnet,
                                             EthFwMcast_SharedMcastInfo *mcastInfo,
                                             const uint8_t *macAddr,
                                             uint8_t coreId)
{
    Enet_IoctlPrms prms;
    CpswAle_MacAddrInfo macAddrInfo;
    int32_t status = ETHFW_SOK;
#if defined(ETHFW_VEPA_SUPPORT)
    /* ALE entry mask == 0 means do not delete any ale entry, just delete the policer entry */
    uint32_t aleEntryMask = 0U;
#endif

    /* Remove mcast address from ALE table */
    if (mcastInfo->refCnt == 1U)
    {
#if defined(ETHFW_VEPA_SUPPORT)        
        /* Multicast address (irrespective of VLAN) should not come to pkt duplication flow */
        status = EthFwVepa_delMcastPolicer(hEnet, coreId,
                                            0U, macAddr,
                                            aleEntryMask);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                            "Failed to remove policer for shared mcast for core %u for packet duplication", coreId);
#endif
        if (status == ETHFW_SOK)
        {
            EnetUtils_copyMacAddr(&macAddrInfo.addr[0U], macAddr);
            /* Multicast address without VLAN */
            macAddrInfo.vlanId = 0U;

            ENET_IOCTL_SET_IN_ARGS(&prms, &macAddrInfo);

            ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_REMOVE_ADDR, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to remove shared mcast entry in ALE");
        }
    }
    else if (mcastInfo->refCnt == 0U)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Shared multicast address not in use, cannot be deleted");
    }

    if (status == ETHFW_SOK)
    {
        mcastInfo->refCnt--;
        if (gEthFwMcastObj.filterDelMacSharedCb != NULL)
        {
            gEthFwMcastObj.filterDelMacSharedCb(macAddr, coreId);
        }
    }

    return status;
}

static int32_t EthFwMcast_filterAddMacExcl(EthRemoteCfg_VirtPort virtPort,
                                           Enet_Handle hEnet,
                                           const uint8_t *macAddr,
                                           uint32_t flowIdxOffset)
{
    Enet_IoctlPrms prms;
    CpswAle_GetMcastEntryInArgs lookupInArgs;
    CpswAle_GetMcastEntryOutArgs lookupOutArgs;
    CpswAle_SetMcastEntryInArgs mcastInArgs;
    CpswAle_SetPolicerEntryInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_MacPort macPort = EthFwVirtPort_getMacPort(virtPort);
    bool isMacPort = EthFwVirtPort_isMacPort(virtPort);
    uint8_t coreId = EnetSoc_getCoreId();
    uint32_t aleSwitchPortMask = gEthFwMcastObj.switchPortMask << 1U;
    uint32_t aleEntry;
    int32_t status = ETHFW_EFAIL;
    uint32_t i = 0U;

    /* Lookup for multicast address (irrespective of VLAN) */
    lookupInArgs.addr.vlanId = 0U;
    EnetUtils_copyMacAddr(&lookupInArgs.addr.addr[0], macAddr);
    lookupInArgs.numIgnBits = 0U;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &lookupInArgs, &lookupOutArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_LOOKUP_MCAST, &prms, status);
    if (status == ENET_SOK)
    {
        /* Multicast address already requested, with requestor having exclusive access.
         * Reject new caller */
        status = ETHFW_EBUSY;
        ETHFWTRACE_ERR(status, "Exclusive mcast already owned by another client");
    }
    else if (status == ENET_ENOTFOUND)
    {
        /* No one has requested this multicast address, caller gets it */
        status = ETHFW_SOK;
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to lookup mcast entry");
    }

    if (status == ETHFW_SOK)
    {
        for (i = 0U; i < ETHFW_EXCLUSIVE_MCAST_LIST_LEN; i++)
        {
            if(gEthFwMcastObj.exclusiveMcastTable.table[i].isFree)
            {
                gEthFwMcastObj.exclusiveMcastTable.table[i].isFree = BFALSE;
                gEthFwMcastObj.exclusiveMcastTable.table[i].refCnt = 1;
                gEthFwMcastObj.exclusiveMcastTable.table[i].virtPort = virtPort;
                EnetUtils_copyMacAddr(&gEthFwMcastObj.exclusiveMcastTable.table[i].macAddr[0U], macAddr);
                break;
            }
        }
        /* Exclusive multicast table full, increase the size of ETHFW_EXCLUSIVE_MCAST_LIST_LEN */
        if (i == ETHFW_EXCLUSIVE_MCAST_LIST_LEN)
        {
            status = ETHFW_EALLOC;
            ETHFWTRACE_ERR(status, "Error, exclusive multicast table full");
        }
    }

    /* Add multicast entry in ALE */
    if (status == ETHFW_SOK)
    {
        mcastInArgs.addr.vlanId = 0U;
        EnetUtils_copyMacAddr(&mcastInArgs.addr.addr[0], macAddr);

        mcastInArgs.info.super    = BFALSE;
        mcastInArgs.info.fwdState = CPSW_ALE_FWDSTLVL_FWD;
        mcastInArgs.info.portMask = CPSW_ALE_HOST_PORT_MASK;
        mcastInArgs.info.numIgnBits = 0U;

        if (isMacPort)
        {
            mcastInArgs.info.portMask |= CPSW_ALE_MACPORT_TO_PORTMASK(macPort);
        }
        else
        {
            mcastInArgs.info.portMask |= aleSwitchPortMask;
        }

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &mcastInArgs, &aleEntry);

        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_ADD_MCAST, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add mcast entry");
    }

    /* Setup classifier */
    if (status == ETHFW_SOK)
    {
        polInArgs.policerMatch.policerMatchEnMask = 0U;

        /* Multicast MAC address as match criteria */
        polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
        polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
        polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
        EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], macAddr);

        /* Ingress port as match criteria to fully qualify the classifier for
         * virtual MAC ports as directed packets will be used and could hit i2148 errata */
        if (isMacPort)
        {
            polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_PORT;
            polInArgs.policerMatch.portNum = CPSW_ALE_MACPORT_TO_ALEPORT(macPort);
            polInArgs.policerMatch.portIsTrunk = BFALSE;
        }

        polInArgs.threadIdEn = BTRUE;
        polInArgs.threadId   = flowIdxOffset;
        polInArgs.peakRateInBitsPerSec   = 0U;
        polInArgs.commitRateInBitsPerSec = 0U;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to setup ALE classifier");
    }

    return status;
}

static int32_t EthFwMcast_filterDelMacExcl(EthRemoteCfg_VirtPort virtPort,
                                           Enet_Handle hEnet,
                                           const uint8_t *macAddr)
{
    Enet_IoctlPrms prms;
    CpswAle_DelPolicerEntryInArgs polInArgs;
    Enet_MacPort macPort = EthFwVirtPort_getMacPort(virtPort);
    bool isMacPort = EthFwVirtPort_isMacPort(virtPort);
    uint8_t coreId = EnetSoc_getCoreId();
    int32_t status = ETHFW_EFAIL;

    polInArgs.policerMatch.policerMatchEnMask = 0U;

    /* Multicast MAC address as match criteria */
    polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
    polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
    polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
    EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], macAddr);

    /* Ingress port as match criteria to fully qualify the classifier for
     * virtual MAC ports as directed packets will be used and could hit i2148 errata */
    if (isMacPort)
    {
        polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_PORT;
        polInArgs.policerMatch.portNum = CPSW_ALE_MACPORT_TO_ALEPORT(macPort);
        polInArgs.policerMatch.portIsTrunk = BFALSE;
    }

    polInArgs.aleEntryMask = CPSW_ALE_POLICER_TABLEENTRY_DELETE_ALL;

    ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to delete ALE classifier");

    return status;
}

void EthFwMcast_printTable(void)
{
    EthFwMcast_SharedMcastInfo *entry;
    uint32_t i;

    ETHFWTRACE_INFO("");
    ETHFWTRACE_INFO(" SNo.      MAC Address     Port Mask   Virt Port Mask  RefCnt");
    ETHFWTRACE_INFO("------  -----------------  ----------  --------------  ------");

    for (i = 0U; i <= gEthFwMcastObj.sharedMcastTable.len; i++)
    {
        entry = &gEthFwMcastObj.sharedMcastTable.table[i];
        ETHFWTRACE_INFO(" %3d    %02x:%02x:%02x:%02x:%02x:%02x      0x%04x       0x%04x         %d",
                        i + 1,
                        entry->macAddr[0], entry->macAddr[1], entry->macAddr[2],
                        entry->macAddr[3], entry->macAddr[4], entry->macAddr[5],
                        entry->portMask, entry->virtPortMask, entry->refCnt);
    }
}
