/*
 *
 * Copyright (c) 2020-2024 Texas Instruments Incorporated
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

/**
 *  \file ethfw.h
 *
 *  \brief Header file for Ethernet Firmware library
 */

#ifndef ETHFW_H_
#define ETHFW_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>
#include <include/per/cpsw.h>
#include <ethremotecfg/protocol/ethremotecfg.h>
#include <ethremotecfg/server/include/ethfw_virtport.h>
#include <ethremotecfg/server/include/ethfw_mcast.h>
#include <ethremotecfg/server/include/ethfw_vlan.h>
#include <ethremotecfg/server/include/ethfw_vepa.h>
#include <ethremotecfg/server/include/ethfw_iet.h>
#include <ethremotecfg/server/include/ethfw_monitor.h>
#include <ethremotecfg/server/include/ethfw_qos.h>
#include <ethremotecfg/server/include/ethfw_portmirror.h>
#include <utils/ethfw_common/include/ethfw_types.h>
#include <ethremotecfg/server/include/ethfw_tsn.h>
#include <utils/ethfw_common/include/ethfw_trace.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup ETHFW_SERVER Ethernet Firmware Server APIs
 *
 * \brief This section contains Ethernet Firmware library APIs.
 *
 * The Ethernet Firmware Server APIs in this module provide a simple interface
 * for RTOS applications to enable Ethernet switch functionality.
 *
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_SERVER
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! Number of octets in year */
#define ETHFW_VERSION_YEARLEN             (4)

/*! Number of octets in month */
#define ETHFW_VERSION_MONTHLEN            (3)

/*! Number of octets in date */
#define ETHFW_VERSION_DATELEN             (2)

/*! Number of octets in hour */
#define ETHFW_VERSION_HOURLEN             (2)

/*! Number of octets in minutes */
#define ETHFW_VERSION_MINLEN              (2)

/*! Number of octets in seconds */
#define ETHFW_VERSION_SECLEN              (2)

/*! GIT Commit SHA length in octets */
#define ETHFW_VERSION_COMMITSHALEN        (8)

/*! Max number of remote clients sharing resources with ETHFW(Linux, QNX, RTOS, AUTOSAR) */
#define ETHFW_REMOTE_CLIENT_MAX           (6U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Ethernet Firmware version
 *
 * Version of the Ethernet Firmware.
 */
typedef struct EthFw_Version_s
{
    /*! Major version number */
    uint32_t major;

    /*! Minor version number */
    uint32_t minor;

    /*! Revision version number */
    uint32_t rev;

    /*! Remote Ethernet Switch Device Firmware Build year: char string in the form YYYY eg: 2019 */
    int8_t year[ETHFW_VERSION_YEARLEN + 1U];

    /*! Remote Ethernet Switch Device Firmware Build month: char string in the form MON eg: Dec */
    int8_t month[ETHFW_VERSION_MONTHLEN + 1U];

    /*! Remote Ethernet Switch Device Firmware Build date: char string in the form DD eg: 12 */
    int8_t date[ETHFW_VERSION_DATELEN + 1U];

    /*! Remote Ethernet Switch Device Firmware Build hour in 24 hour format */
    int8_t hour[ETHFW_VERSION_HOURLEN + 1U];

    /*! Remote Ethernet Switch Device Firmware Build minute */
    int8_t min[ETHFW_VERSION_MINLEN + 1U];

    /*! Remote Ethernet Switch Device Firmware Build second */
    int8_t sec[ETHFW_VERSION_SECLEN + 1U];

    /*! GIT commit SHA of the firmware: char string in the form fd52c34f */
    int8_t commitHash[ETHFW_VERSION_COMMITSHALEN + 1U];
} EthFw_Version;

/*! Callback function for application to set port link parameters
 *  (MII, PHY, speed, duplexity, etc) */
typedef int32_t (*EthFw_setPortCfg)(Enet_MacPort macPort,
                                    CpswMacPort_Cfg *macCfg,
                                    EnetMacPort_Interface *mii,
                                    EnetPhy_Cfg *phyCfg,
                                    EnetMacPort_LinkCfg *linkCfg);

/*! Callback for setting gPTP config parameters from the application */
typedef void (*EthFw_configPtpCb)(void *arg);

/*!
 * \brief Ethernet Firmware configuration
 *
 * Ethernet Firmware configuration parameters.
 */
