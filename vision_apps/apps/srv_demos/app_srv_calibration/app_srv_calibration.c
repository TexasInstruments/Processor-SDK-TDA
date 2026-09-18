
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

#include <TI/tivx.h>
#include <TI/j7_imaging_aewb.h>
#include <TI/tivx_task.h>
#include <tivx_utils_graph_perf.h>
#include <TI/video_io_capture.h>
#include <TI/video_io_display.h>
#include <TI/video_io_kernels.h>

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/iss/include/app_iss.h>
#include <utils/draw2d/include/draw2d.h>

#include <TI/tivx_srv.h>
#include <TI/tivx_img_proc.h>
#include "srv_calibration_applib/srv_calibration_applib.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"

#include <iss_sensors.h>
#include <iss_sensor_if.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#if defined(LINUX) || defined(QNX)
#include <sys/stat.h>
#endif

#include <srv_utils.h>

#define _ENABLE_2A_

#define MAX_NUM_BUF                         (8u)
#define NUM_CAPT_CHANNELS                   (4U)
#define CHANNEL_SWITCH_FRAME_COUNT          (300u)
#define NUM_BUFS                            (4u)
#define CAPT_INST_ID                        (0U)

#define NUM_MAX_CAM 4

#define DISPLAY_WIDTH_2MP                   (1920u)
#define DISPLAY_HEIGHT_2MP                  (1080u)

typedef struct {

    vx_uint32 inWidth;
    vx_uint32 inHeight;
    vx_uint32 outWidth;
    vx_uint32 outHeight;
    float width_scaling_factor;
    float height_scaling_factor;

    /* OpenVX references */
    vx_context context;
    tivx_raw_image sample_raw_img;
    tivx_raw_image raw_img;
    vx_object_array capt_frames[NUM_MAX_CAM];
    vx_user_data_object display_param_obj;
    vx_graph graph;
    vx_graph graph_display;
    vx_node displayNode;
    vx_node captureNode;
    vx_user_data_object capture_param_obj;

    /* VISS Objects */
    vx_image sample_nv12_img;
    vx_image viss_nv12_out_img[MAX_NUM_BUF];
    vx_node vissNode;
    vx_node node_aewb;
    vx_user_data_object viss_configuration;
    vx_distribution histogram;
    vx_object_array histogram_arr;
    vx_object_array ae_awb_result_arr;
    vx_user_data_object ae_awb_result;
    vx_object_array viss_out_frames[MAX_NUM_BUF];
    vx_user_data_object dcc_param_viss;

    /* Aewb objects */
    vx_object_array h3a_aew_af_arr;
    vx_user_data_object h3a_aew_af;
    tivx_aewb_config_t aewb_cfg;
    tivx_ae_awb_params_t ae_awb_params;
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;
    vx_object_array aewb_config_array;

    uint32_t cam_dcc_id;
    char *sensor_name;

    /* SRV Calibration objects */
    vx_graph graph_srv_calib;

    /* Calibration Data */
    srv_calib_handle srv_handle;
    srv_calib_createParams srv_create_params;

    /* OpenVX objects as parameters to applib */
    vx_user_data_object point_detect_in_config;
    vx_user_data_object pose_estimation_in_config;
    vx_user_data_object in_lens_param_object;
    vx_image in;
    vx_image buf_bwluma_frame;
    vx_object_array buf_bwluma_frame_array;
    vx_object_array in_array;
    vx_user_data_object out_calmat;
    vx_object_array in_corners;
    vx_user_data_object corner_element;
    svPointDetect_t in_point_detect_params;
    svACDetectStructFinalCorner_t out_params[NUM_CAPT_CHANNELS];

    /* Mosaic node */
    vx_user_data_object mosaic_config;
    vx_kernel mosaic_kernel;
    vx_node mosaic_node;
    vx_image output_image[MAX_NUM_BUF];
    tivxImgMosaicParams mosaic_params;
    vx_object_array input_arr[TIVX_IMG_MOSAIC_MAX_INPUTS];

    uint32_t is_interactive;
    tivx_task task;
    uint32_t stop_task;
    uint32_t stop_task_done;
    uint32_t write_file;
    uint32_t write_capture_file;
    uint32_t run_calibration;
    uint32_t resume;

    /* Drawing */
    Draw2D_Handle  pHndl;

} SrvCalibAppObj;

SrvCalibAppObj gAppObj;

vx_status copy_objarr_for_calibration(SrvCalibAppObj *obj, vx_object_array out_objarr, vx_uint32 calibrate);
vx_status app_draw_corners(SrvCalibAppObj *obj, vx_image in_display);
vx_status extract_corners(SrvCalibAppObj *obj);
static vx_status app_init(SrvCalibAppObj *obj);
static void app_deinit(SrvCalibAppObj *obj);
static vx_status app_create_graph(SrvCalibAppObj *obj);
static void app_delete_graph(SrvCalibAppObj *obj);
static void app_run_graph(SrvCalibAppObj *obj);
static void app_run_graph_srv_calib(SrvCalibAppObj *obj);
static void app_run_task(void *app_var);
static int32_t app_run_task_create(SrvCalibAppObj *obj);
static vx_status app_run_graph_interactive(SrvCalibAppObj *obj);

static char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
static vx_uint8 num_sensors_found;
static IssSensor_CreateParams sensorParams;

#define NUM_FRAMES_TO_PROCESS 4

