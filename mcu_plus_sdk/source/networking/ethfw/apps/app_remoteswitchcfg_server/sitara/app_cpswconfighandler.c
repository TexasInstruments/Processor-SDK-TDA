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
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
/* lwIP core includes */
#include "lwip/opt.h"
/* SDK includes */
#include <networking/enet/core/utils/include/enet_apputils.h>
#include <networking/enet/core/utils/include/enet_board.h>
#include <enet.h>
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/QueueP.h>
#include "ti_board_config.h"
#include "ti_board_open_close.h"
#include "ti_drivers_open_close.h"
#include "ti_enet_config.h"
#include "ti_enet_open_close.h"
#include <tsn_combase/combase.h>
#include <tsn_combase/combase_link.h>
#include <tsn_combase/tilld/cb_lld_ethernet.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


static void EnetApp_mdioLinkStatusChange(Cpsw_MdioLinkStateChangeInfo *info,
                                         void *appArg);

static void EnetApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                           bool isLinkUp,
                                           void *appArg);

void EthApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                          bool isLinkUp,
                                          void *appArg);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EnetApp_addMCastEntry(Enet_Type enetType,
                          uint32_t instId,
                          uint32_t coreId,
                          const uint8_t *testMCastAddr,
                          uint32_t portMask)
{
    Enet_IoctlPrms prms;
    int32_t status;
    CpswAle_SetMcastEntryInArgs setMcastInArgs;
    uint32_t setMcastOutArgs;

    if (Enet_isCpswFamily(enetType))
    {
        Enet_Handle hEnet = Enet_getHandle(enetType, instId);

        EnetAppUtils_assert(hEnet != NULL);
        memset(&setMcastInArgs, 0, sizeof(setMcastInArgs));
        memcpy(&setMcastInArgs.addr.addr[0U], testMCastAddr,
               sizeof(setMcastInArgs.addr.addr));
        setMcastInArgs.addr.vlanId  = 0;
        setMcastInArgs.info.super = false;
        setMcastInArgs.info.numIgnBits = 0;
        setMcastInArgs.info.fwdState = CPSW_ALE_FWDSTLVL_FWD;
        setMcastInArgs.info.portMask = portMask;
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &setMcastInArgs, &setMcastOutArgs);
        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_ADD_MCAST, &prms, status);
        if (status != ENET_SOK)
        {
           EnetAppUtils_print("EnetTestBcastMcastLimit_AddAleEntry() failed CPSW_ALE_IOCTL_ADD_MCAST: %d\n",
                               status);
        }
    }
}


void EnetApp_updateCpswInitCfg(Enet_Type enetType, uint32_t instId, Cpsw_Cfg *cpswCfg)
{
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_CPDMA)
    EnetDma_Cfg * dmaCfg = (EnetDma_Cfg *)cpswCfg->dmaCfg;

    EnetAppUtils_assert(dmaCfg != NULL);
    EnetAppUtils_assert(EnetAppUtils_isDescCached() == false);
    dmaCfg->rxInterruptPerMSec = 8;
    dmaCfg->txInterruptPerMSec = 2;
#endif


#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 1U)
    cpswCfg->mdioLinkStateChangeCb    = NULL;
    cpswCfg->mdioLinkStateChangeCbArg = NULL;
#else
    cpswCfg->mdioLinkStateChangeCb    = &EnetApp_mdioLinkStatusChange;
    cpswCfg->mdioLinkStateChangeCbArg = NULL;
#endif

#if defined(ETHFW_BOOT_TIME_PROFILING)
    /* Link-up timestamp */
    cpswCfg->portLinkStatusChangeCb    = &EthApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = NULL;
#else
    cpswCfg->portLinkStatusChangeCb = &EnetApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = NULL;
#endif
}

static void EnetApp_mdioLinkStatusChange(Cpsw_MdioLinkStateChangeInfo *info,
                                         void *appArg)
{
    if (info->linkChanged)
    {
        EnetAppUtils_print("Link Status Changed. PHY: 0x%x, state: %s\r\n",
                info->phyAddr,
                info->isLinked? "up" : "down");
    }
}

static void EnetApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                           bool isLinkUp,
                                           void *appArg)
{
    EnetAppUtils_print("MAC Port %u: link %s\r\n",
                       ENET_MACPORT_ID(macPort), isLinkUp ? "up" : "down");
    cb_lld_notify_linkchange();
}
