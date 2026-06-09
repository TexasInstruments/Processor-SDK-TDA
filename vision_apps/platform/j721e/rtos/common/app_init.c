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


#include <app_mem_map.h>
#include APP_CFG_FILE
#include <app.h>
#include <stdio.h>
#include <string.h>

/* Vision_apps utils header files */
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/console_io/include/app_log.h>
#include <utils/file_io/include/app_fileio.h>
#include <utils/console_io/include/app_cli.h>
#include <utils/misc/include/app_misc.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/timer/include/app_timer.h>
#include <utils/mpu/include/app_mpu.h>

#if defined(ENABLE_FVID2)
#include <utils/hwa/include/app_hwa.h>
#endif

#if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
#include <utils/sensors/include/app_sensors.h>
#include <utils/iss/include/app_iss.h>
#endif

#ifdef ENABLE_UDMA
#include <utils/udma/include/app_udma.h>
#endif

#if defined(ENABLE_DSS_SINGLE) || defined(ENABLE_DSS_DUAL)
#include <utils/dss/include/app_dss_defaults.h>
#endif

#ifdef ENABLE_SCICLIENT
#include <utils/sciclient/include/app_sciclient.h>
#endif

#ifdef ENABLE_ETHFW
#include <utils/ethfw/include/app_ethfw.h>
#endif

/* TIOVX header files */
#include <TI/tivx.h>

/* Vision_apps custom kernel header files */
#include <TI/tivx_img_proc.h>
#if defined(C66) || defined(C71)
#include <TI/tivx_srv.h>
#include <TI/tivx_stereo.h>
#endif

/* Imaging header files */
#if defined(ENABLE_TIOVX)

#if defined(ENABLE_VHWA_VPAC)
#include <TI/j7_imaging_aewb.h>
#include <TI/hwa_vpac_viss.h>
#include <TI/hwa_vpac_ldc.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_vpac_nf.h>
#include <TI/hwa_vpac_fc.h>
#endif

#if defined(ENABLE_VHWA_DMPAC)
#include <TI/hwa_dmpac_sde.h>
#endif

#if defined(ENABLE_VHWA_DMPAC) || defined(C66)
#include <TI/hwa_dmpac_dof.h>
#endif

#if defined(ENABLE_CSI2RX)
#include <TI/video_io_capture.h>
#endif

#if defined(ENABLE_DSS_SINGLE) || defined(ENABLE_DSS_DUAL)
#include <TI/video_io_display.h>
#include <TI/video_io_display_m2m.h>
#endif

#if defined(ENABLE_CSI2TX)
#include <TI/video_io_csitx.h>
#endif

#ifdef C71
#include <TI/dl_kernels.h>
#endif

#endif /* #if defined(ENABLE_TIOVX) */


/* PDK header files */
#ifdef ENABLE_BOARD
/* This header is only needed for the definition of UTRUE used */
#include <ti/csl/csl_types.h>
#include <ti/board/board.h>
#endif

#ifdef ENABLE_UART
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#endif

extern app_perf_registration_t * perf_fxns_list;

app_log_shared_mem_t g_app_log_shared_mem
__attribute__ ((section(".bss:app_log_mem")))
__attribute__ ((aligned(4096)))
        ;

app_fileio_shared_mem_t g_app_fileio_shared_mem
__attribute__ ((section(".bss:app_fileio_mem")))
__attribute__ ((aligned(4096)))
        ;

uint8_t g_tiovx_obj_desc_mem[TIOVX_OBJ_DESC_MEM_SIZE]
__attribute__ ((section(".bss:tiovx_obj_desc_mem")))
__attribute__ ((aligned(4096)))
        ;

uint8_t g_ipc_vring_mem[IPC_VRING_MEM_SIZE]
__attribute__ ((section(".bss:ipc_vring_mem")))
__attribute__ ((aligned(4096)))
        ;

