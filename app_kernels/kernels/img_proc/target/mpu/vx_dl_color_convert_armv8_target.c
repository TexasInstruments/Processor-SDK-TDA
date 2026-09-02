/*
*
* Copyright (c) 2022-2026 Texas Instruments Incorporated
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

#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_dl_color_convert_armv8_host.h>
#include <vx_dl_color_convert_armv8_utils.h>

typedef struct {
    bufParams2D_t etn_src[TIVX_IMAGE_MAX_PLANES];
    bufParams2D_t etn_dst[TIVX_IMAGE_MAX_PLANES];
} tivxDLColorConvertArmv8KernelParams;

#define DL_COLOR_CONVERT_MAX_KERNELS 4

static tivx_target_kernel vx_dl_color_convert_armv8_target_kernel[DL_COLOR_CONVERT_MAX_KERNELS] = {NULL};

static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Create
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
);
static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Delete
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
);
static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Process
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
);

static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Create
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* tivx_set_debug_zone(VX_ZONE_INFO); */

    if (num_params != TIVX_DL_COLOR_CONVERT_ARMV8_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        uint32_t i;

        for (i = 0U; i < TIVX_DL_COLOR_CONVERT_ARMV8_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    tivxDLColorConvertArmv8KernelParams * kernelParams = NULL;
    kernelParams = tivxMemAlloc(sizeof(tivxDLColorConvertArmv8KernelParams), TIVX_MEM_EXTERNAL);

    if (NULL == kernelParams)
    {
        status = VX_FAILURE;
    }
    else
    {
        tivx_obj_desc_image_t *in_img_desc;
        tivx_obj_desc_image_t *out_img_desc;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_ARMV8_INPUT_IDX];
        out_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_ARMV8_OUTPUT_IDX];

        /* Setup input params */
        if (in_img_desc->format == VX_DF_IMAGE_RGB)
        {
            kernelParams->etn_src[0].dim_x = in_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_src[0].dim_y = in_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_src[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;
        }
        else if ((in_img_desc->format == VX_DF_IMAGE_UYVY) || (in_img_desc->format == VX_DF_IMAGE_YUYV))
        {
            kernelParams->etn_src[0].dim_x = in_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_src[0].dim_y = in_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_src[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;
        }
        else if ((in_img_desc->format == VX_DF_IMAGE_NV12) || (in_img_desc->format == VX_DF_IMAGE_NV21))
        {
            kernelParams->etn_src[0].dim_x = in_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_src[0].dim_y = in_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_src[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;

            kernelParams->etn_src[1].dim_x = in_img_desc->imagepatch_addr[1].dim_x;
            kernelParams->etn_src[1].dim_y = in_img_desc->imagepatch_addr[1].dim_y /
                                             in_img_desc->imagepatch_addr[1].step_y;
            kernelParams->etn_src[1].stride_y = in_img_desc->imagepatch_addr[1].stride_y;
        }
        else if (in_img_desc->format == VX_DF_IMAGE_IYUV)
        {
            kernelParams->etn_src[0].dim_x = in_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_src[0].dim_y = in_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_src[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;

            kernelParams->etn_src[1].dim_x = in_img_desc->imagepatch_addr[1].dim_x /
                                             in_img_desc->imagepatch_addr[1].step_x;
            kernelParams->etn_src[1].dim_y = in_img_desc->imagepatch_addr[1].dim_y /
                                             in_img_desc->imagepatch_addr[1].step_y;
            kernelParams->etn_src[1].stride_y = in_img_desc->imagepatch_addr[1].stride_y;

            kernelParams->etn_src[2].dim_x = in_img_desc->imagepatch_addr[2].dim_x /
                                             in_img_desc->imagepatch_addr[2].step_x;
            kernelParams->etn_src[2].dim_y = in_img_desc->imagepatch_addr[2].dim_y /
                                             in_img_desc->imagepatch_addr[2].step_y;
            kernelParams->etn_src[2].stride_y = in_img_desc->imagepatch_addr[2].stride_y;
        }
        else if (in_img_desc->format == VX_DF_IMAGE_U8)
        {
            kernelParams->etn_src[0].dim_x = in_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_src[0].dim_y = in_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_src[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;
        }

        /* Setup output params */
        if (out_img_desc->format == VX_DF_IMAGE_RGB)
        {
            kernelParams->etn_dst[0].dim_x = out_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_dst[0].dim_y = out_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_dst[0].stride_y = out_img_desc->imagepatch_addr[0].stride_y;
        }
        else if (out_img_desc->format == VX_DF_IMAGE_NV12)
        {
            kernelParams->etn_dst[0].dim_x = out_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_dst[0].dim_y = out_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_dst[0].stride_y = out_img_desc->imagepatch_addr[0].stride_y;

            kernelParams->etn_dst[1].dim_x = out_img_desc->imagepatch_addr[1].dim_x;
            kernelParams->etn_dst[1].dim_y = out_img_desc->imagepatch_addr[1].dim_y /
                                             out_img_desc->imagepatch_addr[1].step_y;
            kernelParams->etn_dst[1].stride_y = out_img_desc->imagepatch_addr[1].stride_y;
        }
        else if (out_img_desc->format == VX_DF_IMAGE_IYUV)
        {
            kernelParams->etn_dst[0].dim_x = out_img_desc->imagepatch_addr[0].dim_x;
            kernelParams->etn_dst[0].dim_y = out_img_desc->imagepatch_addr[0].dim_y;
            kernelParams->etn_dst[0].stride_y = in_img_desc->imagepatch_addr[0].stride_y;

            kernelParams->etn_dst[1].dim_x = out_img_desc->imagepatch_addr[1].dim_x /
                                             out_img_desc->imagepatch_addr[1].step_x;
            kernelParams->etn_dst[1].dim_y = out_img_desc->imagepatch_addr[1].dim_y /
                                             out_img_desc->imagepatch_addr[1].step_y;
            kernelParams->etn_dst[1].stride_y = out_img_desc->imagepatch_addr[1].stride_y;

            kernelParams->etn_dst[2].dim_x = out_img_desc->imagepatch_addr[2].dim_x /
                                             out_img_desc->imagepatch_addr[2].step_x;
            kernelParams->etn_dst[2].dim_y = out_img_desc->imagepatch_addr[2].dim_y /
                                             out_img_desc->imagepatch_addr[2].step_y;
            kernelParams->etn_dst[2].stride_y = out_img_desc->imagepatch_addr[2].stride_y;
        }

        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxDLColorConvertArmv8KernelParams));
    }

    /* tivx_clr_debug_zone(VX_ZONE_INFO); */

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Delete
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (num_params != TIVX_DL_COLOR_CONVERT_ARMV8_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        uint32_t i;

        for (i = 0U; i < TIVX_DL_COLOR_CONVERT_ARMV8_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDLColorConvertArmv8KernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxDLColorConvertArmv8KernelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status dl_color_convert_process
(
    tivxDLColorConvertArmv8KernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc,
    void *in_image_target_ptr[TIVX_IMAGE_MAX_PLANES],
    void *out_image_target_ptr[TIVX_IMAGE_MAX_PLANES]
)
{
    vx_status status = VX_SUCCESS;

    if (((vx_df_image)VX_DF_IMAGE_RGB == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        status = colorConvert_RGBtoNV12_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0],
            &prms->etn_src[0], (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0],
            (uint8_t *)out_image_target_ptr[1], &prms->etn_dst[1]);
    }
    else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == in_img_desc->format) ) &&
                ((vx_df_image)VX_DF_IMAGE_RGB == out_img_desc->format))
    {
        uint8_t u_pix = (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
        status = colorConvert_NVXXtoRGB_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0], &prms->etn_src[0],
            (uint8_t *)in_image_target_ptr[1], &prms->etn_src[1], (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0], u_pix, in_img_desc->color_space);
    }
    else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == in_img_desc->format) ) &&
                ((vx_df_image)VX_DF_IMAGE_IYUV == out_img_desc->format))
    {
        uint8_t u_pix = (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

        status = colorConvert_NVXXtoIYUV_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0], &prms->etn_src[0],
            (uint8_t *)in_image_target_ptr[1], &prms->etn_src[1], (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0], (uint8_t *)out_image_target_ptr[1],
            &prms->etn_dst[1], (uint8_t *)out_image_target_ptr[2], &prms->etn_dst[2], u_pix);
    }
    else if (((vx_df_image)VX_DF_IMAGE_IYUV == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        status = colorConvert_IYUVtoNV12_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0], &prms->etn_src[0],
            (uint8_t *)in_image_target_ptr[1], &prms->etn_src[1], (uint8_t *)in_image_target_ptr[2], &prms->etn_src[2],
            (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0], (uint8_t *)out_image_target_ptr[1], &prms->etn_dst[1]);
    }
    else if ( ( ((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format) || ((vx_df_image)VX_DF_IMAGE_YUYV == in_img_desc->format) ) &&
                ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        uint8_t y_pix = (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_YUYV) ? 0U : 1U;
        status = colorConvert_YUVXtoNV12_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0], &prms->etn_src[0], (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0],
                    (uint8_t *)out_image_target_ptr[1], &prms->etn_dst[1], y_pix);
    }
    if (((vx_df_image)VX_DF_IMAGE_U8 == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        status = colorConvert_U8toNV12_i8u_o8u_armv8((uint8_t *)in_image_target_ptr[0], &prms->etn_src[0],
                    (uint8_t *)out_image_target_ptr[0], &prms->etn_dst[0], (uint8_t *)out_image_target_ptr[1], &prms->etn_dst[1]);
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (VX_SUCCESS != status)
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}


static vx_status VX_CALLBACK tivxKernelDLColorConvertArmv8Process
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxDLColorConvertArmv8KernelParams *prms = NULL;
    vx_int32 i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDLColorConvertArmv8KernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        tivx_obj_desc_image_t *out_img_desc;
        void* out_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        vx_int32 i;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_ARMV8_INPUT_IDX];
        for (i = 0; i < in_img_desc->planes; i++)
        {
            if(in_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                in_image_target_ptr[i]  = (void *)in_img_desc->mem_ptr[i].host_ptr;
                tivxMemBufferMap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }

        out_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_ARMV8_OUTPUT_IDX];
        for (i = 0; i < out_img_desc->planes; i++)
        {
            if(out_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                out_image_target_ptr[i]  = (void *)out_img_desc->mem_ptr[i].host_ptr;
                tivxMemBufferMap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }

        dl_color_convert_process
        (
            prms,
            in_img_desc,
            out_img_desc,
            in_image_target_ptr,
            out_image_target_ptr
        );

        for (i = 0; i < in_img_desc->planes; i++)
        {
            if (in_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }

        for (i = 0; i < out_img_desc->planes; i++)
        {
            if (out_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }
    }

    tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxDLColorConvertArmv8KernelParams));

    return (status);
}

