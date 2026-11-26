/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
 *
 * All rights not granted herein.
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
 *       permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 *       licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
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
 */

#include "app_viss_module.hpp"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "app_sensor_module.hpp"
#include "app_capture_module.hpp"

VISSObj::VISSObj() { setDefaults(); }

void VISSObj::setDefaults()
{
    /* Initialize VISS parameters */
    memset(&this->params, 0, sizeof(tivx_vpac_viss_params_t));

    /* Initialize arrays and objects */
    this->node = NULL;
    this->config = NULL;
    this->dcc_config = NULL;
    this->output_arr = NULL;
    this->h3a_stats_arr = NULL;
    this->img_write_node = NULL;
    this->h3a_write_node = NULL;
    this->file_path = NULL;
    this->img_file_prefix = NULL;
    this->h3a_file_prefix = NULL;
    this->write_cmd = NULL;
    memset(this->output_file_path, 0, sizeof(this->output_file_path));
    memset(this->obj_name, 0, sizeof(this->obj_name));
    memset(this->objName, 0, sizeof(this->objName));
    memset(this->target, 0, sizeof(this->target));
    this->num_cameras_enabled = 0;

    /* Initialize queued mode arrays */
    for (int i = 0; i < APP_MODULES_MAX_BUFQ_DEPTH; i++) {
        this->h3a_stats_arr_q[i] = NULL;
        this->h3a_stats_q[i] = NULL;
    }

    /* Initialize counters and flags */
    this->en_out_viss_write = 0;
    this->graph_parameter_index = 0;
    this->bufq_depth = 0;
}

vx_status VISSObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_context context = graph->context;

    /* Get the sensor object from the graph */
    AppModuleObj* sensor_module_obj = graph->getModuleByType(AppModuleType::Sensor);
    if (!sensor_module_obj || !sensor_module_obj->module_obj) {
        printf("[VISS-MODULE] Error: Sensor module not found in graph!\n");
        return VX_FAILURE;
    }

    SensorObj* sensorObj = static_cast<SensorObj*>(sensor_module_obj->module_obj);
    if (!sensorObj) {
        printf("[VISS-MODULE] Error: Invalid sensor module object!\n");
        return VX_FAILURE;
    }

    /* Configure VISS parameters based on sensor and object name */
    if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI") == 0)
    {
        if(strcmp(this->objName,"viss_obj") == 0)
        {
            status = this->configureParams(context, sensorObj);
        }
        else if(strcmp(this->objName,"viss_obj1")==0)
        {
            status = this->configureParamsIR(context, sensorObj);
        }
    } else {
        status = this->configureParams(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = this->configureDccParams(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI")==0)
        {
            if(strcmp(this->objName,"viss_obj")==0)
            {
                status = this->createOutputs(context, sensorObj);
            }
            else if(strcmp(this->objName,"viss_obj1")==0)
            {
                status = this->createOutputsIR(context, sensorObj);
            }
        }
        else
        {
            status = this->createOutputs(context, sensorObj);
        }
    }

    return status;
}

vx_status VISSObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    vxReleaseUserDataObject(&this->config);
    vxReleaseObjectArray(&this->output_arr);
    vxReleaseObjectArray(&this->h3a_stats_arr);

    if(this->dcc_config != NULL)
    {
        vxReleaseUserDataObject(&this->dcc_config);
    }

    if(this->en_out_viss_write == 1)
    {
        vxReleaseArray(&this->file_path);
        vxReleaseArray(&this->img_file_prefix);
        vxReleaseArray(&this->h3a_file_prefix);
        vxReleaseUserDataObject(&this->write_cmd);
    }

    return VX_SUCCESS;
}

