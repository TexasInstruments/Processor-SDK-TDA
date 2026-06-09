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
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#if defined(edgeai)
#include "ti/drv/udma/udma.h"
#include "ti/drv/udma/examples/udma_apputils/udma_apputils.h"
#endif
#include <utils/mem/include/app_mem.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <stdint.h>
#if defined(edgeai)
#include <TI/tivx.h>
#include <TI/video_io_capture.h>
#endif
#include <app_mem_map.h>
#include <app_cfg.h>
#include <common.h>

#include <hw/inout.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#if defined(edgeai)
#include <utils/iss/include/app_iss.h>
#include <utils/sensors/include/app_sensors.h>
#include <utils/hwa/include/app_hwa_api.h>
#include <utils/udma/include/app_udma.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/dss/include/app_dss_defaults.h>
#include <TI/j7_kernels_imaging_aewb.h>

#define APP_ASSERT_SUCCESS(x)  { if((x)!=0) while(1); }
#endif
/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

/* Counter for tracking the {init, de-init} calls. This is also used to
 * guarantee a single init/de-init operation.
 */
static uint32_t gInitCount = 0U;

static int32_t appCommonInitLocal();
static int32_t appCommonDeInitLocal();

core_config_t core_config[] = {
#ifdef ENABLE_IPC_MPU1
    {APP_IPC_CPU_MPU1_0, false, true},
#endif
#ifdef ENABLE_IPC_MCU1_0
    {APP_IPC_CPU_MCU1_0, true, true},
#endif
#ifdef ENABLE_IPC_C7x_1
    {APP_IPC_CPU_C7x_1, true, true},
#endif
};

uint8_t num_cpus = sizeof(core_config)/sizeof(core_config[0]);

#if defined(edgeai)
/*
 * UDMA driver objects
 */
struct Udma_DrvObj      *gUdmaDrvObj;
struct Udma_ChObj       *gUdmaChObj;

/*
 * Definition and Macro for debug vs. logging
 */
