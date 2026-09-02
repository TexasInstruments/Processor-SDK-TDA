/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <TI/tivx.h>
#include <app_init.h>

#include <TI/video_io_kernels.h>
#include <TI/video_io_capture.h>
#include <TI/video_io_display.h>
#include <TI/hwa_vpac_msc.h>

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/iss/include/app_iss.h>

#include <utils/perf_stats/include/app_perf_stats.h>


// #define CAPTURE_ONLY_MODE


static const vx_char user_data_object_name[] = "tivx_capture_params_t";

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static uint32_t initSensorParams(uint32_t sensor_features_supported)
{
    uint32_t sensor_features_enabled = 0;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
    }else
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
    }

    return sensor_features_enabled;
}

static vx_int32 write_output_image_raw(char * file_name, tivx_raw_image raw_image)
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
        printf("Unable to open file %s\n", file_name);
        return -1;
    }

    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

    printf("in width =  %d\n", width);
    printf("in height =  %d\n", height);
    printf("in format =  %d\n", format.pixel_container);

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
        printf("data_ptr is NULL \n");
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
    printf("%d bytes written to %s\n", num_bytes_written_to_file, file_name);
    return num_bytes_written_to_file;
}

#if !defined (CAPTURE_ONLY_MODE)
static void write_output_image_yuv(vx_image img, char * fname, char mode[])
{
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t image_addr;
    vx_map_id map_id;
    void * data_ptr;
    vx_uint32  img_width;
    vx_uint32  img_height;
    vx_uint32  img_format;
    vx_uint32  num_bytes = 0;
    vx_uint32  y_bytes = 1;
    vx_uint32  j;
    char *file_name = fname;
    FILE * fp = fopen(file_name, mode);

    if (!fp)
    {
        printf ("File Open Failed \n");
        return;
    }
    vxQueryImage(img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));

    if (img_format == VX_DF_IMAGE_UYVY || img_format == VX_DF_IMAGE_YUYV)
    {
        y_bytes = 2;
    }

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = img_width;
    rect.end_y = img_height;
    vxMapImagePatch(img,
                    &rect,
                    0,
                    &map_id,
                    &image_addr,
                    &data_ptr,
                    VX_READ_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    VX_NOGAP_X);

    /* Copy Luma */
    for (j = 0; j < img_height; j++)
    {
        num_bytes += fwrite(data_ptr, y_bytes, img_width, fp);
        data_ptr += image_addr.stride_y;
    }
    vxUnmapImagePatch(img, map_id);

    if(img_format == VX_DF_IMAGE_NV12)
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height / 2;
        vxMapImagePatch(img,
                        &rect,
                        1,
                        &map_id,
                        &image_addr,
                        &data_ptr,
                        VX_READ_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        VX_NOGAP_X);


        /* Copy CbCr */
        for (j = 0; j < img_height/2; j++)
        {
            num_bytes += fwrite(data_ptr, 1, img_width, fp);
            data_ptr += image_addr.stride_y;
        }

        vxUnmapImagePatch(img, map_id);
    }
    fflush(fp);
    fclose(fp);
    printf("%d bytes written to %s\n", num_bytes, file_name);
}
#endif

