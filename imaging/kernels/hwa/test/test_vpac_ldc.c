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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/hwa_kernels.h>
#include <TI/hwa_vpac_ldc.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <math.h>
#include <float.h>
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"
#include <utils/iss/include/app_iss.h>
#include <inttypes.h>
#include <dcc_ldc_multiRegion_imx390.h>
#include <dcc_ldc_lut_imx390.h>

#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif

TESTCASE(tivxHwaVpacLdc, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacLdcPosCov, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacLdcNegCov, CT_VXContext, ct_setup_vx_context, 0)

#define TEST_NUM_NODE_INSTANCE 2
#define TEST_NUM_MULTI_NODE_INSTANCE 8
#define SET_TIME_STAMP_VALUE  (100U)

/* #define TEST_VISS_PERFORMANCE_LOGGING */
/* #define TEST_LDC_CHECKSUM_LOGGING */
/* #define TEST_GET_ERROR_INTERRUPT_VALUES */
/* #define TEST_PRINT_TIME_STAMP_VALUES */
/* #define TEST_ENABLE_SAFETY_MECHANISIMS */
/* #define TEST_LDC_TRIGGER_WATCHDOG_TIMER */

#define LDC_NODE_ERROR_EVENT  (1U)
#define LDC_NODE_COMPLETED_EVENT (2U)
#define LDC_GRAPH_COMPLETED_EVENT (3U)

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string, *target_string_2;
} SetTarget_Arg;

#if (VPAC_COUNT > 1)
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC2_LDC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC_LDC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, TIVX_TARGET_VPAC2_LDC1))
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC_LDC1))
#endif

#if (VPAC_COUNT > 1)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, NULL))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL))
#endif

#define ADD_SAFETY_MECHANISM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=reconfigReinitMMR", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=statusRegValidate", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=configRegReadback", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=allMechanisms", __VA_ARGS__, 4))

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacLdc, testNodeCreation, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_INT16;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);

        {
            vx_enum ch_matrix_type;
            vx_size ch_matrix_rows, ch_matrix_cols, ch_data_size;

            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_TYPE, &ch_matrix_type, sizeof(ch_matrix_type)));
            if (matrix_type != ch_matrix_type)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_TYPE failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ROWS, &ch_matrix_rows, sizeof(ch_matrix_rows)));
            if (matrix_rows != ch_matrix_rows)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_ROWS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &ch_matrix_cols, sizeof(ch_matrix_cols)));
            if (matrix_cols != ch_matrix_cols)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_COLUMNS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_SIZE, &ch_data_size, sizeof(ch_data_size)));
            if (matrix_data_size > ch_data_size)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_SIZE failed\n");
            }
        }

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                NULL,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static vx_status save_image_from_ldc(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s_y8.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingDcc, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[0] == checksum_actual);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[1] == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}


enum CT_AffineMatrixType {
    VX_MATRIX_IDENT = 0,
    VX_MATRIX_ROTATE_90,
    VX_MATRIX_SCALE,
    VX_MATRIX_SCALE_ROTATE,
    VX_MATRIX_RANDOM,
    VX_MATRIX_MIRROR
};

#define VX_NN_AREA_SIZE         1.5
#define VX_BILINEAR_TOLERANCE   1

static CT_Image warp_affine_read_image_8u(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);

    return image;
}

static CT_Image warp_affine_generate_random(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
    if(NULL == image)
    {
        return NULL;
    }
    return image;
}

static vx_float32 ldc_clip(vx_float32 value, vx_float32 clip_value)
{
    vx_float32 output = value;

    if(value > clip_value)
    {
        output = clip_value;
    }
    else if(value < -clip_value)
    {
        output = -clip_value;
    }
    return output;
}

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);
static void warp_affine_generate_matrix(vx_float32* m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][2];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_MIRROR == type)
    {
        mat[0][0] = -1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = ldc_clip(src_width / (vx_float32)dst_width, 7.9999f);
        scale_y = ldc_clip(src_height / (vx_float32)dst_height, 7.9999f);

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_affine_create_matrix(vx_context context, vx_float32 *m, int32_t natc)
{
    vx_matrix matrix;
    if(natc)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else
    {
        matrix = vxCreateMatrix(context, VX_TYPE_INT16, 2, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[6];

            mfixed[0] = (vx_int16)(ldc_clip(m[0]*(4096.0f),32767));
            mfixed[1] = (vx_int16)(ldc_clip(m[1]*(4096.0f),32767));
            mfixed[2] = (vx_int16)(ldc_clip(m[2]*(4096.0f),32767));
            mfixed[3] = (vx_int16)(ldc_clip(m[3]*(4096.0f),32767));
            mfixed[4] = (vx_int16)(ldc_clip(m[4]*(8.0f),32767));
            mfixed[5] = (vx_int16)(ldc_clip(m[5]*(8.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
}

static vx_matrix warp_affine_create_matrix_cc_test(vx_context context, vx_float32 *m, int32_t natc)
{
    vx_matrix matrix;
    if(natc == 1)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 4);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else if(natc == 0)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_INT16, 2, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[6];

            mfixed[0] = (vx_int16)(ldc_clip(m[0]*(4096.0f),32767));
            mfixed[1] = (vx_int16)(ldc_clip(m[1]*(4096.0f),32767));
            mfixed[2] = (vx_int16)(ldc_clip(m[2]*(4096.0f),32767));
            mfixed[3] = (vx_int16)(ldc_clip(m[3]*(4096.0f),32767));
            mfixed[4] = (vx_int16)(ldc_clip(m[4]*(8.0f),32767));
            mfixed[5] = (vx_int16)(ldc_clip(m[5]*(8.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else
    {
        matrix = vxCreateMatrix(context, VX_TYPE_UINT8, 1, 4);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[6];

            mfixed[0] = (vx_uint8)(ldc_clip(m[0]*(4.0f),32767));
            mfixed[1] = (vx_uint8)(ldc_clip(m[1]*(4.0f),32767));
            mfixed[2] = (vx_uint8)(ldc_clip(m[2]*(4.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
}

static int warp_affine_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[2 * 0 + 0] * (vx_float64)x + (vx_float64)m[2 * 1 + 0] * (vx_float64)y + (vx_float64)m[2 * 2 + 0];
    y0 = (vx_float64)m[2 * 0 + 1] * (vx_float64)x + (vx_float64)m[2 * 1 + 1] * (vx_float64)y + (vx_float64)m[2 * 2 + 1];

    if (VX_INTERPOLATION_NEAREST_NEIGHBOR == interp_type)
    {
        for (yi = (vx_int32)ceil(y0 - VX_NN_AREA_SIZE); (vx_float64)yi <= y0 + VX_NN_AREA_SIZE; yi++)
        {
            for (xi = (vx_int32)ceil(x0 - VX_NN_AREA_SIZE); (vx_float64)xi <= x0 + VX_NN_AREA_SIZE; xi++)
            {
                if (0 < xi && 0 < yi && xi < (vx_int32)input->width-1 && yi < (vx_int32)input->height-1)
                {
                    candidate = *CT_IMAGE_DATA_PTR_8U(input, xi, yi);
                }
                else if (VX_BORDER_CONSTANT == border.mode)
                {
                    candidate = border.constant_value.U8;
                }
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    else if (VX_INTERPOLATION_BILINEAR == interp_type)
    {
        xlower = floor(x0);
        ylower = floor(y0);

        s = x0 - xlower;
        t = y0 - ylower;

        xi = (vx_int32)xlower;
        yi = (vx_int32)ylower;

        candidate = -1;
        if (VX_BORDER_UNDEFINED == border.mode)
        {
            if (1 <= xi && 1 <= yi && xi < (vx_int32)input->width - 2 && yi < (vx_int32)input->height - 2)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        else if (VX_BORDER_REPLICATE == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi    ) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi    ) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi + 1) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi + 1));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_affine_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_affine_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_affine_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == 0) ||
            (interp_type == 1) ||
            (interp_type == VX_MATRIX_MIRROR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) ||
            (border.mode == VX_BORDER_REPLICATE));

    warp_affine_validate(input, output, VX_INTERPOLATION_BILINEAR, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n",
                m[0], m[2], m[4],
                m[1], m[3], m[5]);
    }
}

static vx_image mesh_create(vx_context context, int width, int height, int m)
{
    CT_Image image;
    int factor = 1<<m;
    int table_width = ((width+(factor-1))>>m) + 1;
    int table_height = ((height+(factor-1))>>m) + 1;
    vx_image mesh = 0;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_image(table_width, table_height, VX_DF_IMAGE_U32));
    /* Simple mesh where every output pixel maps to the input which is:
     *   X:  8 pixels to the right (8 << 3 = 64)
     *   Y:  16 pixels down (16 << 3 = 128)
     */
    CT_FILL_IMAGE_32U(return 0, image,
        {
            *dst_data = ((64 << 16) | 128);
        });

    ASSERT_VX_OBJECT_(return 0,
            mesh = ct_image_to_vx_image(image, context), VX_TYPE_IMAGE);

    return mesh;
}

static vx_user_data_object lut_create(
    vx_context context, void* data, vx_size count)
{
    tivx_vpac_ldc_bit_depth_conv_lut_params_t remap_lut;
    vx_user_data_object lut_obj;
    vx_size size = sizeof(vx_uint16);
    void* ptr = NULL;

    lut_obj = vxCreateUserDataObject(context,
        "tivx_vpac_ldc_bit_depth_conv_lut_params_t",
        sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), NULL);

    ASSERT_VX_OBJECT_(return 0, lut_obj,
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    remap_lut.input_bits = 12;
    remap_lut.output_bits = 8;

    memcpy(remap_lut.lut, data, size *
        TIVX_VPAC_LDC_BIT_DEPTH_CONV_LUT_SIZE);

    if (VX_SUCCESS != vxCopyUserDataObject(lut_obj, 0,
            sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), &remap_lut,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
    {
        VX_CALL_(return 0, vxReleaseUserDataObject(&lut_obj));
    }

    return lut_obj;
}

static vx_user_data_object lut_create_new(
    vx_context context, void* data, vx_size count)
{
    tivx_vpac_ldc_bit_depth_conv_lut_params_t remap_lut;
    vx_user_data_object lut_obj;
    vx_size size = sizeof(vx_uint16);
    void* ptr = NULL;

    lut_obj = vxCreateUserDataObject(context,
        "tivx_vpac_ldc_bit_depth_conv_lut_params_t",
        sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), NULL);

    ASSERT_VX_OBJECT_(return 0, lut_obj,
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    remap_lut.input_bits = 13;
    remap_lut.output_bits = 7;

    memcpy(remap_lut.lut, data, size *
        TIVX_VPAC_LDC_BIT_DEPTH_CONV_LUT_SIZE);

    if (VX_SUCCESS != vxCopyUserDataObject(lut_obj, 0,
            sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), &remap_lut,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
    {
        VX_CALL_(return 0, vxReleaseUserDataObject(&lut_obj));
    }

    return lut_obj;
}

static void lut_data_fill_identity(void* data, vx_size count)
{
    int i;

    vx_uint16* data16 = (vx_uint16*)data;
    for (i = 0; i < count; ++i)
        data16[i] = i;
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int src_width, src_height;
    int out_width, out_height;
    vx_border_t border;
    vx_enum interp_type;
    int matrix_type;
    int input_mode;
    int output_mode;
    int mesh_mode;
    char *target_string, *target_string_2;
} Arg;

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int src_width, src_height;
    int out_width, out_height;
    vx_border_t border;
    vx_enum interp_type;
    int matrix_type;
    int input_mode;
    int output_mode;
    int mesh_mode;
    int enable_safety_mechanism;
    char *target_string, *target_string_2;
} ArgSafety;

typedef struct {
    const char* testName;
    int input_data_format;
    int output_data_format;
    char *target_string, *target_string_2;
} ArgFormats;

typedef struct {
    const char* testName;
    int negative_test;
    int condition;
    char *target_string, *target_string_2;
} ArgNegative;

typedef struct {
    const char* testName;
    const char* target_string;
    const char* target_string_2;
    int lut_enabled;
} ArgDccLut;

typedef struct {
    const char* testName;
    const char* target_string;
    const char* target_string_2;
    int lut_enabled;
    int enable_safety_mechanism;
} ArgDccLutSafety;

#define ADD_VX_BORDERS_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} }))

#define ADD_VX_INTERPOLATION_TYPE_BILINEAR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/INTERPOLATION_BILINEAR", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/INTERPOLATION_BICUBIC", __VA_ARGS__, 0))

#define ADD_VX_MATRIX_PARAM_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_IDENT", __VA_ARGS__,        VX_MATRIX_IDENT)), \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_ROTATE_90", __VA_ARGS__,    VX_MATRIX_ROTATE_90)), \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_SCALE", __VA_ARGS__,        VX_MATRIX_SCALE))

#define ADD_VX_INPUT_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LUMA_ONLY", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/LUMA_CHROMA_420", __VA_ARGS__, 1))

#define ADD_VX_INPUT_MODES_LENA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LUMA_ONLY", __VA_ARGS__, 0))

#define ADD_VX_OUTPUT_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/DUAL_OUT_OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/DUAL_OUT_ON", __VA_ARGS__, 1))

#define ADD_VX_MESH_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MESH_OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/MESH_M0", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MESH_M1", __VA_ARGS__, 2))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_SMALL_SET, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERPOLATION_TYPE_BILINEAR, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ADD_VX_INPUT_MODES_LENA, ADD_VX_OUTPUT_MODES, ADD_VX_MESH_MODES, ADD_SET_TARGET_PARAMETERS, ARG, warp_affine_read_image_8u, "lena.bmp", 0, 0)


#define PARAMETERS_MULTI_INST \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_SMALL_SET, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERPOLATION_TYPE_BILINEAR, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ADD_VX_INPUT_MODES_LENA, ADD_VX_OUTPUT_MODES, ADD_VX_MESH_MODES, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG, warp_affine_read_image_8u, "lena.bmp", 0, 0)

#define PARAMETERS_SAFETY \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_SMALL_SET, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERPOLATION_TYPE_BILINEAR, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ADD_VX_INPUT_MODES_LENA, ADD_VX_OUTPUT_MODES, ADD_VX_MESH_MODES, ADD_SAFETY_MECHANISM, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG, warp_affine_read_image_8u, "lena.bmp", 0, 0)

#define ADD_LUT_ENABLED(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/lut_enabled=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/lut_enabled=on", __VA_ARGS__, 1))

#define SET_NODE_TARGET_PARAMETERS_LUT \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ADD_LUT_ENABLED, ARG)

#define SET_NODE_TARGET_PARAMETERS_LUT_SAFETY \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ADD_LUT_ENABLED, ADD_SAFETY_MECHANISM, ARG)

#define ADD_VX_IN_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_NV12", __VA_ARGS__, VX_DF_IMAGE_NV12)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_NV12_P12", __VA_ARGS__, TIVX_DF_IMAGE_NV12_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_UYVY", __VA_ARGS__, VX_DF_IMAGE_UYVY)), \

#define ADD_VX_OUT_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_NV12", __VA_ARGS__, VX_DF_IMAGE_NV12)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_NV12_P12", __VA_ARGS__, TIVX_DF_IMAGE_NV12_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_UYVY", __VA_ARGS__, VX_DF_IMAGE_UYVY)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_YUYV", __VA_ARGS__, VX_DF_IMAGE_YUYV))

#define PARAMETERS_FORMATS \
    CT_GENERATE_PARAMETERS("testFormats", ADD_VX_IN_FORMATS, ADD_VX_OUT_FORMATS, ADD_SET_TARGET_PARAMETERS, ARG)


#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=input_align_12bit", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=luma_interpolation_type", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=init_x", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=init_y", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=yc_mode", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=enable", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=out_block_width", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=out_block_height", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=pixel_pad", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=subsample_factor", __VA_ARGS__, 9))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_SET_TARGET_PARAMETERS, ARG)

static uint32_t ldc_checksums_ref[3*2*3*2*3] = {
    0x362f22f2, 0xd6dcdec7, 0xd6dcdec7, 0x362f22f2,
    0xd6dcdec7, 0xd6dcdec7, 0x463605cc, 0xdbcbcbf0,
    0xdbcbcbf0, 0x463605cc, 0xdbcbcbf0, 0xdbcbcbf0,
    0x72ae30cb, 0xcfcfdcd7, 0xcfcfdcd7, 0x72ae30cb,
    0xcfcfdcd7, 0xcfcfdcd7, 0x2b2516c4, 0xd6dcdec7,
    0xd6dcdec7, 0x2b2516c4, 0xd6dcdec7, 0xd6dcdec7,
    0x4639062e, 0xdbcbcbf0, 0xdbcbcbf0, 0x4639062e,
    0xdbcbcbf0, 0xdbcbcbf0, 0x74ab2ebd, 0xcfcfdcd7,
    0xcfcfdcd7, 0x74ab2ebd, 0xcfcfdcd7, 0xcfcfdcd7,
    0xc2876133, 0xb9237446, 0xb9237446, 0xc2876133,
    0xb9237446, 0xb9237446, 0xc85d43e7, 0xf6012400,
    0xf6012400, 0xc85d43e7, 0xf6012400, 0xf6012400,
    0x26210e60, 0x5459597a, 0x5459597a, 0x26210e60,
    0x5459597a, 0x5459597a, 0xaa634fd0, 0xb9237446,
    0xb9237446, 0xaa634fd0, 0xb9237446, 0xb9237446,
    0xc85d46d0, 0xfafb2600, 0xfafb2600, 0xc85d46d0,
    0xfafb2600, 0xfafb2600, 0x24f65b78, 0x349b9bb2,
    0x349b9bb2, 0x24f65b78, 0x349b9bb2, 0x349b9bb2,
    0xde2e8367, 0x45626142, 0x45626142, 0xde2e8367,
    0x45626142, 0x45626142, 0x4b8f8729, 0xe68094ac,
    0xe68094ac, 0x4b8f8729, 0xe68094ac, 0xe68094ac,
    0xde2e8367, 0x45626142, 0x45626142, 0xde2e8367,
    0x45626142, 0x45626142, 0xfd389db2, 0x7e2f3ed8,
    0x7e2f3ed8, 0xfd389db2, 0x7e2f3ed8, 0x7e2f3ed8,
    0x579899f6, 0xe26696a4, 0xe26696a4, 0x579899f6,
    0xe26696a4, 0xe26696a4, 0xfd389db2, 0x7e2f3ed8,
    0x7e2f3ed8, 0xfd389db2, 0x7e2f3ed8, 0x7e2f3ed8
};

static uint32_t get_checksum(int out_width, vx_enum interp, int matrix, int output, int mesh)
{
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
    uint16_t e;

    if (16 == out_width)
    {
        a = 0U;
    }
    else if (256 == out_width)
    {
        a = 1U;
    }
    else
    {
        a = 2U;
    }

    if (1 == interp)
    {
        b = 0U;
    }
    else
    {
        b = 1U;
    }

    if (VX_MATRIX_IDENT == matrix)
    {
        c = 0U;
    }
    else if (VX_MATRIX_ROTATE_90 == matrix)
    {
        c = 1U;
    }
    else
    {
        c = 2U;
    }

    d = (uint16_t) output;
    e = (uint16_t) mesh;

    return ldc_checksums_ref[(2*3*2*3*a)+(3*2*3*b)+(2*3*c)+(3*d)+e];
}

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessing, Arg,
    PARAMETERS_MULTI_INST
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_NODE_INSTANCE] = {0}, output_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image dual_out_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image mesh_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_user_data_object luma_lut[TEST_NUM_NODE_INSTANCE] = {NULL}, chroma_lut[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_matrix matrix[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference ref[TEST_NUM_NODE_INSTANCE][2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;
    uint32_t i;

    CT_Image input = NULL, output[TEST_NUM_NODE_INSTANCE] = {NULL}, dual_out = NULL;

    vx_border_t border = arg_->border;
    vx_user_data_object safety_mechanism_obj[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference safety_ref[TEST_NUM_NODE_INSTANCE][1] = { NULL };

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_NO_FAILURE(output[i] = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
                ASSERT_VX_OBJECT(matrix[i] = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
                ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_VX_OBJECT(input_image[i] = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(output_image[i] = ct_image_to_vx_image(output[i], context), VX_TYPE_IMAGE);
                }

                if(arg_->output_mode == 1)
                {
                    ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                    ASSERT_VX_OBJECT(dual_out_image[i] = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(luma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if (arg_->input_mode == 1)
                    {
                        ASSERT_VX_OBJECT(chroma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    }
                }

                tivx_vpac_ldc_params_init(&params);
                ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                                    sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                params.luma_interpolation_type = arg_->interp_type;

                VX_CALL(vxCopyUserDataObject(param_obj[i], 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                tivx_vpac_ldc_region_params_init(&region);
                ASSERT_VX_OBJECT(region_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                                     sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                region.out_block_width = 16;
                region.out_block_height = 16;
                region.pixel_pad = 0;

                ASSERT_VX_OBJECT(mesh_params_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                                    sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                tivx_vpac_ldc_mesh_params_init(&mesh_params);

                if(arg_->mesh_mode > 0)
                {
                    mesh_image[i] = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
                    mesh_params.mesh_frame_width = arg_->out_width;
                    mesh_params.mesh_frame_height = arg_->out_height;
                    mesh_params.subsample_factor = arg_->mesh_mode-1;
                }

                VX_CALL(vxCopyUserDataObject(region_obj[i], 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(vxCopyUserDataObject(mesh_params_obj[i], 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                                param_obj[i],
                                matrix[i],
                                region_obj[i],
                                mesh_params_obj[i],
                                mesh_image[i],
                                NULL,
                                input_image[i],
                                output_image[i],
                                dual_out_image[i]),
                                VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border, sizeof(border)));
            }
        }
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_NO_FAILURE(output[i] = ct_image_from_vx_image(output_image[i]));
                    checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                        arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
                    checksum_actual = tivx_utils_simple_image_checksum(output_image[i], 0, rect);
                    ASSERT(checksum_expected == checksum_actual);
                }
            }
        }

        /* Note: Only running if larger than a certain threshold so that
         * framework overhead doesn't become a factor */
        if (arg_->out_width > 256)
        {
            if ((NULL != arg_->target_string) &&
                (NULL != arg_->target_string_2) )
            {
                vx_perf_t perf_node[TEST_NUM_NODE_INSTANCE], perf_graph;

                for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
                {
                    vxQueryNode(node[i], VX_NODE_PERFORMANCE, &perf_node[i], sizeof(perf_node[i]));
                }
                vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

                if (strncmp(arg_->target_string, arg_->target_string_2, TIVX_TARGET_MAX_NAME) == 0)
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are same\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg >= (perf_node[0].avg + perf_node[1].avg));
                }
                else
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are different\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg < (perf_node[0].avg + perf_node[1].avg));
                }
            }
        }

        //save_image_from_ldc(output_image, "output/ldc_y8");

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseMatrix(&matrix[i]));
                VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&region_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&mesh_params_obj[i]));
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&output_image[i]));
                    VX_CALL(vxReleaseImage(&input_image[i]));
                }
            }
        }
        VX_CALL(vxReleaseGraph(&graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->output_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&dual_out_image[i]));
                    VX_CALL(vxReleaseUserDataObject(&luma_lut[i]));

                    if(arg_->input_mode == 1)
                    {
                        VX_CALL(vxReleaseUserDataObject(&chroma_lut[i]));
                    }
                }
                if(arg_->mesh_mode > 0)
                {
                    VX_CALL(vxReleaseImage(&mesh_image[i]));
                }
            }
        }

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(matrix[i] == 0);
                ASSERT(output_image[i] == 0);
                ASSERT(input_image[i] == 0);
                ASSERT(param_obj[i] == 0);
                ASSERT(region_obj[i] == 0);
                ASSERT(mesh_params_obj[i] == 0);
                ASSERT(dual_out_image[i] == 0);
                ASSERT(luma_lut[i] == 0);
                ASSERT(chroma_lut[i] == 0);
                ASSERT(mesh_image[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
                   (arg_->matrix_type != VX_MATRIX_SCALE))
                {
                    ASSERT_NO_FAILURE(warp_affine_check(input, output[i], arg_->interp_type, arg_->border, m));
                }
            }
        }
    }
}

