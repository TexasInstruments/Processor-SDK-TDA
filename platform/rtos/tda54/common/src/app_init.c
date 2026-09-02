/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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


/* Apps utils header files */
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/console_io/include/app_log.h>
#include <utils/file_io/include/app_fileio.h>
#include <utils/console_io/include/app_cli.h>
#include <utils/misc/include/app_misc.h>
// #include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/timer/include/app_timer.h>

#ifdef ENABLE_UDMA
#include <utils/udma/include/app_udma.h>
#endif

#if defined(ENABLE_FVID2)
#include <Fvid2/v0/include/fvid2.h>
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2) || defined(ENABLE_VHWA_DMPAC)
#include <utils/hwa/include/app_hwa.h>
#endif

#if defined(ENABLE_CSI2RX) || defined(ENABLE_CSI2TX)
#include <utils/csi/include/app_csi.h>
#endif

#if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
#include <utils/sensors/include/app_sensors.h>
#include <utils/iss/include/app_iss.h>
#endif

#if defined ENABLE_DSS
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
//#include <TI/tivx_img_proc.h>

#if defined(C7604)
//#include <TI/tivx_srv.h>
//#include <TI/tivx_stereo.h>
#endif

/* Imaging header files */
#if defined(ENABLE_TIOVX)

#if (defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2))
#include <TI/j7_imaging_aewb.h>
#include <TI/hwa_vpac_viss.h>
#include <TI/hwa_vpac_ldc.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_vpac_nf.h>
#endif

#if defined(ENABLE_VHWA_DMPAC)
#include <TI/hwa_dmpac_sde.h>
#endif

#if defined(ENABLE_VHWA_DMPAC) || defined(C7604)
#include <TI/hwa_dmpac_dof.h>
#endif

#if defined(ENABLE_CSI2RX)
#include <TI/video_io_capture.h>
#endif

#if defined ENABLE_DSS
#include <TI/video_io_display.h>
#include <TI/video_io_display_m2m.h>
#endif

#if defined(ENABLE_CSI2TX)
#include <TI/video_io_csitx.h>
#endif

#ifdef C7604
#include <TI/dl_kernels.h>
#endif

#endif /* #if defined(ENABLE_TIOVX) */

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

// extern app_perf_registration_t * perf_fxns_list;

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

