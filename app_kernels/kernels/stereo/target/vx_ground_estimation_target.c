/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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
#include "tivx_kernel_ground_estimation.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <vx_ptk_alg_common.h>


static tivx_target_kernel vx_ground_estimation_target_kernel = NULL;

static vx_status VX_CALLBACK tivxGroundEstimationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGroundEstimationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGroundEstimationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGroundEstimationControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxGroundEstimationProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *input_image_desc;
    tivx_obj_desc_image_t *input_sdedisparity_desc;
    tivx_obj_desc_image_t *output_disparity_desc;
    tivx_obj_desc_user_data_object_t *output_ground_model_desc;

    if ( (num_params != TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX];
        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX];
        input_sdedisparity_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX];
        output_disparity_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX];
        output_ground_model_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX];
    }

    if(VX_SUCCESS == status)
    {
        void *configuration_target_ptr;
        void *input_image_target_ptr;
        void *input_sdedisparity_target_ptr;
        void *output_disparity_target_ptr;
        void *output_ground_model_target_ptr;

        tivx_ptk_alg_if_cntxt     * algCntxt;
        uint32_t                    size;

        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        input_image_target_ptr = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr,
           input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);

        input_sdedisparity_target_ptr = tivxMemShared2TargetPtr(&input_sdedisparity_desc->mem_ptr[0]);
        tivxMemBufferMap(input_sdedisparity_target_ptr,
           input_sdedisparity_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_READ_ONLY);

        output_disparity_target_ptr = tivxMemShared2TargetPtr(&output_disparity_desc->mem_ptr[0]);
        tivxMemBufferMap(output_disparity_target_ptr,
           output_disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);

        output_ground_model_target_ptr = tivxMemShared2TargetPtr(&output_ground_model_desc->mem_ptr);
        tivxMemBufferMap(output_ground_model_target_ptr,
           output_ground_model_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_AND_WRITE);


        /* < DEVELOPER_TODO: (Optional) Modify 'scalar data type' below to be correct type > */
        /*input_confidence_threshold_value = input_confidence_threshold_desc->data.<scalar data type>;*/

        /* call kernel processing function */
        /* get alg handle */
        algCntxt = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&algCntxt, &size);

        if ((VX_SUCCESS != status) || (NULL == algCntxt) ||
            (sizeof(tivx_ptk_alg_if_cntxt) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get algorithm handle!\n");
            status = VX_FAILURE;
        }
        else
        {
            PTK_Alg_StereoAlgo_runGroundEstimation(algCntxt->algHandle[0], (tivx_ground_model_params_t *)output_ground_model_target_ptr,
                                                   input_image_target_ptr, input_sdedisparity_target_ptr, 
                                                   output_disparity_target_ptr);
        }
        
#if 0
        {
            VXLIB_bufParams2D_t vxlib_input_image;
            uint8_t *input_image_addr = NULL;
            VXLIB_bufParams2D_t vxlib_input_sdedisparity;
            uint8_t *input_sdedisparity_addr = NULL;

            tivxInitBufParams(input_image_desc, &vxlib_input_image);
            tivxSetPointerLocation(input_image_desc, &input_image_target_ptr, &input_image_addr);

            tivxInitBufParams(input_sdedisparity_desc, &vxlib_input_sdedisparity);
            tivxSetPointerLocation(input_sdedisparity_desc, &input_sdedisparity_target_ptr, &input_sdedisparity_addr);

            /* call kernel processing function */

            /* < DEVELOPER_TODO: Add target kernel processing code here > */

            /* kernel processing function complete */
        }
#endif

        tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr,
           input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(input_sdedisparity_target_ptr,
           input_sdedisparity_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY);

        tivxMemBufferUnmap(output_disparity_target_ptr,
           output_disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxMemBufferUnmap(output_ground_model_target_ptr,
           output_ground_model_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_AND_WRITE);
    }

    return status;
}

