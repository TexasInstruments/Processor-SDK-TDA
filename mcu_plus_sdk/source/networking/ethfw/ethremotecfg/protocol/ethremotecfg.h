/*
 *
 * Copyright (c) 2023-2025 Texas Instruments Incorporated
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
 * \file ethremotecfg.h
 *
 * \brief Header file with the Ethernet Firmware remote configuration interface
 *        definition.
 */

#ifndef ETHREMOTECFG_H_
#define ETHREMOTECFG_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_MODULE
 * \defgroup ETHFW_ETHREMOTECFG Ethernet Firmware Remote Config Interface
 *
 * \brief EthFw Remote Client message definitions.
 *
 * Remote clients communicate with Ethernet Firmware via IPC based APIs and
 * need a common message structure definitions to understand the communication.
 * This file defines the headers and message structures of remote client
 * communication.
 *
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_ETHREMOTECFG
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \anchor EthRemoteCfg_EndptInfo
 * \name Ethernet Remote Config IPC Endpoint Info.
 * @{
 */
/*! \brief AUTOSAR service endpoint name */
#define ETHREMOTECFG_AUTOSAR_REMOTE_SERVICE_NAME          "ti.autosar.ethdevice"

/*! \brief Linux/RTOS/QNX endpoint name */
#define ETHREMOTECFG_FRAMEWORK_SERVICE_NAME               "ti.ethfw.ethdevice"

/*! \brief Notify service endpoint name */
#define ETHREMOTECFG_REMOTE_NOTIFY_SERVICE                "ti.ethfw.notifyservice"
/*! @} */

/*!
 * \anchor EthRemoteCfg_ErrorCode
 * \name Ethernet Remote Config Error Codes.
 * @{
 */
/*! \brief Success. */
#define ETHREMOTECFG_SOK                                  (0)

/*! \brief Operation in progress. */
#define ETHREMOTECFG_SINPROGRESS                          (1)

/*! \brief Failure. */
#define ETHREMOTECFG_EFAIL                                (-1)

/*! \brief Bad arguments (i.e. NULL pointer). */
#define ETHREMOTECFG_EBADARGS                             (-2)

/*! \brief Invalid parameters (i.e. value out-of-range). */
#define ETHREMOTECFG_EINVALIDPARAMS                       (-3)

/*! \brief Time out while waiting for a given condition to happen. */
#define ETHREMOTECFG_ETIMEOUT                             (-4)

/*! \brief Allocation failure. */
#define ETHREMOTECFG_EALLOC                               (-8)

/*! \brief Unexpected condition occurred (sometimes unrecoverable). */
#define ETHREMOTECFG_EUNEXPECTED                          (-9)

/*! \brief The resource is currently busy performing an operation. */
#define ETHREMOTECFG_EBUSY                                (-10)

/*! \brief Already open error. */
#define ETHREMOTECFG_EALREADYOPEN                         (-11)

/*! \brief Operation not permitted. */
#define ETHREMOTECFG_EPERM                                (-12)

/*! \brief Operation not supported. */
#define ETHREMOTECFG_ENOTSUPPORTED                        (-13)

/*! \brief Resource not found. */
#define ETHREMOTECFG_ENOTFOUND                            (-14)
/*! @} */

/*! \brief MAC Address length in octets. */
#define ETHREMOTECFG_MACADDRLEN                           (6U)

/*! \brief IPv4 Address length in octets. */
#define ETHREMOTECFG_IPV4ADDRLEN                          (4U)

/*! Ethernet Port1 ID. */
#define ETHERNET_PORT1_ID                                 (1U)

/*! Virtual Switch Port1 ID. */
#define ETHREMOTECFG_SWITCH_PORT_FIRST_ID                 (25U)

/*!
 * Indicates to the Ethernet Firmware to use the default VLAN id for
 * the type of port associated with the caller, i.e. virtual MAC or virtual
 * switch.
 */
#define ETHREMOTECFG_ETHSWITCH_VLAN_USE_DFLT              (0xFFFF)

/*! \brief Number of octets in year. */
#define ETHREMOTECFG_FWDATE_YEARLEN                       (4U)

/*! \brief Number of octets in month. */
#define ETHREMOTECFG_FWDATE_MONTHLEN                      (3U)

/*! \brief Number of octets in date. */
#define ETHREMOTECFG_FWDATE_DATELEN                       (2U)

/*! \brief GIT Commit SHA length in octets. */
#define ETHREMOTECFG_FW_COMMITSHALEN                      (8U)

/*!
 * \name Ethernet Firmware version information.
 *
 * API version info for the Ethernet Firmware which any remote client
 * should check to ensure compatibility.
 * @{
 */
/*! \brief Ethernet device API version major version. */
#define ETHREMOTECFG_FW_ETHSWITCH_VERSION_MAJOR           (0U)

/*! \brief Ethernet device API version minor version. */
#define ETHREMOTECFG_FW_ETHSWITCH_VERSION_MINOR           (5U)

/*! \brief Ethernet device API version minor revision. */
#define ETHREMOTECFG_FW_ETHSWITCH_VERSION_REVISION        (0U)
/*! @} */

/*!
 * \anchor EthRemoteCfg_FeatureMask
 * \name Ethernet device supported feature masks.
 * @{
 */
/*! \brief Tx checksum offload feature. */
#define ETHREMOTECFG_FEATURE_TXCSUM                       (1 << 0)

/*! \brief Multicast filter feature. */
#define ETHREMOTECFG_FEATURE_MC_FILTER                    (1 << 1)
/*! @} */

/*! \brief Max IPC message size. */
#define ETHREMOTECFG_IPC_MSG_SIZE                         (496U + 32U)

/*! \brief Max payload size in an IPC messsage. */
#define ETHREMOTECFG_IPC_MSG_PAYLOAD_SIZE                 (496U)

/*! \brief Max number of IPC buffers. */
#define ETHREMOTECFG_IPC_NUM_MSG_BUFS                     (256U)

/*! \brief Buffer size allocated to the channel. */
#define ETHREMOTECFG_IPC_MSG_OBJ_SIZE                     (256U)

/*! \brief IPC data size. */
#define ETHREMOTECFG_IPC_DATA_SIZE                        (ETHREMOTECFG_IPC_MSG_SIZE * \
                                                           ETHREMOTECFG_IPC_NUM_MSG_BUFS + \
                                                           ETHREMOTECFG_IPC_MSG_OBJ_SIZE)

