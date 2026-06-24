/*
 *
 * Copyright (c) 2021-2026 Texas Instruments Incorporated
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
#include "app_single_cam_common.h"


#include "ldc_lut_1920x1080.h"
static uint8_t  ldc_lut[] = LDC_LUT_1920_1080;

char *app_get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static char default_sensor_name[ISS_SENSORS_MAX_NAME] = "Noname";
static uint32_t default_sensor_id = 1234;
void app_set_cfg_default(AppObj *obj)
{
    obj->width_in = 1920;
    obj->height_in = 1080;
    obj->width_out = 1920;
    obj->height_out = 1080;
    obj->is_interactive = 1;
    obj->test_mode = 0;
    obj->ldc_enable = 0;
    obj->table_width = LDC_TABLE_WIDTH;
    obj->table_height = LDC_TABLE_HEIGHT;
    obj->ds_factor = LDC_DS_FACTOR;
    obj->sensor_name = default_sensor_name;
    obj->cam_dcc_id = default_sensor_id;
    obj->selectedCam = 0;
#ifdef VPAC3
    obj->vpac3_dual_fcp_enable = 0;
#endif
}

vx_int32 write_aewb_output(FILE * fp, tivx_ae_awb_params_t * aewb_result)
{
    int i;
    printf("exposure_time = %d \n", aewb_result->exposure_time);
    printf("analog_gain = %d \n", aewb_result->analog_gain);
    printf("color_temperature = %d \n", aewb_result->color_temperature);
    for(i=0;i<4;i++)
    {
        printf("wb_gains[%d] = %d \n", i, aewb_result->wb_gains[i]);
    }
    return 0;
}
vx_int32 write_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    vx_uint32 num_bytes_per_4pixels;
    vx_uint32 num_luma_bytes_written_to_file=0;
    vx_uint32 num_chroma_bytes_written_to_file=0;
    vx_uint32 num_bytes_written_to_file=0;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;
    int i;

    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));


    if(VX_DF_IMAGE_NV12 == df)
    {
        num_bytes_per_4pixels = 4;
    }
    else if(TIVX_DF_IMAGE_NV12_P12 == df)
    {
        num_bytes_per_4pixels = 6;
    }
    else
    {
        num_bytes_per_4pixels = 8;
    }

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
        return -1;
    }

    imgaddr_width  = image_addr.dim_x;
    imgaddr_height = image_addr.dim_y;
    imgaddr_stride = image_addr.stride_y;
    printf("imgaddr_width = %d \n", imgaddr_width);
    printf("imgaddr_height = %d \n", imgaddr_height);
    printf("imgaddr_stride = %d \n", imgaddr_stride);
    printf("width = %d \n", width);
    printf("height = %d \n", height);

    num_luma_bytes_written_to_file = 0;

    for(i=0;i<height;i++)
    {
        num_luma_bytes_written_to_file  += fwrite(data_ptr1, 1, width*num_bytes_per_4pixels/4, fp);
        data_ptr1 += imgaddr_stride;
    }
    vxUnmapImagePatch(out_image, map_id1);

    fflush(fp);


    if(VX_DF_IMAGE_NV12 == df || TIVX_DF_IMAGE_NV12_P12 == df)
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

        imgaddr_width  = image_addr.dim_x;
        imgaddr_height = image_addr.dim_y;
        imgaddr_stride = image_addr.stride_y;

        num_chroma_bytes_written_to_file = 0;
        for(i=0;i<imgaddr_height/2;i++)
        {
            num_chroma_bytes_written_to_file  += fwrite(data_ptr2, 1, imgaddr_width*num_bytes_per_4pixels/4, fp);
            data_ptr2 += imgaddr_stride;
        }

        fflush(fp);

        vxUnmapImagePatch(out_image, map_id2);
    }

    num_bytes_written_to_file = num_luma_bytes_written_to_file + num_chroma_bytes_written_to_file;
    printf("Written %d bytes \n", num_bytes_written_to_file);

    return num_bytes_written_to_file;
}

vx_int32 write_output_image_yuv422_8bit(char * file_name, vx_image out_yuv)
{
    FILE * fp = fopen(file_name, "wb");
    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out_yuv);
    fclose(fp);
    APP_PRINTF("%d bytes written to %s\n", len1, file_name);
    return len1;
}

vx_int32 write_output_image_nv12_8bit(char * file_name, vx_image out_nv12)
{
    FILE * fp = fopen(file_name, "wb");
    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out_nv12);
    fclose(fp);
    APP_PRINTF("%d bytes written to %s\n", len1, file_name);
    return len1;
}

vx_int32 read_test_image_raw(char *raw_image_test_fname, tivx_raw_image raw_image, vx_uint32 test_mode)
{
    char raw_image_fname[MAX_FNAME] = {0};
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = app_get_test_file_path();
    FILE * fp;
    vx_uint32 width, height, i;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    void *data_ptr;
    vx_uint32 num_bytes_per_pixel = 2; /*Supports only RAW 12b Unpacked format*/
    vx_uint32 num_bytes_read_from_file = 0;
    tivx_raw_image_format_t format;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;

    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));
    if(0==test_mode)
    {
        if(NULL == test_data_path)
        {
            printf("Test data path is NULL. Defaulting to current folder \n");
            test_data_path = failsafe_test_data_path;
        }
        snprintf(raw_image_fname, MAX_FNAME, "%s/%s.raw", test_data_path, "img_test");
    }
    else if(1==test_mode)
    {
        char test_data_paths[2][255] = {"psdkra/app_single_cam/IMX390_001/input2",
                                         "psdkra/app_single_cam/AR0233_001/input2"};
        int sensor_idx = 0;
        if ((width == 1920) && (height == 1280))
        {
            sensor_idx = 1;
        }
        snprintf(raw_image_fname, MAX_FNAME, "%s/%s.raw", test_data_path, test_data_paths[sensor_idx]);
    }
    else if(2==test_mode)
    {
        if(NULL != raw_image_test_fname)
        {
            memcpy(raw_image_fname, raw_image_test_fname,MAX_FNAME);
        }
    }

    if ('\0' != raw_image_fname[0])
    {
        fp = fopen(raw_image_fname, "rb");
        if(!fp)
        {
            printf("read_test_image_raw : Unable to open file %s\n", raw_image_fname);
            return -1;
        }
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        tivxMapRawImagePatch(raw_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_READ_AND_WRITE,
            VX_MEMORY_TYPE_HOST,
            TIVX_RAW_IMAGE_PIXEL_BUFFER
            );

        if(!data_ptr)
        {
            APP_PRINTF("data_ptr is NULL \n");
            fclose(fp);
            return -1;
        }
        num_bytes_read_from_file = 0;

        imgaddr_width  = image_addr.dim_x;
        imgaddr_height = image_addr.dim_y;
        imgaddr_stride = image_addr.stride_y;

        printf("imgaddr_width = %d \n", imgaddr_width);
        printf("imgaddr_height = %d \n", imgaddr_height);
        printf("imgaddr_stride = %d \n", imgaddr_stride);

        for(i=0;i<imgaddr_height;i++)
        {
            num_bytes_read_from_file += fread(data_ptr, 1, imgaddr_width*num_bytes_per_pixel, fp);
            data_ptr += imgaddr_stride;
        }

        tivxUnmapRawImagePatch(raw_image, map_id);

        fclose(fp);
        printf("%d bytes read from %s\n", num_bytes_read_from_file, raw_image_fname);
    }

    return num_bytes_read_from_file;
}

