/*
 *  Copyright (c) Texas Instruments Incorporated 2018
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(LINUX) || defined(QNX)
#include <sys/stat.h>
#endif

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/tivx_sample.h>
#include <tivx_sample_kernels_priv.h>
#include <app_init.h>
#include <tivx_utils_file_rd_wr.h>
#include <TI/tivx_task.h>

#define VID_WIDTH                 (640)
#define VID_HEIGHT                (480)

#define OUTPUT_WIDTH              (1920)
#define OUTPUT_HEIGHT             (1080)

#define APP_MAX_FILE_PATH         (256u)

#define NUM_IMAGES                (4U)

static void StartupEmulatorWaitFxn (void)
{
    volatile uint32_t enableDebug = 0;
    do
    {
    }while (enableDebug);
}

static char *get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS) || defined(FREERTOS) || defined(SAFERTOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
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
        if(status == VX_SUCCESS)
        {
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
        }
        if(status == VX_SUCCESS)
        {
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
        }
        if(status==VX_SUCCESS)
        {
            FILE *fp = fopen(filename,"rb+a");

            if(fp!=NULL)
            {
                vx_int32 j;

                num_bytes = 0;
                for (j = 0; j < height; j++)
                {
                    num_bytes += fread(data_ptr1, 1, width, fp);
                    data_ptr1 += image_addr1.stride_y;
                }

                if(num_bytes != (width*height))
                {
                    printf("Luma bytes read = %d, expected = %d\n", num_bytes, width*height);
                    status = VX_FAILURE;
                }

                num_bytes = 0;
                for (j = 0; j < height/2; j++)
                {
                    num_bytes += fread(data_ptr2, 1, width, fp);
                    data_ptr2 += image_addr2.stride_y;
                }

                if(num_bytes != (width*height/2))
                {
                    printf("CbCr bytes read = %d, expected = %d\n", num_bytes, width*height/2);
                    status = VX_FAILURE;
                }

                fclose(fp);
            }
            else
            {
                 printf("app_linux_arm_opengl_mosaic: ERROR: Unable to open file for reading [%s]\n", filename);
            }
            vxUnmapImagePatch(image, map_id1);
            vxUnmapImagePatch(image, map_id2);
        }
    }
    else
    {
        printf("app_linux_arm_opengl_mosaic: ERROR: Invalid image specified for reading\n");
    }
    return status;
}

static vx_status app_save_vximage_rgbx_to_bin_file(char *file_name, vx_image image)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr1;
    vx_rectangle_t rect;
    vx_map_id map_id1;
    vx_df_image df;
    void *data_ptr1;
    vx_status status;

    status = vxGetStatus((vx_reference)image);
    if(status==VX_SUCCESS)
    {
        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        printf("app_linux_arm_opengl_mosaic: writing to file [%s] image of size %d x %d\n",
            file_name,
            width,
            height
            );

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;
        if(status == VX_SUCCESS)
        {
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
        }
        if(status==VX_SUCCESS)
        {
            FILE *fp = fopen(file_name,"wb");

            if(fp!=NULL)
            {
                uint32_t x, y;
                uint8_t *cur_ptr;
                size_t ret = 0;

                for(y=0; y<height; y++)
                {
                    cur_ptr = (uint8_t *)data_ptr1 + image_addr1.stride_y*y;

                    for(x=0; x<width; x++)
                    {
                        /* only write RGB, dont write the A value */
                        ret = fwrite(cur_ptr, 3, 1, fp);
                        if(ret!=1)
                        {
                            printf("app_linux_arm_opengl_mosaic: ERROR: Unable to write data to file [%s]\n", file_name);
                            status = VX_FAILURE;
                            break;
                        }
                        cur_ptr += 4; /* skip over a RGBA pixel */
                    }
                    if(ret!=1)
                    {
                        status = VX_FAILURE;
                        break;
                    }
                }
                fclose(fp);

                printf("app_linux_arm_opengl_mosaic: writing to file [%s] image of size %d x %d ... Done !!!\n",
                    file_name,
                    width,
                    height
                    );
            }
            else
            {
                printf("app_linux_arm_opengl_mosaic: ERROR: Unable to open file for writing [%s]\n", file_name);
                status = VX_FAILURE;
            }
            vxUnmapImagePatch(image, map_id1);
        }
    }
    else
    {
        printf("app_linux_arm_opengl_mosaic: ERROR: Invalid image specified for writing\n");
    }

    return status;
}