vx_status VISSObj::createNode(AppGraphObj *graph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_graph graph_obj = graph->graph;

    /* Get the capture module to access raw_image_arr */
    AppModuleObj* capture_module_obj = graph->getModuleByType(AppModuleType::Capture);
    if (!capture_module_obj || !capture_module_obj->module_obj) {
        printf("[VISS-MODULE] Error: Capture module not found in graph!\n");
        return VX_FAILURE;
    }

    CaptureObj* captureObj = static_cast<CaptureObj*>(capture_module_obj->module_obj);
    if (!captureObj || !captureObj->raw_image_arr[0]) {
        printf("[VISS-MODULE] Error: Invalid capture module object or raw_image_arr!\n");
        return VX_FAILURE;
    }

    /* Create VISS node */
    tivx_raw_image raw_image = (tivx_raw_image)vxGetObjectArrayItem(captureObj->raw_image_arr[0], 0);
    vx_user_data_object h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(this->h3a_stats_arr, 0);
    vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);

    this->node = tivxVpacVissNode(
                                graph_obj,
                                this->config,
                                NULL,
                                this->dcc_config,
                                raw_image, NULL, NULL,
                                output_img, NULL, NULL,
                                h3a_stats, NULL, NULL, NULL);

    tivxReleaseRawImage(&raw_image);
    vxReleaseImage(&output_img);
    vxReleaseUserDataObject(&h3a_stats);

    status = vxGetStatus((vx_reference)this->node);
    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->node, "viss_node");
        vxSetNodeTarget(this->node, VX_TARGET_STRING, this->target);

        vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph_obj, this->node, replicate, 13);

        /* Create write nodes if enabled */
        if(this->en_out_viss_write == 1)
        {
            status = this->createGraphWriteOutput(graph_obj);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-MODULE] Unable to create VISS write output nodes!\n");
            }
        }
    }
    else
    {
        printf("[VISS-MODULE] Unable to create VISS Node! \n");
    }

    return status;
}

vx_status VISSObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    if(this->img_write_node != NULL)
    {
        vxReleaseNode(&this->img_write_node);
    }
    if(this->h3a_write_node != NULL)
    {
        vxReleaseNode(&this->h3a_write_node);
    }

    return VX_SUCCESS;
}

/*==========================================================================*/
/* VISS MODULE HELPER FUNCTIONS                                             */
/*==========================================================================*/

vx_status VISSObj::configureParams(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_viss_params_init(&this->params);

    this->params.sensor_dcc_id    = sensorObj->sensorParams.dccId;
    this->params.fcp[0].ee_mode          = 0;
    this->params.fcp[0].mux_output0      = 0;
    this->params.fcp[0].mux_output1      = 0;
    this->params.fcp[0].mux_output2      = 4;
    this->params.fcp[0].mux_output3      = 0;
    this->params.fcp[0].mux_output4      = 3;
    if (strcmp(sensorObj->sensor_name,"OV2312-UB953_LI")==0){
    #if defined (VPAC3L)
        this->params.bypass_pcid = 0;
    #endif
        this->params.h3a_in      = 4;
    }else{
        this->params.h3a_in      = 3;
    }
    this->params.h3a_aewb_af_mode        = 0;
    this->params.fcp[0].chroma_mode      = 0;
    this->params.bypass_nsf4      = 0;
    this->params.enable_ctx       = 1;
    if(sensorObj->sensor_wdr_enabled == 1)
    {
        this->params.bypass_glbce = 0;
    }
    else
    {
        this->params.bypass_glbce = 1;
    }

    this->config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &this->params);
    status = vxGetStatus((vx_reference)this->config);

    if(status != VX_SUCCESS)
    {
        printf("[VISS-MODULE] Unable to create VISS config object! \n");
    }
    else
    {
        vxSetReferenceName((vx_reference)this->config, "viss_node_config");
    }

    return status;
}

vx_status VISSObj::configureParamsIR(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_viss_params_init(&this->params);

    #if defined (VPAC3L)
        this->params.bypass_pcid      = 0;
        this->params.enable_ir_op     = TIVX_VPAC_VISS_IR_ENABLE;
        this->params.enable_bayer_op  = TIVX_VPAC_VISS_BAYER_DISABLE;
    #endif
    this->params.sensor_dcc_id    = sensorObj->sensorParams.dccId;
    this->params.fcp[0].ee_mode          = TIVX_VPAC_VISS_EE_MODE_OFF;
    #if defined (VPAC3L)
        this->params.fcp[0].mux_output0      = TIVX_VPAC_VISS_MUX0_IR8;
    #endif
    this->params.fcp[0].mux_output1      = 0;
    this->params.fcp[0].mux_output2      = 0;
    this->params.fcp[0].mux_output3      = 0;
    this->params.fcp[0].mux_output4      = 3;
    this->params.h3a_in           = TIVX_VPAC_VISS_H3A_IN_LSC;
    this->params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
    this->params.fcp[0].chroma_mode      = TIVX_VPAC_VISS_CHROMA_MODE_420;
    this->params.bypass_nsf4      = 0;
    this->params.enable_ctx       = 1;
    if(sensorObj->sensor_wdr_enabled == 1)
    {
        this->params.bypass_glbce = 0;
    }
    else
    {
        this->params.bypass_glbce = 1;
    }

    this->config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &this->params);
    status = vxGetStatus((vx_reference)this->config);

    if(status != VX_SUCCESS)
    {
        printf("[VISS-IR-MODULE] Unable to create VISS config object! \n");
    }
    else
    {
        vxSetReferenceName((vx_reference)this->config, "viss_ir_node_config");
    }

    return status;
}