static void testGraphProcessing(int wflag, int port_idx[2], int sensor_idx[2], int wb[8])
{
    int32_t                  status;
    int                      nn= 2;

    /* Graph objects */
    vx_context               context;
    vx_graph                 graph = NULL;
    vx_node                  node_capture[2];
#if !defined (CAPTURE_ONLY_MODE)
    vx_node                  node_viss[2];
#endif
    vx_object_array          capture_frames[2][8];
    vx_user_data_object      h3a_aew_af[2] = {NULL, NULL};

    /* Sensor Parameters */
    uint32_t                 sensor_ch_mask[2];
    IssSensor_CreateParams   sensorParams[2];
    char                     sensor_name[2][ISS_SENSORS_MAX_NAME+16];
#if !defined (CAPTURE_ONLY_MODE)
    int                      width[2], height[2], dcc_id[2];
#endif

    /* Setting to num buf of capture node */
    uint32_t                 num_buf = 3;

#if !defined (CAPTURE_ONLY_MODE)
    /* VISS */
    tivx_raw_image           raw_viss[2];
    vx_image                 yuv_viss[2];
    vx_user_data_object      dcc_config[2] = {NULL, NULL};
    vx_user_data_object      ae_awb_result[2];

    /* MSC */
    vx_user_data_object      msc_coeff_obj[2];
    vx_node                  node_scaler[2];
    vx_image                 yuv_scaler[2];
    uint16_t                 scaler_out_w[2];
    uint16_t                 scaler_out_h[2];

    /* Display */
    tivx_display_params_t    display_params[2];
    vx_user_data_object      display_param_obj[2];
    vx_node                  node_display[2];
#endif

    /* loop count */
    uint32_t                 loop_cnt = 30*3;
    if (wflag > loop_cnt)
    {
        loop_cnt = (wflag>>2) << 2;
    }

    /* Create OpenVx Context */
    context = vxCreateContext();
    status = vxGetStatus((vx_reference)context);
    if (0 != status)
    {
        printf ("Create Context Failed \n");
        return;
    }

    tivxVideoIOLoadKernels(context);
    tivxHwaLoadKernels(context);

    if (port_idx[1] < 0)
    {
        nn = 1;
    }

    graph = vxCreateGraph(context);
    status = vxGetStatus((vx_reference)graph);
    if (0 != status)
    {
        printf ("Create Graph Failed \n");
        return;
    }

    /* Sensor Initialization */
    {
        char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
        char  availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];

        vx_uint8 num_sensors_found;
        memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
        for (int i=0; i<ISS_SENSORS_MAX_SUPPORTED_SENSOR; i++)
        {
            sensor_list[i] = availableSensorNames[i];
        }

        // This is a problem: if not calling, DES is not initilized
        appEnumerateImageSensor(sensor_list, &num_sensors_found);

        // up to 2 sensors
        for (int ci = 0; ci < nn; ci++)
        {
            uint32_t features_enabled = 0, features_supported = 0;
            if (port_idx[ci] >= 0)
            {
                /* Note: 0 for IMX390 RCM */
                strncpy(sensor_name[ci], sensor_list[sensor_idx[ci]], ISS_SENSORS_MAX_NAME);

                sensor_ch_mask[ci] = 1 << port_idx[ci];

                memset(&sensorParams[ci], 0, sizeof(sensorParams[ci]));
                status = appQueryImageSensor(sensor_name[ci], &sensorParams[ci]);
                printf("\n\nquery sensor-%d: status = %d, port_idx = %d\n\n", ci, status, port_idx[ci]);

#if !defined (CAPTURE_ONLY_MODE)
                width[ci] = sensorParams[ci].sensorInfo.raw_params.width;
                height[ci] = sensorParams[ci].sensorInfo.raw_params.height;
                dcc_id[ci] = sensorParams[ci].dccId;
#endif

                features_supported = sensorParams[ci].sensorInfo.features;
                features_enabled = initSensorParams(features_supported);

                status = appInitImageSensor(sensor_name[ci], features_enabled, sensor_ch_mask[ci]);
                printf("\n\ninit sensor-%d: status = %d, ch_mask = %d\n\n", ci, status, sensor_ch_mask[ci]);
            }
        }
    }

#if !defined (CAPTURE_ONLY_MODE)
    /* fixed 2A out */
    for (int ci = 0; ci < nn; ci++)
    {
        tivx_ae_awb_params_t ae_awb_params;
        tivx_ae_awb_params_init(&ae_awb_params);
        ae_awb_params.ae_valid = 1;
        ae_awb_params.awb_valid = 1;
        ae_awb_params.exposure_time = 10000;
        ae_awb_params.analog_gain = 1024;
        ae_awb_params.color_temperature = 3725;
        for (int i=0; i<4; i++)
        {
            ae_awb_params.wb_gains[i] = wb[ci *4 + i];
            printf("[%d] = %d\n", i, ae_awb_params.wb_gains[i]);
            ae_awb_params.wb_offsets[i] = 0;
        }
        ae_awb_result[ci] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), &ae_awb_params);
    }
#endif

    /* Creating objects for graph */
    for (int ci = 0; ci < nn; ci++)
    {
        int                      ch_per_inst = 4;
        vx_user_data_object      capture_config;
        tivx_capture_params_t    local_capture_config;
        tivx_raw_image           raw_image = tivxCreateRawImage(context, &(sensorParams[ci].sensorInfo.raw_params));

        /* capture node */
        /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
        for (int buf_id=0; buf_id<num_buf; buf_id++)
        {
            capture_frames[ci][buf_id] = vxCreateObjectArray(context, (vx_reference)raw_image, 1);  // 1 per Des
        }
        h3a_aew_af[ci] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);
        local_capture_config.timeout        = 33;
        local_capture_config.timeoutInitial = 500;
#if defined(SOC_J784S4) || defined(SOC_J742S2)
        local_capture_config.numInst  = 3U;/* Configure three instances */