#define APP_LOG 1
#define APP_DBG 2
#define APP_PRINT(level, f_, ...)   if(((gVerbose == 1) && (level == APP_DBG)) || \
                                      (level == APP_LOG)) \
                                      printf((f_), ##__VA_ARGS__)
/*
 * Memory file descriptor
 */
int memFd = -1;
uint16_t gVerbose = 0;

void *App_alloc(size_t size, paddr64_t *paddr, int noCache)
{
    off64_t offset;
    size_t contig_len;
    uint64_t physAddr = 0;

    void *buf = MAP_FAILED;
    int fd    = -1;
    int flags = 0;
    int prot  = PROT_READ | PROT_WRITE;


    /* The caller of the API, may or may not have provided a physical address to be mapped */
    if(paddr != NULL)
    {
        physAddr = *paddr;
    }

    if(noCache)
    {
        APP_PRINT(APP_DBG, "%s: Cache Disabled\n",__func__);
        prot |= PROT_NOCACHE;
    }

    /*
     * User did not specify a physical address, then use the memory pool.
     * This is the expected usage.
     *
     * Specifying physical address only required for failure testing
     */
    if(physAddr == 0)
    {
        fd = memFd;
        flags = MAP_SHARED;
    }
    /*
     * User did specify a src/dst physical address so create a memory
     * mapping directly to that address.
     *
     */
    else
    {
        fd = NOFD;
        flags = MAP_PHYS | MAP_PRIVATE;
        APP_PRINT(APP_DBG,
               "%s: physical addr specified, mmap flags = MAP_PHYS | MAP_PRIVATE\n",
               __FUNCTION__);
    }

    buf = mmap64(0, size, prot, flags, fd, physAddr);
    if(buf != MAP_FAILED)
    {
        /*
        * If physical address was not provided, figure out which
        * address was mapped.
        */
        if(physAddr == 0)
        {
            int tmp_fd = -1;
            if (posix_mem_offset64(buf, size, &offset, &contig_len, &tmp_fd) != 0)
            {

                APP_PRINT(APP_LOG, "%s:%d: Error: Could not obtain buffer physical address\n",
                __FUNCTION__, __LINE__);

                munmap(buf, size);

                return MAP_FAILED;
            }
            if (paddr != NULL)
            {
                *paddr = (uint32_t) offset;
            }
            physAddr = offset;
        }

        APP_PRINT(APP_DBG, "%s: Alloc successful; Virt: 0x%p, Phy: 0x%lx Contig_len: %ld\n",
              __FUNCTION__, buf, physAddr, contig_len);
    }
    else
    {
        perror("mmap64 failed");
        APP_PRINT(APP_LOG, "%s: Alloc failed; Virt: 0x%p, Phys: 0x%lx\n",
                    __FUNCTION__,  buf, offset);
    }


    if(paddr != NULL)
    {
        *paddr = physAddr;
    }

    return buf;
}

void App_free(void *addr, size_t size)
{
    munmap(addr, size);
}

uint64_t Udma_qnxVirtToPhyFxn(const void * virtAddr,
                              uint32_t chNum,
                              void *appData)
{
    off64_t    phyAddr = 0;
    uint32_t   length;

    if(appData != NULL_PTR) {
        length = (uint32_t) *((uint32_t *) appData);
    }
    else {
        APP_PRINT(APP_LOG,"%s: Must specify memory size to map\n",__FUNCTION__);
        return -1;
    }

    int     tmp_fd = -1;
    size_t  contig_len = 0;
    if (posix_mem_offset64((void *) virtAddr, length, &phyAddr, &contig_len, &tmp_fd) != 0)
    {
        if (errno != EAGAIN) {
            printf("%s:Error from mem_offset - errno=%d\n", __func__, errno);
        }
        else if (phyAddr == 0) {
            printf("%s:Error from mem_offset - errno=%d and phyAddr is NULL \n", __func__, errno);
        }
    }

    APP_PRINT(APP_DBG,"%s: virt/%p phy/0x%lx\n",__FUNCTION__, virtAddr, phyAddr);

    return (uint64_t ) phyAddr;
}

void * Udma_qnxPhyToVirtFxn(uint64_t phyAddr,
                            uint32_t chNum,
                            void *appData)
{
    uint64_t *temp = 0;
    uint32_t length = 0;
    int prot = PROT_READ | PROT_WRITE;
    int flags;

    if(appData != NULL_PTR) {
        length = (uint32_t) *((uint32_t *) appData);
    }
    else {
        APP_PRINT(APP_LOG,"%s: Must specify memory size to map\n",__FUNCTION__);
        return NULL;
    }

    flags =  MAP_SHARED;
    temp = mmap64(0, length, prot, flags, memFd, phyAddr);
    if(temp != MAP_FAILED)
    {
        APP_PRINT(APP_LOG,"%s: mma64 failed errno/%d, phy/0x%lx\n",__FUNCTION__, errno, phyAddr);
    }
    else
    {
        APP_PRINT(APP_DBG,"%s: virt/%p phy/0x%lx\n",__FUNCTION__, temp, phyAddr);
    }

    return ((void *) temp);
}

static void appRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel init ... !!!\n");
    tivxRegisterVideoIOTargetCaptureKernels();
    tivxRegisterImagingTargetAewbKernels();
    appLogPrintf("APP: OpenVX Target kernel init ... Done !!!\n");
}
#endif
int32_t appCommonInit()
{
    int32_t status = 0;
    #if defined(edgeai)
    app_udma_init_prms_t *prms = NULL;
    #endif
    pthread_mutex_lock(&gMutex);

    if (gInitCount == 0U)
    {
        status = appCommonInitLocal();
        #if defined(edgeai)
        APP_ASSERT_SUCCESS(status);

        /* Allocate UDMA Objects */
        gUdmaDrvObj = (struct Udma_DrvObj *) App_alloc(sizeof(struct Udma_DrvObj), NULL, 0);
        gUdmaChObj  =  (struct Udma_ChObj *) App_alloc(sizeof(struct Udma_ChObj),  NULL, 0);
        if ((gUdmaDrvObj == MAP_FAILED) || (gUdmaDrvObj == MAP_FAILED))
        {
            APP_PRINT(APP_LOG, "%s: Allocation of UDMA objects failed\n",__func__);
            exit(-1);
        }

        status = appFvid2Init();
        APP_ASSERT_SUCCESS(status);
        status = appUdmaInit(prms);
        APP_ASSERT_SUCCESS(status);

        tivxInit();
        tivxHostInit();
        appRegisterOpenVXTargetKernels();

        status = appCsi2RxInit();
        APP_ASSERT_SUCCESS(status);

        status = appIssInit();
        APP_ASSERT_SUCCESS(status);
#endif
    }

    gInitCount++;
    pthread_mutex_unlock(&gMutex);

    return status;
}
#if defined(edgeai)
static void appUnRegisterOpenVXTargetKernels()
{
    appLogPrintf("APP: OpenVX Target kernel deinit ... !!!\n");
    tivxUnRegisterVideoIOTargetCaptureKernels();
    tivxUnRegisterImagingTargetAewbKernels();
    appLogPrintf("APP: OpenVX Target kernel deinit ... Done !!!\n");
}
#endif
int32_t appCommonDeInit()
{
    int32_t status = 0;

    pthread_mutex_lock(&gMutex);

    if (gInitCount != 0U)
    {
        gInitCount--;

        if (gInitCount == 0U)
        {
            #if defined(edgeai)
            appUnRegisterOpenVXTargetKernels();
            tivxHostDeInit();
            tivxDeInit();
            appIssDeInit();
            appCsi2RxDeInit();
            appFvid2DeInit();
            appUdmaDeInit();
           #endif
            status = appCommonDeInitLocal();
        }
    }
    else
    {
        status = -1;
    }

    pthread_mutex_unlock(&gMutex);

    return status;
}

