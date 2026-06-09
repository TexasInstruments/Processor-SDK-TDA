/*
 *
 * Copyright (c) 2019-2024 Texas Instruments Incorporated
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

#ifndef __CPSWPROXY_H__
#define __CPSWPROXY_H__

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ethremotecfg/protocol/ethremotecfg.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup ETHFW_CLIENT Ethernet Firmware Proxy Client APIs
 *
 * This section contains APIs for CPSW Proxy Client.  The CPSW Proxy Client
 * resides on the remote cores and enables clients on remote cores to configure
 * the Ethernet device via RPC.
 *
 * CPSW Proxy Client implements a subset of the commands and notifications
 * supported by Ethernet Firmware Remote Configuration interface described
 * in \ref ETHFW_ETHREMOTECFG.
 *
 * CPSW Proxy Client is currently used by remote clients running RTOS.  It
 * could be repurposed for other OSes, but may or may not require porting.
 *
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_CLIENT
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \anchor CpswProxy_ErrorCodes
 * \name CpswProxy Error Codes
 *
 * Error codes returned by the CPSW Proxy client APIs.
 *
 * @{
 */

/* Below error codes are aligned with CSL's and Enet LLD's to maintain
 * consistency and avoid error code conversion */

/*! \brief Success. */
#define CPSWPROXY_SOK                              (0)

/*! \brief Operation in progress. */
#define CPSWPROXY_SINPROGRESS                      (1)

/*! \brief Generic failure error condition (typically caused by hardware). */
#define CPSWPROXY_EFAIL                            (-1)

/*! \brief Bad arguments (i.e. NULL pointer). */
#define CPSWPROXY_EBADARGS                         (-2)

/*! \brief Invalid parameters (i.e. value out-of-range). */
#define CPSWPROXY_EINVALIDPARAMS                   (-3)

/*! \brief Time out while waiting for a given condition to happen. */
#define CPSWPROXY_ETIMEOUT                         (-4)

/*! \brief Allocation failure. */
#define CPSWPROXY_EALLOC                           (-8)

/*! \brief Unexpected condition occurred (sometimes unrecoverable). */
#define CPSWPROXY_EUNEXPECTED                      (-9)

/*! \brief The resource is currently busy performing an operation. */
#define CPSWPROXY_EBUSY                            (-10)

/*! \brief Already open error. */
#define CPSWPROXY_EALREADYOPEN                     (-11)

/*! \brief Operation not permitted. */
#define CPSWPROXY_EPERM                            (-12)

/*! \brief Operation not supported. */
#define CPSWPROXY_ENOTSUPPORTED                    (-13)

/*! \brief Resource not found. */
#define CPSWPROXY_ENOTFOUND                        (-14)

/*! \brief Unknown IOCTL. */
#define CPSWPROXY_EUNKNOWNIOCTL                    (-15)

/*! \brief Malformed IOCTL (args pointer or size not as expected). */
#define CPSWPROXY_EMALFORMEDIOCTL                  (-16)

/*! @} */

/*! Heartbeat default period. */
#define CPSWPROXY_HB_DEFAULT_POLL_PERIOD_MS        (1000U)

/*! Command response default timeout. */
#define CPSWPROXY_CMD_DEFAULT_TIMEOUT_MS           (1000U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief CPSW remote notification callback
 *
 * Clients can register a callback function of this type to get notified of
 * events on the Ethernet Firmware remote side.
 *
 * \param notifyType  Notification type, see \ref EthRemoteCfg_NotifyType.
 * \param notifyArg   Notification argument, specific to the notification type.
 * \param cbArg       Callback argument passed at the time of registration.
 */
typedef void (*CpswProxy_NotifyCbFxn)(uint32_t notifyType,
                                      void *notifyArg,
                                      void *cbArg);

/*!
 * \brief CPSW Proxy Heartbeat notification callback
 *
 * Clients can register a callback function of this type to get notified of
 * status of the Ethernet Firmware remote side.
 *
 * \param serverStatus    Status of ETHFW server.
 * \param cbArg           Callback argument passed at the time of registration.
 */
typedef void (*CpswProxy_HeartbeatCbFxn)(EthRemoteCfg_ServerStatus serverStatus,
                                         void *cbArg);

/*!
 * \brief Cpsw Proxy client configuration structure
 */
