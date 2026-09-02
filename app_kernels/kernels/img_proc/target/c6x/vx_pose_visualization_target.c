/*
*
* Copyright (c) 2018-2026 Texas Instruments Incorporated
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
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_target_utils.h>

#include <tivx_pose_visualization_host.h>
#include "itidl_ti.h"
#include <math.h>
static tivx_target_kernel vx_poseViz_kernel = NULL;

typedef struct {
    tivxPoseVizParams nodeParams;
    vx_uint32 background_image_copy_reset_value;

} tivxPoseVizKernelParams;

static vx_status VX_CALLBACK tivxKernelPoseVizCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxPoseVizKernelParams * kernelParams = NULL;
    tivxPoseVizParams * nodeParams = NULL;

    tivx_obj_desc_array_t* configuration_desc;
    void * configuration_target_ptr;

    configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_POSE_VISUALIZATION_CONFIGURATION_IDX];
    configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
    tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
    nodeParams = (tivxPoseVizParams*)configuration_target_ptr;

    kernelParams = tivxMemAlloc(sizeof(tivxPoseVizKernelParams), TIVX_MEM_EXTERNAL);

    if(kernelParams == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate memory for kernel object\n");
        status = VX_FAILURE;
    }
    if(status == VX_SUCCESS)
    {
        memcpy(&kernelParams->nodeParams, nodeParams, sizeof(tivxPoseVizParams));
        kernelParams->background_image_copy_reset_value = nodeParams->max_background_image_copy;

        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxPoseVizKernelParams));
    }

  return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelPoseVizDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxPoseVizKernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if(status == VX_SUCCESS)
        {
            tivxMemFree(prms, sizeof(tivxPoseVizKernelParams), TIVX_MEM_EXTERNAL);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to get kernel instance to delete!\n");
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelPoseVizControl
(
    tivx_target_kernel_instance kernel,
    uint32_t node_cmd_id,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxPoseVizKernelParams *prms = NULL;
    tivxPoseVizParams *viz_prms = NULL;

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxPoseVizKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(status == VX_SUCCESS)
    {
        switch (node_cmd_id)
        {
            case TIVX_IMG_PROC_POSE_VIZ_RESET_BACKGROUND:
            {
                viz_prms = (tivxPoseVizParams *)&prms->nodeParams;
                viz_prms->max_background_image_copy = prms->background_image_copy_reset_value;
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxKernelPoseVizControl: Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelPoseVizProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    tivxPoseVizParams* viz_params;
    tivxPoseVizKernelParams *prms = NULL;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    
    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxPoseVizKernelParams) != size))
        {
            status = VX_FAILURE;
        }
        viz_params = (tivxPoseVizParams *)&prms->nodeParams;
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_matrix_t *in_pose;
        void* in_pose_matrix_target_ptr;

        tivx_obj_desc_image_t *bg_img;
        void *bg_y_image_target_ptr;
        void *bg_cbcr_image_target_ptr;

        tivx_obj_desc_image_t *out_img;
        void *out_y_image_target_ptr;
        void *out_cbcr_image_target_ptr;
        int32_t i, j;
        
        in_pose = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_POSE_VISUALIZATION_INPUT_POSE_IDX];
        in_pose_matrix_target_ptr  = tivxMemShared2TargetPtr(&in_pose->mem_ptr);
        tivxMemBufferMap(in_pose_matrix_target_ptr, in_pose->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        bg_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_POSE_VISUALIZATION_INPUT_BACKGROUND_IMAGE_IDX];
        bg_y_image_target_ptr  = tivxMemShared2TargetPtr(&bg_img->mem_ptr[0]);
        tivxMemBufferMap(bg_y_image_target_ptr, bg_img->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        bg_cbcr_image_target_ptr  = tivxMemShared2TargetPtr(&bg_img->mem_ptr[1]);
        tivxMemBufferMap(bg_cbcr_image_target_ptr, bg_img->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        out_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_POSE_VISUALIZATION_OUTPUT_START_IDX];
        out_y_image_target_ptr  = tivxMemShared2TargetPtr(&out_img->mem_ptr[0]);
        tivxMemBufferMap(out_y_image_target_ptr, out_img->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        out_cbcr_image_target_ptr  = tivxMemShared2TargetPtr(&out_img->mem_ptr[1]);
        tivxMemBufferMap(out_cbcr_image_target_ptr, out_img->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        if (viz_params->skip_flag == 0x0)
        {
            if(viz_params->max_background_image_copy > 0)
            {
                memcpy(out_y_image_target_ptr, bg_y_image_target_ptr, bg_img->mem_size[0]);
                memcpy(out_cbcr_image_target_ptr, bg_cbcr_image_target_ptr, bg_img->mem_size[1]);
                
                viz_params->max_background_image_copy--;
            }

            vx_float32   X , Y, Z;
            vx_float32 * pose_matrix = (vx_float32 *)in_pose_matrix_target_ptr;

            X = pose_matrix[0*4 + 3]*viz_params->projMat[0][0] + pose_matrix[1*4 + 3]*viz_params->projMat[0][1] + pose_matrix[2*4 + 3]*viz_params->projMat[0][2] + viz_params->projMat[0][3];
            Y = pose_matrix[0*4 + 3]*viz_params->projMat[1][0] + pose_matrix[1*4 + 3]*viz_params->projMat[1][1] + pose_matrix[2*4 + 3]*viz_params->projMat[1][2] + viz_params->projMat[1][3];
            Z = pose_matrix[0*4 + 3]*viz_params->projMat[2][0] + pose_matrix[1*4 + 3]*viz_params->projMat[2][1] + pose_matrix[2*4 + 3]*viz_params->projMat[2][2] + viz_params->projMat[2][3];

            vx_int32 draw_loc_x = viz_params->cx + ((viz_params->fx*X)/Z);
            vx_int32 draw_loc_y = viz_params->cy + ((viz_params->fy*Y)/Z);

            if(draw_loc_y >= viz_params->img_height){
                draw_loc_y = viz_params->img_height - 1;
            }

            if(draw_loc_y < 0){
                draw_loc_y = 0;
            }

            if(draw_loc_x >= viz_params->img_width){
                draw_loc_x = viz_params->img_width - 1;
            }

            if(draw_loc_x < 0){
                draw_loc_x = 0;
            }

            viz_params->past_xy[viz_params->frame_cnt%TIVX_MAX_PAST_POSES][0] = draw_loc_x;
            viz_params->past_xy[viz_params->frame_cnt%TIVX_MAX_PAST_POSES][1] = draw_loc_y;

            vx_int32 length_pose_info = (viz_params->frame_cnt <= (TIVX_MAX_PAST_POSES-1)) ? (viz_params->frame_cnt + 1) : TIVX_MAX_PAST_POSES;
            vx_int32 start_index      = (viz_params->frame_cnt + 1 + TIVX_MAX_PAST_POSES - length_pose_info) % TIVX_MAX_PAST_POSES;
            vx_int32 color, l;

            for(l = 0; l < length_pose_info; l++)
            {
                draw_loc_x = viz_params->past_xy[(start_index + l)%TIVX_MAX_PAST_POSES][0];
                draw_loc_y = viz_params->past_xy[(start_index + l)%TIVX_MAX_PAST_POSES][1];

                color = ((viz_params->frame_cnt + 1 - length_pose_info + l)*5) % 512; // this 5 hard coding, controls how fast color changes

                /*Color will move from light blue to dark blue. no abrupt change.*/
                if(color >= 256)
                {
                    color = 511 - color; // when color is 300, then color will be modified to 250.
                }

                vx_uint8 R, G, B;
                vx_uint8 Y, U, V;

                B = 255;
                R = color;
                G = color;

                Y = ((66 * (R) +129 * (G)+25 * (B)+128) >> 8) + 16;
                U = ((-38 * (R)-74 * (G)+112 * (B)+128) >> 8) + 128;
                V = ((112 * (R)-94 * (G)-18 * (B)+128) >> 8) + 128;

                U = (U > 0) ? (U > 255 ? 255 : U) : 0;
                V = (V > 0) ? (V > 255 ? 255 : V) : 0;

                for(i = -3; i <= 3; i++)
                {
                    for(j = -3; j <= 3; j++)
                    {
                        ((vx_uint8*)out_y_image_target_ptr)[(draw_loc_y + i)*viz_params->img_width + (draw_loc_x + j)] = Y;
                        ((vx_uint8*)out_cbcr_image_target_ptr)[((draw_loc_y + i) >> 1)*viz_params->img_width + ((draw_loc_x + j) >> 1)*2 + 0 ] = U;
                        ((vx_uint8*)out_cbcr_image_target_ptr)[((draw_loc_y + i) >> 1)*viz_params->img_width + ((draw_loc_x + j) >> 1)*2 + 1 ] = V;
                    }
                }
            }

            viz_params->frame_cnt++;
        }

        tivxMemBufferUnmap(in_pose_matrix_target_ptr, in_pose->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(bg_y_image_target_ptr, bg_img->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(bg_cbcr_image_target_ptr, bg_img->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(out_y_image_target_ptr, out_img->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(out_cbcr_image_target_ptr, out_img->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}

void tivxAddTargetKernelPoseViz()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_poseViz_kernel = tivxAddTargetKernelByName
                            (
                                TIVX_KERNEL_POSE_VISUALIZATION_NAME,
                                target_name,
                                tivxKernelPoseVizProcess,
                                tivxKernelPoseVizCreate,
                                tivxKernelPoseVizDelete,
                                tivxKernelPoseVizControl,
                                NULL
                            );
    }
}

void tivxRemoveTargetKernelPoseViz()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_poseViz_kernel);
    if (status == VX_SUCCESS)
    {
        vx_poseViz_kernel = NULL;
    }
}
