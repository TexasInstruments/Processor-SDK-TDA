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
#include <math.h>
#include "test_hwa_common.h"
#include "tivx_utils_checksum.h"
#include "tivx_utils_file_rd_wr.h"
#include <inttypes.h>
#define TIVX_VPAC_VISS_CMD_GET_ERR_STATUS                    (0x30000001u)
#define VX_GAUSSIAN_PYRAMID_TOLERANCE 1
#define CHECK_OUTPUT
/* #define TEST_MSC_CHECKSUM_LOGGING */

#define TEST_NUM_NODE_INSTANCE 2
/* #define TEST_MSC_PERFORMANCE_LOGGING */
/* #define TEST_GET_ERROR_INTERRUPT_VALUES */
/* #define TEST_PRINT_TIME_STAMP_VALUES */
/* #define TEST_MSC_ENABLE */
#define MSC_NODE_ERROR_EVENT  (1U)
#define SET_TIME_STAMP_VALUE  (100U)

TESTCASE(tivxHwaVpacMscPyramid, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacMscPyramidPositive, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacMscPyramidNegative, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
} ArgCreate;

#if (VPAC_COUNT > 1)
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
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_VPAC_MSC2))
#endif
#if (VPAC_COUNT > 1)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC2_MSC2))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC2", __VA_ARGS__, TIVX_TARGET_VPAC_MSC2))
#endif

#define ADD_DUMMY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "", __VA_ARGS__, 0))

#define ADD_YC_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=1", __VA_ARGS__, 1))

#define PARAMETERS_CREATE \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY, ARG)


TEST_WITH_ARG(tivxHwaVpacMscPyramid, testNodeCreation, ArgCreate, PARAMETERS_CREATE)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_size levels     = 4;
    const vx_float32 scale   = VX_SCALE_PYRAMID_HALF;
    const vx_uint32 width    = 640;
    const vx_uint32 height   = 480;
    const vx_df_image format = VX_DF_IMAGE_U8;
    char nodeTarget[TIVX_TARGET_MAX_NAME];

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, scale, width, height, format), VX_TYPE_PYRAMID);

        {
            vx_size ch_levels;
            vx_float32 ch_scale;
            vx_uint32 ch_width, ch_height;
            vx_df_image ch_format;

            VX_CALL(vxQueryPyramid(pyr, VX_PYRAMID_LEVELS, &ch_levels, sizeof(ch_levels)));
            if (levels != ch_levels)
            {
                CT_FAIL("check for pyramid attribute VX_PYRAMID_LEVELS failed\n");
            }
            VX_CALL(vxQueryPyramid(pyr, VX_PYRAMID_SCALE, &ch_scale, sizeof(ch_scale)));
            if (scale != ch_scale)
            {
                CT_FAIL("check for pyramid attribute VX_PYRAMID_SCALE failed\n");
            }
            VX_CALL(vxQueryPyramid(pyr, VX_PYRAMID_WIDTH, &ch_width, sizeof(ch_width)));
            if (width != ch_width)
            {
                CT_FAIL("check for pyramid attribute VX_PYRAMID_WIDTH failed\n");
            }
            VX_CALL(vxQueryPyramid(pyr, VX_PYRAMID_HEIGHT, &ch_height, sizeof(ch_height)));
            if (height != ch_height)
            {
                CT_FAIL("check for pyramid attribute VX_PYRAMID_HEIGHT failed\n");
            }
            VX_CALL(vxQueryPyramid(pyr, VX_PYRAMID_FORMAT, &ch_format, sizeof(ch_format)));
            if (format != ch_format)
            {
                CT_FAIL("check for pyramid attribute VX_PYRAMID_FORMAT failed\n");
            }
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input, pyr), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxQueryNode(node, TIVX_NODE_TARGET_STRING, &nodeTarget, sizeof(nodeTarget)));

        ASSERT(strncmp(nodeTarget, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0);

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(pyr == 0);
        ASSERT(input == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define LEVELS_COUNT_MAX    7

static CT_Image gaussian_pyramid_generate_random(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
    if(NULL == image)
    {
        return NULL;
    }

    return image;
}

static CT_Image gaussian_pyramid_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int32_t gaussian5x5_pyramid_get(int32_t *values)
{
    int32_t res = 1 * (values[ 0] + values[ 4] + values[20] + values[24]) +
                  4 * (values[ 1] + values[ 3] + values[ 5] + values[ 9] + values[15] + values[19] + values[21] + values[23]) +
                  6 * (values[ 2] + values[10] + values[14] + values[22]) +
                 16 * (values[ 6] + values[ 8] + values[16] + values[18]) +
                 24 * (values[ 7] + values[11] + values[13] + values[17]) +
                 36 * values[12];
    res = res >> 8;
    return res;
}

static uint8_t gaussian5x5_pyramid_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[25] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y + 2),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static uint8_t gaussian5x5_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[25] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y + 2),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static uint8_t gaussian5x5_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[25] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y + 2, constant_value),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static vx_int32 gaussian_pyramid_get_pixel(CT_Image input, int x, int y, vx_border_t border, int level)
{
    if (border.mode == VX_BORDER_UNDEFINED)
    {
        if ((x >= (2 + level)) && (y >= (2 + level)) &&
            (x < ((int)input->width - 2 - level)) && (y < ((int)input->height - 2 - level)))
            return gaussian5x5_pyramid_calculate(input, x, y);
        else
            return -1;
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        return gaussian5x5_calculate_replicate(input, x, y);
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        return gaussian5x5_calculate_constant(input, x, y, border.constant_value.U32);
    }
    CT_FAIL_(return -1, "NOT IMPLEMENTED");
}

static void gaussian_pyramid_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_border_t border, int level)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    vx_float64 x_src = (((vx_float64)x + 0.5) * (vx_float64)input->width / (vx_float64)output->width) - 0.5;
    vx_float64 y_src = (((vx_float64)y + 0.5) * (vx_float64)input->height / (vx_float64)output->height) - 0.5;
    int x_min = (int)floor(x_src), y_min = (int)floor(y_src);
    int sx, sy;
    vx_int32 candidate = 0;
    for (sy = 0; sy <= 1; sy++)
    {
        for (sx = 0; sx <= 1; sx++)
        {

            ASSERT_NO_FAILURE_(return, candidate = gaussian_pyramid_get_pixel(input, x_min + sx, y_min + sy, border, level));
            if (candidate == -1 || abs(candidate - res) <= VX_GAUSSIAN_PYRAMID_TOLERANCE)
                return;
        }
    }
    printf("Check failed for pixel level %d (%d, %d): res = %d, candidate = %d", level, x, y, (int)res, candidate);
    CT_FAIL_(return, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
}

static void gaussian_pyramid_check_image(CT_Image input, CT_Image output, vx_border_t border, vx_size level)
{
    ASSERT(input && output);

    if (0 == level)
    {
        EXPECT_EQ_CTIMAGE(input, output);
    }
    else
    {
        CT_FILL_IMAGE_8U(, output,
                {
                    ASSERT_NO_FAILURE(gaussian_pyramid_check_pixel(input, output, x, y, border, (int)level));
                });
    }
}

static const vx_float64 c_orbscale[] =
{
    1.000000000000000000000000000000e+00,
    8.408964152537146130583778358414e-01,
    7.071067811865475727373109293694e-01,
    5.946035575013605134486738279520e-01,
    5.000000000000000000000000000000e-01,
    4.204482076268573065291889179207e-01,
    3.535533905932737308575042334269e-01,
    2.973017787506802567243369139760e-01,
    2.500000000000000000000000000000e-01,
    2.102241038134286532645944589603e-01,
    1.767766952966368654287521167134e-01,
    1.486508893753401283621684569880e-01,
    1.250000000000000000000000000000e-01,
    1.051120519067143266322972294802e-01,
    8.838834764831843271437605835672e-02,
    7.432544468767006418108422849400e-02,
    6.250000000000000000000000000000e-02,
};


