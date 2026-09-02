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

typedef struct
{
    /** Verbose flag. */
    uint8_t     verbose;

} App_CmdLineParams;

typedef struct
{
    /** Initialization status. */
    uint8_t                 initDone;

    /** Test pattern tracker. */
    uint32_t                testPattern;

    /* VX context. */
    vx_context              vxContext;

    /** Socket Id. */
    int32_t                 sockId;

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

static tivx_utils_ref_ipc_msg_t   prevIpcMsg;

static int32_t App_createObjFromBuffInfo(App_Context   *appCntxt,
                                         App_BuffDesc  *appBuffDesc,
                                         int32_t       *fd,
                                         uint32_t       numFd)
{
    tivx_utils_ref_ipc_msg_t   *ipcMsg;
    vx_reference                ref;
    int32_t                     status;
    vx_status                   vxStatus;
    uint32_t                    objNum;
    #ifdef LINUX
    uint32_t                    i;
    #endif

    status = 0;
    objNum = appCntxt->numValidObjs;
    ipcMsg = &appBuffDesc->ipcMsg;

    #ifdef QNX
    numFd  = ipcMsg->numFd;
    #endif

    #ifdef LINUX
    /* Update the FD information with the kernel translated values. */
    for (i = 0; i < numFd; i++)
    {
        ipcMsg->fd[i] = fd[i];
    }
    #endif

    /* Create a new object. */
    ref = NULL;
    vxStatus = tivx_utils_import_ref_from_ipc_xfer(appCntxt->vxContext,
                                                   ipcMsg,
                                                   &ref);

    appCntxt->refs[objNum] = ref;

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "tivx_utils_import_ref_from_ipc_xfer() failed for Obj [%d]\n",
                 objNum);
        status = -1;
    }
    else
    {
        /* Export the handles and check test pattern. */
        void       *ptrs[VX_IPC_MAX_VX_PLANES];
        uint32_t    handleSizes[VX_IPC_MAX_VX_PLANES];
        uint32_t    numPlanes;
        uint32_t    j;

        vxStatus = tivxReferenceExportHandle(appCntxt->refs[objNum],
                                             ptrs,
                                             handleSizes,
                                             VX_IPC_MAX_VX_PLANES,
                                             &numPlanes);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivxReferenceExportHandle() failed.\n");
        }
        else
        {
            for (j = 0; j < numPlanes; j++)
            {
                uint32_t    expected;

                /* Since the producer app is sending the same obj twice, accounting for this with test pattern */
                expected = appCntxt->testPattern * (objNum/2 + j + 1);

                if (expected != *(uint32_t*)ptrs[j])
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "Data validation check for Obj [%d] plane [%d] "
                             "failed. Expected = 0x%8.8X Received = 0x%8.8X\n",
                             objNum, j, expected, *(uint32_t*)ptrs[j]);
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO,
                             "Data validation check for Obj [%d] plane [%d] "
                             "passed. Expected = 0x%8.8X Received = 0x%8.8X\n",
                             objNum, j, expected, *(uint32_t*)ptrs[j]);
                }
            }
        }

        appCntxt->numValidObjs++;
    }

    /* Validating that the ipcMsg received */
    if (appCntxt->numValidObjs > 1)
    {
        vx_bool         refCompare;

        refCompare = tivx_utils_compare_refs_from_ipc_xfer(ipcMsg, &prevIpcMsg);

        if ( ((appCntxt->numValidObjs % 2) == 0) && (refCompare == (vx_bool)vx_false_e) )
        {
            VX_PRINT(VX_ZONE_ERROR, "The previous IPC message should have matched the previous IPC message but didn't\n");
        }
        else if ( ((appCntxt->numValidObjs % 2) == 1) && (refCompare == (vx_bool)vx_true_e) )
        {
            VX_PRINT(VX_ZONE_ERROR, "The previous IPC message should not have matched the previous IPC message but did\n");
        }
    }

    prevIpcMsg = *ipcMsg;

    return status;
}

