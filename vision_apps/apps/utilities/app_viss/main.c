/*
 *
 * Copyright (c) 2018-2020 Texas Instruments Incorporated
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
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_host_utils.h"
#include <app_init.h>

#include <TI/j7_tidl.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>
#include <utils/iss/include/app_iss.h>
#include <utils/mem/include/app_mem.h>


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(LINUX) || defined(QNX)
#include <sys/stat.h>
#endif
#include <float.h>
#include <math.h>
#include <unistd.h>

#define APP_MAX_FILE_PATH           (512u)

static char *get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static char * ct_alloc_mem (uint32_t size)
{
    return (appMemAlloc(APP_MEM_HEAP_DDR, size, 0));
}

static void ct_free_mem(char *buf, uint32_t size)
{
    appMemFree(APP_MEM_HEAP_DDR, buf, size);
}

static int32_t ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack, uint16_t downshift_bits)
{
    int32_t status = 0;
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[APP_MAX_FILE_PATH];
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if (!fileName)
    {
        printf("Image file name not specified\n");
        return -1;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, APP_MAX_FILE_PATH, "%s/%s", test_data_path, fileName);

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Cannot open File %s\n", file);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        if (NULL != buf)
        {
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
                        return -1;
                    }
                }
                tivxUnmapRawImagePatch(image, map_id);
            }
        }
    }

    ct_free_mem(buf, sz);
    fclose(f);

    return (status);
}

void TestGraProcessingDcc(int32_t load, int32_t time)
{
    vx_context context;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status, iter, sleep_iter, load_iter;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    uint64_t start, end;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    /* Note: these are don't care's given num_exposures = 1, but
     * fixing to satisfy static analysis issues */
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height, i;

        /* Create OpenVx Context */
        context = vxCreateContext();
        if (NULL == context)
        {
            printf ("Could not create context \n");
            return ;
        }

        tivxHwaLoadKernels(context);

        raw = tivxCreateRawImage(context, &raw_params);

        tivxQueryRawImage(raw,
            TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width));
        tivxQueryRawImage(raw,
            TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height));

        // Note: image is non-zero but not validated
        y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);

        params.sensor_dcc_id = sensor_dcc_id;
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;

        configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
            sizeof(tivx_vpac_viss_params_t), &params);

        /* Create/Configure ae_awb_params input structure */
        tivx_ae_awb_params_init(&ae_awb_params);

        ae_awb_params.ae_valid = 1;
        ae_awb_params.exposure_time = 16666;
        ae_awb_params.analog_gain = 1030;
        ae_awb_params.awb_valid = 1;
        ae_awb_params.color_temperature = 3000;
        for (i=0; i<4; i++)
        {
            ae_awb_params.wb_gains[i] = 525;
            ae_awb_params.wb_offsets[i] = 2;
        }

        ae_awb_result = vxCreateUserDataObject(context,
            "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), &ae_awb_params);

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);
        if (0 == dcc_buff_size)
        {
            printf ("Incorrect DCC Buf size \n");
            return ;
        }

        dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL);

        vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0);
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        if (0 != dcc_status)
        {
            printf ("GetDCCBuf failed\n");
            vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id);
            return ;
        }

        vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id);
        /* Done w/ DCC */

        /* Creating H3A output */
        h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);

        if(NULL != h3a_aew_af)
        {
            vxMapUserDataObject(h3a_aew_af,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0);

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id);
        }

        graph = vxCreateGraph(context);

        node = tivxVpacVissNode(graph, configuration, ae_awb_result, dcc_param_viss,
            raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
            h3a_aew_af, histogram, NULL, NULL);

        vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1);

        ct_read_raw_image(raw, file_name, 2, downshift_bits);

        vxVerifyGraph(graph);

        start = tivxPlatformGetTimeInUsecs();
        for (iter = 0; iter < time; iter++) {
            for (load_iter = 0; load_iter < (250 * load / 100); load_iter++)
               vxProcessGraph(graph);
            for (sleep_iter = 0; sleep_iter < (250 * (100 - load) / 100); sleep_iter++)
                usleep(4000);
        }

        end = tivxPlatformGetTimeInUsecs();
        printf ("Processing time for Resolution %dx%d is Time %d.%dms\n",
            width, height, (uint32_t)(end-start)/1000,(uint32_t)(end-start)%1000);


        vxReleaseNode(&node);
        vxReleaseGraph(&graph);
        vxReleaseImage(&y8_r8_c2);
        tivxReleaseRawImage(&raw);
        vxReleaseUserDataObject(&configuration);
        vxReleaseUserDataObject(&ae_awb_result);

        vxReleaseUserDataObject(&h3a_aew_af);
        vxReleaseUserDataObject(&dcc_param_viss);

        tivxHwaUnLoadKernels(context);

        vxReleaseContext(&context);
    }
}


int main(int argc, char *argv[])
{
    int32_t status = 0, load, time;

    printf("The number of args is %d\n", argc);

    printf("The arguments are %s and %s\n", argv[1], argv[2]);

    load = atoi(argv[1]);
    time = atoi(argv[2]);

    if (load >= 0 && load <= 100)
    {
        if (time > 0 && time <= 0x7FFFFFFF)
        {
            appInit();

            TestGraProcessingDcc(load, time);

            appDeInit();
        }
        else
        {
            status = -1;
            printf("Invalid parameter: time = %d; 'time' should be a positive integer\n", time);
        }
    }
    else
    {
        status = -1;
        printf("Invalid parameter: load = %d; 'load' should be between 0 and 100\n", load);
    }

    return status;
}
