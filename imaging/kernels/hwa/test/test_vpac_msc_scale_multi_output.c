/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef BUILD_VPAC_MSC

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/hwa_kernels.h>
#include <TI/hwa_vpac_msc.h>
#include "test_engine/test.h"
#include <string.h>
#include <inttypes.h>

#include <math.h> // floorf
#include "test_hwa_common.h"
#include "tivx_utils_checksum.h"
#include "tivx_utils_file_rd_wr.h"
/* #define TEST_MSC_CHECKSUM_LOGGING */
/* #define TEST_GET_ERROR_INTERRUPT_VALUES */
/* #define TEST_PRINT_TIME_STAMP_VALUES */
/* #define TEST_MSC_ENABLE */
#define TEST_NUM_NODE_INSTANCE 8
#define TEST_NUM_PSA_NODE_INSTANCE 2
#define TEST_MSC_TOTAL_OUTPUTS  (5U)
#define MSC_NODE_ERROR_EVENT  (1U)
#define TEST_NUM_OUTPUTS  (4U)
#define SET_TIME_STAMP_VALUE  (100U)
#define TEST_MSC_MAX_OUTPUTS  (10U)
#define TEST_NUM_OUTPUTS_YUV422I  (4U)

TESTCASE(tivxHwaVpacMscScaleMultiOutput, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacMscScaleMultiOutputPositive, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacMscScaleMultiOutputNegative, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacMscScaleMultiOutputNegative2, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    int isEnableSimulProcessing;
} ArgFixed;

#if (VPAC_COUNT > 1)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC2))

#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC2_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_VPAC2_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC2, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_VPAC_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC1/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC1, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC2/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC2, TIVX_TARGET_VPAC2_MSC2))

#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2))

#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_VPAC_MSC2))
#endif

#define ADD_DUMMY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "", __VA_ARGS__, 0))

#define PARAMETERS_FIX \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ARG)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testNodeCreation, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator)(int width, int height, int* dst_width, int* dst_height);
    int crop_mode;
    uint32_t checksum;
    int width, height;
    char* target_string;
} Arg_OneOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_TwoOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_ThreeOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator3)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_FourOutput;

typedef struct {
    const char* testName;
    int dummy;
    vx_enum interpolation;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    void (*dst_size_generator0)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator1)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator2)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator3)(int width, int height, int* dst_width, int* dst_height);
    void (*dst_size_generator4)(int width, int height, int* dst_width, int* dst_height);
    int exact_result;
    int width, height;
    char* target_string;
    vx_border_t border;
} Arg_FiveOutput;

#define STR_VX_INTERPOLATION_NEAREST_NEIGHBOR "NN"
#define STR_VX_INTERPOLATION_BILINEAR "BILINEAR"
#define STR_VX_INTERPOLATION_AREA "AREA"

#define ADD_CROP_0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/crop=none", __VA_ARGS__, 0))

#define ADD_CROP_1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/crop=1", __VA_ARGS__, 1))


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FixedPattern, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 16, h = 16, i, j, crop_mode = 0;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = j*16;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        vxUnmapImagePatch(src_image, map_id);
#endif
        dst_width = w-4;
        dst_height = h-4;

        if(crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        img_scale_set_coeff(&coeffs, interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        if(crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            /* Center crop of input */
            crop.crop_start_x = w / 4;
            crop.crop_start_y =h / 4;
            crop.crop_width   = w / 2;
            crop.crop_height  = h / 2;

            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = (vx_reference)crop_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 5u));

            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 0
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        vxMapImagePatch(dst_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr2;

        for(j=0; j < dst_height; j++)
        {
            for(i=0; i<dst_width; i++)
            {
                printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            printf("\n");
        }

        vxUnmapImagePatch(dst_image, map_id);
#endif

        //ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        //printf("0x%08x\n", checksum_actual);
        //printf("end_x = %d\n", dst_width);
        //printf("end_y = %d\n", dst_height);

        ASSERT((uint32_t)0xdbdbdbcb == checksum_actual);

        //save_image_from_msc(dst_image, "output/lena_msc");

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FixedPattern_tiovx_1129, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 3840, h = 2160, i, j, crop_mode = 0;
    int dst_width = 1280, dst_height = 720;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = 128;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        vxUnmapImagePatch(src_image, map_id);

        rect.end_y = h/2;
        vxMapImagePatch(src_image,
            &rect,
            1,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h/2; j++)
        {
            for(i=0; i<w; i+=2)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = 128-64;
                data_ptr_u8[j*image_addr.stride_y+i+1] = 128+64;
                //printf("%03d,", data_ptr_u8[j*image_addr.stride_y+i]);
            }
            //printf("\n");
            //printf("\n");
        }

        //printf("%03d,", data_ptr_u8[0]);
        //printf("%03d,", data_ptr_u8[1]);
        //printf("\n");

        vxUnmapImagePatch(src_image, map_id);

#endif
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        img_scale_set_coeff(&coeffs, interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 1
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height/2;

        vxMapImagePatch(dst_image,
            &rect,
            1,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr2;

        //printf("%03d,", data_ptr_u8[0]);
        //printf("%03d,", data_ptr_u8[1]);
        //printf("\n");

        //ASSERT(128+64 == data_ptr_u8[0]);
        //ASSERT(128-64 == data_ptr_u8[1]);


        vxUnmapImagePatch(dst_image, map_id);
#endif

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    rect.start_x = 0;
    rect.start_y = 0;

    for(run_cnt=0; run_cnt<4; run_cnt++)
    {
        VX_CALL(vxProcessGraph(graph));

        //write_output_image_nv12_8bit(file, dst_image);

        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        printf("luma  =0x%08x\n", checksum_actual);
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        printf("chroma=0x%08x\n", checksum_actual);
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}



#define SCALE_TEST_ONE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale, crop_mode, checksum, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale "/crop=" #crop_mode, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, img_dst_size_generator_ ## scale, crop_mode, checksum))

#define ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} }))

#define ADD_DST_SIZE_NN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1_1", __VA_ARGS__, img_dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/2_1", __VA_ARGS__, img_dst_size_generator_2_1)), \
    CT_EXPAND(nextmacro(testArgName "/3_1", __VA_ARGS__, img_dst_size_generator_3_1)), \
    CT_EXPAND(nextmacro(testArgName "/4_1", __VA_ARGS__, img_dst_size_generator_4_1))

#define ADD_DST_SIZE_BILINEAR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1:1", __VA_ARGS__, img_dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/2:1", __VA_ARGS__, img_dst_size_generator_2_1)), \
    CT_EXPAND(nextmacro(testArgName "/3:1", __VA_ARGS__, img_dst_size_generator_3_1)), \
    CT_EXPAND(nextmacro(testArgName "/4:1", __VA_ARGS__, img_dst_size_generator_4_1))

#define ADD_DST_SIZE_AREA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/1:1", __VA_ARGS__, img_dst_size_generator_1_1)), \
    CT_EXPAND(nextmacro(testArgName "/87:100", __VA_ARGS__, img_dst_size_generator_87_100)), \
    CT_EXPAND(nextmacro(testArgName "/4:1", __VA_ARGS__, img_dst_size_generator_4_1))

#define ADD_SIZE_96x96(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=96x96", __VA_ARGS__, 96, 96))

#define ADD_SIZE_100x100(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=100x100", __VA_ARGS__, 100, 100))


#define PARAMETERS_ONE_OUTPUT \
    /* Crop off */ \
    /* NN downscale */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 1_1, 0, 0x41dd742d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 2_1, 0, 0xaad0c491, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 0, 0x08c84775, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 4_1, 0, 0x39393c76, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 0, 0xf4ca5ddd, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* BILINEAR downscales */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 1_1, 0, 0x41dd742d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 2_1, 0, 0x2891bf11, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 3_1, 0, 0xe5404e1a, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 4_1, 0, 0x21264e4f, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 0, 0xb4697795, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* Crop on */ \
    /* NN downscale */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 1_1, 1, 0x8b8c0ce1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 2_1, 1, 0x14dc69f0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 1, 0xe22db6f5, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 4_1, 1, 0x5d69ead7, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 1, 0x87d625f5, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    /* BILINEAR downscales */ \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 1_1, 1, 0x8b8c0ce1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 2_1, 1, 0xded3fe1d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 3_1, 1, 0x8f75b64d, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", 4_1, 1, 0x1f8aa69c, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \
    SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_read_image, "lena.bmp", SCALE_PYRAMID_ORB, 1, 0x2d376dc1, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, 0), \

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_OneOutput, Arg_OneOutput,
    PARAMETERS_ONE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[1] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;

    CT_Image src = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 0
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->width;
        rect.end_y = arg_->height;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );


        vxUnmapImagePatch(src_image, map_id);
#endif
        ASSERT_NO_FAILURE(arg_->dst_size_generator(src->width, src->height, &dst_width, &dst_height));

        if(arg_->crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_coefficients_params_init(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, sizeof(refs)/sizeof(refs[0])));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        if(arg_->crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            /* Center crop of input */
            crop.crop_start_x = src->width / 4;
            crop.crop_start_y = src->height / 4;
            crop.crop_width   = src->width / 2;
            crop.crop_height  = src->height / 2;

            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = (vx_reference)crop_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 1u));

            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        //ASSERT_NO_FAILURE(src = ct_image_from_vx_image(src_image));
        //ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

#if 0
        /*rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        vxMapImagePatch(dst_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr2,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        vxUnmapImagePatch(dst_image, map_id);*/
#endif

        //ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        //printf("0x%08x\n", checksum_actual);
        //printf("end_x = %d\n", dst_width);
        //printf("end_y = %d\n", dst_height);

        ASSERT(arg_->checksum == checksum_actual);

        //save_image_from_msc(dst_image, "output/lena_msc");

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

#define SCALE_TEST_TWO_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, img_dst_size_generator_ ## scale1, img_dst_size_generator_ ## scale2, exact))

#define PARAMETERS_TWO_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_pattern3x3, "pattern3x3", 3_1, 1_1, 0, ADD_SIZE_96x96, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_read_image, "lena.bmp", 3_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* other NN downscales */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 2_1, 1_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR upscale with integer factor */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_2, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 1_3, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* BILINEAR downscales */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 2_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 3_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */\
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", 5_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* SCALE_TEST_ONE_OUTPUT(VX_INTERPOLATION_BILINEAR,         scale_generate_random, "random", SCALE_PYRAMID_ORB, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), */ \
    /* AREA tests */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_AREA,             scale_generate_gradient_16x16, "gradient16x16", 4_1, 2_1, 0, ADD_SIZE_SMALL_SET, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_AREA,             scale_read_image, "lena.bmp", 4_1, 1_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_TwoOutput, Arg_TwoOutput,
    PARAMETERS_TWO_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image0 = 0, dst_image1 = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];
    uint32_t cnt;

    CT_Image src = NULL, dst = NULL, dst2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image0 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image1 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image0, dst_image1, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image0));
        ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        ASSERT_NO_FAILURE(dst2 = ct_image_from_vx_image(dst_image1));
        ASSERT_NO_FAILURE(scale_check(src, dst2, arg_->interpolation, arg_->border, arg_->exact_result));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image0));
        VX_CALL(vxReleaseImage(&dst_image1));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image0 == 0);
    ASSERT(dst_image1 == 0);
    ASSERT(src_image == 0);
}

#define SCALE_TEST_THREE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, img_dst_size_generator_ ## scale1, img_dst_size_generator_ ## scale2, img_dst_size_generator_ ## scale3, exact))

#define PARAMETERS_THREE_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 2_1, 4_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 1_1, 2_1, 4_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0),*/ \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 4_1, 3_1, 2_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_ThreeOutput, Arg_ThreeOutput,
    PARAMETERS_THREE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[3];
    vx_reference refs[1], output_refs[5];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);
        scale_set_output_params(&output_params[0], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);
        scale_set_output_params(&output_params[1], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[2]);
        scale_set_output_params(&output_params[2], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        for (cnt = 0; cnt < 5; cnt++)
        {
            output_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 3; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 5u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 3; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 3; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

#define SCALE_TEST_FOUR_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, scale4, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3 "/" #scale4, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, img_dst_size_generator_ ## scale1, img_dst_size_generator_ ## scale2, img_dst_size_generator_ ## scale3, img_dst_size_generator_ ## scale4, exact))

#define PARAMETERS_FOUR_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 2_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 1_1, 2_1, 3_1, 4_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FourOutput, Arg_FourOutput,
    PARAMETERS_FOUR_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[4] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 4; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 4; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 4; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

#define SCALE_TEST_FIVE_OUTPUT(interpolation, inputDataGenerator, inputDataFile, scale1, scale2, scale3, scale4, scale5, exact, nextmacro, ...) \
    CT_EXPAND(nextmacro(STR_##interpolation "/" inputDataFile "/" #scale1 "/" #scale2 "/" #scale3 "/" #scale4 "/" #scale5, __VA_ARGS__, \
            interpolation, inputDataGenerator, inputDataFile, img_dst_size_generator_ ## scale1, img_dst_size_generator_ ## scale2, img_dst_size_generator_ ## scale3, img_dst_size_generator_ ## scale4, img_dst_size_generator_ ## scale5, exact))

#define PARAMETERS_FIVE_OUTPUT \
    /* 1:1 scale */ \
    SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_BILINEAR, scale_generate_random, "random", 2_1, 2_1, 2_1, 1_1, 1_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0), \
    /*SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_AREA, scale_read_image, "lena.bmp", 1_1, 2_1, 3_1, 4_1, 2_1, 0, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_BORDERS, ARG, 0)*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_FiveOutput, Arg_FiveOutput,
    PARAMETERS_FIVE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator4(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxProcessGraph(graph));

        for (cnt = 0; cnt < 5; cnt ++)
        {
            ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image[cnt]));
            ASSERT_NO_FAILURE(scale_check(src, dst, arg_->interpolation, arg_->border, arg_->exact_result));
        }


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 5; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));


        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 5; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}
typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    int yc_mode;
} ArgFixed_nv12_u8;

#define ADD_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=1", __VA_ARGS__, 1))
#define PARAMETERS_FIX_NV12_U8 \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ADD_YC_MODE, ARG)

static uint32_t expected_cksm[] = {
0xdcdd13c0,
0x50b075db,
0xb457a7b1,
0x0afc8516,
0x93c3b775,

0xe41e5266,
0x50473b25,
0x9e38088b,
0xb7962bd5,
0x6c1a91af

};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessingChecksum_nv12_u8_fiveoutput, ArgFixed_nv12_u8, PARAMETERS_FIX_NV12_U8)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 1)
    {
        dst_height[0] = dst_height[0]/2;
    }
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
    {
        dst_height[1] = dst_height[1]/2;
    }
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 1)
    {
        dst_height[2] = dst_height[2]/2;
    }
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
    {
        dst_height[3] = dst_height[3]/2;
    }
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
    {
        dst_height[4] = dst_height[4]/2;
    }
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
   
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    /*set input */

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = arg_->yc_mode;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    VX_CALL(vxProcessGraph(graph));
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width[i];
        rect.end_y = dst_height[i];
        vx_char temp[256];
        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        sprintf(temp, "output/u8_msc_%d", i);
        save_image_from_msc(dst_image[i], temp);
        #endif
        if(arg_->yc_mode == 1)
        {
            cksm_offset = 5;
        }
        ASSERT(expected_cksm[i+cksm_offset] == checksum_actual);
    }
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}
static uint32_t expected_cksm_10bit_to_8bit[] = {
    0x8bffb56d,
    0xf633a66e
};
typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
} ArgFixed_10bit_to_8bit;
#define PARAMETERS_10BIT_TO_8BIT \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_10bit_to_8bit, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}

typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    int enable_safety_mechanism;
} ArgFixed_10bit_to_8bit_Sm;
#define ADD_SAFETY_MECHANISM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=reconfigReinitReg", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=statusRegValidate", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=configRegValidate", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=all", __VA_ARGS__, 4))
#define PARAMETERS_10BIT_TO_8BIT_SM \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ADD_SAFETY_MECHANISM, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_Safety_Mech, ArgFixed_10bit_to_8bit_Sm,
    PARAMETERS_10BIT_TO_8BIT_SM
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    tivx_vpac_msc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_safety_mech_prms_init(&safety_mechanism_params);

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);

        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        if (0U != arg_->enable_safety_mechanism)
        {
            switch (arg_->enable_safety_mechanism)
            {
                case 1:
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    break;
                case 2:
                    safety_mechanism_params.enable_status_reg_validation = 1U;
                    break;
                case 3:
                    safety_mechanism_params.enable_readback_config_registers = 1U;
                    break;
                case 4:
                    safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                    safety_mechanism_params.enable_status_reg_validation = 1U;
                    safety_mechanism_params.enable_readback_config_registers = 1U;
                    break;
                default:
                    /* Invalid value */
                    break;
            }

            safety_mechanism_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_safety_mechanism_params_t", sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params);
            safety_ref[0] = (vx_reference)safety_mechanism_obj;
            tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1);
            vxReleaseUserDataObject(&safety_mechanism_obj);
        }
        /* Set Coefficients; Must do Safety Mechanism VPAC12 and F2 */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}

#if defined(VPAC3) || defined(VPAC3L)
static uint32_t expected_cksm_msc_simul_processing[] = {
    0x59ab963a,
    0x08661321
};
typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    int isEnableSimulProcessing;
    int enable_safety_mechanism;
} ArgFixed_safety_simul;

#define ADD_SIMUL_PROCESSING_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/isEnableSimulProcessing = 0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/isEnableSimulProcessing = 1", __VA_ARGS__, 1))
#define PARAMETERS_FIX_SIMUL_PROCESSING_SAFETY \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY,ADD_SIMUL_PROCESSING_MODE,ADD_SAFETY_MECHANISM, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Simul_Processing_safety, ArgFixed_safety_simul, PARAMETERS_FIX_SIMUL_PROCESSING_SAFETY)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 

    tivx_vpac_msc_safety_mech_prms_init(&safety_mechanism_params);

    if (0U != arg_->enable_safety_mechanism)
    {
        switch (arg_->enable_safety_mechanism)
        {
            case 1:
                /* safety_mechanism=reconfigReinitReg */
                safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                break;
            case 2:
                safety_mechanism_params.enable_status_reg_validation = 1U;
                break;
            case 3:
                safety_mechanism_params.enable_readback_config_registers = 1U;
                break;
            case 4:
                safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                safety_mechanism_params.enable_status_reg_validation = 1U;
                safety_mechanism_params.enable_readback_config_registers = 1U;
                break;
            default:
                /* Invalid value */
                break;
        }

        safety_mechanism_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_safety_mechanism_params_t", sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params);
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1);
    }

    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    if(1U == arg_->enable_safety_mechanism)
    {
        /* safety_mechanism=reconfigReinitReg */
        safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
        VX_CALL(vxCopyUserDataObject(safety_mechanism_obj, 0, sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1));
        VX_CALL(vxProcessGraph(graph));
    }
    if(0U != arg_->enable_safety_mechanism)
    {
        VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));
    }
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height;
    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("luma  =0x%08x\n", checksum_actual);
    write_output_image_nv12_8bit(file, dst_image);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[0] == checksum_actual);
    rect.end_x = dst_width/2;
    rect.end_y = dst_height/2;

    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("chroma=0x%08x\n", checksum_actual);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[1] == checksum_actual);


    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
    ASSERT(safety_mechanism_obj == 0);
}
#define PARAMETERS_FIX_SIMUL_PROCESSING \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY,ADD_SIMUL_PROCESSING_MODE, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Simul_Processing, ArgFixed, PARAMETERS_FIX_SIMUL_PROCESSING)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height;  
    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("luma  =0x%08x\n", checksum_actual);
    write_output_image_nv12_8bit(file, dst_image);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[0] == checksum_actual);
    rect.end_x = dst_width/2;
    rect.end_y = dst_height/2;

    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("chroma=0x%08x\n", checksum_actual);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[1] == checksum_actual);  


    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
static uint32_t diff_bit_depth_expected_cksm[] = {
0xc2fe1eed,
0xdc821708,
0xe8bf0719,
0xfffe05c0,
0x3c3b31e0,

0x6dda0497,
0x79c9f617,
0x2bb088b8,
0x30923748,
0x5d2a3c00,

0xffff3580,
0xffff9ac0

};
typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    int input0_data_format;
    int input1_data_format;
    int in_img0_yc_mode;
    int in_img1_yc_mode;
} ArgFixed_diff_bit_depth;

#define ADD_IN_IMG0_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/in_img0_yc_mode=0", __VA_ARGS__, 0)) 

#define ADD_IN_IMG1_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/in_img1_yc_mode=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/in_img1_yc_mode=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/in_img1_yc_mode=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/in_img1_yc_mode=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/in_img1_yc_mode=4", __VA_ARGS__, 4)) 

#define ADD_VX_IN0_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12))

#define ADD_VX_IN1_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12))

