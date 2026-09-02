/*

 * Copyright (c) 2012-2026 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include "math.h"
#include "tivx_utils_file_rd_wr.h"
#include <limits.h>
#include <TI/tivx_srv.h>
#include <TI/tivx_task.h>
#include <render.h>

#define MAX_LINE_LEN   (256U)
#define NUM_CAMERAS    (4U)

#define MAX_NUM_BUF               (8u)
#define MAX_IMAGE_PLANES          (3u)
#define MAX_NUM_OBJ_ARR_ELEMENTS  (4u)
#define MAX_ABS_FILENAME          (512u)
#define MAX_ABS_FILENAME_FULL     (1024u)

TESTCASE(tivxGlSrv,  CT_VXContext, ct_setup_vx_context, 0)

/*
 * Utility API to set number of buffers at a node parameter
 * The parameter MUST be a output or bidirectonal parameter for the setting
 * to take effect
 */
static vx_status set_num_buf_by_node_index(vx_node node, vx_uint32 node_parameter_index, vx_uint32 num_buf)
{
    return tivxSetNodeParameterNumBufByIndex(node, node_parameter_index, num_buf);
}

/*
 * Utility API to set pipeline depth for a graph
 */
static vx_status set_graph_pipeline_depth(vx_graph graph, vx_uint32 pipeline_depth)
{
    return tivxSetGraphPipelineDepth(graph, pipeline_depth);
}

/*
 * Utility API to set trigger node for a graph
 */
static vx_status set_graph_trigger_node(vx_graph graph, vx_node node)
{
    return vxEnableGraphStreaming(graph, node);
}

/*
 * Utility API to export graph information to file for debug and visualization
 */
static vx_status export_graph_to_file(vx_graph graph, char *filename_prefix)
{
    return tivxExportGraphToDot(graph, ct_get_test_file_path(), filename_prefix);
}

static void make_filename(char *abs_filename, char *filename_prefix)
{
    snprintf(abs_filename, MAX_ABS_FILENAME, "%s/output/%s.bmp",
        ct_get_test_file_path(), filename_prefix);
}

