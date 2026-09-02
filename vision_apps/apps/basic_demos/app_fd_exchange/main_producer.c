/*
 *  Copyright (c) Texas Instruments Incorporated 2018
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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <getopt.h>

#include <VX/vx.h>
#include <TI/tivx.h>

#include <app_init.h>
#include "app_common.h"

#define APP_STATE_CLI_NOT_CONNECTED     (0U)
#define APP_STATE_CLI_CONNECTED         (1U)
#define APP_STATE_CLI_OBJINFO_XFERD     (2U)

#define TEST_ENTRY(X, v)   {#X, X, (v)}

typedef struct
{
    const char *name;
    vx_enum     type;
    uint32_t    aux;
} TestArg;

static TestArg gTestTbl[] =
{
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_RGB),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_RGBX),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_NV12),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_NV21),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_UYVY),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_IYUV),
    TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_U8),
    TEST_ENTRY(VX_TYPE_TENSOR, 1),
    TEST_ENTRY(VX_TYPE_TENSOR, 2),
    TEST_ENTRY(VX_TYPE_TENSOR, 3),
    TEST_ENTRY(VX_TYPE_TENSOR, 4),
    TEST_ENTRY(VX_TYPE_USER_DATA_OBJECT, 1024),
    TEST_ENTRY(VX_TYPE_ARRAY, 100),
    TEST_ENTRY(VX_TYPE_CONVOLUTION, 9),
    TEST_ENTRY(VX_TYPE_MATRIX, 8),
    TEST_ENTRY(VX_TYPE_DISTRIBUTION, 100),
    TEST_ENTRY(TIVX_TYPE_RAW_IMAGE, 100),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_RGB),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_RGBX),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_NV12),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_NV21),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_UYVY),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_IYUV),
    TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_U8)
};

typedef struct
{
    /** Verbose flag. */
    uint8_t     verbose;

} App_CmdLineParams;

typedef struct
{
    /** Initialization status. */
    uint8_t                 initDone;

    /** Producer state. */
    uint8_t                 state;

    /** Test pattern tracker. */
    uint32_t                testPattern;

    /* VX context. */
    vx_context              vxContext;

    /** IPC handle. */
    AppUtil_NetSrvCntxt     svrCntxt;

    /** Server socket Id. */
    int32_t                 cliSockId;

    /** Number of image objects. */
    uint32_t                numValidObjs;

    /** Image objects. */
    vx_reference            refs[APP_MAX_NUM_REFS];

    /* Command buffer memory. */
    uint8_t                 cmdBuff[APP_MAX_MSG_BUFF_SIZE];

    /* Response buffer memory. */
    uint8_t                 rspBuff[APP_MAX_MSG_BUFF_SIZE];

} App_Context;

static App_Context gAppCntxt;

void App_clientHdlr(void *usrCntxt, void *cliSock);

