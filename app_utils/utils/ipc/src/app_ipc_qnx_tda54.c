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
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/rtos/include/app_rtos.h>
#include <drivers/ipc_rpmsg.h>
#include <drivers/ipc_notify.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

/* #defines */
#define IPC_SOK                 SystemP_SUCCESS
#define CSL_CORE_ID_MAX         (17U)
#define CORE_ID_MAX             (20U)

#define IPC_RPMESSAGE_OBJ_SIZE      (256u)
#define IPC_RPMESSAGE_MSG_SIZE      (496u + 32u)
#define IPC_RPMESSAGE_BUF_SIZE(n)   (IPC_RPMESSAGE_MSG_SIZE*(n)+IPC_RPMESSAGE_OBJ_SIZE)

#define APP_IPC_MAX_TASK_NAME       (12u)
#define APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE   (64*1024u)
#define APP_IPC_RPMESSAGE_RX_TASK_PRI          (10u)

/* HW Spinlock */
#define APP_IPC_HW_SPIN_LOCK_MAX        (256u)
#define APP_IPC_HW_SPIN_LOCK_MMR_SIZE   32768
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x30020000u)
#define APP_IPC_HW_SPIN_LOCK_OFFSET(x)  ((uint32_t)0x800u + (uint32_t)4u*(uint32_t)(x))

/* functions */
#define Ipc_mpGetSelfName()     SOC_getCoreName(CSL_CORE_ID_A720_0)
#define Ipc_mpGetName(name)     SOC_getCoreName(name)

static uint8_t g_app_rpmessage_rx_task_stack[APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;

typedef struct {

    app_ipc_init_prm_t prm;
    uint32_t rpmsg_tx_endpt[APP_IPC_CPU_MAX];
    RPMessage_Object rpmsg_rx_handle;
    app_ipc_notify_handler_f ipc_notify_handler;
    app_rtos_task_handle_t task_handle;
    uint32_t task_stack_size;
    uint8_t *task_stack;
    uint32_t task_pri;
    uint8_t  rpmsg_rx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    char     task_name[APP_IPC_MAX_TASK_NAME];
    uint32_t tiovx_rpmsg_rx_endpt;
    void     *spin_lock_ptr;
} app_ipc_obj_t;

static app_ipc_obj_t g_app_ipc_obj;

#if defined (SOC_TDA54)
#define APP_MCUSDK_IPC_CPU_INVALID (0xFFFFFFFFU)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A720_0,
    CSL_CORE_ID_MCU0,
    CSL_CORE_ID_MCU1,
    CSL_CORE_ID_MCU2,
    CSL_CORE_ID_MCU3,
    CSL_CORE_ID_MCU4,
    CSL_CORE_ID_RMCU0_0,
    CSL_CORE_ID_RMCU0_1,
    CSL_CORE_ID_RMCU1_0,
    CSL_CORE_ID_RMCU1_1,
    CSL_CORE_ID_RMCU2_0,
    CSL_CORE_ID_RMCU2_1,
    CSL_CORE_ID_DSP0,
    CSL_CORE_ID_DSP1,
    CSL_CORE_ID_DSP2,
    CSL_CORE_ID_DSP3
};

static uint32_t g_ipc_to_app_cpu_id[CORE_ID_MAX] =
{
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_MCU0_M55,
    APP_IPC_CPU_MCU1_M55,
    APP_IPC_CPU_MCU2_M55,
    APP_IPC_CPU_MCU3_M55,
    APP_IPC_CPU_MCU4_M55,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4,
    APP_IPC_CPU_MPU1_0,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_MCUSDK_IPC_CPU_INVALID,
    APP_IPC_CPU_RMCU0_0,
    APP_IPC_CPU_RMCU0_1,
    APP_IPC_CPU_RMCU1_0,
    APP_IPC_CPU_RMCU1_1,
    APP_IPC_CPU_RMCU2_0,
    APP_IPC_CPU_RMCU2_1
};
#endif