static int load_from_raw_file(vx_image copy_image, int width, int height, const char* filename, int offset)
{
    void* data;
    FILE* fp;
    int dataread;
    int numbytes = 3 * width * height;
    char file[MAX_ABS_FILENAME_FULL];
    vx_rectangle_t rect             = { 0, 0, width, height };
    vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;


    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = 3;
    addr.stride_y = width*3;

    snprintf(file, MAX_ABS_FILENAME_FULL, "%s/%s", ct_get_test_file_path(), filename);
    fp = fopen(file, "rb");

    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    data = ct_alloc_mem(numbytes);
    if (NULL != data)
    {
        memset(data, 0, numbytes);
        fseek(fp, offset, SEEK_CUR);
        dataread = fread(data, 1, numbytes, fp);
        fclose(fp);
        if(dataread != numbytes) {
            printf("Error in file size != width*height\n");
            return -1;
        }

        vxCopyImagePatch(copy_image, &rect, 0, &addr, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        ct_free_mem(data);
    }
    else
    {
        fclose(fp);
    }


    return 0;
}

static void ct_read_image2(vx_image image, const char* fileName, uint16_t file_byte_pack)
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
        if (NULL != buf)
        {
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
        else
        {
            fclose(f);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

static int get_offset(const char *filename)
{
    FILE *fp;
    uint8_t header[54], inter;
    uint32_t offset;
    char file[MAX_ABS_FILENAME_FULL];
    uint32_t  read_size;

    snprintf(file, MAX_ABS_FILENAME_FULL, "%s/%s", ct_get_test_file_path(), filename);
    fp = fopen(file, "rb");

    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    read_size = fread(header, sizeof(uint8_t), 54, fp);
    if (read_size != 54)
    {
        CT_ADD_FAILURE("Incorrect Bytes read from file: %s\n", filename);
        fclose(fp);
        return -1;
    }

    inter = *(uint8_t *)(&header[10]);

    offset = (uint32_t)inter;

    fclose(fp);

    return offset;
}

static vx_status app_load_vximage_from_bin_file(char *filename, vx_image image)
{
    vx_uint32 width, height, num_bytes;
    vx_imagepatch_addressing_t image_addr1, image_addr2;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    vx_df_image df;
    void *data_ptr1, *data_ptr2;
    vx_status status;

    status = vxGetStatus((vx_reference)image);
    if(status==VX_SUCCESS)
    {
        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        status = vxMapImagePatch(image,
            &rect,
            0,
            &map_id1,
            &image_addr1,
            &data_ptr1,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );
        status = vxMapImagePatch(image,
            &rect,
            1,
            &map_id2,
            &image_addr2,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(status==VX_SUCCESS)
        {
            char file[MAX_ABS_FILENAME_FULL];

            snprintf(file, MAX_ABS_FILENAME_FULL, "%s/%s", ct_get_test_file_path(), filename);
            FILE *fp = fopen(file,"rb+a");

            if(fp!=NULL)
            {
                size_t ret;

                num_bytes = (width*height);
                ret = fread(data_ptr1, sizeof(uint8_t), num_bytes, fp);
                if(ret!=num_bytes)
                {
                    printf("# ERROR: Unable to read data from file [%s]\n", file);
                }

                num_bytes = (width*height)/2;
                ret = fread(data_ptr2, sizeof(uint8_t), num_bytes, fp);
                if(ret!=num_bytes)
                {
                    printf("# ERROR: Unable to read data from file [%s]\n", file);
                }
                fclose(fp);
            }
            else
            {
                 printf("# ERROR: Unable to open file for reading [%s]\n", file);
            }
            vxUnmapImagePatch(image, map_id1);
            vxUnmapImagePatch(image, map_id2);
        }
    }
    return status;
}

/* Loading files to object array */
static vx_status load_input_images(vx_context context, vx_object_array input_array, int in_width, int in_height, int num_cameras)
{
    vx_status status = VX_SUCCESS;
    vx_image copy_image, input_image;
    CT_Image ct_read;
    int i, offset;
    char filename[MAX_ABS_FILENAME];

    for (i = 0; i < num_cameras; i++)
    {
        copy_image = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_RGB);

        input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_array, i);

        if (i == 0)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_front00.bmp");
        }
        else if (i == 1)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_right00.bmp");
        }
        else if (i == 2)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_back00.bmp");
        }
        else if (i == 3)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_left00.bmp");
        }

        offset = get_offset(filename);
        if (offset < 0)
        {
            status = offset;
            return status;
        }

        status = load_from_raw_file(copy_image, in_width, in_height, filename, offset);
        if (VX_SUCCESS != status)
        {
            return status;
        }

        vxuColorConvert(context, copy_image, input_image);

        vxReleaseImage(&input_image);
        vxReleaseImage(&copy_image);
    }
    return status;
}

static vx_status load_input_images_2mp(vx_context context, vx_object_array input_array, int in_width, int in_height, int num_cameras)
{
    vx_status status = VX_SUCCESS;
    vx_image input_image;
    CT_Image ct_read;
    int i, offset;
    char filename[MAX_ABS_FILENAME];

    for (i = 0; i < num_cameras; i++)
    {
        input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_array, i);

        if (i == 0)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/applibTC_2mpix/FRONT_0.YUV");
        }
        else if (i == 1)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/applibTC_2mpix/RIGHT_0.YUV");
        }
        else if (i == 2)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/applibTC_2mpix/BACK_0.YUV");
        }
        else if (i == 3)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/applibTC_2mpix/LEFT_0.YUV");
        }

        status = app_load_vximage_from_bin_file(filename, input_image);
        if (VX_SUCCESS != status)
        {
            return status;
        }

        vxReleaseImage(&input_image);
    }
    return status;
}