static int memInit()
{
    int32_t status = 0;
    uint8_t i = 0;

    app_mem_init_prm_t mem_init_prm;
    app_mem_heap_prm_t *heap_prm;

    appMemInitPrmSetDefault(&mem_init_prm);

    // mem_init_prm.virtToPhyFxn     = appUdmaVirtToPhyAddrConversion;
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
    if(host_os_type == APP_HOST_TYPE_LINUX)
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
    ipc_init_prm.tiovx_log_rt_mem   = (void*)TIOVX_LOG_RT_MEM_ADDR;
    ipc_init_prm.tiovx_log_rt_mem_size   = TIOVX_LOG_RT_MEM_SIZE;
    ipc_init_prm.ipc_vring_mem = g_ipc_vring_mem;
    ipc_init_prm.ipc_vring_mem_size = IPC_VRING_MEM_SIZE;

    status = appIpcInit(&ipc_init_prm);

    uint32_t sync_cpu_id_list[APP_LOG_MAX_CPUS];
    uint32_t num_sync_cpus;

    num_sync_cpus = 0;
    for(i=0; i<ipc_init_prm.num_cpus; i++)
    {
        if(i<APP_LOG_MAX_CPUS)
        {
            if((host_os_type == APP_HOST_TYPE_LINUX) || (host_os_type == APP_HOST_TYPE_QNX))
            {
                /* dont sync with MPU1 running linux/qnx since that is taken care by the kernel */
                if(ipc_init_prm.enabled_cpu_id_list[i] != APP_IPC_CPU_MPU1_0)
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
    int8_t i = 0;

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
            if(core_config[i].enable_fileio)
            {
                appFileIOWrDeInit();
            }
        }
    }
}

int32_t appInit()
{
    int32_t status = 0;

    /* Init and start GTC timer */
    status = appLogGlobalTimeInit();
    APP_ASSERT_SUCCESS(status);

    status = logInit();
    APP_ASSERT_SUCCESS(status);

    status = fileIOInit();
    APP_ASSERT_SUCCESS(status);

//    appPerfStatsInitRegister(perf_fxns_list);

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

    /*
     *
     *
     *
     TODO: Configure RAT for accessing OCRAM for M55
     #if defined(CPU_mcu3) || defined(CPU_mcu4)
     app_mem_rat_prm_t l3_mem_rat_prm;
     #endif

     #if defined(M55)
     app_mem_rat_prm_t ddr_mem_rat_prm;
     #endif
     #if defined(M55)
     #if defined(CPU_dmcu0)
     status = appMemSetRatRegs((CSL_ratRegs *)(CSL_MCU_R5FSS0_RAT_CFG_BASE));
     #else
     status = appMemSetRatRegs((CSL_ratRegs *)(CSL_R5FSS0_RAT_CFG_BASE));
     #endif
     APP_ASSERT_SUCCESS(status);
     #endif

     #if defined(CPU_mcu3) || defined(CPU_mcu4)
     #ifdef L3_MEM_SIZE

     l3_mem_rat_prm.size        = L3_MEM_SIZE;

     #if defined(CPU_mcu2)
     l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU2_0_ADDR;
     l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU2_0_PHYS_ADDR;
     #elif defined(CPU_mcu3)
     l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU2_1_ADDR;
     l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU2_1_PHYS_ADDR;
     #elif defined(CPU_mcu4)
     l3_mem_rat_prm.baseAddress       = MAIN_OCRAM_MCU4_0_ADDR;
     l3_mem_rat_prm.translatedAddress = MAIN_OCRAM_MCU4_0_PHYS_ADDR;
     #endif

     status = appMemAddrTranslate(&l3_mem_rat_prm);
     APP_ASSERT_SUCCESS(status);
     #endif
     #endif
     *
     *
     */

    /*
     *
     *
     *
     * TODO: Making the DDR_SHARED_MEM size aligned to 1GB by adding the UBOOT_RELOC_MEM_SIZE.The UBOOT_RELOC_MEM_SIZE was subtracted from DDR_SHARED_MEM size while creating the memory map for J784S4. This was done because the 1GB DDR_SHARED_MEM size was overlapping the UBOOT_RELOC_MEM_ADDR at the end of low 2GB memory. For more details on this, please refer to 3.1.1.1.6. Available RAM for image download section in processor-sdk-linux documentation

     #if defined(M55)
     // Only programming RAT if the physical address is in high mem
     if (DDR_SHARED_MEM_PHYS_ADDR > 0xFFFFFFFF)
     {
         ddr_mem_rat_prm.size              = DDR_SHARED_MEM_SIZE + UBOOT_RELOC_MEM_SIZE;
         ddr_mem_rat_prm.baseAddress       = DDR_SHARED_MEM_ADDR;
         ddr_mem_rat_prm.translatedAddress = DDR_SHARED_MEM_PHYS_ADDR;

         status = appMemAddrTranslate(&ddr_mem_rat_prm);
         APP_ASSERT_SUCCESS(status);
     }
     #endif
     *
     *
     */

#ifdef ENABLE_SCICLIENT
    status = appSciclientInit(ipc_init_prm.self_cpu_id);
    APP_ASSERT_SUCCESS(status);
#endif

#ifdef ENABLE_UDMA
    {
        app_udma_init_prms_t udma_init_prm;

        appUdmaInitPrmSetDefault(&udma_init_prm);

        #ifdef C7604
            udma_init_prm.virtToPhyFxn = appUdmaVirtToPhyAddrConversion;
        #endif

        status = appUdmaInit(&udma_init_prm);
        APP_ASSERT_SUCCESS(status);
    }

    #ifdef CPU_mcu1
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
    // appPerfStatsRemoteServiceInit();
#endif

#ifdef ENABLE_FVID2
    status = fvid2_init();
    APP_ASSERT_SUCCESS(status);
#endif

#ifdef ENABLE_I2C
    appI2cInit();
#endif

#ifdef ENABLE_DSS
    {
        app_dss_init_params_t prm;

        status = appDssInit(&prm);

        APP_ASSERT_SUCCESS(status);
    }
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    uint32_t vpacInst;

    #if defined(ENABLE_VHWA_VPAC0)
        vpacInst = 0u;
    #endif
    #if defined(ENABLE_VHWA_VPAC1)
        vpacInst = 1u;
    #endif
    #if defined(ENABLE_VHWA_VPAC3)
        vpacInst = 2u;
    #endif

    status = appVhwaVpacInit(vpacInst);
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

#if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
    status = appIssInit();
    APP_ASSERT_SUCCESS(status);

    status = appRemoteServiceSensorInit();
    APP_ASSERT_SUCCESS(status);
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    status = appVissRemoteServiceInit();
    APP_ASSERT_SUCCESS(status);
#endif

#ifdef ENABLE_UDMA_COPY
    status = appUdmaCopyInit();
    APP_ASSERT_SUCCESS(status);
#endif

#ifdef ENABLE_VHWA_DMPAC

    /* Register remote service for SL2 reallocation
     * Can add more conditions if needed.
     */
    status = appVhwaRemoteServiceInit();
    APP_ASSERT_SUCCESS(status);
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
    appUnRegisterOpenVXTargetKernels();
    #ifdef ENABLE_TIOVX_HOST
        tivxHostDeInit();
    #endif
    tivxDeInit();
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    appVhwaVpacDeInit();
#endif

#ifdef ENABLE_VHWA_DMPAC
    appVhwaDmpacDeInit();
#endif

#ifdef ENABLE_DSS
    appDssDeInit();
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
    // appPerfStatsDeInit();
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
    #ifdef CPU_mcu1
        appUdmaCsirxCsitxDeInit();
    #endif
    appUdmaDeInit();
#endif

#ifdef ENABLE_SCICLIENT
    appSciclientDeInit();
#endif

#ifdef ENABLE_I2C
    appI2cDeInit();
#endif

#if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    appVissRemoteServiceDeInit();
#endif

#if defined(ENABLE_I2C) && defined(ENABLE_CSI2RX)
    appIssDeInit();
    appRemoteServiceSensorDeInit();
#endif

    /* De-init GTC timer */
    appLogGlobalTimeDeInit();

#ifdef ENABLE_VHWA_DMPAC
    /* Unregister remote service for SL2 reallocation.
     * Can add more conditions if needed.
     */
    appVhwaRemoteServiceDeInit();
#endif

    appLogPrintf("APP: Deinit ... Done !!!\n");
}

static void appRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel init ... !!!\n");
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxRegisterHwaTargetVpacMscKernels();
    tivxRegisterHwaTargetVpacLdcKernels();
    tivxRegisterHwaTargetVpacVissKernels();
    tivxRegisterHwaTargetVpacNfKernels();
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
    #ifdef ENABLE_DSS
    tivxRegisterVideoIOTargetDisplayKernels();
    /* TODO: DSS M2M to be enabled once M2M VID PIPE is available*/
    /* tivxRegisterVideoIOTargetDisplayM2MKernels(); */
    #endif
    #if 0
    #ifdef C7604
    #if defined(CPU_c7x_1) || defined(CPU_c7x_2) || defined(CPU_c7x_3) || defined(CPU_c7x_4)
    {
      tivxRegisterImgProcTargetAddImageC7xKernels
    }
    #endif
    #endif
    #if 0
    #ifdef CPU_c7x_1
    tivxRegisterTIDLTargetKernels();
    tivxRegisterTVMTargetKernels();
    tivxRegisterImgProcTargetC7xKernels();
    #endif
    #endif
    #if 0
    #ifdef CPU_c7x_2
    tivxRegisterStereoTargetKernels();
    tivxRegisterSrvTargetC7xKernels();
    tivxRegisterHwaTargetArmKernels();
    tivxRegisterImgProcTargetC7xKernels();
    #endif
    #endif
    #endif
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxRegisterImgProcTargetMcuKernels();
    #endif
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxRegisterImagingTargetAewbKernels();
    #endif
    appLogPrintf("APP: OpenVX Target kernel init ... Done !!!\n");
}

