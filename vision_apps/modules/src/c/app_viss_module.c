/*
 *
 * Copyright (c) 2020-24 Texas Instruments Incorporated
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

#include "app_viss_module.h"

static vx_status configure_viss_params(vx_context context, VISSObj *vissObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_viss_params_init(&vissObj->params);

    vissObj->params.sensor_dcc_id    = sensorObj->sensorParams.dccId;
    vissObj->params.fcp[0].ee_mode          = 0;
    vissObj->params.fcp[0].mux_output0      = 0;
    vissObj->params.fcp[0].mux_output1      = 0;
    vissObj->params.fcp[0].mux_output2      = 4;
    vissObj->params.fcp[0].mux_output3      = 0;
    vissObj->params.fcp[0].mux_output4      = 3;
    if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI")==0){
#if defined (VPAC3L)
        vissObj->params.bypass_pcid = 0;
#endif
        vissObj->params.h3a_in      = 4;
    }else{
        vissObj->params.h3a_in      = 3;
    }
    vissObj->params.h3a_aewb_af_mode        = 0;
    vissObj->params.fcp[0].chroma_mode      = 0;
    vissObj->params.bypass_nsf4      = 0;
    vissObj->params.enable_ctx       = 1;
    if(sensorObj->sensor_wdr_enabled == 1)
    {
        vissObj->params.bypass_glbce = 0;
    }
    else
    {
        vissObj->params.bypass_glbce = 1;
    }

    vissObj->config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &vissObj->params);
    status = vxGetStatus((vx_reference)vissObj->config);

    if(status != VX_SUCCESS)
    {
        printf("[VISS-MODULE] Unable to create VISS config object! \n");
    }
    else
    {
        vxSetReferenceName((vx_reference)vissObj->config, "viss_node_config");
    }

    return status;
}

static vx_status configure_viss_params_ir(vx_context context, VISSObj *vissObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_viss_params_init(&vissObj->params);

#if defined (VPAC3L)
    vissObj->params.bypass_pcid      = 0;
    vissObj->params.enable_ir_op     = TIVX_VPAC_VISS_IR_ENABLE;
    vissObj->params.enable_bayer_op  = TIVX_VPAC_VISS_BAYER_DISABLE;
#endif
    vissObj->params.sensor_dcc_id    = sensorObj->sensorParams.dccId;
    vissObj->params.fcp[0].ee_mode          = TIVX_VPAC_VISS_EE_MODE_OFF;
#if defined (VPAC3L)
    vissObj->params.fcp[0].mux_output0      = TIVX_VPAC_VISS_MUX0_IR8;
#endif
    vissObj->params.fcp[0].mux_output1      = 0;
    vissObj->params.fcp[0].mux_output2      = 0;
    vissObj->params.fcp[0].mux_output3      = 0;
    vissObj->params.fcp[0].mux_output4      = 3;
    vissObj->params.h3a_in           = TIVX_VPAC_VISS_H3A_IN_LSC;
    vissObj->params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
    vissObj->params.fcp[0].chroma_mode      = TIVX_VPAC_VISS_CHROMA_MODE_420;
    vissObj->params.bypass_nsf4      = 0;
    vissObj->params.enable_ctx       = 1;
    if(sensorObj->sensor_wdr_enabled == 1)
    {   
        vissObj->params.bypass_glbce = 0;
    }
    else
    {
        vissObj->params.bypass_glbce = 1;
    }

    vissObj->config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &vissObj->params);
    status = vxGetStatus((vx_reference)vissObj->config);

    if(status != VX_SUCCESS)
    {
        printf("[VISS-IR-MODULE] Unable to create VISS config object! \n");
    }
    else
    {
        vxSetReferenceName((vx_reference)vissObj->config, "viss_ir_node_config");
    }

    return status;
}

static vx_status configure_dcc_params(vx_context context, VISSObj *vissObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if(sensorObj->sensor_dcc_enabled)
    {
        int32_t dcc_buff_size;
        uint8_t * dcc_buf;
        vx_map_id dcc_buf_map_id;

        dcc_buff_size = appIssGetDCCSizeVISS(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled);
        if(dcc_buff_size < 0)
        {
            printf("Invalid DCC size for VISS \n");
            return VX_FAILURE;
        }

        vissObj->dcc_config = vxCreateUserDataObject(context, "dcc_viss", dcc_buff_size, NULL);
        status = vxGetStatus((vx_reference)vissObj->dcc_config);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->dcc_config, "viss_node_dcc_config");

            vxMapUserDataObject(
                    vissObj->dcc_config,
                    0,
                    dcc_buff_size,
                    &dcc_buf_map_id,
                    (void **)&dcc_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    0
                );

            status = appIssGetDCCBuffVISS(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled, dcc_buf, dcc_buff_size);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-MODULE] Error getting DCC buffer! \n");
                status = VX_FAILURE;
            }

            vxUnmapUserDataObject(vissObj->dcc_config, dcc_buf_map_id);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create DCC config object! \n");
        }
    }
    else
    {
        vissObj->dcc_config = NULL;
    }

    return status;
}

static vx_status create_viss_outputs(vx_context context, VISSObj *vissObj, SensorObj *sensorObj, uint32_t num_cameras_enabled)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 img_height, img_width;

    /* Create h3a_aew_af output buffer (uninitialized) */
    vx_user_data_object h3a_stats = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)h3a_stats);
    if(status == VX_SUCCESS)
    {
        vissObj->h3a_stats_arr = vxCreateObjectArray(context, (vx_reference)h3a_stats, num_cameras_enabled);
        vxReleaseUserDataObject(&h3a_stats);

        status = vxGetStatus((vx_reference)vissObj->h3a_stats_arr);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create h3a stats object array! \n");
            return status;
        }
        else
        {
            vxSetReferenceName((vx_reference)vissObj->h3a_stats_arr, "viss_node_h3a_stats_arr");
        }
    }
    else
    {
        printf("[VISS-MODULE] Unable to create h3a stats object! \n");
        return status;
    }


    if(status == VX_SUCCESS)
    {
        img_width  = sensorObj->sensorParams.sensorInfo.raw_params.width;
        img_height = sensorObj->sensorParams.sensorInfo.raw_params.height;

        vx_image output_img = vxCreateImage(context, img_width, img_height, VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)output_img);
        if(status == VX_SUCCESS)
        {
            vissObj->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, num_cameras_enabled);
            vxReleaseImage(&output_img);

            status = vxGetStatus((vx_reference)vissObj->output_arr);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-MODULE] Unable to create VISS output image array! \n");
                return status;
            }
            else
            {
                vxSetReferenceName((vx_reference)vissObj->output_arr, "viss_node_output_arr");
            }
        }
        else
        {
            printf("[VISS-MODULE] Unable to create VISS output image! \n");
            return status;
        }

    }

    if(vissObj->en_out_viss_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, vissObj->output_file_path);
        vissObj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->file_path, "viss_write_node_file_path");
            vxAddArrayItems(vissObj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file path object for storing VISS outputs! \n");
        }


        strcpy(file_prefix, "viss_img_output");
        vissObj->img_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->img_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->img_file_prefix, "viss_write_node_img_file_prefix");
            vxAddArrayItems(vissObj->img_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file prefix object for storing VISS output image! \n");
        }

        strcpy(file_prefix, "viss_h3a_output");
        vissObj->h3a_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->h3a_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(vissObj->h3a_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file prefix object for storing VISS h3a stats! \n");
        }

        vissObj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)vissObj->write_cmd);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create fileio write cmd object for VISS node! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)vissObj->write_cmd, "viss_write_node_write_cmd");
        }
    }
    else
    {
        vissObj->file_path        = NULL;
        vissObj->img_file_prefix  = NULL;
        vissObj->h3a_file_prefix  = NULL;
        vissObj->img_write_node   = NULL;
        vissObj->h3a_write_node   = NULL;
        vissObj->write_cmd        = NULL;
    }

    return status;
}