vx_int32 write_output_image_raw(char * file_name, tivx_raw_image raw_image)
{
    FILE * fp = fopen(file_name, "wb");
    vx_uint32 width, height, i;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    void *data_ptr;
    vx_uint32 num_bytes_per_pixel = 2; /*Supports only RAW12b Unpacked format*/
    vx_uint32 num_bytes_written_to_file;
    tivx_raw_image_format_t format;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;

    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }

    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

    APP_PRINTF("in width =  %d\n", width);
    APP_PRINTF("in height =  %d\n", height);
    APP_PRINTF("in format =  %d\n", format.pixel_container);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    tivxMapRawImagePatch(raw_image,
        &rect,
        0,
        &map_id,
        &image_addr,
        &data_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        TIVX_RAW_IMAGE_PIXEL_BUFFER
        );

    if(!data_ptr)
    {
        APP_PRINTF("data_ptr is NULL \n");
        fclose(fp);
        return -1;
    }
    num_bytes_written_to_file = 0;

    imgaddr_width  = image_addr.dim_x;
    imgaddr_height = image_addr.dim_y;
    imgaddr_stride = image_addr.stride_y;

    for(i=0;i<imgaddr_height;i++)
    {
        num_bytes_written_to_file += fwrite(data_ptr, 1, imgaddr_width*num_bytes_per_pixel, fp);
        data_ptr += imgaddr_stride;
    }

    tivxUnmapRawImagePatch(raw_image, map_id);

    fflush(fp);
    fclose(fp);
    APP_PRINTF("%d bytes written to %s\n", num_bytes_written_to_file, file_name);
    return num_bytes_written_to_file;
}