const char *SOC_getCoreName(uint16_t coreId)
{
    static char *coreIdNames[CSL_CORE_ID_MAX] = {
        "dmcu0",
        "mcu0",
        "mcu1",
        "mcu2",
        "mcu3",
        "mcu4",
        "dsp0",
        "dsp1",
        "dsp2",
        "dsp3",
        "a720-0",
        "rmcu0-0",
        "rmcu0-1",
        "rmcu1-0",
        "rmcu1-1",
        "rmcu2-0",
        "rmcu2-1",
    };
    const char *name;

    if ((coreId < CSL_CORE_ID_MAX) && (coreIdNames[coreId] != NULL))
    {
        name = coreIdNames[coreId];
    }
    else
    {
        name = coreIdNames[CSL_CORE_ID_MAX];
    }
    return name;
}


uint32_t SOC_getCoreId(const char * coreName)
{
    if (strcmp("dmcu0", coreName) == 0)
    {
        return CSL_CORE_ID_DMCU0;
    }
    else if (strcmp("mcu0", coreName) == 0)
    {
        return CSL_CORE_ID_MCU0;
    }
    else if (strcmp("mcu1", coreName) == 0)
    {
        return CSL_CORE_ID_MCU1;
    }
    else if (strcmp("mcu2", coreName) == 0)
    {
        return CSL_CORE_ID_MCU2;
    }
    else if (strcmp("mcu3", coreName) == 0)
    {
        return CSL_CORE_ID_MCU3;
    }
    else if (strcmp("mcu4", coreName) == 0)
    {
        return CSL_CORE_ID_MCU4;
    }
    else if (strcmp("dsp0", coreName) == 0)
    {
        return CSL_CORE_ID_DSP0;
    }
    else if (strcmp("dsp1", coreName) == 0)
    {
        return CSL_CORE_ID_DSP1;
    }
    else if (strcmp("dsp2", coreName) == 0)
    {
        return CSL_CORE_ID_DSP2;
    }
    else if (strcmp("dsp3", coreName) == 0)
    {
        return CSL_CORE_ID_DSP3;
    }
    else if (strcmp("rmcu0-0", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU0_0;
    }
    else if (strcmp("rmcu0-1", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU0_1;
    }
    else if (strcmp("rmcu1-0", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU1_0;
    }
    else if (strcmp("rmcu1-1", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU1_1;
    }
    else if (strcmp("rmcu2-0", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU2_0;
    }
    else if (strcmp("rmcu2-1", coreName) == 0)
    {
        return CSL_CORE_ID_RMCU2_1;
    }
    return CSL_CORE_ID_MAX;
}

static void appIpcRpmsgRxHandler(RPMessage_Object rpmsg_handle,
                        void *arg, void *data,
                        uint16_t len, uint32_t src_cpu_id,
                        uint16_t src_endpt, uint16_t dst_endpt)
{
    uint32_t app_cpu_id, payload;
    app_ipc_obj_t *obj = arg;

    if( (src_cpu_id<CSL_CORE_ID_MAX) && (len == sizeof(payload))) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_IPC_QNX_UM13 */
    {
        app_cpu_id = g_ipc_to_app_cpu_id[src_cpu_id];
        payload = *(uint32_t*)data;

        #ifdef APP_IPC_DEBUG
        printf("IPC: RX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
            Ipc_mpGetName(src_cpu_id),
            (uint32_t)src_endpt,
            Ipc_mpGetSelfName(),
            (uint32_t)dst_endpt,
            payload);
        #endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM08
<justification end> */
        if((payload & 0xFFFF0000) == 0xDEAD0000)
        {
            appLogPrintf("IPC: RX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
                appIpcGetCpuName(app_cpu_id),
                (uint32_t)obj->prm.tiovx_rpmsg_port_id,
                appIpcGetCpuName(appIpcGetSelfCpuId()),
                (uint32_t)obj->tiovx_rpmsg_rx_endpt,
                payload);
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM08
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM14
<justification end> */
            if(obj->ipc_notify_handler)
/* LDRA_JUSTIFY_END */
            {
                obj->ipc_notify_handler(app_cpu_id, payload);
            }
        }
    }
}

static void appIpcRpmsgRxTaskMain(void* arg0,
                                  void* arg1)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;
    uint32_t done = 0, reply_endpt;
    uint16_t len, src_cpu_id;
    int32_t status = 0;

    while(!done) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_IPC_QNX_UM15 */
    {
        len = 0;
        src_cpu_id = 0;
        reply_endpt = 0;
        memset(&obj->rpmsg_rx_msg_buf, 0, IPC_RPMESSAGE_MSG_SIZE);
        len = (uint16_t)sizeof(obj->rpmsg_rx_msg_buf);
        status = RPMessage_recv(&obj->rpmsg_rx_handle,
                            &obj->rpmsg_rx_msg_buf,
                            &len,
                            &src_cpu_id,
                            &reply_endpt,
                            SystemP_WAIT_FOREVER
                            );
        if(status == SystemP_SUCCESS)
        {
            appIpcRpmsgRxHandler(obj->rpmsg_rx_handle,
                        obj,
                        obj->rpmsg_rx_msg_buf,
                        len,
                        src_cpu_id,
                        reply_endpt,
                        obj->tiovx_rpmsg_rx_endpt);
        }
    }
}

static int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj)
{
    app_rtos_task_params_t qnx_task_prms;
    int32_t status = 0;

    appRtosTaskParamsInit(&qnx_task_prms);
    qnx_task_prms.stacksize = obj->task_stack_size;
    qnx_task_prms.stack = obj->task_stack;
    qnx_task_prms.priority = obj->task_pri;
    qnx_task_prms.arg0 = NULL;
    qnx_task_prms.arg1 = NULL;
    qnx_task_prms.name = (const char*)&obj->task_name[0];
    qnx_task_prms.taskfxn = &appIpcRpmsgRxTaskMain;

    strncpy(obj->task_name, "IPC_RX", APP_IPC_MAX_TASK_NAME);
    obj->task_name[APP_IPC_MAX_TASK_NAME-1] = 0;

    obj->task_handle = (void*)appRtosTaskCreate(&qnx_task_prms);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM11
<justification end> */
    if(obj->task_handle==NULL)
    {
        printf("IPC: ERROR: Unable to create RX task \n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */

    return status;
}

static void appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj)
{
    RPMessage_unblock(&obj->rpmsg_rx_handle);
    appRtosTaskDelete(&obj->task_handle);
}

void appIpcInitPrmSetDefault(app_ipc_init_prm_t *prm)
{
    uint32_t cpu_id = 0;


    prm->num_cpus = 0;
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        prm->enabled_cpu_id_list[cpu_id] = APP_IPC_CPU_INVALID;
    }
    prm->tiovx_rpmsg_port_id = APP_IPC_TIOVX_RPMSG_PORT_ID;
    prm->tiovx_obj_desc_mem = NULL;
    prm->tiovx_obj_desc_mem_size = 0;
    prm->ipc_vring_mem = NULL;
    prm->ipc_vring_mem_size = 0;
    prm->tiovx_log_rt_mem = NULL;
    prm->tiovx_log_rt_mem_size = 0;
    prm->self_cpu_id = APP_IPC_CPU_MPU1_0;
    prm->ipc_resource_tbl = NULL;
    prm->enable_tiovx_ipc_announce = 0;
}

int32_t appIpcInit(app_ipc_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id = 0;

    app_ipc_obj_t *obj = &g_app_ipc_obj;

    printf("%s: IPC: Init QNX ... !!!\n", __func__);

    obj->prm = *prm;
    obj->ipc_notify_handler = NULL;
    obj->task_handle = NULL;
    obj->task_stack = g_app_rpmessage_rx_task_stack;
    obj->task_stack_size = APP_IPC_RPMESSAGE_RX_TASK_STACK_SIZE;
    obj->task_pri = APP_IPC_RPMESSAGE_RX_TASK_PRI;

    obj->spin_lock_ptr = mmap_device_memory(0, APP_IPC_HW_SPIN_LOCK_MMR_SIZE,
            PROT_READ|PROT_WRITE|PROT_NOCACHE, 0,
            APP_IPC_HW_SPIN_LOCK_MMR_BASE);

    if(obj->spin_lock_ptr == MAP_FAILED)
    {
           printf("IPC: ERROR: Unable to map spin lock memory !!!\n");
           status = -1;
    }
    if(prm->num_cpus>APP_IPC_CPU_MAX)
    {
        printf("IPC: ERROR: Invalid number of CPUs !!!\n");
        status = -1;
    }
    if( (obj->prm.tiovx_obj_desc_mem==MAP_FAILED) || (obj->prm.tiovx_obj_desc_mem_size==0) )
    {
        printf("IPC: ERROR: Invalid tiovx obj desc memory address or size !!!\n");
        status = -1;
    }
    if( (obj->prm.tiovx_log_rt_mem==MAP_FAILED) || (obj->prm.tiovx_log_rt_mem_size==0) )
    {
        printf("IPC: ERROR: Invalid tiovx rt memory address or size !!!\n");
        status = -1;
    }
    if( (prm->ipc_vring_mem==NULL) || (prm->ipc_vring_mem_size==0) )
    {
        printf("IPC: ERROR: Invalid ipc vring memory address/%ld or size/%d !!!\n",(long int) prm->ipc_vring_mem, prm->ipc_vring_mem_size);
        status = -1;
    }
    if(prm->self_cpu_id>=APP_IPC_CPU_MAX)
    {
        printf("IPC: ERROR: Invalid self cpu id !!!\n");
        status = -1;
    }
    if(status==0)
    {
        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
            if(prm->enabled_cpu_id_list[cpu_id]>=APP_IPC_CPU_MAX)
            {
                printf("IPC: ERROR: Invalid cpu id in enabled_cpu_id_list @ index %d !!!\n", cpu_id);
                status = -1;
            }
        }
    }
    if(status==0)
    {
        obj->tiovx_rpmsg_rx_endpt = prm->tiovx_rpmsg_port_id;
        /* Initialize IPC such that it adheres to IPC OSAL*/
        status = RPMessage_init(NULL);
    }
    if(status==0)
    {
        RPMessage_CreateParams rpmsg_createRx_prms;

        RPMessage_CreateParams_init(&rpmsg_createRx_prms);
        rpmsg_createRx_prms.localEndPt = (uint16_t)prm->tiovx_rpmsg_port_id;

        status = RPMessage_construct(&obj->rpmsg_rx_handle, &rpmsg_createRx_prms);

        if(SystemP_SUCCESS !=status)
        {
            appLogPrintf("IPC: ERROR: Unable to create rpmessage rx handle !!!\n");
        }
    }
    if(status==0)
    {
        status = appIpcCreateRpmsgRxTask(obj);
        if(status!=0)
        {
            printf("IPC: ERROR: appIpcCreateRpmsgRxTask failed !!!\n");
        }
    }
    printf("%s: IPC: Init ... Done !!!\n",__func__);

    return status;
}

int32_t appIpcDeInit()
{
    int32_t status = 0;

    app_ipc_obj_t *obj = &g_app_ipc_obj;

    printf("IPC: Deinit ... !!!\n");

    appIpcDeleteRpmsgRxTask(obj);

    RPMessage_destruct(&obj->rpmsg_rx_handle);

    munmap_device_memory(obj->prm.tiovx_log_rt_mem, obj->prm.tiovx_log_rt_mem_size);
    munmap_device_memory(obj->prm.tiovx_obj_desc_mem, obj->prm.tiovx_obj_desc_mem_size);
    munmap_device_memory(obj->spin_lock_ptr, APP_IPC_HW_SPIN_LOCK_MMR_SIZE);

    printf("IPC: Deinit ... Done !!!\n");

    return status;
}

int32_t appIpcRegisterNotifyHandler(app_ipc_notify_handler_f handler)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    obj->ipc_notify_handler = handler;

    return status;
}

int32_t appIpcSendNotifyPort(uint32_t dest_cpu_id, uint32_t payload, uint32_t port_id)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if(dest_cpu_id<APP_IPC_CPU_MAX)
    {
        uint32_t ipc_cpu_id = g_app_to_ipc_cpu_id[dest_cpu_id];

        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: TX: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
            Ipc_mpGetSelfName(),
            obj->tiovx_rpmsg_rx_endpt,
            Ipc_mpGetName(ipc_cpu_id),
            port_id,
            payload);
        #endif

        status = RPMessage_send(
                    &payload,
                    (uint16_t)sizeof(payload),
                    (uint16_t)ipc_cpu_id,
                    (uint16_t)port_id,    /* dst end pt */
                    (uint16_t)obj->prm.tiovx_rpmsg_port_id, /* src endpt */
                    SystemP_WAIT_FOREVER
                    );
        if(status!=0)
        {
            appLogPrintf("IPC: TX: FAILED: %s (port %d) -> %s (port %d) msg = 0x%08x\n",
                Ipc_mpGetSelfName(),
                obj->prm.tiovx_rpmsg_port_id,
                Ipc_mpGetName((uint16_t)ipc_cpu_id),
                port_id,
                payload);
        }
    }

    return status;
}