static int32_t App_sendMsg(App_Context     *appCntxt,
                           uint8_t         *cmdBuff,
                           uint32_t         cmdBuffSize,
                           int32_t         *fd,
                           uint32_t         numFd,
                           uint8_t         *rspBuff)
{
    App_MsgHdr         *hdr;
    App_GenericRspMsg  *rsp;
    int32_t             status = 0;

    hdr = (App_MsgHdr *)cmdBuff;
    rsp = (App_GenericRspMsg *)rspBuff;

    /* Send the command. */
    status = AppUtil_netWriteUnixSock(appCntxt->cliSockId,
                                      cmdBuff,
                                      cmdBuffSize,
                                      fd,
                                      numFd);

    if (status == APPUTIL_NET_RET_SUCCESS)
    {
        /* Wait for the response. */
        status = AppUtil_netReadUnixSock(appCntxt->cliSockId,
                                         rspBuff,
                                         sizeof(App_GenericRspMsg),
                                         NULL,
                                         NULL);

        if (status == APPUTIL_NET_RET_SUCCESS)
        {
            if ((rsp->msgId != hdr->msgId) ||
                (rsp->status != APP_CMD_STATUS_SUCCESS))
            {
                VX_PRINT(VX_ZONE_INFO,
                         "PRODUCER::Received failure status from client.\n");
                status = -1;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_INFO, "PRODUCER::AppUtil_netWriteUnixSock() failed.\n");
    }

    return status;
}

static int32_t App_establishSync(App_Context *appCntxt)
{
    App_MsgHdr *hdr;
    int32_t     status = 0;

    hdr = (App_MsgHdr *)appCntxt->cmdBuff;
    hdr->msgId = APP_MSGTYPE_HELLO_CMD;

    VX_PRINT(VX_ZONE_INFO, "PRODUCER::Sending [APP_MSGTYPE_HELLO_CMD]\n");

    status = App_sendMsg(appCntxt,
                         (uint8_t *)hdr,
                         sizeof(App_MsgHdr),
                         NULL,
                         0,
                         appCntxt->rspBuff);

    if (status == 0)
    {
        VX_PRINT(VX_ZONE_INFO,
                 "PRODUCER::Received [APP_MSGTYPE_HELLO_RSP] with status: "
                 "SUCCESS.\n");
    }

    return status;
}

static int32_t App_deInit(App_Context *appCntxt)
{
    int32_t status = 0;

    if (appCntxt->initDone == 1)
    {
        /* De-allocate any allocated objects. */
        App_Common_DeAllocImageObjects(appCntxt->refs,
                                       appCntxt->numValidObjs);

        vxReleaseContext(&appCntxt->vxContext);

        status = appDeInit();

        if (appCntxt->cliSockId >= 0)
        {
            close(appCntxt->cliSockId);
        }

        appCntxt->initDone = 0;
    }

    return status;
}

static void App_intSigHandler(int sig)
{
    App_Context    *appCntxt = &gAppCntxt;
    int32_t         status;

    status = App_deInit(appCntxt);

    exit(status);
}

static int32_t App_init(App_Context *appCntxt)
{
    int32_t status = 0;

    if (appCntxt->initDone == 0)
    {
        AppUtil_NetSrvCntxt    *svrCntxt;

        appCntxt->testPattern = APP_TEST_PATTERN_INIT;
        svrCntxt              = &appCntxt->svrCntxt;
        svrCntxt->listenCnt   = 1;
        svrCntxt->pathName    = APP_UNIX_STRM_PATH_NAME;
        svrCntxt->auxData     = (void *)appCntxt;
        svrCntxt->cliHdlr     = App_clientHdlr;

        /* Create the socket. */
        status = AppUtil_netCreateSvrSocket(svrCntxt);

        if (status >= 0)
        {
            status = 0;
        }

        if (status == 0)
        {
            status = appInit();
        }

        if (status == 0)
        {
            appCntxt->vxContext = vxCreateContext();

            if (appCntxt->vxContext == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateContext() failed.\n");

                status = -1;
            }
            else
            {
                appCntxt->initDone = 1;
            }
        }
    }

    return status;
}

static int32_t App_allocObjects(App_Context *appCntxt)
{
    uint32_t    numRefs;
    int32_t     status = 0;
    uint32_t    i;

    extern vx_status ownReferenceAllocMem(vx_reference);

    numRefs = sizeof(gTestTbl)/sizeof(TestArg);

    if (numRefs > APP_MAX_NUM_REFS)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Number of objects [%d] exceed limit [%d].\n",
                 numRefs, APP_MAX_NUM_REFS);

        status = -1;
    }
    else
    {
        for (i = 0; i < numRefs; i++)
        {
            TestArg    *e = &gTestTbl[i];
            vx_status   vxStatus;

            appCntxt->refs[i] = App_Common_MemAllocObject(appCntxt->vxContext,
                                                          e->type,
                                                          e->aux);

            if (appCntxt->refs[i] == NULL)
            {
                status = -1;
                break;
            }

            /* Force internal handle allocation by mapping one of the planes. */
            vxStatus = ownReferenceAllocMem(appCntxt->refs[i]);

            /* Export the handles and write test pattern. */
            {
                void       *ptrs[VX_IPC_MAX_VX_PLANES];
                uint32_t    handleSizes[VX_IPC_MAX_VX_PLANES];
                uint32_t    numPlanes;
                uint32_t    j;

                vxStatus = tivxReferenceExportHandle(appCntxt->refs[i],
                                                     ptrs,
                                                     handleSizes,
                                                     VX_IPC_MAX_VX_PLANES,
                                                     &numPlanes);

                if (vxStatus != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "tivxReferenceExportHandle() failed.\n");
                    break;
                }

                for (j = 0; j < numPlanes; j++)
                {
                    *(uint32_t*)ptrs[j] = appCntxt->testPattern *
                                          (appCntxt->numValidObjs + j + 1);
                }

                VX_PRINT(VX_ZONE_INFO,
                         "[PRODUCER CREATED] OBJECT[%d]: "
                         "TYPE: %s NUM DIMS/PLANES: %d\n",
                         appCntxt->numValidObjs, e->name, numPlanes);
            }

            appCntxt->numValidObjs++;
        }
    }

    return status;
}

