/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file lpm_ipc.c
 *
 *  \brief IPC  example code
 *  
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/lpm/include/lpm_ipc.h>
#include <ti/drv/lpm/src/lpm_ipc_rsctable.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define MSGSIZE                 256U
#define SERVICE_PING            "ti.ipc4.ping-pong"
#define ENDPT_PING              13U
#define SERVICE_CHRDEV          "rpmsg_chrdev"
#define ENDPT_CHRDEV            14U
#define NUMMSGS                 10000 /* number of message sent per task */

/* this should be >= RPMessage_getObjMemRequired() */
#define IPC_RPMESSAGE_OBJ_SIZE  256U

#define RPMSG_DATA_SIZE        (256U*512U + IPC_RPMESSAGE_OBJ_SIZE)
#define VQ_BUF_SIZE             2048U

/* Vring start address for each device */
#if defined (SOC_J7200)
#define VRING_BASE_ADDRESS      0xA4000000U
#else
#define VRING_BASE_ADDRESS      0xAA000000U
#endif

typedef struct Lpm_ipcTestParams_s
{
    uint32_t endPt;
    char     name[32];
} Lpm_ipcTestParams;

Lpm_ipcTestParams service_ping = { ENDPT_PING, SERVICE_PING };
Lpm_ipcTestParams service_chrdev = { ENDPT_CHRDEV, SERVICE_CHRDEV };

#if defined (SOC_J721E)
    #define CORE_IN_TEST 8
#elif defined (SOC_J7200)
    #define CORE_IN_TEST 3
#elif defined (SOC_J784S4)
    #define CORE_IN_TEST 11
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define IPC_SETUP_TASK_PRI                  (3)
/**< Priority for sender and receiver tasks */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


uint8_t g_taskStackBuf[(CORE_IN_TEST+3)*IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)));

uint8_t  gCntrlBuf[RPMSG_DATA_SIZE] __attribute__ ((section("ipc_data_buffer"), aligned (8)));
uint8_t  sysVqBuf[VQ_BUF_SIZE]  __attribute__ ((section ("ipc_data_buffer"), aligned (8)));
uint8_t  g_sendBuf[RPMSG_DATA_SIZE * CORE_IN_TEST]  __attribute__ ((section ("ipc_data_buffer"), aligned (8)));
uint8_t  g_rspBuf[RPMSG_DATA_SIZE * 2]  __attribute__ ((section ("ipc_data_buffer"), aligned (8)));

uint8_t *pCntrlBuf = gCntrlBuf;
uint8_t *pTaskBuf = g_taskStackBuf;
uint8_t *pSendTaskBuf = g_sendBuf;
uint8_t *pRecvTaskBuf = g_rspBuf;
uint8_t *pSysVqBuf = sysVqBuf;

uint32_t selfProcId = IPC_MCU1_0;
uint32_t remoteProc[] =
{
#if defined (SOC_J721E)
    IPC_MPU1_0, IPC_MCU2_0, IPC_MCU2_1, IPC_MCU3_0, IPC_MCU3_1, IPC_C66X_1, IPC_C66X_2, IPC_C7X_1
#elif defined (SOC_J7200)
    IPC_MPU1_0, IPC_MCU2_0, IPC_MCU2_1
#elif defined (SOC_J784S4)
    IPC_MPU1_0, IPC_MCU2_0, IPC_MCU2_1, IPC_MCU3_0, IPC_MCU3_1, IPC_MCU4_0, IPC_MCU4_1, IPC_C7X_1, IPC_C7X_2, IPC_C7X_3, IPC_C7X_4
#endif
};

uint32_t *pRemoteProcArray = remoteProc;
uint32_t  gNumRemoteProc = sizeof(remoteProc)/sizeof(uint32_t);

uint32_t gRecvTaskBufIdx = 0;
uint32_t gSendTaskBufIdx[IPC_MAX_PROCS] = {0};

uint32_t rpmsgDataSize = RPMSG_DATA_SIZE;

__attribute((section(".tracebuf"))) char Ipc_traceBuffer[IPC_TRACE_BUFFER_MAX_SIZE];

uint8_t gOcmcShadowRscTable[0x8C];
uint8_t *pOcmcShadowRscTable = gOcmcShadowRscTable;

#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
static uint32_t		RecvEndPt = 0;
RPMessage_Handle  g_ResponderHandleLinux;
#endif

//#define DEBUG_PRINT