typedef struct CpswProxy_Config_s
{
    /*! Virtual port id */
    EthRemoteCfg_VirtPort virtPort;
} CpswProxy_Config;

/*!
 * \brief Cpsw Proxy Heartbeat Callback info.
 */
typedef struct CpswProxy_HeartbeatCb_s
{
    /*! Callback function */
    CpswProxy_HeartbeatCbFxn cbFxn;

    /*! Heartbeat argument */
    void *cbArg;
} CpswProxy_HeartbeatCb;

/*!
 * \brief Cpsw Proxy TimeSync Callback info.
 */
typedef struct CpswProxy_TimeSyncCb_s
{
    /*! Callback function */
    CpswProxy_NotifyCbFxn cbFxn;

    /*! TimeSync function argument */
    void *cbArg;
} CpswProxy_TimeSyncCb;

/*!
 * \brief Cpsw Proxy init params structure
 */
typedef struct CpswProxy_initParams_s
{
    /*! Heartbeat period in milliseconds */
    uint32_t hbPeriodInMsecs;

    /*! Command response timeout in milliseconds, applicable to all commands sent by
     * CPSW Proxy.
     * Proxy Client API functions will return \ref CPSWPROXY_ETIMEOUT if no response 
     * was received from server within this period. */
    uint32_t cmdTimeoutInMsecs;

    /* Heartbeat callback function */
    CpswProxy_HeartbeatCb hbNotifyCb;

    /* TimeSync callback function */
    CpswProxy_TimeSyncCb tsNotifyCb;
} CpswProxy_initParams;

/*!
 * \brief CPSW Proxy handle
 *
 * CPSW Proxy opaque handle.
 */
typedef struct CpswProxy_ClientObj_s *CpswProxy_Handle;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief CPSW Proxy init configuration parameters.
 *
 * Sets the CPSW Proxy configuration parameters. It needs to be called
 * only once per core and it is optional function that app should call, but not mandatory.
 *
 * \param params    Init configuration params.
 * 
 */
void CpswProxy_initConfig(CpswProxy_initParams *params);

/*!
 * \brief Initialize CPSW Proxy on a given core
 *
 * Performs one-time initialization of the CPSW Proxy layer. It needs to be called
 * only once per core and it is mandatory function that app should call.
 * 
 * \param params    Init configuration params.
 * 
 * \returns \ref CPSWPROXY_SOK if initialization was successful, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_init(const CpswProxy_initParams *params);

/*!
 * \brief De-initialize CPSW Proxy on a given core
 *
 * Performs one-time de-initialization of the CPSW Proxy layer. It needs to be called
 * only once per core and it must be the very last CpswProxy API to be called.
 */
void CpswProxy_deinit(void);

/*!
 * \brief Connect to the Cpsw Proxy server
 *
 * Connect with the server side if it has been initialized. Client side then continues
 * its initialization steps which are dependent on server.
 *
 * Application should call this function until connection is established before calling
 * \ref CpswProxy_open() or any other CpswProxy API.
 *
 * \returns \ref CPSWPROXY_SOK if able to connect to CPSW Proxy server, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_connect(void);

/*!
 * \brief Open CPSW proxy client instance with the given configuration
 *
 * Application will get a handle to Cpsw Proxy which will be used in all CPSW
 * Proxy APIs. Each virtPort will get a proxy handle, which will be used in
 * CPSW Proxy APIs.
 *
 * \param cfg    Configuration of the CPSW Proxy client.
 *
 * \returns      Cpsw Proxy Handle which will be used in all Cpsw Proxy APIs.
 *               NULL value indicates \ref CpswProxy_open() failed.
 */
CpswProxy_Handle CpswProxy_open(const CpswProxy_Config *cfg);

/*!
 * \brief Close CPSW proxy client.
 *
 * Close and free the CPSW proxy client instance.
 *
 * \param hProxy   Cpsw Proxy Handle
 */
void CpswProxy_close(CpswProxy_Handle hProxy);