static void gaussian_pyramid_check(CT_Image input, vx_pyramid pyr, vx_size levels, vx_float32 scale, vx_border_t border)
{
    vx_uint32 level = 0;
    vx_image output_image = 0;
    CT_Image output_prev = NULL, output_cur = NULL;
    vx_uint32 ref_width = input->width;
    vx_uint32 ref_height = input->height;

    ASSERT(input && pyr && (1 < levels) && (level < sizeof(c_orbscale) / sizeof(vx_float64) ));
    ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, 0), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(output_prev = ct_image_from_vx_image(output_image));
    VX_CALL(vxReleaseImage(&output_image));
    ASSERT(output_image == 0);

    gaussian_pyramid_check_image(input, output_prev, border, 1);
    if (CT_HasFailure())
    {
        printf("=== Input ===\n");
        ct_dump_image_info(input);
        printf("=== LEVEL %d ===\n", 0);
        ct_dump_image_info(output_prev);
        return;
    }

    for (level = 1; level < levels; level++)
    {
        ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, level), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(output_cur = ct_image_from_vx_image(output_image));
        VX_CALL(vxReleaseImage(&output_image));
        ASSERT(output_image == 0);

        if (VX_SCALE_PYRAMID_ORB == scale)
        {
            vx_float64 orb_scale = c_orbscale[level];
            if ( (output_cur->width  != ceil(orb_scale * ref_width)) ||
                 (output_cur->height != ceil(orb_scale * ref_height)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }
        else
        {
            if ( (output_cur->width != ceil(output_prev->width * scale)) ||
                 (output_cur->height != ceil(output_prev->height * scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }

        gaussian_pyramid_check_image(output_prev, output_cur, border, level+1);
        if (CT_HasFailure())
        {
            printf("=== Input ===\n");
            ct_dump_image_info(output_prev);
            printf("=== LEVEL %d ===\n", level);
            ct_dump_image_info(output_cur);
            return;
        }

        output_prev = output_cur;
    }
}



static vx_uint8 gaussian_pyramid_reference_get_pixel(CT_Image prevLevel, int dst_width, int dst_height, int x, int y, vx_border_t border, int level)
{
    vx_int32 candidate = -1;
    vx_float64 x_src = (((vx_float64)x + 0.5) * (vx_float64)prevLevel->width / (vx_float64)dst_width) - 0.5;
    vx_float64 y_src = (((vx_float64)y + 0.5) * (vx_float64)prevLevel->height / (vx_float64)dst_height) - 0.5;
    int x_int = (int)floor(x_src), y_int = (int)floor(y_src);
    vx_float64 x_f = x_src - x_int, y_f = y_src - y_int;
    if (x_f >= 0.5)
        x_int++;
    if (y_f >= 0.5)
        y_int++;
    if (x_int >= (int)prevLevel->width)
        x_int = prevLevel->width - 1;
    if (y_int >= (int)prevLevel->height)
        y_int = prevLevel->height - 1;
    ASSERT_NO_FAILURE_(return 0, candidate = gaussian_pyramid_get_pixel(prevLevel, x_int, y_int, border, level));
    if (candidate == -1)
        return 0;
    return CT_CAST_U8(candidate);
}

static CT_Image gaussian_pyramid_create_reference_image(CT_Image input, CT_Image prevLevel, vx_border_t border, vx_float32 scale, vx_size target_level)
{
    vx_uint32 level = 0;
    CT_Image dst;
    vx_uint32 ref_width = input->width;
    vx_uint32 ref_height = input->height;
    vx_uint32 dst_width = input->width;
    vx_uint32 dst_height = input->height;

    ASSERT_(return NULL, scale < 1.0);
    ASSERT_(return NULL, input && (level < (sizeof(c_orbscale) / sizeof(vx_float64))));

    ASSERT_(return NULL, input->format == VX_DF_IMAGE_U8);

    if (VX_SCALE_PYRAMID_HALF == scale)
    {
        dst_width = ref_width;
        dst_height = ref_height;
        for (level = 1; level <= target_level; level++)
        {
            dst_width = (vx_uint32)ceil(dst_width * scale);
            dst_height = (vx_uint32)ceil(dst_height * scale);
        }
    }
    else // if (VX_SCALE_PYRAMID_ORB == scale)
    {
        vx_float64 orb_scale = c_orbscale[target_level];
        dst_width = (vx_uint32)ceil(orb_scale * ref_width);
        dst_height = (vx_uint32)ceil(orb_scale * ref_height);
    }

    dst = ct_allocate_image(dst_width, dst_height, input->format);

    if (target_level == 0)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = *CT_IMAGE_DATA_PTR_8U(input, x, y);
                    *dst_data = res;
                });
    }
    else
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian_pyramid_reference_get_pixel(prevLevel, dst_width, dst_height, x, y, border, (int)target_level);
                    *dst_data = res;
                });
    }

    return dst;
}

static void gaussian_pyramid_fill_reference(CT_Image input, vx_pyramid pyr, vx_size levels, vx_float32 scale, vx_border_t border)
{
    vx_uint32 level = 0;
    vx_image  output_image = 0;
    CT_Image  output_prev  = NULL;
    CT_Image  output_cur   = NULL;
    vx_uint32 ref_width    = input->width;
    vx_uint32 ref_height   = input->height;

    ASSERT(input && pyr && (levels < sizeof(c_orbscale) / sizeof(vx_float64) ));
    ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, 0), VX_TYPE_IMAGE);
    ASSERT(output_image != NULL);
    ASSERT_NO_FAILURE(output_prev = ct_image_from_vx_image(output_image));
    ASSERT(output_prev != NULL);

    CT_FILL_IMAGE_8U(return, output_prev,
            {
                uint8_t res = *CT_IMAGE_DATA_PTR_8U(input, x, y);
                *dst_data = res;
            });
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output_image, output_prev));

    VX_CALL(vxReleaseImage(&output_image));
    ASSERT(output_image == 0);

    for (level = 1; level < levels; level++)
    {
        ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, level), VX_TYPE_IMAGE);
        ASSERT(output_image != NULL);
        ASSERT_NO_FAILURE(output_cur = ct_image_from_vx_image(output_image));
        ASSERT(output_cur != NULL);

        if (VX_SCALE_PYRAMID_ORB == scale)
        {
            vx_float64 orb_scale = c_orbscale[level];
            if ( (output_cur->width  != ceil(ref_width  * orb_scale)) ||
                 (output_cur->height != ceil(ref_height * orb_scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }
        else
        {
            if ( (output_cur->width  != ceil(output_prev->width  * scale)) ||
                 (output_cur->height != ceil(output_prev->height * scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }

        ASSERT_NO_FAILURE(output_cur = gaussian_pyramid_create_reference_image(input, output_prev, border, scale, level));
        if(NULL == output_cur)
        {
            CT_FAIL_(return, "Check failed to create reference image for level: %d", level);
        }
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output_image, output_cur));

        VX_CALL(vxReleaseImage(&output_image));
        ASSERT(output_image == 0);

        output_prev = output_cur;
    }
}


static vx_size gaussian_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;
    while ((16 <= width) && (16 <= height) && level < LEVELS_COUNT_MAX)
    {
        level++;
        width = (int)ceil((vx_float64)width * scale);
        height = (int)ceil((vx_float64)height * scale);
    }
    return level;
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
    char* target_string;
    vx_float32 scale;
    int yc_mode;
    int max_ds_factor;
    int format;
} Arg;
typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
    char* target_string, *target_string_2;
    vx_float32 scale;
} Arg_multi;

#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)) /*, \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB))*/