uint8_t g_ddr_local_mem[DDR_HEAP_MEM_SIZE]
__attribute__ ((section(".bss:ddr_local_mem")))
__attribute__ ((aligned(4096)))
        ;

#ifdef L1_MEM_SIZE
uint8_t g_l1_mem[L1_MEM_SIZE]
__attribute__ ((section(".bss:l1mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef L2_MEM_SIZE
uint8_t g_l2_mem[L2_MEM_SIZE]
__attribute__ ((section(".bss:l2mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef L3_MEM_SIZE
uint8_t g_l3_mem[L3_MEM_SIZE]
__attribute__ ((section(".bss:l3mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_SCRATCH_SIZE
uint8_t g_ddr_scratch_mem[DDR_SCRATCH_SIZE]
__attribute__ ((section(".bss:ddr_scratch_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_HEAP_NON_CACHE_MEM_SIZE
uint8_t g_ddr_non_cache_mem[DDR_HEAP_NON_CACHE_MEM_SIZE]
__attribute__ ((section(".bss:ddr_non_cache_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_VISS_HEAP_MEM_SIZE
uint8_t g_ddr_cache_wt_mem[DDR_VISS_HEAP_MEM_SIZE]
__attribute__ ((section(".bss:ddr_cache_wt_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef ENABLE_TIOVX
#ifdef ENABLE_TIOVX_HOST
uint8_t g_tiovx_host_shared_mem[TIOVX_HOST_DDR_SHARED_MEM_SIZE]
__attribute__ ((section(".bss:tiovx_host_shared_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif
#endif

static void appRegisterOpenVXTargetKernels();
static void appUnRegisterOpenVXTargetKernels();
void appRtosTestRegister();
void appRtosTestUnRegister();

#ifdef ENABLE_UART
void appLogDeviceWrite(char *string, uint32_t max_size)
{
    UART_puts(string, max_size);
}
#endif

int32_t appInit()
{
    int32_t status = 0;
    app_mem_init_prm_t mem_init_prm;
    app_log_init_prm_t log_init_prm;
    app_fileio_init_prm_t fileio_init_prm;
    app_ipc_init_prm_t ipc_init_prm;

    app_mem_heap_prm_t *heap_prm;

    #ifdef ENABLE_IPC
    uint32_t host_os_type;
    void *ipc_resource_table = NULL;
    #endif

    /* Init and start GTC timer */
    status = appLogGlobalTimeInit();
    APP_ASSERT_SUCCESS(status);

    #ifdef ENABLE_IPC
    /* appGetIpcResourceTable() returns NULL in RTOS only mode and returns a valid resource table
     * in Linux+RTOS mode
     */
    ipc_resource_table = appGetIpcResourceTable();
    host_os_type = appGetHostOSType();
    #endif

    appMemInitPrmSetDefault(&mem_init_prm);
    appLogInitPrmSetDefault(&log_init_prm);
    appFileIOInitPrmSetDefault(&fileio_init_prm);
    appIpcInitPrmSetDefault(&ipc_init_prm);

    mem_init_prm.virtToPhyFxn = appUdmaVirtToPhyAddrConversion;

    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_DDR];
    heap_prm->base = g_ddr_local_mem;
    strncpy(heap_prm->name, "DDR_LOCAL_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = DDR_HEAP_MEM_SIZE;
    heap_prm->flags = 0;

    #ifdef L1_MEM_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_L1];
    heap_prm->base = g_l1_mem;
    strncpy(heap_prm->name, "L1_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = L1_MEM_SIZE;
    heap_prm->flags = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE;
    #endif
    #ifdef L2_MEM_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_L2];
    heap_prm->base = g_l2_mem;
    strncpy(heap_prm->name, "L2_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = L2_MEM_SIZE;
    heap_prm->flags = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE;
    #endif
    #ifdef L3_MEM_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_L3];
    heap_prm->base = g_l3_mem;
    strncpy(heap_prm->name, "L3_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = L3_MEM_SIZE;
    #ifdef CPU_mcu2_0
    heap_prm->flags = 0; /* when CPU is mcu2-0 use it as normal heap */
    #else
    heap_prm->flags = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE;
    #endif
    #endif
    #ifdef DDR_SCRATCH_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_DDR_SCRATCH];
    heap_prm->base = g_ddr_scratch_mem;
    strncpy(heap_prm->name, "DDR_SCRATCH_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = DDR_SCRATCH_SIZE;
    heap_prm->flags = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE;
    #endif
    #ifdef DDR_HEAP_NON_CACHE_MEM_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_DDR_NON_CACHE];
    heap_prm->base = g_ddr_non_cache_mem;
    strncpy(heap_prm->name, "DDR_NON_CACHE_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = DDR_HEAP_NON_CACHE_MEM_SIZE;
    heap_prm->flags = 0;
    #endif
    #ifdef DDR_VISS_HEAP_MEM_SIZE
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_DDR_WT_CACHE];
    heap_prm->base = g_ddr_cache_wt_mem;
    strncpy(heap_prm->name, "DDR_CACHE_WT_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = DDR_VISS_HEAP_MEM_SIZE;
    heap_prm->flags = 0;
    #endif
    #ifdef ENABLE_TIOVX
    #ifdef ENABLE_TIOVX_HOST
    heap_prm = &mem_init_prm.heap_info[APP_MEM_HEAP_DDR_SHARED];
    heap_prm->base = g_tiovx_host_shared_mem;
    strncpy(heap_prm->name, "DDR_SHARED_MEM", APP_MEM_HEAP_NAME_MAX -1U);
    heap_prm->name[APP_MEM_HEAP_NAME_MAX-1U] = '\0';
    heap_prm->size = TIOVX_HOST_DDR_SHARED_MEM_SIZE;
    heap_prm->flags = APP_MEM_HEAP_FLAGS_IS_SHARED;
    heap_prm->align = 4096; /* To accomodate MPU core access of buffers aligned to 4k bytes */
    #endif
    #endif

    #ifdef ENABLE_IPC
    /* appGetIpcResourceTable() returns NULL in RTOS only mode and returns a valid resource table
     * in Linux+RTOS mode
     */
    ipc_init_prm.ipc_resource_tbl = ipc_resource_table;
    if(host_os_type == APP_HOST_TYPE_LINUX)
    {
        ipc_init_prm.enable_tiovx_ipc_announce = 1;
    }
    else
    {
        ipc_init_prm.enable_tiovx_ipc_announce = 0;
    }
    ipc_init_prm.num_cpus = 0;
    #ifdef ENABLE_IPC_MPU1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MPU1_0;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MPU1_0] = 1;
    #endif
    #ifdef ENABLE_IPC_MCU1_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU1_0;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MCU1_0] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_MCU1_0] = 1;
    #endif
    #ifdef ENABLE_IPC_MCU2_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU2_0;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MCU2_0] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_MCU2_0] = 1;
    #endif
    #ifdef ENABLE_IPC_MCU2_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU2_1;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MCU2_1] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_MCU2_1] = 1;
    #endif
    #ifdef ENABLE_IPC_MCU3_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU3_0;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MCU3_0] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_MCU3_0] = 1;
    #endif
    #ifdef ENABLE_IPC_MCU3_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU3_1;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_MCU3_1] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_MCU3_1] = 1;
    #endif
    #ifdef ENABLE_IPC_C6x_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C6x_1;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_C6x_1] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_C6x_1] = 1;
    #endif
    #ifdef ENABLE_IPC_C6x_2
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C6x_2;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_C6x_2] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_C6x_2] = 1;
    #endif
    #ifdef ENABLE_IPC_C7x_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C7x_1;
    ipc_init_prm.num_cpus++;
    log_init_prm.log_rd_cpu_enable[APP_IPC_CPU_C7x_1] = 1;
    fileio_init_prm.fileio_rd_cpu_enable[APP_IPC_CPU_C7x_1] = 1;
    #endif
    ipc_init_prm.tiovx_obj_desc_mem = (void*)g_tiovx_obj_desc_mem;
    ipc_init_prm.tiovx_obj_desc_mem_size = TIOVX_OBJ_DESC_MEM_SIZE;
    ipc_init_prm.tiovx_log_rt_mem   = (void*)TIOVX_LOG_RT_MEM_ADDR;
    ipc_init_prm.tiovx_log_rt_mem_size   = TIOVX_LOG_RT_MEM_SIZE;
    ipc_init_prm.ipc_vring_mem = g_ipc_vring_mem;
    ipc_init_prm.ipc_vring_mem_size = IPC_VRING_MEM_SIZE;
    #ifdef CPU_mpu1
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MPU1_0;
    #endif
    #ifdef CPU_mcu1_0
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MCU1_0;
    #endif
    #ifdef CPU_mcu2_0
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MCU2_0;
    #endif
    #ifdef CPU_mcu2_1
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MCU2_1;
    #endif
    #ifdef CPU_mcu3_0
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MCU3_0;
    #endif
    #ifdef CPU_mcu3_1
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_MCU3_1;
    #endif
    #ifdef CPU_c6x_1
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_C6x_1;
    #endif
    #ifdef CPU_c6x_2
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_C6x_2;
    #endif
    #ifdef CPU_c7x_1
    ipc_init_prm.self_cpu_id = APP_IPC_CPU_C7x_1;
    #endif
    #endif

    log_init_prm.shared_mem = &g_app_log_shared_mem;
    log_init_prm.self_cpu_index = ipc_init_prm.self_cpu_id;
    #ifdef CPU_mpu1
    strncpy(log_init_prm.self_cpu_name, "MPU1" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_mcu1_0
    strncpy(log_init_prm.self_cpu_name, "MCU1_0" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_mcu2_0
    strncpy(log_init_prm.self_cpu_name, "MCU2_0" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_mcu2_1
    strncpy(log_init_prm.self_cpu_name, "MCU2_1" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_mcu3_0
    strncpy(log_init_prm.self_cpu_name, "MCU3_0" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_mcu3_1
    strncpy(log_init_prm.self_cpu_name, "MCU3_1" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_c6x_1
    strncpy(log_init_prm.self_cpu_name, "C6x_1" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_c6x_2
    strncpy(log_init_prm.self_cpu_name, "C6x_2" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef CPU_c7x_1
    strncpy(log_init_prm.self_cpu_name, "C7x_1" , APP_LOG_MAX_CPU_NAME);
    #endif
    #ifdef ENABLE_UART
    log_init_prm.log_rd_max_cpus = APP_IPC_CPU_MAX;
    log_init_prm.device_write = appLogDeviceWrite;
    #endif

    fileio_init_prm.shared_mem = &g_app_fileio_shared_mem;
    fileio_init_prm.self_cpu_index = ipc_init_prm.self_cpu_id;
    strncpy(fileio_init_prm.self_cpu_name, log_init_prm.self_cpu_name, APP_LOG_MAX_CPU_NAME);

    appPerfStatsInitRegister(perf_fxns_list);

    #ifdef ENABLE_BOARD
    {
        app_pinmux_cfg_t pinmux_cfg;

        appPinMuxCfgSetDefault(&pinmux_cfg);

        #if defined(ENABLE_DSS_SINGLE)
            pinmux_cfg.enable_i2c = UTRUE; /* i2c is needed for on board HDMI mux config, eDP to HDMI adapter config */
            #ifdef ENABLE_DSS_HDMI
                pinmux_cfg.enable_hdmi = UTRUE;
            #endif
        #endif
        #if defined(ENABLE_DSS_DUAL)
            pinmux_cfg.enable_hdmi = UTRUE; /* enable HDMI unconditionally for dual display */
            pinmux_cfg.enable_i2c = UTRUE; /* i2c is needed for on board HDMI mux config, eDP to HDMI adapter config */
        #endif

        appSetPinmux(&pinmux_cfg);
    }
    #endif

    #ifdef ENABLE_UART
    status = appLogRdInit(&log_init_prm);
    APP_ASSERT_SUCCESS(status);
    #endif

    status = appLogWrInit(&log_init_prm);
    APP_ASSERT_SUCCESS(status);

    status = appFileIOWrInit(&fileio_init_prm);
    APP_ASSERT_SUCCESS(status);

    #ifdef ENABLE_PRINTF_REDIRECT
    status = appLogCioInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    appUtilsPrintCpuHz();

    #if defined(FREERTOS)
    appLogPrintf("CPU is running FreeRTOS\n");
    #elif defined(SAFERTOS)
    appLogPrintf("CPU is running SafeRTOS\n");
    #endif

    appLogPrintf("APP: Init ... !!!\n");

    #if defined(R5F)
    /* NOTE: This does not initialize or set anything, it is just error checking address alignments in the MPU table and forcing
             a fatal error and error log so that the developer is forced to fix rather than a bug going unnoticed
             This can be removed in production code once the memory map is locked
    */
    status = appVerifyMpuConfigAlignment();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_UART
    {
        app_cli_init_prm_t cli_init_prm;

        appCliInitPrmSetDefault(&cli_init_prm);
        status = appCliInit(&cli_init_prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif

    #ifdef ENABLE_SCICLIENT
    status = appSciclientInit(ipc_init_prm.self_cpu_id);
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_UDMA
    {
        app_udma_init_prms_t udma_init_prm;

        appUdmaInitPrmSetDefault(&udma_init_prm);

        #ifdef C71
        udma_init_prm.virtToPhyFxn = appUdmaVirtToPhyAddrConversion;
        #endif

        status = appUdmaInit(&udma_init_prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif

    status = appMemInit(&mem_init_prm);
    APP_ASSERT_SUCCESS(status);
    
    #ifdef ENABLE_IPC
    status = appIpcInit(&ipc_init_prm);
    APP_ASSERT_SUCCESS(status);
    {
        uint32_t sync_cpu_id_list[APP_LOG_MAX_CPUS];
        uint32_t i, self_cpu_id, master_cpu_id, num_sync_cpus;

        if((host_os_type == APP_HOST_TYPE_LINUX) || (host_os_type == APP_HOST_TYPE_QNX))
        {
            master_cpu_id = APP_IPC_CPU_MCU2_0;
        }
        else
        {
            master_cpu_id = APP_IPC_CPU_MPU1_0;
        }
        self_cpu_id = ipc_init_prm.self_cpu_id;
        num_sync_cpus = 0;
        for(i=0; i<ipc_init_prm.num_cpus; i++)
        {
            if(i<APP_LOG_MAX_CPUS)
            {
                if((host_os_type == APP_HOST_TYPE_LINUX) || (host_os_type == APP_HOST_TYPE_QNX))
                {
                    /* dont sync with MPU1 running linux/qnx since that is taken care by the kernel */
                    if(ipc_init_prm.enabled_cpu_id_list[i]!=APP_IPC_CPU_MPU1_0)
                    {
                        sync_cpu_id_list[num_sync_cpus] = ipc_init_prm.enabled_cpu_id_list[i];
                        num_sync_cpus++;
                    }
                }
                else
                {
                    sync_cpu_id_list[num_sync_cpus] = ipc_init_prm.enabled_cpu_id_list[i];
                    num_sync_cpus++;
                }
            }
        }
        appLogPrintf("APP: Syncing with %d CPUs ... !!!\n", num_sync_cpus);
        appLogCpuSyncInit(master_cpu_id, self_cpu_id, sync_cpu_id_list, num_sync_cpus);
        appLogPrintf("APP: Syncing with %d CPUs ... Done !!!\n", num_sync_cpus);
    }
    {
        app_remote_service_init_prms_t init_prms;

        appRemoteServiceInitSetDefault(&init_prms);
        appRemoteServiceInit(&init_prms);
        appRtosTestRegister();
        appPerfStatsRemoteServiceInit();
    }
    #endif

    #ifdef ENABLE_FVID2
    status = appFvid2Init();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_I2C
    appI2cInit();
    #endif

    #ifdef ENABLE_DSS_SINGLE
    {
        app_dss_default_prm_t prm;

        appDssDefaultSetDefaultPrm(&prm);

        #ifdef ENABLE_DSS_HDMI
        prm.display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI;
        #endif
        #ifdef ENABLE_DSS_EDP
        prm.display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_EDP;
        #endif

        prm.enableM2m            = true;
        /* Do not rely on "init". Always provide known good tmings */
        prm.timings.width        = 1920U;
        prm.timings.height       = 1080U;
        prm.timings.hFrontPorch  = 88U;
        prm.timings.hBackPorch   = 148U;
        prm.timings.hSyncLen     = 44U;
        prm.timings.vFrontPorch  = 4U;
        prm.timings.vBackPorch   = 36U;
        prm.timings.vSyncLen     = 5U;
        prm.timings.pixelClock   = 148500000ULL;

        #ifdef ENABLE_DSS_DSI
            prm.display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_DSI;

            prm.timings.width        = 1280U;
            prm.timings.height       = 800U;
            prm.timings.hFrontPorch  = 110U;
            prm.timings.hBackPorch   = 220U;
            prm.timings.hSyncLen     = 40U;
            prm.timings.vFrontPorch  = 5U;
            prm.timings.vBackPorch   = 20U;
            prm.timings.vSyncLen     = 5U;
            prm.timings.pixelClock   = 74250000ULL;
        #endif
        status = appDssDefaultInit(&prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif

    #ifdef ENABLE_DSS_DUAL
    {
        app_dss_dual_display_default_prm_t prm;
        uint32_t i;

        /* default parameters are enough to enable both EDP and HDMI */
        appDssDualDisplayDefaultSetDefaultPrm(&prm);

        prm.enableM2m                           = true;
        /* Do not rely on "init". Always provide known good tmings */
        for(i=0; i<2; i++)
        {
            prm.display[i].timings.width        = 1920U;
            prm.display[i].timings.height       = 1080U;
            prm.display[i].timings.hFrontPorch  = 88U;
            prm.display[i].timings.hBackPorch   = 148U;
            prm.display[i].timings.hSyncLen     = 44U;
            prm.display[i].timings.vFrontPorch  = 4U;
            prm.display[i].timings.vBackPorch   = 36U;
            prm.display[i].timings.vSyncLen     = 5U;
            prm.display[i].timings.pixelClock   = 148500000ULL;
        }

        status = appDssDualDisplayDefaultInit(&prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif

    #ifdef ENABLE_VHWA_VPAC
    status = appVhwaVpacInit(0u);
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_VHWA_DMPAC
    status = appVhwaDmpacInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_TIOVX
    tivxInit();
    #ifdef ENABLE_TIOVX_HOST
    tivxHostInit();
    #endif
    #endif
    appRegisterOpenVXTargetKernels();

    #ifdef ENABLE_CSI2RX
    status = appCsi2RxInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_CSI2TX
    status = appCsi2TxInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
    status = appIssInit();
    APP_ASSERT_SUCCESS(status);

    status = appRemoteServiceSensorInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #if defined(ENABLE_VHWA_VPAC)
    status = appVissRemoteServiceInit();
    APP_ASSERT_SUCCESS(status);

    /* Register remote service for VPAC firewall control (LDC, MSC, NF, VISS) */
    status = appVhwaVpacRemoteServiceInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_UDMA_COPY
    status = appUdmaCopyInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    /* Register remote service for SL2 reallocation
     * Can add more conditions if needed.
     */
    #ifdef ENABLE_VHWA_DMPAC
    status = appVhwaRemoteServiceInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_ETHFW
    status = appEthFwInit();
    APP_ASSERT_SUCCESS(status);

    status = appEthFwRemoteServerInit();
    APP_ASSERT_SUCCESS(status);

    appLogWaitMsecs(50); /* Temporary workaround for ETHFW-1629 */
    #endif

    appLogPrintf("APP: Init ... Done !!!\n");

    return status;
}

void appDeInit()
{
    appLogPrintf("APP: Deinit ... !!!\n");

    appUnRegisterOpenVXTargetKernels();
    #ifdef ENABLE_UDMA_COPY
    appUdmaCopyDeinit();
    #endif
    #ifdef ENABLE_TIOVX
    #ifdef ENABLE_TIOVX_HOST
    tivxHostDeInit();
    #endif
    tivxDeInit();
    #endif
    #ifdef ENABLE_VHWA_VPAC
    appVhwaVpacDeInit();
    #endif
    #ifdef ENABLE_VHWA_DMPAC
    appVhwaDmpacDeInit();
    #endif
    #ifdef ENABLE_DSS_SINGLE
    appDssDefaultDeInit();
    #endif
    #ifdef ENABLE_DSS_DUAL
    appDssDualDisplayDefaultDeInit();
    #endif
    #ifdef ENABLE_CSI2RX
    appCsi2RxDeInit();
    #endif
    #ifdef ENABLE_CSI2TX
    appCsi2TxDeInit();
    #endif
    #ifdef ENABLE_FVID2
    appFvid2DeInit();
    #endif
    #ifdef ENABLE_ETHFW
    appEthFwDeInit();
    #endif
    #ifdef ENABLE_IPC
    appPerfStatsDeInit();
    appRtosTestUnRegister();
    appRemoteServiceDeInit();
    appIpcDeInit();
    #endif
    appMemDeInit();
    #ifdef ENABLE_PRINTF_REDIRECT
    appLogCioDeInit();
    #endif
    appLogWrDeInit();
    appFileIOWrDeInit();
    #ifdef ENABLE_UART
    appLogRdDeInit();
    appCliDeInit();
    #endif
    #ifdef ENABLE_UDMA
    appUdmaDeInit();
    #endif
    #ifdef ENABLE_SCICLIENT
    appSciclientDeInit();
    #endif

    #ifdef ENABLE_I2C
    appI2cDeInit();
    #endif

    #if defined(ENABLE_VHWA_VPAC)
    appVhwaVpacRemoteServiceDeInit();
    appVissRemoteServiceDeInit();
    #endif

    #if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
    appIssDeInit();
    appRemoteServiceSensorDeInit();
    #endif

    /* De-init GTC timer */
    appLogGlobalTimeDeInit();

    /* Unregister remote service for SL2 reallocation.
     * Can add more conditions if needed.
     */
    #ifdef ENABLE_VHWA_DMPAC
    appVhwaRemoteServiceDeInit();
    #endif

    appLogPrintf("APP: Deinit ... Done !!!\n");
}

static void appRegisterOpenVXTargetKernels()
{
    #ifdef ENABLE_TIOVX
    appLogPrintf("APP: OpenVX Target kernel init ... !!!\n");
        #ifdef ENABLE_VHWA_VPAC
        tivxRegisterHwaTargetVpacMscKernels();
        tivxRegisterHwaTargetVpacLdcKernels();
        tivxRegisterHwaTargetVpacVissKernels();
        tivxRegisterHwaTargetVpacNfKernels();
        #ifndef x86_64
        tivxRegisterHwaTargetVpacFcKernels();
        #endif
        #endif
        #ifdef ENABLE_VHWA_DMPAC
        tivxRegisterHwaTargetDmpacSdeKernels();
        tivxRegisterHwaTargetDmpacDofKernels();
        tivxRegisterHwaTargetArmKernels();
        #endif
        #ifdef ENABLE_CSI2RX
        tivxRegisterVideoIOTargetCaptureKernels();
        #endif
        #ifdef ENABLE_CSI2TX
        tivxRegisterVideoIOTargetCsitxKernels();
        #endif
        #if defined(ENABLE_DSS_SINGLE) || defined(ENABLE_DSS_DUAL)
        tivxRegisterVideoIOTargetDisplayKernels();
        tivxRegisterVideoIOTargetDisplayM2MKernels();
        #endif
        #ifdef C71
        {
            void app_c7x_target_kernel_img_add_register(void);

            app_c7x_target_kernel_img_add_register();
        }
        tivxRegisterTIDLTargetKernels();
        tivxRegisterTVMTargetKernels();
        tivxRegisterStereoTargetKernels();
        tivxRegisterImgProcTargetC71Kernels();
        #endif
        #ifdef C66
        tivxRegisterStereoTargetKernels();
        tivxRegisterSrvTargetC66Kernels();
        tivxRegisterHwaTargetArmKernels();
        tivxRegisterImgProcTargetC66Kernels();
        #endif
        #ifdef ENABLE_VHWA_VPAC
        tivxRegisterImgProcTargetR5FKernels();
        tivxRegisterImagingTargetAewbKernels();
        #endif
    appLogPrintf("APP: OpenVX Target kernel init ... Done !!!\n");
    #endif
}

static void appUnRegisterOpenVXTargetKernels()
{
    #ifdef ENABLE_TIOVX
    appLogPrintf("APP: OpenVX Target kernel deinit ... !!!\n");
        #ifdef ENABLE_VHWA_VPAC
        tivxUnRegisterHwaTargetVpacMscKernels();
        tivxUnRegisterHwaTargetVpacLdcKernels();
        tivxUnRegisterHwaTargetVpacNfKernels();
        tivxUnRegisterHwaTargetVpacVissKernels();
        #ifndef x86_64
        tivxUnRegisterHwaTargetVpacFcKernels();
        #endif
        #endif
        #ifdef ENABLE_VHWA_DMPAC
        tivxUnRegisterHwaTargetDmpacSdeKernels();
        tivxUnRegisterHwaTargetDmpacDofKernels();
        tivxUnRegisterHwaTargetArmKernels();
        #endif
        #if defined(ENABLE_DSS_SINGLE) || defined(ENABLE_DSS_DUAL)
        tivxUnRegisterVideoIOTargetDisplayKernels();
        tivxUnRegisterVideoIOTargetDisplayM2MKernels();
        #endif
        #ifdef ENABLE_CSI2RX
        tivxUnRegisterVideoIOTargetCaptureKernels();
        #endif
        #ifdef ENABLE_CSI2TX
        tivxUnRegisterVideoIOTargetCsitxKernels();
        #endif
        #ifdef C71
        {
            void app_c7x_target_kernel_img_add_unregister(void);

            app_c7x_target_kernel_img_add_unregister();
        }
        tivxUnRegisterTIDLTargetKernels();
        tivxUnRegisterTVMTargetKernels();
        tivxUnRegisterStereoTargetKernels();
        tivxUnRegisterImgProcTargetC71Kernels();
        #endif
        #ifdef C66
        tivxUnRegisterStereoTargetKernels();
        tivxUnRegisterSrvTargetC66Kernels();
        tivxUnRegisterHwaTargetArmKernels();
        tivxUnRegisterImgProcTargetC66Kernels();
        #endif
        #ifdef ENABLE_VHWA_VPAC
        tivxUnRegisterImgProcTargetR5FKernels();
        tivxUnRegisterImagingTargetAewbKernels();
        #endif
    appLogPrintf("APP: OpenVX Target kernel deinit ... Done !!!\n");
    #endif
}

void appIdleLoop(void)
{
    #if defined(__C7100__) || defined(C66)
   __asm(" IDLE");
   #endif
}
