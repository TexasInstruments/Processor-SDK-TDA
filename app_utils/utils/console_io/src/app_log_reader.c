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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define  APP_LOG_RD_TASK_STACK_SIZE   (16*1024)

#if defined(R5F) && defined(SAFERTOS)
#define APP_LOG_RD_TASK_ALIGNMENT    APP_LOG_RD_TASK_STACK_SIZE
#else
#define APP_LOG_RD_TASK_ALIGNMENT    (8192u)
#endif

static app_log_rd_obj_t g_app_log_rd_obj;

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
static uint8_t g_app_log_rd_task_stack[APP_LOG_RD_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(APP_LOG_RD_TASK_ALIGNMENT)))
    ;

void appLogInitPrmSetDefault(app_log_init_prm_t *prms)
{
    uint32_t i;

    prms->shared_mem = NULL;
    prms->self_cpu_index = APP_LOG_MAX_CPUS;
    (void)strncpy(prms->self_cpu_name, "UNKNOWN-CPU", APP_LOG_MAX_CPU_NAME);
    prms->self_cpu_name[APP_LOG_MAX_CPU_NAME-1]=0;
    prms->log_rd_task_pri = 10;
    prms->log_rd_poll_interval_in_msecs = 10;
    prms->log_rd_max_cpus = 0;
    for(i=0; i<APP_LOG_MAX_CPUS; i++)
    {
        prms->log_rd_cpu_enable[i] = 0;
    }
}

int32_t  appLogRdInit(app_log_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id;
    app_log_rd_obj_t *obj = &g_app_log_rd_obj;

    if(prm->shared_mem == NULL || prm->log_rd_max_cpus > APP_LOG_MAX_CPUS)
    {
        status = -1;
    }
    if(status==0)
    {
        obj->shared_mem = appMemMap(prm->shared_mem, sizeof(app_log_shared_mem_t));

        if(obj->shared_mem!=NULL)
        {
            obj->log_rd_max_cpus = prm->log_rd_max_cpus;
            obj->log_rd_poll_interval_in_msecs = prm->log_rd_poll_interval_in_msecs;
            obj->device_write = prm->device_write;
            obj->task_stack = g_app_log_rd_task_stack;
            obj->task_stack_size = APP_LOG_RD_TASK_STACK_SIZE;

            for(cpu_id=0; cpu_id<APP_LOG_MAX_CPUS; cpu_id++)
            {
                obj->log_rd_cpu_enable[cpu_id] = prm->log_rd_cpu_enable[cpu_id];
            }

            for(cpu_id=0; cpu_id<obj->log_rd_max_cpus; cpu_id++)
            {
                app_log_cpu_shared_mem_t *cpu_shared_mem;

                cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];
                #ifdef LINUX
                /* dont reset to 0 in Linux mode since this is reset to zero already by remote core */
                #elif QNX
                /* dont reset to 0 in Qnx mode since this is reset to zero already by remote core */
                #else
                cpu_shared_mem->log_wr_idx = 0;
                #endif
                cpu_shared_mem->log_rd_idx = 0;

            }
            /* task is never deleted to have log till the very end of CPU shutdown */
            status = appLogRdCreateTask(obj, prm);
        }
    }
    return status;
}

int32_t  appLogRdDeInit()
{
    int32_t status = 0;

    return status;
}

