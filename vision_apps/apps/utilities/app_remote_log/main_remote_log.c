/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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
#include <string.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/file_io/include/app_fileio.h>
#include <utils/ipc/include/app_ipc.h>
#include <stdint.h>

#include <app_mem_map.h>
#include <app_cfg.h>
#include <common.h>

#if defined(QNX)
#include <sys/slog.h>
#define _SLOG_VISION_APPS 50
#endif

extern core_config_t core_config[];
extern uint8_t num_cpus;

#if 0
#if defined(QNX)
void app_log_device_send_string(char *string, uint32_t max_size)
{
    slogf(_SLOG_VISION_APPS,_SLOG_INFO,string);
}
#endif
#endif


#if defined(LINUX) || defined(QNX)
void app_log_device_send_string(char *string, uint32_t max_size)
{
    printf(string);
}
#endif

int main(void)
{
    /*******************  Create tasks for log & file io ***************/
    app_log_init_prm_t log_init_prm;
    app_fileio_init_prm_t fileio_init_prm;

    appLogInitPrmSetDefault(&log_init_prm);
    appFileIOInitPrmSetDefault(&fileio_init_prm);

    for (int i = 0; i < num_cpus; i++)
    {
        if (core_config[i].enable_log_rd_cpu)
        {
            log_init_prm.log_rd_cpu_enable[core_config[i].cpu] = 1;
        }
        if (core_config[i].enable_fileio)
        {
            fileio_init_prm.fileio_rd_cpu_enable[core_config[i].cpu] = 1;
        }
    }

    log_init_prm.shared_mem = (app_log_shared_mem_t *)APP_LOG_MEM_ADDR;
    log_init_prm.self_cpu_index = APP_IPC_CPU_MPU1_0;
    strncpy(log_init_prm.self_cpu_name, "MPU1_0", APP_LOG_MAX_CPU_NAME);
    log_init_prm.log_rd_max_cpus = APP_IPC_CPU_MAX;
    log_init_prm.device_write = app_log_device_send_string;

    fileio_init_prm.shared_mem = (app_fileio_shared_mem_t *)APP_FILEIO_MEM_ADDR;
    fileio_init_prm.self_cpu_index = APP_IPC_CPU_MPU1_0;
    strncpy(fileio_init_prm.self_cpu_name, "MPU1_0", APP_FILEIO_MAX_CPU_NAME);
    fileio_init_prm.fileio_rd_max_cpus = APP_IPC_CPU_MAX;

    appLogRdInit(&log_init_prm);
    appFileIORdInit(&fileio_init_prm);

    while(1)
    {
        appLogWaitMsecs(1000);
    }

    return 0;
}