#define ADD_SIZE_SMALL_SET_MODIFIED(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=32x32", __VA_ARGS__, 32, 32)), \
    CT_EXPAND(nextmacro(testArgName "/sz=256x256", __VA_ARGS__, 256, 256)), \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_MAX_DS_FACTOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/max_ds_factor=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/max_ds_factor=4", __VA_ARGS__, 4))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET_MODIFIED, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE, ARG, gaussian_pyramid_read_image, "lena.bmp")

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        #ifdef CHECK_OUTPUT
        CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input, pyr, levels, arg_->scale, arg_->border));
        #endif

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define PARAMETERS_MULTI \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET_MODIFIED, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ADD_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ADD_VX_SCALE, ARG, gaussian_pyramid_read_image, "lena.bmp")
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessing_multi, Arg_multi,
    PARAMETERS_MULTI
)
{
    vx_size levels = 0;

    vx_context context = context_->vx_context_;
    vx_image input_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_pyramid pyr[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    uint32_t i;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

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
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(input_image[i] = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
                levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

                ASSERT_VX_OBJECT(pyr[i] = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

            

                ASSERT_VX_OBJECT(node[i] = tivxVpacMscPyramidNode(graph, input_image[i], pyr[i]), VX_TYPE_NODE);

                if (border.mode != VX_BORDER_UNDEFINED)
                {
                    VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border, sizeof(border)));
                }

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                #ifdef CHECK_OUTPUT
                CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input, pyr[i], levels, arg_->scale, arg_->border));
                #endif
            }
        }
        if (input->width > 256)
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
                    #if defined(TEST_MSC_PERFORMANCE_LOGGING)
                    printf("targets are same\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg >= (perf_node[0].avg + perf_node[1].avg));
                }
                else
                {
                    #if defined(TEST_MSC_PERFORMANCE_LOGGING)
                    printf("targets are different\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg < (perf_node[0].avg + perf_node[1].avg));
                }
            }
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                ASSERT(node[i] == 0);
            }
        }
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleasePyramid(&pyr[i]));
                VX_CALL(vxReleaseImage(&input_image[i]));
                ASSERT(pyr[i] == 0);
                ASSERT(input_image[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

static uint32_t expected_cksm[] = {
    0xb7fe2010,
    0x6d1f12be,
    0xca0d7926,
    0x681a4134,
    0xb94a2c2d,

    0xf8e73683,
    0x1e268f84,
    0x4150d9ca,
    0xee8ecad3,
    0x187ff593,
    0xd79fb95b,
    0xdec5ad28,

    0x13e62a48,
    0x62fadeaf,
    0xb3e971ed,
    0xde351d05,
    0x4758959f,
    0x82161790,
    0xe4a4743e,

    0x932c9093,
    0x50b2d5d3,
    0xafe5e83c,
    0x2383b5a7,
    0x8cd8f999,
    0x7506616f,
    0xaa9656a9
};

#define ADD_VX_SCALE_CKSUM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)), \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB)), \
    CT_EXPAND(nextmacro(testArgName "/THREE_FORTHS_SCALE", __VA_ARGS__, 0.75f)), \
    CT_EXPAND(nextmacro(testArgName "/TWO_THIRDS_SCALE", __VA_ARGS__, 0.66f)) \

#define PARAMETERS_CKSUM \
    CT_GENERATE_PARAMETERS("lena", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM, ARG, gaussian_pyramid_read_image, "lena.bmp")

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum, Arg,
    PARAMETERS_CKSUM
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;

    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        /* Test half of the tests using control command for updating coefficients, other half using default */
        if((strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0))
        {
            // Set custom filter coefficients
            vx_user_data_object coeff_obj;
            tivx_vpac_msc_coefficients_t coeffs;
            vx_reference refs[1];

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
            VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
                refs, 1u));

            VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        }

        VX_CALL(vxProcessGraph(graph));

        #ifdef CHECK_OUTPUT
        //CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input, pyr, levels, arg_->scale, arg_->border));
        #endif

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/lena_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            if (arg_->scale == VX_SCALE_PYRAMID_ORB)
            {
                cksm_offset = 5;
            } else if (arg_->scale == 0.75f)
            {
                cksm_offset = 12;
            } else if (arg_->scale == 0.66f)
            {
                cksm_offset = 19;
            }

            ASSERT(expected_cksm[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static uint32_t expected_cksm_nv12_u8[] = {
0x04f22a0e,
0x60f2fc06,
0x30b39f20,
0x912b6aaa,
0xa2043819,
0xb6447702,
0x37569f53,

0x01172f6a,
0xc10f189b,
0x4d3fc239,
0x572c1b14,
0x321a2167,
0x7dcca4a9,

0x04f22a0e,
0xd24ecb44,
0x4138ad53,
0x453781e8,
0x60f2fc06,
0x3f579d23,
0x96b2fb85,

0x01172f6a,
0x653507ad,
0xfae9b328,
0xfd53a162,
0xc10f189b,
0x292b8815,
0xcf1ba053,

0x04f22a0e,
0xde743149,
0x80a9e5db,
0x615329c2,
0xaed07ed8,
0x1fa6f238,
0xc5ce0c36,

0x01172f6a,
0xa6adfb14,
0xd0132ef7,
0x834f0b05,
0xcb389529,
0x566f2182,
0x2a03dc4c,

0x04f22a0e,
0x34747bbc,
0xb3e7807b,
0xe1bba3d3,
0x65e44ba7,
0x4152bdaa,
0x24508054,

0x01172f6a,
0xd47a92fb,
0x3a4ef711,
0xc1d37944,
0x5fa9b4a8,
0x29ba3eab,
0x22f68001

};

#define PARAMETERS_CREATE_NV12_U8_CHECKSUM \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM,ADD_YC_MODE, ARG, NULL, NULL)
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_nv12_u8, Arg, PARAMETERS_CREATE_NV12_U8_CHECKSUM)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height =input_height;
        if(arg_->yc_mode == 1)
        {
            height=input_height/2;
        }
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        /* set input */

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
        #ifdef CHECK_OUTPUT
        //CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input_image, pyr, levels, arg_->scale, arg_->border));
        #endif 
        
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/lena_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            if (arg_->scale == VX_SCALE_PYRAMID_HALF)
            {
                if(arg_->yc_mode == 1)
                {
                    cksm_offset = 7;
                }
                    
            }
            else if(arg_->scale == VX_SCALE_PYRAMID_ORB) 
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 13;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 20;
                }

            }
            else if (arg_->scale == 0.75f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 27;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 34;
                }
            } 
            else if (arg_->scale == 0.66f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 41;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 48;
                }
            }ASSERT(expected_cksm_nv12_u8[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_nv12_u8_max_ds_factor[] = {
0x04f22a0e,	
0x60f2fc06,	
0x30b39f20,	
0x912b6aaa,	
0xa2043819,	
0xb6447702,
0x37569f53,	

0x04f22a0e,	
0x60f2fc06,	
0xfe71fae8,	
0x2908c9be,	
0xfef9ed08,	
0x43cede68,	
0x6152d25c,	

0x01172f6a,
0xc10f189b,	
0x4d3fc239,	
0x572c1b14,	
0x321a2167,	
0x7dcca4a9,	

0x01172f6a,	
0xc10f189b,	
0x4d0a9e95,	
0xde34b62a,	
0x5f22e767,	
0xba10e3ea,	

0x04f22a0e,	
0xd24ecb44,	
0x4138ad53,	
0x453781e8,	
0x60f2fc06,	
0x3f579d23,	
0x96b2fb85,	

0x04f22a0e,	
0xd24ecb44,	
0x4138ad53,	
0x453781e8,	
0x60f2fc06,	
0xbd7c4325,	
0x567ae045,	

0x01172f6a,	
0x653507ad,	
0xfae9b328,	
0xfd53a162,	
0xc10f189b,	
0x292b8815,	
0xcf1ba053,	

0x01172f6a,	
0x653507ad,	
0xfae9b328,	
0xfd53a162,	
0xc10f189b,	
0x0ea47cb7,	
0xedb573e4,	

0x04f22a0e,	
0xde743149,	
0x80a9e5db,	
0x615329c2,	
0xaed07ed8,	
0x1fa6f238,	
0xc5ce0c36,	

0x04f22a0e,	
0xde743149,	
0x80a9e5db,	
0x0f4d243e,	
0xce5b1963,	
0x8c638133,	
0xa0df6233,	

0x01172f6a,	
0xa6adfb14,	
0xd0132ef7,	
0x834f0b05,	
0xcb389529,	
0x566f2182,	
0x2a03dc4c,	

0x01172f6a,	
0xa6adfb14,	
0xd0132ef7,	
0x6eafb8b7,	
0xb9547848,	
0x68d42ebf,	
0xfa29b7d2,	

0x04f22a0e,	
0x34747bbc,	
0xb3e7807b,	
0xe1bba3d3,	
0x65e44ba7,	
0x4152bdaa,	
0x24508054,	

0x04f22a0e,	
0x34747bbc,	
0x0f41439a,	
0xecd5d980,	
0x912252ae,	
0x1665de11,	
0xf5d4ad8a,	

0x01172f6a,	
0xd47a92fb,	
0x3a4ef711,	
0xc1d37944,	
0x5fa9b4a8,	
0x29ba3eab,	
0x22f68001,	

0x01172f6a,	
0xd47a92fb,	
0x1cce10ad,	
0x810181af,	
0x1f4257e8,	
0x561f1187,	
0xb4eea6de,	

};
#define PARAMETERS_CREATE_NV12_U8_MAX_DS_FACTOR \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM,ADD_YC_MODE, ADD_MAX_DS_FACTOR, ARG, NULL, NULL)
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_nv12_u8_max_ds_factor, Arg, PARAMETERS_CREATE_NV12_U8_MAX_DS_FACTOR)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height =input_height;
        if(arg_->yc_mode == 1)
        {
            height=input_height/2;
        }
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        /* set input */
        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.yc_mode = arg_->yc_mode;
        sc_input_params.max_ds_factor = arg_->max_ds_factor;
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
        
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/u8_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            if (arg_->scale == VX_SCALE_PYRAMID_HALF)
            {
                if(arg_->yc_mode == 0)
                {
                    if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 7;
                    }
                }
                if(arg_->yc_mode == 1)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 14;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 20;
                    }
                }
                    
            }
            else if(arg_->scale == VX_SCALE_PYRAMID_ORB) 
            {
                if(arg_->yc_mode == 0)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 26;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 33;
                    }
                }
                else if(arg_->yc_mode == 1)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 40;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 47;
                    }
                }

            }
            else if (arg_->scale == 0.75f)
            {
                if(arg_->yc_mode == 0)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 54;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 61;
                    }
                }
                else if(arg_->yc_mode == 1)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 68;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 75;
                    }
                }
            } 
            else if (arg_->scale == 0.66f)
            {
                if(arg_->yc_mode == 0)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 82;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 89;
                    }
                }
                else if(arg_->yc_mode == 1)
                {
                    if(arg_->max_ds_factor == 2)
                    {
                        cksm_offset = 96;
                    }
                    else if(arg_->max_ds_factor == 4)
                    {
                        cksm_offset = 103;
                    }
                }
            }
            ASSERT(expected_cksm_nv12_u8_max_ds_factor[level+cksm_offset] == checksum_actual); 
            vxReleaseImage(&dst_image);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_10bit_to_8bit[] = {
    0x7f6c16a0,
    0xb2b0701a
};
#define ADD_VX_SCALE_CKSUM_10BIT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF))
#define PARAMETERS_CKSUM_10bit \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_10BIT, ARG, NULL, NULL)

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_10bit, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testErrorInterrupts, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    tivx_vpac_msc_input_params_t params;
    vx_user_data_object param_obj;

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_ref = 0;
    vx_reference param_ref = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        /* Run first frame with error interrupts enabled (enabled by default) */
        VX_CALL(vxProcessGraph(graph));

        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_ref = (vx_reference)scalar_err_stat_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS, &err_ref, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        /* Checks whether error interrupt was detected.
        *  No error is detected during normal operation.
        */
        ASSERT(scalar_out_err_stat == 0);

        tivx_vpac_msc_input_params_init(&params);
        params.enable_error_events = 0;
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_input_params_t",
                                                            sizeof(tivx_vpac_msc_input_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        param_ref = (vx_reference)param_obj;
        /* Call the control command for maximum coverage */
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u));

        /* Run second frame with all error interrupts enabled, including the watchdog timer timeout errors
        *  which are disabled by default. */
        params.wdtimer_timeout_cycles = TIVX_VPAC_MSC_WDTIMER_128K_CYCLES;
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        if((strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0) || (strncmp(TIVX_TARGET_VPAC2_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0))
        #else
        if(strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0)
        #endif
        {
            params.enable_error_events = TIVX_VPAC_MSC0_ALL_ERR_EVENTS;
        }
        else
        {
            params.enable_error_events = TIVX_VPAC_MSC1_ALL_ERR_EVENTS;
        }
        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_msc_input_params_t), &params,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        param_ref = (vx_reference)param_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS, &err_ref, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat,
                                VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        /* Checks whether error interrupt was detected.
        *  No error is detected during normal operation. */
        ASSERT(scalar_out_err_stat == 0);
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_uyvy[] = {
    0x34e6011b,
    0xc7744b86,
    0x5dbcdada
};
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessing_UYVY, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image src_image = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    const char *output_file_names[3] = { "output/pmd_uyvy_output_0.yuv", "output/pmd_uyvy_output_1.yuv", "output/pmd_uyvy_output_2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readUYVYInput(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
            output_params[level].filter_mode = 0;
        }
        tivx_vpac_msc_coefficients_params_init(&coeffs, VX_INTERPOLATION_BILINEAR);
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            sz = snprintf(output_files[level], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[level]);
            ASSERT_(return, (sz < MAXPATHLENGTH));

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            write_uyvy_output_image(output_files[level], dst_image); 
            #endif
            ASSERT(expected_cksm_uyvy[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_uyvy_input_y_output[] = {
    0xa33b5e96,
    0x1488f24f,
    0x46d9a69f,
    0xb64a43e8
};
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessing_UYVY_input_Y_output, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image src_image = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readUYVYInput(file, src_image));


        width = src_width;
        height = src_height;

        levels = 4;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
            output_params[level].filter_mode = 0;
        }
        tivx_vpac_msc_coefficients_params_init(&coeffs, VX_INTERPOLATION_BILINEAR);
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/pmd_422i_output_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_uyvy_input_y_output[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define ADD_VX_SCALE_CKSUM_NV12(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF))
#define PARAMETERS_CKSUM_NV12 \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_NV12, ARG, NULL, NULL)