vx_status VISSObj::configureDccParams(vx_context context, SensorObj *sensorObj)
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

        this->dcc_config = vxCreateUserDataObject(context, "dcc_viss", dcc_buff_size, NULL);
        status = vxGetStatus((vx_reference)this->dcc_config);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->dcc_config, "viss_node_dcc_config");

            vxMapUserDataObject(
                    this->dcc_config,
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

            vxUnmapUserDataObject(this->dcc_config, dcc_buf_map_id);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create DCC config object! \n");
        }
    }
    else
    {
        this->dcc_config = NULL;
    }

    return status;
}

vx_status VISSObj::createOutputs(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 img_height, img_width;

    /* Create h3a_aew_af output buffer (uninitialized) */
    vx_user_data_object h3a_stats = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)h3a_stats);
    if(status == VX_SUCCESS)
    {
        this->h3a_stats_arr = vxCreateObjectArray(context, (vx_reference)h3a_stats, this->num_cameras_enabled);
        vxReleaseUserDataObject(&h3a_stats);

        status = vxGetStatus((vx_reference)this->h3a_stats_arr);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create h3a stats object array! \n");
            return status;
        }
        else
        {
            vxSetReferenceName((vx_reference)this->h3a_stats_arr, "viss_node_h3a_stats_arr");
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
            this->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, this->num_cameras_enabled);
            vxReleaseImage(&output_img);

            status = vxGetStatus((vx_reference)this->output_arr);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-MODULE] Unable to create VISS output image array! \n");
                return status;
            }
            else
            {
                vxSetReferenceName((vx_reference)this->output_arr, "viss_node_output_arr");
            }
        }
        else
        {
            printf("[VISS-MODULE] Unable to create VISS output image! \n");
            return status;
        }
    }

    if(this->en_out_viss_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, this->output_file_path);
        this->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)this->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->file_path, "viss_write_node_file_path");
            vxAddArrayItems(this->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file path object for storing VISS outputs! \n");
        }


        strcpy(file_prefix, "viss_img_output");
        this->img_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)this->img_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->img_file_prefix, "viss_write_node_img_file_prefix");
            vxAddArrayItems(this->img_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file prefix object for storing VISS output image! \n");
        }

        strcpy(file_prefix, "viss_h3a_output");
        this->h3a_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)this->h3a_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(this->h3a_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create file prefix object for storing VISS h3a stats! \n");
        }

        this->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)this->write_cmd);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create fileio write cmd object for VISS node! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->write_cmd, "viss_write_node_write_cmd");
        }
    }
    else
    {
        this->file_path        = NULL;
        this->img_file_prefix  = NULL;
        this->h3a_file_prefix  = NULL;
        this->img_write_node   = NULL;
        this->h3a_write_node   = NULL;
        this->write_cmd        = NULL;
    }

    return status;
}

vx_status VISSObj::createOutputsIR(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 img_height, img_width;

    vx_user_data_object h3a_stats = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)h3a_stats);
    if(status == VX_SUCCESS)
    {
        this->h3a_stats_arr = vxCreateObjectArray(context, (vx_reference)h3a_stats, this->num_cameras_enabled);
        vxReleaseUserDataObject(&h3a_stats);

        status = vxGetStatus((vx_reference)this->h3a_stats_arr);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create h3a stats object array! \n");
            return status;
        }
        else
        {
            vxSetReferenceName((vx_reference)this->h3a_stats_arr, "viss_node_h3a_stats_arr");
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
            this->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, this->num_cameras_enabled);
            vxReleaseImage(&output_img);

            status = vxGetStatus((vx_reference)this->output_arr);
            if(status != VX_SUCCESS)
            {
                printf("[VISS-IR-MODULE] Unable to create VISS output image array! \n");
                return status;
            }
            else
            {
                vxSetReferenceName((vx_reference)this->output_arr, "viss_node_output_arr");
            }
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create VISS output image! \n");
            return status;
        }
    }

    if(this->en_out_viss_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, this->output_file_path);
        this->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)this->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->file_path, "viss_write_node_file_path");
            vxAddArrayItems(this->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file path object for storing VISS outputs! \n");
        }

        strcpy(file_prefix, "viss_img_output");
        this->img_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)this->img_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->img_file_prefix, "viss_write_node_img_file_prefix");
            vxAddArrayItems(this->img_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file prefix object for storing VISS output image! \n");
        }

        strcpy(file_prefix, "viss_h3a_output");
        this->h3a_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)this->h3a_file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(this->h3a_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[VISS-IR-MODULE] Unable to create file prefix object for storing VISS h3a stats! \n");
        }

        this->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)this->write_cmd);
        if(status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create fileio write cmd object for VISS node! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->write_cmd, "viss_write_node_write_cmd");
        }
    }
    else
    {
        this->file_path        = NULL;
        this->img_file_prefix  = NULL;
        this->h3a_file_prefix  = NULL;
        this->img_write_node   = NULL;
        this->h3a_write_node   = NULL;
        this->write_cmd        = NULL;
    }

    return status;
}

