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
#include <utils/mem/include/app_mem.h>
#include <linux/dma-heap.h>
#include <linux/dma-buf.h>

/*MACROS*/
/* #define APP_MEM_DEBUG */
#ifdef SOC_AM62A
#define DMA_HEAP_NAME          "/dev/dma_heap/carveout_edgeai_shared-memories"
#else
#define DMA_HEAP_NAME          "/dev/dma_heap/carveout_vision_apps_shared-memories"
#endif
#define DMA_HEAP_ALLOC_FLAGS   (0u)

/*STRUCTURES*/

typedef struct
{
    uint32_t dma_buf_fd;
    int32_t  dmabuf2phys_fd;
    uint64_t phys_addr;
    uint64_t virt_addr;
    uint64_t offset;
    uint32_t size;
}app_mem_type_t;

typedef struct mem_type_node
{
    app_mem_type_t mem_data;
    struct mem_type_node *next;
} app_mem_list_t;

typedef struct
{
    app_mem_list_t *plist;
    pthread_mutex_t list_mutex;
    int dma_heap_fd;
    uint32_t app_heap_id;

    uint64_t total_alloc_bytes;
    uint64_t total_free_bytes;
    uint64_t cur_alloc_bytes;
    uint64_t num_alloc;
    uint64_t num_free;
    uint64_t cur_alloc;

}app_mem_obj_t;

/*GLOBALS*/
app_mem_obj_t g_app_mem_obj;

/*INTERNAL FUNCTION DEFINITIONS*/
static void appMemRemoveFromList(app_mem_list_t **list, app_mem_list_t *node);
static int  appMemPushToList(app_mem_list_t **list, app_mem_type_t *data);
static int  appMemDmaHeapGetPhys(int phys_fd, int fd, uint64_t *phys);
static void appMemFreeList(app_mem_list_t *list);
static int32_t appMemAddTupleToList(uint32_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr);
static void appMemFreeListItem(app_mem_list_t *mem_map_list);
static int appMemDmaHeapAlloc(int dma_heap_fd, size_t len, unsigned int flags, int *dma_buf_fd);

/* Return true if specified memory region is enabled, return false otherwise*/
bool appMemRegionQuery(uint32_t heap_id)
{
    return ((heap_id == APP_MEM_HEAP_DDR) || (heap_id == APP_MEM_HEAP_DDR_SHARED));
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    int32_t status = 0;

    /* Initialize our book-keeping structures */
    app_mem_obj_t *obj = &g_app_mem_obj;

    obj->total_alloc_bytes = 0;
    obj->total_free_bytes = 0;
    obj->cur_alloc_bytes = 0;
    obj->num_alloc = 0;
    obj->num_free = 0;
    obj->cur_alloc = 0;

    obj->plist = NULL;
    obj->dma_heap_fd = -1;
    obj->app_heap_id = (uint32_t)-1;

    /* Initialize the pthread mutex */
    (void)pthread_mutex_init(&obj->list_mutex, NULL);

    appLogPrintf("MEM: Init ... !!!\n");

    /* Initialize the DMA HEAP module */
    obj->dma_heap_fd = open(DMA_HEAP_NAME, (int)((uint32_t)O_RDONLY | (uint32_t)O_CLOEXEC));
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM01
<justification end>*/
    if (obj->dma_heap_fd < 0)
    {
        appLogPrintf("MEM: ERROR: Failed to initialize DMA HEAP [%s] !!!\n", DMA_HEAP_NAME);
        status = -1;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM01
<justification end>*/
    else
/* LDRA_JUSTIFY_END */
    {
        appLogPrintf("MEM: Initialized DMA HEAP (fd=%d) !!!\n", obj->dma_heap_fd);
    }

    /* RPMSG init required for dmabuf virt to phys conversion */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR010
<justification end> */
    if (status==0)
/* LDRA_JUSTIFY_END */
    {
        status = rpmsg_char_init(NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM13
<justification end>*/
        if (status!=0)
        {
            appLogPrintf("MEM: ERROR: rpmsg_char_init() failed: %d\n", status);
            status = -1;
        }
/* LDRA_JUSTIFY_END */
    }

    appLogPrintf("MEM: Init ... Done !!!\n");

    return(status);
}

void appMemPrintMemAllocInfo(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    appLogPrintf("DDR_SHARED_MEM: Alloc's: %ld alloc's of %ld bytes \n", obj->num_alloc, obj->total_alloc_bytes );
    appLogPrintf("DDR_SHARED_MEM: Free's : %ld free's  of %ld bytes \n", obj->num_free, obj->total_free_bytes );
    appLogPrintf("DDR_SHARED_MEM: Open's : %ld allocs  of %ld bytes \n", obj->cur_alloc, obj->cur_alloc_bytes );
}

int32_t appMemDeInit(void)
{
    int32_t status = 0;
    int status1 = 0;

    app_mem_obj_t *obj = &g_app_mem_obj;

    appLogPrintf("MEM: Deinit ... !!!\n");

    (void)pthread_mutex_lock(&obj->list_mutex);

    /* Clear the heap off our memory map list */
    appMemFreeList(obj->plist);

    appMemPrintMemAllocInfo();

    (void)pthread_mutex_unlock(&obj->list_mutex);

    /* Deinitialize the DMA HEAP module */
    status1 = close(obj->dma_heap_fd);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM02
<justification end>*/
    if (status1 < 0)
    {
        appLogPrintf("MEM: ERROR: Failed to deinit the DMA HEAP module !!!\n");
        status = -1;
    }
/* LDRA_JUSTIFY_END */

    appLogPrintf("MEM: Deinit ... Done !!!\n");

    return(status);
}

uint32_t appMemGetDmaBufFd(void *virPtr, volatile uint32_t *dmaBufFdOffset)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    int dmaBufFd = -1;
    uint64_t virt_addr = (uint64_t)(virPtr);

    *dmaBufFdOffset = 0;

    app_mem_list_t *mem_map_list;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->list_mutex);
    mem_map_list = obj->plist;
    while(mem_map_list != NULL)
    {
        if( (virt_addr >= mem_map_list->mem_data.virt_addr)
            && (virt_addr < (mem_map_list->mem_data.virt_addr+mem_map_list->mem_data.size))
            )
        {
            uint32_t offset = (uint32_t)(virt_addr - mem_map_list->mem_data.virt_addr);

            dmaBufFd = (int)mem_map_list->mem_data.dma_buf_fd;
            *dmaBufFdOffset = offset;
            break;
        }
        mem_map_list = mem_map_list->next;
    }
    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->list_mutex);
    #ifdef APP_MEM_DEBUG
    if (dmaBufFd < 0)
    {
        printf("MEM: ERROR: Failed to export dmaBufFd for virtPtr %p !!!\n", virPtr);
    }
    printf("MEM: Exported dmaBufFd %d @ offset = %d bytes !!!\n", dmaBufFd, *dmaBufFdOffset);
    #endif

    return (uint32_t)(dmaBufFd);
}

