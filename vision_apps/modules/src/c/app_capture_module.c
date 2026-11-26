/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#include "app_capture_module.h"
#include <iss_sensors.h>
#include <iss_sensor_if.h>

#define MAX_FNAME (256)

static vx_status configure_capture_params(vx_context context, CaptureObj *captureObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 num_capt_instances = 0;
    vx_int32 id, lane, ch, vcNum;
    int32_t ch_mask = sensorObj->ch_mask;

    if (ch_mask <= 0xF)
    {
        num_capt_instances = 1;
    }
    else if ((ch_mask > 0xF) && (ch_mask <= 0xFF))
    {
        num_capt_instances = 2;
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if ((ch_mask > 0xFF) && (ch_mask <= 0xFFF))
    {
        num_capt_instances = 3;
    }
    #endif
    #if defined(SOC_TDA54)
    else if ((ch_mask > 0xFFF) && (ch_mask <= 0xFFFF))
    {
        num_capt_instances = 4;
    }
    #endif
    else
    {
        printf("[CAPTURE_MODULE] - ch_mask parameter is invalid! \n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if(status == VX_SUCCESS)
    {
        captureObj->capture_format = sensorObj->sensor_out_format;

        tivx_capture_params_init(&captureObj->params);

        if (captureObj->enable_error_detection || captureObj->test_mode)
        {
            captureObj->params.timeout        = 90;
            captureObj->params.timeoutInitial = 500;
        }
        captureObj->params.numInst  = num_capt_instances;
        captureObj->params.numCh    = sensorObj->num_cameras_enabled;

        for(id = 0; id < num_capt_instances; id++)
        {
            captureObj->params.instId[id]                       = id;
            #if defined(SOC_FAMILY_J7) || defined(SOC_FAMILY_AM)
            captureObj->params.instCfg[id].enableCsiv2p0Support = (uint32_t)vx_true_e;
            #elif defined(SOC_FAMILY_TDA5)
            captureObj->params.instCfg[id].phyMode              = TIVX_CAPTURE_DPHY_MODE;
            #endif
            captureObj->params.instCfg[id].numDataLanes         = sensorObj->sensorParams.sensorInfo.numDataLanes;
            APP_PRINTF("captureObj->params.numDataLanes = %d \n", captureObj->params.instCfg[id].numDataLanes);
            captureObj->params.instCfg[id].laneBandSpeed        = sensorObj->sensorParams.sensorInfo.csi_laneBandSpeed;

            #if defined(SOC_FAMILY_J7) || defined(SOC_FAMILY_AM)
            if (0 == strcmp(sensorObj->sensorParams.name, SENSOR_SONY_IMX728_UB971_SONY))
            {
                captureObj->params.instCfg[id].numPixels = 1U;
            }
            #endif

            for (lane = 0; lane < captureObj->params.instCfg[id].numDataLanes; lane++)
            {
                captureObj->params.instCfg[id].dataLanesMap[lane] = lane + 1;
                APP_PRINTF("captureObj->params.dataLanesMap[%d] = %d \n",
                            lane,
                            captureObj->params.instCfg[id].dataLanesMap[lane]);
            }
        }

        ch = 0;/*Camera Physical Channel Number*/
        vcNum = 0;/*CSI2 Virtual Channel Number*/
        id = 0;/*CSI2 Instance ID*/
        while(ch_mask > 0)
        {
            if(ch > 11)
            {
                id = 3;
            }
            if((ch > 7) && (ch <= 11))
            {
                id = 2;
            }
            else if( (ch > 3) && (ch <= 7) )
            {
                id = 1;
            }
            else
            {
                id = 0;
            }
            if(ch_mask & 0x1)
            {
                captureObj->params.chVcNum[vcNum] = ch%4;
                captureObj->params.chInstMap[vcNum] = id;
                vcNum++;
            }
            ch++;
            ch_mask = ch_mask >> 1;
        }

        captureObj->config = vxCreateUserDataObject(context, "tivx_capture_params_t", sizeof(tivx_capture_params_t), &captureObj->params);
        status = vxGetStatus((vx_reference)captureObj->config);
    }

    if(status != VX_SUCCESS)
    {
        printf("[CAPTURE_MODULE] - Unable to create capture config object! \n");
    }
    else
    {
        vxSetReferenceName((vx_reference)captureObj->config, "capture_node_config");
    }

    return status;
}

static vx_status create_capture_output(vx_context context, CaptureObj *captureObj, SensorObj *sensorObj,  int32_t bufq_depth)
{
    vx_status status = VX_SUCCESS;

    IssSensor_CreateParams *sensorParams = &sensorObj->sensorParams;
    vx_int32 q;

    if(0 == captureObj->capture_format)
    {
        tivx_raw_image raw_image = tivxCreateRawImage(context, &sensorParams->sensorInfo.raw_params);
        status = vxGetStatus((vx_reference)raw_image);

        if(status == VX_SUCCESS)
        {
            for(q = 0; q < bufq_depth; q++)
            {
                captureObj->raw_image_arr[q] = vxCreateObjectArray(context, (vx_reference)raw_image, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)captureObj->raw_image_arr[q]);
                if(status != VX_SUCCESS)
                {
                    printf("[CAPTURE-MODULE] Unable to create RAW image object array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "capture_node_raw_image_arr_%d", q);

                    vxSetReferenceName((vx_reference)captureObj->raw_image_arr[q], name);
                }
            }
            tivxReleaseRawImage(&raw_image);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create RAW image object! \n");
        }
    }
    else
    {
        vx_image cap_yuv_image = vxCreateImage(context, sensorParams->sensorInfo.raw_params.width, sensorParams->sensorInfo.raw_params.height, VX_DF_IMAGE_UYVY);
        status = vxGetStatus((vx_reference)cap_yuv_image);

        if(status == VX_SUCCESS)
        {
            for(q = 0; q < bufq_depth; q++)
            {
                captureObj->raw_image_arr[q] = vxCreateObjectArray(context, (vx_reference)cap_yuv_image, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)captureObj->raw_image_arr[q]);
                if(status != VX_SUCCESS)
                {
                    printf("[CAPTURE-MODULE] Unable to create YUV image object array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "capture_node_raw_image_arr_%d", q);

                    vxSetReferenceName((vx_reference)captureObj->raw_image_arr[q], name);
                }
            }
            vxReleaseImage(&cap_yuv_image);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create YUV image object! \n");
        }
    }

    return status;
}

static vx_status create_error_detection_frame(vx_context context, CaptureObj *captureObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    IssSensor_CreateParams *sensorParams = &sensorObj->sensorParams;

    /*Error detection is currently enabled only for RAW input*/
    if(0 != captureObj->capture_format)
    {
        captureObj->enable_error_detection = 0;
    }

    /* If in test mode or error detection is enabled, send the test frame */
    if ((1 == captureObj->test_mode) || (1 == captureObj->enable_error_detection))
    {
        uint32_t path_index = 0;
        vx_int32 bytes_read = 0;
#if defined (QNX)
        const char * test_data_path = "/ti_fs/vision_apps/test_data/";
#elif defined (PC)
        const char * test_data_path = "./";
#else
        const char * test_data_path = "/opt/vision_apps/test_data/";
#endif
        char raw_image_fname[MAX_FNAME] = {0};
        const char test_image_paths[3][64] =
        {
            "psdkra/app_single_cam/IMX390_001/input2",  /* Used in test mode */
            "psdkra/app_single_cam/AR0233_001/input2",  /* Used in test mode */
            "img_test"                                  /* Used as error frame when not in test mode */
        };

        if (1 == captureObj->test_mode)
        {
            path_index = sensorObj->sensor_index;
        }
        else
        {
            /* Point to last index in the test_image_paths list */
            path_index = ((sizeof(test_image_paths)/64)/sizeof(char))-1;
        }

        snprintf(raw_image_fname, MAX_FNAME, "%s/%s.raw", test_data_path, test_image_paths[path_index]);

        captureObj->error_frame_raw_image = read_error_image_raw(context, &(sensorParams->sensorInfo),
                                                raw_image_fname,
                                                &bytes_read);

        APP_PRINTF("%d bytes were read by read_error_image_raw() from path %s\n", bytes_read, test_data_paths[sensorObj->sensor_index]);
        status = vxGetStatus((vx_reference)captureObj->error_frame_raw_image);

        if(status == VX_SUCCESS)
        {
            if(bytes_read < 0)
            {
                printf("[CAPTURE_MODULE] Bytes read by error frame for RAW image is < 0! \n");
                tivxReleaseRawImage(&captureObj->error_frame_raw_image);
                captureObj->error_frame_raw_image = NULL; /* Is this required after releasing the reference? */
            }
        }
        else
        {
            printf("[CAPTURE_MODULE] Unable to create error frame RAW image!\n");
        }
    }
    else
    {
        captureObj->error_frame_raw_image = NULL;
    }

    return status;
}

static vx_status configure_capture_output_write(vx_context context, CaptureObj *captureObj)
{
    vx_status status = VX_SUCCESS;

    if(captureObj->en_out_capture_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, captureObj->output_file_path);
        captureObj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)captureObj->file_path);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)captureObj->file_path, "capture_write_node_file_path");

            vxAddArrayItems(captureObj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create file path object for writing capture output! \n");
        }

        strcpy(file_prefix, "csix_raw_output");
        captureObj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);

        status = vxGetStatus((vx_reference)captureObj->file_prefix);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)captureObj->file_prefix, "capture_write_node_file_prefix");

            vxAddArrayItems(captureObj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create file prefix object for writing capture output! \n");
        }


        captureObj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)captureObj->write_cmd);

        if(status != VX_SUCCESS)
        {
            printf("[CAPTURE-MODULE] Unable to create file write cmd object for writing capture output! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)captureObj->write_cmd, "capture_write_node_write_cmd");
        }
    }
    else
    {
        captureObj->file_path   = NULL;
        captureObj->file_prefix = NULL;
        captureObj->write_node  = NULL;
        captureObj->write_cmd   = NULL;
    }

    return status;
}

