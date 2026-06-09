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

/*!
 * \file ethfw_portmirror.c
 *
 * \brief This file contains the implementation of port mirroring helper functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x109

#include <stdint.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <ethremotecfg/server/include/ethfw_portmirror.h>
#include <utils/ethfw_common/include/ethfw_trace.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t EthFwPortMirroring_addDestPortMirror(Enet_Handle hEnet,
                                                    EthFwPortMirroring_DstPortMirrorCfg *dstPortMirCfg);

static int32_t EthFwPortMirroring_addSrcPortMirror(Enet_Handle hEnet,
                                                   EthFwPortMirroring_SrcPortMirrorCfg *srcPortMirCfg);

static int32_t EthFwPortMirroring_addTblEntryPortMirror(Enet_Handle hEnet,
                                                        EthFwPortMirroring_TblEntryPortMirrorCfg *tblEntryPortMirCfg);

static int32_t EthFwPortMirroring_disablePortMirror(Enet_Handle hEnet);

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static int32_t EthFwPortMirroring_addDestPortMirror(Enet_Handle hEnet,
                                                    EthFwPortMirroring_DstPortMirrorCfg *dstPortMirCfg)
{
    Enet_IoctlPrms prms;
    CpswAle_PortMirroringCfg inArgs;
    int32_t status;
    uint32_t coreId;

    coreId = EnetSoc_getCoreId();
    /* All packets which are sent out/egress from dstPortNum will also mirrored to toPortNum */
    memset(&inArgs, 0U, sizeof (CpswAle_PortMirroringCfg));
    inArgs.srcEn = BFALSE;
    inArgs.dstEnEn = BTRUE;
    inArgs.matchEn = BFALSE;
    /* Port number whose output to be mirriored */
    inArgs.dstPortNum = dstPortMirCfg->dstPortNum;
    /* To port number, where mirror traffic to flow */
    inArgs.toPortNum = dstPortMirCfg->toPortNum;
    inArgs.srcPortNumMask = 0U;

    ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_PORT_MIRROR_CFG, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to enable Destination Port mirroring");
    return status;
}

static int32_t EthFwPortMirroring_addSrcPortMirror(Enet_Handle hEnet,
                                                   EthFwPortMirroring_SrcPortMirrorCfg *srcPortMirCfg)
{
    Enet_IoctlPrms prms;
    CpswAle_PortMirroringCfg inArgs;
    int32_t status;
    uint32_t coreId;

    coreId = EnetSoc_getCoreId();
    /* All packets which are received/ingress on any port from srcPortNumMask will also be switched
     * to egress toPortNum as well as intended egress destination port */
    memset(&inArgs, 0U, sizeof (CpswAle_PortMirroringCfg));
    inArgs.srcEn = BTRUE;
    inArgs.dstEnEn = BFALSE;
    inArgs.matchEn = BFALSE;
    inArgs.dstPortNum = 0U;
    /* To port number, where mirror traffic to flow */
    inArgs.toPortNum = srcPortMirCfg->toPortNum;
    /* Port Mask whose intput to be mirriored */
    inArgs.srcPortNumMask = srcPortMirCfg->srcPortNumMask;

    ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_PORT_MIRROR_CFG, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to enable Source Port mirroring");
    return status;
}

static int32_t EthFwPortMirroring_addTblEntryPortMirror(Enet_Handle hEnet,
                                                        EthFwPortMirroring_TblEntryPortMirrorCfg *tblEntryPortMirCfg)
{
    Enet_IoctlPrms prms;
    CpswAle_PortMirroringCfg inArgs;
    int32_t status;
    uint32_t coreId;

    coreId = EnetSoc_getCoreId();
    /* Table entry mirroring */
    memset(&inArgs, 0U, sizeof (CpswAle_PortMirroringCfg));
    inArgs.srcEn = BFALSE;
    inArgs.dstEnEn = BFALSE;
    inArgs.matchEn = BTRUE;
    inArgs.dstPortNum = 0U;
    inArgs.srcPortNumMask = 0U;
    /* To port number, where mirror traffic to flow */
    inArgs.toPortNum = tblEntryPortMirCfg->toPortNum;
    /* ALE table entry to match. When a match occurs this will cause this
     * traffic to be mirrored to the toPortNum port */
    inArgs.matchParams = tblEntryPortMirCfg->matchParams;

    ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_PORT_MIRROR_CFG, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to enable Table entry mirroring");
    return status;
}

int32_t EthFwPortMirroring_disablePortMirror(Enet_Handle hEnet)
{
    Enet_IoctlPrms prms;
    int32_t status;
    uint32_t coreId;

    coreId = EnetSoc_getCoreId();

    ENET_IOCTL_SET_NO_ARGS(&prms);
    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_DISABLE_PORT_MIRROR, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to disable mirroring");
    return status;
}

int32_t EthFwPortMirroring_init(Enet_Handle hEnet,
                                EthFwPortMirroring_Cfg *portMirCfg)
{
    int32_t status = ETHFW_SOK;

    if (portMirCfg == NULL)
    {
        status = ETHFW_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Error NULL port mirroring configuration passed");
    }

    if (status == ETHFW_SOK)
    {
        switch (portMirCfg->mirroringType)
        {
            case SRC_PORT_MIRRORING:
            {
                status = EthFwPortMirroring_addSrcPortMirror(hEnet,
                                                            &portMirCfg->mirroringMode.srcPortMirCfg);
                break;
            }

            case DST_PORT_MIRRORING:
            {
                status = EthFwPortMirroring_addDestPortMirror(hEnet,
                                                            &portMirCfg->mirroringMode.dstPortMirCfg);
                break;
            }

            case TBL_ENTRY_PORT_MIRRORING:
            {
                status = EthFwPortMirroring_addTblEntryPortMirror(hEnet,
                                                                &portMirCfg->mirroringMode.tblEntryPortMirCfg);
                break;
            }

            case DISABLE_PORT_MIRRORING:
            {
                status = EthFwPortMirroring_disablePortMirror(hEnet);
                break;
            }

            default:
            {
                status = ETHFW_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "Error invalid parameter passed for port mirroring %u",
                            portMirCfg->mirroringType);
                break;
            }
        }
    }

    return status;
}
