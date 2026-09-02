/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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
#include <getopt.h>
#include <thread>
#include <cstdlib>

#include <utils/mem/include/app_mem.h>
#include <utils/mem/include/app_mem_limits.h>
#include <app_init.h>

#define APP_ASSERT_SUCCESS(x)  { if ((x)!=0) while(1); }

#define MAX_LOOP_COUNT          (APP_MEM_HEAP_MAX_NUM_MEM_BLKS_IN_USE - 48)
#define MAX_NUM_THREADS         (2)
#define MAX_BUFFS_PER_THREAD    (MAX_LOOP_COUNT/MAX_NUM_THREADS)

typedef struct
{
    /** Verbose flag. */
    uint8_t     verbose;

} App_CmdLineParams;

static void                *gPtr[MAX_LOOP_COUNT];
static std::thread          gThreadIds[MAX_NUM_THREADS];
static App_CmdLineParams    cmdParams = {0};

static int32_t appMemAllocFailTest(bool verbose)
{
    void   *ptr;
    int32_t status = 0;

    printf("APP_MEM: Negative test.\n");

    ptr = appMemAlloc(APP_MEM_HEAP_DDR, 1024, 1);

    if (ptr != NULL)
    {
        printf("APP_MEM: ERROR: Expecting failure but got success.\n");
        status = -1;
    }
    else
    {
        printf("APP_MEM: Negative test passed.\n");
    }

    return status;
}

static int32_t appMemAllocTest(int32_t  loopCnt, void **ptr, const char *str, bool verbose)
{
    uint32_t    size = 1024;
    int32_t     i;
    uint32_t    offset;
    uint64_t    cur_time;
    int32_t     status = 0;
    int32_t     numAllocs = 0;
    int32_t     numDeAllocs = 0;
    int32_t     numFail = 0;

    printf("[loopCnt = %d] %s\n", loopCnt, str);

    for (i = 0; i < loopCnt; i++)
    {
        int32_t  dmaBufFd;

        ptr[i] = appMemAlloc(APP_MEM_HEAP_DDR, size, 1);

        if (ptr[i])
        {
            uint64_t phyPtr;

            numAllocs++;
            phyPtr   = appMemGetVirt2PhyBufPtr((uint64_t)ptr[i], APP_MEM_HEAP_DDR);
            dmaBufFd = appMemGetDmaBufFd(ptr[i], &offset);

            if (verbose)
            {
                printf("APP_MEM: %d: Allocated memory @ %p of size %d bytes \n", i, ptr[i], size);
                printf("APP_MEM: %d: Translated virtual addr = %p -> phyical addr = %lx \n", i, ptr[i], phyPtr);
                printf("APP_MEM: %d: Exported dmaBufId %d with offset %d\n", i, dmaBufFd, offset);
            }
        }

        if (ptr[i] == NULL)
        {
            printf("APP_MEM: %d: ERROR: Unable to allocate memory size %d bytes.\n", i, size);
            numFail++;
        }
        else if (dmaBufFd < 0)
        {
            printf("APP_MEM: %d: ERROR: appMemGetDmaBufFd() failed.\n", i);
            numFail++;
        }
    }

    for (i = 0; i < numAllocs; i++)
    {
        if (ptr[i])
        {
            uint32_t k;
            uint8_t *ptr8 = (uint8_t*)ptr[i];

            for (k = 0; k < size; k++)
            {
                ptr8[k] = k;
            }

            for (k = 0; k < size; k++)
            {
                ptr8[k] = k*k;
            }
        }
    }

    for (i = 1; i < numAllocs; i++)
    {
        if (ptr[i] && ptr[i-1])
        {
            uint32_t k;
            uint8_t *dst8 = (uint8_t*)ptr[i];
            uint8_t *src8 = (uint8_t*)ptr[i-1];

            for (k = 0; k < size; k++)
            {
                dst8[k] = src8[k]*dst8[k];
            }
        }
    }

    for (i = 0; i < numAllocs; i++)
    {
        if (ptr[i])
        {
            appMemFree(APP_MEM_HEAP_DDR, ptr[i], size);
            numDeAllocs++;

            if (verbose)
            {
                printf("APP_MEM: %d: Free'ed memory @ %p of size %d bytes \n", i, ptr[i], size);
            }
        }
    }

    if (numFail != 0)
    {
        printf("APP_MEM: ERROR: Number of Allocations failed = %d\n", numFail);

        status = -1;
    }

    if (numAllocs != numDeAllocs)
    {
        printf("APP_MEM: ERROR: Number of Allocations and Deallocations do "
               "not match.\n");

        status = -1;
    }

    return status;
}

static int32_t appParallelMemAllocTest(bool verbose)
{
    void      **threadBuff[MAX_NUM_THREADS];
    int32_t     status = 0;

    printf("APP_MEM: Testing parallel allocation/de-allocation.\n");

    const char *s = "APP_MEM: Testing multi-thread allocation/de-allocation.";

    for (int32_t i = 0; i < MAX_NUM_THREADS; i++)
    {
        threadBuff[i] = (void **)malloc(sizeof(void*) * MAX_BUFFS_PER_THREAD);
        gThreadIds[i] = std::thread(appMemAllocTest, MAX_BUFFS_PER_THREAD, threadBuff[i], s, verbose);
    }

    for (int32_t i = 0; i < MAX_NUM_THREADS; i++)
    {
        gThreadIds[i].join();
        free(threadBuff[i]);
    }

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
    printf("# (c) Texas Instruments 2022\n");
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

int32_t main(int argc, char *argv[])
{
    int32_t status = 0;
    int32_t status1 = 0;
    int32_t initDone = 0;

    /* Parse command line arguments. */
    App_parseCmdLineArgs(argc, argv, &cmdParams);

    /* Nagative Test. */
    status = appMemAllocFailTest(cmdParams.verbose);

    if (status == 0)
    {
        status = appCommonInit();
    }

    if (status == 0)
    {
        initDone = 1;
    }

    if (status == 0)
    {
        const char *s = "APP_MEM: Testing single-thread allocation/de-allocation.";
        status = appMemAllocTest(MAX_LOOP_COUNT, gPtr, s, cmdParams.verbose);
    }

    if (status == 0)
    {
        status = appParallelMemAllocTest(cmdParams.verbose);
    }

    if (initDone == 1)
    {
        status1 = appCommonDeInit();
    }

    if ((status == 0) && (status1 < 0))
    {
        status = status1;
    }

    return status;
}