#define PARAMETERS_FIX_DIFF_BIT_DEPTH \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY,ADD_VX_IN0_FORMATS, ADD_VX_IN1_FORMATS, ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE, ARG)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Diff_Bit_Depth, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    vx_char temp[256];

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && ((arg_->in_img1_yc_mode == 1) || (arg_->in_img1_yc_mode == 2))))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
        }
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else if(arg_->in_img1_yc_mode == 1)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
            else if(arg_->in_img1_yc_mode == 2)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/chroma_nv21.raw", 1);
            }
            else if(arg_->in_img1_yc_mode == 3)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/yuv422_Bower_1920x1080_chroma8.raw", 1);
            }
            else if(arg_->in_img1_yc_mode == 4)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/color_checker_vu_1920x1080.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
        }
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else if(arg_->in_img1_yc_mode == 1)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
            else if(arg_->in_img1_yc_mode == 2)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/chroma_nv21_16b.raw", 2);
            }
            else if(arg_->in_img1_yc_mode == 3)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/yuv422_Bower_1920x1080_Chroma_16b.raw", 2);
            }
            else if(arg_->in_img1_yc_mode == 4)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/color_checker_vu_1920x1080_16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    VX_CALL(vxProcessGraph(graph));
    for(i = 0; i < 10; i ++)
    {
        int dst_img_fmt;
        vx_char temp[256];
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width[i];
        rect.end_y = dst_height[i];        
        if(i < 5)
        {
            dst_img_fmt = arg_->input0_data_format;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        }
        else
        {
            dst_img_fmt = arg_->input1_data_format;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        }
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        
        if(dst_img_fmt == VX_DF_IMAGE_U8)
        {
            if(arg_->in_img0_yc_mode == 0)
            {
                if(arg_->in_img1_yc_mode == 0)
                {
                    ASSERT(diff_bit_depth_expected_cksm[0] == checksum_actual);
                }
                else
                {
                    if(i < 5)
                    {
                        ASSERT(diff_bit_depth_expected_cksm[0] == checksum_actual);
                    }
                    else
                    {
                        if(arg_->in_img1_yc_mode == 1)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[1] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 2)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[2] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 3)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[3] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 4)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[4] == checksum_actual);
                        } 
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
                    ASSERT(diff_bit_depth_expected_cksm[5] == checksum_actual);
                }
                else
                {
                    if(i < 5)
                    {
                        ASSERT(diff_bit_depth_expected_cksm[5] == checksum_actual);
                    }
                    else
                    {
                        if(arg_->in_img1_yc_mode == 1)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[6] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 2)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[7] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 3)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[8] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 4)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[9] == checksum_actual);
                        } 
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
                    ASSERT(diff_bit_depth_expected_cksm[10] == checksum_actual);
                }
                else
                {
                    if(i < 5)
                    {
                        ASSERT(diff_bit_depth_expected_cksm[10] == checksum_actual);
                    }
                    else
                    {
                        if(arg_->in_img1_yc_mode == 1)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[11] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 2)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[11] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 3)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[10] == checksum_actual);
                        }
                        else if(arg_->in_img1_yc_mode == 4)
                        {
                            ASSERT(diff_bit_depth_expected_cksm[10] == checksum_actual);
                        } 
                    }
                }
            }
        }
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_UYVY, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    char *output_file_name = "output/msc_out_uyvy.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_reference refs[5] = {0};
    vx_reference output_refs[2] = {0};
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params;
    vx_user_data_object coeff_obj, output_obj;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    tivx_vpac_msc_output_params_init(&output_params);
    output_params.filter_mode = 0;
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readUYVYInput(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    tivx_vpac_msc_coefficients_params_init(&coeffs, interpolation);
    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    /* Set Output params */
    ASSERT_VX_OBJECT(output_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_output_params_t",
        sizeof(tivx_vpac_msc_output_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(output_obj, 0,
        sizeof(tivx_vpac_msc_output_params_t), &output_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    output_refs[0] = (vx_reference)output_obj;
    
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
        output_refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&output_obj));

    VX_CALL(vxProcessGraph(graph));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height;

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("yuv422i checksum = 0x%08x\n", checksum_actual);
    write_uyvy_output_image(file, dst_image);
    #endif
    ASSERT((uint32_t)0xc7744b86 == checksum_actual);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testErrorInterrupts, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_refs[5] = {0};
    vx_event_t event = { 0 };
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    /* enable events generation */
    vxEnableEvents(context);
    /* Register an error event for the node */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Configure simul_processing using SET_INPUT_PARAMS */
    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = 1;
    sc_input_params.enable_error_events = 0;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj));

    /* Enable watchdog timer error events using ENABLE_ERROR_EVENTS */
    tivx_vpac_msc_input_params_init(&sc_input_params);
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    if((strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0) || (strncmp(TIVX_TARGET_VPAC2_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0))
    #else
    if(strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0)
    #endif
    {
        sc_input_params.enable_error_events = TIVX_VPAC_MSC0_ALL_ERR_EVENTS;
    }
    else
    {
        sc_input_params.enable_error_events = TIVX_VPAC_MSC1_ALL_ERR_EVENTS;
    }
    /* Configure WDTimer with 128K cycle timeout */
    sc_input_params.wdtimer_timeout_cycles = TIVX_VPAC_MSC_WDTIMER_128K_CYCLES;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    
    /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
     * and prints an error message saying that there are no events in the queue */
    vxWaitEvent(context, &event, vx_true_e);

    if(event.app_value == MSC_NODE_ERROR_EVENT)
    {
        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_refs[0] = (vx_reference)scalar_err_stat_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        printf("MSC error event triggered\n");
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        ASSERT(scalar_out_err_stat == 0);
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 
    }
    else
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;  
        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("luma  =0x%08x\n", checksum_actual);
        write_output_image_nv12_8bit(file, dst_image);
        #endif
        ASSERT(expected_cksm_msc_simul_processing[0] == checksum_actual);
        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("chroma=0x%08x\n", checksum_actual);
        #endif
        ASSERT(expected_cksm_msc_simul_processing[1] == checksum_actual);  
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
#endif
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testErrorInterrupts_j721e, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_refs[5] = {0};
    vx_event_t event = { 0 };
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    /* enable events generation */
    vxEnableEvents(context);
    /* Register an error event for the node */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        /* Disable all events by default */
        sc_input_params.enable_error_events = 0U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(vxProcessGraph(graph));
    
    /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
     * and prints an error message saying that there are no events in the queue */
    vxWaitEvent(context, &event, vx_true_e);

    if(event.app_value == MSC_NODE_ERROR_EVENT)
    {
        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_refs[0] = (vx_reference)scalar_err_stat_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        ASSERT(scalar_out_err_stat == 0);
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 
    }
    else
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("luma  =0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("chroma=0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }
    

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxDisableEvents(context));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
static uint32_t luma_chroma_cropAlign_expected_cksm[] = {
0xeea1f3cb,
0xda4b57fe,

0x8ab49e10,
0xe34b4b1a,

0x6eff147a,
0x0a9fa0ff
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_LumaChroma_CropAlign, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[3];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[3];
    vx_reference refs[5] = {0};
    int crop_start_y_values[3] = {20, 80, 148};
    int cnt = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    for (i = 0; i < 3; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    for (i = 0; i < 3; i++)
    {
        crop_refs[i] = NULL;
    }

    /* Set crop parameters */
    
    for (i = 0; i < 3; i++) 
    {
        crop[i].crop_start_x = 150;
        crop[i].crop_width = dst_width;
        crop[i].crop_height = dst_height;
        crop[i].crop_start_y = crop_start_y_values[i];

        ASSERT_VX_OBJECT(crop_obj[i] = vxCreateUserDataObject(context, 
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL), 
                VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(crop_obj[i], 0, 
            sizeof(tivx_vpac_msc_crop_params_t), &crop[i], VX_WRITE_ONLY, 
            VX_MEMORY_TYPE_HOST));
        crop_refs[i] = (vx_reference)crop_obj[i];
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
        crop_refs, 3u));

    VX_CALL(vxProcessGraph(graph));
    for (i = 0; i < 3; i++) 
    {
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(output_files[i], dst_image[i]);
        #endif
        
        rect.start_x = 0;
        rect.start_y = crop_start_y_values[i];
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Luma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(luma_chroma_cropAlign_expected_cksm[cnt] == checksum_actual);

        rect.end_x = dst_width / 2;
        rect.end_y = dst_height / 2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Chroma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(luma_chroma_cropAlign_expected_cksm[cnt+1] == checksum_actual);
        cnt = cnt + 2;
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&crop_obj[i]));
        ASSERT(dst_image[i] == 0 && crop_obj[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

    tivxHwaUnLoadKernels(context); 
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_UYVY_input_Y_output, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image[TEST_NUM_OUTPUTS_YUV422I] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    vx_char temp[256];
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_reference refs[5] = {0};
    vx_reference output_refs[TEST_NUM_OUTPUTS_YUV422I] = {0};
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[TEST_NUM_OUTPUTS_YUV422I];
    vx_user_data_object coeff_obj, output_obj[TEST_NUM_OUTPUTS_YUV422I];
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++) 
    {
        tivx_vpac_msc_output_params_init(&output_params[i]);
        output_params[i].filter_mode = 0;
        output_refs[i] = NULL;
    }
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readUYVYInput(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    tivx_vpac_msc_coefficients_params_init(&coeffs, interpolation);
    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    /* Set Output params */
    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++)
    {
        ASSERT_VX_OBJECT(output_obj[i] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t",
            sizeof(tivx_vpac_msc_output_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(output_obj[i], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[i], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        output_refs[i] = (vx_reference)output_obj[i];
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, TEST_NUM_OUTPUTS_YUV422I));
    VX_CALL(vxProcessGraph(graph));
    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++) 
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("yuv422i checksum = 0x%08x\n", checksum_actual);
        sprintf(temp, "output/422i_input_y_output_%d", i);
        save_image_from_msc(dst_image[i], temp);
        #endif
        ASSERT((uint32_t)0x1488f24f == checksum_actual);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&output_obj[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(src_image == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Perf, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    rect.start_x = 0;
    rect.start_y = 0;

    for(run_cnt=0; run_cnt<4; run_cnt++)
    {
        VX_CALL(vxProcessGraph(graph));
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(file, dst_image);
        #endif

        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("luma  =0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("chroma=0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
static uint32_t expected_psa_values[2][5] = {
{0x943ede1b, 0xee91e2d7, 0xa32b6ca5,0x9557f41b,0x9557f41b},
{0x487eeeba, 0xf5ddd4ba, 0xde3f842f,0x59fd8828,0x59fd8828}
};

static uint32_t psa_sign_expected_cksm[] = {
0xeea1f3cb,
0xda4b57fe,

0x8ab49e10,
0xe34b4b1a,

0x6eff147a,
0x0a9fa0ff,

0x490dc0d8,
0x3c77bd36,

0x490dc0d8,
0x3c77bd36
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessingGetPsaValues, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i,j;
    vx_image src_image = 0, dst_image[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[TEST_MSC_TOTAL_OUTPUTS];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[TEST_MSC_TOTAL_OUTPUTS];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[TEST_MSC_TOTAL_OUTPUTS] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv", "output/msc_out3.yuv", "output/msc_out4.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[TEST_MSC_TOTAL_OUTPUTS][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[TEST_MSC_TOTAL_OUTPUTS];
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    int crop_start_y_values[TEST_MSC_TOTAL_OUTPUTS] = {20, 80, 148,0,0};
    int cnt = 0;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_user_data_object sc_input_params_obj;
    uint32_t psa_values[TEST_NUM_PSA_NODE_INSTANCE][TEST_MSC_MAX_OUTPUTS] = {0};
    vx_reference psa_refs [TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    input_timestamp = SET_TIME_STAMP_VALUE;
    VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
    &input_timestamp, sizeof(input_timestamp)));
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.enable_psa = 1u;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj));

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
    {
        crop_refs[i] = NULL;
    }

    /* Set crop parameters */
    
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        crop[i].crop_start_x = 150;
        crop[i].crop_width = dst_width;
        crop[i].crop_height = dst_height;
        crop[i].crop_start_y = crop_start_y_values[i];

        ASSERT_VX_OBJECT(crop_obj[i] = vxCreateUserDataObject(context, 
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL), 
                VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(crop_obj[i], 0, 
            sizeof(tivx_vpac_msc_crop_params_t), &crop[i], VX_WRITE_ONLY, 
            VX_MEMORY_TYPE_HOST));
        crop_refs[i] = (vx_reference)crop_obj[i];
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
        crop_refs, 5u));

    VX_CALL(vxProcessGraph(graph));

    ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);

    psa_refs[0] = (vx_reference) psa_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
        psa_refs, 1u));
    
    VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t), 
        &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxReleaseUserDataObject(&psa_obj));

    /*Compare PSA signature values*/
    for(i = 0; i < TEST_NUM_PSA_NODE_INSTANCE; i ++)
    {
        for(j = 0; j < TEST_MSC_TOTAL_OUTPUTS; j ++)
        {
            if((0u == (strcmp(arg_->target_string, "VPAC_MSC1"))) ||
               (0u == (strcmp(arg_->target_string, "VPAC2_MSC1"))))
            {
                ASSERT(psa_status.psa_values[i][j] == expected_psa_values[i][j]);
            }
            else
            {
                ASSERT(psa_status.psa_values[i][9-j] == expected_psa_values[i][j]);
            }
        }
    }
    #if defined TEST_PRINT_TIME_STAMP_VALUES
    printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
    #endif
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(output_files[i], dst_image[i]);
        #endif
        
        rect.start_x = 0;
        rect.start_y = crop_start_y_values[i];
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Luma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(psa_sign_expected_cksm[cnt] == checksum_actual);

        rect.end_x = dst_width / 2;
        rect.end_y = dst_height / 2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Chroma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(psa_sign_expected_cksm[cnt+1] == checksum_actual);
        cnt = cnt + 2;
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&crop_obj[i]));
        ASSERT(dst_image[i] == 0 && crop_obj[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

    tivxHwaUnLoadKernels(context);
}

#ifndef VPAC3L
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessingSl2Firewall, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i,j;
    vx_image src_image = 0, dst_image[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[TEST_MSC_TOTAL_OUTPUTS];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[TEST_MSC_TOTAL_OUTPUTS];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[TEST_MSC_TOTAL_OUTPUTS] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv", "output/msc_out3.yuv", "output/msc_out4.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[TEST_MSC_TOTAL_OUTPUTS][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[TEST_MSC_TOTAL_OUTPUTS];
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    int crop_start_y_values[TEST_MSC_TOTAL_OUTPUTS] = {20, 80, 148,0,0};
    int cnt = 0;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_user_data_object sc_input_params_obj;
    uint32_t psa_values[TEST_NUM_PSA_NODE_INSTANCE][TEST_MSC_MAX_OUTPUTS] = {0};
    vx_reference psa_refs [TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;
    vx_status status;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

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
    }

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    input_timestamp = SET_TIME_STAMP_VALUE;
    VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
    &input_timestamp, sizeof(input_timestamp)));
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.enable_psa = 1u;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj));

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
    {
        crop_refs[i] = NULL;
    }

    /* Set crop parameters */

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
    {
        crop[i].crop_start_x = 150;
        crop[i].crop_width = dst_width;
        crop[i].crop_height = dst_height;
        crop[i].crop_start_y = crop_start_y_values[i];

        ASSERT_VX_OBJECT(crop_obj[i] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(crop_obj[i], 0,
            sizeof(tivx_vpac_msc_crop_params_t), &crop[i], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
        crop_refs[i] = (vx_reference)crop_obj[i];
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
        crop_refs, 5u));

    VX_CALL(vxProcessGraph(graph));

    ASSERT_VX_OBJECT(psa_obj =
        vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);

    psa_refs[0] = (vx_reference) psa_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
        psa_refs, 1u));

    VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t),
        &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxReleaseUserDataObject(&psa_obj));

    /*Compare PSA signature values*/
    for(i = 0; i < TEST_NUM_PSA_NODE_INSTANCE; i ++)
    {
        for(j = 0; j < TEST_MSC_TOTAL_OUTPUTS; j ++)
        {
            if((0u == (strcmp(arg_->target_string, "VPAC_MSC1"))) ||
               (0u == (strcmp(arg_->target_string, "VPAC2_MSC1"))))
            {
                ASSERT(psa_status.psa_values[i][j] == expected_psa_values[i][j]);
            }
            else
            {
                ASSERT(psa_status.psa_values[i][9-j] == expected_psa_values[i][j]);
            }
        }
    }
    #if defined TEST_PRINT_TIME_STAMP_VALUES
    printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
    #endif
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
    {
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(output_files[i], dst_image[i]);
        #endif

        rect.start_x = 0;
        rect.start_y = crop_start_y_values[i];
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Luma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(psa_sign_expected_cksm[cnt] == checksum_actual);

        rect.end_x = dst_width / 2;
        rect.end_y = dst_height / 2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Chroma = 0x%08x\n", i, checksum_actual);
        #endif
        ASSERT(psa_sign_expected_cksm[cnt+1] == checksum_actual);
        cnt = cnt + 2;
    }

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&crop_obj[i]));
        ASSERT(dst_image[i] == 0 && crop_obj[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

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
#endif /* VPAC3L */

#if defined(VPAC3) || defined(VPAC3L)
typedef struct {
    const char* testName;
    char* target_string, *target_string_2;
    int dummy;
    int isEnableSimulProcessing;
} ArgFixed_rgb;

#define PARAMETERS_FIX_RGB \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_MULTI_INST, ADD_DUMMY, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_RGB_Plane_Split, ArgFixed_rgb, PARAMETERS_FIX_RGB)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 720, dst_width = w / 2, dst_height = h / 2, run_cnt;
    vx_image src_image_R = 0, src_image_G = 0, src_image_B = 0;
    vx_image dst_image_R[TEST_MSC_TOTAL_OUTPUTS] = {0}, dst_image_G[TEST_MSC_TOTAL_OUTPUTS] = {0}, dst_image_B[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node_rg = 0, node_b = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_rectangle_t rect;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_char temp[256];

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image_R = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image_G = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image_B = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ct_read_raw_image_to_vx_image(src_image_R, "psdkra/app_single_cam/1280x720_r8.raw", 1);
    ct_read_raw_image_to_vx_image(src_image_G, "psdkra/app_single_cam/1280x720_g8.raw", 1);
    ct_read_raw_image_to_vx_image(src_image_B, "psdkra/app_single_cam/1280x720_b8.raw", 1);

    for (run_cnt = 0; run_cnt < TEST_MSC_TOTAL_OUTPUTS; run_cnt++) 
    {
        ASSERT_VX_OBJECT(dst_image_R[run_cnt] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image_G[run_cnt] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image_B[run_cnt] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* First pass: Process R and G planes with tivxVpacMscScaleNode2 */
    ASSERT_VX_OBJECT(node_rg = tivxVpacMscScaleNode2(graph,
        src_image_R, dst_image_R[0], dst_image_R[1], dst_image_R[2], dst_image_R[3], dst_image_R[4],
        src_image_G, dst_image_G[0], dst_image_G[1], dst_image_G[2], dst_image_G[3], dst_image_G[4]), VX_TYPE_NODE);

    /* Second pass: Process B plane with tivxVpacMscScaleNode */
    ASSERT_VX_OBJECT(node_b = tivxVpacMscScaleNode(graph, src_image_B, 
        dst_image_B[0], dst_image_B[1], dst_image_B[2], dst_image_B[3], dst_image_B[4]), VX_TYPE_NODE);
    
    ASSERT_NO_FAILURE(vxSetNodeTarget(node_rg, VX_TARGET_STRING, arg_->target_string));
    if (NULL != arg_->target_string_2)
    {
        ASSERT_NO_FAILURE(vxSetNodeTarget(node_b, VX_TARGET_STRING, arg_->target_string_2));
    }
    else
    {
        ASSERT_NO_FAILURE(vxSetNodeTarget(node_b, VX_TARGET_STRING, arg_->target_string));
    }

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.is_enable_simul_processing = 1;
        sc_input_params.in_img0_yc_mode = 0;
        sc_input_params.in_img1_yc_mode = 0;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node_rg, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 

    VX_CALL(vxProcessGraph(graph));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height; 
    
    for (run_cnt = 0; run_cnt < TEST_MSC_TOTAL_OUTPUTS; run_cnt++) 
    {
        checksum_actual = tivx_utils_simple_image_checksum(dst_image_R[run_cnt], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf(" R plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_R_%d", run_cnt);
        save_image_from_msc(dst_image_R[run_cnt], temp);
        #endif
        ASSERT((uint32_t)0xa11db1be == checksum_actual);
        checksum_actual = tivx_utils_simple_image_checksum(dst_image_G[run_cnt], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf(" G plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_G_%d", run_cnt);
        save_image_from_msc(dst_image_G[run_cnt], temp);
        #endif 
        ASSERT((uint32_t)0x17649566 == checksum_actual);

        checksum_actual = tivx_utils_simple_image_checksum(dst_image_B[run_cnt], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf(" B plane checksum_actual = 0x%x\n", checksum_actual);
        sprintf(temp, "output/dst_image_B_%d", run_cnt);
        save_image_from_msc(dst_image_B[run_cnt], temp);
        #endif
        ASSERT((uint32_t)0xe331d4f7 == checksum_actual);
    } 

    VX_CALL(vxReleaseNode(&node_rg));
    VX_CALL(vxReleaseNode(&node_b));
    VX_CALL(vxReleaseGraph(&graph));
    for (run_cnt = 0; run_cnt < TEST_MSC_TOTAL_OUTPUTS; run_cnt++) 
    {
        VX_CALL(vxReleaseImage(&dst_image_R[run_cnt]));
        VX_CALL(vxReleaseImage(&dst_image_G[run_cnt]));
        VX_CALL(vxReleaseImage(&dst_image_B[run_cnt]));
    }
    VX_CALL(vxReleaseImage(&src_image_R));
    VX_CALL(vxReleaseImage(&src_image_G));
    VX_CALL(vxReleaseImage(&src_image_B));

    tivxHwaUnLoadKernels(context);

    ASSERT(node_rg == 0);
    ASSERT(node_b == 0);
    ASSERT(graph == 0);
    ASSERT(src_image_R == 0);
    ASSERT(src_image_G == 0);
    ASSERT(src_image_B == 0); 

}
static uint32_t crop_fix_expected_cksm[] = {
    0xeea1f3cb,
    0xda4b57fe,

    0x8ab49e10,
    0xe34b4b1a,

    0x6eff147a,
    0x0a9fa0ff
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Simul_Processing_crop_fix, ArgFixed, PARAMETERS_FIX_SIMUL_PROCESSING)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = 600, dst_height = 772;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[3], output_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[3];
    tivx_vpac_msc_output_params_t output_params[3];
    vx_reference refs[5] = {0}, output_refs[3] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_reference crop_refs[3];
    int crop_start_y_values[3] = {20, 80, 148}, cnt = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    for (i = 0; i < 3; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        tivx_vpac_msc_output_params_init(&output_params[i]);
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    for (i = 0; i < 3; i++)
    {
        crop_refs[i] = NULL;
        output_refs[i] = NULL;
    }
    /* Set Output params */
    for (i = 0; i < 3; i++)
    {
        ASSERT_VX_OBJECT(output_obj[i] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t",
            sizeof(tivx_vpac_msc_output_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(output_obj[i], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[i], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        output_refs[i] = (vx_reference)output_obj[i];
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
        output_refs, 3u));
    
    for (i = 0; i < 3; i++) 
    {
        crop[i].crop_start_x = 150;
        crop[i].crop_width = dst_width;
        crop[i].crop_height = dst_height;
        crop[i].crop_start_y = crop_start_y_values[i];

        ASSERT_VX_OBJECT(crop_obj[i] = vxCreateUserDataObject(context, 
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL), 
                VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(crop_obj[i], 0, 
            sizeof(tivx_vpac_msc_crop_params_t), &crop[i], VX_WRITE_ONLY, 
            VX_MEMORY_TYPE_HOST));
        crop_refs[i] = (vx_reference)crop_obj[i];
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
        crop_refs, 3u));
    
    VX_CALL(vxProcessGraph(graph));

    for (i = 0; i < 3; i++) 
    {
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(output_files[i], dst_image[i]);
        #endif
        
        rect.start_x = 0;
        rect.start_y = crop_start_y_values[i];
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Luma = 0x%08x\n", i, checksum_actual);
        #endif 
        ASSERT(crop_fix_expected_cksm[cnt] == checksum_actual);

        rect.end_x = dst_width / 2;
        rect.end_y = dst_height / 2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("Output %d Chroma = 0x%08x\n", i, checksum_actual);
        #endif
        if(crop_fix_expected_cksm[cnt+1] == checksum_actual)
        cnt = cnt + 2;
    }


    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&crop_obj[i]));
        VX_CALL(vxReleaseUserDataObject(&output_obj[i]));
        ASSERT(dst_image[i] == 0 && crop_obj[i] == 0 && output_obj[i] == 0);
    }

    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(src_image == 0);
}
#endif

typedef struct {
    const char* testName;
    char* target_string, *target_string_2;
    int dummy;
} ArgFixed_multi;
#define PARAMETERS_MULTI \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_MULTI_INST, ADD_DUMMY, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_multi, ArgFixed_multi,
    PARAMETERS_MULTI
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width = 2592 , dst_height = 1944, idx,i;
    vx_image y12[TEST_NUM_NODE_INSTANCE] = {0}, dst_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        if (NULL != arg_->target_string)
        {
            ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
        }
        if (NULL != arg_->target_string_2)
        {
            ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
        }
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {
                ASSERT_VX_OBJECT(y12[i] = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
                ct_read_raw_image_to_vx_image(y12[i], "psdkra/app_single_cam/x5b_rgb.bin", 2);
                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
                
                ASSERT_VX_OBJECT(node[i] = tivxVpacMscScaleNode(graph, y12[i],
                        dst_image[i], NULL, NULL, NULL, NULL), VX_TYPE_NODE);
                VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
    
            }
        }
        VX_CALL(vxVerifyGraph(graph));
        
        VX_CALL(vxProcessGraph(graph));        

         for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                ASSERT(node[i] == 0);
            }
        }
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( (NULL != arg_->target_string) ||
                 (NULL != arg_->target_string_2) )
            {
                VX_CALL(vxReleaseImage(&y12[i]));
                ASSERT(y12[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < TEST_NUM_NODE_INSTANCE; cnt ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[cnt]));
        ASSERT(dst_image[cnt] == 0);
    }
}


static uint32_t expected_cksm_10bit_to_8bit_single_phase_coeff[] = {
    0x47879cc5,
    0x7026d555,
    0x95ae66f8
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_10bit_8bit_with_single_phase_coeff, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[3] , dst_height[3], idx;
    vx_image y12 = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[3], crop_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[3];
    tivx_vpac_msc_crop_params_t crop[3];
    vx_reference refs[5], output_refs[3], crop_refs[3];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1944;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1944/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        dst_width[2] = 2592/4;
        dst_height[2] = 1944/4;
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[2]);
        

        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 6u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 1u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 6u;
        output_params[1].single_phase.horz_coef_src = 1u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 1u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        output_params[2].filter_mode = 0u;
        output_params[2].coef_shift = 6u;
        output_params[2].single_phase.horz_coef_src = 0u;
        output_params[2].single_phase.horz_coef_sel = 0u;
        output_params[2].single_phase.vert_coef_src = 0u;
        output_params[2].single_phase.vert_coef_sel = 1u;
        output_params[2].multi_phase.phase_mode = 0u;
        output_params[2].multi_phase.horz_coef_sel = 0u;
        output_params[2].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 0;
        coeffs.single_phase[1][idx ++] = 0;
        coeffs.single_phase[1][idx ++] = 256;
        coeffs.single_phase[1][idx ++] = 0;
        coeffs.single_phase[1][idx ++] = 0;
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeffs.multi_phase[0][idx++] = 16;
            coeffs.multi_phase[0][idx++] = 64;
            coeffs.multi_phase[0][idx++] = 96;
            coeffs.multi_phase[0][idx++] = 64;
            coeffs.multi_phase[0][idx++] = 16;
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 3; cnt++)
        {
            output_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 3; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 3u));
        
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 3; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_init_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_single_phase_coeff[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 3; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
static uint32_t expected_cksm_10bit_to_8bit_non_auto_compute[] = {
    0x8bffb56d,
    0xf633a66e
};

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_10bit_8bit_non_auto_compute, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;
        output_params[0].offset_x = 0u;
        output_params[0].multi_phase.init_phase_y = 0u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;
        output_params[1].offset_y = 0u;
        output_params[1].multi_phase.init_phase_x = 0u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_init_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_non_auto_compute[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative,  testGraphProcessing_Nv12Input_InvalidOutputFormat, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1] = {0};
    // vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;

    // Ensure target is enabled
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    // Intentionally create image with RGB format to mismatch NV12 input data
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    // Input file is still NV12 → will mismatch image format
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    // Setup scaling coefficients
    img_scale_set_coeff(&coeffs, interpolation);

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    // Create coefficient user data object
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;

    // This test expects tivxNodeSendCommand to fail because of image format mismatch
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF, refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    

   
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
#define ADD_SET_TARGET_PARAMETERS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))
#define ADD_SIZE_SMALL_SET_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=16x16", __VA_ARGS__, 16, 16))
#define ADD_SET_TARGET_PARAMETERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))
#define ADD_VX_BORDERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} }))
#define PARAMETERS_TWO_OUTPUT_TEST \
    /* 1:1 scale */ \
    SCALE_TEST_TWO_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 0, ADD_SIZE_SMALL_SET_TEST, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_BORDERS_TEST, ARG, 0), \

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_TwoOutput_diff_outimg, Arg_TwoOutput,
    PARAMETERS_TWO_OUTPUT_TEST
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image0 = 0, dst_image1 = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];
    uint32_t cnt;

    CT_Image src = NULL, dst2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image0 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image1 = vxCreateImage(context, dst_width, dst_height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image0, dst_image1, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

            ASSERT_NO_FAILURE(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image0));
        VX_CALL(vxReleaseImage(&dst_image1));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image0 == 0);
    ASSERT(dst_image1 == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_TwoOutput_outputDim_greaterThanInput, Arg_TwoOutput,
    PARAMETERS_TWO_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int dst_width = 256, dst_height = 256, w = 16, h = 16;
    vx_image src_image = 0, dst_image0 = 0, dst_image1 = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];
    uint32_t cnt;

    CT_Image src = NULL, dst = NULL, dst2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image0 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image1 = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image0, dst_image1, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        ASSERT_NO_FAILURE(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image0));
        VX_CALL(vxReleaseImage(&dst_image1));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image0 == 0);
    ASSERT(dst_image1 == 0);
    ASSERT(src_image == 0);
}
#define ADD_SET_TARGET_PARAMETERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))
#define ADD_VX_BORDERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} }))
#define PARAMETERS_THREE_OUTPUT_TEST \
    /* 1:1 scale */ \
    SCALE_TEST_THREE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_BORDERS_TEST, ARG, 0)