static vx_status create_viss_outputs_ir(vx_context context, VISSObj *vissObj, SensorObj *sensorObj, uint32_t num_cameras_enabled)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 img_height, img_width;

    vx_user_data_object h3a_stats = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)h3a_stats);
    if(status == VX_SUCCESS)
    {
        vissObj->h3a_stats_arr = vxCreateObjectArray(context, (vx_reference)h3a_stats, num_cameras_enabled);
        vxReleaseUserDataObject(&h3a_stats);

        status = vxGetStatus((vx_reference)vissObj->h3a_stats_arr);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create h3a stats object array! \n");
            return status;
        }
        else
        {
            vxSetReferenceName((vx_reference)vissObj->h3a_stats_arr, "viss_node_h3a_stats_arr");
        }
    }
    else
    {
        printf("[VISS-IR-MODULE] Unable to create h3a stats object! \n");
        return status;
    }

    if(status == VX_SUCCESS)
    {
        img_width  = sensorObj->sensorParams.sensorInfo.raw_params.width;
        img_height = sensorObj->sensorParams.sensorInfo.raw_params.height;

        vx_image output_img = vxCreateImage(context, img_width, img_height, VX_DF_IMAGE_U8);
        status = vxGetStatus((vx_reference)output_img);
        if(status == VX_SUCCESS)
        {
            vissObj->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, num_cameras_enabled);
            vxReleaseImage(&output_img);

            status = vxGetStatus((vx_reference)vissObj->output_arr);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-IR-MODULE] Unable to create VISS output image array! \n");
                return status;
            }
            else
            {
                vxSetReferenceName((vx_reference)vissObj->output_arr, "viss_node_output_arr");
            }
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create VISS output image! \n");
            return status;
        }
    }

    if(vissObj->en_out_viss_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, vissObj->output_file_path);
        vissObj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->file_path, "viss_write_node_file_path");
            vxAddArrayItems(vissObj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file path object for storing VISS outputs! \n");
        }

        strcpy(file_prefix, "viss_img_output");
        vissObj->img_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->img_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->img_file_prefix, "viss_write_node_img_file_prefix");
            vxAddArrayItems(vissObj->img_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file prefix object for storing VISS output image! \n");
        }

        strcpy(file_prefix, "viss_h3a_output");
        vissObj->h3a_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)vissObj->h3a_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(vissObj->h3a_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file prefix object for storing VISS h3a stats! \n");
        }

        vissObj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)vissObj->write_cmd);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create fileio write cmd object for VISS node! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)vissObj->write_cmd, "viss_write_node_write_cmd");
        }
    }
    else
    {
        vissObj->file_path        = NULL;
        vissObj->img_file_prefix  = NULL;
        vissObj->h3a_file_prefix  = NULL;
        vissObj->img_write_node   = NULL;
        vissObj->h3a_write_node   = NULL;
        vissObj->write_cmd        = NULL;
    }

    return status;
}

