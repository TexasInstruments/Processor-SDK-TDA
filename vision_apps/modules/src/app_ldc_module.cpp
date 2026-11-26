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
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
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

#include "app_ldc_module.hpp"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "ldc_lut_1920x1080.h"

// Define the static const member - shared by all LDC instances
const uint8_t LDCObj::ldc_lut[] = LDC_LUT_1920_1080;

LDCObj::LDCObj() { setDefaults(); }

void LDCObj::setDefaults()
{
    /* Initialize LDC parameters */
    memset(&this->params, 0, sizeof(tivx_vpac_ldc_params_t));
    memset(&this->mesh_params, 0, sizeof(tivx_vpac_ldc_mesh_params_t));
    memset(&this->region_params, 0, sizeof(tivx_vpac_ldc_region_params_t));

    /* Initialize objects and arrays */
    this->node = NULL;
    this->mesh_img = NULL;
    this->config = NULL;
    this->mesh_config = NULL;
    this->region_config = NULL;
    this->dcc_config = NULL;
    this->output_arr = NULL;
    this->write_node = NULL;
    this->file_path = NULL;
    this->file_prefix = NULL;
    this->write_cmd = NULL;
    memset(this->objName, 0, sizeof(this->objName));
    memset(this->output_file_path, 0, sizeof(this->output_file_path));

    /* Default LDC parameters */
    this->table_width     = LDC_TABLE_WIDTH;
    this->table_height    = LDC_TABLE_HEIGHT;
    this->ds_factor       = LDC_DS_FACTOR;
    this->en_out_ldc_write = 0;

    /* Initialize counters and flags */
    this->num_cameras_enabled = 0;
    this->input_arr = NULL;
    memset(this->target, 0, sizeof(this->target));
}

vx_status LDCObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_context context = graph->context;

    /* Get sensor object from graph */
    AppModuleObj* sensor_module_obj = graph->getModuleByType(AppModuleType::Sensor);
    if (!sensor_module_obj || !sensor_module_obj->module_obj) {
        printf("[LDC_MODULE] Sensor object not found\n");
        return VX_FAILURE;
    }

    SensorObj *sensorObj = static_cast<SensorObj*>(sensor_module_obj->module_obj);

    /* Execute initialization sequence */
    return initLdc(context, sensorObj);
}

vx_status LDCObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    deinitLdc();
    return VX_SUCCESS;
}

vx_status LDCObj::createNode(AppGraphObj *graph, vx_int32 module_index)
{
    return createGraphLdc(graph->graph, this->input_arr, this->target);
}

vx_status LDCObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    deleteLdc();
    return VX_SUCCESS;
}

/*==========================================================================*/
/* LDC MODULE PRIVATE HELPER FUNCTIONS                                     */
/*==========================================================================*/

vx_status LDCObj::configureDccParams(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if(sensorObj->sensor_dcc_enabled)
    {
        int32_t dcc_buff_size;
        uint8_t * dcc_buf;
        vx_map_id dcc_buf_map_id;

        dcc_buff_size = appIssGetDCCSizeLDC(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled);

        if (dcc_buff_size > 0)
        {
            this->dcc_config = vxCreateUserDataObject(context, "dcc_ldc", dcc_buff_size, NULL );
            status = vxGetStatus((vx_reference)this->dcc_config);

            if(status == VX_SUCCESS)
            {
                vxSetReferenceName((vx_reference)this->dcc_config, "ldc_node_dcc_config");

                vxMapUserDataObject(
                        this->dcc_config, 0,
                        dcc_buff_size,
                        &dcc_buf_map_id,
                        (void **)&dcc_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST, 0);

                status = appIssGetDCCBuffLDC(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled,  dcc_buf, dcc_buff_size);
                if(status != VX_SUCCESS)
                {
                    printf("[LDC-MODULE] Error getting DCC buffer \n");
                }
                vxUnmapUserDataObject(this->dcc_config, dcc_buf_map_id);
            }
            else
            {
                printf("[LDC-MODULE] Unable to create DCC config object! \n");
            }
        }
    }
    else
    {
        this->dcc_config = NULL;
    }

    return status;
}

