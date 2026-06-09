/*
 *  Copyright (c) Texas Instruments Incorporated 2023
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
 * \file  ethfw_mcast.h
 *
 * \brief This file contains the type definitions, helper macros related to
 *        Ethernet Firmware multicast support.
 */

#ifndef ETHFW_MCAST_H_
#define ETHFW_MCAST_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ethremotecfg/protocol/ethremotecfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_SERVER_MCAST Multicast Support
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! Size of shared multicast address table */
#define ETHFW_SHARED_MCAST_LIST_LEN       (8U)

/*! Size of reserved multicast address table */
#define ETHFW_RSVD_MCAST_LIST_LEN         (4U)

/*! Size of Exclusive multicast address table */
#define ETHFW_EXCLUSIVE_MCAST_LIST_LEN       (32U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Filter 'add' shared multicast callback.
 *
 * Application callback function called when a remote client requests the addition of
 * a shared multicast address to the filter.
 *
 * \param macAddr    Multicast address being added
 * \param coreId     Remote core id which originated the multicast 'add' request
 */
typedef void (*EthFwMcast_FilterAddMacSharedCb)(const uint8_t *macAddr,
                                                uint8_t coreId);

/*!
 * \brief Filter delete shared multicast callback.
 *
 * Application callback function called when a remote client requests the deletion of
 * a shared multicast address from the filter.
 *
 * \param macAddr    Multicast address being added
 * \param coreId     Remote core id which originated the multicast 'remove' request
 */
typedef void (*EthFwMcast_FilterDelMacSharedCb)(const uint8_t *macAddr,
                                                uint8_t coreId);

/*!
 * \brief Multicast configuration params.
 *
 * Multicast params composed of the multicast address, port mask and the virtual port mask.
 */
typedef struct EthFwMcast_McastCfg_s
{
    /*! Multicast address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /*! Hardware port mask (Ethernet port and host port):
     *  - Use `CPSW_ALE_MACPORT_TO_ALEPORT()` to set the MAC port bitmask
     *  - Use `CPSW_ALE_HOST_PORT_MASK` to set the host port bitmask */
    uint32_t portMask;

    /*! Virtual port mask. Use `ETHFW_BIT(ETHREMOTECFG_SWITCH_PORT_n)`
     *  to create the member mask. */
    uint32_t virtPortMask;
} EthFwMcast_McastCfg;

/*!
 * \brief Multicast configuration parameters.
 */
typedef struct EthFwMcast_SharedMcastCfg_s
{
    /*! Shared multicast address table */
    const EthFwMcast_McastCfg *mcastCfg;

    /*! Number of multicast address actually populated in the shared table.
     *  Must be < \ref ETHFW_SHARED_MCAST_LIST_LEN */
    uint32_t numMcast;

    /*! Application callback function to handle addition of a shared multicast address. */
    EthFwMcast_FilterAddMacSharedCb filterAddMacSharedCb;

    /*! Application callback function to handle deletion of a shared multicast address. */
    EthFwMcast_FilterDelMacSharedCb filterDelMacSharedCb;
} EthFwMcast_SharedMcastCfg;

/*!
 * \brief Reserved multicast.
 */
typedef struct EthFwMcast_RsvdMcast_s
{
    /*! Multicast address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /*! VLAN id */
    uint16_t vlanId;

    /*! Hardware port mask (Ethernet port and host port):
     *  - Use `CPSW_ALE_MACPORT_TO_ALEPORT()` to set the MAC port bitmask
     *  - Use `CPSW_ALE_HOST_PORT_MASK` to set the host port bitmask */
    uint32_t portMask;
} EthFwMcast_RsvdMcast;

/*!
 * \brief Reserved multicast configuration.
 *
 * Configuration parameters for reserved multicast addresses, that is, multicast
 * addresses that are restricted to Ethernet Firmware only.
 */
typedef struct EthFwMcast_RsvdMcastCfg_s
{
    /*! Reserved multicast address table */
    const EthFwMcast_RsvdMcast *mcastCfg;

    /*! Number of multicast address actually populated in the reserved address table.
     *  Must be < \ref ETHFW_RSVD_MCAST_LIST_LEN */
    uint32_t numMcast;
} EthFwMcast_RsvdMcastCfg;

/*!
 * \brief Multicast configuration (shared and reserved addresses).
 */
typedef struct EthFwMcast_Cfg_s
{
    /*! Shared multicast configuration */
    EthFwMcast_SharedMcastCfg sharedMcastCfg;

    /*! Reserved multicast configuration */
    EthFwMcast_RsvdMcastCfg rsvdMcastCfg;
} EthFwMcast_Cfg;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_MCAST_H_ */