static vx_status create_viss_outputs_q(vx_context context,
                                       VISSObj *vissObj,
                                       SensorObj *sensorObj,
                                       uint32_t num_cameras_enabled)
{
    vx_status status = VX_SUCCESS;

    vx_int32 q = 0;
    vx_int32 bufq_depth = vissObj->bufq_depth;

    vx_uint32 img_height = 0;
    vx_uint32 img_width = 0;

    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    if (bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    h3a_stats = vxCreateUserDataObject(context,
                                       "tivx_h3a_data_t",
                                       sizeof(tivx_h3a_data_t),
                                       NULL);

    status = vxGetStatus((vx_reference)h3a_stats);

    if (status != VX_SUCCESS)
    {
        printf("[VISS-MODULE] Unable to create h3a stats object!\n");
    }

    if (status == VX_SUCCESS)
    {
        for (q = 0; q < bufq_depth; q++)
        {
            vissObj->h3a_stats_arr_q[q] =
                vxCreateObjectArray(context,
                                    (vx_reference)h3a_stats,
                                    num_cameras_enabled);

            status = vxGetStatus((vx_reference)vissObj->h3a_stats_arr_q[q]);

            if (status == VX_SUCCESS)
            {
                vissObj->h3a_stats_q[q] =
                    (vx_user_data_object)vxGetObjectArrayItem(
                        (vx_object_array)vissObj->h3a_stats_arr_q[q],
                        0);

                status = vxGetStatus((vx_reference)vissObj->h3a_stats_q[q]);
            }

            if (status != VX_SUCCESS)
            {
                printf("[VISS-MODULE] Unable to create h3a stats object array!\n");
                break;
            }
        }
    }

    if (h3a_stats != NULL)
    {
        (void)vxReleaseUserDataObject(&h3a_stats);
    }

    if (status == VX_SUCCESS)
    {
        img_width  = sensorObj->sensorParams.sensorInfo.raw_params.width;
        img_height = sensorObj->sensorParams.sensorInfo.raw_params.height;

        output_img = vxCreateImage(context,
                                  img_width,
                                  img_height,
                                  VX_DF_IMAGE_NV12);

        status = vxGetStatus((vx_reference)output_img);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create VISS output image!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        vissObj->output_arr =
            vxCreateObjectArray(context,
                                (vx_reference)output_img,
                                num_cameras_enabled);

        status = vxGetStatus((vx_reference)vissObj->output_arr);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create VISS output image array!\n");
        }

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->output_arr,
                               "viss_node_output_arr");
        }
    }

    if (output_img != NULL)
    {
        (void)vxReleaseImage(&output_img);
    }

    return status;
}

