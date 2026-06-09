/*
 *
 * Copyright (c) 2023-2024 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*!
 * \file ethfw_vepa_utils.c
 *
 * \brief VEPA utils functions for Ethernet Firmware.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x104

/* Enet LLD header files */
#include <enet.h>

/* EthFw header files */
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include "ethfw_vepa_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of entries in VEPA table */
#define ETHFW_VEPA_TABLE_SIZE                          (32U)

/* Private VLAN id to use when not provided by the application */
#define ETHFW_VEPA_PRIVATE_VLAN_ID_START               (2000U)

/* Table entry private VLAN id and MAC address for remote cores. */
typedef struct EthFwVepa_AddrEntry_s
{
    /* Mask of which all virtual switch ports needs to send the packet */
    uint32_t virtPortMask;

    /* Multicast(shared)/broadcast address */
    struct eth_addr hwAddr;

    /* Inner VLAN */
    uint16_t vlanId;

    /* Whether this entry is free or not */
    bool isFree;
} EthFwVepa_AddrEntry;

/* Vepa Utils object. */
typedef struct EthFwVepa_Obj_s
{
    /* Vepa table */
    EthFwVepa_AddrEntry remoteCoreVepaTable[ETHFW_VEPA_TABLE_SIZE];

    /* Table to store private VLAN for each virtual switch port */
    uint16_t privVlanCfg[ETHREMOTECFG_SWITCH_PORT_LAST + 1];

    /* MAC address associated with virtual switch port */
    struct eth_addr virtPortToMacAddr[ETHREMOTECFG_SWITCH_PORT_LAST + 1];

    /* Mutex handle. Used to protect Vepa table from concurrent accesses */
    EthFwOsal_MutexHandle hMutex;

    /* Flow idx where packets to be duplicated arrive at */
    uint32_t packetDuplicationFlowIdx;
} EthFwVepa_Obj;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t EthFwVepa_getVirtPortMask(struct eth_addr *hwAddr,
                                         uint16_t vlanId,
                                         uint32_t *virtPortMask);



static int32_t EthFwVepa_getPrivateVlanId(EthRemoteCfg_VirtPort virtPort,
                                          uint16_t *privVlanId);

