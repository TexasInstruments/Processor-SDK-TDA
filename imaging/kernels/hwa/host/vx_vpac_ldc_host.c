/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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

#ifdef BUILD_VPAC_LDC

#include "TI/tivx.h"
#include "TI/hwa_vpac_ldc.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

/* Stride alignment requirements for input/output of LDC.
 * i.e., it must be a multiple of these numbers */
#define TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT  16
#define TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT 8

static vx_kernel vx_vpac_ldc_kernel = NULL;
static vx_kernel vx_vpac_ldc2_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVpacLdcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVpacLdcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelVpacLdcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_vpac_ldc_params_t params;

    vx_matrix warp_matrix = NULL;
    vx_enum warp_matrix_type;
    vx_size warp_matrix_w, warp_matrix_h;

    vx_user_data_object region_prms = NULL;
    vx_char region_prms_name[VX_MAX_REFERENCE_NAME];
    vx_size region_prms_size;
    tivx_vpac_ldc_region_params_t ldc_region_params;

    vx_user_data_object mesh_prms = NULL;
    vx_char mesh_prms_name[VX_MAX_REFERENCE_NAME];
    vx_size mesh_prms_size;
    tivx_vpac_ldc_mesh_params_t ldc_mesh_params;

    vx_image mesh_img = NULL;
    vx_df_image mesh_img_fmt;
    vx_uint32 mesh_img_stride_y_alignment;

    vx_user_data_object dcc_db = NULL;
    vx_char dcc_db_name[VX_MAX_REFERENCE_NAME];
    vx_size dcc_db_size;

    vx_image in0_img = NULL;
    vx_df_image in0_img_fmt;
    vx_uint32 in0_img_stride_y_alignment;
    vx_uint32 in0_img_w;
    vx_uint32 in0_img_h;

    vx_image in1_img = NULL;
    vx_df_image in1_img_fmt;
    vx_uint32 in1_img_stride_y_alignment;
    vx_uint32 in1_img_w;
    vx_uint32 in1_img_h;

    vx_image out0_img = NULL;
    vx_uint32 out0_img_w;
    vx_uint32 out0_img_h;
    vx_df_image out0_img_fmt;
    vx_uint32 out0_img_stride_y_alignment;

    vx_image out1_img = NULL;
    vx_uint32 out1_img_w;
    vx_uint32 out1_img_h;
    vx_df_image out1_img_fmt;
    vx_uint32 out1_img_stride_y_alignment;

     vx_image out2_img = NULL;
    vx_uint32 out2_img_w;
    vx_uint32 out2_img_h;
    vx_df_image out2_img_fmt = 0;
    vx_uint32 out2_img_stride_y_alignment;

     vx_image out3_img = NULL;
    vx_uint32 out3_img_w;
    vx_uint32 out3_img_h;
    vx_df_image out3_img_fmt = 0;
    vx_uint32 out3_img_stride_y_alignment;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( ((num != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
        && ((num != TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX]))
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((vx_status)VX_SUCCESS == status) && (num == TIVX_KERNEL_VPAC_LDC_MAX_PARAMS))
    /* LDRA_JUSTIFY_END */
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
        warp_matrix = (vx_matrix)parameters[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
        region_prms = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC_REGION_PRMS_IDX];
        mesh_prms = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC_MESH_PRMS_IDX];
        mesh_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_MESH_IMG_IDX];
        dcc_db = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC_DCC_DB_IDX];
        in0_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX];
        out0_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
        out1_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((vx_status)VX_SUCCESS == status) && (num == TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS))
    /* LDRA_JUSTIFY_END */
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX];
        warp_matrix = (vx_matrix)parameters[TIVX_KERNEL_VPAC_LDC2_WARP_MATRIX_IDX];
        region_prms = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC2_REGION_PRMS_IDX];
        mesh_prms = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC2_MESH_PRMS_IDX];
        mesh_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_MESH_IMG_IDX];
        dcc_db = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_LDC2_DCC_DB_IDX];
        in0_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX];
        in1_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_IN1_IMG_IDX];
        out0_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX];
        out1_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_OUT1_IMG_IDX];
        out2_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_OUT2_IMG_IDX];
        out3_img = (vx_image)parameters[TIVX_KERNEL_VPAC_LDC2_OUT3_IMG_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_ldc_params_t), &params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        if (NULL != warp_matrix)
        {
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, (vx_enum)VX_MATRIX_TYPE, &warp_matrix_type, sizeof(warp_matrix_type)));
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, (vx_enum)VX_MATRIX_COLUMNS, &warp_matrix_w, sizeof(warp_matrix_w)));
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, (vx_enum)(vx_enum)VX_MATRIX_ROWS, &warp_matrix_h, sizeof(warp_matrix_h)));
        }

        if (NULL != region_prms)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(region_prms, (vx_enum)VX_USER_DATA_OBJECT_NAME, &region_prms_name, sizeof(region_prms_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(region_prms, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &region_prms_size, sizeof(region_prms_size)));
            tivxCheckStatus(&status, vxCopyUserDataObject(region_prms, 0, sizeof(tivx_vpac_ldc_region_params_t), &ldc_region_params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));
        }

        if (NULL != mesh_prms)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(mesh_prms, (vx_enum)VX_USER_DATA_OBJECT_NAME, &mesh_prms_name, sizeof(mesh_prms_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(mesh_prms, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &mesh_prms_size, sizeof(mesh_prms_size)));
            tivxCheckStatus(&status, vxCopyUserDataObject(mesh_prms, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &ldc_mesh_params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));
        }

        if (NULL != mesh_img)
        {
            tivxCheckStatus(&status, vxQueryImage(mesh_img, (vx_enum)VX_IMAGE_FORMAT, &mesh_img_fmt, sizeof(mesh_img_fmt)));
            tivxCheckStatus(&status, vxQueryImage(mesh_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &mesh_img_stride_y_alignment, sizeof(mesh_img_stride_y_alignment)));
        }

        if (NULL != dcc_db)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_db, (vx_enum)VX_USER_DATA_OBJECT_NAME, &dcc_db_name, sizeof(dcc_db_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_db, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &dcc_db_size, sizeof(dcc_db_size)));
        }

        tivxCheckStatus(&status, vxQueryImage(in0_img, (vx_enum)VX_IMAGE_FORMAT, &in0_img_fmt, sizeof(in0_img_fmt)));
        tivxCheckStatus(&status, vxQueryImage(in0_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &in0_img_stride_y_alignment, sizeof(in0_img_stride_y_alignment)));
        tivxCheckStatus(&status, vxQueryImage(in0_img, (vx_enum)VX_IMAGE_WIDTH, &in0_img_w, sizeof(in0_img_w)));
        tivxCheckStatus(&status, vxQueryImage(in0_img, (vx_enum)VX_IMAGE_HEIGHT, &in0_img_h, sizeof(in0_img_h)));

        if (NULL != in1_img)
        {
            tivxCheckStatus(&status, vxQueryImage(in1_img, (vx_enum)VX_IMAGE_FORMAT, &in1_img_fmt, sizeof(in1_img_fmt)));
            tivxCheckStatus(&status, vxQueryImage(in1_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &in1_img_stride_y_alignment, sizeof(in1_img_stride_y_alignment)));
            tivxCheckStatus(&status, vxQueryImage(in1_img, (vx_enum)VX_IMAGE_WIDTH, &in1_img_w, sizeof(in1_img_w)));
            tivxCheckStatus(&status, vxQueryImage(in1_img, (vx_enum)VX_IMAGE_HEIGHT, &in1_img_h, sizeof(in1_img_h)));
        }

        tivxCheckStatus(&status, vxQueryImage(out0_img, (vx_enum)VX_IMAGE_WIDTH, &out0_img_w, sizeof(out0_img_w)));
        tivxCheckStatus(&status, vxQueryImage(out0_img, (vx_enum)VX_IMAGE_HEIGHT, &out0_img_h, sizeof(out0_img_h)));
        tivxCheckStatus(&status, vxQueryImage(out0_img, (vx_enum)VX_IMAGE_FORMAT, &out0_img_fmt, sizeof(out0_img_fmt)));
        tivxCheckStatus(&status, vxQueryImage(out0_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &out0_img_stride_y_alignment, sizeof(out0_img_stride_y_alignment)));

        if (NULL != out1_img)
        {
            tivxCheckStatus(&status, vxQueryImage(out1_img, (vx_enum)VX_IMAGE_WIDTH, &out1_img_w, sizeof(out1_img_w)));
            tivxCheckStatus(&status, vxQueryImage(out1_img, (vx_enum)VX_IMAGE_HEIGHT, &out1_img_h, sizeof(out1_img_h)));
            tivxCheckStatus(&status, vxQueryImage(out1_img, (vx_enum)VX_IMAGE_FORMAT, &out1_img_fmt, sizeof(out1_img_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out1_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &out1_img_stride_y_alignment, sizeof(out1_img_stride_y_alignment)));
        }

        if (NULL != out2_img)
        {
            tivxCheckStatus(&status, vxQueryImage(out2_img, (vx_enum)VX_IMAGE_WIDTH, &out2_img_w, sizeof(out2_img_w)));
            tivxCheckStatus(&status, vxQueryImage(out2_img, (vx_enum)VX_IMAGE_HEIGHT, &out2_img_h, sizeof(out2_img_h)));
            tivxCheckStatus(&status, vxQueryImage(out2_img, (vx_enum)VX_IMAGE_FORMAT, &out2_img_fmt, sizeof(out2_img_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out2_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &out2_img_stride_y_alignment, sizeof(out2_img_stride_y_alignment)));
        }

        if (NULL != out3_img)
        {
            tivxCheckStatus(&status, vxQueryImage(out3_img, (vx_enum)VX_IMAGE_WIDTH, &out3_img_w, sizeof(out3_img_w)));
            tivxCheckStatus(&status, vxQueryImage(out3_img, (vx_enum)VX_IMAGE_HEIGHT, &out3_img_h, sizeof(out3_img_h)));
            tivxCheckStatus(&status, vxQueryImage(out3_img, (vx_enum)VX_IMAGE_FORMAT, &out3_img_fmt, sizeof(out3_img_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out3_img, (vx_enum)TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &out3_img_stride_y_alignment, sizeof(out3_img_stride_y_alignment)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_vpac_ldc_params_t)) ||
            (strncmp(configuration_name, "tivx_vpac_ldc_params_t", sizeof(configuration_name)) != (int32_t)0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_vpac_ldc_params_t \n");
        }

        if (NULL != warp_matrix)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if( ((vx_enum)VX_TYPE_INT16 != warp_matrix_type) &&
                ((vx_enum)VX_TYPE_FLOAT32 != warp_matrix_type))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'warp_matrix' should be a matrix of type:\n VX_TYPE_INT16 or VX_TYPE_FLOAT32 \n");
            }
            if ((2U != warp_matrix_w) && (3U != warp_matrix_w))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR,
                    "'warp_matrix' columns should be 2 (affine warp) or 3 (perspective warp) \n");
            }
            /* LDRA_JUSTIFY_END */
            if (3U != warp_matrix_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR,
                    "'warp_matrix' rows should be 3 \n");
            }
        }

        if (NULL != region_prms)
        {
            if( ((region_prms_size != sizeof(tivx_vpac_ldc_region_params_t)) ||
                 (strncmp(region_prms_name, "tivx_vpac_ldc_region_params_t", sizeof(region_prms_name)) != (int32_t)0)) &&
                ((region_prms_size != sizeof(tivx_vpac_ldc_multi_region_params_t)) ||
                 (strncmp(region_prms_name, "tivx_vpac_ldc_multi_region_params_t", sizeof(region_prms_name)) != (int32_t)0)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'region_prms' should be a user_data_object of type:\n tivx_vpac_ldc_region_params_t or tivx_vpac_ldc_multi_region_params_t \n");
            }
        }

        if (NULL != mesh_prms)
        {
            if ((mesh_prms_size != sizeof(tivx_vpac_ldc_mesh_params_t)) ||
                (strncmp(mesh_prms_name, "tivx_vpac_ldc_mesh_params_t", sizeof(mesh_prms_name)) != (int32_t)0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'mesh_prms' should be a user_data_object of type:\n tivx_vpac_ldc_mesh_params_t \n");
            }
        }

        if (NULL != mesh_img)
        {
            if ((vx_df_image)VX_DF_IMAGE_U32 != mesh_img_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'mesh_img' should be an image of type:\n VX_DF_IMAGE_U32 \n");
            }

            if ( (mesh_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT)) != 0U)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'mesh_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT);
            }
        }

        if (NULL != dcc_db)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((dcc_db_size < 1U) ||
                (strncmp(dcc_db_name, "dcc_ldc", sizeof(dcc_db_name)) != (int32_t)0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'dcc_db' should be a user_data_object of type:\n dcc_ldc \n");
            }
            /* LDRA_JUSTIFY_END */
        }

        if ( (in0_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT)) != 0U)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in0_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT);
        }
        if(NULL != in1_img)
        {
            if ( (in1_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT)) != 0U)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in1_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_INPUT_STRIDE_ALIGNMENT);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ( (out0_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT)) != 0U)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out0_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT);
        }
        /* LDRA_JUSTIFY_END */
        if(NULL != out1_img)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ( (out1_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT)) != 0U)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out1_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT);
            }
            /* LDRA_JUSTIFY_END */
        }
        if(NULL != out2_img)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ( (out2_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT)) != 0U)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out2_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT);
            }
            /* LDRA_JUSTIFY_END */
        }
        if(NULL != out3_img)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ( (out3_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT)) != 0U)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out3_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT);
            }
            /* LDRA_JUSTIFY_END */
        }
        if(NULL == in1_img)
        {
            if((NULL != out2_img) || (NULL != out3_img))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out2_img' and 'out3_img' should be NULL when in1_img is NULL \n");
            }
            if( ((vx_df_image)VX_DF_IMAGE_NV12 != in0_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 != in0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != in0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U8 != in0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != in0_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != in0_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' should be an image of type:\n VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if( ((vx_df_image)VX_DF_IMAGE_NV12 != out0_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 != out0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != out0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_YUYV != out0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U8 != out0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != out0_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != out0_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out0_img' should be an image of type:\n VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
            /* LDRA_JUSTIFY_END */

            if ((((vx_df_image)VX_DF_IMAGE_UYVY == out0_img_fmt) ||
                ((vx_df_image)VX_DF_IMAGE_YUYV == out0_img_fmt)) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != in0_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out0_img' is YUV422I but 'in_img' is not YUV422I \n");
            }

            if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U8) &&
                !((out0_img_fmt == (vx_df_image)VX_DF_IMAGE_U8) || (out0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_U8 but 'out0_img' is not VX_DF_IMAGE_U8 or TIVX_DF_IMAGE_P12 \n");
            }

            if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U16) &&
                (out0_img_fmt != (vx_df_image)VX_DF_IMAGE_U16))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_U16 but 'out0_img' is not VX_DF_IMAGE_U16 \n");
            }

            if ( (in0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12) &&
                !((out0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12) || (out0_img_fmt == (vx_df_image)VX_DF_IMAGE_U8)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is TIVX_DF_IMAGE_P12 but 'out0_img' is not TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_U8 \n");
            }

            if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) &&
                !((out0_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) || (out0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_NV12 but 'out0_img' is not VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12 \n");
            }

            if ( (in0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) &&
                !((out0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) || (out0_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is TIVX_DF_IMAGE_NV12_P12 but 'out0_img' is not TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_NV12 \n");
            }

            if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_UYVY) &&
                !((out0_img_fmt == (vx_df_image)VX_DF_IMAGE_UYVY) || (out0_img_fmt == (vx_df_image)VX_DF_IMAGE_YUYV) ||
                (out0_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) || (out0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) ))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_UYVY but 'out0_img' is not VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12\n");
            }

            if (NULL != out1_img)
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ( (out1_img_stride_y_alignment % ((uint32_t)TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT)) != 0U)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out1_img' stride y should be a multiple of %d \n", TIVX_KERNEL_VPAC_LDC_OUTPUT_STRIDE_ALIGNMENT);
                }
                /* LDRA_JUSTIFY_END */
                if( ((vx_df_image)VX_DF_IMAGE_NV12 != out1_img_fmt) &&
                    ((vx_df_image)VX_DF_IMAGE_UYVY != out1_img_fmt) &&
                    ((vx_df_image)VX_DF_IMAGE_YUYV != out1_img_fmt) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 != out1_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out1_img' should be an image of type:\n VX_DF_IMAGE_NV12 or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_U8 \n");
                }

                if ((((vx_df_image)VX_DF_IMAGE_UYVY == out1_img_fmt) ||
                    ((vx_df_image)VX_DF_IMAGE_YUYV == out1_img_fmt)) &&
                    ((vx_df_image)VX_DF_IMAGE_UYVY != in0_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Output1 is YUV422I Input is not YUV422I \n");
                }

                if ((((vx_df_image)VX_DF_IMAGE_NV12 == out0_img_fmt) ||
                    ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == out0_img_fmt) ||
                    ((vx_df_image)VX_DF_IMAGE_UYVY == out1_img_fmt) ||
                    ((vx_df_image)VX_DF_IMAGE_YUYV == out1_img_fmt)) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == out1_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Output1 cannot be single plane if output0 is multi-plane \n");
                }

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((((vx_df_image)VX_DF_IMAGE_U8 == out0_img_fmt) ||
                    ((vx_df_image)VX_DF_IMAGE_U16 == out0_img_fmt) ||
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out0_img_fmt)) &&
                    ((vx_df_image)VX_DF_IMAGE_NV12 == out1_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Output1 cannot be multi-plane if output0 is single-plane \n");
                }
                /* LDRA_JUSTIFY_END */

                if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U8) &&
                    !((out1_img_fmt == (vx_df_image)VX_DF_IMAGE_U8) || (out1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_U8 but 'out1_img' is not VX_DF_IMAGE_U8 or TIVX_DF_IMAGE_P12 \n");
                }

                if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U16) &&
                    !((out1_img_fmt == (vx_df_image)VX_DF_IMAGE_U16) || (out1_img_fmt == (vx_df_image)VX_DF_IMAGE_U8)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_U16 but 'out1_img' is not VX_DF_IMAGE_U16 or VX_DF_IMAGE_U8 \n");
                }

                if ( (in0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12) &&
                    !((out1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12) || (out1_img_fmt == (vx_df_image)VX_DF_IMAGE_U8)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is TIVX_DF_IMAGE_P12 but 'out1_img' is not TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_U8 \n");
                }

                if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) &&
                    !((out1_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) || (out1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_NV12 but 'out1_img' is not VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12 \n");
                }

                if ( (in0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) &&
                    !((out1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) || (out1_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is TIVX_DF_IMAGE_NV12_P12 but 'out1_img' is not TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_NV12 \n");
                }

                if ( (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_UYVY) &&
                    !((out1_img_fmt == (vx_df_image)VX_DF_IMAGE_UYVY) || (out1_img_fmt == (vx_df_image)VX_DF_IMAGE_YUYV) ||
                    (out1_img_fmt == (vx_df_image)VX_DF_IMAGE_NV12) || (out1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_NV12_P12) ))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in_img' is VX_DF_IMAGE_UYVY but 'out1_img' is not VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_NV12 or TIVX_DF_IMAGE_NV12_P12\n");
                }
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((NULL != out1_img) && (NULL == in1_img))
        {
            if (out1_img_w != out0_img_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'out1_img' and 'out0_img' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (out1_img_h != out0_img_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'out1_img' and 'out0_img' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        
        if(NULL != in1_img)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if( ((vx_df_image)VX_DF_IMAGE_U8 != in0_img_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != in0_img_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != in0_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in0_img' should be an image of type:\n  VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
            if( ((vx_df_image)VX_DF_IMAGE_U8 != in1_img_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != in1_img_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != in1_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in1_img' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
            /* LDRA_JUSTIFY_END */
            if(NULL == out2_img)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out2_img'  shouldn't be NULL when 'in1_img' is not NULL \n");
            }
            if(((NULL == out1_img) && (NULL != out3_img)) || ((NULL != out1_img) && (NULL == out3_img)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Both 'out1_img' and 'out3_img'  should be NULL or not NULL when 'in1_img' is not NULL \n");
            }
            if( ((vx_df_image)VX_DF_IMAGE_U8 != out0_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != out0_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != out0_img_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out0_img' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
            if(NULL != out1_img)
            {
                if( ((vx_df_image)VX_DF_IMAGE_U8 != out1_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != out1_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != out1_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out1_img' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
                }
            }
            if(NULL != out2_img)
            {
                if( ((vx_df_image)VX_DF_IMAGE_U8 != out2_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != out2_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != out2_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out2_img' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
                }
            }
            if(NULL != out3_img)
            {
                if( ((vx_df_image)VX_DF_IMAGE_U8 != out3_img_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != out3_img_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != out3_img_fmt))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out3_img' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
                }
            }
            if (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U8)
            {
                if((out0_img_fmt != (vx_df_image)VX_DF_IMAGE_U8))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in0_img' is VX_DF_IMAGE_U8 but 'out0_img' is not VX_DF_IMAGE_U8 \n");
                }
                if(NULL != out1_img)
                {
                    if(out1_img_fmt != (vx_df_image)VX_DF_IMAGE_U8)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in0_img' is VX_DF_IMAGE_U8 but 'out1_img' is not VX_DF_IMAGE_U8 \n");
                    }
                }
            }
            if (in1_img_fmt == (vx_df_image)VX_DF_IMAGE_U8)
            {
                if(out2_img_fmt != (vx_df_image)VX_DF_IMAGE_U8)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in1_img' is VX_DF_IMAGE_U8 but 'out2_img' is not VX_DF_IMAGE_U8 \n");
                }
                if(NULL != out3_img)
                {
                    if(out3_img_fmt != (vx_df_image)VX_DF_IMAGE_U8)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in1_img' is VX_DF_IMAGE_U8 but 'out3_img' is not VX_DF_IMAGE_U8 \n");
                    }
                }
            }
            if (in0_img_fmt == (vx_df_image)VX_DF_IMAGE_U16)
            {
                if(out0_img_fmt != (vx_df_image)VX_DF_IMAGE_U16)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in0_img' is VX_DF_IMAGE_U16 but 'out0_img' is not VX_DF_IMAGE_U16 \n");
                }
                if(NULL != out1_img)
                {
                    if(out1_img_fmt != (vx_df_image)VX_DF_IMAGE_U16)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in0_img' is VX_DF_IMAGE_U16 but 'out1_img' is not VX_DF_IMAGE_U16 \n");
                    }
                }
            }
            if (in1_img_fmt == (vx_df_image)VX_DF_IMAGE_U16)
            {
                if(out2_img_fmt != (vx_df_image)VX_DF_IMAGE_U16)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in1_img' is VX_DF_IMAGE_U16 but 'out2_img' is not VX_DF_IMAGE_U16 \n");
                }
                if(NULL != out3_img)
                {
                    if(out3_img_fmt != (vx_df_image)VX_DF_IMAGE_U16)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in1_img' is VX_DF_IMAGE_U16 but 'out3_img' is not VX_DF_IMAGE_U16 \n");
                    }
                }
            }
            if (in0_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12)
            {
                if(out0_img_fmt != (vx_df_image)TIVX_DF_IMAGE_P12)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in0_img' is TIVX_DF_IMAGE_P12 but 'out0_img' is not TIVX_DF_IMAGE_P12 \n");
                }
                if(NULL != out1_img)
                {
                    if(out1_img_fmt != (vx_df_image)TIVX_DF_IMAGE_P12)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in0_img' is TIVX_DF_IMAGE_P12 but 'out1_img' is not TIVX_DF_IMAGE_P12 \n");
                    }
                }
            }
            if (in1_img_fmt == (vx_df_image)TIVX_DF_IMAGE_P12)
            {
                if(out2_img_fmt != (vx_df_image)TIVX_DF_IMAGE_P12)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'in1_img' is TIVX_DF_IMAGE_P12 but 'out2_img' is not TIVX_DF_IMAGE_P12 \n");
                }
                if(NULL != out3_img)
                {
                    if(out3_img_fmt != (vx_df_image)TIVX_DF_IMAGE_P12)
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'in1_img' is TIVX_DF_IMAGE_P12 but 'out3_img' is not TIVX_DF_IMAGE_P12 \n");
                    }
                }
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if ((NULL != in0_img) && (NULL != in1_img))
        /* LDRA_JUSTIFY_END */
        {
            if (in1_img_w != in0_img_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'in1_img' and 'in0_img' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (out2_img_w != out0_img_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'out2_img' and 'out0_img' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if((NULL != out1_img) && (NULL != out3_img))
            {
                if (out3_img_w != out1_img_w)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'out3_img' and 'out1_img' should have the same value for VX_IMAGE_WIDTH\n");
                }
            }
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (1U < params.input_align_12bit)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter input_align_12bit should be either 0 or 1\n");
        }
        if (1U < params.luma_interpolation_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter luma_interpolation_type should be either 0 or 1\n");
        }
        if (8191U < params.init_x)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter init_x should be between 0 and 8191 inclusive\n");
        }
        else if (0U != (params.init_x % 8U))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter init_x should be a multiple of 8\n");
        }
        else
        {
            /* do nothing */
        }
        if (8191U < params.init_y)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter init_y should be between 0 and 8191 inclusive\n");
        }
        else if (0U != (params.init_y % 2U))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter init_y should be a multiple of 2\n");
        }
        else
        {
            /* do nothing */
        }
        if (1U < params.yc_mode)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter yc_mode should be either 0 or 1\n");
        }
        if (NULL != region_prms)
        {
            if (1U < ldc_region_params.enable)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter enable should be either 0 or 1\n");
            }
            if ((8U > ldc_region_params.out_block_width) ||
                (255U < ldc_region_params.out_block_width))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter out_block_width should be between 8 and 255 inclusive\n");
            }
            else if (0U != (ldc_region_params.out_block_width % 8U))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter out_block_width should be a multiple of 8\n");
            }
            else
            {
                /* do nothing */
            }
            if ((2U > ldc_region_params.out_block_height) ||
                (255U < ldc_region_params.out_block_height))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter out_block_height should be between 2 and 255 inclusive\n");
            }
            else if (0U != (ldc_region_params.out_block_height % 2U))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter out_block_height should be a multiple of 2\n");
            }
            else
            {
                /* do nothing */
            }
            if (15U < ldc_region_params.pixel_pad)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Region parameter pixel_pad should be between 0 and 15 inclusive\n");
            }
        }
        if (NULL != mesh_prms)
        {
            if (7U < ldc_mesh_params.subsample_factor)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Mesh parameter subsample_factor should be between 0 and 7 inclusive\n");
            }
        }

        if (NULL != dcc_db)
        {
            if ((NULL != warp_matrix) || (NULL != region_prms) || (NULL != mesh_prms) || (NULL != mesh_img))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "If dcc_db is used, then the following inputs should be NULL: warp_matrix, region_prms, mesh_prms, mesh_img.\n");
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacLdcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
        && ((num_params != TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX]))
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] =
            (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX];
        prms.num_input_images = 1U;

        prms.out_img[0U] =
            (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
        prms.num_output_images = 1U;

        if(NULL != parameters[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX])
        {
            prms.out_img[1U] =
                (vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
            prms.num_output_images = 2U;
        }

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelVpacLdc(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_VPAC_LDC_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_LDC_MAX_PARAMS,
                    tivxAddKernelVpacLdcValidate,
                    tivxAddKernelVpacLdcInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        param_idx = 0U;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_MATRIX,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_LDC1));
            #if (VPAC_COUNT > 1)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_LDC1));
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            (void)vxReleaseKernel(&kernel);
            kernel = NULL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_vpac_ldc_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacLdc(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_ldc_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_ldc_kernel = NULL;

    return status;
}

vx_status tivxAddKernelVpacLdc2(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_VPAC_LDC2_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS,
                    tivxAddKernelVpacLdcValidate,
                    tivxAddKernelVpacLdcInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    {
        /* LDRA_JUSTIFY_END */
        param_idx = 0U;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_MATRIX,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_LDC1));
            #if (VPAC_COUNT > 1)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_LDC1));
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            (void)vxReleaseKernel(&kernel);
            kernel = NULL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_vpac_ldc2_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacLdc2(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_ldc2_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_ldc2_kernel = NULL;

    return status;
}
void tivx_vpac_ldc_region_params_init(tivx_vpac_ldc_region_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_ldc_region_params_t));
        prms->enable = 0u;
        prms->out_block_width = 16u;
        prms->out_block_height = 16u;
    }
}