volatile uint32_t g_exitRespTsk = 0x0U;
RPMessage_Handle  g_ResponderHandleRTOS;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_ipcExitResponseTask()
{
    g_exitRespTsk = 0x1U;
#ifdef DEBUG_PRINT
    UART_printf("Calling Ipc de inits!\n");
#endif
    RPMessage_delete(&g_ResponderHandleRTOS);
#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
    RPMessage_delete(&g_ResponderHandleLinux);
#endif
    RPMessage_deInit();
    Ipc_deinit();
    memcpy(pOcmcShadowRscTable, (const void*)0xa0100000, 0x8c);
    ((Ipc_ResourceTable *)pOcmcShadowRscTable)->rpmsg_vdev.status = 0x00;
#ifdef DEBUG_PRINT
    UART_printf("Ipc de inits done!\n");
#endif
    return;
}

/*
 * This "Task" waits for a "ping" message from any processor
 * then replies with a "pong" message.
 */
void Lpm_ipcResponderFxn(void *arg0, void *arg1)
{
    RPMessage_Params    params;
    RPMessage_Handle  g_ResponderHandle;
    uint32_t		myEndPt = 0;
    uint32_t		remoteEndPt;
    uint32_t		remoteProcId;
    uint16_t		len;
    int32_t		n;
    int32_t		status = 0;
    void		*buf;
    uint32_t            requestedEpt = (uint32_t)(*(uint32_t*)arg0);
    char *              name = (char *)arg1;

    uint32_t            bufSize = rpmsgDataSize;
    char                str[MSGSIZE];

#ifdef DEBUG_PRINT
    UART_printf("RecvTask: Task Created!\n");
#endif

    buf = &pRecvTaskBuf[gRecvTaskBufIdx++ * rpmsgDataSize];
    if(buf == NULL) 
    {
        UART_printf("RecvTask: buffer allocation failed\n");
        return;
    }

    RPMessageParams_init(&params);
    params.requestedEndpt = requestedEpt;
    params.buf = buf;
    params.bufSize = bufSize;

    g_ResponderHandle = RPMessage_create(&params, &myEndPt);
    if(!g_ResponderHandle) 
    {
        UART_printf("RecvTask: Failed to create endpoint\n");
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("RecvTask: Success to create endpoint\n");
#endif
    }

    if (requestedEpt == ENDPT_PING)
    {
        g_ResponderHandleRTOS = g_ResponderHandle;
    }
    else
    {
#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
        g_ResponderHandleLinux = g_ResponderHandle;
#endif
    }

#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
    if (requestedEpt == ENDPT_PING)
    {
        RecvEndPt = myEndPt;
    }
#endif

    status = RPMessage_announce(RPMESSAGE_ALL, myEndPt, name);
    if(status != IPC_SOK) 
    {
        UART_printf("RecvTask: RPMessage_announce() for %s failed\n", name);
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("RecvTask: RPMessage_announce() for %s success\n", name);
#endif
    }

    while(g_exitRespTsk == 0x0U)
    {
        status = RPMessage_recv(g_ResponderHandle, (Ptr)str, &len, &remoteEndPt, &remoteProcId,
                IPC_RPMESSAGE_TIMEOUT_FOREVER);
        if(status != IPC_SOK) 
        {
            UART_printf("RecvTask: failed with code %d\n", status);
        }
        else
        {
            /* NULL terminated string */
            str[len] = '\0';
#ifdef DEBUG_PRINT
            UART_printf("RecvTask: Revcvd msg \"%s\" len %d from %s\n",
                    str, len, Ipc_mpGetName(remoteProcId));
#endif
        }

        status = sscanf(str, "ping %d", &n);
        if(status == 1) 
        {
            memset(str, 0, MSGSIZE);
            len = snprintf(str, 255, "pong %d", n);
            if(len > 255)
            { 
                UART_printf("RecvTask: snprintf failed, len %d\n", len);
                len = 255;
            }
            str[len++] = '\0';
        }
        else
        {
            /* If this is not ping/pong message, just print the message */
            UART_printf("%s <--> %s : %s recvd\n",
                    Ipc_mpGetSelfName(),
                    Ipc_mpGetName(remoteProcId),
                    str);
        }
#ifdef DEBUG_PRINT
        UART_printf("RecvTask: Sending msg \"%s\" len %d from %s to %s\n",
                str, len, Ipc_mpGetSelfName(),
                Ipc_mpGetName(remoteProcId));
#endif
        status = RPMessage_send(g_ResponderHandle, remoteProcId, remoteEndPt, myEndPt, str, len);
        if (status != IPC_SOK) 
        {
            UART_printf("RecvTask: Sending msg \"%s\" len %d from %s to %s failed!!!\n",
                str, len, Ipc_mpGetSelfName(),
                Ipc_mpGetName(remoteProcId));
        }
    }

#ifdef DEBUG_PRINT
    UART_printf("RecvTask: %s responder task exiting ...\n",
                    Ipc_mpGetSelfName());
#endif
    return;
}

