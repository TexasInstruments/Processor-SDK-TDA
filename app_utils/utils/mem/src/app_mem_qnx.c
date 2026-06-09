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
 * have made, use, import, offer to sell and sell ("Utilize") this soFftware subject to the
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/mman_peer.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <hw/inout.h>
#include <string.h>
#include <process.h>
#include <pthread.h>
#include <app_mem_priv.h>
#include <utils/console_io/include/app_log.h>
#include <utils/mem/include/app_mem.h>
#include <SharedMemoryAllocatorUsr.h>
#include <CacheP.h>

//#define APP_MEM_DEBUG
/**
 * \brief Memory module initialization parameters
 */
typedef struct {

} app_qnx_init_prm_t;

uint32_t appMemGetNumMaps(void);
uint32_t appMemGetNumBufElements(void);

//static app_qnx_init_prm_t g_app_mem_init_prm;
#define MAX_BUFS 10240U

typedef struct
{
    pthread_mutex_t mem_mutex;

    uint32_t num_allocs;
    uint32_t num_maps;

    uint64_t base;                   /**< heap memory base address */
    uint32_t size;                   /**< heap size in bytes */
    uint32_t valid_base_mem;         /**< base memory values are to be used in checking region */

}app_mem_obj_t;


typedef struct
{
    /* Shared memory buffer */
    shm_buf buf;
    /* Value of 1 indicates that it is natively allocated */
    uint8_t native_alloc;
    /* Value of 0 indicates that it is not in use, value of 1 indicates that it is in use */
    uint8_t is_buf_used;
}app_mem_bufs_t;

/*GLOBALS*/
app_mem_obj_t g_app_mem_obj;

app_mem_bufs_t g_app_mem_bufs[MAX_BUFS];

/* Return true if specified memory region is enabled, return false otherwise*/
bool appMemRegionQuery(uint32_t heap_id)
{
    return ((heap_id == APP_MEM_HEAP_DDR) || (heap_id == APP_MEM_HEAP_DDR_SHARED));
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int32_t status = (int32_t)0;

    /* Initialize the pthread mutex */
    (void)pthread_mutex_init(&obj->mem_mutex, NULL);

    obj->num_allocs = (uint32_t)0U;
    obj->num_maps   = (uint32_t)0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Init ... !!!\n");
    #endif

    /* Get I/O privilege */
    (void)ThreadCtl (_NTO_TCTL_IO, 0);

    (void)memset(&g_app_mem_bufs[0], 0, (sizeof(app_mem_bufs_t) * MAX_BUFS));

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM01
<justification end> */
    if (NULL != prm)
/* LDRA_JUSTIFY_END */
    {
        obj->base = prm->base;
        obj->size = prm->size;
        obj->valid_base_mem = (uint32_t)1U;
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM01
<justification end> */
    else
    {
        obj->valid_base_mem = (uint32_t)0U;
    }
/* LDRA_JUSTIFY_END */

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Init ... Done !!!\n");
    #endif
    return(status);
}

int32_t appMemDeInit(void)
{
    int32_t status = 0;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Deinit ... empty function !!!\n");
    appLogPrintf("MEM: Deinit ... Done !!!\n");
    #endif

    return(status);
}


uint64_t appMemGetDmaBufFd(void *virPtr, volatile uint32_t *dmaBufFdOffset)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    uint64_t physAddr = (uint32_t)-1;
    uint32_t i = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemGetDmaBufFd ... !!!\n");
    #endif

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->mem_mutex);

    /* Find buffer */
    for(i=0U; i < MAX_BUFS; i++)
    {
        if((void *) g_app_mem_bufs[i].buf.vir_addr == virPtr)
        {
            break;
        }
    }
    if(i == MAX_BUFS)
    {
        #ifdef APP_MEM_DEBUG
        appLogPrintf("appMemGetDmaBufFd: Unable to locate buffer\n");
        #endif
    }
    else
    {
        physAddr = g_app_mem_bufs[i].buf.phy_addr;
    }
    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->mem_mutex);

    return(physAddr);
}