vx_status LDCObj::configureMeshParams(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 table_width_ds, table_height_ds;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;

    table_width_ds = (((this->table_width / (1 << this->ds_factor)) + 1u) + 15u) & (~15u);
    table_height_ds = ((this->table_height / (1 << this->ds_factor)) + 1u);

    /* Mesh Image */
    this->mesh_img = vxCreateImage(context, table_width_ds, table_height_ds, VX_DF_IMAGE_U32);
    status = vxGetStatus((vx_reference)this->mesh_img);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->mesh_img, "ldc_node_mesh_img");

        /* Copy Mesh table */
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = table_width_ds;
        rect.end_y = table_height_ds;

        image_addr.dim_x = table_width_ds;
        image_addr.dim_y = table_height_ds;
        image_addr.stride_x = 4u;
        image_addr.stride_y = table_width_ds * 4u;

        status = vxCopyImagePatch(this->mesh_img,
                                &rect, 0,
                                &image_addr,
                                const_cast<void*>(static_cast<const void*>(LDCObj::ldc_lut)),
                                VX_WRITE_ONLY,
                                VX_MEMORY_TYPE_HOST);
        if (status == VX_SUCCESS)
        {
            /* Mesh Parameters */
            memset(&this->mesh_params, 0, sizeof(tivx_vpac_ldc_mesh_params_t));

            tivx_vpac_ldc_mesh_params_init(&this->mesh_params);

            this->mesh_params.mesh_frame_width  = this->table_width;
            this->mesh_params.mesh_frame_height = this->table_height;
            this->mesh_params.subsample_factor  = this->ds_factor;

            this->mesh_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL);
            status = vxGetStatus((vx_reference)this->mesh_config);

            if(status == VX_SUCCESS)
            {
                vxSetReferenceName((vx_reference)this->mesh_config, "ldc_node_mesh_config");

                status = vxCopyUserDataObject(this->mesh_config, 0,
                                        sizeof(tivx_vpac_ldc_mesh_params_t),
                                        &this->mesh_params,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST);
                if(status != VX_SUCCESS)
                {
                    printf("[LDC-MODULE] Unable to copy mesh params into buffer! \n");
                }
            }
            else
            {
                printf("[LDC-MODULE] Unable to create mesh config object! \n");
            }
        }
        else
        {
            printf("[LDC-MODULE] Unable to copy mesh image! \n");
        }
    }
    else
    {
        printf("[LDC-MODULE] Unable to create mesh image! \n");
    }

    return status;
}

vx_status LDCObj::configureRegionParams(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    /* Block Size parameters */
    this->region_params.out_block_width  = LDC_BLOCK_WIDTH;
    this->region_params.out_block_height = LDC_BLOCK_HEIGHT;
    this->region_params.pixel_pad        = LDC_PIXEL_PAD;

    this->region_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t),  NULL);
    status = vxGetStatus((vx_reference)this->region_config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->region_config, "ldc_node_region_config");

        status = vxCopyUserDataObject(this->region_config, 0,
                            sizeof(tivx_vpac_ldc_region_params_t),
                            &this->region_params,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            printf("[LDC-MODULE] Unable to copy ldc region params to buffer! \n");
        }
    }
    else
    {
        printf("[LDC-MODULE] Unable to create region config object! \n");
    }

    return status;
}

vx_status LDCObj::configureLdcParams(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    /* LDC Configuration */
    tivx_vpac_ldc_params_init(&this->params);
    this->params.luma_interpolation_type = 1;
    this->params.dcc_camera_id = sensorObj->sensorParams.dccId;

    this->config = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL);
    status = vxGetStatus((vx_reference)this->config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->config, "ldc_node_config");

        status = vxCopyUserDataObject(this->config, 0,
                            sizeof(tivx_vpac_ldc_params_t),
                            &this->params,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            printf("[LDC-MODULE] Unable to copy ldc config params into buffer! \n");
        }
    }
    else
    {
        printf("[LDC-MODULE] Unable to create ldc params object! \n");
    }

    return status;
}