static void load_input_images_old(vx_context context, vx_object_array input_array, int in_width, int in_height, int num_cameras)
{
    vx_image copy_image, input_image;
    CT_Image ct_read;
    int i, offset;
    char filename[MAX_ABS_FILENAME];

    for (i = 0; i < num_cameras; i++)
    {
        ASSERT_VX_OBJECT(copy_image = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_array, i), VX_TYPE_IMAGE);

        if (i == 0)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_front00.bmp");
        }
        else if (i == 1)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_right00.bmp");
        }
        else if (i == 2)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_back00.bmp");
        }
        else if (i == 3)
        {
            snprintf(filename, MAX_ABS_FILENAME, "%s", "psdkra/srv/standalone/sa_left00.bmp");
        }

        offset = get_offset(filename);
        load_from_raw_file(copy_image, in_width, in_height, filename, offset);

        ct_read = ct_image_from_vx_image(copy_image);
        ct_image_copyto_vx_image(input_image, ct_read);

        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseImage(&copy_image));
    }
}

static vx_status app_save_vximage_to_bin_file(char *filename, vx_image image)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    vx_df_image df;
    void *data_ptr;
    vx_status status;

    status = vxGetStatus((vx_reference)image);
    if(status==VX_SUCCESS)
    {
        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        status = vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(status==VX_SUCCESS)
        {
            FILE *fp = fopen(filename,"wb");

            if(fp!=NULL)
            {
                size_t ret;

                ret = fwrite(data_ptr, image_addr.stride_y, height, fp);
                if(ret!=height)
                {
                    printf("# ERROR: Unable to write data to file [%s]\n", filename);
                }
                fclose(fp);
            }
            else
            {
                printf("# ERROR: Unable to open file for writing [%s]\n", filename);
                status = VX_FAILURE;
            }
            vxUnmapImagePatch(image, map_id);
        }
    }
    return status;
}


typedef struct {
    const char* name;
    int stream_time;
} Arg;

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("gpu_srv", ARG, 100), \

TEST(tivxGlSrv, testGraph1MP)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array input_array, srv_views_array;
    vx_array galign_lut;
    vx_image img_exemplar, output_image;
    vx_user_data_object param_obj, srv_views;
    vx_uint32 i, in_width, in_height, out_width, out_height, num_cameras, num_views, capacity;
    char filename[MAX_ABS_FILENAME];
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    //CT_Image ct_output_ref, ct_output_tst, ct_output_write;
    vx_pixel_value_t value = {{ 0 }};
    //char output_file_path[MAX_ABS_FILENAME];

    in_width = 1280;
    in_height = 720;
    out_width = 1920;
    out_height = 1080;
    num_cameras = 4;
    num_views = 1;
    capacity = 1;

    tivxSrvLoadKernels(context);

    //strncpy(output_file_path, "./gpu_srv_output_1mp.bin", MAX_ABS_FILENAME);

    memset(&params, 0, sizeof(tivx_srv_params_t));
#ifdef x86_64
    params.cam_bpp = 24;
#else
    params.cam_bpp = 12;