static vx_status create_viss_outputs_ir_q(vx_context context,
                                          VISSObj *vissObj,
                                          SensorObj *sensorObj,
                                           uint32_t num_cameras_enabled)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 img_height = 0;
    vx_uint32 img_width = 0;
    vx_int32 q = 0;
    vx_int32 bufq_depth = vissObj->bufq_depth;

    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    if (bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    h3a_stats = vxCreateUserDataObject(context,
                                       "tivx_h3a_data_t",
                                       sizeof(tivx_h3a_data_t),
                                       NULL);

    status = vxGetStatus((vx_reference)h3a_stats);

    if (status != VX_SUCCESS)
    {
        printf("[VISS-IR-MODULE] Unable to create h3a stats object!\n");
    }

    if (status == VX_SUCCESS)
    {
        static char name[128];

        for (q = 0; q < bufq_depth; q++)
        {
            vissObj->h3a_stats_arr_q[q] =
                vxCreateObjectArray(context,
                                    (vx_reference)h3a_stats,
                                    num_cameras_enabled);

            status = vxGetStatus((vx_reference)vissObj->h3a_stats_arr_q[q]);

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "viss_h3a_stats_arr_q%d", q);
                vxSetReferenceName((vx_reference)vissObj->h3a_stats_arr_q[q], name);

                vissObj->h3a_stats_q[q] =
                    (vx_user_data_object)vxGetObjectArrayItem(
                        (vx_object_array)vissObj->h3a_stats_arr_q[q],
                        0);

                status = vxGetStatus((vx_reference)vissObj->h3a_stats_q[q]);
            }

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "viss_h3a_stats_q%d_cam0", q);
                vxSetReferenceName((vx_reference)vissObj->h3a_stats_q[q], name);
            }

            if (status != VX_SUCCESS)
            {
                printf("[VISS-IR-MODULE] Unable to create h3a stats object array!\n");
                break;
            }
        }
    }

    if (h3a_stats != NULL)
    {
        (void)vxReleaseUserDataObject(&h3a_stats);
    }

    if (status == VX_SUCCESS)
    {
        img_width  = sensorObj->sensorParams.sensorInfo.raw_params.width;
        img_height = sensorObj->sensorParams.sensorInfo.raw_params.height;

        output_img = vxCreateImage(context,
                                  img_width,
                                  img_height,
                                  VX_DF_IMAGE_U8);

        status = vxGetStatus((vx_reference)output_img);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create VISS output image!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        vissObj->output_arr =
            vxCreateObjectArray(context,
                                (vx_reference)output_img,
                                num_cameras_enabled);

        status = vxGetStatus((vx_reference)vissObj->output_arr);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create VISS output image array!\n");
        }

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->output_arr,
                               "viss_node_output_arr");
        }
    }

    if (output_img != NULL)
    {
        (void)vxReleaseImage(&output_img);
    }

    return status;
}