/*! \brief Ethernet device C2S request type count. */
#define ETHREMOTECFG_CMD_TYPE_COUNT                       (ETHREMOTECFG_CMD_TYPE_LAST + 1)

/*! \brief Ethernet device S2C notify message type count. */
#define ETHREMOTECFG_NOTIFY_TYPE_COUNT                    (ETHREMOTECFG_NOTIFY_LAST + 1)

/*! \brief Remote client token value when none is assigned by the server. */
#define ETHREMOTECFG_TOKEN_NONE                           ((uint32_t)0xFFFFFFFFU)

/*! \brief Remote notify service endpoint Id needed by server to send notify messages. */
#define ETHREMOTECFG_NOTIFY_SERVICE_ENDPT_ID              (30U)

/*!
 * \brief De-normalize #EthRemoteCfg_VirtPort.
 *
 * Macro to denormalize VirtPort number. It takes a zero-based
 * virtport number and converts it to a #EthRemoteCfg_VirtPort enum.
 */
#define ETHREMOTECFG_VIRTPORT_DENORM(n)                ((EthRemoteCfg_VirtPort)((n)))

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Ethernet Firmware commands.
 *
 * The commands supported by Ethernet Firmware.  Initially, remote clients will
 * send a command request message to the server which will carry out the operation
 * and then send the command response back.  Both, the request and response
 * messages will carry one of these command ids.
 */