vx_status VISSObj::createOutputsQueued(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 q = 0;
    vx_uint32 img_width = 0;
    vx_uint32 img_height = 0;

    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    if (this->bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        this->bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
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
        for (q = 0; q < this->bufq_depth; q++)
        {
            this->h3a_stats_arr_q[q] =
                vxCreateObjectArray(context,
                                    (vx_reference)h3a_stats,
                                    this->num_cameras_enabled);

            status = vxGetStatus((vx_reference)this->h3a_stats_arr_q[q]);

            if (status == VX_SUCCESS)
            {
                this->h3a_stats_q[q] =
                    (vx_user_data_object)vxGetObjectArrayItem(
                        (vx_object_array)this->h3a_stats_arr_q[q],
                        0);

                status = vxGetStatus((vx_reference)this->h3a_stats_q[q]);
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
        this->output_arr =
            vxCreateObjectArray(context,
                                (vx_reference)output_img,
                                this->num_cameras_enabled);

        status = vxGetStatus((vx_reference)this->output_arr);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-MODULE] Unable to create VISS output image array!\n");
        }

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->output_arr,
                               "viss_node_output_arr");
        }
    }

    if (output_img != NULL)
    {
        (void)vxReleaseImage(&output_img);
    }

    return status;
}

vx_status VISSObj::createOutputsIRQueued(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 img_height = 0;
    vx_uint32 img_width = 0;
    vx_int32 q = 0;
    vx_char name[VX_MAX_REFERENCE_NAME];

    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    if (this->bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        this->bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
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
        for (q = 0; q < this->bufq_depth; q++)
        {
            this->h3a_stats_arr_q[q] =
                vxCreateObjectArray(context,
                                    (vx_reference)h3a_stats,
                                    this->num_cameras_enabled);

            status = vxGetStatus((vx_reference)this->h3a_stats_arr_q[q]);

            if (status == VX_SUCCESS)
            {
                snprintf(name, sizeof(name), "viss_h3a_stats_arr_q%d", q);
                vxSetReferenceName((vx_reference)this->h3a_stats_arr_q[q], name);

                this->h3a_stats_q[q] =
                    (vx_user_data_object)vxGetObjectArrayItem(
                        (vx_object_array)this->h3a_stats_arr_q[q],
                        0);

                status = vxGetStatus((vx_reference)this->h3a_stats_q[q]);
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
        this->output_arr =
            vxCreateObjectArray(context,
                                (vx_reference)output_img,
                                this->num_cameras_enabled);

        status = vxGetStatus((vx_reference)this->output_arr);

        if (status != VX_SUCCESS)
        {
            printf("[VISS-IR-MODULE] Unable to create VISS output image array!\n");
        }

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->output_arr,
                               "viss_node_output_arr");
        }
    }

    if (output_img != NULL)
    {
        (void)vxReleaseImage(&output_img);
    }

    return status;
}

vx_status VISSObj::createGraphWriteOutput(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);
    this->img_write_node = tivxWriteImageNode(graph, output_img, this->file_path, this->img_file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)this->img_write_node);
    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->img_write_node, "viss_img_write_node");
        vxSetNodeTarget(this->img_write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, this->img_write_node, replicate, 3);
    }
    else
    {
        printf("[VISS-MODULE] Unable to create node to write VISS output! \n");
    }

    if(status == VX_SUCCESS)
    {
        vx_user_data_object output_h3a = (vx_user_data_object)vxGetObjectArrayItem(this->h3a_stats_arr, 0);
        this->h3a_write_node = tivxWriteUserDataObjectNode(graph, output_h3a, this->file_path, this->h3a_file_prefix);
        vxReleaseUserDataObject(&output_h3a);

        status = vxGetStatus((vx_reference)this->h3a_write_node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->h3a_write_node, "viss_h3a_write_node");
            vxSetNodeTarget(this->h3a_write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, this->h3a_write_node, replicate, 3);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create node to write H3A stats! \n");
        }
    }

    return (status);
}

