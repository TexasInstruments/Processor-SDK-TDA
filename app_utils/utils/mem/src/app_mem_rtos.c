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

#include <string.h>
#include <utils/mem/include/app_mem.h>
#include <HeapP.h>
#include <HwiP.h>
#if defined(MCU_SDK)
#include <arch_cache.h>
#elif defined (MCU_PLUS_SDK)
#include <CacheP.h>
#endif

#if defined(PDK)
#include <MemoryP.h>
#endif

#include <utils/console_io/include/app_log.h>

#if defined(R5F) && (defined(SOC_J784S4) || defined(SOC_J721S2) || defined(SOC_J742S2))
#include <ti/csl/csl_rat.h>
#endif

/* #define APP_MEM_DEBUG */

#define ENABLE_CACHE_OPS

#if 0
#if defined(__C7100__) || defined(__C7120__)
#undef ENABLE_CACHE_OPS /* since C7x is coherent */
#elif defined(__C7504__)
#define ENABLE_CACHE_OPS /* since C7504 is incoherent */
#endif
#endif

#if defined(C7X_FAMILY)
#include <c6x_migration.h>
#include <c7x.h>
#endif

/** \brief Minimum number of bytes that will be used for cache operations, MUST >= max CPU cache line size */
#define APP_MEM_ALIGN_MIN_BYTES     (128u)

#if (defined (SOC_J784S4) || defined (SOC_J742S2)) && defined (C7120)

#define APP_MEM_DMC_L1DWBINV_WBINV_MASK                             (0x00000001ULL)
#define APP_MEM_DMC_L1DWBINV_WBINV_SHIFT                            (0ULL)
#define APP_MEM_DMC_L1DWBINV_WBINV_RESETVAL                         (0x00000000U)
#define APP_MEM_DMC_L1DWBINV_WBINV_MAX                              (0x00000001U)
#define APP_MEM_UMC_L2WBINV_WBINV_MASK                              (0x00000001ULL)
#define APP_MEM_UMC_L2WBINV_WBINV_SHIFT                             (0ULL)
#define APP_MEM_UMC_L2WBINV_WBINV_RESETVAL                          (0x00000000ULL)
#define APP_MEM_UMC_L2WBINV_WBINV_MAX                               (0x00000001ULL)

__asm__ __volatile__("appMemC7xSetL1DWBINV: \n"
                     "    MVC   .S1 A4,     ECR259; \n"
                     "    RET   .B1\n"
          );
__asm__ __volatile__("appMemC7xGetL1DWBINV: \n"
        " MVC   .S1 ECR259, A4  ; \n"
          " RET .B1; \n"
       );

__asm__ __volatile__("appMemC7xSetL2WBINV: \n"
                     "    MVC   .S1 A4,     ECR387  ; \n"
                     "    RET   .B1\n"
          );
__asm__ __volatile__("appMemC7xGetL2WBINV: \n"
        " MVC   .S1 ECR387, A4  ; \n"
          " RET .B1; \n"
       );

void appMemC7xCleaninvalidateL1DCache(void)
{
    volatile uint64_t wbinv;
    wbinv  = appMemC7xGetL1DWBINV();
    wbinv  &= ~APP_MEM_DMC_L1DWBINV_WBINV_MASK;
    wbinv |= (0x1ULL << APP_MEM_DMC_L1DWBINV_WBINV_SHIFT) & APP_MEM_DMC_L1DWBINV_WBINV_MASK;
    appMemC7xSetL1DWBINV(wbinv);
    return;
}

void appMemC7xCleaninvalidateL2Cache(void)
{
    volatile uint64_t       wbinv;
    wbinv = appMemC7xGetL2WBINV() & ~APP_MEM_UMC_L2WBINV_WBINV_MASK;
    wbinv |= (0x1U << APP_MEM_UMC_L2WBINV_WBINV_SHIFT) & APP_MEM_UMC_L2WBINV_WBINV_MASK;
    appMemC7xSetL2WBINV(wbinv);
    return;
}
#endif

typedef struct {

    uint16_t is_valid;

    #if defined(PDK)
    HeapP_Handle rtos_heap_handle;
    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
    HeapP_Object rtos_heap_handle;
    #endif

    uint32_t alloc_offset;
    uint32_t max_offset;
    app_mem_heap_prm_t heap_prm;

} app_mem_heap_obj_t;

