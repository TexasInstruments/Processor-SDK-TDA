/*
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
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
 * \file ethfw_vepa.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware VEPA support on devices with CPSW ALE
 *        multihost feature.
 */

#ifndef ETHFW_VEPA_H_
#define ETHFW_VEPA_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "lwip/prot/ethernet.h"
#include "netif/ethernet.h"
#include <ethremotecfg/protocol/ethremotecfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_SERVER_VEPA VEPA Support
 * @{
 *
 * Ethernet Firmware also provides support to enable VEPA (Virtual Ethernet
 * Port Aggregator) functionality in Jacinto devices with CPSW capable of
 * multihost data flow.  _Multihost_ is a CPSW ALE feature that enables packets
 * to be sent and received on host port, which is mandatory for VEPA.
 *
 * The VEPA implementation allocates a secondary RX flow exclusively for
 * broadcast and registered multicast packets to be routed to.  This flow is
 * in addition to the RX flows used by Ethernet Firmware to receive all other
 * traffic (i.e. TCP/IP, gPTP).
 *
 * Ethernet Firmware server registers multicast MAC addresses that need to be
 * forwarded to remote clients.  These multicast addresses are added to an
 * internal VEPA table, which can be printed for debug purpose.  An ALE policer
 * entry is also added for each multicast address in the table so that when
 * multicast packets arrive at any of the  MAC ports configured in non MAC-only
 * mode, they are routed to its dedicated flow, allocated at init time.
 *
 * When a multicast packet whose MAC address is registered in the VEPA table,
 * it will be passed to a VEPA specific handle function, which then calls
 * \ref EthFwVepa_sendRaw() to send a copy of the multicast packets to all
 * relevant remote cores.
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Ethernet Firmware VEPA configuration.
 */
typedef struct EthFwVepa_Cfg_s
{
    /*! Private VLAN id for each remote core */
    uint32_t privVlanId[ETHREMOTECFG_SWITCH_PORT_LAST+1];
} EthFwVepa_Cfg;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Sets packet duplication flow.
 *
 * \param flowIdx    RX flow idx to be set
 *
 * \returns \ref ETHFW_SOK if packet duplication flow was set successful,
 *          or a negative error code if failed.
 */
uint32_t EthFwVepa_setPacketDuplicationFlowIdx(uint32_t flowIdx);

/*!
 * \brief Clear packet duplication flow.
 */
void EthFwVepa_clearPacketDuplicationFlowIdx(void);

/*!
 * \brief Sends pbuf to all the required virtual switch ports after
 *        inserting their corresponding private VLAN.
 *
 * \param netif      Enet netif on which packet received
 * \param pbuf       Pointer to the data buffer received
 * \param ethSrcAddr Source MAC address
 * \param ethDstAddr Destination MAC address
 *
 * \returns ETHFW_SOK if packet was sent successfully to required virtual
 *          switch ports, or a negative error code if failed.
 */
int32_t EthFwVepa_sendRaw(struct netif *netif,
                          struct pbuf *pbuf,
                          struct eth_addr *ethSrcAddr,
                          struct eth_addr *ethDstAddr);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_VEPA_H_ */