static EthFwVepa_AddrEntry *EthFwVepa_getMcastInfo(struct eth_addr *hwAddr,
                                                   uint16_t vlanId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EthFwVepa_Obj gEthFwVepaObj;
struct eth_addr gEthFwBcastAddr = {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwVepa_initCfg(EthFwVepa_Cfg *vepaCfg)
{
    uint32_t vlanId = ETHFW_VEPA_PRIVATE_VLAN_ID_START;
    uint32_t i;

    for (i = 0U; i <= ETHREMOTECFG_SWITCH_PORT_LAST; i++)
    {
        /* Default private VLAN ids */
        vepaCfg->privVlanId[i] = vlanId;
        vlanId += 1U;
    }
}

int32_t EthFwVepa_init(const EthFwVepa_Cfg *vepaCfg)
{
    int32_t status = ETHFW_SOK;
    uint32_t i;

    /* Create mutex to protect VEPA table */
    gEthFwVepaObj.hMutex = EthFwOsal_createMutex();
    if (gEthFwVepaObj.hMutex == NULL)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to create mutex");
    }

    /* Mark all entries in VEPA table as free */
    if (status == ETHFW_SOK)
    {
        for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
        {
            gEthFwVepaObj.remoteCoreVepaTable[i].isFree = BTRUE;
        }

        for (i = 0U; i <= ETHREMOTECFG_SWITCH_PORT_LAST; i++)
        {
            /* Private VLAN id is passed from application */
            if (vepaCfg->privVlanId[i] != 0U)
            {
                gEthFwVepaObj.privVlanCfg[i] = vepaCfg->privVlanId[i];
            }
        }

        /* Initialized packet duplication flow to be un-defined */
        gEthFwVepaObj.packetDuplicationFlowIdx = ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED;
    }

    return status;
}

void EthFwVepa_deinit(void)
{
    if (gEthFwVepaObj.hMutex != NULL)
    {
        EthFwVepa_flushTable();
        EthFwOsal_deleteMutex(gEthFwVepaObj.hMutex);
        gEthFwVepaObj.hMutex = NULL;
    }
    else
    {
        ETHFWTRACE_ERR(ETHFW_EFAIL, "Failed to de-init Vepa");
    }
}

uint32_t EthFwVepa_setPacketDuplicationFlowIdx(uint32_t flowIdx)
{
    int32_t status = ETHFW_EFAIL;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    if (gEthFwVepaObj.packetDuplicationFlowIdx == ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED)
    {
        gEthFwVepaObj.packetDuplicationFlowIdx = flowIdx;
        status = ETHFW_SOK;
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);

    return status;
}

void EthFwVepa_clearPacketDuplicationFlowIdx(void)
{
    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    if (gEthFwVepaObj.packetDuplicationFlowIdx != ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED)
    {
        gEthFwVepaObj.packetDuplicationFlowIdx = ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED;
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);
}

/* Given a multicast/broadcast address, it returns virtPortMask
 * of clients to which packet will be forwarded */
static int32_t EthFwVepa_getVirtPortMask(struct eth_addr *hwAddr,
                                         uint16_t vlanId,
                                         uint32_t *virtPortMask)
{
    EthFwVepa_AddrEntry *entry;
    int32_t status = ETHFW_EFAIL;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    /* Check if VEPA table has a corresponding entry */
    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        entry = &gEthFwVepaObj.remoteCoreVepaTable[i];
        /* Check if MAC address matches and VLAN id matches */
        if (!entry->isFree && 
            (entry->vlanId == vlanId) &&
            EnetUtils_cmpMacAddr(entry->hwAddr.addr, hwAddr->addr))
        {
            /* Get the port mask */
            *virtPortMask = entry->virtPortMask;
            status = ETHFW_SOK;
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);

    return status;
}

/* Returns the private VLAN Id of the given host */
static int32_t EthFwVepa_getPrivateVlanId(EthRemoteCfg_VirtPort virtPort,
                                          uint16_t *privVlanId)
{
    int32_t status = ETHFW_EFAIL;

    /* Virtual port passed is valid and client has registered */
    if ((virtPort <= ETHREMOTECFG_SWITCH_PORT_LAST) &&
        (gEthFwVepaObj.privVlanCfg[virtPort] != 0U))
    {
        *privVlanId = gEthFwVepaObj.privVlanCfg[virtPort];
        status = ETHFW_SOK;
    }

    return status;
}

/* Returns the corresponding entry from VEPA table */
static EthFwVepa_AddrEntry *EthFwVepa_getMcastInfo(struct eth_addr *hwAddr,
                                                   uint16_t vlanId)
{
    EthFwVepa_AddrEntry *entry;
    bool found = BFALSE;
    uint32_t i;

    /* Check if VEPA table already has a corresponding entry */
    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        entry = &gEthFwVepaObj.remoteCoreVepaTable[i];

        /* Check if MAC address matches and VLAN id matches */
        if (!entry->isFree &&
            (entry->vlanId == vlanId) &&
            EnetUtils_cmpMacAddr(entry->hwAddr.addr, hwAddr->addr))
        {
            found = BTRUE;
            break;
        }
    }

    return found ? entry : NULL;;
}

int32_t EthFwVepa_addMcastPolicer(Enet_Handle hEnet,
                                  uint32_t coreId,
                                  uint32_t vlanId,
                                  const uint8_t *mcastAddr,
                                  CpswAle_PolicerPartLevel policerPartLevel)
{
    CpswAle_SetPolicerEntryInPartitionInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_IoctlPrms prms;
    int32_t status = ETHFW_SOK;

    if (gEthFwVepaObj.packetDuplicationFlowIdx == ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to get flow index of packet duplication flow");
    }

    if (status == ETHFW_SOK)
    {
        polInArgs.policerMatch.policerMatchEnMask         = CPSW_ALE_POLICER_MATCH_MACDST;
        polInArgs.policerMatch.dstMacAddrInfo.portNum     = CPSW_ALE_HOST_PORT_NUM;
        polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = vlanId;
        polInArgs.policerMatch.portIsTrunk                = BFALSE;
        polInArgs.threadIdEn                              = BTRUE;
        polInArgs.threadId                                = gEthFwVepaObj.packetDuplicationFlowIdx;
        polInArgs.peakRateInBitsPerSec                    = 0U;
        polInArgs.commitRateInBitsPerSec                  = 0U;
        polInArgs.policerPartLevel                        = policerPartLevel;
        EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], mcastAddr);

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        status = Enet_ioctl(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER_IN_PARTITION, &prms);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                        "Failed to add policer for mcast addr in partition %u", (uint32_t)policerPartLevel);
    }

    return status;
}

