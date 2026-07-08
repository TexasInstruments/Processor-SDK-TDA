/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include "test_hwa_common.h"
#include <TI/hwa_kernels.h>

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <utils/iss/include/app_iss.h>
#include "tivx_utils_checksum.h"
#include <dcc_viss_imx390_wdr_14b.h>
#include <dcc_viss_imx390_wdr_cac.h>
#include <dcc_viss_ov2312_color_diff.h>
#include <inttypes.h>

#include <TI/hwa_vpac_ldc.h>
#include <math.h>
#include <float.h>
#include <TI/hwa_dmpac_dof.h>
#include "test_tiovx/utils/test_utils_file_bmp_rd_wr.h"
#include <TI/hwa_dmpac_sde.h>

void ct_teardown_hwa_kernels(void/*vx_context*/ **context_)
{
    vx_context context = (vx_context)*context_;
    if (context == NULL)
        return;

    if (CT_HasFailure())
    {
        tivxHwaUnLoadKernels(context);
    }
}

void ct_read_raw_image_to_vx_image(vx_image image, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];
    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_uint32 width, height;
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            vx_map_id map_id;
            vx_df_image df;
            void *data_ptr;
            vx_uint32 num_bytes = 1;
            vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
            vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
            vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
            if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
            {
                num_bytes = 2;
            }
            else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
            {
                num_bytes = 4;
            }
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;
            vxMapImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                VX_NOGAP_X
                );
            if(file_byte_pack == num_bytes)
            {
                memcpy(data_ptr, buf, width*height*num_bytes);
            }
            else if((file_byte_pack == 2) && (num_bytes == 1))
            {
                int i;
                uint8_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(i = 0; i < width*height; i++)
                {
                    dst[i] = src[i];
                }
            }
            vxUnmapImagePatch(image, map_id);
        }
    }
    ct_free_mem(buf);
    fclose(f);
}

void ct_write_image2(vx_image image, const char* fileName)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "wb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }
    else
    {
        vx_uint32 width, height;
        vx_imagepatch_addressing_t image_addr;
        vx_rectangle_t rect;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr;
        vx_uint32 num_bytes = 1;

        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
        {
            num_bytes = 2;
        }
        else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
        {
            num_bytes = 4;
        }

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        fwrite(data_ptr, 1, width*height*num_bytes, f);

        vxUnmapImagePatch(image, map_id);
    }

    fclose(f);
}