typedef enum EthRemoteCfg_CmdType_e
{
    /*!
     * \brief Command to offer the virtual ports queried by the remote core.
     *
     * Clients can query EthFw to provide the details regarding allocated
     * virtual switch and virtual MAC ports.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_OfferVirtPortRes
     */
    ETHREMOTECFG_CMD_VIRT_PORT_INFO,

    /*!
     * \brief Command to attach to the Ethernet device.
     *
     * All remote clients should first attach before issuing any further
     * commands to the Ethernet device.
     *
     * Request (C2S): \ref EthRemoteCfg_AttachReq
     * Response (S2C): \ref EthRemoteCfg_AttachRes
     */
    ETHREMOTECFG_CMD_ATTACH,

    /*!
     * \brief Command to attach to the Ethernet device which returns extended
     *        attach info.
     *
     * All remote clients should first attach before issuing any further
     * commands to the Ethernet device.
     *
     * Remote clients that require only a single data path can use a single
     * \ref ETHREMOTECFG_CMD_ATTACH_EXT command which will return all
     * the parameters required to establish data path including destination
     * MAC address, Rx flow, Tx channel.
     *
     * Request (C2S): \ref EthRemoteCfg_AttachReq
     * Response (S2C): \ref EthRemoteCfg_AttachExtRes
     */
    ETHREMOTECFG_CMD_ATTACH_EXT,

    /*!
     * \brief Command to detach remote client from the Ethernet device.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_ATTACH and
     * \ref ETHREMOTECFG_CMD_ATTACH_EXT. All resources allocated to the remote
     * client like Tx channel, Rx flow, MAC address are freed by the server core.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DETACH,

    /*!
     * \brief Command to query link status of a port.
     *
     * Command allows clients to query for the link status of a port.
     * Server also returns the link speed and duplexity of the virtual port.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_PortLinkStatusRes
     */
    ETHREMOTECFG_CMD_PORT_LINK_STATUS,

    /*!
     * \brief Command to allocate Tx channel.
     *
     * Client should use the allocated Tx channel as Tx DMA channel CPSW PSIL
     * destination thread id when configuring the Tx DMA channel.
     *
     * Request (C2S): \ref EthRemoteCfg_AllocTxReq
     * Response (S2C): \ref EthRemoteCfg_AllocTxRes
     */
    ETHREMOTECFG_CMD_ALLOC_TX,

    /*!
     * \brief Command to allocate Rx flow.
     *
     * Client should use the allocated Rx flow id to configure the DNA Rx flow
     * to establish Rx data flow path on remote client.
     *
     * Request (C2S): \ref EthRemoteCfg_AllocRxReq
     * Response (S2C): \ref EthRemoteCfg_AllocRxRes
     */
    ETHREMOTECFG_CMD_ALLOC_RX,

    /*!
     * \brief Command to allocate a MAC address to the client.
     *
     * Commands allows client to allocate a host port destination MAC address
     * which it can register using \ref ETHREMOTECFG_CMD_REGISTER_MAC.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_AllocMacRes
     */
    ETHREMOTECFG_CMD_ALLOC_MAC,

    /*!
     * \brief Command to free previously allocated Tx channel.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_ALLOC_TX and frees
     * the previously allocated Tx channels PSIL destination thread.
     *
     * Request (C2S): \ref EthRemoteCfg_FreeTxReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_FREE_TX,

    /*!
     * \brief Command to free previously allocated Rx flow id.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_ALLOC_RX and frees
     * the previously allocated Rx flow id.
     *
     * Request (C2S): \ref EthRemoteCfg_FreeRxReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_FREE_RX,

    /*!
     * \brief Command to free previously allocated MAC address.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_ALLOC_MAC and frees
     * the previously allocated MAC address.
     *
     * Request (C2S): \ref EthRemoteCfg_FreeMacReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
    */
    ETHREMOTECFG_CMD_FREE_MAC,

    /*!
     * \brief Command to register a destination MAC address by the remote
     *        client to a specific Rx flow id.
     *
     * Command allows remote client to register all traffic received on
     * the host port with a specific destination MAC address to be routed to
     * the given Rx flow id.
     *
     * Request (C2S): \ref EthRemoteCfg_MacAddrRxFlowReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_REGISTER_MAC,

    /*!
     * \brief Command to de-register a destination MAC address by the remote
     *        client.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_REGISTER_MAC and
     * disables the routing of traffic with given destination MAC address to
     * a specific Rx flow id.
     *
     * Once de-registered further traffic with the given destination MAC address
     * will be routed to default Rx flow.
     *
     * Request (C2S): \ref EthRemoteCfg_MacAddrRxFlowReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEREGISTER_MAC,

    /*!
     * \brief Command to register default flow routing to client.
     *
     * Commands allows remote client to receive traffic directed to the default
     * flow.  Default flow is the flow to which Ethernet device will direct
     * traffic if it does not match any classifier which has a thread id
     * configured.
     *
     * Default flow registration is possible only if no core including server
     * core has registered for the default flow.
     *
     * Request (C2S): \ref EthRemoteCfg_RxDefaultFlowRegisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW,

    /*!
     * \brief Command to de-register default flow routing to client.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW and
     * disables the routing of default flow traffic to the given Rx flow id.
     * Once default flow is de-registered, all traffic destined to default flow
     * will be routed to reserved flow and will be dropped.
     *
     * Request (C2S): \ref EthRemoteCfg_RxDefaultFlowRegisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW,

    /*!
     * \brief Command to associate IPv4 address with MAC address.
     *
     * Command allows server core to register IPv4 address with a MAC address.
     * ARP queries from external nodes will be received on host port. The
     * ARP queries can be routed to only a single core.  This is typically the
     * server core which maintains ARP database of all the IP entries: MAC
     * address mapping used by all clients in all remote cores.
     * The remote client must explicitly register its IP address:MAC address
     * with the server core using this command.
     *
     * Request (C2S): \ref EthRemoteCfg_IPv4AddrRegisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_REGISTER_IPv4,

    /*!
     * \brief Command to remove IPv4 address:MAC address mapping.
     *
     * Command removes ARP entry of IP address:MAC address mapping.
     * This is inverse operation of \ref ETHREMOTECFG_CMD_REGISTER_IPv4.
     *
     * Request (C2S): \ref EthRemoteCfg_IPv4AddrDeregisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEREGISTER_IPv4,

    /*!
     * \brief Command to join a VLAN.
     *
     * Used by remote clients to join a VLAN.  Note that the remote client
     * must be allowed in the VLAN, which is a static configuration on
     * server side.
     *
     * Request (C2S): \ref EthRemoteCfg_VlanJoinReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_JOIN_VLAN,

    /*!
     * \brief Command to leave a VLAN.
     *
     * Used by remote clients to leave VLAN previously joined through
     * \ref ETHREMOTECFG_CMD_JOIN_VLAN.
     *
     * Request (C2S): \ref EthRemoteCfg_VlanLeaveReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_LEAVE_VLAN,

    /*!
     * \brief Command to add multicast MAC address to receive filter.
     *
     * Command allows remote core client to add a multicast address to
     * receive filter.
     *
     * Server side differentiates three types of multicast addresses:
     * - Exclusive multicast: single client owns multicast traffic.
     *   Packets will be forwarded in hardware via Rx flow.
     * - Shared multicast: traffic is forwarded to multiple core with
     *   server core involvement.
     * - Reserved multicast: clients cannot add them to receive filter,
     *   reserved for server core.
     *
     * Request (C2S): \ref EthRemoteCfg_FilterMacAddReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_ADD_FILTER_MAC,

    /*!
     * \brief Command to delete multicast MAC address from receive filter.
     *
     * Command allows remote client to delete a multicast address from
     * receive filter.
     *
     * Request (C2S): \ref EthRemoteCfg_FilterMacDelReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEL_FILTER_MAC,

    /*!
     * \brief Command to setup an EtherType-based packet route.
     *
     * Commands allows remote client to register all traffic received on
     * the host port with a specific EtherType to be routed to the given Rx
     * flow id.
     *
     * Request (C2S): \ref EthRemoteCfg_MatchEthertypeAddReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_REGISTER_MATCH_ETHTYPE,

    /*!
     * \brief Command to tear-down an EtherType-based packet route.
     *
     * This is inverse operation of \ref ETHREMOTECFG_CMD_REGISTER_MATCH_ETHTYPE
     * and disables the routing of traffic with given EtherType to a specific
     * Rx flow id.
     *
     * Once unregistered further traffic with the given EtherType will be
     * routed to default Rx flow.
     *
     * Request (C2S): \ref EthRemoteCfg_MatchEthertypeDelReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEREGISTER_MATCH_ETHTYPE,

    /*!
     * \brief Command to register a remote timer with EthFw for multicore
     *        time synchronization.
     *
     * Commands allows remote core client to register a timer which it intends
     * to use for multicore time synchronization.  Master core will setup the
     * time sync router for timer periodic events towards CPTS hardware push
     * events.
     *
     * Request (C2S): \ref EthRemoteCfg_RemoteTimerRegisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_REGISTER_REMOTE_TIMER,

    /*!
     * \brief Command to de-register a remote timer with EthFw.
     *
     * Commands allows remote core client to de-register a timer used by
     * for multicore time synchronization with CPTS timer.
     *
     * Request (C2S): \ref EthRemoteCfg_RemoteTimerDeregisterReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DEREGISTER_REMOTE_TIMER,

    /*!
     * \brief Command to query the remote configuration server status.
     *
     * Command allows remote client to query what is the current status
     * of the server state machine.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_ServerStatusRes
     */
    ETHREMOTECFG_CMD_GET_SERVER_STATUS,

    /*!
     * \brief Command to notify client's DMA teardown completion.
     *
     * Command send by remote client after tearing down the DMA channels
     * in response to server notification \ref ETHREMOTECFG_NOTIFY_HWERROR.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_TEARDOWN_COMPLETION,

    /*!
     * \brief Command from remote client to Server to dump CPSW stats.
     *
     * Command to dump network statistics, forwarding table, policer table
     * and other server internal tables on master core's UART console.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_DUMP,

    /*!
     * \brief Command to allocate CPTS HW Push instance.
     *
     * Command to allocate CPTS HW Push instance.
     *
     * Request (C2S): \ref EthRemoteCfg_CommonReq
     * Response (S2C): \ref EthRemoteCfg_AllocHwPushRes
     */
    ETHREMOTECFG_CMD_ALLOC_CPTS_HW_PUSH,

    /*!
     * \brief Command to free CPTS HW Push instance.
     *
     * Command to free CPTS HW Push instance.
     *
     * Request (C2S): \ref EthRemoteCfg_FreeHwPushReq
     * Response (S2C): \ref EthRemoteCfg_StatusRes
     */
    ETHREMOTECFG_CMD_FREE_CPTS_HW_PUSH,

    /*!
     * \brief Last command id.
     */
    ETHREMOTECFG_CMD_TYPE_LAST = ETHREMOTECFG_CMD_FREE_CPTS_HW_PUSH,
} EthRemoteCfg_CmdType;

