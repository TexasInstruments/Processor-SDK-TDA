/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <TI/hwa_kernels.h>
#include <TI/hwa_vpac_ldc.h>
#include <TI/hwa_vpac_msc.h>
#include <TI/hwa_vpac_viss.h>
#include <TI/hwa_vpac_nf.h>
#include <TI/hwa_dmpac_dof.h>
#include <TI/hwa_dmpac_sde.h>
#include "tivx_hwa_kernels.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_hwa_host_priv.h"
#include <TI/hwa_vpac_fc.h>

void tivxSetSelfCpuId(vx_enum cpu_id);

static vx_status VX_CALLBACK publishKernels(vx_context context);
static vx_status VX_CALLBACK unPublishKernels(vx_context context);

static uint32_t gIsHwaKernelsLoad = 0u;

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
#ifdef BUILD_VPAC_NF
    {&tivxAddKernelVpacNfGeneric, &tivxRemoveKernelVpacNfGeneric},
    {&tivxAddKernelVpacNfBilateral, &tivxRemoveKernelVpacNfBilateral},
#endif
#ifdef BUILD_DMPAC_SDE
    {&tivxAddKernelDmpacSde, &tivxRemoveKernelDmpacSde},
#endif
#ifdef BUILD_VPAC_LDC
    {&tivxAddKernelVpacLdc, &tivxRemoveKernelVpacLdc},
    {&tivxAddKernelVpacLdc2, &tivxRemoveKernelVpacLdc2},
#endif
#ifdef BUILD_VPAC_MSC
    {&tivxAddKernelVpacMscMultiScale, &tivxRemoveKernelVpacMscMultiScale},
    {&tivxAddKernelVpacMscPyramid, &tivxRemoveKernelVpacMscPyramid},
    {&tivxAddKernelVpacMscMultiOut, &tivxRemoveKernelVpacMscMultiOut},
#endif
#ifdef BUILD_DMPAC_DOF
    {&tivxAddKernelDmpacDof, &tivxRemoveKernelDmpacDof},
    {&tivxAddKernelDofVisualize, &tivxRemoveKernelDofVisualize},
#endif
#ifdef BUILD_VPAC_VISS
    {&tivxAddKernelVpacViss, &tivxRemoveKernelVpacViss},
#endif
#ifdef BUILD_VPAC_FC
    {&tivxAddKernelVpacFc, &tivxRemoveKernelVpacFc},
#endif
};

static vx_status VX_CALLBACK publishKernels(vx_context context)
{
    return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

static vx_status VX_CALLBACK unPublishKernels(vx_context context)
{
    return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

void tivxRegisterHwaKernels(void)
{
    vx_status status;

    status = tivxRegisterModule(TIVX_MODULE_NAME_HWA, publishKernels, unPublishKernels);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to register HWA kernels module\n");
    }
}

void tivxUnRegisterHwaKernels(void)
{
    vx_status status;

    status = tivxUnRegisterModule(TIVX_MODULE_NAME_HWA);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to unregister HWA kernels module\n");
    }
}

