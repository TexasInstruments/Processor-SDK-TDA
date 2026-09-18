/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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

/*
 *  \file ipc_rsctable.h
 *
 *  \brief Define the resource table entries for all R5F cores. This will be
 *  incorporated into corresponding base images, and used by the remoteproc
 *  on the host-side to allocated/reserve resources.
 *
 */

#ifndef IPC_RSCTABLE_H_
#define IPC_RSCTABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drv/ipc/include/ipc_rsctypes.h>

#define IPC_TRACE_MAX_LINE_LENGTH     (512)

#define IPC_TRACE_BUFFER_MAX_SIZE     (0x80000)

extern char Ipc_traceBuffer[IPC_TRACE_BUFFER_MAX_SIZE];

/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define R5F_RPMSG_VQ0_SIZE      256U
#define R5F_RPMSG_VQ1_SIZE      256U
#define C66_RPMSG_VQ0_SIZE      256U
#define C66_RPMSG_VQ1_SIZE      256U
#define C7X_RPMSG_VQ0_SIZE      256U
#define C7X_RPMSG_VQ1_SIZE      256U

/* flip up bits whose indices represent features we support */
#define RPMSG_R5F_C0_FEATURES   1U
#define RPMSG_C66_DSP_FEATURES  1U
#define RPMSG_C7X_DSP_FEATURES  1U

#define TRACEBUFADDR ((uintptr_t)&Ipc_traceBuffer)

#define RPMSG_VRING_ADDR_ANY FW_RSC_ADDR_ANY

const Ipc_ResourceTable ti_ipc_remoteproc_ResourceTable __attribute__ ((section (".resource_table"), aligned (4096))) = 
{
    .base =
    {
        .num = NUM_ENTRIES,         /* number of entries in the table */
        .ver = 1U,                  /* we're the first version that implements this */
        .reserved = {0U, 0U},       /* reserved, must be zero */
    },

    /* offsets to entries */
    .offset =
    {
        offsetof(Ipc_ResourceTable, rpmsg_vdev),
        offsetof(Ipc_ResourceTable, trace),
    },

    /* rpmsg vdev entry */
    .rpmsg_vdev =
    {
        TYPE_VDEV, VIRTIO_ID_RPMSG, 0U,
        RPMSG_R5F_C0_FEATURES, 0U, 0U, 0U, 2U, { 0U, 0U },
        /* no config data */
    },
    /* the two vrings */
    .rpmsg_vring0 =
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ0_SIZE, 1U, 0U },
    .rpmsg_vring1 =
    { RPMSG_VRING_ADDR_ANY, 4096U, R5F_RPMSG_VQ1_SIZE, 2U, 0U },
    .trace =
    {
        (TRACE_INTS_VER0 | TYPE_TRACE), TRACEBUFADDR, IPC_TRACE_BUFFER_MAX_SIZE, 0, "trace:r5f0",
    },
};

#ifdef __cplusplus
}
#endif

#endif /* IPC_RSCTABLE_H_ */