/*!
 * \brief Remote core id used by server to identify the application running
 *        on the core.
 */
typedef enum EthRemoteCfg_ClientId_e
{
    /*! No client. */
    ETHREMOTECFG_CLIENTID_NONE = 0U,

    /*! AUTOSAR client id. */
    ETHREMOTECFG_CLIENTID_AUTOSAR,

    /*! RTOS client id. */
    ETHREMOTECFG_CLIENTID_RTOS,

    /*! Linux client id. */
    ETHREMOTECFG_CLIENTID_LINUX,

    /*! QNX client id. */
    ETHREMOTECFG_CLIENTID_QNX,

    /*! Last client id. */
    ETHREMOTECFG_CLIENTID_LAST = ETHREMOTECFG_CLIENTID_QNX
} EthRemoteCfg_ClientId;

/*!
 * \brief Server-to-client (S2C) notify types.
 */
typedef enum EthRemoteCfg_NotifyType_e
{
    /*!
     * \brief Notify type for sending EthFw version info to the remote clients.
     *
     * Params (S2C): \ref EthRemoteCfg_DeviceData
     */
    ETHREMOTECFG_NOTIFY_FWINFO,

    /*!
     * \brief Notify type for sending CPTS HW push events to the remote clients.
     *
     * Params (S2C): \ref EthRemoteCfg_NotifyServiceHwPushMsg
     */
    ETHREMOTECFG_NOTIFY_HWPUSH,

    /*!
     * \brief Notify type for hardware error, typically requires a reset to recover.
     *
     * Params (S2C): \ref EthRemoteCfg_CommonNotify
     */
    ETHREMOTECFG_NOTIFY_HWERROR,

    /*!
     * \brief Notify type for hardware recovery completion.
     *
     * Params (S2C): \ref EthRemoteCfg_CommonNotify
     */
    ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE,

    /*!
     * \brief Custom notify type.
     *
     * Params (S2C): \ref EthRemoteCfg_CommonNotify
     */
    ETHREMOTECFG_NOTIFY_CUSTOM,

    /*! Last notification type. */
    ETHREMOTECFG_NOTIFY_LAST = ETHREMOTECFG_NOTIFY_CUSTOM
} EthRemoteCfg_NotifyType;

/*!
 * \brief Message type.
 */
typedef enum EthRemoteCfg_MsgType_e
{
    /*! Request message expecting an ACK/response. */
    ETHREMOTECFG_MSGTYPE_REQUEST,

    /*! Notify message which notifies the client and expects no ACK. */
    ETHREMOTECFG_MSGTYPE_NOTIFY,

    /*! Response message for the request message. */
    ETHREMOTECFG_MSGTYPE_RESPONSE
} EthRemoteCfg_MsgType;

/*!
 * \brief Ethernet Firmware server status.
 */
typedef enum EthRemoteCfg_ServerStatus_e
{
    /*! Ethernet Firmware server is not initialized. */
    ETHREMOTECFG_SERVERSTATUS_UNINIT,

    /*! Ethernet Firmware server is initialized and active. */
    ETHREMOTECFG_SERVERSTATUS_READY,

    /*! Underlying Ethernet device in under recovery. */
    ETHREMOTECFG_SERVERSTATUS_RECOVERY,

    /*! Ethernet Firmware server is in bad/crashed state. */
    ETHREMOTECFG_SERVERSTATUS_BAD
} EthRemoteCfg_ServerStatus;