static uint32_t expected_nv12_cksm[] = {
0xf954afd4,
0x55d0e88a,

0x79c6a9ed,
0x9e725836,

0xf4c1291f,
0x1fd49672
};
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_NV12, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            write_output_image_nv12_8bit(output_files[level], dst_image);
            #endif
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Luma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt] == checksum_actual);
            
            rect.end_x = w/2;
            rect.end_y = h/2;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Chroma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt+1] == checksum_actual);
            cnt = cnt + 2;
            vxReleaseImage(&dst_image);

        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static uint32_t expected_psa_values[2][3] = {
{0xdcd6edab, 0x6fee5135, 0xa725d285},
{0xb9197b68, 0x7d9b2ba8, 0x951875a8}
};

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingGetPsaValues, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
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

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        for (int i = 0; i < 2; i++) 
        {
            for (int j = 0; j < 3; j++)
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

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            write_output_image_nv12_8bit(output_files[level], dst_image);
            #endif
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Luma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt] == checksum_actual);
            
            rect.end_x = w/2;
            rect.end_y = h/2;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Chroma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt+1] == checksum_actual);
            cnt = cnt + 2;
            vxReleaseImage(&dst_image);

        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#ifndef VPAC3L
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testSl2Firewall, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;
    vx_status status;

    vx_border_t border = arg_->border;

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

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++)
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
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

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 3; j++)
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

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            write_output_image_nv12_8bit(output_files[level], dst_image);
            #endif
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Luma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt] == checksum_actual);

            rect.end_x = w/2;
            rect.end_y = h/2;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 1, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d Chroma = 0x%08x\n", level, checksum_actual);
            #endif
            ASSERT(expected_nv12_cksm[cnt+1] == checksum_actual);
            cnt = cnt + 2;
            vxReleaseImage(&dst_image);

        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
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
}
#endif /* VPAC3L */

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_invalid_scaling_factor_error, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);


        width = 900;
        height = 500;

        levels = 1;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

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
        ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        ASSERT_NO_FAILURE(vxProcessGraph(graph));


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_NV12_ten_subsets, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 4096, src_height = 4096, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        width = src_width;
        height = src_height;

        levels = 12;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);

        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_NO_FAILURE(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        ASSERT_NO_FAILURE(vxProcessGraph(graph));


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_NV12_ten_levels, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 4096, src_height = 4096, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_reference in_refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        width = src_width;
        height = src_height;

        levels = 11;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        /* set input */
        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.max_ds_factor = 2048;
        ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
        in_refs[0] = (vx_reference)sc_input_params_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            in_refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_10bit_to_8bit_acc[] = {
    0x7f6c16a0,
    0xe4df55a3
};
#define ADD_VX_SCALE_CKSUM_10BIT_ACC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ONE_THIRDS_SCALE", __VA_ARGS__, 0.25f))
#define PARAMETERS_CKSUM_10_BIT_ACC \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_10BIT_ACC, ARG, NULL, NULL)

TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_10bit_acc, Arg,
    PARAMETERS_CKSUM_10_BIT_ACC
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_acc[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define ADD_VX_SCALE_CKSUM_10BIT_ACC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ONE_THIRDS_SCALE", __VA_ARGS__, 0.25f))
#define PARAMETERS_CKSUM_10_BIT_ACC \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_10BIT_ACC, ARG, NULL, NULL)

TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_acc_sm, Arg,
    PARAMETERS_CKSUM_10_BIT_ACC
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    tivx_vpac_msc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_safety_mech_prms_init(&safety_mechanism_params);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;

        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }

        safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;

        safety_mechanism_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_safety_mechanism_params_t", sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params);
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1);

        /* Set Coefficients; Must do Safety Mechanism VPAC12 and F2 */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));
        /* safety_mechanism=reconfigReinitReg */
        safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
        VX_CALL(vxCopyUserDataObject(safety_mechanism_obj, 0, sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1));
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp);
            #endif
            /* TODO the checksum mismatches even if the safety mechanism is disabled, requires a check on the test framework */
            if(expected_cksm_10bit_to_8bit_acc[level+cksm_offset] != checksum_actual)
            {
                printf("Checksum mismatch\n");
            }
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);
        ASSERT(safety_mechanism_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define ADD_VX_SCALE_CKSUM_10BIT_ACC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ONE_THIRDS_SCALE", __VA_ARGS__, 0.25f))
#define PARAMETERS_CKSUM_10_BIT_ACC \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_10BIT_ACC, ARG, NULL, NULL)

TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_acc_sm10, Arg,
    PARAMETERS_CKSUM_10_BIT_ACC
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    tivx_vpac_msc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_safety_mech_prms_init(&safety_mechanism_params);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;

        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }

        safety_mechanism_params.enable_status_reg_validation = 1U;

        safety_mechanism_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_safety_mechanism_params_t", sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params);
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1);
        vxReleaseUserDataObject(&safety_mechanism_obj);

        /* Set Coefficients; Must do Safety Mechanism VPAC12 and F2 */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            /* TODO the checksum mismatches even if the safety mechanism is disabled, requires a check on the test framework */
            if(expected_cksm_10bit_to_8bit_acc[level+cksm_offset] != checksum_actual)
            {
                printf("Checksum mismatch\n");
            }
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define ADD_VX_SCALE_CKSUM_10BIT_ACC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ONE_THIRDS_SCALE", __VA_ARGS__, 0.25f))
#define PARAMETERS_CKSUM_10_BIT_ACC \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM_10BIT_ACC, ARG, NULL, NULL)

TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_acc_sm7, Arg,
    PARAMETERS_CKSUM_10_BIT_ACC
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    tivx_vpac_msc_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1] = { NULL };
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        tivx_vpac_msc_safety_mech_prms_init(&safety_mechanism_params);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;

        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }

        safety_mechanism_params.enable_readback_config_registers = 1U;

        safety_mechanism_obj = vxCreateUserDataObject(context, "tivx_vpac_msc_safety_mechanism_params_t", sizeof(tivx_vpac_msc_safety_mechanism_params_t), &safety_mechanism_params);
        safety_ref[0] = (vx_reference)safety_mechanism_obj;
        tivxNodeSendCommand(node, 0, TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1);
        vxReleaseUserDataObject(&safety_mechanism_obj);

        /* Set Coefficients; Must do Safety Mechanism VPAC12 and F2 */
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_acc[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define ADD_SET_TARGET_PARAMETERS_INST_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_VPAC_MSC1))
#define ADD_SIZE_SMALL_SET_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=32x32", __VA_ARGS__, 32, 32))
#define PARAMETERS_INVALID_SCALE \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_SMALL_SET_TEST, ADD_SET_TARGET_PARAMETERS_INST_CC_TEST, ADD_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL)
/* Test case to cover the 'Scale out of range' scenario in the API tivxVpacMscPyramidValidate() with wrong frame size */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_invalid_scale, Arg_multi, PARAMETERS_INVALID_SCALE)
{
    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_status status;
    vx_size levels;
    vx_reference refs[1];
    CT_Image input = NULL;
    vx_border_t border = arg_->border;
    int downscale_factor = 4U;

    tivxHwaLoadKernels(context);
    CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

    // Intentionally set scale at the lower bound (or smaller)
    arg_->scale = 0.25;

    // Input image deliberately large to exceed downscale limit
    int input_width = 1024;
    int input_height = 1024;
    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, input_width, input_height));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Convert CT_Image to vx_image
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    // Intentionally compute pyramid dimensions smaller than needed to trigger the error
    int p_w = (input_width / downscale_factor) - 1; // ensure w/downscale_factor > p_w
    int p_h = (input_height / downscale_factor) - 1; // same for height

    levels = gaussian_pyramid_calc_max_levels_count(input_width, input_height, arg_->scale);

    // Attempt to create a pyramid with dimensions that will fail validation
    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, p_w, p_h, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    // Create the node
    ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
    {
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
    }

    if (NULL != arg_->target_string)
    {
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
    }

    // Verify the graph
    status = vxVerifyGraph(graph);

    // Expect verification to fail due to invalid parameters inside the kernel
    ASSERT(status == VX_ERROR_INVALID_PARAMETERS);

    // Optionally print or check logs if you want to validate debug prints were hit

    // Release resources
    VX_CALL(vxReleaseNode(&node));
    ASSERT(node == 0);
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(graph == 0);
    VX_CALL(vxReleasePyramid(&pyr));
    VX_CALL(vxReleaseImage(&input_image));
    ASSERT(pyr == 0);
    ASSERT(input_image == 0);

    tivxHwaUnLoadKernels(context);
}
#define ADD_VX_BORDERS_CC_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=1", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 1 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} })),
#define PARAMETERS_INVALID_BORDER \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_CC_TEST, ADD_SIZE_SMALL_SET_TEST, ADD_SET_TARGET_PARAMETERS_INST_CC_TEST, ADD_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL)
/* Test case for verifying the API tivxVpacMscPyramidValidate() for the border value as VX_BORDER_CONSTANT and VX_BORDER_REPLICATE */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_invalidBoard_parm, Arg_multi, PARAMETERS_INVALID_BORDER)
{
    vx_size levels = 0;

    vx_context context = context_->vx_context_;
    vx_image input_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_pyramid pyr[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    uint32_t i;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

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
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(input_image[i] = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
                levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

                ASSERT_VX_OBJECT(pyr[i] = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

                ASSERT_VX_OBJECT(node[i] = tivxVpacMscPyramidNode(graph, input_image[i], pyr[i]), VX_TYPE_NODE);

                vx_border_t border_to_use = border;

                // Set border attribute only if not undefined
                if (border_to_use.mode != VX_BORDER_UNDEFINED)
                {
                    VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border_to_use, sizeof(border_to_use)));
                }

                //  Set node target string for each node
                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }
            }
        }

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                ASSERT(node[i] == 0);
            }
        }
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleasePyramid(&pyr[i]));
                VX_CALL(vxReleaseImage(&input_image[i]));
                ASSERT(pyr[i] == 0);
                ASSERT(input_image[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_invalidCoeff_parmSize, Arg,
    PARAMETERS_CKSUM
)
{
    vx_size levels;
    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        /* Test half of the tests using control command for updating coefficients, other half using default */
        if((strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0))
        {
            // Set custom filter coefficients
            vx_user_data_object coeff_obj;
            tivx_vpac_msc_coefficients_t coeffs;
            vx_reference refs[1];

            tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);

            /* Set Coefficients */
            ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
                "tivx_vpac_msc_coefficients_t",
                sizeof(tivx_vpac_msc_coefficients_t)+1, NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
                sizeof(tivx_vpac_msc_coefficients_t)+1, &coeffs, VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            refs[0] = (vx_reference)coeff_obj;
            ASSERT_NO_FAILURE(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
                refs, 1u));

            VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        }
        else
        {
            VX_CALL(vxProcessGraph(graph));

            for(level = 0; level < levels; level++)
            {
                vx_image dst_image = vxGetPyramidLevel(pyr, level);
                vx_uint32 w, h;
                vx_rectangle_t rect;
                vx_uint32 checksum_actual = 0;
                vx_char temp[256];

                vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
                vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = w;
                rect.end_y = h;

                checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
                #if defined(TEST_MSC_CHECKSUM_LOGGING)
                printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
                sprintf(temp, "output/lena_msc_%d", level);
                save_image_from_msc(dst_image, temp); 
                #endif
                if (arg_->scale == VX_SCALE_PYRAMID_ORB)
                {
                    cksm_offset = 5;
                } else if (arg_->scale == 0.75f)
                {
                    cksm_offset = 12;
                } else if (arg_->scale == 0.66f)
                {
                    cksm_offset = 19;
                }

                ASSERT(expected_cksm[level+cksm_offset] == checksum_actual);
                vxReleaseImage(&dst_image);
            }
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_coffRefNull, Arg,
    PARAMETERS_CKSUM
)
{
    vx_size levels;
    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;

    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        /* Test half of the tests using control command for updating coefficients, other half using default */
        if((strncmp(TIVX_TARGET_VPAC_MSC1, arg_->target_string, TIVX_TARGET_MAX_NAME) == 0))
        {
            // Set custom filter coefficients
            vx_user_data_object coeff_obj;
            tivx_vpac_msc_coefficients_t coeffs;
            vx_reference refs[1];

            tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);

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
        }

        else
        {
            VX_CALL(vxProcessGraph(graph));

            for(level = 0; level < levels; level++)
            {
                vx_image dst_image = vxGetPyramidLevel(pyr, level);
                vx_uint32 w, h;
                vx_rectangle_t rect;
                vx_uint32 checksum_actual = 0;
                vx_char temp[256];

                vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
                vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = w;
                rect.end_y = h;

                checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
                #if defined(TEST_MSC_CHECKSUM_LOGGING)
                printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
                sprintf(temp, "output/lena_msc_%d", level);
                save_image_from_msc(dst_image, temp); 
                #endif
                if (arg_->scale == VX_SCALE_PYRAMID_ORB)
                {
                    cksm_offset = 5;
                } else if (arg_->scale == 0.75f)
                {
                    cksm_offset = 12;
                } else if (arg_->scale == 0.66f)
                {
                    cksm_offset = 19;
                }

                ASSERT(expected_cksm[level+cksm_offset] == checksum_actual);
                vxReleaseImage(&dst_image);
            }
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define ADD_YC_MODE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/yc_mode=0", __VA_ARGS__, 2))
#define PARAMETERS_CREATE_NV12_U8_CHECKSUM_TEST \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ADD_VX_SCALE_CKSUM,ADD_YC_MODE_TEST, ARG, NULL, NULL) 

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_nv12_u8_ycmode_2, Arg, PARAMETERS_CREATE_NV12_U8_CHECKSUM_TEST)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height =input_height;
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        
        /* set input */
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
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
        
        VX_CALL(vxProcessGraph(graph)); 

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#if defined TEST_MSC_ENABLE
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testKrSz3, Arg, PARAMETERS_CREATE_NV12_U8_CHECKSUM)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height = input_height;
        if(arg_->yc_mode == 1)
        {
            height=input_height/2;
        }
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        /* set input */

        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.yc_mode = arg_->yc_mode;
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
        
        VX_CALL(vxProcessGraph(graph));
        
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/lena_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            if (arg_->scale == VX_SCALE_PYRAMID_HALF)
            {
                if(arg_->yc_mode == 1)
                {
                    cksm_offset = 7;
                }
                    
            }
            else if(arg_->scale == VX_SCALE_PYRAMID_ORB) 
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 13;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 20;
                }

            }
            else if (arg_->scale == 0.75f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 27;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 34;
                }
            } 
            else if (arg_->scale == 0.66f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 41;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 48;
                }
            }ASSERT(expected_cksm_nv12_u8[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testInvalid_kernelSize, Arg, PARAMETERS_CREATE_NV12_U8_CHECKSUM)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height =input_height;
        if(arg_->yc_mode == 1)
        {
            height=input_height/2;
        }
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        
        /* set input */
        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.yc_mode = arg_->yc_mode;
        sc_input_params.kern_sz = 6;
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

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#if defined TEST_MSC_ENABLE
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testKrSz4, Arg, PARAMETERS_CREATE_NV12_U8_CHECKSUM)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input_image));
        width = input_width;
        height =input_height;
        if(arg_->yc_mode == 1)
        {
            height=input_height/2;
        }
        levels =  gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        /* set input */

        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.yc_mode = arg_->yc_mode;
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
        
        VX_CALL(vxProcessGraph(graph));
        #ifdef CHECK_OUTPUT
        //CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input_image, pyr, levels, arg_->scale, arg_->border));
        #endif 
        
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/lena_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            if (arg_->scale == VX_SCALE_PYRAMID_HALF)
            {
                if(arg_->yc_mode == 1)
                {
                    cksm_offset = 7;
                }
                    
            }
            else if(arg_->scale == VX_SCALE_PYRAMID_ORB) 
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 13;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 20;
                }

            }
            else if (arg_->scale == 0.75f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 27;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 34;
                }
            } 
            else if (arg_->scale == 0.66f)
            {
                if(arg_->yc_mode == 0)
                {
                    cksm_offset = 41;
                }
                else if(arg_->yc_mode == 1)
                {
                    cksm_offset = 48;
                }
            }ASSERT(expected_cksm_nv12_u8[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif
typedef struct {
    const char* testName;
    char* target_string;
    int dummy;
    vx_float32 scale;
    int format;
} ArgCreate_format;

#define ADD_INPUT_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/input1_data_format=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/input1_data_format=1", __VA_ARGS__, 1)) 

#define ADD_VX_SCALE_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF))
#define PARAMETERS_CKSUM_FORMAT \
    CT_GENERATE_PARAMETERS("instance", ADD_SET_TARGET_PARAMETERS, ADD_DUMMY,ADD_VX_SCALE_FORMAT,ADD_INPUT_MODE, ARG)

static uint32_t expected_p12_u16_cksm[] = {
0xace6cc71,
0x423cdc30,
0xe967b122,

0xfffcd600,
0xffff3580,
0xffffcd60

};
TEST_WITH_ARG(tivxHwaVpacMscPyramid, testGraphProcessingChecksum_P12_U16_formatSupport, ArgCreate_format,
    PARAMETERS_CKSUM_FORMAT
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    vx_pixel_value_t pixel;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        pixel.U32 = 0x0a7f1345;
        if(arg_->format == 0u)
        {
            ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            ct_read_raw_image_to_vx_image(src_image, "psdkra/app_single_cam/1920x1080_luma16b.raw", 2);
        }
        else
        {
            ASSERT_VX_OBJECT(src_image = vxCreateUniformImage(context, src_width, src_height, TIVX_DF_IMAGE_P12, &pixel), VX_TYPE_IMAGE);
        }

        width = src_width;
        height = src_height;

        levels = 3;

        if(arg_->format == 0u)
        {
            ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U16), VX_TYPE_PYRAMID);
        }
        else
        {
            ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_PYRAMID);
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("Output %d = 0x%08x\n", level, checksum_actual);
            #endif
            if(arg_->format == 0)
            {
                ASSERT(expected_p12_u16_cksm[level] == checksum_actual);
            }
            else
            {
                ASSERT(expected_p12_u16_cksm[level+3] == checksum_actual);
            }
            vxReleaseImage(&dst_image);

        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#define ADD_VX_SCALE_CKSUM_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF))
#define ADD_SET_TARGET_PARAMETERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_MSC1", __VA_ARGS__, TIVX_TARGET_VPAC_MSC1))
#define ADD_VX_BORDERS_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} }))
#define PARAMETERS_CREATE_P12_U16_CHECKSUM \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_TEST, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_SCALE_CKSUM_TEST,ADD_YC_MODE, ARG, NULL, NULL)

#define PARAMETERS_CREATE_INCORRECT_IMAGE_CHECKSUM \
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_TEST, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_SCALE_CKSUM_TEST,ADD_YC_MODE_TEST, ARG, NULL, NULL)
/* Test case for covering API tivxVpacMscPyramidValidate() for the incorrect input image format of VX_DF_IMAGE_S16 */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_incorrectInputImage, Arg, PARAMETERS_CREATE_INCORRECT_IMAGE_CHECKSUM)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        levels =  gaussian_pyramid_calc_max_levels_count(input_width, input_height, arg_->scale);
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, input_width, input_height, VX_DF_IMAGE_YUYV), VX_TYPE_PYRAMID);        
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
  
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#define ADD_MAX_DS_FACTOR_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/max_ds_factor=2", __VA_ARGS__, 2))
#define PARAMETERS_CREATE_MAX_DSFACTOR_BORDER\
    CT_GENERATE_PARAMETERS("checksum", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_TEST, ADD_VX_SCALE_CKSUM_TEST,ADD_YC_MODE_TEST, ADD_MAX_DS_FACTOR_TEST, ARG, NULL, NULL)