#if defined(VPAC3)
TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingSafety, ArgSafety,
    PARAMETERS_SAFETY
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_NODE_INSTANCE] = {0}, output_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image dual_out_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image mesh_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_user_data_object luma_lut[TEST_NUM_NODE_INSTANCE] = {NULL}, chroma_lut[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_matrix matrix[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference ref[TEST_NUM_NODE_INSTANCE][2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;
    uint32_t i;

    CT_Image input = NULL, output[TEST_NUM_NODE_INSTANCE] = {NULL}, dual_out = NULL;

    vx_border_t border = arg_->border;

    tivx_vpac_ldc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference safety_ref[TEST_NUM_NODE_INSTANCE][1] = { NULL };

    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference bw_ref[TEST_NUM_NODE_INSTANCE][1];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
            ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_NO_FAILURE(output[i] = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
                ASSERT_VX_OBJECT(matrix[i] = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
                ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));
                
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_VX_OBJECT(input_image[i] = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(output_image[i] = ct_image_to_vx_image(output[i], context), VX_TYPE_IMAGE);
                }
                
                if(arg_->output_mode == 1)
                {
                    ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                    ASSERT_VX_OBJECT(dual_out_image[i] = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(luma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    
                    if (arg_->input_mode == 1)
                    {
                        ASSERT_VX_OBJECT(chroma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    }
                }

                tivx_vpac_ldc_params_init(&params);
                ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                                    sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                params.luma_interpolation_type = arg_->interp_type;

                VX_CALL(vxCopyUserDataObject(param_obj[i], 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                tivx_vpac_ldc_region_params_init(&region);
                ASSERT_VX_OBJECT(region_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                                     sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                tivx_vpac_ldc_safety_mech_prms_init(&safety_mechanism_params);
                
                region.out_block_width = 16;
                region.out_block_height = 16;
                region.pixel_pad = 0;

                ASSERT_VX_OBJECT(mesh_params_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                                    sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                tivx_vpac_ldc_mesh_params_init(&mesh_params);

                if(arg_->mesh_mode > 0)
                {
                    mesh_image[i] = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
                    mesh_params.mesh_frame_width = arg_->out_width;
                    mesh_params.mesh_frame_height = arg_->out_height;
                    mesh_params.subsample_factor = arg_->mesh_mode-1;
                }

                VX_CALL(vxCopyUserDataObject(region_obj[i], 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(vxCopyUserDataObject(mesh_params_obj[i], 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                                param_obj[i],
                                matrix[i],
                                region_obj[i],
                                mesh_params_obj[i],
                                mesh_image[i],
                                NULL,
                                input_image[i],
                                output_image[i],
                                dual_out_image[i]),
                                VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border, sizeof(border)));
            }
        }

        VX_CALL(vxVerifyGraph(graph));
        /* IMPORTANT: Enable safety mechanism FIRST before sending LUT commands
         * This ensures that golden memory buffers are allocated before LUT configuration */
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(0U != arg_->enable_safety_mechanism)
                {
                    switch (arg_->enable_safety_mechanism)
                    {
                        case 1:
                            /* safety_mechanism=reconfigReinitMmr */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            break;
                        case 2:
                            /* safety_mechanism=statusRegValidate */
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            break;
                        case 3:
                            /* safety_mechanism=configRegReadback */
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        case 4:
                            /* safety_mechanism=allMechanisms */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        default:
                        // Invalid value
                        break;
                    }

                    ASSERT_VX_OBJECT(safety_mechanism_obj[i] = vxCreateUserDataObject(context,
                    "tivx_vpac_ldc_safety_mechanism_params_t",
                    sizeof(tivx_vpac_ldc_safety_mechanism_params_t),
                    &safety_mechanism_params),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    safety_ref[i][0] = (vx_reference)safety_mechanism_obj[i];

                    VX_CALL(tivxNodeSendCommand(node[i], i, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref[i], 1));
                }
            }
        }

        /* Send LUT command AFTER safety mechanism is enabled */
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->output_mode == 1)
                {
                    ref[i][0] = (vx_reference) luma_lut[i];
                    ref[i][1] = (vx_reference) chroma_lut[i];
                    if (arg_->input_mode == 1)
                    {
                        VX_CALL(tivxNodeSendCommand(node[i], 0,
                        TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS, ref[i], 2));
                    }
                    
                }
            }
        }

        /* Send bandwidth limiter command - ensures VBUSMR_CFG is properly initialized */
        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
        bandwidth_params.bandwidth_control = 4095u;
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(bandwidth_params_obj[i] = vxCreateUserDataObject(context,
                    "tivx_vpac_ldc_bandwidth_params_t",
                    sizeof(tivx_vpac_ldc_bandwidth_params_t),
                    &bandwidth_params),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                bw_ref[i][0] = (vx_reference)bandwidth_params_obj[i];
                VX_CALL(tivxNodeSendCommand(node[i], 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, bw_ref[i], 1));
            }
        }

        VX_CALL(vxProcessGraph(graph));

        /* Update safety mechanism params and send command again for periodic reconfiguration test */
        if(1U == arg_->enable_safety_mechanism)
        {
            for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
            {
                if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
                {
                    /* safety_mechanism=reconfigReinitMmr */
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    VX_CALL(vxCopyUserDataObject(safety_mechanism_obj[i], 0, sizeof(tivx_vpac_ldc_safety_mechanism_params_t), &safety_mechanism_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    safety_ref[i][0] = (vx_reference)safety_mechanism_obj[i];
                    VX_CALL(tivxNodeSendCommand(node[i], i, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref[i], 1));
                }
            }
            VX_CALL(vxProcessGraph(graph));
        }
        /* Release safety mechanism objects after second graph execution */
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(0U != arg_->enable_safety_mechanism)
                {
                    VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj[i]));
                }
            }
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_NO_FAILURE(output[i] = ct_image_from_vx_image(output_image[i]));
                    checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                        arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
                    checksum_actual = tivx_utils_simple_image_checksum(output_image[i], 0, rect);
                    ASSERT(checksum_expected == checksum_actual);
                }
            }
        }

        /* Note: Only running if larger than a certain threshold so that
         * framework overhead doesn't become a factor */
        if (arg_->out_width > 256)
        {
            if ((NULL != arg_->target_string) &&
                (NULL != arg_->target_string_2) )
            {
                vx_perf_t perf_node[TEST_NUM_NODE_INSTANCE], perf_graph;

                for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
                {
                    vxQueryNode(node[i], VX_NODE_PERFORMANCE, &perf_node[i], sizeof(perf_node[i]));
                }
                vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

                if (strncmp(arg_->target_string, arg_->target_string_2, TIVX_TARGET_MAX_NAME) == 0)
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are same\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg >= (perf_node[0].avg + perf_node[1].avg));
                }
                else
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are different\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg < (perf_node[0].avg + perf_node[1].avg));
                }
            }
        }

        //save_image_from_ldc(output_image, "output/ldc_y8");

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseMatrix(&matrix[i]));
                VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&region_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&mesh_params_obj[i]));
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&output_image[i]));
                    VX_CALL(vxReleaseImage(&input_image[i]));
                }
            }
        }
        VX_CALL(vxReleaseGraph(&graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->output_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&dual_out_image[i]));
                    VX_CALL(vxReleaseUserDataObject(&luma_lut[i]));

                    if(arg_->input_mode == 1)
                    {
                        VX_CALL(vxReleaseUserDataObject(&chroma_lut[i]));
                    }
                }
                if(arg_->mesh_mode > 0)
                {
                    VX_CALL(vxReleaseImage(&mesh_image[i]));
                }

                /* Release bandwidth limiter params object */
                if (NULL != bandwidth_params_obj[i])
                {
                    VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj[i]));
                }
            }
        }

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(matrix[i] == 0);
                ASSERT(output_image[i] == 0);
                ASSERT(input_image[i] == 0);
                ASSERT(param_obj[i] == 0);
                ASSERT(region_obj[i] == 0);
                ASSERT(mesh_params_obj[i] == 0);
                ASSERT(dual_out_image[i] == 0);
                ASSERT(luma_lut[i] == 0);
                ASSERT(chroma_lut[i] == 0);
                ASSERT(mesh_image[i] == 0);
                ASSERT(safety_mechanism_obj[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
                   (arg_->matrix_type != VX_MATRIX_SCALE))
                {
                    ASSERT_NO_FAILURE(warp_affine_check(input, output[i], arg_->interp_type, arg_->border, m));
                }
            }
        }
    }
}
#endif

