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

#include <platform.h>
#include <app_init.h>
#include CORE_CFG_FILE

/* app utils header files */
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
#include <ti/drv/fvid2/fvid2.h>
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_DMPAC)
#include <utils/hwa/include/app_hwa.h>
#endif

#if defined(ENABLE_CSI2RX) || defined(ENABLE_CSI2TX)
#include <utils/csi/include/app_csi.h>
#endif

#if (defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)) || defined(ENABLE_AEWB_KERNELS)
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
#if defined(C7120)
#include <TI/tivx_srv.h>
#include <TI/tivx_stereo.h>
#endif

/* Imaging header files */
#if defined(ENABLE_TIOVX)

#if defined(ENABLE_AEWB_KERNELS)
#include <TI/j7_imaging_aewb.h>
#endif
#if (defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1))
#include <TI/hwa_vpac_viss.h>
#include <TI/hwa_vpac_ldc.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_vpac_nf.h>
#include <TI/hwa_vpac_fc.h>
#endif

#if defined(ENABLE_VHWA_DMPAC)
#include <TI/hwa_dmpac_sde.h>
#endif

#if defined(ENABLE_VHWA_DMPAC) || defined(C7120)
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

#ifdef C7120
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

extern app_perf_registration_t * perf_fxns_list;

extern core_config_t core_config[];
extern uint8_t num_cores;

extern soc_mem_config_t mem_regions[];
extern uint8_t num_mem_regions;

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

#if defined(ENABLE_FVID2)
static int32_t fvid2_init()
{
    int32_t retVal = FVID2_SOK;
    Fvid2_InitPrms initPrmsFvid2;

    appLogPrintf("FVID2: Init ... !!!\n");

    Fvid2InitPrms_init(&initPrmsFvid2);
    initPrmsFvid2.printFxn = appLogPrintf;
    retVal = Fvid2_init(&initPrmsFvid2);

    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("FVID2: ERROR: Fvid2_init failed !!!\n");
    }

    appLogPrintf("FVID2: Init ... Done !!!\n");

    return (retVal);
}

static int32_t fvid2_deinit()
{
    int32_t retVal = FVID2_SOK;

    retVal = Fvid2_deInit(NULL);

    if(retVal!=FVID2_SOK)
    {
        appLogPrintf("FVID2: ERROR: Fvid2_deInit failed !!!\n");
    }

    return (retVal);
}
#endif