static int32_t App_sendObjInfo(App_Context     *appCntxt,
                               App_BuffDesc    *appBuffDesc)
{
    #ifdef LINUX
    int32_t     fd32[VX_IPC_MAX_VX_PLANES];
    #endif
    int32_t     status;
    uint32_t    i;

    status = 0;

    appBuffDesc->msgId   = APP_MSGTYPE_IMAGE_BUF_CMD;
    appBuffDesc->lastObj = 0;

    /* Sending the same object twice in order to verify the usage of comparing what got sent */
    for (i = 0; i < 2*appCntxt->numValidObjs; i++)
    {
        tivx_utils_ref_ipc_msg_t   *ipcMsg;
        vx_reference                ref;
        vx_status                   vxStatus;

        ipcMsg  = &appBuffDesc->ipcMsg;

        ref = appCntxt->refs[i/2];

        /* Create data to transport it over IPC to a different process. */
        vxStatus = tivx_utils_export_ref_for_ipc_xfer(ref, ipcMsg);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivx_utils_export_ref_for_ipc_xfer() failed for "
                     "object [%d]\n", i);
            break;
        }
        
        if (i == (2*appCntxt->numValidObjs - 1))
        {
            appBuffDesc->lastObj = 1;
        }

        if (status == 0)
        {
            VX_PRINT(VX_ZONE_INFO,
                     "PRODUCER::Sending [APP_MSGTYPE_IMAGE_BUF_CMD]\n");

            #ifdef QNX
            status = App_sendMsg(appCntxt,
                                 (uint8_t *)appBuffDesc,
                                 sizeof(App_BuffDesc),
                                 NULL,
                                 0,
                                 appCntxt->rspBuff);
            #else
            {
                uint32_t    j;

                for (j = 0; j < ipcMsg->numFd; j++)
                {
                    fd32[j] = (int32_t)ipcMsg->fd[j];
                }

                status = App_sendMsg(appCntxt,
                                     (uint8_t *)appBuffDesc,
                                     sizeof(App_BuffDesc),
                                     fd32,
                                     ipcMsg->numFd,
                                     appCntxt->rspBuff);
            }
            #endif
            if (status == 0)
            {
                VX_PRINT(VX_ZONE_INFO,
                         "PRODUCER::Received [APP_MSGTYPE_IMAGE_BUF_RSP] with "
                         "status: SUCCESS.\n");
            }
        }

    } /* for (i = 0; i < 2*appBuffDesc->numObjs; i++) */

    return status;
}

int32_t App_msgProcThread(App_Context  *appCntxt)
{
    uint32_t    done = 0;
    int32_t     status = 0;

    while (!done)
    {
        if (appCntxt->state == APP_STATE_CLI_NOT_CONNECTED)
        {
            status = App_establishSync(appCntxt);

            if (status < 0)
            {
                break;
            }

            appCntxt->state = APP_STATE_CLI_CONNECTED;
        }
        else if (appCntxt->state == APP_STATE_CLI_CONNECTED)
        {
            App_BuffDesc  *appBuffDesc;

            appBuffDesc = (App_BuffDesc *)appCntxt->cmdBuff;

            status = App_sendObjInfo(appCntxt, appBuffDesc);

            if (status < 0)
            {
                break;
            }

            appCntxt->state = APP_STATE_CLI_OBJINFO_XFERD;
        }
        else if (appCntxt->state == APP_STATE_CLI_OBJINFO_XFERD)
        {
            break;
        }

    } /* while (!done) */

    return status;
}

