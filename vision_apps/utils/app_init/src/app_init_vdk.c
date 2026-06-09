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

#include <Ipc_Notify_Hal.h>
#include <RPMessage_Hal_Linux_ResourceTable.h>
#include <RPMessage_Hal.h>

#include <utils/ipc/include/app_ipc.h>
#include <utils/ipc/include/mcu_sdk_ipc.h>
#include <utils/console_io/include/app_log.h>
#include <utils/remote_service/include/app_remote_service.h>

#include <TI/tivx.h>
#include <TI/tivx_ext_vdk.h>
#include <common/tivx_platform_pc.h>

#include <app_mem_map.h>
#include <common/app_cfg.h>
#include <utils/app_init/include/app_init.h>

#include <TI/dl_kernels.h>
#include <TI/j7_imaging_aewb.h>
#include <TI/hwa_vpac_viss.h>
#include <TI/hwa_vpac_ldc.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_vpac_nf.h>
#include <TI/hwa_dmpac_sde.h>
#include <TI/hwa_dmpac_dof.h>
#if !defined(VDK_STUB)
#include <TI/tivx_img_proc.h>
#endif

#include <sys/prctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DUMMY_HANDLER_INIT_ADDR 0xDE7EC7ED
#define RPMSG_VRING_ADDR_ANY (~0)

typedef struct
{
    uint32_t cur_core;
    uint32_t enabled_cores;
} vdk_core_init_args_t;

static uint8_t gAppCommonInitDone = 0;
static uint8_t gAppWrLogInitDone = 0;

#if !defined(QNX)
static uint64_t cpu_dts_addr_list[CORE_ID_MAX] =
    {
        0x0,
        DDR_MCU0_DTS_ADDR,
        DDR_MCU1_DTS_ADDR,
        DDR_MCU2_DTS_ADDR,
        DDR_MCU3_DTS_ADDR,
        DDR_MCU4_DTS_ADDR,
        DDR_C7x_1_DTS_ADDR,
        DDR_C7x_2_DTS_ADDR,
        DDR_C7x_3_DTS_ADDR,
        DDR_C7x_4_DTS_ADDR,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0
};
#endif /* #if !defined(QNX) */

static uint32_t g_ipc_to_app_cpu_id[CORE_ID_MAX] =
{
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_MCU0_M55,
    APP_IPC_CPU_MCU1_M55,
    APP_IPC_CPU_MCU2_M55,
    APP_IPC_CPU_MCU3_M55,
    APP_IPC_CPU_MCU4_M55,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4,
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_INVALID,
    APP_IPC_CPU_INVALID,
};

static uint32_t g_ipc_to_ovx_cpu_id[CORE_ID_MAX] =
{
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_MCU0,
    TIVX_CPU_ID_MCU1,
    TIVX_CPU_ID_MCU2,
    TIVX_CPU_ID_MCU3,
    TIVX_CPU_ID_MCU4,
    TIVX_CPU_ID_DSP_C7_1,
    TIVX_CPU_ID_DSP_C7_2,
    TIVX_CPU_ID_DSP_C7_3,
    TIVX_CPU_ID_DSP_C7_4,
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_INVALID,
    TIVX_CPU_ID_INVALID,
};

static char* g_ipc_to_cpu_name[CORE_ID_MAX] =
{
    "INVALID",
    "TIVX_MCU0",
    "TIVX_MCU1",
    "TIVX_MCU2",
    "TIVX_MCU3",
    "TIVX_MCU4",
    "TIVX_C71",
    "TIVX_C72",
    "TIVX_C73",
    "TIVX_C74",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
};

static void appRegisterOpenVXTargetKernels();
static void appUnRegisterOpenVXTargetKernels();

static void *appVdkIpcInitTaskMain(void *args);

#if !defined(QNX)
static RPMessage_Hal_ResourceTable *ipc_resource_table[CORE_ID_MAX];

static void appVdkResourceTableInit(uint32_t core_id);