#else
        local_capture_config.numInst  = 2U;/* Configure both instances */
#endif
        local_capture_config.numCh          = 1;                    // 1x camera per Des

        local_capture_config.instId[0]                       = ci;
        local_capture_config.instCfg[0].enableCsiv2p0Support = (uint32_t)vx_true_e;
        local_capture_config.instCfg[0].numDataLanes         = sensorParams[ci].sensorInfo.numDataLanes;
        local_capture_config.instCfg[0].laneBandSpeed        = sensorParams[ci].sensorInfo.csi_laneBandSpeed;

        for (int lane = 0; lane < local_capture_config.instCfg[0].numDataLanes; lane++)
        {
            local_capture_config.instCfg[0].dataLanesMap[lane] = lane + 1;
        }

        local_capture_config.chVcNum[0]   = port_idx[ci] % ch_per_inst;
        local_capture_config.chInstMap[0] = port_idx[ci] / ch_per_inst;

        capture_config = vxCreateUserDataObject(context, user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config);

        printf("\n\nto create capture node %d\n\n", ci); 
        node_capture[ci] = tivxCaptureNode(graph, capture_config, capture_frames[ci][0]);

#if !defined (CAPTURE_ONLY_MODE)
        /* Create DCC data for viss node input */
        {
            int32_t dcc_buff_size = 0;

            dcc_buff_size = appIssGetDCCSizeVISS(sensor_name[ci], 1);

            dcc_config[ci] = vxCreateUserDataObject(context, "dcc_viss", dcc_buff_size, NULL);
            status = vxGetStatus((vx_reference)dcc_config[ci]);

            if (status == VX_SUCCESS)
            {
                vx_map_id dcc_buf_map_id;
                uint8_t *dcc_buf;

                vxMapUserDataObject(
                    dcc_config[ci],
                    0,
                    dcc_buff_size,
                    &dcc_buf_map_id,
                    (void **)&dcc_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST, 0);

                status = appIssGetDCCBuffVISS(sensor_name[ci], 1, dcc_buf, dcc_buff_size);
                if(status != VX_SUCCESS)
                {
                    printf("Couldn't get VISS DCC buffer from sensor driver \n");
                }

                vxUnmapUserDataObject(dcc_config[ci], dcc_buf_map_id);
            }
            else
            {
                printf("Unable to create DCC config object! \n");
            }
        }

        //add VISS node here
        printf("\n\nto create viss node %d\n\n", ci);
        {
            tivx_vpac_viss_params_t params;
            tivx_vpac_viss_params_init(&params);

            params.sensor_dcc_id = dcc_id[ci];
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
            params.enable_ctx = 1;

#if defined(VPAC3) || defined(VPAC3L)
            params.bypass_cac = 1;
#endif
#if defined(VPAC3L)
            params.bypass_pcid = 1;
#endif

            vx_user_data_object viss_config;
            viss_config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &params);

            raw_viss[ci] = (tivx_raw_image) vxGetObjectArrayItem(capture_frames[ci][0], 0);
            yuv_viss[ci] = vxCreateImage(context, width[ci], height[ci], VX_DF_IMAGE_NV12);
            node_viss[ci] = tivxVpacVissNode(
                graph,
                viss_config,
                ae_awb_result[ci],
                dcc_config[ci],      //dcc_config,
                raw_viss[ci],        //raw
                NULL,                // y12
                NULL,                // uv12_c1
                yuv_viss[ci],        // y8_r8_c2
                NULL,                // uv8_g8_c3
                NULL,                // s8_b8_c4
                h3a_aew_af[ci],      // h3a_aew_af
                NULL,                // histogram
                NULL,                // histogram for FCPB
                NULL);               // raw_hist

            vxSetNodeTarget(node_viss[ci], VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1);
            tivxSetNodeParameterNumBufByIndex(node_viss[ci], 6u, 3);

            vxReleaseUserDataObject(&capture_config);
            vxReleaseUserDataObject(&viss_config);
            tivxReleaseRawImage(&raw_image);
        }
#endif
    }