/*tets case for size<_>TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS*/
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_ThreeOutput_incorrectOutputParamsSize, Arg_ThreeOutput,
    PARAMETERS_THREE_OUTPUT_TEST
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[3];
    vx_reference refs[1], output_refs[5];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);
        scale_set_output_params(&output_params[0], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);
        scale_set_output_params(&output_params[1], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[2]);
        scale_set_output_params(&output_params[2], arg_->interpolation, src->width, src->height, dst_width, dst_height);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        for (cnt = 0; cnt < 5; cnt++)
        {
            output_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 3; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t)+1, NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 5u));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 3; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_ThreeOutput_outputHeight_greaterThanInput, Arg_ThreeOutput,
    PARAMETERS_THREE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 256, dst_height = 256;int w = 256, h = 16;
    vx_image src_image = 0, dst_image[3] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[3];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[3];
    vx_reference refs[1], output_refs[5];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[0]= vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[1]= vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image[2]= vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);


        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        for (cnt = 0; cnt < 5; cnt++)
        {
            output_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 3; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 5u));

        ASSERT_NO_FAILURE(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 3; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 3; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_Five_UYVY_outputs, Arg_ThreeOutput,
    PARAMETERS_THREE_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image src_image = 0, dst_image[5] = {0};
    vx_user_data_object coeff_obj = 0, output_obj[5] = {0};
    vx_reference refs[1], output_refs[5];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[5];
    int dst_width = 0, dst_height = 0;
    CT_Image src = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));

    for (int i = 0; i < 5; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[i]);
        scale_set_output_params(&output_params[i], arg_->interpolation, src->width, src->height, dst_width, dst_height);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));
    img_scale_set_coeff(&coeffs, arg_->interpolation);

    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t", sizeof(tivx_vpac_msc_coefficients_t), NULL), VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF, refs, 1u));

    for (int i = 0; i < 5; i++)
    {
        ASSERT_VX_OBJECT(output_obj[i] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t", sizeof(tivx_vpac_msc_output_params_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(output_obj[i], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[i], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        output_refs[i] = (vx_reference)output_obj[i];
    }

    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
        output_refs, 5u));

    //  Expect failure here due to cnt > 3 outputs in YUYV/UYVY
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    // Cleanup
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&src_image));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    for (int i = 0; i < 5; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&output_obj[i]));
    }

    tivxHwaUnLoadKernels(context);

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(src_image == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_InvalidCommand_FourOutputFormats, Arg_FourOutput,
    PARAMETERS_FOUR_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[4] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    // Ensure target is enabled before proceeding
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        // Load source image from file
        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        // Create destination images with mixed formats to increase test coverage
        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        // Create a graph and add MSC scale node with 4 outputs
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        // Set border attributes for the node
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        // Prepare interpolation coefficients
        img_scale_set_coeff(&coeffs, arg_->interpolation);

        // Verify the graph before execution
        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        // Create a user data object for coefficients
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        // Copy coefficients to the user data object
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;

        // Intentionally send an invalid/unsupported command to test negative scenario
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        // Process the graph even after the command failure
        ASSERT_NO_FAILURE(vxProcessGraph(graph));

        // Release all OpenVX objects to clean up
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        for (cnt = 0; cnt < 4; cnt++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    // Assert that all images are properly released
    for (cnt = 0; cnt < 4; cnt++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_FourOutput_Invalidformats, Arg_FourOutput,
    PARAMETERS_FOUR_OUTPUT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int w = 1280, h = 512;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[4] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string)); // Ensure target is enabled

    {
        tivxHwaLoadKernels(context); // Load hardware acceleration kernels
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        // Generate input image (but intentionally creating src_image directly to test format handling)
        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

        // Create output images with mixed supported formats for coverage
        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        // Create the graph and node
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        // Set border attributes for the node
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation); // Fill coefficient struct

        ASSERT_NO_FAILURE(vxVerifyGraph(graph)); // Verify the graph is valid

        // Create a user data object to hold coefficients
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        // Copy the coefficient data into the object
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;

        // Intentionally expecting failure when sending the command (negative test)
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        // Process the graph (should still complete despite invalid command)
        ASSERT_NO_FAILURE(vxProcessGraph(graph));

        // Cleanup resources
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        for (cnt = 0; cnt < 4; cnt++) {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }

        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context); // Unload kernels
    }

    // Ensure all objects are released
    for (cnt = 0; cnt < 4; cnt++) {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}
