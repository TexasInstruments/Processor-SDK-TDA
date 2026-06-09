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
 * \file ethfw_portmirror.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware port mirroring support.
 */

#ifndef ETHFW_PORTMIRROR_H_
#define ETHFW_PORTMIRROR_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <enet.h>
#include <include/mod/cpsw_ale.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_PORTMIRROR_TYPES Port Mirroring Support
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
 * \brief Source port mirroring configuration
 */
typedef struct EthFwPortMirroring_SrcPortMirrorCfg_e
{
    /*! Source port number mirroring enable bitmask
     * Range: 1U (Host port) to 1 << CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_LAST) */
    uint32_t srcPortNumMask;

    /*! The destination port for the mirror traffic
     * Range: 0U (Host port) to CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_LAST) */
    uint32_t toPortNum;
} EthFwPortMirroring_SrcPortMirrorCfg;

/*!
 * \brief Destination port mirroring configuration
 */
typedef struct EthFwPortMirroring_DstPortMirrorCfg_e
{
    /*! The port to which destination traffic destined will be duplicated
     *  Range: 0U (Host port) to CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_LAST) */
    uint32_t dstPortNum;

    /*! The destination port for the mirror traffic
     * Range: 0U (Host port) to CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_LAST) */
    uint32_t toPortNum;
} EthFwPortMirroring_DstPortMirrorCfg;

/*!
 * \brief Table entry port mirroring configuration
 */
typedef struct EthFwPortMirroring_TblEntryPortMirrorCfg_e
{
    /*! ALE lookup table entry info that when a match occurs will cause this
     *  traffic to be mirrored to the toPortNum port */
    CpswAle_MirrorMatchParams matchParams;

    /*! The destination port for the mirror traffic
     * Range: 0U (Host port) to CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_LAST) */
    uint32_t toPortNum;
} EthFwPortMirroring_TblEntryPortMirrorCfg;


/*!
 * \brief Union of modes/types of port mirroring possible
 */
typedef union
{
    /*! Source port mirroring configuration */
    EthFwPortMirroring_SrcPortMirrorCfg srcPortMirCfg;

    /*! Destination port mirroring configuration */
    EthFwPortMirroring_DstPortMirrorCfg dstPortMirCfg;

    /*! Table entry port mirroring configuration */
    EthFwPortMirroring_TblEntryPortMirrorCfg tblEntryPortMirCfg;
} EthFwPortMirroringMode;

/*!
 * \brief Enum for port mirroring type
 */
typedef enum EthFwPortMirroringType_e
{
    /*! Source port mirroring */
    SRC_PORT_MIRRORING = 0U,
    /*! Destination port mirroring */
    DST_PORT_MIRRORING,
    /*! Table entry port mirroring */
    TBL_ENTRY_PORT_MIRRORING,
    /*! Port Mirroring disabled */
    DISABLE_PORT_MIRRORING
} EthFwPortMirroringType;

/*!
 * \brief Port mirroring configuration
 */
typedef struct EthFwPortMirroring_Cfg_e
{
    /*! Mode of port mirroring */
    EthFwPortMirroringMode mirroringMode;

    /*! Type of port mirroring to enable */
    EthFwPortMirroringType mirroringType;
} EthFwPortMirroring_Cfg;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Enables/Disables port mirroring
 *
 * \param hEnet            Handle to CPSW
 * \param portMirCfg       Port mirroring configuration
 *
 * \returns ETHFW_SOK if port mirroring was enabled/disabled successfully
 *          or a negative error code if failed.
 */

int32_t EthFwPortMirroring_init(Enet_Handle hEnet,
                                EthFwPortMirroring_Cfg *portMirCfg);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_PORTMIRROR_H_ */
