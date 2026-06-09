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

static app_log_wr_obj_t g_app_log_wr_obj;

int32_t  appLogWrInit(app_log_init_prm_t *prm)
{
    int32_t status = 0;
    app_log_wr_obj_t *obj = &g_app_log_wr_obj;

    obj->cpu_shared_mem = NULL;
    obj->buf[0u] = 0u;

    if (prm->self_cpu_index >= APP_LOG_MAX_CPUS)
    {
        status = -1;
    }

    if (status == 0)
    {
        app_log_shared_mem_t *shared_mem = appMemMap(prm->shared_mem, sizeof(app_log_shared_mem_t));
        if (shared_mem != NULL)
        {
            uint32_t i;

            obj->shared_mem = shared_mem;

            obj->cpu_shared_mem = &shared_mem->cpu_shared_mem[prm->self_cpu_index];

            obj->cpu_shared_mem->log_cpu_name[0] = 0;

            for (i = 0; i < APP_LOG_MAX_CPU_NAME; i++)
            {
                obj->cpu_shared_mem->log_cpu_name[i] = prm->self_cpu_name[i];
            }

            if (obj->cpu_shared_mem->log_area_is_valid == APP_LOG_AREA_VALID_FLAG)
            {
                /* log memory already initialized no need to init again */
            }
            else
            {
                obj->cpu_shared_mem->log_wr_idx = 0;
                obj->cpu_shared_mem->log_area_is_valid = APP_LOG_AREA_VALID_FLAG;
            }
        }
        else
        {
            status = -1;
        }
    }

    return status;
}

int32_t  appLogWrDeInit(void)
{
    int32_t status = 0;

    return status;
}

int32_t  appLogWrPutString(app_log_wr_obj_t *obj)
{
    int32_t status = 0;
    volatile uint32_t max_bytes, num_bytes, copy_bytes;
    volatile uint32_t wr_idx, rd_idx, idx = 0U;
    volatile uint8_t *dst, *buf = (uint8_t *)obj->buf;
    app_log_cpu_shared_mem_t *cpu_shared_mem = obj->cpu_shared_mem;

    if (cpu_shared_mem == NULL || cpu_shared_mem->log_area_is_valid != APP_LOG_AREA_VALID_FLAG)
    {
        status = -1;
    }

    if (0 == status)
    {
        num_bytes = strlen((char *)buf);

        if (num_bytes <= 0)
        {
            status = -1;
        }
    }

    if (0 == status)
    {
        wr_idx = cpu_shared_mem->log_wr_idx;
        if (wr_idx >= APP_LOG_PER_CPU_MEM_SIZE)
        {
            wr_idx = 0;
        }

        rd_idx = cpu_shared_mem->log_rd_idx;
        if (rd_idx >= APP_LOG_PER_CPU_MEM_SIZE)
        {
            rd_idx = 0;
        }

        if (wr_idx < rd_idx)
        {
            max_bytes = rd_idx - wr_idx;
        }
        else
        {
            max_bytes = (APP_LOG_PER_CPU_MEM_SIZE - wr_idx) + rd_idx;
        }

        if (num_bytes > (max_bytes - 1U))
        {
            status = -1;
        }
    }

    if (0 == status)
    {
        dst = cpu_shared_mem->log_mem;
        for (copy_bytes = 0; copy_bytes < num_bytes; copy_bytes++)
        {
            if (wr_idx >= APP_LOG_PER_CPU_MEM_SIZE)
            {
                wr_idx = 0;
            }

            /* MISRA.PTR.ARITH
             * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
             * Pointer is used in arithmetic or array index expression.
             * KW State: Ignore -> Waiver -> Case by case
             * MISRAC_WAIVER:
             * Pointer is accessed as an array
             * Check is added to make sure that
             * pointer is never accessed beyond size of the array.
             */
            dst[wr_idx] = buf[idx];
            wr_idx++;
            idx++;
        }

        if (wr_idx >= APP_LOG_PER_CPU_MEM_SIZE)
        {
            wr_idx = 0U;
        }

        /* MISRA.PTR.ARITH
         * MISRAC_2004 Rule_17.1 and MISRAC_2004 Rule_17.4
         * Pointer is used in arithmetic or array index expression.
         * KW State: Ignore -> Waiver -> Case by case
         * MISRAC_WAIVER:
         * Pointer is accessed as an array
         * Check is added to make sure that
         * pointer is never accessed beyond size of the array.
         */
        dst[wr_idx] = 0U;
        wr_idx++;

        if (wr_idx >= APP_LOG_PER_CPU_MEM_SIZE)
        {
            wr_idx = 0U;
        }

        cpu_shared_mem->log_wr_idx = wr_idx;

        /* dummy read to resure data is written to memory */
        wr_idx = cpu_shared_mem->log_wr_idx;
    }

    return status;
}