#endif

    ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(img_exemplar = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_array = vxCreateObjectArray(context, (vx_reference)img_exemplar, num_cameras), VX_TYPE_OBJECT_ARRAY);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, load_input_images(context, input_array, in_width, in_height, num_cameras));

    VX_CALL(vxReleaseImage(&img_exemplar));

    ASSERT_VX_OBJECT(srv_views = vxCreateUserDataObject(context, "srv_coords_t", sizeof(srv_coords_t), &local_srv_coords), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(srv_views_array = vxCreateObjectArray(context, (vx_reference)srv_views, num_views), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseUserDataObject(&srv_views));

    for (i = 0; i < num_views; i++)
    {
        local_srv_coords.camx =     0.0f;
        local_srv_coords.camy =     0.0f;
        local_srv_coords.camz =   240.0f;
        local_srv_coords.targetx =  0.0f;
        local_srv_coords.targety =  0.0f;
        local_srv_coords.targetz =  0.0f;
        local_srv_coords.anglex =   0.0f;
        local_srv_coords.angley =   0.0f;
        local_srv_coords.anglez =   0.0f;

        ASSERT_VX_OBJECT(srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)srv_views_array, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&srv_views));
    }

    /* TODO: Fill array */
    /* Not using for now */
    ASSERT_VX_OBJECT(galign_lut = vxCreateArray(context, VX_TYPE_UINT16, capacity), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(output_image = vxCreateUniformImage(context, out_width, out_height, VX_DF_IMAGE_RGBX, &value), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxGlSrvNode(graph, param_obj, input_array, srv_views_array, NULL, output_image), VX_TYPE_NODE);
    vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));
    tivxTaskWaitMsecs(500);

    //ct_output_write = ct_image_from_vx_image(output_image);

    //ct_write_image("output/output_reference.bmp", ct_output_write);

    // TODO: Fix reference image check
    //ASSERT_EQ_CTIMAGE(ct_output_ref, ct_output_write);
    //ASSERT_CTIMAGE_NEARWRAP(ct_output_ref, ct_output_write, 1, 0);

    VX_CALL(vxReleaseUserDataObject(&param_obj));
    VX_CALL(vxReleaseObjectArray(&input_array));
    VX_CALL(vxReleaseObjectArray(&srv_views_array));
    VX_CALL(vxReleaseArray(&galign_lut));
    VX_CALL(vxReleaseImage(&output_image));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxSrvUnLoadKernels(context);
}

TEST(tivxGlSrv, testGraph1MPViewChange)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array input_array, srv_views_array;
    vx_array galign_lut;
    vx_image img_exemplar, output_image;
    vx_user_data_object param_obj, srv_views;
    vx_uint32 i, in_width, in_height, out_width, out_height, num_cameras, num_views, capacity;
    vx_status status;
    char filename[MAX_ABS_FILENAME];
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    vx_pixel_value_t value = {{ 0 }};

    in_width = 1280;
    in_height = 720;
    out_width = 1920;
    out_height = 1080;
    num_cameras = 4;
    num_views = 1;
    capacity = 1;

    tivxSrvLoadKernels(context);

    memset(&params, 0, sizeof(tivx_srv_params_t));
#ifdef x86_64
    params.cam_bpp = 24;
#else
    params.cam_bpp = 12;
