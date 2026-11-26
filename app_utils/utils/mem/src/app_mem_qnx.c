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
#include <stdbool.h>
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


//static app_qnx_init_prm_t g_app_mem_init_prm;
#define MAX_BUFS 10240U
#define MAX_HEAP_REGIONS (8u)

typedef struct
{
    uint64_t base_phys_addr;   /* base physical addr of this heap/carveout */
    uint64_t virt_addr;        /* valid only when is_imported==true: whole-region mapped virt base */
    uint64_t size;              /* heap size in bytes */
    uint32_t allow_alloc;       /* 1 = this host may SHM_alloc from it, 0 = remote-only/imported */
    uint32_t heap_id;           /* Logical heap this slot serves, see APP_MEM_HEAP_* */
    bool     in_use;             /* Is this heap slot initialized? */
    bool     is_imported;        /* true if whole-region mmap'd from prm->base (remote-only heap) */
    char     name[APP_MEM_HEAP_NAME_MAX]; /* Heap name for debugging */

    /* Per-heap statistics */
    uint64_t heap_alloc_bytes;      /* Total bytes allocated in this heap */
    uint64_t heap_free_bytes;       /* Total bytes freed in this heap */
    uint64_t heap_cur_alloc_bytes;  /* Currently allocated bytes in this heap */
    uint64_t heap_num_alloc;        /* Number of allocations in this heap */
    uint64_t heap_num_free;         /* Number of frees in this heap */
    uint64_t heap_cur_alloc;        /* Current number of active allocations */
}app_mem_heap_obj_t;