vx_int32 write_h3a_image(char * file_name, vx_user_data_object out_h3a)
{
    FILE * fp = fopen(file_name, "wb");
    tivx_h3a_data_t *h3a_data = NULL;
    vx_map_id h3a_buf_map_id;
    vx_uint8 *h3a_payload = NULL;
    vx_uint32 h3a_size = 0;
    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }

    vxMapUserDataObject(
            out_h3a,
            0,
            sizeof(tivx_h3a_data_t),
            &h3a_buf_map_id,
            (void **)&h3a_data,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

    h3a_payload = h3a_data->data;// + 12; /*C model 12-byte header*/
    h3a_size = fwrite(h3a_payload, 1, TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES, fp);
    APP_PRINTF("%d bytes saved from H3A output buffer \n", h3a_size);
    fflush(fp);
    fclose(fp);
    vxUnmapUserDataObject(out_h3a, h3a_buf_map_id);

    return h3a_size;
}

vx_status app_create_ldc(AppObj *obj, vx_image ldc_in_image)
{
    uint32_t sensor_dcc_enabled = 1;
    vx_status status = VX_SUCCESS;
    uint32_t table_width, table_height;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    int32_t dcc_buff_size = 0;
    int32_t dcc_buff_size_driver = 0;

#ifdef x86_64
    int32_t dcc_buff_size_fs = 0;
#endif

    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    uint32_t sensor_wdr_enabled = 1;

    printf ("Creating LDC \n");

    obj->dcc_param_ldc = NULL;
    if (1 == sensor_dcc_enabled)
    {
        dcc_buff_size_driver = appIssGetDCCSizeLDC(obj->sensor_name, sensor_wdr_enabled);

        if(dcc_buff_size_driver > 0)
        {
            dcc_buff_size += dcc_buff_size_driver;
        }
        else
        {
            dcc_buff_size_driver = 0;
        }

#ifdef x86_64
        dcc_buff_size_fs = obj->fs_dcc_numbytes_ldc;
        if(dcc_buff_size_fs > 0)
        {
            dcc_buff_size += dcc_buff_size_fs;
        }
#endif

        if (dcc_buff_size <= 0)
        {
            printf("Invalid DCC size for LDC. Disabling DCC \n");
            obj->dcc_param_ldc = NULL;
        }
        else
        {
            obj->dcc_param_ldc = vxCreateUserDataObject(
                    obj->context,
                    (const vx_char*)&dcc_ldc_user_data_object_name,
                    dcc_buff_size,
                    NULL
                    );

            if(status == VX_SUCCESS)
            {
                status = vxMapUserDataObject(
                                obj->dcc_param_ldc,
                                0,
                                dcc_buff_size,
                                &dcc_ldc_buf_map_id,
                                (void **)&dcc_ldc_buf,
                                VX_WRITE_ONLY,
                                VX_MEMORY_TYPE_HOST,
                                0
                         );
            }
            if(status == VX_SUCCESS)
            {
#ifdef x86_64
                if(dcc_buff_size_fs > 0)
                {
                    memcpy(dcc_ldc_buf, obj->fs_dcc_buf_ldc, dcc_buff_size_fs);
                }
                status = appIssGetDCCBuffLDC(obj->sensor_name, sensor_wdr_enabled, &dcc_ldc_buf[dcc_buff_size_fs], dcc_buff_size_driver);
#else
                status = appIssGetDCCBuffLDC(obj->sensor_name, sensor_wdr_enabled,  dcc_ldc_buf, dcc_buff_size_driver);
#endif
                if(status != VX_SUCCESS)
                {
                        printf("Couldn't get LDC DCC buffer from sensor driver \n");
                }

                if(status == VX_SUCCESS)
                {
                    vxUnmapUserDataObject(obj->dcc_param_ldc, dcc_ldc_buf_map_id);
                }
            }
        }
    }

    table_width = (((obj->table_width / (1 << obj->ds_factor)) + 1u) + 15u) & (~15u);
    table_height = ((obj->table_height / (1 << obj->ds_factor)) + 1u);
    /* Mesh Image */
    obj->mesh_img = vxCreateImage(obj->context,table_width, table_height, VX_DF_IMAGE_U32);

    /* Copy Mesh table */
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = table_width;
    rect.end_y = table_height;

    image_addr.dim_x = table_width;
    image_addr.dim_y = table_height;
    image_addr.stride_x = 4u;
    image_addr.stride_y = table_width * 4u;
    if(status == VX_SUCCESS)
    {
        status = vxCopyImagePatch(obj->mesh_img, &rect, 0, &image_addr,
            ldc_lut, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if (VX_SUCCESS != status)
    {
        APP_PRINTF("Copy Image Failed\n");
    }

    /* LDC Output image in NV12 format */
    obj->ldc_out = vxCreateImage(obj->context,
        obj->table_width, obj->table_height,
        VX_DF_IMAGE_NV12);

    /* Mesh Parameters */
    obj->mesh_params_obj = vxCreateUserDataObject(obj->context,
        "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL);
    memset(&obj->mesh_params, 0, sizeof(tivx_vpac_ldc_mesh_params_t));

    tivx_vpac_ldc_mesh_params_init(&obj->mesh_params);
    obj->mesh_params.mesh_frame_width = obj->table_width;
    obj->mesh_params.mesh_frame_height = obj->table_height;
    obj->mesh_params.subsample_factor = obj->ds_factor;
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->mesh_params_obj, 0,
                        sizeof(tivx_vpac_ldc_mesh_params_t), &obj->mesh_params,
                        VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    /* Block Size parameters */
    obj->region_params_obj = vxCreateUserDataObject(obj->context,
        "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t),
        NULL);
    obj->region_params.out_block_width = LDC_BLOCK_WIDTH;
    obj->region_params.out_block_height = LDC_BLOCK_HEIGHT;
    obj->region_params.pixel_pad = LDC_PIXEL_PAD;
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->region_params_obj, 0,
                    sizeof(tivx_vpac_ldc_region_params_t), &obj->region_params,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    /* LDC Configuration */
    tivx_vpac_ldc_params_init(&obj->ldc_params);
    obj->ldc_params.luma_interpolation_type = 1;

    obj->ldc_params.dcc_camera_id = obj->cam_dcc_id;

    obj->ldc_param_obj = vxCreateUserDataObject(obj->context,
        "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL);
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->ldc_param_obj, 0, sizeof(tivx_vpac_ldc_params_t),
                        &obj->ldc_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }

    if (NULL == obj->dcc_param_ldc)
    {
        obj->node_ldc = tivxVpacLdcNode(obj->graph, obj->ldc_param_obj, NULL,
                obj->region_params_obj, obj->mesh_params_obj,
                obj->mesh_img, NULL, ldc_in_image,
                obj->ldc_out, NULL);
    }
    else
    {
        obj->node_ldc = tivxVpacLdcNode(obj->graph, obj->ldc_param_obj, NULL,
                NULL, NULL,
                NULL, obj->dcc_param_ldc, ldc_in_image,
                obj->ldc_out, NULL);
    }
    return status;
}

vx_status app_delete_ldc(AppObj *obj)
{
    if(NULL != obj->dcc_param_ldc)
    {
        vxReleaseUserDataObject(&obj->dcc_param_ldc);
    }

    if(NULL != obj->mesh_img)
    {
        vxReleaseImage(&obj->mesh_img);
    }

    if(NULL != obj->ldc_out)
    {
        vxReleaseImage(&obj->ldc_out);
    }

    if(NULL != obj->mesh_params_obj)
    {
        vxReleaseUserDataObject(&obj->mesh_params_obj);
    }

    if(NULL != obj->region_params_obj)
    {
        vxReleaseUserDataObject(&obj->region_params_obj);
    }

    if(NULL != obj->ldc_param_obj)
    {
        vxReleaseUserDataObject(&obj->ldc_param_obj);
    }

    if(NULL != obj->node_ldc)
    {
        vxReleaseNode(&obj->node_ldc);
    }

    return VX_SUCCESS;
}

vx_status app_create_viss(AppObj *obj, uint32_t sensor_wdr_mode)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 sensor_dcc_enabled = 1;

    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    uint8_t * dcc_viss_buf;
    vx_map_id dcc_viss_buf_map_id;
    unsigned int image_width;
    unsigned int image_height;
    int32_t dcc_buff_size = 0;
    int32_t dcc_buff_size_driver;

#ifdef x86_64
    int32_t dcc_buff_size_fs;
#endif
    vx_uint32 sensor_wdr_enabled = sensor_wdr_mode;

    image_width = obj->width_in;
    image_height = obj->height_in;

    obj->num_viss_out_buf = 3;
    obj->y8_r8_c2 = vxCreateImage(obj->context, image_width, image_height, VX_DF_IMAGE_NV12);
    obj->uv8_g8_c3 = NULL;

    obj->y12 = NULL;
    obj->uv12_c1 = NULL;

#ifdef VPAC3
    /* YUV8 output from dual CC for HV and MV */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        obj->y12 = vxCreateImage(obj->context, image_width, image_height, VX_DF_IMAGE_NV12);
        obj->uv12_c1 = NULL;
    }
#endif

    obj->s8_b8_c4 = NULL;
    obj->histogram = NULL;

    /* VISS Initialize parameters */
    tivx_vpac_viss_params_init(&obj->viss_params);
    obj->viss_params.sensor_dcc_id = obj->cam_dcc_id;
    printf("app_create_viss : sensor_dcc_id = %d \n", obj->viss_params.sensor_dcc_id);
    obj->viss_params.fcp[0].ee_mode = 0;
    obj->viss_params.fcp[0].mux_output0 = 0;
    obj->viss_params.fcp[0].mux_output1 = 0;
    obj->viss_params.fcp[0].mux_output2 = 4;
    obj->viss_params.fcp[0].mux_output3 = 0;
    obj->viss_params.fcp[0].mux_output4 = 3;
    obj->viss_params.h3a_in = 3;
    obj->viss_params.h3a_aewb_af_mode = 0;
    obj->viss_params.channel_id = obj->selectedCam;
    obj->viss_params.fcp[0].chroma_mode = 0;

#if defined(VPAC3) || defined(VPAC3L)
    if(obj->cac_enable == 1U)
    {
        obj->viss_params.bypass_cac = 0;  /* CAC on */
    }
    else
    {
        obj->viss_params.bypass_cac = 1;  /* CAC off */
    }
#endif

#ifdef VPAC3
    /* turn on dual FCP, and NV12 output */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        obj->viss_params.fcp1_config = 1; /* RAWFE --> FCP1 */
        
        /* HV pipeline 8bit YUV output on output2 and output3 */
        obj->viss_params.fcp[0].mux_output0 = 0;
        obj->viss_params.fcp[0].mux_output1 = 0;
        obj->viss_params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        obj->viss_params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        obj->viss_params.fcp[0].mux_output4 = 0;

        /* MV pipeline 8bit YUV output on output2 and output3 */
        obj->viss_params.fcp[1].mux_output0 = 0;
        obj->viss_params.fcp[1].mux_output1 = 0;
        obj->viss_params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        obj->viss_params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        obj->viss_params.fcp[1].mux_output4 = 0;

        /* Mapping for FCP output to select FCP1 or FCP2 for the output muxes */
        obj->viss_params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
        obj->viss_params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
        obj->viss_params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
        obj->viss_params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
        obj->viss_params.output_fcp_mapping[4] = 0;

        obj->viss_params.fcp[1].ee_mode = 0;
        obj->viss_params.fcp[1].chroma_mode = 0;
    }
#endif

    obj->viss_params.enable_ctx = 1;

    if(sensor_wdr_enabled == 1)
    {
        obj->viss_params.bypass_glbce = 0;
    }else
    {
        obj->viss_params.bypass_glbce = 1;
    }
    obj->viss_params.bypass_nsf4 = 0;
    obj->configuration = vxCreateUserDataObject(obj->context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &obj->viss_params);

    /* Create h3a_aew_af output buffer (uninitialized) */
    obj->h3a_aew_af = vxCreateUserDataObject(obj->context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);

    if(sensor_dcc_enabled)
    {
        dcc_buff_size_driver = appIssGetDCCSizeVISS(obj->sensor_name, sensor_wdr_enabled);
        if(dcc_buff_size_driver > 0)
        {
            dcc_buff_size += dcc_buff_size_driver;
        }
        else
        {
            dcc_buff_size_driver = 0;
        }

#ifdef x86_64
        dcc_buff_size_fs = obj->fs_dcc_numbytes_viss;
        if(dcc_buff_size_fs > 0)
        {
            dcc_buff_size += dcc_buff_size_fs;
        }
#endif

        if(dcc_buff_size<=0)
        {
            printf("Invalid DCC size for VISS. Disabling DCC \n");
            obj->dcc_param_viss = NULL;
        }
        else
        {
            obj->dcc_param_viss = vxCreateUserDataObject(
                obj->context,
                (const vx_char*)&dcc_viss_user_data_object_name,
                dcc_buff_size,
                NULL
            );
            vxMapUserDataObject(
                    obj->dcc_param_viss,
                    0,
                    dcc_buff_size,
                    &dcc_viss_buf_map_id,
                    (void **)&dcc_viss_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    0
                );

#ifdef x86_64
            if (dcc_buff_size_fs > 0)
            {
                memcpy(dcc_viss_buf, obj->fs_dcc_buf_viss, dcc_buff_size_fs);
            }
#endif
            if(dcc_buff_size_driver > 0)
            {
#ifdef x86_64
                status = appIssGetDCCBuffVISS(obj->sensor_name, sensor_wdr_enabled, &dcc_viss_buf[dcc_buff_size_fs], dcc_buff_size_driver);
#else
                status = appIssGetDCCBuffVISS(obj->sensor_name, sensor_wdr_enabled, dcc_viss_buf, dcc_buff_size_driver);
#endif
                if(status != VX_SUCCESS)
                {
                    printf("Couldn't get VISS DCC buffer from sensor driver \n");
                }
            }

            if(obj->dcc_param_viss)
            {
                vxUnmapUserDataObject(obj->dcc_param_viss, dcc_viss_buf_map_id);
            }
        }
    }else
    {
        obj->dcc_param_viss = NULL;
    }

    obj->node_viss = tivxVpacVissNode(
                                obj->graph,
                                obj->configuration,
                                NULL,
                                obj->dcc_param_viss,
                                obj->raw, obj->y12, NULL,
                                obj->y8_r8_c2, NULL, NULL,
                                obj->h3a_aew_af, NULL, NULL, NULL
            );

    vxSetReferenceName((vx_reference)obj->node_viss, "VISS_Processing");

    return status;
}