int32_t appIpcSendNotify(uint32_t dest_cpu_id, uint32_t payload)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if(dest_cpu_id<APP_IPC_CPU_MAX)
    {
        status = appIpcSendNotifyPort(dest_cpu_id, payload,
            (uint32_t)obj->prm.tiovx_rpmsg_port_id);
    }

    return status;
}

uint32_t appIpcGetSelfCpuId()
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    return obj->prm.self_cpu_id;
}

uint32_t appIpcGetHostPortId(uint16_t cpu_id)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    return obj->tiovx_rpmsg_rx_endpt;
}

uint32_t appIpcIsCpuEnabled(uint32_t cpu_id)
{
    uint32_t is_enabled = 0, cur_cpu_id;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if(cpu_id>=APP_IPC_CPU_MAX)
    {
        is_enabled = 0;
    }
    for(cur_cpu_id=0; cur_cpu_id<obj->prm.num_cpus; cur_cpu_id++)
    {
        if(cpu_id==obj->prm.enabled_cpu_id_list[cur_cpu_id])
        {
            is_enabled = 1;
            break;
        }
    }
    return is_enabled;
}

int32_t appIpcGetTiovxObjDescSharedMemInfo(void **addr, uint32_t *size)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM05
<justification end> */
    if(obj->prm.tiovx_obj_desc_mem==NULL||obj->prm.tiovx_obj_desc_mem_size==0)
    {
        printf("%s: Error, obj_desc_mem not set\n",__func__);
        *addr = NULL;
        *size = 0;
        status = -1;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM05
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        *addr = obj->prm.tiovx_obj_desc_mem;
        *size = obj->prm.tiovx_obj_desc_mem_size;
    }

    return status;
}