typedef struct
{
    pthread_mutex_t mem_mutex;      /* protects g_app_mem_bufs, num_allocs/num_maps */
    pthread_mutex_t global_mutex;   /* protects heaps[]/num_heaps and per-heap stats */

    uint32_t num_allocs;
    uint32_t num_maps;

    app_mem_heap_obj_t heaps[MAX_HEAP_REGIONS];
    uint32_t num_heaps;

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

/*INTERNAL FUNCTION DEFINITIONS*/
static void appMemUpdateHeapFreeStats(uint32_t heap_id, uint64_t phy_addr, uint32_t size);

/*GLOBALS*/
app_mem_obj_t g_app_mem_obj;

app_mem_bufs_t g_app_mem_bufs[MAX_BUFS];

/* Return true if specified memory region is enabled, return false otherwise*/
bool appMemRegionQuery(uint32_t heap_id)
{
    return ((heap_id == APP_MEM_HEAP_DDR) || (heap_id == APP_MEM_HEAP_DDR_SHARED) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST));
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    app_mem_heap_obj_t *heap = NULL;
    uint32_t heap_idx = 0U;
    int32_t status = (int32_t)0;

    static bool global_initialized = false;

    if (!global_initialized)
    {
        (void)pthread_mutex_init(&obj->mem_mutex, NULL);
        (void)pthread_mutex_init(&obj->global_mutex, NULL);

        obj->num_allocs = (uint32_t)0U;
        obj->num_maps   = (uint32_t)0U;
        obj->num_heaps  = (uint32_t)0U;

        for (uint32_t i = 0U; i < MAX_HEAP_REGIONS; i++)
        {
            obj->heaps[i].in_use = false;
        }

        /* Get I/O privilege */
        (void)ThreadCtl (_NTO_TCTL_IO, 0);

        (void)memset(&g_app_mem_bufs[0], 0, (sizeof(app_mem_bufs_t) * MAX_BUFS));

        global_initialized = true;

        #ifdef APP_MEM_DEBUG
        appLogPrintf("MEM: Global init ... !!!\n");
        #endif
    }

    if (NULL == prm)
    {
        appLogPrintf("MEM: ERROR: appMemInit: NULL prm !!!\n");
        status = (int32_t)-1;
    }

    if ((int32_t)0 == status)
    {
        if (pthread_mutex_lock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }

        if (obj->num_heaps >= MAX_HEAP_REGIONS)
        {
            appLogPrintf("MEM: ERROR: Maximum number of heap regions (%d) reached!\n", MAX_HEAP_REGIONS);
            status = (int32_t)-1;
        }

        if ((int32_t)0 == status)
        {
            if ((prm->heap_id != APP_MEM_HEAP_DDR_SHARED) &&
                (prm->heap_id != APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) &&
                (prm->heap_id != APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST))
            {
                appLogPrintf("MEM: ERROR: Invalid heap_id %d - must be APP_MEM_HEAP_DDR_SHARED, APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY, or APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST!\n",
                            prm->heap_id);
                status = (int32_t)-1;
            }
        }

        if ((int32_t)0 == status)
        {
            for (heap_idx = 0U; heap_idx < MAX_HEAP_REGIONS; heap_idx++)
            {
                if (!obj->heaps[heap_idx].in_use)
                {
                    heap = &obj->heaps[heap_idx];
                    break;
                }
            }

            if (NULL == heap)
            {
                appLogPrintf("MEM: ERROR: No available heap slot found!\n");
                status = (int32_t)-1;
            }
        }

        if ((int32_t)0 == status)
        {
            heap->base_phys_addr = prm->base;
            heap->virt_addr = (uint64_t)0U;
            heap->size = prm->size;
            heap->allow_alloc = (prm->flags == 0U) ? (uint32_t)1U : (uint32_t)0U;
            heap->heap_id = prm->heap_id;
            heap->is_imported = false;
            (void)snprintf(heap->name, sizeof(heap->name), "%s", prm->name);

            heap->heap_alloc_bytes = (uint64_t)0U;
            heap->heap_free_bytes = (uint64_t)0U;
            heap->heap_cur_alloc_bytes = (uint64_t)0U;
            heap->heap_num_alloc = (uint64_t)0U;
            heap->heap_num_free = (uint64_t)0U;
            heap->heap_cur_alloc = (uint64_t)0U;

            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Init heap[%d] '%s' (heap_id=%d) ... !!!\n", heap_idx, heap->name, heap->heap_id);
            #endif
        }

        if (pthread_mutex_unlock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    if (((int32_t)0 == status) && (heap->allow_alloc == 0U))
    {
        /* Remote-only heap: this host has no allocation rights on it, the owning
         * host's SHM resmgr block already covers the underlying physical carveout.
         * The base/size is already known statically via 'prm', so simply map the
         * whole region once so that shared pointers falling inside it can be
         * translated to a locally-usable virtual pointer. */
        void *virt_ptr;

        virt_ptr = mmap64(NULL, heap->size, (int)((uint32_t)PROT_READ | (uint32_t)PROT_WRITE),
                           (int)((uint32_t)MAP_PHYS | (uint32_t)MAP_SHARED), NOFD, (int64_t)prm->base);

        if (virt_ptr == MAP_FAILED)
        {
            appLogPrintf("MEM: ERROR: Failed to mmap remote-only heap '%s' at phys 0x%lx size %lu !!!\n",
                          heap->name, (uint64_t)prm->base, heap->size);
            status = (int32_t)-1;
        }
        else
        {
            heap->virt_addr = (uint64_t)virt_ptr;
            heap->is_imported = true;

            appLogPrintf("MEM: Heap '%s' - Imported remote-only heap region\n", heap->name);
            appLogPrintf("MEM:   Physical base: 0x%lx\n", heap->base_phys_addr);
            appLogPrintf("MEM:   Virtual base:  0x%lx\n", heap->virt_addr);
            appLogPrintf("MEM:   Size:          0x%lx\n", heap->size);
            appLogPrintf("MEM:   Allow alloc:   NO\n");
        }

        if ((int32_t)0 != status)
        {
            /* Roll back - do not mark this heap slot in_use */
            heap = NULL;
        }
    }

    if (((int32_t)0 == status) && (NULL != heap))
    {
        if (pthread_mutex_lock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        heap->in_use = true;
        obj->num_heaps++;
        if (pthread_mutex_unlock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }

        appLogPrintf("MEM: Init heap[%d] '%s' ... Done !!!\n", heap_idx, heap->name);
    }

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Init ... Done !!!\n");
    #endif

    return(status);
}

int32_t appMemDeInit(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    int32_t status = 0;
    uint32_t i;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Deinit ... !!!\n");
    #endif

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    for (i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];

        if (heap->in_use)
        {
            if (heap->is_imported && (heap->virt_addr != (uint64_t)0U))
            {
                (void)munmap((void *)heap->virt_addr, heap->size);
            }

            heap->in_use = false;
            heap->is_imported = false;
            heap->virt_addr = (uint64_t)0U;
        }
    }

    obj->num_heaps = 0U;

    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    #ifdef APP_MEM_DEBUG
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
    if (pthread_mutex_lock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

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
    if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return(physAddr);
}

int32_t appMemTranslateDmaBufFd(uint64_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int32_t status = 0, free_buf_idx = -1;
    uint32_t i = 0U;
    bool valid_range = false;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemTranslateDmaBufFd ... !!!\n");
    #endif

    /* If no heaps have been registered, preserve legacy unrestricted behavior.
     * Otherwise, check that the provided memory falls within one of the
     * registered heap/carveout regions. */
    if (obj->num_heaps == 0U)
    {
        valid_range = true;
    }
    else
    {
        if (pthread_mutex_lock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        for (i = 0U; i < MAX_HEAP_REGIONS; i++)
        {
            app_mem_heap_obj_t *heap = &obj->heaps[i];

            if (heap->in_use &&
                (dmaBufFd >= heap->base_phys_addr) &&
                (dmaBufFd < (heap->base_phys_addr + heap->size)))
            {
                valid_range = true;
                break;
            }
        }
        if (pthread_mutex_unlock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    if (valid_range)
    {
        /* Enter critical section */
        if (pthread_mutex_lock(&obj->mem_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }

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

        if ((int32_t)0==status)
        {
            if (1U == g_app_mem_bufs[i].is_buf_used)
            {
                *virtPtr = g_app_mem_bufs[i].buf.vir_addr;
            }
            else
            {
                /* Since the virtual pointer is not used in this process, map the virtual pointer to this process */
                *virtPtr = (uint64_t)mmap64(NULL, size, (int)((uint32_t)PROT_READ | (uint32_t)PROT_WRITE), (int)((uint32_t)MAP_PHYS | (uint32_t)MAP_SHARED), NOFD, (int64_t)dmaBufFd);

                if (virtPtr == MAP_FAILED)
                {
                    status = (int32_t)-1;
                }
                else
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
        if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
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

void *appMemAlloc(uint32_t heap_id, uint32_t size, uint32_t align)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    void *virtPtr = 0;
    int32_t status = 0;
    uint32_t i = 0U;
    app_mem_heap_obj_t *heap = NULL;
    uint32_t heap_idx = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemAlloc ... !!!\n");
    #endif

    if ((APP_MEM_HEAP_DDR_SHARED == heap_id) || (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY == heap_id) ||
        (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST == heap_id))
    {
        /* Pick the first heap that this host is allowed to allocate from.
         * The underlying QNX SHM block has no concept of a per-heap size
         * quota (it is one shared arena), so enforce the size passed to
         * appMemInit() for this heap here in software, and refuse the
         * allocation if it would exceed that heap's configured budget. */
        if (pthread_mutex_lock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        for (heap_idx = 0U; heap_idx < MAX_HEAP_REGIONS; heap_idx++)
        {
            if (obj->heaps[heap_idx].in_use && (obj->heaps[heap_idx].allow_alloc == 1U) &&
                (obj->heaps[heap_idx].heap_id == heap_id) &&
                ((obj->heaps[heap_idx].heap_cur_alloc_bytes + size) <= obj->heaps[heap_idx].size))
            {
                heap = &obj->heaps[heap_idx];
                break;
            }
        }
        if (pthread_mutex_unlock(&obj->global_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }

        if (NULL == heap)
        {
            appLogPrintf("MEM: ERROR: No heap available for allocation of size %d from heap_id %d! All \
                          heaps are remote-only, uninitialized, mismatched, or over their configured size \
                          quota.\n", size, heap_id);
        }
        else
        {
            /* Enter critical section */
            if (pthread_mutex_lock(&obj->mem_mutex) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
            }

            /* Find empty buffer */
            for(i=0U; i < MAX_BUFS; i++)
            {
                if(g_app_mem_bufs[i].is_buf_used == 0U)
                {
                    break;
                }
            }
            if (i < MAX_BUFS)
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
            else
            {
                appLogPrintf("MEM: Error: Insufficient number of buffers available\n");
            }

            /* Exit critical section */
            if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
            }

            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Allocated memory - virt base address = %p, size = %d \n", (void *) virtPtr, size);
            #endif

            if ((0 == status) && (NULL != virtPtr))
            {
                if (pthread_mutex_lock(&obj->global_mutex) != 0)
                {
                    appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
                }
                heap->heap_alloc_bytes += size;
                heap->heap_cur_alloc_bytes += size;
                heap->heap_num_alloc++;
                heap->heap_cur_alloc++;
                if (pthread_mutex_unlock(&obj->global_mutex) != 0)
                {
                    appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
                }
            }
        }
    }
    else if (APP_MEM_HEAP_DDR == heap_id)
    {
        virtPtr = malloc(size);
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block/heap_id\n");
    }

    return(virtPtr);
}

static void appMemUpdateHeapFreeStats(uint32_t heap_id, uint64_t phy_addr, uint32_t size)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t i;

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    for (i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];

        if (heap->in_use && (heap->allow_alloc == 1U) &&
            (heap->heap_id == heap_id) &&
            (phy_addr >= heap->base_phys_addr) &&
            (phy_addr < (heap->base_phys_addr + heap->size)))
        {
            heap->heap_free_bytes += size;
            heap->heap_num_free++;

            if ((heap->heap_cur_alloc_bytes < size) || (heap->heap_cur_alloc == 0U))
            {
                appLogPrintf("MEM: WARNING: Free'ing more memory than allocated in heap[%d] '%s'\n", i, heap->name);
            }
            else
            {
                heap->heap_cur_alloc_bytes -= size;
                heap->heap_cur_alloc--;
            }
            break;
        }
    }

    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }
}

int32_t appMemFree(uint32_t heap_id, void *ptr, uint32_t size)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int32_t status = 0;
    uint32_t i = 0U;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: appMemFree ... !!!\n");
    #endif

    if(((APP_MEM_HEAP_DDR_SHARED == heap_id) || (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY == heap_id) ||
        (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST == heap_id)) &&
        (ptr != NULL) && (size != 0U))
    {
        /* Enter critical section */
        if (pthread_mutex_lock(&obj->mem_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }

        /* Find buffer */
        for(i=0U; i < MAX_BUFS; i++)
        {
            if( ((void *) g_app_mem_bufs[i].buf.vir_addr == ptr) )
            {
                break;
            }
        }
        if(i == MAX_BUFS)
        {
            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Unable to locate buffer to free\n");
            #endif
        }
        else
        {
            #ifdef APP_MEM_DEBUG
            appLogPrintf("MEM: Found virtPtr at buf/%d, virt/%p phy/0x%08lx size/%d\n",i, (void *) g_app_mem_bufs[i].buf.vir_addr, g_app_mem_bufs[i].buf.phy_addr, size);
            #endif
            if (1U == g_app_mem_bufs[i].native_alloc)
            {
                uint64_t freed_phy_addr = g_app_mem_bufs[i].buf.phy_addr;

                status = SHM_release(&g_app_mem_bufs[i].buf);
                if(0!=status)
                {
                #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: unable to release shared memory virtual address/%p size %d \n", ptr, size);
                #endif
                }
                else
                {
                    g_app_mem_bufs[i].buf.vir_addr = 0;
                    /* Note: retaining the pid given that the shm_buf structure contains this parameter */
                    g_app_mem_bufs[i].buf.pid = 0;
                    g_app_mem_bufs[i].buf.size = 0;
                    g_app_mem_bufs[i].buf.phy_addr = 0;
                    g_app_mem_bufs[i].buf.blockID = 0;
                    g_app_mem_bufs[i].native_alloc = 0U;
                    g_app_mem_bufs[i].is_buf_used = 0U;
                    obj->num_allocs--;

                    appMemUpdateHeapFreeStats(heap_id, freed_phy_addr, size);
                }
            }
            else
            {
                status = munmap(ptr, size);

                if(0!=status)
                {
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: unable to unmap shared memory virtual address/%p size %d \n", ptr, size);
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
        }

        /* Exit critical section */
        if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }
    else if (APP_MEM_HEAP_DDR == heap_id)
    {
        if ( ptr != NULL )
        {
            free(ptr);
        }
        else
        {
            appLogPrintf("MEM: ERROR: NULL Pointer\n");
            status = -1;
        }
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block/heap_id\n");
        status = -1;
    }

    return(status);
}

int32_t appMemStats(uint32_t heap_id, app_mem_stats_t *stats)
{
    if (stats != NULL)
    {
        if ((heap_id == APP_MEM_HEAP_DDR_SHARED) || (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST))
        {
            app_mem_obj_t *obj = &g_app_mem_obj;
            const char *heap_name_str;
            uint64_t total_size = 0;
            uint64_t used_size = 0;
            uint32_t i;

            if (heap_id == APP_MEM_HEAP_DDR_SHARED)
            {
                heap_name_str = "DDR_SHARED";
            }
            else if (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY)
            {
                heap_name_str = "DDR_SHARED_64B";
            }
            else
            {
                heap_name_str = "DDR_SHARED_64BC";
            }

            stats->heap_id = heap_id;
            (void)strncpy(stats->heap_name, heap_name_str, APP_MEM_HEAP_NAME_MAX);
            stats->heap_name[APP_MEM_HEAP_NAME_MAX-1u]=(char)0;

            (void)pthread_mutex_lock(&obj->global_mutex);
            for (i = 0U; i < MAX_HEAP_REGIONS; i++)
            {
                if ((obj->heaps[i].in_use) && (obj->heaps[i].heap_id == heap_id))
                {
                    total_size += obj->heaps[i].size;
                    used_size += obj->heaps[i].heap_cur_alloc_bytes;
                }
            }
            (void)pthread_mutex_unlock(&obj->global_mutex);

            stats->heap_size = total_size;
            stats->free_size = (total_size > used_size) ?
                            (total_size - used_size) : 0U;
        }
        else
        {
            stats->heap_id = heap_id;
            (void)strncpy(stats->heap_name, "INVALID", APP_MEM_HEAP_NAME_MAX);
            stats->heap_name[APP_MEM_HEAP_NAME_MAX-1u]=(char)0;
            stats->heap_size = 0;
            stats->free_size = 0;
        }
    }

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

uint8_t appMemIsCharInit(void)
{
    /* rpmsg char is not relevant for QNX */
    return 1U;
}

void appMemPrintMemAllocInfo(void)
{
    uint32_t i, cur_bufs_used = 0, cur_num_allocs = 0, cur_num_maps = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;

    /* Enter critical section */
    if (pthread_mutex_lock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

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
    if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    appLogPrintf("Total number of buffers allocated = %d\n", cur_num_allocs);
    appLogPrintf("Total number of buffers mapped = %d\n", cur_num_maps);
    appLogPrintf("Total number of buf array elements used = %d\n", cur_bufs_used);

    appLogPrintf("=== Heap Info ===\n");
    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }
    for (i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];

        if (heap->in_use)
        {
            appLogPrintf("HEAP[%d] '%s' (heap_id=%d, base=0x%lx, size=%luKB, alloc=%s):\n",
                         i, heap->name, heap->heap_id, heap->base_phys_addr,
                         heap->size / 1024U, (heap->allow_alloc == 1U) ? "YES" : "NO");
            appLogPrintf("  Alloc's: %ld alloc's of %ld bytes\n",
                         heap->heap_num_alloc, heap->heap_alloc_bytes);
            appLogPrintf("  Free's : %ld free's  of %ld bytes\n",
                         heap->heap_num_free, heap->heap_free_bytes);
            appLogPrintf("  Open's : %ld allocs  of %ld bytes\n",
                         heap->heap_cur_alloc, heap->heap_cur_alloc_bytes);
        }
    }
    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return;
}

uint32_t appMemGetNumAllocs(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t num_allocs;

    /* Enter critical section */
    if (pthread_mutex_lock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    num_allocs = obj->num_allocs;

    /* Exit critical section */
    if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return num_allocs;
}

uint32_t appMemGetNumMaps(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t num_maps;

    /* Enter critical section */
    if (pthread_mutex_lock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    num_maps = obj->num_maps;

    /* Exit critical section */
    if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return num_maps;
}

uint32_t appMemGetNumBufElements(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t cur_bufs_used = 0U, i;

    /* Enter critical section */
    if (pthread_mutex_lock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    for(i=0U; i < MAX_BUFS; i++)
    {
        if(g_app_mem_bufs[i].is_buf_used == 1U)
        {
            cur_bufs_used++;
        }
    }

    /* Exit critical section */
    if (pthread_mutex_unlock(&obj->mem_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return cur_bufs_used;
}

uint64_t appMemShared2PhysPtr(uint64_t shared_ptr, uint32_t heap_id)
{
    return shared_ptr;
}

uint64_t appMemShared2TargetPtr(uint64_t shared_ptr)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint64_t virt_addr = shared_ptr;
    uint32_t i;

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

    for (i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];

        if (heap->in_use && heap->is_imported &&
            (shared_ptr >= heap->base_phys_addr) &&
            (shared_ptr < (heap->base_phys_addr + heap->size)))
        {
            virt_addr = heap->virt_addr + (shared_ptr - heap->base_phys_addr);
            break;
        }
    }

    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    return virt_addr;
}

void *appMemMap(void *phys_addr, uint32_t size)
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
        appLogPrintf("APP_MEM: Mapping %p ...\n", phys_addr);
        #endif
        /* Mapping this physical address to qnx user space */
        taddr = (uintptr_t)phys_addr;
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
            virt_ptr = (void*)((uintptr_t)virt_ptr + ((uintptr_t)phys_addr % pageSize));
        }
        #ifdef APP_MEM_DEBUG
        appLogPrintf("APP_MEM: Mapped %p -> %p of size %d bytes \n", phys_addr, virt_ptr, size);
        #endif
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM05
<justification end> */
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_addr, size);
    }
/* LDRA_JUSTIFY_END */
#else
    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: Mapping %p ...\n", phys_addr);
    #endif

    /* Mapping this physical address to qnx user space */
    taddr = (uintptr_t)phys_addr;
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
        virt_ptr = (void*)((uintptr_t)virt_ptr + ((uintptr_t)phys_addr % pageSize));
    }
    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: Mapped %p -> %p of size %d bytes \n", phys_addr, virt_ptr, size);
    #endif
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_QNX_MEM_UM06
<justification end> */
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_addr, size);
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
