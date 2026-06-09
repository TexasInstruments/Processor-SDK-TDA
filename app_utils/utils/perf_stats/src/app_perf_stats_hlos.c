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
#include <stdlib.h>
#include <string.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <pthread.h>
#include "app_perf_stats_priv.h"

#if defined(QNX)
#include <dirent.h>
#include <sys/procfs.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <fcntl.h>
#endif

typedef struct {

    uint64_t total_time;
    uint64_t busy_time;
    uint64_t irq_time;
    uint64_t softirq_time;

} app_perf_cpu_load_stats_t;

typedef struct {

    pthread_mutex_t lock;
    app_perf_stats_hwa_stats_t hwaLoad;

    app_perf_cpu_load_stats_t cpuLoad;


} app_perf_stats_obj_t;

static app_perf_stats_obj_t g_app_perf_stats_obj;

void appPerfStatsResetHwaLoadCalcAll();

void appPerfStatsLock(app_perf_stats_obj_t *obj)
{
   pthread_mutex_lock(&obj->lock);
}

void appPerfStatsUnLock(app_perf_stats_obj_t *obj)
{
    pthread_mutex_unlock(&obj->lock);
}

#if defined(LINUX)
void appPerfStatsReadProcStat(uint64_t cnt[], uint32_t num_cnt)
{
    uint32_t i;
    FILE *fp;
    char str[256];
    const char d[2] = " ";
    char* token;

    for(i=0; i<num_cnt; i++)
    {
        cnt[i] = 0;
    }

    fp = fopen("/proc/stat","r");
    if(fp != NULL)
    {
        str[0] = 0;
        fgets(str,256,fp);
        fclose(fp);

        token = strtok(str,d);

        i = 0;
        while(token!=NULL && i < num_cnt)
        {
            token = strtok(NULL,d);
            if(token!=NULL)
            {
                cnt[i] = atoi(token);
                i++;
            }
        }
    }
}
#endif
#if defined(QNX)
void appPerfStatsReadProcStat(uint64_t cnt[], uint32_t num_cnt)
{
    /*
     * The only supported values in QNX via DCMD_PROC_INFO and DCMD_PROC_TIDSTATUS
     * are:
     *  i         0     2       3
     *  cnt[i]    user  system  idle
     */
    uint32_t i;
    char buffer[512];
    procfs_info info;
    procfs_status p_status;
    int procFd;
    DIR* dirp;
    struct dirent* direntp;

    for(i=0; i<num_cnt; i++)
    {
        cnt[i] = 0;
    }

    /* /proc/stat is not available in QNX, so we read /proc/<pid>/as for each process.
     * We will sum up the user and system time for all processes, and idle time from
     * the OS idle threads for pid=1.
     */
    dirp = opendir("/proc");
    if (dirp != NULL)
    {
        direntp = readdir( dirp );
        while ((direntp = readdir(dirp)) != NULL)
        {
            int pid = atoi(direntp->d_name);
            if(0 != pid)
            {
                sprintf(buffer, "/proc/%d/as", pid);
                procFd = open(buffer, O_RDONLY);
                if (procFd != -1)
                {
                    int status = devctl(procFd, DCMD_PROC_INFO, &info, sizeof(info), NULL);
                    if (status == EOK)
                    {
                        /* For the kernel process (pid=1), the idle threads are tids 1 to num_cpu.
                         * Sum up to find the total idle time. */
                        if (info.pid == 1)
                        {
                            for(i = 1; i <= _syspage_ptr->num_cpu; i++)
                            {
                                p_status.tid = i;
                                if(EOK == devctl(procFd, DCMD_PROC_TIDSTATUS, &p_status, sizeof(p_status), NULL))
                                {
                                    cnt[3] += p_status.sutime;
                                }
                            }
                        }
                        /* Record and sum the user and system time for all other PIDs. */
                        else
                        {
                            cnt[0] += info.utime;
                            cnt[2] += info.stime;
                        }
                    }
                    close(procFd);
                }
            }
        }
        closedir( dirp );
    }
}
#endif

void appPerfStatsCpuLoadCalc()
{
#define NUM_PROC_STAT_COUNTERS  (10u)

    static uint32_t is_first_time = 1;
    static uint64_t last_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t cur_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t diff_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t total_time;
    uint32_t i;
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    if(is_first_time)
    {
        is_first_time = 0;

        appPerfStatsReadProcStat(last_cnt, NUM_PROC_STAT_COUNTERS);
    }

    /*
     *  cat /proc/stat
     *       0    1    2      3    4       5    6       7     8     9
     *       user nice system idle iowait  irq  softirq steal guest guest_nice
     *  cpu  4705 356  584    3699   23    23     0       0     0          0
     */
    appPerfStatsReadProcStat(cur_cnt, NUM_PROC_STAT_COUNTERS);

    total_time = 0;
    for(i=0; i<NUM_PROC_STAT_COUNTERS; i++)
    {
        diff_cnt[i] = cur_cnt[i] - last_cnt[i];
        total_time += diff_cnt[i];
    }

    appPerfStatsLock(obj);

    obj->cpuLoad.total_time   = total_time;
    obj->cpuLoad.busy_time    = (total_time - (diff_cnt[3]+diff_cnt[4]));
    obj->cpuLoad.irq_time     = diff_cnt[5];
    obj->cpuLoad.softirq_time = diff_cnt[6];

    appPerfStatsUnLock(obj);

    for(i=0; i<NUM_PROC_STAT_COUNTERS; i++)
    {
        last_cnt[i] = cur_cnt[i];
    }
}