/* Extracts calmat data and writes to file */
static void write_calmat(SrvCalibAppObj *obj)
{
    vx_uint32 idx, jdx;
    svACCalmatStruct_t   out_calmat_params;
    svCalmat_t            calmat_out;

    memset(&out_calmat_params, 0, sizeof(svACCalmatStruct_t));

    vxCopyUserDataObject(obj->out_calmat, 0, sizeof(svACCalmatStruct_t), &out_calmat_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    calmat_out.numCameras = NUM_CAPT_CHANNELS;

    for (idx=0;idx<NUM_CAMERAS;idx++) {
        calmat_out.calMatSize[idx] = SRV_CALMAT_SIZE;
        for (jdx=0;jdx<12;jdx++ ) {
          calmat_out.calMatBuf[12*idx + jdx] = out_calmat_params.outcalmat[12*idx + jdx];
        }
    }

    write_calmat_file(&calmat_out, "psdkra/srv/srv_app/CALMAT.BIN");
}

vx_status copy_objarr_for_calibration(SrvCalibAppObj *obj, vx_object_array out_objarr, vx_uint32 calibrate)
{
    vx_status status;
    vx_uint32 start_x, start_y, end_x, end_y, buf_id;
    vx_image src_image, dst_image;

    vx_imagepatch_addressing_t src_image_addr1 = { obj->inWidth, obj->inHeight, 1, obj->inWidth };
    vx_imagepatch_addressing_t src_image_addr2 = { obj->inWidth, obj->inHeight, 2, obj->inWidth };
    vx_rectangle_t src_rect, dst_rect;
    void *src_data_ptr1, *src_data_ptr2;
    vx_imagepatch_addressing_t dst_image_addr1 = { obj->inWidth, obj->inHeight, 1, obj->inWidth };
    vx_imagepatch_addressing_t dst_image_addr2 = { obj->inWidth, obj->inHeight, 2, obj->inWidth };

    start_x = (obj->inWidth-obj->inWidth)/2;
    end_x   = start_x+obj->inWidth;
    start_y = (obj->inHeight-obj->inHeight)/2;
    end_y   = start_y+obj->inHeight;

    src_data_ptr1 = malloc(obj->inWidth*obj->inHeight);
    if (NULL == src_data_ptr1)
    {
        APP_PRINTF("src_data_ptr1 allocation failed!!\n");
        return VX_FAILURE;
    }

    memset(src_data_ptr1, 0, obj->inWidth*obj->inHeight);

    src_data_ptr2 = malloc( (obj->inWidth*obj->inHeight) / 2);
    if (NULL == src_data_ptr2)
    {
        APP_PRINTF("src_data_ptr2 allocation failed!!\n");
        free(src_data_ptr1);
        return VX_FAILURE;
    }

    memset(src_data_ptr2, 0, (obj->inWidth*obj->inHeight) / 2);

    for (buf_id = 0; buf_id < NUM_CAPT_CHANNELS; buf_id++)
    {
        src_image = (vx_image)vxGetObjectArrayItem(out_objarr, buf_id);
        dst_image = (vx_image)vxGetObjectArrayItem(obj->in_array, buf_id);

        src_rect.start_x = start_x;
        src_rect.start_y = start_y;
        src_rect.end_x = end_x;
        src_rect.end_y = end_y;

        dst_rect.start_x = 0;
        dst_rect.start_y = 0;
        dst_rect.end_x = obj->inWidth;
        dst_rect.end_y = obj->inHeight;

        /* Map each and copy */
        status = vxCopyImagePatch(src_image, &src_rect, 0, &src_image_addr1, src_data_ptr1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        APP_PRINTF("status = %d\n", status);

        status = vxCopyImagePatch(src_image, &src_rect, 1, &src_image_addr2, src_data_ptr2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        APP_PRINTF("status = %d\n", status);

        status = vxCopyImagePatch(dst_image, &dst_rect, 0, &dst_image_addr1, (void *)src_data_ptr1, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        APP_PRINTF("status = %d\n", status);

        status = vxCopyImagePatch(dst_image, &dst_rect, 1, &dst_image_addr2, (void *)src_data_ptr2, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        APP_PRINTF("status = %d\n", status);

        vxReleaseImage(&dst_image);
        vxReleaseImage(&src_image);
    }

    free(src_data_ptr1);
    free(src_data_ptr2);

    if ( (VX_SUCCESS == status) && (calibrate) )
    {
        app_run_graph_srv_calib(obj);
        status = extract_corners(obj);
        if ( (VX_SUCCESS == status) && (calibrate) )
        {
            write_calmat(obj);
        }
        else
        {
            printf("Calibration Failed!!! Re-run calibration\n");
        }
    }

    return status;
}

/* Function for drawing corners onto image */
vx_status app_draw_corners(SrvCalibAppObj *obj, vx_image in_display)
{
    vx_status status = VX_SUCCESS;
    Draw2D_RegionPrm sBufRegion;
    int box_size = 8, i = 0;
    uint32_t x_offset = 0, y_offset = 0;

    for (i = 0; i < NUM_CAPT_CHANNELS; i++)
    {
        if (0==i)
        {
            x_offset = 0;
            y_offset = 0;
        }
        else if (1 == i)
        {
            x_offset = obj->outWidth/2;
            y_offset = 0;
        }
        else if (2 == i)
        {
            x_offset = 0;
            y_offset = obj->outHeight/2;
        }
        else if (3 == i)
        {
            x_offset = obj->outWidth/2;
            y_offset = obj->outHeight/2;
        }

        /* Notes: 1. Scaling factor takes care of input->output discrepancy since this is using the input size but scales down on output. *
         *        2. Corners are given by obj->out_params[i].finalCorners[0][1]>>4.  Additional shift is to divide by 2 to separate into quadrants *
         *        3. (box_size/2) gives a starting point so that the rectangle is centered on point.
         *        4. X/Y offset addition is to put it in correct quadrant.
         */
        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[0][1]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[0][0]>>5) - (box_size/2) + y_offset));
        sBufRegion.width  = box_size;
        sBufRegion.height = box_size;
        sBufRegion.color = RGB888_TO_RGB565(0, 255, 255);
        sBufRegion.colorFormat = DRAW2D_DF_YUV420SP_UV;

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[0][3]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[0][2]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[0][5]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[0][4]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[0][7]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[0][6]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[1][1]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[1][0]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[1][3]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[1][2]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[1][5]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[1][4]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);

        sBufRegion.startX = obj->width_scaling_factor  * (((obj->out_params[i].finalCorners[1][7]>>5) - (box_size/2) + x_offset));
        sBufRegion.startY = obj->height_scaling_factor * (((obj->out_params[i].finalCorners[1][6]>>5) - (box_size/2) + y_offset));

        status |= Draw2D_fillRegion(obj->pHndl, &sBufRegion);
    }

    return status;
}

/* Function for extracting corners from calibration data */
vx_status extract_corners(SrvCalibAppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    obj->in_corners = srv_calib_get_corners(obj->srv_handle);

    for (i = 0; i < NUM_CAPT_CHANNELS; i++)
    {
        obj->corner_element = (vx_user_data_object)vxGetObjectArrayItem(obj->in_corners, i);

        vxCopyUserDataObject(obj->corner_element, 0, sizeof(svACDetectStructFinalCorner_t), &obj->out_params[i], VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        APP_PRINTF("Final Corners 1 = %d %d %d %d %d %d %d %d\n",obj->out_params[i].finalCorners[0][0]>>4,
        obj->out_params[i].finalCorners[0][1]>>4,obj->out_params[i].finalCorners[0][2]>>4, obj->out_params[i].finalCorners[0][3]>>4,
        obj->out_params[i].finalCorners[0][4]>>4,obj->out_params[i].finalCorners[0][5]>>4, obj->out_params[i].finalCorners[0][6]>>4,
        obj->out_params[i].finalCorners[0][7]>>4);

        APP_PRINTF("Final Corners 2 = %d %d %d %d %d %d %d %d\n",obj->out_params[i].finalCorners[1][0]>>4,
        obj->out_params[i].finalCorners[1][1]>>4,obj->out_params[i].finalCorners[1][2]>>4, obj->out_params[i].finalCorners[1][3]>>4,
        obj->out_params[i].finalCorners[1][4]>>4,obj->out_params[i].finalCorners[1][5]>>4, obj->out_params[i].finalCorners[1][6]>>4,
        obj->out_params[i].finalCorners[1][7]>>4);
        APP_PRINTF("numFPDetected = %d \n",obj->out_params[i].numFPDetected);

        if (8 != obj->out_params[i].numFPDetected)
        {
            printf("Calibration failed for Camera #%d\n", i);
            status = VX_FAILURE;
        }

        vxReleaseUserDataObject(&obj->corner_element);
    }

    return status;
}

int app_srv_calibration_main(int argc, char* argv[])
{
    SrvCalibAppObj *obj = &gAppObj;
    vx_status status;
    status = app_init(obj);
    APP_PRINTF("app_init done\n");

    if (VX_SUCCESS == status)
    {
        status = app_create_graph(obj);
        if (VX_SUCCESS == status)
        {
            APP_PRINTF("app_create_graph done\n");
            if(obj->is_interactive)
            {
                APP_PRINTF("starting interactive graph\n");
                app_run_graph_interactive(obj);
            }
            else
            {
                APP_PRINTF("starting non-interactive graph\n");
                app_run_graph(obj);
            }
            APP_PRINTF("app_run_graph done\n");

            status = appStopImageSensor(obj->sensor_name, 0xF);/*Mask = 0xF for 4 cameras*/
            app_delete_graph(obj);
            APP_PRINTF("app_delete_graph done\n");
        }
    }
    else
    {
        APP_PRINTF("app_create_graph failed\n");
    }

    app_deinit(obj);
    APP_PRINTF("app_deinit done\n");
    return 0;
}

static vx_status app_init(SrvCalibAppObj *obj)
{
    vx_status status = VX_FAILURE;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 count = 0;
    vx_uint8 selectedSensor = 0xFF;

    obj->stop_task = 0;
    obj->resume = 0;
    obj->stop_task_done = 0;
    obj->is_interactive = 1;
    obj->write_file = 0;
    obj->write_capture_file = 0;
    obj->run_calibration = 0;
    #if defined(x86_64)
    obj->is_interactive = 0;
    #endif

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxSrvLoadKernels(obj->context);
    APP_PRINTF("tivxSrvLoadKernels done\n");

    tivxHwaLoadKernels(obj->context);
    APP_PRINTF("tivxHwaLoadKernels done\n");

    tivxVideoIOLoadKernels(obj->context);
    APP_PRINTF("tivxVideoIOLoadKernels done\n");

    tivxImagingLoadKernels(obj->context);
    APP_PRINTF("tivxImagingLoadKernels done\n");

    memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
    for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
    {
        sensor_list[count] = availableSensorNames[count];
    }
    status = appEnumerateImageSensor(sensor_list, &num_sensors_found);
    if(VX_SUCCESS != status)
    {
        printf("appCreateImageSensor returned %d\n", status);
        return status;
    }
    else
    {
        /* TODO: Add in support for other sensors when available */
        #if 0
        while(selectedSensor > (num_sensors_found-1))
        {
            printf("%d sensor(s) found \n", num_sensors_found);
            printf("Supported sensor list: \n");
            for(count=0;count<num_sensors_found;count++)
            {
               printf("%c : %s \n", count+'a', sensor_list[count]);
            }

            printf("Select a sensor \n");
            ch = getchar();
            printf("\n");
            selectedSensor = ch - 'a';
            if(selectedSensor > (num_sensors_found-1))
            {
                printf("Invalid selection %c. Try again \n", ch);
            }
        }
        #endif
        selectedSensor = 0;
        printf("Calibration only supports IMX390 sensor\n");
    }
    obj->sensor_name = sensor_list[selectedSensor];
    printf("Sensor selected : %s\n", obj->sensor_name);

    return status;
}
static void app_deinit(SrvCalibAppObj *obj)
{
    tivxSrvUnLoadKernels(obj->context);
    APP_PRINTF("tivxSrvUnLoadKernels done\n");

    tivxHwaUnLoadKernels(obj->context);
    APP_PRINTF("tivxHwaUnLoadKernels done\n");

    tivxVideoIOUnLoadKernels(obj->context);
    APP_PRINTF("tivxVideoIOUnLoadKernels done\n");

    tivxImagingUnLoadKernels(obj->context);
    APP_PRINTF("tivxImagingUnLoadKernels done\n");

    vxReleaseContext(&obj->context);
    APP_PRINTF("vxReleaseContext done\n");

    appDeInitImageSensor(obj->sensor_name);
    APP_PRINTF("sensor deinit done\n");
}

/* Creating capture node */
static vx_status app_create_capture(SrvCalibAppObj *obj)
{
        vx_status status = VX_SUCCESS;
        tivx_capture_params_t capture_params;
        vx_uint32 buf_id, loop_id;

        obj->sample_raw_img = tivxCreateRawImage(obj->context, &(sensorParams.sensorInfo.raw_params));
        if (vxGetStatus((vx_reference)obj->sample_raw_img) != VX_SUCCESS)
        {
            APP_PRINTF("sample_raw_img create failed\n");
            return VX_FAILURE;
        }

        /* Allocate frames */
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            obj->capt_frames[buf_id] =
                vxCreateObjectArray(obj->context,
                (vx_reference)obj->sample_raw_img, NUM_CAPT_CHANNELS);
            if (vxGetStatus((vx_reference)obj->capt_frames[buf_id]) != VX_SUCCESS)
            {
                APP_PRINTF("obj->capt_frames[buf_id] create failed\n");
                return VX_FAILURE;
            }
        }
        tivxReleaseRawImage(&obj->sample_raw_img);

        /* Capture initialization */
        tivx_capture_params_init(&capture_params);
        capture_params.numInst                          = 1U;
        capture_params.numCh                            = 4U;
        capture_params.instId[0U]                       = CAPT_INST_ID;
        #if defined(SOC_FAMILY_J7) || defined(SOC_FAMILY_AM)
        capture_params.instCfg[0U].enableCsiv2p0Support = (uint32_t)vx_true_e;
        #elif defined(SOC_FAMILY_TDA5)
        capture_params.instCfg[0U].phyMode              = TIVX_CAPTURE_DPHY_MODE;
        #endif
        capture_params.instCfg[0U].numDataLanes         = 4U;
        for (loop_id=0U; loop_id<capture_params.instCfg[0U].numDataLanes; loop_id++)
        {
            capture_params.instCfg[0U].dataLanesMap[loop_id] = loop_id+1;
        }

        obj->capture_param_obj =
            vxCreateUserDataObject(obj->context, "tivx_capture_params_t" ,
            sizeof(tivx_capture_params_t), &capture_params);
        if (vxGetStatus((vx_reference)obj->capture_param_obj) != VX_SUCCESS)
        {
            APP_PRINTF("capture_param_obj create failed\n");
            return VX_FAILURE;
        }

        obj->captureNode = tivxCaptureNode(obj->graph, obj->capture_param_obj, obj->capt_frames[0]);
        if (vxGetStatus((vx_reference)obj->captureNode) != VX_SUCCESS)
        {
            APP_PRINTF("captureNode create failed\n");
            return VX_FAILURE;
        }

        vxSetNodeTarget(obj->captureNode, VX_TARGET_STRING,
            TIVX_TARGET_CAPTURE1);
        vxSetReferenceName((vx_reference)obj->captureNode, "Capture_node");

        return status;
}

/* Creating viss and aewb nodes */
static vx_status app_create_viss_aewb(SrvCalibAppObj *obj)
{
        vx_status status = VX_SUCCESS;
        tivx_vpac_viss_params_t viss_params;
        tivx_ae_awb_params_t ae_awb_params;
        vx_uint32 buf_id, loop_id;

        /* Sensor Params */
        uint32_t sensor_features_enabled = 0;
        uint32_t sensor_features_supported = 0;
        uint32_t sensor_dcc_enabled = 0;
        uint32_t sensor_wdr_enabled = 0;
        uint32_t sensor_exp_control_enabled = 0;
        uint32_t sensor_gain_control_enabled = 0;

        /* DCC Params */
        vx_size dcc_buff_size;
        const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
        uint8_t * dcc_viss_buf;
        vx_map_id dcc_viss_buf_map_id;
        vx_distribution histogram_exemplar;
        vx_user_data_object ae_awb_result_exemplar;

#ifdef _ENABLE_2A_
        const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
        vx_map_id dcc_2a_buf_map_id;
        uint8_t * dcc_2a_buf;
        vx_user_data_object h3a_aew_af_exemplar;
        vx_bool viss_prms_replicate[] =
            {vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
             vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
        vx_bool aewb_prms_replicate[] =
            {vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e};
#else
        vx_bool viss_prms_replicate[] =
            {vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
             vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e};
#endif
        vx_user_data_object aewb_config_exemplar;

        /*
        Check for supported sensor features.
        It is upto the application to decide which features should be enabled.
        This demo app enables WDR, DCC and 2A if the sensor supports it.
        */
        sensor_features_supported = sensorParams.sensorInfo.features;

        if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
        {
            APP_PRINTF("WDR mode is supported \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
            sensor_wdr_enabled = 1;
        }else
        {
            APP_PRINTF("WDR mode is not supported. Defaulting to linear \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
            sensor_wdr_enabled = 0;
        }

        if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
        {
            APP_PRINTF("Expsoure control is supported \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
            sensor_exp_control_enabled = 1;
        }

        if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
        {
            APP_PRINTF("Gain control is supported \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
            sensor_gain_control_enabled = 1;
        }

        if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
        {
            sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
            sensor_dcc_enabled = 1;
        }else
        {
            sensor_dcc_enabled = 0;
        }

        APP_PRINTF("Sensor width = %d\n", sensorParams.sensorInfo.raw_params.width);
        APP_PRINTF("Sensor height = %d\n", sensorParams.sensorInfo.raw_params.height);
        APP_PRINTF("Sensor DCC ID = %d\n", sensorParams.dccId);
        APP_PRINTF("Sensor Supported Features = 0x%x\n", sensor_features_supported);
        APP_PRINTF("Sensor Enabled Features = 0x%x\n", sensor_features_enabled);
        appInitImageSensor(obj->sensor_name, sensor_features_enabled, 0xF);/*Mask = 0xF for 4 cameras*/

        /* VISS Initialization */

        /* Allocate sample NV12 image, using which object array of NV12
         * would be created */
        obj->sample_nv12_img =
            vxCreateImage(obj->context, obj->inWidth, obj->inHeight,
            VX_DF_IMAGE_NV12);
        if (vxGetStatus((vx_reference)obj->sample_nv12_img) != VX_SUCCESS)
        {
            APP_PRINTF("sample_nv12_img create failed\n");
            return VX_FAILURE;
        }

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            /* Allocate object array for the output frames */
            obj->viss_out_frames[buf_id] = vxCreateObjectArray(obj->context,
                (vx_reference)obj->sample_nv12_img, NUM_CAPT_CHANNELS);
            if (vxGetStatus((vx_reference)obj->viss_out_frames[buf_id]) != VX_SUCCESS)
            {
                APP_PRINTF("viss_out_frames[buf_id] create failed\n");
                return VX_FAILURE;
            }

            obj->viss_nv12_out_img[buf_id] = (vx_image)vxGetObjectArrayItem(obj->viss_out_frames[buf_id], 0);
            if (vxGetStatus((vx_reference)obj->viss_nv12_out_img[buf_id]) != VX_SUCCESS)
            {
                APP_PRINTF("viss_nv12_out_img[buf_id] create failed\n");
                return VX_FAILURE;
            }
        }

        /* Sample image is no longer required */
        vxReleaseImage(&obj->sample_nv12_img);

        memset(&viss_params, 0, sizeof(tivx_vpac_viss_params_t));

        obj->viss_configuration =
                vxCreateUserDataObject(obj->context, "tivx_vpac_viss_params_t",
                sizeof(tivx_vpac_viss_params_t), NULL);
        if (vxGetStatus((vx_reference)obj->viss_configuration) != VX_SUCCESS)
        {
            APP_PRINTF("viss_configuration create failed\n");
            return VX_FAILURE;
        }

        /* VISS Initialize parameters */
        tivx_vpac_viss_params_init(&viss_params);

        viss_params.sensor_dcc_id = obj->cam_dcc_id;
        viss_params.use_case = 0;
        viss_params.fcp[0].ee_mode = 0;
        viss_params.fcp[0].mux_output0 = 0;
        viss_params.fcp[0].mux_output1 = 0;
        viss_params.fcp[0].mux_output2 = 4;
        viss_params.fcp[0].mux_output3 = 0;
        viss_params.fcp[0].mux_output4 = 3;
        viss_params.bypass_nsf4 = 1;
        viss_params.h3a_in = 3;
        viss_params.h3a_aewb_af_mode = 0;
        viss_params.fcp[0].chroma_mode = 0;
        viss_params.enable_ctx = 1;
        if(sensor_wdr_enabled == 1)
        {
            viss_params.bypass_glbce = 0;
        }else
        {
            viss_params.bypass_glbce = 1;
        }


        /* Create h3a_aew_af output buffer (uninitialized) */
#ifdef _ENABLE_2A_

        h3a_aew_af_exemplar = vxCreateUserDataObject(obj->context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
        if (vxGetStatus((vx_reference)h3a_aew_af_exemplar) != VX_SUCCESS)
        {
            APP_PRINTF("h3a_aew_af create failed\n");
            return VX_FAILURE;
        }

        obj->h3a_aew_af_arr = vxCreateObjectArray(obj->context,
            (vx_reference)h3a_aew_af_exemplar, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->h3a_aew_af_arr) != VX_SUCCESS)
        {
            APP_PRINTF("h3a_aew_af_arr create failed\n");
            return VX_FAILURE;
        }

        obj->h3a_aew_af = (vx_user_data_object) vxGetObjectArrayItem(obj->h3a_aew_af_arr, 0);
        if (vxGetStatus((vx_reference)obj->h3a_aew_af) != VX_SUCCESS)
        {
            APP_PRINTF("h3a_aew_af create failed\n");
            return VX_FAILURE;
        }

        vxReleaseUserDataObject(&h3a_aew_af_exemplar);
#else
        obj->h3a_aew_af_arr = NULL;
        obj->h3a_aew_af = NULL;
#endif

        vxCopyUserDataObject(obj->viss_configuration, 0,
            sizeof(tivx_vpac_viss_params_t), &viss_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ae_awb_result_exemplar =
            vxCreateUserDataObject(obj->context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL);
        if (vxGetStatus((vx_reference)ae_awb_result_exemplar) != VX_SUCCESS)
        {
            APP_PRINTF("ae_awb_result_exemplar create failed\n");
            return VX_FAILURE;
        }


        obj->ae_awb_result_arr = vxCreateObjectArray(obj->context,
            (vx_reference)ae_awb_result_exemplar, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->ae_awb_result_arr) != VX_SUCCESS)
        {
            APP_PRINTF("obj->ae_awb_result_arr create failed\n");
            return VX_FAILURE;
        }

        obj->ae_awb_result = (vx_user_data_object)vxGetObjectArrayItem(obj->ae_awb_result_arr, 0);
        if (vxGetStatus((vx_reference)obj->ae_awb_result) != VX_SUCCESS)
        {
            APP_PRINTF("obj->ae_awb_result create failed\n");
            return VX_FAILURE;
        }

        vxReleaseUserDataObject(&ae_awb_result_exemplar);

        /* Get sample RAW Image */
        obj->raw_img = (tivx_raw_image) vxGetObjectArrayItem(obj->capt_frames[0], 0);

        if(sensor_dcc_enabled)
        {
            dcc_buff_size = appIssGetDCCSizeVISS(obj->sensor_name, sensor_wdr_enabled);

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

            status = appIssGetDCCBuffVISS(obj->sensor_name, sensor_wdr_enabled, dcc_viss_buf, dcc_buff_size);
            if(status != VX_SUCCESS)
            {
                printf("Error getting VISS DCC buffer \n");
                return VX_FAILURE;
            }
            vxUnmapUserDataObject(obj->dcc_param_viss, dcc_viss_buf_map_id);
        }else
        {
            obj->dcc_param_viss = NULL;
        }

        histogram_exemplar = vxCreateDistribution(obj->context, 256, 0, 256);

        obj->histogram_arr = vxCreateObjectArray(obj->context,
                (vx_reference)histogram_exemplar, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->histogram_arr) != VX_SUCCESS)
        {
            APP_PRINTF("histogram_arr create failed\n");
            return VX_FAILURE;
        }

        obj->histogram = (vx_distribution)vxGetObjectArrayItem(obj->histogram_arr, 0);
        if (vxGetStatus((vx_reference)obj->histogram) != VX_SUCCESS)
        {
            APP_PRINTF("histogram create failed\n");
            return VX_FAILURE;
        }

        vxReleaseDistribution(&histogram_exemplar);

        obj->vissNode = tivxVpacVissNode(
                             obj->graph,
                             obj->viss_configuration,
                             NULL,
                             obj->dcc_param_viss,
                             obj->raw_img,
                             NULL,
                             NULL,
                             obj->viss_nv12_out_img[0],
                             NULL,
                             NULL,
                             obj->h3a_aew_af,
                             NULL, NULL, NULL);
        if (vxGetStatus((vx_reference)obj->vissNode) != VX_SUCCESS)
        {
            APP_PRINTF("vissNode create failed\n");
            return VX_FAILURE;
        }

        tivxSetNodeParameterNumBufByIndex(obj->vissNode, 10u, NUM_BUFS);

        vxSetReferenceName((vx_reference)obj->vissNode, "VISS_Processing");
        vxSetNodeTarget(obj->vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);
        vxReplicateNode(obj->graph, obj->vissNode, viss_prms_replicate, 13u);
#ifdef _ENABLE_2A_
        obj->aewb_cfg.sensor_dcc_id = obj->cam_dcc_id;
        obj->aewb_cfg.sensor_img_format = 0;
        obj->aewb_cfg.sensor_img_phase = 3;
        if(sensor_exp_control_enabled || sensor_gain_control_enabled )
        {
            obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_AUTO;
        }
        else
        {
            obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        }
        obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_AUTO;
        obj->aewb_cfg.awb_num_skip_frames = 9;
        obj->aewb_cfg.ae_num_skip_frames = 9;
        obj->aewb_cfg.channel_id = 0;

        if(sensor_dcc_enabled)
        {
            dcc_buff_size = appIssGetDCCSize2A(obj->sensor_name, sensor_wdr_enabled);

            obj->dcc_param_2a = vxCreateUserDataObject(
                obj->context,
                (const vx_char*)&dcc_2a_user_data_object_name,
                dcc_buff_size,
                NULL
            );

            vxMapUserDataObject(
                obj->dcc_param_2a,
                0,
                dcc_buff_size,
                &dcc_2a_buf_map_id,
                (void **)&dcc_2a_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

            status = appIssGetDCCBuff2A(obj->sensor_name, sensor_wdr_enabled,  dcc_2a_buf, dcc_buff_size);
            if(status != VX_SUCCESS)
            {
                printf("Error getting 2A DCC buffer \n");
                return VX_FAILURE;
            }

            vxUnmapUserDataObject(obj->dcc_param_2a, dcc_2a_buf_map_id);
        }else
        {
            obj->dcc_param_2a = NULL;
        }

        /* Separate config for AEWB nodes to pass unique channel ID */
        aewb_config_exemplar = vxCreateUserDataObject(obj->context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &obj->aewb_cfg);
        obj->aewb_config_array = vxCreateObjectArray(obj->context, (vx_reference)aewb_config_exemplar, NUM_CAPT_CHANNELS);
        vxReleaseUserDataObject(&aewb_config_exemplar);
        for(loop_id=0;loop_id<NUM_CAPT_CHANNELS;loop_id++)
        {
            aewb_config_exemplar = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_config_array, loop_id);
            obj->aewb_cfg.channel_id = loop_id;
            vxCopyUserDataObject(aewb_config_exemplar, 0, sizeof(tivx_aewb_config_t), &obj->aewb_cfg, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            vxReleaseUserDataObject(&aewb_config_exemplar);
        }
        obj->aewb_config = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_config_array, 0);

        obj->node_aewb = tivxAewbNode(obj->graph,
                                      obj->aewb_config,
                                      obj->histogram,
                                      obj->h3a_aew_af,
                                      NULL,
                                      (vx_user_data_object)obj->ae_awb_result,
                                      obj->dcc_param_2a);
        vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU2_0);

        if(NULL != obj->node_aewb)
            vxSetReferenceName((vx_reference)obj->node_aewb, "2A_AlgNode");
        else
        {
            APP_PRINTF("tivxAewbNode returned NULL \n");
            return VX_FAILURE;
        }
        APP_PRINTF("AEWB Set Reference done\n");

        vxReplicateNode(obj->graph, obj->node_aewb, aewb_prms_replicate, 6u);
        tivxSetNodeParameterNumBufByIndex(obj->node_aewb, 4u, NUM_BUFS);
#else
        obj->aewb_config = NULL;
        obj->node_aewb = NULL;
        obj->dcc_param_2a = NULL;
#endif

        return status;
}

static vx_status app_create_graph_img_mosaic(SrvCalibAppObj *obj)
{
  vx_status status = VX_SUCCESS;
  vx_int32 i, num_inputs;

  num_inputs   = 1;

  tivxImgMosaicParamsSetDefaults(&obj->mosaic_params);

  obj->mosaic_params.num_windows  = 4;
  obj->mosaic_params.clear_count  = 4;

  for (i = 0; i < NUM_CAPT_CHANNELS; i++)
  {
    if (0 == i)
    {
      obj->mosaic_params.windows[i].startX  = 0;
      obj->mosaic_params.windows[i].startY  = 0;
    }
    else if (1 == i)
    {
      obj->mosaic_params.windows[i].startX  = obj->inWidth/2;
      obj->mosaic_params.windows[i].startY  = 0;
    }
    else if (2 == i)
    {
      obj->mosaic_params.windows[i].startX  = 0;
      obj->mosaic_params.windows[i].startY  = obj->inHeight/2;
    }
    else if (3 == i)
    {
      obj->mosaic_params.windows[i].startX  = obj->inWidth/2;
      obj->mosaic_params.windows[i].startY  = obj->inHeight/2;
    }

    obj->mosaic_params.windows[i].width   = obj->inWidth/2;
    obj->mosaic_params.windows[i].height  = obj->inHeight/2;
    obj->mosaic_params.windows[i].input_select   = 0;
    obj->mosaic_params.windows[i].channel_select = i;
  }

  obj->mosaic_config = vxCreateUserDataObject(obj->context, "ImgMosaicConfig", sizeof(tivxImgMosaicParams), NULL);
  status = vxGetStatus((vx_reference)obj->mosaic_config);

  if(status == VX_SUCCESS)
  {
      status = vxCopyUserDataObject(obj->mosaic_config, 0, sizeof(tivxImgMosaicParams),\
                &obj->mosaic_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
  }
  if(status == VX_SUCCESS)
  {
      for (i = 0; i < NUM_BUFS; i++)
      {
          obj->output_image[i] = vxCreateImage(obj->context, obj->inWidth, obj->inHeight, VX_DF_IMAGE_NV12);
          status = vxGetStatus((vx_reference)obj->output_image[i]);
      }
  }

  if(status == VX_SUCCESS)
  {
      obj->mosaic_kernel = tivxAddKernelImgMosaic(obj->context, num_inputs);
      status = vxGetStatus((vx_reference)obj->mosaic_kernel);
  }

  for(i = 0; i < TIVX_IMG_MOSAIC_MAX_INPUTS; i++)
  {
    obj->input_arr[i] = NULL;
  }

  obj->input_arr[0] = obj->viss_out_frames[0];

  obj->mosaic_node = tivxImgMosaicNode(obj->graph,
                                         obj->mosaic_kernel,
                                         obj->mosaic_config,
                                         obj->output_image[0],
                                         NULL,
                                         obj->input_arr,
                                         num_inputs);

  APP_ASSERT_VALID_REF(obj->mosaic_node);
  vxSetNodeTarget(obj->mosaic_node, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);
  vxSetReferenceName((vx_reference)obj->mosaic_node, "MosaicNode");

  return status;
}


/* Creating display node */
static vx_status app_create_display(SrvCalibAppObj *obj)
{
        vx_status status = VX_SUCCESS;
        tivx_display_params_t display_params;
        vx_uint32 posX, posY, pipeId;

        posX      = 0;
        posY      = 0;
        pipeId    = 0;

        /* Display initialization */
        memset(&display_params, 0, sizeof(tivx_display_params_t));
        display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        display_params.pipeId=pipeId;
        display_params.outWidth=obj->outWidth;
        display_params.outHeight=obj->outHeight;
        display_params.posX=posX;
        display_params.posY=posY;

        obj->display_param_obj =
            vxCreateUserDataObject(obj->context, "tivx_display_params_t",
            sizeof(tivx_display_params_t), &display_params);
        if (vxGetStatus((vx_reference)obj->display_param_obj) != VX_SUCCESS)
        {
            APP_PRINTF("display_param_obj create failed\n");
            return VX_FAILURE;
        }

        obj->displayNode =
            tivxDisplayNode(obj->graph_display, obj->display_param_obj, obj->output_image[0]);

        if (vxGetStatus((vx_reference)obj->display_param_obj) != VX_SUCCESS)
        {
            APP_PRINTF("displayNode create failed\n");
            return VX_FAILURE;
        }

        vxSetNodeTarget(obj->displayNode, VX_TARGET_STRING,
            TIVX_TARGET_DISPLAY1);
        vxSetReferenceName((vx_reference)obj->displayNode, "Display_node");

        return status;
}

/* Creating and configuring draw handle */
/* Note: also clearing input buffer to display */
static vx_status app_create_draw(SrvCalibAppObj *obj)
{
    vx_status status = VX_SUCCESS;
    Draw2D_BufInfo sBufInfo;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    uint8_t *data_ptr[2];
    uint32_t stride_y[2];
    int i, j, image_size;

    Draw2D_create(&obj->pHndl);
    sBufInfo.bufWidth    = obj->inWidth;
    sBufInfo.bufHeight   = obj->inHeight;
    sBufInfo.dataFormat = DRAW2D_DF_YUV420SP_UV;
    sBufInfo.transperentColor = 0;
    sBufInfo.transperentColorFormat = DRAW2D_DF_YUV420SP_UV;

    for (j = 0; j < NUM_BUFS; j++)
    {
        for (i = 0; i < 2; i++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = obj->inWidth;
            rect.end_y = obj->inHeight;

            status |= vxMapImagePatch(obj->output_image[j],
                          &rect,
                          i,
                          &map_id,
                          &image_addr,
                          (void*)&data_ptr[i],
                          VX_READ_AND_WRITE,
                          VX_MEMORY_TYPE_HOST,
                          VX_NOGAP_X
                          );

            stride_y[i] = image_addr.stride_y;

            if (0 == i)
            {
                image_size = obj->inWidth * obj->inHeight;
                memset(data_ptr[i], 0, image_size);
            }
            else
            {
                image_size = obj->inWidth * (obj->inHeight / 2);
                memset(data_ptr[i], 128, image_size);
            }

            vxUnmapImagePatch(obj->output_image[j], map_id);
        }
    }

    sBufInfo.bufPitch[0] = stride_y[0];
    sBufInfo.bufPitch[1] = stride_y[1];
    sBufInfo.bufAddr[0] = data_ptr[0];
    sBufInfo.bufAddr[1] = data_ptr[1];

    status |= Draw2D_setBufInfo(obj->pHndl, &sBufInfo);

    return status;
}

/* Note: draws box in all quadrants */
static vx_status app_draw_one_box(Draw2D_Handle pHndl, uint32_t left,
                             uint32_t top, uint32_t right, uint32_t bottom, Draw2D_LinePrm sLinePrm)
{
    vx_status status = VX_SUCCESS;

    status |= Draw2D_drawLine(pHndl,
                             left,
                             top,
                             left,
                             bottom,
                             &sLinePrm);

    status |= Draw2D_drawLine(pHndl,
                             left,
                             top,
                             right,
                             top,
                             &sLinePrm);

    status |= Draw2D_drawLine(pHndl,
                             right,
                             top,
                             right,
                             bottom,
                             &sLinePrm);

    status |= Draw2D_drawLine(pHndl,
                             left,
                             bottom,
                             right,
                             bottom,
                             &sLinePrm);

    return status;
}

/* Note: draws box in all quadrants */
static vx_status app_draw_quad_box(SrvCalibAppObj *obj, uint32_t left,
                             uint32_t top, uint32_t right, uint32_t bottom, Draw2D_LinePrm sLinePrm)
{
    vx_status status = VX_SUCCESS;
    uint32_t scaled_left    = left/2;
    uint32_t scaled_top     = top/2;
    uint32_t scaled_right   = right/2;
    uint32_t scaled_bottom  = bottom/2;
    uint32_t scaled_width   = obj->inWidth/2;
    uint32_t scaled_height  = obj->inHeight/2;

    status |= app_draw_one_box(obj->pHndl,
                             scaled_left,
                             scaled_top,
                             scaled_right,
                             scaled_bottom,
                             sLinePrm);

    status |= app_draw_one_box(obj->pHndl,
                             scaled_left+scaled_width,
                             scaled_top,
                             scaled_right+scaled_width,
                             scaled_bottom,
                             sLinePrm);

    status |= app_draw_one_box(obj->pHndl,
                             scaled_left,
                             scaled_top+scaled_height,
                             scaled_right,
                             scaled_bottom+scaled_height,
                             sLinePrm);

    status |= app_draw_one_box(obj->pHndl,
                             scaled_left+scaled_width,
                             scaled_top+scaled_height,
                             scaled_right+scaled_width,
                             scaled_bottom+scaled_height,
                             sLinePrm);

    return status;
}

static vx_status app_draw_bounding_box(SrvCalibAppObj *obj, vx_image image)
{
        vx_status status = VX_SUCCESS;
        Draw2D_LinePrm sLinePrm;
        vx_imagepatch_addressing_t image_addr;
        vx_rectangle_t rect;
        vx_map_id map_id;
        uint8_t *data_ptr[3];
        int i;

        for (i = 0; i < 2; i++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = obj->inWidth;
            rect.end_y = obj->inHeight;

            status |= vxMapImagePatch(image,
                          &rect,
                          i,
                          &map_id,
                          &image_addr,
                          (void*)&data_ptr[i],
                          VX_READ_ONLY,
                          VX_MEMORY_TYPE_HOST,
                          VX_NOGAP_X
                          );

            vxUnmapImagePatch(image, map_id);
        }

        Draw2D_updateBufAddr(obj->pHndl, data_ptr);

        /* Draw box */
        sLinePrm.lineColor = RGB888_TO_RGB565(255, 0, 0);
        sLinePrm.lineSize  = 3;
        sLinePrm.lineColorFormat = DRAW2D_DF_YUV420SP_UV;

        /* Drawing left box */
        status |= app_draw_quad_box(obj,
                                 obj->width_scaling_factor*obj->in_point_detect_params.firstROILeft,
                                 obj->height_scaling_factor*obj->in_point_detect_params.firstROITop,
                                 obj->width_scaling_factor*obj->in_point_detect_params.firstROIRight,
                                 obj->height_scaling_factor*obj->in_point_detect_params.firstROIBottom,
                                 sLinePrm);

        /* Drawing right box */
        status |= app_draw_quad_box(obj,
                                 obj->width_scaling_factor*obj->in_point_detect_params.secondROILeft,
                                 obj->height_scaling_factor*obj->in_point_detect_params.secondROITop,
                                 obj->width_scaling_factor*obj->in_point_detect_params.secondROIRight,
                                 obj->height_scaling_factor*obj->in_point_detect_params.secondROIBottom,
                                 sLinePrm);

        return status;
}

/* Creating calibration node */
static vx_status app_create_calibration_graph(SrvCalibAppObj *obj)
{
        vx_status status = VX_SUCCESS;

        /* Structs for populating OpenVX SRV calib objects */
        vx_uint8 is2MP = 0;
        svPoseEstimation_t    in_pose_estimation_params;
        svLdcLut_t      in_lens_params;
        ldc_lensParameters lens_params; /* This is only for reading from file */

        if (DISPLAY_WIDTH_2MP == obj->outWidth)
        {
            is2MP = 1;
        }

        obj->graph_srv_calib = vxCreateGraph(obj->context);
        if (vxGetStatus((vx_reference)obj->graph_srv_calib) != VX_SUCCESS)
        {
            APP_PRINTF("graph create failed\n");
            return VX_FAILURE;
        }

        /* Loading lens params */
        read_lut_file(&lens_params,"psdkra/srv/srv_app/LENS.BIN" );

        /* Clearing all params */
        memset(&obj->in_point_detect_params, 0, sizeof(svPointDetect_t));
        obj->point_detect_in_config = vxCreateUserDataObject(obj->context, "svPointDetect_t",
                                       sizeof(svPointDetect_t), NULL);
        if (vxGetStatus((vx_reference)obj->point_detect_in_config) != VX_SUCCESS)
        {
            APP_PRINTF("point_detect_in_config create failed\n");
            return VX_FAILURE;
        }

        memset(&in_pose_estimation_params, 0, sizeof(svPoseEstimation_t));
        obj->pose_estimation_in_config = vxCreateUserDataObject(obj->context, "svPoseEstimation_t",
                                       sizeof(svPoseEstimation_t), NULL);
        if (vxGetStatus((vx_reference)obj->pose_estimation_in_config) != VX_SUCCESS)
        {
            APP_PRINTF("pose_estimation_in_config create failed\n");
            return VX_FAILURE;
        }

        memset(&in_lens_params, 0, sizeof(svLdcLut_t));
        obj->in_lens_param_object = vxCreateUserDataObject(obj->context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL);
        if (vxGetStatus((vx_reference)obj->in_lens_param_object) != VX_SUCCESS)
        {
            APP_PRINTF("in_lens_param_object create failed\n");
            return VX_FAILURE;
        }

        obj->out_calmat = vxCreateUserDataObject(obj->context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL);
        if (vxGetStatus((vx_reference)obj->out_calmat) != VX_SUCCESS)
        {
            APP_PRINTF("out_calmat create failed\n");
            return VX_FAILURE;
        }

        /* Setting Point Detect Params */
        if (1 == is2MP)
        {
            obj->in_point_detect_params.thresholdMode = 0;
            obj->in_point_detect_params.windowMode = 0;
            obj->in_point_detect_params.Ransac = 0;
            obj->in_point_detect_params.SVROIWidth = obj->inWidth;
            obj->in_point_detect_params.SVROIHeight = obj->inHeight;
            obj->in_point_detect_params.binarizeOffset = 80;
            obj->in_point_detect_params.borderOffset = 80;
            obj->in_point_detect_params.smallestCenter = 10;
            obj->in_point_detect_params.largestCenter = 50;
            obj->in_point_detect_params.maxWinWidth  = 400;
            obj->in_point_detect_params.maxWinHeight = 400;
            obj->in_point_detect_params.maxBandLen = 400;
            obj->in_point_detect_params.minBandLen = 4;
            obj->in_point_detect_params.minSampleInCluster = 16;
            obj->in_point_detect_params.firstROITop = 300;
            obj->in_point_detect_params.firstROIBottom = 1050;
            obj->in_point_detect_params.firstROILeft = 50;
            obj->in_point_detect_params.firstROIRight = 900;
            obj->in_point_detect_params.secondROITop = 300;
            obj->in_point_detect_params.secondROIBottom = 1050;
            obj->in_point_detect_params.secondROILeft = 950;
            obj->in_point_detect_params.secondROIRight = 1870;
            obj->in_point_detect_params.camera_id     = 3;
        }
        else
        {
            obj->in_point_detect_params.thresholdMode = 0;
            obj->in_point_detect_params.windowMode = 0;
            obj->in_point_detect_params.Ransac = 0;
            obj->in_point_detect_params.SVROIWidth = 1280;
            obj->in_point_detect_params.SVROIHeight = 720;
            obj->in_point_detect_params.binarizeOffset = 75;
            obj->in_point_detect_params.borderOffset = 50;
            obj->in_point_detect_params.smallestCenter = 2;
            obj->in_point_detect_params.largestCenter = 50;
            obj->in_point_detect_params.maxWinWidth = 180;
            obj->in_point_detect_params.maxWinHeight = 180;
            obj->in_point_detect_params.maxBandLen = 160;
            obj->in_point_detect_params.minBandLen = 2;
            obj->in_point_detect_params.minSampleInCluster = 4;
            obj->in_point_detect_params.firstROITop = 150;
            obj->in_point_detect_params.firstROIBottom = 670;
            obj->in_point_detect_params.firstROILeft = 100;
            obj->in_point_detect_params.firstROIRight = 600;
            obj->in_point_detect_params.secondROITop = 150;
            obj->in_point_detect_params.secondROIBottom = 670;
            obj->in_point_detect_params.secondROILeft = 700;
            obj->in_point_detect_params.secondROIRight = 1200;
            obj->in_point_detect_params.camera_id     = 0;
        }

        /* Setting Lens Params */
        int i;
        for (i=0;i<NUM_CAPT_CHANNELS; i++) {
            LDC_Init(&in_lens_params.ldc[i],
                 lens_params.ldcLUT_distortionCenters[i*2],
                 lens_params.ldcLUT_distortionCenters[i*2+1],
                 lens_params.ldcLUT_focalLength,
                 lens_params.ldcLUT_D2U_table,
                 lens_params.ldcLUT_D2U_length,
                 lens_params.ldcLUT_D2U_step,
                 lens_params.ldcLUT_U2D_table,
                 lens_params.ldcLUT_U2D_length,
                 lens_params.ldcLUT_U2D_step);
        }

        /* Setting Pose Estimation Params */
        in_pose_estimation_params.Ransac = 0;
        in_pose_estimation_params.SingleChartPose = 0;
        in_pose_estimation_params.numCameras = NUM_CAPT_CHANNELS;

        read_chartpos_file(in_pose_estimation_params.inChartPos ,"psdkra/srv/srv_app/CHARTPOS.BIN");

        /* Copy data to user data objects */
        vxCopyUserDataObject(obj->point_detect_in_config, 0, sizeof(svPointDetect_t),
                                 &obj->in_point_detect_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST);

        vxCopyUserDataObject(obj->pose_estimation_in_config, 0, sizeof(svPoseEstimation_t),
                                 &in_pose_estimation_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST);

        vxCopyUserDataObject(obj->in_lens_param_object, 0, sizeof(svLdcLut_t),
                                 &in_lens_params, VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST);


        obj->in = vxCreateImage(obj->context, obj->inWidth, obj->inHeight, VX_DF_IMAGE_NV12);
        if (vxGetStatus((vx_reference)obj->in) != VX_SUCCESS)
        {
            APP_PRINTF("obj->in create failed\n");
            return VX_FAILURE;
        }

        obj->in_array = vxCreateObjectArray(obj->context, (vx_reference)obj->in, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->in_array) != VX_SUCCESS)
        {
            APP_PRINTF("obj->in_array create failed\n");
            return VX_FAILURE;
        }

        vxReleaseImage(&obj->in);

        obj->buf_bwluma_frame = vxCreateImage(obj->context, obj->inWidth, obj->inHeight, VX_DF_IMAGE_U8);
        if (vxGetStatus((vx_reference)obj->buf_bwluma_frame) != VX_SUCCESS)
        {
            APP_PRINTF("obj->buf_bwluma_frame create failed\n");
            return VX_FAILURE;
        }

        obj->buf_bwluma_frame_array = vxCreateObjectArray(obj->context, (vx_reference)obj->buf_bwluma_frame, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->buf_bwluma_frame_array) != VX_SUCCESS)
        {
            APP_PRINTF("obj->buf_bwluma_frame_array create failed\n");
            return VX_FAILURE;
        }

        vxReleaseImage(&obj->buf_bwluma_frame);

        /* Creating applib */
        obj->srv_create_params.vxContext = obj->context;
        obj->srv_create_params.vxGraph   = obj->graph_srv_calib;

        /* Data object */
        obj->srv_create_params.point_detect_in_config    = obj->point_detect_in_config;
        obj->srv_create_params.in_ldclut                 = obj->in_lens_param_object;
        obj->srv_create_params.in_array                  = obj->in_array;
        obj->srv_create_params.buf_bwluma_frame_array    = obj->buf_bwluma_frame_array;
        obj->srv_create_params.pose_estimation_in_config = obj->pose_estimation_in_config;
        obj->srv_create_params.out_calmat                = obj->out_calmat;

        obj->srv_handle = srv_calib_create(&obj->srv_create_params);

        return status;
}

/*
 * Graph,
 *           viss_config
 *               |
 *               v
 * input_img -> VISS -----> output_img
 *
 */
static vx_status app_create_graph(SrvCalibAppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3], graph_parameters_queue_params_list_display[1];

    printf("Querying %s \n", obj->sensor_name);
    memset(&sensorParams, 0, sizeof(sensorParams));
    status = appQueryImageSensor(obj->sensor_name, &sensorParams);
    if(VX_SUCCESS != status)
    {
        printf("appQueryImageSensor returned %d\n", status);
        return status;
    }

    obj->outWidth  = DISPLAY_WIDTH_2MP;
    obj->outHeight = DISPLAY_HEIGHT_2MP;

    obj->inWidth    = sensorParams.sensorInfo.raw_params.width;
    obj->inHeight   = sensorParams.sensorInfo.raw_params.height;

    obj->cam_dcc_id = sensorParams.dccId;

    obj->width_scaling_factor  = (float)obj->inWidth  / (float)obj->outWidth;
    obj->height_scaling_factor = (float)obj->inHeight / (float)obj->outHeight;

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1)) )
    {
        obj->graph = vxCreateGraph(obj->context);
        if (vxGetStatus((vx_reference)obj->graph) != VX_SUCCESS)
        {
            APP_PRINTF("graph create failed\n");
            return VX_FAILURE;
        }

        obj->graph_display = vxCreateGraph(obj->context);
        if (vxGetStatus((vx_reference)obj->graph_display) != VX_SUCCESS)
        {
            APP_PRINTF("graph_display create failed\n");
            return VX_FAILURE;
        }

        APP_PRINTF("before capture create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_capture(obj);

            if (VX_SUCCESS != status)
            {
                APP_PRINTF("capture create failed\n");
                return VX_FAILURE;
            }
        }

        APP_PRINTF("before viss/aewb create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_viss_aewb(obj);
            if (VX_SUCCESS != status)
            {
                APP_PRINTF("viss/aewb create failed\n");
                return VX_FAILURE;
            }
        }

        APP_PRINTF("before mosaic create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_graph_img_mosaic(obj);
            if (VX_SUCCESS != status)
            {
                APP_PRINTF("mosaic create failed\n");
                return VX_FAILURE;
            }
        }

        APP_PRINTF("before display create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_display(obj);
            if (VX_SUCCESS != status)
            {
                APP_PRINTF("display create failed\n");
                return VX_FAILURE;
            }
        }

        APP_PRINTF("before draw create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_draw(obj);
            if (VX_SUCCESS != status)
            {
                APP_PRINTF("draw create failed\n");
                return VX_FAILURE;
            }
        }

        /* Creating SRV Calibration graph */
        APP_PRINTF("before calibration graph create\n");
        if (VX_SUCCESS == status)
        {
            status = app_create_calibration_graph(obj);
            if (VX_SUCCESS != status)
            {
                APP_PRINTF("calibration graph create failed\n");
                return VX_FAILURE;
            }
        }

        if (VX_SUCCESS != status)
        {
            APP_PRINTF("app_create failed!\n");
            return status;
        }

        APP_PRINTF("before pipelining setup\n");
        /* Setting up pipelining parameters */
        add_graph_parameter_by_node_index(obj->graph, obj->captureNode, 1);
        add_graph_parameter_by_node_index(obj->graph, obj->vissNode, 6);
        add_graph_parameter_by_node_index(obj->graph, obj->mosaic_node, 1);

        int graph_parameter_num = 0;
        /* Set graph schedule config such that graph parameter @ index 0 is
         * enqueuable */
        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&obj->capt_frames[0];
        graph_parameter_num++;

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&obj->viss_nv12_out_img[0];
        graph_parameter_num++;

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&obj->output_image[0];
        graph_parameter_num++;

        tivxSetGraphPipelineDepth(obj->graph, NUM_BUFS);

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        APP_PRINTF("before vxSetGraphScheduleConfig\n");
        vxSetGraphScheduleConfig(obj->graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                3,
                graph_parameters_queue_params_list
                );

        APP_PRINTF("vxSetGraphScheduleConfig done\n");


        /* Setting up pipelining parameters */
        add_graph_parameter_by_node_index(obj->graph_display, obj->displayNode, 1);

        graph_parameter_num = 0;

        /* Set graph schedule config such that graph parameter @ index 0 is
         * enqueuable */
        graph_parameters_queue_params_list_display[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list_display[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list_display[graph_parameter_num].refs_list =
            (vx_reference*)&obj->output_image[0];
        graph_parameter_num++;

        tivxSetGraphPipelineDepth(obj->graph_display, NUM_BUFS);

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        APP_PRINTF("before vxSetGraphScheduleConfig\n");
        vxSetGraphScheduleConfig(obj->graph_display,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list_display
                );

        APP_PRINTF("vxSetGraphScheduleConfig done\n");

        status = vxVerifyGraph(obj->graph);
        if (status != VX_SUCCESS)
        {
            APP_PRINTF("graph verification failed\n");
            return VX_FAILURE;
        }

        tivxExportGraphToDot(obj->graph, ".", "srv_calibration_camera_graph");

        status = vxVerifyGraph(obj->graph_display);
        if (status != VX_SUCCESS)
        {
            APP_PRINTF("graph verification failed\n");
            return VX_FAILURE;
        }

        APP_PRINTF("app_create_graph exiting\n");

        tivxExportGraphToDot(obj->graph_display, ".", "srv_calibration_display");

        status = vxVerifyGraph(obj->graph_srv_calib);
        if (status != VX_SUCCESS)
        {
            APP_PRINTF("graph verification failed\n");
            return VX_FAILURE;
        }

        APP_PRINTF("app_create_graph exiting\n");

        tivxExportGraphToDot(obj->graph_srv_calib, ".", "srv_calibration");
    }
    else
    {
        APP_PRINTF("app_create_graph failed: appropriate cores not enabled\n");
        status = -1;
    }

    return status;
}

static void app_delete_graph(SrvCalibAppObj *obj)
{
    vx_uint8 buf_id = 0;

    Draw2D_delete(obj->pHndl);
    APP_PRINTF("releasing draw handle done\n");

    /* Deleting applib */
    srv_calib_delete(obj->srv_handle);
    APP_PRINTF("releasing srv_handle done\n");

    vxReleaseObjectArray(&obj->buf_bwluma_frame_array);
    APP_PRINTF("releasing node_aewb done\n");

    vxReleaseObjectArray(&obj->in_array);
    APP_PRINTF("releasing in_array done\n");

    vxReleaseUserDataObject(&obj->point_detect_in_config);
    APP_PRINTF("releasing point_detect_in_config done\n");

    vxReleaseUserDataObject(&obj->pose_estimation_in_config);
    APP_PRINTF("releasing pose_estimation_in_config done\n");

    vxReleaseUserDataObject(&obj->in_lens_param_object);
    APP_PRINTF("releasing in_lens_param_object done\n");

    vxReleaseUserDataObject(&obj->out_calmat);
    APP_PRINTF("releasing out_calmat done\n");

    vxReleaseGraph(&obj->graph_srv_calib);
    APP_PRINTF("releasing graph_srv_calib done\n");

    vxReleaseNode(&obj->mosaic_node);
    APP_PRINTF("releasing mosaic_node done\n");

    vxRemoveKernel(obj->mosaic_kernel);
    APP_PRINTF("removing mosaic_kernel done\n");

    vxReleaseUserDataObject(&obj->mosaic_config);
    APP_PRINTF("releasing mosaic_config done\n");

    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseImage(&obj->output_image[buf_id]);
        APP_PRINTF("releasing output_image[buf_id] done\n");
    }

    if(NULL != obj->node_aewb)
    {
        vxReleaseNode(&obj->node_aewb);
        APP_PRINTF("releasing node_aewb done\n");
    }

    if(NULL != obj->vissNode)
    {
        vxReleaseNode(&obj->vissNode);
        APP_PRINTF("releasing vissNode done\n");
    }

    vxReleaseNode(&obj->displayNode);
    APP_PRINTF("releasing graph done\n");

    vxReleaseNode(&obj->captureNode);
    APP_PRINTF("releasing captureNode done\n");

    vxReleaseGraph(&obj->graph_display);
    APP_PRINTF("releasing graph_display done\n");

    vxReleaseGraph(&obj->graph);
    APP_PRINTF("releasing graph done\n");

    vxReleaseUserDataObject(&obj->display_param_obj);
    APP_PRINTF("releasing display_param_obj done\n");

    vxReleaseUserDataObject(&obj->capture_param_obj);
    APP_PRINTF("releasing capture_param_obj done\n");

    tivxReleaseRawImage(&obj->raw_img);
    APP_PRINTF("releasing raw_img done\n");

    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseObjectArray(&obj->capt_frames[buf_id]);
        APP_PRINTF("releasing capt_frames done\n");
    }

    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseObjectArray(&obj->viss_out_frames[buf_id]);
    }
    APP_PRINTF("releasing viss_out_frames done\n");

    if (NULL != obj->ae_awb_result_arr)
    {
        vxReleaseObjectArray(&obj->ae_awb_result_arr);
        APP_PRINTF("releasing ae_awb_result_arr done\n");
    }

    if (NULL != obj->ae_awb_result)
    {
        vxReleaseUserDataObject(&obj->ae_awb_result);
        APP_PRINTF("releasing ae_awb_result done\n");
    }

    vxReleaseUserDataObject(&obj->viss_configuration);
    APP_PRINTF("releasing viss_configuration done\n");

    if(NULL != obj->histogram)
    {
        vxReleaseDistribution(&obj->histogram);
        APP_PRINTF("releasing histogram done\n");
    }

    if(NULL != obj->histogram_arr)
    {
        vxReleaseObjectArray(&obj->histogram_arr);
        APP_PRINTF("releasing histogram_arr done\n");
    }

    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseImage(&obj->viss_nv12_out_img[buf_id]);
    }
    APP_PRINTF("releasing viss_nv12_out_img done\n");

    if(NULL != obj->dcc_param_viss)
    {
        vxReleaseUserDataObject(&obj->dcc_param_viss);
        APP_PRINTF("releasing VISS DCC Data Object done\n");
    }

    if(NULL != obj->h3a_aew_af)
    {
        vxReleaseUserDataObject(&obj->h3a_aew_af);
        APP_PRINTF("releasing h3a_aew_af done\n");
    }

    if(NULL != obj->h3a_aew_af_arr)
    {
        vxReleaseObjectArray(&obj->h3a_aew_af_arr);
        APP_PRINTF("releasing h3a_aew_af_arr done\n");
    }

    if(NULL != obj->aewb_config)
    {
        vxReleaseUserDataObject(&obj->aewb_config);
        APP_PRINTF("releasing aewb_config done\n");
    }

    if(NULL != obj->aewb_config_array)
    {
        vxReleaseObjectArray(&obj->aewb_config_array);
        APP_PRINTF("releasing aewb_config_array done\n");
    }

    if(NULL != obj->dcc_param_2a)
    {
        vxReleaseUserDataObject(&obj->dcc_param_2a);
        APP_PRINTF("releasing 2A DCC Data Object done\n");
    }
}