vx_status LDCObj::createLdcOutputs(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_cameras_enabled = this->num_cameras_enabled;

    /* LDC Output image in NV12 format */
    vx_image output_img = vxCreateImage(context, this->table_width, this->table_height, VX_DF_IMAGE_NV12);
    status = vxGetStatus((vx_reference)output_img);
    if(status == VX_SUCCESS)
    {
        this->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, num_cameras_enabled);
        vxReleaseImage(&output_img);

        status = vxGetStatus((vx_reference)this->output_arr);
        if(status != VX_SUCCESS)
        {
            printf("[LDC-MODULE] Unable to create output image array! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->output_arr, "ldc_node_output_arr");
        }
    }
    else
    {
        printf("[LDC-MODULE] Unable to create output image! \n");
    }

    if(this->en_out_ldc_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, this->output_file_path);
        this->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)this->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->file_path, "ldc_write_node_file_path");

            vxAddArrayItems(this->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            printf("[LDC-MODULE] Unable to create file path object for writing outputs! \n");
        }

        strcpy(file_prefix, "ldc_output");
        this->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)this->file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->file_prefix, "ldc_write_node_file_prefix");

            vxAddArrayItems(this->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            printf("[LDC-MODULE] Unable to create file prefix object for writing outputs! \n");
        }

        this->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)this->write_cmd);
        if(status != VX_SUCCESS)
        {
            printf("[LDC-MODULE] Unable to create file write cmd object! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)this->write_cmd, "ldc_write_node_write_cmd");
        }
    }
    else
    {
        this->file_path   = NULL;
        this->file_prefix = NULL;
        this->write_node  = NULL;
        this->write_cmd   = NULL;
    }

    return status;
}

vx_status LDCObj::initLdc(vx_context context, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    this->table_width  = LDC_TABLE_WIDTH;
    this->table_height = LDC_TABLE_HEIGHT;
    this->ds_factor    = LDC_DS_FACTOR;

    status = configureDccParams(context, sensorObj);

    if(status == VX_SUCCESS)
    {
        status = configureMeshParams(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configureRegionParams(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configureLdcParams(context, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = createLdcOutputs(context, sensorObj);
    }

    return (status);
}

void LDCObj::deinitLdc()
{
    vxReleaseUserDataObject(&this->config);
    vxReleaseUserDataObject(&this->region_config);
    vxReleaseUserDataObject(&this->mesh_config);

    vxReleaseImage(&this->mesh_img);
    vxReleaseObjectArray(&this->output_arr);

    if(this->dcc_config != NULL)
    {
        vxReleaseUserDataObject(&this->dcc_config);
    }
    if(this->en_out_ldc_write == 1)
    {
        vxReleaseArray(&this->file_path);
        vxReleaseArray(&this->file_prefix);
        vxReleaseUserDataObject(&this->write_cmd);
    }
}

void LDCObj::deleteLdc()
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    if(this->write_node != NULL)
    {
        vxReleaseNode(&this->write_node);
    }
}

vx_status LDCObj::createGraphLdc(vx_graph graph, vx_object_array input_arr, const char *target)
{
    vx_status status = VX_SUCCESS;

    if (this->dcc_config != NULL)
    {
        vx_image input_img = (vx_image)vxGetObjectArrayItem(input_arr, 0);
        vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);

        this->node = tivxVpacLdcNode(graph, this->config, NULL,
                                       NULL, NULL,
                                       NULL, this->dcc_config, input_img,
                                       output_img, NULL);

        vxReleaseImage(&input_img);
        vxReleaseImage(&output_img);

        status = vxGetStatus((vx_reference)this->node);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->node, "ldc_node");
            vxSetNodeTarget(this->node, VX_TARGET_STRING, target);

            vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e, vx_false_e};
            vxReplicateNode(graph, this->node, replicate, 9);
        }
        else
        {
            printf("[LDC-MODULE] Unable to create LDC node with DCC config! \n");
        }

    }
    else
    {
        vx_image input_img = (vx_image)vxGetObjectArrayItem(input_arr, 0);
        vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);

        this->node = tivxVpacLdcNode(graph, this->config, NULL,
                                    this->region_config, this->mesh_config,
                                    this->mesh_img, NULL, input_img,
                                    output_img, NULL);

        vxReleaseImage(&input_img);
        vxReleaseImage(&output_img);

        status = vxGetStatus((vx_reference)this->node);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)this->node, "ldc_node");
            vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1);

            vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e, vx_false_e};
            vxReplicateNode(graph, this->node, replicate, 9);
        }
        else
        {
            printf("[LDC-MODULE] Unable to create LDC node without DCC config! \n");
        }
    }

    if(this->en_out_ldc_write == 1)
    {
        status = createGraphLdcWriteOutput(graph);
    }

    return status;
}

vx_status LDCObj::createGraphLdcWriteOutput(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(this->output_arr, 0);
    this->write_node = tivxWriteImageNode(graph, output_img, this->file_path, this->file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)this->write_node);
    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)this->write_node, "ldc_write_node");
        vxSetNodeTarget(this->write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, this->write_node, replicate, 3);
    }
    else
    {
        printf("[LDC-MODULE] Unable to create node to write LDC output! \n");
    }

    return (status);
}

vx_status LDCObj::sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;
    /* TODO: Implement actual command sending logic if needed */
    return status;
}