/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <ti_rpmsg_char.h>
#include <utils/ipc/src/app_ipc_linux_priv.h>

#if defined (SOC_J721E)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    3,
    1,
    4,
    2,
    5,
    6,
    7,
    8
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "mcu1_1",
    "mcu2_0",
    "mcu2_1",
    "mcu3_0",
    "mcu3_1",
    "c6x_1",
    "c6x_2",
    "c7x_1"
};
#endif

#if defined (SOC_J721S2)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    3,
    1,
    4,
    2,
    5,
    6,
    7
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "mcu1_1",
    "mcu2_0",
    "mcu2_1",
    "mcu3_0",
    "mcu3_1",
    "c7x_1",
    "c7x_2"
};
#endif

#if defined (SOC_J784S4)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    3,
    1,
    4,
    2,
    5,
    6,
    7,
    8,
    9,
    10,
    11
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "mcu1_1",
    "mcu2_0",
    "mcu2_1",
    "mcu3_0",
    "mcu3_1",
    "mcu4_0",
    "mcu4_1",
    "c7x_1",
    "c7x_2",
    "c7x_3",
    "c7x_4"
};
#endif

#if defined (SOC_J742S2)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    3,
    1,
    4,
    2,
    5,
    6,
    7,
    8,
    9,
    10
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "mcu1_1",
    "mcu2_0",
    "mcu2_1",
    "mcu3_0",
    "mcu3_1",
    "mcu4_0",
    "mcu4_1",
    "c7x_1",
    "c7x_2",
    "c7x_3"
};
#endif

#if defined (SOC_AM62A)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    1
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "c7x_1"
};
#endif

#if defined (SOC_J722S)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    1,
    2,
    3,
    4
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu1_0",
    "mcu2_0",
    "c7x_1",
    "c7x_2",
    "mcu_r5"
};
#endif

#if defined (SOC_TDA54)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    (uint32_t)(-1),
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
};

static const char *g_app_cpu_id_name[APP_IPC_CPU_MAX] =
{
    "mpu1_0",
    "mcu0",
    "mcu1",
    "mcu2",
    "mcu3",
    "mcu4",
    "rmcu0_0",
    "rmcu0_1",
    "rmcu1_0",
    "rmcu1_1",
    "rmcu2_0",
    "rmcu2_1",
    "c7x_1",
    "c7x_2",
    "c7x_3",
    "c7x_4",
};
#endif

app_ipc_obj_t g_app_ipc_obj;

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
}