void App_clientHdlr(void *usrCntxt, void *cliSock)
{
    App_Context    *appCntxt;

    appCntxt = (App_Context *)usrCntxt;

    if (appCntxt == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "usrCntxt is NULL.\n");
    }
    else
    {
        appCntxt->cliSockId = *(uint32_t *)cliSock;

        App_msgProcThread(appCntxt);

        appCntxt->svrCntxt.exitFlag = 1;
    }

    VX_PRINT(VX_ZONE_INFO, "Exiting %s\n", __FUNCTION__);
}

static void App_showUsage( char * name)
{
    printf(" \n");
    printf("# \n");
    printf("# %s PARAMETERS [OPTIONAL PARAMETERS]\n", name);
    printf("# OPTIONS:\n");
    printf("#  [--verbose |-v]\n");
    printf("#  [--help    |-h]\n");
    printf("# \n");
    printf("# \n");
    printf("# (c) Texas Instruments 2020\n");
    printf("# \n");
    printf("# \n");

    exit(0);
}

static void
App_parseCmdLineArgs(int32_t            argc,
                     char              *argv[],
                     App_CmdLineParams *clParams)
{
    int32_t              longIndex;
    int32_t              opt;
    static struct option long_options[] = {
        {"help",         no_argument,       0,  'h' },
        {"verbose",      no_argument,       0,  'v' },
        {0,              0,                 0,   0  }
    };

    while ((opt = getopt_long(argc, argv,"hvc:i:", 
                   long_options, &longIndex )) != -1) {
        switch (opt)
        {
            case 'v' :
                clParams->verbose = 1;
                break;

            case 'h' :
            default:
                App_showUsage(argv[0]);

        } // switch (opt)

    } // while ((opt = getopt_long(argc, argv

    return;
}

int main(int argc, char *argv[])
{
    App_Context        *appCntxt;
    App_CmdLineParams   cmdParams = {0};
    int32_t             status;
    uint32_t    numAllocInitial, numAllocFinal;
#if defined(QNX)
    uint32_t    numMapInitial,   numMapFinal;
    uint32_t    numBufsInitial, numBufsFinal;
#endif

    extern uint32_t appMemGetNumAllocs();

#if defined(QNX)
    extern uint32_t appMemGetNumMaps();
    extern uint32_t appMemGetNumBufElements();
#endif

    numAllocInitial = appMemGetNumAllocs();

#if defined(QNX)
    numMapInitial   = appMemGetNumMaps();
    numBufsInitial  = appMemGetNumBufElements();
#endif

    appCntxt = &gAppCntxt;
    status   = 0;

    /* Register the signal handler. */
    signal(SIGINT, App_intSigHandler);

    memset(appCntxt, 0, sizeof(App_Context));

    /* Parse command line arguments. */
    App_parseCmdLineArgs(argc, argv, &cmdParams);

    status = App_init(appCntxt);

    if (status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "App_init() failed.\n");
    }

    if (cmdParams.verbose)
    {
        tivx_set_debug_zone(VX_ZONE_INFO);
    }

    /* Create image objects. */
    if (status == 0)
    {
        status = App_allocObjects(appCntxt);
    }

    if (status == 0)
    {
        AppUtil_netIterSvr(&appCntxt->svrCntxt);
    }

    status = App_deInit(appCntxt);

    if (status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "App_deInit() failed.\n");
    }

    numAllocFinal = appMemGetNumAllocs();

    if (numAllocInitial != numAllocFinal)
    {
        VX_PRINT(VX_ZONE_ERROR, "Memory leak: some memory that was allocated was not freed\n");
        status = -1;
    }

#if defined(QNX)
    numMapFinal = appMemGetNumMaps();
    if (numMapInitial != numMapFinal)
    {
        VX_PRINT(VX_ZONE_ERROR, "Resource leak: some memory that was mapped was not unmapped\n");
        status = -1;
    }

    numBufsFinal  = appMemGetNumBufElements();
    if (numBufsInitial != numBufsFinal)
    {
        VX_PRINT(VX_ZONE_ERROR, "Memory leak: buffer array element was not released\n");
        status = -1;
    }
#endif

    return status;
}

