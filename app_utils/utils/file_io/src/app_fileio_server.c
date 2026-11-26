/*
 *
 * Copyright (c) 2023-2025 Texas Instruments Incorporated
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
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <utils/mem/include/app_mem.h>
#include "app_fileio.h"
#include "app_fileio_priv.h"
#include "utils/console_io/include/app_log.h"
#include <utils/timer/include/app_timer.h>

static app_fileio_rd_obj_t g_app_fileio_rd_obj;

typedef struct ServerFileStruct
{
    int available;
    uint64_t fd;
} ServerFileStruct;

static int searchFileStruct(struct ServerFileStruct fileStruct[], uint64_t fd_arg)
{ 
    int i, idx = -1;
    for(i = 0; i < APP_FILEIO_MAX_NUM_FILES; i++)
    {
        if( (fileStruct[i].available == 1) && (fileStruct[i].fd == fd_arg))
        {
            idx = i;
            break;
        }
    }
    return idx;
}


static int getFreeIndex(struct ServerFileStruct fileStruct[])
{
    int i, idx = -1;
        for(i = 0; i < APP_FILEIO_MAX_NUM_FILES; i++)
        {
            if( fileStruct[i].available == 0 )
            {
                idx = i;
                break;
            }
        }
    return idx;
}

struct ServerFileStruct openFileInfo[APP_FILEIO_MAX_CPUS][APP_FILEIO_MAX_NUM_FILES];

int32_t fileIOServer(app_fileio_cpu_shared_mem_t * cpu_shared_mem, uint32_t cpu_id)
{
    char * buffer = (char *)(&cpu_shared_mem->fileio_mem[0]);
    int32_t i;
    int32_t status = (int32_t)APP_FILEIO_STATUS_OK;

    if (cpu_shared_mem->opStruct.opCode == (int32_t)TI_FILEIO_OPCODE_FOPEN)
    {
        FILE *fp = fopen((const char *)cpu_shared_mem->opStruct.fileName, (const char *)cpu_shared_mem->opStruct.mode);

        if (fp == NULL)
        {
            appLogPrintf("FOPEN : Could not open file \n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;
        }

        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            cpu_shared_mem->fp[cpu_shared_mem->fileCtr] = (uint64_t)fp;

            /* set the details in ServerFileStruct openFileInfo */
            i = getFreeIndex(openFileInfo[cpu_id]);
            if(i != -1)
            {
                openFileInfo[cpu_id][i].available = 1;
                openFileInfo[cpu_id][i].fd = cpu_shared_mem->fp[cpu_shared_mem->fileCtr];
            }
            else
            {
                appLogPrintf("FOPEN : Could not get free index for file, increase APP_FILEIO_MAX_NUM_FILES\n");
                status = (int32_t)APP_FILEIO_STATUS_ERROR;
            }
        }
        cpu_shared_mem->retValue = status;
    }

    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FCLOSE)
    {
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FCLOSE : File not open\n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;  
        }

        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            status = fclose((FILE *)(cpu_shared_mem->opStruct.fid));
            if (status == 0)
            {
                openFileInfo[cpu_id][i].available = 0;
                openFileInfo[cpu_id][i].fd = 0U;
            }
            else
            {
                appLogPrintf("FCLOSE : File close failed \n");
                status = (int32_t)APP_FILEIO_STATUS_ERROR;
            }
        }
        cpu_shared_mem->retValue = status;
    }

    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FREAD)
    {
        /* check if file is open */
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FREAD : File not open\n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;  
        }
        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            /* fread results in bus error while reading some specific counts. This is due to alignment requirements while reading from "device memory"
            Issue similar to one documented here for memset : https://jira.itg.ti.com/browse/LCPD-34600. One of the possible workarounds is to mark entire memory
            as normal memory in mmu, but cannot be permanently done in SDK.
            Using below as workaround - do a single fread to heap memory and copy one element at a time from heap memory to mapped device memory */
            void * heapMem = (void *)malloc(APP_FILEIO_PER_CPU_MEM_SIZE); /* Allocate maximum size which can be written/read to/from file at a time */
            if (NULL != heapMem)
            {
                int res = fread(heapMem, cpu_shared_mem->opStruct.size, cpu_shared_mem->opStruct.count, (FILE *)cpu_shared_mem->opStruct.fid);
                if (res != cpu_shared_mem->opStruct.count)
                {
                    appLogPrintf("FREAD error\n");
                    status = (int32_t)APP_FILEIO_STATUS_ERROR;
                }
                if(status == (int32_t)APP_FILEIO_STATUS_OK)
                {
                    int count = 0;
                    
                    /* Typecast pointer to 64 bit to copy 8 bytes at a time */
                    uint64_t * buf64 = (uint64_t *)buffer;
                    uint64_t * heapMem64 = (uint64_t *)heapMem;
                    /* Full 8 byte transfers */
                    for(count = 0; count < (cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) / 8; count++)
                    {
                        *buf64++ = *heapMem64++;
                    }
                    /* Remnant transfer as single byte transfers */
                    if((cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) % 8 > 0)
                    {
                        uint8_t * buf8 = (uint8_t *)buf64;
                        uint8_t * heapMem8 = (uint8_t *)heapMem64;
                        for(count = 0; count < (cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) % 8; count++)
                        {
                            *buf8++ = *heapMem8++;
                        }
                    }
                }
                free(heapMem);
            }
        }
        cpu_shared_mem->retValue = status;
    }

    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FWRITE)
    {
        int res = 0, count = 0;
        /* check if file is open */
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FWRITE : File not open\n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;
        }
        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            /* fwrite(memcpy within) results in bus error while writing some specific counts. This is due to alignment requirements while writing to "device memory"
            Issue similar to one documented here for memset : https://jira.itg.ti.com/browse/LCPD-34600. One of the possible workarounds is to mark entire memory
            as normal memory in mmu, but cannot be permanently done in SDK.
            Using below as workaround - copy one element at a time from mapped device memory to heap memory and do a single fwrite from heap memory */
            #if defined CODE_COVERAGE_ENABLED_FOR_TIDL || defined LDRA_COVERAGE_ENABLED
            setvbuf((FILE *)cpu_shared_mem->opStruct.fid, NULL, _IONBF, 0);
            #endif
            void * heapMem = (void *)malloc(APP_FILEIO_PER_CPU_MEM_SIZE); /* Allocate maximum size which can be written/read to/from file at a time */
            
            if(NULL != heapMem)
            {
                /* Typecast pointer to 64 bit to copy 8 bytes at a time */
                uint64_t * buf64 = (uint64_t *)buffer;
                uint64_t * heapMem64 = (uint64_t *)heapMem;
                /* Full 8 byte transfers */
                for(count = 0; count < (cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) / 8; count++)
                {
                    *heapMem64++ = *buf64++;
                }
                /* Remnant transfer as single byte transfers */
                if((cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) % 8 > 0)
                {
                    uint8_t * buf8 = (uint8_t *)buf64;
                    uint8_t * heapMem8 = (uint8_t *)heapMem64;
                    for(count = 0; count < (cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size) % 8; count++)
                    {
                        *heapMem8++ = *buf8++;
                    }
                }
                
                /* Fwrite of entire data to file */
                res = fwrite(heapMem, 1, cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size, (FILE *)cpu_shared_mem->opStruct.fid);

                free(heapMem);

                if (res != cpu_shared_mem->opStruct.count * cpu_shared_mem->opStruct.size)
                {
                    appLogPrintf("Fwrite error - count %d written %d\n", cpu_shared_mem->opStruct.count, res);
                    status = (int32_t)APP_FILEIO_STATUS_ERROR;
                }
            }
        }
        cpu_shared_mem->retValue = status;
    }

    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FSEEK)
    {
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FSEEK : File not open\n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;
        }
        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            cpu_shared_mem->retValue = fseek((FILE *)cpu_shared_mem->opStruct.fid, cpu_shared_mem->opStruct.offset, cpu_shared_mem->opStruct.count);
        }
        else
        {
            cpu_shared_mem->retValue = status;
        }
    }

    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FTELL)
    {
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FTELL : File not open\n");
            status = (int32_t)APP_FILEIO_STATUS_ERROR;
        }
        if(status == (int32_t)APP_FILEIO_STATUS_OK)
        {
            long int res = ftell((FILE *)cpu_shared_mem->opStruct.fid);
            cpu_shared_mem->retValue = res;
        }
        else
        {
            cpu_shared_mem->retValue = status;
        }
    }

