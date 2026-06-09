/*
 *
 * Copyright (c) 2021-2024 Texas Instruments Incorporated
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
 *  \file ethfw_arp.c
 *
 *  \brief Ethernet Firmware proxy ARP implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x102

#include <string.h>

/* EthFw header files */
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include "ethfw_arp_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of entries in the address table */
#define ETHFW_ARP_TABLE_SIZE                       (16U)

/*!
 * \brief Table entry with IP address and MAC address for remote cores.
 */
typedef struct EthFwArp_AddrEntry_s
{
    /*! Remote core's IP address */
    ip4_addr_t ipAddr;

    /*! Remote core's MAC address */
    struct eth_addr hwAddr;

    /*! VLAN id */
    uint16_t vlanId;

    /*! Whether this entry is free or not */
    bool isFree;
} EthFwArp_AddrEntry;

/*!
 * \brief lwIP ARP Utils object.
 */
typedef struct EthFwArp_Obj_s
{
    /*! ARP table */
    EthFwArp_AddrEntry remoteCoreArpTable[ETHFW_ARP_TABLE_SIZE];

    /*! Mutex handle. Used to protect ARP table from concurrent accesses */
    EthFwOsal_MutexHandle hMutex;
} EthFwArp_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EthFwArp_Obj gEthFwArpObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwArp_init(void)
{
    int32_t status = ETHFW_SOK;
    uint32_t i;

    /* Create mutex to protect ARP table */
    gEthFwArpObj.hMutex = EthFwOsal_createMutex();
    if (gEthFwArpObj.hMutex == NULL)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to create mutex");
    }

    /* Mark all entries in table as free */
    if (status == ETHFW_SOK)
    {
        for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
        {
            gEthFwArpObj.remoteCoreArpTable[i].isFree = BTRUE;
        }
    }

    return status;
}

void EthFwArp_deinit(void)
{
    if (gEthFwArpObj.hMutex != NULL)
    {
        EthFwOsal_deleteMutex(gEthFwArpObj.hMutex);
    }
    else
    {
        ETHFWTRACE_WARN("ETHFW ARP module is not initialized");
    }
}

int32_t EthFwArp_getHwAddr(const ip4_addr_t *ipAddr,
                           struct eth_addr *hwAddr,
                           uint16_t vlanId)
{
    EthFwArp_AddrEntry *entry;
    int32_t status = ETHFW_EFAIL;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwArpObj.hMutex);

    for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
    {
        entry = &gEthFwArpObj.remoteCoreArpTable[i];

        /* Check if there is an entry matching given IP address */
        if (!entry->isFree &&
            (entry->vlanId == vlanId) &&
            ip4_addr_cmp(&entry->ipAddr, ipAddr))
        {
            SMEMCPY(hwAddr, &entry->hwAddr, ETH_HWADDR_LEN);
            status = ETHFW_SOK;
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwArpObj.hMutex);

    return status;
}

int32_t EthFwArp_addAddr(const ip4_addr_t *ipAddr,
                         const struct eth_addr *hwAddr,
                         uint16_t vlanId)
{
    EthFwArp_AddrEntry *entry;
    int32_t status = ETHFW_EALLOC;
    uint32_t i;
    bool done = BFALSE;

    if (ETHFW_IS_BIT_SET(hwAddr->addr[0], 0))
    {
        status = ETHFW_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "mcast MAC address cannot be added");
    }
    else if (ip4_addr_ismulticast(ipAddr))
    {
        status = ETHFW_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "mcast IP address cannot be added");
    }
    else
    {
        EthFwOsal_lockMutex(gEthFwArpObj.hMutex);

        /* Check if an entry already in table needs to be updated */
        for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
        {
             entry = &gEthFwArpObj.remoteCoreArpTable[i];

             /* Check if an entry for the IP address is already in table,
              * if so, update MAC address */
             if (!entry->isFree &&
                 (entry->vlanId == vlanId) &&
                 ip4_addr_cmp(ipAddr, &entry->ipAddr))
             {
                SMEMCPY(&entry->hwAddr, hwAddr, ETH_HWADDR_LEN);
                done = BTRUE;
                break;
            }
        }

        /* Look for new entry in table */
        if (!done)
        {
            for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
            {
                entry = &gEthFwArpObj.remoteCoreArpTable[i];

                /* Take free entry and populate IP address / MAC address pair */
                if (entry->isFree)
                {
                    ip4_addr_copy(entry->ipAddr, *ipAddr);
                    SMEMCPY(&entry->hwAddr, hwAddr, ETH_HWADDR_LEN);
                    entry->vlanId = vlanId;
                    entry->isFree = BFALSE;
                    done = BTRUE;
                    break;
                }
            }
        }

        EthFwOsal_unlockMutex(gEthFwArpObj.hMutex);

        status = done ? ETHFW_SOK : ETHFW_EALLOC;
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                          "Failed to add IP addr %s, HW addr %02x:%02x:%02x:%02x:%02x:%02x",
                          ip4addr_ntoa(ipAddr),
                          hwAddr->addr[0U], hwAddr->addr[1U], hwAddr->addr[2U],
                          hwAddr->addr[3U], hwAddr->addr[4U], hwAddr->addr[5U]);
    }

    return status;
}