int main(int argc, char *argv[])
{
    int32_t i, status = VX_SUCCESS;
    int32_t inputWidth, inputHeight;
    int32_t outputWidth, outputHeight;
    tivx_opengl_mosaic_params_t params;
    char output_file_path[APP_MAX_FILE_PATH];
    char input1_file_path[APP_MAX_FILE_PATH];
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();
    struct stat s;

    vx_user_data_object param_obj;
    vx_context context;
    vx_graph graph;
    vx_node node;
    vx_object_array in_obj_array;
    vx_image input[NUM_IMAGES], output;

    /* This is for debug purpose - see the description of function header */
    StartupEmulatorWaitFxn();

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

    if (stat(test_data_path, &s))
    {
        printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
        test_data_path = failsafe_test_data_path;
    }

    snprintf(output_file_path, APP_MAX_FILE_PATH, "%s/output/%s", test_data_path, "mosaic_output_file.bin");
    snprintf(input1_file_path, APP_MAX_FILE_PATH, "%s/psdkra/app_opengl_mosaic/%s", test_data_path, "input1_file.bin");

    appInit();

    context = vxCreateContext();
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) context);
    }
    tivxSampleLoadKernels(context);
    #ifndef PC
    tivxRegisterSampleTargetMpuKernels();
    #endif

    graph = vxCreateGraph(context);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) graph);
    }
    inputWidth = VID_WIDTH;
    inputHeight = VID_HEIGHT;

    outputWidth = OUTPUT_WIDTH;
    outputHeight = OUTPUT_HEIGHT;

    memset(&params, 0, sizeof(tivx_opengl_mosaic_params_t));
    params.renderType = TIVX_KERNEL_OPENGL_MOSAIC_TYPE_2x2;

    input[0] = vxCreateImage(context, inputWidth, inputHeight, VX_DF_IMAGE_NV12);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) input[0]);
    }
    in_obj_array = vxCreateObjectArray(context, (vx_reference)input[0], NUM_IMAGES);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) in_obj_array);
    }
    if(status == VX_SUCCESS)
    {
        status = vxReleaseImage(&input[0]);
    }
    if(status == VX_SUCCESS)
    {
        output = vxCreateImage(context, outputWidth, outputHeight, VX_DF_IMAGE_RGBX);
        status = vxGetStatus((vx_reference) output);
    }
    if(status == VX_SUCCESS)
    {
        param_obj = vxCreateUserDataObject(context, "tivx_opengl_mosaic_params_t", sizeof(tivx_opengl_mosaic_params_t), &params);
        status = vxGetStatus((vx_reference) param_obj);
    }
    for(i = 0; i < NUM_IMAGES; i++)
    {
        if(status == VX_SUCCESS)
        {
            input[i] = (vx_image)vxGetObjectArrayItem(in_obj_array, i);
            status = vxGetStatus((vx_reference) input[i]);
        }
        if(status == VX_SUCCESS)
        {
            status = app_load_vximage_from_bin_file(input1_file_path, (vx_image)input[i]);
        }
    }

    if(status == VX_SUCCESS)
    {
        node = tivxOpenglMosaicNode (graph, param_obj, in_obj_array, output);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    }
    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }
    for (i=0; i<1; i++)
    {
        if(status == VX_SUCCESS)
        {
            status = vxScheduleGraph(graph);
        }
        if(status == VX_SUCCESS)
        {
            status = vxWaitGraph(graph);
        }
    }
    if(status == VX_SUCCESS)
    {
        status = app_save_vximage_rgbx_to_bin_file(output_file_path, (vx_image)output);
    }
    for(i = 0; i < NUM_IMAGES; i++)
    {
        if(status == VX_SUCCESS)
        {
            status = vxReleaseImage(&input[i]);
        }
    }

    /* no status checking for these releases */
    vxReleaseObjectArray(&in_obj_array);
    vxReleaseImage(&output);
    vxReleaseUserDataObject(&param_obj);

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    #ifndef PC
    tivxUnRegisterSampleTargetMpuKernels();
    #endif
    tivxSampleUnLoadKernels(context);
    vxReleaseContext(&context);

    appDeInit();
    return status;
}

