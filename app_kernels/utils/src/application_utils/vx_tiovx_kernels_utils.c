/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#include <vx_tiovx_kernels_utils.h>

vx_status appKernelsReadImage(char* file_name, vx_image img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_KERNELS_APP_ERROR("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32  num_bytes = 0;
            vx_size    num_planes;
            vx_uint32  plane;
            vx_uint32  plane_size;
            vx_df_image img_format;

            vxQueryImage(img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_PLANES, &num_planes, sizeof(vx_size));
            vxQueryImage(img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_df_image));

            for (plane = 0; plane < num_planes; plane++)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height;
                status = vxMapImagePatch(img,
                                        &rect,
                                        plane,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);

                APP_KERNELS_APP_PRINTF("image_addr.dim_x = %d\n ", image_addr.dim_x);
                APP_KERNELS_APP_PRINTF("image_addr.dim_y = %d\n ", image_addr.dim_y);
                APP_KERNELS_APP_PRINTF("image_addr.step_x = %d\n ", image_addr.step_x);
                APP_KERNELS_APP_PRINTF("image_addr.step_y = %d\n ", image_addr.step_y);
                APP_KERNELS_APP_PRINTF("image_addr.stride_x = %d\n ", image_addr.stride_x);
                APP_KERNELS_APP_PRINTF("image_addr.stride_y = %d\n ", image_addr.stride_y);
                APP_KERNELS_APP_PRINTF("\n");

                num_bytes = 0;
                for (j = 0; j < (image_addr.dim_y/image_addr.step_y); j++)
                {
                    num_bytes += image_addr.stride_x * fread(data_ptr, image_addr.stride_x, (image_addr.dim_x/image_addr.step_x), fp);
                    data_ptr += image_addr.stride_y;
                }

                plane_size = (image_addr.dim_y/image_addr.step_y) * ((image_addr.dim_x * image_addr.stride_x)/image_addr.step_x);

                if(num_bytes != plane_size)
                    APP_KERNELS_APP_ERROR("Plane [%d] bytes read = %d, expected = %d\n", plane, num_bytes, plane_size);

                vxUnmapImagePatch(img, map_id);
            }

        }

        fclose(fp);
    }

    return(status);
}

vx_status appKernelsWriteImage(char* file_name, vx_image img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_KERNELS_APP_ERROR("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32  num_bytes = 0;
            vx_size    num_planes;
            vx_uint32  plane;
            vx_uint32  plane_size;
            vx_df_image img_format;

            vxQueryImage(img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_PLANES, &num_planes, sizeof(vx_size));
            vxQueryImage(img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_df_image));

            for (plane = 0; plane < num_planes; plane++)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height;
                status = vxMapImagePatch(img,
                                        &rect,
                                        plane,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_READ_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);

                APP_KERNELS_APP_PRINTF("image_addr.dim_x = %d\n ", image_addr.dim_x);
                APP_KERNELS_APP_PRINTF("image_addr.dim_y = %d\n ", image_addr.dim_y);
                APP_KERNELS_APP_PRINTF("image_addr.step_x = %d\n ", image_addr.step_x);
                APP_KERNELS_APP_PRINTF("image_addr.step_y = %d\n ", image_addr.step_y);
                APP_KERNELS_APP_PRINTF("image_addr.stride_x = %d\n ", image_addr.stride_x);
                APP_KERNELS_APP_PRINTF("image_addr.stride_y = %d\n ", image_addr.stride_y);
                APP_KERNELS_APP_PRINTF("\n");

                num_bytes = 0;
                for (j = 0; j < (image_addr.dim_y/image_addr.step_y); j++)
                {
                    num_bytes += image_addr.stride_x * fwrite(data_ptr, image_addr.stride_x, (image_addr.dim_x/image_addr.step_x), fp);
                    data_ptr += image_addr.stride_y;
                }

                plane_size = (image_addr.dim_y/image_addr.step_y) * ((image_addr.dim_x * image_addr.stride_x)/image_addr.step_x);

                if(num_bytes != plane_size)
                    APP_KERNELS_APP_ERROR("Plane [%d] bytes written = %d, expected = %d\n", plane, num_bytes, plane_size);

                vxUnmapImagePatch(img, map_id);
            }

        }

        fclose(fp);
    }

    return(status);
}

static vx_uint32 get_bit_depth(vx_enum data_type)
{
    vx_uint32 size = 0;

    if((data_type == VX_TYPE_UINT8) || (data_type == VX_TYPE_INT8))
    {
        size = sizeof(vx_uint8);
    }
    else if((data_type == VX_TYPE_UINT16) || (data_type == VX_TYPE_INT16))
    {
        size = sizeof(vx_uint16);
    }
    else if((data_type == VX_TYPE_UINT32) || (data_type == VX_TYPE_INT32))
    {
        size = sizeof(vx_uint32);
    }
    else if(data_type == VX_TYPE_FLOAT32)
    {
        size = sizeof(vx_float32);
    }

    return size;
}

vx_status appKernelsWriteTensor(char* file_name, vx_tensor tensor_o)
{
    vx_status status = VX_SUCCESS;

    vx_size num_dims;
    void *data_ptr;
    vx_map_id map_id;

    vx_size start[APP_KERNELS_APP_MAX_TENSOR_DIMS];
    vx_size tensor_strides[APP_KERNELS_APP_MAX_TENSOR_DIMS];
    vx_size tensor_sizes[APP_KERNELS_APP_MAX_TENSOR_DIMS];
    vx_enum data_type;

    vxQueryTensor(tensor_o, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    if(num_dims != 3)
    {
        APP_KERNELS_APP_ERROR("Number of dims are != 3 \n");
        status = VX_FAILURE;
    }

    vxQueryTensor(tensor_o, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum));
    vx_uint32 bit_depth = get_bit_depth(data_type);
    if(bit_depth == 0)
    {
        APP_KERNELS_APP_ERROR("Incorrect data_type/bit-depth!\n \n");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vxQueryTensor(tensor_o, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = bit_depth;
        tensor_strides[1] = tensor_sizes[0] * tensor_strides[0];
        tensor_strides[2] = tensor_sizes[1] * tensor_strides[1];

        status = tivxMapTensorPatch(tensor_o, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        FILE *fp = fopen(file_name, "wb");
        if(NULL == fp)
        {
            APP_KERNELS_APP_ERROR("Unable to open file %s \n", file_name);
            status = VX_FAILURE;
        }

        if(VX_SUCCESS == status)
        {
            int32_t size = fwrite(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth, fp);
            if (size != (tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth))
            {
                APP_KERNELS_APP_ERROR("fwrite() size %d not matching with expected size! %d \n", size, (int32_t)(tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth));
                status = VX_FAILURE;
            }

            tivxUnmapTensorPatch(tensor_o, map_id);
        }

        if(fp)
        {
            fclose(fp);
        }
    }

    return(status);
}
