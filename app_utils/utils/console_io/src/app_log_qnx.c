/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   /* for nanosleep */
int nanosleep(const struct timespec *req, struct timespec *rem);
#else
#include <unistd.h> /* for usleep */
extern int usleep (useconds_t __useconds);
#endif
#include "app_log_priv.h"
#include <inttypes.h>

#undef APP_LOG_DEBUG

typedef struct
{
    pthread_mutex_t lock;
    pthread_t task;
} app_log_qnx_obj_t;

app_log_qnx_obj_t g_app_log_qnx_obj;

uintptr_t appLogWrLock(app_log_wr_obj_t *obj)
{
    pthread_mutex_lock(&g_app_log_qnx_obj.lock);
    return 0;
}

void appLogWrUnLock(app_log_wr_obj_t *obj, uintptr_t key)
{
    pthread_mutex_unlock(&g_app_log_qnx_obj.lock);
}

int32_t appLogWrCreateLock(app_log_wr_obj_t *obj)
{
    return 0;
}

int32_t appLogRdCreateTask(app_log_rd_obj_t *obj, app_log_init_prm_t *prm)
{
    pthread_mutexattr_t mutex_attr;
    pthread_attr_t thread_attr;
    int32_t status = 0;

    status |= pthread_mutexattr_init(&mutex_attr);
    if(status==0)
    {
        status |= pthread_mutex_init(&g_app_log_qnx_obj.lock, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }
    if(status!=0)
    {
        printf("APP_LOG: ERROR: Unable to create mutex !!!\n");
    }
    if(status==0)
    {
        status |= pthread_attr_init(&thread_attr);
        if(status!=0)
        {
            printf("APP_LOG: ERROR: Unable to set thread attr !!!\n");
        }
        if(status==0)
        {
            status |= pthread_create(&g_app_log_qnx_obj.task, &thread_attr, (void*)appLogRdRun, obj);
        }
        pthread_attr_destroy(&thread_attr);
    }
    if(status!=0)
    {
        printf("APP_LOG: ERROR: Unable to create thread !!!\n");
    }

    return status;
}
