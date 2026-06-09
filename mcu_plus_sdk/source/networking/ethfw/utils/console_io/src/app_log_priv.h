/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#ifndef APP_LOG_PRIV_H_
#define APP_LOG_PRIV_H_

#include <utils/console_io/include/app_log.h>

#define APP_LOG_BUF_MAX  (1024u)

#define APP_LOG_PRINT_BUF_MAX (APP_LOG_BUF_MAX + APP_LOG_MAX_CPU_NAME + 32u)

typedef struct
{
    char buf[APP_LOG_BUF_MAX];
    app_log_shared_mem_t *shared_mem;
    app_log_cpu_shared_mem_t *cpu_shared_mem;
} app_log_wr_obj_t;

typedef struct
{
    char buf[APP_LOG_BUF_MAX];
    char print_buf[APP_LOG_PRINT_BUF_MAX];
    app_log_shared_mem_t *shared_mem;
    uint32_t log_rd_max_cpus;
    uint32_t log_rd_poll_interval_in_msecs;
    app_log_device_send_string_f device_write;
    uint8_t *task_stack;
    uint32_t task_stack_size;
    void *task_handle;
} app_log_rd_obj_t;

#define appAlign(value, align)   ((((value) + ((align) - 1)) / (align)) * (align))
#define appFloor(value, align)   (((value) / (align)) * (align))
#define appCeil(value, align)    appLogAlign(value, align)

uintptr_t appLogWrLock(app_log_wr_obj_t *obj);

void      appLogWrUnLock(app_log_wr_obj_t *obj,
                         uintptr_t key);

int32_t   appLogRdCreateTask(app_log_rd_obj_t *obj,
                             app_log_init_prm_t *prm);

#if defined(FREERTOS) || defined(SAFERTOS)
void appLogRdRun(void *arg0, void *arg1);
#else
void *appLogRdRun(app_log_rd_obj_t *obj);
#endif

uint64_t  appLogGetTimeInUsec(void);

void      appLogWaitMsecs(uint32_t time_in_msecs);

void *appMemMap(void *phys_addr,
                uint32_t size);

int32_t   appMemUnMap(void *virt_addr,
                      uint32_t size);

#endif