vx_status app_init_viss(vx_context context, VISSObj *vissObj, SensorObj *sensorObj,  char *objName, uint32_t num_cameras_enabled)
{
    vx_status status = VX_SUCCESS;
    if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI") == 0)
    {
        if(strcmp(objName,"viss_obj") == 0)
        {
            status = configure_viss_params(context, vissObj, sensorObj);
        }
        else if(strcmp(objName,"viss_obj1")==0)
        {
            status = configure_viss_params_ir(context, vissObj, sensorObj);
        }
    } else {
        status = configure_viss_params(context, vissObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configure_dcc_params(context, vissObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI")==0)
        {
            if(strcmp(objName,"viss_obj")==0)
            {
                status = create_viss_outputs(context, vissObj, sensorObj, num_cameras_enabled);
            }
            else if(strcmp(objName,"viss_obj1")==0)
            {
                status = create_viss_outputs_ir(context, vissObj, sensorObj, num_cameras_enabled);
            }
        }
        else
        {
            status = create_viss_outputs(context, vissObj, sensorObj, num_cameras_enabled);
        }
    }

    return (status);
}

void app_deinit_viss(VISSObj *vissObj)
{
    vxReleaseUserDataObject(&vissObj->config);
    vxReleaseObjectArray(&vissObj->output_arr);
    vxReleaseObjectArray(&vissObj->h3a_stats_arr);

    if(vissObj->dcc_config != NULL)
    {
        vxReleaseUserDataObject(&vissObj->dcc_config);
    }

    if(vissObj->en_out_viss_write == 1)
    {
        vxReleaseArray(&vissObj->file_path);
        vxReleaseArray(&vissObj->img_file_prefix);
        vxReleaseArray(&vissObj->h3a_file_prefix);
        vxReleaseUserDataObject(&vissObj->write_cmd);
    }
}

void app_delete_viss(VISSObj *vissObj)
{
    if(vissObj->node != NULL)
    {
        vxReleaseNode(&vissObj->node);
    }

    if(vissObj->img_write_node != NULL)
    {
        vxReleaseNode(&vissObj->img_write_node);
    }

    if(vissObj->h3a_write_node != NULL)
    {
        vxReleaseNode(&vissObj->h3a_write_node);
    }
}

vx_status app_create_graph_viss(vx_graph graph, VISSObj *vissObj, vx_object_array raw_image_arr, const char *target)
{
    vx_status status = VX_SUCCESS;

    tivx_raw_image raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
    vx_user_data_object h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(vissObj->h3a_stats_arr, 0);
    vx_image output_img = (vx_image)vxGetObjectArrayItem(vissObj->output_arr, 0);

    vissObj->node = tivxVpacVissNode(
                                graph,
                                vissObj->config,
                                NULL,
                                vissObj->dcc_config,
                                raw_image, NULL, NULL,
                                output_img, NULL, NULL,
                                h3a_stats, NULL, NULL, NULL);

    tivxReleaseRawImage(&raw_image);
    vxReleaseImage(&output_img);
    vxReleaseUserDataObject(&h3a_stats);

    status = vxGetStatus((vx_reference)vissObj->node);
    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)vissObj->node, "viss_node");
        vxSetNodeTarget(vissObj->node, VX_TARGET_STRING, target);

        vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, vissObj->node, replicate, 13);

        if(vissObj->en_out_viss_write == 1)
        {
            status = app_create_graph_viss_write_output(graph, vissObj);
        }
    }
    else
    {
        printf("[VISS-MODULE] Unable to create VISS Node! \n");
    }

    return status;
}