int32_t appMemTranslateDmaBufFd(uint32_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    app_mem_list_t *mem_map_list;
    int32_t         status = 0;

    *virtPtr = 0;
    *phyPtr  = 0;

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->list_mutex);
    mem_map_list = obj->plist;
    while(mem_map_list != NULL)
    {
        if (dmaBufFd == mem_map_list->mem_data.dma_buf_fd)
        {
            app_mem_type_t *mem_data = &mem_map_list->mem_data;
            uint32_t offset = (uint32_t)mem_data->offset;

            *virtPtr = mem_data->virt_addr + offset;
            *phyPtr  = mem_data->phys_addr + offset;
            break;
        }

        mem_map_list = mem_map_list->next;
    }

    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->list_mutex);

    if (*virtPtr == 0u)
    {
        /* We do not have the record of the 'dmaBufFd' in the list.
         * Create a record, translate it to 'virtPtr, 'phyPtr' and add a
         * record for later use.
         */
        status = appMemAddTupleToList(dmaBufFd, size, virtPtr, phyPtr);
    }

    if ((*virtPtr == 0u) || (*phyPtr == 0u))
    {
        appLogPrintf("MEM: ERROR: Failed to translate dmaBufFd [%d]\n", dmaBufFd);
        status = -1;
    }

    #ifdef APP_MEM_DEBUG
    printf("MEM: Translated dmaBufFd %d to virtPtr %p and phyPtr %p!!!\n",
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

uint64_t appMemGetVirt2PhyBufPtr(uint64_t virtPtr, uint32_t heap_id)
{
    off_t phyPtr = 0;
    app_mem_obj_t *obj = &g_app_mem_obj;
    /* Find out the physical address of this buffer */

    app_mem_list_t *mem_map_list;

    uint64_t virt_addr = (uint64_t)(virtPtr);

    /* Enter critical section */
    (void)pthread_mutex_lock(&obj->list_mutex);
    mem_map_list = obj->plist;
    while(mem_map_list != NULL)
    {
        if( (virt_addr >= mem_map_list->mem_data.virt_addr)
            && (virt_addr < (mem_map_list->mem_data.virt_addr+mem_map_list->mem_data.size))
            )
        {
            uint32_t offset = (uint32_t)(virt_addr - mem_map_list->mem_data.virt_addr);

            phyPtr = (off_t)(mem_map_list->mem_data.phys_addr) + (off_t)offset;
            break;
        }
        mem_map_list = mem_map_list->next;
    }
    /* Exit critical section */
    (void)pthread_mutex_unlock(&obj->list_mutex);
    #ifdef APP_MEM_DEBUG
    if (phyPtr == 0)
    {
        printf("MEM: ERROR: Failed to get physical address of virt addr = %p !!!\n", (void*)virtPtr);
    }
    printf("MEM: Translated virt addr = %p -> phy addr = %lx !!!\n", (void*)virtPtr, phyPtr);
    #endif

    return (uint64_t)(phyPtr);
}

void *appMemAlloc(uint32_t block, uint32_t size, uint32_t align)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    uint64_t        virtual_ptr = 0;
    int             dma_buf_fd = -1;
    uint64_t        phys_addr = 0;
    uint32_t        flags = DMA_HEAP_ALLOC_FLAGS;
    int             status = -1;

    /* On MPU, direct both shared and non-shared requests to the same
     * shared heap for backward compatibility */
    if((APP_MEM_HEAP_DDR == block) ||
       (APP_MEM_HEAP_DDR_SHARED == block))
    {
        /* align is ignored, all DMA HEAP allcoated buffers are minimum 4KB aligned
        * The alignment information can be found in ${PSDKL}/board-support/linux-*\/
        * dma-buf/dma-heap.c or by running "getconf PAGESIZE" on the target
        */
        /* alloc memory from DMA HEAP */
        status = appMemDmaHeapAlloc(obj->dma_heap_fd,
                                        size,
                                        flags,
                                        &dma_buf_fd
                                    );

        if(status != 0)
        {
            appLogPrintf("MEM: ERROR: Alloc failed with status = %d !!!\n", status);
        }

        if (status == 0)
        {
            /* Create the {dma_buf_fd, virtual_ptr, phys_addr} mapping and add it
            * to the list for later use.
            */
            (void)appMemAddTupleToList((uint32_t)dma_buf_fd, size, &virtual_ptr, &phys_addr);
        }
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block\n");
    }

    return ((void *)(uintptr_t)virtual_ptr);
}

