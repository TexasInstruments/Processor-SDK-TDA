/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include <vx_perf_stats_utils.h>

/*  ################################
    UTILS FOR PERFORMANCE STATISTICS
    ################################
*/

/**************** CPU LOAD *************************/
void perfStatsCpuStatsInit(perfStatsCpuLoad *cpuLoad)
{
    cpuLoad->total_time = 0;
    cpuLoad->busy_time = 0;
    cpuLoad->irq_time = 0;
    cpuLoad->softirq_time = 0;
}

void perfStatsReadProcStat(uint64_t cnt[], uint32_t num_cnt)
{
    uint32_t i;
    FILE *fp;
    char str[256];
    const char d[2] = " ";
    char* token;
    char* status = NULL;

    for(i=0; i<num_cnt; i++)
    {
        cnt[i] = 0;
    }

    fp = fopen("/proc/stat","r");
    if(fp != NULL)
    {
        str[0] = 0;
        status = fgets(str,256,fp);
        fclose(fp);

        if (status != NULL)
        {
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
}

void perfStatsCpuLoadCalc(perfStatsCpuLoad *cpuLoad)
{
#define NUM_PROC_STAT_COUNTERS  (10u)

    static uint32_t is_first_time = 1;
    static uint64_t last_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t cur_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t diff_cnt[NUM_PROC_STAT_COUNTERS];
    uint64_t total_time;
    uint32_t i;

    /*
     *  cat /proc/stat
     *       0    1    2      3    4       5    6       7     8     9
     *       user nice system idle iowait  irq  softirq steal guest guest_nice
     *  cpu  4705 356  584    3699   23    23     0       0     0          0
     */
    if(is_first_time)
    {
        is_first_time = 0;

        perfStatsReadProcStat(last_cnt, NUM_PROC_STAT_COUNTERS);
    }

    perfStatsReadProcStat(cur_cnt, NUM_PROC_STAT_COUNTERS);

    total_time = 0;
    for(i=0; i<NUM_PROC_STAT_COUNTERS; i++)
    {
        diff_cnt[i] = cur_cnt[i] - last_cnt[i];
        total_time += diff_cnt[i];
    }

    cpuLoad->total_time   += total_time;
    cpuLoad->busy_time    += (total_time - (diff_cnt[3]+diff_cnt[4]));
    cpuLoad->irq_time     += diff_cnt[5];
    cpuLoad->softirq_time += diff_cnt[6];

    for(i=0; i<NUM_PROC_STAT_COUNTERS; i++)
    {
        last_cnt[i] = cur_cnt[i];
    }

    if (0 == cpuLoad->total_time)
    {
        cpuLoad->total_time = 1; //avoid division by zero
    }

    cpuLoad->cpu_load = (cpuLoad->busy_time*10000)/cpuLoad->total_time;
    cpuLoad->hwi_load = (cpuLoad->irq_time*10000)/cpuLoad->total_time;
    cpuLoad->swi_load = (cpuLoad->softirq_time*10000)/cpuLoad->total_time;

}

void perfStatsResetCpuLoadCalc(perfStatsCpuLoad *cpuLoad)
{
    cpuLoad->total_time = 0;
    cpuLoad->busy_time = 0;
    cpuLoad->irq_time = 0;
    cpuLoad->softirq_time = 0;
}

void perfStatsCpuStatsPrint(perfStatsCpuLoad *cpu_load)
{
    printf("Summary of CPU load,\n");
    printf("====================\n\n");

    perfStatsCpuLoadCalc(cpu_load);

    printf("CPU: %6s: TOTAL LOAD = %3d.%2d %% ( HWI = %3d.%2d %%, SWI = %3d.%2d %% )\n",
                "mpu1_0",
                cpu_load->cpu_load/100u,
                cpu_load->cpu_load%100u,
                cpu_load->hwi_load/100u,
                cpu_load->hwi_load%100u,
                cpu_load->swi_load/100u,
                cpu_load->swi_load%100u
            );
    printf("\n");
}

/**************** DDR LOAD *************************/
#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2) || defined(SOC_TDA54)
#define PERF_DDR_MHZ                (2133u)  /* DDR clock speed in MHZ */
#elif defined (SOC_AM62A)
#define PERF_DDR_MHZ                (1866u)  /* DDR clock speed in MHZ */
#elif defined (SOC_AM62X)
#define PERF_DDR_MHZ                (800u)  /* DDR clock speed in MHZ */
#elif defined (SOC_AM62P)
#define PERF_DDR_MHZ                (1600u)  /* DDR clock speed in MHZ */
#endif

#if defined (SOC_AM62X)
#define PERF_DDR_BUS_WIDTH          (  16u)  /* in units of bits */
#else
#define PERF_DDR_BUS_WIDTH          (  32u)  /* in units of bits */
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2) || defined(SOC_AM62A) || defined(SOC_AM62P) || defined(SOC_TDA54)
#define PERF_DDR_BURST_SIZE_BYTES   (  64u)  /* in units of bytes */
#elif defined (SOC_AM62X)
#define PERF_DDR_BURST_SIZE_BYTES   (  32u)  /* in units of bytes */
#endif

#define PERF_DDR_STATS_CTR0         (0x00) /* A value of 0x00 configures counter 0 to return number of write transactions  */
#define PERF_DDR_STATS_CTR1         (0x01) /* A value of 0x01 configures counter 1 to return number of read transactions   */
/* Use counter 2 and 3 to provide stats other than read/write transactions */
#define PERF_DDR_STATS_CTR2         (0x03) /* A value of 0x03 configures counter 3 to return number of command activations */
#define PERF_DDR_STATS_CTR3         (0x1C) /* A value of 0x1C configures counter 4 to return number of queue full states   */

#if defined(SOC_J721E) || defined(SOC_AM62A) || defined(SOC_AM62X) || defined(SOC_J722S) || defined(SOC_AM62P)
#define PERF_NUM_DDR_INSTANCES      (1u)
#elif defined (SOC_J721S2) || defined(SOC_J742S2)
#define PERF_NUM_DDR_INSTANCES      (2u)
#elif defined (SOC_J784S4) || defined (SOC_TDA54)
#define PERF_NUM_DDR_INSTANCES      (4u)
#endif

/* Specify the duration for with counter2 and counter3 values are to be accumulated before printing */
#define PERF_SNAPSHOT_WINDOW_WIDTH (500000 * 4) /* Configured for 2 seconds */

typedef struct {
    perf_stats_ddr_stats_t ddr_stats;
    uint64_t total_time;
    uint64_t last_timestamp;
    uint64_t total_read;
    uint64_t total_write;
    int32_t snapshot_count;
} perf_stats_ddr_load_t;

static perf_stats_ddr_load_t g_perf_stats_ddr_load_obj;

uint64_t getTimeInUsec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec/1000;
}

