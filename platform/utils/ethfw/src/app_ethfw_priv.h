/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#ifndef APP_ETHFW_PRIV_H
#define APP_ETHFW_PRIV_H

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>

#if defined(MCU_PLUS_SDK)
#include <include/per/cpsw.h>
#include <enet.h>
#include <enet_apputils.h>
#include <ethfw_al.h>
#elif defined(PDK)
/* PDK Driver Header files */
#include <ipc/ipc.h>
#include <udma/udma.h>
#include <enet/include/per/cpsw.h>
#include <enet/enet.h>
#include <enet/examples/utils/include/enet_apputils.h>
#endif

/* EthFw header files */
#include <utils/board/include/ethfw_board_utils.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>
#if defined(ETHFW_DEMO_SUPPORT)
#include <utils/intervlan/include/eth_hwintervlan.h>
#include <utils/intervlan/include/eth_swintervlan.h>
#endif
#include <ethremotecfg/server/include/ethfw.h>
#include <ethremotecfg/protocol/ethremotecfg.h>

/* Vision Apps utils */
#include <utils/udma/include/app_udma.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>

#if defined(ETHFW_GPTP_SUPPORT)
/* Timesync header files */
#if defined(MCU_PLUS_SDK)
#include <tsn_buildconf/sitara_buildconf.h>
#elif defined(PDK)
#include <tsn_buildconf/jacinto_buildconf.h>
#endif
#include <tsn_combase/tilld/cb_lld_ethernet.h>
#include <tsn_gptp/gptpconf/gptpgcfg.h>
#include <tsn_gptp/gptpconf/xl4-extmod-xl4gptp.h>
#include <ethremotecfg/server/include/ethfw_tsn.h>
#endif

#if defined(ETHAPP_ENABLE_IPERF_SERVER)
#include "examples/lwiperf/lwiperf_example.h"
#endif

#if defined (SYSBIOS)

#include <utils/ethfw_callbacks/include/ethfw_callbacks_nimu.h>
#include <utils/ethfw_callbacks/include/ethfw_callbacks_ndk.h>

#endif

#if defined (FREERTOS) || defined (SAFERTOS)
/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "netif/bridgeif.h"

#if defined(PDK)
#include <ti/drv/enet/lwipif/inc/default_netif.h>
#include <ti/drv/enet/lwipif/inc/lwip2lwipif.h>
#elif defined(MCU_PLUS_SDK)
#include <lwip2lwipif.h>
#endif

#include <utils/ethfw_callbacks/include/ethfw_callbacks_lwipif.h>

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
#if defined(MCU_PLUS_SDK)
#include <lwip2enet_ic.h>
#include <lwip2lwipif_ic.h>
#elif defined(PDK)
#include <ti/drv/enet/lwipific/inc/netif_ic.h>
#include <ti/drv/enet/lwipific/inc/lwip2enet_ic.h>
#include <ti/drv/enet/lwipific/inc/lwip2lwipif_ic.h>
#endif
#endif

#if (MCU_PLUS_SDK)
#include "ti_enet_lwipif.h"
#include "ti_enet_config.h"
#endif
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define ETHAPP_OK                       (0)

#define ETHAPP_ERROR                    (-1)

#define ARRAY_SIZE(x)                   (sizeof((x)) / sizeof(x[0U]))

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    /* Core Id */
    uint32_t coreId;

    /* CPSW instance type */
    Enet_Type enetType;

    /* Ethernet Firmware handle */
    EthFw_Handle hEthFw;

    /* Semaphore for synchronizing EthFw and NDK initialization */
    app_rtos_semaphore_handle_t hInitSem;

    /* Host MAC address */
    uint8_t hostMacAddr[ENET_MAC_ADDR_LEN];

#if defined(ETHFW_GPTP_SUPPORT)
    /* Semaphore used to indicate when host port MAC address has been allocated */
    app_rtos_semaphore_handle_t hHostMacAllocSem;
#endif

    /* Enet instance id */
    uint32_t instId;

    /* Enable/disable PPS gen check */
    bool enablePPS;

#if defined(FREERTOS) || defined(SAFERTOS)
    /* DHCP network interface */
    struct dhcp dhcpNetif;
#endif
} EthAppObj;

#endif //APP_ETHFW_PRIV_H