static void app_run_graph_srv_calib(SrvCalibAppObj *obj)
{
    APP_PRINTF("app_srv_calibration: Running graph 1 ...\n");
    vxProcessGraph(obj->graph_srv_calib);
    APP_PRINTF("app_srv_calibration: Running graph 1 ... Done\n");
    return;
}

static void app_run_graph(SrvCalibAppObj *obj)
{
    vx_status status;
    uint32_t num_refs, buf_id;
    int graph_parameter_num = 0;
    char yuv_image_fname[APP_MAX_FILE_PATH];
    uint8_t pause = 0, set_pause = 0;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

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

    if(NULL == obj->sensor_name)
    {
        printf("sensor name is NULL \n");
        return;
    }
    status = appStartImageSensor(obj->sensor_name, 0xF);
    APP_PRINTF("Sensor start status = %d\n", status);
    assert(status == VX_SUCCESS);

    /* Enqueue buf for pipe up but don't trigger graph execution */
    for(buf_id=0; buf_id<NUM_BUFS-2; buf_id++)
    {
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
            (vx_reference*)&obj->capt_frames[buf_id], 1);
        graph_parameter_num++;

        vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
            (vx_reference*)&obj->viss_nv12_out_img[buf_id], 1);
        graph_parameter_num++;

        vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
            (vx_reference*)&obj->output_image[buf_id], 1);

        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(obj->graph_display, graph_parameter_num,
            (vx_reference*)&obj->output_image[buf_id], 1);
    }

    /* Need to trigger again since display holds on to a buffer */
    graph_parameter_num = 0;
    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->capt_frames[NUM_BUFS-2], 1);
    graph_parameter_num++;

    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->viss_nv12_out_img[NUM_BUFS-2], 1);
    graph_parameter_num++;

    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->output_image[NUM_BUFS-2], 1);

    graph_parameter_num = 0;
    vxGraphParameterEnqueueReadyRef(obj->graph_display, graph_parameter_num,
        (vx_reference*)&obj->output_image[NUM_BUFS-2], 1);

    /* Need to trigger again since display holds on to a buffer */
    graph_parameter_num = 0;
    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->capt_frames[NUM_BUFS-1], 1);
    graph_parameter_num++;

    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->viss_nv12_out_img[NUM_BUFS-1], 1);
    graph_parameter_num++;

    vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
        (vx_reference*)&obj->output_image[NUM_BUFS-1], 1);

    graph_parameter_num = 0;
    vxGraphParameterEnqueueReadyRef(obj->graph_display, graph_parameter_num,
        (vx_reference*)&obj->output_image[NUM_BUFS-1], 1);

    /* wait for graph instances to complete, compare output and
     * recycle data buffers, schedule again */
    while(1)
    {
        vx_image out_viss_image, out_mosaic, in_display;
        vx_object_array out_capture_frames;
        graph_parameter_num = 0;

        if (0U == pause)
        {

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num,
                (vx_reference*)&out_capture_frames, 1, &num_refs);
            graph_parameter_num++;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num,
                (vx_reference*)&out_viss_image, 1, &num_refs);

            graph_parameter_num++;
            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num,
                (vx_reference*)&out_mosaic, 1, &num_refs);

            graph_parameter_num = 0;
            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(obj->graph_display, graph_parameter_num,
                (vx_reference*)&in_display, 1, &num_refs);

            /* Drawing bounding box */
            status = app_draw_bounding_box(obj, in_display);
        }

        graph_parameter_num = 0;

        /* Runs calibration on dequeued images from VISS on user prompt */
        if (obj->run_calibration)
        {
            vx_object_array out_objarr;

            printf("Running calibration....\n");

            out_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)out_viss_image);

            status = copy_objarr_for_calibration(obj, out_objarr, 1);

            if (VX_SUCCESS == status)
            {
                printf("Calibration successful!!\n");
                app_draw_corners(obj, in_display);

                set_pause = 1;
            }
            else
            {
                printf("Calibration failed, please re-run calibration\n");
            }

            obj->run_calibration = 0;
        }

        /* Dumps dequeued images from VISS to file on user prompt */
        if (obj->write_file)
        {
            vx_object_array out_objarr;

            out_objarr = (vx_object_array)tivxGetReferenceParent((vx_reference)out_viss_image);

            status = copy_objarr_for_calibration(obj, out_objarr, 0);

            if (VX_SUCCESS == status)
            {
                for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
                {
                    vx_image image;
                    printf("writing capture channel %d...\n", buf_id);
                    if (0 == buf_id)
                    {
                        snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/viss_output_front.yuv");
                    }
                    else if (1 == buf_id)
                    {
                        snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/viss_output_right.yuv");
                    }
                    else if (2 == buf_id)
                    {
                        snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/viss_output_back.yuv");
                    }
                    else if (3 == buf_id)
                    {
                        snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/viss_output_left.yuv");
                    }
                    image = (vx_image)vxGetObjectArrayItem(obj->in_array, buf_id);
                    write_output_image_nv12_8bit(yuv_image_fname, image);
                    vxReleaseImage(&image);
                }
            }

            obj->write_file = 0;
        }

        if (obj->write_capture_file)
        {
            for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
            {
                tivx_raw_image raw_img;
                printf("writing capture channel %d...\n", buf_id);
                if (0 == buf_id)
                {
                    snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/capture_output_front.raw");
                }
                else if (1 == buf_id)
                {
                    snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/capture_output_right.raw");
                }
                else if (2 == buf_id)
                {
                    snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/capture_output_back.raw");
                }
                else if (3 == buf_id)
                {
                    snprintf(yuv_image_fname, APP_MAX_FILE_PATH, "%s/%s", test_data_path, "output/capture_output_left.raw");
                }
                raw_img = (tivx_raw_image)vxGetObjectArrayItem(out_capture_frames, buf_id);
                write_output_image_raw(yuv_image_fname, raw_img);
                tivxReleaseRawImage(&raw_img);
            }

            obj->write_capture_file = 0;
        }

        if (0U == pause)
        {
            vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1);
            graph_parameter_num++;

            vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num, (vx_reference*)&out_viss_image, 1);
            graph_parameter_num++;

            vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num, (vx_reference*)&in_display, 1);

            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(obj->graph_display, graph_parameter_num, (vx_reference*)&out_mosaic, 1);
        }

        /* Note: putting this after enqueue to not disrupt the dequeue/enqueue */
        if (set_pause)
        {
            pause = 1;
            set_pause = 0;
        }

        if (obj->resume)
        {
            pause = 0;
            obj->resume = 0;
        }

        if(obj->stop_task)
        {
            break;
        }
    }
    APP_PRINTF("app_srv_calibration: Exiting pipelining loop ...\n");

    /* ensure all graph processing is complete */
    vxWaitGraph(obj->graph);
    vxWaitGraph(obj->graph_display);

    APP_PRINTF("app_srv_calibration: after vxWaitGraph ...\n");

    /* Dequeue buf for pipe down */
    for(buf_id=0; buf_id<NUM_BUFS-2; buf_id++)
    {
        APP_PRINTF("app_srv_calibration: dequeueing %d\n", buf_id);
        vx_object_array out_capture_frames;

        graph_parameter_num = 0;
        APP_PRINTF(" Dequeuing aewb objects # %d...\n", buf_id);
        vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1, &num_refs);
        graph_parameter_num++;
    }
}