#ifndef x86_64
TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingDccWithLut, ArgDccLut, SET_NODE_TARGET_PARAMETERS_LUT)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_image output_lut = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_actual;
    /* Expected checksums: [0]=1st output Luma, [1]=1st output Chroma,
     *                     [2]=2nd output (LUT) Luma, [3]=2nd output (LUT) Chroma */
    uint32_t checksum_expected[4] = {0x5c06583a, 0x3bb3bfed, 0x11617b52, 0x77d7e1ac};
    size_t sz;
    vx_rectangle_t rect;
    char output_file[MAXPATHLENGTH];

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/in0.raw";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12P12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC based on lut_enabled flag */
        if(1 == arg_->lut_enabled)
        {
            ASSERT_VX_OBJECT(output_lut = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            /* Use DCC with LUT parameters enabled */
            dcc_buff_size = DCC_LDC_LUT_IMX390_CFG_NUM_ELEM;

            ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxMapUserDataObject(
                dcc_param_ldc,
                0,
                dcc_buff_size,
                &dcc_ldc_buf_map_id,
                (void **)&dcc_ldc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            ));
            memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

            static uint8_t dcc_ldc_imx390_with_lut[DCC_LDC_LUT_IMX390_CFG_NUM_ELEM] = DCC_LDC_LUT_IMX390_CFG;
            memcpy(dcc_ldc_buf, dcc_ldc_imx390_with_lut, dcc_buff_size);

            VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        }
        else
        {
            /* Use normal DCC without LUT */
            dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

            ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxMapUserDataObject(
                dcc_param_ldc,
                0,
                dcc_buff_size,
                &dcc_ldc_buf_map_id,
                (void **)&dcc_ldc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            ));
            memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

            dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
            ASSERT(dcc_status == 0);

            VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        }
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                output_lut), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        #if defined(TEST_LDC_CHECKSUM_LOGGING)
        /* Save outputs based on lut_enabled flag */
        if(1 == arg_->lut_enabled)
        {
            /* When LUT is enabled, save both outputs */

            /* Save 1st output (NV12_P12 format - without LUT) */
            sz = snprintf(output_file, MAXPATHLENGTH, "%s/output/ldc_12to12_no_lut_enabled.yuv",
                         ct_get_test_file_path());
            ASSERT_(return, (sz < MAXPATHLENGTH));
            write_output_image_nv12_p12(output_file, output);
            printf("Saved 1st output (NV12_P12 - no LUT) to: %s\n", output_file);

            /* Save 2nd output (NV12 8-bit format - with LUT) */
            sz = snprintf(output_file, MAXPATHLENGTH, "%s/output/ldc_12to8_lut_enabled.yuv",
                         ct_get_test_file_path());
            ASSERT_(return, (sz < MAXPATHLENGTH));
            write_output_image_nv12_8bit(output_file, output_lut);
            printf("Saved 2nd output (NV12 8-bit - with LUT) to: %s\n", output_file);
        }
        else
        {
            /* When LUT is disabled, save only the primary output */
            sz = snprintf(output_file, MAXPATHLENGTH, "%s/output/ldc_12to12_lut_disabled.yuv",
                         ct_get_test_file_path());
            ASSERT_(return, (sz < MAXPATHLENGTH));
            write_output_image_nv12_p12(output_file, output);
            printf("Saved output (NV12_P12) to: %s\n", output_file);
        }
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        /* Verify checksums */
        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        #if defined(TEST_LDC_CHECKSUM_LOGGING)
        printf("LUT %s - 1st output (no LUT) Luma checksum: 0x%08x\n",
               arg_->lut_enabled ? "ENABLED" : "DISABLED", checksum_actual);
        #endif
        ASSERT(checksum_expected[0] == checksum_actual);

        if(1 == arg_->lut_enabled)
        {
            checksum_actual = tivx_utils_simple_image_checksum(output_lut, 0, rect);
            #if defined(TEST_LDC_CHECKSUM_LOGGING)
            printf("LUT %s - 2nd output (with LUT) Luma checksum: 0x%08x\n",
               arg_->lut_enabled ? "ENABLED" : "DISABLED", checksum_actual);
            #endif
            ASSERT(checksum_expected[2] == checksum_actual);
        }

        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        #if defined(TEST_LDC_CHECKSUM_LOGGING)
        printf("LUT %s - 1st output (no LUT) Chroma checksum: 0x%08x\n",
               arg_->lut_enabled ? "ENABLED" : "DISABLED", checksum_actual);
        #endif
        ASSERT(checksum_expected[1] == checksum_actual);

        if(1 == arg_->lut_enabled)
        {
            checksum_actual = tivx_utils_simple_image_checksum(output_lut, 1, rect);
            #if defined(TEST_LDC_CHECKSUM_LOGGING)
            printf("LUT %s - 2nd output (with LUT) Chroma checksum: 0x%08x\n",
                arg_->lut_enabled ? "ENABLED" : "DISABLED", checksum_actual);
            #endif
            ASSERT(checksum_expected[3] == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        if(1 == arg_->lut_enabled)
        {
            VX_CALL(vxReleaseImage(&output_lut));
        }
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(output_lut == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#if defined(VPAC3)
TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingDccWithLutSafety, ArgDccLutSafety, SET_NODE_TARGET_PARAMETERS_LUT_SAFETY)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0, output_lut = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_actual;
    /* Expected checksums: [0]=1st output Luma, [1]=1st output Chroma,
     *                     [2]=2nd output (LUT) Luma, [3]=2nd output (LUT) Chroma */
    uint32_t checksum_expected[4] = {0x5c06583a, 0x3bb3bfed, 0x11617b52, 0x77d7e1ac};
    size_t sz;
    vx_rectangle_t rect;

    /* Safety mechanism variables */
    tivx_vpac_ldc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };

    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj = NULL;
    vx_reference bw_ref[1];

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/in0.raw";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12P12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create DCC based on lut_enabled flag */
        if(1 == arg_->lut_enabled)
        {
            ASSERT_VX_OBJECT(output_lut = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            /* Use DCC with LUT parameters enabled */
            dcc_buff_size = DCC_LDC_LUT_IMX390_CFG_NUM_ELEM;

            ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxMapUserDataObject(
                dcc_param_ldc,
                0,
                dcc_buff_size,
                &dcc_ldc_buf_map_id,
                (void **)&dcc_ldc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            ));
            memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

            static uint8_t dcc_ldc_imx390_with_lut[DCC_LDC_LUT_IMX390_CFG_NUM_ELEM] = DCC_LDC_LUT_IMX390_CFG;
            memcpy(dcc_ldc_buf, dcc_ldc_imx390_with_lut, dcc_buff_size);

            VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        }
        else
        {
            /* Use normal DCC without LUT */
            dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

            ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxMapUserDataObject(
                dcc_param_ldc,
                0,
                dcc_buff_size,
                &dcc_ldc_buf_map_id,
                (void **)&dcc_ldc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            ));
            memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

            dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
            ASSERT(dcc_status == 0);

            VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        }
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                output_lut), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));

        /* IMPORTANT: Enable safety mechanism FIRST before sending other commands */
        if(0U != arg_->enable_safety_mechanism)
        {
            tivx_vpac_ldc_safety_mech_prms_init(&safety_mechanism_params);
            switch (arg_->enable_safety_mechanism)
            {
                case 1:
                    /* safety_mechanism=reconfigReinitMMR */
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    break;
                case 2:
                    /* safety_mechanism=statusRegValidate */
                    safety_mechanism_params.enable_status_reg_validation = 1U;
                    break;
                case 3:
                    /* safety_mechanism=configRegReadback */
                    safety_mechanism_params.enable_readback_config_registers = 1U;
                    break;
                case 4:
                    /* safety_mechanism=allMechanisms */
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    safety_mechanism_params.enable_status_reg_validation = 1U;
                    safety_mechanism_params.enable_readback_config_registers = 1U;
                    break;
                default:
                // Invalid value
                break;
            }

            ASSERT_VX_OBJECT(safety_mechanism_obj = vxCreateUserDataObject(context,
            "tivx_vpac_ldc_safety_mechanism_params_t",
            sizeof(tivx_vpac_ldc_safety_mechanism_params_t),
            &safety_mechanism_params),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            safety_ref[0] = (vx_reference)safety_mechanism_obj;
            VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref, 1));
        }

        /* Send bandwidth limiter command - ensures VBUSMR_CFG is properly initialized */
        if(0U != arg_->enable_safety_mechanism)
        {
            tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
            bandwidth_params.bandwidth_control = 4095u;

            ASSERT_VX_OBJECT(bandwidth_params_obj = vxCreateUserDataObject(context,
                "tivx_vpac_ldc_bandwidth_params_t",
                sizeof(tivx_vpac_ldc_bandwidth_params_t),
                &bandwidth_params),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            bw_ref[0] = (vx_reference)bandwidth_params_obj;
            VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, bw_ref, 1));
                    
        }

        VX_CALL(vxProcessGraph(graph));

        /* Periodic reconfiguration test - execute graph again with safety mechanism */
        if(1U == arg_->enable_safety_mechanism)
        {
            VX_CALL(vxCopyUserDataObject(safety_mechanism_obj, 0, sizeof(tivx_vpac_ldc_safety_mechanism_params_t), &safety_mechanism_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            safety_ref[0] = (vx_reference)safety_mechanism_obj;
            VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref, 1));
            VX_CALL(vxProcessGraph(graph));
        }

        /* Verify checksums */
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        ASSERT(checksum_expected[0] == checksum_actual);

        rect.end_y = 540;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        ASSERT(checksum_expected[1] == checksum_actual);
        rect.end_y = 1080;

        if(1 == arg_->lut_enabled)
        {
            checksum_actual = tivx_utils_simple_image_checksum(output_lut, 0, rect);
            ASSERT(checksum_expected[2] == checksum_actual);

            rect.end_y = 540;
            checksum_actual = tivx_utils_simple_image_checksum(output_lut, 1, rect);
            ASSERT(checksum_expected[3] == checksum_actual);
            rect.end_y = 1080;
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        if(1 == arg_->lut_enabled)
        {
            VX_CALL(vxReleaseImage(&output_lut));
        }
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        if(0U != arg_->enable_safety_mechanism)
        {
            VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));
            VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj));
        }

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(output_lut == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        if(0U != arg_->enable_safety_mechanism)
        {
            ASSERT(safety_mechanism_obj == 0);
            ASSERT(bandwidth_params_obj == 0);
        }

        tivxHwaUnLoadKernels(context);
    }
}
#endif
#endif

static void warp_perspective_generate_matrix(vx_float32 *m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][3];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;
        mat[0][2] = 0.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;
        mat[1][2] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_MIRROR == type)
    {
        mat[0][0] = -1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = ldc_clip(src_width / (vx_float32)dst_width, 7.9999f);
        scale_y = ldc_clip(src_height / (vx_float32)dst_height, 7.9999f);

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;
        mat[0][2] = 0.f;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[0][2] = RND_FLT(0.f, 0.1f);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);
        mat[1][2] = RND_FLT(0.f, 0.1f);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][2] = 1.f;
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_perspective_create_matrix(vx_context context, vx_float32 *m, int32_t natc)
{
    vx_matrix matrix;
    if(natc)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else
    {
        matrix = vxCreateMatrix(context, VX_TYPE_INT16, 3, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[9];

            mfixed[0] = (vx_int16)(ldc_clip((m[0]/m[8])*(4096.0f),32767));
            mfixed[1] = (vx_int16)(ldc_clip((m[1]/m[8])*(4096.0f),32767));
            mfixed[2] = (vx_int16)(ldc_clip((m[2]/m[8])*(8388608.0f),32767));
            mfixed[3] = (vx_int16)(ldc_clip((m[3]/m[8])*(4096.0f),32767));
            mfixed[4] = (vx_int16)(ldc_clip((m[4]/m[8])*(4096.0f),32767));
            mfixed[5] = (vx_int16)(ldc_clip((m[5]/m[8])*(8388608.0f),32767));
            mfixed[6] = (vx_int16)(ldc_clip((m[6]/m[8])*(8.0f),32767));
            mfixed[7] = (vx_int16)(ldc_clip((m[7]/m[8])*(8.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
}

static int warp_perspective_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, z0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[3 * 0 + 0] * (vx_float64)x + (vx_float64)m[3 * 1 + 0] * (vx_float64)y + (vx_float64)m[3 * 2 + 0];
    y0 = (vx_float64)m[3 * 0 + 1] * (vx_float64)x + (vx_float64)m[3 * 1 + 1] * (vx_float64)y + (vx_float64)m[3 * 2 + 1];
    z0 = (vx_float64)m[3 * 0 + 2] * (vx_float64)x + (vx_float64)m[3 * 1 + 2] * (vx_float64)y + (vx_float64)m[3 * 2 + 2];
    if (fabs(z0) < DBL_MIN)
        return 0;

    x0 = x0 / z0;
    y0 = y0 / z0;
    if (VX_INTERPOLATION_NEAREST_NEIGHBOR == interp_type)
    {
        for (yi = (vx_int32)ceil(y0 - VX_NN_AREA_SIZE); (vx_float64)yi <= y0 + VX_NN_AREA_SIZE; yi++)
        {
            for (xi = (vx_int32)ceil(x0 - VX_NN_AREA_SIZE); (vx_float64)xi <= x0 + VX_NN_AREA_SIZE; xi++)
            {
                if (0 <= xi && 0 <= yi && xi < (vx_int32)input->width-1 && yi < (vx_int32)input->height-1)
                {
                    candidate = *CT_IMAGE_DATA_PTR_8U(input, xi, yi);
                }
                else if (VX_BORDER_CONSTANT == border.mode)
                {
                    candidate = border.constant_value.U8;
                }
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    else if (VX_INTERPOLATION_BILINEAR == interp_type)
    {
        xlower = floor(x0);
        ylower = floor(y0);

        s = x0 - xlower;
        t = y0 - ylower;

        xi = (vx_int32)xlower;
        yi = (vx_int32)ylower;

        candidate = -1;
        if (VX_BORDER_UNDEFINED == border.mode)
        {
            if (xi >= 1 && yi >= 1 && xi < (vx_int32)input->width - 2 && yi < (vx_int32)input->height - 2)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        else if (VX_BORDER_REPLICATE == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi    ) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi    ) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi + 1) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi + 1));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_perspective_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_perspective_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_perspective_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == 0) ||
            (interp_type == 1) ||
            (interp_type == VX_MATRIX_MIRROR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) ||
            (border.mode == VX_BORDER_REPLICATE));

    warp_perspective_validate(input, output, VX_INTERPOLATION_BILINEAR, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n%g %g %g\n",
                m[0], m[3], m[6],
                m[1], m[4], m[7],
                m[2], m[5], m[8]);
    }
}


TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingPerspective, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_user_data_object luma_lut = NULL, chroma_lut = NULL;
    vx_matrix matrix = 0;
    vx_reference ref[2];
    vx_float32 m[9];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;

    CT_Image input = NULL, output = NULL, dual_out = NULL;

    vx_border_t border = arg_->border;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_perspective_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
        ASSERT_VX_OBJECT(matrix = warp_perspective_create_matrix(context, m, 1), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }

        if(arg_->output_mode == 1)
        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            if (arg_->input_mode == 1)
            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = arg_->interp_type;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        if(arg_->mesh_mode > 0)
        {
            mesh_image = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
            mesh_params.mesh_frame_width = arg_->out_width;
            mesh_params.mesh_frame_height = arg_->out_height;
            mesh_params.subsample_factor = arg_->mesh_mode-1;
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_NO_FAILURE(output = ct_image_from_vx_image(output_image));
            checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }

        if(arg_->output_mode == 1)
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            if(arg_->input_mode == 1)
            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }

        if(arg_->mesh_mode > 0)
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);

        if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
           (arg_->matrix_type != VX_MATRIX_SCALE))
        {
            ASSERT_NO_FAILURE(warp_perspective_check(input, output, arg_->interp_type, arg_->border, m));
        }
    }
}



typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
    char *target_string, *target_string_2;
} ArgCmd;

#define PARAMETERS_CMD \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, warp_affine_read_image_8u, "lena.bmp")

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingCommand, ArgCmd, PARAMETERS_CMD)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_matrix matrix = 0, matrix2 = 0;
    vx_reference ref[1];
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL, output2 = NULL, dual_out = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);

        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(output2 = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
        ASSERT_VX_OBJECT(matrix2 = warp_affine_create_matrix(context, m2, 0), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        //VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(output = ct_image_from_vx_image(output_image));
        ASSERT_NO_FAILURE(warp_affine_check(input, output, VX_MATRIX_IDENT, border, m));
        //ct_write_image("output/ident.bmp", output);

        ref[0] = (vx_reference)matrix2;
        VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_LDC_PARAMS, ref, 1));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(output2 = ct_image_from_vx_image(output_image));
        ASSERT_NO_FAILURE(warp_affine_check(input, output2, VX_MATRIX_MIRROR, border, m2));
        //ct_write_image("output/mirror.bmp", output2);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseMatrix(&matrix2));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(matrix2 == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);

    }
}


static uint8_t isFormatComboValid(int in, int out, int out_num)
{
    uint8_t valid = 1;

    if ( (in == VX_DF_IMAGE_U8) &&
         !((out == VX_DF_IMAGE_U8) || (out == TIVX_DF_IMAGE_P12)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_U16) &&
         !((out == VX_DF_IMAGE_U16) || ((out_num == 1) && (out == VX_DF_IMAGE_U8))))
    {
        valid = 0;
    }

    if ( (in == TIVX_DF_IMAGE_P12) &&
         !((out == TIVX_DF_IMAGE_P12) || (out == VX_DF_IMAGE_U8)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_NV12) &&
         !((out == VX_DF_IMAGE_NV12) || (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        valid = 0;
    }

    if ( (in == TIVX_DF_IMAGE_NV12_P12) &&
         !((out == TIVX_DF_IMAGE_NV12_P12) || (out == VX_DF_IMAGE_NV12)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_UYVY) &&
         !((out == VX_DF_IMAGE_UYVY) || (out == VX_DF_IMAGE_YUYV) ||
           (out == VX_DF_IMAGE_NV12) || (out == TIVX_DF_IMAGE_NV12_P12) ))
    {
        valid = 0;
    }

    return valid;
}

static uint32_t ldc_formats_checksums_ref[] = {
    0x2d2cdc00, 0xffffa600, 0xbfa9b800, 0x2d2cf000,
    0x49e48800, 0xe529b800, 0xffff8800
};

static uint32_t get_formats_checksum(int in, int out)
{
    uint32_t index = 0;

    if ( ((in == VX_DF_IMAGE_U8) && (out == VX_DF_IMAGE_U8)) ||
         ((in == VX_DF_IMAGE_NV12) && (out == VX_DF_IMAGE_NV12)) ||
         ((in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_NV12)))
    {
        index = 0;
    }
    else if ( ((in == VX_DF_IMAGE_U8) && (out == TIVX_DF_IMAGE_P12)) ||
              ((in == VX_DF_IMAGE_NV12) && (out == TIVX_DF_IMAGE_NV12_P12)) ||
              ((in == VX_DF_IMAGE_UYVY) && (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        index = 1U;
    }
    else if ( (in == VX_DF_IMAGE_U16) && (out == VX_DF_IMAGE_U16))
    {
        index = 2U;
    }
    else if ( ((in == TIVX_DF_IMAGE_P12) && (out == VX_DF_IMAGE_U8)) ||
              ((in == TIVX_DF_IMAGE_NV12_P12) && (out == VX_DF_IMAGE_NV12)))
    {
        index = 3U;
    }
    else if ( (in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_UYVY))
    {
        index = 4U;
    }
    else if ( (in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_YUYV))
    {
        index = 5U;
    }
    else if ( ((in == TIVX_DF_IMAGE_P12) && (out == TIVX_DF_IMAGE_P12)) ||
              ((in == TIVX_DF_IMAGE_NV12_P12) && (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        index = 6U;
    }
    return ldc_formats_checksums_ref[index];
}


TEST_WITH_ARG(tivxHwaVpacLdc, testFormats, ArgFormats,
    PARAMETERS_FORMATS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, arg_->input_data_format, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, arg_->output_data_format), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (0 == isFormatComboValid(arg_->input_data_format, arg_->output_data_format, 0))
        {
            EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
            checksum_expected = get_formats_checksum(arg_->input_data_format, arg_->output_data_format);
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

TEST_WITH_ARG(tivxHwaVpacLdc, testNegativeGraph, ArgNegative, PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    tivx_vpac_ldc_params_t params;
    tivx_vpac_ldc_region_params_t region_params;
    tivx_vpac_ldc_mesh_params_t mesh_params;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_user_data_object mesh_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_INT16;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);

        {
            vx_enum ch_matrix_type;
            vx_size ch_matrix_rows, ch_matrix_cols, ch_data_size;

            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_TYPE, &ch_matrix_type, sizeof(ch_matrix_type)));
            if (matrix_type != ch_matrix_type)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_TYPE failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ROWS, &ch_matrix_rows, sizeof(ch_matrix_rows)));
            if (matrix_rows != ch_matrix_rows)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_ROWS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &ch_matrix_cols, sizeof(ch_matrix_cols)));
            if (matrix_cols != ch_matrix_cols)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_COLUMNS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_SIZE, &ch_data_size, sizeof(ch_data_size)));
            if (matrix_data_size > ch_data_size)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_SIZE failed\n");
            }
        }

        tivx_vpac_ldc_params_init(&params);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);
        tivx_vpac_ldc_region_params_init(&region_params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(mesh_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.input_align_12bit = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.input_align_12bit = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.input_align_12bit = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.input_align_12bit = 2;
                }
                else
                {
                    params.input_align_12bit = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.luma_interpolation_type = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.luma_interpolation_type = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.luma_interpolation_type = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.luma_interpolation_type = 2;
                }
                else
                {
                    params.luma_interpolation_type = 2;
                }
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.init_x = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.init_x = 8184;
                }
                else if (2U == arg_->condition)
                {
                    params.init_x = 8192;
                }
                else if (3U == arg_->condition)
                {
                    params.init_x = 8192;
                }
                else
                {
                    params.init_x = 4004;
                }
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.init_y = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.init_y = 8184;
                }
                else if (2U == arg_->condition)
                {
                    params.init_y = 8192;
                }
                else if (3U == arg_->condition)
                {
                    params.init_y = 8192;
                }
                else
                {
                    params.init_y = 4001;
                }
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.yc_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.yc_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.yc_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.yc_mode = 2;
                }
                else
                {
                    params.yc_mode = 2;
                }
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    region_params.enable = 0;
                }
                else if (1U == arg_->condition)
                {
                    region_params.enable = 1;
                }
                else if (2U == arg_->condition)
                {
                    region_params.enable = 2;
                }
                else if (3U == arg_->condition)
                {
                    region_params.enable = 2;
                }
                else
                {
                    region_params.enable = 2;
                }
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    region_params.out_block_width = 8;
                }
                else if (1U == arg_->condition)
                {
                    region_params.out_block_width = 248;
                }
                else if (2U == arg_->condition)
                {
                    region_params.out_block_width = 0;
                }
                else if (3U == arg_->condition)
                {
                    region_params.out_block_width = 256;
                }
                else
                {
                    region_params.out_block_width = 124;
                }
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    region_params.out_block_height = 2;
                }
                else if (1U == arg_->condition)
                {
                    region_params.out_block_height = 254;
                }
                else if (2U == arg_->condition)
                {
                    region_params.out_block_height = 0;
                }
                else if (3U == arg_->condition)
                {
                    region_params.out_block_height = 256;
                }
                else
                {
                    region_params.out_block_height = 127;
                }
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    region_params.pixel_pad = 0;
                }
                else if (1U == arg_->condition)
                {
                    region_params.pixel_pad = 15;
                }
                else if (2U == arg_->condition)
                {
                    region_params.pixel_pad = 16;
                }
                else if (3U == arg_->condition)
                {
                    region_params.pixel_pad = 16;
                }
                else
                {
                    region_params.pixel_pad = 16;
                }
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    mesh_params.subsample_factor = 0;
                }
                else if (1U == arg_->condition)
                {
                    mesh_params.subsample_factor = 7;
                }
                else if (2U == arg_->condition)
                {
                    mesh_params.subsample_factor = 8;
                }
                else if (3U == arg_->condition)
                {
                    mesh_params.subsample_factor = 8;
                }
                else
                {
                    mesh_params.subsample_factor = 8;
                }
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                mesh_obj,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if(2 > arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacLdc, testNegativeMatrixType, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_UINT8;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                NULL,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVpacLdc, testNegativeStrideInMesh)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node;
    vx_image input_image, output_image;
    vx_image dual_out_image;
    vx_image mesh_image;
    vx_user_data_object luma_lut, chroma_lut;
    vx_matrix matrix;
    vx_reference ref[2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    uint32_t i;
    vx_uint32 set_stride_y_alignment = 8;

    CT_Image input, output, dual_out = NULL;

    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_LDC1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_NO_FAILURE(input = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(output = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, 16, 16, 16, 16, VX_TYPE_UINT8));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        {
            ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }

        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        {
            ASSERT_VX_OBJECT(mesh_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
            mesh_params.mesh_frame_width = 16;
            mesh_params.mesh_frame_height = 16;
            mesh_params.subsample_factor = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(mesh_image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }
        VX_CALL(vxReleaseGraph(&graph));
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(graph == 0);

        ASSERT(node == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVpacLdc, testNegativeStrideInImage)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node;
    vx_image input_image, output_image;
    vx_image dual_out_image;
    vx_image mesh_image;
    vx_user_data_object luma_lut, chroma_lut;
    vx_matrix matrix;
    vx_reference ref[2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    uint32_t i;
    vx_uint32 set_stride_y_alignment = 8;

    CT_Image output, dual_out = NULL;

    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_LDC1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_NO_FAILURE(output = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, 16, 16, 16, 16, VX_TYPE_UINT8));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        {
            ASSERT_VX_OBJECT(input_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(input_image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));

        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        {
            ASSERT_VX_OBJECT(mesh_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
            mesh_params.mesh_frame_width = 16;
            mesh_params.mesh_frame_height = 16;
            mesh_params.subsample_factor = 0;
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }
        VX_CALL(vxReleaseGraph(&graph));
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(graph == 0);

        ASSERT(node == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#if defined(VPAC3) || defined(VPAC3L)
static uint32_t ldc_diff_bit_depth[] = {
    0x2d2cdc00, 0x96966e00, 0xbfa9b800, 0xdfd4dc00,
    0xffff8800, 0xffffc400
};
typedef struct {
    const char* testName;
    int input0_data_format;
    int input1_data_format;
    int in_img0_yc_mode;
    int in_img1_yc_mode;
    char *target_string, *target_string_2;
} ArgFormats_diff_bit_depth;
#define ADD_IN_IMG0_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/in_img0_yc_mode=0", __VA_ARGS__, 0)) 

#define ADD_IN_IMG1_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/input1_data_format=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/input1_data_format=1", __VA_ARGS__, 1)) 

#define ADD_VX_IN0_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12))

#define ADD_VX_IN1_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12)) 

#define PARAMETERS_FORMATS_DIFF_BIT_DEPTH \
    CT_GENERATE_PARAMETERS("testFormats", ADD_VX_IN0_FORMATS, ADD_VX_IN1_FORMATS, ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacLdc, GraphProcessing_Diff_Depth, ArgFormats_diff_bit_depth,
    PARAMETERS_FORMATS_DIFF_BIT_DEPTH
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image src_image0 = 0, src_image1 = 0;
    int src_w0 = 640, src_h0 = 480, src_w1 = 640, src_h1 = 480;
    int dst_width[2], dst_height[2];
    vx_image dst_image[2] = {0};
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        for(int i=0; i<2; i++)
        {
            if(i % 2 == 0)
            {
                dst_width[i] = src_w0;
                dst_height[i] = src_h0;
            }
            else
            {
                dst_width[i] = src_w1;
                dst_height[i] = src_h1;
            }
        }
        if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
        {
            src_h1 = src_h1/2;
            dst_height[1] = dst_height[1]/2;        
        }
        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
        for(int i = 0; i < 2; i++)
        {
            if(i % 2 == 0)
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
            }
        }
        tivx_vpac_ldc_params_init(&params);
        params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        src_image0,
                        dst_image[0],
                        NULL,
                        src_image1,
                        dst_image[1],
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        for(int i=0; i<2; i++)
        {
            int dst_img_fmt;
            vx_char temp[256];
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];        
            if(i % 2 == 0)
            {
                dst_img_fmt = arg_->input0_data_format;
                checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            }
            else
            {
                dst_img_fmt = arg_->input1_data_format;
                checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            }
            #if defined(TEST_LDC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            if(dst_img_fmt == VX_DF_IMAGE_U8)
            {
                if(arg_->in_img0_yc_mode == 0)
                {
                    if(arg_->in_img1_yc_mode == 0)
                    {
                        ASSERT(ldc_diff_bit_depth[0] == checksum_actual);
                    }
                    else
                    {
                        if(i % 2 == 0)
                        {
                            ASSERT(ldc_diff_bit_depth[0] == checksum_actual);
                        }
                        else
                        {
                            ASSERT(ldc_diff_bit_depth[1] == checksum_actual);
                        }
                    }
                }
            }
            if(dst_img_fmt == VX_DF_IMAGE_U16)
            {
                if(arg_->in_img0_yc_mode == 0)
                {
                    if(arg_->in_img1_yc_mode == 0)
                    {
                        ASSERT(ldc_diff_bit_depth[2] == checksum_actual);
                    }
                    else
                    {
                        if(i % 2 == 0)
                        {
                            ASSERT(ldc_diff_bit_depth[2] == checksum_actual);
                        }
                        else
                        {
                            ASSERT(ldc_diff_bit_depth[3] == checksum_actual);
                        }
                    }
                }
            }
            if(dst_img_fmt == TIVX_DF_IMAGE_P12)
            {
                if(arg_->in_img0_yc_mode == 0)
                {
                    if(arg_->in_img1_yc_mode == 0)
                    {
                        ASSERT(ldc_diff_bit_depth[4] == checksum_actual);
                    }
                    else
                    {
                        if(i % 2 == 0)
                        {
                            ASSERT(ldc_diff_bit_depth[4] == checksum_actual);
                        }
                        else
                        {
                            ASSERT(ldc_diff_bit_depth[5] == checksum_actual);
                        }
                    }
                }
            }
        }
        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&src_image0));
        VX_CALL(vxReleaseImage(&src_image1));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        for (int i = 0; i < 2; i++)
        {
            VX_CALL(vxReleaseImage(&dst_image[i]));
            ASSERT(dst_image[i] == 0);
        }
        ASSERT(src_image0 == 0);
        ASSERT(src_image1 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}
#define SET_NODE_TARGET_PARAMETERS_RGB \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG, NULL)
TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessing_RGB_Plane_Split, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS_RGB
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node_rg = 0, node_b = 0;
    vx_image src_image_R = 0, src_image_G = 0, src_image_B = 0;
    vx_image dst_image_R = 0, dst_image_G = 0, dst_image_B = 0;
    int w = 1280, h = 720;
    int dst_width = w, dst_height = h;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;
    vx_char temp[256];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image_R = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image_G = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image_B = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ct_read_raw_image_to_vx_image(src_image_R, "psdkra/app_single_cam/1280x720_r8.raw", 1);
        ct_read_raw_image_to_vx_image(src_image_G, "psdkra/app_single_cam/1280x720_g8.raw", 1);
        ct_read_raw_image_to_vx_image(src_image_B, "psdkra/app_single_cam/1280x720_b8.raw", 1);
        
        ASSERT_VX_OBJECT(dst_image_R = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image_G = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image_B = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        tivx_vpac_ldc_params_init(&params);
        params.in_img0_yc_mode = 0;
        params.in_img1_yc_mode = 0;
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_rg = tivxVpacLdcNode2(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        src_image_R,
                        dst_image_R,
                        NULL,
                        src_image_G,
                        dst_image_G,
                        NULL),
                        VX_TYPE_NODE);

        ASSERT_VX_OBJECT(node_b = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        src_image_B,
                        dst_image_B,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_rg, VX_TARGET_STRING, arg_->target_string));
        if (NULL != arg_->target_string_2)
        {
            VX_CALL(vxSetNodeTarget(node_b, VX_TARGET_STRING, arg_->target_string_2));
        }
        else
        {
            VX_CALL(vxSetNodeTarget(node_b, VX_TARGET_STRING, arg_->target_string));
        }

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height; 

        checksum_actual = tivx_utils_simple_image_checksum(dst_image_R, 0, rect);
        #if defined TEST_LDC_CHECKSUM_LOGGING
        printf(" R plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_R_%d", 0);
        save_image_from_ldc(dst_image_R, temp);
        #endif
        ASSERT((uint32_t)0x7a26526b == checksum_actual);

        checksum_actual = tivx_utils_simple_image_checksum(dst_image_G, 0, rect);
        #if defined TEST_LDC_CHECKSUM_LOGGING
        printf(" G plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_G_%d", 0);
        save_image_from_ldc(dst_image_G, temp);
        #endif
        ASSERT((uint32_t)0x4e575912 == checksum_actual);     

        checksum_actual = tivx_utils_simple_image_checksum(dst_image_B, 0, rect);
        #if defined TEST_LDC_CHECKSUM_LOGGING
        printf(" B plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_B_%d", 0);
        save_image_from_ldc(dst_image_B, temp);
        #endif
        ASSERT((uint32_t)0x1f1d41c0 == checksum_actual); 


        VX_CALL(vxReleaseNode(&node_rg));
        VX_CALL(vxReleaseNode(&node_b));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        ASSERT(node_rg == 0);
        ASSERT(node_b == 0);
        ASSERT(graph == 0);
        
        VX_CALL(vxReleaseImage(&dst_image_R));
        VX_CALL(vxReleaseImage(&dst_image_G));
        VX_CALL(vxReleaseImage(&dst_image_B));

        VX_CALL(vxReleaseImage(&src_image_R));
        VX_CALL(vxReleaseImage(&src_image_G));
        VX_CALL(vxReleaseImage(&src_image_B));
        
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
        ASSERT(src_image_R == 0);
        ASSERT(src_image_G == 0);
        ASSERT(src_image_B == 0); 

    }
}
#endif
#if defined(TEST_LDC_TRIGGER_WATCHDOG_TIMER)
TEST(tivxHwaVpacLdc, testErrorInterrupts)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image0 = 0, output_image1 = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_reference param_ref = 0;

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    pixel.U32 = 0x0a7f1345;

    ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_UYVY, &pixel), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image0 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

    tivx_vpac_ldc_params_init(&params);
    ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                        sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                    param_obj,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    input_image,
                    output_image0,
                    output_image1),
                    VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1));

    VX_CALL(vxVerifyGraph(graph));

    /* Call the control command for maximum coverage */
    params.wdtimer_timeout_cycles = TIVX_VPAC_LDC_WDTIMER_128K_CYCLES;
    params.enable_error_events = TIVX_VPAC_LDC_WDTIMER_ERR;
    VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params,
        VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    param_ref = (vx_reference)param_obj;
    tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u);
       

    /* Run first frame with error interrupts enabled (enabled by default) */
    VX_CALL(vxProcessGraph(graph));

    /* Get error interrupt status values */
    ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
    VX_TYPE_UINT32 , &scalar_out_err_stat),
    (enum vx_type_e)VX_TYPE_SCALAR);

    err_refs[0] = (vx_reference)scalar_err_stat_obj;
    VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS, err_refs, 1u));
    VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST));
    #if defined TEST_GET_ERROR_INTERRUPT_VALUES
    printf("LDC error status value is %d\n", scalar_out_err_stat);
    #endif
    /* Checks whether error interrupt was detected */
    ASSERT(scalar_out_err_stat != 0);

    /* Run second frame with error interrupts disabled */
    params.enable_error_events = 0;
    VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params,
        VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    param_ref = (vx_reference)param_obj;
    tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u);
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS, err_refs, 1u));
    VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat,
                            VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    #if defined TEST_GET_ERROR_INTERRUPT_VALUES
    printf("LDC error status value is %d\n", scalar_out_err_stat);
    #endif
    /* Checks whether error interrupt was detected.
    *  No error interrupt should be detected as they have been disabled */
    ASSERT(scalar_out_err_stat == 0);
    VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxDisableEvents(context));
    VX_CALL(vxReleaseUserDataObject(&param_obj));
    VX_CALL(vxReleaseImage(&output_image0));
    VX_CALL(vxReleaseImage(&output_image1));
    VX_CALL(vxReleaseImage(&input_image));


    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(output_image0 == 0);
    ASSERT(output_image1 == 0);
    ASSERT(input_image == 0);
    ASSERT(param_obj == 0);

    tivxHwaUnLoadKernels(context);

}
#endif
/* This test is used to demonstrate the use of events to capture
*  error interrupt status. This tests triggers a watchdog timer error,
*  and therefore cannot be a part of the main test run */
TEST_WITH_ARG(tivxHwaVpacLdc, testErrorInterruptEvents, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params[TEST_NUM_MULTI_NODE_INSTANCE];
    vx_user_data_object param_obj[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0}, output_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0}, output_image2 = 0;
    vx_pixel_value_t pixel;
    vx_scalar scalar_err_stat_obj[TEST_NUM_MULTI_NODE_INSTANCE] = {NULL};
    vx_enum scalar_out_err_stat[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_reference err_refs[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_event_t event = { 0 };
    vx_reference param_ref[TEST_NUM_MULTI_NODE_INSTANCE] = { 0 };
    uint8_t node_completed = 0;
    uint8_t graph_completed = 0;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        /* enable events generation */
        VX_CALL(vxEnableEvents(context));

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            tivx_vpac_ldc_params_init(&params[i]);

            ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                    sizeof(tivx_vpac_ldc_params_t), &params[i]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            if(i == 3)
            {
                ASSERT_VX_OBJECT(input_image[i] = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_UYVY, &pixel), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image[i] = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                    param_obj[i],
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    input_image[i],
                    output_image[i],
                    output_image2),
                    VX_TYPE_NODE);
            }
            else
            {
                ASSERT_VX_OBJECT(input_image[i] = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image[i] = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                    param_obj[i],
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    input_image[i],
                    output_image[i],
                    NULL),
                    VX_TYPE_NODE);
            }

            /* Register an error event for the node */
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node[i], VX_EVENT_NODE_ERROR, 0, LDC_NODE_ERROR_EVENT));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_COMPLETED, 0, LDC_GRAPH_COMPLETED_EVENT));
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node[i], VX_EVENT_NODE_COMPLETED, 0, LDC_NODE_COMPLETED_EVENT));

            VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
        }

        VX_CALL(vxVerifyGraph(graph));

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            /* Enable all events, including wdtimer */
            params[i].enable_error_events = TIVX_VPAC_LDC_ALL_ERR_EVENTS;
            VX_CALL(vxCopyUserDataObject(param_obj[i], 0, sizeof(tivx_vpac_ldc_params_t), &params[i],
                VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            param_ref[i] = (vx_reference)param_obj[i];
            tivxNodeSendCommand(node[i], 0u, TIVX_VPAC_LDC_CMD_ENABLE_ERROR_EVENTS, &param_ref[i], 1u);
        }

        VX_CALL(vxScheduleGraph(graph));
        do
        {
            VX_CALL(vxWaitEvent(context, &event, vx_false_e));
            if(event.app_value == LDC_NODE_ERROR_EVENT)
            {
                #ifdef LDC_EVENT_INFO_LOGGING
                printf("LDC NODE ERROR EVENT received\n");
                printf("Event info = %d\n", event.event_info.node_error.status);
                #endif
                if(event.event_info.node_error.status == VX_ERROR_TIMEOUT)
                {
                    #ifdef LDC_EVENT_INFO_LOGGING
                    printf("Stalling Error\n");
                    /* Function call to reset SOC */
                    #endif
                }
                else if(event.event_info.node_error.status == VX_FAILURE)
                {
                    #ifdef LDC_EVENT_INFO_LOGGING
                    printf("Non-Stalling Error\n");
                    #endif
                }
            }
            else if(event.app_value == LDC_GRAPH_COMPLETED_EVENT)
            {
                graph_completed = 1;
                #ifdef LDC_EVENT_INFO_LOGGING
                printf("LDC GRAPH COMPLETED EVENT received \n");
                #endif
            }
            else if(event.app_value == LDC_NODE_COMPLETED_EVENT)
            {
                #ifdef LDC_EVENT_INFO_LOGGING
                node_completed++;
                printf("LDC NODE COMPLETED EVENT received, nodes completed = %d\n", node_completed);
                #endif
            }
            else
            {
                #ifdef LDC_EVENT_INFO_LOGGING
                printf("Error: Unknown Event Received\n");
                #endif
            }
            
        } while (0 == graph_completed);
        

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            ASSERT_VX_OBJECT(scalar_err_stat_obj[i] = vxCreateScalar(context,
                VX_TYPE_UINT32 , &scalar_out_err_stat[i]),
                (enum vx_type_e)VX_TYPE_SCALAR);

            err_refs[i] = (vx_reference)scalar_err_stat_obj[i];
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node[i], 0, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS, &err_refs[i], 1));

            VX_CALL(vxCopyScalar(scalar_err_stat_obj[i], &scalar_out_err_stat[i], VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST));

            #if defined TEST_GET_ERROR_INTERRUPT_VALUES
            printf("LDC error status value for node %d is %d\n", i, scalar_out_err_stat[i]);
            #endif

            VX_CALL(vxReleaseScalar(&scalar_err_stat_obj[i]));
        }


        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            VX_CALL(vxReleaseNode(&node[i]));
            VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
            VX_CALL(vxReleaseImage(&output_image[i]));
            VX_CALL(vxReleaseImage(&input_image[i]));
        }
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        VX_CALL(vxReleaseImage(&output_image2));

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            ASSERT(node[i] == 0);
            ASSERT(output_image[i] == 0);
            ASSERT(input_image[i] == 0);
            ASSERT(param_obj[i] == 0);
        }
        ASSERT(graph == 0);
        ASSERT(output_image2 == 0);

        tivxHwaUnLoadKernels(context);

    }
}
typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string, *target_string_2;
    int enable_safety_mechanism;
} SetTarget_safety_Arg;