vx_status readNV12Input(char* file_name, vx_image in_img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)in_img);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_size arr_len;
        vx_int32 i, j;

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            uint8_t *data_ptr_8;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32 img_format;
            vx_uint32  num_bytes = 0;

            vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(in_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);

            if(status != VX_SUCCESS)
            {
                /* Ensure file is closed if mapping falis*/
                fclose(fp);
                vxUnmapImagePatch(in_img, map_id);
                return status; 
            }
            /* Copy Luma */
            data_ptr_8 = (uint8_t *)data_ptr;
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fread(data_ptr_8, 1, img_width, fp);
                data_ptr_8 += image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height)) {
                printf("Luma bytes read = %d, expected = %d\n", num_bytes, img_width*img_height);
                vxUnmapImagePatch(in_img, map_id);
                fclose(fp);
                return (VX_FAILURE);
            }

            vxUnmapImagePatch(in_img, map_id);

            if(img_format == VX_DF_IMAGE_NV12)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height / 2;
                status = vxMapImagePatch(in_img,
                                        &rect,
                                        1,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);


                if(status != VX_SUCCESS)
                {
                    /* Ensure file is closed if mapping falis*/
                    fclose(fp);
                    vxUnmapImagePatch(in_img, map_id);
                    return status; 
                }
                /* Copy CbCr */
                num_bytes = 0;
                data_ptr_8 = (uint8_t *)data_ptr;
                for (j = 0; j < img_height/2; j++)
                {
                    num_bytes += fread(data_ptr_8, 1, img_width, fp);
                    data_ptr_8 += image_addr.stride_y;
                }

                if(num_bytes != (img_width*img_height/2)) {
                    printf("CbCr bytes read = %d, expected = %d\n", num_bytes, img_width*img_height/2);
                    vxUnmapImagePatch(in_img, map_id);
                    fclose(fp);
                    return (VX_FAILURE);
                }

                vxUnmapImagePatch(in_img, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

vx_int32 write_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    uint8_t *temp_ptr = NULL;
    vx_uint32 num_bytes_per_pixel = 1;
    vx_uint32 num_luma_bytes_written_to_file = 0, num_chroma_bytes_written_to_file = 0, num_bytes_written_to_file = 0;
    vx_int32 i;

    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(out_image,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        fclose(fp);
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr1;
    for(i=0; i<height; i++)
    {
        num_luma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    vxMapImagePatch(out_image,
        &rect,
        1,
        &map_id2,
        &image_addr,
        &data_ptr2,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr2)
    {
        printf("data_ptr2 is NULL \n");
        fclose(fp);
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr2;
    for(i=0; i<height/2; i++)
    {
        num_chroma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    num_bytes_written_to_file = num_luma_bytes_written_to_file + num_chroma_bytes_written_to_file;

    vxUnmapImagePatch(out_image, map_id1);
    vxUnmapImagePatch(out_image, map_id2);

    return num_bytes_written_to_file;
}

/* Open and write NV12 output image */
vx_int32 write_output_image_nv12_8bit(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);

    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out);
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1 ;
}


/* VISS common functions */

void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack, uint16_t downshift_bits)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_uint32 width, height;
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            vx_map_id map_id;
            void *data_ptr;
            vx_uint32 num_bytes = 1;
            tivx_raw_image_format_t format[3];

            tivxQueryRawImage(image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

            if( format[0].pixel_container == TIVX_RAW_IMAGE_16_BIT )
            {
                num_bytes = 2;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_8_BIT )
            {
                num_bytes = 1;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_P12_BIT )
            {
                num_bytes = 0;
            }

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            tivxMapRawImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                TIVX_RAW_IMAGE_PIXEL_BUFFER
                );

            if((file_byte_pack == num_bytes) && (downshift_bits == 0))
            {
                int i;
                uint8_t *dst = data_ptr;
                uint8_t *src = (uint8_t*)buf;
                for(i = 0; i < height; i++)
                {
                    memcpy((void*)&dst[image_addr.stride_y*i], (void*)&src[width*num_bytes*i], width*num_bytes);
                }
            }
            else if((file_byte_pack == 2) && (num_bytes == 2))
            {
                int i, j;
                uint16_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(j = 0; j < height; j++)
                {
                    for(i = 0; i < width; i++)
                    {
                        dst[i] = src[i] >> downshift_bits;
                    }
                    dst += image_addr.stride_y/2;
                    src += width;
                }
            }
            else if((file_byte_pack == 2) && (num_bytes == 1))
            {
                int i, j;
                uint8_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(j = 0; j < height; j++)
                {
                    for(i = 0; i < width; i++)
                    {
                        dst[i] = src[i];
                    }
                    dst += image_addr.stride_y;
                    src += width;
                }
            }
            else
            {
                if(file_byte_pack != num_bytes)
                {
                    printf("ct_read_raw_image: size mismatch!!\n");
                    fclose(f);
                    return;
                }
            }
            tivxUnmapRawImagePatch(image, map_id);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

void ct_read_hist(vx_distribution hist, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Hist file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open hist file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_map_id map_id;
            vx_df_image df;
            void *data_ptr;
            vx_uint32 num_bytes = 1;

            vxCopyDistribution (hist, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

void ct_read_user_data_object(vx_user_data_object user_data_object, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("User data object file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open arrays file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_size size = 0;
            vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &size, sizeof(size));
            if (sz < size)
            {
                vxCopyUserDataObject(user_data_object, 0, sz, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
            else
            {
                vxCopyUserDataObject(user_data_object, 0, size, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

void ct_write_user_data_object(vx_user_data_object user_data_object, const char* fileName)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];
    vx_size size;
    vx_status status;

    if (!fileName)
    {
        CT_ADD_FAILURE("User data object file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "wb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open user data object file: %s\n", fileName);
        return;
    }

    status = vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &size, sizeof(size));
    if (VX_SUCCESS != status)
    {
        fclose(f);
        return;
    }

    if( size > 0 )
    {
        buf = (char*)ct_alloc_mem(size);
        if (NULL != buf)
        {
            status = vxCopyUserDataObject(user_data_object, 0, size, buf, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS != status)
            {
                ct_free_mem(buf);
                fclose(f);
                return;
            }
            fwrite(buf, 1, size, f);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}


vx_status save_image_from_viss(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s_y8.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

vx_int32 ct_cmp_image2(vx_image image, vx_image image_ref)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr, ref_addr;
    vx_rectangle_t rect;
    vx_map_id map_id, map_id_ref;
    vx_df_image df;
    void *data_ptr, *ref_ptr;
    vx_uint32 num_bytes = 1;
    vx_int32 i, j;
    vx_int32 error = 0;

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(image,
        &rect,
        0,
        &map_id,
        &image_addr,
        &data_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    vxMapImagePatch(image_ref,
        &rect,
        0,
        &map_id_ref,
        &ref_addr,
        &ref_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if( df == VX_DF_IMAGE_U8)
    {
        for(j=0; j<height; j++)
        {
            vx_uint8 *d_ptr = (vx_uint8 *)((vx_uint8 *)data_ptr + (j * image_addr.stride_y));
            vx_uint8 *r_ptr = (vx_uint8 *)((vx_uint8 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }
    else if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
    {
        for(j=0; j<height; j++)
        {
            vx_uint16 *d_ptr = (vx_uint16 *)((vx_uint16 *)data_ptr + (j * image_addr.stride_y));
            vx_uint16 *r_ptr = (vx_uint16 *)((vx_uint16 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }
    else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
    {
        for(j=0; j<height; j++)
        {
            vx_uint32 *d_ptr = (vx_uint32 *)((vx_uint32 *)data_ptr + (j * image_addr.stride_y));
            vx_uint32 *r_ptr = (vx_uint32 *)((vx_uint32 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }

    vxUnmapImagePatch(image, map_id);
    vxUnmapImagePatch(image_ref, map_id_ref);

    return error;
}

vx_int32 ct_cmp_hist(vx_distribution hist, vx_distribution hist_ref)
{
    vx_map_id map_id, map_id_ref;
    uint32_t *data_ptr, *ref_ptr;
    vx_uint32 num_bytes = 1;
    vx_size histogram_numBins;
    vx_int32 i, j;
    vx_int32 error = 0;

    vxQueryDistribution(hist, VX_DISTRIBUTION_BINS, &histogram_numBins, sizeof(histogram_numBins));

    vxMapDistribution(hist,
        &map_id,
        (void *)&data_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        0);

    vxMapDistribution(hist_ref,
        &map_id_ref,
        (void *)&ref_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        0);

    for(i=0; i<histogram_numBins; i++)
    {
        if(data_ptr[i] != ref_ptr[i])
        {
            error++;
        }
    }

    vxUnmapDistribution(hist, map_id);
    vxUnmapDistribution(hist_ref, map_id_ref);

    return error;
}

vx_int32 get_dcc_file_size(char * file_name)
{
    vx_uint32 num_bytes;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    num_bytes = ftell(fp);
    fclose(fp);

    return num_bytes;
}

vx_int32 read_dcc_file(char * file_name, uint8_t * buf, uint32_t num_bytes)
{
    vx_uint32 num_bytes_read_from_file;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    num_bytes_read_from_file = fread(buf, sizeof(uint8_t), num_bytes, fp);
    fclose(fp);

    return num_bytes_read_from_file;
}

CT_Image raw_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

/* Write NV12 output image */
vx_int32 write_viss_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    uint8_t *temp_ptr = NULL;

    vx_uint32 num_bytes_per_pixel = 1;
    vx_uint32 num_bytes_written_to_file = 0;
    vx_int32 i;
    uint8_t width_multiplier = 1;
    vx_bool is_yuv422 = 0;

    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    if(df == VX_DF_IMAGE_UYVY || df == VX_DF_IMAGE_YUYV){
        width_multiplier = 2;
        is_yuv422 =1;
    }

    printf("out width =  %d\n", width);
    printf("out height =  %d\n", height);
    printf("out format =  %d\n", df);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width_multiplier*width;
    rect.end_y = height;

    vxMapImagePatch(out_image,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr1;
    for(i=0; i<height; i++)
    {
        num_bytes_written_to_file += fwrite(temp_ptr, 1, width_multiplier*width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    vxUnmapImagePatch(out_image, map_id1);

    if(is_yuv422 == 0U)
    {
        vxMapImagePatch(out_image,
            &rect,
            1,
            &map_id2,
            &image_addr,
            &data_ptr2,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(!data_ptr2)
        {
            printf("data_ptr2 is NULL \n");
            return -1;
        }

        temp_ptr = (uint8_t *)data_ptr2;
        for(i=0; i<height/2; i++)
        {
            num_bytes_written_to_file += fwrite(temp_ptr, 1, width_multiplier*width*num_bytes_per_pixel, fp);
            temp_ptr += image_addr.stride_y;
        }
        
        vxUnmapImagePatch(out_image, map_id2);
    }

    return num_bytes_written_to_file;
}

/* Open and write 8bit output image */
vx_int32 write_viss_output_image(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);

    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_viss_output_image_fp(fp, out);
    printf("Written image \n");
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1 ;
}

uint8_t isMuxValid(int mux0, int mux1, int mux2, int mux3, int mux4)
{
    uint8_t retVal = 1;

    if ( (mux0 == 1) || (mux0 == 2) || (mux0 == 5) )
    {
        retVal = 0;
    }

    if ( (mux1 == 1) || (mux1 == 3) || (mux1 == 4) || (mux1 == 5) )
    {
        retVal = 0;
    }

    if ( (mux3 == 3) || (mux3 == 4) || (mux3 == 5) )
    {
        retVal = 0;
    }

    if ( (mux4 == 0) || (mux4 == 4) || (mux4 == 5) )
    {
        retVal = 0;
    }

    return retVal;
}

vx_int32 write_output_ir_image(char * file_name, vx_image out, vx_uint32 ir_format)
{
    FILE * fp;
    vx_uint32 width, height;
    vx_rectangle_t rect;
    void *data_ptr1;
    vx_map_id map_id1;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_uint32 num_bytes_written_to_file = 0;

    printf("Opening file %s \n", file_name);
    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }

    vxQueryImage(out, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    printf("out width =  %d\n", width);
    printf("out height =  %d\n", height);
    printf("out format =  %d\n", df);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(out,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        fclose(fp);
        return -1;
    }


    if(0 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*1, fp);
    else if(1 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*1.5, fp);
    else if(2 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*2, fp);

    printf("Written image \n");
    fclose(fp);
    printf("%d bytes written to %s\n", num_bytes_written_to_file, file_name);

    return num_bytes_written_to_file;
}

/* MSC common functions */

void scale_set_output_params(tivx_vpac_msc_output_params_t *params,
    uint32_t interpolation, uint32_t iw, uint32_t ih, uint32_t ow, uint32_t oh)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    params->signed_data = 0;
    params->filter_mode = 1; // Multi-phase scaling
    params->coef_shift = 8;
    params->saturation_mode = 0;
    params->offset_x = 0;
    params->offset_y = 0;
    //params->output_align_12bit =
    params->multi_phase.phase_mode = 0;
    params->multi_phase.horz_coef_sel = 0;
    params->multi_phase.vert_coef_sel = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        params->multi_phase.init_phase_x =
                    (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;

        params->multi_phase.init_phase_y =
                    (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
    }
    else
    {
        params->multi_phase.init_phase_x = 0;

        params->multi_phase.init_phase_y = 0;
    }
}

void img_scale_set_coeff(tivx_vpac_msc_coefficients_t *coeff,
    uint32_t interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 256;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    idx = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 256;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256-weight;
            coeff->multi_phase[0][idx ++] = weight;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256-weight;
            coeff->multi_phase[1][idx ++] = weight;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256-weight;
            coeff->multi_phase[2][idx ++] = weight;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256-weight;
            coeff->multi_phase[3][idx ++] = weight;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
    else /* STR_VX_INTERPOLATION_NEAREST_NEIGHBOR */
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
}

CT_Image scale_generate_random(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
    if(NULL == image)
    {
        return NULL;
    }

    return image;
}

CT_Image _scale_generate_simple_gradient(int width, int height, int step_x, int step_y, int offset)
{
    CT_Image image = NULL;
    uint32_t x, y;

    ASSERT_(return 0, step_x > 0);
    ASSERT_(return 0, step_y > 0);

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_image(width, height, VX_DF_IMAGE_U8));

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            uint8_t* ptr = CT_IMAGE_DATA_PTR_8U(image, x, y);
            int v = offset + (y / step_y) + (x / step_x);
            *ptr = (uint8_t)v;
        }
    }

    return image;
}

CT_Image scale_generate_gradient_2x2(const char* fileName, int width, int height)
{
    return _scale_generate_simple_gradient(width, height, 2, 2, 0);
}

CT_Image scale_generate_gradient_16x16(const char* fileName, int width, int height)
{
    return _scale_generate_simple_gradient(width, height, 16, 16, 32);
}

CT_Image scale_generate_pattern3x3(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    uint32_t x, y;

    image = ct_allocate_image(width, height, VX_DF_IMAGE_U8);
    if(NULL == image)
    {
        return NULL;
    }

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            uint8_t* ptr = CT_IMAGE_DATA_PTR_8U(image, x, y);
            int v = ((y % 3) == 1 && (x % 3) == 1) ? 0 : 255;
            *ptr = (uint8_t)v;
        }
    }

    return image;
}

CT_Image scale_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

vx_int32 ct_image_get_pixel_8u(CT_Image img, int x, int y, vx_border_t border)
{
    if (border.mode == VX_BORDER_UNDEFINED)
    {
        if (x < 0 || x >= (int)img->width || y < 0 || y >= (int)img->height)
            return -1; //border
        return *CT_IMAGE_DATA_PTR_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        return CT_IMAGE_DATA_REPLICATE_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        return CT_IMAGE_DATA_CONSTANT_8U(img, x, y, border.constant_value.U8);
    }
    else
    {
        CT_FAIL_(return -1, "Invalid border type");
    }
}

int scale_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    int x_min = (int)floorf(x_src), y_min = (int)floorf(y_src);
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        int sx, sy;
        for (sy = -1; sy <= 1; sy++)
        {
            for (sx = -1; sx <= 1; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1 || candidate == res)
                    return 1;
            }
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_min;
        vx_float32 t = y_src - y_min;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // If the computed coordinate is very close to the boundary (1e-7), we don't
        // consider it out-of-bound, in order to handle potential float accuracy errors
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e)
        {
            vx_bool defined_any = (vx_bool)((p00 != -1) || (p10 != -1) || (p01 != -1) || (p11 != -1));
            if (defined_any)
            {
                if ((p00 == -1 || p10 == -1) && fabs(t - 1.0) <= 1e-7)
                    p00 = p10 = 0;
                else if ((p01 == -1 || p11 == -1) && fabs(t - 0.0) <= 1e-7)
                    p01 = p11 = 0;
                if ((p00 == -1 || p01 == -1) && fabs(s - 1.0) <= 1e-7)
                    p00 = p01 = 0;
                else if ((p10 == -1 || p11 == -1) && fabs(s - 0.0) <= 1e-7)
                    p10 = p11 = 0;
                defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
            }
        }
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // A difference of 1 is allowed
        if (abs(res - ref) <= 1) {
            return 1;
        }
        else
        {
            //printf("res = %d\n", res);
            //printf("ref = %d\n", ref);
        }

        return 0; // don't generate failure, we will check num failed pixels later
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 v_min = 256, v_max = -1;
        int sx, sy;
        // check values at 5x5 area
        for (sy = -2; sy <= 2; sy++)
        {
            for (sx = -2; sx <= 2; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1)
                    return 1;
                if (v_min > candidate)
                    v_min = candidate;
                if (v_max < candidate)
                    v_max = candidate;
            }
            if (v_min <= res && v_max >= res)
                return 1;
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

int scale_check_pixel_exact(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    vx_float32 x_minf = floorf(x_src);
    vx_float32 y_minf = floorf(y_src);
    int x_min = (vx_int32)x_minf;
    int y_min = (vx_int32)y_minf;
    int x_ref = x_min;
    int y_ref = y_min;
    if (x_src - x_minf >= 0.5f)
        x_ref++;
    if (y_src - y_minf >= 0.5f)
        y_ref++;
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        vx_int32 ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border); // returning an incorrect value
        if (ref == -1 || ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_minf;
        vx_float32 t = y_src - y_minf;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // If the computed coordinate is very close to the boundary (1e-7), we don't
        // consider it out-of-bound, in order to handle potential float accuracy errors
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e)
        {
            vx_bool defined_any = (vx_bool)((p00 != -1) || (p10 != -1) || (p01 != -1) || (p11 != -1));
            if (defined_any)
            {
                if ((p00 == -1 || p10 == -1) && fabs(t - 1.0) <= 1e-7)
                    p00 = p10 = 0;
                else if ((p01 == -1 || p11 == -1) && fabs(t - 0.0) <= 1e-7)
                    p01 = p11 = 0;
                if ((p00 == -1 || p01 == -1) && fabs(s - 1.0) <= 1e-7)
                    p00 = p01 = 0;
                else if ((p10 == -1 || p11 == -1) && fabs(s - 0.0) <= 1e-7)
                    p10 = p11 = 0;
                defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
            }
        }
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // The result must be exact
        if (ref == res) {
            return 1;
        }

        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 ref;
        ASSERT_(return 0, dst->width % src->width == 0 && dst->height % src->height == 0);
        ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border);
        if (ref == -1)
            return 1;
        if (ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

void scale_validate(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    int num_failed = 0;
    if (src->width == dst->width && src->height == dst->height) // special case for scale=1.0
    {
        ASSERT_EQ_CTIMAGE(src, dst);
        return;
    }
    CT_FILL_IMAGE_8U(, dst,
            {
                int check;
                if (exact == 0)
                    ASSERT_NO_FAILURE(check = scale_check_pixel(src, dst, x, y, interpolation, border));
                else
                    ASSERT_NO_FAILURE(check = scale_check_pixel_exact(src, dst, x, y, interpolation, border));
                if (check == 0) {
                    num_failed++;
                }
            });
    //if (interpolation == VX_INTERPOLATION_BILINEAR)
   // {
        int total = dst->width * dst->height;
        if (num_failed ) // 98% should be valid
        {
            printf("exact = %d\n", exact);
            printf("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
            CT_FAIL("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
        }
    //}
}

void scale_check(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    ASSERT(src && dst);
    scale_validate(src, dst, interpolation, border, exact);
    /*int i;
    for (i = 0; i < 16*4; i++)
    {
        printf("src->data.y[%d] = %d\n", i, src->data.y[i]);
        printf("dst->data.y[%d] = %d\n", i, dst->data.y[i]);
    }*/
#if 1
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info_ex(src, 16, 8);
        printf("=== DST ===\n");
        ct_dump_image_info_ex(dst, 16, 8);
    }
#endif
}

void img_dst_size_generator_1_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width;
    *dst_height = height;
}

void img_dst_size_generator_1_3(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width * 3;
    *dst_height = height * 3;
}

void img_dst_size_generator_2_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 1) / 2;
    *dst_height = (height + 1) / 2;
}

void img_dst_size_generator_3_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 2) / 3;
    *dst_height = (height + 2) / 3;
}

void img_dst_size_generator_4_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 3) / 4;
    *dst_height = (height + 3) / 4;
}

void img_dst_size_generator_5_1(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (width + 4) / 5;
    *dst_height = (height + 4) / 5;
}

void img_dst_size_generator_SCALE_PYRAMID_ORB(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = (int)(width * VX_SCALE_PYRAMID_ORB);
    *dst_height = (int)(height * VX_SCALE_PYRAMID_ORB);
}

void img_dst_size_generator_SCALE_NEAR_UP(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width + 1;
    *dst_height = height + 1;
}

void img_dst_size_generator_SCALE_NEAR_DOWN(int width, int height, int* dst_width, int* dst_height)
{
    *dst_width = width - 1;
    *dst_height = height - 1;
}

vx_status save_image_from_msc(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

vx_status readUYVYInput(char* file_name, vx_image in_img)
{
    vx_status status;
    status = vxGetStatus((vx_reference)in_img);
    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_size arr_len;
        vx_int32 i, j;
        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }
        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            uint8_t *data_ptr_8;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32  img_format;
            vx_uint32  num_bytes = 0;
            vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = 2*img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(in_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);
            if(status != VX_SUCCESS)
            {
                /* Ensure file is closed if mapping falis*/
                fclose(fp);
                vxUnmapImagePatch(in_img, map_id);
                return status; 
            }
            /* Copy input data */
            data_ptr_8 = (uint8_t *)data_ptr;
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fread(data_ptr_8, 1, 2*img_width, fp);
                data_ptr_8 += image_addr.stride_y;
            }
            if(num_bytes != (2*img_width*img_height)) {
                printf("Luma bytes read = %d, expected = %d\n", num_bytes, 2*img_width*img_height);
                vxUnmapImagePatch(in_img, map_id);
                fclose(fp);
                return (VX_FAILURE);
            }
            vxUnmapImagePatch(in_img, map_id);
        }
        fclose(fp);
    }
    return(status);
}
/* Write UYVY output image */
vx_int32 write_uyvy_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    uint8_t *temp_ptr = NULL;
    vx_uint32 num_luma_bytes_written_to_file = 0, num_chroma_bytes_written_to_file = 0, num_bytes_written_to_file = 0;
    vx_int32 i;
    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = 2*width;
    rect.end_y = height;
    vxMapImagePatch(out_image,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );
    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        fclose(fp);
        return -1;
    }
    temp_ptr = (uint8_t *)data_ptr1;
    for(i=0; i<height; i++)
    {
        num_luma_bytes_written_to_file += fwrite(temp_ptr, 1, 2*width, fp);
        temp_ptr += image_addr.stride_y;
    }
    num_bytes_written_to_file = num_luma_bytes_written_to_file;
    vxUnmapImagePatch(out_image, map_id1);
    return num_bytes_written_to_file;
}
/* Open and write yuyv or uyvy output image */
vx_int32 write_uyvy_output_image(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);
    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_uyvy_output_image_fp(fp, out);
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1 ;
}

/* Read NV12 P12 input image */
vx_status readNV12P12Input(char* file_name, vx_image in_img)
{
    vx_status status = VX_SUCCESS;
    FILE * fp = fopen(file_name, "rb");
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t image_addr1;
    vx_imagepatch_addressing_t image_addr2;
    vx_map_id map_id1;
    vx_map_id map_id2;
    void * data_ptr1;
    void * data_ptr2;
    vx_uint32 img_width;
    vx_uint32 img_height;
    vx_uint32 num_bytes_per_pixel = 3; /* 3 bytes for 2 pixels in P12 */
    vx_uint32 num_bytes_read_from_file;
    vx_uint32 num_bytes;

    if(!fp)
    {
        printf("readNV12P12Input: Unable to open file %s\n", file_name);
        return (VX_FAILURE);
    }

    vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
    vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = img_width;
    rect.end_y = img_height;

    /* Read Y plane - P12 format: 3 bytes for 2 pixels */
    status = vxMapImagePatch(in_img, &rect, 0, &map_id1, &image_addr1, &data_ptr1,
                             VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    if(status == VX_SUCCESS)
    {
        num_bytes = ((img_width + 1) / 2) * num_bytes_per_pixel * img_height;
        num_bytes_read_from_file = fread(data_ptr1, 1, num_bytes, fp);

        if(num_bytes_read_from_file != num_bytes)
        {
            printf("readNV12P12Input: Incorrect bytes read for Y plane from file %s\n", file_name);
            status = VX_FAILURE;
        }
        vxUnmapImagePatch(in_img, map_id1);
    }
    else
    {
        printf("readNV12P12Input: Unable to map Y plane\n");
    }

    rect.start_y = 0;
    rect.end_y = img_height / 2;

    /* Read UV plane - P12 format: 3 bytes for 2 pixels */
    if(status == VX_SUCCESS)
    {
        status = vxMapImagePatch(in_img, &rect, 1, &map_id2, &image_addr2,
                                 &data_ptr2, VX_READ_AND_WRITE,
                                 VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    }

    if(status == VX_SUCCESS)
    {
        num_bytes = ((img_width + 1) / 2) * num_bytes_per_pixel * (img_height / 2);
        num_bytes_read_from_file = fread(data_ptr2, 1, num_bytes, fp);

        if(num_bytes_read_from_file != num_bytes)
        {
            printf("readNV12P12Input: Incorrect bytes read for UV plane from file %s\n", file_name);
            status = VX_FAILURE;
        }
        vxUnmapImagePatch(in_img, map_id2);
    }
    else
    {
        printf("readNV12P12Input: Unable to map UV plane\n");
    }

    fclose(fp);

    return status;
}

/* Write NV12 P12 output image to file pointer */
vx_int32 write_output_image_fp_nv12_p12(FILE * fp, vx_image out_image)
{
    vx_status status;

    status = vxGetStatus((vx_reference)out_image);

    if(status == VX_SUCCESS)
    {
        vx_rectangle_t rect;
        vx_imagepatch_addressing_t image_addr1;
        vx_imagepatch_addressing_t image_addr2;
        vx_map_id map_id1;
        vx_map_id map_id2;
        void * data_ptr1;
        void * data_ptr2;
        vx_uint32 img_width;
        vx_uint32 img_height;
        vx_uint32 num_bytes;
        vx_uint32 num_bytes_per_pixel = 3; /* 3 bytes for 2 pixels in P12 */
        vx_uint32 num_bytes_written_to_file = 0;

        vxQueryImage(out_image, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(out_image, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height;

        /* Write Y plane */
        status = vxMapImagePatch(out_image, &rect, 0, &map_id1, &image_addr1, &data_ptr1,
                                 VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

        num_bytes = ((img_width + 1) / 2) * num_bytes_per_pixel * img_height;

        if(status == VX_SUCCESS)
        {
            num_bytes_written_to_file += fwrite(data_ptr1, 1, num_bytes, fp);

            if(num_bytes_written_to_file != num_bytes)
            {
                printf("write_output_image_fp_nv12_p12: Incorrect bytes written for Y plane\n");
            }
        }
        else
        {
            printf("write_output_image_fp_nv12_p12: Unable to map Y plane\n");
        }

        rect.start_y = 0;
        rect.end_y = img_height / 2;

        /* Write UV plane */
        status = vxMapImagePatch(out_image, &rect, 1, &map_id2, &image_addr2,
                                 &data_ptr2, VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

        num_bytes = ((img_width + 1) / 2) * num_bytes_per_pixel * (img_height / 2);

        if(status == VX_SUCCESS)
        {
            num_bytes_written_to_file += fwrite(data_ptr2, 1, num_bytes, fp);

            if(num_bytes_written_to_file != (num_bytes + ((img_width + 1) / 2) * num_bytes_per_pixel * img_height))
            {
                printf("write_output_image_fp_nv12_p12: Incorrect bytes written for UV plane\n");
            }
        }
        else
        {
            printf("write_output_image_fp_nv12_p12: Unable to map UV plane\n");
        }

        vxUnmapImagePatch(out_image, map_id1);
        vxUnmapImagePatch(out_image, map_id2);

        return num_bytes_written_to_file;
    }

    return -1;
}

/* Open and write NV12 P12 output image */
vx_int32 write_output_image_nv12_p12(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);

    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp_nv12_p12(fp, out);
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1;
}

/* LDRA Coverage related functions to capture test coverage data */
#if defined(LDRA_COVERAGE)
int32_t imaging_vpac_coverage_start()
{
    printf("Debug print at coverage_start function start\n");
    int32_t status = VX_SUCCESS;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    CT_Image input = NULL, output = NULL, output2 = NULL, dual_out = NULL;
    
    
    vx_context context = vxCreateContext();
    tivxHwaLoadKernels(context);
    
    input_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_NV12);
    output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_NV12);
    
    
    tivx_vpac_ldc_params_init(&params);
    param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), &params);
    

    graph = vxCreateGraph(context);                                              
    node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL);
    
    vxVerifyGraph(graph);
   

    scalar_err_stat_obj = vxCreateScalar(context,
                VX_TYPE_UINT32 , &scalar_out_err_stat);
    err_refs[0] = (vx_reference)scalar_err_stat_obj;

    status |= tivxNodeSendCommand(node, 0, TIVX_VPAC_CMD_GET_COVERAGE_START, err_refs, 1);
    
    
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);    
    vxReleaseUserDataObject(&param_obj);
    vxReleaseImage(&output_image);
    vxReleaseImage(&input_image);
    tivxHwaUnLoadKernels(context);
    vxReleaseContext(&context);
    printf("Debug print at coverage_start function End\n");
 
    return status;
}

int32_t imaging_vpac_coverage_end()
{
    int32_t status = VX_SUCCESS;
    vx_scalar scalar_err_stat_obj;
    vx_reference err_refs[5] = {0};
    vx_image y8_r8_c2 = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    tivx_raw_image_create_params_t raw_params;
    tivx_raw_image raw = NULL;
    vx_user_data_object configuration = NULL;
    vx_uint32 width, height;
    vx_node node = 0;
    vx_enum scalar_out_err_stat;
    tivx_vpac_viss_params_t params;
 
    raw_params.width = 100;//arg_->width;
    raw_params.height = 100;//arg_->height;
    raw_params.num_exposures = 1;//arg_->exposures;
    raw_params.line_interleaved = 0;//arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;
 
 
    printf("Debug print at coverage_end function start\n");
    vx_context context = vxCreateContext();
    tivxHwaLoadKernels(context);
    
    raw = tivxCreateRawImage(context, &raw_params);
    tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width));
    tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height));
    y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV);  
 
    tivx_vpac_viss_params_init(&params);
    
    params.enable_error_events = 0;
 
    configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL);    
 
    params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_Y8;
    params.fcp[0].mux_output0 = 0;
    params.fcp[0].mux_output1 = 0;
    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
    params.fcp[0].mux_output3 = 0;
    params.fcp[0].mux_output4 = 3;
    params.h3a_aewb_af_mode = 0;
    params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
    params.bypass_glbce = 1;
    params.bypass_nsf4 = 1;
    vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);   
    vx_graph graph = vxCreateGraph(context);                                             
 
    node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL);
    vxVerifyGraph(graph);
 
    
    status |= tivxNodeSendCommand(node, 0, TIVX_VPAC_COVERAGE_END, NULL, 0u);
 
    printf("Debug print at coverage_end function End\n");
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    tivxHwaUnLoadKernels(context);
    vxReleaseContext(&context);
 
    return status;
}  
#if !defined(SOC_AM62A)
int32_t imaging_dmpac_coverage_start()
{
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_image flow_vector_out_img = NULL, confidence_img = NULL;
    vx_image sof_mask = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_user_data_object cs_obj;
    vx_user_data_object sof_config_obj = NULL;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_node node_dof_vis = 0;
    vx_status status = (vx_status)VX_SUCCESS;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    char output_file[256];
    vx_reference ref[1];
    vx_enum flowVectorType = VX_DF_IMAGE_U32;

    uint32_t width = 256, height = 128;
    uint32_t flow_width = width;
    uint32_t flow_height = height;
    uint32_t levels = 2, i;
    vx_enum format = VX_DF_IMAGE_U8;
   
    printf("Debug print at tivx_dmpac_dof_r5_coverage function start\n");
    vx_context context = vxCreateContext();
    tivxHwaLoadKernels(context);
    //CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    tivx_dmpac_dof_params_init(&params);
    param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL);
    
    vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format);
    input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format);
    flow_vector_out = vxCreateImage(context, flow_width, flow_height, flowVectorType);

    graph = vxCreateGraph(context);                                             
 
    node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        NULL,
                        sof_config_obj,
                        sof_mask,
                        flow_vector_out,
                        confidence_histogram);
    vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF);
    vxVerifyGraph(graph);
 
    status |= tivxNodeSendCommand(node_dof, 0, TIVX_DMPAC_CMD_GET_COVERAGE_START, NULL, 0u);
 
    printf("Debug print at tivx_dmpac_dof_r5_coverage function End\n");
    vxReleaseNode(&node_dof);
    vxReleaseGraph(&graph);
    vxReleasePyramid(&input_current);
    vxReleasePyramid(&input_reference);
    vxReleaseImage(&flow_vector_out);
    vxReleaseUserDataObject(&param_obj);
    tivxHwaUnLoadKernels(context);
    vxReleaseContext(&context);
 
    return status;
} 