void appLogPrintf(const char *format,
                  ...)
{
    va_list va_args_ptr;
    uint32_t cookie;
    uint32_t str_len = 0;
    uint64_t cur_time;
    app_log_wr_obj_t *obj = &g_app_log_wr_obj;

    cookie = appLogWrLock(obj);

    cur_time = appLogGetTimeInUsec();
    str_len = (uint32_t)snprintf(obj->buf, APP_LOG_BUF_MAX,
                                 "%6d.%06u s: ",
                                 (uint32_t)(cur_time / 1000000U),
                                 (uint32_t)(cur_time % 1000000U));

    /* if str_len is equal to APP_LOG_BUF_MAX, i.e string overflows buffer,
     * then don't write string. */
    if (str_len < APP_LOG_BUF_MAX)
    {
        va_start(va_args_ptr, format);

        /* MISRA.PTR.ARITH
         * MISRAC_2004_Rule_17.1 and MISRAC_2004_Rule_17.4
         * Pointer is used in arithmatic or array index expression
         * KW State: Ignore -> Waiver -> Case by case
         * MISRAC_WAIVER: buf is pointing to printBuf array of size
         * REMOTE_LOG_SERVER_PRINT_BUF_LEN and it is passed to vsnprint api,
         * which makes sure that the buf is never accessed beyond
         * its REMOTE_LOG_SERVER_PRINT_BUF_LEN size
         */
        vsnprintf((char *)(obj->buf + str_len),
                  APP_LOG_BUF_MAX - str_len,
                  format, va_args_ptr);
        va_end(va_args_ptr);

        appLogWrPutString(obj);

        appLogWrUnLock(obj, cookie);
    }
}

/** \brief Flag to sync CPU to make sure their init is done before moving ahead */
#define APP_LOG_CPU_SYNC_STATE_INVALID            (0u)
#define APP_LOG_CPU_SYNC_STATE_INIT_DONE          (1u)
#define APP_LOG_CPU_SYNC_STATE_TEST_INIT_DONE     (2u)
#define APP_LOG_CPU_SYNC_STATE_CONFIRM_INIT_DONE  (3u)
#define APP_LOG_CPU_SYNC_STATE_RUN                (4u)

void appLogSetCpuSyncState(uint32_t cpu_id,
                           uint32_t state)
{
    app_log_wr_obj_t *obj = &g_app_log_wr_obj;

    if (cpu_id < APP_LOG_MAX_CPUS && obj->shared_mem != NULL)
    {
        app_log_cpu_shared_mem_t *cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];

        cpu_shared_mem->log_cpu_sync_state = state;
    }
}

void appLogGetCpuSyncState(uint32_t cpu_id,
                           volatile uint32_t *state)
{
    app_log_wr_obj_t *obj = &g_app_log_wr_obj;

    *state = APP_LOG_CPU_SYNC_STATE_INVALID;

    if (cpu_id < APP_LOG_MAX_CPUS && obj->shared_mem != NULL)
    {
        app_log_cpu_shared_mem_t *cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];

        *state = cpu_shared_mem->log_cpu_sync_state;
    }
}

void appLogCpuSyncWithMaster(uint32_t self_cpu_id)
{
    volatile uint32_t state;

    appLogSetCpuSyncState(self_cpu_id, APP_LOG_CPU_SYNC_STATE_INIT_DONE);

    do
    {
        appLogGetCpuSyncState(self_cpu_id, &state);
    }
    while (state != APP_LOG_CPU_SYNC_STATE_TEST_INIT_DONE);

    appLogSetCpuSyncState(self_cpu_id, APP_LOG_CPU_SYNC_STATE_CONFIRM_INIT_DONE);

    do
    {
        appLogGetCpuSyncState(self_cpu_id, &state);
    }
    while (state != APP_LOG_CPU_SYNC_STATE_RUN);
}

void appLogCpuSyncWithSlave(uint32_t slave_cpu_id)
{
    volatile uint32_t state;

    appLogSetCpuSyncState(slave_cpu_id, APP_LOG_CPU_SYNC_STATE_TEST_INIT_DONE);

    do
    {
        appLogGetCpuSyncState(slave_cpu_id, &state);
        if (state == APP_LOG_CPU_SYNC_STATE_INIT_DONE)
        {
            appLogSetCpuSyncState(slave_cpu_id, APP_LOG_CPU_SYNC_STATE_TEST_INIT_DONE);
        }
    }
    while (state != APP_LOG_CPU_SYNC_STATE_CONFIRM_INIT_DONE);
}

void appLogCpuSyncStartSlave(uint32_t slave_cpu_id)
{
    appLogSetCpuSyncState(slave_cpu_id, APP_LOG_CPU_SYNC_STATE_RUN);
}

void appLogCpuSyncInit(uint32_t master_cpu_id,
                       uint32_t self_cpu_id,
                       uint32_t sync_cpu_id_list[],
                       uint32_t num_cpus)
{
    if (self_cpu_id == master_cpu_id)
    {
        uint32_t i, slave_cpu_id;

        /* master CPU, sync with each slave CPU */
        for (i = 0; i < num_cpus; i++)
        {
            slave_cpu_id = sync_cpu_id_list[i];
            if (slave_cpu_id != self_cpu_id)
            {
                appLogCpuSyncWithSlave(slave_cpu_id);
            }
        }

        /* all slaves have finished their init, now start all slave's */
        for (i = 0; i < num_cpus; i++)
        {
            slave_cpu_id = sync_cpu_id_list[i];
            if (slave_cpu_id != self_cpu_id)
            {
                appLogCpuSyncStartSlave(slave_cpu_id);
            }
        }
    }
    else
    {
        /* slave CPU, sync's with master CPU */
        appLogCpuSyncWithMaster(self_cpu_id);
    }
}