static void appVdkResourceTableInit(uint32_t core_id)
{
    uint64_t dts_addr = cpu_dts_addr_list[core_id];
    void *dts_mem = (void *)tivxVdkGetHostPtrFromPhyPtr(dts_addr);
    void *mapped_mem = mmap(dts_mem, sizeof(RPMessage_Hal_ResourceTable), \
    PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | 0x20, -1, 0);

    if (mapped_mem == MAP_FAILED) {
        appLogPrintf("mmap failed\n");
    }
    else
    {
        ipc_resource_table[core_id] = (RPMessage_Hal_ResourceTable*)mapped_mem;

        RPMessage_Hal_ResourceTable temp =
        {
            {
                1U,         /* we're the first version that implements this */
                2U,         /* number of entries in the table */
                { 0U, 0U, } /* reserved, must be zero */
            },

            /* offsets to entries */
            {
                offsetof(RPMessage_Hal_ResourceTable, Vdev),
                offsetof(RPMessage_Hal_ResourceTable, Trace),
            },

            /* rpmsg Vdev entry */
            {
                RPMESSAGE_RSC_TYPE_VDEV, RPMESSAGE_RSC_VIRTIO_ID_RPMSG, 0U,
                1U, 0U, 0U, 0U, 2U, { 0U, 0U },
                /* no config data */
            },
            /* the two vrings */
            { RPMSG_VRING_ADDR_ANY, 4096U, 256U, 1U, 0U },
            { RPMSG_VRING_ADDR_ANY, 4096U, 256U, 2U, 0U },

            {
                (RPMESSAGE_RSC_TRACE_INTS_VER0 | RPMESSAGE_RSC_TYPE_TRACE), 0, 0x80000, 0, "trace:pc",
            },
        };

        *(RPMessage_Hal_ResourceTable *)(ipc_resource_table[core_id]) = temp;
    }
}
#endif /* #if !defined(QNX) */

int32_t appVdkInit(void)
{
    int32_t status = 0;

    if (gAppCommonInitDone == 0)
    {
        pthread_attr_t thread_attr;
        pthread_t handle[CORE_ID_MAX];
        uint64_t handler_addr;
        app_ipc_notify_handler_f dummy_handler;
        uint32_t enabled_cores = tivxVdkGetEmulatedCores();
        uint32_t cur_core;
        vdk_core_init_args_t *core_init_args[CORE_ID_MAX];

        tivx_set_debug_zone(VX_ZONE_INFO);

        for (cur_core = 0; cur_core < CORE_ID_MAX; cur_core++)
        {
            if (enabled_cores & (1U << cur_core))
            {
                prctl(PR_SET_NAME, g_ipc_to_cpu_name[cur_core]);
                dummy_handler = (app_ipc_notify_handler_f)DUMMY_HANDLER_INIT_ADDR;

                core_init_args[cur_core] = (vdk_core_init_args_t*)malloc(sizeof(vdk_core_init_args_t));

                core_init_args[cur_core]->cur_core = cur_core;
                core_init_args[cur_core]->enabled_cores = enabled_cores;

                prctl(PR_SET_NAME, g_ipc_to_cpu_name[cur_core]);

                appIpcVdkRegisterGetCpuId((app_vdk_get_cpu_id_f)tivxVdkGetSelfAppIpcCpuId);

                appIpcRegisterNotifyHandler(dummy_handler);

                #if !defined(QNX)
                appVdkResourceTableInit(cur_core);
                #endif /* #if defined(QNX) */

                status = pthread_attr_init(&thread_attr);

                if (status == 0)
                {
                    status = pthread_attr_setstacksize(&thread_attr, (64u*1024u));

                    if (status == 0)
                    {
                        status = pthread_create(&handle[cur_core], &thread_attr, &appVdkIpcInitTaskMain, (void*)core_init_args[cur_core]);
                        if (status != 0)
                        {
                            appLogPrintf("pthread_create failed\n");
                        }
                    }

                    (void)pthread_attr_destroy(&thread_attr);
                }

                if (status != 0)
                {
                    appLogPrintf("Failed to create VDK IPC init task %d\n", status);
                }

                /* This spawned thread must initialize certain structures within appIpcInit such */
                /* as the ipc_notify_handler before app init can proceed in the current thread. */
                do {
                    sleep(0.5);
                    handler_addr = (uint64_t)appIpcGetNotifyHandler();
                } while(handler_addr == DUMMY_HANDLER_INIT_ADDR);

                tivxSetSelfCpuId(g_ipc_to_ovx_cpu_id[cur_core]);
                tivxInit();
                appRegisterOpenVXTargetKernels();
            }
        }
    }

    gAppCommonInitDone++;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    return status;
}