void tivx_vpac_ldc_multi_region_prms_init(tivx_vpac_ldc_multi_region_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_ldc_multi_region_params_t));
    }
}
void tivx_vpac_ldc_mesh_params_init(tivx_vpac_ldc_mesh_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_ldc_mesh_params_t));
        prms->subsample_factor = 0u;
    }
}


void tivx_vpac_ldc_bandwidth_prms_init(tivx_vpac_ldc_bandwidth_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_ldc_bandwidth_params_t));

        /* Default values are set based on the reset values in registers */
        prms->bandwidth_control = 0u;
        prms->tag_count = 31u;
        prms->max_burst_length = 1u;
    }
}

void tivx_vpac_ldc_params_init(tivx_vpac_ldc_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_ldc_params_t));
        /* By Default LSB Aligned pixels */
        prms->input_align_12bit = TIVX_VPAC_LDC_ALIGN_LSB;
        /* By Default Bilinear Interpolation */
        prms->luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;
        prms->init_x = 0u;
        prms->init_y = 0u;
        prms->yc_mode = TIVX_VPAC_LDC_MODE_LUMA_ONLY;
        #if defined(VPAC3) || defined(VPAC3L)
        prms->in_img0_yc_mode = TIVX_VPAC_LDC_MODE_LUMA_ONLY;
        prms->in_img1_yc_mode = TIVX_VPAC_LDC_MODE_LUMA_ONLY;
        #endif
        prms->enable_error_events = 0u;
        prms->enable_psa = 0u;
        #if defined(VPAC3) || defined(VPAC3L)
        /* By Default One Shot mode */
        prms->continuous_mode = TIVX_VPAC_LDC_CONTINUOUS_MODE_ONE_SHOT;
        #endif
        prms->wdtimer_timeout_cycles = TIVX_VPAC_LDC_WDTIMER_64K_CYCLES;

    }
}

#if !defined(VPAC3L)
void tivx_vpac_ldc_safety_mech_prms_init(tivx_vpac_ldc_safety_mechanism_params_t *prms)
{
    if (NULL != prms)
    {
        prms->enable_reconfig_and_reinit_Reg = (uint32_t)vx_false_e;
        prms->enable_status_reg_validation = (uint32_t)vx_false_e;
        prms->enable_readback_config_registers = (uint32_t)vx_false_e;
    }
}
#endif

#endif
