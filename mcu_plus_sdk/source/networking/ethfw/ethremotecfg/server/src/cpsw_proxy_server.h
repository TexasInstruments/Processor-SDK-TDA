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

#ifndef __CPSWPROXYSERVER_H__
#define __CPSWPROXYSERVER_H__


#include <stdint.h>
#include <enet.h>
#include <utils/include/enet_mcm.h>
#include <ethremotecfg/protocol/ethremotecfg.h>
#include <ethremotecfg/server/include/ethfw_virtport.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup CPSW_PROXY_SERVER_API Ethernet Switch Proxy Server APIs
 *
 * \brief This section contains APIs for CPSW Proxy Server APIs
 *
 * The CPSW Proxy Server resides on the master core and enables clients on
 * remote cores to configure the Ethernet Switch. The CPSW Proxy Server is the
 * application interface to the ethernet remote device server.
 *
 * The application configures and instantiates a CPSW Proxy Server instance.
 * Once instantiated the CPSW will listen to, process and respond to
 * CPSW_PROXY_CLIENT messages from remote cores.
 *
 *  @{
 */
/*! @} */

/*!
 * \addtogroup CPSW_PROXY_SERVER_API
 * @{
 */

/*! Max number of CPSW MAC ports supported */
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
#define CPSWPROXYSERVER_MAC_PORT_MAX                  (8U)
#else
#define CPSWPROXYSERVER_MAC_PORT_MAX                  (4U)
#endif

/*! Max number of AUTOSAR client cores */
#define CPSWPROXYSERVER_AUTOSAR_REMOTE_CLIENT_MAX     (2U)

/*! Max number of Remote clients supported on a remote core (VirtPort Clients) */
#define CPSWPROXYSERVER_VIRTPORT_PER_CLIENT_MAX       (2U)

/*! Max number of remote client virtual ports */
#define CPSWPROXYSERVER_REMOTE_CLIENT_VIRTPORT_MAX    (6U)

/*!
 * \brief Application callback function pointer to initialize Ethernet Firmware data
 *
 * When a client connection from remote core to  cpsw proxy server is
 * established, the server will invoke this application callback to populate
 * firmware info which is exported as Ethernet device data to the remote core
 * client.
 *
 * \param host_id       Remote Core Id
 * \param eth_dev_data  Firmware device data to be populated
 */
typedef void  (*CpswProxyServer_InitEthfwDeviceDataCb)(EthRemoteCfg_DeviceData *ethdevData);

/*!
 * \brief Application callback function pointer to get Multiclient Manager (MCM)
 *        command mailbox.
 *
 * The MCM manages access to single CPSW LLD handle across multiple clients on 
 * both remote core and local core.
 *
 * The CPSW Proxy server needs the MCM command interface to perform ATTACH.
 * The MCM command interface is obtained by invoking this application callback.
 *
 * \param enetType         Enet instance type
 * \param pMcmCmdIfHandle  Pointer to MCM command interface structure which will
 *                         be populated by application
 */
typedef void  (*CpswProxyServer_GetMcmCmdIfCb)(Enet_Type  enetType, EnetMcm_CmdIf  **pMcmCmdIfHandle);

/*!
 * \brief Application Callback function pointer to handle custom notification
 *        from remote client
 *
 * This is application handler for custom client to server notification from
 * remote cores.
 * The proxy layer just passes the notify info and notify_info_len.
 * The client and server application interpretation of the custom notify info
 * should match.
 *
 * \param host_id      Remote Core IPC core id
 * \param hEnet        Handle to CPSW
 * \param enetType     Enet instance type
 * \param notifyid     Custom notify type
 * \param notify_info  Notify info
 * \param notify_info_len Notify info length
 */
typedef void  (*CpswProxyServer_NotifyCb)(uint32_t hostId,
                                          Enet_Handle hEnet,
                                          Enet_Type enetType,
                                          uint32_t coreKey,
                                          EthRemoteCfg_NotifyType notifyid,
                                          uint8_t *notifyInfo,
                                          uint32_t notifyInfoLen);

/*!
 * \brief Function to broadcast a notification for all attached clients.
 *
 * Function to broadcast a notification event to all the remote clients.
 *
 * \param notifyId     The type of notify send to the remote clients.
 *
 * \returns status
 */
int32_t CpswProxyServer_bcastNotify(uint32_t notifyId);

/*!
 * \brief Function to get the status of active and idle clients.
 *
 * Function to get the status of active and idle clients.  A client is
 * considered to be idled after it has torn down its DMA channel/flow.
 *
 * \params attachedClients  Number of attached clients.
 * \params idleClients      Number of idled clients.
 *
 * \returns status
 */
int32_t CpswProxyServer_getIdleClientCnt(uint32_t *attachedClients,
                                         uint32_t *idleClients);

/*!
 * \brief Cpsw Proxy Server Remote Configuration structure
 *
 * Structure passed by application to configure the CPSW Proxy server.
 */
typedef struct CpswProxyServer_Config_s
{
    /*! Enet instance type */
    Enet_Type enetType;

    /*! Enet instance id */
    uint32_t instId;

    /*! Application callback to populate Ethernet Remote Device data */
    CpswProxyServer_InitEthfwDeviceDataCb initEthfwDeviceDataCb;

    /*! Application callback to get MCM command interface */
    CpswProxyServer_GetMcmCmdIfCb         getMcmCmdIfCb;

    /*! Application callback to handle custom notify from client */
    CpswProxyServer_NotifyCb              notifyCb;

    /*! Virtual port configuration */
    EthFwVirtPort_VirtPortCfg virtPortCfg[CPSWPROXYSERVER_REMOTE_CLIENT_VIRTPORT_MAX];

    /*! Number of remote virtual ports that remotes cores can attach to */
    uint32_t numVirtPorts;

    /*! Default VLAN id to be used for MAC ports configured in MAC-only mode */
    uint16_t dfltVlanIdMacOnlyPorts;

    /*! Default VLAN id to be used for MAC ports configured in switch mode (non MAC-only) */
    uint16_t dfltVlanIdSwitchPorts;

    /*! Port mask of all enabled MAC ports */
    uint32_t enabledPortMask;

    /*! Port mask of all MAC-only ports */
    uint32_t macOnlyPortMask;
} CpswProxyServer_Config_t;

/*!
 * \brief Cpsw proxy server initialization function
 *
 * \param cfg  Cpsw Proxy Server configuration
 */
int32_t CpswProxyServer_init(CpswProxyServer_Config_t *cfg);

/*!
 * \brief Late announces ETHFW endpoint to Linux
 */
int32_t CpswProxyServer_lateAnnounce(uint32_t procId);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __CPSWPROXYSERVER_H__ */