#if !defined (CAPTURE_ONLY_MODE)
    /* MSC */
    for (int ci = 0; ci < nn; ci++)
    {
        printf("\n\nto create msc node %d\n\n", ci);
        appIssGetResizeParams(width[ci], height[ci], 1920/2, 1080/2, &scaler_out_w[ci], &scaler_out_h[ci]);
        yuv_scaler[ci] = vxCreateImage(context, scaler_out_w[ci], scaler_out_h[ci], VX_DF_IMAGE_NV12);
        node_scaler[ci] = tivxVpacMscScaleNode(graph, yuv_viss[ci], yuv_scaler[ci], NULL, NULL, NULL, NULL);
        vxSetNodeTarget(node_scaler[ci], VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);
        tivxSetNodeParameterNumBufByIndex(node_scaler[ci], 1u, 3);
    }

    /* Display */
    for (int ci = 0; ci < nn; ci++)
    {
        printf("\n\nto create display node %d\n\n", ci);
        memset(&display_params[ci], 0, sizeof(tivx_display_params_t));
        display_params[ci].opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params[ci].pipeId = ci;
        display_params[ci].outWidth = scaler_out_w[ci];
        display_params[ci].outHeight = scaler_out_h[ci];
        display_params[ci].posX = 1920/2 * ci;
        display_params[ci].posY = 0;
        display_param_obj[ci] = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &display_params[ci]);
        node_display[ci] = tivxDisplayNode(graph, display_param_obj[ci], yuv_scaler[ci]);
    }
#endif

    /* Pipelining and graph verification */
    vx_graph_parameter_queue_params_t   graph_parameters_queue_params_list[2];
    for (int ci = 0; ci < nn; ci++)
    {
        /* input @ node index 0, becomes graph parameter 1 */
        add_graph_parameter_by_node_index(graph, node_capture[ci], 1);

        /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
        graph_parameters_queue_params_list[ci].graph_parameter_index = ci;
        graph_parameters_queue_params_list[ci].refs_list_size = num_buf;
        graph_parameters_queue_params_list[ci].refs_list = (vx_reference*)&capture_frames[ci][0];
        vxSetNodeTarget(node_capture[ci], VX_TARGET_STRING, TIVX_TARGET_CAPTURE1);
    }

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
                             VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                             nn,
                             graph_parameters_queue_params_list);
    vxVerifyGraph(graph);