int32_t EthFwVepa_delMcastPolicer(Enet_Handle hEnet,
                                  uint32_t coreId,
                                  uint32_t vlanId,
                                  const uint8_t *mcastAddr,
                                  uint32_t aleEntryMask)
{
    CpswAle_DelPolicerEntryInArgs polInArgs;
    Enet_IoctlPrms prms;
    int32_t status = ETHFW_EFAIL;

    polInArgs.policerMatch.policerMatchEnMask         = CPSW_ALE_POLICER_MATCH_MACDST;
    polInArgs.policerMatch.dstMacAddrInfo.portNum     = CPSW_ALE_HOST_PORT_NUM;
    polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = vlanId;
    polInArgs.policerMatch.portIsTrunk                = BFALSE;
    polInArgs.aleEntryMask                            = aleEntryMask;
    EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], mcastAddr);

    ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

    /* Delete ALE policer */
    status = Enet_ioctl(hEnet, coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to delete policer for mcast addr");

    return status;
}

int32_t EthFwVepa_addAddr(struct eth_addr *hwAddr,
                          uint16_t vlanId,
                          EthRemoteCfg_VirtPort virtPort)
{
    EthFwVepa_AddrEntry *entry;
    CpswAle_PolicerPartLevel policerPartLevel = CPSW_ALE_POLICER_PARTITION_DEFAULT;
    int32_t status = ETHFW_SOK;
    uint32_t i;
    bool done = BFALSE;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    if (status == ETHFW_SOK)
    {
        /* Check if VEPA table already has a corresponding entry */
        for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
        {
            entry = &gEthFwVepaObj.remoteCoreVepaTable[i];

            /* Check if MAC address matches and VLAN id matches */
            if (!entry->isFree &&
                (entry->vlanId == vlanId) &&
                EnetUtils_cmpMacAddr(entry->hwAddr.addr, hwAddr->addr))
            {
                /* Update the port mask */
                entry->virtPortMask |= ETHFW_BIT(virtPort);
                done = BTRUE;
                status = ETHFW_SOK;
                break;
            }
        }
    }

    /* Add new entry in the table and add policer entry as well */
    if (!done &&
        (status == ETHFW_SOK))
    {
        for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
        {
            entry = &gEthFwVepaObj.remoteCoreVepaTable[i];

            /* Take the first free entry */
            if (entry->isFree)
            {
                if (status == ETHFW_SOK)
                {
                    SMEMCPY(&entry->hwAddr, hwAddr, ETH_HWADDR_LEN);
                    entry->vlanId = vlanId;
                    entry->virtPortMask = ETHFW_BIT(virtPort);
                    entry->isFree = BFALSE;
                }
                else
                {
                    ETHFWTRACE_ERR(status, "Failed to add policer for packet duplication");
                }
                break;
            }
        }
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);

    return status;
}