static int32_t appCommonInitLocal()
{
    int32_t status = 0;
    app_log_init_prm_t log_init_prm;
    app_ipc_init_prm_t ipc_init_prm;
    app_remote_service_init_prms_t remote_service_init_prm;

    printf("APP: Init QNX ... !!!\n");

    status = appLogGlobalTimeInit();
    if(status!=0)
    {
        printf("APP: ERROR: Global timer init failed !!!\n");
    }

    if(status==0)
    {
        appLogInitPrmSetDefault(&log_init_prm);

        log_init_prm.shared_mem = (app_log_shared_mem_t *)APP_LOG_MEM_ADDR;
        log_init_prm.self_cpu_index = APP_IPC_CPU_MPU1_0;
        strncpy(log_init_prm.self_cpu_name, "MPU1_0", APP_LOG_MAX_CPU_NAME);

        status = appLogWrInit(&log_init_prm);
        if(status!=0)
        {
            printf("APP: ERROR: Log writer init failed !!!\n");
        }
    }
    if(status==0)
    {
        app_mem_init_prm_t mem_init_prm;

        mem_init_prm.base = DDR_SHARED_MEM_ADDR;
        mem_init_prm.size = DDR_SHARED_MEM_SIZE;

        status = appMemInit(&mem_init_prm);
        if(status!=0)
        {
            printf("APP: ERROR: Memory init failed !!!\n");
        }
    }
    if(status==0)
    {
        appIpcInitPrmSetDefault(&ipc_init_prm);
        appRemoteServiceInitSetDefault(&remote_service_init_prm);

        #ifdef ENABLE_IPC
        for (int i = 0; i < num_cpus; i++)
        {
            ipc_init_prm.enabled_cpu_id_list[i] = core_config[i].cpu;
        }

        ipc_init_prm.num_cpus = num_cpus;
        ipc_init_prm.self_cpu_id = APP_IPC_CPU_MPU1_0;

        /* Adding so vring memory not NULL */
        ipc_init_prm.ipc_vring_mem =  (void *)  IPC_VRING_MEM_ADDR;
        ipc_init_prm.ipc_vring_mem_size = IPC_VRING_MEM_SIZE;

        ipc_init_prm.tiovx_obj_desc_mem   = (void *) mmap_device_memory(0, TIOVX_OBJ_DESC_MEM_SIZE,
                PROT_READ|PROT_WRITE|PROT_NOCACHE, 0,
                TIOVX_OBJ_DESC_MEM_ADDR);
        ipc_init_prm.tiovx_obj_desc_mem_size = TIOVX_OBJ_DESC_MEM_SIZE;

        ipc_init_prm.tiovx_log_rt_mem   = (void *) mmap_device_memory(0, TIOVX_LOG_RT_MEM_SIZE,
                PROT_READ|PROT_WRITE|PROT_NOCACHE, 0,
                TIOVX_LOG_RT_MEM_ADDR);
        ipc_init_prm.tiovx_log_rt_mem_size = TIOVX_LOG_RT_MEM_SIZE;

        status = appIpcInit(&ipc_init_prm);
        if(status!=0)
        {
            printf("APP: ERROR: IPC init failed !!!\n");
        }
        status = appRemoteServiceInit(&remote_service_init_prm);
        if(status!=0)
        {
            printf("APP: ERROR: Remote service init failed !!!\n");
        }
        status = appPerfStatsInit();
        if(status!=0)
        {
            printf("APP: ERROR: Perf stats init failed !!!\n");
        }
        status = appPerfStatsRemoteServiceInit();
        if(status!=0)
        {
            printf("APP: ERROR: Perf stats remote service init failed !!!\n");
        }
        #endif

        appLogPrintGtcFreq();
    }
    printf("APP: Init ... Done !!!\n");
    return status;
}

static int32_t appCommonDeInitLocal()
{
    int32_t status = 0;

    printf("APP: Deinit ... !!!\n");

    appRemoteServiceDeInit();
    appIpcDeInit();
    appLogWrDeInit();
    appMemDeInit();

    /* De-init GTC timer */
    status = appLogGlobalTimeDeInit();

    printf("APP: Deinit ... Done !!!\n");

    return status;
}

/* Dummy function to avoid build issues */
void appUtilsTaskInit(void)
{
    #if defined(FREERTOS)
    /* Any task that uses the floating point unit MUST call portTASK_USES_FLOATING_POINT()
     * before any floating point instructions are executed.
     */
    portTASK_USES_FLOATING_POINT();
    #endif
}
