/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <utils/pc_osal/include/dpl_osal.h>

typedef struct {
    uint16_t is_set;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SemaphoreP_Struct;

int32_t SemaphoreP_constructBinary(SemaphoreP_Object *obj, uint32_t initValue)
{
    int32_t status = 0;
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)obj;
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    if (pSemaphore != NULL)
    {
        (void)pthread_mutexattr_init(&mutex_attr);
        (void)pthread_condattr_init(&cond_attr);

        status = pthread_mutex_init(&pSemaphore->mutex, &mutex_attr);
        if (status == 0)
        {
            status = pthread_cond_init(&pSemaphore->cond, &cond_attr);
            if (status != 0)
            {
                (void)pthread_mutex_destroy(&pSemaphore->mutex);
            }
        }

        (void)pthread_condattr_destroy(&cond_attr);
        (void)pthread_mutexattr_destroy(&mutex_attr);

        if (status == 0)
        {
            pSemaphore->is_set = (initValue != 0U) ? 1U : 0U;
        }

    }

    return status;
}

void SemaphoreP_destruct(SemaphoreP_Object *obj)
{
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)obj;

    if (pSemaphore != NULL)
    {
        (void)pthread_mutex_destroy(&pSemaphore->mutex);
        (void)pthread_cond_destroy(&pSemaphore->cond);
    }
}

void SemaphoreP_post(SemaphoreP_Object *obj)
{
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)obj;

    if (obj != NULL)
    {
        pthread_mutex_lock(&pSemaphore->mutex);
        pSemaphore->is_set = 1U;

        pthread_cond_signal(&pSemaphore->cond);
        pthread_mutex_unlock(&pSemaphore->mutex);
    }
}

int32_t SemaphoreP_pend(SemaphoreP_Object *obj, uint32_t timeToWaitInTicks)
{
    int32_t status = 0;
    SemaphoreP_Struct *pSemaphore = (SemaphoreP_Struct *)obj;

    if (pSemaphore != NULL)
    {
        pthread_mutex_lock(&pSemaphore->mutex);

        if (timeToWaitInTicks == 0xFFFFFFFF)
        {
            while (pSemaphore->is_set == 0U)
            {
                pthread_cond_wait(&pSemaphore->cond, &pSemaphore->mutex);
            }
            pSemaphore->is_set = 0U;
        }
        else
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);

            uint32_t timeToWaitInNs = timeToWaitInTicks * 1000000;
            ts.tv_sec += timeToWaitInNs / 1000000000;
            ts.tv_nsec += timeToWaitInNs % 1000000000;

            if (ts.tv_nsec >= 1000000000)
            {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }

            while (pSemaphore->is_set == 0U)
            {
                if (pthread_cond_timedwait(&pSemaphore->cond, &pSemaphore->mutex, &ts) == ETIMEDOUT)
                {
                    status = -1;
                    break;
                }
            }

            if (status == 0)
            {
                pSemaphore->is_set = 0U;
            }
        }
        pthread_mutex_unlock(&pSemaphore->mutex);
    }

    return status;
}