/*!
 * \brief Virtual port id.
 * Each ETHREMOTECFG_MAC_PORT_i corresponds to ETHERNET PORT i.
 */
typedef enum EthRemoteCfg_VirtPort_e
{
    /*! Virtual MAC port 1. */
    ETHREMOTECFG_MAC_PORT_1 = ETHERNET_PORT1_ID,

    /*! Virtual MAC port 2. */
    ETHREMOTECFG_MAC_PORT_2,

    /*! Virtual MAC port 3. */
    ETHREMOTECFG_MAC_PORT_3,

    /*! Virtual MAC port 4. */
    ETHREMOTECFG_MAC_PORT_4,

    /*! Virtual MAC port 5. */
    ETHREMOTECFG_MAC_PORT_5,

    /*! Virtual MAC port 6. */
    ETHREMOTECFG_MAC_PORT_6,

    /*! Virtual MAC port 7. */
    ETHREMOTECFG_MAC_PORT_7,

    /*! Virtual MAC port 8. */
    ETHREMOTECFG_MAC_PORT_8,

    /*! Last virtual MAC port id. */
    ETHREMOTECFG_MAC_PORT_LAST = ETHREMOTECFG_MAC_PORT_8,

    /*! First virtual switch port id*/
    ETHREMOTECFG_SWITCH_PORT_FIRST = ETHREMOTECFG_SWITCH_PORT_FIRST_ID,

    /*! Virtual switch port 0. */
    ETHREMOTECFG_SWITCH_PORT_0 = ETHREMOTECFG_SWITCH_PORT_FIRST,

    /*! Virtual switch port 1. */
    ETHREMOTECFG_SWITCH_PORT_1,

    /*! Virtual switch port 2. */
    ETHREMOTECFG_SWITCH_PORT_2,

    /*! Virtual switch port 3. */
    ETHREMOTECFG_SWITCH_PORT_3,

    /*! Last virtual switch port id. */
    ETHREMOTECFG_SWITCH_PORT_LAST = ETHREMOTECFG_SWITCH_PORT_3,

} EthRemoteCfg_VirtPort;

/*! @} */

/*!
 * \brief Message header structure.
 *
 * Header structure for any type of messages: request, response and
 * notify.
 */
typedef struct EthRemoteCfg_MsgHdr_s
{
    /*! Unique token to identify all operations related to this client. */
    uint32_t token;

    /*! Unique client identity for all remote core applications defined
     *  by \ref EthRemoteCfg_ClientId. */
    uint32_t clientId;

    /*! Describes if it is a request, response or notify message. */
    uint32_t msgType;
} __attribute__((packed)) EthRemoteCfg_MsgHdr;

/*!
 * \brief Command request message structure.
 *
 * Structure of every request message which consists of common message header,
 * command id (`reqType`) and a request id (`reqId`).
 */
typedef struct EthRemoteCfg_ReqHdr_s
{
    /*! Common message header containing details about the remote client. */
    EthRemoteCfg_MsgHdr common;

    /*! Request type used to denote the type of request message. */
    uint32_t reqType;

    /*! Request id used to compare with respone id, starts from zero and
     *  increments for each request. */
    uint32_t reqId;
} __attribute__((packed)) EthRemoteCfg_ReqHdr;

/*!
 * \brief Command response message structure.
 *
 * Structure of every command response message, which consists of common
 * message header, command id (`resType`), response id (`resId`) and
 * command status.
 */
typedef struct EthRemoteCfg_ResHdr_s
{
    /*! Common message header containing details about the remote client. */
    EthRemoteCfg_MsgHdr common;

    /*! Response type used to denote the type of respose message. */
    uint32_t resType;

    /*! Response id used to compare with the request id. */
    uint32_t resId;

    /*! Status of the request. */
    int32_t status;
} __attribute__((packed)) EthRemoteCfg_ResHdr;

/*!
 * \brief Notify message structure.
 *
 * Structure of every notify message and consists of common message header
 * \ref EthRemoteCfg_MsgHdr and the notification type (id).
 */
typedef struct EthRemoteCfg_NotifyHdr_s
{
    /*! Common message header containing details about the remote client. */
    EthRemoteCfg_MsgHdr common;

    /*! Notify type used to denote the type of notify message. */
    uint32_t notifyType;
} __attribute__((packed)) EthRemoteCfg_NotifyHdr;

/*!
 * \brief Common request message structure.
 *
 * Common message structure for all requests which carry only request header.
 */
typedef struct EthRemoteCfg_CommonReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;
} __attribute__((packed)) EthRemoteCfg_CommonReq;

/*!
 * \brief Common response message structure.
 *
 * Most of the response messages carry the status of the corresponding
 * request message. This structure defines the generic response message,
 */
typedef struct EthRemoteCfg_StatusRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;
} __attribute__((packed)) EthRemoteCfg_StatusRes;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_VIRT_PORT_INFO command.
 *
 * Virtual port allocation sends the info about the virtual ports assigned
 * to a given remote core.
 */
