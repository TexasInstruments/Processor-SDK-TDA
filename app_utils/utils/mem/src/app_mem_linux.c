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
/*INCLUDES*/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ti_rpmsg_char.h>

#include <app_mem_priv.h>
#include <utils/console_io/include/app_log.h>
#include <utils/mem/include/linux/dma_buf_phys.h>
#include <utils/mem/include/linux/dma-heap-export.h>
#include <utils/mem/include/app_mem.h>
#include <linux/dma-heap.h>
#include <linux/dma-buf.h>

/*MACROS*/
/* #define APP_MEM_DEBUG */
#define DMA_HEAP_ALLOC_FLAGS   (0u)
#define MAX_HEAP_REGIONS       (8u)

/*STRUCTURES*/

typedef struct
{
    uint32_t dma_buf_fd;
    int32_t  dmabuf2phys_fd;
    uint64_t phys_addr;
    uint64_t virt_addr;
    uint64_t offset;
    uint64_t size;
}app_mem_type_t;

typedef struct mem_type_node
{
    app_mem_type_t mem_data;
    struct mem_type_node *next;
} app_mem_list_t;


typedef struct
{
    app_mem_list_t *plist;          /* Linked list of allocated buffers in this heap */
    pthread_mutex_t list_mutex;     /* Mutex for this heap's list */
    int dma_heap_fd;                /* File descriptor for the dma heap device */
    uint32_t allow_alloc;           /* 1 = MPU can allocate, 0 = only remote cores allocate */
    uint32_t heap_id;               /* Logical heap this slot serves, see APP_MEM_HEAP_* */
    uint64_t base_phys_addr;        /* Base physical address of this heap region */
    uint64_t size;                  /* Size of this heap region in bytes */
    bool in_use;                    /* Is this heap slot initialized? */
    char name[APP_MEM_HEAP_NAME_MAX]; /* Heap name for debugging */

    /* Per-heap statistics */
    uint64_t heap_alloc_bytes;      /* Total bytes allocated in this heap */
    uint64_t heap_free_bytes;       /* Total bytes freed in this heap */
    uint64_t heap_cur_alloc_bytes;  /* Currently allocated bytes in this heap */
    uint64_t heap_num_alloc;        /* Number of allocations in this heap */
    uint64_t heap_num_free;         /* Number of frees in this heap */
    uint64_t heap_cur_alloc;        /* Current number of active allocations */
} app_mem_heap_obj_t;

typedef struct
{
    app_mem_heap_obj_t heaps[MAX_HEAP_REGIONS];  /* Array of heap regions */
    uint32_t num_heaps;                          /* Number of initialized heaps */
    pthread_mutex_t global_mutex;                /* Mutex for global operations */

    /* Global statistics across all heaps */
    uint64_t total_alloc_bytes;
    uint64_t total_free_bytes;
    uint64_t cur_alloc_bytes;
    uint64_t num_alloc;
    uint64_t num_free;
    uint64_t cur_alloc;
}app_mem_obj_t;

/*GLOBALS*/
app_mem_obj_t g_app_mem_obj;
uint8_t g_char_init_done = 0U;

/*INTERNAL FUNCTION DEFINITIONS*/
static void appMemRemoveFromList(app_mem_list_t **list, app_mem_list_t *node);
static int  appMemPushToList(app_mem_list_t **list, app_mem_type_t *data);
static int  appMemDmaHeapGetPhys(int phys_fd, int fd, uint64_t *phys);
static void appMemFreeList(uint32_t heap_idx, app_mem_list_t *list);
static int32_t appMemAddTupleToList(uint32_t heap_idx, uint32_t dmaBufFd, uint64_t size, uint64_t *virtPtr, uint64_t *phyPtr);
static void appMemFreeListItem(uint32_t heap_idx, app_mem_list_t *mem_map_list);
static int appMemDmaHeapAlloc(int dma_heap_fd, size_t len, unsigned int flags, int *dma_buf_fd);
static int appMemDmaHeapExport(int dma_heap_fd, uint64_t offset, size_t len, unsigned int flags, int *dma_buf_fd);