void perfStatsResetDdrLoadCalcAll()
{
    perf_stats_ddr_load_t *ddrLoad = &g_perf_stats_ddr_load_obj;

    ddrLoad->ddr_stats.read_bw_avg = 0;
    ddrLoad->ddr_stats.write_bw_avg = 0;
    ddrLoad->ddr_stats.read_bw_peak = 0;
    ddrLoad->ddr_stats.write_bw_peak = 0;
    ddrLoad->ddr_stats.total_available_bw = PERF_DDR_MHZ*PERF_DDR_BUS_WIDTH*PERF_NUM_DDR_INSTANCES*2/8;
    ddrLoad->total_time = 0;
    ddrLoad->total_read = 0;
    ddrLoad->total_write = 0;
    ddrLoad->last_timestamp = getTimeInUsec();
    ddrLoad->snapshot_count = PERF_SNAPSHOT_WINDOW_WIDTH;

    ddrLoad->ddr_stats.counter0_total = 0;
    ddrLoad->ddr_stats.counter1_total = 0;
    ddrLoad->ddr_stats.counter2_total = 0;
    ddrLoad->ddr_stats.counter3_total = 0;
}

/* read EMIF counter and calculate read and write bytes since last read */
void perfStatsDdrStatsReadCounters(uint32_t *val0, uint32_t *val1, uint32_t *val2, uint32_t *val3)
{
    static uint32_t is_first_time = 1;
    static int fd = -1;
    static void *base;
    static volatile uint32_t *cnt_sel[PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt0[PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt1[PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt2[PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt3[PERF_NUM_DDR_INSTANCES];

    static volatile uint32_t last_cnt0 = 0, last_cnt1 = 0, last_cnt2 = 0, last_cnt3 = 0;
    volatile uint32_t cur_cnt0 = 0, cur_cnt1 = 0, cur_cnt2 = 0, cur_cnt3 = 0;
    uint32_t diff_cnt0, diff_cnt1, diff_cnt2, diff_cnt3, ddr_inst;

    if(is_first_time)
    {
        is_first_time = 0;
        fd = open("/dev/mem", O_RDWR | O_SYNC);
        base = mmap(0, 0xF400000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x0);

        #if defined(SOC_AM62A) || defined(SOC_AM62X) || defined(SOC_J722S) || defined(SOC_AM62P)
        cnt_sel[0] = (volatile uint32_t *)(base + 0x0f300000);
        cnt0[0]    = (volatile uint32_t *)(base + 0x0f300104);
        cnt1[0]    = (volatile uint32_t *)(base + 0x0f300108);
        cnt2[0]    = (volatile uint32_t *)(base + 0x0f30010C);
        cnt3[0]    = (volatile uint32_t *)(base + 0x0f300110);
        #else
        cnt_sel[0] = (volatile uint32_t *)(base + 0x02980100);
        cnt0[0]    = (volatile uint32_t *)(base + 0x02980104);
        cnt1[0]    = (volatile uint32_t *)(base + 0x02980108);
        cnt2[0]    = (volatile uint32_t *)(base + 0x0298010C);
        cnt3[0]    = (volatile uint32_t *)(base + 0x02980110);
        #endif

        #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        cnt_sel[1] = (volatile uint32_t *)(base + 0x029A0100);
        cnt0[1]    = (volatile uint32_t *)(base + 0x029A0104);
        cnt1[1]    = (volatile uint32_t *)(base + 0x029A0108);
        cnt2[1]    = (volatile uint32_t *)(base + 0x029A010C);
        cnt3[1]    = (volatile uint32_t *)(base + 0x029A0110);
        #endif

        #if defined(SOC_J784S4)
        cnt_sel[2] = (volatile uint32_t *)(base + 0x029C0100);
        cnt0[2]    = (volatile uint32_t *)(base + 0x029C0104);
        cnt1[2]    = (volatile uint32_t *)(base + 0x029C0108);
        cnt2[2]    = (volatile uint32_t *)(base + 0x029C010C);
        cnt3[2]    = (volatile uint32_t *)(base + 0x029C0110);

        cnt_sel[3] = (volatile uint32_t *)(base + 0x029E0100);
        cnt0[3]    = (volatile uint32_t *)(base + 0x029E0104);
        cnt1[3]    = (volatile uint32_t *)(base + 0x029E0108);
        cnt2[3]    = (volatile uint32_t *)(base + 0x029E010C);
        cnt3[3]    = (volatile uint32_t *)(base + 0x029E0110);
        #endif

        for (ddr_inst = 0; ddr_inst < PERF_NUM_DDR_INSTANCES; ddr_inst++)
        {
            /* cnt0 is counting reads, cnt1 is counting writes, cnt2, cnt3 not used */
            *cnt_sel[ddr_inst] = (PERF_DDR_STATS_CTR0 <<  0u) |
                       (PERF_DDR_STATS_CTR1 <<  8u) |
                       (PERF_DDR_STATS_CTR2 << 16u) |
                       (PERF_DDR_STATS_CTR3 << 24u);

            last_cnt0 += *cnt0[ddr_inst];
            last_cnt1 += *cnt1[ddr_inst];
            last_cnt2 += *cnt2[ddr_inst];
            last_cnt3 += *cnt3[ddr_inst];
        }
    }

    for (ddr_inst = 0; ddr_inst < PERF_NUM_DDR_INSTANCES; ddr_inst++)
    {
        cur_cnt0 += *cnt0[ddr_inst];
        cur_cnt1 += *cnt1[ddr_inst];
        cur_cnt2 += *cnt2[ddr_inst];
        cur_cnt3 += *cnt3[ddr_inst];
    }

    if(cur_cnt0 < last_cnt0)
    {
        /* wrap around case */
        diff_cnt0 = (0xFFFFFFFFu - last_cnt0) + cur_cnt0;
    }
    else
    {
        diff_cnt0 = cur_cnt0 - last_cnt0;
    }

    if(cur_cnt1 < last_cnt1)
    {
        /* wrap around case */
        diff_cnt1 = (0xFFFFFFFFu - last_cnt1) + cur_cnt1;
    }
    else
    {
        diff_cnt1 = cur_cnt1 - last_cnt1;
    }

    if(cur_cnt2 < last_cnt2)
    {
        /* wrap around case */
        diff_cnt2 = (0xFFFFFFFFu - last_cnt2) + cur_cnt2;
    }
    else
    {
        diff_cnt2 = cur_cnt2 - last_cnt2;
    }

    if(cur_cnt3 < last_cnt3)
    {
        /* wrap around case */
        diff_cnt3 = (0xFFFFFFFFu - last_cnt3) + cur_cnt3;
    }
    else
    {
        diff_cnt3 = cur_cnt3 - last_cnt3;
    }


    last_cnt0 = cur_cnt0;
    last_cnt1 = cur_cnt1;
    last_cnt2 = cur_cnt2;
    last_cnt3 = cur_cnt3;

    *val0 = (uint32_t)diff_cnt0;
    *val1 = (uint32_t)diff_cnt1;
    *val2 = (uint32_t)diff_cnt2;
    *val3 = (uint32_t)diff_cnt3;
}

perf_stats_ddr_stats_t *perfStatsDdrStatsGet()
{
    perf_stats_ddr_load_t *ddrLoad = &g_perf_stats_ddr_load_obj;
    uint32_t val0 = 0, val1 = 0, val2 = 0, val3 = 0;
    uint64_t cur_time;
    uint32_t elapsed_time;

    cur_time = getTimeInUsec();

    if(cur_time > ddrLoad->last_timestamp)
    {
        elapsed_time = cur_time - ddrLoad->last_timestamp;
        if(elapsed_time==0)
            elapsed_time = 1; /* to avoid divide by 0 */
        ddrLoad->total_time += elapsed_time;

        perfStatsDdrStatsReadCounters(&val0, &val1, &val2, &val3);

        uint64_t write_bytes = val0 * PERF_DDR_BURST_SIZE_BYTES;
        uint64_t read_bytes  = val1 * PERF_DDR_BURST_SIZE_BYTES;

        ddrLoad->total_read += read_bytes;
        ddrLoad->total_write += write_bytes;

        ddrLoad->ddr_stats.read_bw_avg = (ddrLoad->total_read/ddrLoad->total_time); /* in MB/s */
        ddrLoad->ddr_stats.write_bw_avg = (ddrLoad->total_write/ddrLoad->total_time); /* in MB/s */

        uint32_t read_bw_peak = read_bytes/elapsed_time; /* in MB/s */
        uint32_t write_bw_peak = write_bytes/elapsed_time; /* in MB/s */
        if(read_bw_peak > ddrLoad->ddr_stats.read_bw_peak)
            ddrLoad->ddr_stats.read_bw_peak = read_bw_peak;
        if(write_bw_peak > ddrLoad->ddr_stats.write_bw_peak)
            ddrLoad->ddr_stats.write_bw_peak = write_bw_peak;
    }

    ddrLoad->last_timestamp = cur_time;

    return &(ddrLoad->ddr_stats);
}

void perfStatsDdrStatsPrintAll()
{
    perf_stats_ddr_stats_t *ddr_load;
    ddr_load = perfStatsDdrStatsGet();

    printf("DDR performance statistics,\n");
    printf("===========================\n\n");

    printf("DDR: READ  BW: AVG = %6d MB/s\n",
        ddr_load->read_bw_avg);
    printf("DDR: WRITE BW: AVG = %6d MB/s\n",
        ddr_load->write_bw_avg);
    printf("DDR: TOTAL BW: AVG = %6d MB/s",
        ddr_load->read_bw_avg+ddr_load->write_bw_avg);
    printf("\n");
}

/**************** SOC Temp *************************/
void perfStatsSocTempInit(perfStatsSOCTemp *socTemp)
{

#if defined(SOC_J721E)
    char mapping[NUM_THERMAL_ZONE][16] = {"WKUP","CPU","C7x","GPU","R5F"};
#elif defined(SOC_J721S2)
    char mapping[NUM_THERMAL_ZONE][16] = {"MCU_R5F","MCU","CODEC","DPHY","R5F","CPU","C7x"};
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
    char mapping[NUM_THERMAL_ZONE][16] = {"MCU_R5F","MCU","GPU","C7x","CPU","C7x","DDR"};
#elif defined(SOC_AM62A)
    char mapping[NUM_THERMAL_ZONE][16] = {"DDR","CPU","C7x"};
#else
    char mapping[0][16] = {};
#endif

    for (uint32_t i = 0; i < NUM_THERMAL_ZONE; i++)
    {
        socTemp->thermal_zone_temp[i] = 0.0;
        if (i < sizeof(mapping) / sizeof(mapping[0]))
        {
            sprintf(socTemp->thermal_zone_name[i], "thermal_zone%d(%s)", i, mapping[i]);
        }
        else
        {
            sprintf(socTemp->thermal_zone_name[i], "thermal_zone%d", i);
        }
    }
}

void perfStatsSocTempGet(perfStatsSOCTemp *socTemp)
{
    uint32_t i, ret = 0;
    FILE *fp;
    char fileName[50];

    for (i = 0; i < NUM_THERMAL_ZONE; i++)
    {
        uint32_t temp;

        sprintf(fileName, "/sys/class/thermal/thermal_zone%d/temp", i);

        fp = fopen(fileName, "rb");
        if (NULL != fp)
        {
            ret = fscanf(fp, "%u", &temp);
            if (1 == ret)
            {
                socTemp->thermal_zone_temp[i] = temp/1000.0;
            }
            fclose(fp);
        }
    }
}