static void *appVdkIpcInitTaskMain(void *args)
{
    #if !defined(VDK_STUB)
    app_remote_service_init_prms_t init_prms;
    #endif /* #if !defined(VDK_STUB) */

    int32_t status = 0;
    app_ipc_init_prm_t ipc_init_prm;
    app_log_init_prm_t log_init_prm;
    vdk_core_init_args_t *core_init_args = (vdk_core_init_args_t*)args;
    uint32_t cur_core = core_init_args->cur_core;
    void *tiovx_obj_desc_mem = (void *)TIOVX_OBJ_DESC_MEM_ADDR;
    void *ipc_vring_mem = (void *)IPC_VRING_MEM_ADDR;
    void *log_shared_mem = tivxVdkGetHostPtrFromSymbol("app_log_shared_mem");

    prctl(PR_SET_NAME, g_ipc_to_cpu_name[cur_core]);

    appIpcInitPrmSetDefault(&ipc_init_prm);

    ipc_init_prm.tiovx_obj_desc_mem = (void*)tiovx_obj_desc_mem;
    ipc_init_prm.tiovx_obj_desc_mem_size = TIOVX_OBJ_DESC_MEM_SIZE;

    ipc_init_prm.ipc_vring_mem = (void*)ipc_vring_mem;
    ipc_init_prm.ipc_vring_mem_size = IPC_VRING_MEM_SIZE;

    ipc_init_prm.num_cpus = 0;

    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MPU1_0;
    ipc_init_prm.num_cpus++;

    #ifdef ENABLE_IPC_MCU0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU0_M55;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_MCU1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU1_M55;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_MCU2
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU2_M55;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_MCU3
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU3_M55;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_MCU4
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_MCU4_M55;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU0_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU0_0;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU0_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU0_1;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU1_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU1_0;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU1_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU1_1;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU2_0
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU2_0;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_RMCU2_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_RMCU2_1;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_C7x_1
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C7x_1;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_C7x_2
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C7x_2;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_C7x_3
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C7x_3;
    ipc_init_prm.num_cpus++;
    #endif

    #ifdef ENABLE_IPC_C7x_4
    ipc_init_prm.enabled_cpu_id_list[ipc_init_prm.num_cpus] = APP_IPC_CPU_C7x_4;
    ipc_init_prm.num_cpus++;
    #endif

    ipc_init_prm.self_cpu_id = g_ipc_to_app_cpu_id[cur_core];

    strncpy(log_init_prm.self_cpu_name, "HOSTEMU", APP_LOG_MAX_CPU_NAME);
    #if !defined(QNX)
    ipc_init_prm.enable_tiovx_ipc_announce = 1;
    #else
    ipc_init_prm.enable_tiovx_ipc_announce = 0;
    #endif /* #if !defined(QNX) */

    #if !defined(QNX)
    ipc_init_prm.ipc_resource_tbl = (void *)(RPMessage_Hal_ResourceTable *)ipc_resource_table[cur_core];
    #endif /* #if defined(QNX) */

    log_init_prm.shared_mem = log_shared_mem;
    log_init_prm.self_cpu_index = ipc_init_prm.self_cpu_id;

    #if !defined(VDK_STUB)
    if (gAppWrLogInitDone == 0U)
    {
        status = appLogWrInit(&log_init_prm);
        gAppWrLogInitDone++;

        if (status != 0)
        {
            appLogPrintf("Call to appLogWrInit failed\n");
        }
    }
    #endif /* #if !defined(VDK_STUB) */

    status = appIpcInit(&ipc_init_prm);

    #if !defined(VDK_STUB)
    if (status == 0)
    {
        VX_PRINT(VX_ZONE_INFO, "Call to appIpcInit passsed\n");

        uint32_t sync_cpu_id_list[APP_LOG_MAX_CPUS];
        uint32_t i, self_cpu_id, master_cpu_id, num_sync_cpus;
        master_cpu_id = APP_IPC_CPU_MCU0_M55;
        self_cpu_id = ipc_init_prm.self_cpu_id;
        num_sync_cpus = 0;

        for(i=0; i<ipc_init_prm.num_cpus; i++)
        {
            if(i<APP_LOG_MAX_CPUS)
            {
                /* dont sync with MPU1 running linux/qnx since that is taken care by the kernel */
                if(ipc_init_prm.enabled_cpu_id_list[i]!=APP_IPC_CPU_MPU1_0)
                {
                    sync_cpu_id_list[num_sync_cpus] = ipc_init_prm.enabled_cpu_id_list[i];
                    num_sync_cpus++;
                }
            }
        }

        appLogPrintf("APP: Syncing with %d CPUs ... !!!\n", num_sync_cpus);
        appLogCpuSyncInit(master_cpu_id, self_cpu_id, sync_cpu_id_list, num_sync_cpus);
        appLogPrintf("APP: Syncing with %d CPUs ... Done !!!\n", num_sync_cpus);

        appRemoteServiceInitSetDefault(&init_prms);
        appRemoteServiceInit(&init_prms);
    }
    else
    {
        appLogPrintf("Call to appIpcInit failed\n");
    }
    #endif /* #if !defined(VDK_STUB) */

    free(core_init_args);

    return NULL;
}