int32_t EthFwVepa_delAddr(struct eth_addr *hwAddr,
                          uint16_t vlanId,
                          EthRemoteCfg_VirtPort virtPort)
{
    EthFwVepa_AddrEntry *entry;
    /* ALE entry mask == 0 means do not delete any ale entry, just delete the policer entry */
    uint32_t aleEntryMask = 0U;
    int32_t status = ETHFW_EFAIL;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    /* Check if VEPA table has a corresponding entry */
    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        entry = &gEthFwVepaObj.remoteCoreVepaTable[i];

        /* Check if MAC address matches and VLAN id matches */
        if (!entry->isFree &&
            (entry->vlanId == vlanId) &&
            EnetUtils_cmpMacAddr(entry->hwAddr.addr, hwAddr->addr))
        {
            /* Update the port mask */
            entry->virtPortMask &= ~ETHFW_BIT(virtPort);
            status = ETHFW_SOK;

            /* No virtual switch port needs this multicast address, free the entry and
             * remove the policer */
            if (entry->virtPortMask == 0U)
            {
                if (status == ETHFW_SOK)
                {
                    entry->isFree = BTRUE;
                    entry->vlanId = 0U;
                    memset(&entry->hwAddr, 0U, sizeof(struct eth_addr));
                }
                else
                {
                    ETHFWTRACE_ERR(status, "Failed to delete policer for packet duplication");
                }
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);

    return status;
}

void EthFwVepa_printTable(void)
{
    EthFwVepa_AddrEntry *entry;
    uint32_t used = 0U;
    uint32_t i;

    ETHFWTRACE_INFO("");
    ETHFWTRACE_INFO(" SNo.   Virtport Id    Private VLAN       MAC Address    ");
    ETHFWTRACE_INFO("------  -----------    ------------    ------------------");
    for (i = 0U; i <= ETHREMOTECFG_SWITCH_PORT_LAST; i++)
    {
        if (gEthFwVepaObj.privVlanCfg[i] != 0U)
        {
            ETHFWTRACE_INFO("  %d        %d           %d        %02x:%02x:%02x:%02x:%02x:%02x",
                            ++used,
                            i,
                            gEthFwVepaObj.privVlanCfg[i],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[0],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[1],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[2],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[3],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[4],
                            gEthFwVepaObj.virtPortToMacAddr[i].addr[5]);
        }
    }

    used = 0U;
    ETHFWTRACE_INFO("");
    ETHFWTRACE_INFO(" SNo.    Port Mask       VLAN id         MAC Address   ");
    ETHFWTRACE_INFO("------   ---------    -------------   -----------------");

    /* Check if VEPA table already has a corresponding entry */
    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        entry = &gEthFwVepaObj.remoteCoreVepaTable[i];

        /* Check if MAC address matches and VLAN id matches */
        if (!entry->isFree)
        {
            ETHFWTRACE_INFO("  %d        0x%04x            %d          %02x:%02x:%02x:%02x:%02x:%02x",
                            ++used,
                            entry->virtPortMask,
                            entry->vlanId,
                            entry->hwAddr.addr[0] & 0xFFU, entry->hwAddr.addr[1] & 0xFFU,
                            entry->hwAddr.addr[2] & 0xFFU, entry->hwAddr.addr[3] & 0xFFU,
                            entry->hwAddr.addr[4] & 0xFFU, entry->hwAddr.addr[5] & 0xFFU);
        }
    }
}

void EthFwVepa_flushTable(void)
{
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    /* Removes all entries from the VEPA table */
    memset(gEthFwVepaObj.remoteCoreVepaTable, 0U, sizeof(gEthFwVepaObj.remoteCoreVepaTable));
    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        gEthFwVepaObj.remoteCoreVepaTable[i].isFree = BTRUE;
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);
}

int32_t EthFwVepa_registerClient(Enet_Handle hEnet,
                                 uint32_t coreId,
                                 uint32_t flowIdx,
                                 uint16_t vlanId,
                                 EthRemoteCfg_VirtPort virtPort,
                                 struct eth_addr *srcAddr)
{
    Enet_IoctlPrms prms;
    CpswAle_VlanEntryInfo inArgs;
    CpswAle_SetPolicerEntryInPartitionInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    uint32_t entryIdx;
    uint32_t privVlanId;
    int32_t status = ETHFW_EFAIL;
    CpswAle_PolicerPartLevel policerPartLevel = CPSW_ALE_POLICER_PARTITION_DEFAULT;
    EthFwVepa_AddrEntry *mcastInfo;

    privVlanId = gEthFwVepaObj.privVlanCfg[virtPort];

