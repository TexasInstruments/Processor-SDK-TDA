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
 * \file ethfw_vepa_priv.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware VEPA support on devices with CPSW ALE
 *        multihost feature.
 */

#ifndef ETHFW_VEPA_PRIV_H_
#define ETHFW_VEPA_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>
#include <include/per/cpsw.h>
#include <ethremotecfg/server/include/ethfw_vepa.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \addtogroup ETHFW_VEPA_UTILS
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief Flow index used to received packets for duplication is not defined */
#define ETHFW_VEPA_PKT_DUP_FLOW_IDX_UNDEFINED      (0xFFFFU)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize VEPA configuration parameters.
 *
 * \param vepaCfg    Configuration parameters to be initialized
 */
void EthFwVepa_initCfg(EthFwVepa_Cfg *vepaCfg);

/*!
 * \brief Initialize VEPA utils module.
 *
 * Initializes VEPA utils module. User should call \ref EthFwVepa_initCfg
 * to initialize configuration parameters and make updates if needed
 * before calling this function.
 *
 * \param vepaCfg    Configuration parameters
 *
 * \returns ETHFW_SOK if VEPA initialization was successful
 */
int32_t EthFwVepa_init(const EthFwVepa_Cfg *vepaCfg);

/*!
 * \brief De-initialize VEPA utils module.
 *
 * Deinitializes VEPA utils, it must be the last VEPA utils function to be
 * called.
 */
void EthFwVepa_deinit(void);

/*!
 * \brief Add a multicast entry in VEPA table for client
 *
 * Untagged traffic received on any MAC port in switch mode gets the port's
 * default VLAN at ingress, is processed by ALE with that VLAN id and finally
 * untagged on egress.  In this case, `vlanId` argument will be 0, otherwise
 * argument value will be the VLAN id of client.
 *
 * For VLAN tagged traffic, both `vlanId` and `hwVlanId` will both be the same.
 *
 * \param hwAddr         MAC address to add in VEPA table
 * \param vlanId         VLAN id (client VLAN)
 * \param virtPort       Remote core virtual port id
 *
 * \returns ETHFW_SOK if multicast entry addition was successful, or a
 *          negative error code if failed.
 */
int32_t EthFwVepa_addAddr(struct eth_addr *hwAddr,
                          uint16_t vlanId,
                          EthRemoteCfg_VirtPort virtPort);

/*!
 * \brief Removes an entry in VEPA table for client
 *
 * Untagged traffic received on any MAC port in switch mode gets the port's
 * default VLAN at ingress, is processed by ALE with that VLAN id and finally
 * untagged on egress.  In this case, `vlanId` argument will be 0 otherwise
 * argument value will be the VLAN id of client.
 *
 * For VLAN tagged traffic, both `vlanId` and `hwVlanId` will both be the same.
 *
 * \param hwAddr         MAC address to add in VEPA table
 * \param vlanId         VLAN id (client VLAN)
 * \param virtPort       Remote core virtual port id
 *
 * \returns ETHFW_SOK if multicast entry deletion was successful, or a
 *          negative error code if failed.
 */
int32_t EthFwVepa_delAddr(struct eth_addr *hwAddr,
                          uint16_t vlanId,
                          EthRemoteCfg_VirtPort virtPort);

/*!
 * \brief Print VEPA table with private VLAN associated to each virtual port.
 */
void EthFwVepa_printTable(void);

/*!
 * \brief Flush all entries in VEPA table.
 */
void EthFwVepa_flushTable(void);

/*!
 * \brief Adds ALE entry and policer with private VLAN when a client registers.
 *
 * \param hEnet      Handle to CPSW
 * \param coreId     Remote core IPC core id
 * \param flowIdx    RX flow id of virtual port
 * \param vlanId     VLAN id
 * \param virtPort   Remote core virtual port id
 * \param srcAddr    MAC address of virtual port
 *
 * \returns ETHFW_SOK if client registered successfully, or a negative
 *          error code if failed.
 */
int32_t EthFwVepa_registerClient(Enet_Handle hEnet,
                                 uint32_t coreId,
                                 uint32_t flowIdx,
                                 uint16_t vlanId,
                                 EthRemoteCfg_VirtPort virtPort,
                                 struct eth_addr *srcAddr);

/*!
 * \brief Removes ALE entry and policer with private VLAN when
 *        a client de-registers.
 *
 * \param hEnet      Handle to CPSW
 * \param coreId     Remote core IPC core id
 * \param flowIdx    RX flow id of virtual port
 * \param vlanId     VLAN id
 * \param virtPort   Remote core virtual port id
 *
 * \returns ETHFW_SOK if client de-registered successfully, or a
 *          negative error code if failed.
 */
int32_t EthFwVepa_unregisterClient(Enet_Handle hEnet,
                                   uint32_t coreId,
                                   uint32_t flowIdx,
                                   uint16_t vlanId,
                                   EthRemoteCfg_VirtPort virtPort);

/*!
 * \brief Get the number of used entries in VEPA Table.
 *
 * \returns Number of entries used in VEPA Table.
 */
uint32_t EthFwVepa_getUseCnt(void);

/*!
 * \brief Adds a multicast address in ALE Policer table
 *
 * \param hEnet               Handle to CPSW
 * \param coreId              Remote core IPC core id
 * \param vlanId              VLAN id
 * \param mcastAddr           MAC address to add in policer table
 * \param policerPartLevel    Partition level to add policer
 *
 * \returns ETHFW_SOK if policer created successfully, or a
 *          negative error code if failed.
 */
int32_t EthFwVepa_addMcastPolicer(Enet_Handle hEnet,
                                  uint32_t coreId,
                                  uint32_t vlanId,
                                  const uint8_t *mcastAddr,
                                  CpswAle_PolicerPartLevel policerPartLevel);

/*!
 * \brief Deletes a multicast address in ALE Policer table
 *
 * \param hEnet               Handle to CPSW
 * \param coreId              Remote core IPC core id
 * \param vlanId              VLAN id
 * \param mcastAddr           MAC address to add in policer table
 * \param aleEntryMask        Deletes ALE entry as well based on aleEntryMask (value of 0 means do not delete corresponding ALE)
 * 
 * \returns ETHFW_SOK if policer deleted successfully, or a
 *          negative error code if failed.
 */
int32_t EthFwVepa_delMcastPolicer(Enet_Handle hEnet,
                                  uint32_t coreId,
                                  uint32_t vlanId,
                                  const uint8_t *mcastAddr,
                                  uint32_t aleEntryMask);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_VEPA_PRIV_H_ */