#endif

    ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(img_exemplar = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_array = vxCreateObjectArray(context, (vx_reference)img_exemplar, num_cameras), VX_TYPE_OBJECT_ARRAY);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, load_input_images(context, input_array, in_width, in_height, num_cameras));

    VX_CALL(vxReleaseImage(&img_exemplar));

    ASSERT_VX_OBJECT(srv_views = vxCreateUserDataObject(context, "srv_coords_t", sizeof(srv_coords_t), &local_srv_coords), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(srv_views_array = vxCreateObjectArray(context, (vx_reference)srv_views, num_views), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseUserDataObject(&srv_views));

    for (i = 0; i < num_views; i++)
    {
        local_srv_coords.camx =     0.0f;
        local_srv_coords.camy =     0.0f;
        local_srv_coords.camz =   240.0f;
        local_srv_coords.targetx =  0.0f;
        local_srv_coords.targety =  0.0f;
        local_srv_coords.targetz =  0.0f;
        local_srv_coords.anglex =   0.0f;
        local_srv_coords.angley =   0.0f;
        local_srv_coords.anglez =   0.0f;

        ASSERT_VX_OBJECT(srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)srv_views_array, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&srv_views));
    }

    /* TODO: Fill array */
    /* Not using for now */
    ASSERT_VX_OBJECT(galign_lut = vxCreateArray(context, VX_TYPE_UINT16, capacity), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(output_image = vxCreateUniformImage(context, out_width, out_height, VX_DF_IMAGE_RGBX, &value), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxGlSrvNode(graph, param_obj, input_array, srv_views_array, NULL, output_image), VX_TYPE_NODE);
    vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    tivxTaskWaitMsecs(500);

    VX_CALL(vxProcessGraph(graph));

    tivxTaskWaitMsecs(500);

    for (i = 0; i < num_views; i++)
    {
        local_srv_coords.camx =     0.0f;
        local_srv_coords.camy =     0.0f;
        local_srv_coords.camz =   440.0f;
        local_srv_coords.targetx =  0.0f;
        local_srv_coords.targety =  0.0f;
        local_srv_coords.targetz =  0.0f;
        local_srv_coords.anglex =   0.0f;
        local_srv_coords.angley =   0.0f;
        local_srv_coords.anglez =   0.0f;

        ASSERT_VX_OBJECT(srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)srv_views_array, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&srv_views));
    }

    VX_CALL(vxProcessGraph(graph));
    tivxTaskWaitMsecs(500);

    for (i = 0; i < num_views; i++)
    {
        local_srv_coords.camx =     0.0f;
        local_srv_coords.camy =     0.0f;
        local_srv_coords.camz =   640.0f;
        local_srv_coords.targetx =  0.0f;
        local_srv_coords.targety =  0.0f;
        local_srv_coords.targetz =  0.0f;
        local_srv_coords.anglex =   0.0f;
        local_srv_coords.angley =   0.0f;
        local_srv_coords.anglez =   0.0f;

        ASSERT_VX_OBJECT(srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)srv_views_array, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&srv_views));
    }

    VX_CALL(vxProcessGraph(graph));
    tivxTaskWaitMsecs(500);

    for (i = 0; i < 25; i++)
    {
        VX_CALL(vxProcessGraph(graph));
        tivxTaskWaitMsecs(50);
    }

    VX_CALL(vxReleaseUserDataObject(&param_obj));
    VX_CALL(vxReleaseObjectArray(&input_array));
    VX_CALL(vxReleaseObjectArray(&srv_views_array));
    VX_CALL(vxReleaseArray(&galign_lut));
    VX_CALL(vxReleaseImage(&output_image));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxSrvUnLoadKernels(context);
}

TEST(tivxGlSrv, testGraph2MP)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array input_array, srv_views_array;
    vx_array galign_lut;
    vx_image img_exemplar, output_image;
    vx_user_data_object param_obj, srv_views;
    vx_uint32 i, in_width, in_height, out_width, out_height, num_cameras, num_views, capacity;
    vx_status status;
    char filename[MAX_ABS_FILENAME];
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    //CT_Image ct_output_ref, ct_output_tst, ct_output_write;
    vx_pixel_value_t value = {{ 0 }};
    //char output_file_path[MAX_ABS_FILENAME];

    in_width = 1920;
    in_height = 1080;
    out_width = 1920;
    out_height = 1080;
    num_cameras = 4;
    num_views = 1;
    capacity = 1;

    tivxSrvLoadKernels(context);

    //strncpy(output_file_path, "./gpu_srv_output.bin", MAX_ABS_FILENAME);

    memset(&params, 0, sizeof(tivx_srv_params_t));
#ifdef x86_64
    params.cam_bpp = 24;
#else
    params.cam_bpp = 12;
