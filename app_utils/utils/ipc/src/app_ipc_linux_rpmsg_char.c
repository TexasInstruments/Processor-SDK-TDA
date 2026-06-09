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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <ti_rpmsg_char.h>
#include <utils/ipc/src/app_ipc_linux_priv.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*INTERNAL FUNCTION DEFINITIONS*/
static void *appIpcRpmsgRxTaskMain(void *arg);

static uint32_t map_vision_apps_cpu_id_to_rpmsg_char_cpu_id(uint32_t cpu_id)
{
    uint32_t rpmsg_char_id = (uint32_t)RPROC_ID_MAX;

    #if defined(SOC_FAMILY_TDA5)
    if (APP_IPC_CPU_MCU0_M55 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)M55_MAIN_0;
    }
    else if (APP_IPC_CPU_MCU1_M55 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)M55_MAIN_1;
    }
    else if (APP_IPC_CPU_MCU2_M55 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)M55_MAIN_2;
    }
    else if (APP_IPC_CPU_MCU3_M55 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)M55_MAIN_3;
    }
    else if (APP_IPC_CPU_MCU4_M55 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)M55_MAIN_4;
    }
    else if (APP_IPC_CPU_RMCU0_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU0_0;
    }
    else if (APP_IPC_CPU_RMCU0_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU0_1;
    }
    else if (APP_IPC_CPU_RMCU1_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU1_0;
    }
    else if (APP_IPC_CPU_RMCU1_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU1_1;
    }
    else if (APP_IPC_CPU_RMCU2_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU2_0;
    }
    else if (APP_IPC_CPU_RMCU2_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R52_MCU2_1;
    }
    else if (APP_IPC_CPU_C7x_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C76_0;
    }
    else if (APP_IPC_CPU_C7x_2 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C76_1;
    }
    else if (APP_IPC_CPU_C7x_3 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C76_2;
    }
    else if (APP_IPC_CPU_C7x_4 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C76_3;
    }
    #elif defined(SOC_FAMILY_AM)
    if (APP_IPC_CPU_MCU1_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_WKUP0_0;
    }
    else if (APP_IPC_CPU_C7x_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C71_0;
    }
    #elif defined(SOC_FAMILY_J7)
    if (APP_IPC_CPU_MCU1_0 == cpu_id)
    {
        #ifdef SOC_J722S
        rpmsg_char_id = (uint32_t)R5F_WKUP0_0;
        #else
        rpmsg_char_id = (uint32_t)R5F_MCU0_0;
        #endif
    }
    #ifndef SOC_J722S
    else if (APP_IPC_CPU_MCU1_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MCU0_1;
    }
    #endif
    else if (APP_IPC_CPU_MCU2_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN0_0;
    }
    #ifndef SOC_J722S
    else if (APP_IPC_CPU_MCU2_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN0_1;
    }
    else if (APP_IPC_CPU_MCU3_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN1_0;
    }
    else if (APP_IPC_CPU_MCU3_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN1_1;
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if (APP_IPC_CPU_MCU4_0 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN2_0;
    }
    else if (APP_IPC_CPU_MCU4_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)R5F_MAIN2_1;
    }
    #endif
    #endif
    #ifdef SOC_J721E
    else if (APP_IPC_CPU_C6x_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C66_0;
    }
    else if (APP_IPC_CPU_C6x_2 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C66_1;
    }
    #endif
    else if (APP_IPC_CPU_C7x_1 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C71_0;
    }
    #if defined (SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    else if (APP_IPC_CPU_C7x_2 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C71_1;
    }
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if (APP_IPC_CPU_C7x_3 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C71_2;
    }
    #endif
    #if defined(SOC_J784S4)
    else if (APP_IPC_CPU_C7x_4 == cpu_id)
    {
        rpmsg_char_id = (uint32_t)DSP_C71_3;
    }
    #endif
    #endif
    else
    {
        appLogPrintf("RTOS: Semaphore creation failed\n");
    }
    return rpmsg_char_id;
}

static void appIpcRpmsgRxHandler(uint32_t app_cpu_id, uint32_t payload)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR001
<justification end> */
    if(app_cpu_id<APP_IPC_CPU_MAX)