#if !defined (CAPTURE_ONLY_MODE)
    /* MSC setup */
    for (int ci = 0; ci < nn; ci++)
    {
        tivx_vpac_msc_coefficients_t sc_coeffs;
        vx_reference refs[1];
        tivx_vpac_msc_coefficients_params_init(&sc_coeffs, VX_INTERPOLATION_BILINEAR);
        msc_coeff_obj[ci] = vxCreateUserDataObject(context, "tivx_vpac_msc_coefficients_t", sizeof(tivx_vpac_msc_coefficients_t), NULL);
        if(status == VX_SUCCESS)
        {
            status = vxCopyUserDataObject(msc_coeff_obj[ci], 0, sizeof(tivx_vpac_msc_coefficients_t), &sc_coeffs, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
        refs[0] = (vx_reference)msc_coeff_obj[ci];
        if(status == VX_SUCCESS)
        {
            status = tivxNodeSendCommand(node_scaler[ci], 0u, TIVX_VPAC_MSC_CMD_SET_COEFF, refs, 1u);
        }
    }
#endif

    /* start sensor */
    for (int ci = 0; ci < nn; ci++)
    {
        status = appStartImageSensor(sensor_name[ci], sensor_ch_mask[ci]);
        printf("\n\nstart sensor-%d: status = %d, ch_mask = %d\n\n", ci, status, sensor_ch_mask[ci]);
    }

    /* enqueue buf for pipeup but dont trigger graph execution */
    /* after pipeup, now enqueue a buffer to trigger graph scheduling */
    for(int buf_id=0; buf_id<num_buf; buf_id++)
    {
        for (int ci = 0; ci < nn; ci++)
        {
            vxGraphParameterEnqueueReadyRef(graph, ci, (vx_reference*)&capture_frames[ci][buf_id], 1);
        }
    }

    app_perf_point_t perf_point;
    appPerfPointReset(&perf_point);
    
    /* wait for graph instances to complete, compare output and recycle data buffers, schedule again */
    for (int loop_id = 0; loop_id < loop_cnt + num_buf; loop_id++)
    {
        uint32_t        num_refs;
        vx_object_array out_capture_frames[2];
        static int      cnt[2] = {0, 0};

        for (int ci = 0; ci < nn; ci++)
        {
            appPerfPointBegin(&perf_point);

            /* Get output reference, waits until a reference is available */
            vxGraphParameterDequeueDoneRef(graph, ci, (vx_reference*)&out_capture_frames[ci], 1, &num_refs);

            cnt[ci]++;
            if ((0x01 & wflag) && (cnt[ci] == 30))
            {
                tivx_raw_image  f = (tivx_raw_image) vxGetObjectArrayItem(out_capture_frames[ci], 0);
                char * fnames[2] = {"img_csi0.raw", "img_csi1.raw"};
                write_output_image_raw(fnames[ci], f);
                tivxReleaseRawImage(&f);
            }
#if !defined (CAPTURE_ONLY_MODE)
            if ((0x02 & wflag) && (cnt[ci] == 60))
            {
                char * fnames[2] = {"img_csi0.yuv", "img_csi1.yuv"};
                write_output_image_yuv(yuv_viss[ci], fnames[ci], "wb");
            }
#endif
            /* The 30 FPS sensor parameters of IMX390 is around 32.4 ms latency in capture */
            vxGraphParameterEnqueueReadyRef(graph, ci, (vx_reference*)&out_capture_frames[ci], 1);

            appPerfPointEnd(&perf_point);
        }
    }

    printf("\n\nFPS for %s: \n", sensor_name[0]);
    appPerfPointPrintFPS(&perf_point);

    /* ensure all graph processing is complete */
    vxWaitGraph(graph);

    /* Dequeue all buffers */
    for (int ci = 0; ci < nn; ci++)
    {
        uint32_t        out_num_refs;
        vx_bool         done = vx_false_e;
        vx_object_array dequeue_capture_array;
        while (!done)
        {
            vxGraphParameterCheckDoneRef(graph, ci, &out_num_refs);

            if (out_num_refs > 0)
            {
                vxGraphParameterDequeueDoneRef(graph, ci, (vx_reference*)&dequeue_capture_array, 1, &out_num_refs);
            }

            if (out_num_refs == 0)
            {
                done = vx_true_e;
            }
        }
    }

    for (int ci = 0; ci < nn; ci++)
    {
        appStopImageSensor(sensor_name[ci], sensor_ch_mask[ci]);
        appDeInitImageSensor(sensor_name[ci]);
    }

    for (int ci = 0; ci < nn; ci++)
    {
#if !defined (CAPTURE_ONLY_MODE)
        tivxReleaseRawImage(&raw_viss[ci]);
        vxReleaseImage(&yuv_viss[ci]);
        vxReleaseUserDataObject(&dcc_config[ci]);
        vxReleaseUserDataObject(&ae_awb_result[ci]);
        vxReleaseNode(&node_viss[ci]);
#endif
        vxReleaseNode(&node_capture[ci]);
        vxReleaseUserDataObject(&h3a_aew_af[ci]);
    }

#if !defined (CAPTURE_ONLY_MODE)
    /* MSC */
    for (int ci = 0; ci < nn; ci++)
    {
        vxReleaseImage(&yuv_scaler[ci]);
        vxReleaseUserDataObject(&msc_coeff_obj[ci]);
        vxReleaseNode(&node_scaler[ci]);
    }
    
    /* Display */
    for (int ci = 0; ci < nn; ci++)
    {
        vxReleaseUserDataObject(&display_param_obj[ci]);
        vxReleaseNode(&node_display[ci]);
    }
#endif

    for (int ci = 0; ci < nn; ci++)
    {
        for (int buf_id=0; buf_id<num_buf; buf_id++)
        {
            vxReleaseObjectArray(&capture_frames[ci][buf_id]);
        }
    }

    vxReleaseGraph(&graph);
    tivxVideoIOUnLoadKernels(context);
    tivxHwaUnLoadKernels(context);
    vxReleaseContext(&context);
}

int main(int argc, char *argv[])
{
    int status = 0, wflag = 0;
    int sensor_idx[2] = {-1, -1};
    int port_idx[2]   = {-1, -1};
    int wb[8] = {512, 512, 512, 512,       512, 512, 512, 512};

    if ((argc != 4) && (argc != 6) && (argc != 14))
    {
       printf("\n usage: app_capture wflag port1_idx sensor1_idx [port2_idx sensor2_idx [wb1 wb2 wb3 wb4 wb5 wb6 wb7 wb8]]\n");
       return 0;
    }

    wflag = atoi(argv[1]);
    port_idx[0] = atoi(argv[2]);
    sensor_idx[0] = atoi(argv[3]);

    if (argc >= 6)
    {
        port_idx[1] = atoi(argv[4]);
        sensor_idx[1] = atoi(argv[5]);
    }

    if (argc == 14)
    {
        for (int i = 0; i < 8; i++)
        {
            wb[i] = atoi(argv[6+i]);
        }
    }

    status = appInit();

    if(status==0)
    {
        testGraphProcessing(wflag, port_idx, sensor_idx, wb);

        appDeInit();
    }
}