static int memInit()
{
    int32_t status = 0;
    uint8_t i = 0;

    app_mem_init_prm_t mem_init_prm;
    app_mem_heap_prm_t *heap_prm;

    appMemInitPrmSetDefault(&mem_init_prm);

    mem_init_prm.virtToPhyFxn     = appUdmaVirtToPhyAddrConversion;
    mem_init_prm.shared2TargetFxn = appShared2TargetConversion;
    mem_init_prm.target2SharedFxn = appTarget2SharedConversion;

    for (i = 0; i < num_mem_regions; i++)
    {
        heap_prm = &mem_init_prm.heap_info[mem_regions[i].heap_id];
        heap_prm->base = mem_regions[i].base;
        strncpy(heap_prm->name, mem_regions[i].name, APP_MEM_HEAP_NAME_MAX);
        heap_prm->size = mem_regions[i].size;
        heap_prm->flags = mem_regions[i].flag;
    }

    status = appMemInit(&mem_init_prm);

    #if defined(R5F)
    #if defined(CPU_mcu1_0)
    status = appMemSetRatRegs((CSL_ratRegs *)(CSL_MCU_R5FSS0_RAT_CFG_BASE));
    #else
    status = appMemSetRatRegs((CSL_ratRegs *)(CSL_R5FSS0_RAT_CFG_BASE));
    #endif
    APP_ASSERT_SUCCESS(status);
    #endif

    #if defined(CPU_mcu2_0) || defined(CPU_mcu2_1) || defined(CPU_mcu4_0)
    app_mem_rat_prm_t l3_mem_rat_prm;
    #ifdef L3_MEM_SIZE

    l3_mem_rat_prm.size        = L3_MEM_SIZE;

    #if defined(CPU_mcu2_0)
    l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU2_0_ADDR;
    l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU2_0_PHYS_ADDR;
    #elif defined(CPU_mcu2_1)
    l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU2_1_ADDR;
    l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU2_1_PHYS_ADDR;
    #elif defined(CPU_mcu4_0)
    l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU4_0_ADDR;
    l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU4_0_PHYS_ADDR;
    #endif

    status = appMemAddrTranslate(&l3_mem_rat_prm);
    APP_ASSERT_SUCCESS(status);
    #endif
    #endif

    #if defined(R5F)
    app_mem_rat_prm_t ddr_mem_rat_prm;
    /* Only programming RAT if the physical address is in high mem */
    if (DDR_SHARED_MEM_PHYS_ADDR > 0xFFFFFFFF)
    {
        /* Making the DDR_SHARED_MEM size aligned to 1GB by adding the UBOOT_RELOC_MEM_SIZE.                        */
        /* The UBOOT_RELOC_MEM_SIZE was subtracted from DDR_SHARED_MEM size while creating the memory map for J784S4*/
        /* This was done because the 1GB DDR_SHARED_MEM size was overlapping the UBOOT_RELOC_MEM_ADDR at the end of */
        /* low 2GB memory. For more details on this, please refer to 3.1.1.1.6. Available RAM for image download    */
        /* section in processor-sdk-linux documentation                                                             */
        ddr_mem_rat_prm.size              = DDR_SHARED_MEM_SIZE + UBOOT_RELOC_MEM_SIZE;
        ddr_mem_rat_prm.baseAddress       = DDR_SHARED_MEM_ADDR;
        ddr_mem_rat_prm.translatedAddress = DDR_SHARED_MEM_PHYS_ADDR;

        status = appMemAddrTranslate(&ddr_mem_rat_prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif

    #if defined(R5F)
    /* NOTE: This does not initialize or set anything, it is just error checking address alignments in the MPU table and forcing
             a fatal error and error log so that the developer is forced to fix rather than a bug going unnoticed
             This can be removed in production code once the memory map is locked
    */
    status = appVerifyMpuConfigAlignment();
    APP_ASSERT_SUCCESS(status);
    #endif

    return status;
}

static int ipcInit()
{
    int32_t status = 0;
    uint8_t i = 0;

    app_ipc_init_prm_t ipc_init_prm;

    uint32_t host_os_type;
    void *ipc_resource_table = NULL;

    host_os_type = appGetHostOSType();
    if (host_os_type == APP_HOST_TYPE_LINUX)
    {
      ipc_resource_table = appGetIpcResourceTable();
    }

    appIpcInitPrmSetDefault(&ipc_init_prm);

    ipc_init_prm.ipc_resource_tbl = ipc_resource_table;
    if (host_os_type == APP_HOST_TYPE_LINUX)
    {
        ipc_init_prm.enable_tiovx_ipc_announce = 1;
    }
    else
    {
        ipc_init_prm.enable_tiovx_ipc_announce = 0;
    }

    for (i = 0; i < num_cores; i++)
    {
        ipc_init_prm.enabled_cpu_id_list[i] = core_config[i].cpu;
    }
    ipc_init_prm.num_cpus = num_cores;

    ipc_init_prm.self_cpu_id = CPU_ID;
    ipc_init_prm.tiovx_obj_desc_mem = (void*)g_tiovx_obj_desc_mem;
    ipc_init_prm.tiovx_obj_desc_mem_size = TIOVX_OBJ_DESC_MEM_SIZE;
    ipc_init_prm.tiovx_log_rt_mem      = (void*)TIOVX_LOG_RT_MEM_ADDR;
    ipc_init_prm.tiovx_log_rt_mem_size = TIOVX_LOG_RT_MEM_SIZE;
    ipc_init_prm.ipc_vring_mem = g_ipc_vring_mem;
    ipc_init_prm.ipc_vring_mem_size = IPC_VRING_MEM_SIZE;

    status = appIpcInit(&ipc_init_prm);

    uint32_t sync_cpu_id_list[APP_LOG_MAX_CPUS];
    uint32_t num_sync_cpus = 0;

    for (i = 0; i < ipc_init_prm.num_cpus; i++)
    {
        if (i < APP_LOG_MAX_CPUS)
        {
            if ((host_os_type == APP_HOST_TYPE_LINUX) || (host_os_type == APP_HOST_TYPE_QNX))
            {
                if (ipc_init_prm.enabled_cpu_id_list[i] != APP_IPC_CPU_MPU1_0)
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
    appLogCpuSyncInit(MASTER_CPU_ID, CPU_ID, sync_cpu_id_list, num_sync_cpus);
    appLogPrintf("APP: Syncing with %d CPUs ... Done !!!\n", num_sync_cpus);

    return status;
}

static int logInit()
{
    int32_t status = 0;
    uint8_t i = 0;

    app_log_init_prm_t log_init_prm;
    appLogInitPrmSetDefault(&log_init_prm);

    for (i = 0; i < num_cores; i++)
    {
        log_init_prm.log_rd_cpu_enable[i] = core_config[i].enable_log_rd_cpu;
        strncpy(log_init_prm.self_cpu_name, CPU_NAME, APP_LOG_MAX_CPU_NAME);
    }
    log_init_prm.shared_mem = &g_app_log_shared_mem;
    log_init_prm.self_cpu_index = CPU_ID;

#ifdef ENABLE_UART
    {
        log_init_prm.log_rd_max_cpus = APP_IPC_CPU_MAX;
        log_init_prm.device_write = appLogDeviceWrite;
        status = appLogRdInit(&log_init_prm);
        APP_ASSERT_SUCCESS(status);

        app_cli_init_prm_t cli_init_prm;
        appCliInitPrmSetDefault(&cli_init_prm);
        status = appCliInit(&cli_init_prm);
        APP_ASSERT_SUCCESS(status);
    }
#endif

    status = appLogWrInit(&log_init_prm);

    return status;
}

static int fileIOInit()
{
    int32_t status = 0;
    uint8_t i = 0;

    app_fileio_init_prm_t fileio_init_prm;
    appFileIOInitPrmSetDefault(&fileio_init_prm);

    for (i = 0; i < num_cores; i++)
    {
        fileio_init_prm.fileio_rd_cpu_enable[i] = core_config[i].enable_fileio;
        strncpy(fileio_init_prm.self_cpu_name, CPU_NAME, APP_LOG_MAX_CPU_NAME);
    }
    fileio_init_prm.shared_mem = &g_app_fileio_shared_mem;
    fileio_init_prm.self_cpu_index = CPU_ID;

    status = appFileIOWrInit(&fileio_init_prm);

    return status;
}

static void logDeInit()
{
    appLogWrDeInit();

#ifdef ENABLE_UART
    {
        appLogRdDeInit();
        appCliDeInit();
    }
#endif
}

static void fileIODeInit()
{
    uint8_t i = 0;

    for (i = 0; i < num_cores; i++)
    {
        if (core_config[i].cpu == CPU_ID)
        {
            if (core_config[i].enable_fileio)
            {
                appFileIOWrDeInit();
            }
        }
    }
}

#ifdef ENABLE_BOARD
static void pinmuxInit()
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

static int32_t dssInit()
{
    int32_t status = 0;
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
            prm.timings.width        = 1920U;
            prm.timings.height       = 1080U;
            prm.timings.hFrontPorch  = 60U;
            prm.timings.hBackPorch   = 70U;
            prm.timings.hSyncLen     = 62U;
            prm.timings.vFrontPorch  = 55U;
            prm.timings.vBackPorch   = 60U;
            prm.timings.vSyncLen     = 55U;
            prm.timings.pixelClock   = 158400000ULL;
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
            if(i==0)
            {
                prm.display[i].display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_EDP;
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
            else if(i==1)

            {
                prm.display[i].display_type = APP_DSS_DEFAULT_DISPLAY_TYPE_DSI;
                prm.display[i].timings.width        = 1920U;
                prm.display[i].timings.height       = 1080U;
                prm.display[i].timings.hFrontPorch  = 60U;
                prm.display[i].timings.hBackPorch   = 70U;
                prm.display[i].timings.hSyncLen     = 62U;
                prm.display[i].timings.vFrontPorch  = 55U;
                prm.display[i].timings.vBackPorch   = 60U;
                prm.display[i].timings.vSyncLen     = 55U;
                prm.display[i].timings.pixelClock   = 158400000ULL;
            }
        }

        status = appDssDualDisplayDefaultInit(&prm);
        APP_ASSERT_SUCCESS(status);
    }
    #endif
    return status;
}

int32_t appInit()
{
    int32_t status = 0;

    /* Init and start GTC timer */
    status = appLogGlobalTimeInit();
    APP_ASSERT_SUCCESS(status);

    appPerfStatsInitRegister(perf_fxns_list);

    #ifdef ENABLE_BOARD
    pinmuxInit();
    #endif

    status = logInit();
    APP_ASSERT_SUCCESS(status);

    status = fileIOInit();
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

    #ifdef ENABLE_SCICLIENT
    status = appSciclientInit(CPU_ID);
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_UDMA
    app_udma_init_prms_t udma_init_prm;
    appUdmaInitPrmSetDefault(&udma_init_prm);

    #ifdef C7120
    udma_init_prm.virtToPhyFxn = appUdmaVirtToPhyAddrConversion;
    #endif

    status = appUdmaInit(&udma_init_prm);
    APP_ASSERT_SUCCESS(status);

    #ifdef CPU_mcu2_0
    status = appUdmaCsirxCsitxInit();
    APP_ASSERT_SUCCESS(status);
    #endif
    #endif

    status = memInit();
    APP_ASSERT_SUCCESS(status);

    #ifdef ENABLE_IPC
    status = ipcInit();
    APP_ASSERT_SUCCESS(status);

    app_remote_service_init_prms_t init_prms;
    appRemoteServiceInitSetDefault(&init_prms);
    appRemoteServiceInit(&init_prms);
    appRtosTestRegister();
    appPerfStatsRemoteServiceInit();
    #endif

    #ifdef ENABLE_FVID2
    status = fvid2_init();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_I2C
    appI2cInit();
    #endif

    status = dssInit();
    APP_ASSERT_SUCCESS(status);

    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
    {
        uint32_t vpacInst;

        #if defined(ENABLE_VHWA_VPAC0)
            vpacInst = 0u;
        #elif defined(ENABLE_VHWA_VPAC1)
            vpacInst = 1u;
        #endif

        status = appVhwaVpacInit(vpacInst);
        APP_ASSERT_SUCCESS(status);
    }
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

    appRegisterOpenVXTargetKernels();
    #endif

    #ifdef ENABLE_CSI2RX
    status = appCsi2RxInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #ifdef ENABLE_CSI2TX
    status = appCsi2TxInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX) || defined(ENABLE_AEWB_KERNELS)
    status = appIssInit();
    APP_ASSERT_SUCCESS(status);

    status = appRemoteServiceSensorInit();
    APP_ASSERT_SUCCESS(status);
    #endif

    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
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

    #ifdef ENABLE_UDMA_COPY
    appUdmaCopyDeinit();
    #endif

    #ifdef ENABLE_TIOVX
    #ifdef ENABLE_TIOVX_HOST
        tivxHostDeInit();
    #endif
    appUnRegisterOpenVXTargetKernels();
    tivxDeInit();
    #endif

    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
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
    fvid2_deinit();
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

    fileIODeInit();
    logDeInit();

    #ifdef ENABLE_UDMA
    appUdmaDeInit();
    #endif

    #ifdef ENABLE_SCICLIENT
    appSciclientDeInit();
    #endif

    #ifdef ENABLE_I2C
    appI2cDeInit();
    #endif

    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
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
        #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
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
        #ifdef C7120
        #if defined(CPU_c7x_1) || defined(CPU_c7x_2) || defined(CPU_c7x_3) || defined(CPU_c7x_4)
        {
          tivxRegisterImgProcTargetAddImageC7xKernels();
        }
        tivxRegisterTIDLTargetKernels();
        tivxRegisterTVMTargetKernels();
        #endif
        #ifdef CPU_c7x_1
        tivxRegisterImgProcTargetC7xKernels();
        tivxRegisterImgProcTargetC6xKernels();
        #endif
        #ifdef CPU_c7x_2
        tivxRegisterStereoTargetKernels();
        tivxRegisterSrvTargetC7xKernels();
        tivxRegisterHwaTargetArmKernels();
        tivxRegisterImgProcTargetC6xKernels();
        #endif
        #endif
        #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
        tivxRegisterImgProcTargetMcuKernels();
        #endif
        #ifdef ENABLE_AEWB_KERNELS
        tivxRegisterImagingTargetAewbKernels();
        #endif
    appLogPrintf("APP: OpenVX Target kernel init ... Done !!!\n");
    #endif
}

static void appUnRegisterOpenVXTargetKernels()
{
    #ifdef ENABLE_TIOVX
    appLogPrintf("APP: OpenVX Target kernel deinit ... !!!\n");
        #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
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
        #ifdef C7120
        #if defined(CPU_c7x_1) || defined(CPU_c7x_2) || defined(CPU_c7x_3) || defined(CPU_c7x_4)
        {
          tivxUnRegisterImgProcTargetAddImageC7xKernels();
        }
        tivxUnRegisterTIDLTargetKernels();
        tivxUnRegisterTVMTargetKernels();
        #endif
        #ifdef CPU_c7x_1
        tivxUnRegisterImgProcTargetC7xKernels();
        tivxUnRegisterImgProcTargetC6xKernels();
        #endif
        #ifdef CPU_c7x_2
        tivxUnRegisterStereoTargetKernels();
        tivxUnRegisterSrvTargetC7xKernels();
        tivxUnRegisterHwaTargetArmKernels();
        tivxUnRegisterImgProcTargetC6xKernels();
        #endif
        #endif
        #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1)
        tivxUnRegisterImgProcTargetMcuKernels();
        #endif
        #ifdef ENABLE_AEWB_KERNELS
        tivxUnRegisterImagingTargetAewbKernels();
        #endif
    appLogPrintf("APP: OpenVX Target kernel deinit ... Done !!!\n");
    #endif
}

void appIdleLoop(void)
{
    #if defined(__C7120__)
   __asm(" IDLE");
   #endif
}