vx_status app_create_graph_viss_write_output(vx_graph graph, VISSObj *vissObj)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(vissObj->output_arr, 0);
    vissObj->img_write_node = tivxWriteImageNode(graph, output_img, vissObj->file_path, vissObj->img_file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)vissObj->img_write_node);
    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)vissObj->img_write_node, "viss_img_write_node");
        vxSetNodeTarget(vissObj->img_write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, vissObj->img_write_node, replicate, 3);
    }
    else
    {
        printf("[VISS-MODULE] Unable to create node to write VISS output! \n");
    }

    if(status == VX_SUCCESS)
    {
        vx_user_data_object output_h3a = (vx_user_data_object)vxGetObjectArrayItem(vissObj->h3a_stats_arr, 0);

        vissObj->h3a_write_node = tivxWriteUserDataObjectNode(graph, output_h3a, vissObj->file_path, vissObj->h3a_file_prefix);
        vxReleaseUserDataObject(&output_h3a);

        status = vxGetStatus((vx_reference)vissObj->h3a_write_node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)vissObj->h3a_write_node, "viss_h3a_write_node");
            vxSetNodeTarget(vissObj->h3a_write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, vissObj->h3a_write_node, replicate, 3);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create node to write H3A stats! \n");
        }
    }

    return (status);
}

vx_status app_send_cmd_viss_write_node(VISSObj *vissObj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(vissObj->write_cmd, 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if(status == VX_SUCCESS)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)vissObj->write_cmd;

        status = tivxNodeSendCommand(vissObj->img_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                 TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                 refs, 1u);

        if(VX_SUCCESS != status)
        {
            printf("VISS Img Write Node send command failed!\n");
        }

        APP_PRINTF("VISS Img Write node send command success!\n");
    }
    if(status == VX_SUCCESS)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)vissObj->write_cmd;

        status = tivxNodeSendCommand(vissObj->h3a_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                 TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                 refs, 1u);

        if(VX_SUCCESS != status)
        {
            printf("VISS H3A Write Node send command failed!\n");
        }

        APP_PRINTF("VISS H3A Write node send command success!\n");
    }
    return (status);
}