typedef struct EthRemoteCfg_OfferVirtPortRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Port mask denoting absolute virtual switch ports allocated. */
    uint32_t switchPortMask;

    /*! Port mask indicating which virtual MAC ports are allocated.
    * Each virtual MAC port directly corresponds to a Ethernet port.

    *   +--------------+----------------------------+---------------+
    *   | Bit Position |        Virtual Port        | Ethernet Port |
    *   +--------------+----------------------------+---------------+
    *   | 0            | Not Defined                | Not Defined   |
    *   | 1            | ETHREMOTECFG_MAC_PORT_1    | PORT1         |
    *   | 2            | ETHREMOTECFG_MAC_PORT_2    | PORT2         |
    *   | 3            | ETHREMOTECFG_MAC_PORT_3    | PORT3         |
    *   | 4            | ETHREMOTECFG_MAC_PORT_4    | PORT4         |
    *   | 5            | ETHREMOTECFG_MAC_PORT_5    | PORT5         |
    *   | N            | ETHREMOTECFG_MAC_PORT_N    | PORTN         |
    *   +--------------+----------------------------+---------------+

    */
    uint32_t macPortMask;
} __attribute__((packed)) EthRemoteCfg_OfferVirtPortRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_ATTACH or
 *        \ref ETHREMOTECFG_CMD_ATTACH_EXT commands.
 *
 * This requests provides the virtual port for which it is requesting the core
 * attach and recevies and handle called token which will be further used by
 * the client to communicate with the server.  Each token is unique for a
 * given virtual port.
 */
typedef struct EthRemoteCfg_AttachReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Virtual port which needs core attach. */
    uint32_t virtPort;
} __attribute__((packed)) EthRemoteCfg_AttachReq;

/*!
 * \brief Response params holding data for \ref ETHREMOTECFG_CMD_ATTACH request.
 *
 * This response provides the token handle for the requested virtual port which
 * will be placed in the \ref EthRemoteCfg_ResHdr and MTU info of Tx and Rx.
 */
typedef struct EthRemoteCfg_AttachRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Maximum receive unit. */
    uint32_t rxMtu;

    /*! Maximum transmit unit. */
    uint32_t txMtu;

    /*! Feature bitmask based on defines \ref EthRemoteCfg_FeatureMask. */
    uint32_t features;

    /*! Number of tx channel allocated for this virtual port */
    uint32_t numTxCh;

    /*! Number of rx flows allocated for this virtual port */
    uint32_t numRxFlow;
} __attribute__((packed)) EthRemoteCfg_AttachRes;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_ATTACH_EXT command.
 *
 * This response provides additional information to that of
 * \ref EthRemoteCfg_AttachRes. Additionally it provides the allocated RX flow
 * ids, Tx channel id and MAC address.
 */
typedef struct EthRemoteCfg_AttachExtRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Maximum receive unit. */
    uint32_t rxMtu;

    /*! Maximum transmit unit. */
    uint32_t txMtu;

    /*! Allocated Rx flow index base. */
    uint32_t rxFlowIdxBase;

    /*! Allocated flow index offset. */
    uint32_t rxFlowIdxOffset;

    /*! Tx PSIL Peer destination thread id which should be paired with the
     *  Tx UDMA channel. */
    uint32_t txPsilDstId;

    /*! MAC address allocated. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! Feature bitmask based on \ref EthRemoteCfg_FeatureMask. */
    uint32_t features;

    /*! Rx PSIL peer source thread id */
    uint32_t rxPsilSrcId;
} __attribute__((packed)) EthRemoteCfg_AttachExtRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_ALLOC_RX command.
 *
 * This request provides the rx flow to the client which needs to be allocated.
 * FlowIdx 0U being given to default flow and any custom policers cannot be
 * created for it.
 */
typedef struct EthRemoteCfg_AllocRxReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Relative index of RX flow among available numRxFlow */
    uint32_t flowIdx;
} __attribute__((packed)) EthRemoteCfg_AllocRxReq;

/*!
 * \brief Response params \ref ETHREMOTECFG_CMD_ALLOC_RX command.
 *
 * This response holds the information of the  allocated Rx flow which is
 * `rxFlowIdxbase` and `rxFlowIdxoffset` for the given virtual port.
 */
typedef struct EthRemoteCfg_AllocRxRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Allocated Rx flow index base. */
    uint32_t rxFlowIdxBase;

    /*! Allocated flow index offset. */
    uint32_t rxFlowIdxOffset;

    /*! Rx PSIL peer source thread id */
    uint32_t rxPsilSrcId;
} __attribute__((packed)) EthRemoteCfg_AllocRxRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_ALLOC_TX command.
 *
 * This request provides the tx channel priority (relative) by client
 * which needs to be allocated.
 */
typedef struct EthRemoteCfg_AllocTxReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Tx channel priority (relative) which client want to allocate */
    uint32_t chRelPriority;
} __attribute__((packed)) EthRemoteCfg_AllocTxReq;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_ALLOC_TX command.
 *
 * This response holds the information of the allocated Tx channel which is
 * PSIL channel id for the given virtual port.
 */
typedef struct EthRemoteCfg_AllocTxRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Tx PSIL peer destination thread id which should be paired with the
     *  Tx UDMA channel. */
    uint32_t txPsilDstId;
} __attribute__((packed)) EthRemoteCfg_AllocTxRes;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_ALLOC_MAC command.
 *
 * This response holds the information of the allocated MAC address for the
 * given virtual port.
 */
typedef struct EthRemoteCfg_AllocMacRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Allocated MAC address. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
} __attribute__((packed)) EthRemoteCfg_AllocMacRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_FREE_RX command.
 *
 * This request provides the rxflowIdxBase and rxFlowIdxoffset of the rxflowIdx
 * which needs to be freed.
 */
typedef struct EthRemoteCfg_FreeRxReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Rx flow index base. */
    uint32_t rxFlowIdxBase;

    /*! Rx flow index offset. */
    uint32_t rxFlowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_FreeRxReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_FREE_TX command.
 *
 * This request provides the TX PSIL destination id of the Tx channel which
 * needs to be freed.
 */