static vx_status load_image_to_sde(vx_image image, char* filename)
{
    vx_status status;
    char full_filename[1024];

    snprintf(full_filename, 1024, "%s/%s",
        ct_get_test_file_path(), filename);

    status = tivx_utils_load_vximage_from_bmpfile(image, full_filename, vx_false_e);
    return status;
}

int32_t imaging_dmpac_coverage_end()
{
    int32_t status = VX_SUCCESS;
    vx_image left_image = 0, right_image = 0, dst_image = 0;
    vx_distribution histogram = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    uint32_t width = 1280;
    uint32_t height  = 720;
    int i;
   
    printf("Debug print at tivx_dmpac_sde_r5_coverage function start\n");
    vx_context context = vxCreateContext();
    tivxHwaLoadKernels(context);
    
    left_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    right_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    dst_image = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    histogram = vxCreateDistribution(context, 128, 0, 4096);
    
    load_image_to_sde(left_image, "left_rect.bmp");
    load_image_to_sde(right_image, "right_rect.bmp");
    memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
    param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL);
    /* Disable all events by default */
    params.enable_error_events = 0u;
    
    params.median_filter_enable = 0;
    params.disparity_min = 0;
    params.disparity_max = 1;
    params.texture_filter_enable = 1;
    for(i = 0; i < 8; i++) {
            params.confidence_score_map[i] = i*8;
        }
    params.threshold_left_right = 0;
    params.threshold_texture = 0;
    params.aggregation_penalty_p1 = 0;
    params.aggregation_penalty_p2 = 0;
    params.reduced_range_search_enable = 0; 

    vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_sde_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    graph = vxCreateGraph(context);
    node = tivxDmpacSdeNode(graph,
                        param_obj,
                        left_image, right_image, dst_image, histogram);

    vxVerifyGraph(graph);

    status |= tivxNodeSendCommand(node, 0, TIVX_DMPAC_CMD_GET_COVERAGE_END, NULL, 0u);

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    vxReleaseUserDataObject(&param_obj);
    tivxHwaUnLoadKernels(context);
    vxReleaseContext(&context);
 
    return status;
} 
#endif

#endif