void appIpcGetTiovxLogRtSharedMemInfo(void **shm_base, uint32_t *shm_size)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM06
<justification end> */
    if(obj->prm.tiovx_log_rt_mem==NULL||obj->prm.tiovx_log_rt_mem_size==0)
    {
        *shm_base = NULL;
        *shm_size = 0;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM06
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        *shm_base = obj->prm.tiovx_log_rt_mem;
        *shm_size = obj->prm.tiovx_log_rt_mem_size;
    }
}

int32_t appIpcHwLockAcquire(uint32_t hw_lock_id, uint32_t timeout)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if( hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX)
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM19
<justification end> */
        if(obj->spin_lock_ptr != NULL)
/* LDRA_JUSTIFY_END */
        {
            volatile uint32_t *reg_addr;

            reg_addr = (obj->spin_lock_ptr +
                    APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id));

            /* spin until lock is free */
            while(*reg_addr == 1u )
            {
                sched_yield();
                /* keep spinning */
            }
            status = 0;
        }
    }

    return status;
}

int32_t appIpcHwLockRelease(uint32_t hw_lock_id)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if(hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX)
    {
        volatile uint32_t *reg_addr;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM07
<justification end> */
        if(obj->spin_lock_ptr != NULL)
/* LDRA_JUSTIFY_END */
        {
            reg_addr = (obj->spin_lock_ptr +
                                APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id));

            *reg_addr = 0; /* free the lock */

            status = 0;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_QNX_UM07
<justification end> */
        else
        {
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

uint32_t appIpcGetIpcCpuId(uint32_t app_cpu_id)
{
    uint32_t ipc_cpu_id = APP_MCUSDK_IPC_CPU_INVALID;
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        ipc_cpu_id = g_app_to_ipc_cpu_id[app_cpu_id];
    }
    return ipc_cpu_id;
}


uint32_t appIpcGetAppCpuId(char *name)
{
    uint32_t ipc_cpu_id;
    uint32_t app_cpu_id = APP_IPC_CPU_INVALID;

    ipc_cpu_id = SOC_getCoreId(name);
    if(ipc_cpu_id < CORE_ID_MAX)
    {
        app_cpu_id = g_ipc_to_app_cpu_id[ipc_cpu_id];
    }
    return app_cpu_id;
}

const char *appIpcGetCpuName(uint32_t app_cpu_id)
{
    const char *name = "invalid";
    if(app_cpu_id < APP_IPC_CPU_MAX)
    {
        name = (char*)Ipc_mpGetName(g_app_to_ipc_cpu_id[app_cpu_id]);
    }
    return name;
}