vx_status app_init_capture(vx_context context, CaptureObj *captureObj, SensorObj *sensorObj, char *objName, int32_t bufq_depth)
{
    vx_status status = VX_SUCCESS;

    status = configure_capture_params(context, captureObj, sensorObj);

    if(status == VX_SUCCESS)
    {
        status = create_capture_output(context, captureObj, sensorObj, bufq_depth);
    }

    if(status == VX_SUCCESS)
    {
        status = create_error_detection_frame(context, captureObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configure_capture_output_write(context, captureObj);
    }

    return (status);
}


/* This function is largely taken from single_cam_app :
    this function takes a raw_image that is unpopulated
    and populates it with the path set below */
tivx_raw_image read_error_image_raw(vx_context context,
                             IssSensor_Info *sensorInfo, char raw_image_fname[],
                             vx_int32 *bytes_read)
{
    /* Taking the test data from the app_single_cam demo to save space in the repo */
    FILE * fp;
    vx_uint32 width, height, i;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    void *data_ptr;
    vx_uint32 num_bytes_per_pixel = 2; /*Supports only RAW 12b Unpacked format*/
    vx_uint32 num_bytes_read_from_file;
    tivx_raw_image raw_image = NULL;
    tivx_raw_image_format_t format;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;
    vx_status status = VX_SUCCESS;

    /* Nothing is being populated here - just an empty frame */
    raw_image = tivxCreateRawImage(context, &(sensorInfo->raw_params));

    status = vxGetStatus((vx_reference)raw_image);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)raw_image, "capture_node_error_frame_raw_image");
    }
    else
    {
        printf("[CAPTURE_MODULE] Unable to create error frame RAW image!\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

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
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            TIVX_RAW_IMAGE_PIXEL_BUFFER
            );

        if(!data_ptr)
        {
            printf("data_ptr is NULL \n");
            tivxReleaseRawImage(&raw_image);
            return NULL;
        }

        APP_PRINTF("Reading test RAW image %s \n", raw_image_fname);
        fp = fopen(raw_image_fname, "rb");

        if(!fp)
        {
            printf("read_test_image_raw : Unable to open file %s, setting error message as all 0s\n", raw_image_fname);
            memset(data_ptr, 0x00, image_addr.stride_y*height);
            *bytes_read = 0;
        }
        else
        {
            num_bytes_read_from_file = 0;

            imgaddr_width  = image_addr.dim_x;
            imgaddr_height = image_addr.dim_y;
            imgaddr_stride = image_addr.stride_y;

            for(i=0;i<imgaddr_height;i++)
            {
                num_bytes_read_from_file += fread(data_ptr, 1, imgaddr_width*num_bytes_per_pixel, fp);
                data_ptr += imgaddr_stride;
            }

            fclose(fp);

            APP_PRINTF("%d bytes read from %s\n", num_bytes_read_from_file, raw_image_fname);
            *bytes_read = num_bytes_read_from_file;
        }

        tivxUnmapRawImagePatch(raw_image, map_id);
    }
    return raw_image;
}