#define ADD_SAFETY_MECHANISM_VPAC12_10(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=reconfigReinitMMR", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=statusRegValidate", __VA_ARGS__, 2))

#define SET_NODE_TARGET_SAFETY_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ADD_SAFETY_MECHANISM_VPAC12_10, ARG, NULL)

#if !defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacLdc, testPsaSignValue_safety_10_12, SetTarget_safety_Arg, SET_NODE_TARGET_SAFETY_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;
    tivx_vpac_ldc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj;
    vx_reference safety_ref[1];

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        
        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        tivx_vpac_ldc_safety_mech_prms_init(&safety_mechanism_params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));

        if(0U != arg_->enable_safety_mechanism)
        {
            switch (arg_->enable_safety_mechanism)
            {
                case 1:
                    /* safety_mechanism=reconfigReinitMmr */
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    break;
                case 2:
                    /* safety_mechanism=statusRegValidate */
                    safety_mechanism_params.enable_status_reg_validation = 1U;
                    break;
                default:
                // Invalid value
                break;
            }

            ASSERT_VX_OBJECT(safety_mechanism_obj = vxCreateUserDataObject(context,
            "tivx_vpac_ldc_safety_mechanism_params_t",
            sizeof(tivx_vpac_ldc_safety_mechanism_params_t),
            &safety_mechanism_params),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
            safety_ref[0] = (vx_reference)safety_mechanism_obj;
            
            VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref, 1));
            VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));
        }
        
       
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_ldc_psa_timestamp_data_t", sizeof(tivx_vpac_ldc_psa_timestamp_data_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);
    
        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
        
        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        ASSERT(checksum_expected[0] == checksum_actual);
        ASSERT(psa_expected[0] == psa_status.psa_values[0]);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        ASSERT(checksum_expected[1] == checksum_actual);
        ASSERT(psa_expected[1] == psa_status.psa_values[1]);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacLdc, testPsaSignValue, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;
#if !defined(VPAC3L)
    tivx_vpac_ldc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj;
    vx_reference safety_ref[1];
#endif
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        
        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
#if !defined(VPAC3L)
        tivx_vpac_ldc_safety_mech_prms_init(&safety_mechanism_params);
#endif
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;
        safety_mechanism_params.enable_readback_config_registers = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
#if !defined(VPAC3L)
        ASSERT_VX_OBJECT(safety_mechanism_obj = vxCreateUserDataObject(context,
        "tivx_vpac_ldc_safety_mechanism_params_t",
        sizeof(tivx_vpac_ldc_safety_mechanism_params_t),
        &safety_mechanism_params),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        
        VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM, safety_ref, 1));
        VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));
#endif
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_ldc_psa_timestamp_data_t", sizeof(tivx_vpac_ldc_psa_timestamp_data_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);
    
        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
        
        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        ASSERT(checksum_expected[0] == checksum_actual);
        ASSERT(psa_expected[0] == psa_status.psa_values[0]);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        ASSERT(checksum_expected[1] == checksum_actual);
        ASSERT(psa_expected[1] == psa_status.psa_values[1]);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif
#ifndef VPAC3L
#ifndef x86_64
/*
 * Test name: testSl2Firewall
 *
 * Purpose: Test LDC with SL2 firewall protection enabled
 *
 * Test Details: This test is identical to testPsaSignValue but with SL2 firewall
 *               protection enabled. Since firewall is transparent to output data,
 *               it reuses the same PSA signature values and checksums for validation.
 *
 * Pass Criteria:
 *    - PSA signature values match expected values
 *    - Checksums match expected values
 *    - No error events occur
 *    - Firewall protection is successfully configured and doesn't affect output
 */
