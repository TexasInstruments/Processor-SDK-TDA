/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/resource.h>

#include <utils/mem/include/app_mem.h>
#include <utils/mem/include/app_mem_limits.h>

#if defined(SOC_FAMILY_TDA5)
#include <Ipc_Notify_Hal_Hostemu.h>
#endif /* #if defined(SOC_FAMILY_TDA5) */

/* #define APP_MEM_DEBUG */

#define MAX_SHM_BUFF_NAME_LEN   (256U)
#define MAX_SHM_PATH_NAME_LEN   (64U)

typedef struct
{
    uint32_t buf_id;
    uint32_t buf_fd;
    uint64_t phys_addr;
    uint64_t virt_addr;
    uint64_t offset;
    uint32_t size;
} app_mem_type_t;

typedef struct mem_type_node
{
    app_mem_type_t          mem_data;
    struct mem_type_node   *next;
} app_mem_list_t;

typedef struct
{
    app_mem_list_t     *plist;
    pthread_mutex_t     list_mutex;
    int32_t             dma_heap_fd;

    uint64_t            total_alloc_bytes;
    uint64_t            total_free_bytes;
    uint64_t            cur_alloc_bytes;
    uint64_t            num_alloc;
    uint64_t            num_free;
    uint64_t            cur_alloc;
    uint32_t            next_free_id;
    char                shm_name[MAX_SHM_BUFF_NAME_LEN-MAX_SHM_PATH_NAME_LEN-MAX_SHM_PATH_NAME_LEN];

} app_mem_obj_t;

/*GLOBALS*/
int mkstemp(char *template);

static app_mem_obj_t g_app_mem_obj;

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

/* Counter for tracking the {init, de-init} calls. This is also used to
 * guarantee a single init/de-init operation.
 */
static uint32_t gInitCount = 0U;

/* Name of path in file system where file handles are allocated */
static char fd_path[MAX_SHM_PATH_NAME_LEN] = {0};


/* Return true if specified memory region is enabled, return false otherwise*/
bool appMemRegionQuery(uint32_t heap_id)
{
    return ((heap_id == APP_MEM_HEAP_DDR) || (heap_id == APP_MEM_HEAP_DDR_SHARED));
}

void appMemPrintMemAllocInfo()
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    printf("MEM: Alloc's: %ld alloc's of %ld bytes \n", obj->num_alloc, obj->total_alloc_bytes );
    printf("MEM: Free's : %ld free's  of %ld bytes \n", obj->num_free, obj->total_free_bytes );
    printf("MEM: Open's : %ld allocs  of %ld bytes \n", obj->cur_alloc, obj->cur_alloc_bytes );
}

/* Function to increase the number of open files at a process
 * level. The default number of open files is 1024 and this causes
 * memory allocation errors since the PC emulation mode uses shm_open()
 * for memory allocations.
 *
 * The logic below allows setting the upper limit on the max number of
 * open files.
 */
static int32_t increaseOpenFileLimits(int32_t  maxOpenFiles)
{
  struct rlimit limit;
  
  limit.rlim_cur = maxOpenFiles;
  limit.rlim_max = maxOpenFiles;

  if (setrlimit(RLIMIT_NOFILE, &limit) != 0)
  {
      printf("setrlimit() failed with errno=%d\n", errno);
      return -1;
  }

  /* Get max number of files. */
  if (getrlimit(RLIMIT_NOFILE, &limit) != 0)
  {
      printf("getrlimit() failed with errno=%d\n", errno);
      return -1;
  }

  printf("The soft limit is %lu\n", limit.rlim_cur);
  printf("The hard limit is %lu\n", limit.rlim_max);

  return 0;
}

static void appMemFreeListItem(app_mem_list_t *mem_map_list)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    app_mem_type_t *m = &mem_map_list->mem_data;
    char            name[MAX_SHM_BUFF_NAME_LEN];

    munmap((void*)m->virt_addr, m->size);
    close(m->buf_fd);

    obj->total_free_bytes += m->size;
    obj->num_free++;

    if (obj->cur_alloc_bytes < m->size || obj->cur_alloc==0)
    {
        printf("MEM: WARNING: Free'ing more memory than allocated\n");
    }
    else
    {
        obj->cur_alloc_bytes -= m->size;
        obj->cur_alloc--;
    }

    sprintf(name, "%s/%s_%d", fd_path, obj->shm_name, m->buf_id);

    if(fd_path[0] == 0)
    {
        shm_unlink(name);
    }
    else
    {
        unlink(name);
    }