static void appUnRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel deinit ... !!!\n");
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxUnRegisterHwaTargetVpacMscKernels();
    tivxUnRegisterHwaTargetVpacLdcKernels();
    tivxUnRegisterHwaTargetVpacNfKernels();
    tivxUnRegisterHwaTargetVpacVissKernels();
    #endif
    #ifdef ENABLE_VHWA_DMPAC
    tivxUnRegisterHwaTargetDmpacSdeKernels();
    tivxUnRegisterHwaTargetDmpacDofKernels();
    tivxUnRegisterHwaTargetArmKernels();
    #endif
    #ifdef ENABLE_DSS
    tivxUnRegisterVideoIOTargetDisplayKernels();
    /* TODO: DSS M2M to be enabled once M2M VID PIPE is available*/
    /* tivxUnRegisterVideoIOTargetDisplayM2MKernels(); */
    #endif
    #ifdef ENABLE_CSI2RX
    tivxUnRegisterVideoIOTargetCaptureKernels();
    #endif
    #ifdef ENABLE_CSI2TX
    tivxUnRegisterVideoIOTargetCsitxKernels();
    #endif
    #ifdef C7604
    #if 0
    #if defined(CPU_c7x_1) || defined(CPU_c7x_2) || defined(CPU_c7x_3) || defined(CPU_c7x_4)
    {
      tivxUnRegisterImgProcTargetAddImageC7xKernels();
    }
    #endif
    #endif
    #if 0
    tivxUnRegisterTIDLTargetKernels();
    tivxUnRegisterImgProcTargetC7xKernels();
    tivxUnRegisterTVMTargetKernels();
    #endif
    #ifdef CPU_c7x_1
    #endif
    #if 0
    #ifdef CPU_c7x_2
    tivxUnRegisterStereoTargetKernels();
    tivxUnRegisterSrvTargetDspKernels();
    tivxUnRegisterHwaTargetC7xKernels();
    tivxUnRegisterImgProcTargetC7xKernels();
    #endif
    #endif
    #endif
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxUnRegisterImgProcTargetMcuKernels();
    #endif
    #if defined(ENABLE_VHWA_VPAC0) || defined(ENABLE_VHWA_VPAC1) || defined(ENABLE_VHWA_VPAC2)
    tivxUnRegisterImagingTargetAewbKernels();
    #endif
    appLogPrintf("APP: OpenVX Target kernel deinit ... Done !!!\n");
}

void appIdleLoop(void)
{
    #if defined(__C7604__)
   __asm(" IDLE");
   #endif
}