TEST_WITH_ARG(tivxHwaVpacLdc, testSl2Firewall, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    vx_status status;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        uint32_t cpu_id = APP_IPC_CPU_MCU2_0;  /* Default to MCU2_0 for VPAC0 */
        uint32_t vpacInstId = 0;  /* Default to VPAC0 */

#if defined(SOC_J784S4) || defined(SOC_J742S2)
        /* For J784S4/J742S2: VPAC1 (VPAC2_xxx targets) runs on MCU4_0 */
        if ((arg_->target_string != NULL) && (strstr(arg_->target_string, "VPAC2") != NULL))
        {
            cpu_id = APP_IPC_CPU_MCU4_0;
            vpacInstId = 1;  /* VPAC1 */
        }
#endif

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        status = appRemoteServiceRun(cpu_id, APP_VHWA_VPAC_SERVICE_NAME,
                                 APP_VPAC_ENABLE_FIREWALL, &vpacInstId, sizeof(uint32_t), 0);

        ASSERT(status == 0);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj =
            vxCreateUserDataObject(context, "tivx_vpac_ldc_psa_timestamp_data_t", sizeof(tivx_vpac_ldc_psa_timestamp_data_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);

        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_PSA_STATUS,
            psa_refs, 1u));

        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t),
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        ASSERT(checksum_expected[0] == checksum_actual);
        ASSERT(psa_expected[0] == psa_status.psa_values[0]);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        ASSERT(checksum_expected[1] == checksum_actual);
        ASSERT(psa_expected[1] == psa_status.psa_values[1]);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        /* Disable firewall to restore default state */
        {
            uint32_t cpu_id = APP_IPC_CPU_MCU2_0;  /* Default to MCU2_0 for VPAC0 */
            uint32_t vpacInstId = 0;  /* Default to VPAC0 */

#if defined(SOC_J784S4) || defined(SOC_J742S2)
            /* For J784S4/J742S2: VPAC1 (VPAC2_xxx targets) runs on MCU4_0 */
            if ((arg_->target_string != NULL) && (strstr(arg_->target_string, "VPAC2") != NULL))
            {
                cpu_id = APP_IPC_CPU_MCU4_0;
                vpacInstId = 1;  /* VPAC1 */
            }
#endif

            status = appRemoteServiceRun(cpu_id, APP_VHWA_VPAC_SERVICE_NAME,
                                         APP_VPAC_DISABLE_FIREWALL, &vpacInstId, sizeof(uint32_t), 0);
            ASSERT(status == 0);
        }

        tivxHwaUnLoadKernels(context);
    }
}
#endif /* x86_64*/
#endif /* VPAC3L */
// Null check for tivxVpacLdcGetErrStatusCmd
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeErrorInterruptsNullCheck, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj = NULL, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, LDC_NODE_ERROR_EVENT));

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
            
            /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
             * and prints an error message saying that there are no events in the queue */
        vxWaitEvent(context, &event, vx_true_e);

            
        err_refs[0] = (vx_reference)scalar_err_stat_obj;
                
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS,
                    err_refs, 1u);
            

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));


        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testInitFunctions, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{

    //TEST 1
    tivx_vpac_ldc_bandwidth_params_t   prms;
    prms.bandwidth_control = 1u;
    prms.tag_count = 0u;
    prms.max_burst_length = 0u;
    tivx_vpac_ldc_bandwidth_prms_init(&prms);
    ASSERT(prms.bandwidth_control == 0u);
    ASSERT(prms.tag_count == 31u);
    ASSERT(prms.max_burst_length == 1u);
    tivx_vpac_ldc_bandwidth_prms_init(NULL);
    prms.bandwidth_control = 1u;
    prms.tag_count = 0u;
    prms.max_burst_length = 0u;
    ASSERT(prms.bandwidth_control == 1u);
    ASSERT(prms.tag_count == 0u);
    ASSERT(prms.max_burst_length == 0u);

    //TEST 2
    tivx_vpac_ldc_multi_region_params_t params_t;
    params_t.reg_width[0] = 1;
    params_t.reg_width[1] = 1;
    params_t.reg_width[2] = 1;
    tivx_vpac_ldc_multi_region_prms_init(&params_t);
    ASSERT(params_t.reg_width[0] == 0);
    ASSERT(params_t.reg_width[1] == 0);
    ASSERT(params_t.reg_width[2] == 0);
    params_t.reg_width[0] = 1;
    params_t.reg_width[1] = 1;
    params_t.reg_width[2] = 1;
    tivx_vpac_ldc_multi_region_prms_init(NULL);
    ASSERT(params_t.reg_width[0] == 1);
    ASSERT(params_t.reg_width[1] == 1);
    ASSERT(params_t.reg_width[2] == 1);

    //TEST 3
    tivx_vpac_ldc_mesh_params_t prms_2;
    prms_2.subsample_factor = 1;
    tivx_vpac_ldc_mesh_params_init(&prms_2);
    ASSERT(prms_2.subsample_factor == 0);

    prms_2.subsample_factor = 1;
    tivx_vpac_ldc_mesh_params_init(NULL);
    ASSERT(prms_2.subsample_factor == 1);

    //TEST 4
    tivx_vpac_ldc_region_params_t prms_3;
    prms_3.enable = 1u;
    prms_3.out_block_width = 1u;
    prms_3.out_block_height = 1u;
    tivx_vpac_ldc_region_params_init(NULL);
    ASSERT(prms_3.enable == 1u);
    ASSERT(prms_3.out_block_width == 1u);
    ASSERT(prms_3.out_block_height == 1u);

}

/* Test case for out3_img check  */

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testOut3, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, out0 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 5;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        
        vxSetImageAttribute(out3, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                NULL,
                                NULL,
                                NULL,
                                out3), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for size check  */

typedef struct
{
    const char* testName;
    int neg_size;
    char *target_string, *target_string_2;
} Negative_Size;


#define ADD_NEGATIVE_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/neg_size=ldc", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=region", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=multiregion", __VA_ARGS__, 2)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=mesh", __VA_ARGS__, 3))

#define ADD_SET_TARGET_PARAMETERS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL))

#define NEGATIVESIZE_PARAMETERS \
    CT_GENERATE_PARAMETERS("NegativeSize", ADD_NEGATIVE_SIZE, ADD_SET_TARGET_PARAMETERS_CC_TEST, ARG)

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeSize, Negative_Size, NEGATIVESIZE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    tivx_vpac_ldc_multi_region_params_t   multiregion;
    vx_user_data_object multiregion_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        tivx_vpac_ldc_region_params_init(&region);
        tivx_vpac_ldc_multi_region_prms_init(&multiregion);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);
       
        if(arg_->neg_size == 0)
        {
            ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                sizeof(tivx_vpac_ldc_params_t)+1, &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                    sizeof(tivx_vpac_ldc_region_params_t), &region), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
            sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        }
        if(arg_->neg_size == 1)
        {
            ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(multiregion_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_multi_region_params_t",
                    sizeof(tivx_vpac_ldc_multi_region_params_t), &multiregion), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
            ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                    sizeof(tivx_vpac_ldc_region_params_t)+1, &region), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                        sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(multiregion_obj, 0, sizeof(tivx_vpac_ldc_multi_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));     
        }
        if(arg_->neg_size == 2)
        {
            ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                    sizeof(tivx_vpac_ldc_region_params_t), &region), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                        sizeof(tivx_vpac_ldc_mesh_params_t)+1, &mesh_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));          
        } 
        if(arg_->neg_size == 3)
        {
            ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(multiregion_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                    sizeof(tivx_vpac_ldc_multi_region_params_t)+1, &region), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                        sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            VX_CALL(vxCopyUserDataObject(multiregion_obj, 0, sizeof(tivx_vpac_ldc_multi_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));              
        } 

    VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));


    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if((arg_->neg_size == 0) || (arg_->neg_size == 2))
    {
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                    param_obj,
                    NULL,
                    region_obj,
                    mesh_params_obj,
                    NULL,
                    NULL,
                    input_image,
                    output_image,
                    NULL),
                    VX_TYPE_NODE);
    }
    else{
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                    param_obj,
                    NULL,
                    multiregion_obj,
                    mesh_params_obj,
                    NULL,
                    NULL,
                    input_image,
                    output_image,
                    NULL),
                    VX_TYPE_NODE);
    }    

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if(arg_->neg_size == 1){
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,vxVerifyGraph(graph));
        }
        else{
            ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        if((arg_->neg_size == 0) || (arg_->neg_size == 2) || (arg_->neg_size == 1))
        {
            VX_CALL(vxReleaseUserDataObject(&region_obj));
        }
        if((arg_->neg_size == 1) || (arg_->neg_size == 3))
        {
            VX_CALL(vxReleaseUserDataObject(&multiregion_obj));
        }
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* Test case for output_format check  */

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeFormats, ArgFormats,
    PARAMETERS_FORMATS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0 , out1 = 0, in0 = 0, out0 = 0;
    static int testcount = 1;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, 640, 480, arg_->input_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, arg_->output_data_format), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 480, arg_->output_data_format), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        out1),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
       
        if((arg_->input_data_format == VX_DF_IMAGE_U8) || (arg_->input_data_format == VX_DF_IMAGE_U16) || (arg_->input_data_format == TIVX_DF_IMAGE_P12) || (arg_->input_data_format == VX_DF_IMAGE_NV12) || (arg_->input_data_format == TIVX_DF_IMAGE_NV12_P12) || (arg_->input_data_format == VX_DF_IMAGE_UYVY))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseImage(&out1));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(out1 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
        testcount++;
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeGraphProcessing_config, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(NULL);
        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        //Wrong configuration_size from sizeof ( tivx_vpac_ldc_params_t )
        
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t)+1, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        
        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeGraphProcessing_matrix, ArgCmd, PARAMETERS_CMD)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_matrix matrix = 0;
    vx_reference ref[1];
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL, dual_out = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);

        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));
        
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix_cc_test(context, m, 1), VX_TYPE_MATRIX);
        
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVpacLdcNegCov, testNegativeInMeshFormat)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node;
    vx_image input_image, output_image;
    vx_image dual_out_image;
    vx_image mesh_image;
    vx_user_data_object luma_lut, chroma_lut;
    vx_matrix matrix;
    vx_reference ref[2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    uint32_t i;
    vx_uint32 set_stride_y_alignment = 8;

    CT_Image input, output, dual_out = NULL;

    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_LDC1));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_NO_FAILURE(input = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(output = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, 16, 16, 16, 16, VX_TYPE_UINT8));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        {
            ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }

        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        {
            ASSERT_VX_OBJECT(mesh_image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            mesh_params.mesh_frame_width = 16;
            mesh_params.mesh_frame_height = 16;
            mesh_params.subsample_factor = 0;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(mesh_image, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment)));
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }
        VX_CALL(vxReleaseGraph(&graph));
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(graph == 0);

        ASSERT(node == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeIn1Image, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 8;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        vxSetImageAttribute(in1, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxSetImageAttribute(out2, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                out1,
                                in1,
                                out2,
                                out3), VX_TYPE_NODE);
        

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#if defined(VPAC3) || defined(VPAC3L)
/* Test case to cover if(out2_height != (out3_height * 2)) else part*/

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeout2out3heightchek, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 8;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_IYUV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        params.in_img0_yc_mode = 0;
        params.in_img1_yc_mode = 0;
        vxSetImageAttribute(in1, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxSetImageAttribute(out2, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                out1,
                                in1,
                                out2,
                                out3), VX_TYPE_NODE);
        

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeout1Image, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 8;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        vxSetImageAttribute(in1, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxSetImageAttribute(out2, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                out1,
                                NULL,
                                out2,
                                out3), VX_TYPE_NODE);
        

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeout1Image_HeightWidth, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 8;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 64, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxSetImageAttribute(out2, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                out1,
                                NULL,
                                NULL,
                                NULL), VX_TYPE_NODE);
        

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct
{
    const char* testName;
    int neg_size;
    char *target_string, *target_string_2;
} Negative_InImage;
#define ADD_NEGATIVE_INIMAGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/neg_size=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=2", __VA_ARGS__, 2)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=3", __VA_ARGS__, 3)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=4", __VA_ARGS__, 4)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=5", __VA_ARGS__, 5)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=6", __VA_ARGS__, 6)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=7", __VA_ARGS__, 7)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=8", __VA_ARGS__, 8)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=9", __VA_ARGS__, 9))

#define NEGATIVEIMAGE_PARAMETERS \
    CT_GENERATE_PARAMETERS("NegativeSize", ADD_NEGATIVE_INIMAGE, ADD_SET_TARGET_PARAMETERS_CC_TEST, ARG)

/*Test case for validating all negative formats when NULL == in1_img*/
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeout1Image_in1Image, Negative_InImage, NEGATIVEIMAGE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 set_stride_y_alignment = 8;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        if(2 != arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        if(5 != arg_->neg_size){
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        if((4 != arg_->neg_size) && (5 != arg_->neg_size)) {
            ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
  
        if (0 == arg_->neg_size){
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if(1 == arg_->neg_size){
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }
        else if(2 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if(3 == arg_->neg_size){
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if(4 == arg_->neg_size){
            ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if(5 == arg_->neg_size){
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        if(arg_->neg_size < 6){
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        if (6 == arg_->neg_size){
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if(7 == arg_->neg_size) {
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if(8 == arg_->neg_size) {
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if(9 == arg_->neg_size) {
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vxSetImageAttribute(out2, TIVX_IMAGE_STRIDE_Y_ALIGNMENT, &set_stride_y_alignment, sizeof(set_stride_y_alignment));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        if (arg_->neg_size <= 5) 
        {
            ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                out1,
                                in1,
                                out2,
                                NULL), VX_TYPE_NODE);
        }
        else{
            ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                in0,
                                out0,
                                NULL,
                                in1,
                                NULL,
                                out3), VX_TYPE_NODE);
        }

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        
        VX_CALL(vxReleaseImage(&out0));
        if (arg_->neg_size <= 5) 
        {
            VX_CALL(vxReleaseImage(&out1));
        }      
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        if (arg_->neg_size <= 5) 
        {
            ASSERT(out1 == 0);
        }  
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define ADD_NEGATIVE_INOUTIMAGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/neg_size=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=2", __VA_ARGS__, 2)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=3", __VA_ARGS__, 3)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=4", __VA_ARGS__, 4)),\
    CT_EXPAND(nextmacro(testArgName "/neg_size=5", __VA_ARGS__, 5))

#define NEGATIVE_OUTIMAGE_PARAMETERS \
    CT_GENERATE_PARAMETERS("NegativeSize", ADD_NEGATIVE_INOUTIMAGE, ADD_SET_TARGET_PARAMETERS_CC_TEST, ARG)
/*Test case for validating all negative formats when NULL != in1_img*/
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeoutFormat_in1NotNull, Negative_InImage, NEGATIVE_OUTIMAGE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        if (0 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if(1 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if(2 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if(3 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if(4 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }
        else if(5 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define ADD_NEGATIVE_INOUTIMAGE_WH(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/neg_size=ldc", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=region", __VA_ARGS__, 1))

#define NEGATIVE_IMAGEWH_PARAMETERS \
    CT_GENERATE_PARAMETERS("NegativeSize", ADD_NEGATIVE_INOUTIMAGE_WH, ADD_SET_TARGET_PARAMETERS_CC_TEST, ARG)
/* Test case for validatig negative scenarios of width vales between input and output images*/
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeImage_inOutWH, Negative_InImage, NEGATIVE_IMAGEWH_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 64, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        if (0 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 64, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 64, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if(1 == arg_->neg_size){
            ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 128, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define PARAMETERS_CMD_NULL\
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_CC_TEST, ARG, warp_affine_read_image_8u, "lena.bmp")

/* Test for making  mesh_img as  NULL */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeGraphProcessing_Null_mesh_img, ArgCmd, PARAMETERS_CMD_NULL)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_matrix matrix = 0;
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    vx_user_data_object dcc_param_ldc = NULL;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 1), VX_TYPE_MATRIX);

        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);
        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        NULL,
                        dcc_param_ldc,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dcc_param_ldc == 0);


        tivxHwaUnLoadKernels(context);
    }
}

/* Test for making  region_prms as NULL */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeGraphProcessing_Null_region, ArgCmd, PARAMETERS_CMD_NULL)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_matrix matrix = 0;
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    vx_user_data_object dcc_param_ldc = NULL;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 1), VX_TYPE_MATRIX);

        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);
        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        region_obj,
                        mesh_params_obj,
                        NULL,
                        dcc_param_ldc,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dcc_param_ldc == 0);


        tivxHwaUnLoadKernels(context);
    }
}

/* Test for making  mesh_img as  NULL */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeGraphProcessing_Null_meshprms, ArgCmd, PARAMETERS_CMD_NULL)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_matrix matrix = 0;
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    vx_user_data_object dcc_param_ldc = NULL;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 1), VX_TYPE_MATRIX);

        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);
        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        mesh_params_obj,
                        NULL,
                        dcc_param_ldc,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dcc_param_ldc == 0);


        tivxHwaUnLoadKernels(context);
    }
}