int32_t EthFwArp_delAddr(const ip4_addr_t *ipAddr,
                         uint16_t vlanId)
{
    EthFwArp_AddrEntry *entry;
    int32_t status = ETHFW_EFAIL;
    bool done = BFALSE;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwArpObj.hMutex);

    for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
    {
        entry = &gEthFwArpObj.remoteCoreArpTable[i];

        /* Clear entry if matching IP address is found */
        if (!entry->isFree &&
            (entry->vlanId == vlanId) &&
            ip4_addr_cmp(&entry->ipAddr, ipAddr))
        {
            ip4_addr_set_zero(&entry->ipAddr);
            memset(&entry->hwAddr, 0, sizeof(struct eth_addr));
            entry->isFree = BTRUE;
            done = BTRUE;
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwArpObj.hMutex);

    status = done ? ETHFW_SOK : ETHFW_EFAIL;

    return status;
}

uint32_t EthFwArp_getUseCnt(void)
{
    uint32_t cnt = 0U;
    uint32_t i;

    EthFwOsal_lockMutex(gEthFwArpObj.hMutex);

    for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
    {
        if (!gEthFwArpObj.remoteCoreArpTable[i].isFree)
        {
            cnt++;
        }
    }

    EthFwOsal_unlockMutex(gEthFwArpObj.hMutex);

    return cnt;
}

void EthFwArp_printTable(void)
{
    EthFwArp_AddrEntry *entry;
    const uint8_t *hwAddr;
    uint32_t used = 0U;
    uint32_t i;

    ETHFWTRACE_INFO("\n SNo.      MAC Address        VLAN     IP Address");
    ETHFWTRACE_INFO("------  -------------------  ------  -----------------");

    for (i = 0U; i < ETHFW_ARP_TABLE_SIZE; i++)
    {
        entry = &gEthFwArpObj.remoteCoreArpTable[i];

        if (!entry->isFree)
        {
            hwAddr = &entry->hwAddr.addr[0U];

            ETHFWTRACE_INFO("  %3d    %02x:%02x:%02x:%02x:%02x:%02x    %4d    %s",
                            ++used,
                            hwAddr[0] & 0xFF, hwAddr[1] & 0xFF, hwAddr[2] & 0xFF,
                            hwAddr[3] & 0xFF, hwAddr[4] & 0xFF, hwAddr[5] & 0xFF,
                            entry->vlanId,
                            ip4addr_ntoa(&entry->ipAddr));
        }
    }
}

void EthFwArp_sendRaw(struct netif *netif,
                      const struct eth_addr *ethSrcAddr,
                      const struct eth_addr *ethDstAddr,
                      const struct eth_addr *hwSrcAddr,
                      const ip4_addr_t *ipSrcAddr,
                      const struct eth_addr *hwDstAddr,
                      const ip4_addr_t *ipDstAddr,
                      uint16_t vlanId,
                      const u16_t opcode)
{
    struct pbuf *pbuf;
    struct eth_hdr *ethHdr;
    struct eth_vlan_hdr *vlanHdr;
    struct etharp_hdr *arpHdr;
    uint8_t *payload;
    uint16_t len = SIZEOF_ETH_HDR + SIZEOF_ETHARP_HDR;
    uint16_t pcpDei = 0U;

    if (vlanId != 0U)
    {
        len += SIZEOF_VLAN_HDR;
    }

    /* allocate a pbuf for the outgoing ARP request packet */
    pbuf = pbuf_alloc(PBUF_RAW_TX, len, PBUF_RAM);
    /* could allocate a pbuf for an ARP request? */
    if (pbuf == NULL)
    {
        ETHFWTRACE_ERR(ETHFW_EFAIL, "Could not allocate pbuf for ARP request");
    }
    else
    {
        payload = (uint8_t *)pbuf->payload;

        /* Add Layer-2 header */
        ethHdr = (struct eth_hdr *)payload;
        SMEMCPY(&ethHdr->dest, ethDstAddr, ETH_HWADDR_LEN);
        SMEMCPY(&ethHdr->src,  ethSrcAddr, ETH_HWADDR_LEN);
        payload += SIZEOF_ETH_HDR;

        if (vlanId != 0U)
        {
            ethHdr->type = lwip_htons(ETHTYPE_VLAN);

            vlanHdr = (struct eth_vlan_hdr *)payload;
            vlanHdr->prio_vid = lwip_htons(pcpDei | vlanId);
            vlanHdr->tpid = lwip_htons(ETHTYPE_ARP);

            payload += SIZEOF_VLAN_HDR;
        }
        else
        {
            ethHdr->type = lwip_htons(ETHTYPE_ARP);
        }

        /* Write ARP reply */
        arpHdr = (struct etharp_hdr *)payload;
        arpHdr->opcode = lwip_htons(opcode);

        /* Write the ARP MAC-Addresses */
        SMEMCPY(&arpHdr->shwaddr, hwSrcAddr, ETH_HWADDR_LEN);
        SMEMCPY(&arpHdr->dhwaddr, hwDstAddr, ETH_HWADDR_LEN);

        /* Copy struct ip4_addr_wordaligned to aligned ip4_addr, to support compilers without
         * structure packing. */
        IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&arpHdr->sipaddr, ipSrcAddr);
        IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&arpHdr->dipaddr, ipDstAddr);

        arpHdr->hwtype = PP_HTONS(1);
        arpHdr->proto = PP_HTONS(ETHTYPE_IP);
        /* set hwlen and protolen */
        arpHdr->hwlen = ETH_HWADDR_LEN;
        arpHdr->protolen = sizeof(ip4_addr_t);

        LOCK_TCPIP_CORE();
        netif->linkoutput(netif, pbuf);
        UNLOCK_TCPIP_CORE();

        pbuf_free(pbuf);
    }
}
