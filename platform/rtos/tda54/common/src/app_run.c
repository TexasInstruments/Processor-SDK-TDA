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

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>

#include <app_run.h>
#include CORE_CFG_FILE
#include <platform.h>

#if defined(ENABLE_TIOVX_HOST)
#include <stdio.h>
#include <string.h>

#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/console_io/include/app_cli.h>

#include <TI/tivx.h>
#include <ti/board/board.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

int app_single_cam_main(int argc, char* argv[]);

char vx_tutorial_get_char()
{
    char buf[8u];

    while(1)
    {
        buf[0]=0;
        UART_gets(buf, 8u);
        if(buf[0]!=0)
            break;
    }

    return buf[0];
}

int appRunTiovxTutorial(int argc, char *argv[])
{
    vx_tutorial_run_interactive();
    return 0;
}

int appRunTiovxConformance(int argc, char *argv[])
{
    TestModuleRegister();
    vx_conformance_test_main(argc, argv);
    TestModuleUnRegister();
    return 0;
}

int appRunSampleCliAppSubCmd(int argc, char *argv[])
{
    printf("Executing sample cli app sub command %s\n", argv[0]);
    return 0;
}

int appRunSampleCliApp(int argc, char *argv[])
{
    uint32_t is_exit = 0;

    printf("Entering sample cli app ...\n");

    appCliRegisterAppSubCmd("sub_cmd_1", "This is a sample app sub command", appRunSampleCliAppSubCmd);

    while(!is_exit)
    {
        appCliShowPrompt(&is_exit);
    }

    printf("Exiting sample cli app ...\n");

    return 0;
}

#define FS_APP_PATH_BUF_SIZE  (512U)
#define FS_APP_DATA_BUF_SIZE  (512U)
static char gFsInFileName[FS_APP_PATH_BUF_SIZE];
static char gFsOutFileName[FS_APP_PATH_BUF_SIZE];
static char gFsAppDataBuf[FS_APP_DATA_BUF_SIZE];

int32_t appRunFileioTest(int argc, char *argv[])
{
    FILE *src=NULL, *dst=NULL;
    uint32_t bytesWrite = 0;
    uint32_t bytesRead = 0;
    uint32_t done = 0;
    int32_t retStat = 0;
    size_t bufSize = FS_APP_PATH_BUF_SIZE;
    size_t fileSize;

    strcpy(gFsInFileName, "test_rd.txt");
    /* Open the file for reading. */
    src = fopen(gFsInFileName, "rb");
    /* If there was some problem opening the file, then return an error. */
    if(src == NULL)
    {
        appLogPrintf("Failed to open file %s for read !!!\n", gFsInFileName);
        retStat = -1;
    }

    if(retStat == 0)
    {
        fseek(src, 0, SEEK_END);
        fileSize = ftell(src);

        appLogPrintf("Opened %s file of size %d bytes for reading !!!\n", gFsInFileName, fileSize);
    }

    strcpy(gFsOutFileName, "test_wr.txt");
    dst = fopen(gFsOutFileName, "wb");
    if(dst == NULL)
    {
        appLogPrintf("Failed to open file %s for write !!!\n", gFsOutFileName);
        retStat = -1;
    }

    if(retStat == 0)
    {
        appLogPrintf("Opened %s file for writing !!!\n", gFsOutFileName, fileSize);

        /*
         * Enter a loop to repeatedly read data from the file and copy to destination file,
         * until the end of the file is reached.
         */
        do
        {
            /*
             * Read a block of data from the file.  Read as much as can fit in
             * temporary buffer, including a space for the trailing null.
             */
            bytesRead = fread(gFsAppDataBuf, 1, bufSize, src);

            /*
             * If there was an error reading, then print a newline and return
             * error to the user.
             */
            if(bytesRead != bufSize)
            {
                done = 1;
            }

            if(bytesRead > 0)
            {
                /*
                 * Write the data to the destination file user has selected.
                 * If there was an error writing, then print a newline and return
                 * error to the user.
                 */
                bytesWrite = fwrite(gFsAppDataBuf, 1, bytesRead, dst);
                if(bytesRead != bytesWrite)
                {
                    appLogPrintf("Fail to write into file !!!!\n");
                    done = 1;
                    retStat = -1;
                }
            }
            /*
             * Continue reading until less than the full number of bytes are
             * read. That means the end of the buffer was reached.
             */
        } while(!done);
    }

    /*
     * Close the files.
     */
    if(src!=NULL)
    {
        fclose(src);
    }
    if(dst!=NULL)
    {
        fclose(dst);
    }
    if(retStat==0)
    {
        appLogPrintf("Copied %s file of size %d bytes to file %s !!!\n",
            gFsInFileName, fileSize, gFsOutFileName);
    }
    return(retStat);
}

void appMenuMain()
{
    uint32_t is_exit = 0;

    appCliRegisterAppCmd("app_vx_tutorial", "TIOVX tutorials", appRunTiovxTutorial);
    appCliRegisterAppCmd("app_vx_conformance", "TIOVX conformance test", appRunTiovxConformance);
    appCliRegisterAppCmd("app_c7x_kernel", "C7x kernel example", app_c7x_kernel_main);
    appCliRegisterAppCmd("app_rtos_test", "Runs RTOS task sleep test", appRunRtosTest);
    /* Below apps not yet supported in RTOS */
    /* appCliRegisterAppCmd("app_tidl", "TIDL Object Classification Demo", app_tidl_main); */
    /* appCliRegisterAppCmd("app_singlecam_test", "Single camera : Capture+VISS+2A+DSS", app_single_cam_main); */
    /* appCliRegisterAppCmd("app_cli_sample", "Sample cli app example", appRunSampleCliApp); */
    /* appCliRegisterAppCmd("app_remote_service_test", "Remote service test", appRunRemoteServiceTest); */
    /* appCliRegisterAppCmd("app_tidl_od", "TIDL Object Detection Demo", app_tidl_od_main); */
    /* appCliRegisterAppCmd("app_tidl_pc", "TIDL Pixel Classification Demo", app_tidl_pc_main); */
    /* appCliRegisterAppCmd("app_fileio_test", "Runs a file IO test using MMCSD", appRunFileioTest); */
    /* appCliRegisterAppCmd("app_dof_example", "DOF Example Demo", app_dof_main); */

    appCliShowBanner();

    while(!is_exit)
    {
        appCliShowPrompt(&is_exit);
    }
}

#endif


void appRun()
{
    appLogPrintf("APP: Run ... !!!\n");

    #if defined(ENABLE_IPC_ECHO_TEST) && defined(ENABLE_IPC)
    appIpcEchoTestStart();
    #endif
    #if defined(ENABLE_TIOVX_HOST)
    appMenuMain();
    #endif

    appLogPrintf("APP: Run ... Done !!!\n");
}