#if defined(VPAC3) || defined(VPAC3L)
typedef struct {
    const char* testName;
    int input0_data_format;
    int input1_data_format;
    int in_img0_yc_mode;
    int in_img1_yc_mode;
    char *target_string, *target_string_2;
} ArgFormats_diff_bit_depth_copy;

#define ADD_IN_IMG0_YC_MODE_COPY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/in_img0_yc_mode=0", __VA_ARGS__, 0)) ,\
    CT_EXPAND(nextmacro(testArgName "/in_img0_yc_mode=1", __VA_ARGS__, 1))

#define PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY \
    CT_GENERATE_PARAMETERS("testFormats", ADD_VX_IN0_FORMATS, ADD_VX_IN1_FORMATS, ADD_IN_IMG0_YC_MODE_COPY, ADD_IN_IMG1_YC_MODE, ADD_SET_TARGET_PARAMETERS, ARG)


/*Test for Create function IN case*/

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testIn0In1ImgDesc, ArgFormats_diff_bit_depth_copy, PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_status status;
    tivx_vpac_ldc_params_t params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 640, 480, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 640, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 640, 128, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 640, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 640, 256, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);

        params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        params.in_img1_yc_mode = arg_->in_img1_yc_mode;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if((arg_->input0_data_format == VX_DF_IMAGE_U8) || (arg_->input0_data_format == VX_DF_IMAGE_U16) || (arg_->input0_data_format == TIVX_DF_IMAGE_P12))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testOut2Out3ImgDesc, ArgFormats_diff_bit_depth_copy, PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;
    vx_matrix matrix = 0;
    vx_float32 m[6];
    vx_status status;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 640, 480, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 640, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 640, 128, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 640, 16, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 640, 256, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, VX_TYPE_INT16, 3, 3), VX_TYPE_MATRIX);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);

        params.input_align_12bit = 1;
        params.yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            matrix,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if((arg_->input0_data_format == VX_DF_IMAGE_U8) || (arg_->input0_data_format == VX_DF_IMAGE_U16) || (arg_->input0_data_format == TIVX_DF_IMAGE_P12))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(matrix == 0);
        ASSERT(param_obj == 0);
        
        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testOut2Out3Height, ArgFormats_diff_bit_depth_copy, PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;
    vx_matrix matrix = 0;
    vx_float32 m[6];
    vx_status status;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 640, 480, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 640, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 640, 128, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 640, 16, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 640, 256, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, VX_TYPE_INT16, 3, 3), VX_TYPE_MATRIX);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);

        params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        params.in_img1_yc_mode = arg_->in_img1_yc_mode;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
       
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            matrix,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1));

        if((arg_->input0_data_format == VX_DF_IMAGE_U8) || (arg_->input0_data_format == VX_DF_IMAGE_U16) || (arg_->input0_data_format == TIVX_DF_IMAGE_P12))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(matrix == 0);
        ASSERT(param_obj == 0);
        
        tivxHwaUnLoadKernels(context);
    }
}

#endif

TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testNodeControl, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
        ASSERT_VX_OBJECT(bandwidth_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_bandwidth_params_t), &bandwidth_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        
        ref[0] = (vx_reference)bandwidth_params_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, ref, 1);
                               

        VX_CALL(vxProcessGraph(graph)); 
        checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect); 
        checksum_expected = 0x2d2cdc00;
        ASSERT(checksum_expected == checksum_actual);      

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(bandwidth_params_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* Test case to cover invalid size of tivx_vpac_ldc_bandwidth_params_t in tivxVpacLdcSetRdBwLimitCmd */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testBandwidthParamSizeCheck, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
        ASSERT_VX_OBJECT(bandwidth_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_bandwidth_params_t)+1, &bandwidth_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
       
        ref[0] = (vx_reference)bandwidth_params_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, ref, 1);
                                      
        VX_CALL(vxProcessGraph(graph));        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(bandwidth_params_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testControlErrStatus, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj = NULL, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, LDC_NODE_ERROR_EVENT));

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        
        /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
         * and prints an error message saying that there are no events in the queue */
        vxWaitEvent(context, &event, vx_true_e);


            ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
            VX_TYPE_UINT32 , &scalar_out_err_stat),
            (enum vx_type_e)VX_TYPE_SCALAR);
        
            err_refs[0] = (vx_reference)scalar_err_stat_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS, err_refs, 1));    

            VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST));
            #if defined TEST_GET_ERROR_INTERRUPT_VALUES
            printf("LDC error status value is %d\n", scalar_out_err_stat);
            #endif
           
            VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 
        
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect); 
            checksum_expected = 0x2d2cdc00;
            ASSERT(checksum_expected == checksum_actual); 


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));


        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* The below test is multi-handle test case with a stalling error in one of the nodes.
 * This cannot be part of the test run due to the stalling error.
 * However, this test is required to test error interrupt features. */
TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testControlErrStatusMultiHandle, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0}, output_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0}, output_image2 = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_scalar scalar_err_stat_obj[TEST_NUM_MULTI_NODE_INSTANCE] = {NULL};
    vx_enum scalar_out_err_stat[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_reference err_refs[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            tivx_vpac_ldc_params_init(&params);
            /* Enable all events by default */
            params.enable_error_events =
                TIVX_VPAC_LDC_PIX_BLK_OUTOFBOUND_ERR_STATUS |
                TIVX_VPAC_LDC_MESH_BLK_OUTOFBOUND_ERR_STATUS |
                TIVX_VPAC_LDC_PIX_MEM_OVF_ERR_STATUS |
                TIVX_VPAC_LDC_MESH_MEM_OVF_ERR_STATUS |
                TIVX_VPAC_LDC_PIX_FRM_OUTOFBOUND_ERR_STATUS |
                TIVX_VPAC_LDC_SZ_OVF_ERR_STATUS |
                TIVX_VPAC_LDC_SL2_WR_ERR_STATUS |
                TIVX_VPAC_LDC_RD_ERR_STATUS |
                TIVX_VPAC_LDC_WDTIMER_ERR;

            ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                    sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            if(i == 3)
            {
                ASSERT_VX_OBJECT(input_image[i] = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_UYVY, &pixel), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image[i] = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                    param_obj[i],
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    input_image[i],
                    output_image[i],
                    output_image2),
                    VX_TYPE_NODE);
            }
            else
            {
                ASSERT_VX_OBJECT(input_image[i] = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image[i] = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                    param_obj[i],
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    input_image[i],
                    output_image[i],
                    NULL),
                    VX_TYPE_NODE);
            }

            VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
        }

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            ASSERT_VX_OBJECT(scalar_err_stat_obj[i] = vxCreateScalar(context,
                VX_TYPE_UINT32 , &scalar_out_err_stat[i]),
                (enum vx_type_e)VX_TYPE_SCALAR);

            err_refs[i] = (vx_reference)scalar_err_stat_obj[i];
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node[i], 0, TIVX_VPAC_LDC_CMD_GET_ERR_STATUS, &err_refs[i], 1));

            VX_CALL(vxCopyScalar(scalar_err_stat_obj[i], &scalar_out_err_stat[i], VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST));

            #ifdef TEST_GET_ERROR_INTERRUPT_VALUES
            printf("LDC error status value for node %d is %d\n", i, scalar_out_err_stat[i]);
            #endif

            VX_CALL(vxReleaseScalar(&scalar_err_stat_obj[i]));
            /* i>3 because the third node will trigger watchdog timer timeout, and no processing will
             take place for downstream nodes */
            if(0u == (scalar_out_err_stat[i] & ~(TIVX_VPAC_LDC_ALL_ERR_EVENTS & ~TIVX_VPAC_LDC_WDTIMER_ERR)) && i > 3 )
            {
                checksum_actual = tivx_utils_simple_image_checksum(output_image[i], 0, rect);
                checksum_expected = 0x2d2cdc00;
                ASSERT(checksum_expected == checksum_actual);
            }
        }


        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            VX_CALL(vxReleaseNode(&node[i]));
            VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
            VX_CALL(vxReleaseImage(&output_image[i]));
            VX_CALL(vxReleaseImage(&input_image[i]));
        }
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output_image2));

        for (int i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            ASSERT(node[i] == 0);
            ASSERT(output_image[i] == 0);
            ASSERT(input_image[i] == 0);
            ASSERT(param_obj[i] == 0);
        }
        ASSERT(graph == 0);
        ASSERT(output_image2 == 0);

        tivxHwaUnLoadKernels(context);

    }
}

typedef struct {
    const char* testName;
    int fmt;
    char *target_string, *target_string_2;
} ArgFmt;

#define ADD_FMT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fmt=VX_DF_IMAGE_U16", __VA_ARGS__, 0))

#define PARAMETERS_FMT \
    CT_GENERATE_PARAMETERS("testSeFmt", ADD_FMT, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testSetFmt, ArgFmt,
    PARAMETERS_FMT)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        if(0==arg_->fmt)
        {
        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U16, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }
        else
        {
        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, TIVX_DF_IMAGE_P12, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        }

        tivx_vpac_ldc_params_init(&params);

        params.yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                              
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxProcessGraph(graph));
        
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testWarpMatrix, ArgFormats_diff_bit_depth_copy, PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;
    vx_matrix matrix = 0;
    vx_float32 m[6];
    vx_status status;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 640, 480, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 640, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 640, 128, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 640, 16, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 640, 256, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3), VX_TYPE_MATRIX);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);

        params.input_align_12bit = 1;
        params.yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            matrix,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if((arg_->input0_data_format == VX_DF_IMAGE_U8) || (arg_->input0_data_format == VX_DF_IMAGE_U16) || (arg_->input0_data_format == TIVX_DF_IMAGE_P12))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(matrix == 0);
        ASSERT(param_obj == 0);
        
        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testIn0ImgChroma, ArgFormats_diff_bit_depth_copy, PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY)
{
    vx_context context = context_->vx_context_;
    vx_image in0 = 0, in1 = 0, out0 = 0, out1 = 0, out2 = 0, out3 = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;
    vx_status status;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
   
        ASSERT_VX_OBJECT(in0 = vxCreateImage(context, 640, 480, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out0 = vxCreateImage(context, 640, 120, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(in1 = vxCreateImage(context, 640, 128, arg_->input1_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out1 = vxCreateImage(context, 640, 64, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out2 = vxCreateImage(context, 640, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(out3 = vxCreateImage(context, 640, 256, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_params_init(&params);

        params.in_img0_yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                            param_obj,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            in0,
                            out0,
                            out1,
                            in1,
                            out2,
                            out3), VX_TYPE_NODE);

        
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if((arg_->input0_data_format == VX_DF_IMAGE_U8) || (arg_->input0_data_format == VX_DF_IMAGE_U16) || (arg_->input0_data_format == TIVX_DF_IMAGE_P12))
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&out0));
        VX_CALL(vxReleaseImage(&out1));
        VX_CALL(vxReleaseImage(&in0));
        VX_CALL(vxReleaseImage(&in1));
        VX_CALL(vxReleaseImage(&out2));
        VX_CALL(vxReleaseImage(&out3));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(out0 == 0);
        ASSERT(in0 == 0);
        ASSERT(in1 == 0);
        ASSERT(out1 == 0);
        ASSERT(out2 == 0);
        ASSERT(out3 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testinput_align, ArgFormats_diff_bit_depth,
    PARAMETERS_FORMATS_DIFF_BIT_DEPTH
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image src_image0 = 0, src_image1 = 0;
    int src_w0 = 640, src_h0 = 480, src_w1 = 640, src_h1 = 480;
    int dst_width[2], dst_height[2];
    vx_image dst_image[2] = {0};
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        for(int i=0; i<2; i++)
        {
            if(i % 2 == 0)
            {
                dst_width[i] = src_w0;
                dst_height[i] = src_h0;
            }
            else
            {
                dst_width[i] = src_w1;
                dst_height[i] = src_h1;
            }
        }
        if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
        {
            src_h1 = src_h1/2;
            dst_height[1] = dst_height[1]/2;        
        }
        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
        for(int i = 0; i < 2; i++)
        {
            if(i % 2 == 0)
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
            }
        }
        tivx_vpac_ldc_params_init(&params);
        params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        params.in_img1_yc_mode = arg_->in_img1_yc_mode;

        params.input_align_12bit = TIVX_VPAC_LDC_ALIGN_MSB;
        params.yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        src_image0,
                        dst_image[0],
                        NULL,
                        src_image1,
                        dst_image[1],
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        for(int i=0; i<2; i++)
        {
            int dst_img_fmt;
            vx_char temp[256];
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];        
            if(i % 2 == 0)
            {
                dst_img_fmt = arg_->input0_data_format;
                checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            }
            else
            {
                dst_img_fmt = arg_->input1_data_format;
                checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            }
            #if defined(TEST_LDC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            if(dst_img_fmt == VX_DF_IMAGE_U8)
            {
                if(arg_->in_img0_yc_mode == 0)
                {
                    if(arg_->in_img1_yc_mode == 0)
                    {
                        ASSERT(ldc_diff_bit_depth[0] == checksum_actual);
                    }
                    else
                    {
                        if(i % 2 == 0)
                        {
                            ASSERT(ldc_diff_bit_depth[0] == checksum_actual);
                        }
                        else
                        {
                            ASSERT(ldc_diff_bit_depth[1] == checksum_actual);
                        }
                    }
                }
            }
            if(dst_img_fmt == TIVX_DF_IMAGE_P12)
            {
                if(arg_->in_img0_yc_mode == 0)
                {
                    if(arg_->in_img1_yc_mode == 0)
                    {
                        ASSERT(ldc_diff_bit_depth[4] == checksum_actual);
                    }
                    else
                    {
                        if(i % 2 == 0)
                        {
                            ASSERT(ldc_diff_bit_depth[4] == checksum_actual);
                        }
                        else
                        {
                            ASSERT(ldc_diff_bit_depth[5] == checksum_actual);
                        }
                    }
                }
            }
        }
   
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&src_image0));
        VX_CALL(vxReleaseImage(&src_image1));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        for (int i = 0; i < 2; i++)
        {
            VX_CALL(vxReleaseImage(&dst_image[i]));
            ASSERT(dst_image[i] == 0);
        }
        ASSERT(src_image0 == 0);
        ASSERT(src_image1 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}
#endif

TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testDCCParams, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];
    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ref[0] = (vx_reference)dcc_param_ldc;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS, ref, 1);

        VX_CALL(vxProcessGraph(graph));        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* negative test case but keep here because negative test group crossed the limit*/
TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testPsaStatus, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        
        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_ldc_psa_timestamp_data_t", sizeof(tivx_vpac_ldc_psa_timestamp_data_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);
    
        psa_refs[0] = (vx_reference) NULL;
        
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS,
            psa_refs, 1u);
        
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[0] == checksum_actual);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[1] == checksum_actual); 

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#if defined(VPAC3) || defined(VPAC3L)
/* To check input size in tivxAddKernelVpacLdcValidate*/
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeinputsize, ArgFormats_diff_bit_depth_copy,
    PARAMETERS_FORMATS_DIFF_BIT_DEPTH_COPY
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image src_image0 = 0, src_image1 = 0;
    int src_w0 = 640, src_h0 = 480, src_w1 = 640, src_h1 = 480;
    int dst_width[2], dst_height[2];
    vx_image dst_image[2] = {0};
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        for(int i=0; i<2; i++)
        {
            if(i % 2 == 0)
            {
                dst_width[i] = src_w0;
                dst_height[i] = src_h0;
            }
            else
            {
                dst_width[i] = src_w1;
                dst_height[i] = src_h1;
            }
        }
        if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
        {
            src_h1 = src_h1/2;
            dst_height[1] = dst_height[1]/2;        
        }
        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
        for(int i = 0; i < 2; i++)
        {
            if(i % 2 == 0)
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
            }
        }
        tivx_vpac_ldc_params_init(&params);
        params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        params.in_img1_yc_mode = arg_->in_img1_yc_mode;

        params.input_align_12bit = TIVX_VPAC_LDC_ALIGN_MSB;
        params.yc_mode = TIVX_VPAC_LDC_MODE_CHROMA_ONLY;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t)+1, &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode2(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        src_image0,
                        dst_image[0],
                        NULL,
                        src_image1,
                        dst_image[1],
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));
         
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&src_image0));
        VX_CALL(vxReleaseImage(&src_image1));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        for (int i = 0; i < 2; i++)
        {
            VX_CALL(vxReleaseImage(&dst_image[i]));
            ASSERT(dst_image[i] == 0);
        }
        ASSERT(src_image0 == 0);
        ASSERT(src_image1 == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}