void appPerfStatsResetCpuLoadCalc(app_perf_stats_obj_t *obj)
{
    appPerfStatsLock(obj);

    obj->cpuLoad.total_time = 0;
    obj->cpuLoad.busy_time = 0;
    obj->cpuLoad.irq_time = 0;
    obj->cpuLoad.softirq_time = 0;

    appPerfStatsUnLock(obj);
}

int32_t appPerfStatsHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t status = 0;
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    switch(cmd)
    {
        case APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC:
            appPerfStatsResetHwaLoadCalcAll();
            break;
        case APP_PERF_STATS_CMD_GET_HWA_LOAD:
            if(prm_size == sizeof(app_perf_stats_hwa_stats_t))
            {
                app_perf_stats_hwa_stats_t *hwa_load = (app_perf_stats_hwa_stats_t*)prm;

                appPerfStatsLock(obj);

                *hwa_load = obj->hwaLoad;

                appPerfStatsUnLock(obj);

            }
            break;
        case APP_PERF_STATS_CMD_RESET_LOAD_CALC:
            appPerfStatsResetCpuLoadCalc(obj);
            break;
        case APP_PERF_STATS_CMD_GET_CPU_LOAD:
            if(prm_size == sizeof(app_perf_stats_cpu_load_t))
            {
                app_perf_stats_cpu_load_t *cpu_load = (app_perf_stats_cpu_load_t*)prm;
                uint64_t total_time;

                appPerfStatsCpuLoadCalc();

                appPerfStatsLock(obj);

                total_time = obj->cpuLoad.total_time;

                if(total_time==0)
                {
                    total_time = 1; /* to avoid divide by 0 */
                }

                cpu_load->cpu_load = (obj->cpuLoad.busy_time*10000)/obj->cpuLoad.total_time;
                cpu_load->hwi_load = (obj->cpuLoad.irq_time*10000)/obj->cpuLoad.total_time;
                cpu_load->swi_load = (obj->cpuLoad.softirq_time*10000)/obj->cpuLoad.total_time;

                appPerfStatsUnLock(obj);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_cpu_load_t)
                    );
            }
            break;

        default:
            status = -1;
            printf("PERF STATS: ERROR: Invalid command (cmd = %08x, prm_size = %d B\n",
                cmd,
                prm_size
                );
            break;
    }

    return status;
}

int32_t appPerfStatsInit()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    int32_t status = 0;

    memset(obj, 0, sizeof(app_perf_stats_obj_t));

    pthread_mutexattr_t mutex_attr;

    status |= pthread_mutexattr_init(&mutex_attr);
    if(status==0)
    {
        status |= pthread_mutex_init(&obj->lock, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }
    if(status!=0)
    {
        printf("PERF STATS: Unable to create lock mutex\n");
        status = -1;
    }
    if(status==0)
    {
        appPerfStatsResetHwaLoadCalcAll();
        appPerfStatsCpuLoadCalc();
    }

    return status;
}

int32_t appPerfStatsRemoteServiceInit()
{
    int32_t status;

    status = appRemoteServiceRegister(APP_PERF_STATS_SERVICE_NAME, appPerfStatsHandler);
    if(status!=0)
    {
        printf("PERF STATS: ERROR: Unable to register service \n");
    }
    return status;
}

int32_t appPerfStatsDeInit()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    appRemoteServiceUnRegister(APP_PERF_STATS_SERVICE_NAME);
    pthread_mutex_destroy(&obj->lock);
    return 0;
}

void appPerfStatsHwaResetLoadCalc(app_perf_hwa_id_t id)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_hwa_load_t *hwaLoad;

    if(id < APP_PERF_HWA_MAX)
    {
        hwaLoad = &obj->hwaLoad.hwa_stats[id];

        appPerfStatsLock(obj);

        hwaLoad->total_time = 0;
        hwaLoad->active_time = 0;
        hwaLoad->pixels_processed = 0;
        hwaLoad->last_timestamp = appLogGetTimeInUsec();

        appPerfStatsUnLock(obj);
    }
}

void appPerfStatsResetHwaLoadCalcAll()
{
    app_perf_hwa_id_t hwa_id;

    for(hwa_id = (app_perf_hwa_id_t)0; hwa_id <APP_PERF_HWA_MAX;hwa_id++)
    {
        appPerfStatsHwaResetLoadCalc(hwa_id);
    }
}

void appPerfStatsHwaUpdateLoad(app_perf_hwa_id_t id, uint32_t active_time_in_usecs, uint32_t pixels_processed)
{
    #ifndef VDK
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_hwa_load_t *hwaLoad;
    uint64_t cur_time;

    if(id < APP_PERF_HWA_MAX)
    {
        hwaLoad = &obj->hwaLoad.hwa_stats[id];

        appPerfStatsLock(obj);

        cur_time = appLogGetTimeInUsec();

        if(cur_time > hwaLoad->last_timestamp)
        {
            hwaLoad->total_time = (cur_time - hwaLoad->last_timestamp);
            hwaLoad->active_time = active_time_in_usecs;
            hwaLoad->pixels_processed = pixels_processed;
        }
        hwaLoad->last_timestamp = cur_time;

        appPerfStatsUnLock(obj);
    }
    #endif
}

