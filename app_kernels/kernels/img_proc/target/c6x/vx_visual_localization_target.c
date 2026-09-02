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
#include <tivx_alg_ivision_if.h>
#include <tivx_kernels_target_utils.h>

#include <tivx_visual_localization_host.h>
#include "tiadalg_interface.h"
#include <math.h>

static int32_t vl_alg_fe_correct(int32_t* in_pt_list, int32_t num_points, float* lens_dist_table, int32_t table_len,
  int32_t* in_pruned_pt_list, float* out_pt_list, int32_t dl_width, int32_t dl_height,
  int32_t img_width, int32_t img_height);

static inline uint8_t vl_alg_interpolate(float xPosition, float xRangeLower, float xRangeHigher, float yRangeLower,
    float yRangeHigher, float *yInterpolated);

static tivx_target_kernel vx_egoLocal_kernel = NULL;

typedef struct
{
    tivxVisualLocalizationParams prms_host;
    IVISION_BufDesc     inBufDesc[TIADALG_EL_IN_BUFDESC_TOTAL];
    IVISION_BufDesc    *inBufDescList[TIADALG_EL_IN_BUFDESC_TOTAL];
    IVISION_InBufs      inBufs;

    TIADALG_el_in_args         inArgs;
    TIADALG_el_out_args        outArgs;

    TIADALG_el_create_params   createParams;

    void                         *algHandle;
    void                         *key_point_list;
    void                         *out_desc_buf;
    void                         *fe_kp_list;
    void                         *fe_pruned_kp_list;
    void                         *sparse_upsampling_scratch;

    vx_int32                      is_sparse_upsampling_scratch_filled;

} tivxVisualLocalizationKernelParams;

static void tivxVLFreeMem(tivxVisualLocalizationKernelParams *prms);
static int32_t tiadalg_vl_AllocInputMem(IVISION_BufDesc *BufDescList, vx_int32 num_frame_feat)
{
    vx_uint16 numBuffs = 2;

    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].numPlanes                       = 1;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].frameROI.topLeft.x = 0;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].frameROI.topLeft.y = 0;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].width              = sizeof(tiadalg_ext_feat);
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].height             = num_frame_feat;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].frameROI.width     = sizeof(tiadalg_ext_feat);
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].frameROI.height    = num_frame_feat;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].planeType          = 0;

    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].numPlanes                       = 1;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].frameROI.topLeft.x = 0;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].frameROI.topLeft.y = 0;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].width              = sizeof(tiadalg_feat_desc);
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].height             = num_frame_feat;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].frameROI.width     = sizeof(tiadalg_feat_desc);
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].frameROI.height    = num_frame_feat;
    BufDescList[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].planeType          = 0;

    return numBuffs;
}