/*!
 * \brief Attach to Ethernet device.
 *
 * This is the first function to be called by a client after the connection with
 * the remote Ethernet device has been established via \ref CpswProxy_open().
 *
 * Alternatively, clients that require a single Rx and single Tx channel could
 * use \ref CpswProxy_attachExtended() which provides the same functionality as
 * this function, but it also performs allocation of one Tx channel, one Rx
 * flow and one MAC address.
 *
 * \param hProxy      Handle to Cpsw Proxy.
 * \param virtPort    Virtual port id to attach to.
 * \param rxMtu       Pointer to maximum receive packet length. Populated by
 *                    this function.
 * \param txMtu       Array of maximum transmit packet length per priority
 *                    supported by the Ethernet device.
 * \param pNumTxCh    Pointer to number of tx channels allocated to the virtual port
 * 
 * \param pNumRxFlow  Pointer to number of rx flows allocated to the virtual port
 * \param features    Supported features flag (see \ref EthRemoteCfg_FeatureMask)
 *
 * \returns \ref CPSWPROXY_SOK if client has been successfully attached, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_attach(CpswProxy_Handle hProxy,
                         EthRemoteCfg_VirtPort virtPort,
                         uint32_t *rxMtu,
                         uint32_t *txMtu,
                         uint32_t *pNumTxCh,
                         uint32_t *pNumRxFlow,
                         uint32_t *features);

/*!
 * \brief Attach to Ethernet device with extended response.
 *
 * This is the first function to be called by a client after the connection with
 * the remote Ethernet device has been established via \ref CpswProxy_open().
 *
 * It's an alternative option to \ref CpswProxy_attach() that can be used in
 * clients that require a single Rx and single Tx channel.  In addition to
 * attaching to the remote Ethernet device, this function also allocates one
 * Tx channel, one Rx flow and one MAC address.
 *
 * \param hProxy          Handle to Cpsw Proxy.
 * \param virtPort        Virtual port id to attach to.
 * \param rxMtu           Pointer to maximum receive packet length. Populated by
                          this function.
 * \param txMtu           Array of maximum transmit packet length per priority
 *                        supported by Ethernet device.
 * \param txPSILThreadId  Pointer to allocated Tx Channel CPSW PSIL destination
 *                        thread id populated by this function.
 * \param rxFlowIdxBase   Pointer to RX flow index base value.
 * \param rxFlowIdxOffset Pointer to RX flow offset to be allocated.
 * \param macAddr         Pointer to allocated destination MAC address allocated
 *                        to remote core populated by this function.
 * \param features        Supported features flag (see \ref EthRemoteCfg_FeatureMask)
 *
 * \returns \ref CPSWPROXY_SOK if client has been successfully attached, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_attachExtended(CpswProxy_Handle hProxy,
                                 EthRemoteCfg_VirtPort virtPort,
                                 uint32_t *rxMtu,
                                 uint32_t *txMtu,
                                 uint32_t *txPSILThreadId,
                                 uint32_t *rxFlowIdxBase,
                                 uint32_t *rxFlowIdxOffset,
                                 uint8_t *macAddr,
                                 uint32_t *features);

/*!
 * \brief Detach from Ethernet device.
 *
 * Detaches the local client from remote Ethernet device.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 *
 * \returns \ref CPSWPROXY_SOK if client has been detached, 
 *          \ref CPSWPROXY_EINVALIDPARAMS error in case a client
 *          makes a detach call before attaching itself,
 *          or negative error in case of a failure, 
 *          see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_detach(CpswProxy_Handle hProxy);

/*!
 * \brief Allocate Tx channel.
 *
 * Allocates a TX channel to be used exclusively by this client.  The channel
 * is returned as a Tx PSIL destination thread id which client can use to
 * configure the DMA channel.
 *
 * \param hProxy         Handle to Cpsw Proxy.
 * \param txPSILThreadId Allocated Tx channel CPSW PSIL destination thread id,
 *                       populated by this function.
 * \param chRelPriority  Tx channel priority (relative) which client want to allocate
 *
 * \returns \ref CPSWPROXY_SOK if TX channel has been successfully allocated, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_allocTxCh(CpswProxy_Handle hProxy,
                            uint32_t *txPSILThreadId,
                            uint32_t chRelPriority);

/*!
 * \brief Free Tx channel.
 *
 * Deallocates a TX channel previously allocated through \ref CpswProxy_allocTxCh()
 * or \ref CpswProxy_attachExtended().
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param txChNum   Tx channel CPSW PSIL destination thread id to be freed.
 *
 * \returns \ref CPSWPROXY_SOK if TX channel has been freed, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_freeTxCh(CpswProxy_Handle hProxy,
                           uint32_t txChNum);

/*!
 * \brief Alloc Rx flow.
 *
 * Allocates a Rx flow to be used exclusively by this client.  The Rx flow idx is
 * returned as a pair of base and offset values.  The absolute flow idx is:
 * `rxFlowIdxBase + rxFlowIdxOffset`.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param rxFlowIdxBase    Pointer to RX flow index base value.
 * \param rxFlowIdxOffset  Pointer to RX flow offset to be allocated.
 * \param flowIdx          Relative index of rx flow among available numRxFlow
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been successfully allocated, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_allocRxFlow(CpswProxy_Handle hProxy,
                              uint32_t *rxFlowIdxBase,
                              uint32_t *rxFlowIdxOffset,
                              uint32_t flowIdx);

/*!
 * \brief Free Rx flow.
 *
 * Frees a Rx flow previously allocated through \ref CpswProxy_allocRxFlow() or
 * \ref CpswProxy_attachExtended().
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param rxFlowIdxBase    RX flow index base value.
 * \param rxFlowIdxOffset  RX flow offset to be freed.
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been freed, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_freeRxFlow(CpswProxy_Handle hProxy,
                             uint32_t rxFlowIdxBase,
                             uint32_t rxFlowIdxOffset);

/*!
 * \brief Allocate MAC address.
 *
 * Allocates a MAC address from server's MAC address pool.  Client may choose to
 * use a locally allocated MAC address instead, in which case this function is
 * not required.
 *
 * \param hProxy     Handle to Cpsw Proxy
 * \param macAddr    Pointer to MAC address to be allocated.
 *
 * \returns \ref CPSWPROXY_SOK if MAC address has been successfully allocated,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_allocMac(CpswProxy_Handle hProxy,
                           uint8_t *macAddr);

/*!
 * \brief Free MAC address.
 *
 * Frees a MAC address previously allocated through \ref CpswProxy_allocMac() or
 * \ref CpswProxy_attachExtended().
 *
 * \param hProxy     Handle to Cpsw Proxy.
 * \param macAddr    MAC address to be freed.
 *
 * \returns \ref CPSWPROXY_SOK if MAC address has been freed, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_freeMac(CpswProxy_Handle hProxy,
                          const uint8_t *macAddr);

/*!
 * \brief Register destination MAC address with the given flow index.
 *
 * Registers the destination MAC to the given Rx flow index.  This causes all
 * packets with the given destination MAC address to be routed to the given
 * Rx flow index (flowIdxBase + flowIdxOffset).
 *
 * RX flow idx must be set up prior to calling this function.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param macAddr          Destination MAC address to be registered.
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been registered, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerDstMacRxFlow(CpswProxy_Handle hProxy,
                                       uint32_t flowIdxBase,
                                       uint32_t flowIdxOffset,
                                       const uint8_t *macAddr);

/*!
 * \brief Unregister destination MAC address from the given flow index.
 *
 * Unregister a destination MAC address previously registered via
 * \ref CpswProxy_registerDstMacRxFlow(). Upon successful unregistration,
 * packets with the provided MAC address will no longer be routed to
 * RX flow index (flowIdxBase + flowIdxBase).
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param macAddr          Destination MAC address to be unregistered.
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been unregistered, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterDstMacRxFlow(CpswProxy_Handle hProxy,
                                         uint32_t flowIdxBase,
                                         uint32_t flowIdxOffset,
                                         const uint8_t *macAddr);

/*!
 * \brief Register EtherType to the given Rx flow id.
 *
 * Registers an EtherType-based route to the provided flow idx.
 *
 * RX flow idx must be set up prior to calling this function.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param etherType        EtherType to be associated with the Rx flow idx.
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been registered, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerEthertypeRxFlow(CpswProxy_Handle hProxy,
                                          uint32_t flowIdxBase,
                                          uint32_t flowIdxOffset,
                                          uint16_t etherType);

/*!
 * \brief Unregister the given EtherType to the given rx flow id
 *
 * Unregisters the EtherType-based route.  Upon successful unregistration,
 * packets with the given EtherType will be routed to the default flow.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param etherType Ethertype to be disassociated from the given Rx flow idx.
 *
 * \returns \ref CPSWPROXY_SOK if RX flow has been unregistered, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterEthertypeRxFlow(CpswProxy_Handle hProxy,
                                            uint16_t etherType);

/*!
 * \brief Register default flow to the given flow index.
 *
 * Enables routing of default traffic (traffic not matching any classifier with
 * thread id configured) to the given Rx flowIdx.
 *
 * RX flow idx must be set up prior to calling this function.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 *
 * \returns \ref CPSWPROXY_SOK if default RX flow has been registered, or negative
 *          error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerDefaultRxFlow(CpswProxy_Handle hProxy,
                                        uint32_t flowIdxBase,
                                        uint32_t flowIdxOffset);

/*!
 * \brief Unregister default flow from the given flow index.
 *
 * Disables routing of default traffic (traffic not matching any classifier with
 * thread id configured) to the given Rx flowIdx. Once disabled, all default
 * traffic will be routed to the reserved flow resulting in all packets of
 * default flow being dropped.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 *
 * \returns \ref CPSWPROXY_SOK if default RX flow has been unregistered, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterDefaultRxFlow(CpswProxy_Handle hProxy,
                                          uint32_t flowIdxBase,
                                          uint32_t flowIdxOffset);

/*!
 * \brief Register association of IPv4 address with MAC address by adding ARP
 *        entry in the master core.
 *
 * Register an IPv4 address in master core ARP table which will respond to
 * ARP request messages on behalf of the client.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param macAddr   MAC address with which the IPv4 address will be associated.
 * \param ipv4Addr  IPv4 address to be added to ARP database.
 *
 * \returns \ref CPSWPROXY_SOK if successfully registered IPv4 address, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerIPV4Addr(CpswProxy_Handle hProxy,
                                   uint8_t *macAddr,
                                   uint8_t *ipv4Addr);

/*!
 * \brief Unregister association of IPv4 address with MAC address by removing
 *        ARP entry in the master core.
 *
 * Unregisters an IPv4 address from master core ARP table.  Upon successful
 * unregistration, the remote core will no longer respond to ARP request messages
 * for the IPv4 address.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param ipv4Addr  IPv4 address to be unregistered.
 *
 * \returns \ref CPSWPROXY_SOK if successfully unregistered IPv4 address, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterIPV4Addr(CpswProxy_Handle hProxy,
                                     uint8_t *ipv4Addr);

/*!
 * \brief Join a VLAN.
 *
 * Joins a client to a VLAN registered on the server side.
 *
 * VLAN creation is done by Ethernet Firmware server at init time, it sets up
 * all VLANs, including the VLAN member list, flood masks, etc.  The server
 * also defines which remote clients can join the VLAN.
 *
 * If the client is allowed to join the VLAN, the server will setup a route
 * for VLAN tagged unicast packets matching the destination MAC address
 * provided in this function.
 *
 * RX flow idx must be set up prior to calling this function.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param macAddr          MAC address (unicast).
 * \param vlanId           VLAN id to join.
 *
 * \returns \ref CPSWPROXY_SOK if successfully joined VLAN, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_joinVlan(CpswProxy_Handle hProxy,
                           uint32_t flowIdxBase,
                           uint32_t flowIdxOffset,
                           const uint8_t *macAddr,
                           uint16_t vlanId);

/*!
 * \brief Leave a VLAN.
 *
 * Leaves a previously joined VLAN, registered on the server side.
 *
 * Client's route setup at the time of joining will be deleted after client
 * leaves the VLAN.
 *
 * \param hProxy           Handle to Cpsw Proxy.
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param macAddr          MAC address (unicast).
 * \param vlanId           VLAN id to leave.
 *
 * \returns \ref CPSWPROXY_SOK if successfully left VLAN, or negative error
 *          in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_leaveVlan(CpswProxy_Handle hProxy,
                            uint32_t flowIdxBase,
                            uint32_t flowIdxOffset,
                            const uint8_t *macAddr,
                            uint16_t vlanId);

/*!
 * \brief Add multicast address to receive filter.
 *
 * Adds a multicast address to the receive filter.  Ethernet Firmware differentiates
 * multicast addresses as _shared_ or _exclusive).
 *
 * - _Exclusive_ multicast address - Use is allowed only on a single remote client,
 *   the first one to request it.  Traffic received with exclusive address is
 *   routed to the remote client in hardware to the given RX flow index.
 * - _Shared_ multicast address - Traffic received with shared address is fanned out
 *   to all remote clients that request it via virtual intercore interface. RX
 *   flow index is not relevant in this case.
 *
 * RX flow idx must be set up prior to calling this function.
 *
 * \param hProxy           Handle to Cpsw Proxy
 * \param flowIdxBase      RX flow index base value.
 * \param flowIdxOffset    RX flow offset.
 * \param macAddr          Multicast MAC address to be added to receive filter.
 * \param vlanId           VLAN id.
 *
 * \returns \ref CPSWPROXY_SOK if multicast address has been successfully added
 *          to the filter, or negative error in case of a failure, see
 *          \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_filterAddMac(CpswProxy_Handle hProxy,
                               uint32_t flowIdxBase,
                               uint32_t flowIdxOffset,
                               const uint8_t *macAddr,
                               uint16_t vlanId);

/*!
 * \brief Delete multicast address from receive filter.
 *
 * Deletes a multicast address to the receive filter previous added through
 * \ref CpswProxy_filterAddMac().
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param macAddr   Multicast MAC address to be deleted from receive filter.
 * \param vlanId    VLAN id.
 *
 * \returns \ref CPSWPROXY_SOK if multicast address has been deleted from
 *          filter, or negative error in case of a failure, see
 *          \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_filterDelMac(CpswProxy_Handle hProxy,
                               const uint8_t *macAddr,
                               uint16_t vlanId);

/*!
 * \brief Query the link status.
 *
 * Queries the link status of the virtual port which depends on the virtual
 * port type:
 *
 * - Virtual switch ports - They are not associated with a specific hardware
 *   Ethernet port, so their link status is always reported as linked.
 * - Virtual MAC ports - They are associated with one actual Ethernet port
 *   on the Ethernet device, so the status reported by this function is the
 *   actual link state between the MAC port and the link partner.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 *
 * \return Whether link is up or not.
 */
