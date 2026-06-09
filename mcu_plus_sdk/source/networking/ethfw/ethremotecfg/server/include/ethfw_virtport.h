/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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
 * \file ethfw_virtport.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware proxy server virtport support.
 */

#ifndef ETHFW_VIRTPORT_H_
#define ETHFW_VIRTPORT_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>
#include <ethremotecfg/protocol/ethremotecfg.h>
#include <ethremotecfg/server/include/ethfw_qos.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_SERVER_VIRTPORT Virtport Support
 * @{
 */


/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Ethernet Firmware virtual port configuration.
 *
 * Ethernet Firmware configuration parameters for virtual ports on remote cores.
 */
typedef struct EthFwVirtPort_VirtPortCfg_s
{
    /*! Virtual port id */
    EthRemoteCfg_VirtPort portId;

    /*! Remote core id */
    uint32_t remoteCoreId;

    /*! Number of tx channels allocated for a given virtual port */
    uint32_t numTxCh;

    /*! Array of tx channels allocated for a given virtual port */
    EnetRm_TxCh txCh[ENET_CFG_TX_CHANNELS_NUM];

    /*! Number of rx channels allocated for a given virtual port */
    uint32_t numRxFlow;

    /*! Array of rx flow information for a given virtual port */
    EthFwQos_RxFlowInfo rxFlowsInfo[ENET_CFG_RX_FLOWS_NUM];

    /*! Number of mac address allocated allocated for a given virtual port */
    uint32_t numMacAddress;

    /*! Mask of client id's using this virtual port */
    uint32_t clientIdMask;

    /*! Local rpmsg endpoint for the virtual port */
    uint32_t endPointId;
} EthFwVirtPort_VirtPortCfg;

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
static inline bool EthFwVirtPort_isSwitchPort(EthRemoteCfg_VirtPort portId)
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
static inline bool EthFwVirtPort_isMacPort(EthRemoteCfg_VirtPort portId)
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
static inline uint32_t EthFwVirtPort_getPortNum(EthRemoteCfg_VirtPort portId)
{
    uint32_t portNum;

    if (EthFwVirtPort_isSwitchPort(portId))
    {
        portNum = (uint32_t)portId;
    }
    else
    {
        portNum = (uint32_t)(portId - ETHREMOTECFG_MAC_PORT_1) + 1U;
    }

    return portNum;
}

/*!
 * \brief Get Enet MAC port number corresponding to a virtual port id.
 *
 * Gets the Enet MAC port number corresponding to a virtual MAC port.  It will return
 * `ENET_MAC_PORT_INV` for virtual switch ports.
 *
 * The returned value of this function could be used as is to populate the port
 * number used for TX directed packets.
 *
 * \param portId    Virtual port id.
 *
 * \return Enet MAC port number for virtual MAC ports, `ENET_MAC_PORT_INV` for
 *         virtual switch ports.
 */
static inline Enet_MacPort EthFwVirtPort_getMacPort(EthRemoteCfg_VirtPort portId)
{
    Enet_MacPort macPort = ENET_MAC_PORT_INV;

    if (EthFwVirtPort_isMacPort(portId))
    {
        macPort = ENET_MACPORT_DENORM(portId - ETHREMOTECFG_MAC_PORT_1);
    }

    return macPort;
}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_VIRTPORT_H_ */