typedef struct {

    app_mem_heap_obj_t          heap_obj[APP_MEM_HEAP_MAX];
    /* JIRA: ADASVISION-7146 - Temporarily disabled, to be enabled once udma utils are up*/
    #if defined(SOC_FAMILY_AM) || defined(SOC_FAMILY_J7)
    Udma_VirtToPhyFxn           virtToPhyFxn;
    #endif
    app_mem_shared_target_fxn   shared2TargetFxn;
    app_mem_target_shared_fxn   target2SharedFxn;
    #if defined(R5F) && (defined(SOC_J784S4) || defined(SOC_J721S2) || defined(SOC_J742S2))
    /**< The CSL RAT base address.  Different across main domain and MCU domain */
    CSL_ratRegs *pRatRegs;
    #endif
} app_mem_obj_t;

static app_mem_obj_t g_app_mem_obj;
static void appMemFence(void);

/* Return true if specified memory region is enabled, return false otherwise*/
bool appMemRegionQuery(uint32_t heap_id)
{
    return g_app_mem_obj.heap_obj[heap_id].is_valid != 0u;
}

void appMemInitPrmSetDefault(app_mem_init_prm_t *prm)
{
    uint32_t heap_id;

    if(NULL != prm)
    {
        /* JIRA: ADASVISION-7146 - Temporarily disabled, to be enabled once udma utils are up*/
        #if !defined(SOC_TDA54)
        prm->virtToPhyFxn     = NULL;
        #endif
        prm->shared2TargetFxn = NULL;
        prm->target2SharedFxn = NULL;

        #if defined(R5F) && (defined(SOC_J784S4) || defined(SOC_J721S2) || defined(SOC_J742S2))
        g_app_mem_obj.pRatRegs = NULL;
        #endif

        for(heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
        {
            app_mem_heap_prm_t *heap_prm;

            heap_prm = &prm->heap_info[heap_id];

            heap_prm->base = NULL;
            heap_prm->size = 0u;
            heap_prm->name[0u] = (char)0;
            heap_prm->flags = 0;
            heap_prm->align = 256U;
        }
    }
    else
    {
        appLogPrintf("ERROR: prm is NULL !!!\n");
    }
}

int32_t appMemInit(app_mem_init_prm_t *prm)
{
    int32_t status = 0;

    uint32_t heap_id;

    appLogPrintf("MEM: Init ... !!!\n");

    if(NULL != prm)
    {
        /* JIRA: ADASVISION-7146 - Temporarily disabled, to be enabled once udma utils are up*/
        #if !defined(SOC_TDA54)
        g_app_mem_obj.virtToPhyFxn = prm->virtToPhyFxn;
        #endif
        g_app_mem_obj.shared2TargetFxn = prm->shared2TargetFxn;
        g_app_mem_obj.target2SharedFxn = prm->target2SharedFxn;

        for(heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
        {
            app_mem_heap_prm_t *heap_prm;
            app_mem_heap_obj_t *heap_obj;

            heap_prm = &prm->heap_info[heap_id];
            heap_obj = &g_app_mem_obj.heap_obj[heap_id];

            heap_obj->is_valid = 0;
            heap_obj->alloc_offset = 0;
            heap_obj->max_offset = 0;

            #if defined(PDK)
            heap_obj->rtos_heap_handle = NULL;
            #endif

            if( (heap_prm->base == NULL) || (heap_prm->size == 0u))
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM04
<justification end> */
                if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
                {
                    /* no rtos heap, linear allocator based on offset */
                    heap_obj->alloc_offset = 0;
                }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM04
<justification end> */
                else
/* LDRA_JUSTIFY_END */
                {
                    /* create a rtos heap */
                    #if defined(PDK)
                    HeapP_Params rtos_heap_prm;

                    HeapP_Params_init(&rtos_heap_prm);
                    rtos_heap_prm.buf = APP_MEM_ALIGNPTR(
                        heap_prm->base, heap_prm->align);
                    rtos_heap_prm.size = APP_MEM_ALIGN32(
                        heap_prm->size, heap_prm->align);
                    rtos_heap_prm.align = heap_prm->align;

                    heap_obj->rtos_heap_handle = HeapP_create(&rtos_heap_prm);

                    heap_prm->base   = rtos_heap_prm.buf;
                    heap_prm->size   = rtos_heap_prm.size;
                    heap_prm->align  = rtos_heap_prm.align;
                    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                    void *heap_buf = APP_MEM_ALIGNPTR(heap_prm->base, heap_prm->align);
                    uint32_t heap_size = APP_MEM_ALIGN32(heap_prm->size, heap_prm->align);

                    heap_prm->base   = heap_buf;
                    heap_prm->size   = heap_size;
                    HeapP_construct(&heap_obj->rtos_heap_handle, heap_buf, heap_size);
                    #endif
                    appLogPrintf("MEM: Created heap (%s, id=%d, flags=0x%08x) @ %p of size %d bytes with alignment: %d !!!\n",
                        heap_prm->name,
                        heap_id,
                        heap_prm->flags,
                        heap_prm->base,
                        heap_prm->size,
                        heap_prm->align);
                }
            }
        }
    }
    else
    {
        status = -1;
        appLogPrintf("ERROR: prm is NULL !!!\n");
    }
    appLogPrintf("MEM: Init ... Done !!!\n");
    return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t appMemDeInit.* <function end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM05
<justification end> */
int32_t appMemDeInit(void)
{
    int32_t status = 0;
    uint32_t heap_id;

    appLogPrintf("MEM: Deinit ... !!!\n");

    for(heap_id = 0; heap_id < APP_MEM_HEAP_MAX; heap_id++)
    {
        app_mem_heap_obj_t *heap_obj;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];

        if(0U != heap_obj->is_valid)
        {

            #if defined(PDK)
            if(heap_obj->rtos_heap_handle != NULL)
            {
                status = (int32_t)HeapP_delete(&heap_obj->rtos_heap_handle);
            }
            heap_obj->rtos_heap_handle = NULL;
            #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
            HeapP_destruct(&heap_obj->rtos_heap_handle);
            #endif

            heap_obj->is_valid = 0;
            heap_obj->alloc_offset = 0;
        }
    }

    appLogPrintf("MEM: Deinit ... Done !!!\n");

    return status;
}

void    *appMemAlloc(uint32_t heap_id, uint32_t size, uint32_t align)
{
    void *ptr = NULL;

    if( (heap_id < APP_MEM_HEAP_MAX) && (size != 0u) )
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        if(0U != heap_obj->is_valid)
        {
            #if defined(R5F)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM001
            <justification end> */
            #endif
            if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uint32_t offset;
                uintptr_t key;

                key = HwiP_disable();

                offset = APP_MEM_ALIGN32(heap_obj->alloc_offset, align);

                if( (offset+size) <= heap_prm->size)
                {
                    ptr = (void*)((uintptr_t)heap_prm->base + offset);

                    heap_obj->alloc_offset = (offset+size);

                    if (heap_obj->alloc_offset > heap_obj->max_offset)
                    {
                        heap_obj->max_offset = heap_obj->alloc_offset;
                    }
                }
                else
                {
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: heap mem size insufficient for memory to be allocated\n");
                    #endif
                }

                HwiP_restore(key);
            }
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM001
            <justification end> */
            #endif
            else
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            #endif
            {
                #if defined(PDK)
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR004
                <justification end> */
                if(heap_obj->rtos_heap_handle!=NULL)
                /* LDRA_JUSTIFY_END */
                {
                    size  = APP_MEM_ALIGN32(size, heap_prm->align);

                    ptr = HeapP_alloc(heap_obj->rtos_heap_handle,
                               size);

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR005
                    <justification end> */
                    if(ptr!=NULL)
                    /* LDRA_JUSTIFY_END */
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM13
                        <justification end> */
                        if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_DO_CLEAR_ON_ALLOC))
                        {
                            (void)memset(ptr, 0, size);
                        }
                        /* LDRA_JUSTIFY_END */
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_C7X_UBR001
                        <justification end> */
                        if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_IS_SHARED))
                        {
                            appMemCacheWbInv(ptr, size);
                        }