void app_deinit_capture(CaptureObj *captureObj, vx_int32 bufq_depth)
{
    vx_int32 i;

    vxReleaseUserDataObject(&captureObj->config);
    for(i = 0; i < bufq_depth; i++)
    {
        vxReleaseObjectArray(&captureObj->raw_image_arr[i]);
    }

    if(NULL != captureObj->error_frame_raw_image)
    {
        tivxReleaseRawImage(&captureObj->error_frame_raw_image);
    }

    if(captureObj->en_out_capture_write == 1)
    {
        vxReleaseArray(&captureObj->file_path);
        vxReleaseArray(&captureObj->file_prefix);
        vxReleaseUserDataObject(&captureObj->write_cmd);
    }
}

void app_delete_capture(CaptureObj *captureObj)
{
    if(captureObj->node != NULL)
    {
        vxReleaseNode(&captureObj->node);
    }
    if(captureObj->write_node != NULL)
    {
        vxReleaseNode(&captureObj->write_node);
    }
}

vx_status app_create_graph_capture(vx_graph graph, CaptureObj *captureObj)
{
    vx_status status = VX_SUCCESS;

    captureObj->node = tivxCaptureNode(graph, captureObj->config, captureObj->raw_image_arr[0]);
    status = vxGetStatus((vx_reference)captureObj->node);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)captureObj->node, "capture_node");
        vxSetNodeTarget(captureObj->node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE1);

        if(captureObj->en_out_capture_write == 1)
        {
            status = app_create_graph_capture_write_output(graph, captureObj);
        }
    }
    else
    {
        printf("[CAPTURE-MODULE] Unable to create capture node! \n");
    }

    return status;
}