static vx_status VX_CALLBACK tivxKernelVisualLocalizationCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    tivx_obj_desc_user_data_object_t * config;
    tivxVisualLocalizationParams *prms_host;
    tivxVisualLocalizationKernelParams* prms;
    void* config_target_ptr;

    #ifdef VL_DEBUG
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

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
        /* IMPORTANT! Config data is assumed to be available at index 0 */
        config    = (tivx_obj_desc_user_data_object_t *)obj_desc[0];

        prms = tivxMemAlloc(sizeof(tivxVisualLocalizationKernelParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVisualLocalizationKernelParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if(status == VX_SUCCESS)
        {
            prms->createParams.visionParams.algParams.size   = sizeof(TIADALG_el_create_params);
            prms->createParams.visionParams.cacheWriteBack   = NULL;

            /*Host parameter related parameters updates*/
            config_target_ptr = tivxMemShared2TargetPtr(&config->mem_ptr);
            tivxMemBufferMap(config_target_ptr, config->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            prms_host = (tivxVisualLocalizationParams *)config_target_ptr;

            memcpy(&prms->prms_host,prms_host, sizeof(tivxVisualLocalizationParams));

            prms->createParams.max_frame_feat    = prms_host->max_frame_feat;
            prms->createParams.max_map_feat  = prms_host->max_map_feat;
            /* for visual localization module, features are always external */
            prms->createParams.is_feat_comp_ext  = 1;
            prms->createParams.max_feat_match    = prms_host->max_feat_match;

            prms->createParams.map_info.num_voxels    = prms_host->num_voxels;
            prms->createParams.map_info.num_map_feat  = prms_host->num_map_feat;
            prms->createParams.map_info.desc_size     = sizeof(tiadalg_feat_desc);
            prms->createParams.map_info.desc_type     = 1;
            prms->createParams.map_info.voxel_size[0] = prms_host->voxel_size[0];
            prms->createParams.map_info.voxel_size[1] = prms_host->voxel_size[1];
            prms->createParams.map_info.voxel_size[2] = prms_host->voxel_size[2];
            prms->createParams.map_info.map_range[0][0]        = prms_host->map_range[0][0];
            prms->createParams.map_info.map_range[0][1]        = prms_host->map_range[0][1];
            prms->createParams.map_info.map_range[1][0]        = prms_host->map_range[1][0];
            prms->createParams.map_info.map_range[1][1]        = prms_host->map_range[1][1];
            prms->createParams.map_info.map_range[2][0]        = prms_host->map_range[2][0];
            prms->createParams.map_info.map_range[2][1]        = prms_host->map_range[2][1];
            prms->inArgs.en_reset = 1;

            prms->key_point_list    = tivxMemAlloc(prms_host->max_frame_feat*sizeof(tiadalg_ext_feat), TIVX_MEM_EXTERNAL);
            prms->out_desc_buf      = tivxMemAlloc(prms_host->max_frame_feat*sizeof(tiadalg_feat_desc), TIVX_MEM_EXTERNAL);
            prms->fe_kp_list        = tivxMemAlloc(prms_host->max_frame_feat*sizeof(int32_t)*2, TIVX_MEM_EXTERNAL);

            /*fe_pruned_kp_list is also used for NMS scratch buffer.*/
            vx_int32 nms_scratch_size = prms_host->dl_width * sizeof(uint16_t);
            vx_int32 fe_pruned_kp_list_size = prms_host->max_frame_feat*sizeof(int32_t)*2;

            prms->fe_pruned_kp_list = tivxMemAlloc((nms_scratch_size>fe_pruned_kp_list_size)?nms_scratch_size:fe_pruned_kp_list_size, TIVX_MEM_EXTERNAL);
            prms->sparse_upsampling_scratch = tivxMemAlloc(16*12*64*sizeof(uint16_t), TIVX_MEM_EXTERNAL);
            prms->is_sparse_upsampling_scratch_filled = 0;

            if((prms->key_point_list == NULL) || (prms->out_desc_buf == NULL) || (prms->fe_kp_list == NULL) || (prms->fe_pruned_kp_list == NULL)){
                status = VX_ERROR_NO_MEMORY;
            }

            /*create pointers for voxel and other map data into memtab*/
            /*memtab[1] is voxel, memtab[2] is map points, memtab[3] is map desc*/
            tivx_obj_desc_tensor_t* inTensor1  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_INPUT_START_IDX + 0];
            void* in_tensor_target_ptr1  = tivxMemShared2TargetPtr(&inTensor1->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr1, inTensor1->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            prms->createParams.map_voxel_ptr = in_tensor_target_ptr1;

            tivx_obj_desc_tensor_t* inTensor2  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_MAP_PTS_IDX];
            void* in_tensor_target_ptr2  = tivxMemShared2TargetPtr(&inTensor2->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr2, inTensor2->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            prms->createParams.map_pt_ptr = in_tensor_target_ptr2;

            tivx_obj_desc_tensor_t* inTensor3  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_MAP_DESC_IDX];
            void* in_tensor_target_ptr3  = tivxMemShared2TargetPtr(&inTensor3->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr3, inTensor3->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            prms->createParams.map_desc_ptr = in_tensor_target_ptr3;

            /* Free the internal scratch memory before calling create */
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);

            prms->algHandle = tivxAlgiVisionCreate
                                (
                                &VL_VISION_FXNS,
                                (IALG_Params *)(&prms->createParams)
                                );

            if (NULL == prms->algHandle)
            {
                status = VX_FAILURE;
            }

            tivxMemBufferUnmap(in_tensor_target_ptr1, inTensor1->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            tivxMemBufferUnmap(in_tensor_target_ptr2, inTensor2->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            tivxMemBufferUnmap(in_tensor_target_ptr3, inTensor3->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxVisualLocalizationKernelParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVLFreeMem(prms);
            }
        }
        #ifdef VL_DEBUG
        tivx_clr_debug_zone(VX_ZONE_INFO);
        #endif
    }

    return(status);
}

static vx_status VX_CALLBACK tivxKernelVisualLocalizationDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    vx_uint32 size;
    tivxVisualLocalizationKernelParams *prms = NULL;

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
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) && (sizeof(tivxVisualLocalizationKernelParams) == size))
        {
            if (prms->algHandle)
            {
                tivxAlgiVisionDelete(prms->algHandle);
            }

            if ((VX_SUCCESS == status) && (NULL != prms) &&
                (sizeof(tivxVisualLocalizationKernelParams) == size))
            {
                tivxMemFree(prms->key_point_list, prms->prms_host.max_frame_feat*sizeof(tiadalg_ext_feat), TIVX_MEM_EXTERNAL);
                tivxMemFree(prms->out_desc_buf, prms->prms_host.max_frame_feat*sizeof(tiadalg_feat_desc), TIVX_MEM_EXTERNAL);
                tivxMemFree(prms->fe_kp_list, prms->prms_host.max_frame_feat*sizeof(int32_t)*2, TIVX_MEM_EXTERNAL);
                tivxMemFree(prms->fe_pruned_kp_list, prms->prms_host.max_frame_feat*sizeof(int32_t)*2, TIVX_MEM_EXTERNAL);
                tivxMemFree(prms->sparse_upsampling_scratch, 16*12*64*sizeof(uint16_t), TIVX_MEM_EXTERNAL);
            }
            tivxVLFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelVisualLocalizationControl
(
    tivx_target_kernel_instance kernel,
    uint32_t node_cmd_id,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxVisualLocalizationKernelParams* prms;

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxVisualLocalizationKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(status == VX_SUCCESS)
    {
        switch (node_cmd_id)
        {
            case TIVX_IMG_PROC_VIZ_LOC_RESET_POSE:
            {
                prms->inArgs.en_reset = 1; 
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxKernelVisualLocalizationControl: Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelVisualLocalizationProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxVisualLocalizationKernelParams *prms;
    vx_int32 i;
    vx_uint32 size;

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
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||  (sizeof(tivxVisualLocalizationKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }


    if ((VX_SUCCESS == status) && (prms->prms_host.skip_flag == 0))
    {
        tivx_obj_desc_tensor_t *inTensor;
        tivx_obj_desc_matrix_t *outTensor;

        void *in_tensor_target_ptr;
        void *out_tensor_target_ptr;

        vx_int32 num_key_points;
        int32_t i;

        prms->inBufs.size     = sizeof(prms->inBufs);
        prms->inBufs.bufDesc  = prms->inBufDescList;
        prms->inBufs.numBufs  = 2;

        tivx_obj_desc_user_data_object_t *out_args;
        void *out_args_tensor_target_ptr = NULL;

        /*Get the TIDL output tensor scales*/
        out_args  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_TIDL_OUT_ARGS_IDX];
        out_args_tensor_target_ptr  = tivxMemShared2TargetPtr(&out_args->mem_ptr);
        tivxMemBufferMap(out_args_tensor_target_ptr, out_args->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        prms->prms_host.lo_res_desc_scale_pw2 =
                        log2(((TIDL_outArgs*)out_args_tensor_target_ptr)->scale[prms->prms_host.lo_res_desc_lyr_id]);

        prms->prms_host.score_scale_pw2 =
                        log2(((TIDL_outArgs*)out_args_tensor_target_ptr)->scale[prms->prms_host.score_lyr_id]);

        prms->prms_host.bias_scale_pw2 = prms->prms_host.filter_scale_pw2 + prms->prms_host.lo_res_desc_scale_pw2;

        tivxMemBufferUnmap(out_args_tensor_target_ptr, out_args->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        /* Feature point tensor. NMS has to be applied before passing the feature point score plane */
        inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_FEAT_PT_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if(prms->prms_host.is_feat_comp_ext != 0x0)
        {
            /*External feature flow. Not enabled from app side. kernel has this feature to bypass TIDL*/
            prms->inBufDesc[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].buf = in_tensor_target_ptr;
            in_tensor_target_ptr = (vx_int32*)in_tensor_target_ptr + (sizeof(tiadalg_ext_feat) >> 2)*prms->createParams.max_frame_feat;
            num_key_points = ((vx_int32*)in_tensor_target_ptr)[0];
        }
        else
        {
            /*TIDL flow*/
            /* Feature is computed by TIDL node and it is dense */
            vx_int8 * score_buf = (vx_int8 *) in_tensor_target_ptr;
            vx_int32 guard_pixels = 32;
            vx_int32 shift_amt; // in terms of right shift. As float_value = fixed_value/scale.
            vx_int32 rnd;
            vx_int32 cur_score_th;

            // float_value = fixed_value/scale.
            // float_value > score_th, means there is key points.
            // i.e. (fixed_value/scale) > score_th or fixed_value > score_th*scale
            if(prms->prms_host.score_scale_pw2 > 0)
            {
                shift_amt = prms->prms_host.score_scale_pw2;
                cur_score_th = prms->prms_host.score_th << shift_amt;
            }
            else if(prms->prms_host.score_scale_pw2 < 0)
            {
                //float_value = fixed_value/scale.
                shift_amt = -prms->prms_host.score_scale_pw2;
                rnd = 1<<(shift_amt-1);
                cur_score_th = ((prms->prms_host.score_th+rnd) >> shift_amt);
            }
            else
            {
                cur_score_th = prms->prms_host.score_th;
            }

            tivx_obj_desc_tensor_t *in_tensor_r2r_table  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_R2R_TABLE_IDX];
            vx_int8* in_tensor_target_r2r_table  = tivxMemShared2TargetPtr(&in_tensor_r2r_table->mem_ptr);
            tivxMemBufferMap(in_tensor_target_r2r_table, in_tensor_r2r_table->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            /*Sparse valid key points generation from dense plane of key points score*/
            num_key_points = 0;
            score_buf = score_buf + prms->prms_host.tidl_tensor_startx[prms->prms_host.score_lyr_id];

            /*NMS of score plane*/
            num_key_points =
                tiadalg_image_recursive_nms_c66(score_buf,
                                    prms->prms_host.dl_width, /*Deep learning input width*/
                                    prms->prms_host.tidl_tensor_pitch[prms->prms_host.score_lyr_id], /*Deep learning score output pitch*/
                                    prms->prms_host.dl_height,/*Deep learning input height*/
                                    cur_score_th,
                                    prms->prms_host.max_frame_feat,
                                    guard_pixels,
                                    prms->fe_pruned_kp_list, /*used as scratch buffer here*/
                                    prms->prms_host.score_lyr_elm_type,
                                    prms->fe_kp_list);

            /*Fish eye lens distortion correction*/
            if(prms->prms_host.is_ip_fe == 1)
            {
                num_key_points =
                vl_alg_fe_correct(prms->fe_kp_list, /*key points list , detected after NMS in DL resolution domain*/
                                    num_key_points,
                                    (float*)in_tensor_target_r2r_table, /*Fish eye correction table. R_fish to R rect*/
                                    prms->prms_host.lens_dist_table_size, /*number of entries in fish eye correction table*/
                                    prms->fe_pruned_kp_list,  /* pruned list of 'fe_kp_list' */
                                    (float*) prms->key_point_list, /*Fish eye corrected key points, in original image domain*/
                                    prms->prms_host.dl_width,
                                    prms->prms_host.dl_height,
                                    prms->prms_host.img_width,
                                    prms->prms_host.img_height);
            }
            else
            {
                // in this flow just scale up the key points to orginal image resolution
                float x_scale_fct = ((float)prms->prms_host.img_width)/prms->prms_host.dl_width;
                float y_scale_fct = ((float)prms->prms_host.img_height)/prms->prms_host.dl_height;
                vx_int32 i;

                for(i = 0; i < num_key_points; i++)
                {
                    ((float*) prms->key_point_list)[2*i]     = ((int32_t*)prms->fe_kp_list)[2*i] * x_scale_fct;
                    ((float*) prms->key_point_list)[2*i + 1] = ((int32_t*)prms->fe_kp_list)[2*i + 1] * y_scale_fct;

                    ((int32_t*)prms->fe_pruned_kp_list)[2*i]     = ((int32_t*)prms->fe_kp_list)[2*i];
                    ((int32_t*)prms->fe_pruned_kp_list)[2*i + 1] = ((int32_t*)prms->fe_kp_list)[2*i + 1];
                }
            }

            /*Key points list buffer to be povided to visual localization module*/
            prms->inBufDesc[TIADALG_EL_IN_BUFDESC_EXTERNAL_FEAT].bufPlanes[0].buf = prms->key_point_list;
            tivxMemBufferUnmap(in_tensor_target_r2r_table, in_tensor_r2r_table->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        /* Feature descriptor tensor*/
        /* TIDL lower resolution tensor is descriptor tensor, and higher resolution tesnor is score tensor*/
        /* Descriptor need to be extrapolated in native resolution before feeding to visual localization API*/
        inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_FEAT_DESC_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if(prms->prms_host.is_feat_comp_ext != 0x0)
        {
            /*External feature flow, as it is tensor is provided to visual localization API.*/
            prms->inBufDesc[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].buf = in_tensor_target_ptr;
        }
        else
        {
            /* Feature descriptor is computed by TIDL node and it is dense. In this flow tensor data needs to be processed */

            /* skipping locations because of TIDL padding */
            in_tensor_target_ptr = (void*)((uintptr_t)in_tensor_target_ptr +
                                                        prms->prms_host.tidl_tensor_startx[prms->prms_host.lo_res_desc_lyr_id]);

            /*Filter coefficients for doing sparse upsampling*/
            tivx_obj_desc_tensor_t *inTensor_f  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_FILTER_IDX];
            vx_int8* in_tensor_target_ptr_f  = tivxMemShared2TargetPtr(&inTensor_f->mem_ptr);
            tivxMemBufferMap(in_tensor_target_ptr_f, inTensor_f->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            vx_status status = 0;
            tiadalg_sparse_upsampling_c66((vx_int8*)in_tensor_target_ptr ,
                                                            prms->prms_host.tidl_tensor_pitch[prms->prms_host.lo_res_desc_lyr_id],
                                                            prms->prms_host.desc_plane_size,
                                                            64, /*descriptor size in terms of int16_t*/
                                                            in_tensor_target_ptr_f,  /*Convolution filter coefficients pointer*/
                                                            (vx_int16*)(in_tensor_target_ptr_f + 7*7*64), /*Convolution Bias pointer*/
                                                            prms->fe_pruned_kp_list,
                                                            num_key_points,
                                                            prms->sparse_upsampling_scratch,
                                                            prms->is_sparse_upsampling_scratch_filled,
                                                            prms->prms_host.lo_res_desc_scale_pw2,   /*TIDL descriptor output scale*/
                                                            prms->prms_host.filter_scale_pw2, /*Convolution filter coefficients scale*/
                                                            prms->prms_host.bias_scale_pw2,   /*Convolution bias scale*/
                                                            (prms->prms_host.hi_res_desc_scale_pw2),    /*Output desired scale*/
                                                            (void*)prms->out_desc_buf, /*Output sparse descriptor*/
                                                            prms->prms_host.lo_res_desc_elm_type,
                                                            ((sizeof(VL_DESC_DATA_TYPE) == 1)?TIADALG_DATA_TYPE_U08:TIADALG_DATA_TYPE_U16));

            prms->is_sparse_upsampling_scratch_filled  = 0x1;

            if(status!=TIADALG_PROCESS_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tiadalg failed !!!\n");
            }

            tivxMemBufferUnmap(in_tensor_target_ptr_f, inTensor_f->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            prms->inBufDesc[TIADALG_EL_IN_BUFDESC_EXTERNAL_DESC].bufPlanes[0].buf = prms->out_desc_buf;
        }
        tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tiadalg_vl_AllocInputMem(prms->inBufDesc, num_key_points);

        for(i = 0; i < prms->inBufs.numBufs; i++)
        {
            prms->inBufDescList[i]     = &prms->inBufDesc[i];
        }

        prms->outArgs.iVisionOutArgs.size       = sizeof(TIADALG_el_out_args);
        prms->inArgs.iVisionInArgs.size         = sizeof(TIADALG_el_in_args);
        prms->inArgs.iVisionInArgs.subFrameInfo = 0;
        prms->inArgs.search_range[0] = 36;
        prms->inArgs.search_range[1] = 36;
        prms->inArgs.search_range[2] = 36;
        prms->inArgs.p3p_params.fx                      =  prms->prms_host.fx;
        prms->inArgs.p3p_params.fy                      =  prms->prms_host.fy;
        prms->inArgs.p3p_params.cx                      =  prms->prms_host.cx;
        prms->inArgs.p3p_params.cy                      =  prms->prms_host.cy;
        prms->inArgs.p3p_params.iterationsCount         =  200;
        prms->inArgs.p3p_params.reprojErrorThreshold    =  8.0;
        prms->inArgs.p3p_params.inliersRatio            =  0.95;
        prms->inArgs.p3p_params.seed                    =  258001;
        prms->inArgs.num_cur_feat                       =  num_key_points;
        prms->inArgs.en_pose_filtering                  =  1;
        prms->inArgs.en_subsample_map                   =  1;
        prms->inArgs.est_loc[0]                         = prms->prms_host.init_est[0];
        prms->inArgs.est_loc[1]                         = prms->prms_host.init_est[1];
        prms->inArgs.est_loc[2]                         = prms->prms_host.init_est[2];

        if(prms->prms_host.is_feat_comp_ext != 0x0)
        {
            /*Non TIDL flow*/
            prms->inArgs.feat_q_factor                      =  1024;
        }
        else
        {
            /*TIDL flow when features are computed internally, they are in integer format so no need to divide any value */
            prms->inArgs.feat_q_factor                      =  1;
        }

        status = tivxAlgiVisionProcess
                (
                    prms->algHandle,
                    &prms->inBufs,
                    NULL,
                    (IVISION_InArgs  *)&prms->inArgs,
                    (IVISION_OutArgs *)&prms->outArgs,
                    0
                );

        if(status!=TIADALG_PROCESS_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tiadalg failed !!!\n");
        }

        prms->inArgs.en_reset = 0;
        /*copy outArgs in last tensor*/
        outTensor = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_VISUAL_LOCALIZATION_OUTPUT_POSE_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        ((vx_float32*)out_tensor_target_ptr)[0*4 + 0] = prms->outArgs.rotation[0];
        ((vx_float32*)out_tensor_target_ptr)[0*4 + 1] = prms->outArgs.rotation[1];
        ((vx_float32*)out_tensor_target_ptr)[0*4 + 2] = prms->outArgs.rotation[2];
        ((vx_float32*)out_tensor_target_ptr)[1*4 + 0] = prms->outArgs.rotation[3];
        ((vx_float32*)out_tensor_target_ptr)[1*4 + 1] = prms->outArgs.rotation[4];
        ((vx_float32*)out_tensor_target_ptr)[1*4 + 2] = prms->outArgs.rotation[5];
        ((vx_float32*)out_tensor_target_ptr)[2*4 + 0] = prms->outArgs.rotation[6];
        ((vx_float32*)out_tensor_target_ptr)[2*4 + 1] = prms->outArgs.rotation[7];
        ((vx_float32*)out_tensor_target_ptr)[2*4 + 2] = prms->outArgs.rotation[8];

        ((vx_float32*)out_tensor_target_ptr)[0*4 + 3] = prms->outArgs.translation[0];
        ((vx_float32*)out_tensor_target_ptr)[1*4 + 3] = prms->outArgs.translation[1];
        ((vx_float32*)out_tensor_target_ptr)[2*4 + 3] = prms->outArgs.translation[2];

        tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

    }

    return (status);
}

void tivxAddTargetKernelVisualLocalization()
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
        vx_egoLocal_kernel = tivxAddTargetKernelByName
                                (
                                  TIVX_KERNEL_VISUAL_LOCALIZATION_NAME,
                                  target_name,
                                  tivxKernelVisualLocalizationProcess,
                                  tivxKernelVisualLocalizationCreate,
                                  tivxKernelVisualLocalizationDelete,
                                  tivxKernelVisualLocalizationControl,
                                  NULL
                                );
    }
}

void tivxRemoveTargetKernelVisualLocalization()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_egoLocal_kernel);
    if (status == VX_SUCCESS)
    {
        vx_egoLocal_kernel = NULL;
    }
}
static void tivxVLFreeMem(tivxVisualLocalizationKernelParams *prms)
{
    if (NULL != prms)
    {
        tivxMemFree(prms, sizeof(tivxVisualLocalizationKernelParams), TIVX_MEM_EXTERNAL);
    }
}
static int32_t vl_alg_fe_correct(int32_t* in_pt_list, int32_t num_points, float* lens_dist_table, int32_t table_len,
                                  int32_t* in_pruned_pt_list, float* out_pt_list, int32_t dl_width, int32_t dl_height,
                                  int32_t img_width, int32_t img_height)
{
    int32_t i;
    int32_t orig_x_fe,orig_y_fe;
    float in_x_fe,in_y_fe,in_x_fec,in_y_fec;
    float cx = (float)(img_width  >> 1);
    float cy = (float)(img_height >> 1);
    int32_t num_out_points=0;
    float r_rect_0,r_rect_1,r_fe_0,r_fe_1,r_fe;
    float r_rect = 0.0f;

    for(i = 0; i < num_points; i++)
    {
        orig_x_fe = in_pt_list[2*i];
        orig_y_fe = in_pt_list[2*i + 1];

        in_x_fe = ((float)(orig_x_fe * img_width))/dl_width;
        in_y_fe = ((float)(orig_y_fe * img_height))/dl_height;

        r_fe     = sqrtf((in_x_fe - cx)*(in_x_fe - cx) + (in_y_fe - cy)*(in_y_fe - cy));

        r_fe_1   = (ceilf(r_fe) < (table_len - 1.0) ? ceilf(r_fe) : (table_len - 1.0));
        r_fe_1   = (0.f > r_fe_1) ? 0.0f : r_fe_1;
        r_fe_0   = (r_fe_1 - 1);
        r_fe_0   = (0.f > r_fe_0) ? 0.0f : r_fe_0;

        r_rect_1   = lens_dist_table[(int32_t)r_fe_1];
        r_rect_0   = lens_dist_table[(int32_t)r_fe_0];

        vl_alg_interpolate(r_fe,r_fe_0,r_fe_1,r_rect_0,r_rect_1,&r_rect);

        in_x_fec  = cx + ((r_rect/r_fe)*(in_x_fe - cx));
        in_y_fec  = cy + ((r_rect/r_fe)*(in_y_fe - cy));

        if((in_x_fec > 0) && (in_x_fec < img_width) && (in_y_fec > 0) && (in_y_fec < img_height))
        {
            out_pt_list[2*num_out_points] = in_x_fec;
            out_pt_list[2*num_out_points + 1] = in_y_fec;
            in_pruned_pt_list[2*num_out_points] = orig_x_fe;
            in_pruned_pt_list[2*num_out_points + 1] = orig_y_fe;
            num_out_points++;
        }
    }
    return(num_out_points);
}

static inline uint8_t vl_alg_interpolate(float xPosition, float xRangeLower, float xRangeHigher, float yRangeLower,
                                float yRangeHigher, float *yInterpolated)
{
    if (xPosition >= xRangeLower && xPosition <= xRangeHigher)
    {
        float locXPosition = xPosition;
        float locXRangeLower = xRangeLower;
        float locXRangeHigher = xRangeHigher;
        float locYRangeLower = yRangeLower;
        float locYRangeHigher = yRangeHigher;
        float locYInterpolated;
        float weight1 = 1.0 - (locXPosition - locXRangeLower) / (locXRangeHigher - locXRangeLower);
        float weight2 = 1.0 - (locXRangeHigher - locXPosition) / (locXRangeHigher - locXRangeLower);

        locYInterpolated = weight1 * locYRangeLower + weight2 * locYRangeHigher;
        *yInterpolated = (float)(locYInterpolated);
        return 0x1;
    }
    else {
        *yInterpolated = yRangeLower;
        return 0x0;
    }
}