vx_status app_delete_viss(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint32_t buf_id;

    if(NULL != obj->node_viss)
    {
        APP_PRINTF("releasing node_viss\n");
        status |= vxReleaseNode(&obj->node_viss);
    }

    for(buf_id=0; buf_id<obj->num_viss_out_buf; buf_id++)
    {
      if(NULL != obj->viss_out_luma[buf_id])
      {
        APP_PRINTF("releasing y8 buffer # %d\n", buf_id);
        status |= vxReleaseImage(&(obj->viss_out_luma[buf_id]));
      }
    }

    if(NULL != obj->y12)
    {
        APP_PRINTF("releasing y12\n");
        status |= vxReleaseImage(&obj->y12);
    }

    if(NULL != obj->uv12_c1)
    {
        APP_PRINTF("releasing uv12_c1\n");
        status |= vxReleaseImage(&obj->uv12_c1);
    }

    if(NULL != obj->s8_b8_c4)
    {
        APP_PRINTF("releasing s8_b8_c4\n");
        status |= vxReleaseImage(&obj->s8_b8_c4);
    }

    if(NULL != obj->y8_r8_c2)
    {
        APP_PRINTF("releasing y8_r8_c2\n");
        status |= vxReleaseImage(&obj->y8_r8_c2);
    }

    if(NULL != obj->uv8_g8_c3)
    {
        APP_PRINTF("releasing uv8_g8_c3\n");
        status |= vxReleaseImage(&obj->uv8_g8_c3);
    }

    if(NULL != obj->histogram)
    {
        APP_PRINTF("releasing histogram\n");
        status |= vxReleaseDistribution(&obj->histogram);
    }

    if(NULL != obj->configuration)
    {
        APP_PRINTF("releasing configuration\n");
        status |= vxReleaseUserDataObject(&obj->configuration);

    }

    if(NULL != obj->dcc_param_viss)
    {
        APP_PRINTF("releasing VISS DCC Data Object\n");
        status |= vxReleaseUserDataObject(&obj->dcc_param_viss);
    }

    if(NULL != obj->h3a_aew_af)
    {
        APP_PRINTF("releasing h3a_aew_af\n");
        status |= vxReleaseUserDataObject(&obj->h3a_aew_af);
    }

    return status;
}