    /* ALE entry for VLAN used for packet forwarding from ETHFW to Remote Client */
    inArgs.vlanIdInfo.vlanId       = privVlanId;
    inArgs.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
    inArgs.vlanMemberList          = CPSW_ALE_HOST_PORT_MASK;
    inArgs.unregMcastFloodMask     = 0U;
    inArgs.regMcastFloodMask       = CPSW_ALE_HOST_PORT_MASK;
    inArgs.forceUntaggedEgressMask = CPSW_ALE_HOST_PORT_MASK;
    inArgs.noLearnMask             = CPSW_ALE_HOST_PORT_MASK;
    inArgs.vidIngressCheck         = BFALSE;
    inArgs.limitIPNxtHdr           = BFALSE;
    inArgs.disallowIPFrag          = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &entryIdx);

    status = Enet_ioctl(hEnet, coreId, CPSW_ALE_IOCTL_ADD_VLAN, &prms);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add priv VLAN %u in ALE", privVlanId);

    if (status == ETHFW_SOK)
    {
        /* ALE policer for VLAN used for packet forwarding from ETHFW to Remote Client */
        polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_IVLAN;
        polInArgs.policerMatch.ivlanId            = privVlanId;
        polInArgs.threadIdEn                      = BTRUE;
        polInArgs.threadId                        = flowIdx;
        polInArgs.peakRateInBitsPerSec            = 0U;
        polInArgs.commitRateInBitsPerSec          = 0U;
        polInArgs.policerPartLevel                = CPSW_ALE_POLICER_PARTITION_LEVEL_1;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        status = Enet_ioctl(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER_IN_PARTITION, &prms);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to add policer for priv VLAN %u in partition %u",
                          privVlanId, (uint32_t)polInArgs.policerPartLevel);
    }

    if (status == ETHFW_SOK)
    {
        /* Set MAC address for the registered virtual switch port */
        SMEMCPY(&gEthFwVepaObj.virtPortToMacAddr[virtPort], srcAddr, ETH_HWADDR_LEN);

        /* If no client has registered before */
        mcastInfo = EthFwVepa_getMcastInfo(&gEthFwBcastAddr, 0U); 
        if(mcastInfo == NULL)
        {
            /* Add policer: All broadcast packet (irrespective of vlan) should go to packet duplication flow
             * at the same time reaches all ports (including host port) */
            status = EthFwVepa_addMcastPolicer(hEnet, coreId,
                                               0U, gEthFwBcastAddr.addr,
                                               policerPartLevel);
        }

        /* Add broadcast entry in VEPA table */
        status = EthFwVepa_addAddr(&gEthFwBcastAddr, 0U, virtPort);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                          "Failed to add bcast addr for VLAN %u into VEPA table", vlanId);
    }

    return status;
}

int32_t EthFwVepa_unregisterClient(Enet_Handle hEnet,
                                   uint32_t coreId,
                                   uint32_t flowIdx,
                                   uint16_t vlanId,
                                   EthRemoteCfg_VirtPort virtPort)
{
    Enet_IoctlPrms prms;
    CpswAle_VlanIdInfo inArgs;
    CpswAle_DelPolicerEntryInArgs polInArgs;
    uint32_t privVlanId;
    uint32_t i;
    int32_t status = ETHFW_EFAIL;
    EthFwVepa_AddrEntry *mcastInfo;
    /* ALE entry mask == 0 means do not delete any ale entry, just delete the policer entry */
    uint32_t aleEntryMask = 0U;

    privVlanId = gEthFwVepaObj.privVlanCfg[virtPort];

    /* Remove ALE policer for VLAN used for packet forwarding from ETHFW to Remote Client
     * Note: It removes the entry from ALE as well */
    polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_IVLAN;
    polInArgs.policerMatch.ivlanId            = privVlanId;
    polInArgs.aleEntryMask                    = CPSW_ALE_POLICER_TABLEENTRY_DELETE_IVLAN;

    ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

    status = Enet_ioctl(hEnet, coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                      "Failed to delete policer for priv VLAN %u", privVlanId);

    if (status == ETHFW_SOK)
    {
        /* Remove MAC address for the registered virtual switch port */
        memset(&gEthFwVepaObj.virtPortToMacAddr[virtPort], 0U, sizeof(struct eth_addr));

        /* Remove broadcast entry from VEPA table for the virtual switch port */
        status = EthFwVepa_delAddr(&gEthFwBcastAddr, 0U, virtPort);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                          "Failed to delete bcast addr for VLAN %u from VEPA table", vlanId);

        /* If last client has de-registered */
        mcastInfo = EthFwVepa_getMcastInfo(&gEthFwBcastAddr, 0U); 
        if(mcastInfo == NULL)
        {
            /* Remove policer: broadcast packet of (irrespective of vlan) should not go to packet duplication flow */
            status = EthFwVepa_delMcastPolicer(hEnet, coreId,
                                               0U, gEthFwBcastAddr.addr,
                                               aleEntryMask);
        }
    }

    return status;
}

int32_t EthFwVepa_sendRaw(struct netif *netif,
                          struct pbuf *pbuf,
                          struct eth_addr *ethSrcAddr,
                          struct eth_addr *ethDstAddr)
{
    struct pbuf *copyPbuf = NULL;
    struct eth_hdr *ethHdr;
    struct eth_vlan_hdr *vlanhdr;
    uint16_t ethType;
    uint32_t virtPortMask = 0U;
    uint16_t i;
    uint16_t privVlanId = 0U;
    uint16_t vlanId = 0U;
    int32_t status = ETHFW_SOK;
    /* Priority code point and drop eligible indicator
     * Read this for more info: https://en.wikipedia.org/wiki/IEEE_802.1Q */
    uint16_t pcpDei = 0U;