#endif

TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testDefaultControl, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ref[0] = (vx_reference)param_obj;
        tivxNodeSendCommand(node, 0, 0x10000006u, ref, 1);                   

        VX_CALL(vxProcessGraph(graph));        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);


        tivxHwaUnLoadKernels(context);

    }
}

/* Test case to cover if(0u != cfg->outputStartX)  and if(0u != cfg->outputStartY) in CSL_ldcSetConfig function */
TEST_WITH_ARG(tivxHwaVpacLdcPosCov, testxy, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS )
{    
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        
        params.init_x = 8;
        params.init_y = 8;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
        ASSERT_VX_OBJECT(bandwidth_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_bandwidth_params_t), &bandwidth_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        
        ref[0] = (vx_reference)bandwidth_params_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, ref, 1);
                                      
        VX_CALL(vxProcessGraph(graph));    
        checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect); 
        checksum_expected = 0x2d2cdc00;
        ASSERT(checksum_expected == checksum_actual);     

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(bandwidth_params_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* Test case to cover CSL_ldcSetLumaToneMapLutCfg when lutCfg->inputBits is 13 and lutCfg->outputBits is 7 , to cover this we should enable LDC_REMAP_LUT_DRV_EN */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testlutcreatenew, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_user_data_object luma_lut = NULL, chroma_lut = NULL;
    vx_matrix matrix = 0;
    vx_reference ref[2];
    vx_float32 m[9];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;

    CT_Image input = NULL, output = NULL, dual_out = NULL;

    vx_border_t border = arg_->border;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_perspective_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
        ASSERT_VX_OBJECT(matrix = warp_perspective_create_matrix(context, m, 1), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }

        if(arg_->output_mode == 1)
        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create_new(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            if (arg_->input_mode == 1)
            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create_new(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = arg_->interp_type;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        if(arg_->mesh_mode > 0)
        {
            mesh_image = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
            mesh_params.mesh_frame_width = arg_->out_width;
            mesh_params.mesh_frame_height = arg_->out_height;
            mesh_params.subsample_factor = arg_->mesh_mode-1;
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));

        if(arg_->output_mode == 1)  
        {
            ref[0] = (vx_reference) luma_lut;
            ref[1] = (vx_reference) chroma_lut;
            VX_CALL(tivxNodeSendCommand(node, 0,
                TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS, ref, 2));
        }

        VX_CALL(vxProcessGraph(graph));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_NO_FAILURE(output = ct_image_from_vx_image(output_image));
            checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }

        if(arg_->output_mode == 1)
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            if(arg_->input_mode == 1)
            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }

        if(arg_->mesh_mode > 0)
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);

        if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
           (arg_->matrix_type != VX_MATRIX_SCALE))
        {
            ASSERT_NO_FAILURE(warp_perspective_check(input, output, arg_->interp_type, arg_->border, m));
        }
    }
}

// negative size check for tivxVpacLdcGetPsaStatusCmd
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativePsaSize, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        
        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_ldc_psa_timestamp_data_t", sizeof(tivx_vpac_ldc_psa_timestamp_data_t)+1, NULL),
            VX_TYPE_USER_DATA_OBJECT);
    
        psa_refs[0] = (vx_reference) psa_obj;
        
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_PSA_STATUS,
            psa_refs, 1u);
        
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}

// null check for tivxVpacLdcGetPsaStatusCmd
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativePsaNullObj, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t psa_expected[2] = {0xf63d6931,0xf11770c2};
    uint32_t checksum_actual;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[4] = {0};
    vx_user_data_object psa_obj = NULL;
    tivx_vpac_ldc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        
        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)input, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;
        params.enable_psa = 1U;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

    
        psa_refs[0] = (vx_reference) psa_obj;
        
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_LDC_CMD_GET_PSA_STATUS,
            psa_refs, 1u);
        
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_ldc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));   

        ASSERT_NO_FAILURE(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}

// Null check for tivxVpacLdcSetRdBwLimitCmd
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testNegativeBwLimitCmdNullCheck, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj = NULL;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        
        ref[0] = (vx_reference)bandwidth_params_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, ref, 1);
        
                               

        VX_CALL(vxProcessGraph(graph));        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

/* Test case to cover  else if (bwLimitCfg->rdMaxBurstLength >= VHWA_LDC_MAX_BURST_LENGTH_MAX) in CSL_ldcSetBwLimitConfig function */
TEST_WITH_ARG(tivxHwaVpacLdcNegCov, testbwLimitCfg_rdMaxBurstLength, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_bandwidth_params_t bandwidth_params;
    vx_user_data_object bandwidth_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_reference ref[1];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_vpac_ldc_bandwidth_prms_init(&bandwidth_params);
        bandwidth_params.max_burst_length = 4;
        ASSERT_VX_OBJECT(bandwidth_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_bandwidth_params_t), &bandwidth_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        
        ref[0] = (vx_reference)bandwidth_params_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS, ref, 1);
        
                               
        VX_CALL(vxProcessGraph(graph));        

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&bandwidth_params_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(bandwidth_params_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

static uint32_t multiregion_expected_cksm[] = {
    0x00f3ac34,
    0xc8295b0e,
    
    0xf9fead28,
    0xc82a5a0f,
    
    };
    typedef struct
    {
        const char* testName;
        CT_Image(*generator)(const char* fileName, int width, int height);
        char *target_string, *target_string_2;
        int multiregion;
    } ArgDcc;
    
    #define ADD_MULTIREGION(testArgName, nextmacro, ...) \
        CT_EXPAND(nextmacro(testArgName "/multiregion=off", __VA_ARGS__, 0)), \
        CT_EXPAND(nextmacro(testArgName "/multiregion=on", __VA_ARGS__, 1))
    
    #define SET_NODE_TARGET_PARAMETERS_MULTI_REGION \
        CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ADD_MULTIREGION, ARG, NULL)
    
    TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingDccMultiRegion, ArgDcc, SET_NODE_TARGET_PARAMETERS_MULTI_REGION)
    {
        vx_context context = context_->vx_context_;
        vx_image input = 0, output = 0;
        vx_user_data_object param_obj;
        vx_graph graph = 0;
        vx_node node = 0;
        tivx_vpac_ldc_params_t params;
    
        vx_user_data_object dcc_param_ldc = NULL;
        const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
        vx_size dcc_buff_size = 1;
        vx_map_id dcc_ldc_buf_map_id;
        uint8_t * dcc_ldc_buf;
        int32_t dcc_status;
        uint32_t sensor_dcc_id;
        uint32_t sensor_dcc_mode;
        char *sensor_name = NULL;
        char *file_name = NULL;
        char file[MAXPATHLENGTH];
        uint32_t checksum_actual;
        size_t sz;
        vx_rectangle_t rect;
    
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
        sensor_dcc_id = 390;
        file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    
        if (NULL != arg_->target_string)
        {
            ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
        }
        if (NULL != arg_->target_string_2)
        {
            ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
        }
    
        {
            tivxHwaLoadKernels(context);
            CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    
            ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    
            ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    
            sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
            ASSERT_(return, (sz < MAXPATHLENGTH));
    
            VX_CALL(readNV12Input(file, input));
    
            tivx_vpac_ldc_params_init(&params);
            params.luma_interpolation_type = 1;
            params.dcc_camera_id = sensor_dcc_id;
    
            ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                                sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    
            VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    
            if(1 == arg_->multiregion)
            {
                /* Creating DCC */
                dcc_buff_size = DCC_LDC_IMX390_DCC_MULTI_REGION_CFG_NUM_ELEM;
    
                ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                    dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    
                VX_CALL(vxMapUserDataObject(
                    dcc_param_ldc,
                    0,
                    dcc_buff_size,
                    &dcc_ldc_buf_map_id,
                    (void **)&dcc_ldc_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    0
                ));
                memset(dcc_ldc_buf, 0xAB, dcc_buff_size);
    
                static uint8_t  dcc_ldc_imx390_multiregion[DCC_LDC_IMX390_DCC_MULTI_REGION_CFG_NUM_ELEM] = DCC_LDC_IMX390DCC__MULTI_REGION_CFG;
                memcpy(dcc_ldc_buf, dcc_ldc_imx390_multiregion, dcc_buff_size);
    
                VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
                /* Done w/ DCC */
            }
            else
            {
                /* Creating DCC */
                dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);
    
                ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
                    dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    
                VX_CALL(vxMapUserDataObject(
                    dcc_param_ldc,
                    0,
                    dcc_buff_size,
                    &dcc_ldc_buf_map_id,
                    (void **)&dcc_ldc_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    0
                ));
                memset(dcc_ldc_buf, 0xAB, dcc_buff_size);
    
                dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
                ASSERT(dcc_status == 0);
    
                VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
                /* Done w/ DCC */
            }
    
            ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                    param_obj,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    dcc_param_ldc,
                                    input,
                                    output,
                                    NULL), VX_TYPE_NODE);
    
            VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
    
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = 1920;
            rect.end_y = 1080;
    
            checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
    
            if(0 == arg_->multiregion)
            {
                ASSERT(multiregion_expected_cksm[0] == checksum_actual);
            }
            else
            {
                ASSERT(multiregion_expected_cksm[2] == checksum_actual);
            }
            rect.end_y /= 2;
            checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
    
            if(0 == arg_->multiregion)
            {
                ASSERT(multiregion_expected_cksm[1] == checksum_actual);
            }
            else
            {
                ASSERT(multiregion_expected_cksm[3] == checksum_actual);
            }
    
            VX_CALL(vxReleaseNode(&node));
            VX_CALL(vxReleaseGraph(&graph));
            VX_CALL(vxReleaseImage(&output));
            VX_CALL(vxReleaseImage(&input));
            VX_CALL(vxReleaseUserDataObject(&param_obj));
            VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));
    
            ASSERT(node == 0);
            ASSERT(graph == 0);
            ASSERT(output == 0);
            ASSERT(input == 0);
            ASSERT(param_obj == 0);
            ASSERT(dcc_param_ldc == 0);
    
            tivxHwaUnLoadKernels(context);
        }
    }

    #define PARAMETERS_FORMATS_MULTI_NODE \
    CT_GENERATE_PARAMETERS("testFormats", ADD_VX_IN_FORMATS, ADD_VX_OUT_FORMATS, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

TEST_WITH_ARG(tivxHwaVpacLdc, testFormats_multi_node, ArgFormats,
    PARAMETERS_FORMATS_MULTI_NODE
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_user_data_object param_obj[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0}, output_image[TEST_NUM_MULTI_NODE_INSTANCE] = {0};
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;
    int i;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {

                ASSERT_VX_OBJECT(input_image[i] = vxCreateUniformImage(context, 640, 480, arg_->input_data_format, &pixel), VX_TYPE_IMAGE);
                ASSERT_VX_OBJECT(output_image[i] = vxCreateImage(context, 640, 480, arg_->output_data_format), VX_TYPE_IMAGE);

                tivx_vpac_ldc_params_init(&params[i]);
                ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                                    sizeof(tivx_vpac_ldc_params_t), &params[i]), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                                param_obj[i],
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                input_image[i],
                                output_image[i],
                                NULL),
                                VX_TYPE_NODE);

                
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));

                if (0 == isFormatComboValid(arg_->input_data_format, arg_->output_data_format, 0))
                {
                    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
                }
                else
                {
                    VX_CALL(vxVerifyGraph(graph));
                    VX_CALL(vxProcessGraph(graph));
                    checksum_expected = get_formats_checksum(arg_->input_data_format, arg_->output_data_format);
                    checksum_actual = tivx_utils_simple_image_checksum(output_image[i], 0, rect);
                    ASSERT(checksum_expected == checksum_actual);
                }
            }
        }

        for (i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
                VX_CALL(vxReleaseImage(&output_image[i]));
                VX_CALL(vxReleaseImage(&input_image[i]));
            }
        }
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_MULTI_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {
                ASSERT(node[i] == 0);
                ASSERT(output_image[i] == 0);
                ASSERT(input_image[i] == 0);
                ASSERT(param_obj[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);

    }
}

TESTCASE_TESTS(tivxHwaVpacLdc,
    testNodeCreation,
    testGraphProcessingDcc,
    testGraphProcessing
    #if defined(VPAC3)
    #ifndef x86_64
    ,
    testGraphProcessingSafety,
    testGraphProcessingDccWithLut,
    testGraphProcessingDccWithLutSafety
    #endif
    #endif
    ,
    testGraphProcessingPerspective,
    testFormats,
    testNegativeGraph,
    testNegativeMatrixType,
    testNegativeStrideInMesh,
    testNegativeStrideInImage,
    testGraphProcessingCommand,
    testGraphProcessingDccMultiRegion,
    testFormats_multi_node
    #if defined(VPAC3) || defined(VPAC3L)
    ,
    GraphProcessing_Diff_Depth,
    testGraphProcessing_RGB_Plane_Split
    #endif
    #ifndef x86_64
    #if defined(TEST_LDC_TRIGGER_WATCHDOG_TIMER)
    ,
    testErrorInterrupts
    #endif
    #if !defined(VPAC3L)
    ,
    testPsaSignValue,
    testPsaSignValue_safety_10_12
    #endif
    #ifndef VPAC3L
    ,
    testSl2Firewall
    #endif
    #endif
    )

TESTCASE_TESTS(tivxHwaVpacLdcPosCov,
    testNodeControl,

#if defined(VPAC3) || defined(VPAC3L)
    testinput_align,
#endif
    #ifndef x86_64     
    testDCCParams,
    testPsaStatus,
    testControlErrStatus,
    // testControlErrStatusMultiHandle,
    #endif 
    testxy    
    )
TESTCASE_TESTS(tivxHwaVpacLdcNegCov,
    testNegativePsaSize,
    testNegativePsaNullObj,
    testNegativeErrorInterruptsNullCheck,
    testNegativeBwLimitCmdNullCheck,
    testNegativeSize,
    testNegativeFormats,
    testNegativeInMeshFormat,
    testNegativeGraphProcessing_config,
    testNegativeGraphProcessing_matrix,
    testNegativeIn1Image,
    testNegativeout1Image,
#if defined(VPAC3) || defined(VPAC3L)
    testNegativeinputsize,
#endif 
    testNegativeout1Image_HeightWidth,
    testNegativeout1Image_in1Image, 
    testNegativeoutFormat_in1NotNull,
    testNegativeImage_inOutWH,
    testNegativeGraphProcessing_Null_mesh_img,
    testNegativeGraphProcessing_Null_region,
    testNegativeGraphProcessing_Null_meshprms,
    testbwLimitCfg_rdMaxBurstLength,
#if defined(VPAC3) || defined(VPAC3L)
    testIn0In1ImgDesc,
    testOut2Out3ImgDesc,
    testOut2Out3Height,
#endif
    testSetFmt,
#if defined(VPAC3) || defined(VPAC3L)
    testWarpMatrix,
    testIn0ImgChroma,
    testNegativeout2out3heightchek,
#endif
    testDefaultControl,
    testOut3,
    testBandwidthParamSizeCheck,
    testInitFunctions
    #if defined(TEST_ENABLE_SAFETY_MECHANISIMS)
    ,
    testlutcreatenew
    #endif
    )
#endif /* BUILD_VPAC_LDC */