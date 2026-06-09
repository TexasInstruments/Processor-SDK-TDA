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

#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "app_remote_service_linux_priv.h"

/* #define APP_REMOTE_SERVICE_DEBUG */

#if !defined (SOC_AM62A)
#define APP_REMOTE_SERVICE_HANDLERS_MAX  (  10u)
#else
#define APP_REMOTE_SERVICE_HANDLERS_MAX  (  8u)
#endif

typedef struct {

    app_remote_service_init_prms_t prm;
    uint8_t rpmsg_tx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    uint8_t rpmsg_rx_msg_buf[IPC_RPMESSAGE_MSG_SIZE] __attribute__ ((aligned(1024)));
    pthread_mutex_t tx_lock;
    pthread_mutex_t rx_lock;

    int tx_fds[APP_IPC_CPU_MAX];
    uint32_t tx_local_endpt[APP_IPC_CPU_MAX];

    int rx_fds[APP_IPC_CPU_MAX];
    uint32_t rx_local_endpt[APP_IPC_CPU_MAX];

    rpmsg_char_dev_t *rcdev[APP_IPC_CPU_MAX];

    app_remote_service_handler_t handlers[APP_REMOTE_SERVICE_HANDLERS_MAX];
    char service_name[APP_REMOTE_SERVICE_HANDLERS_MAX][APP_REMOTE_SERVICE_NAME_MAX];

    pthread_t task;
    int unblockfd;
} app_remote_service_obj_t;

app_remote_service_obj_t g_app_remote_service_obj;

static int32_t appRemoteServiceRunHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i, is_found = 0;

    pthread_mutex_lock(&obj->rx_lock);
    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(obj->handlers[i]!=NULL && (strcmp(obj->service_name[i], service_name)==0))
        {
            is_found = 1;
            status = obj->handlers[i](service_name, cmd, prm, prm_size, flags);
            break;
        }
    }
    pthread_mutex_unlock(&obj->rx_lock);
    if(!is_found)
    {
        printf("REMOTE_SERVICE: ERROR: Unable to find handler for service [%s]\n", service_name);
    }
    return status;
}

int32_t appRemoteServiceRun(uint32_t dst_app_cpu_id, const char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = 0;

    if(prm_size > APP_REMOTE_SERVICE_PRM_SIZE_MAX)
    {
        printf("REMOTE_SERVICE: ERROR: Parameter size of %d bytes exceeds message buffer size of %d bytes\n",
                prm_size, (uint32_t)APP_REMOTE_SERVICE_PRM_SIZE_MAX);
        status = -1;
    }
    if(!appIpcIsCpuEnabled(dst_app_cpu_id))
    {
        printf("REMOTE_SERVICE: ERROR: CPU %d is not enabled or invalid CPU ID\n", dst_app_cpu_id );
        status = -1;
    }
    if(status==0)
    {
        if(dst_app_cpu_id == appIpcGetSelfCpuId())
        {
            /* destination CPU is self CPU so call the handler locally */
            status = appRemoteServiceRunHandler((char *)service_name, cmd, prm, prm_size, flags);
        }
        else
        {
            uint16_t tx_payload_size;
            app_service_msg_header_t *header;

            if(prm==NULL)
                prm_size = 0;

            /* take a lock since to make this call thread safe */
            pthread_mutex_lock(&obj->tx_lock);

            /* copy content to temp buffer */
            header = (app_service_msg_header_t *)&obj->rpmsg_tx_msg_buf[0];
            strncpy((char*)header->service_name, service_name, APP_REMOTE_SERVICE_NAME_MAX);
            header->service_name[APP_REMOTE_SERVICE_NAME_MAX-1]=0;
            header->cmd = cmd;
            header->flags = flags;
            header->status = status;
            header->prm_size = prm_size;
            if(prm!=NULL)
            {
                memcpy(&obj->rpmsg_tx_msg_buf[sizeof(app_service_msg_header_t)], prm  , prm_size);
            }
            tx_payload_size = prm_size + sizeof(app_service_msg_header_t);

            #ifdef APP_REMOTE_SERVICE_DEBUG
            printf("REMOTE_SERVICE: TX: %s -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ...\n",
                appIpcGetCpuName(appIpcGetSelfCpuId()),
                appIpcGetCpuName(dst_app_cpu_id),
                obj->prm.rpmsg_rx_endpt,
                cmd, prm_size);
            #endif

            status = write(obj->tx_fds[dst_app_cpu_id], obj->rpmsg_tx_msg_buf, tx_payload_size);
            if(status < 0 || status != tx_payload_size)
            {
                printf("REMOTE_SERVICE: TX: FAILED: %s -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes\n",
                    appIpcGetCpuName(appIpcGetSelfCpuId()),
                    appIpcGetCpuName(dst_app_cpu_id),
                    obj->prm.rpmsg_rx_endpt,
                    cmd, prm_size);
            }

            if(status > 0)
            {
                if(flags & APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK)
                {
                    /* No need to wait for ACK */
                }
                else
                {
                    /* wait for ACK from destination */

                    /* same file handle used for RX and TX */
                    status = read(
                                obj->tx_fds[dst_app_cpu_id],
                                obj->rpmsg_tx_msg_buf, tx_payload_size);

                    if(status < 0 || status != tx_payload_size)
                    {
                        printf("REMOTE_SERVICE: RX: %s -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... Failed !!!\n",
                            appIpcGetCpuName(dst_app_cpu_id),
                            appIpcGetCpuName(appIpcGetSelfCpuId()),
                            obj->prm.rpmsg_rx_endpt,
                            cmd, prm_size);
                    }
                    else
                    {
                        status = header->status;
                        if(prm!=NULL)
                        {
                            memcpy(prm, &obj->rpmsg_tx_msg_buf[sizeof(app_service_msg_header_t)], prm_size);
                        }

                        #ifdef APP_REMOTE_SERVICE_DEBUG
                        printf("REMOTE_SERVICE: RX: %s (port %d) -> %s (port %d) cmd = 0x%08x, prm_size = %d bytes ... Done !!!\n",
                            appIpcGetCpuName(dst_app_cpu_id),
                            obj->prm.rpmsg_rx_endpt,
                            appIpcGetCpuName(appIpcGetSelfCpuId()),
                            obj->prm.rpmsg_rx_endpt,
                            cmd, prm_size);
                        #endif
                    }
                }
            }
            /* take a lock since to make this call thread safe */
            pthread_mutex_unlock(&obj->tx_lock);
        }
    }

    return status;
}