void tivxAddTargetKernelDLColorConvertArmv8()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[DL_COLOR_CONVERT_MAX_KERNELS][TIVX_TARGET_MAX_NAME];

    strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if( (vx_status)VX_SUCCESS == status)
    {
        int i;

        for (i = 0; i < DL_COLOR_CONVERT_MAX_KERNELS; i++)
        {
            vx_dl_color_convert_armv8_target_kernel[i] = tivxAddTargetKernelByName
                                                         (
                                                             TIVX_KERNEL_DL_COLOR_CONVERT_ARMV8_NAME,
                                                             target_name[i],
                                                             tivxKernelDLColorConvertArmv8Process,
                                                             tivxKernelDLColorConvertArmv8Create,
                                                             tivxKernelDLColorConvertArmv8Delete,
                                                             NULL,
                                                             NULL
                                                         );
        }
    }
}

void tivxRemoveTargetKernelDLColorConvertArmv8()
{
    vx_status status = VX_SUCCESS;
    int i;

    for (i = 0; i < DL_COLOR_CONVERT_MAX_KERNELS; i++)
    {
        status = tivxRemoveTargetKernel(vx_dl_color_convert_armv8_target_kernel[i]);
        if (status == VX_SUCCESS)
        {
            vx_dl_color_convert_armv8_target_kernel[i] = NULL;
        }
    }
}