/* LDRA_JUSTIFY_END */
                        #ifdef APP_MEM_DEBUG
                        appLogPrintf("MEM: Allocated %d bytes @ 0x%08x\n", size, (uint32_t)(uintptr_t)ptr);
                        #endif
                    }
                }
                #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                size  = APP_MEM_ALIGN32(size, heap_prm->align);

                ptr = HeapP_alloc(&heap_obj->rtos_heap_handle,
                            size);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR015
                <justification end> */
                if(ptr!=NULL)
                /* LDRA_JUSTIFY_END */
                {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM005
                <justification end> */
                    if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_DO_CLEAR_ON_ALLOC))
                    {
                        (void)memset(ptr, 0, size);
                    }
                    /* LDRA_JUSTIFY_END */
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM006
                    <justification end> */
                    if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_IS_SHARED))
                    {
                        appMemCacheWbInv(ptr, size);
                    }
                    /* LDRA_JUSTIFY_END */
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: Allocated %d bytes @ 0x%08x\n", size, (uint32_t)(uintptr_t)ptr);
                    #endif
                }
                #endif
            }
        }
    }
    return ptr;
}

int32_t appMemResetScratchHeap(uint32_t heap_id)
{
    int32_t status = -1;

    if(heap_id < APP_MEM_HEAP_MAX)
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_C7X_UBR002
        <justification end> */
        if(0U != heap_obj->is_valid)
        /* LDRA_JUSTIFY_END */
        {
            #if defined(R5F)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM002
            <justification end> */
            #endif
            if( 0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uintptr_t key;

                key = HwiP_disable();

                heap_obj->alloc_offset = 0;

                HwiP_restore(key);

                status = 0;
            }
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM002
            <justification end> */
            #endif
            else
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            #endif
            {
                #ifdef APP_MEM_DEBUG
                appLogPrintf("appMemResetScratchHeap: invalid heap ID\n");
                #endif
            }
        }
    }
    return status;
}