bool appMemRegionQuery(uint32_t heap_id)
{
    return ((heap_id == APP_MEM_HEAP_DDR) || (heap_id == APP_MEM_HEAP_DDR_SHARED) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST));
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    int32_t status = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;
    app_mem_heap_obj_t *heap = NULL;
    uint32_t heap_idx = 0U;
    char dma_carveout_name[APP_MEM_DMA_CARVEOUT_NAME_MAX];

    static bool global_initialized = false;
    if (!global_initialized)
    {
        obj->num_heaps = 0U;
        obj->total_alloc_bytes = 0U;
        obj->total_free_bytes = 0U;
        obj->cur_alloc_bytes = 0U;
        obj->num_alloc = 0U;
        obj->num_free = 0U;
        obj->cur_alloc = 0U;
        (void)pthread_mutex_init(&obj->global_mutex, NULL);

        for (uint32_t i = 0U; i < MAX_HEAP_REGIONS; i++)
        {
            obj->heaps[i].in_use = false;
            obj->heaps[i].dma_heap_fd = -1;
        }

        global_initialized = true;
        appLogPrintf("MEM: Global init ... !!!\n");
    }

    if (NULL == prm)
    {
        appLogPrintf("MEM: ERROR: appMemInit: NULL prm !!!\n");
        status = (int32_t)-1;
    }

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM16
<justification end>*/
    if (obj->num_heaps >= MAX_HEAP_REGIONS)
    {
        appLogPrintf("MEM: ERROR: Maximum number of heap regions (%d) reached!\n", MAX_HEAP_REGIONS);
        status = -1;
    }

    if (status == 0)
    {
        if ((prm->heap_id != APP_MEM_HEAP_DDR_SHARED) &&
            (prm->heap_id != APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) &&
            (prm->heap_id != APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST))
        {
            appLogPrintf("MEM: ERROR: Invalid heap_id %d - must be APP_MEM_HEAP_DDR_SHARED, APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY, or APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST!\n",
                        prm->heap_id);
            status = -1;
        }
    }

    if (status == 0)
    {
        for (heap_idx = 0U; heap_idx < MAX_HEAP_REGIONS; heap_idx++)
        {
            if (!obj->heaps[heap_idx].in_use)
            {
                heap = &obj->heaps[heap_idx];
                break;
            }
        }

        if (heap == NULL)
        {
            appLogPrintf("MEM: ERROR: No available heap slot found!\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }
    if (status == 0)
    {
        heap->plist = NULL;
        heap->dma_heap_fd = -1;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM15
<justification end>*/
        heap->allow_alloc = (prm->flags == 0) ? 1 : 0;  /* flags=0 means MPU can allocate */
/* LDRA_JUSTIFY_END */
        heap->heap_id = prm->heap_id;
        heap->base_phys_addr = prm->base;
        heap->size = prm->size;
        heap->heap_alloc_bytes = 0;
        heap->heap_free_bytes = 0;
        heap->heap_cur_alloc_bytes = 0;
        heap->heap_num_alloc = 0;
        heap->heap_num_free = 0;
        heap->heap_cur_alloc = 0;
        (void)snprintf(heap->name, sizeof(heap->name), "%s", prm->name);

        (void)pthread_mutex_init(&heap->list_mutex, NULL);

        appLogPrintf("MEM: Init heap[%d] '%s' (heap_id=%d) ... !!!\n", heap_idx, heap->name, heap->heap_id);
    }

    if (status == 0)
    {
        (void)snprintf(dma_carveout_name, sizeof(dma_carveout_name), "%s", prm->dma_carveout_name);
        heap->dma_heap_fd = open(dma_carveout_name, (int)((uint32_t)O_RDONLY | (uint32_t)O_CLOEXEC));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM17
<justification end>*/
        if (heap->dma_heap_fd < 0)
        {
            appLogPrintf("MEM: ERROR: Failed to initialize DMA HEAP [%s] !!!\n", dma_carveout_name);
            status = -1;
        }
        else
/* LDRA_JUSTIFY_END */
        {
            appLogPrintf("MEM: Initialized DMA HEAP (fd=%d) for heap '%s' !!!\n", heap->dma_heap_fd, heap->name);
        }
    }

    if (status == 0)
    {
        heap->in_use = true;
        obj->num_heaps++;
    }

    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM13
<justification end>*/
    if (status == 0)
    {
        if (heap->allow_alloc == 0U)
        {
            int dma_buf_fd = -1;
            uint64_t DDR_shared_addr_virt = 0;
            uint64_t DDR_shared_addr_phys = 0;
            uint32_t flags = DMA_HEAP_ALLOC_FLAGS;
            uint64_t offset;

            if (prm->start < prm->base)
            {
                appLogPrintf("MEM: ERROR: Invalid carveout config - start (0x%lx) < base (0x%lx) !!!\n",
                            (uint64_t)prm->start, (uint64_t)prm->base);
                (void)close(heap->dma_heap_fd);
                heap->dma_heap_fd = -1;
                status = -1;
            }
            else
            {
                offset = (prm->start - prm->base);
            }

            if (status == 0)
            {
                status = appMemDmaHeapExport(heap->dma_heap_fd,
                                            offset,
                                            prm->size,
                                            flags,
                                            &dma_buf_fd);

                if (status != 0)
                {
                    appLogPrintf("MEM: ERROR: Failed to export DMA HEAP at offset 0x%lx [%s] !!!\n",
                                (uint64_t)prm->base, dma_carveout_name);
                    (void)close(heap->dma_heap_fd);
                    heap->dma_heap_fd = -1;
                }
            }

            if (status == 0)
            {
                status = appMemAddTupleToList(heap_idx, (uint32_t)dma_buf_fd, prm->size,
                                            &DDR_shared_addr_virt, &DDR_shared_addr_phys);
                if (status != 0)
                {
                    appLogPrintf("MEM: ERROR: Failed to map DMA HEAP export !!!\n");
                    (void)close(heap->dma_heap_fd);
                    heap->dma_heap_fd = -1;
                }
            }

            if (status == 0)
            {
                appLogPrintf("MEM: Heap '%s' - Exported DMA heap region\n", heap->name);
                appLogPrintf("MEM:   Carveout physical base: 0x%lx\n",(int64_t)prm->base);
                appLogPrintf("MEM:   Heap physical start:    0x%lx\n",(int64_t)DDR_shared_addr_phys);
                appLogPrintf("MEM:   Virtual start:          0x%lx\n",(int64_t)DDR_shared_addr_virt);
                appLogPrintf("MEM:   Size:                   0x%lx\n",prm->size);
                appLogPrintf("MEM:   Allow A72 alloc:        %s\n", (heap->allow_alloc == 1U) ? "YES" : "NO");
            }
            else
            {
                if (pthread_mutex_lock(&obj->global_mutex) != 0)
                {
                    appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
                }
                heap->in_use = false;
                if (obj->num_heaps > 0U)
                {
                    obj->num_heaps--;
                }
                if (pthread_mutex_unlock(&obj->global_mutex) != 0)
                {
                    appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
                }
            }
        }
    }
/* LDRA_JUSTIFY_END */

    appLogPrintf("MEM: Init ... Done !!!\n");

    if (status == 0)
    {
        if (g_char_init_done == 0U)
        {
            status = rpmsg_char_init(NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM14
<justification end>*/
            if (status != 0)
            {
                appLogPrintf("MEM: ERROR: rpmsg_char_init() failed: %d\n", status);
            }
/* LDRA_JUSTIFY_END */
            g_char_init_done++;
        }
    }

    if (status == 0)
    {
        appLogPrintf("MEM: Init heap[%d] '%s' ... Done !!!\n", heap_idx, heap->name);
    }

    return status;
}

uint8_t appMemIsCharInit(void)
{
    return g_char_init_done;
}

void appMemPrintMemAllocInfo(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    appLogPrintf("=== Memory Allocation Info ===\n");

    for (uint32_t i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (heap->in_use)
        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM20
<justification end>*/
            appLogPrintf("HEAP[%d] '%s' (heap_id=%d, base=0x%lx, size=%luMB, alloc=%s):\n",
                         i, heap->name, heap->heap_id, heap->base_phys_addr, heap->size / (1024U*1024U),
                         (heap->allow_alloc == 1U) ? "YES" : "NO");
/* LDRA_JUSTIFY_END */
            appLogPrintf("  Alloc's: %ld alloc's of %ld bytes\n",
                         heap->heap_num_alloc, heap->heap_alloc_bytes);
            appLogPrintf("  Free's : %ld free's  of %ld bytes\n",
                         heap->heap_num_free, heap->heap_free_bytes);
            appLogPrintf("  Open's : %ld allocs  of %ld bytes\n",
                         heap->heap_cur_alloc, heap->heap_cur_alloc_bytes);
        }
    }

    appLogPrintf("GLOBAL TOTAL:\n");
    appLogPrintf("  Alloc's: %ld alloc's of %ld bytes\n", obj->num_alloc, obj->total_alloc_bytes);
    appLogPrintf("  Free's : %ld free's  of %ld bytes\n", obj->num_free, obj->total_free_bytes);
    appLogPrintf("  Open's : %ld allocs  of %ld bytes\n", obj->cur_alloc, obj->cur_alloc_bytes);
    appLogPrintf("==============================\n");
}

int32_t appMemDeInit(void)
{
    int32_t status = 0;
    int status1 = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;

    appLogPrintf("MEM: Deinit ... !!!\n");

    appMemPrintMemAllocInfo();

    for (uint32_t i = 0U; i < MAX_HEAP_REGIONS; i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (heap->in_use)
        {
            if (pthread_mutex_lock(&heap->list_mutex) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
            }

            appMemFreeList(i, heap->plist);

            if (pthread_mutex_unlock(&heap->list_mutex) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
            }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_U18
<justification end>*/
            if (heap->dma_heap_fd >= 0)
/* LDRA_JUSTIFY_END */
            {
                status1 = close(heap->dma_heap_fd);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM19
<justification end>*/
                if (status1 < 0)
                {
                    appLogPrintf("MEM: ERROR: Failed to deinit DMA HEAP for heap[%d] '%s' !!!\n",
                                 i, heap->name);
                    status = -1;
                }
/* LDRA_JUSTIFY_END */
                heap->dma_heap_fd = -1;
            }

            /* Destroy the heap's mutex */
            (void)pthread_mutex_destroy(&heap->list_mutex);

            heap->in_use = false;
            appLogPrintf("MEM: Deinit heap[%d] '%s' ... Done !!!\n", i, heap->name);
        }
    }

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }
    obj->num_heaps = 0;
    obj->total_alloc_bytes = 0;
    obj->total_free_bytes = 0;
    obj->cur_alloc_bytes = 0;
    obj->num_alloc = 0;
    obj->num_free = 0;
    obj->cur_alloc = 0;
    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    appLogPrintf("MEM: Deinit ... Done !!!\n");

    return(status);
}

uint32_t appMemGetDmaBufFd(void *virPtr, volatile uint32_t *dmaBufFdOffset)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    int dmaBufFd = -1;
    uint64_t virt_addr = (uint64_t)(virPtr);
    bool found = false;
    uint32_t i;

    *dmaBufFdOffset = 0;

    for (i = 0U; (i < MAX_HEAP_REGIONS) && (!found); i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (!heap->in_use)
        {
            continue;
        }

        app_mem_list_t *mem_map_list;

        if (pthread_mutex_lock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        mem_map_list = heap->plist;
        while((mem_map_list != NULL) && (!found))
        {
            if( (virt_addr >= mem_map_list->mem_data.virt_addr)
                && (virt_addr < (mem_map_list->mem_data.virt_addr+mem_map_list->mem_data.size))
                )
            {
                uint32_t offset = (uint32_t)(virt_addr - mem_map_list->mem_data.virt_addr);

                dmaBufFd = (int)mem_map_list->mem_data.dma_buf_fd;
                *dmaBufFdOffset = offset;
                found = true;
            }
            else
            {
                mem_map_list = mem_map_list->next;
            }
        }
        if (pthread_mutex_unlock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    if (dmaBufFd < 0)
    {
        appLogPrintf("MEM: ERROR: Failed to get dmaBufFd for virtPtr %p !!!\n", virPtr);
        appLogPrintf("MEM: ERROR: Buffer may be APP_MEM_HEAP_DDR (local-only, not DMA-heap backed) !!!\n");
    }
    else
    {
        #ifdef APP_MEM_DEBUG
        appLogPrintf("MEM: Exported dmaBufFd %d @ offset = %d bytes !!!\n", dmaBufFd, *dmaBufFdOffset);
        #endif
    }

    return (uint32_t)(dmaBufFd);
}

int32_t appMemTranslateDmaBufFd(uint32_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    int32_t         status = 0;
    bool            found = false;
    uint32_t i;

    *virtPtr = 0;
    *phyPtr  = 0;

    for (i = 0U; (i < MAX_HEAP_REGIONS) && (!found); i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (!heap->in_use)
        {
            continue;
        }

        app_mem_list_t *mem_map_list;

        if (pthread_mutex_lock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        mem_map_list = heap->plist;
        while((mem_map_list != NULL) && (!found))
        {
            if (dmaBufFd == mem_map_list->mem_data.dma_buf_fd)
            {
                app_mem_type_t *mem_data = &mem_map_list->mem_data;
                uint32_t offset = (uint32_t)mem_data->offset;

                *virtPtr = mem_data->virt_addr + offset;
                *phyPtr  = mem_data->phys_addr + offset;
                found = true;
            }
            else
            {
                mem_map_list = mem_map_list->next;
            }
        }

        if (pthread_mutex_unlock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    if (!found)
    {
        uint64_t temp_phys = 0;
        uint32_t target_heap_idx = 0;
        bool heap_found = false;
        int32_t dmabuf2phys_fd = -1;

        dmabuf2phys_fd = open("/dev/remoteproc0", (int)((uint32_t)O_RDONLY | (uint32_t)O_CLOEXEC));

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_U21
<justification end>*/
        if (dmabuf2phys_fd < 0)
        {
            appLogPrintf("MEM: ERROR: Failed to open /dev/remoteproc0 for dmaBufFd translation\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
        else
        {
            status = appMemDmaHeapGetPhys(dmabuf2phys_fd, (int)dmaBufFd, &temp_phys);
            (void)close(dmabuf2phys_fd);

            if (status != 0)
            {
                appLogPrintf("MEM: ERROR: Failed to get physical address for dmaBufFd %d\n", dmaBufFd);
            }
        }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_U22
<justification end>*/
        if (status == 0)
        {
            for (i = 0U; i < MAX_HEAP_REGIONS; i++)
            {
                app_mem_heap_obj_t *heap = &obj->heaps[i];

                if (heap->in_use)
                {
                    uint64_t heap_start = heap->base_phys_addr;
                    uint64_t heap_end = heap->base_phys_addr + heap->size;

                    if ((temp_phys >= heap_start) && (temp_phys < heap_end))
                    {
                        target_heap_idx = i;
                        heap_found = true;
                        appLogPrintf("MEM: dmaBufFd %d (phys=0x%lx) belongs to heap[%d] '%s'\n",
                                     dmaBufFd, temp_phys, i, heap->name);
                        break;
                    }
                }
            }

            if (!heap_found)
            {
                /* Physical address doesn't match any known heap - use first allocatable heap as fallback */
                appLogPrintf("MEM: WARNING: dmaBufFd %d (phys=0x%lx) does not match any heap range\n",
                             dmaBufFd, temp_phys);
                for (i = 0U; i < MAX_HEAP_REGIONS; i++)
                {
                    if ((obj->heaps[i].in_use) && (obj->heaps[i].allow_alloc == 1U))
                    {
                        target_heap_idx = i;
                        heap_found = true;
                        appLogPrintf("MEM: Using fallback heap[%d] '%s'\n", i, obj->heaps[i].name);
                        break;
                    }
                }
            }

            if (heap_found)
            {
                status = appMemAddTupleToList(target_heap_idx, dmaBufFd, size, virtPtr, phyPtr);
            }
            else
            {
                appLogPrintf("MEM: ERROR: No suitable heap found for dmaBufFd %d\n", dmaBufFd);
                status = -1;
            }
        }
    }
/* LDRA_JUSTIFY_END */

    if ((*virtPtr == 0u) || (*phyPtr == 0u))
    {
        appLogPrintf("MEM: ERROR: Failed to translate dmaBufFd [%d]\n", dmaBufFd);
        status = -1;
    }

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Translated dmaBufFd %d to virtPtr %p and phyPtr %p!!!\n",
           dmaBufFd, (void*)*virtPtr, (void*)*phyPtr);
    #endif

    return status;
}

/* Delete the table entry when someone closes the dma buf fd */
void appMemCloseDmaBufFd(uint32_t dmaBufFd)
{
    /* This is done in appMemFree */
}

static int appMemDmaHeapAlloc(int dma_heap_fd, size_t len, unsigned int flags, int *dma_buf_fd)
{
    int ret = 0;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM03
<justification end>*/
    if (dma_buf_fd == NULL)
    {
        ret = -EINVAL;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM03
<justification end>*/
    if (0 == ret)
    {
        struct dma_heap_allocation_data data = {
            .len = len,
            .fd_flags = (uint32_t)O_CLOEXEC | (uint32_t)O_RDWR,
            .heap_flags = flags,
        };

        *dma_buf_fd = -1;

        ret = ioctl(dma_heap_fd, DMA_HEAP_IOCTL_ALLOC, &data);
        if (ret < 0)
        {
            ret = errno;
        }
        else
        {
            *dma_buf_fd = (int)data.fd;
        }
    }
/* LDRA_JUSTIFY_END */
    return ret;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<function start> static int appMemDmaHeapExport.* <function end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_U23
<justification end>*/
static int appMemDmaHeapExport(int dma_heap_fd, uint64_t offset, size_t len, unsigned int flags, int *dma_buf_fd)
{
    int ret = 0;
    if (dma_buf_fd == NULL)
    {
        ret = -EINVAL;
    }
    if (0 == ret)
    {
        struct dma_heap_export_data data = {
            .offset = offset,
            .len = len,
            .fd = 0,
            .fd_flags = (uint32_t)O_CLOEXEC | (uint32_t)O_RDWR,
            .heap_flags = flags,
        };

        *dma_buf_fd = -1;

        ret = ioctl(dma_heap_fd, DMA_HEAP_IOCTL_EXPORT, &data);
        if (ret < 0)
        {
            ret = errno;
        }
        else
        {
            *dma_buf_fd = (int)data.fd;
        }
    }
    return ret;
}

uint64_t appMemGetVirt2PhyBufPtr(uint64_t virtPtr, uint32_t heap_id)
{
    off_t phyPtr = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint64_t virt_addr = (uint64_t)(virtPtr);
    bool found = false;
    uint32_t i;

    for (i = 0U; (i < MAX_HEAP_REGIONS) && (!found); i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (!heap->in_use)
        {
            continue;
        }

        app_mem_list_t *mem_map_list;

        if (pthread_mutex_lock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        mem_map_list = heap->plist;
        while((mem_map_list != NULL) && (!found))
        {
            if( (virt_addr >= mem_map_list->mem_data.virt_addr)
                && (virt_addr < (mem_map_list->mem_data.virt_addr+mem_map_list->mem_data.size))
                )
            {
                uint32_t offset = (uint32_t)(virt_addr - mem_map_list->mem_data.virt_addr);

                phyPtr = (off_t)(mem_map_list->mem_data.phys_addr) + (off_t)offset;
                found = true;
            }
            else
            {
                mem_map_list = mem_map_list->next;
            }
        }
        /* Exit critical section */
        if (pthread_mutex_unlock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    #ifdef APP_MEM_DEBUG
    if (phyPtr == 0)
    {
        appLogPrintf("MEM: ERROR: Failed to get physical address for virt addr %p !!!\n", (void*)virtPtr);
        appLogPrintf("MEM: ERROR: Buffer may be APP_MEM_HEAP_DDR (local-only, not DMA-heap backed) !!!\n");
    }
    else
    {
        appLogPrintf("MEM: Translated virt addr = %p -> phy addr = %lx !!!\n", (void*)virtPtr, phyPtr);
    }
    #endif

    return (uint64_t)(phyPtr);
}

void *appMemAlloc(uint32_t heap_id, uint32_t size, uint32_t align)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    uint64_t        virtual_ptr = 0;
    int             dma_buf_fd = -1;
    uint64_t        phys_addr = 0;
    uint32_t        flags = DMA_HEAP_ALLOC_FLAGS;
    int             status = -1;
    uint32_t        heap_idx = 0;
    uint32_t        i;

    if ((APP_MEM_HEAP_DDR_SHARED == heap_id) || (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY == heap_id) ||
        (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST == heap_id))
    {
        /* align is ignored, all DMA HEAP allocated buffers are minimum 4KB aligned
        * The alignment information can be found in ${PSDKL}/board-support/linux-*\/
        * dma-buf/dma-heap.c or by running "getconf PAGESIZE" on the target
        */

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM24
<justification end>*/
        for (i = 0U; i < MAX_HEAP_REGIONS; i++)
        {
            if ((obj->heaps[i].in_use) && (obj->heaps[i].allow_alloc == 1U) && (obj->heaps[i].heap_id == heap_id))
            {
                heap_idx = i;
                break;
            }
        }
/* LDRA_JUSTIFY_END */
    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM25
<justification end>*/
        if (i != MAX_HEAP_REGIONS)
/* LDRA_JUSTIFY_END */
        {

            app_mem_heap_obj_t *heap = &obj->heaps[heap_idx];

            status = appMemDmaHeapAlloc(heap->dma_heap_fd,
                                            size,
                                            flags,
                                            &dma_buf_fd
                                        );

            if(status != 0)
            {
                appLogPrintf("MEM: ERROR: Alloc failed for heap[%d] '%s' with status = %d !!!\n",
                            heap_idx, heap->name, status);
            }

            if (status == 0)
            {
                (void)appMemAddTupleToList(heap_idx, (uint32_t)dma_buf_fd, size, &virtual_ptr, &phys_addr);
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM26
<justification end>*/
        else
        {
            appLogPrintf("MEM: ERROR: No heap available for MPU allocation from heap_id %d!\n", heap_id);
        }
/* LDRA_JUSTIFY_END */
    }
    else if (APP_MEM_HEAP_DDR == heap_id)
    {
        /* APP_MEM_HEAP_DDR uses malloc() for local-only allocations.
         * WARNING: These buffers are NOT tracked in the DMA heap list,
         * so appMemGetVirt2PhyBufPtr() and appMemGetDmaBufFd() will fail.
         * Use APP_MEM_HEAP_DDR_SHARED for buffers that need physical address
         * translation or DMA buffer fd access. */
        virtual_ptr = (uint64_t)malloc(size);
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block/heap_id\n");
    }

    return ((void *)(uintptr_t)virtual_ptr);
}

static void appMemFreeListItem(uint32_t heap_idx, app_mem_list_t *mem_map_list)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    app_mem_heap_obj_t *heap = &obj->heaps[heap_idx];
    uint32_t dmaBufFd = (uint32_t)-1;
    uint32_t dmabuf2phys_fd = (uint32_t)-1;
    uint32_t size;

    (void)munmap((void*)mem_map_list->mem_data.virt_addr, mem_map_list->mem_data.size);
    dmaBufFd = mem_map_list->mem_data.dma_buf_fd;
    dmabuf2phys_fd = (uint32_t)mem_map_list->mem_data.dmabuf2phys_fd;
    size = mem_map_list->mem_data.size;
    (void)close((int)dmabuf2phys_fd);
    (void)close((int)dmaBufFd);

    heap->heap_free_bytes += size;
    heap->heap_num_free++;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM27
<justification end>*/
    if( (heap->heap_cur_alloc_bytes < size) || (heap->heap_cur_alloc==0u) )
    {
        appLogPrintf("MEM: WARNING: Free'ing more memory than allocated in heap[%d] '%s'\n",
                     heap_idx, heap->name);
    }
    else
    {
        heap->heap_cur_alloc_bytes -= size;
        heap->heap_cur_alloc--;
    }
/* LDRA_JUSTIFY_END */

    if (pthread_mutex_lock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
    }
    obj->total_free_bytes += size;
    obj->num_free++;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM28
<justification end>*/
    if( (obj->cur_alloc_bytes < size) || (obj->cur_alloc==0u) )
    {
        appLogPrintf("MEM: WARNING: Free'ing more memory than allocated globally\n");
    }
    else
    {
        obj->cur_alloc_bytes -= size;
        obj->cur_alloc--;
    }
/* LDRA_JUSTIFY_END */
    if (pthread_mutex_unlock(&obj->global_mutex) != 0)
    {
        appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
    }

    #ifdef APP_MEM_DEBUG
    appMemPrintMemAllocInfo();
    #endif
}


int32_t appMemFree(uint32_t heap_id, void *ptr, uint32_t size)
{
    int32_t status = 0;
    uint8_t b_found_addr = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t i;

    if( (APP_MEM_HEAP_DDR_SHARED == heap_id) || (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY == heap_id) ||
        (APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST == heap_id) )
    {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM29
<justification end>*/
        for (i = 0U; (i < MAX_HEAP_REGIONS) && (b_found_addr == 0u); i++)
/* LDRA_JUSTIFY_END */
        {
            app_mem_heap_obj_t *heap = &obj->heaps[i];
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM30
<justification end>*/
            if ((!heap->in_use) || (heap->heap_id != heap_id))
            {
                continue;
            }
/* LDRA_JUSTIFY_END */

            app_mem_list_t *mem_map_list;

            if (pthread_mutex_lock(&(heap->list_mutex)) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
            }
            mem_map_list = heap->plist;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM31
<justification end>*/
            while((mem_map_list != NULL) && (b_found_addr == 0u))
/* LDRA_JUSTIFY_END */
            {
                if(mem_map_list->mem_data.virt_addr == (uint64_t)(ptr))
                {
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: Freeing virt addr = %p -> phy addr = %lx, size = %d (FD's = %d %d) from heap[%d] '%s' !!!\n",
                        (void*)ptr, mem_map_list->mem_data.phys_addr, size,
                        mem_map_list->mem_data.dmabuf2phys_fd,
                        mem_map_list->mem_data.dma_buf_fd,
                        i, heap->name
                        );
                    #endif

                    appMemFreeListItem(i, mem_map_list);
                    appMemRemoveFromList(&(heap->plist), mem_map_list);

                    b_found_addr = 1;
                }
                else
                {
                    mem_map_list = mem_map_list->next;
                }
            }

            if (pthread_mutex_unlock(&(heap->list_mutex)) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
            }
        }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM32
<justification end>*/
        if(1u != b_found_addr)
        {
            status = -1;
            appLogPrintf("MEM: ERROR: Failed to free memory at virt addr = %p !!!\n", ptr);
        }
/* LDRA_JUSTIFY_END */
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
    uint32_t i;
    app_mem_obj_t *obj = &g_app_mem_obj;

    if( stats != NULL )
    {
        if ((heap_id == APP_MEM_HEAP_DDR_SHARED) || (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY) ||
            (heap_id == APP_MEM_HEAP_DDR_SHARED_64BIT_ONLY_CONST))
        {
            const char *heap_name_str;

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

            uint64_t total_size = 0;
            uint64_t used_size = 0;

            for (i = 0U; i < MAX_HEAP_REGIONS; i++)
            {
                if ((obj->heaps[i].in_use) && (obj->heaps[i].heap_id == heap_id))
                {
                    total_size += obj->heaps[i].size;
                    used_size += obj->heaps[i].heap_cur_alloc_bytes;
                }
            }

            stats->heap_size = total_size;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM33
<justification end>*/
            stats->free_size = (total_size > used_size) ?
                            (total_size - used_size) : 0U;
/* LDRA_JUSTIFY_END */
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
    int32_t status = 0;
    uint32_t offset = 0;
    uint32_t dmaBufFd = appMemGetDmaBufFd(ptr, &offset);
    struct dma_buf_sync sync_flags = {0};

    sync_flags.flags = (uint32_t)DMA_BUF_SYNC_START | (uint32_t)DMA_BUF_SYNC_RW;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR001
<justification end> */
    if(dmaBufFd > 0u)
/* LDRA_JUSTIFY_END */
    {
        status = ioctl((int)dmaBufFd, DMA_BUF_IOCTL_SYNC, &sync_flags);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR002
<justification end>*/
        if(status < 0)
        {
            appLogPrintf("MEM: ERROR: DMA_BUF_IOCTL_SYNC failed for appMemCacheInv(%p, %d) !!!\n",
                ptr, size
                );
        }
/* LDRA_JUSTIFY_END */
    }
}

void  appMemCacheWb(void *ptr, uint32_t size)
{
    int32_t status = 0;
    uint32_t offset = 0;
    uint32_t dmaBufFd = appMemGetDmaBufFd(ptr, &offset);
    struct dma_buf_sync sync_flags ={0};

    sync_flags.flags = (uint32_t)DMA_BUF_SYNC_RW | (uint32_t)DMA_BUF_SYNC_END;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR003
<justification end> */
    if(dmaBufFd > 0u)
/* LDRA_JUSTIFY_END */
    {
        status = ioctl((int)dmaBufFd, DMA_BUF_IOCTL_SYNC, &sync_flags);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR004
<justification end>*/
        if(status < 0)
        {
            appLogPrintf("MEM: ERROR: DMA_BUF_IOCTL_SYNC failed for appMemCacheWb(%p, %d) !!!\n",
                ptr, size
                );
        }
/* LDRA_JUSTIFY_END */
    }
}

void  appMemCacheWbInv(void *ptr, uint32_t size)
{
    appMemCacheWb(ptr, size);
    appMemCacheInv(ptr, size);
}

/* NOT SUPPORTED on Linux A72, needed for linking tivx mem platform layer */
int32_t appMemResetScratchHeap(uint32_t heap_id)
{
    return(-1);
}

/* give a 'dma_buf_fd' get physical address presented by that 'dma_buf_fd' */
static int appMemDmaHeapGetPhys(int phys_fd, int dma_buf_fd, uint64_t *phys)
{
    int ret = 0;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM05
<justification end>*/
    if (phys == NULL)
    {
        ret = -EINVAL;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM05
<justification end>*/
    if (0 == ret)
    {
        struct dma_buf_phys_data data = {
            .fd = (__u32)dma_buf_fd,
        };

        data.phys = 0;

        ret = ioctl(phys_fd, RPROC_IOC_DMA_BUF_ATTACH, &data);
        if (ret < 0)
        {
            ret = -errno;
        }
        else
        {
            *phys = (uint64_t)data.phys;
        }
    }
/* LDRA_JUSTIFY_END */
    return ret;
}

/* Linked list manipulation functions */
static int appMemPushToList(app_mem_list_t **list, app_mem_type_t *data)
{
    app_mem_list_t *new_node = (app_mem_list_t *)malloc(sizeof(app_mem_list_t));
    int status = 0;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR005
<justification end>*/
    if(NULL != new_node)
/* LDRA_JUSTIFY_END */
    {
        new_node->next = *list;
        new_node->mem_data.virt_addr = data->virt_addr;
        new_node->mem_data.phys_addr = data->phys_addr;
        new_node->mem_data.dma_buf_fd = data->dma_buf_fd;
        new_node->mem_data.dmabuf2phys_fd = data->dmabuf2phys_fd;
        new_node->mem_data.offset = data->offset;
        new_node->mem_data.size = data->size;
        *list = new_node;
        status = 0;
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM06
<justification end>*/
    else
    {
        status = -1;
    }
/* LDRA_JUSTIFY_END */
    return status;
}

static void appMemFreeList(uint32_t heap_idx, app_mem_list_t *list)
{
    app_mem_list_t *tmp;
    while(list != NULL)
    {
        tmp = list;
        list = list->next;
        appMemFreeListItem(heap_idx, tmp);
        free(tmp);
    }
}

static void appMemRemoveFromList(app_mem_list_t **list, app_mem_list_t *node)
{
    app_mem_list_t *curr = *list;
    app_mem_list_t *prev = *list;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR006
<justification end>*/
    while(curr != NULL)
/* LDRA_JUSTIFY_END */
    {
        /*Check for match found*/
        if(curr == node)
        {
            /*Check if match found is HEAD*/
            if(*list == node)
            {
                *list = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }
            free(curr);
            break;
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}

static int32_t appMemAddTupleToList(uint32_t heap_idx, uint32_t dmaBufFd, uint64_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    app_mem_heap_obj_t *heap = &obj->heaps[heap_idx];
    void           *vPtr = NULL;
    app_mem_type_t  m;
    int32_t         dmabuf2phys_fd = -1;
    int32_t         status = 0;

    *virtPtr = 0;
    *phyPtr  = 0;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM34
<justification end>*/
    if (size == 0u)
    {
        appLogPrintf("MEM: ERROR: Mapped memory size cannot be 0.\n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */

    if (status == 0)
    {
        vPtr = mmap((void *)0x00000000u,
                    size,
                    (int)((uint32_t)PROT_WRITE | (uint32_t)PROT_READ),
                    MAP_SHARED,
                    (int)dmaBufFd,
                    (off_t)0u);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM07
<justification end>*/
        if ((uintptr_t)vPtr == (uintptr_t)MAP_FAILED)
        {
            appLogPrintf("MEM: ERROR: Failed to translate dmaBufFd %d to "
                   "virtPtr!!! errno=%d (%s)\n", dmaBufFd, errno, strerror(errno));
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }

    /* open driver to convert to physical address */
    if (status == 0)
    {
        *virtPtr = (uint64_t)vPtr;

        dmabuf2phys_fd = open("/dev/remoteproc0",(int)((uint32_t)O_RDONLY | (uint32_t)O_CLOEXEC));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM08
<justification end>*/
        if (dmabuf2phys_fd < 0)
        {
            appLogPrintf("MEM: ERROR: /dev/remoteproc0 open failed !!!\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM35
<justification end>*/
    /* get physical address */
    if (status == 0)
    {
        status = appMemDmaHeapGetPhys(dmabuf2phys_fd, (int)dmaBufFd, phyPtr);

        if(status != 0)
        {
            appLogPrintf("MEM: ERROR: appMemDmaHeapGetPhys failed "
                   "(dma buf = %d, dmabuf2phys = %d)!!!\n",
                   dmaBufFd, dmabuf2phys_fd);
        }
    }
/* LDRA_JUSTIFY_END */

    /* add to local linked list for later retrival and conversion */
    if(status==0)
    {
        m.dma_buf_fd = dmaBufFd;
        m.dmabuf2phys_fd = dmabuf2phys_fd;
        m.phys_addr = *phyPtr;
        m.virt_addr = *virtPtr;
        m.offset    = 0;
        m.size      = size;

        /* Enter critical section */
        if (pthread_mutex_lock(&(heap->list_mutex)) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }

        status = appMemPushToList(&(heap->plist), &m);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR007
<justification end>*/
        if(status==0)
        {
            heap->heap_alloc_bytes += size;
            heap->heap_cur_alloc_bytes += size;
            heap->heap_num_alloc++;
            heap->heap_cur_alloc++;

            if (pthread_mutex_lock(&(obj->global_mutex)) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
            }
            obj->total_alloc_bytes += size;
            obj->cur_alloc_bytes += size;
            obj->num_alloc++;
            obj->cur_alloc++;
            if (pthread_mutex_unlock(&(obj->global_mutex)) != 0)
            {
                appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
            }

            #ifdef APP_MEM_DEBUG
            appMemPrintMemAllocInfo();
            #endif
        }
/* LDRA_JUSTIFY_END */
        /* Exit critical section */
        if (pthread_mutex_unlock(&(heap->list_mutex)) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM09
<justification end>*/
        if(status!=0)
        {
            appLogPrintf("MEM: ERROR: appMemPushToList failed !!!\n");
        }
/* LDRA_JUSTIFY_END */
    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM36
<justification end>*/
    if(status==0)
    {
        #ifdef APP_MEM_DEBUG
        appLogPrintf("MEM: Allocated virt addr = %lx -> phy addr = %lx, size = %d (FD's = %d %d) !!!\n",
            *virtPtr, *phyPtr, size,
            dmaBufFd,
            dmabuf2phys_fd
            );
        #endif
    }
    else
    {
        /* release resources incase of alloc failure */
        appLogPrintf("MEM: ERROR: memory alloc of size = %d bytes, failed with status = %d !!!\n", size, status);
        if ((uintptr_t)vPtr != (uintptr_t)MAP_FAILED)
        {
            (void)munmap(vPtr, size);
        }
        if(dmabuf2phys_fd>=0)
        {
            (void)close(dmabuf2phys_fd);
            dmabuf2phys_fd = -1;
        }
        if((int32_t)dmaBufFd>=0)
        {
            (void)close((int)dmaBufFd);
        }
        appMemPrintMemAllocInfo();
        /* Resetting virtPtr back to NULL from the previously unmapped vPtr value */
        /* This is needed for appMemAlloc success checks to fail when allocation is failed here */
        *virtPtr = 0;
    }

    if ((*virtPtr == 0u) || (*phyPtr == 0u))
    {
        appLogPrintf("MEM: ERROR: Failed to translate dmaBufFd [%d]\n", dmaBufFd);
        status = -1;
    }
/* LDRA_JUSTIFY_END */

    #ifdef APP_MEM_DEBUG
    appLogPrintf("MEM: Translated dmaBufFd %d to virtPtr %p and phyPtr %p!!!\n",
           dmaBufFd, (void*)*virtPtr, (void*)*phyPtr);
    #endif

    return status;
}

uint64_t appMemShared2PhysPtr(uint64_t shared_ptr, uint32_t heap_id)
{
    return shared_ptr;
}

uint64_t appMemShared2TargetPtr(uint64_t shared_ptr)
{
    uint64_t virt_addr = 0U;
    app_mem_obj_t *obj = &g_app_mem_obj;
    bool found = false;
    uint32_t i;

    for (i = 0U; (i < MAX_HEAP_REGIONS) && (!found); i++)
    {
        app_mem_heap_obj_t *heap = &obj->heaps[i];
        if (!heap->in_use)
        {
            continue;
        }

        app_mem_list_t *mem_map_list;

        if (pthread_mutex_lock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_lock failed !!!\n");
        }
        mem_map_list = heap->plist;
        while((mem_map_list != NULL) && (!found))
        {
            app_mem_type_t *mem_data = &mem_map_list->mem_data;
            uint64_t phys_base = mem_data->phys_addr;
            uint64_t phys_end = phys_base + mem_data->size;

            if ((shared_ptr >= phys_base) && (shared_ptr < phys_end))
            {
                uint64_t offset = shared_ptr - phys_base;
                virt_addr = mem_data->virt_addr + offset;
                found = true;
            }
            else
            {
                mem_map_list = mem_map_list->next;
            }
        }
        if (pthread_mutex_unlock(&heap->list_mutex) != 0)
        {
            appLogPrintf("MEM: ERROR: pthread_mutex_unlock failed !!!\n");
        }
    }

    if (virt_addr == 0U)
    {
        virt_addr = shared_ptr;
    }

    return virt_addr;
}

uint32_t appMemGetNumAllocs(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    return (uint32_t)obj->cur_alloc;
}

void *appMemMap(void *phys_addr, uint32_t size)
{
    uint32_t  pageSize = (uint32_t)getpagesize ();
    uintptr_t taddr;
    uint32_t  tsize;
    void     *virt_ptr = NULL;
    int32_t   status = 0;
    static int dev_mem_fd = -1;

    if(dev_mem_fd == -1)
    {
        dev_mem_fd = open("/dev/mem",(int)((uint32_t)O_RDWR|(uint32_t)O_SYNC));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM10
<justification end>*/
        if(dev_mem_fd  < 0)
        {
            appLogPrintf("APP_LOG: ERROR: Unable to open /dev/mem !!!\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }
    if( (status==0) /* TIOVX-1909- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR008 */
    && (dev_mem_fd >= 0) ) /* TIOVX-1909- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR009 */
    {
        #ifdef APP_MEM_DEBUG
        appLogPrintf("APP_MEM: Mapping %p ...\n", phys_addr);
        #endif
        /* Mapping this physical address to linux user space */
        taddr = (uintptr_t)phys_addr;
        tsize = size;

        /* Align the physical address to page boundary */
        tsize = (uint32_t)appAlign(tsize + (uint32_t)(taddr % pageSize), pageSize);
        taddr = appFloor(taddr, pageSize);

        virt_ptr  = mmap(NULL, tsize,
                        (int)((uint32_t)PROT_READ | (uint32_t)PROT_WRITE),
                        (MAP_SHARED), dev_mem_fd, (off_t)taddr);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM11
<justification end>*/
        if((uintptr_t)virt_ptr==(uintptr_t)MAP_FAILED)
        {
            virt_ptr = NULL;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM11
<justification end>*/
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
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM12
<justification end>*/
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_addr, size);
    }
/* LDRA_JUSTIFY_END */
    return virt_ptr;
}

int32_t appMemUnMap(void *virt_ptr, uint32_t size)
{
    int32_t status=0;
    uint32_t pageSize = (uint32_t)getpagesize ();
    uintptr_t taddr;
    uint32_t  tsize;

    #ifdef APP_MEM_DEBUG
    appLogPrintf("APP_MEM: UnMapped memory at virtual address @ 0x%p of size %d bytes \n", virt_ptr, size);
    #endif

    taddr = (uint64_t)virt_ptr;
    tsize = size;

    tsize = (uint32_t)appAlign((tsize + (uint32_t)(taddr % pageSize)), pageSize);
    taddr = (uintptr_t)appFloor(taddr, pageSize);

    (void)munmap((void *)taddr, tsize);

    return status;
}