#define PARAMETERS_FIVE_OUTPUT_TEST \
    /* 1:1 scale */ \
    SCALE_TEST_FIVE_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 2_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_BORDERS_TEST, ARG, 0)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_FiveOutput_TriggerCntGt3Branch, Arg_FiveOutput,
    PARAMETERS_FIVE_OUTPUT_TEST
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL, dst = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        // Create input image with UYVY format
        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src->width, src->height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator4(src->width, src->height, &dst_width, &dst_height));
        // Create 5th output as UYVY to trigger the condition
        ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        // Expect failure due to invalid parameters (branch triggers error)
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxProcessGraph(graph));

        for (cnt = 0; cnt < 5; cnt ++)
        {
            if (dst_image[cnt])
            {
                VX_CALL(vxReleaseImage(&dst_image[cnt]));
            }
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context);
    }

    for (cnt = 0; cnt < 5; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}
#define ADD_YC_MODE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=1", __VA_ARGS__, 1)),\
    CT_EXPAND(nextmacro(testArgName "/yc_mode=2", __VA_ARGS__, 2))
#define PARAMETERS_FIX_NV12_U8_TEST \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ADD_YC_MODE_TEST, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessingChecksum_nv12_u8_fiveoutput_ycmode_2, ArgFixed_nv12_u8, PARAMETERS_FIX_NV12_U8_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 1)
    {
        dst_height[0] = dst_height[0]/2;
    }
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
    {
        dst_height[1] = dst_height[1]/2;
    }
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 1)
    {
        dst_height[2] = dst_height[2]/2;
    }
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
    {
        dst_height[3] = dst_height[3]/2;
    }
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
    {
        dst_height[4] = dst_height[4]/2;
    }
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
   
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    /*set input */

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = 2;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}

#if defined(VPAC3) || defined(VPAC3L)
/*test case for in_img0_yc_mode == TIVX_VPAC_MSC_MODE_CHROMA_ONLY*/
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessingChecksum_nv12_u8_fiveoutput_CHROMA, ArgFixed_nv12_u8, PARAMETERS_FIX_NV12_U8_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 1)
    {
        dst_height[0] = dst_height[0]/2;
    }
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
    {
        dst_height[1] = dst_height[1]/2;
    }
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 1)
    {
        dst_height[2] = dst_height[2]/2;
    }
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
    {
        dst_height[3] = dst_height[3]/2;
    }
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
    {
        dst_height[4] = dst_height[4]/2;
    }
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
   
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    /*set input */

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = arg_->yc_mode;
    sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
        ASSERT_NO_FAILURE(
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}
#endif
/*testcase for invalid size in control call   TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS*/
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessingChecksum_nv12_u8_fiveoutput_INVALID_size, ArgFixed_nv12_u8, PARAMETERS_FIX_NV12_U8_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 0)
    {
        dst_height[0] = dst_height[0]/2;
    }
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
    {
        dst_height[1] = dst_height[1]/2;
    }
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 2)
    {
        dst_height[2] = dst_height[2]/2;
    }
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
    {
        dst_height[3] = dst_height[3]/2;
    }
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
    {
        dst_height[4] = dst_height[4]/2;
    }
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
   
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    /*set input */

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = arg_->yc_mode;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t)+1, NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    VX_CALL(vxProcessGraph(graph));
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}

/*testcase for NULL in control call   TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS*/
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessingChecksum_nv12_u8_fiveoutput_NULL, ArgFixed_nv12_u8, PARAMETERS_FIX_NV12_U8_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 0)
    {
        dst_height[0] = dst_height[0]/2;
    }
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
    {
        dst_height[1] = dst_height[1]/2;
    }
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 2)
    {
        dst_height[2] = dst_height[2]/2;
    }
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
    {
        dst_height[3] = dst_height[3]/2;
    }
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
    {
        dst_height[4] = dst_height[4]/2;
    }
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
   
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    /*set input */

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = arg_->yc_mode;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = NULL;
    ASSERT_NO_FAILURE(
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);
    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i ++)
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}
static uint32_t filtermode1_phasemode1_vshsmpcoeffsei1[] = {
    0x8bffb56d,
    0xa7344876
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_filtermode1_phasemode1_vshsmpcoeffsei1, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 1u;
        output_params[0].multi_phase.vert_coef_sel = 1u;
        output_params[0].multi_phase.init_phase_x = 0U;
        output_params[0].multi_phase.init_phase_y = 0U;
        output_params[0].offset_x = 0U;
        output_params[0].offset_y = 0U;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 1u;
        output_params[1].multi_phase.vert_coef_sel = 1u;
        output_params[0].multi_phase.init_phase_x = 0U;
        output_params[0].multi_phase.init_phase_y = 0U;
        output_params[0].offset_x = 0U;
        output_params[0].offset_y = 0U;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(filtermode1_phasemode1_vshsmpcoeffsei1[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_10bit_to_8bit_Invalid_interpolation, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, VX_INTERPOLATION_AREA);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_10bit_to_8bit_incorrectCoffConfig, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 4u;
        output_params[0].multi_phase.vert_coef_sel = 4u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
static uint32_t expected_cksm_10bit_to_8bit_filt0_src0_1[] = {
    0x8bffb56d,
    0xa7344876
};
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_filt0_src_0_1, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 2u;
        output_params[0].multi_phase.vert_coef_sel = 2u;

        output_params[1].filter_mode = 1U;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 1u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 1u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 2u;
        output_params[1].multi_phase.vert_coef_sel = 2u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_filt0_src0_1[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_target_params_filtermode1_phasemode0, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[0].multi_phase.horz_coef_sel = 0u;
            output_params[0].multi_phase.vert_coef_sel = 0u;
        }
        else
        {
            output_params[0].multi_phase.horz_coef_sel = 2u;
            output_params[0].multi_phase.vert_coef_sel = 2u;
        }

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[1].multi_phase.horz_coef_sel = 0u;
            output_params[1].multi_phase.vert_coef_sel = 0u;
        }
        else
        {
            output_params[1].multi_phase.horz_coef_sel = 2u;
            output_params[1].multi_phase.vert_coef_sel = 2u;
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
static uint32_t expected_cksm_10bit_to_8bit_TEST1[] = {
    0x8bffb56d,
    0xa7344876
};

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_target_params_filtermode1_phasemode1, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[0].multi_phase.horz_coef_sel = 0u;
            output_params[0].multi_phase.vert_coef_sel = 0u;
        }
        else
        {
            output_params[0].multi_phase.horz_coef_sel = 1u;
            output_params[0].multi_phase.vert_coef_sel = 1u;
        }

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 1u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[0].multi_phase.horz_coef_sel = 2u;
            output_params[0].multi_phase.vert_coef_sel = 2u;
        }
        else
        {
            output_params[0].multi_phase.horz_coef_sel = 3u;
            output_params[0].multi_phase.vert_coef_sel = 3u;
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_TEST1[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}
static uint32_t expected_cksm_10bit_to_8bit_TEST2[] = {
    0x8bffb56d,
    0xa7344876
};


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_target_params_filtermode1_horzsel1, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 1u;
        output_params[0].multi_phase.vert_coef_sel = 1u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 1u;
        output_params[1].multi_phase.vert_coef_sel = 1u;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_TEST2[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}

#define ADD_SIMUL_PROCESSING_MODE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/isEnableSimulProcessing = 0", __VA_ARGS__, 0))
#define PARAMETERS_FIX_SIMUL_PROCESSING_TEST \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_TEST, ADD_DUMMY,ADD_SIMUL_PROCESSING_MODE_TEST, ARG)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_Processing_ERR_STATUS, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_refs[5] = {0};
    vx_event_t event = { 0 };
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    /* enable events generation */
    vxEnableEvents(context);
    /* Register an error event for the node */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        /* Disable all events by default */
        sc_input_params.enable_error_events = 0U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(vxProcessGraph(graph));
    
    /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
     * and prints an error message saying that there are no events in the queue */
    vxWaitEvent(context, &event, vx_true_e);

    if(sc_input_params.enable_error_events == 0)
    {
        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_refs[0] = (vx_reference)scalar_err_stat_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        ASSERT(scalar_out_err_stat == 0);
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 
    }
    else
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("luma  =0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("chroma=0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }
    

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxDisableEvents(context));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testErrorInterrupts_Processing_ERR_STATUS_NULL, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_refs[5] = {0};
    vx_event_t event = { 0 };
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    /* enable events generation */
    vxEnableEvents(context);
    /* Register an error event for the node */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        /* Disable all events by default */
        sc_input_params.enable_error_events = 0U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(vxProcessGraph(graph));
    
    /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
     * and prints an error message saying that there are no events in the queue */
    vxWaitEvent(context, &event, vx_true_e);

    if(sc_input_params.enable_error_events == 0u)
    {
        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_refs[0] = NULL;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        ASSERT_NO_FAILURE(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 
    }
    else
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("luma  =0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x59ab963a == checksum_actual);

        rect.end_x = dst_width/2;
        rect.end_y = dst_height/2;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("chroma=0x%08x\n", checksum_actual);
        #endif
        ASSERT((uint32_t)0x08661321 == checksum_actual);
    }
    

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxDisableEvents(context));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_Nv12_Simul_Processing_STATUS_default, ArgFixed, PARAMETERS_FIX_SIMUL_PROCESSING_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_INIT_DIV_FACTOR,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_NO_FAILURE(
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_INIT_DIV_FACTOR,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}


#define ADD_IN_IMG1_YC_MODE_TEST(testArgName, nextmacro, ...) \
CT_EXPAND(nextmacro(testArgName "/input1_data_format=0", __VA_ARGS__, 0))
#define ADD_SET_TARGET_PARAMETERS_TEST(testArgName, nextmacro, ...) \
CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))
#define PARAMETERS_FIX_DIFF_BIT_DEPTH_TEST \
CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_TEST, ADD_DUMMY,ADD_VX_IN0_FORMATS, ADD_VX_IN1_FORMATS, ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE_TEST, ARG)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_Diff_Bit_Depth_With_Invalid_Params, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, src_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_reference refs[1] = {0};
    vx_rectangle_t rect;
    vx_status status;
    int i;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    for(i = 0; i < 10; i++)
    {
        if (i < 5)
        {
            dst_width[i] = src_w0 / 2;
            dst_height[i] = src_h0 / 2;
        }
        else
        {
            dst_width[i] = src_w1 / 2;
            dst_height[i] = src_h1 / 2;
        }
    }

    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    // Create valid outputs first
    for(i = 0; i < 10; i++)
    {
        if (i < 5)
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        else
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4],
        src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8], dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL), VX_TYPE_USER_DATA_OBJECT);
    sc_input_params.in_img0_yc_mode = arg_->in_img0_yc_mode;
    sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0, sizeof(tivx_vpac_msc_input_params_t),
        &sc_input_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS, refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj));
    VX_CALL(vxReleaseImage(&dst_image[1]));  // release dst_image[1] to make it NULL
    status = vxProcessGraph(graph);
    ASSERT(status == VX_ERROR_INVALID_PARAMETERS);

    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], arg_->input0_data_format), VX_TYPE_IMAGE);

    vxReleaseImage(&dst_image[0]);
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], src_h0 + 10, arg_->input0_data_format), VX_TYPE_IMAGE);  // taller
    status = vxProcessGraph(graph);
    ASSERT(status == VX_ERROR_INVALID_PARAMETERS);

    vxReleaseImage(&dst_image[0]);
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], arg_->input0_data_format), VX_TYPE_IMAGE);

    vxReleaseImage(&dst_image[0]);
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);  // assuming input0_data_format != U8
    if(arg_->input0_data_format != VX_DF_IMAGE_U8)
    {
        status = vxProcessGraph(graph);
        ASSERT(status == VX_ERROR_INVALID_PARAMETERS);
    }

    vxReleaseImage(&dst_image[0]);
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], arg_->input0_data_format), VX_TYPE_IMAGE);


    ASSERT_NO_FAILURE(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    for(i = 0; i < 10; i++) VX_CALL(vxReleaseImage(&dst_image[i]));
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_Diff_Bit_Depth_InvalidOutputWidth, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    for(i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0 / 2;
            dst_height[i] = src_h0 / 2;
        }
        else
        {
            dst_width[i] = src_w1 / 2;
            dst_height[i] = src_h1 / 2;
        }
    }

    //  Inject invalid output width to trigger branch
    dst_width[0] = src_w0 + 100;  // > input0 width → triggers 'out_img_w[cnt] > in_img0_w'
    // Optionally: dst_width[5] = src_w1 + 100; // > input1 width

    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1 / 2;
        for(i = 5; i < 10; i++)
        {
            dst_height[i] = dst_height[i] / 2;
        }
    }

    pixel.U32 = 0x0a7f1345;

    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }

    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8], dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    sc_input_params.in_img0_yc_mode = arg_->in_img0_yc_mode;
    sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj));

    //  Expect failure here because of invalid output width
    vx_status status = vxProcessGraph(graph);
    ASSERT(status != VX_SUCCESS); // or ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, status);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}