static void app_run_task(void *app_var)
{
    SrvCalibAppObj *obj = (SrvCalibAppObj *)app_var;

    appPerfStatsCpuLoadResetAll();

    app_run_graph(obj);

    obj->stop_task_done = 1;
}

static int32_t app_run_task_create(SrvCalibAppObj *obj)
{
    tivx_task_create_params_t params;
    int32_t status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_task;
    params.app_var = obj;

    obj->stop_task_done = 0;
    obj->stop_task = 0;

    status = tivxTaskCreate(&obj->task, &params);

    return status;
}

static void app_run_task_delete(SrvCalibAppObj *obj)
{
    while(obj->stop_task_done==0)
    {
         tivxTaskWaitMsecs(100);
    }
    tivxTaskDelete(&obj->task);
}

static const char menu[] = {
    "\n"
    "\n =========================="
    "\n Demo : SRV Calibration"
    "\n =========================="
    "\n"
    "\n p: Print performance statistics"
    "\n"
    "\n a: Run calibration"
    "\n"
    "\n b: Dump VISS output images"
    "\n"
    "\n c: Resume graph to recalibrate"
    "\n"
    "\n d: Dump capture raw images"
    "\n"
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};

static vx_status app_run_graph_interactive(SrvCalibAppObj *obj)
{
    vx_status status;
    uint32_t done = 0;
    char ch;

    status = app_run_task_create(obj);
    if(status!=0)
    {
        printf("ERROR: Unable to create task\n");
    }
    else
    {
        while(!done)
        {
            printf(menu);
            ch = getchar();
            printf("\n");

            switch(ch)
            {
                case 'p':
                case 'P':
                    appPerfStatsPrintAll();
                    tivx_utils_graph_perf_print(obj->graph);
                    break;
                case 'x':
                case 'X':
                    obj->stop_task = 1;
                    done = 1;
                    break;
                case 'a':
                case 'A':
                    obj->run_calibration = 1;
                    break;
                case 'b':
                case 'B':
                    obj->write_file = 1;
                    break;
                case 'c':
                case 'C':
                    obj->resume = 1;
                    break;
                case 'd':
                case 'D':
                    obj->write_capture_file = 1;
                    break;
                default:
                    break;
            }
        }
        app_run_task_delete(obj);
    }
    return status;
}
