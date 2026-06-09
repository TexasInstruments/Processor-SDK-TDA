/*
 *  Copyright (c) Texas Instruments Incorporated 2024
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file  ethfw_al.h
 *
 * \brief Ethernet Firmware abstraction layer for sitara
 */

#ifndef ETHFW_AL_H
#define ETHFW_AL_H
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/ipc_rpmsg.h>
#include <kernel/dpl/MailboxP.h>

#include <udma.h>
#include <enet_utils.h>
#include <enet_apputils.h>
#include <include/core/enet_osal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define IPC_MPU1_0                                  (CSL_CORE_ID_A53SS0_0)
#define IPC_MCU2_0                                  (CSL_CORE_ID_MAIN_R5FSS0_0)
#define IPC_MCU2_1                                  (CSL_CORE_ID_MCU_R5FSS0_0)
#define IPC_MCU2_2                                  (CSL_CORE_ID_WKUP_R5FSS0_0)
#define IPC_MAX_PROCS                               (11U)
/* 0xFF doesn't work due to ENET_BIT overflow for IPC_MCU1_0 */
#define IPC_MCU1_0                                  (0x0F)

#define RPMESSAGE_ANY                               (0xFFFFFFFFU)

#define NUM_ENTRIES                                 2
#define TYPE_VDEV                                   RPMESSAGE_RSC_TYPE_VDEV
#define VIRTIO_ID_RPMSG                             RPMESSAGE_RSC_VIRTIO_ID_RPMSG
#define TRACE_INTS_VER0                             RPMESSAGE_RSC_TRACE_INTS_VER0
#define TRACE_INTS_VER1                             RPMESSAGE_RSC_TRACE_INTS_VER0
#define TYPE_TRACE                                  RPMESSAGE_RSC_TYPE_TRACE

#define rpmsg_vdev                                  vdev

#define ETHFWOSAL_WAIT_FOREVER                      SystemP_WAIT_FOREVER
#define ETHFWIPC_WAIT_FOREVER                       SystemP_WAIT_FOREVER

#define ETHFWOSAL_SUCCESS                           SystemP_SUCCESS
#define ETHFWOSAL_FAILURE                           SystemP_FAILURE
#define ETHFWOSAL_TIMEOUT                           SystemP_TIMEOUT

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

typedef RPMessage_ResourceTable Ipc_ResourceTable;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ToDo: Add implementation for this API in MCU SDK */
Udma_DrvHandle EnetAppUtils_udmaOpen(Enet_Type enetType,
                                     Udma_InitPrms *pInitPrms);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */


#ifdef __cplusplus
}
#endif

#endif /* ETHFW_AL_H */