typedef struct EthFw_Config_s
{
    /*! CPSW configuration */
    Cpsw_Cfg cpswCfg;

    /*! MAC ports owned by EthFw. It must be provided as an array of
     *  MAC port ids */
    Enet_MacPort *ports;

    /*! Number of MAC ports owned by EthFw, that is, the size of
     *  EthFw_Config::ports array */
    uint32_t numPorts;

    /*! Virtual ports accessed via remote_device framework (A72 Linux,
     *  A72 QNX, RTOS cores) */
    EthFwVirtPort_VirtPortCfg *virtPortCfg;

    /*! Number of virtual ports accessed via remote_device framework */
    uint32_t numVirtPorts;

    /*! VLAN configuration */
    EthFwVlan_VlanCfg *vlanCfg;

    /*! Number of static VLANs configured. */
    uint32_t numStaticVlans;

    /*! Default port mask for all ports in switch mode */
    uint32_t defaultPortMask;

    /*! Default virtual port mask for all ports in switch mode */
    uint32_t defaultVirtPortMask;

    /*! Whether forwarding to all Switch ports for dynamic VLANs is enabled */
    bool dVlanSwtFwdEn;

    /*! Multicast configuration. */
    EthFwMcast_Cfg mcastCfg;

    /*! Default VLAN id to be used for MAC ports configured in MAC-only mode */
    uint16_t dfltVlanIdMacOnlyPorts;

    /*! Default VLAN id to be used for MAC ports configured in switch mode (non MAC-only) */
    uint16_t dfltVlanIdSwitchPorts;

    /*! Callback function for application to set port link parameters
     *  (MII, PHY, speed, duplexity, etc) */
    EthFw_setPortCfg setPortCfg;

    /*! Callback for setting gPTP config parameters from the application */
    EthFw_configPtpCb configPtpCb;

    /*! Argument to be passed to gPTP config callback function */
    void *configPtpCbArg;

    /*! VEPA configuration passed from application */
    EthFwVepa_Cfg vepaCfg;

    /*! IET configuration passed from application */
    EthFwIET_Config ietCfg;
    
    /*! CPSW monitor and recovery configuration */
    EthFwMon_Cfg monitorCfg;

    /*! Flag to let application allocate absolute tx channels */
    bool isStaticTxChanAllocated;

    /*! PPS config params */
    EthFwTsn_PpsConfig ppsConfig;

    /*! Pass packets with errors to host port. Error packets will be passed to
     *  the application through `handleErrPktFxn()` callback of lwIP adaptation
     *  layer's output args `LwipifEnetAppIf_GetHandleOutArgs`. */
    bool passErrPkt;

    /*! Port mirroring configuration */
    EthFwPortMirroring_Cfg *portMirCfg;
} EthFw_Config;

/*!
 * \brief Ethernet Firmware handle
 *
 * Ethernet Firmware opaque handle.
 */
typedef struct EthFw_Obj_s *EthFw_Handle;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Set EthFw configuration parameters to default values
 *
 * Sets the EthFw configuration parameters to default values. The application
 * must (at least) set the following parameters after this call:
 *  - The list of MAC ports to be used by Ethernet Firmware via
 *    EthFw_Config::ports and EthFw_Config::numPorts.
 *  - The UDMA driver handle via dmaCfg's UDMA driver handle in
 *    EthFw_Config::cpswCfg.
 *
 * \param enetType    Enet instance type
 * \param instId      Enet instance id
 * \param config      Pointer to the EthFw config to be initialized
 */
void EthFw_initConfigParams(Enet_Type enetType,
                            uint32_t instId,
                            EthFw_Config *config);

/*!
 * \brief Initialize EthFw
 *
 * Initializes the EthFw with the provided configuration parameters. The firmware
 * will open the CPSW low-level driver and its multi-client manager (CPSW MCM).
 * It must be called from task context, cannot be called from main().
 *
 * \param enetType    Enet instance type
 * \param instId      Enet instance id
 * \param config      EthFw configuration
 *
 * \retval EthFw handle if initialization was successful
 * \retval NULL if initialization failed
 */
EthFw_Handle EthFw_init(Enet_Type enetType,
                        uint32_t instId,
                        const EthFw_Config *config);

/*!
 * \brief De-initialize EthFw
 *
 * De-initialize the EthFw.
 *
 * \param hEthFw    EthFw handle
 */
void EthFw_deinit(EthFw_Handle hEthFw);

/*!
 * \brief Initialize remote configuration server.
 *
 * Initializes the firmware's remote configuration server which is in charge
 * of servicing commands sent by remote cores.
 *
 * \param hEthFw      EthFw handle
 *
 * \retval ENET_SOK if remote config initialization was successful
 * \retval Negative error code if initialization failed
 */
int32_t EthFw_initRemoteConfig(EthFw_Handle hEthFw);

/*!
 * \brief Late announce to remote processor
 *
 * Perform a late announce operation to remote processor.  The processor is
 * identified by the IPC driver's core id definition.
 *
 * This function is typically used to late attach to MPU1_0 core running Linux
 * after Ethernet Firmware had been loaded by u-boot.
 *
 * \param hEthFw      EthFw handle
 * \param procId      IPC processor id, refer to IPC driver definitions.
 *
 * \retval ENET_SOK if remote services initialization was successful
 * \retval Negative error code if announcement failed
 */
int32_t EthFw_lateAnnounce(EthFw_Handle hEthFw,
                           uint32_t procId);

/*!
 * \brief Get EthFw version
 *
 * Gets the EthFw version which includes firmware version, build date and time,
 * and commit hash.
 *
 * \param hEthFw      EthFw handle
 * \param version     Pointer to EthFw version to be populated
 */
void EthFw_getVersion(EthFw_Handle hEthFw,
                      EthFw_Version *version);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_H_ */