vx_status app_create_aewb(AppObj *obj, uint32_t sensor_wdr_mode)
{
    vx_status status = VX_SUCCESS;
    int32_t dcc_buff_size = 0;
    int32_t dcc_buff_size_driver = 0;

#ifdef x86_64
    int32_t dcc_buff_size_fs = 0;
#endif

    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    uint8_t * dcc_2a_buf;
    vx_map_id dcc_2a_buf_map_id;
    vx_int32 sensor_dcc_enabled = 1;

    obj->aewb_cfg.sensor_dcc_id = obj->cam_dcc_id;
    obj->aewb_cfg.sensor_img_format = 0;
    obj->aewb_cfg.sensor_img_phase = 3;
    obj->aewb_cfg.awb_num_skip_frames = 0;
    obj->aewb_cfg.ae_num_skip_frames = 0;
    obj->aewb_cfg.channel_id = obj->selectedCam;

    obj->aewb_config = vxCreateUserDataObject(obj->context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &obj->aewb_cfg);

    if(sensor_dcc_enabled)
    {
        dcc_buff_size_driver = appIssGetDCCSize2A(obj->sensor_name, sensor_wdr_mode);

        if(dcc_buff_size_driver > 0)
        {
            dcc_buff_size += dcc_buff_size_driver;
        }
        else
        {
            dcc_buff_size_driver = 0;
        }

#ifdef x86_64
        dcc_buff_size_fs = obj->fs_dcc_numbytes_2a;
        if(dcc_buff_size_fs > 0)
        {
            dcc_buff_size += dcc_buff_size_fs;
        }
#endif

        if(dcc_buff_size<=0)
        {
            printf("Invalid DCC size for 2A. Disabling DCC \n");
            obj->dcc_param_2a = NULL;
        }
        else
        {
            obj->dcc_param_2a = vxCreateUserDataObject(
                obj->context,
                (const vx_char*)&dcc_2a_user_data_object_name,
                dcc_buff_size,
                NULL
            );
            if(status == VX_SUCCESS)
            {
                status = vxMapUserDataObject(
                                    obj->dcc_param_2a,
                                    0,
                                    dcc_buff_size,
                                    &dcc_2a_buf_map_id,
                                    (void **)&dcc_2a_buf,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    0
                                );
            }

            if(status == VX_SUCCESS)
            {
#ifdef x86_64
                if(dcc_buff_size_fs> 0)
                {
                    memcpy(dcc_2a_buf, obj->fs_dcc_buf_2a, dcc_buff_size_fs);
                }
                status = appIssGetDCCBuff2A(obj->sensor_name, sensor_wdr_mode, &dcc_2a_buf[dcc_buff_size_fs], dcc_buff_size_driver);
#else
                status = appIssGetDCCBuff2A(obj->sensor_name, sensor_wdr_mode,  dcc_2a_buf, dcc_buff_size_driver);
#endif
                if(status != VX_SUCCESS)
                {
                    printf("Couldn't get 2A DCC buffer from sensor driver \n");
                    status = VX_SUCCESS;
                }

            }
            vxUnmapUserDataObject(obj->dcc_param_2a, dcc_2a_buf_map_id);
        }
    }
    else
    {
        obj->dcc_param_2a = NULL;
    }

    obj->ae_awb_result =
            vxCreateUserDataObject(obj->context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL);
    if (vxGetStatus((vx_reference)obj->ae_awb_result) != VX_SUCCESS)
    {
        APP_PRINTF("obj->ae_awb_result) create failed\n");
        return VX_FAILURE;
    }

    obj->node_aewb = tivxAewbNode(obj->graph,
                                      obj->aewb_config,
                                      obj->histogram,
                                      obj->h3a_aew_af,
                                      NULL,
                                      obj->ae_awb_result,
                                      obj->dcc_param_2a);
#if defined(edgeai) && defined(SOC_AM62A) && defined(QNX)
    vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
#elif defined(adas) && defined(SOC_AM62A)
    vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU1_0);
#else
    vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU2_0);
#endif
    tivxSetNodeParameterNumBufByIndex(obj->node_aewb, 4u, NUM_BUFS);
    if(NULL != obj->node_aewb)
    {
        vxSetReferenceName((vx_reference)obj->node_aewb, "2A_AlgNode");
    }
    else
    {
        APP_PRINTF("tivxAewbNode returned NULL \n");
        status = VX_FAILURE;
    }
    APP_PRINTF("AEWB Set Reference done\n");

    return status;
}

vx_status app_delete_aewb(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->ae_awb_result)
    {
        APP_PRINTF("releasing ae_awb_result\n");
        vxReleaseUserDataObject(&obj->ae_awb_result);
    }

    if(obj->node_aewb)
    {
        APP_PRINTF("releasing node_aewb\n");
        vxReleaseNode(&obj->node_aewb);
    }

    if(obj->aewb_config)
    {
        APP_PRINTF("releasing aewb_config\n");
        vxReleaseUserDataObject(&obj->aewb_config);
    }

    if(obj->dcc_param_2a)
    {
        APP_PRINTF("releasing 2A DCC Data Object\n");
        vxReleaseUserDataObject(&obj->dcc_param_2a);
    }

    return status;
}