#endif

    ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(img_exemplar = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_array = vxCreateObjectArray(context, (vx_reference)img_exemplar, num_cameras), VX_TYPE_OBJECT_ARRAY);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, load_input_images_2mp(context, input_array, in_width, in_height, num_cameras));

    VX_CALL(vxReleaseImage(&img_exemplar));

    ASSERT_VX_OBJECT(srv_views = vxCreateUserDataObject(context, "srv_coords_t", sizeof(srv_coords_t), &local_srv_coords), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(srv_views_array = vxCreateObjectArray(context, (vx_reference)srv_views, num_views), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseUserDataObject(&srv_views));

    for (i = 0; i < num_views; i++)
    {
        local_srv_coords.camx =     0.0f;
        local_srv_coords.camy =     0.0f;
        local_srv_coords.camz =   480.0f;
        local_srv_coords.targetx =  0.0f;
        local_srv_coords.targety =  0.0f;
        local_srv_coords.targetz =  0.0f;
        local_srv_coords.anglex =   0.0f;
        local_srv_coords.angley =   0.0f;
        local_srv_coords.anglez =   0.0f;

        ASSERT_VX_OBJECT(srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)srv_views_array, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&srv_views));
    }

    /* TODO: Fill array */
    /* Not using for now */
    ASSERT_VX_OBJECT(galign_lut = vxCreateArray(context, VX_TYPE_UINT16, capacity), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(output_image = vxCreateUniformImage(context, out_width, out_height, VX_DF_IMAGE_RGBX, &value), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxGlSrvNode(graph, param_obj, input_array, srv_views_array, NULL, output_image), VX_TYPE_NODE);
    vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    tivxTaskWaitMsecs(500);

    //ct_output_ref = ct_read_image("psdkra/srv/standalone/output_reference_2mp.bmp", 4);

    //ct_output_write = ct_image_from_vx_image(output_image);

    //ct_write_image("output/output_reference_2mp.bmp", ct_output_write);

    // TODO: Fix reference image check
    //ASSERT_EQ_CTIMAGE(ct_output_ref, ct_output_write);
    //ASSERT_CTIMAGE_NEARWRAP(ct_output_ref, ct_output_write, 1, 0);

    VX_CALL(vxReleaseUserDataObject(&param_obj));
    VX_CALL(vxReleaseObjectArray(&input_array));
    VX_CALL(vxReleaseObjectArray(&srv_views_array));
    VX_CALL(vxReleaseArray(&galign_lut));
    VX_CALL(vxReleaseImage(&output_image));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxSrvUnLoadKernels(context);
}

TEST(tivxGlSrv, testNullSrvViews)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node node;
    vx_object_array input_array;
    vx_image img_exemplar, output_image;
    vx_user_data_object param_obj;
    vx_uint32 i, in_width, in_height, out_width, out_height, num_cameras;
    vx_status status;
    char filename[MAX_ABS_FILENAME];
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    //CT_Image ct_output_ref, ct_output_tst, ct_output_write;
    vx_pixel_value_t value = {{ 0 }};
    in_width = 1280;
    in_height = 720;
    out_width = 1920;
    out_height = 1080;
    num_cameras = 4;

    tivxSrvLoadKernels(context);

    memset(&params, 0, sizeof(tivx_srv_params_t));

#ifdef x86_64
    params.cam_bpp = 24;
#else
    params.cam_bpp = 12;
#endif
    ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(img_exemplar = vxCreateImage(context, in_width, in_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(input_array = vxCreateObjectArray(context, (vx_reference)img_exemplar, num_cameras), VX_TYPE_OBJECT_ARRAY);

    load_input_images(context, input_array, in_width, in_height, num_cameras);

    VX_CALL(vxReleaseImage(&img_exemplar));

    ASSERT_VX_OBJECT(output_image = vxCreateUniformImage(context, out_width, out_height, VX_DF_IMAGE_RGBX, &value), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxGlSrvNode(graph, param_obj, input_array, NULL, NULL, output_image), VX_TYPE_NODE);
    vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    //ct_output_ref = ct_read_image("psdkra/srv/standalone/output_reference_null_views.bmp", 4);

    //ct_output_write = ct_image_from_vx_image(output_image);

    // TODO: Fix reference image check
    //ASSERT_EQ_CTIMAGE(ct_output_ref, ct_output_write);
    //ASSERT_CTIMAGE_NEARWRAP(ct_output_ref, ct_output_write, 1, 0);
    VX_CALL(vxReleaseUserDataObject(&param_obj));
    VX_CALL(vxReleaseObjectArray(&input_array));
    VX_CALL(vxReleaseImage(&output_image));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxSrvUnLoadKernels(context);
}

TESTCASE_TESTS(tivxGlSrv,
               testGraph1MP,
               testGraph1MPViewChange,
               testGraph2MP,
               testNullSrvViews)

