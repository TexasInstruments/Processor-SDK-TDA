/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include <app_mem_map.h>
#include CORE_CFG_FILE
#include <platform_util_mem.h>

#include <utils/mem/include/app_mem.h>

#ifdef DDR_HEAP_MEM_SIZE
uint8_t g_ddr_local_mem[DDR_HEAP_MEM_SIZE]
__attribute__ ((section(".bss:ddr_local_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef L1_MEM_SIZE
uint8_t g_l1_mem[L1_MEM_SIZE]
__attribute__ ((section(".bss:l1mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef L2_MEM_SIZE
uint8_t g_l2_mem[L2_MEM_SIZE]
__attribute__ ((section(".bss:l2mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef L3_MEM_SIZE
uint8_t g_l3_mem[L3_MEM_SIZE]
__attribute__ ((section(".bss:l3mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_SCRATCH_SIZE
uint8_t g_ddr_scratch_mem[DDR_SCRATCH_SIZE]
__attribute__ ((section(".bss:ddr_scratch_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_HEAP_NON_CACHE_MEM_SIZE
uint8_t g_ddr_non_cache_mem[DDR_HEAP_NON_CACHE_MEM_SIZE]
__attribute__ ((section(".bss:ddr_non_cache_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_SCRATCH_NON_CACHE_SIZE
uint8_t g_ddr_scratch_non_cache_mem[DDR_SCRATCH_NON_CACHE_SIZE]
__attribute__ ((section(".bss:ddr_scratch_non_cache_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

#ifdef DDR_VISS_HEAP_MEM_SIZE
uint8_t g_ddr_cache_wt_mem[DDR_VISS_HEAP_MEM_SIZE]
__attribute__ ((section(".bss:ddr_cache_wt_mem")))
__attribute__ ((aligned(4096)))
        ;
#endif

soc_mem_config_t mem_regions[] = {
#ifdef DDR_HEAP_MEM_SIZE
        {
            .name = "DDR_LOCAL_MEM",
            .heap_id = APP_MEM_HEAP_DDR,
            .size = DDR_HEAP_MEM_SIZE,
            .base = g_ddr_local_mem,
            .flag = APP_MEM_HEAP_FLAGS_IS_SHARED,
        },
#endif
#ifdef L1_MEM_SIZE
        {
            .name = "L1_MEM",
            .heap_id = APP_MEM_HEAP_L1,
            .size = L1_MEM_SIZE,
            .base = g_l1_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,  /* Normal heap for mcu0 */
        },
#endif
#ifdef L2_MEM_SIZE
        {
            .name = "L2_MEM",
            .heap_id = APP_MEM_HEAP_L2,
            .size = L2_MEM_SIZE,
            .base = g_l2_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,  /* Normal heap for mcu0 */
        },
#endif
#ifdef L3_MEM_SIZE
        {
            .name = "L3_MEM",
            .heap_id = APP_MEM_HEAP_L3,
            .size = L3_MEM_SIZE,
            .base = g_l3_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,  /* Normal heap for mcu0 */
        },
#endif
#ifdef DDR_SCRATCH_SIZE
        {
            .name = "DDR_SCRATCH_MEM",
            .heap_id = APP_MEM_HEAP_DDR_SCRATCH,
            .size = DDR_SCRATCH_SIZE,
            .base = g_ddr_scratch_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,
        },
#endif
#ifdef DDR_HEAP_NON_CACHE_MEM_SIZE
        {
            .name = "DDR_NON_CACHE_MEM",
            .heap_id = APP_MEM_HEAP_DDR_NON_CACHE,
            .size = DDR_HEAP_NON_CACHE_MEM_SIZE,
            .base = g_ddr_non_cache_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,
        },
#endif
#ifdef DDR_SCRATCH_NON_CACHE_SIZE
        {
            .name = "DDR_SCRATCH_NON_CACHE_MEM",
            .heap_id = APP_MEM_HEAP_DDR_NON_CACHE_SCRATCH,
            .size = DDR_SCRATCH_NON_CACHE_SIZE,
            .base = g_ddr_scratch_non_cache_mem,
            .flag = APP_MEM_HEAP_FLAGS_TYPE_LINEAR_ALLOCATE,
        },
#endif
#ifdef DDR_VISS_HEAP_MEM_SIZE
        {
            .name = "DDR_CACHE_WT_MEM",
            .heap_id = APP_MEM_HEAP_DDR_WT_CACHE,
            .size = DDR_VISS_HEAP_MEM_SIZE,
            .base = g_ddr_cache_wt_mem,
            .flag = 0,
        }
#endif
};

uint8_t num_mem_regions = sizeof(mem_regions)/sizeof(mem_regions[0]);