vx_status VISSObj::sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(this->write_cmd, 0, sizeof(tivxFileIOWriteCmd),
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if(status == VX_SUCCESS)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)this->write_cmd;

        status = tivxNodeSendCommand(this->img_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
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

        refs[0] = (vx_reference)this->write_cmd;

        status = tivxNodeSendCommand(this->h3a_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
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

vx_status VISSObj::initQueued(vx_context context, SensorObj *sensorObj, char *objName)
{
    vx_status status = VX_SUCCESS;
    vx_bool is_ov2312 = vx_false_e;
    vx_bool is_viss_obj = vx_false_e;
    vx_bool is_viss_obj1 = vx_false_e;

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
        if (this->bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
        {
            printf("[VISS-MODULE] Warning: this->bufq_depth %u exceeds maximum %u, limiting\n",
                   (uint32_t)this->bufq_depth, (uint32_t)APP_MODULES_MAX_BUFQ_DEPTH);
            this->bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (strcmp(sensorObj->sensor_name, "OV2312-UB953_LI") == 0)
        {
            is_ov2312 = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        if (strcmp(objName, "viss_obj") == 0)
        {
            is_viss_obj = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
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
                status = this->configureParams(context, sensorObj);
            }
            else if (is_viss_obj1 == vx_true_e)
            {
                status = this->configureParamsIR(context, sensorObj);
            }
            else
            {
                status = this->configureParams(context, sensorObj);
            }
        }
        else
        {
            status = this->configureParams(context, sensorObj);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = this->configureDccParams(context, sensorObj);
    }

    if (status == VX_SUCCESS)
    {
        if (is_ov2312 == vx_true_e)
        {
            if (is_viss_obj == vx_true_e)
            {
                status = this->createOutputsQueued(context, sensorObj);
            }
            else if (is_viss_obj1 == vx_true_e)
            {
                status = this->createOutputsIRQueued(context, sensorObj);
            }
            else
            {
                status = this->createOutputsQueued(context, sensorObj);
            }
        }
        else
        {
            status = this->createOutputsQueued(context, sensorObj);
        }
    }

    return status;
}

void VISSObj::deinitQueued()
{
    vx_uint32 q = 0;

    if (this->bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        printf("[VISS-MODULE] Warning: bufq_depth %u exceeds maximum %u, limiting\n",
               (uint32_t)this->bufq_depth, (uint32_t)APP_MODULES_MAX_BUFQ_DEPTH);
        this->bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    if (this->config != NULL)
    {
        (void)vxReleaseUserDataObject(&this->config);
    }

    if (this->output_arr != NULL)
    {
        (void)vxReleaseObjectArray(&this->output_arr);
    }

    for (q = 0; q < this->bufq_depth; q++)
    {
        if (this->h3a_stats_arr_q[q] != NULL)
        {
            (void)vxReleaseObjectArray(&this->h3a_stats_arr_q[q]);
        }

        if (this->h3a_stats_q[q] != NULL)
        {
            (void)vxReleaseUserDataObject(&this->h3a_stats_q[q]);
        }
    }

    if (this->dcc_config != NULL)
    {
        (void)vxReleaseUserDataObject(&this->dcc_config);
    }
}

vx_status VISSObj::createGraphQueued(vx_graph graph, vx_object_array raw_image_arr, const char *target)
{
    vx_status status = VX_SUCCESS;

    tivx_raw_image raw_image = NULL;
    vx_user_data_object h3a_stats = NULL;
    vx_image output_img = NULL;

    raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
    status = vxGetStatus((vx_reference)raw_image);

    if (status == VX_SUCCESS)
    {
        h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(this->h3a_stats_arr_q[0], 0);
        status = vxGetStatus((vx_reference)h3a_stats);
    }

    if (status == VX_SUCCESS)
    {
        output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);
        status = vxGetStatus((vx_reference)output_img);
    }

    if (status == VX_SUCCESS)
    {
        this->node = tivxVpacVissNode(graph,
                                        this->config,
                                        NULL,
                                        this->dcc_config,
                                        raw_image, NULL, NULL,
                                        output_img, NULL, NULL,
                                        h3a_stats, NULL, NULL, NULL);

        status = vxGetStatus((vx_reference)this->node);

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
        vxSetReferenceName((vx_reference)this->node, "viss_node");

        status = vxSetNodeTarget(this->node,
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

        status = vxReplicateNode(graph, this->node, replicate, 13);
    }

    return status;
}