#define PARAMETERS_FIX_DIFF_BIT_DEPTH_CC \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_TEST, ADD_DUMMY,ADD_VX_IN0_FORMATS_CC_TEST, ADD_VX_IN1_FORMATS_CC_TEST,  ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE_CC_TEST, ARG)
#define ADD_VX_IN0_FORMATS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))
#define ADD_VX_IN0_FORMATS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))
#define ADD_VX_IN1_FORMATS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))
#define ADD_SET_TARGET_PARAMETERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))

#define ADD_IN_IMG1_YC_MODE_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/input1_data_format=0", __VA_ARGS__, 0))
#define PARAMETERS_OUT_HEIGHT_TEST \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_CC_TEST, ADD_DUMMY,ADD_VX_IN0_FORMATS_CC_TEST, ADD_VX_IN1_FORMATS_CC_TEST, ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE_CC_TEST, ARG)
/* Test case for validating the negative scenario of output height more than input for API tivxVpacMscScaleValidate() */
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_OutHeightvsInHeight, ArgFixed_diff_bit_depth, PARAMETERS_OUT_HEIGHT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0+2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1+2;
        }
    }
    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);

    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutHeightvsInHeight_odd, ArgFixed_diff_bit_depth, PARAMETERS_OUT_HEIGHT_TEST)

{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, src_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    for (int i = 0; i < 10; i++)
    {
        if (i < 5)
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
 
    dst_height[6] = src_h1 * 2; // 2160
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    if (arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    else if (arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if (arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if (arg_->in_img1_yc_mode == 0)
        {
            ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
        }
        else
        {
            ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
        }
    }
    else if (arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if (arg_->in_img1_yc_mode == 0)
        {
            ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
        }
        else
        {
            ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
        }
    }
    for (int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], (i < 5) ? arg_->input0_data_format : arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0, dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4],
        src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8], dst_image[9]),VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
    tivxHwaUnLoadKernels(context);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_Diff_Bit_Depth_diff_inimg1, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5) 
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }

    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    /* Configuring the output format as wrong compared to input for negative case */
    if(arg_->in_img1_yc_mode == 0){
        VX_CALL(vxReleaseImage(&dst_image[0]));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }
    else{
        VX_CALL(vxReleaseImage(&dst_image[1]));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
    tivxHwaUnLoadKernels(context);


}
/* Test case for validating the negative scenario of output format not equal to input for API tivxVpacMscScaleValidate() */
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutFormatvsInFormat, ArgFixed_diff_bit_depth, PARAMETERS_OUT_HEIGHT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }

    
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
    }

    /* Configuring the output format as wrong compared to input for negative case */
    if(arg_->in_img1_yc_mode == 0){
        VX_CALL(vxReleaseImage(&dst_image[0]));
        ASSERT(dst_image[0] == 0);
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }
    else{
        VX_CALL(vxReleaseImage(&dst_image[1]));
        ASSERT(dst_image[1] == 0);
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}

/* Test case for validating the negative scenario of output format not equal to input for API tivxVpacMscScaleValidate() */
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat, ArgFixed_diff_bit_depth, PARAMETERS_OUT_HEIGHT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
    }
    if(arg_->in_img1_yc_mode == 0){
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    }
    else {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    /* Configuring the output format as wrong compared to input for negative case */
    if(arg_->in_img1_yc_mode == 0){
        for(int i = 0; i < 10; i++)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
    }
    else {
        for(int i = 0; i < 10; i++)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if(arg_->in_img1_yc_mode == 0){
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], (vx_image)NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    }
    else {
        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], (vx_image)NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    }

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}

#define PARAMETERS_OUT_FORMAT_TEST \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_CC_TEST, ADD_DUMMY,ADD_VX_IN0_FORMATS_CC_TEST, ADD_VX_IN1_FORMATS_CC_TEST, ADD_IN_IMG0_YC_MODE, ADD_IN_IMG1_YC_MODE_CC_TEST, ARG)
/* Test case for validating the negative scenario of output format as U16 for API tivxVpacMscScaleValidate() */
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_U16, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }

   
    
    if(arg_->in_img1_yc_mode == 0){
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }
    else {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    /* Configuring the output format as wrong compared to input for negative case */
    if(arg_->in_img1_yc_mode == 0){
        for(int i = 0; i < 10; i++)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
    }
    else {
        for(int i = 0; i < 10; i++)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if(arg_->in_img1_yc_mode == 0){
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], (vx_image)NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    }
    else {
        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], (vx_image)NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    }

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_outN12, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_S16, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
    
    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_NV12, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
   
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_YUYV, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
   
    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
    }
    VX_CALL(vxReleaseImage(&dst_image[4]));
    ASSERT(dst_image[4] == 0);
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_UYVY, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
   
    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    }
    VX_CALL(vxReleaseImage(&dst_image[4]));
    ASSERT(dst_image[4] == 0);
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_img_2cnt_1, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;

    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
    
    
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    VX_CALL(vxReleaseImage(&dst_image[7]));
    ASSERT(dst_image[7] == 0);
    ASSERT_VX_OBJECT(dst_image[7] = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}
/* Test case for validating the negative scenario of NULL != in_img0 && NULL == in_img1 and cnt =10 for API tivxVpacMscScaleValidate() */
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_CntFalseCases, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    // Expect process to fail due to invalid output image format
    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], (vx_image)NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}
#endif
#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_UYVY_cnt8, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image[4] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    const char *output_file_names[4] = { "output/msc_uyvy0.yuv", "output/msc_uyvy1.yuv", "output/msc_uyvy2.yuv", "output/msc_uyvy3.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[4][MAXPATHLENGTH];

    size_t sz;
    vx_reference refs[5] = {0};
    vx_reference output_refs[4] = {0};
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[4];
    vx_user_data_object coeff_obj, output_obj[4];
    vx_pixel_value_t pixel;
    int i = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    for(i = 0; i < 4; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readUYVYInput(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    tivx_vpac_msc_coefficients_params_init(&coeffs, interpolation);
    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    /* Set Output params */
    for(i = 0; i < 4; i ++)
    {
        tivx_vpac_msc_output_params_init(&output_params[i]);
        output_params[i].filter_mode = 0;
        ASSERT_VX_OBJECT(output_obj[i] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t",
            sizeof(tivx_vpac_msc_output_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        VX_CALL(vxCopyUserDataObject(output_obj[i], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[i], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    

            output_refs[i] = (vx_reference)output_obj[i];
        }    
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 4u));
    for(i=0; i<4; i++)
    {
        VX_CALL(vxReleaseUserDataObject(&output_obj[i]));
    }
    VX_CALL(vxProcessGraph(graph));
    for (i = 0; i < 4; i++) 
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = dst_width;
        rect.end_y = dst_height;

        checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
        #if defined(TEST_MSC_CHECKSUM_LOGGING)
        printf("yuv422i checksum = 0x%08x\n", checksum_actual);
        write_uyvy_output_image(output_files[i], dst_image[i]);
        #endif
        ASSERT((uint32_t)0xc7744b86 == checksum_actual);
    }


    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    for (int i = 0; i < 4; i++) 
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    for (int i = 0; i < 4; i++) 
    {
        ASSERT(dst_image[i] == 0);
    }
    ASSERT(src_image == 0);
}
#endif

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testErrorInterrupts_invalidCoeff, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[5] = {0};
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    // Create invalid coefficient object (incorrect size)
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t) - 4, NULL),  // intentionally wrong size
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    ASSERT_NO_FAILURE(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
#define PARAMETERS_FIX_TEST\
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_TEST, ADD_DUMMY, ARG)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_UYVY_input_Y_output_yuyv_uyvy, ArgFixed, PARAMETERS_FIX_TEST)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image[TEST_NUM_OUTPUTS_YUV422I] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    vx_char temp[256];
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_reference refs[5] = {0};
    tivx_vpac_msc_coefficients_t coeffs;
    vx_user_data_object coeff_obj;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
    for (i = 1; i < TEST_NUM_OUTPUTS_YUV422I; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readUYVYInput(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    for (i = 0; i < TEST_NUM_OUTPUTS_YUV422I; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(src_image == 0);
}
/*tivxVpacMscScaleSetCropParamsCmd() numparams  changing the size*/

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_Invalid_crop_prms_size, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i,j;
    vx_image src_image = 0, dst_image[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[TEST_MSC_TOTAL_OUTPUTS];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[TEST_MSC_TOTAL_OUTPUTS];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[TEST_MSC_TOTAL_OUTPUTS] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv", "output/msc_out3.yuv", "output/msc_out4.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[TEST_MSC_TOTAL_OUTPUTS][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[TEST_MSC_TOTAL_OUTPUTS];
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    int crop_start_y_values[TEST_MSC_TOTAL_OUTPUTS] = {20, 80, 148,0,0};
    int cnt = 0;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_user_data_object sc_input_params_obj;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    
     for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
     {
         crop_refs[i] = NULL;
     }

    /* Set crop parameters */
    
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        crop[i].crop_start_x = 150;
        crop[i].crop_width = dst_width;
        crop[i].crop_height = dst_height;
        crop[i].crop_start_y = crop_start_y_values[i];

        ASSERT_VX_OBJECT(crop_obj[i] = vxCreateUserDataObject(context, 
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t)+1, NULL), 
                VX_TYPE_USER_DATA_OBJECT);
        ASSERT_NO_FAILURE(vxCopyUserDataObject(crop_obj[i], 0, 
            sizeof(tivx_vpac_msc_crop_params_t), &crop[i], VX_WRITE_ONLY, 
            VX_MEMORY_TYPE_HOST));
        crop_refs[i] = (vx_reference)crop_obj[i];
    }
    ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
        crop_refs, 5u));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        VX_CALL(vxReleaseUserDataObject(&crop_obj[i]));
        ASSERT(dst_image[i] == 0 && crop_obj[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

    tivxHwaUnLoadKernels(context); 
}
/*passing null for usr_data_obj and giving different size other than tivx_vpac_msc_psa_timestamp_data_t*/
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessingGetPsaValues_usr_data_obj_NULL, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i,j;
    vx_image src_image = 0, dst_image[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[TEST_MSC_TOTAL_OUTPUTS];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[TEST_MSC_TOTAL_OUTPUTS];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[TEST_MSC_TOTAL_OUTPUTS] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv", "output/msc_out3.yuv", "output/msc_out4.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[TEST_MSC_TOTAL_OUTPUTS][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[TEST_MSC_TOTAL_OUTPUTS];
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    int crop_start_y_values[TEST_MSC_TOTAL_OUTPUTS] = {20, 80, 148,0,0};
    int cnt = 0;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_user_data_object sc_input_params_obj;
    uint32_t psa_values[TEST_NUM_NODE_INSTANCE][TEST_MSC_MAX_OUTPUTS] = {0};
    vx_reference psa_refs [TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    input_timestamp = SET_TIME_STAMP_VALUE;
    VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
    &input_timestamp, sizeof(input_timestamp)));
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    VX_CALL(vxProcessGraph(graph));

    ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);
    psa_refs[0] = NULL;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
        psa_refs, 1u));
    ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t), 
        &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxReleaseUserDataObject(&psa_obj));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

    tivxHwaUnLoadKernels(context); 
}
/*giving different size other than tivx_vpac_msc_psa_timestamp_data_t*/ 
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessingGetPsaValues_diff_size, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, dst_width = 600, dst_height = 772, i,j;
    vx_image src_image = 0, dst_image[TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[TEST_MSC_TOTAL_OUTPUTS];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[TEST_MSC_TOTAL_OUTPUTS];
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[TEST_MSC_TOTAL_OUTPUTS] = { "output/msc_out0.yuv", "output/msc_out1.yuv", "output/msc_out2.yuv", "output/msc_out3.yuv", "output/msc_out4.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[TEST_MSC_TOTAL_OUTPUTS][MAXPATHLENGTH];
    size_t sz;
    vx_reference crop_refs[TEST_MSC_TOTAL_OUTPUTS];
    vx_reference refs[TEST_MSC_TOTAL_OUTPUTS] = {0};
    int crop_start_y_values[TEST_MSC_TOTAL_OUTPUTS] = {20, 80, 148,0,0};
    int cnt = 0;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_user_data_object sc_input_params_obj;
    uint32_t psa_values[TEST_NUM_NODE_INSTANCE][TEST_MSC_MAX_OUTPUTS] = {0};
    vx_reference psa_refs [TEST_MSC_TOTAL_OUTPUTS] = {0};
    vx_user_data_object psa_obj;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    input_timestamp = SET_TIME_STAMP_VALUE;
    VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
    &input_timestamp, sizeof(input_timestamp)));
    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
        ASSERT_(return, (sz < MAXPATHLENGTH));
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image, 
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));
    ASSERT_VX_OBJECT(psa_obj = 
    vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t)+1, NULL),
    VX_TYPE_USER_DATA_OBJECT);

    psa_refs[0] = (vx_reference) psa_obj;

    tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
        psa_refs, 1u);

    ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t), 
        &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL(vxReleaseUserDataObject(&psa_obj));
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0 && graph == 0);

    for (i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++) 
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
    }
    VX_CALL(vxReleaseImage(&src_image));
    ASSERT(src_image == 0);

    tivxHwaUnLoadKernels(context); 
}
#define ADD_YC_MODE_CC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=0", __VA_ARGS__, 0))
#define PARAMETERS_FIX_CC \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS_CC_TEST, ADD_DUMMY, ADD_YC_MODE_CC, ARG)
#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testNullUserData, ArgFixed, PARAMETERS_FIX_CC)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    char *output_file_name = "output/msc_out_uyvy.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_reference refs[5] = {0};
    vx_reference output_refs[2] = {0};
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params;
    vx_user_data_object coeff_obj, output_obj;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    tivx_vpac_msc_output_params_init(&output_params);
    output_params.filter_mode = 0;
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readUYVYInput(file, src_image));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));
    
    tivx_vpac_msc_coefficients_params_init(&coeffs, interpolation);
    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)NULL;

    ASSERT_EQ_VX_STATUS(VX_FAILURE,tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,refs, 1u));
    
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));
    
    /* Set Output params */
    ASSERT_VX_OBJECT(output_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_output_params_t",
        sizeof(tivx_vpac_msc_output_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(output_obj, 0,
        sizeof(tivx_vpac_msc_output_params_t), &output_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    output_refs[0] = (vx_reference)output_obj;
    
    VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,output_refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&output_obj));

    VX_CALL(vxProcessGraph(graph));

    

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
#endif
#if defined TEST_MSC_ENABLE
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Simul_Processing_with_kernel_size_3, ArgFixed, PARAMETERS_FIX_SIMUL_PROCESSING)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0, cnt = 0, idx=0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0}, output_refs[2] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj,output_obj[1];
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_output_params_t output_params[1];
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));
    tivx_vpac_msc_output_params_init(&output_params[0]);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    sc_input_params.kern_sz = 3;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    for (cnt = 0; cnt < 1; cnt++)
    {
        ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t",
            sizeof(tivx_vpac_msc_output_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        output_refs[cnt] = (vx_reference)output_obj[cnt];
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
        output_refs, 1u));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height;  
    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("luma  =0x%08x\n", checksum_actual);
    write_output_image_nv12_8bit(file, dst_image);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[0] == checksum_actual);
    rect.end_x = dst_width/2;
    rect.end_y = dst_height/2;

    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("chroma=0x%08x\n", checksum_actual);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[1] == checksum_actual);  

    VX_CALL(vxReleaseUserDataObject(&output_obj[0]));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Nv12_Simul_Processing_with_kernel_size_4, ArgFixed, PARAMETERS_FIX_SIMUL_PROCESSING)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0, cnt = 0, idx=0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0}, output_refs[2] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj,output_obj[1];
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_output_params_t output_params[1];
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image));
    tivx_vpac_msc_output_params_init(&output_params[0]);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.is_enable_simul_processing = arg_->isEnableSimulProcessing;
    sc_input_params.kern_sz = 4;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    img_scale_set_coeff(&coeffs, interpolation);

    /* Set Coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)coeff_obj;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&coeff_obj));

    for (cnt = 0; cnt < 1; cnt++)
    {
        ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
            "tivx_vpac_msc_output_params_t",
            sizeof(tivx_vpac_msc_output_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
            sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        output_refs[cnt] = (vx_reference)output_obj[cnt];
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
        output_refs, 1u));

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));
    VX_CALL(vxProcessGraph(graph));
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = dst_width;
    rect.end_y = dst_height;  
    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("luma  =0x%08x\n", checksum_actual);
    write_output_image_nv12_8bit(file, dst_image);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[0] == checksum_actual);
    rect.end_x = dst_width/2;
    rect.end_y = dst_height/2;

    checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
    #if defined(TEST_MSC_CHECKSUM_LOGGING)
    printf("chroma=0x%08x\n", checksum_actual);
    #endif
    ASSERT(expected_cksm_msc_simul_processing[1] == checksum_actual);  

    VX_CALL(vxReleaseUserDataObject(&output_obj[0]));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));
    tivxHwaUnLoadKernels(context);

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
#endif

#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testKrSz6, ArgFixed_nv12_u8, PARAMETERS_FIX_CC)
{
    vx_context context = context_->vx_context_;
    int w = 1280, h = 512, i, j, crop_mode = 0;
    int dst_width[5] = {0}, dst_height[5] = {0};
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_uint32 cksm_offset = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    // Create input image (NV12)
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    // --- Output 0: U8 image ---
    dst_width[0] = w;
    dst_height[0] = h;
    if(arg_->yc_mode == 1)
        dst_height[0] /= 2;
    ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // --- Output 1: U8 image ---
    dst_width[1] = w/2;
    dst_height[1] = h/2;
    if(arg_->yc_mode == 1)
        dst_height[1] /= 2;
    ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // --- Output 2: S16 image (modified format) ---
    dst_width[2] = w-1;
    dst_height[2] = h-1;
    if(arg_->yc_mode == 1)
        dst_height[2] /= 2;
    ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width[2], dst_height[2], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // --- Output 3: U8 image ---
    dst_width[3] = w-3;
    dst_height[3] = h-3;
    if(arg_->yc_mode == 1)
        dst_height[3] /= 2;
    ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width[3], dst_height[3], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // --- Output 4: NV12 image (modified format) ---
    dst_width[4] = w-2;
    dst_height[4] = h-2;
    if(arg_->yc_mode == 1)
        dst_height[4] /= 2;
    ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width[4], dst_height[4], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    VX_CALL(readNV12Input(file, src_image)); 

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    img_scale_set_coeff(&coeffs, interpolation);

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    sc_input_params.yc_mode = arg_->yc_mode;
    sc_input_params.kern_sz = 6;
    sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
    sc_input_params.is_enable_simul_processing = 0;
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));
    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 

    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
        VX_CALL(vxReleaseImage(&dst_image[i]));

    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context);

    for(i = 0; i < TEST_MSC_TOTAL_OUTPUTS; i++)
        ASSERT(dst_image[i] == 0);

    ASSERT(src_image == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testImg0ycChroma, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testImg0ycLUMACHROMA, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w0, src_h0, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        sc_input_params.is_enable_simul_processing = 1U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    ASSERT_NO_FAILURE(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testImg0ycLUMACHROMA_u8u16, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        sc_input_params.is_enable_simul_processing = 1U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    ASSERT_NO_FAILURE(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testImg0ycLUMACHROMA_p12u16, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        sc_input_params.is_enable_simul_processing = 1U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    ASSERT_NO_FAILURE(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testImg0ycLUMACHROMA_S16S16, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        sc_input_params.is_enable_simul_processing = 1U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    ASSERT_NO_FAILURE(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testImg0ycLUMACHROMA_S16S16_enableprocessing0, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_NO_FAILURE(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
        sc_input_params.is_enable_simul_processing = 0U;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    ASSERT_NO_FAILURE(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testLumaLuma_diff_in0_in1_height, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 540, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
        sc_input_params.in_img1_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    VX_CALL(vxProcessGraph(graph));
    

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testLumaChroma_diff_in0_in1_height_conflict, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH_CC)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
        sc_input_params.in_img1_yc_mode = TIVX_VPAC_MSC_MODE_CHROMA_ONLY;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    VX_CALL(vxProcessGraph(graph));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}
#endif

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_FixedPattern_num_params_NullData, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 16, h = 16, i, j, crop_mode = 0;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;

    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;

        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        data_ptr_u8 = data_ptr;

        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = j*16;
            }

        }

        vxUnmapImagePatch(src_image, map_id);
#endif
        dst_width = w-4;
        dst_height = h-4;

        if(crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        img_scale_set_coeff(&coeffs, interpolation);

        VX_CALL(vxVerifyGraph(graph));

        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        if(crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            /* Center crop of input */
            crop.crop_start_x = w / 4;
            crop.crop_start_y =h / 4;
            crop.crop_width   = w / 2;
            crop.crop_height  = h / 2;

            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t)+1, &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = NULL;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 10U));//changing here

            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));

        tivxHwaUnLoadKernels(context);
    }

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}


TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testNullUserDataSetCoeff, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 1920, h = 1080, i, j, crop_mode = 0;
    int dst_width = w/2, dst_height = h/2;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    char *output_file_name = "output/msc_out.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    int run_cnt;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string)); // Ensure target is enabled

    tivxHwaLoadKernels(context); // Load VPAC MSC kernels
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    // Create input and output images (NV12 format)
    ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

    // Prepare input file path
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    // Read NV12 image into input image
    VX_CALL(readNV12Input(file, src_image));

    // Create graph and node
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Pass same dst_image as both output 0 and output 4 → triggers an error scenario
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
        dst_image, NULL, NULL, NULL, dst_image), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    // Fill coefficients with interpolation setting
    img_scale_set_coeff(&coeffs, interpolation);

    ASSERT_NO_FAILURE(vxVerifyGraph(graph)); // Verify graph

    /* Create user data object for coefficients */
    ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_coefficients_t",
        sizeof(tivx_vpac_msc_coefficients_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    // Copy coefficients to user data object
    VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
        sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));

    refs[0] = (vx_reference)NULL;//coeff_obj;

    // Intentionally expect VX_FAILURE due to invalid node config (duplicated output image handle)
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&coeff_obj)); // Release coeff object

    // Prepare output file path
    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_name);
    ASSERT_(return, (sz < MAXPATHLENGTH));


   
    // Release resources
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    tivxHwaUnLoadKernels(context); // Unload kernels

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_FixedPattern_usr_obj_null, ArgFixed, PARAMETERS_FIX)
{
    vx_context context = context_->vx_context_;
    int w = 16, h = 16, i, j, crop_mode = 0;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
 
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
 
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
 
/* Note: for debug--load in a file by putting a breakpoint at unmap */
#if 1
        vx_imagepatch_addressing_t image_addr;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr, *data_ptr2;
        uint8_t *data_ptr_u8;
 
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = w;
        rect.end_y = h;
 
        vxMapImagePatch(src_image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );
 
        data_ptr_u8 = data_ptr;
 
        for(j=0; j < h; j++)
        {
            for(i=0; i<w; i++)
            {
                data_ptr_u8[j*image_addr.stride_y+i] = j*16;
            }

        }
 
        vxUnmapImagePatch(src_image, map_id);
#endif
        dst_width = w-4;
        dst_height = h-4;
 
        if(crop_mode == 1)
        {
            dst_width /= 2;
            dst_height /= 2;
        }
 
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image, NULL, NULL, NULL, NULL), VX_TYPE_NODE);
 
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
 
        img_scale_set_coeff(&coeffs, interpolation);
 
        VX_CALL(vxVerifyGraph(graph));
 
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
 
        refs[0] = NULL;
        ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
 
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
 
        if(crop_mode == 1)
        {
            /* Set Input Crop */
            ASSERT_VX_OBJECT(crop_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
            /* Center crop of input */
            crop.crop_start_x = w / 4;
            crop.crop_start_y =h / 4;
            crop.crop_width   = w / 2;
            crop.crop_height  = h / 2;
 
            VX_CALL(vxCopyUserDataObject(crop_obj, 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));
 
            refs[0] = (vx_reference)crop_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
                refs, 5u));
 
            VX_CALL(vxReleaseUserDataObject(&crop_obj));
        }
 
        VX_CALL(vxProcessGraph(graph));
       
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
 
        ASSERT(node == 0);
        ASSERT(graph == 0);
 
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
 
        tivxHwaUnLoadKernels(context);
    }
 
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
}
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputPositive, testGraphProcessing_10bit_to_8bit_init_phase_x_y, ArgFixed_10bit_to_8bit,
    PARAMETERS_10BIT_TO_8BIT
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int src_width = 2592, src_height = 1944, dst_width[2] , dst_height[2], idx;
    vx_image y12 = 0, dst_image[2] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, output_obj[2], crop_obj[2];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_output_params_t output_params[2];
    tivx_vpac_msc_crop_params_t crop[2];
    vx_reference refs[5], output_refs[2], crop_refs[2];
    vx_char temp[256];
    vx_rectangle_t rect;
    int i, checksum_actual = 0;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        dst_width[0] = 2592;
        dst_height[0] = 1458;
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width[0], dst_height[0], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[0]);

        dst_width[1] = 2592/2;
        dst_height[1] = 1458/2;
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width[1], dst_height[1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        tivx_vpac_msc_output_params_init(&output_params[1]);

        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;
        output_params[0].offset_x = 0;
        output_params[0].offset_y = 0;
        output_params[0].multi_phase.init_phase_x = 0;
        output_params[0].multi_phase.init_phase_y = 0;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 0u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 0u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;
        output_params[1].offset_x = 0;
        output_params[1].offset_y = 0;
        output_params[1].multi_phase.init_phase_x = 0;
        output_params[1].multi_phase.init_phase_y= 0;

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, y12,
            dst_image[0], dst_image[1], NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        /* Set Coefficients */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (cnt = 0; cnt < 2; cnt++)
        {
            output_refs[cnt] = NULL;
            crop_refs[cnt] = NULL;
        }

        /* Set Output params */
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(output_obj[cnt] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[cnt], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[cnt] = (vx_reference)output_obj[cnt];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, 2u));
        for (cnt = 0; cnt < 2; cnt++)
        {
            ASSERT_VX_OBJECT(crop_obj[cnt] = vxCreateUserDataObject(context,
                    "tivx_vpac_msc_crop_params_t",
                    sizeof(tivx_vpac_msc_crop_params_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            crop[cnt].crop_start_x = 0;
            crop[cnt].crop_start_y = 0;
            crop[cnt].crop_width   = 2592;
            crop[cnt].crop_height  = 1458;

            VX_CALL(vxCopyUserDataObject(crop_obj[cnt], 0,
                sizeof(tivx_vpac_msc_crop_params_t), &crop[cnt], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            crop_refs[cnt] = (vx_reference)crop_obj[cnt];
        }
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            crop_refs, 2u));
        VX_CALL(vxProcessGraph(graph));
        for(i = 0; i < 2; i ++)
        {
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = dst_width[i];
            rect.end_y = dst_height[i];
            checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            sprintf(temp, "output/10bit_to_8bit_output_%d", i);
            save_image_from_msc(dst_image[i], temp);
            #endif
            ASSERT(expected_cksm_10bit_to_8bit[i] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        for (cnt = 0; cnt < 2; cnt ++)
        {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
            VX_CALL(vxReleaseUserDataObject(&output_obj[cnt]));
            VX_CALL(vxReleaseUserDataObject(&crop_obj[cnt]));
        }
        VX_CALL(vxReleaseImage(&y12));

        tivxHwaUnLoadKernels(context);
    }
    for (cnt = 0; cnt < 2; cnt ++)
    {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(y12 == 0);
}

#define PARAMETERS_FOUR_OUTPUT_TEST \
    /* 1:1 scale */ \
    SCALE_TEST_FOUR_OUTPUT(VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_generate_random, "random", 1_1, 2_1, 2_1, 4_1, 0, ADD_SIZE_256x256, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_BORDERS_TEST, ARG, 0)

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative, testGraphProcessing_FourOutput_Invalid_DstImg_YUYV, Arg_FourOutput,
    PARAMETERS_FOUR_OUTPUT_TEST
)
{
    vx_context context = context_->vx_context_;
    int cnt;
    int w = 1280, h = 512;
    int dst_width = 0, dst_height = 0;
    vx_image src_image = 0, dst_image[5] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference refs[1];

    CT_Image src = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string)); // Ensure target is enabled

    {
        tivxHwaLoadKernels(context); // Load hardware acceleration kernels
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        // Generate input image (but intentionally creating src_image directly to test format handling)
        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, w, h, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

        // Create output images with mixed supported formats for coverage
        ASSERT_NO_FAILURE(arg_->dst_size_generator0(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[0] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator1(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[1] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator2(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[2] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[3] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(arg_->dst_size_generator3(src->width, src->height, &dst_width, &dst_height));
        ASSERT_VX_OBJECT(dst_image[4] = vxCreateImage(context, dst_width, dst_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        // Create the graph and node
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode(graph, src_image,
            dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4]), VX_TYPE_NODE);
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        // Set border attributes for the node
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &arg_->border, sizeof(arg_->border)));

        img_scale_set_coeff(&coeffs, arg_->interpolation); // Fill coefficient struct

        ASSERT_NO_FAILURE(vxVerifyGraph(graph)); // Verify the graph is valid

        // Create a user data object to hold coefficients
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        // Copy the coefficient data into the object
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;

        // Intentionally expecting failure when sending the command (negative test)
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        // Process the graph (should still complete despite invalid command)
        ASSERT_NO_FAILURE(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        for (cnt = 0; cnt < 5; cnt++) {
            VX_CALL(vxReleaseImage(&dst_image[cnt]));
        }

        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        tivxHwaUnLoadKernels(context); // Unload kernels
    }

    // Ensure all objects are released
    for (cnt = 0; cnt < 5; cnt++) {
        ASSERT(dst_image[cnt] == 0);
    }
    ASSERT(src_image == 0);
}

#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutputNegative2, testGraphProcessing_OutImageFormat_out_S16_input2null, ArgFixed_diff_bit_depth, PARAMETERS_OUT_FORMAT_TEST)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop;
    vx_reference refs[5] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        dst_width[i] = src_w0/2;
        dst_height[i] = src_h0/2;
    }
    ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
    if(arg_->input0_data_format == VX_DF_IMAGE_U8)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U8)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma8b.raw", 1);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma8b.raw", 1);
            }
        }
    }
    if(arg_->input0_data_format == VX_DF_IMAGE_U16)
    {
        ct_read_raw_image_to_vx_image(src_image0, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
    }
    if(arg_->input1_data_format == VX_DF_IMAGE_U16)
    {
        if(arg_->in_img0_yc_mode == 0)
        {
            if(arg_->in_img1_yc_mode == 0)
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
            }
            else
            {
                ct_read_raw_image_to_vx_image(src_image1, "psdkra/app_single_cam/1920x540_chroma16b.raw", 2);
            }
        }
    }

    for(int i = 0; i < 10; i++)
    {
        ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], NULL, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0);
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);
}