    /* Find VLAN id of the packet if it is VLAN tagged */
    ethHdr = (struct eth_hdr *)(pbuf->payload);
    if (ethHdr->type == lwip_htons(ETHTYPE_VLAN))
    {
        vlanhdr = (struct eth_vlan_hdr *)((uint8_t *)pbuf->payload + SIZEOF_ETH_HDR);
        vlanId = lwip_htons(vlanhdr->prio_vid) & 0xFFFU;
    }

    status = EthFwVepa_getVirtPortMask(&ethHdr->dest, vlanId, &virtPortMask);
    if (status == ETHFW_SOK)
    {
        /* Allocate a pbuf for the outgoing duplicated packet, 4 bytes are added for double VLAN tag */
        copyPbuf = pbuf_alloc(PBUF_RAW_TX, pbuf->tot_len+sizeof(struct eth_vlan_hdr), PBUF_RAM);
        /* Could allocate a pbuf ? */
        if (copyPbuf == NULL)
        {
            status = ETHFW_EFAIL;
            ETHFWTRACE_ERR(status, "Could not allocate pbuf for packet duplication");
        }
    }

    if ((status == ETHFW_SOK) &&
        (copyPbuf != NULL))
    {
        for (i = 0U; i <= ETHREMOTECFG_SWITCH_PORT_LAST; i++)
        {
            /* i'th virtual switch port will get the packet
             * Also source virtual switch port should not receive the packet */
            if (ETHFW_IS_BIT_SET(virtPortMask, i) &&
                !EnetUtils_cmpMacAddr(ethSrcAddr->addr, gEthFwVepaObj.virtPortToMacAddr[i].addr))
            {
                status = EthFwVepa_getPrivateVlanId(i, &privVlanId);
                if (status == ETHFW_SOK)
                {
                    ethType = lwip_htons(ETHTYPE_VLAN);
                    ethHdr = (struct eth_hdr *)copyPbuf->payload;

                    /* Adding source and destination for the packet */
                    SMEMCPY(&ethHdr->dest, ethDstAddr, ETH_HWADDR_LEN);
                    SMEMCPY(&ethHdr->src,  ethSrcAddr, ETH_HWADDR_LEN);

                    /* Adding tpid as 0x8100 (16 bits)
                     * CPSW is switching using INNER VLAN tag in VLAN aware mode and inner VLAN ltype is 0x8100
                     * Double VLAN tagged packet with 2 customer tags
                     * (i.e.  dst_mac    src_mac    8100 VLAN_1    8100 VLAN_2  ...)
                     * will be switched based on the first VLAN (i.e. VLAN_1)
                     * This helps us to do packet switching based on private vlan added on top of existing packet */
                    ethHdr->type = ethType;

                    /* Adding priority and VLAN id tags (16 bits) */
                    vlanhdr = (struct eth_vlan_hdr *)(((uint8_t *)copyPbuf->payload) + SIZEOF_ETH_HDR);
                    vlanhdr->prio_vid = lwip_htons(pcpDei | privVlanId);

                    /* Adding EtherType of the packet (16 bits) */
                    vlanhdr->tpid = ((struct eth_hdr*)pbuf->payload)->type;

                    /* Adding payload in the packet */
                    SMEMCPY(copyPbuf->payload + SIZEOF_ETH_HDR + sizeof(struct eth_vlan_hdr),
                            pbuf->payload + SIZEOF_ETH_HDR,
                            pbuf->tot_len - SIZEOF_ETH_HDR);

                    /* Send the packet */
                    LOCK_TCPIP_CORE();
                    netif->linkoutput(netif, copyPbuf);
                    UNLOCK_TCPIP_CORE();
                }
                else
                {
                    ETHFWTRACE_ERR(status, "Failed to get priv VLAN for virtual port %u", i);
                }
            }
        }
        pbuf_free(copyPbuf);
    }

    return status;
}

uint32_t EthFwVepa_getUseCnt(void)
{
    uint32_t cnt = 0U;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwVepaObj.hMutex);

    for (i = 0U; i < ETHFW_VEPA_TABLE_SIZE; i++)
    {
        if (!gEthFwVepaObj.remoteCoreVepaTable[i].isFree)
        {
            cnt++;
        }
    }

    EthFwOsal_unlockMutex(gEthFwVepaObj.hMutex);

    return cnt;
}
