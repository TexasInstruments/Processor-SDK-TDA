/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include <platform_util_ipc.h>

#include "ipc_trace_priv.h"

/* ipc_rsctypes.h checks for BUILD_C7X to determine if the da field in Ipc_Trace should be uint64_t or uint32_t.*/
#if defined(C7X_FAMILY)
#define BUILD_C7X
#if defined(SOC_J722S) || defined(SOC_AM62A)
#pragma diag_suppress 70
#endif
#endif

#if defined (MCU_SDK)
#include <hal/RPMessage/v0/src/RPMessage_Hal_Linux_ResourceTable.h>
#elif defined (MCU_PLUS_SDK)
#include <drivers/ipc_rpmsg/include/ipc_rpmsg_linux_resource_table.h>
#elif defined (PDK)
#include <ti/drv/ipc/include/ipc_rsctypes.h>
#endif

#if defined (MCU_SDK) || defined (MCU_PLUS_SDK)
#define NUM_ENTRIES      2
#define TYPE_VDEV        RPMESSAGE_RSC_TYPE_VDEV
#define VIRTIO_ID_RPMSG  RPMESSAGE_RSC_VIRTIO_ID_RPMSG
#define TRACE_INTS_VER0  RPMESSAGE_RSC_TRACE_INTS_VER0
#define TRACE_INTS_VER1  RPMESSAGE_RSC_TRACE_INTS_VER0
#define TYPE_TRACE       RPMESSAGE_RSC_TYPE_TRACE
#endif

#if defined (MCU_SDK)
#define rpmsg_vdev  Vdev
#elif defined (MCU_PLUS_SDK)
#define rpmsg_vdev  vdev
#endif

#if defined (MCU_SDK) || defined (PDK)
#define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#endif

#if defined (MCU_SDK)
typedef struct RPMessage_Hal_ResourceTable_s Ipc_ResourceTable;
#elif defined (MCU_PLUS_SDK)
typedef RPMessage_ResourceTable Ipc_ResourceTable;
#endif

/* Linux kernel defines this as (-1), below define avoids compile warnings */
/** \brief Macro to specify memory needs to be dynamically allocated */
#define RPMSG_VRING_ADDR_ANY (~0)

/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define R5F_RPMSG_VQ0_SIZE       256U
#define R5F_RPMSG_VQ1_SIZE       256U
#define M55_RPMSG_VQ0_SIZE       256U
#define M55_RPMSG_VQ1_SIZE       256U
#define R52P_RPMSG_VQ0_SIZE      256U
#define R52P_RPMSG_VQ1_SIZE      256U
#define C66_RPMSG_VQ0_SIZE       256U
#define C66_RPMSG_VQ1_SIZE       256U
#define C7X_RPMSG_VQ0_SIZE       256U
#define C7X_RPMSG_VQ1_SIZE       256U

/* flip up bits whose indices represent features we support */
#define RPMSG_R52P_FEATURES     1U
#define RPMSG_M55_FEATURES      1U
#define RPMSG_R5F_FEATURES      1U
#define RPMSG_C66_DSP_FEATURES  1U
#define RPMSG_C7X_DSP_FEATURES  1U
#define TRACEBUFADDR ((uintptr_t)&Ipc_traceBuffer)

const Ipc_ResourceTable ti_ipc_remoteproc_ResourceTable __attribute__ ((section (".resource_table"), aligned (4096))) =
{
    1U,                    /* we're the first version that implements this */
    NUM_ENTRIES,           /* number of entries in the table */
    0U, 0U,                /* reserved, must be zero */

    /* offsets to entries */
    {
        offsetof(Ipc_ResourceTable, rpmsg_vdev),
#if defined (MCU_PLUS_SDK) || defined (PDK)
        offsetof(Ipc_ResourceTable, trace),
#elif defined (MCU_SDK)
        offsetof(Ipc_ResourceTable, Trace),
#endif
    },

    /* rpmsg vdev entry */
    {
        TYPE_VDEV, VIRTIO_ID_RPMSG, 0U,
#if defined (C7X_FAMILY)
        RPMSG_C7X_DSP_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
#elif defined (C6X_FAMILY)
        RPMSG_C66_DSP_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
#elif defined (M55)
        RPMSG_M55_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
#elif defined (R5F)
        RPMSG_R5F_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
#elif defined (R52P)
        RPMSG_R52P_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
#endif
        /* no config data */
    },
    /* the two vrings */
#if defined (R5F)
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
#elif defined (M55)
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
#elif defined (R52P)
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
#elif defined (C7X_FAMILY)
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
#elif defined (C6X_FAMILY)
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
#else
    #error Core not defined
#endif

    {
#if defined(C7X_FAMILY)
        (TRACE_INTS_VER1 | TYPE_TRACE), TRACEBUFADDR, 0x80000, 0, "trace:c7x",
#elif defined(C6X_FAMILY)
        (TRACE_INTS_VER1 | TYPE_TRACE), TRACEBUFADDR, 0x80000, 0, "trace:c6x",
#elif defined (M55)
        (TRACE_INTS_VER0 | TYPE_TRACE), TRACEBUFADDR, 0x80000, 0, "trace:m55",
#elif defined (R5F)
        (TRACE_INTS_VER0 | TYPE_TRACE), TRACEBUFADDR, 0x80000, 0, "trace:r5f0",
#elif defined (R52P)
        (TRACE_INTS_VER0 | TYPE_TRACE), TRACEBUFADDR, 0x80000, 0, "trace:r52p",
#endif
    },
};

void *appGetIpcResourceTable()
{
  return (void*)&ti_ipc_remoteproc_ResourceTable;
}
