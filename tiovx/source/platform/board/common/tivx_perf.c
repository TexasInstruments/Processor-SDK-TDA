/*
*
* Copyright (c) 2025-2026 Texas Instruments Incorporated
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


#include <tivx_target_config.h>
#include <tivx_platform.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#if !defined(HLOS)
#include <utils/rtos/src/app_rtos_priv.h>
#endif /* #if !defined(HLOS) */

#if defined(BUILD_DEV)
extern tivx_resource_stats_t g_tivx_resource_stats_table[TIVX_RESOURCE_STATS_TABLE_SIZE];
extern uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX];

void ownPlatformGetTargetPerfStats(uint32_t app_cpu_id,
                                    uint32_t target_values[TIVX_TARGET_RESOURCE_COUNT],
                                    uint32_t rtos_sem_object_types[TIVX_TARGET_RTOS_MAX_COUNT][TIVX_TARGET_RTOS_MAX_COUNT],
                                    uint32_t rtos_task_object_types[TIVX_TARGET_RTOS_MAX_COUNT][TIVX_TARGET_RTOS_MAX_COUNT])
{
    int32_t status;
    app_perf_stats_tiovx_stats_t tiovx_stats;
    uint32_t i, j;

    status = appPerfStatsCpuTiovxStatsGet(app_cpu_id, &tiovx_stats);

    if (status == (vx_status)VX_SUCCESS)
    {
        target_values[0] = tiovx_stats.targets_in_cpu;
        target_values[1] = tiovx_stats.max_targets_per_kernel;
        target_values[2] = tiovx_stats.target_kernel_instance_max;
        target_values[3] = tiovx_stats.target_kernel_max;
        target_values[4] = tiovx_stats.osal_semaphore_max_objects;
        target_values[5] = tiovx_stats.osal_task_max_objects;
        target_values[6] = tiovx_stats.rtos_max_semaphorep;
        target_values[7] = tiovx_stats.rtos_max_taskp;

        tivx_os_obj_sizes_t os_obj_sizes =
        {
            .posix_event_size    = 0U,
            .posix_mutex_size    = 0U,
            .posix_queue_size    = 0U,
            .posix_task_size     = 0U,
            .rtos_sem_obj_size   = tiovx_stats.rtos_sem_object_types[0][0],
            .rtos_sem_pool_size  = tiovx_stats.rtos_sem_object_types[0][1],
            .rtos_task_obj_size  = tiovx_stats.rtos_task_object_types[0][0],
            .rtos_task_pool_size = tiovx_stats.rtos_task_object_types[0][1]
        };

        ownUpdateRemoteEnumTypeSizes((void*)&os_obj_sizes);

        for (i = 0U; i < TIVX_TARGET_RTOS_MAX_COUNT; i++)
        {
            for (j = 0U; j < TIVX_TARGET_RTOS_MAX_COUNT; j++)
            {
                rtos_sem_object_types[i][j] = tiovx_stats.rtos_sem_object_types[i][j];
                rtos_task_object_types[i][j] = tiovx_stats.rtos_task_object_types[i][j];
            }
        }
    }
}

void ownIpcGetCpuMap(uint32_t cpu_id_map[TIVX_CPU_ID_MAX])
{
    uint32_t i;

    for (i = 0; i < (uint32_t)TIVX_CPU_ID_MAX; i++)
    {
        cpu_id_map[i] = g_ipc_cpu_id_map[i];
    }
}
#endif /* #if defined(BUILD_DEV) */

void tivxGetTiovxStats(app_perf_stats_tiovx_stats_t *tiovx_stats);

app_perf_registration_t perf_fxns = {&tivxGetTiovxStats};
app_perf_registration_t * perf_fxns_list = &perf_fxns;

void tivxGetTiovxStats(app_perf_stats_tiovx_stats_t *tiovx_stats)
{
#if defined(BUILD_DEV)
    if (NULL != tiovx_stats)
    {
        uint32_t i;

        for (i = 0; i < TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_MAX_TARGETS_IN_CPU", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->targets_in_cpu = g_tivx_resource_stats_table[i].max_used_value;
            }
            else if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_MAX_TARGETS_PER_KERNEL", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->max_targets_per_kernel = g_tivx_resource_stats_table[i].max_used_value;
            }
            else if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_KERNEL_INSTANCE_MAX", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->target_kernel_instance_max = g_tivx_resource_stats_table[i].max_used_value;
            }
            else if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_KERNEL_MAX", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->target_kernel_max = g_tivx_resource_stats_table[i].max_used_value;
            }
            else if ( strncmp(g_tivx_resource_stats_table[i].name, "RTOS_OSAL_SEMAPHORE_MAX_OBJECTS", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->osal_semaphore_max_objects = g_tivx_resource_stats_table[i].max_used_value;
            }
            else if ( strncmp(g_tivx_resource_stats_table[i].name, "RTOS_OSAL_TASK_MAX_OBJECTS", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->osal_task_max_objects = g_tivx_resource_stats_table[i].max_used_value;
            }
        }

        #if !defined(HLOS)
        appPerfStatsUpdateOSObjectSizes(tiovx_stats);
        tiovx_stats->rtos_sem_object_types[1][0] = TIVX_TYPE_RTOS_OSAL_SEM_OBJ;
        tiovx_stats->rtos_sem_object_types[1][1] = TIVX_TYPE_RTOS_OSAL_SEM_POOL;
        tiovx_stats->rtos_task_object_types[1][0] = TIVX_TYPE_RTOS_OSAL_TASK_OBJ;
        tiovx_stats->rtos_task_object_types[1][1] = TIVX_TYPE_RTOS_OSAL_TASK_POOL;
        #endif /* #if !defined(HLOS) */
    }
#endif /* #if defined(BUILD_DEV) */
}