TEST_WITH_ARG(tivxHwaVpacMscScaleMultiOutput, testGraphProcessing_Diff_Bit_Depth_negative, ArgFixed_diff_bit_depth, PARAMETERS_FIX_DIFF_BIT_DEPTH)
{
    vx_context context = context_->vx_context_;
    int src_w0 = 1920, src_h0 = 1080, src_w1 = 1920, src_h1 = 1080, i, j, crop_mode = 0, cnt = 0;
    int dst_width[10], dst_height[10];
    vx_image src_image0 = 0, dst_image0 = 0;
    vx_image src_image1 = 0, dst_image1 = 0;
    vx_image dst_image[10] = {0};
    vx_graph graph = 0;
    vx_node node = 0;
    vx_user_data_object coeff_obj, crop_obj[10], output_obj[10];
    tivx_vpac_msc_coefficients_t coeffs;
    tivx_vpac_msc_crop_params_t crop[10];
    tivx_vpac_msc_output_params_t output_params[10];
    vx_reference refs[5] = {0}, output_refs[10] = {0};
    vx_rectangle_t rect;
    uint32_t checksum_actual;
    vx_enum interpolation = VX_INTERPOLATION_BILINEAR;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    vx_pixel_value_t pixel;
    vx_reference crop_refs[10];
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
    for(i=0; i<10; i++)
    {
        if(i < 5)
        {
            dst_width[i] = src_w0/2;
            dst_height[i] = src_h0/2;
        }
        else
        {
            dst_width[i] = src_w1/2;
            dst_height[i] = src_h1/2;
        }
    }
    if((arg_->in_img0_yc_mode == 0 && arg_->in_img1_yc_mode == 1))
    {
        src_h1 = src_h1/2;
        for(i = 5; i< 10; i++)
        {
            dst_height[i] = dst_height[i]/2;
        }
    }
    pixel.U32 = 0x0a7f1345;
    if(arg_->input0_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateImage(context, src_w0, src_h0, arg_->input0_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format != TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateImage(context, src_w1, src_h1+1, arg_->input1_data_format), VX_TYPE_IMAGE);
    }
    if(arg_->input0_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image0 = vxCreateUniformImage(context, src_w0, src_h0, arg_->input0_data_format, &pixel), VX_TYPE_IMAGE);
    }
    if(arg_->input1_data_format == TIVX_DF_IMAGE_P12)
    {
        ASSERT_VX_OBJECT(src_image1 = vxCreateUniformImage(context, src_w1, src_h1+1, arg_->input1_data_format, &pixel), VX_TYPE_IMAGE);
    }

    for(int i = 0; i < 10; i++)
    {
        if(i < 5)
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input0_data_format), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, dst_width[i], dst_height[i], arg_->input1_data_format), VX_TYPE_IMAGE);
        }
        tivx_vpac_msc_output_params_init(&output_params[i]);
    }

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxVpacMscScaleNode2(graph, src_image0,
        dst_image[0], dst_image[1], dst_image[2], dst_image[3], dst_image[4], src_image1, dst_image[5], dst_image[6], dst_image[7], dst_image[8],dst_image[9]), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

    VX_CALL(vxVerifyGraph(graph));

    tivx_vpac_msc_input_params_init(&sc_input_params);
    ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
        "tivx_vpac_msc_input_params_t",
        sizeof(tivx_vpac_msc_input_params_t), NULL),
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        sc_input_params.in_img0_yc_mode = arg_->in_img0_yc_mode;
        sc_input_params.in_img1_yc_mode = arg_->in_img1_yc_mode;
    VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
        sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST));
    refs[0] = (vx_reference)sc_input_params_obj;
    ASSERT_EQ_VX_STATUS(VX_FAILURE,
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
        refs, 1u));

    VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
    
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    for (int i = 0; i < 10; i++)
    {
        VX_CALL(vxReleaseImage(&dst_image[i]));
        ASSERT(dst_image[i] == 0 );
    }
    VX_CALL(vxReleaseImage(&src_image0));
    VX_CALL(vxReleaseImage(&src_image1));
    tivxHwaUnLoadKernels(context);
    ASSERT(src_image0 == 0);
    ASSERT(src_image1 == 0);

}
#endif

TESTCASE_TESTS(tivxHwaVpacMscScaleMultiOutput,
    testNodeCreation,
    testGraphProcessing_FixedPattern,
    testGraphProcessing_FixedPattern_tiovx_1129,
    testGraphProcessing_Nv12,
    testGraphProcessing_OneOutput,
    testGraphProcessing_TwoOutput,
    testGraphProcessing_ThreeOutput,
    testGraphProcessing_FourOutput,
    testGraphProcessing_FiveOutput,
    testGraphProcessingChecksum_nv12_u8_fiveoutput,
    testGraphProcessing_10bit_to_8bit
    #if defined(VPAC3) || defined(VPAC3L)
    ,
    testGraphProcessing_Nv12_Simul_Processing
    #ifndef x86_64
    ,
    testGraphProcessing_Nv12_Simul_Processing_safety
    #endif
    ,
    testGraphProcessing_Diff_Bit_Depth,
    testGraphProcessing_UYVY,
    testGraphProcessing_UYVY_input_Y_output,
    testGraphProcessing_RGB_Plane_Split,
    testGraphProcessing_Diff_Bit_Depth_negative,
    testGraphProcessing_Nv12_Simul_Processing_crop_fix
    #if defined (TEST_MSC_ENABLE)
    ,
    testGraphProcessing_Nv12_Simul_Processing_with_kernel_size_3,
    testGraphProcessing_Nv12_Simul_Processing_with_kernel_size_4
    #endif
    #ifndef x86_64
    ,
    testErrorInterrupts
    #endif
    #endif
    #ifndef x86_64
    ,
    testErrorInterrupts_j721e,
    testGraphProcessingGetPsaValues
    #ifndef VPAC3L
    ,
    testGraphProcessingSl2Firewall
    #endif
    #endif
    ,
    testGraphProcessing_LumaChroma_CropAlign,
    testGraphProcessing_Nv12_Perf,
    testGraphProcessing_multi,
    testGraphProcessing_10bit_8bit_with_single_phase_coeff,
    testGraphProcessing_10bit_8bit_non_auto_compute
)

TESTCASE_TESTS(tivxHwaVpacMscScaleMultiOutputPositive,
    testGraphProcessing_10bit_to_8bit_init_phase_x_y,
    testGraphProcessing_10bit_to_8bit_filt0_src_0_1,
    testGraphProcessing_10bit_to_8bit_target_params_filtermode1_phasemode0,
    testGraphProcessing_10bit_to_8bit_target_params_filtermode1_phasemode1,
    testGraphProcessing_10bit_to_8bit_target_params_filtermode1_horzsel1,
    testGraphProcessing_10bit_to_8bit_filtermode1_phasemode1_vshsmpcoeffsei1
    #if defined(VPAC3) || defined(VPAC3L)
    ,    
    testGraphProcessing_UYVY_cnt8,
    testGraphProcessing_OutHeightvsInHeight
    #ifndef x86_64
    ,    
    testGraphProcessing_10bit_to_8bit_Safety_Mech
    #endif
    #endif
)

TESTCASE_TESTS(tivxHwaVpacMscScaleMultiOutputNegative,
    testGraphProcessing_FixedPattern_num_params_NullData,
    testGraphProcessingChecksum_nv12_u8_fiveoutput_INVALID_size,
    testGraphProcessingChecksum_nv12_u8_fiveoutput_NULL,
    #ifndef x86_64
    testGraphProcessingGetPsaValues_diff_size,
    testGraphProcessingGetPsaValues_usr_data_obj_NULL,
    testErrorInterrupts_invalidCoeff,
    testErrorInterrupts_Processing_ERR_STATUS_NULL,
    testGraphProcessing_Processing_ERR_STATUS,
    #endif
    testGraphProcessing_FourOutput_Invalid_DstImg_YUYV,
    testGraphProcessing_FixedPattern_usr_obj_null,
    testNullUserDataSetCoeff,
    testGraphProcessing_FourOutput_Invalidformats,
    testGraphProcessing_InvalidCommand_FourOutputFormats,
    testGraphProcessing_Nv12Input_InvalidOutputFormat
    #if defined(VPAC3) || defined(VPAC3L)
    ,
    testNullUserData,
    testGraphProcessing_Nv12_Simul_Processing_STATUS_default,
    testKrSz6,
    testGraphProcessing_Diff_Bit_Depth_With_Invalid_Params,
    testGraphProcessing_Diff_Bit_Depth_InvalidOutputWidth,
    testImg0ycChroma,
    testImg0ycLUMACHROMA,
    testImg0ycLUMACHROMA_p12u16,
    testImg0ycLUMACHROMA_u8u16,
    testLumaLuma_diff_in0_in1_height,
    testLumaChroma_diff_in0_in1_height_conflict,
    testGraphProcessing_CntFalseCases 
    #endif
)
TESTCASE_TESTS(tivxHwaVpacMscScaleMultiOutputNegative2,
    testGraphProcessing_10bit_to_8bit_incorrectCoffConfig,
    testGraphProcessing_10bit_to_8bit_Invalid_interpolation,
    testGraphProcessingChecksum_nv12_u8_fiveoutput_ycmode_2,
    testGraphProcessing_TwoOutput_diff_outimg,
    testGraphProcessing_TwoOutput_outputDim_greaterThanInput,
    testGraphProcessing_ThreeOutput_incorrectOutputParamsSize,
    testGraphProcessing_ThreeOutput_outputHeight_greaterThanInput,
    testGraphProcessing_Five_UYVY_outputs,
    testGraphProcessing_FiveOutput_TriggerCntGt3Branch,
    testGraphProcessing_UYVY_input_Y_output_yuyv_uyvy,
    testGraphProcessing_Invalid_crop_prms_size
    #if defined(VPAC3) || defined(VPAC3L)
    ,
    testGraphProcessing_OutHeightvsInHeight_odd,
    testImg0ycLUMACHROMA_S16S16,
    testImg0ycLUMACHROMA_S16S16_enableprocessing0,
    testGraphProcessingChecksum_nv12_u8_fiveoutput_CHROMA, 
    testGraphProcessing_Diff_Bit_Depth_diff_inimg1,
    testGraphProcessing_OutFormatvsInFormat,
    testGraphProcessing_OutImageFormat,
    testGraphProcessing_OutImageFormat_U16,
    testGraphProcessing_OutImageFormat_outN12,
    testGraphProcessing_OutImageFormat_out_S16,
    testGraphProcessing_OutImageFormat_out_YUYV,
    testGraphProcessing_OutImageFormat_out_UYVY,
    testGraphProcessing_OutImageFormat_out_NV12,
    testGraphProcessing_OutImageFormat_out_img_2cnt_1,
    testGraphProcessing_OutImageFormat_out_S16_input2null
    #endif
)
#endif /* BUILD_VPAC_MSC */