void appRemoteServiceInitSetDefault(app_remote_service_init_prms_t *prm)
{
    prm->rpmsg_rx_endpt = APP_IPC_REMOTE_SERVICE_RPMSG_PORT_ID;
}

void appRemoteServiceRpmsgRxHandler(app_remote_service_obj_t *obj, int cpu_id)
{
    app_service_msg_header_t *header;

    header = (app_service_msg_header_t *)&obj->rpmsg_rx_msg_buf[0];

    appRemoteServiceRunHandler((char *)header->service_name, header->cmd,
        &obj->rpmsg_rx_msg_buf[sizeof(app_service_msg_header_t)],
        header->prm_size, header->flags);
}

void *appRemoteServiceRpmsgRxTaskMain(void *arg)
{
    app_remote_service_obj_t *obj = (app_remote_service_obj_t *)arg;
    uint32_t maxfd;
    uint32_t nfds;
    int32_t status;
    fd_set rfds;
    uint32_t i;
    uint32_t done;

    done = 0;
    while(!done)
    {
        maxfd  = 0;
        status = 0;

        FD_ZERO(&rfds);

        /* Initialize rx fds to wait on messages  */
        for(i = 0; i< APP_IPC_CPU_MAX; i++)
        {
            if(appIpcIsCpuEnabled(i) && i != appIpcGetSelfCpuId())
            {
                /* same file handle used for RX and TX */
                maxfd = MAX(maxfd, obj->rx_fds[i]);
                FD_SET(obj->rx_fds[i], &rfds);
            }
        }

        /* add fd to unblock from select and break from loop on exit */
        FD_SET(obj->unblockfd, &rfds);

        /* Add one to last fd created, this is mandated by select() */
        nfds = MAX(maxfd, obj->unblockfd) + 1;

        status = (int32_t)select(nfds, &rfds, NULL, NULL, NULL);

        if (status)
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
                done = 1;
            }
            else
            {

                /* Process all messages received on different Rx sockets */
                for(i = 0; i< APP_IPC_CPU_MAX; i++)
                {
                    if(     appIpcIsCpuEnabled(i)
                        &&  i != appIpcGetSelfCpuId()
                        &&  FD_ISSET(obj->rx_fds[i], &rfds)
                        )
                    {
                        /* same file handle used for RX and TX */
                        status = read(
                                    obj->rx_fds[i],
                                    &obj->rpmsg_rx_msg_buf[0],
                                    IPC_RPMESSAGE_MSG_SIZE);
                        if(status > 0)
                        {
                            appRemoteServiceRpmsgRxHandler(obj, i);
                        }
                    }
                }
            }
        }
    }   /* while(! done) */
    return NULL;
}


