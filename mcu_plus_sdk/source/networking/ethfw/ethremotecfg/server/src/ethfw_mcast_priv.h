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
 * \file  ethfw_mcast_priv.h
 *
 * \brief This file contains the private type definitions, helper macros
 *        and functions required for Ethernet Firmware multicast support.
 */

#ifndef ETHFW_MCAST_PRIV_H_
#define ETHFW_MCAST_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>
#include <ethremotecfg/server/include/ethfw_mcast.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t EthFwMcast_init(const EthFwMcast_Cfg *cfg,
                        uint32_t switchPortMask,
                        uint32_t macOnlyPortMask);

void EthFwMcast_deinit(void);

int32_t EthFwMcast_filterAddMac(EthRemoteCfg_VirtPort virtPort,
                                Enet_Handle hEnet,
                                const uint8_t *macAddr,
                                uint16_t vlanId,
                                uint16_t hwVlanId,
                                uint32_t flowIdxOffset,
                                uint8_t hostId);

int32_t EthFwMcast_filterDelMac(EthRemoteCfg_VirtPort virtPort,
                                Enet_Handle hEnet,
                                uint8_t *macAddr,
                                uint16_t vlanId,
                                uint16_t hwVlanId,
                                uint8_t hostId);

void EthFwMcast_printTable(void);

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_MCAST_PRIV_H_ */