static vx_status VX_CALLBACK tivxGroundEstimationCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */
    if ( (num_params != TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Interface parameter check failed.\n");
        status = VX_FAILURE;
    }
    else
    {
        tivx_ptk_alg_if_cntxt            * algCntxt;
        tivx_ground_estimation_params_t  * cfgParams;
        PTK_Api_MemoryReq                  memReq;
        int32_t                            algRetCode;
        uint32_t                           numMemReq;

        tivx_obj_desc_user_data_object_t * configuration_desc;
        tivx_obj_desc_image_t            * input_image_desc;
        tivx_obj_desc_image_t            * input_sdedisparity_desc;
        tivx_obj_desc_image_t            * output_disparity_desc;
        tivx_obj_desc_user_data_object_t * output_ground_model_desc;

        void                             * configuration_target_ptr;
        void                             * input_image_target_ptr;
        void                             * input_sdedisparity_target_ptr;
        void                             * output_disparity_target_ptr;
        void                             * output_ground_model_target_ptr;

        // configuration
        configuration_desc       = (tivx_obj_desc_user_data_object_t *)
                                   obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX];
        input_image_desc         = (tivx_obj_desc_image_t *)
                                   obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX];
        input_sdedisparity_desc  = (tivx_obj_desc_image_t *)
                                   obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX];
        output_disparity_desc    = (tivx_obj_desc_image_t *)
                                   obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX];
        output_ground_model_desc = (tivx_obj_desc_user_data_object_t *)
                                   obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX];
                                
        configuration_target_ptr       = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        input_image_target_ptr         = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        input_sdedisparity_target_ptr  = tivxMemShared2TargetPtr(&input_sdedisparity_desc->mem_ptr[0]);
        output_disparity_target_ptr    = tivxMemShared2TargetPtr(&output_disparity_desc->mem_ptr[0]);
        output_ground_model_target_ptr = tivxMemShared2TargetPtr(&output_ground_model_desc->mem_ptr);

        tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);
        tivxMemBufferMap(input_image_target_ptr,
           input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY);
        tivxMemBufferMap(input_sdedisparity_target_ptr,
           input_sdedisparity_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
           (vx_enum)VX_READ_ONLY);
        tivxMemBufferMap(output_disparity_target_ptr,
           output_disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY);
        tivxMemBufferMap(output_ground_model_target_ptr,
           output_ground_model_desc->mem_size, VX_MEMORY_TYPE_HOST,
           VX_READ_AND_WRITE);

        cfgParams  = (tivx_ground_estimation_params_t *)configuration_target_ptr;
        algRetCode = PTK_Alg_StereoAlgo_GroundEstimationConfig(cfgParams, &memReq);

        numMemReq = 1;
        /* Based on the API specification, 3 blocks are expected. */
        if ((algRetCode != PTK_ALG_RET_SUCCESS) ||
            !memReq.numBlks                    ||
            (memReq.numBlks != 2))
        {
            PTK_printf("PTK_Alg_StereoAlgo_GroundEstimationConfig() failed!\n");
            return ALGORITHM_PROCESS_FAIL;
        }

        if (status == VX_SUCCESS)
        {
            /* Create alg object */
            algCntxt = tivxPtkAlgCommonCreate(kernel, &memReq, numMemReq);

            if (NULL == algCntxt)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxPtkAlgCommonCreate() failed!\n");
                status = VX_FAILURE;
            }
            else
            {
                /* Initialize the stereo object detection library. */
                algCntxt->algHandle[0] =  PTK_Alg_StereoAlgo_GroundEstimationInit(cfgParams, &algCntxt->memRsp[0]);

                if (!algCntxt->algHandle[0])
                {
                    VX_PRINT(VX_ZONE_ERROR, "PTK_Alg_StereoAlgo_GroundEstimationInit() failed!\n");
                    status = VX_FAILURE;
                }

                PTK_Alg_StereoAlgo_GroundEstimationSetParams(cfgParams, (PTK_Alg_StereoAlgo_GroundEstimationObj *)algCntxt->algHandle[0]);
            }
        }

        tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr,
           input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(input_sdedisparity_target_ptr,
           input_sdedisparity_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY);

        tivxMemBufferUnmap(output_disparity_target_ptr,
           output_disparity_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxMemBufferUnmap(output_ground_model_target_ptr,
           output_ground_model_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_AND_WRITE);
    }


    return status;
}

static vx_status VX_CALLBACK tivxGroundEstimationDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    if ( (num_params != TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Interface parameter check failed.\n");
        status = VX_FAILURE;
    } else
    {
        tivx_ptk_alg_if_cntxt * algCntxt;
        uint32_t                size;

        /* Get the kernel context. */
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **)&algCntxt,
                                                    &size);

        if ((VX_SUCCESS != status) ||
            (NULL == algCntxt)     ||
            (sizeof(tivx_ptk_alg_if_cntxt) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get algorithm handle!\n");
            status = VX_FAILURE;
        } else
        {
            // TBD
        }

        status = tivxPtkAlgCommonDelete(kernel);

    }

    return status;
}

static vx_status VX_CALLBACK tivxGroundEstimationControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands */
    /*                   the user can call to modify the processing of the kernel at run-time) > */
    tivx_ptk_alg_if_cntxt * algCntxt;
    uint32_t                size;
    /* Get the kernel context. */
    status = tivxGetTargetKernelInstanceContext(kernel,
                                                (void **)&algCntxt,
                                                &size);
    if ((VX_SUCCESS != status) ||
        (NULL == algCntxt)     ||
        (sizeof(tivx_ptk_alg_if_cntxt) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to get algorithm handle!\n");
        status = VX_FAILURE;
    } else
    {
        if (node_cmd_id == TIVX_KERNEL_GROUND_ESTIMATION_RESET)
        {
            PTK_AlgHandle algHandle = algCntxt->algHandle[0];

            /* Issue the reset command. */
            PTK_Alg_StereoAlgo_GroundEstimationReset(algHandle);
        }
    }

    return status;
}

void tivxAddTargetKernelGroundEstimation(void)
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
            vx_ground_estimation_target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_GROUND_ESTIMATION_NAME,
                                target_name[i],
                                tivxGroundEstimationProcess,
                                tivxGroundEstimationCreate,
                                tivxGroundEstimationDelete,
                                tivxGroundEstimationControl,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelGroundEstimation(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_ground_estimation_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_ground_estimation_target_kernel = NULL;
    }
}


