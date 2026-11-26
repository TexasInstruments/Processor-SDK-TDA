/*
 * Copyright (C) 2024-2026 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ENET_LAYER2_TYPES_H_
#define ENET_LAYER2_TYPES_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <enet.h>
#include <enet_cfg.h>
#include <include/core/enet_dma.h>
#include <include/per/icssg.h>
#include <enet_apputils.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/* Max number of ports supported per context */
#define ENETMP_PORT_MAX                          (ENET_SYSCFG_NUM_EXT_MAC_PORTS)

/* Max number of ports that can be tested with this app */
#define ENETMP_ICSSG_INSTANCE_MAX                (2U)
#define ENETMP_PER_MAX                           (ENETMP_ICSSG_INSTANCE_MAX)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* Context of a peripheral/port */
typedef struct EnetMp_PerCtxt_s
{
    /* Peripheral type */
    Enet_Type enetType;

    /* Peripheral instance */
    uint32_t instId;

    /* Number of RX Flows associated to this context */
    uint32_t rxChCount;

    /* Start Channel ID of the RX Flow associated to this context. It is assumed
     * that 'rxChCount' number of Rx Flows for this context is allocated contiguously
     * from rxChStartId. Note that concept of channels inside flows is hidden to application */
    uint32_t rxChStartId;

    /* Number of TX Channels associated to this context */
    uint32_t txChCount;

    /* Start Channel ID of the TX Channels associated to this context. It is assumed
     * that 'txChCount' number of Tx Channels for this context is allocated contiguously
     * from txChStartId */
    uint32_t txChStartId;

    /* Peripheral's MAC ports to use */
    Enet_MacPort macPort[ENETMP_PORT_MAX];

    /* Number of MAC ports in macPorts array */
    uint8_t macPortNum;

    /* Name of this port to be used for logging */
    char *name;

    /* ICSSG configuration */
    Icssg_Cfg icssgCfg;

    /* Number of valid MAC address entries present in macAddr variable below*/
    uint8_t numValidMacAddress;

    /* MAC address. It's port's MAC address in Dual-MAC or
     * host port's MAC addres in Switch */
    uint8_t macAddr[ENET_SYSCFG_RX_FLOWS_NUM][ENET_MAC_ADDR_LEN];

    /* UDMA driver configuration */
    EnetDma_Cfg dmaCfg;

    /* TX channel number */
    uint32_t txChNum[ENET_SYSCFG_TX_CHANNELS_NUM];

    /* TX channel handle */
    EnetDma_TxChHandle hTxCh[ENET_SYSCFG_TX_CHANNELS_NUM];

    /* Start flow index */
    uint32_t rxStartFlowIdx[ENET_SYSCFG_RX_FLOWS_NUM];

    /* Flow index */
    uint32_t rxFlowIdx[ENET_SYSCFG_RX_FLOWS_NUM];

    /* RX channel handle */
    EnetDma_RxChHandle hRxCh[ENET_SYSCFG_RX_FLOWS_NUM];

    /* RX task handle - receives packets, changes source/dest MAC addresses
     * and transmits the packets back */
    TaskP_Object rxTaskObj;

    /* Semaphore posted from RX callback when packets have arrived */
    SemaphoreP_Object rxSemObj;

    /* Semaphore used to synchronize all RX tasks exits */
    SemaphoreP_Object rxDoneSemObj;

    /* Semaphore posted from event callback upon asynchronous IOCTL completion */
    SemaphoreP_Object ayncIoctlSemObj;

    /* Timestamp of the last received packets */
    uint64_t rxTs[ENET_SYSCFG_TOTAL_NUM_RX_PKT];

    /* Timestamp of the last transmitted packets */
    uint64_t txTs[ENET_SYSCFG_TOTAL_NUM_RX_PKT];

    /* Sequence number used as cookie for timestamp events. This value is passed
     * to the DMA packet when submitting a packet for transmission. Driver will
     * pass this same value when timestamp for that packet is ready. */
    uint32_t txTsSeqId;

    /* Semaphore posted from event callback upon asynchronous IOCTL completion */
    SemaphoreP_Object txTsSemObj;

    /* Enet and Udma handle info for the peripheral */
    EnetApp_HandleInfo handleInfo;

   /* Core attach info for the peripheral */
    EnetPer_AttachCoreOutArgs attachInfo;

} EnetMp_PerCtxt;

typedef struct EnetMp_Obj_s
{
    /* Flag which indicates if test shall run */
    volatile bool run;

    /* This core's id */
    uint32_t coreId;

    /* Queue of free TX packets */
    EnetDma_PktQ txFreePktInfoQ;

    /* Array of all peripheral/port contexts used in the test */
    EnetMp_PerCtxt perCtxt[ENETMP_PER_MAX];

    /* Number of active contexts being used */
    uint32_t numPerCtxts;

    /* Whether promiscuous mode is enabled or not */
    bool promisc;

    /* Whether timestamp are enabled or not */
    bool enableTs;
} EnetMp_Obj;

#ifdef __cplusplus
}
#endif

#endif /* ENET_LAYER2_TYPES_H_ */