static int32_t App_deInit(App_Context *appCntxt)
{
    int32_t status = 0;

    if (appCntxt->initDone == 1)
    {
        vxReleaseContext(&appCntxt->vxContext);

        status = appDeInit();

        if (appCntxt->sockId >= 0)
        {
            close(appCntxt->sockId);
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
        appCntxt->testPattern = APP_TEST_PATTERN_INIT;

        /* Create a socket connection. */
        appCntxt->sockId = AppUtil_netUnixConnect(APP_UNIX_STRM_PATH_NAME);

        if (appCntxt->sockId < 0)
        {
            status = -1;
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

int32_t App_msgProcThread(App_Context  *appCntxt)
{
    int32_t             fd[VX_IPC_MAX_VX_PLANES];
    uint32_t            numFd;
    uint32_t           *numFdPtr;
    App_GenericRspMsg  *rspMsg;
    App_MsgHdr         *msgHdr;
    uint32_t            done = 0;
    uint32_t            size;
    int32_t             status = 0;

    rspMsg   = (App_GenericRspMsg *)appCntxt->rspBuff;
    numFdPtr = NULL;
    size     = sizeof(App_MsgHdr);

    while (!done)
    {
        /* Wait for a command. */
        status = AppUtil_netReadUnixSock(appCntxt->sockId,
                                         appCntxt->cmdBuff,
                                         size,
                                         fd,
                                         numFdPtr);

        if (status < 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "AppUtil_netReadUnixSock() failed.\n");
            break;
        }

        msgHdr        = (App_MsgHdr *)appCntxt->cmdBuff;
        rspMsg->msgId = msgHdr->msgId;

        switch (msgHdr->msgId)
        {
            case APP_MSGTYPE_HELLO_CMD:
                VX_PRINT(VX_ZONE_INFO,
                         "CONSUMER::Received [APP_MSGTYPE_HELLO_CMD]\n");
                status = 0;
                numFdPtr = &numFd;
                size = sizeof(App_BuffDesc);
                break;

            case APP_MSGTYPE_IMAGE_BUF_CMD:
                VX_PRINT(VX_ZONE_INFO,
                         "CONSUMER::Received [APP_MSGTYPE_IMAGE_BUF_CMD]\n");

                {
                    App_BuffDesc  *appBuffDesc;

                    appBuffDesc = (App_BuffDesc *)appCntxt->cmdBuff;

                    status = App_createObjFromBuffInfo(appCntxt,
                                                       appBuffDesc,
                                                       fd,
                                                       numFd);
                    if (status < 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "App_createObjFromBuffInfo() failed.\n");
                    }

                    if (1==appBuffDesc->lastObj)
                    {
                        numFdPtr = NULL;
                        done = 1;

                        /* De-allocate any allocated objects. */
                        App_Common_DeAllocImageObjects(appCntxt->refs,
                                                       appCntxt->numValidObjs);
                    }
                }
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR,
                         "CONSUMER::Received [UNKNOWN MESSAGE]\n");
                status = -1;
                break;
        }

        if (status < 0)
        {
            rspMsg->status = APP_CMD_STATUS_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "TEST STATUS: FAILED.\n");
        }
        else
        {
            rspMsg->status = APP_CMD_STATUS_SUCCESS;
            VX_PRINT(VX_ZONE_INFO, "TEST STATUS: PASSED.\n");
        }

        /* Send the response. */
        status = AppUtil_netWriteUnixSock(appCntxt->sockId,
                                          (uint8_t *)rspMsg,
                                          sizeof(App_GenericRspMsg),
                                          NULL,
                                          0);

        if (status < 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "AppUtil_netWriteUnixSock() failed.\n");
        }

    } /* while (!done) */

    return status;
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
    App_Context        *appCntxt = &gAppCntxt;
    App_CmdLineParams   cmdParams = {0};
    int32_t             status = 0;
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

    if (status == 0)
    {
        status = App_msgProcThread(appCntxt);
    }

    if (status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "App_msgProcThread() failed.\n");
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