/* LDRA_JUSTIFY_END */
    {
        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: RX: %s -> %s (port %d) msg = 0x%08x\n",
            appIpcGetCpuName(app_cpu_id),
            appIpcGetCpuName(appIpcGetSelfCpuId()),
            (uint32_t)obj->local_endpt[app_cpu_id],
            payload);
        #endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM01
<justification end> */
        if((payload & 0xFFFF0000U) == 0xDEAD0000U)
        {
            /* echo message dont send to handler */
            appLogPrintf("IPC: RX: %s -> %s (port %d) msg = 0x%08x\n",
                appIpcGetCpuName(app_cpu_id),
                appIpcGetCpuName(appIpcGetSelfCpuId()),
                (uint32_t)obj->prm.tiovx_rpmsg_port_id,
                payload);
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM01
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR002
<justification end> */
            if(obj->ipc_notify_handler != NULL)
            {
                obj->ipc_notify_handler(app_cpu_id, payload);
            }
/* LDRA_JUSTIFY_END */
        }
    }
}

static void *appIpcRpmsgRxTaskMain(void *arg)
{
    app_ipc_obj_t *obj = &g_app_ipc_obj;
    uint32_t payload;
    uint32_t maxfd;
    uint32_t nfds;
    int32_t status = 0;
    fd_set rfds;
    uint32_t i;
    bool done;

    done = (bool)0;
    while(!done)
    {
        maxfd  = 0;

        FD_ZERO(&rfds);

        /* Initialize rx fds to wait on messages  */
        for(i = 0; i< APP_IPC_CPU_MAX; i++)
        {
            if((1u==appIpcIsCpuEnabled(i)) && (i != appIpcGetSelfCpuId()) )
            {
                /* same file handle used for RX and TX */
                maxfd = (uint32_t)MAX((int64_t)maxfd, obj->tx_fds[i]);
                FD_SET(obj->tx_fds[i], &rfds);
            }
        }

        /* add fd to unblock from select and break from loop on exit */
        FD_SET(obj->unblockfd, &rfds);

        /* Add one to last fd created, this is mandated by select() */
        nfds = (uint32_t)MAX((int64_t)maxfd, obj->unblockfd) + 1u;

        status = select((int32_t)nfds, &rfds, NULL, NULL, NULL);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR003
<justification end> */
        if (status > 0)
/* LDRA_JUSTIFY_END */
        {
            if (FD_ISSET(obj->unblockfd, &rfds))
            {
                /*
                 * Event was signalled to break the loop
                 *
                 * This is typically done during a shutdown sequence, where
                 * the intention of the client would be to ignore (i.e. not fetch)
                 * any pending messages in the transport's queue.
                 * Thus, we shall not check for nor return any messages.
                 */
                done = (bool)1;
            }
            else
            {

                /* Process all messages received on different Rx sockets */
                for(i = 0; i< APP_IPC_CPU_MAX; i++)
                {
                    if((1u==appIpcIsCpuEnabled(i))
                        && (i != appIpcGetSelfCpuId())
                        &&  FD_ISSET(obj->tx_fds[i], &rfds)
                        )
                    {
                        /* same file handle used for RX and TX */
                        status = (int32_t)read(
                                    obj->tx_fds[i],
                                    &payload, sizeof(uint32_t));
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR004
<justification end> */
                        if(status > 0)
                        {
                            appIpcRpmsgRxHandler(
                                        i,
                                        payload);
                        }
/* LDRA_JUSTIFY_END */
                    }
                }
            }
        }
    }   /* while(! done) */
    return NULL;
}

