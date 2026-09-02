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



#include "TI/tivx.h"
#include "TI/tivx_stereo.h"
#include "VX/vx.h"
#include "tivx_stereo_kernels_priv.h"
#include "tivx_kernel_sde_histogram_visualize.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

#include "stdio.h"



static void visualizeSdeHistogram(
        uint32_t * histogram,
        uint8_t * histogram_img,
        int32_t   numBins,
        int32_t   image_size[2]);


static tivx_target_kernel vx_sde_histogram_visualize_target_kernel = NULL;

static vx_status VX_CALLBACK tivxSdeHistogramVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeHistogramVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeHistogramVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxSdeHistogramVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_image_t *histogram_image_desc;

    if ( num_params != TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_HISTOGRAM_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_HISTOGRAM_IMAGE_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        void *histogram_target_ptr;
        void *histogram_image_target_ptr;

        histogram_desc = (tivx_obj_desc_distribution_t *) obj_desc[TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_HISTOGRAM_IDX];
        histogram_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_HISTOGRAM_IMAGE_IDX];

        histogram_target_ptr = tivxMemShared2TargetPtr(&histogram_desc->mem_ptr);
        histogram_image_target_ptr = tivxMemShared2TargetPtr(&histogram_image_desc->mem_ptr[0]);

        tivxMemBufferMap(histogram_target_ptr,
            histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(histogram_image_target_ptr,
            histogram_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        /* call kernel processing function */
        {
            int histogram_img_size[2];

            histogram_img_size[0] = histogram_image_desc->height;
            histogram_img_size[1] = histogram_image_desc->imagepatch_addr[0].stride_y/sizeof(uint8_t);

            visualizeSdeHistogram(
                (uint32_t *)histogram_target_ptr,
                (uint8_t *) histogram_image_target_ptr,
                histogram_desc->num_bins,
                histogram_img_size
            );
        }

        /* kernel processing function complete */

        tivxMemBufferUnmap(histogram_target_ptr,
            histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(histogram_image_target_ptr,
            histogram_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }

    return status;
}

static vx_status VX_CALLBACK tivxSdeHistogramVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxSdeHistogramVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelSdeHistogramVisualize()
{
    vx_status status = VX_FAILURE;
    char target_name[4][TIVX_TARGET_MAX_NAME];
    uint32_t num_targets = 1;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
#if defined (SOC_TDA54)
    if ( self_cpu == TIVX_CPU_ID_MCU1)
    {
        strncpy(target_name[0], TIVX_TARGET_MCU1 , TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
#else
    if ( self_cpu == TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name[0], TIVX_TARGET_MCU2_0 , TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
#endif
    else
    if ( self_cpu == TIVX_CPU_ID_MPU_0 )
    {
        strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
        num_targets = 4;
        status = VX_SUCCESS;
    }    
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name[0]);
    }

    if (status == VX_SUCCESS)
    {
        uint32_t    i;

        for (i = 0; i < num_targets; i++)
        {        
            vx_sde_histogram_visualize_target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_NAME,
                                target_name[i],
                                tivxSdeHistogramVisualizeProcess,
                                tivxSdeHistogramVisualizeCreate,
                                tivxSdeHistogramVisualizeDelete,
                                NULL,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelSdeHistogramVisualize()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_sde_histogram_visualize_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_sde_histogram_visualize_target_kernel = NULL;
    }
}

void visualizeSdeHistogram(
    uint32_t *histogram,
    uint8_t  *histogram_img,
    int32_t   numBins,
    int32_t   image_size[2])
{
    int32_t i, j, k;
    int32_t max, binWidth;
    int32_t width  = image_size[1];
    int32_t height = image_size[0];

    uint8_t *addr0 = NULL, *addr1 = NULL;

    // first bin and the last bin freuqencies are so huge, so remove it for plotting
    int32_t start_offset = 1, end_offset = 1;

    // normalize bins
    max = 0;
    for(i = start_offset; i < numBins-end_offset; i++)
    {
        if(histogram[i] > max)
            max = histogram[i];
    }

    // scale to image height
    for(i = start_offset; i < numBins-end_offset; i++)
    {
        histogram[i] = (uint32_t)(((float)histogram[i]/max) * height + 0.5);
        if(histogram[i] > height)
            histogram[i] = height;
    }

    // draw histogram image
    binWidth = width/numBins;

    // initialize to white, since histogram is rendered by black
    for (i = 0; i < width * height; i++)
        histogram_img[i] = 0xFF;

    for(i = start_offset; i < numBins-end_offset; i++)
    {
        /*
        addr0 = histogram_img + i * binWidth;

        for(j=0; j<height-histogram[i]; j++)
        {
            addr1 = addr0 + j*width;
            for(k=0; k<binWidth; k++)
            {
                addr1[k] = 0xFF;
            }
        }
        */

        addr0 = histogram_img + i*binWidth + (height - histogram[i]) * width;

        for(j=0; j<histogram[i]; j++)
        {
            addr1 = addr0 + j*width;
            for(k=0; k<binWidth; k++)
            {
                addr1[k] = 0x0;
            }
        }
    }
}