bool CpswProxy_isPhyLinked(CpswProxy_Handle hProxy);

/*!
 * \brief Send DMA teardown completion notification.
 *
 * Sends a DMA tear-down completion notification to ETHFW so it can proceed
 * with Ethernet device recovery.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 *
 * \returns \ref CPSWPROXY_SOK if teardown completion notification was sent,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_teardownCompletion(CpswProxy_Handle hProxy);

/*!
 * \brief Dump network statistics on remote side.
 *
 * Request network statistics, ALE table, policer table to be dumped into
 * ETHFW logs.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 *
 * \returns \ref CPSWPROXY_SOK if successful, or negative error in case of a
 *          failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_dumpStats(CpswProxy_Handle hProxy);

/*!
 * \brief Get Server status.
 *
 * Get the status of the Ethernet Firmware.
 *
 * \param hProxy        Handle to Cpsw Proxy.
 * \param serverStatus  Pointer to status of ETHFW server.
 *
 * \returns \ref CPSWPROXY_SOK if successful, or negative error in case of a
 *          failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_getServerStatus(CpswProxy_Handle hProxy,
                                  EthRemoteCfg_ServerStatus *serverStatus);

/*!
 * \brief Register remote core's timer for synchronization.
 *
 * Register the remote core's timer with Ethernet Firmware, which will configure
 * the Time Sync Router (TSR) to route timer `timerId` events to the given
 * CPTS hardware push number.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param timerId   Input Id number of timer in TSR.
 * \param hwPushNum CPTS hardware push number.
 *
 * \returns \ref CPSWPROXY_SOK if remote timer is registered successfully,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerRemoteTimer(CpswProxy_Handle hProxy,
                                      uint8_t timerId,
                                      uint8_t hwPushNum);

/*!
 * \brief Unregister remote core's timer for synchronization,
 *
 * Unregisters the remote core's timer previously registered through
 * \ref CpswProxy_registerRemoteTimer().  Ethernet Firmware will clear
 * the Time Sync Router (TSR) configuration previously done for the given
 * CPTS hardware push number.
 *
 * \param hProxy    Handle to Cpsw Proxy.
 * \param hwPushNum Hardware push number of CPTS.
 *
 * \returns \ref CPSWPROXY_SOK if remote timer is unregistered successfully,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterRemoteTimer(CpswProxy_Handle hProxy,
                                        uint8_t hwPushNum);

/*!
 * \brief Register notification callback.
 *
 * Registers a callback function for the provided notification type in oder to
 * get notified of events on the Ethernet Firmware remote side.
 *
 * \param hProxy      Handle to Cpsw Proxy.
 * \param notifyType  Notification type, see \ref EthRemoteCfg_NotifyType.
 * \param cbFxn       Callback function to be called when event occurs.
 * \param cbArg       App's specific callback arguments.
 *
 * \returns \ref CPSWPROXY_SOK if callback is registered successfully, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_registerNotifyCb(CpswProxy_Handle hProxy,
                                   uint32_t notifyType,
                                   CpswProxy_NotifyCbFxn cbFxn,
                                   void *cbArg);

/*!
 * \brief Unregister notification callback.
 *
 * Unregisters a callback function for the provided notification type.
 *
 * \param hProxy      Handle to Cpsw Proxy.
 * \param notifyType  Notification type, see \ref EthRemoteCfg_NotifyType.
 *
 * \returns \ref CPSWPROXY_SOK if callback is unregistered successfully, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_unregisterNotifyCb(CpswProxy_Handle hProxy,
                                     uint32_t notifyType);

/*!
 * \brief Allocate CPTS HW push instance.
 *
 * Allocates a CPTS HW push instance from RM.
 *
 * \param hProxy     Handle to Cpsw Proxy
 * \param hwPushNum  Pointer to HW push instance to be allocated.
 *
 * \returns \ref CPSWPROXY_SOK if HW push instance has been successfully allocated,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_allocHwPushInst(CpswProxy_Handle hProxy,
                                  uint32_t *hwPushNum);

/*!
 * \brief Free CPTS HW push instance.
 *
 * Frees an allocated CPTS HW push instance from RM.
 *
 * \param hProxy     Handle to Cpsw Proxy
 * \param hwPushNum  HW push instance to be freed.
 *
 * \returns \ref CPSWPROXY_SOK if HW push instance has been successfully freed,
 *          or negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_freeHwPushInst(CpswProxy_Handle hProxy,
                                 uint32_t hwPushNum);

/*!
 * \brief Send an remote command.
 *
 * Send an command to ETHFW.This function can be used to send any supported 
 * remote commands to ETHFW.Note that it's recommended to use the dedicated
 * CPSW Proxy Client API if one exists for the desired functionality.
 *
 * \param hProxy      Handle to Cpsw Proxy.
 * \param reqType     Request type.
 * \param req         Command request message, see \ref EthRemoteCfg_ReqHdr.
 * \param reqLen      Command request length.
 * \param res         Command response message, see \ref EthRemoteCfg_ResHdr.
 * \param resLen      Command response length.
 *
 * \returns \ref CPSWPROXY_SOK if callback is unregistered successfully, or
 *          negative error in case of a failure, see \ref CpswProxy_ErrorCodes.
 */