/* The following commented out section of code contains support for more file operations, but it is not validated
    Can be updated and uncommented if required based on more validation */
#if 0
    else if (cpu_shared_mem->opStruct.opCode == TI_FILEIO_OPCODE_FGETS)
    {
        i = searchFileStruct(openFileInfo[cpu_id], cpu_shared_mem->opStruct.fid);
        if (i == -1) /* if file is not open */
        {
            appLogPrintf("FGETS : File not open\n");
        }

        fgets(buffer, cpu_shared_mem->opStruct.size, (FILE *)cpu_shared_mem->opStruct.fid)
    }
#endif
    else
    {
        appLogPrintf("UNSUPPORTED OP TYPE : \n");
        cpu_shared_mem->retValue = (int32_t)APP_FILEIO_STATUS_ERROR;
    }
    return status;
}

int appFileIORdInit(app_fileio_init_prm_t *prm)
{
    int32_t status = 0;
    uint32_t cpu_id;
    int32_t i;
    app_fileio_rd_obj_t *obj = &g_app_fileio_rd_obj;

    if(prm->shared_mem == NULL || prm->fileio_rd_max_cpus > APP_FILEIO_MAX_CPUS)
    {
        appLogPrintf("ERROR: APP_FILEIO_MEM_ADDR is NULL or fileio_rd_max_cpus is too large\n");
        status = -1;
    }

    if(status==0)
    {
        obj->shared_mem = appFileIOMemMap(prm->shared_mem, sizeof(app_fileio_shared_mem_t));

        if(obj->shared_mem!=NULL)
        {
            obj->fileio_rd_max_cpus = prm->fileio_rd_max_cpus;
            obj->fileio_rd_poll_interval_in_msecs = prm->fileio_rd_poll_interval_in_msecs;
            for(cpu_id=0; cpu_id<APP_FILEIO_MAX_CPUS; cpu_id++)
            {
                obj->fileio_rd_cpu_enable[cpu_id] = prm->fileio_rd_cpu_enable[cpu_id];
            }

            for(cpu_id=0; cpu_id<obj->fileio_rd_max_cpus; cpu_id++)
            {
                app_fileio_cpu_shared_mem_t *cpu_shared_mem;

                cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];
                cpu_shared_mem->writeIndicatorFlag = 0;

                for(i = 0; i < APP_FILEIO_MAX_NUM_FILES; i++)
                {
                    openFileInfo[cpu_id][i].available = 0;
                    openFileInfo[cpu_id][i].fd = 0U;
                }
            }
            /* task is never deleted to have file io till the very end of CPU shutdown */
            status = appFileIORdCreateTask(obj, prm);
        }
    }

    return status;
}

void appFileIORdRun(app_fileio_rd_obj_t * obj)
{
    uint32_t done = 0;
    uint32_t cpu_id;
    int32_t status = (int32_t)APP_FILEIO_STATUS_OK;

    while(done == 0)
    {
        appLogWaitMsecs(10);
        for(cpu_id=0; cpu_id<obj->fileio_rd_max_cpus; cpu_id++)
        {
            app_fileio_cpu_shared_mem_t *cpu_shared_mem;

            cpu_shared_mem = &obj->shared_mem->cpu_shared_mem[cpu_id];
            if(obj->fileio_rd_cpu_enable[cpu_id] == 1)
            {
                if (cpu_shared_mem->writeIndicatorFlag == 1)
                {
                    status = fileIOServer(cpu_shared_mem, cpu_id);
                    if(status == APP_FILEIO_STATUS_ERROR)
                    {
                        appLogPrintf("SERVER: ERROR on cpu_id : %d\n", cpu_id);       
                    }
                    cpu_shared_mem->writeIndicatorFlag = 0;
                }
            }
        }
    }
}

int32_t  appFileIODeInit()
{
    int32_t status = 0;
    /* task is never deleted to have file io till the very end of CPU shutdown */
    return status;
}