int32_t appIpcSendNotifyPort(uint32_t dest_cpu_id, uint32_t payload, uint32_t port_id)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    /* NOTE: port_id is unsed in this function */

    if( (dest_cpu_id<APP_IPC_CPU_MAX) && (obj->tx_fds[dest_cpu_id] > 0))
    {
        #ifdef APP_IPC_DEBUG
        appLogPrintf("IPC: TX: %s -> %s (port %d) msg = 0x%08x\n",
            appIpcGetCpuName(appIpcGetSelfCpuId()),
            appIpcGetCpuName(dest_cpu_id),
            (uint32_t)port_id,
            payload);
        #endif

        status = (int32_t)write(obj->tx_fds[dest_cpu_id], &payload, sizeof(payload));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM02
<justification end> */
        if( (status < 0) || (status != (int32_t)sizeof(payload)))
        {
            appLogPrintf("IPC: TX: FAILED: %s -> %s (port %d) msg = 0x%08x\n",
                appIpcGetCpuName(appIpcGetSelfCpuId()),
                appIpcGetCpuName(dest_cpu_id),
                (uint32_t)port_id,
                payload);
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM02
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            status = 0;
        }
    }

    return status;
}

int32_t appIpcSendNotify(uint32_t dest_cpu_id, uint32_t payload)
{
    int32_t status = -1;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    if( (dest_cpu_id<APP_IPC_CPU_MAX) && (obj->tx_fds[dest_cpu_id] > 0))
    {
        status = appIpcSendNotifyPort(dest_cpu_id, payload,
            (uint32_t)obj->prm.tiovx_rpmsg_port_id);
    }

    return status;
}

int32_t appIpcCreateTxCh(uint32_t remote_app_cpu_id, uint32_t remote_endpt, uint32_t *local_endpt, rpmsg_char_dev_t **rcdev, char *eptdev_name)
{
    int fd = -1;
    rpmsg_char_dev_t rcdev_local;
    uint32_t ipc_rproc_id;

    ipc_rproc_id = map_vision_apps_cpu_id_to_rpmsg_char_cpu_id(remote_app_cpu_id);

    *rcdev = rpmsg_char_open((enum rproc_id) ipc_rproc_id ,
                NULL,
                (uint32_t)RPMSG_ADDR_ANY,
                remote_endpt,
                eptdev_name,
                0
    );

    if (NULL != *rcdev)
    {
        rcdev_local = **rcdev;
        *local_endpt = (uint32_t)rcdev_local.endpt;

        fd = rcdev_local.fd;
    }

    return fd;
}

int32_t appIpcDeleteCh(rpmsg_char_dev_t *rcdev)
{
    int32_t status = 0;
    status = rpmsg_char_close(rcdev);
    if ((int32_t)0 != status)
    {
        appLogPrintf("IPC: ERROR: Failed to close created endpoint !!!\n");
    }

    return status;
}

static void appIpcUnblockRpmsgTask(app_ipc_obj_t *obj)
{
    uint64_t     buf = 1;

    /* Write 8 bytes to shutdown */
    (void)write(obj->unblockfd, &buf, sizeof(buf));
}

int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj)
{
    pthread_attr_t thread_attr;
    int32_t status = 0;
    uint32_t temp_status = 0;
    obj->unblockfd = eventfd(0, 0);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM03
<justification end> */
    if(obj->unblockfd < 0)
    {
        status = -1;
        appLogPrintf("IPC: ERROR: Unable to create unblock event !!!\n");
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR005
<justification end> */
    if(status==0)
/* LDRA_JUSTIFY_END */
    {
        temp_status = (uint32_t)status;
        temp_status |= (uint32_t)pthread_attr_init(&thread_attr);
        status = (int32_t)temp_status;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM04
<justification end> */
        if(status!=0)
        {
            appLogPrintf("IPC: ERROR: Unable to set thread attr !!!\n");
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RPMSG_CHAR_UBR006
<justification end> */
        if(status==0)
        {
            temp_status = (uint32_t)status;
            temp_status |= (uint32_t)pthread_create(&obj->task, &thread_attr, appIpcRpmsgRxTaskMain, obj);
            status = (int32_t)temp_status;
        }
/* LDRA_JUSTIFY_END */
        (void)pthread_attr_destroy(&thread_attr);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_RPMSG_CHAR_UM05
<justification end> */
    if(status!=0)
    {
        appLogPrintf("IPC: ERROR: Unable to create RX thread !!!\n");
    }
/* LDRA_JUSTIFY_END */
    return status;
}

int32_t appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj)
{
    void *task_status;

    appIpcUnblockRpmsgTask(obj);
    (void)pthread_join(obj->task, &task_status);
    (void)close(obj->unblockfd);

    return 0;
}