vx_status app_create_graph_capture_write_output(vx_graph graph, CaptureObj *captureObj)
{
    vx_status status = VX_SUCCESS;

    if(0 == captureObj->capture_format)
    {
        tivx_raw_image raw_img = (tivx_raw_image)vxGetObjectArrayItem(captureObj->raw_image_arr[0], 0);
        captureObj->write_node = tivxWriteRawImageNode(graph, raw_img, captureObj->file_path, captureObj->file_prefix);
        tivxReleaseRawImage(&raw_img);

        status = vxGetStatus((vx_reference)captureObj->write_node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)captureObj->write_node, "capture_write_node");
            vxSetNodeTarget(captureObj->write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, captureObj->write_node, replicate, 3);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create RAW image write node! \n");
        }
    }
    else
    {
        vx_image raw_img = (vx_image)vxGetObjectArrayItem(captureObj->raw_image_arr[0], 0);
        captureObj->write_node = tivxWriteImageNode(graph, raw_img, captureObj->file_path, captureObj->file_prefix);
        vxReleaseImage(&raw_img);

        status = vxGetStatus((vx_reference)captureObj->write_node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)captureObj->write_node, "capture_write_node");
            vxSetNodeTarget(captureObj->write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, captureObj->write_node, replicate, 3);
        }
        else
        {
            printf("[CAPTURE-MODULE] Unable to create YUV image write node! \n");
        }
    }
    return (status);
}

vx_status app_send_cmd_capture_write_node(CaptureObj *captureObj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(captureObj->write_cmd, 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if(status == VX_SUCCESS)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)captureObj->write_cmd;

        status = tivxNodeSendCommand(captureObj->write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                 TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                 refs, 1u);

        if(VX_SUCCESS != status)
        {
            printf("Capture node send command failed!\n");
        }

        APP_PRINTF("Capture node send command success!\n");
    }

    return (status);
}

vx_status app_send_error_frame(CaptureObj *captureObj)
{
    vx_status status = VX_SUCCESS;

    status = tivxCaptureRegisterErrorFrame(captureObj->node, (vx_reference)captureObj->error_frame_raw_image);

    return status;
}
