/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include "app_log_priv.h"
#include <ti/osal/HwiP.h>
#include <ti/osal/TimerP.h>
#include <ti/osal/TaskP.h>
#include <ti/osal/SemaphoreP.h>


uintptr_t appLogWrLock(app_log_wr_obj_t *obj)
{
    return HwiP_disable();
}

void appLogWrUnLock(app_log_wr_obj_t *obj,
                    uintptr_t key)
{
    HwiP_restore(key);
}

uint64_t appLogGetTimeInUsec(void)
{
    return TimerP_getTimeInUsecs(); /* in units of usecs */
}

void appLogWaitMsecs(uint32_t time_in_msecs)
{
    TaskP_sleepInMsecs(time_in_msecs);
}

int32_t   appLogRdCreateTask(app_log_rd_obj_t *obj,
                             app_log_init_prm_t *prm)
{
    TaskP_Params rtos_task_prms;
    int32_t status = 0;

    TaskP_Params_init(&rtos_task_prms);

    rtos_task_prms.stacksize = obj->task_stack_size;
    rtos_task_prms.stack = obj->task_stack;
    rtos_task_prms.priority = prm->log_rd_task_pri;
    rtos_task_prms.arg0 = (void*)(obj);
    rtos_task_prms.arg1 = NULL;

    obj->task_handle = (void *)TaskP_create(
                                            &appLogRdRun,
                                            &rtos_task_prms);
    if (obj->task_handle == NULL)
    {
        status = -1;
    }

    return status;
}

void *appMemMap(void *phys_ptr,
                uint32_t size)
{
    return phys_ptr; /* phys == virtual in rtos */
}

int32_t appMemUnMap(void *virt_ptr,
                    uint32_t size)
{
    return 0; /* nothing to do in rtos */
}