int32_t appRemoteServiceCreateRpmsgRxTask(app_remote_service_obj_t *obj)
{
    pthread_attr_t thread_attr;
    int32_t status = 0;

    obj->unblockfd = eventfd(0, 0);
    if(obj->unblockfd < 0)
    {
        status = -1;
        printf("IPC: ERROR: Unable to create unblock event !!!\n");
    }
    if(status==0)
    {
        status |= pthread_attr_init(&thread_attr);
        if(status!=0)
        {
            printf("IPC: ERROR: Unable to set thread attr !!!\n");
        }
        if(status==0)
        {
            status |= pthread_create(&obj->task, &thread_attr, (void*)appRemoteServiceRpmsgRxTaskMain, obj);
        }
        pthread_attr_destroy(&thread_attr);
    }
    if(status!=0)
    {
        printf("IPC: ERROR: Unable to create RX thread !!!\n");
    }
    return status;
}

int32_t appRemoteServiceDeleteRpmsgRxTask(app_remote_service_obj_t *obj)
{
    void        *task_status;
    uint64_t     buf = 1;

    /* Write 8 bytes to shutdown */
    write(obj->unblockfd, &buf, sizeof(buf));

    pthread_join(obj->task, &task_status);
    close(obj->unblockfd);

    return 0;
}

int32_t appRemoteServiceInit(app_remote_service_init_prms_t *prm)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    pthread_mutexattr_t mutex_attr;
    int32_t status = 0;
    uint32_t i;

    printf("REMOTE_SERVICE: Init ... !!!\n");

    obj->prm = *prm;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        obj->handlers[i] = NULL;
        obj->service_name[i][0] = 0;
    }

    status |= pthread_mutexattr_init(&mutex_attr);
    if(status==0)
    {
        status |= pthread_mutex_init(&obj->tx_lock, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }
    else
    {
        pthread_mutexattr_destroy(&mutex_attr);
    }
    if(status!=0)
    {
        printf("REMOTE_SERVICE: ERROR: Unable to create tx mutex !!!\n");
    }
    status |= pthread_mutexattr_init(&mutex_attr);
    if(status==0)
    {
        status |= pthread_mutex_init(&obj->rx_lock, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }
    else
    {
        pthread_mutexattr_destroy(&mutex_attr);
    }
    if(status!=0)
    {
        printf("REMOTE_SERVICE: ERROR: Unable to create rx mutex !!!\n");
    }

    if(status==0)
    {
        uint32_t i;

        for(i=0; i<APP_IPC_CPU_MAX; i++)
        {
            if(appIpcIsCpuEnabled(i) && i != appIpcGetSelfCpuId())
            {
                uint32_t host_port_id = RPMSG_ADDR_ANY;
                char eptdev_name[32] = { 0 };

                sprintf(eptdev_name, "rpmsg-char-%d-%d-rs-tx", i, getpid());

                obj->tx_fds[i] = appIpcCreateTxCh(i, prm->rpmsg_rx_endpt, &host_port_id, &obj->rcdev[i], eptdev_name);

                obj->tx_local_endpt[i] = host_port_id;
            }
        }
    }

    printf("REMOTE_SERVICE: Init ... Done !!!\n");

    return status;
}

int32_t appRemoteServiceDeInit()
{
    int32_t status = 0;
    uint32_t i;
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;

    printf("REMOTE_SERVICE: Deinit ... !!!\n");

    pthread_mutex_destroy(&obj->tx_lock);
    pthread_mutex_destroy(&obj->rx_lock);

    for(i=0; i<APP_IPC_CPU_MAX; i++)
    {
        if(appIpcIsCpuEnabled(i) && i != appIpcGetSelfCpuId())
        {
            appIpcDeleteCh(obj->rcdev[i]);
        }
    }

    printf("REMOTE_SERVICE: Deinit ... Done !!!\n");

    return status;
}

int32_t appRemoteServiceRegister(const char *service_name, app_remote_service_handler_t handler)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(obj->handlers[i]==NULL)
        {
            strncpy(obj->service_name[i], service_name, APP_REMOTE_SERVICE_NAME_MAX);
            obj->service_name[i][APP_REMOTE_SERVICE_NAME_MAX-1]=0;
            obj->handlers[i] = handler;
            status = 0;
            break;
        }
    }
    if(status!=0)
    {
        printf("REMOTE_SERVICE: ERROR: Unable to register handler for service [%s]\n", service_name);
    }

    return status;
}

int32_t appRemoteServiceUnRegister(const char *service_name)
{
    app_remote_service_obj_t *obj = &g_app_remote_service_obj;
    int32_t status = -1;
    uint32_t i;

    for(i=0; i<APP_REMOTE_SERVICE_HANDLERS_MAX; i++)
    {
        if(strcmp(service_name, obj->service_name[i])==0)
        {
            obj->service_name[i][0] = 0;
            obj->handlers[i] = NULL;
            status = 0;
            break;
        }
    }
    if(status!=0)
    {
        printf("REMOTE_SERVICE: ERROR: Unable to unregister handler for service [%s]\n", service_name);
    }
    return status;
}