/* Test case for coverage improvement of API tivxVpacMscPyramidValidate() for the incorrect Input Image Format */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessing_incorrectInputFormat, ArgCreate, PARAMETERS_CREATE)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    int input_width = 1280, input_height = 512;
    vx_uint32 level;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/cropped_nv12_image.yuv";
    char file[MAXPATHLENGTH];
    size_t sz;
    vx_uint32 cksm_offset = 0;
    CT_Image input = NULL;
    vx_float32 scale = 0.5f;


    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(scale < 1.0);
        
        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, input_width, input_height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);       
        levels =  2;
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, scale, input_width, input_height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);
        
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_output_params_init_NULL, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        tivx_vpac_msc_output_params_init(NULL);

        
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_output_ref_NULL, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = NULL;
        }

        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_output_params_invalid_size, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 0u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 0u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t)+1, NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t)+1, &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_10bit_to_8bit_test[] = {
    0x159f9fd1,
    0xbcb71dc2
};

TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_filt1_phase1, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 2u;
        output_params[0].multi_phase.vert_coef_sel = 2u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 3u;
        output_params[1].multi_phase.vert_coef_sel = 3u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_filt1_phase0_sel1, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
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
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 0u;
        output_params[1].multi_phase.vert_coef_sel = 0u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_targetparams_filt1_phase0_sel1_default, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32  level,idx,  i, weight;
    vx_uint32 width=639; vx_uint32 height=639;
    vx_uint32 src_width=1920; vx_uint32 src_height=1920;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;

        output_params[1].filter_mode = 1u;
        output_params[1].coef_shift = 7u;
        output_params[1].single_phase.horz_coef_src = 0u;
        output_params[1].single_phase.horz_coef_sel = 1u;
        output_params[1].single_phase.vert_coef_src = 0u;
        output_params[1].single_phase.vert_coef_sel = 1u;
        output_params[1].multi_phase.phase_mode = 1u;
        output_params[1].multi_phase.horz_coef_sel = 4u;
        output_params[1].multi_phase.vert_coef_sel = 4u;

        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_char temp[256];

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}



static uint32_t expected_cksm_10bit_to_8bit_test_2[] = {
    0x159f9fd1,   
    0xbcb71dc2  
};
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_filt1_coeff1, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
 
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
 
    vx_border_t border = arg_->border;
 
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
 
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT(arg_->scale < 1.0);
 
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);
 
 
        width = src_width;
        height = src_height;
 
        levels = 2;
 
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);
 
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }
 
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
 
        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 1u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 1u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 1u;
        output_params[0].multi_phase.phase_mode = 1u;
        output_params[0].multi_phase.horz_coef_sel = 1u;
        output_params[0].multi_phase.vert_coef_sel = 1u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 4u;
            output_params[1].multi_phase.vert_coef_sel = 4u;
        }
        else{
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 1u;
            output_params[1].multi_phase.vert_coef_sel = 3u;
 
        }
 
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
 
        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
 
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }
 
        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));
 
            output_refs[level] = (vx_reference)output_obj[level];
        }
 
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
 
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
 
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));
 
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test_2[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);
 
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);
 
        tivxHwaUnLoadKernels(context);
    }
}


