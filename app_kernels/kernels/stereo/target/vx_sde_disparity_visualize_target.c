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
#include "tivx_kernel_sde_disparity_visualize.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

#include "stdio.h"

#define NUM_FRAC_BITS        4
#define SDE_DISPARITY_OFFSET 3

const unsigned char SDE_falseColorLUT_RGB[3][260] = {
    {128,64,0,24,33,28,24,21,18,14,10,6,3,1,2,2,2,3,2,3,2,2,2,2,3,3,3,2,2,2,3,3,2,3,1,3,3,2,2,3,2,3,3,2,2,3,2,2,3,3,3,3,2,2,4,2,3,3,2,3,3,2,2,3,3,3,2,2,3,2,2,3,1,3,2,3,2,3,3,3,2,2,2,2,3,2,3,2,3,3,3,3,2,2,2,3,2,3,2,4,2,1,3,2,2,2,3,3,3,2,2,2,1,8,13,20,26,31,38,44,50,56,63,67,74,81,86,93,99,104,110,117,123,129,136,140,147,155,159,166,172,177,183,191,196,202,209,214,219,225,231,238,244,249,255,254,255,255,255,255,255,255,255,255,254,255,255,254,255,255,255,255,255,255,255,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,254,255,255,255,255,255,255,255,255,255,255,255,254,255,255,254,255,255,255,255,255,255,255,254,254,255,254,255,255,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,255,255,255,254,255,255,255,254,255,254,255,255,255,255,255,254,255,255,255,255,255,255,254,255},
    {128,64,0,4,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,6,12,19,25,32,37,43,51,57,63,70,76,82,89,95,101,109,115,120,127,133,140,146,152,158,165,172,178,184,190,196,204,209,216,222,229,236,241,247,254,254,254,255,254,254,255,254,254,255,254,255,254,254,254,254,253,254,253,254,254,253,255,253,253,254,254,254,254,254,254,254,253,254,253,254,254,253,254,254,254,254,253,254,253,254,254,253,254,254,254,253,254,254,254,254,253,254,253,254,255,254,254,254,254,254,254,255,254,255,254,254,255,254,254,254,255,254,255,255,255,255,255,252,249,247,244,241,239,237,234,231,230,227,225,222,219,217,215,211,209,207,205,201,200,198,195,192,189,187,184,181,179,177,174,171,169,168,164,162,160,157,154,152,150,147,144,142,139,138,135,132,130,126,124,122,120,116,114,112,109,107,105,100,97,94,90,87,83,81,76,73,70,67,63,59,57,52,49,45,43,39,35,31,29,25,21,18,15,11,7,4,1,0,0,1,0,1,0,1,1,0,1,1,1,1,1,255},
    {128,64,0,74,96,101,104,108,113,116,120,125,129,135,142,148,153,160,166,174,179,185,192,198,205,211,217,224,230,235,242,248,255,255,255,255,255,254,255,255,255,255,255,254,253,255,255,255,254,255,255,255,255,255,255,255,254,254,255,255,255,255,254,254,255,255,255,255,255,255,255,255,255,249,242,236,231,224,217,210,205,199,192,186,179,173,169,162,155,149,144,138,130,123,117,112,105,99,91,87,80,73,67,60,54,48,41,35,28,23,17,9,2,5,4,4,3,3,4,3,3,2,3,4,4,4,4,4,3,3,2,3,3,2,5,4,4,4,3,4,3,3,2,3,3,4,4,4,4,3,3,4,3,3,2,2,3,4,5,2,3,4,5,2,3,4,3,3,4,4,3,3,4,3,3,3,4,3,4,3,4,3,3,4,2,3,3,4,3,4,3,2,3,4,3,2,3,4,4,3,3,4,2,3,4,3,2,3,4,2,2,3,4,2,3,2,2,3,3,2,2,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,2,2,2,3,2,2,2,3,9,16,23,27,34,40,48,53,59,66,73,77,85,89,255}
};


static void visualizeSdeDisparity(
        tivx_sde_disparity_vis_params_t* params,
        int16_t * disparity,
        uint8_t * disparity_rgb,
        int32_t   disparity_size[2],
        int32_t   disparity_stride,
        int32_t   disparity_rgb_stride);


static tivx_target_kernel vx_sde_disparity_visualize_target_kernel = NULL;