int32_t appMemTranslateDmaBufFd(uint64_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int32_t status = 0, free_buf_idx = -1;
    uint32_t i = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemTranslateDmaBufFd ... !!!\n");
    #endif

    /* If we have been given a valid region, check to see that the provided memory falls within the carveout space */
    if ( ((obj->valid_base_mem == 1U) &&
          ((dmaBufFd >= (uint64_t)(g_app_mem_obj.base)) && (dmaBufFd < (uint64_t)(g_app_mem_obj.base + g_app_mem_obj.size))) ) ||
         (obj->valid_base_mem == 0U) ) /* TIOVX-1940- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_QNX_MEM_UM13 */
    {
        /* Enter critical section */
        (void)pthread_mutex_lock(&obj->mem_mutex);

        /* Find buffer */
        for(i=0U; i < MAX_BUFS; i++)
        {
            if( (g_app_mem_bufs[i].buf.phy_addr == dmaBufFd) )
            {
                break;
            }
            else if ( (0U == g_app_mem_bufs[i].is_buf_used) &&
                      (-1 == free_buf_idx) )
            {
                free_buf_idx = (int32_t)i;
            }
            else
            {
                /* nothing to do */
            }
        }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM08
<justification end> */
        if( i == MAX_BUFS )
        {
            if (-1 == free_buf_idx)
            {
                #ifdef APP_MEM_DEBUG
                appLogPrintf("appMemTranslateDmaBufFd: Unable to locate buffer and no free buffers available\n");
                #endif
                status = (int32_t)-1;
            }
            else
            {
                i = (uint32_t)free_buf_idx;
            }
        }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM08
<justification end> */
        if ((int32_t)0==status)
/* LDRA_JUSTIFY_END */
        {
            if (1U == g_app_mem_bufs[i].is_buf_used)
            {
                *virtPtr = g_app_mem_bufs[i].buf.vir_addr;
            }
            else
            {
                /* Since the virtual pointer is not used in this process, map the virtual pointer to this process */
                *virtPtr = (uint64_t)mmap64(NULL, size, (int)((uint32_t)PROT_READ | (uint32_t)PROT_WRITE), (int)((uint32_t)MAP_PHYS | (uint32_t)MAP_SHARED), NOFD, (int64_t)dmaBufFd);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM11
<justification end> */
                if (virtPtr == MAP_FAILED)
                {
                    status = (int32_t)-1;
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM11
<justification end> */
                else
/* LDRA_JUSTIFY_END */
                {
                    int32_t pid;

                    /* Note: retaining the pid given that the shm_buf structure contains this parameter */
                    pid = getpid();

                    /* Virtual pointer in the buffer structure set to newly mapped virtual pointer and pid updated */
                    g_app_mem_bufs[i].buf.vir_addr = (uint64_t)(*virtPtr);
                    g_app_mem_bufs[i].buf.pid = pid;
                    g_app_mem_bufs[i].buf.size = (int)size;
                    g_app_mem_bufs[i].buf.phy_addr = dmaBufFd;
                    g_app_mem_bufs[i].native_alloc = 0U;
                    g_app_mem_bufs[i].is_buf_used = 1U;
                    obj->num_maps++;
                }
            }

            *phyPtr  = g_app_mem_bufs[i].buf.phy_addr;
        }

        /* Exit critical section */
        (void)pthread_mutex_unlock(&obj->mem_mutex);
    }
    else
    {
        status = -1;
        appLogPrintf("ERROR: appMemTranslateDmaBufFd: dmaBufFd falls outside of memory allocated in shared memory region!!!\n");
    }

    return(status);
}

/* Maintained for compatibility with Linux */
void appMemCloseDmaBufFd(uint64_t dmaBufFd)
{
    return;
}

uint64_t appMemGetVirt2PhyBufPtr(uint64_t virtPtr, uint32_t heap_id)
{
    off_t   phyPtr = 0;
    size_t  len = 0;
    int32_t ret = 0;
    int64_t offset = 0;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemGetVirt2PhyBufPtr ... !!!\n");
    #endif

    ret = mem_offset64((void *) virtPtr, NOFD, 4096, &offset, &len);
    if (0 != ret) {
        appLogPrintf("MEM: Error from mem_offset\n");
    }
    else
    {
        phyPtr = (off_t)offset;
    }

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: phyPtr 0x%x len/%d for virtPtr/0x%lx\n",(uint32_t) phyPtr, (int) len, virtPtr);
    #endif
    return (uint64_t)(phyPtr);
}

void *appMemAlloc(uint32_t block, uint32_t size, uint32_t align)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    void *virtPtr = 0;
    int32_t status = 0;
    uint32_t i = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemAlloc ... !!!\n");
    #endif

    /* On MPU, direct both shared and non-shared requests to the same
     * shared heap for backward compatibility */
    if((APP_MEM_HEAP_DDR == block) ||
       (APP_MEM_HEAP_DDR_SHARED == block))
    {
        /* Enter critical section */
        (void)pthread_mutex_lock(&obj->mem_mutex);

        /* Find empty buffer */
        for(i=0U;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM09
<justification end> */
        i < MAX_BUFS;
/* LDRA_JUSTIFY_END */
        i++)
        {
            if(g_app_mem_bufs[i].is_buf_used == 0U)
            {
                break;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM09
<justification end> */
        if (i < MAX_BUFS)
/* LDRA_JUSTIFY_END */
        {
            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Using buf/%d\n",i);
            #endif
            if(align > 0U) {
               status = SHM_alloc_aligned((int)size, align, &g_app_mem_bufs[i].buf);
               #ifdef APP_MEM_DEBUG
               appLogPrintf("\nMEM : SHM_alloc_aligned(%d, %d) : %s\n", size, align, status?"FAILED":"PASSED");
               #endif
            }
            else {
               status = SHM_alloc((int)size, &g_app_mem_bufs[i].buf);
               #ifdef APP_MEM_DEBUG
               appLogPrintf("\nMEM : SHM_alloc(%d) : %s\n", size, status?"FAILED":"PASSED");
               #endif
            }

            if(0!=status)
            {
               appLogPrintf("MEM: allocation of size %d failed with status %d\n", size, status);
            }
            else
            {
                virtPtr = (void *) (g_app_mem_bufs[i].buf.vir_addr);
                g_app_mem_bufs[i].native_alloc = 1U;
                g_app_mem_bufs[i].is_buf_used  = 1U;
                obj->num_allocs++;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM09
<justification end> */
        else
        {
            appLogPrintf("MEM: Error: Insufficient number of buffers available\n");
        }
/* LDRA_JUSTIFY_END */

        /* Exit critical section */
        (void)pthread_mutex_unlock(&obj->mem_mutex);

        #ifdef APP_MEM_DEBUG
        appLogPrintf("MEM: Allocated memory - virt base address = %p, phy/0x%08lx size = %d \n", (void *) virtPtr, g_app_mem_bufs[i].buf.phy_addr, size);
        #endif
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block\n");
    }

    return(virtPtr);
}

int32_t appMemFree(uint32_t block, void *virtPtr, uint32_t size )
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int32_t status = 0;
    uint32_t i = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemFree ... !!!\n");
    #endif

    /* On MPU, direct both shared and non-shared requests to the same
     * shared heap for backward compatibility */
    if(((APP_MEM_HEAP_DDR == block) ||
        (APP_MEM_HEAP_DDR_SHARED == block)) &&
        (virtPtr != NULL) && (size != 0U))
    {
        /* Enter critical section */
        (void)pthread_mutex_lock(&obj->mem_mutex);

        /* Find buffer */
        for(i=0U; i < MAX_BUFS; i++)
        {
            if( ((void *) g_app_mem_bufs[i].buf.vir_addr == virtPtr) )
            {
                break;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM10
<justification end> */
        if(i == MAX_BUFS)
        {
            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Unable to locate buffer to free\n");
            #endif
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM10
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Found virtPtr at buf/%d, virt/%p phy/0x%08lx size/%d\n",i, (void *) g_app_mem_bufs[i].buf.vir_addr, g_app_mem_bufs[i].buf.phy_addr, size);
            #endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM15
<justification end> */
            if (1U == g_app_mem_bufs[i].native_alloc)
/* LDRA_JUSTIFY_END */
            {
                status = SHM_release(&g_app_mem_bufs[i].buf);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM02
<justification end> */
                if(0!=status)
                {
                #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: unable to release shared memory virtual address/%p size %d \n", virtPtr, size);
                #endif
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM02
<justification end> */
                else
/* LDRA_JUSTIFY_END */
                {
                    g_app_mem_bufs[i].buf.vir_addr = 0;
                    /* Note: retaining the pid given that the shm_buf structure contains this parameter */
                    g_app_mem_bufs[i].buf.pid = 0;
                    g_app_mem_bufs[i].buf.size = 0;
                    g_app_mem_bufs[i].buf.phy_addr = 0;
                    g_app_mem_bufs[i].native_alloc = 0U;
                    g_app_mem_bufs[i].is_buf_used = 0U;
                    obj->num_allocs--;
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM15
<justification end> */
            else
            {
                status = munmap(virtPtr, size);

                if(0!=status)
                {
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: unable to unmap shared memory virtual address/%p size %d \n", virtPtr, size);
                    #endif
                }
                else
                {
                    g_app_mem_bufs[i].buf.vir_addr = 0;
                    /* Note: retaining the pid given that the shm_buf structure contains this parameter */
                    g_app_mem_bufs[i].buf.pid = 0;
                    g_app_mem_bufs[i].buf.size = 0;
                    g_app_mem_bufs[i].buf.phy_addr = 0;
                    g_app_mem_bufs[i].native_alloc = 0U;
                    g_app_mem_bufs[i].is_buf_used = 0U;
                    obj->num_maps--;
                }
            }
/* LDRA_JUSTIFY_END */
        }

        /* Exit critical section */
        (void)pthread_mutex_unlock(&obj->mem_mutex);
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block\n");
        status = -1;
    }

    return(status);
}

int32_t appMemStats(uint32_t block, app_mem_stats_t *stats)
{
    /* TBD */
    return(0);
}

void  appMemCacheInv(void *ptr, uint32_t size)
{
    /* Use the OSAL Cache APIs */
#ifdef SOC_FAMILY_TDA5
    CacheP_inv(ptr, size, 0);
#else
    CacheP_Inv(ptr, size);
#endif
}

void  appMemCacheWbInv(void *ptr, uint32_t size)
{
    /* Use the OSAL Cache APIs */
#ifdef SOC_FAMILY_TDA5
    CacheP_wbInv(ptr, size, 0);
#else
    CacheP_wbInv(ptr, size);
#endif
}

void  appMemCacheWb(void *ptr, uint32_t size)
{
    /* Use the OSAL Cache APIs */
#ifdef SOC_FAMILY_TDA5
    CacheP_wb(ptr, size, 0);
#else
    CacheP_wb(ptr, size);
#endif
}

/* Note: needed for linking tivx mem platform layer*/
int32_t appMemResetScratchHeap(uint32_t heap_id)
{
    return(-1);
}

void appMemPrintMemAllocInfo(void)
{
    uint32_t i, cur_bufs_used = 0, cur_num_allocs = 0, cur_num_maps = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->mem_mutex);

    cur_num_allocs = obj->num_allocs;
    cur_num_maps   = obj->num_maps;

    for(i=0U; i < MAX_BUFS; i++)
    {
        if(g_app_mem_bufs[i].is_buf_used == 1U)
        {
            cur_bufs_used++;
        }
    }

    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->mem_mutex);

    appLogPrintf("Total number of buffers allocated = %d\n", cur_num_allocs);
    appLogPrintf("Total number of buffers mapped = %d\n", cur_num_maps);
    appLogPrintf("Total number of buf array elements used = %d\n", cur_bufs_used);

    return;
}

uint32_t appMemGetNumAllocs(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t num_allocs;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->mem_mutex);

    num_allocs = obj->num_allocs;

    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->mem_mutex);

    return num_allocs;
}

uint32_t appMemGetNumMaps(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t num_maps;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->mem_mutex);

    num_maps = obj->num_maps;

    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->mem_mutex);

    return num_maps;
}

uint32_t appMemGetNumBufElements(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t cur_bufs_used = 0U, i;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->mem_mutex);

    for(i=0U; i < MAX_BUFS; i++)
    {
        if(g_app_mem_bufs[i].is_buf_used == 1U)
        {
            cur_bufs_used++;
        }
    }

    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->mem_mutex);

    return cur_bufs_used;
}

