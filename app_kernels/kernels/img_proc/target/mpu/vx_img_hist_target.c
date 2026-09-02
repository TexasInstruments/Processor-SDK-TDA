/*
*
* Copyright (c) 2020-2026 Texas Instruments Incorporated
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
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"
#include "tivx_img_hist_host.h"

#if defined(SOC_AM62A)
#define NUM_MPU_TARGETS (1)
#else
#define NUM_MPU_TARGETS (4)
#endif

static tivx_target_kernel vx_img_hist_target_kernel[NUM_MPU_TARGETS];

static vx_status VX_CALLBACK tivxKernelImgHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelImgHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelImgHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

vx_status VXLIB_histogram_i8u_o32u_cn(uint8_t src[],
                                 VXLIB_bufParams2D_t *src_addr,
                                 uint32_t dist[],
                                 uint8_t offset,
                                 uint16_t range,
                                 uint16_t numBins);

static vx_status VX_CALLBACK tivxKernelImgHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    uint8_t *src_addr[2];
    VXLIB_bufParams2D_t vxlib_src[2];

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_IMG_HIST_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr[2];
        void *dst_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMG_HIST_INPUT_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_IMG_HIST_DISTRIBUTION_IDX];

        src_target_ptr[0] = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr);

        tivxMemBufferMap(src_target_ptr[0], src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        tivxMemBufferMap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

        tivxSetPointerLocation(src, src_target_ptr, src_addr);
        tivxInitBufParams(src, vxlib_src);

        memset(dst_target_ptr, 0, dst->mem_size);

        status = (vx_status)VXLIB_histogram_i8u_o32u_cn(src_addr[0], &vxlib_src[0],
                dst_target_ptr, (uint8_t)dst->offset, (uint16_t)dst->range, (uint16_t)dst->num_bins);

        tivxMemBufferUnmap(src_target_ptr[0], src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        tivxMemBufferUnmap(dst_target_ptr, dst->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelImgHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_IMG_HIST_MAX_PARAMS);

    return (status);
}

static vx_status VX_CALLBACK tivxKernelImgHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_IMG_HIST_MAX_PARAMS);

    return (status);
}

void tivxAddTargetKernelImgHist(void)
{
    char target_name[NUM_MPU_TARGETS][TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    int32_t i;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_MPU_0 )
    {
        strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        #if !defined(SOC_AM62A)
        strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
        #endif
    }

    for(i = 0; i < NUM_MPU_TARGETS; i++)
    {
        vx_img_hist_target_kernel[i] = tivxAddTargetKernelByName(
                                        TIVX_KERNEL_IMG_HIST_NAME,
                                        target_name[i],
                                        tivxKernelImgHistProcess,
                                        tivxKernelImgHistCreate,
                                        tivxKernelImgHistDelete,
                                        NULL,
                                        NULL);
    }
}


void tivxRemoveTargetKernelImgHist(void)
{
    int32_t i;

    for(i = 0; i < NUM_MPU_TARGETS; i++)
    {
        tivxRemoveTargetKernel(vx_img_hist_target_kernel[i]);
    }
}

vx_status VXLIB_histogram_i8u_o32u_cn(uint8_t src[],
                                 VXLIB_bufParams2D_t *src_addr,
                                 uint32_t dist[],
                                 uint8_t offset,
                                 uint16_t range,
                                 uint16_t numBins)
{
    vx_status status = VX_SUCCESS;

    uint32_t  x, y;

    for( x = 0; x < numBins; x++ )
    {
        dist[x] = 0;
    }

    for( y=0; y < src_addr->dim_y; y++ )
    {
        for( x=0; x < src_addr->dim_x; x++ )
        {
            uint8_t pixel = src[y * src_addr->stride_y + x];
            if((offset <= pixel) && (pixel < (offset + range)))
            {
                uint8_t index = ((pixel * pixel) - offset) * numBins / range;
                dist[index]++;
            }
        }
    }


    return (status);
}
