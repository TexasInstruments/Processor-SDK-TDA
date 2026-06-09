/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include <string.h>
#include <utils/mem/include/app_mem.h>
#include <ti/osal/HeapP.h>
#include <ti/osal/MemoryP.h>
#include <ti/osal/HwiP.h>
#include <ti/osal/CacheP.h>

/** \brief Minmum number of bytes that will be used for alignment, MUST >= max CPU cache line size */
#define APP_MEM_ALIGN_MIN_BYTES     (256u)

typedef struct
{
    uint16_t is_valid;
    HeapP_Handle rtos_heap_handle;
    uint32_t alloc_offset;
    app_mem_heap_prm_t heap_prm;
} app_mem_heap_obj_t;

typedef struct
{
    app_mem_heap_obj_t heap_obj[APP_MEM_HEAP_MAX];
} app_mem_obj_t;

static app_mem_obj_t g_app_mem_obj;

void appMemInitPrmSetDefault(app_mem_init_prm_t *prm)
{
    uint32_t heap_id;

    for (heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
    {
        app_mem_heap_prm_t *heap_prm;

        heap_prm = &prm->heap_info[heap_id];

        heap_prm->base = NULL;
        heap_prm->size = 0u;
        heap_prm->name[0u] = 0u;
        heap_prm->flags = 0;
    }
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    int32_t status = 0;

    uint32_t heap_id;

    for (heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
    {
        app_mem_heap_prm_t *heap_prm;
        app_mem_heap_obj_t *heap_obj;

        heap_prm = &prm->heap_info[heap_id];
        heap_obj = &g_app_mem_obj.heap_obj[heap_id];

        heap_obj->is_valid = 0;
        heap_obj->alloc_offset = 0;
        heap_obj->rtos_heap_handle = NULL;

        if ((heap_prm->base == NULL) || (heap_prm->size == 0))
        {
            /* no heap specified by user */
        }
        else
        {
            /* copy user parameters to local object */
            heap_obj->heap_prm = *heap_prm;
            /* point local variable to local object */
            heap_prm = &heap_obj->heap_prm;

            heap_obj->is_valid = 1;

            if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                /* no rtos heap, linear allocator based on offset */
                heap_obj->alloc_offset = 0;
            }
            else
            {
                /* create a rtos heap */
                HeapP_Params rtos_heap_prm;

                HeapP_Params_init(&rtos_heap_prm);
                rtos_heap_prm.buf = APP_MEM_ALIGNPTR(
                    heap_prm->base, APP_MEM_ALIGN_MIN_BYTES);
                rtos_heap_prm.size = APP_MEM_ALIGN32(
                    heap_prm->size, APP_MEM_ALIGN_MIN_BYTES);

                heap_obj->rtos_heap_handle = HeapP_create(&rtos_heap_prm);

                heap_prm->base   = rtos_heap_prm.buf;
                heap_prm->size   = rtos_heap_prm.size;
            }
        }
    }

    return status;
}

int32_t appMemDeInit(void)
{
    int32_t status = 0;

    uint32_t heap_id;

    for (heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
    {
        app_mem_heap_obj_t *heap_obj;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];

        if (heap_obj->is_valid)
        {
            if(heap_obj->rtos_heap_handle != NULL)
            {
                HeapP_delete(&heap_obj->rtos_heap_handle);
            }

            heap_obj->is_valid = 0;
            heap_obj->alloc_offset = 0;
            heap_obj->rtos_heap_handle = NULL;
        }
    }

    return status;
}

void *appMemAlloc(uint32_t heap_id,
                  uint32_t size,
                  uint32_t align)
{
    void *ptr = NULL;

    if (heap_id < APP_MEM_HEAP_MAX && size != 0)
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        if (heap_obj->is_valid)
        {
            if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uint32_t offset;
                uintptr_t key;

                key = HwiP_disable();

                offset = APP_MEM_ALIGN32(heap_obj->alloc_offset, align);

                if ((offset + size) <= heap_prm->size)
                {
                    ptr = (void *)((uintptr_t)heap_prm->base + offset);

                    heap_obj->alloc_offset = (offset + size);
                }

                HwiP_restore(key);
            }
            else
            {
                if(heap_obj->rtos_heap_handle!=NULL)
                {
                    size  = APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES);

                    ptr = HeapP_alloc(heap_obj->rtos_heap_handle,
                               size);

                   if(ptr!=NULL)
                    {
                        if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_DO_CLEAR_ON_ALLOC))
                        {
                            memset(ptr, 0, size);
                        }
                        if( (heap_prm->flags & APP_MEM_HEAP_FLAGS_IS_SHARED))
                        {
                            CacheP_wbInv(ptr, size);
                        }
                    }
                }
            }
        }
    }
    return ptr;
}

int32_t appMemFree(uint32_t heap_id,
                   void *ptr,
                   uint32_t size)
{
    int32_t status = -1;

    if (heap_id < APP_MEM_HEAP_MAX && ptr != NULL && size != 0)
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        if (heap_obj->is_valid)
        {
            if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uintptr_t key;

                key = HwiP_disable();

                heap_obj->alloc_offset = 0;

                HwiP_restore(key);

                status = 0;
            }
            else
            {
                if(heap_obj->rtos_heap_handle != NULL && ptr != NULL && size != 0)
                {
                    size = APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES);

                    if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_IS_SHARED))
                    {
                        CacheP_wbInv(ptr, size);
                    }

                    HeapP_free(heap_obj->rtos_heap_handle,
                                ptr,
                                size);

                    status = 0;
                }
            }
        }
    }

    return status;
}

int32_t appMemStats(uint32_t heap_id,
                    app_mem_stats_t *stats)
{
    int32_t status = -1;

    if (heap_id < APP_MEM_HEAP_MAX && stats != NULL)
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        if (stats != NULL)
        {
            stats->heap_id = APP_MEM_HEAP_MAX;
            stats->heap_name[0] = 0;
            stats->heap_size = 0;
            stats->free_size = 0;
        }

        if (heap_obj->is_valid)
        {
            stats->heap_id = heap_id;
            strncpy(stats->heap_name, heap_prm->name, APP_MEM_HEAP_NAME_MAX);
            stats->heap_size = heap_prm->size;

            if ((heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uintptr_t key;

                key = HwiP_disable();

                stats->free_size = heap_prm->size - heap_obj->alloc_offset;

                HwiP_restore(key);

                status = 0;
            }
            else
            {
                if(heap_obj->rtos_heap_handle!=NULL)
                {
                    HeapP_MemStats rtos_heap_stats;

                    HeapP_getHeapStats(heap_obj->rtos_heap_handle, &rtos_heap_stats);

                    stats->free_size = rtos_heap_stats.totalFreeSize;

                    status = 0;
                }
            }
        }
    }

    return status;
}