void tivxHwaLoadKernels(vx_context context)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((0U == gIsHwaKernelsLoad) && (NULL != context))
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;

        tivxRegisterHwaKernels();
        status = vxLoadKernels(context, TIVX_MODULE_NAME_HWA);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to load HWA kernels\n");
        }

        #ifdef x86_64
        /* for PC register target kernels here */
        /* for actual Si, target kernel registration will happen
         * during platform init on the CPU that supports the target
         */
        /* This only works on PC emulation mode */
        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_VPAC1);

        #ifdef BUILD_VPAC_NF
        tivxRegisterHwaTargetVpacNfKernels();
        #endif

        #ifdef BUILD_VPAC_LDC
        tivxRegisterHwaTargetVpacLdcKernels();
        #endif

        #ifdef BUILD_VPAC_MSC
        tivxRegisterHwaTargetVpacMscKernels();
        #endif

        #ifdef BUILD_VPAC_VISS
        tivxRegisterHwaTargetVpacVissKernels();
        #endif
        
        #ifdef BUILD_VPAC_FC
        #ifndef x86_64
        tivxRegisterHwaTargetVpacFcKernels();
        #endif
        #endif

        #if defined(BUILD_DMPAC_SDE) || defined(BUILD_DMPAC_DOF)
        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_DMPAC);
        #endif

        #ifdef BUILD_DMPAC_SDE
        tivxRegisterHwaTargetDmpacSdeKernels();
        #endif

        #ifdef BUILD_DMPAC_DOF
        tivxRegisterHwaTargetDmpacDofKernels();
        #endif

        #if (VPAC_COUNT > 1)
        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_VPAC2);
        #ifdef BUILD_VPAC_NF
        tivxRegisterHwaTargetVpacNfKernels();
        #endif

        #ifdef BUILD_VPAC_LDC
        tivxRegisterHwaTargetVpacLdcKernels();
        #endif

        #ifdef BUILD_VPAC_MSC
        tivxRegisterHwaTargetVpacMscKernels();
        #endif

        #ifdef BUILD_VPAC_VISS
        tivxRegisterHwaTargetVpacVissKernels();
        #endif

        #ifdef BUILD_VPAC_FC
        #ifndef x86_64
        tivxRegisterHwaTargetVpacFcKernels();
        #endif
        #endif
        #endif

        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_DSP1);

        tivxRegisterHwaTargetArmKernels();
        #endif
    }
    gIsHwaKernelsLoad++;
}

void tivxHwaUnLoadKernels(vx_context context)
{
    if (gIsHwaKernelsLoad > 0U)
    {
        gIsHwaKernelsLoad--;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((0u == gIsHwaKernelsLoad) && (NULL != context))
        /* LDRA_JUSTIFY_END */
        {
            vx_status status;

            status = vxUnloadKernels(context, TIVX_MODULE_NAME_HWA);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to unload HWA kernels\n");
            }
            tivxUnRegisterHwaKernels();

            #ifdef x86_64
            /* This line only works on PC emulation mode */
            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_VPAC1);

            #ifdef BUILD_VPAC_NF
            tivxUnRegisterHwaTargetVpacNfKernels();
            #endif

            #ifdef BUILD_VPAC_LDC
            tivxUnRegisterHwaTargetVpacLdcKernels();
            #endif

            #ifdef BUILD_VPAC_MSC
            tivxUnRegisterHwaTargetVpacMscKernels();
            #endif

            #ifdef BUILD_VPAC_VISS
            tivxUnRegisterHwaTargetVpacVissKernels();
            #endif

            #ifdef BUILD_VPAC_FC
            #ifndef x86_64
            tivxUnRegisterHwaTargetVpacFcKernels();
            #endif
            #endif

            #if defined(BUILD_DMPAC_SDE) || defined(BUILD_DMPAC_DOF)
            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_DMPAC);
            #endif

            #ifdef BUILD_DMPAC_SDE
            tivxUnRegisterHwaTargetDmpacSdeKernels();
            #endif

            #ifdef BUILD_DMPAC_DOF
            tivxUnRegisterHwaTargetDmpacDofKernels();
            #endif

            #if (VPAC_COUNT > 1)
            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_VPAC2);
            #ifdef BUILD_VPAC_NF
            tivxUnRegisterHwaTargetVpacNfKernels();
            #endif

            #ifdef BUILD_VPAC_LDC
            tivxUnRegisterHwaTargetVpacLdcKernels();
            #endif

            #ifdef BUILD_VPAC_MSC
            tivxUnRegisterHwaTargetVpacMscKernels();
            #endif

            #ifdef BUILD_VPAC_VISS
            tivxUnRegisterHwaTargetVpacVissKernels();
            #endif

            #ifdef BUILD_VPAC_FC
            #ifndef x86_64
            tivxUnRegisterHwaTargetVpacFcKernels();
            #endif
            #endif

            #endif

            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_DSP1);

            tivxUnRegisterHwaTargetArmKernels();
            #endif
        }
    }
}