uint32_t appLogRdGetString(app_log_cpu_shared_mem_t *cpu_shared_mem,
                char *buf, uint32_t buf_size, uint32_t *str_len )
{
    uint32_t num_bytes = 0, idx = 0, wr_idx, rd_idx, copy_bytes = 0;
    uint32_t break_loop = 0;
    uint8_t cur_char;
    volatile uint8_t *src;

    wr_idx = cpu_shared_mem->log_wr_idx;
    rd_idx = cpu_shared_mem->log_rd_idx;

    if(rd_idx > wr_idx)
    {
        num_bytes = (APP_LOG_PER_CPU_MEM_SIZE - rd_idx) + wr_idx;
    }
    else
    {
        num_bytes = wr_idx - rd_idx;
    }

    if(num_bytes > 0U)
    {
        /* MISRA.PTR.ARITH
         * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
         * Pointer is used in arithmetic or array index expression.
         * KW State: Ignore -> Waiver -> Case by case
         * MISRAC_WAIVER:
         * Pointer is initialized with the address of array variable and
         * then accessed as an array. Check is added to make sure that
         * pointer is never accessed beyond size of the array.
         */
        src = cpu_shared_mem->log_mem;

        for(copy_bytes = 0U; copy_bytes < num_bytes; copy_bytes ++)
        {
            if(rd_idx >= APP_LOG_PER_CPU_MEM_SIZE)
            {
                rd_idx = 0;
            }

            /* MISRA.PTR.ARITH
             * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
             * MISRAC_WAIVER:
             * Pointer is initialized with the address of array variable and
             * then accessed as an array
             * Check is added to make sure that array/pointer access
             * is never beyond its size
             */
            cur_char = src[rd_idx];

            rd_idx++;

            if (cur_char==(uint8_t)0xA0)
            {
                break_loop = 1;
            }
            else if (cur_char==(uint8_t)'\r')
            {
                break_loop = 1;
            }
            else if (cur_char==(uint8_t)'\n')
            {
                break_loop = 1;
            }
            else if (cur_char==(uint8_t)0)
            {
                break_loop = 1;
            }
            else if (copy_bytes >= (APP_LOG_PER_CPU_MEM_SIZE))
            {
                break_loop = 1;
            }
            else if (copy_bytes > (buf_size))
            {
                break_loop = 1;
            }
            else
            {
                buf[idx] = (char)cur_char;
                idx ++;
                break_loop = 0;
            }

            if ( 1 == break_loop)
            {
                break;
            }
        }

        cpu_shared_mem->log_rd_idx = rd_idx;

        /* dummy read to resure data is written to memory */
        rd_idx = cpu_shared_mem->log_rd_idx;
    }

    buf[idx]   = (char)0u;
    *str_len   = idx;

    return num_bytes;
}


#if defined(FREERTOS) || defined(SYSBIOS) || defined(SAFERTOS) || defined(THREADX)
void appLogRdRun(void *arg0, void *arg1)
#else
void* appLogRdRun(app_log_rd_obj_t *obj)
#endif
{
    uint32_t done = 0, cpu_id, i;
    uint32_t num_bytes, str_len;
    uint8_t  log_cpu_name[APP_LOG_MAX_CPU_NAME];

    #if defined(FREERTOS) || defined(SYSBIOS) || defined(SAFERTOS) || defined(THREADX)
    app_log_rd_obj_t *obj =  (app_log_rd_obj_t *)arg0;

    appUtilsTaskInit();
    #endif

    while(!done)
    {
        appLogWaitMsecs(obj->log_rd_poll_interval_in_msecs);

        for(cpu_id=0; cpu_id<obj->log_rd_max_cpus; cpu_id++)
        {
            app_log_cpu_shared_mem_t *cpu_shared_mem;

            cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];

            for (i=0; i<APP_LOG_MAX_CPU_NAME; i++)
            {
                log_cpu_name[i] = cpu_shared_mem->log_cpu_name[i];
            }

            if(cpu_shared_mem->log_area_is_valid == APP_LOG_AREA_VALID_FLAG
                && obj->log_rd_cpu_enable[cpu_id] == 1
             )
            {
                do
                {
                    str_len = 0;
                    num_bytes = appLogRdGetString(cpu_shared_mem,
                                    obj->buf,
                                    APP_LOG_BUF_MAX,
                                    &str_len );
                    if(str_len > 0)
                    {
                        if(obj->device_write)
                        {
                            (void)snprintf(obj->print_buf, APP_LOG_PRINT_BUF_MAX, "[%-6s] %s\r\n",
                                log_cpu_name,
                                obj->buf);

                            obj->device_write(obj->print_buf, APP_LOG_PRINT_BUF_MAX);
                        }
                    }
                } while(num_bytes);
            }
        }
    }

    #if defined(FREERTOS) || defined(SYSBIOS) || defined(SAFERTOS) || defined(THREADX)
    return;
    #else
    return NULL;
    #endif
}