uint64_t appMemShared2PhysPtr(uint64_t shared_ptr, uint32_t heap_id)
{
    return shared_ptr;
}

uint64_t appMemShared2TargetPtr(uint64_t shared_ptr)
{
    return shared_ptr;
}

void *appMemMap(void *phys_ptr, uint32_t size)
{
    uint32_t  pageSize = (uint32_t)getpagesize();
    uintptr_t taddr;
    uint32_t  tsize;
    void     *virt_ptr = NULL;
#ifdef MEM_DEV_OPEN
    int32_t   status = 0;
    static int dev_mem_fd = -1;

    if(dev_mem_fd == -1)
    {
        dev_mem_fd = open("/dev/mem",O_RDWR|O_SYNC);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM03
<justification end> */
        if(dev_mem_fd  < 0)
        {
            appLogPrintf("APP_LOG: ERROR: Unable to open /dev/mem !!!\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }
    if(status==0
    && dev_mem_fd >= 0)
    {
        #ifdef APP_MEM_DEBUG
        appLogPrintf("APP_MEM: Mapping %p ...\n", phys_ptr);
        #endif
        /* Mapping this physical address to qnx user space */
        taddr = (uintptr_t)phys_ptr;
        tsize = size;

        /* Align the physical address to page boundary */
        tsize = appAlign(tsize + (taddr % pageSize), pageSize);
        taddr = appFloor(taddr, pageSize);

        virt_ptr  = mmap64(0, tsize,
                        (PROT_READ | PROT_WRITE | PROT_NOCACHE),
                        (MAP_SHARED), dev_mem_fd, taddr);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM04
<justification end> */
        if(virt_ptr==MAP_FAILED)
        {
            virt_ptr = NULL;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM04
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            virt_ptr = (void*)((uintptr_t)virt_ptr + ((uintptr_t)phys_ptr % pageSize));
        }
        #ifdef APP_MEM_DEBUG
        appLogPrintf("APP_MEM: Mapped %p -> %p of size %d bytes \n", phys_ptr, virt_ptr, size);
        #endif
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM05
<justification end> */
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_ptr, size);
    }
/* LDRA_JUSTIFY_END */
#else
    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: Mapping %p ...\n", phys_ptr);
    #endif

    /* Mapping this physical address to qnx user space */
    taddr = (uintptr_t)phys_ptr;
    tsize = size;

    /* Align the physical address to page boundary */
    tsize = (uint32_t)appAlign(tsize + (uint32_t)(taddr % pageSize), pageSize);
    taddr = appFloor(taddr, pageSize);


    virt_ptr  = mmap_device_memory(0, (size_t)tsize, (int32_t)((uint32_t)PROT_READ|(uint32_t)PROT_WRITE|(uint32_t)PROT_NOCACHE), 0, taddr);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM06
<justification end> */
    if(virt_ptr==MAP_FAILED)
    {
        virt_ptr = NULL;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM06
<justification end> */
    else
/* LDRA_JUSTIFY_END */
    {
        virt_ptr = (void*)((uintptr_t)virt_ptr + ((uintptr_t)phys_ptr % pageSize));
    }
    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: Mapped %p -> %p of size %d bytes \n", phys_ptr, virt_ptr, size);
    #endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM06
<justification end> */
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_ptr, size);
    }
/* LDRA_JUSTIFY_END */
#endif
    return virt_ptr;
}

int32_t appMemUnMap(void *virt_ptr, uint32_t size)
{
    int32_t status=0;
    uint32_t pageSize = (uint32_t)getpagesize();
    uintptr_t taddr;
    uint32_t  tsize;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: UnMapped memory at virtual address @ 0x%p of size %d bytes \n", virt_ptr, size);
    #endif

    taddr = (uint64_t)virt_ptr;
    tsize = size;

    tsize = (uint32_t)appAlign((tsize + (uint32_t)(taddr % pageSize)), pageSize);
    taddr = appFloor(taddr, pageSize);

#ifdef MEM_DEV_OPEN
    status = munmap((void *)taddr, tsize);
#else
    status = munmap_device_memory((void *)taddr, tsize);
#endif

    return status;
}
