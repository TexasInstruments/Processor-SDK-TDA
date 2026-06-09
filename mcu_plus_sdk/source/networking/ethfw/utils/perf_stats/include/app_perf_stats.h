/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef __APP_PERF_STATS_H__
#define __APP_PERF_STATS_H__

#include <stdint.h>
#include <stdio.h>

/**
 * \defgroup apps_utils_perf_stats Performance statistics reporting APIs
 *
 * \brief These APIs allows user to get performance information of RTOS based remote cores
 *
 * \ingroup apps_utils_perf_stats
 *
 * @{
 */

/** \brief Max size of task name string */
#define APP_PERF_STATS_TASK_NAME_MAX (12u)

/** \brief Max number of tasks whoose information can be retrived */
#define APP_PERF_STATS_TASK_MAX (16u)

/**
 * \brief Summary of CPU load
 */
typedef struct {

    uint32_t cpu_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t hwi_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t swi_load; /**< divide by 100 to get load in units of xx.xx % */

} app_perf_stats_cpu_load_t;

/**
 * \brief CPU task statistics information
 */
typedef struct {

    char task_name[APP_PERF_STATS_TASK_NAME_MAX]; /**< task name */
    uint32_t task_load; /**< divide by 100 to get load in units of xx.xx % */

} app_perf_stats_cpu_task_stats_t;

/**
 * \brief Detailed CPU Task Stats information
 */
typedef struct {

    uint32_t num_tasks; /**< Number of tasks in task_stats[] */

    app_perf_stats_cpu_task_stats_t task_stats[APP_PERF_STATS_TASK_MAX]; /**< task level performance info */

} app_perf_stats_task_stats_t;

/**
 * \brief Initialize perf statistics collector module
 *
 *        RTOS only API
 *        MUST be called before any other API
 */
int32_t appPerfStatsInit();

/**
 * \brief Initialize perf statistics collector module
 *
 *        RTOS only API
 *        MUST be called after IPC init
 */
int32_t appPerfStatsRemoteServiceInit();

/**
 * \brief Register a task for task load calculation
 *
 *        Linux, RTOS only API
 *        For linux, this API does nothing as of now
 *        For RTOS, task_handle MUST point to TaskP_Handle
 *
 * \return 0 on success
 */
int32_t appPerfStatsRegisterTask(void *task_handle, char *name);

/**
 * \brief De-Initialize perf statistics collector module
 *
 *        RTOS only API
 *
 * \return 0 on success
 */
int32_t appPerfStatsDeInit();

/*! @} */

#endif
