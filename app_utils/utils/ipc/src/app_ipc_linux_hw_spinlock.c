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

#include <stdint.h>
#include <utils/ipc/src/app_ipc_linux_priv.h>

#define APP_IPC_HW_SPIN_LOCK_MAX        (256u)
#define APP_IPC_HW_SPIN_LOCK_OFFSET(x)  ((uint64_t)0x800UL + ((uint64_t)4UL*(uint32_t)(x)))
#if defined(MCU_PLUS_SDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x2A000000u)
#elif defined(MCU_SDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x30020000u)
#elif defined(PDK)
#define APP_IPC_HW_SPIN_LOCK_MMR_BASE   ((uint32_t)0x30E00000u)
#endif
#define APP_IPC_HW_SPIN_LOCK_MMR_SIZE   (APP_IPC_HW_SPIN_LOCK_OFFSET(APP_IPC_HW_SPIN_LOCK_MAX))

int32_t appIpcHwLockInit(void)
{
    int32_t status = 0;
    app_ipc_obj_t *obj = &g_app_ipc_obj;

    obj->hw_spin_lock_addr = appMemMap((void*)APP_IPC_HW_SPIN_LOCK_MMR_BASE, APP_IPC_HW_SPIN_LOCK_MMR_SIZE);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_LINUX_HW_SPINLOCK_UM01
<justification end> */
    if(obj->hw_spin_lock_addr==NULL)
    {
        appLogPrintf("IPC: ERROR: Unable to mmap hw spinlock MMRs (%p of %ld bytes) !!!\n", (void*)APP_IPC_HW_SPIN_LOCK_MMR_BASE, APP_IPC_HW_SPIN_LOCK_MMR_SIZE );
        status = -1;
    }
/* LDRA_JUSTIFY_END */
    return status;
}

int32_t appIpcHwLockAcquire(uint32_t hw_lock_id, uint32_t timeout)
{
    int32_t status = -1;

    if( (hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX) && (g_app_ipc_obj.hw_spin_lock_addr != NULL) )
    {
        volatile uint32_t *reg_addr;

        reg_addr =
                (volatile uint32_t*)(uintptr_t)(
                    (uintptr_t)g_app_ipc_obj.hw_spin_lock_addr +
                    APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id)
                        );

        /* spin until lock is free */
        while( *reg_addr == 1u )
        {
            /* keep spining */
        }
        status = 0;
    }

    return status;
}

int32_t appIpcHwLockRelease(uint32_t hw_lock_id)
{
    int32_t status = -1;

    if( (hw_lock_id < APP_IPC_HW_SPIN_LOCK_MAX) && (g_app_ipc_obj.hw_spin_lock_addr != NULL) )
    {
        volatile uint32_t *reg_addr;

        reg_addr =
                (volatile uint32_t*)(uintptr_t)(
                    (uintptr_t)g_app_ipc_obj.hw_spin_lock_addr +
                    APP_IPC_HW_SPIN_LOCK_OFFSET(hw_lock_id)
                        );

        *reg_addr = 0; /* free the lock */
        status = 0;
    }

    return status;
}