void Lpm_ipcSenderFxn(void *arg0, void *arg1)
{
    RPMessage_Handle    handle;
    RPMessage_Params    params;
    uint32_t            myEndPt = 0;
    uint32_t            remoteEndPt;
    uint32_t            remoteProcId;
    uint32_t            dstProc;
    uint16_t            len;
    int32_t             i;
    int32_t             status = 0;
    char                buf[256];   
    uint8_t            *buf1;

    uint32_t            cntPing = 0;
    uint32_t            cntPong = 0;

    buf1 = &pSendTaskBuf[rpmsgDataSize * (uint32_t)(*(uint32_t*)arg1)];
    dstProc = (uint32_t)(*(uint32_t*)arg0);

#ifdef DEBUG_PRINT
    UART_printf("SendTask%d: Task Created!\n",
                dstProc);
#endif

    /* Create the endpoint for receiving. */
    RPMessageParams_init(&params);
    params.numBufs = 2;
    params.buf = buf1;
    params.bufSize = rpmsgDataSize;
    handle = RPMessage_create(&params, &myEndPt);
    if(!handle) 
    {
        UART_printf("SendTask%d: Failed to create message endpoint\n",
                dstProc);
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("SendTask%d: Success to create message endpoint\n",
                dstProc);
#endif
    }

    status = RPMessage_getRemoteEndPt(dstProc, SERVICE_PING, &remoteProcId,
            &remoteEndPt, SemaphoreP_WAIT_FOREVER);
    if(dstProc != remoteProcId) 
    {
        UART_printf("SendTask%d: RPMessage_getRemoteEndPt() malfunctioned, status %d\n",
                dstProc, status);
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("SendTask%d: RPMessage_getRemoteEndPt() passed, status %d\n",
                dstProc, status);
#endif
    }

    for (i = 0; i < NUMMSGS; i++) 
    {
        /* Send data to remote endPt: */
        memset(buf, 0, 256);
        len = snprintf(buf, 255, "ping %d", i);
        if(len > 255)
        {
            UART_printf("SendTask%d: snprintf failed, len %d\n", dstProc, len);
            len = 255;
        }
        buf[len++] = '\0';

#ifdef DEBUG_PRINT
        UART_printf("SendTask%d: Sending \"%s\" from %s to %s...\n", dstProc,
                buf, Ipc_mpGetSelfName(),
                Ipc_mpGetName(dstProc));
#endif
        /* Increase the Ping Counter */
        cntPing++;

        status = RPMessage_send(handle, dstProc, ENDPT_PING, myEndPt, (Ptr)buf, len);
        if (status != IPC_SOK) 
        {
            UART_printf("SendTask%d: RPMessage_send Failed Msg-> \"%s\" from %s to %s...\n", 
                dstProc,
                buf, Ipc_mpGetSelfName(),
                Ipc_mpGetName(dstProc));
            break;
        }
        

        /* wait a for a response message: */
        status = RPMessage_recv(handle, (Ptr)buf, &len, &remoteEndPt,
                &remoteProcId, IPC_RPMESSAGE_TIMEOUT_FOREVER);

        if(status != IPC_SOK) 
        {
            UART_printf("SendTask%d: RPMessage_recv failed with code %d\n",
                    dstProc, status);
            break;
        }

        /* Make it NULL terminated string */
        if(len >= MSGSIZE)
        {
            buf[MSGSIZE-1] = '\0';
        }
        else
        {
            buf[len] = '\0';
        }

#ifdef DEBUG_PRINT
        UART_printf("SendTask%d: Received \"%s\" len %d from %s endPt %d \n",
                dstProc, buf, len, Ipc_mpGetName(remoteProcId),
                remoteEndPt);
#endif
        cntPong++;
        if((i+1)%50 == 0)
        {
            //UART_printf("%s <--> %s, ping/pong iteration %d ...\n",
            //        Ipc_mpGetSelfName(), Ipc_mpGetName(dstProc), i);
        }
    }

    UART_printf("%s <--> %s, Ping- %d, pong - %d completed\n",
            Ipc_mpGetSelfName(),
            Ipc_mpGetName(dstProc),
            cntPing, cntPong);

    /* Delete the RPMesg object now */
    RPMessage_delete(&handle);
    return;
}

/*
 * This "Task" waits for Linux vdev ready, and late create the vrings
 *
 */