int32_t CpswProxy_sendCmd(CpswProxy_Handle hProxy,
                          uint32_t reqType,
                          EthRemoteCfg_ReqHdr *req,
                          uint16_t reqLen,
                          EthRemoteCfg_ResHdr *res,
                          uint16_t resLen);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */
/*!
 * \brief Check whether port is a virtual switch port or not.
 *
 * \param portId [in]   Virtual port id.
 *
 * \return true if virtual switch port, false otherwise.
 */
static inline bool CpswProxy_isSwitchPort(EthRemoteCfg_VirtPort portId)
{
    return ((portId >= ETHREMOTECFG_SWITCH_PORT_0) && (portId <= ETHREMOTECFG_SWITCH_PORT_LAST));
}

/*!
 * \brief Check whether port is a virtual MAC port or not.
 *
 * \param portId [in]   Virtual port id.
 *
 * \return true if virtual MAC port, false otherwise.
 */
static inline bool CpswProxy_isMacPort(EthRemoteCfg_VirtPort portId)
{
    return ((portId >= ETHREMOTECFG_MAC_PORT_1) && (portId <= ETHREMOTECFG_MAC_PORT_LAST));
}

/*!
 * \brief Get virtual port number.
 *
 * Gets the port number of a virtual port. Virtual switch ports numbers are
 * 0-relative and virtual MAC ports are 1-relative.
 *
 * \param portId [in]   Virtual port id.
 *
 * \return Port number.
 */
static inline uint32_t CpswProxy_getPortNum(EthRemoteCfg_VirtPort portId)
{
    uint32_t portNum;

    if (CpswProxy_isSwitchPort(portId))
    {
        portNum = (uint32_t)portId;
    }
    else
    {
        portNum = (uint32_t)(portId - ETHREMOTECFG_MAC_PORT_1) + 1U;
    }

    return portNum;
}

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __CPSWPROXY_H__ */