#ifdef APP_MEM_DEBUG
    appMemPrintMemAllocInfo();
#endif

}

static int appMemPushToList(app_mem_list_t **list, app_mem_type_t *data)
{
    app_mem_list_t *new_node = (app_mem_list_t *)malloc(sizeof(app_mem_list_t));
    int status = -1;

    if (NULL != new_node)
    {
        new_node->mem_data = *data;
        new_node->next     = *list;
        *list              = new_node;
        status             = 0;
    }

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

void appMemRemoveFromList(app_mem_list_t **list, app_mem_list_t *node)
{
    app_mem_list_t *curr = *list;
    app_mem_list_t *prev = *list;

    while(curr != NULL)
    {
        /*Check for match found*/
        if (curr == node)
        {
            /*Check if match found is HEAD*/
            if (*list == node)
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

static int32_t appMemAddTupleToList(uint64_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    void           *vPtr = NULL;
    app_mem_type_t  m;
    int32_t         status = 0;

    *virtPtr = 0;
    *phyPtr  = 0;

    if (size == 0)
    {
        printf("MEM: ERROR: Mapped memory size cannot be 0.\n");
        status = -1;
    }

    if (status == 0)
    {
        vPtr = mmap((void *)0x00000000u,
                    size,
                    PROT_WRITE | PROT_READ,
                    MAP_SHARED,
                    dmaBufFd,
                    0u);

        if (vPtr == MAP_FAILED)
        {
            printf("MEM: ERROR: Failed to translate dmaBufFd %ld to "
                   "virtPtr!!! errno=%d (%s)\n", dmaBufFd, errno, strerror(errno));
            status = -1;
        }
    }

    /* add to local linked list for later retrival and conversion */
    if (status == 0)
    {
        *virtPtr = (uint64_t)vPtr;
        *phyPtr  = *virtPtr;

        m.buf_id    = obj->next_free_id;
        m.buf_fd    = dmaBufFd;
        m.phys_addr = *phyPtr;
        m.virt_addr = *virtPtr;
        m.offset    = 0;
        m.size      = size;

        status = appMemPushToList(&(obj->plist), &m);

        if (status == 0)
        {
            obj->total_alloc_bytes += size;
            obj->cur_alloc_bytes += size;
            obj->num_alloc++;
            obj->cur_alloc++;

#ifdef APP_MEM_DEBUG
            appMemPrintMemAllocInfo();
#endif
        }

        if (status!=0)
        {
            printf("MEM: ERROR: appMemPushToList failed !!!\n");
        }
    }

    if (status == 0)
    {
#ifdef APP_MEM_DEBUG
        printf("MEM: Allocated virt addr = %lx -> phy addr = %lx, size = %d (FD = %ld) !!!\n",
               *virtPtr, *phyPtr, size, dmaBufFd);
#endif
    }
    else
    {
        /* release resources incase of alloc failure */
        printf("MEM: ERROR: memory alloc of size = %d bytes, failed with status = %d !!!\n",
               size, status);

        if (vPtr == MAP_FAILED)
        {
            vPtr = NULL;
        }
        else
        {
            munmap(vPtr, size);
        }

        if (dmaBufFd >= 0)
        {
            close(dmaBufFd);
        }

        appMemPrintMemAllocInfo();
    }

    if ((*virtPtr == 0) || (*phyPtr == 0))
    {
        printf("MEM: ERROR: Failed to translate dmaBufFd [%ld]\n", dmaBufFd);
        status = -1;
    }

#ifdef APP_MEM_DEBUG
    printf("MEM: Translated dmaBufFd %ld to virtPtr %p and phyPtr %p!!!\n",
           dmaBufFd, (void*)*virtPtr, (void*)*phyPtr);
#endif

    return status;
}

static int32_t appMemInitLocal(app_mem_init_prm_t *prm)
{
    int32_t fd;
    int32_t status = 0;

    /* 'prm' is unused in this function. */
    (void)prm;

    status = increaseOpenFileLimits(APP_MEM_HEAP_MAX_NUM_MEM_BLKS_IN_USE);

    if (status < 0)
    {
        printf("MEM: Init ... (increaseOpenFileLimits) failed!!!\n");
        perror("MEM:");
    }
    else
    {
        /* Initialize our book-keeping structures */
        app_mem_obj_t *obj = &g_app_mem_obj;

        printf("MEM: Init ... !!!\n");

        obj->total_alloc_bytes = 0;
        obj->total_free_bytes  = 0;
        obj->cur_alloc_bytes   = 0;
        obj->num_alloc         = 0;
        obj->num_free          = 0;
        obj->cur_alloc         = 0;
        obj->next_free_id      = 0;
        obj->plist             = NULL;
        obj->dma_heap_fd       = -1;

        /* Within the same system, the names used for creating the shared memory
         * buffers has to be unique for avoiding memory allocation error. So, create
         * a unique string per process based on a template string.
         */
        strcpy(obj->shm_name, "vashm_buff_XXXXXX");

        fd = mkstemp(obj->shm_name);

        if (fd < 0)
        {
            printf("MEM: Init ... (mkstemp) failed!!!\n");
            perror("MEM:");
            status = -1;
        }
        else
        {
            close(fd);

            /* Delete the file created by mkstemp() call. */
            unlink(obj->shm_name);
        }

        /* Initialize the pthread mutex */
        pthread_mutex_init(&obj->list_mutex, NULL);
    }

    printf("MEM: Init ... Done !!!\n");

    return status;
}

static int32_t appMemDeInitLocal(void)
{
    int32_t status = 0;

    app_mem_obj_t *obj = &g_app_mem_obj;

    printf("MEM: Deinit ... !!!\n");

    pthread_mutex_lock(&obj->list_mutex);

    /* Clear the heap off our memory map list */
    appMemFreeList(obj->plist);

    appMemPrintMemAllocInfo();

    pthread_mutex_unlock(&obj->list_mutex);

    printf("MEM: Deinit ... Done !!!\n");

    return(status);
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    int32_t status = 0;

    pthread_mutex_lock(&gMutex);

    if (gInitCount == 0U)
    {
        status = appMemInitLocal(prm);
    }

    gInitCount++;

    pthread_mutex_unlock(&gMutex);

    return status;
}

int32_t appMemDeInit()
{
    int32_t status = 0;

    pthread_mutex_lock(&gMutex);

    if (gInitCount != 0U)
    {
        gInitCount--;

        if (gInitCount == 0U)
        {
            status = appMemDeInitLocal();
        }
    }
    else
    {
        status = -1;
    }

    pthread_mutex_unlock(&gMutex);

    return status;
}

uint64_t appMemGetDmaBufFd(void *virPtr, volatile uint32_t *dmaBufFdOffset)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    uint64_t        virt_addr = (uint64_t)(virPtr);
    int             dmaBufFd = -1;

    *dmaBufFdOffset = 0;

    /* Enter critical section */
    pthread_mutex_lock(&obj->list_mutex);

    app_mem_list_t *mem_map_list = obj->plist;

    while (mem_map_list != NULL)
    {
        if ( (virt_addr >= mem_map_list->mem_data.virt_addr)
            && (virt_addr < (mem_map_list->mem_data.virt_addr+mem_map_list->mem_data.size))
            )
        {
            uint32_t offset = (virt_addr - mem_map_list->mem_data.virt_addr);

            dmaBufFd = mem_map_list->mem_data.buf_fd;
            *dmaBufFdOffset = offset;
            break;
        }

        mem_map_list = mem_map_list->next;
    }

    /* Exit critical section */
    pthread_mutex_unlock(&obj->list_mutex);

#ifdef APP_MEM_DEBUG
    if (dmaBufFd < 0)
    {
        printf("MEM: ERROR: Failed to export dmaBufFd for virtPtr %p !!!\n", virPtr);
    }

    printf("MEM: Exported dmaBufFd %d @ offset = %d bytes !!!\n", dmaBufFd, *dmaBufFdOffset);
#endif

    return (uint32_t)(dmaBufFd);
}

int32_t appMemTranslateDmaBufFd(uint64_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    app_mem_list_t *mem_map_list = obj->plist;
    int32_t         status = 0;

    *virtPtr = 0;
    *phyPtr  = 0;

    /* Enter critical section */
    pthread_mutex_lock(&obj->list_mutex);

    while (mem_map_list != NULL)
    {
        if (dmaBufFd == mem_map_list->mem_data.buf_fd)
        {
            app_mem_type_t *mem_data = &mem_map_list->mem_data;
            uint32_t offset = mem_data->offset;

            *virtPtr = mem_data->virt_addr + offset;
            *phyPtr  = mem_data->phys_addr + offset;
            break;
        }

        mem_map_list = mem_map_list->next;
    }

    if (*virtPtr == 0)
    {
        /* We do not have the record of the 'dmaBufFd' in the list.
         * Create a record, translate it to 'virtPtr, 'phyPtr' and add a
         * record for later use.
         */
        status = appMemAddTupleToList(dmaBufFd, size, virtPtr, phyPtr);
    }

    if ((*virtPtr == 0) || (*phyPtr == 0))
    {
        printf("MEM: ERROR: Failed to translate dmaBufFd [%ld]\n", dmaBufFd);
        status = -1;
    }

    /* Exit critical section */
    pthread_mutex_unlock(&obj->list_mutex);

#ifdef APP_MEM_DEBUG
    printf("MEM: Translated dmaBufFd %ld to virtPtr %p and phyPtr %p!!!\n",
           dmaBufFd, (void*)*virtPtr, (void*)*phyPtr);
#endif

    return status;
}

uint64_t appMemGetVirt2PhyBufPtr(uint64_t virtPtr, uint32_t heap_id)
{
    return (uint64_t)virtPtr;
}

void *appMemAlloc(uint32_t block, uint32_t size, uint32_t align)
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    uint64_t        virtual_ptr = 0;
    int32_t         buf_fd = -1;
    uint64_t        phys_addr = 0;
    int32_t         status = 0;
    char            name[MAX_SHM_BUFF_NAME_LEN];

    name[0] = '\0';

    /* Enter critical section */
    pthread_mutex_lock(&(obj->list_mutex));

    if (gInitCount == 0U)
    {
        status = -1;
        printf("MEM: ERROR: Memory allocator has not been initialized.\n");
    }

    if ((status == 0) && (size == 0))
    {
        printf("MEM: ERROR: Alloc failed. Invalid size: %d bytes\n", size);
        status = -1;
    }

    if (status == 0)
    {
        int32_t oflag = O_RDWR | O_CREAT | O_EXCL;
        mode_t  mode = S_IRUSR | S_IWUSR;

        /* Need to prepend "/" to the name as per the convention.
         * Files are created under /dev/shm directory, i.e. file
         * names like /dev/shm/vashm_buff_<randomstring>_<integer>
         */
        sprintf(name, "%s/%s_%d", fd_path, obj->shm_name, obj->next_free_id);

        if(fd_path[0] == 0)
        {
            buf_fd = shm_open(name, oflag, mode);
        }
        else
        {
            buf_fd = open(name, oflag, mode);
        }

        if (buf_fd < 0)
        {
            printf("MEM: ERROR: Alloc failed (shm_open)\n");

            if (EEXIST == errno)
            {
                printf("MEM: ERROR: Shared memory handle name (%s) is already "
                       "open. Perhaps appMemInit (or appInit) is not called "
                       "first.\n", name);
            }

#ifdef APP_MEM_DEBUG
            appMemPrintMemAllocInfo();
#endif

            status = -1;
        }
    }

    if (status == 0)
    {
        status = ftruncate(buf_fd, size);

        if (status < 0)
        {
            printf("MEM: ERROR: Alloc failed (ftruncate)\n");
            close(buf_fd);
            buf_fd = -1;
            if (fd_path[0] == 0)
            {
                shm_unlink(name);
            }
            else
            {
                unlink(name);
            }
        }
    }

    if (status == 0)
    {
        /* Touch one byte per page to force physical page allocation on
         * /dev/shm.  ftruncate creates a sparse file: mmap()
         * succeeds but accessing an unallocated page causes SIGBUS when
         * the filesystem is full.  Writing to each page boundary here
         * converts that deferred SIGBUS into a clean write error. */
        static const char zero = 0;
        uint32_t page_off;
        for (page_off = 0U; page_off < size && status == 0; page_off += (uint32_t)sysconf(_SC_PAGE_SIZE))
        {
            if (lseek(buf_fd, (off_t)page_off, SEEK_SET) < 0 ||
                write(buf_fd, &zero, 1) != 1)
            {
                printf("MEM: ERROR: Alloc failed writing page at offset %u\n", page_off);
                close(buf_fd);
                buf_fd = -1;
                if (fd_path[0] == 0)
                {
                    shm_unlink(name);
                }
                else
                {
                    unlink(name);
                }
                status = -1;
            }
        }
    }

    if (status == 0)
    {
        /* Create the {buf_fd, virtual_ptr, phys_addr} mapping and add it
         * to the list for later use.
         */
        status = appMemAddTupleToList(buf_fd, size, &virtual_ptr, &phys_addr);
    }

    if (status == 0)
    {
        obj->next_free_id++;
    }

    /* Exit critical section */
    pthread_mutex_unlock(&obj->list_mutex);

    return ((void *)(uintptr_t)virtual_ptr);
}

int32_t appMemFree(uint32_t block, void *virPtr, uint32_t size )
{
    app_mem_obj_t  *obj = &g_app_mem_obj;
    app_mem_list_t *mem_map_list = obj->plist;
    int32_t         status = 0;
    uint8_t         b_found_addr = 0;

    /* Enter critical section */
    pthread_mutex_lock(&(obj->list_mutex));

    while(mem_map_list != NULL)
    {
        if (mem_map_list->mem_data.virt_addr == (uint64_t)(virPtr))
        {
#ifdef APP_MEM_DEBUG
            printf("MEM: Freeing virt addr = %p -> phy addr = %lx, size = %d (FD = %d) !!!\n",
                (void*)virPtr, mem_map_list->mem_data.phys_addr, size,
                mem_map_list->mem_data.buf_fd
                );
#endif

            appMemFreeListItem(mem_map_list);

            b_found_addr = 1;
            break;
        }

        mem_map_list = mem_map_list->next;
    }

    if (1 != b_found_addr)
    {
        status = -1;
        printf("MEM: ERROR: Failed to free memory at virt addr = %p !!!\n", virPtr);
    }
    else
    {
        appMemRemoveFromList(&(obj->plist), mem_map_list);
    }

    /* Exit critical section */
    pthread_mutex_unlock(&(obj->list_mutex));

    return(status);
}

uint64_t appMemShared2PhysPtr(uint64_t shared_ptr, uint32_t heap_id)
{
    return shared_ptr;
}

int32_t appMemResetScratchHeap(uint32_t heap_id)
{
    return 0;
}

uint64_t appMemShared2TargetPtr(uint64_t shared_ptr)
{
#if defined (SOC_FAMILY_TDA5)
    if (IpcNotify_Hal_VdkGetVdkWrapperObj() != NULL)
    {
        shared_ptr = IpcNotify_Hal_VdkGetHostPtr(shared_ptr);
    }
#endif /* #if defined (SOC_FAMILY_TDA5) */
    return shared_ptr;
}

uint32_t appMemGetNumAllocs()
{
    app_mem_obj_t *obj = &g_app_mem_obj;

    return obj->cur_alloc;
}

#if defined(x86_64)
int32_t appMemSetFdPath(char *path, uint32_t size)
{
    int32_t status = 0;

    if (size < MAX_SHM_PATH_NAME_LEN)
    {
        strncpy(fd_path, path, size);
    }
    else
    {
        printf("MEM: ERROR: appMemSetFdPath : size is >= MAX_SHM_PATH_NAME_LEN \n");
        status = -1;
    }
    return status;
}
#endif

void *appMemMap(void *phys_addr, uint32_t size)
{
    return phys_addr;
}

void appMemCacheInv(void *ptr, uint32_t size)
{
    return;
}

void  appMemCacheWbInv(void *ptr, uint32_t size)
{
    return;
}
