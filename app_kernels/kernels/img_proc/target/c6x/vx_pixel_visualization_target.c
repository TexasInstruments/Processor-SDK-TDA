/*
*
* Copyright (c) 2018-2026 Texas Instruments Incorporated
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
#include <tivx_kernels_target_utils.h>
#include <utils/mem/include/app_mem.h>

#include <tivx_pixel_visualization_host.h>
#include "tiadalg_interface.h"
#include "itidl_ti.h"
#include <math.h>

#ifndef x86_64
#if defined (__C7100__) || defined (__C7120__) || defined (__C7524__)
    #include <c7x.h>
    #if defined (C6X_MIGRATION)
        #include <c6x_migration.h>
    #endif
    #define RESTRICT restrict
#else
    #include <c6x.h>
#endif
#endif

static tivx_target_kernel vx_pixViz_kernel = NULL;

static void convert_16bit_to_8bit(uint16_t *pIn, uint8_t *pOut, uint32_t tensor_width, uint32_t tensor_height, uint32_t tensor_pitch);

static vx_status VX_CALLBACK tivxKernelPixelVizCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelPixelVizDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelPixelVizProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    tivxPixelVizParams* viz_params;
    tivx_obj_desc_user_data_object_t* configuration_desc;
    void * configuration_target_ptr = NULL;

    if(obj_desc[0] != NULL)
    {
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_PIXEL_VISUALIZATION_CONFIGURATION_IDX];
        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        viz_params = (tivxPixelVizParams*)configuration_target_ptr;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        for (i = 1U; i < TIVX_KERNEL_PIXEL_VISUALIZATION_BASE_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
        for (i = 0; i < viz_params->num_output_tensors; i++)
        {
            if (NULL == obj_desc[i + TIVX_KERNEL_PIXEL_VISUALIZATION_BASE_PARAMS])
            {
                status = VX_FAILURE;
                break;
            }
        }
        for (; i < (viz_params->num_output_tensors * 2); i++)
        {
            if (NULL == obj_desc[i + TIVX_KERNEL_PIXEL_VISUALIZATION_BASE_PARAMS])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if ((VX_SUCCESS == status) && (viz_params->skip_flag != 1))
    {
        tivx_obj_desc_tensor_t *in_det;
        void* in_tensor_target_ptr = NULL;

        tivx_obj_desc_image_t *out_img_desc;
        void *out_img_target_ptr[2];

        tivx_obj_desc_image_t *in_img_desc;
        void* in_img_target_ptr[2];

        tivx_obj_desc_user_data_object_t *out_args;
        void *out_args_tensor_target_ptr = NULL;

        vx_int32 i,k;
        vx_int32 R,G,B,U,V;

        in_img_target_ptr[0] = NULL;
        in_img_target_ptr[1] = NULL;

        out_img_target_ptr[0] = NULL;
        out_img_target_ptr[1] = NULL;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_PIXEL_VISUALIZATION_INPUT_IMAGE_IDX];
        in_img_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if(viz_params->ip_rgb_or_yuv == 1)
        {
            in_img_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_args  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_PIXEL_VISUALIZATION_TIDLOUTARGS_IDX];
        out_args_tensor_target_ptr  = tivxMemShared2TargetPtr(&out_args->mem_ptr);
        tivxMemBufferMap(out_args_tensor_target_ptr, out_args->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        for(k = 0; k < viz_params->num_output_tensors; k++)
        {
            void *detPtr = NULL;
            uint8_t *in_img[3];
            uint8_t *out_img[3];
            vx_int32  img_height  = in_img_desc->height;
            vx_int32  img_width   = in_img_desc->width;
            vx_int32  det_pitch   = viz_params->output_buffer_pitch[k];
            vx_uint8 uv_color_map[TIVX_PIXEL_VIZ_MAX_CLASS][2];
            uint8_t *pTemp = NULL;
            uint32_t tempBufSize = 0;

            in_det = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + k];
            in_tensor_target_ptr  = tivxMemShared2TargetPtr(&in_det->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr, in_det->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            out_img_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + viz_params->num_output_tensors + k];
            out_img_target_ptr[0] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[0]);
            tivxMemBufferMap(out_img_target_ptr[0], out_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

            if(viz_params->op_rgb_or_yuv == 1)
            {
                out_img_target_ptr[1] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[1]);
                tivxMemBufferMap(out_img_target_ptr[1], out_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }

            /* Initialize for 8-bit, but override for 16-bit */
            detPtr = (vx_uint8  *)in_tensor_target_ptr + viz_params->output_buffer_offset[k];

            if(viz_params->tidl_8bit_16bit_flag == 1)
            {
                vx_uint16 *pIn = (vx_uint16 *)in_tensor_target_ptr + viz_params->output_buffer_offset[k];

                /* ADASVISION-4203 - app_tidl_seg output corrupted for 16bit inference              */
                /* This is a workaround as tiadalg_image_color_blending_c66 does not support 16-bit */ 
                /* Basically converting 16-bit detections to 8-bit detections before calling optimized function */
                vx_int32 tensor_width  = in_det->dimensions[0];
                vx_int32 tensor_height = in_det->dimensions[1];
                tempBufSize = tensor_width * tensor_height;
                pTemp = (uint8_t *)tivxMemAlloc(tempBufSize, TIVX_MEM_EXTERNAL);

                if(pTemp != NULL)
                {
                    convert_16bit_to_8bit(pIn, pTemp, tensor_width, tensor_height, tensor_width);
                    detPtr = (vx_uint8  *)pTemp + viz_params->output_buffer_offset[k];
                }
            }

            for(i = 0; i < TIVX_PIXEL_VIZ_MAX_CLASS; i++)
            {
                R   =  viz_params->color_map[k][i][0];
                G   =  viz_params->color_map[k][i][1];
                B   =  viz_params->color_map[k][i][2];

                U = ((-38 * (R)-74 * (G)+112 * (B)+128) >> 8) + 128;
                V = ((112 * (R)-94 * (G)-18 * (B)+128) >> 8) + 128;

                U = (U > 0) ? (U > 255 ? 255 : U) : 0;
                V = (V > 0) ? (V > 255 ? 255 : V) : 0;

                uv_color_map[i][0] = U;
                uv_color_map[i][1] = V;
            }
            if ((viz_params->op_rgb_or_yuv == 0x1) && (viz_params->ip_rgb_or_yuv == 0x1))
            {

                in_img[0] = in_img_target_ptr[0];
                in_img[1] = in_img_target_ptr[1];
                in_img[2] = NULL;

                out_img[0] = out_img_target_ptr[0];
                out_img[1] = out_img_target_ptr[1];
                out_img[2] = NULL;

                /*If input and output is YUV then call the optimized kernel.
                Currently below kernel is not optimized for other format of input and output*/

                tiadalg_image_color_blending_c66(in_img,
                                                img_width,
                                                img_width,
                                                img_height,
                                                2,0,0,
                                                viz_params->valid_region[k],
                                                detPtr,
                                                det_pitch,
                                                1.0,
                                                &uv_color_map[0][0],
                                                out_img,
                                                img_width
                                                );
            }
            else
            {
                in_img[0] = in_img_target_ptr[0];
                in_img[1] = NULL;
                in_img[2] = NULL;

                out_img[0] = out_img_target_ptr[0];
                out_img[1] = NULL;
                out_img[2] = NULL;

                /*If input and output is RGB then call the un-optimized kernel.
                Currently below kernel is not optimized for RGB input and output.
                This flow is used for accuracy measurement */

                tiadalg_image_color_blending_cn(in_img,
                                                img_width,
                                                img_width,
                                                img_height,
                                                3,1,1,
                                                viz_params->valid_region[k],
                                                detPtr,
                                                det_pitch,
                                                ((TIDL_outArgs*) out_args_tensor_target_ptr)->scale[k],
                                                &uv_color_map[0][0],
                                                out_img,
                                                img_width
                                                );
            }

            if(pTemp != NULL)
            {
                tivxMemFree(pTemp, tempBufSize, TIVX_MEM_EXTERNAL);
            }

            tivxMemBufferUnmap(in_tensor_target_ptr, in_det->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            tivxMemBufferUnmap(out_img_target_ptr[0], out_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            if(viz_params->op_rgb_or_yuv == 1)
            {
                tivxMemBufferUnmap(out_img_target_ptr[1], out_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }

        tivxMemBufferUnmap(out_args_tensor_target_ptr, out_args->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(viz_params->ip_rgb_or_yuv == 1)
        {
          tivxMemBufferUnmap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
    }

    return (status);
}

void tivxAddTargetKernelPixelViz()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_pixViz_kernel = tivxAddTargetKernelByName
                            (
                                TIVX_KERNEL_PIXEL_VISUALIZATION_NAME,
                                target_name,
                                tivxKernelPixelVizProcess,
                                tivxKernelPixelVizCreate,
                                tivxKernelPixelVizDelete,
                                NULL,
                                NULL
                            );
    }
}

void tivxRemoveTargetKernelPixelViz()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_pixViz_kernel);
    if (status == VX_SUCCESS)
    {
        vx_pixViz_kernel = NULL;
    }
}

static void convert_16bit_to_8bit(uint16_t *pIn, uint8_t *pOut, uint32_t tensor_width, uint32_t tensor_height, uint32_t tensor_pitch)
{
    int32_t i, j;
#ifdef x86_64
    for(j = 0; j < tensor_height; j++)
    {
        for(i = 0; i < tensor_width; i++)
        {
            int32_t offset = (tensor_pitch * j) + i;
            pOut[offset] = pIn[offset];
        }
    }
#else
    for(j = 0; j < tensor_height; j++)
    {
        for(i = 0; i < tensor_width; i+=8)
        {
            int32_t offset = (tensor_pitch * j) + i;

            uint64_t i0_3 = _mem8(pIn + offset);
            uint64_t i4_7 = _mem8(pIn + offset + 4);
            _mem8(pOut + offset) = _dspacku4(i0_3, i4_7);
        }

        if((i != tensor_width) && (i > tensor_width))
        {
            i = i - 8;
            for(; i < tensor_width; i++)
            {
                int32_t offset = (tensor_pitch * j) + i;
                pOut[offset] = pIn[offset];
            }
        }
    }
#endif
}