int32_t appVdkDeInit()
{
    uint32_t cur_core;
    uint32_t enabled_cores = tivxVdkGetEmulatedCores();

    for (cur_core = 0; cur_core <= CORE_ID_MAX; cur_core++)
    {
        if (enabled_cores & (1U << cur_core))
        {
            prctl(PR_SET_NAME, g_ipc_to_cpu_name[cur_core]);
            appIpcDeInit();
            tivxSetSelfCpuId(g_ipc_to_ovx_cpu_id[cur_core]);
            appUnRegisterOpenVXTargetKernels();
        }
    }

    return 0;
}

void appVdkRegisterKernels()
{
    uint32_t cur_core;
    uint32_t enabled_cores = tivxVdkGetEmulatedCores();

    for (cur_core = 0; cur_core <= CORE_ID_MAX; cur_core++)
    {
        if (enabled_cores & (1U << cur_core))
        {
            tivxSetSelfCpuId(g_ipc_to_ovx_cpu_id[cur_core]);
            appRegisterOpenVXTargetKernels();
        }
    }
}

static void appRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel init ... !!!\n");

    #if !defined(VDK_STUB)

    tivx_set_debug_zone(VX_ZONE_INFO);

    void tivxRegisterTestKernelsTargetArmKernels(void);
    tivxRegisterTestKernelsTargetArmKernels();

    void tivxRegisterCaptureTargetArmKernels(void);
    tivxRegisterCaptureTargetArmKernels();

    void tivxRegisterTestKernelsTargetDspKernels(void);
    tivxRegisterTestKernelsTargetDspKernels();

    tivxRegisterHwaTargetVpacMscKernels();

    tivxRegisterTIDLTargetKernels();

    tivxRegisterImgProcTargetR5FKernels();

    /*
    void app_c7x_target_kernel_img_add_register(void);
    app_c7x_target_kernel_img_add_register();

    tivxRegisterHwaTargetVpacLdcKernels();
    tivxRegisterHwaTargetVpacVissKernels();
    tivxRegisterHwaTargetVpacNfKernels();

    tivxRegisterHwaTargetDmpacSdeKernels();
    tivxRegisterHwaTargetDmpacDofKernels();
    tivxRegisterHwaTargetArmKernels();
    */

    tivx_clr_debug_zone(VX_ZONE_INFO);

    #endif /* #if !defined(VDK_STUB) */

    appLogPrintf("APP: OpenVX Target kernel init ... Done !!!\n");
}

static void appUnRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel deinit ... !!!\n");

    #if !defined(VDK_STUB)

    void tivxUnRegisterTestKernelsTargetDspKernels(void);
    tivxUnRegisterTestKernelsTargetDspKernels();

    void tivxUnRegisterCaptureTargetArmKernels(void);
    tivxUnRegisterCaptureTargetArmKernels();

    void tivxUnRegisterTestKernelsTargetArmKernels(void);
    tivxUnRegisterTestKernelsTargetArmKernels();

    tivxUnRegisterHwaTargetVpacMscKernels();

    tivxUnRegisterTIDLTargetKernels();

    tivxUnRegisterImgProcTargetR5FKernels();

    /*
    void app_c7x_target_kernel_img_add_unregister(void);
    app_c7x_target_kernel_img_add_unregister();

    tivxUnRegisterHwaTargetVpacLdcKernels();
    tivxUnRegisterHwaTargetVpacNfKernels();
    tivxUnRegisterHwaTargetVpacVissKernels();
    tivxUnRegisterHwaTargetDmpacSdeKernels();
    tivxUnRegisterHwaTargetDmpacDofKernels();
    tivxUnRegisterHwaTargetArmKernels();
    */

    #endif /* #if !defined(VDK_STUB) */

    appLogPrintf("APP: OpenVX Target kernel deinit ... Done !!!\n");
}

uint32_t appGetHostOSType(void);
uint32_t appGetHostOSType(void)
{
    uint32_t host_type;

    #if defined(QNX)
    host_type = APP_HOST_TYPE_QNX;
    #else
    host_type = APP_HOST_TYPE_LINUX;
    #endif /* #if defined(QNX) */

    return host_type;
}