int32_t appMemFree(uint32_t heap_id, void *ptr, uint32_t size)
{
    int32_t status = -1;

    if(heap_id < APP_MEM_HEAP_MAX)
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        if(0U != heap_obj->is_valid)
        {
            #if defined(R5F)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM003
            <justification end> */
            #endif
            if(0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uintptr_t key;

                key = HwiP_disable();

                heap_obj->alloc_offset = 0;
                heap_obj->max_offset = 0;

                HwiP_restore(key);

                status = 0;
            }
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM003
            <justification end> */
            #endif
            else
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            #endif
            {
                #if defined(PDK)
                if( (heap_obj->rtos_heap_handle!=NULL) && /* TIOVX-1914- LDRA Uncovered Branch Id: APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR006 */
                (ptr != NULL) &&  (size != 0u) )
                #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                if( (ptr != NULL) && (size != 0U))
                #endif
                {
                    size  = APP_MEM_ALIGN32(size, heap_prm->align);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM005
                    <justification end> */
                    if(0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_IS_SHARED))
                    {
                        appMemCacheWbInv(ptr, size);
                    }
                    /* LDRA_JUSTIFY_END */
                    #ifdef APP_MEM_DEBUG
                    appLogPrintf("MEM: Freeing %d bytes @ 0x%08x\n", size, (uint32_t)(uintptr_t)ptr);
                    #endif
                    #if defined(PDK)
                    status = (int32_t)HeapP_free(heap_obj->rtos_heap_handle,
                        ptr,
                        size);
                    #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                    HeapP_free(&heap_obj->rtos_heap_handle,
                        ptr);
                    status = 0;
                    #endif
                }
            }
        }
    }
    return status;
}