typedef struct EthRemoteCfg_FreeTxReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Tx PSIL Peer destination thread id to be freed. */
    uint32_t txPsilDstId;
} __attribute__((packed)) EthRemoteCfg_FreeTxReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_FREE_MAC command.
 *
 * This request provides the MAC address which needs to be freed.
 */
typedef struct EthRemoteCfg_FreeMacReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! MAC address to be freed. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
} __attribute__((packed)) EthRemoteCfg_FreeMacReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_REGISTER_MAC or
 *        \ref ETHREMOTECFG_CMD_DEREGISTER_MAC commands.
 *
 * This structure holds the MAC address and the RxflowId which needs to be
 * registered for a given virtual port.
 */
typedef struct EthRemoteCfg_MacAddrRxFlowReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! MAC address which needs to be registered. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! Rx flow index base. */
    uint32_t flowIdxBase;

    /*! Rx flow index offset to which the MAC address needs to be registered. */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_MacAddrRxFlowReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_REGISTER_IPv4 command.
 *
 * This structure holds IPv4 address-MAC address pair which needs to be added
 * to the proxy ARP table of the server.
 */
typedef struct EthRemoteCfg_IPv4AddrRegisterReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! IP address. */
    uint8_t ipAddr[ETHREMOTECFG_IPV4ADDRLEN];

    /*! MAC address associated with the IP address which should be added to
     *  the ARP table. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
} __attribute__((packed)) EthRemoteCfg_IPv4AddrRegisterReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_DEREGISTER_IPv4 command.
 *
 * This structure holds IPv4 address which will be used as a key to
 * remove the entry from the proxy ARP table of the server.
 */
typedef struct EthRemoteCfg_IPv4AddrDeregisterReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! IP address. */
    uint8_t ipAddr[ETHREMOTECFG_IPV4ADDRLEN];
} __attribute__((packed)) EthRemoteCfg_IPv4AddrDeregisterReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_JOIN_VLAN command.
 *
 * Parameters provided by remote clients to join a VLAN previously setup by
 * Ethernet Firmware.  The remote client must be allowed to join the requested
 * \ref EthRemoteCfg_VlanJoinReq::vlanId for the request to succeed.
 * If allowed, VLAN traffic will be routed to the RX flow provided by the client.
 */
typedef struct EthRemoteCfg_VlanJoinReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! VLAN id. */
    uint16_t vlanId;

    /*! MAC address. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! RX flow index base. */
    uint32_t flowIdxBase;

    /*! RX flow index offset to route unicast VLAN packets to. */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_VlanJoinReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_LEAVE_VLAN command.
 *
 * Parameters provided by remote clients to leave a VLAN previously joined using
 * \ref ETHREMOTECFG_CMD_JOIN_VLAN command.  After successfully leaving the VLAN,
 * VLAN traffic will stop on the provided RX flow.
 */
typedef struct EthRemoteCfg_VlanLeaveReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! VLAN id. */
    uint16_t vlanId;

    /*! MAC address. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! RX flow index base. */
    uint32_t flowIdxBase;

    /*! RX flow index offset to route unicast VLAN packets to. */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_VlanLeaveReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW or
 *        \ref ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW commands.
 *
 * This structure holds info about the rxFlowIdx which needs to be registered
 * or de-registred as default flow.
 */
typedef struct EthRemoteCfg_RxDefaultFlowRegisterReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Rx flow index base. */
    uint32_t flowIdxBase;

    /*! Rx flow index offset */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_RxDefaultFlowRegisterReq;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_PORT_LINK_STATUS command.
 *
 * Response to a periodic request sent by the client for a given virtual port
 * where the server sends back the link status and the config params
 * (link speed, duplexity) of the port.
 */
typedef struct EthRemoteCfg_PortLinkStatusRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Link status of the port. */
    bool isLinked;

    /*! Link speed. */
    uint32_t speed;

    /*! Duplexity. */
    uint32_t duplexity;
} __attribute__((packed)) EthRemoteCfg_PortLinkStatusRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_REGISTER_MATCH_ETHTYPE command.
 *
 * Request message structure for setting an Ethertype classifier for a given
 * Rx flow, hence takes EtherType and the RX flow (base and offset) values as
 * input arguments.
 */
typedef struct EthRemoteCfg_MatchEthertypeAddReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! EtherType to be associated with the flow. */
    uint16_t ethertype;

    /*! Rx flow index base. */
    uint32_t flowIdxBase;

    /*! Rx flow index offset. */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_MatchEthertypeAddReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_DEREGISTER_MATCH_ETHTYPE command.
 *
 * Request message structure for deleting the EtherType classifier which has
 * been set for a given Rx flow.
 */
typedef struct EthRemoteCfg_MatchEthertypeDelReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! EtherType to be associated with the flow. */
    uint16_t ethertype;
} __attribute__((packed)) EthRemoteCfg_MatchEthertypeDelReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_ADD_FILTER_MAC command.
 *
 * This structure holds vlanId, MAC address and Rx flowIdx details used for
 * adding multicast MAC address to the receive filter.
 */
typedef struct EthRemoteCfg_FilterMacAddReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Multicast MAC address to be added. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! VLAN id. */
    uint16_t vlanId;

    /*! Rx flow index base from which the MAC_address association will be added.
     *  It's applicable only for _exclusive multicast traffic_. */
    uint32_t flowIdxBase;

    /*! Rx flow index offset. */
    uint32_t flowIdxOffset;
} __attribute__((packed)) EthRemoteCfg_FilterMacAddReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_DEL_FILTER_MAC command.
 *
 * This structure holds vlanId, MAC address which will be used for deleting
 * multicast MAC address from the receive filter.
 */
