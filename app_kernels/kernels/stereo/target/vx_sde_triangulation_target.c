/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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



#include "TI/tivx.h"
#include "TI/tivx_stereo.h"
#include "VX/vx.h"
#include "tivx_stereo_kernels_priv.h"
#include "tivx_kernel_sde_triangulation.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

#include <perception/perception.h>

#include "stdio.h"


static vx_status triangulateStereoDisparity(
        tivx_stereo_cam_params_t        * scParams,
        tivx_stereo_pointcloud_params_t * pcParams,
        int32_t                           width,
        int32_t                           height,
        int32_t                           disparityStride,
        int32_t                           rgbImageStride,
        uint8_t                         * rgbImage,
        int16_t                         * disparity,
        PTK_PointCloud                  * pc);



static tivx_target_kernel vx_sde_triangulation_target_kernel = NULL;

static vx_status VX_CALLBACK tivxSdeTriangulationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeTriangulationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxSdeTriangulationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxSdeTriangulationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_user_data_object_t *stereo_cam_config_desc;
    tivx_obj_desc_user_data_object_t *stereo_pc_config_desc;
    tivx_obj_desc_image_t *rgb_image_desc;
    tivx_obj_desc_image_t *disparity_desc;
    tivx_obj_desc_user_data_object_t *point_cloud_out_desc;

    tivx_stereo_cam_params_t * scParams;
    tivx_stereo_pointcloud_params_t  * pcParams;

    if ( (num_params != TIVX_KERNEL_SDE_TRIANGULATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_CAM_CONFIG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_PC_CONFIG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_RGB_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_PCL_IDX])
    )
    {
        printf("VX FAILURE\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        void *stereo_cam_config_target_ptr;
        void *stereo_pc_config_target_ptr;
        void *rgb_image_target_ptr;
        void *disparity_target_ptr;
        void *point_cloud_out_target_ptr;

        stereo_cam_config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_CAM_CONFIG_IDX];
        stereo_pc_config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_PC_CONFIG_IDX];
        rgb_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_RGB_IMAGE_IDX];
        disparity_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_DISPARITY_IDX];
        point_cloud_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_PCL_IDX];

        stereo_cam_config_target_ptr = tivxMemShared2TargetPtr(&stereo_cam_config_desc->mem_ptr);
        tivxMemBufferMap(stereo_cam_config_target_ptr,
            stereo_cam_config_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        stereo_pc_config_target_ptr = tivxMemShared2TargetPtr(&stereo_pc_config_desc->mem_ptr);
        tivxMemBufferMap(stereo_pc_config_target_ptr,
            stereo_pc_config_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        rgb_image_target_ptr = tivxMemShared2TargetPtr(&rgb_image_desc->mem_ptr[0]);
        tivxMemBufferMap(rgb_image_target_ptr,
            rgb_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        disparity_target_ptr = tivxMemShared2TargetPtr(&disparity_desc->mem_ptr[0]);
        tivxMemBufferMap(disparity_target_ptr,
            disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        point_cloud_out_target_ptr = tivxMemShared2TargetPtr(&point_cloud_out_desc->mem_ptr);
        tivxMemBufferMap(point_cloud_out_target_ptr,
            point_cloud_out_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        {
            /* call kernel processing function */

            /* < DEVELOPER_TODO: Add target kernel processing code here > */
            int32_t disparity_stride = disparity_desc->imagepatch_addr[0].stride_y / disparity_desc->imagepatch_addr[0].stride_x;
            int32_t rgb_image_stride = rgb_image_desc->imagepatch_addr[0].stride_y;

            PTK_PointCloudConfig cfg = {
                .maxPoints = disparity_desc->width * disparity_desc->height
            };

            // Should init for every frame, otherwise it does not work with pipelining
            PTK_PointCloud * cloud = PTK_PointCloud_init((uint8_t *)point_cloud_out_target_ptr, &cfg);

            scParams = (tivx_stereo_cam_params_t *) stereo_cam_config_target_ptr;
            pcParams = (tivx_stereo_pointcloud_params_t *) stereo_pc_config_target_ptr;

            status = triangulateStereoDisparity(scParams,
                                                pcParams,
                                                disparity_desc->width,
                                                disparity_desc->height,
                                                disparity_stride,
                                                rgb_image_stride,
                                                (uint8_t *)rgb_image_target_ptr,
                                                (int16_t *)disparity_target_ptr,
                                                cloud);

            /* kernel processing function complete */
        }

        tivxMemBufferUnmap(stereo_cam_config_target_ptr,
            stereo_cam_config_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(stereo_pc_config_target_ptr,
            stereo_pc_config_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(rgb_image_target_ptr,
            rgb_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(disparity_target_ptr,
            disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(point_cloud_out_target_ptr,
            point_cloud_out_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }


    return status;
}

static vx_status VX_CALLBACK tivxSdeTriangulationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_SDE_TRIANGULATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_CAM_CONFIG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_STEREO_PC_CONFIG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_RGB_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SDE_TRIANGULATION_PCL_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxSdeTriangulationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelSdeTriangulation()
{
    vx_status status = VX_FAILURE;
    char target_name[4][TIVX_TARGET_MAX_NAME];
    uint32_t num_targets = 1;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_MPU_0 )
    {
        strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
        num_targets = 4;
        status = VX_SUCCESS;
    }
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name[0]);
    }


    if (status == VX_SUCCESS)
    {
        uint32_t    i;

        for (i = 0; i < num_targets; i++)
        {        
            vx_sde_triangulation_target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_SDE_TRIANGULATION_NAME,
                                target_name[i],
                                tivxSdeTriangulationProcess,
                                tivxSdeTriangulationCreate,
                                tivxSdeTriangulationDelete,
                                NULL,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelSdeTriangulation()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_sde_triangulation_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_sde_triangulation_target_kernel = NULL;
    }
}

vx_status triangulateStereoDisparity(
        tivx_stereo_cam_params_t         * scParams,
        tivx_stereo_pointcloud_params_t  * pcParams,
        int32_t                            width,
        int32_t                            height,
        int32_t                            disparityStride,
        int32_t                            rgbImageStride,
        uint8_t                          * rgbImage,
        int16_t                          * disparity,
        PTK_PointCloud                   * pc)
{
    PTK_Point  *points;
    vx_int32    i, j, p;
    vx_int16    d;

    //vx_int8     sign;
    vx_uint8    valid;
    vx_uint8    *R, *G, *B;
    vx_int16    pixDisparity;
    vx_uint8    step = pcParams->subSampleRatio;

    vx_int16 dcx = scParams->dist_center_x;
    vx_int16 dcy = scParams->dist_center_y;

    vx_float32 baseline = scParams->baseline;
    vx_float32 focal_length = scParams->focal_length;
    vx_float32 boverd;
    vx_float32 X, Y, Z;

    // clear point cloud
    p = 0;

    points = PTK_PointCloud_getPoints(pc);

    R = (vx_uint8 *)rgbImage;
    G = (vx_uint8 *)rgbImage + 1;
    B = (vx_uint8 *)rgbImage + 2;

    for (j = 0; j < height; j+= step)
    {
        for (i = 0; i < width; i+= step)
        {
            // In operation mode, minimum disparity should be non-negative
            pixDisparity = disparity[i];
            //sign = (pixDisparity >> 15) == 0 ? 1: -1;
            d = (pixDisparity >> 3) & 0xFFF;
            //d *= sign;

            // check if disparity is valid based on confidence threshould
            valid = !pcParams->usePCConfig;
            valid = valid || (pcParams->usePCConfig && 
                              ((pixDisparity & 0x7) >= pcParams->thConfidence));
            if (valid == 0)
            {
                R += 3; G += 3; B += 3;
                continue;
            }

            boverd =  baseline*16.0/d;
            Z = focal_length * boverd;
            X = (i - dcx) * boverd;
            Y = (j - dcy) * boverd;

            // check if disparity is valid based on position
            valid = !pcParams->usePCConfig;
            valid = valid || (pcParams->usePCConfig &&
                              (X >=  pcParams->lowLimitX && X <=  pcParams->highLimitX) && 
                              (Z >=  pcParams->lowLimitY && Z <=  pcParams->highLimitY) &&
                              (Y <= -pcParams->lowLimitZ && Y >= -pcParams->highLimitZ));
            if (valid == 0)
            {
                R += 3; G += 3; B += 3;
                continue;
            }

            // add a point into PCL
            // based on PointCloud_addPoint()
            points[p].x = X;
            points[p].y = Z;
            points[p].z = -Y;
            points[p].meta.w = (float) (((*R) << 16) | ((*G) << 8) | (*B));

            pc->numPoints += 1;
            p++;

            R += 3;
            G += 3;
            B += 3;
        }

        R += (rgbImageStride - 3*width);
        G += (rgbImageStride - 3*width);
        B += (rgbImageStride - 3*width);

        disparity += disparityStride;
    }

    // make sure numPoints is less than maxPoints
    if (pc->numPoints == pc->config.maxPoints)
    {
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}