int32_t appMemStats(uint32_t heap_id, app_mem_stats_t *stats)
{
    int32_t status = -1;

    if( (heap_id < APP_MEM_HEAP_MAX) && (stats != NULL) )
    {
        app_mem_heap_obj_t *heap_obj;
        app_mem_heap_prm_t *heap_prm;

        heap_obj = &g_app_mem_obj.heap_obj[heap_id];
        heap_prm = &heap_obj->heap_prm;

        stats->heap_id = heap_id;
        (void)strncpy(stats->heap_name, "INVALID", APP_MEM_HEAP_NAME_MAX);
        stats->heap_name[APP_MEM_HEAP_NAME_MAX-(1U)]=(char)0;
        stats->heap_size = 0;
        stats->free_size = 0;

        if(0U != heap_obj->is_valid)
        {
            stats->heap_id = heap_id;
            (void)strncpy(stats->heap_name, heap_prm->name, APP_MEM_HEAP_NAME_MAX -1U);
            stats->heap_name[APP_MEM_HEAP_NAME_MAX-1u]=(char)0;
            stats->heap_size = heap_prm->size;

            #if defined(R5F)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM004
            <justification end> */
            #endif
            if(0U != (heap_prm->flags & APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE))
            {
                uintptr_t key;

                key = HwiP_disable();

                stats->free_size = heap_prm->size - heap_obj->max_offset;

                HwiP_restore(key);

                status = 0;
            }
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_R5F_UM004
            <justification end> */
            #endif
            else
            #if defined(R5F)
            /* LDRA_JUSTIFY_END */
            #endif
            {
                #if defined(PDK)
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR007
                <justification end> */
                if(heap_obj->rtos_heap_handle!=NULL)
                {
                    HeapP_MemStats rtos_heap_stats;

                    status = (int32_t)HeapP_getHeapStats(heap_obj->rtos_heap_handle, &rtos_heap_stats);

                    stats->free_size = rtos_heap_stats.totalFreeSize;

                }
                /* LDRA_JUSTIFY_END */
                #elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
                HeapP_MemStats rtos_heap_stats;

                HeapP_getHeapStats(&heap_obj->rtos_heap_handle, &rtos_heap_stats);

                stats->free_size = (uint32_t)rtos_heap_stats.availableHeapSpaceInBytes;

                status = 0;
                #endif
            }
        }
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
    return status;
}

static void appMemFence(void)
{
    #if defined(C7X_FAMILY)
    _mfence();
    _mfence();
    #endif
}

void  appMemCacheInv(void *ptr, uint32_t size)
{
#ifdef ENABLE_CACHE_OPS
    #if defined(PDK)
    CacheP_Inv(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    /*JIRA: ADASVISION-7149 - TODO: to be enabled once Cache Ops are supported under mcu_sdk
    #elif defined(MCU_SDK)
    Arch_Cache_Inv(
        (uintptr_t)ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    */
    #elif defined(MCU_PLUS_SDK)
    CacheP_inv(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES),
        CacheP_TYPE_L1D);
    #endif
#endif
    appMemFence();
}

void  appMemCacheWb(void *ptr, uint32_t size)
{
#ifdef ENABLE_CACHE_OPS
    #if defined(PDK)
    CacheP_wb(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    /*JIRA: ADASVISION-7149 - TODO: to be enabled once Cache Ops are supported under mcu_sdk
    #elif defined(MCU_SDK)
    Arch_Cache_Wb(
        (uintptr_t)ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    */
    #elif defined(MCU_PLUS_SDK)
    CacheP_wb(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES),
        CacheP_TYPE_L1D);
    #endif
#endif
    appMemFence();

}

void  appMemCacheWbInv(void *ptr, uint32_t size)
{
#ifdef ENABLE_CACHE_OPS
    #if defined(PDK)
    CacheP_wbInv(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    #elif defined(MCU_SDK)
    /*JIRA: ADASVISION-7149 - TODO: to be enabled once Cache Ops are supported under mcu_sdk
     Arch_Cache_WbInv(
         (uintptr_t)ptr,
         APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
    */
    #elif defined(MCU_PLUS_SDK)
    CacheP_wbInv(
        ptr,
        APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES),
        CacheP_TYPE_L1D);
    #endif
#endif
    appMemFence();
}

uint64_t appMemGetVirt2PhyBufPtr(uint64_t virtPtr, uint32_t heap_id)
{
#ifndef C66
    uint64_t physPtr;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR008
<justification end> */
/* JIRA: ADASVISION-7146 - Temporarily disabled, to be enabled once udma utils are up*/
#if defined(SOC_FAMILY_AM) || defined(SOC_FAMILY_J7)
    if (NULL != g_app_mem_obj.virtToPhyFxn)
/* LDRA_JUSTIFY_END */
    {
        physPtr = (uint64_t)g_app_mem_obj.virtToPhyFxn((void*)virtPtr, 0, NULL);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM06
<justification end> */
    else
    {
        physPtr = virtPtr;
    }
#else
    physPtr = virtPtr;
#endif
/* LDRA_JUSTIFY_END */

    return physPtr;
#else
    return virtPtr;
#endif
}

uint64_t appMemShared2TargetPtr(uint64_t shared_ptr)
{
    uint64_t target_ptr;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR009
<justification end> */
    if (NULL != g_app_mem_obj.shared2TargetFxn)
/* LDRA_JUSTIFY_END */
    {
        target_ptr = (uint64_t)g_app_mem_obj.shared2TargetFxn(shared_ptr);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM10
<justification end> */
    else
    {
        target_ptr = shared_ptr;
    }
/* LDRA_JUSTIFY_END */

    return target_ptr;
}

uint64_t appMemShared2PhysPtr(uint64_t shared_ptr, uint32_t heap_id)
{
    uint64_t physPtr;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR010
<justification end> */
    if (NULL != g_app_mem_obj.target2SharedFxn)
/* LDRA_JUSTIFY_END */
    {
        physPtr = (uint64_t)g_app_mem_obj.target2SharedFxn(shared_ptr);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM11
<justification end> */
    else
    {
        physPtr = shared_ptr;
    }
/* LDRA_JUSTIFY_END */

    return physPtr;
}

uint32_t appMemGetDmaBufFd(void *virPtr, volatile uint32_t *dmaBufFdOffset)
{
   /* For rtos implementation, dmaBufFd is not valid and just return 0
     */
    *dmaBufFdOffset = 0;
    return (uint32_t)(-1);
}

int32_t appMemTranslateDmaBufFd(uint32_t dmaBufFd, uint32_t size, uint64_t *virtPtr, uint64_t *phyPtr)
{
    /* For rtos implementation, dmaBufFd is not valid and just return -1. */
    return -1;
}

void appMemCloseDmaBufFd(uint32_t dmaBufFd)
{
   /* For rtos implementation, dmaBufFd is not valid and just return 0
     */
    return;
}

void appMemPrintMemAllocInfo(void)
{
    return;
}

uint32_t appMemGetNumAllocs(void)
{
    return (uint32_t)(-1);
}

#if defined(R5F) && (defined(SOC_J784S4) || defined(SOC_J721S2) || defined(SOC_J742S2))

int32_t appMemAddrTranslate(app_mem_rat_prm_t *prm)
{
    int32_t status = 0;
    bool ret = false, availableRatRegion = false;
    uint32_t maxRegions, i;
    bool        bIsEnabled;
    uintptr_t key;

    CSL_RatTranslationCfgInfo translationCfg;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR011
<justification end> */
    if (NULL != g_app_mem_obj.pRatRegs)
/* LDRA_JUSTIFY_END */
    {
        CSL_ratRegs *pRatRegs = g_app_mem_obj.pRatRegs;
        maxRegions = CSL_ratGetMaxRegions(pRatRegs);

        key = HwiP_disable();

        for (i = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR012
<justification end> */
        i < maxRegions;
/* LDRA_JUSTIFY_END */
        i++)
        {
            bIsEnabled = CSL_ratIsRegionTranslationEnabled(pRatRegs,i);
            if (false == bIsEnabled)
            {
                #ifdef APP_MEM_DEBUG
                appLogPrintf("appMemAddrTranslate(): Using RAT region %d for translation of address : %llx\n", i, prm->translatedAddress);
                #endif
                availableRatRegion = (bool)true;
                break;
            }
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR013
<justification end> */
        if ((bool)true == availableRatRegion)
/* LDRA_JUSTIFY_END */
        {
            translationCfg.sizeInBytes       = prm->size;
            translationCfg.baseAddress       = prm->baseAddress;
            translationCfg.translatedAddress = prm->translatedAddress;

            ret = CSL_ratConfigRegionTranslation(pRatRegs, i, &translationCfg);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM07
<justification end> */
            if(ret == (bool)false)
            {
                appLogPrintf("appMemAddrTranslate(): Error in CSL_ratConfigRegionTranslation()\n");
                status = -1;
            }
/* LDRA_JUSTIFY_END */

            #ifdef APP_MEM_DEBUG
            for (i = 0; i < maxRegions; i++)
            {
                bIsEnabled = CSL_ratIsRegionTranslationEnabled(pRatRegs,i);
                appLogPrintf("appMemAddrTranslate(): RAT region %d translation enabled status after OCMC map (0: disabled, 1: enabled): %d\n", i, bIsEnabled);
            }
            #endif
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM08
<justification end> */
        else
        {
            appLogPrintf("appMemAddrTranslate(): No available RAT regions for mapping\n");
            status = -1;
        }
/* LDRA_JUSTIFY_END */

        HwiP_restore(key);
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM09
<justification end> */  
    else
    {
        appLogPrintf("appMemAddrTranslate(): pRatRegs has not been set.  Use appMemSetRatRegs function to set before calling\n");
    }
/* LDRA_JUSTIFY_END */

    return status;
}

int32_t appMemSetRatRegs(CSL_ratRegs *pRatRegs)
{
    int32_t status = -1;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_MEM_FREE_RTOS_UBR014
<justification end> */
    if (NULL == g_app_mem_obj.pRatRegs)
/* LDRA_JUSTIFY_END */
    {
        g_app_mem_obj.pRatRegs = pRatRegs;
        status = 0;
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_MEM_FREE_RTOS_UM12
<justification end> */
    else
    {
        appLogPrintf("appMemSetRatRegs(): pRatRegs has already been set\n");
    }
/* LDRA_JUSTIFY_END */

    return status;
}

#endif

void *appMemMap(void *phys_ptr, uint32_t size)
{
    return phys_ptr; /* phys == virtual in rtos */
}

int32_t appMemUnMap(void *virt_ptr, uint32_t size)
{
    return 0; /* nothing to do in rtos */
}
