/*
 *  Copyright (c) Texas Instruments Incorporated 2025
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
 * \file  remote_main.c
 *
 * \brief This file contains the implementation of the C7x application
 *        entry point
 */

/* ========================================================================== */
/*                              Include Files                                 */
/* ========================================================================== */

#include <stdio.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/ClockP.h>
#include "kernel/dpl/TimerP.h"
#include "ti_dpl_config.h"
#include "shm_cirbuf.h"
#include "string.h"

/* A copy of params in ethfw_avtp.c */
#define SHM_AVB_RX_BUFFER_SIZE       (0x20000)
#define AAF_SYNC_FRAME_SIZE_CLASSA   (192)
#define AAF_SYNC_FRAME_SIZE_CLASSD   (768)

/* AVB Shared Memory Address. */
uint8_t *gSourceShmAddress_classA = (uint8_t *)(0xA3000000+ 2*SHM_AVB_RX_BUFFER_SIZE);

SemaphoreP_Object gRemoteAppTimerSem;

shm_handle remoteAppInitShm(void* const address, const int blockSize, const int totalSize)
{
    const uint32_t shmOvrHd     = shm_metadata_overhead();

    /* Floor() the totalSize to be a multiple of blocksize, exclude the overhead size. */
    const uint32_t rxShmBufSize = ((totalSize-shmOvrHd)/blockSize)*blockSize + shmOvrHd;

    memset(address, 0, rxShmBufSize);
    shm_handle handle = shm_create((uint32_t)address, rxShmBufSize);

    DebugP_assert(handle != NULL);
    return handle;
}

void RemoteApp_mainTask(void *args)
{
    shm_handle shmHandle = remoteAppInitShm(gSourceShmAddress_classA,
                                            AAF_SYNC_FRAME_SIZE_CLASSA,
                                            2*SHM_AVB_RX_BUFFER_SIZE);

    SemaphoreP_constructBinary(&gRemoteAppTimerSem, 0);
    TimerP_start(gTimerBaseAddr[CONFIG_TIMER0]);
    while (1)
    {
        uint8_t test[AAF_SYNC_FRAME_SIZE_CLASSA] = {0};
        SemaphoreP_pend(&gRemoteAppTimerSem, SystemP_WAIT_FOREVER);
        uint16_t dataLen = sizeof(test);
        ShdMemStatus status = shm_read(shmHandle, test, &dataLen);

        if (dataLen > 0)
        {
            /* Reading from shm successful. */
            DebugP_log("Rcvd: %d\n", dataLen);
        }
    }
}

void remoteTimerIsr(void)
{
    SemaphoreP_post(&gRemoteAppTimerSem);
}