#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
void Lpm_ipcVdevMonitorFxn(void* arg0, void* arg1)
{
    int32_t status;

    UART_printf("Waiting for Linux VDev ready...\n");
    while(!Ipc_isRemoteReady(IPC_MPU1_0))
    {
        if(!g_exitRespTsk)
        {
            TaskP_sleep(10);
        }
        else
        {
            UART_printf("Exiting rpmsg_vdevMonitorFxn!\n");
            return;
        }
    }
    UART_printf("Linux VDev is ready!\n");

    /* Create the VRing now ... */
    status = Ipc_lateVirtioCreate(IPC_MPU1_0);
    if(status != IPC_SOK)
    {
        UART_printf("%s: Ipc_lateVirtioCreate failed\n", __func__);
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("%s: Ipc_lateVirtioCreate passed\n", __func__);
#endif
    }

    status = RPMessage_lateInit(IPC_MPU1_0);
    if(status != IPC_SOK)
    {
        UART_printf("%s: RPMessage_lateInit failed\n", __func__);
        return;
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("%s: RPMessage_lateInit passed\n", __func__);
#endif
    }

    status = RPMessage_announce(IPC_MPU1_0, RecvEndPt, SERVICE_PING);
    if(status != IPC_SOK)
    {
        UART_printf("rpmsg_vdevMonitorFxn: RPMessage_announce() failed\n");
    }
    else
    {
#ifdef DEBUG_PRINT
        UART_printf("rpmsg_vdevMonitorFxn: RPMessage_announce() passed\n");
#endif
    }

    return;
}
#endif /* defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)*/

static void Lpm_ipcAppPrintf(const char *str)
{
    UART_printf("%s", str);

    return;
}