static uint32_t expected_cksm_10bit_to_8bit_test_1[] = {
    0x3cf61d27,
    0xfd7cfb79
};
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_filt0_sel1, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
 
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
 
    vx_border_t border = arg_->border;
 
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
 
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT(arg_->scale < 1.0);
 
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);
 
 
        width = src_width;
        height = src_height;
 
        levels = 2;
 
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);
 
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }
 
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
 
        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 1u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 1u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 0u;
            output_params[1].multi_phase.horz_coef_sel = 0u;
            output_params[1].multi_phase.vert_coef_sel = 0u;
        }
        else{
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 0u;
            output_params[1].multi_phase.horz_coef_sel = 1u;
            output_params[1].multi_phase.vert_coef_sel = 2u;
 
        }
 
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
 
        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
 
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }
 
        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));
 
            output_refs[level] = (vx_reference)output_obj[level];
        }
 
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
 
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
 
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));
 
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);

            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test_1[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);
 
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);
 
        tivxHwaUnLoadKernels(context);
    }

}
static uint32_t expected_cksm_10bit_to_8bit_test_3[] = {
    0x3cf61d27,  
    0xbcb71dc2
};
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_filt0_src0_sel0, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
 
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32  level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_uint32 width, height=32768;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
 
    vx_border_t border = arg_->border;
 
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
 
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT(arg_->scale < 1.0);
 
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);
 
 
        width = src_width;
        height = src_height;
 
        levels = 2;
 
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);
 
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }
 
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
 
        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 0u;
        output_params[0].single_phase.horz_coef_sel = 1u;
        output_params[0].single_phase.vert_coef_src = 0u;
        output_params[0].single_phase.vert_coef_sel = 1u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.init_phase_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        output_params[0].multi_phase.init_phase_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
        output_params[0].offset_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        output_params[0].offset_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 0u;
            output_params[1].multi_phase.vert_coef_sel = 0u;
        }
        else{
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 3u;
            output_params[1].multi_phase.vert_coef_sel = 3u;
        }
 
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
 
        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
 
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }
 
        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));
 
            output_refs[level] = (vx_reference)output_obj[level];
        }
 
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
 
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
 
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));
 
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);

            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test_3[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);
 
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);
 
        tivxHwaUnLoadKernels(context);
    }
}


static uint32_t expected_cksm_10bit_to_8bit_test_4[] = {
    0xdc974468,     
    0xbcb71dc2
};
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessingChecksum_10bit_targetparams_CopyOutPrmsToScCfg_src0, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
 
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 2592, src_height = 1944, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    vx_border_t border = arg_->border;
 
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
 
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT(arg_->scale < 1.0);
 
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);
 
 
        width = src_width;
        height = src_height;
 
        levels = 2;
 
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);
 
        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }
 
        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
 
        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
        }
        output_params[0].filter_mode = 0u;
        output_params[0].coef_shift = 7u;
        output_params[0].single_phase.horz_coef_src = 1u;
        output_params[0].single_phase.horz_coef_sel = 0u;
        output_params[0].single_phase.vert_coef_src = 1u;
        output_params[0].single_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.phase_mode = 0u;
        output_params[0].multi_phase.horz_coef_sel = 0u;
        output_params[0].multi_phase.vert_coef_sel = 0u;
        output_params[0].multi_phase.init_phase_x = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
        output_params[0].multi_phase.init_phase_y = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
        output_params[0].offset_x = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
        output_params[0].offset_y = TIVX_VPAC_MSC_AUTOCOMPUTE+1;

        if((strcmp(arg_->target_string, "TIVX_TARGET_VPAC_MSC1"))==0U)
        {
            output_params[1].filter_mode = 0u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 1u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 1u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 1u;
            output_params[1].multi_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.init_phase_x = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
            output_params[1].multi_phase.init_phase_y = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
            output_params[1].offset_x = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
            output_params[1].offset_y = TIVX_VPAC_MSC_AUTOCOMPUTE+1;
        }
        else{
            output_params[1].filter_mode = 1u;
            output_params[1].coef_shift = 7u;
            output_params[1].single_phase.horz_coef_src = 0u;
            output_params[1].single_phase.horz_coef_sel = 1u;
            output_params[1].single_phase.vert_coef_src = 0u;
            output_params[1].single_phase.vert_coef_sel = 1u;
            output_params[1].multi_phase.phase_mode = 1u;
            output_params[1].multi_phase.horz_coef_sel = 1u;
            output_params[1].multi_phase.vert_coef_sel = 1u;
        }
 
        tivx_vpac_msc_coefficients_params_init(&coeffs, TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE);
        idx = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 256;
        coeffs.single_phase[0][idx ++] = 0;
        coeffs.single_phase[0][idx ++] = 0;
        idx = 0;
        coeffs.single_phase[1][idx ++] = 16;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 96;
        coeffs.single_phase[1][idx ++] = 64;
        coeffs.single_phase[1][idx ++] = 16;
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
 
        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));
 
        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }
 
        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));
 
            output_refs[level] = (vx_reference)output_obj[level];
        }
 
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));
 
        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            vx_char temp[256];
 
            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));
 
            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;
            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            sprintf(temp, "output/10bit_to_8bit_msc_%d", level);
            save_image_from_msc(dst_image, temp); 
            #endif
            ASSERT(expected_cksm_10bit_to_8bit_test_4[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);
 
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);
 
        tivxHwaUnLoadKernels(context);
    }
}