vx_status app_init_viss_queued(vx_context context,
                              VISSObj *vissObj,
                              SensorObj *sensorObj,
                              char *objName,
                              uint32_t num_cameras_enabled,
                              vx_uint32 bufq_depth)
{
    vx_status status = VX_SUCCESS;
    vx_bool is_ov2312 = vx_false_e;
    vx_bool is_viss_obj = vx_false_e;
    vx_bool is_viss_obj1 = vx_false_e;

    if (vissObj == NULL)
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        if (sensorObj == NULL)
        {
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (objName == NULL)
        {
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
        {
            printf("[VISS-MODULE] Warning: bufq_depth %u exceeds maximum %u, limiting\n",
                   (uint32_t)bufq_depth, (uint32_t)APP_MODULES_MAX_BUFQ_DEPTH);
            bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
        }

        vissObj->bufq_depth = (vx_int32)bufq_depth;

        if (strcmp(sensorObj->sensor_name, "OV2312-UB953_LI") == 0)
        {
            is_ov2312 = vx_true_e;
        }

        if (strcmp(objName, "viss_obj") == 0)
        {
            is_viss_obj = vx_true_e;
        }

        if (strcmp(objName, "viss_obj1") == 0)
        {
            is_viss_obj1 = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (is_ov2312 == vx_true_e)
        {
            if (is_viss_obj == vx_true_e)
            {
                status = configure_viss_params(context, vissObj, sensorObj);
            }
            else if (is_viss_obj1 == vx_true_e)
            {
                status = configure_viss_params_ir(context, vissObj, sensorObj);
            }
            else
            {
                status = configure_viss_params(context, vissObj, sensorObj);
            }
        }
        else
        {
            status = configure_viss_params(context, vissObj, sensorObj);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = configure_dcc_params(context, vissObj, sensorObj);
    }

    if (status == VX_SUCCESS)
    {
        if (is_ov2312 == vx_true_e)
        {
            if (is_viss_obj == vx_true_e)
            {
                status = create_viss_outputs_q(context,
                                               vissObj,
                                               sensorObj,
                                               num_cameras_enabled);
            }
            else if (is_viss_obj1 == vx_true_e)
            {
                status = create_viss_outputs_ir_q(context,
                                                  vissObj,
                                                  sensorObj,
                                                  num_cameras_enabled);
            }
            else
            {
                status = create_viss_outputs_q(context,
                                               vissObj,
                                               sensorObj,
                                               num_cameras_enabled);
            }
        }
        else
        {
            status = create_viss_outputs_q(context,
                                           vissObj,
                                           sensorObj,
                                           num_cameras_enabled);
        }
    }

    return status;
}

void app_deinit_viss_queued(VISSObj *vissObj)
{
    vx_int32 q = 0;

    vx_int32 bufq_depth = vissObj->bufq_depth;

    if (bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        printf("[VISS-MODULE] Warning: bufq_depth %u exceeds maximum %u, limiting\n",
                (uint32_t)bufq_depth, (uint32_t)APP_MODULES_MAX_BUFQ_DEPTH);
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    if (vissObj == NULL)
    {
        return;
    }

    if (vissObj->config != NULL)
    {
        (void)vxReleaseUserDataObject(&vissObj->config);
    }

    if (vissObj->output_arr != NULL)
    {
        (void)vxReleaseObjectArray(&vissObj->output_arr);
    }

    for (q = 0; q < bufq_depth; q++)
    {
        if (vissObj->h3a_stats_arr_q[q] != NULL)
        {
            (void)vxReleaseObjectArray(&vissObj->h3a_stats_arr_q[q]);
        }

        if (vissObj->h3a_stats_q[q] != NULL)
        {
            (void)vxReleaseUserDataObject(&vissObj->h3a_stats_q[q]);
        }
    }

    if (vissObj->dcc_config != NULL)
    {
        (void)vxReleaseUserDataObject(&vissObj->dcc_config);
    }
}


vx_status app_create_graph_viss_queued(vx_graph graph,
                                      VISSObj *vissObj,
                                      vx_object_array raw_image_arr,
                                      const char *target)
{
    vx_status status = VX_SUCCESS;

    tivx_raw_image raw_image = NULL;
    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
    status = vxGetStatus((vx_reference)raw_image);

    if (status == VX_SUCCESS)
    {
        h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(vissObj->h3a_stats_arr_q[0], 0);
        status = vxGetStatus((vx_reference)h3a_stats);
    }

    if (status == VX_SUCCESS)
    {
        output_img = (vx_image)vxGetObjectArrayItem(vissObj->output_arr, 0);
        status = vxGetStatus((vx_reference)output_img);
    }

    if (status == VX_SUCCESS)
    {
        vissObj->node = tivxVpacVissNode(graph,
                                        vissObj->config,
                                        NULL,
                                        vissObj->dcc_config,
                                        raw_image, NULL, NULL,
                                        output_img, NULL, NULL,
                                        h3a_stats, NULL, NULL, NULL);

        status = vxGetStatus((vx_reference)vissObj->node);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create VISS Node!\n");
        }
    }

    if (raw_image != NULL)
    {
        (void)tivxReleaseRawImage(&raw_image);
    }

    if (output_img != NULL)
    {
        (void)vxReleaseImage(&output_img);
    }

    if (h3a_stats != NULL)
    {
        (void)vxReleaseUserDataObject(&h3a_stats);
    }

    if (status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)vissObj->node, "viss_node");

        status = vxSetNodeTarget(vissObj->node,
                                 VX_TARGET_STRING,
                                 target);
    }

    if (status == VX_SUCCESS)
    {
        vx_bool replicate[] =
        {
            vx_false_e, vx_false_e, vx_false_e,
            vx_true_e,
            vx_false_e, vx_false_e,
            vx_true_e,
            vx_false_e, vx_false_e,
            vx_true_e,
            vx_false_e, vx_false_e, vx_false_e
        };

        status = vxReplicateNode(graph, vissObj->node, replicate, 13);
    }

    return status;
}
