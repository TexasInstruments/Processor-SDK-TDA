/*
 *
 * Copyright (c) 2018-2025 Texas Instruments Incorporated
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
#include <stdint.h>
#include <utils/mem/include/app_mem.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <app_init.h>

#if defined (QNX)
#include <hw/inout.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#endif

#define APP_ASSERT_SUCCESS(x)  { if((x)!=0) while(1); }

#define MAX_NUM     (64)

void appMemAllocTest()
{
    void *ptr[MAX_NUM];
    int dmaBufFd[MAX_NUM];
    uint64_t phyPtr = 0;
    uint32_t size = 128*1024;
    uint32_t loop = 16;
    uint32_t i;
    uint32_t offset;
    uint64_t cur_time;

    for(i=0; i<loop; i++)
    {
        ptr[i] = appMemAlloc(APP_MEM_HEAP_DDR_SHARED, size, 1);
        if(ptr[i])
        {
            printf("APP_MEM: %d: Allocated memory @ %p of size %d bytes \n", i, ptr[i], size);

            phyPtr = appMemGetVirt2PhyBufPtr((uint64_t)ptr[i], APP_MEM_HEAP_DDR_SHARED);
            printf("APP_MEM: %d: Translated virtual addr = %p -> phyical addr = %lx \n", i, ptr[i], phyPtr);

            dmaBufFd[i] = appMemGetDmaBufFd(ptr[i], &offset);
            printf("APP_MEM: %d: Exported dmaBufId %d with offset %d\n", i, dmaBufFd[i], offset);
        }
        else
        {
            printf("APP_MEM: %d: ERROR: Unable to allocate memory size %d bytes \n", i, size);
        }
    }
    cur_time = appLogGetTimeInUsec();
    for(i=0; i<loop; i++)
    {
        if(ptr[i])
        {
            uint32_t k;
            uint8_t *ptr8 = (uint8_t*)ptr[i];

            for(k=0; k<size; k++)
            {
                ptr8[k] = k;
            }
            for(k=0; k<size; k++)
            {
                ptr8[k] = k*k;
            }
        }
    }
    for(i=1; i<loop; i++)
    {
        if(ptr[i] && ptr[i-1])
        {
            uint32_t k;
            uint8_t *dst8 = (uint8_t*)ptr[i];
            uint8_t *src8 = (uint8_t*)ptr[i-1];

            for(k=0; k<size; k++)
            {
                dst8[k] = src8[k]*dst8[k];
            }
        }
    }
    cur_time = appLogGetTimeInUsec() - cur_time;
    printf("APP_MEM: elasped time for data processing is %lu usecs\n", cur_time);
    for(i=0; i<loop; i++)
    {
        if(ptr[i])
        {
            appMemFree(APP_MEM_HEAP_DDR_SHARED, ptr[i], size);
            printf("APP_MEM: %d: Free'ed memory @ %p of size %d bytes \n", i, ptr[i], size);
        }
    }

}

int main(void)
{
    int32_t status = 0;

    status = appCommonInit();
#if defined (QNX)
    /* Get IO privileges */
    if (ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
        perror("ThreadCtl(_NTO_TCTL_IO");
        return 1;
    }
#endif
    if (status == 0)
    {
        appMemAllocTest();
        status = appCommonDeInit();
    }
    return status;
}