static vx_status VX_CALLBACK tivxSdeDisparityVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeDisparityVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeDisparityVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxSdeDisparityVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *disparity_desc;
    tivx_obj_desc_image_t *disparity_rgb_desc;

    if ( num_params != TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_RGB_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        void *configuration_target_ptr;
        void *disparity_target_ptr;
        void *disparity_rgb_target_ptr;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_CONFIGURATION_IDX];
        disparity_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_IDX];
        disparity_rgb_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_RGB_IDX];

        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        disparity_target_ptr = tivxMemShared2TargetPtr(&disparity_desc->mem_ptr[0]);
        disparity_rgb_target_ptr = tivxMemShared2TargetPtr(&disparity_rgb_desc->mem_ptr[0]);

        tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(disparity_target_ptr,
            disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(disparity_rgb_target_ptr,
            disparity_rgb_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        /* call kernel processing function */
        {
            int32_t disparity_stride;
            int32_t disparity_size[2];
            tivx_sde_disparity_vis_params_t * visParams;

            visParams = (tivx_sde_disparity_vis_params_t *)configuration_target_ptr;

            disparity_size[0] = disparity_desc->height;
            disparity_size[1] = disparity_desc->width;
            disparity_stride  = disparity_desc->imagepatch_addr[0].stride_y/disparity_desc->imagepatch_addr[0].stride_x;

            visualizeSdeDisparity(
                visParams,
                (int16_t *)disparity_target_ptr,
                (uint8_t *)disparity_rgb_target_ptr,
                disparity_size,
                disparity_stride,
                disparity_rgb_desc->imagepatch_addr[0].stride_y
            );
        }

        /* kernel processing function complete */

        tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(disparity_target_ptr,
            disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(disparity_rgb_target_ptr,
            disparity_rgb_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }

    return status;
}

static vx_status VX_CALLBACK tivxSdeDisparityVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxSdeDisparityVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelSdeDisparityVisualize(void)
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
    else if ( self_cpu == TIVX_CPU_ID_MPU_0 )
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
            vx_sde_disparity_visualize_target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_NAME,
                                target_name[i],
                                tivxSdeDisparityVisualizeProcess,
                                tivxSdeDisparityVisualizeCreate,
                                tivxSdeDisparityVisualizeDelete,
                                NULL,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelSdeDisparityVisualize()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_sde_disparity_visualize_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_sde_disparity_visualize_target_kernel = NULL;
    }
}


void visualizeSdeDisparity(
    tivx_sde_disparity_vis_params_t* params,
    int16_t * disparity,
    uint8_t * disparity_rgb,
    int32_t   disparity_size[2],
    int32_t   disparity_stride,
    int32_t   disparity_rgb_stride)
{
    int32_t i, j, value;
    int16_t local_max;
    int16_t local_min;
    int32_t winWidth  = disparity_size[1];
    int32_t winHeight = disparity_size[0];

    local_min = ((int16_t) params->disparity_min) * -3;
    local_max = (((int16_t) params->disparity_max) * 64) + 63;

    float scaleFactor = (float)((1 << NUM_FRAC_BITS) * (local_max - local_min)) / 255;

    int8_t sign;
    int32_t outDisparity;

    uint8_t valid;
    uint8_t * R, *G, *B;

    int16_t pixDisparity;

    // create disparity_rgb
    R = (uint8_t *)disparity_rgb;
    G = (uint8_t *)disparity_rgb + 1;
    B = (uint8_t *)disparity_rgb + 2;

    // create the falseColor map
    scaleFactor = ((float)1.0) / scaleFactor;

    if (params->disparity_only == 1)
    {
        for (j= 0; j < winHeight; j++)
        {
            for (i= 0; i < winWidth; i++)
            {
                // In operation mode, minimum disparity should be non-negative,
                // so sign bit can be ignored
                pixDisparity = disparity[i];
                // check if disparity is valid based on confidence threshold
                valid = 1;

                // Shift disparity so that minimum disparity and unknown disparity are both zero
                value = (int)(pixDisparity * scaleFactor * valid) + SDE_DISPARITY_OFFSET;
    
                *R = (unsigned char)(SDE_falseColorLUT_RGB[0][value]);
                *G = (unsigned char)(SDE_falseColorLUT_RGB[1][value]);
                *B = (unsigned char)(SDE_falseColorLUT_RGB[2][value]);

                R += 3;
                G += 3;
                B += 3;
            }

            R += (disparity_rgb_stride - 3*winWidth);
            G += (disparity_rgb_stride - 3*winWidth);
            B += (disparity_rgb_stride - 3*winWidth);

            disparity += disparity_stride;
        }

    } else
    {
        for (j= 0; j < winHeight; j++)
        {
            for (i= 0; i < winWidth; i++)
            {
                // In operation mode, minimum disparity should be non-negative,
                // so sign bit can be ignored
                pixDisparity = disparity[i];
                sign = (pixDisparity >> 15) == 0 ? 1: -1;
                outDisparity = (pixDisparity >> 3) & 0xFFF;
                outDisparity *= sign;

                // check if disparity is valid based on confidence threshold
                valid = ((pixDisparity & 0x3) >= params->vis_confidence);

                // Shift disparity so that minimum disparity and unknown disparity are both zero
                value = (int)(outDisparity * scaleFactor * valid) + SDE_DISPARITY_OFFSET;
    
                *R = (unsigned char)(SDE_falseColorLUT_RGB[0][value]);
                *G = (unsigned char)(SDE_falseColorLUT_RGB[1][value]);
                *B = (unsigned char)(SDE_falseColorLUT_RGB[2][value]);

                R += 3;
                G += 3;
                B += 3;
            }

            R += (disparity_rgb_stride - 3*winWidth);
            G += (disparity_rgb_stride - 3*winWidth);
            B += (disparity_rgb_stride - 3*winWidth);

            disparity += disparity_stride;
        }
    }
}