int32_t appIpcInit(app_ipc_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id = 0;
    uint32_t i;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    appLogPrintf("IPC: Init ... !!!\n");

    obj->hw_spin_lock_addr = NULL;
    obj->ipc_notify_handler = NULL;
    obj->prm = *prm;
    obj->unblockfd = -1;
    for(i=0; i<APP_IPC_CPU_MAX; i++)
    {
        obj->tx_fds[i] = -1;
        obj->local_endpt[i] = (uint32_t)RPMSG_ADDR_ANY;
        obj->rcdev[i] = NULL;
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM01
<justification end> */
    if(prm->num_cpus>APP_IPC_CPU_MAX)
    {
        appLogPrintf("IPC: ERROR: Invalid number of CPUs !!!\n");
        status = -1;
    }
    if( (prm->tiovx_obj_desc_mem==NULL) || (prm->tiovx_obj_desc_mem_size==0u) )
    {
        appLogPrintf("IPC: ERROR: Invalid tiovx obj desc memory address or size !!!\n");
        status = -1;
    }
    if(prm->self_cpu_id>=APP_IPC_CPU_MAX)
    {
        appLogPrintf("IPC: ERROR: Invalid self cpu id !!!\n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR001
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        for(cpu_id=0; cpu_id<prm->num_cpus; cpu_id++)
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM02
<justification end> */
            if(prm->enabled_cpu_id_list[cpu_id]>APP_IPC_CPU_MAX)
            {
                appLogPrintf("IPC: ERROR: Invalid cpu id in enabled_cpu_id_list @ index %d !!!\n", cpu_id);
                status = -1;
            }
/* LDRA_JUSTIFY_END */
        }
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR002
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        obj->prm.tiovx_obj_desc_mem = appMemMap(prm->tiovx_obj_desc_mem, prm->tiovx_obj_desc_mem_size);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM03
<justification end> */
        if(obj->prm.tiovx_obj_desc_mem==NULL)
        {
            appLogPrintf("IPC: ERROR: Unable to mmap tiovx obj desc memory (%p of %d bytes) !!!\n", prm->tiovx_obj_desc_mem, prm->tiovx_obj_desc_mem_size);
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR003
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        obj->prm.tiovx_log_rt_mem = appMemMap(prm->tiovx_log_rt_mem, prm->tiovx_log_rt_mem_size);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM04
<justification end> */
        if(obj->prm.tiovx_log_rt_mem==NULL)
        {
            appLogPrintf("IPC: ERROR: Unable to mmap tiovx log rt memory (%p of %d bytes) !!!\n", prm->tiovx_log_rt_mem, prm->tiovx_log_rt_mem_size);
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR004
<justification end> */
    if(status==0)
    {
        status = appIpcHwLockInit();
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR005
<justification end> */
    if(status==0)
    {
        status = rpmsg_char_init(NULL);
    }
/* LDRA_JUSTIFY_END */
    /* status is a negative value for failures, and may be a positive value if rpmsg_char_init has previously been called */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR006
<justification end> */
    if(status>=0)
/* LDRA_JUSTIFY_END */
    {
        for(i=0; i<APP_IPC_CPU_MAX; i++)
        {
            if((1u==appIpcIsCpuEnabled(i)) && (i != appIpcGetSelfCpuId()))
            {
                uint32_t host_port_id = (uint32_t)RPMSG_ADDR_ANY;
                char eptdev_name[32] = { 0 };

                (void)snprintf(eptdev_name,sizeof(eptdev_name)-(1u),"rpmsg-char-%d-%d-ipc", i, getpid());

                obj->tx_fds[i] = appIpcCreateTxCh(i, prm->tiovx_rpmsg_port_id, &host_port_id, &obj->rcdev[i], eptdev_name);

                obj->local_endpt[i] = host_port_id;

                #ifdef APP_IPC_DEBUG
                printf("IPC: %6s : rpmsg local endpt is %d\n", appIpcGetCpuName(i), obj->local_endpt[i]);
                #endif

            }
        }
        for(i=0; i<APP_IPC_CPU_MAX; i++)
        {
            if((1u==appIpcIsCpuEnabled(i)) && (i != appIpcGetSelfCpuId()))
            {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM05
<justification end> */
                if(obj->tx_fds[i] < 0)
                {
                    appLogPrintf("IPC: ERROR: Unable to create TX channels for CPU [%s] !!!\n",
                        appIpcGetCpuName(i));
                    status = -1;
                }
/* LDRA_JUSTIFY_END */
            }
        }
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_IPC_LINUX_UBR007
<justification end> */
    if(status>=0)
    {
        status = appIpcCreateRpmsgRxTask(obj);
    }
/* LDRA_JUSTIFY_END */
    appLogPrintf("IPC: Init ... Done !!!\n");
    return status;
}

int32_t appIpcDeInit(void)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;
    int32_t status = 0;
    int32_t temp_status = 0;
    uint32_t i;

    appLogPrintf("IPC: Deinit ... !!!\n");

    (void)appIpcDeleteRpmsgRxTask(obj);

    for(i=0; i<APP_IPC_CPU_MAX; i++)
    {
        if((1u==appIpcIsCpuEnabled(i)) && (i != appIpcGetSelfCpuId()))
        {
            temp_status = appIpcDeleteCh(obj->rcdev[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM06
<justification end> */
            if ((int32_t)0 != temp_status)
            {
                status = temp_status;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    rpmsg_char_exit();

    appLogPrintf("IPC: DeInit ... Done !!!\n");
    return status;
}

int32_t appIpcRegisterNotifyHandler(app_ipc_notify_handler_f handler)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    obj->ipc_notify_handler = handler;

    return status;
}

uint32_t appIpcGetSelfCpuId(void)
{
    return APP_IPC_CPU_MPU1_0;
}

uint32_t appIpcGetHostPortId(uint16_t cpu_id)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;
    uint32_t host_port_id = 0;

    if(cpu_id < APP_IPC_CPU_MAX)
    {
        host_port_id = obj->local_endpt[cpu_id];
    }
    return host_port_id;
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
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM07
<justification end> */
    if( (obj->prm.tiovx_obj_desc_mem==NULL) || (obj->prm.tiovx_obj_desc_mem_size==0u) )
    {
        *addr = NULL;
        *size = 0;
        status = -1;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM07
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
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM08
<justification end> */
    if( (obj->prm.tiovx_log_rt_mem==NULL) || (obj->prm.tiovx_log_rt_mem_size==0u) )
    {
        *shm_base = NULL;
        *shm_size = 0;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_LINUX_UM08
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        *shm_base = obj->prm.tiovx_log_rt_mem;
        *shm_size = obj->prm.tiovx_log_rt_mem_size;
    }
}

uint32_t appIpcGetIpcCpuId(uint32_t app_cpu_id)
{
    uint32_t ipc_cpu_id = (uint32_t)-1;

    if(app_cpu_id<APP_IPC_CPU_MAX)
    {
        ipc_cpu_id = g_app_to_ipc_cpu_id[app_cpu_id];
    }
    return ipc_cpu_id;
}

uint32_t appIpcGetAppCpuId(char *name)
{
    uint32_t app_cpu_id = APP_IPC_CPU_INVALID;
    uint32_t i;

    for(i=0; i<APP_IPC_CPU_MAX; i++)
    {
        if(strcmp(name, g_app_cpu_id_name[i])==0)
        {
            app_cpu_id = i;
            break;
        }
    }
    return app_cpu_id;
}

const char *appIpcGetCpuName(uint32_t app_cpu_id)
{
    const char *name = "invalid";
    if(app_cpu_id<APP_IPC_CPU_MAX)
    {
        name = g_app_cpu_id_name[app_cpu_id];
    }
    return name;
}