typedef struct EthRemoteCfg_FilterMacDelReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Multicast MAC address to be added. */
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];

    /*! VLAN id. */
    uint16_t vlanId;
} __attribute__((packed)) EthRemoteCfg_FilterMacDelReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_REGISTER_REMOTE_TIMER command.
 *
 * This request structure contains hardware push number and timerId for
 * registering HWPUSH callback and setting TimeSyncRouter.
 */
typedef struct EthRemoteCfg_RemoteTimerRegisterReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Hardware Push number to be used for timesync router configuration. */
    uint8_t hwPushNum;

    /*! Timer id to be used for timesync router configuration. */
    uint8_t timerId;
} __attribute__((packed)) EthRemoteCfg_RemoteTimerRegisterReq;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_DEREGISTER_REMOTE_TIMER command.
 *
 * This request structure contains hardware push number de-registering HWPUSH
 * callback and clearing TimeSyncRouter.
 */
typedef struct EthRemoteCfg_RemoteTimerDeregisterReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! Hardware Push number to be used for timesync router configuration. */
    uint8_t hwPushNum;
} __attribute__((packed)) EthRemoteCfg_RemoteTimerDeregisterReq;

/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_GET_SERVER_STATUS command.
 *
 * This response returns the status of Ethernet Firmware server, see
 * \ref EthRemoteCfg_ServerStatus.
 */
typedef struct EthRemoteCfg_ServerStatusRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! Server status. */
    EthRemoteCfg_ServerStatus status;
} __attribute__((packed)) EthRemoteCfg_ServerStatusRes;

/*!
 * \brief Notify params for \ref ETHREMOTECFG_NOTIFY_HWPUSH notification.
 *
 * This is S2C notify message which notifies the remote client everytime there
 * is a hardware push notification with a event timestamp for a given
 * CPTS HW push number.
 */
typedef struct EthRemoteCfg_NotifyServiceHwPushMsg_s
{
    /*! Common notify message header. */
    EthRemoteCfg_NotifyHdr hdr;

    /*! CPTS hardware push number. */
    uint32_t hwPushNum;

    /*! CPTS hardware push event timestamp. */
    uint64_t timeStamp;
} __attribute__((packed)) EthRemoteCfg_NotifyServiceHwPushMsg;

/*!
 * \brief Common notification message.
 *
 * Common message structure for all notifications which carry only notify header.
 */
typedef struct EthRemoteCfg_CommonNotify_s
{
    /*! Notify message common header. */
    EthRemoteCfg_NotifyHdr hdr;
} __attribute__((packed)) EthRemoteCfg_CommonNotify;


/*!
 * \brief Response params for \ref ETHREMOTECFG_CMD_ALLOC_CPTS_HW_PUSH command.
 *
 * This response holds the information of the allocated CPTS HW push instance.
 */
typedef struct EthRemoteCfg_AllocHwPushRes_s
{
    /*! Response message common header. */
    EthRemoteCfg_ResHdr hdr;

    /*! CPTS hardware push number. */
    uint32_t hwPushNum;
} __attribute__((packed)) EthRemoteCfg_AllocHwPushRes;

/*!
 * \brief Request params for \ref ETHREMOTECFG_CMD_FREE_CPTS_HW_PUSH command.
 *
 * This request structure contains information of the CPTS HW push instance to be freed.
 */
typedef struct EthRemoteCfg_FreeHwPushReq_s
{
    /*! Request message common header. */
    EthRemoteCfg_ReqHdr hdr;

    /*! CPTS hardware push number. */
    uint32_t hwPushNum;
} __attribute__((packed)) EthRemoteCfg_FreeHwPushReq;

/*!
 * \brief Firmware version info.
 */
typedef struct EthRemoteCfg_FwVerInfo_s
{
    /*! Ethernet Firmware API Major version number. */
    uint32_t major;

    /*! Ethernet Firmware API Minor version number. */
    uint32_t minor;

    /*! Ethernet Firmware API Revision version number. */
    uint32_t rev;

    /*! Ethernet Firmware build year: string in the form YYYY eg: 2019. */
    char year[ETHREMOTECFG_FWDATE_YEARLEN];

    /*! Ethernet Firmware build month: string in the form MON eg: Dec. */
    char month[ETHREMOTECFG_FWDATE_MONTHLEN];

    /*! Ethernet Firmware build month: string in the form DD eg: 12. */
    char date[ETHREMOTECFG_FWDATE_DATELEN];

    /*! GIT commit SHA of the firmware: char string in the form fd52c34f. */
    char commitHash[ETHREMOTECFG_FW_COMMITSHALEN];
} __attribute__((packed)) EthRemoteCfg_FwVerInfo;

/*!
 * \brief Ethernet device data.
 *
 * The framework will return the below device data to the remote client.
 */
typedef struct EthRemoteCfg_DeviceData_s
{
    /*! Notify message common header. */
    EthRemoteCfg_NotifyHdr hdr;

    /*! Ethernet device Firmware version info. */
    EthRemoteCfg_FwVerInfo fwVer;

    /*! Flag indicating permission enabled for each command for the
     *  connecting client. */
    uint64_t permissionFlags;

    /*! Flag indicating if UART is connected: 1 indicates UART connected,
     *  0 indicates UART not connected. */
    uint32_t uartConnected;

    /*! UART ID used by firmware for log prints. */
    uint32_t uartId;
} __attribute__((packed)) EthRemoteCfg_DeviceData;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHREMOTECFG_H_ */