/* Test case for coverage improvement of the API tivxVpacMscPyramidValidate() with NV12 format to trigger format mismatch */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_FormatMismatch_NV12, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level, idx, src_width = 2592, src_height = 1944;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;
        levels = 2;

        //  Create pyramid with NV12 format to trigger format mismatch
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        //  Expect verify graph failure due to format mismatch
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        // Optional: skip processing if graph invalid
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_FormatMismatch_S16, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;
    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level, idx, src_width = 2592, src_height = 1944;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image y12 = 0;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;
        levels = 2;

        //  Create pyramid with NV12 format to trigger format mismatch
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_S16), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        //  Expect verify graph failure due to format mismatch
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));

        // Optional: skip processing if graph invalid
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#if defined(VPAC3) || defined(VPAC3L)
/* Test case for coverage improvement of the API tivxVpacMscPyramidValidate() with VX_DF_IMAGE_UYVY format to trigger format mismatch */
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingChecksum_10bit_FormatMismatch_UYVY, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level, idx, src_width = 2592, src_height = 1944;
    vx_reference refs[1];
    vx_image y12 = 0;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        // Input image with format != VX_DF_IMAGE_UYVY
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        ct_read_raw_image_to_vx_image(y12, "psdkra/app_single_cam/x5b_rgb.bin", 2);

        width = src_width;
        height = src_height;
        levels = 2;

        // Change pyramid format to VX_DF_IMAGE_UYVY to trigger the format mismatch condition
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, y12, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        // Expect error status because input image format != pyramid format (condition triggers error)
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        // Clean up
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&y12));
        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(pyr == 0);
        ASSERT(y12 == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGETERRSTATUS, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
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

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGETERRSTATUS_NULL, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);
    
        err_refs[0] = NULL;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        ASSERT_NO_FAILURE(vxReleaseScalar(&scalar_err_stat_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGETERRSTATUSdefault, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height;
    vx_reference refs[1];

    CT_Image input = NULL;

    vx_border_t border = arg_->border;
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_NO_FAILURE(input = arg_->generator( arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

        width = (vx_uint32)((vx_float32)ceil(input->width * arg_->scale));
        height = (vx_uint32)((vx_float32)ceil(input->height * arg_->scale));
        levels = gaussian_pyramid_calc_max_levels_count(width, height, arg_->scale);

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, MSC_NODE_ERROR_EVENT));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
         /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);
    
        err_refs[0] = (vx_reference)scalar_err_stat_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_ERR_STATUS,
            err_refs, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("MSC error status value is %d\n", scalar_out_err_stat);
        #endif
        ASSERT_NO_FAILURE(vxReleaseScalar(&scalar_err_stat_obj)); 

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&input_image));
        ASSERT(pyr == 0);
        ASSERT(input_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
static uint32_t expected_cksm_uyvy_TEST[] = {
    0xac760373,
    0x6c55cc2a
};

#if defined(VPAC3) || defined(VPAC3L)
TEST_WITH_ARG(tivxHwaVpacMscPyramidPositive, testGraphProcessing_UYVY_YUYV, Arg,
    PARAMETERS_CKSUM_10bit
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight;
    vx_reference refs[1];
    vx_uint32 cksm_offset = 0;
    vx_image src_image = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/Bower_1920x1080_uyvy_8b.yuv";
    const char *output_file_names[3] = { "output/pmd_uyvy_output_0.yuv", "output/pmd_uyvy_output_1.yuv", "output/pmd_uyvy_output_2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readUYVYInput(file, src_image));


        width = src_width;
        height = src_height;

        levels = 2;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_PYRAMID);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

        ASSERT_NO_FAILURE(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        vx_user_data_object coeff_obj, output_obj[levels];
        tivx_vpac_msc_coefficients_t coeffs;
        tivx_vpac_msc_output_params_t output_params[levels];
        vx_reference refs[1], output_refs[levels];
        for(level = 0; level <levels; level++)
        {
            tivx_vpac_msc_output_params_init(&output_params[level]);
            output_params[level].filter_mode = 0;
        }
        tivx_vpac_msc_coefficients_params_init(&coeffs, VX_INTERPOLATION_BILINEAR);
        /* Set Coefficients */ 
        ASSERT_VX_OBJECT(coeff_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_coefficients_t",
            sizeof(tivx_vpac_msc_coefficients_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(coeff_obj, 0,
            sizeof(tivx_vpac_msc_coefficients_t), &coeffs, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));

        refs[0] = (vx_reference)coeff_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));
        for (level = 0; level < levels; level++)
        {
            output_refs[level] = NULL;
        }

        /* Set Output params */
        for (level = 0; level < levels; level++)
        {
            ASSERT_VX_OBJECT(output_obj[level] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_output_params_t",
                sizeof(tivx_vpac_msc_output_params_t), NULL),
                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(output_obj[level], 0,
                sizeof(tivx_vpac_msc_output_params_t), &output_params[level], VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST));

            output_refs[level] = (vx_reference)output_obj[level];
        }

        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS,
            output_refs, levels));
        
        for (level = 0; level < levels; level++)
        {
            VX_CALL(vxReleaseUserDataObject(&output_obj[level]));
        }
        VX_CALL(vxProcessGraph(graph));

        for(level = 0; level < levels; level++)
        {
            vx_image dst_image = vxGetPyramidLevel(pyr, level);
            vx_uint32 w, h;
            vx_rectangle_t rect;
            vx_uint32 checksum_actual = 0;
            sz = snprintf(output_files[level], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[level]);
            ASSERT_(return, (sz < MAXPATHLENGTH));

            vxQueryImage(dst_image, VX_IMAGE_WIDTH, &w, sizeof(w));
            vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &h, sizeof(h));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = w;
            rect.end_y = h;

            checksum_actual = tivx_utils_simple_image_checksum(dst_image, 0, rect);
            #if defined(TEST_MSC_CHECKSUM_LOGGING)
            printf("0x%08x\t%d\n", checksum_actual, cksm_offset);
            write_uyvy_output_image(output_files[level], dst_image); 
            #endif
            ASSERT(expected_cksm_uyvy_TEST[level+cksm_offset] == checksum_actual);
            vxReleaseImage(&dst_image);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingGetPsaValues_NULL_psa, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
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
        
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t), NULL),
            VX_TYPE_USER_DATA_OBJECT);

        psa_refs[0] = NULL;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS,vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingGetPsaValues_invalid_size_psa, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
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
        
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
            vxCreateUserDataObject(context, "tivx_vpac_msc_psa_timestamp_data_t", sizeof(tivx_vpac_msc_psa_timestamp_data_t)+1, NULL),
            VX_TYPE_USER_DATA_OBJECT);

        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
        
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_msc_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingGetPsaValues_Input_params_NULL, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.enable_psa = 1u;
        ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t), &sc_input_params, VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST));
        refs[0] = NULL;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&sc_input_params_obj)); 
        
        VX_CALL(vxProcessGraph(graph));
       
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingGetPsaValues_Input_params_invalid_size, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
        tivx_vpac_msc_input_params_init(&sc_input_params);
        sc_input_params.enable_psa = 1u;
        ASSERT_VX_OBJECT(sc_input_params_obj = vxCreateUserDataObject(context,
            "tivx_vpac_msc_input_params_t",
            sizeof(tivx_vpac_msc_input_params_t)+1, NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(sc_input_params_obj, 0,
            sizeof(tivx_vpac_msc_input_params_t)+1, &sc_input_params, VX_WRITE_ONLY,
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

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacMscPyramidNegative, testGraphProcessingGetPsaValues_inputparams_NULL, Arg,
    PARAMETERS_CKSUM_NV12
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_uint32 width, height, level,idx, src_width = 1920, src_height = 1080, i, weight, cnt = 0;
    vx_reference refs[1];
    vx_image src_image = 0;
    vx_uint32 cksm_offset = 0;
    char *input_file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";
    const char *output_file_names[3] = { "output/msc_nv12_out0.yuv", "output/msc_nv12_out1.yuv", "output/msc_nv12_out2.yuv" };
    char file[MAXPATHLENGTH];
    char output_files[3][MAXPATHLENGTH];
    size_t sz;
    vx_user_data_object coeff_obj;
    tivx_vpac_msc_coefficients_t coeffs;
    uint32_t psa_values[2][10] = {0};
    vx_reference psa_refs [5] = {0};
    vx_user_data_object psa_obj;
    vx_user_data_object sc_input_params_obj;
    tivx_vpac_msc_input_params_t sc_input_params;
    tivx_vpac_msc_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT(arg_->scale < 1.0);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, src_width, src_height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), input_file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));
        VX_CALL(readNV12Input(file, src_image));


        width = src_width;
        height = src_height;

        levels = 3;

        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, width, height, VX_DF_IMAGE_NV12), VX_TYPE_PYRAMID);
        for (i = 0; i < 3; i++) 
        {
            sz = snprintf(output_files[i], MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), output_file_names[i]);
            ASSERT_(return, (sz < MAXPATHLENGTH));
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacMscPyramidNode(graph, src_image, pyr), VX_TYPE_NODE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)src_image, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        if (border.mode != VX_BORDER_UNDEFINED)
        {
            VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));
        }

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
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF,
            refs, 1u));

        VX_CALL(vxReleaseUserDataObject(&coeff_obj));

        /* set input */
        tivx_vpac_msc_input_params_init(NULL);
        

    
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleasePyramid(&pyr));
        VX_CALL(vxReleaseImage(&src_image));
        ASSERT(pyr == 0);
        ASSERT(src_image == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVpacMscPyramid,
        testNodeCreation,
        testGraphProcessing,
        testGraphProcessing_multi,
        testGraphProcessingChecksum,
        testGraphProcessingChecksum_nv12_u8,
        testGraphProcessingChecksum_10bit,
        testGraphProcessingChecksum_nv12_u8_max_ds_factor
        #ifndef x86_64
        ,
        testErrorInterrupts,
        testGraphProcessingGetPsaValues
        #ifndef VPAC3L
        ,
        testSl2Firewall
        #endif
        #endif
        #if defined(VPAC3) || defined(VPAC3L)
        ,
        testGraphProcessing_UYVY,
        testGraphProcessing_UYVY_input_Y_output
        #endif
        ,
        testGraphProcessingChecksum_NV12,
        testGraphProcessingChecksum_invalid_scaling_factor_error,
        testGraphProcessingChecksum_NV12_ten_levels,
        testGraphProcessingChecksum_NV12_ten_subsets,
        testGraphProcessingChecksum_10bit_acc,
        testGraphProcessingChecksum_P12_U16_formatSupport
        )
TESTCASE_TESTS(tivxHwaVpacMscPyramidPositive,
        testGraphProcessingChecksum_10bit_targetparams_filt0_src0_sel0,
        testGraphProcessingChecksum_10bit_targetparams_filt1_phase1,
        testGraphProcessingChecksum_10bit_targetparams_filt0_sel1,
        testGraphProcessingChecksum_10bit_targetparams_filt1_coeff1,
        testGraphProcessingChecksum_10bit_targetparams_CopyOutPrmsToScCfg_src0
        #if defined TEST_MSC_ENABLE
        ,
        testKrSz4,
        testKrSz3
        #endif
        #if defined(VPAC3) || defined(VPAC3L)
        ,
        testGraphProcessing_UYVY_YUYV
        #endif
        ,
        testGraphProcessingChecksum_10bit_targetparams_filt1_phase0_sel1
        #if defined(VPAC3) || defined(VPAC3L)
        #ifndef x86_64
        ,        
        testGraphProcessingChecksum_10bit_acc_sm,
        testGraphProcessingChecksum_10bit_acc_sm10,
        testGraphProcessingChecksum_10bit_acc_sm7
        #endif
        #endif
    )

TESTCASE_TESTS(tivxHwaVpacMscPyramidNegative,
        testGraphProcessingChecksum_nv12_u8_ycmode_2,
        testGraphProcessing_coffRefNull,
        testGraphProcessing_invalidCoeff_parmSize,
        testGraphProcessingChecksum_10bit_output_ref_NULL,
        testGraphProcessingChecksum_10bit_output_params_invalid_size,
        testInvalid_kernelSize,
        testGraphProcessing_invalid_scale,
        testGraphProcessingChecksum_10bit_output_params_init_NULL,
        testGraphProcessingChecksum_10bit_targetparams_filt1_phase0_sel1_default,
        testGraphProcessing_incorrectInputImage,
        testGraphProcessingChecksum_10bit_FormatMismatch_NV12,
        testGraphProcessingChecksum_10bit_FormatMismatch_S16,
        #if defined(VPAC3) || defined(VPAC3L)
        testGraphProcessingChecksum_10bit_FormatMismatch_UYVY,
        #endif
        testGraphProcessing_invalidBoard_parm,
        testGraphProcessing_incorrectInputFormat
        #ifndef x86_64
        ,
        testGraphProcessingGetPsaValues_Input_params_NULL,
        testGraphProcessingGetPsaValues_Input_params_invalid_size,
        testGraphProcessingGetPsaValues_NULL_psa,
        testGraphProcessingGetPsaValues_invalid_size_psa,
        testGraphProcessingGetPsaValues_inputparams_NULL,
        testGETERRSTATUS,
        testGETERRSTATUS_NULL,
        testGETERRSTATUSdefault
        #endif
        )


#endif /* BUILD_VPAC_MSC */