volatile uint32_t loop_ipcEcho = 0x0;
int32_t Lpm_ipcEchoApp(void)
{
    uint32_t          t; 
    TaskP_Params      params;
    uint32_t          numProc = gNumRemoteProc;
    Ipc_VirtIoParams  vqParam;
    Ipc_InitPrms      initPrms;
    uint32_t          index = 0;
    int32_t           retVal = IPC_SOK;

    while(loop_ipcEcho);

    if(g_exitRespTsk == 0x1U)
    {
        memset(pCntrlBuf, 0, RPMSG_DATA_SIZE);
        memset(pSendTaskBuf, 0, RPMSG_DATA_SIZE * CORE_IN_TEST);
        memset(pRecvTaskBuf, 0, RPMSG_DATA_SIZE * 2);
        memset(pSysVqBuf, 0, VQ_BUF_SIZE);
#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
        RecvEndPt = 0;
#endif
        gRecvTaskBufIdx = 0;
        memset(&gSendTaskBufIdx, 0, IPC_MAX_PROCS);
        memcpy((void*)0xa0100000, pOcmcShadowRscTable, 0x8c);
        g_exitRespTsk = 0;
    }

    /* Step1 : Initialize the multiproc */
    retVal += Ipc_mpSetConfig(selfProcId, numProc, pRemoteProcArray);
    if(retVal != IPC_SOK)
    {
        UART_printf("Ipc_mpSetConfig failed!\n");
    }
    else
    {
        UART_printf("IPC_echo_test (core : %s) .....\r\n", Ipc_mpGetSelfName());


        /* Initialize params with defaults */
        IpcInitPrms_init(0U, &initPrms);
        
        initPrms.printFxn = &Lpm_ipcAppPrintf;

        retVal += Ipc_init(&initPrms);
        if(retVal != IPC_SOK)
        {
            UART_printf("Ipc_init failed!\n");
        }
        else
        {
#ifdef DEBUG_PRINT
            UART_printf("Required Local memory for Virtio_Object = %d\r\n",
            numProc * Ipc_getVqObjMemoryRequiredPerCore());
#endif

#if defined(A72_LINUX_OS)
            /* If A72 remote core is running Linux OS, then
            * load resource table
            */
            retVal += Ipc_loadResourceTable((void*)&ti_ipc_remoteproc_ResourceTable);
            if(retVal != IPC_SOK)
            {
                UART_printf("Ipc_loadResourceTable failed!\n");
            }
            else
            {
#if !defined(A72_LINUX_OS_IPC_ATTACH)
                /* Wait for Linux VDev ready... */
                for(t = 0; t < numProc; t++)
                {
                    while(!Ipc_isRemoteReady(pRemoteProcArray[t]))
                    {
                        TaskP_sleep(10);
                    }
                }
                //UART_printf("Linux VDEV ready now .....\n");
#endif
#endif
                /* Step2 : Initialize Virtio */
                vqParam.vqObjBaseAddr = (void*)pSysVqBuf;
                vqParam.vqBufSize     = numProc * Ipc_getVqObjMemoryRequiredPerCore();
                vqParam.vringBaseAddr = (void*)VRING_BASE_ADDRESS;
                vqParam.vringBufSize  = IPC_VRING_BUFFER_SIZE;
                vqParam.timeoutCnt    = 100;  /* Wait for counts */
                retVal += Ipc_initVirtIO(&vqParam);
                if(retVal != IPC_SOK)
                {
                    UART_printf("Ipc_initVirtIO failed!\n");
                }
                else
                {
                    /* Step 3: Initialize RPMessage */
                    RPMessage_Params cntrlParam;

#ifdef DEBUG_PRINT
                    UART_printf("Required Local memory for RPMessage Object = %d\n",
                       RPMessage_getObjMemRequired());
#endif

                    /* Initialize the param */
                    RPMessageParams_init(&cntrlParam);

                    /* Set memory for HeapMemory for control task */
                    cntrlParam.buf         = pCntrlBuf;
                    cntrlParam.bufSize     = rpmsgDataSize;
                    cntrlParam.stackBuffer = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
                    cntrlParam.stackSize   = IPC_TASK_STACKSIZE;
                    retVal += RPMessage_init(&cntrlParam);
                    if(retVal != IPC_SOK)
                    {
                        UART_printf("RPMessage_init failed!\n");
                    }
                    else
                    {
#ifdef DEBUG_PRINT
                        UART_printf("RPMessage_init successful!\n");
#endif
                        /* Respond to messages coming in to endPt ENDPT_PING */
                        TaskP_Params_init(&params);
                        params.priority   = IPC_SETUP_TASK_PRI;
                        params.stack      = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
                        params.stacksize  = IPC_TASK_STACKSIZE;
                        params.arg0       = (void *)&service_ping.endPt;
                        params.arg1       = (void *)&service_ping.name[0];
                        TaskP_create(&Lpm_ipcResponderFxn, &params);

#ifdef DEBUG_PRINT
                        UART_printf("Lpm_ipcResponderFxn for ENDPT_PING created!\n");
#endif
                        TaskP_sleep(1000);

#if defined(A72_LINUX_OS)
                        /* Respond to messages coming in to endPt ENDPT_CHRDEV (for testing rpmsg_chrdev) */
                        TaskP_Params_init(&params);
                        params.priority   = IPC_SETUP_TASK_PRI;
                        params.stack      = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
                        params.stacksize  = IPC_TASK_STACKSIZE;
                        params.arg0       = (void *)&service_chrdev.endPt;
                        params.arg1       = (void *)&service_chrdev.name[0];
                        TaskP_create(&Lpm_ipcResponderFxn, &params);
#ifdef DEBUG_PRINT
                        UART_printf("Lpm_ipcResponderFxn for ENDPT_CHRDEV created!\n");
#endif
                        TaskP_sleep(1000);
#endif

                        for(t = 0; t < numProc; t++)
                        {
#if defined(A72_LINUX_OS)
                            /* Linux does not have a responder func running */
                            if(pRemoteProcArray[t] == IPC_MPU1_0)
                            {
                                continue;
                            }
#endif
                            /* Store index in global array.
                            * the pointer of which is to be passed as argument to sender task */
                            gSendTaskBufIdx[t] = t;
                            /* send messages to peer(s) on ENDPT_PING */
                            TaskP_Params_init(&params);
                            params.priority  = IPC_SETUP_TASK_PRI;
                            params.stack     = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
                            params.stacksize = IPC_TASK_STACKSIZE;
                            params.arg0      = (void *)&pRemoteProcArray[t];
                            params.arg1      = (void *)&gSendTaskBufIdx[t];
                            TaskP_create(&Lpm_ipcSenderFxn, &params);

                        }

#if defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH)
                        /* Respond to messages coming in to endPt ENDPT_PING */
                        TaskP_Params_init(&params);
                        params.priority  = 7;
                        params.stack     = &pTaskBuf[index++ * IPC_TASK_STACKSIZE];
                        params.stacksize = IPC_TASK_STACKSIZE;
                        params.arg0 = 0;
                        TaskP_create(&Lpm_ipcVdevMonitorFxn, &params);
#endif /* defined(A72_LINUX_OS) && defined(A72_LINUX_OS_IPC_ATTACH) */
                    }
                }
            }
#if defined(A72_LINUX_OS)
        }
#endif
    }

    //loop_ipcEcho = 0xDEADBEEF;
    return 1;
}
