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

#ifndef APP_MEM_H_
#define APP_MEM_H_

#include <stdint.h>

/*!
 * \defgroup group_vision_apps_utils_mem Memory allocation APIs
 *
 * \brief This section contains APIs for memory allocation
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/*! \brief Max characters to use for heap name */
#define APP_MEM_HEAP_NAME_MAX   (16U)

/*! \brief Heap located in DDR */
#define APP_MEM_HEAP_DDR        (0U)

/*! \brief Heap located in L3 memory (MSMC) */
#define APP_MEM_HEAP_L3_MSMC    (1U)

/*! \brief Heap located in L2 local memory of a CPU */
#define APP_MEM_HEAP_L2_LOCAL   (2U)

/*! \brief Max heaps in system */
#define APP_MEM_HEAP_MAX        (3U)

/*!
 * \brief Heap flag to indicate heap is of type "linear allocater"
 *
 * Here alloc increments a offset by alloc size and free resets
 * the offset. This is used typically for L2 memory to allocate it
 * as scratch among multiple algorithms on DSP.
 *
 * When this flag is not set heap type is normal dynamic memory allocator heap
 */
#define APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE     (0x00000001U)

/*!
 * \brief Heap flag to indicate if memory that is allcoate will be shared
 *        with another CPU
 */
#define APP_MEM_HEAP_FLAGS_IS_SHARED                (0x00000004U)

/*! \brief Heap flag to indicate if memory should be set to 0 after alloc */
#define APP_MEM_HEAP_FLAGS_DO_CLEAR_ON_ALLOC        (0x00000008U)

/*!
 * \brief Heap initialization parameters
 */
typedef struct
{
    /*! Heap memory base address */
    void *base;

    /*! Heap name */
    char name[APP_MEM_HEAP_NAME_MAX];

    /*! Heap size in bytes */
    uint32_t size;

    /*! Flags, see APP_MEM_HEAP_FLAGS_* */
    uint32_t flags;
} app_mem_heap_prm_t;

/*!
 * \brief Memory module initialization parameters
 */
typedef struct
{
    /*! Heap init parameters */
    app_mem_heap_prm_t heap_info[APP_MEM_HEAP_MAX];
} app_mem_init_prm_t;

/*!
 * \brief Heap statistics and information
 */
typedef struct
{
    /*! Heap ID, see APP_MEM_HEAP_* */
    uint32_t heap_id;

    /*! Heap name */
    char heap_name[APP_MEM_HEAP_NAME_MAX];

    /*! Heap size in bytes */
    uint32_t heap_size;

    /*! Free space in bytes */
    uint32_t free_size;
} app_mem_stats_t;

/*!
 * \brief Align ptr value to 'align' bytes
 */
static inline void *APP_MEM_ALIGNPTR(void *val,
                                     uint32_t align)
{
    return (void *)((((uintptr_t)val + align - 1) / align) * align);
}

/*!
 * \brief Align 64b value to 'align' bytes
 */
static inline uint64_t APP_MEM_ALIGN64(uint64_t val,
                                       uint32_t align)
{
    return (uint64_t)((uint64_t)(val + align - 1) / align) * align;
}

/*!
 * \brief Align 32b value to 'align' bytes
 */
static inline uint32_t APP_MEM_ALIGN32(uint32_t val,
                                       uint32_t align)
{
    return (uint32_t)((uint32_t)(val + align - 1) / align) * align;
}

/*!
 * \brief Set defaults to app_mem_init_prm_t
 *
 * Recommend to be called before calling appMemInit()
 * Override default with user specific parameters
 *
 * By default all heaps are disabled.
 *
 * \param prm [out] Default initialized parameters
 */
void    appMemInitPrmSetDefault(app_mem_init_prm_t *prm);

/*!
 * \brief Init heaps for memory allocation
 *
 * \param prm [in] Initialization parameters
 *
 * \return 0 on success else failure
 */
int32_t appMemInit(app_mem_init_prm_t *prm);

/*!
 * \brief De-Init heaps for memory allocation
 *
 * \return 0 on success else failure
 */
int32_t appMemDeInit(void);

/*!
 * \brief Alloc memory from specific heap
 *
 * \param heap_id [in] See APP_MEM_HEAP_*
 * \param size    [in] Size in bytes to allocate
 * \param align   [in] Minimum alignment requested
 *
 * \return Pointer to memory or NULL in case of failure
 */
void *appMemAlloc(uint32_t heap_id,
                  uint32_t size,
                  uint32_t align);

/*!
 * \brief Free memory that was previously allocated
 *
 * \param heap_id [in] See APP_MEM_HEAP_*
 * \param ptr     [in] Pointer to allocated memory
 * \param size    [in] Size of allocated memory
 *
 * \return 0 on success else failure
 */
int32_t appMemFree(uint32_t heap_id,
                   void *ptr,
                   uint32_t size);

/*!
 * \brief Return heap statistics and information
 *
 * \param heap_id [in] See APP_MEM_HEAP_*
 * \param stats   [in] Heap statistics and information
 *
 * \return 0 on success else failure
 */
int32_t appMemStats(uint32_t heap_id,
                    app_mem_stats_t *stats);

/* @} */

#endif