static void appMemFreeListItem(app_mem_list_t *mem_map_list)
{
    app_mem_obj_t *obj = &g_app_mem_obj;
    uint32_t dmaBufFd = (uint32_t)-1;
    uint32_t dmabuf2phys_fd = (uint32_t)-1;
    uint32_t size;

    (void)munmap((void*)mem_map_list->mem_data.virt_addr, mem_map_list->mem_data.size);
    dmaBufFd = mem_map_list->mem_data.dma_buf_fd;
    dmabuf2phys_fd = (uint32_t)mem_map_list->mem_data.dmabuf2phys_fd;
    size = mem_map_list->mem_data.size;
    (void)close((int)dmabuf2phys_fd);
    (void)close((int)dmaBufFd);

    obj->total_free_bytes += size;
    obj->num_free++;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM04
<justification end>*/
    if( (obj->cur_alloc_bytes < size) || (obj->cur_alloc==0u) )
    {
        appLogPrintf("MEM: WARNING: Free'ing more memory than allocated\n");
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM04
<justification end>*/
    else
/* LDRA_JUSTIFY_END */
    {
        obj->cur_alloc_bytes -= size;
        obj->cur_alloc--;
    }

    #ifdef APP_MEM_DEBUG
    appMemPrintMemAllocInfo();
    #endif
}


int32_t appMemFree(uint32_t block, void *virPtr, uint32_t size )
{
    int32_t status = 0;
    uint8_t b_found_addr = 0;

    app_mem_obj_t *obj = &g_app_mem_obj;
    app_mem_list_t *mem_map_list;

    /* On MPU, direct both shared and non-shared requests to the same
     * shared heap for backward compatibility */
    if((APP_MEM_HEAP_DDR == block) ||
       (APP_MEM_HEAP_DDR_SHARED == block))
    {
        (void)pthread_mutex_lock(&(obj->list_mutex));
        mem_map_list = obj->plist;

        while(mem_map_list != NULL)
        {
            if(mem_map_list->mem_data.virt_addr == (uint64_t)(virPtr))
            {
                #ifdef APP_MEM_DEBUG
                printf("MEM: Freeing virt addr = %p -> phy addr = %lx, size = %d (FD's = %d %d) !!!\n",
                    (void*)virPtr, mem_map_list->mem_data.phys_addr, size,
                    mem_map_list->mem_data.dmabuf2phys_fd,
                    mem_map_list->mem_data.dma_buf_fd
                    );
                #endif

                appMemFreeListItem(mem_map_list);

                b_found_addr = 1;
                break;
            }
            mem_map_list = mem_map_list->next;
        }

        if(1u != b_found_addr)
        {
            status = -1;
            appLogPrintf("MEM: ERROR: Failed to free memory at virt addr = %p !!!\n", virPtr);
        }
        else
        {
            appMemRemoveFromList(&(obj->plist), mem_map_list);
        }
        (void)pthread_mutex_unlock(&(obj->list_mutex));
    }
    else
    {
        appLogPrintf("MEM: ERROR: Invalid memory block\n");
        status = -1;
    }

    return(status);
}

int32_t appMemStats(uint32_t heap_id, app_mem_stats_t *stats)
{
    if( (heap_id==APP_MEM_HEAP_DDR_SHARED) && (stats != NULL) )
    {
        stats->heap_id = heap_id;
        (void)strncpy(stats->heap_name, "DDR", APP_MEM_HEAP_NAME_MAX);
        stats->heap_name[APP_MEM_HEAP_NAME_MAX-1u]=(char)0;
        stats->heap_size = 0;
        stats->free_size = 0;
    }
    else
    {
        if(stats!=NULL)
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

static void appMemFreeList(app_mem_list_t *list)
{
    app_mem_list_t *tmp;
    while(list != NULL)
    {
        tmp = list;
        list = list->next;
        appMemFreeListItem(tmp);
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

static int32_t appMemAddTupleToList(uint32_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    void           *vPtr = NULL;
    app_mem_type_t  m;
    int32_t         dmabuf2phys_fd = -1;
    int32_t         status = 0;

    *virtPtr = 0;
    *phyPtr  = 0;

    if (size == 0u)
    {
        appLogPrintf("MEM: ERROR: Mapped memory size cannot be 0.\n");
        status = -1;
    }

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
        if (vPtr == MAP_FAILED)
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
        (void)pthread_mutex_lock(&(obj->list_mutex));

        status = appMemPushToList(&(obj->plist), &m);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_DMA_HEAP_UBR007
<justification end>*/
        if(status==0)
        {
            obj->total_alloc_bytes += size;
            obj->cur_alloc_bytes += size;
            obj->num_alloc++;
            obj->cur_alloc++;

            #ifdef APP_MEM_DEBUG
            appMemPrintMemAllocInfo();
            #endif
        }
/* LDRA_JUSTIFY_END */
        /* Exit critical section */
        (void)pthread_mutex_unlock(&(obj->list_mutex));
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

    if(status==0)
    {
        #ifdef APP_MEM_DEBUG
        printf("MEM: Allocated virt addr = %lx -> phy addr = %lx, size = %d (FD's = %d %d) !!!\n",
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
        if (vPtr == MAP_FAILED)
        {
            vPtr = NULL;
        }
        else
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

    #ifdef APP_MEM_DEBUG
    printf("MEM: Translated dmaBufFd %d to virtPtr %p and phyPtr %p!!!\n",
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
    return shared_ptr;
}

uint32_t appMemGetNumAllocs(void)
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    return (uint32_t)obj->cur_alloc;
}

void *appMemMap(void *phys_ptr, uint32_t size)
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
        printf("APP_MEM: Mapping %p ...\n", phys_ptr);
        #endif
        /* Mapping this physical address to linux user space */
        taddr = (uintptr_t)phys_ptr;
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
        if(virt_ptr==MAP_FAILED)
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
            virt_ptr = (void*)((uintptr_t)virt_ptr + ((uintptr_t)phys_ptr % pageSize));
        }
        #ifdef APP_MEM_DEBUG
        printf("APP_MEM: Mapped %p -> %p of size %d bytes \n", phys_ptr, virt_ptr, size);
        #endif
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_DMA_HEAP_UM12
<justification end>*/
    if(virt_ptr==NULL)
    {
        appLogPrintf("APP_MEM: ERROR: Unable to map memory @ %p of size %d bytes !!!\n", phys_ptr, size);
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
    printf("APP_MEM: UnMapped memory at virtual address @ 0x%p of size %d bytes \n", virt_ptr, size);
    #endif

    taddr = (uint64_t)virt_ptr;
    tsize = size;

    tsize = (uint32_t)appAlign((tsize + (uint32_t)(taddr % pageSize)), pageSize);
    taddr = (uintptr_t)appFloor(taddr, pageSize);

    (void)munmap((void *)taddr, tsize);

    return status;
}
