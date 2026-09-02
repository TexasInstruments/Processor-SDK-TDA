/*
 *
 * Copyright (c) 2018-2020 Texas Instruments Incorporated
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
#ifdef BUILD_VPAC_VISS


#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <utils/iss/include/app_iss.h>
#include "test_hwa_common.h"
#include "tivx_utils_checksum.h"
#include <dcc_viss_imx390_wdr_14b.h>
#include <dcc_viss_imx390_wdr_cac.h>
#include <dcc_viss_imx390_wdr_gamma.h>
#include <dcc_viss_imx390_wdr_rfe_b1.h>
#include <dcc_viss_imx390_wdr_ccmv.h>
#include <dcc_viss_imx390_wdr_lsc.h>
#include <dcc_viss_imx390_wdr_cfa.h>
#include <dcc_viss_ov2312_color_diff.h>
#include <inttypes.h>
#include <TI/hwa_vpac_viss.h>


#ifdef VPAC3L
#define VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE 156
#define VISS_CHECKSUMS_H3A_REF_SIZE 13
#else
#define VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE 144
#define VISS_CHECKSUMS_H3A_REF_SIZE 12
#endif

#define TEST_NUM_NODE_INSTANCE 2
#define SET_TIME_STAMP_VALUE  (100U)

/* #define TEST_VISS_PERFORMANCE_LOGGING */

/* #define TEST_VISS_CHECKSUM_LOGGING */
/* #define TEST_GET_ERROR_INTERRUPT_VALUES */
/* #define TEST_PRINT_TIME_STAMP_VALUES */

#define VISS_NODE_ERROR_EVENT  (1U)

#define APP_MAX_FILE_PATH           (512u)

#if defined(SOC_J784S4) || defined(SOC_J742S2)
#define VISS_VPAC0_READBACK_REG_ADDR 0xB0C00000
#define VISS_VPAC1_READBACK_REG_ADDR 0xB0E00000
static uint32_t vpac0_buffer_allocation_flags[8] = {0};
static uint32_t vpac1_buffer_allocation_flags[8] = {0};
#elif defined(SOC_J721S2)
#define VISS_VPAC0_READBACK_REG_ADDR 0xADC00000
static uint32_t vpac0_buffer_allocation_flags[8] = {0};
#elif defined(SOC_AM62A)
#define VISS_VPAC0_READBACK_REG_ADDR 0xADE00000
static uint32_t vpac0_buffer_allocation_flags[8] = {0};
#elif defined(SOC_J722S)
#define VISS_VPAC0_READBACK_REG_ADDR 0xB7A00000
static uint32_t vpac0_buffer_allocation_flags[8] = {0};
#endif

#define ADD_SIZE_64x48(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=64x48", __VA_ARGS__, 64, 48))

TESTCASE(tivxHwaVpacViss, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacVissPositiveCov, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacVissNegativeCov, CT_VXContext, ct_setup_vx_context, 0)
TESTCASE(tivxHwaVpacVissNegativeCov2, CT_VXContext, ct_setup_vx_context, 0)


typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;

} SetTarget_Arg;

#if (VPAC_COUNT > 1)
#if defined(x86_64)
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_VPAC2_VISS1))
#else
#if defined IMAGING_COVERAGE_ENABLED
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1))
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC2_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_VPAC2_VISS1))
#endif
#endif
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1))
#endif

#if (VPAC_COUNT > 1)
#if defined IMAGING_COVERAGE_ENABLED
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1))
#endif
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1))
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacViss, testNodeCreation, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


typedef struct {
    const char* testName;
    int width, height, exposures;
    vx_bool line_interleaved;
    char* target_string;
} Arg;

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

#define ADD_EXP1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=1", __VA_ARGS__, 1))

#define ADD_EXP2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=2", __VA_ARGS__, 2))

#define ADD_EXP3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=3", __VA_ARGS__, 3))

#define ADD_LINE_FALSE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=false", __VA_ARGS__, vx_false_e))

#define ADD_LINE_TRUE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=true", __VA_ARGS__, vx_true_e))

#define ADD_SIZE_640x480(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_FALSE, ADD_SET_TARGET_PARAMETERS, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_FALSE, ADD_SET_TARGET_PARAMETERS, ARG), \
   /* CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_FALSE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_2048x1024, ADD_EXP1, ARG)*/

/* Test case for TIOVX-1236 */
TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFile, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);*/
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);*/

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 1;
        params.enable_ctx = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_viss_output_image(file, y8_r8_c2);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));
        /*VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));*/
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        /*VX_CALL(vxReleaseImage(&uv12_c1));*/
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int op_fmt;
    int dcc;
    int results_2a;
    int bypass_glbce;
    int bypass_nsf4;
    char *target_string, *target_string_2;
} ArgDcc;

typedef struct {
    const char* testName;
    int op_fmt;
    int dcc;
    int results_2a;
    int bypass_glbce;
    int bypass_nsf4;
    int enable_safety_mechanism;
    char *target_string, *target_string_2;
} ArgDcc_safety;

static uint32_t viss_checksums_luma_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0xf47beefd, (uint32_t) 0x03b9eda6, (uint32_t) 0x98c0c9df, (uint32_t) 0xade9ca14,
    (uint32_t) 0xf47beefd, (uint32_t) 0x03b9eda6, (uint32_t) 0x98c0c9df, (uint32_t) 0xade9ca14,
    (uint32_t) 0x781f7630, (uint32_t) 0xe7d95550, (uint32_t) 0xeaed49e2, (uint32_t) 0x60b695a7,
    (uint32_t) 0x080d9c2d, (uint32_t) 0x4bed2b15, (uint32_t) 0xb019be0a, (uint32_t) 0xdca1c62f,
    (uint32_t) 0x080d9c2d, (uint32_t) 0x4bed2b15, (uint32_t) 0xb019be0a, (uint32_t) 0xdca1c62f,
    (uint32_t) 0x908a52ee, (uint32_t) 0x271d8c34, (uint32_t) 0xa9deaf7a, (uint32_t) 0xcfbdbebe,
    (uint32_t) 0xffab8e7d, (uint32_t) 0x51cbd1f6, (uint32_t) 0x65995e4a, (uint32_t) 0x41bad204,
    (uint32_t) 0x627e6edc, (uint32_t) 0xde350c54, (uint32_t) 0xa81d15ee, (uint32_t) 0x120dacb6,
    (uint32_t) 0xfefd6eb9, (uint32_t) 0xa0da7954, (uint32_t) 0x17eb771b, (uint32_t) 0xdbe70fff,
    (uint32_t) 0xa822fcba, (uint32_t) 0xdcb0c966, (uint32_t) 0x3af111cf, (uint32_t) 0xb720e2e6,
    (uint32_t) 0xa822fcba, (uint32_t) 0xdcb0c966, (uint32_t) 0x3af111cf, (uint32_t) 0xb720e2e6,
    (uint32_t) 0xe2263148, (uint32_t) 0x0d47a692, (uint32_t) 0x61c52600, (uint32_t) 0x9f39b173,
    (uint32_t) 0xe5ca9fb9, (uint32_t) 0x5d90b6fb, (uint32_t) 0xfe5f53fc, (uint32_t) 0x84acc97c,
    (uint32_t) 0x0924d246, (uint32_t) 0xbe1f4acb, (uint32_t) 0xc6750795, (uint32_t) 0xcd55e141,
    (uint32_t) 0x8c1cf5bf, (uint32_t) 0x03e50bc8, (uint32_t) 0x122b99ea, (uint32_t) 0x535c63b3
    #ifdef VPAC3L
    ,(uint32_t) 0x0c90fad6, (uint32_t) 0x833c9b64, (uint32_t) 0x0c90fad6, (uint32_t) 0x833c9b64
    ,(uint32_t) 0x24188991, (uint32_t) 0x3e003428, (uint32_t) 0x24188991, (uint32_t) 0x3e003428
    ,(uint32_t) 0x6605773f, (uint32_t) 0x7b42c5cc, (uint32_t) 0x6605773f, (uint32_t) 0x7b42c5cc
    #endif
    ,(uint32_t) 0xb4959c51, (uint32_t) 0x2a8b4288, (uint32_t) 0x6a6e4d9b, (uint32_t) 0xb5322fac
    ,(uint32_t) 0x0a18f9ba, (uint32_t) 0x510ed8c4, (uint32_t) 0x08ed9d48, (uint32_t) 0x2ee83e49
    ,(uint32_t) 0xe75745e4, (uint32_t) 0x8ed3901e, (uint32_t) 0x21cbb3d7, (uint32_t) 0x26ee9eb4
    ,(uint32_t) 0xd8b50905, (uint32_t) 0x6d800791, (uint32_t) 0xb8ae5de6, (uint32_t) 0xbc0f0a5b
    ,(uint32_t) 0x470d4a0d, (uint32_t) 0x62d6546a, (uint32_t) 0x8d2fe148, (uint32_t) 0xed150971
    ,(uint32_t) 0xc981856a, (uint32_t) 0x3475eb7c, (uint32_t) 0x90573605, (uint32_t) 0xea9b6a7a
    ,(uint32_t) 0xbd224c85, (uint32_t) 0x808064c9, (uint32_t) 0x7755fe0b, (uint32_t) 0xd95b5d64
    ,(uint32_t) 0x45a2aafc, (uint32_t) 0xed90e80a, (uint32_t) 0x58afe4e1, (uint32_t) 0x0d2280cc
    ,(uint32_t) 0x8cb49716, (uint32_t) 0xde14e15b, (uint32_t) 0xfd8bfe39, (uint32_t) 0x4ac8ca56
    ,(uint32_t) 0x1b64a5d1, (uint32_t) 0xb1e6178d, (uint32_t) 0x48a5e164, (uint32_t) 0x3e5f9b50
    ,(uint32_t) 0xea111c63, (uint32_t) 0x1d3a1c93, (uint32_t) 0x6bddf9f4, (uint32_t) 0x498cddfe
    ,(uint32_t) 0x511b4de6, (uint32_t) 0xb279986e, (uint32_t) 0x8ebc66d8, (uint32_t) 0xf5f52161
    ,(uint32_t) 0xbd224c85, (uint32_t) 0x808064c9, (uint32_t) 0x7755fe0b, (uint32_t) 0xd95b5d64
    ,(uint32_t) 0x45a2aafc, (uint32_t) 0xed90e80a, (uint32_t) 0x58afe4e1, (uint32_t) 0x0d2280cc
    ,(uint32_t) 0xa1c4b64b, (uint32_t) 0x6f8e30f0, (uint32_t) 0x12d9c5f0, (uint32_t) 0xef23b408
    ,(uint32_t) 0x2813e514, (uint32_t) 0x4d1b0d82, (uint32_t) 0xd53d9f33, (uint32_t) 0xa40458dd
    ,(uint32_t) 0xbcca4642, (uint32_t) 0x725d1db8, (uint32_t) 0x64315052, (uint32_t) 0x1eb3c8d8
    ,(uint32_t) 0xda693f08, (uint32_t) 0xe108d21e, (uint32_t) 0x949c7c97, (uint32_t) 0x30efb18d
    ,(uint32_t) 0x2bc471ac, (uint32_t) 0x6aee69a8, (uint32_t) 0x7b8a8c3b, (uint32_t) 0xd6e2c360
    ,(uint32_t) 0x8f23b6c1, (uint32_t) 0xffc0d386, (uint32_t) 0x116b094f, (uint32_t) 0xc2e986d8
    ,(uint32_t) 0xa327b62f, (uint32_t) 0xfa604f69, (uint32_t) 0xc084bbae, (uint32_t) 0x15d0a5c3
};
static uint32_t viss_checksums_chroma_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0x22cdd15d, (uint32_t) 0x485cc747, (uint32_t) 0x190b425e, (uint32_t) 0x9a4dc6c0,
    (uint32_t) 0x22cdd15d, (uint32_t) 0x485cc747, (uint32_t) 0x190b425e, (uint32_t) 0x9a4dc6c0,
    (uint32_t) 0xff5c5d31, (uint32_t) 0xadb0bec7, (uint32_t) 0x661e6851, (uint32_t) 0xda4e097f,
    (uint32_t) 0x3083ca4b, (uint32_t) 0x5fec52b8, (uint32_t) 0x2ce1134c, (uint32_t) 0x05cbea74,
    (uint32_t) 0x3083ca4b, (uint32_t) 0x5fec52b8, (uint32_t) 0x2ce1134c, (uint32_t) 0x05cbea74,
    (uint32_t) 0x2d19a877, (uint32_t) 0x62c6d227, (uint32_t) 0x4207c710, (uint32_t) 0x4f88f026,
    (uint32_t) 0x456ea247, (uint32_t) 0x845f901a, (uint32_t) 0x3a9dafd1, (uint32_t) 0x4b91a897,
    (uint32_t) 0x54c9af0c, (uint32_t) 0x686fe091, (uint32_t) 0x3b149b94, (uint32_t) 0xdd7220a3,
    (uint32_t) 0x4f673a33, (uint32_t) 0xbd582de8, (uint32_t) 0x292394dd, (uint32_t) 0xb9eb728d,
    (uint32_t) 0x987a6c65, (uint32_t) 0x7bbf7d1d, (uint32_t) 0x99a80276, (uint32_t) 0x8f833a56,
    (uint32_t) 0x987a6c65, (uint32_t) 0x7bbf7d1d, (uint32_t) 0x99a80276, (uint32_t) 0x8f833a56,
    (uint32_t) 0x5dc5ce7a, (uint32_t) 0x3f14c7b0, (uint32_t) 0x0ff5279e, (uint32_t) 0x35356d95,
    (uint32_t) 0x227bafb5, (uint32_t) 0xe165e425, (uint32_t) 0x3073d950, (uint32_t) 0xfa2b5cae,
    (uint32_t) 0xb026e875, (uint32_t) 0xd7075a56, (uint32_t) 0x2f9e5202, (uint32_t) 0xef5d8951,
    (uint32_t) 0x1ed6ba03, (uint32_t) 0x8eb710fd, (uint32_t) 0x05da456a, (uint32_t) 0x1b0fc632
    #ifdef VPAC3L
    ,(uint32_t) 0x77188952, (uint32_t) 0x8a2a986c, (uint32_t) 0x77188952, (uint32_t) 0x8a2a986c
    ,(uint32_t) 0xe6e31d5d, (uint32_t) 0xf8fd4571, (uint32_t) 0xe6e31d5d, (uint32_t) 0xf8fd4571
    ,(uint32_t) 0x1826c8e5, (uint32_t) 0xf2d2692d, (uint32_t) 0x1826c8e5, (uint32_t) 0xf2d2692d
    #endif
    ,(uint32_t) 0x8911fe1c, (uint32_t) 0x4a1cfa6d, (uint32_t) 0x89ae2a1b, (uint32_t) 0x68cb1719
    ,(uint32_t) 0x2f7fe13a, (uint32_t) 0x0e8f62b4, (uint32_t) 0x2ce8befb, (uint32_t) 0x50dabff0
    ,(uint32_t) 0x3b545581, (uint32_t) 0xde874c3c, (uint32_t) 0x518073b8, (uint32_t) 0xaca1f9c5
    ,(uint32_t) 0x07614df0, (uint32_t) 0x18d05e74, (uint32_t) 0xa752da14, (uint32_t) 0xec68fa2b
    ,(uint32_t) 0xd7e57af8, (uint32_t) 0xdf502a20, (uint32_t) 0x7439af7a, (uint32_t) 0x8689c2a7
    ,(uint32_t) 0xe6221794, (uint32_t) 0xb94ce249, (uint32_t) 0xa09dfc2f, (uint32_t) 0x7ec4ecd9
    ,(uint32_t) 0xda9da6bc, (uint32_t) 0x29fd66de, (uint32_t) 0x998ec143, (uint32_t) 0xd173b33d
    ,(uint32_t) 0x4221e3b1, (uint32_t) 0x9116cf27, (uint32_t) 0xdc34950e, (uint32_t) 0xc3616729
    ,(uint32_t) 0x756bdb9a, (uint32_t) 0x8bd99aec, (uint32_t) 0x760966d8, (uint32_t) 0x01bcc13d
    ,(uint32_t) 0x2421c9e1, (uint32_t) 0x2a247b1f, (uint32_t) 0xb8156d0d, (uint32_t) 0xae2f5c0a
    ,(uint32_t) 0xc125244f, (uint32_t) 0x756f70d4, (uint32_t) 0x72d8b76c, (uint32_t) 0xc6dde363
    ,(uint32_t) 0x9856a2db, (uint32_t) 0x5d150424, (uint32_t) 0x1f26979b, (uint32_t) 0xb3b42414
    ,(uint32_t) 0xda9da6bc, (uint32_t) 0x29fd66de, (uint32_t) 0x998ec143, (uint32_t) 0xd173b33d
    ,(uint32_t) 0x4221e3b1, (uint32_t) 0x9116cf27, (uint32_t) 0xdc34950e, (uint32_t) 0xc3616729
    ,(uint32_t) 0x084157a5, (uint32_t) 0xa1a56b87, (uint32_t) 0x00fec240, (uint32_t) 0x2167b28e
    ,(uint32_t) 0x3d025d2e, (uint32_t) 0x21893c0a, (uint32_t) 0xca9d5a1c, (uint32_t) 0xb8dd564e
    ,(uint32_t) 0x54303035, (uint32_t) 0x7ecee80d, (uint32_t) 0xfc79890c, (uint32_t) 0x10f3b678
    ,(uint32_t) 0x78789b8c, (uint32_t) 0xc5ed03b3, (uint32_t) 0x88b2306a, (uint32_t) 0xa9fc6f32
    ,(uint32_t) 0x066f029f, (uint32_t) 0xbf18af85, (uint32_t) 0xf370332e, (uint32_t) 0xfa742b48
    ,(uint32_t) 0x423ae7b7, (uint32_t) 0x23e3cc98, (uint32_t) 0x17429189, (uint32_t) 0x41367270
    ,(uint32_t) 0xf887bfa8, (uint32_t) 0xa7674cdd, (uint32_t) 0x3ae93f21, (uint32_t) 0xe99e7bf7
};
static uint32_t viss_checksums_yuv422_uyvy_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0x452ec654, (uint32_t) 0x0e588bce, (uint32_t) 0xae19ac94, (uint32_t) 0xa35af8d5,
    (uint32_t) 0x452ec654, (uint32_t) 0x0e588bce, (uint32_t) 0xae19ac94, (uint32_t) 0xa35af8d5,
    (uint32_t) 0xebc6cb6d, (uint32_t) 0xeef37fa2, (uint32_t) 0x00f7594f, (uint32_t) 0xa19cd5a0,
    (uint32_t) 0x516d0dde, (uint32_t) 0x04eadf3b, (uint32_t) 0xd8808df6, (uint32_t) 0xf529309b,
    (uint32_t) 0x516d0dde, (uint32_t) 0x04eadf3b, (uint32_t) 0xd8808df6, (uint32_t) 0xf529309b,
    (uint32_t) 0xb6e52cae, (uint32_t) 0x8706ec40, (uint32_t) 0xaac5a82b, (uint32_t) 0x2b93a612,
    (uint32_t) 0xbdd93ecd, (uint32_t) 0x2564c8df, (uint32_t) 0xb431210b, (uint32_t) 0xeb7ef704,
    (uint32_t) 0x5a721b16, (uint32_t) 0x498c437c, (uint32_t) 0x74b957a0, (uint32_t) 0x59ec11dd,
    (uint32_t) 0x0a8a4ed5, (uint32_t) 0xfc01f91e, (uint32_t) 0xa7e23646, (uint32_t) 0xda2a003c,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000
    #ifdef VPAC3L
    ,(uint32_t) 0x2e36654a, (uint32_t) 0x43409de3, (uint32_t) 0x2e36654a, (uint32_t) 0x43409de3
    ,(uint32_t) 0xf9583346, (uint32_t) 0xbc4cb14b, (uint32_t) 0xf9583346, (uint32_t) 0xbc4cb14b
    ,(uint32_t) 0x07a512f3, (uint32_t) 0xa4582f58, (uint32_t) 0x07a512f3, (uint32_t) 0xa4582f58
    #endif
    ,(uint32_t) 0xe48c7cee, (uint32_t) 0xe2b99952, (uint32_t) 0xa48ffb30, (uint32_t) 0xcf49cfd2
    ,(uint32_t) 0xc0d9761e, (uint32_t) 0xcd3d6409, (uint32_t) 0xab732833, (uint32_t) 0x704f2220
    ,(uint32_t) 0x852011d9, (uint32_t) 0xc5c4ab92, (uint32_t) 0x1e82c984, (uint32_t) 0x16b4c50a
    ,(uint32_t) 0x308e5316, (uint32_t) 0x9a9e92c5, (uint32_t) 0x06e0c89d, (uint32_t) 0xb7cc9e22
    ,(uint32_t) 0xac6c2f16, (uint32_t) 0xac133c65, (uint32_t) 0x8db10f2e, (uint32_t) 0x158a1eb0
    ,(uint32_t) 0x005e9a2e, (uint32_t) 0xa5bfce8a, (uint32_t) 0xee1e403e, (uint32_t) 0x1751fb12
    ,(uint32_t) 0x1c43a7a2, (uint32_t) 0x1b2260c3, (uint32_t) 0xad131201, (uint32_t) 0x6ecc6fa9
    ,(uint32_t) 0xb261f7cd, (uint32_t) 0xc22719ec, (uint32_t) 0x406f35ba, (uint32_t) 0x8f3594b8
    ,(uint32_t) 0x01827322, (uint32_t) 0x58869d92, (uint32_t) 0xce22bc41, (uint32_t) 0x985001f7
    ,(uint32_t) 0x59018f62, (uint32_t) 0x7ef7e00d, (uint32_t) 0x3d97e7b4, (uint32_t) 0xeb4a8ce9
    ,(uint32_t) 0x653149b5, (uint32_t) 0xb9b1bd56, (uint32_t) 0x8826f908, (uint32_t) 0x4879b0bc
    ,(uint32_t) 0x039c4813, (uint32_t) 0x4e9f1e41, (uint32_t) 0x6bf71157, (uint32_t) 0x83e79473
    ,(uint32_t) 0x1c43a7a2, (uint32_t) 0x1b2260c3, (uint32_t) 0xad131201, (uint32_t) 0x6ecc6fa9
    ,(uint32_t) 0xb261f7cd, (uint32_t) 0xc22719ec, (uint32_t) 0x406f35ba, (uint32_t) 0x8f3594b8
    ,(uint32_t) 0x4a06906b, (uint32_t) 0x16a65bd1, (uint32_t) 0x0775f676, (uint32_t) 0x60bc0171
    ,(uint32_t) 0xe80ff914, (uint32_t) 0xffbb4fe2, (uint32_t) 0xb5d1c83a, (uint32_t) 0x3cba3916
    ,(uint32_t) 0x93a7a89a, (uint32_t) 0xe9698f83, (uint32_t) 0x603ff567, (uint32_t) 0x921aff0f
    ,(uint32_t) 0x7ed2593d, (uint32_t) 0x77a02588, (uint32_t) 0xda335d98, (uint32_t) 0xf7831c82
    ,(uint32_t) 0xabf08563, (uint32_t) 0xfbf4b637, (uint32_t) 0x47fce443, (uint32_t) 0xd7f8608e
    ,(uint32_t) 0xb05d9f8d, (uint32_t) 0x4ad5fea3, (uint32_t) 0x1984eff1, (uint32_t) 0x4581f5ed
    ,(uint32_t) 0xa49353aa, (uint32_t) 0x2950740d, (uint32_t) 0x645c3fdf, (uint32_t) 0x79d0ecbb
};
static uint32_t viss_checksums_yuv422_yuyv_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0x18822271, (uint32_t) 0x4147e932, (uint32_t) 0x1acfb494, (uint32_t) 0x5bc119e0,
    (uint32_t) 0x18822271, (uint32_t) 0x4147e932, (uint32_t) 0x1acfb494, (uint32_t) 0x5bc119e0,
    (uint32_t) 0xb068f4e5, (uint32_t) 0xda658095, (uint32_t) 0xf63956ca, (uint32_t) 0x9ad6cc46,
    (uint32_t) 0x434c9d11, (uint32_t) 0xbc1227c2, (uint32_t) 0x877d33e2, (uint32_t) 0x2e9af183,
    (uint32_t) 0x434c9d11, (uint32_t) 0xbc1227c2, (uint32_t) 0x877d33e2, (uint32_t) 0x2e9af183,
    (uint32_t) 0xa89b2d69, (uint32_t) 0xdc261414, (uint32_t) 0x0f7278e9, (uint32_t) 0x9d13fddb,
    (uint32_t) 0xe0485674, (uint32_t) 0x6cb895f6, (uint32_t) 0x35d76fed, (uint32_t) 0x841680bc,
    (uint32_t) 0x7ddfd0ab, (uint32_t) 0x97dc61c5, (uint32_t) 0xbbbf0d20, (uint32_t) 0xeeb562cc,
    (uint32_t) 0xa7125f86, (uint32_t) 0x114ae16b, (uint32_t) 0xf08fa910, (uint32_t) 0x2b12143f,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000,
    (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000, (uint32_t) 0x00000000
    #ifdef VPAC3L
    ,(uint32_t) 0x57978c2d, (uint32_t) 0x61ac2865, (uint32_t) 0x57978c2d, (uint32_t) 0x61ac2865
    ,(uint32_t) 0x78f45045, (uint32_t) 0x6db658c2, (uint32_t) 0x78f45045, (uint32_t) 0x6db658c2
    ,(uint32_t) 0xcb7618c2, (uint32_t) 0x7b8eaef8, (uint32_t) 0xcb7618c2, (uint32_t) 0x7b8eaef8
    #endif
    ,(uint32_t) 0x56e16617, (uint32_t) 0x81dce836, (uint32_t) 0x922f240f, (uint32_t) 0x4c5ac7e3
    ,(uint32_t) 0x874baa26, (uint32_t) 0xeb52ae1a, (uint32_t) 0x65de8b13, (uint32_t) 0x41a27a0e
    ,(uint32_t) 0x0b4783db, (uint32_t) 0x751c01ca, (uint32_t) 0x77baf8ee, (uint32_t) 0xa8f5e13c
    ,(uint32_t) 0x94b057ae, (uint32_t) 0xa3182df2, (uint32_t) 0xe2bb1afe, (uint32_t) 0xcf6aa0d3
    ,(uint32_t) 0xc892a4e2, (uint32_t) 0x6e8e17f6, (uint32_t) 0xbf3725a3, (uint32_t) 0x98c0a7b1
    ,(uint32_t) 0xdffff077, (uint32_t) 0x1a86043b, (uint32_t) 0x2d594cf6, (uint32_t) 0x614f5119
    ,(uint32_t) 0x55056a51, (uint32_t) 0x33076c03, (uint32_t) 0x1786a1b8, (uint32_t) 0xd1474a16
    ,(uint32_t) 0x41dc928d, (uint32_t) 0x06f59399, (uint32_t) 0x62ece4b0, (uint32_t) 0x293ee20e
    ,(uint32_t) 0xc2cd1e80, (uint32_t) 0x41562325, (uint32_t) 0x1e976b16, (uint32_t) 0x4bbbe190
    ,(uint32_t) 0x16dc4a1a, (uint32_t) 0x0c02dd64, (uint32_t) 0xa2264cbf, (uint32_t) 0x54d48364
    ,(uint32_t) 0x3c70e1fc, (uint32_t) 0xbcc86f5f, (uint32_t) 0x1038c3ac, (uint32_t) 0x62f87964
    ,(uint32_t) 0xb17a8853, (uint32_t) 0xa128825b, (uint32_t) 0xea440e5d, (uint32_t) 0xcfec92f1
    ,(uint32_t) 0x55056a51, (uint32_t) 0x33076c03, (uint32_t) 0x1786a1b8, (uint32_t) 0xd1474a16
    ,(uint32_t) 0x41dc928d, (uint32_t) 0x06f59399, (uint32_t) 0x62ece4b0, (uint32_t) 0x293ee20e
    ,(uint32_t) 0x36c108dc, (uint32_t) 0x8887ccc9, (uint32_t) 0x6c8db659, (uint32_t) 0xae66de28
    ,(uint32_t) 0x0e44a452, (uint32_t) 0xba5aa6ac, (uint32_t) 0xd229fd70, (uint32_t) 0xbab1dae0
    ,(uint32_t) 0xa3d26424, (uint32_t) 0x66248013, (uint32_t) 0x3eaed5ad, (uint32_t) 0x19e27eb5
    ,(uint32_t) 0xe96fd789, (uint32_t) 0x9c696fa7, (uint32_t) 0x3f4a03db, (uint32_t) 0x838af325
    ,(uint32_t) 0xf13a8c0c, (uint32_t) 0xf58b4241, (uint32_t) 0xfce80a8f, (uint32_t) 0xf878580b
    ,(uint32_t) 0x56fcff91, (uint32_t) 0xd095fa00, (uint32_t) 0x807bb414, (uint32_t) 0x7da7b31a
    ,(uint32_t) 0x83bb2c85, (uint32_t) 0x4933dbce, (uint32_t) 0x5819d8f0, (uint32_t) 0xcdb49660
};

static uint32_t viss_checksums_h3a_ref[VISS_CHECKSUMS_H3A_REF_SIZE] = {
    (uint32_t) 0x00000000, (uint32_t) 0xccff4f2d, (uint32_t) 0x2d7a8f71, (uint32_t) 0x7b0ba698, (uint32_t) 0x824ffb91
    #ifdef VPAC3L
    ,(uint32_t) 0x3d1ff5ac
    #endif
    ,(uint32_t) 0x14af31d5, (uint32_t) 0x560c7a50, (uint32_t) 0x560c7a50, (uint32_t) 0x6896a0fd, (uint32_t) 0x560c7a50, (uint32_t) 0x16618545, (uint32_t) 0x560c7a50
};


static uint32_t get_checksum(uint32_t *table, vx_int32 dcc, vx_int32 results_2a, vx_int32 bypass_glbce, vx_int32 bypass_nsf4)
{
    return table[(2*2*3)*dcc + (2*2)*results_2a + (2)*bypass_glbce + bypass_nsf4];
}

#define ADD_OP_FMT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=YUV420_NV12", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=YUV422_UYVY", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=YUV422_YUYV", __VA_ARGS__, 2))

#define ADD_GLBCE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/glbce=on", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/glbce=bypass", __VA_ARGS__, 1))

#define ADD_NSF4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/nsf4=on", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/nsf4=bypass", __VA_ARGS__, 1))

#define ADD_2A(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/2a=NULL", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/2a=invalid", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/2a=valid", __VA_ARGS__, 2))

#ifdef VPAC3L
#define ADD_DCC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dcc=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0233", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ov2312", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=14bit_companded_inp", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=CAC_inp", __VA_ARGS__, 7))
#elif defined(VPAC3)
#define ADD_DCC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dcc=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0233", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx_390_14bit_inp", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=CAC_inp", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390_gamma", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390_rfe_b1", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390_ccmv", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390_lsc", __VA_ARGS__, 10)),\
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390_cfa", __VA_ARGS__, 11))
#else
#define ADD_DCC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dcc=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0233", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx_390_14bit_inp", __VA_ARGS__, 5))
#endif

#if 0
    CT_EXPAND(nextmacro(testArgName "/dcc=ub9xx", __VA_ARGS__, 3))
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0820", __VA_ARGS__, 4))
#endif

#define PARAMETERS_DCC \
    CT_GENERATE_PARAMETERS("cksm", ADD_OP_FMT, ADD_DCC, ADD_2A, ADD_GLBCE, ADD_NSF4, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

#define ADD_SAFETY_MECHANISM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=reconfigReinitReg", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=statusRegValidate", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=ConfigRegValidate", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/safety_mechanism=all", __VA_ARGS__, 4))

#define PARAMETERS_DCC_SAFETY \
    CT_GENERATE_PARAMETERS("cksm", ADD_OP_FMT, ADD_DCC, ADD_2A, ADD_GLBCE, ADD_NSF4, ADD_SAFETY_MECHANISM, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDcc, ArgDcc, PARAMETERS_DCC)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image y8_r8_c2[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_distribution raw_hist[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    #ifdef VPAC3L
    if(7 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(6 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/1936x1096_R_14b.raw";
        downshift_bits = 0;
    }
    else if(5 == arg_->dcc)
    {
        raw_params.width = 1600;
        raw_params.height = 1300;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 2312;
        sensor_name = SENSOR_OV2312_UB953_LI;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
        downshift_bits = 0;
    }
    else
    #else
    if(11 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(10 == arg_->dcc)
    {
        raw_params.width = 1920;
        raw_params.height = 1080;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/imx219-image-10b-D50.raw";
        downshift_bits = 0;
    }
    else if(9 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(8 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(7 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(6 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(5 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/1936x1096_R_14b.raw";
        downshift_bits = 0;
    }
    else
    #endif
    if(4 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 820;
        sensor_name = SENSOR_ONSEMI_AR0820_UB953_LI;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/AR0820_001/AR0820_12bWDR_3840x2160_GRBG.raw";
        downshift_bits = 0;
    }
    else if(3 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 9702;
        sensor_name = UB9XX_RAW_TESTPAT;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/UB960/ub960_test_pattern_3840x2160.raw";
        downshift_bits = 0;
    }
    else if(2 == arg_->dcc)
    {
        raw_params.width = 2048;
        raw_params.height = 1280;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 233;
        sensor_name = SENSOR_ONSEMI_AR0233_UB953_MARS;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/AR0233_001/input1.raw";
        downshift_bits = 4;
    }
    else
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    #ifdef VPAC3L
    if(6 == arg_->dcc)
        raw_params.format[0].msb = 13;
    else if(5 == arg_->dcc)
        raw_params.format[0].msb = 9;
    else
    #else
    if(5 == arg_->dcc)
        raw_params.format[0].msb = 13;
    else
    #endif
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                if(arg_->op_fmt == 0U)
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
                }
                else if (arg_->op_fmt == 1U)
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
                }
                else
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
                }
                
                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
                params.fcp[0].mux_output0 = 0;
                params.fcp[0].mux_output1 = 0;
                if(arg_->op_fmt == 0U)
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                }
                else if(arg_->op_fmt == 1U)
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                }
                else
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                }                
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
                else
                #endif
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
                params.bypass_glbce = arg_->bypass_glbce;
                params.bypass_nsf4 = arg_->bypass_nsf4;

                #ifdef VPAC3L
                if(7 == arg_->dcc)
                #else
                if(6 == arg_->dcc)
                #endif
                {
                    params.bypass_cac = 0;
                }
                else
                {
                    params.bypass_cac = 1;
                }

                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.bypass_pcid = 0;
                #endif

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 1;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        #ifdef VPAC3L
                        if(5 == arg_->dcc)
                            ae_awb_params.awb_valid = 0;
                        else
                        #endif
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                if(0 != arg_->dcc)
                {
                    #ifdef VPAC3L
                    if(7 == arg_->dcc)
                    #else
                    if(11 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CFA_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_cfa[DCC_VISS_IMX390_WDR_CFA_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CFADCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cfa, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(10 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_LSC_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_lsc[DCC_VISS_IMX390_WDR_LSC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_LSCDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_lsc, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(9 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CCMV_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_ccmv[DCC_VISS_IMX390_WDR_CCMV_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CCMVDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_ccmv, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(8 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_RFE_B1_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_rfe_b1[DCC_VISS_IMX390_WDR_RFE_B1_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_RFE_B1DCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_rfe_b1, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(7 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_GAMMA_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_gamma[DCC_VISS_IMX390_WDR_GAMMA_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_GAMMADCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_gamma, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(6 == arg_->dcc)
                    #endif
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_cac[DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CACDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cac, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    #ifdef VPAC3L
                    else if(6 == arg_->dcc)
                    #else
                    else if(5 == arg_->dcc)
                    #endif
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_14B_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_14b[DCC_VISS_IMX390_WDR_14B_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_14BDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_14b, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else
                    {
                        /* Creating DCC */
                        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                        ASSERT(dcc_status == 0);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    
                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if(NULL != h3a_aew_af[i])
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                    }

                }
                #if defined (VPAC3) || defined (VPAC3L)
                /* Creating raw hist output */
                int nbins = 128U;
                int offset = 0U;
                int range = 128U;
                ASSERT_VX_OBJECT(raw_hist[i] = vxCreateDistribution(context, nbins, offset, range),
                    (enum vx_type_e)VX_TYPE_DISTRIBUTION);
                #endif

                ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, y8_r8_c2[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, raw_hist[i]), VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        if(arg_->bypass_glbce == 0)
        {
            VX_CALL(vxProcessGraph(graph));
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_dcc_out_", i, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_viss_output_image(file, y8_r8_c2[i]);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                if(arg_->op_fmt == 0U)
                {
                    checksum_expected = get_checksum(viss_checksums_luma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                else if(arg_->op_fmt == 1U)
                {
                    checksum_expected = get_checksum(viss_checksums_yuv422_uyvy_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                else
                {
                    checksum_expected = get_checksum(viss_checksums_yuv422_yuyv_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                if(arg_->op_fmt == 0U)
                {
                    rect.end_x = width/2;
                    rect.end_y = height/2;
                    checksum_expected = get_checksum(viss_checksums_chroma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                    checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 1, rect);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    #endif
                    ASSERT(checksum_expected == checksum_actual);
                }
                
                if( (0 != arg_->dcc) && (NULL != h3a_aew_af[i]))
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    /* TIOVX-1247: Setting the cpu_id and channel_id to 0 indiscriminately in order to re-use the same checksums */
                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    printf("%zu\n", h3a_valid_size);
                    #endif
                    ASSERT(viss_checksums_h3a_ref[arg_->dcc] == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&y8_r8_c2[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                if(0 != arg_->dcc)
                {
                    VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                    VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
                }
                #if defined (VPAC3) || defined (VPAC3L)
                VX_CALL(vxReleaseDistribution(&raw_hist[i]));
                #endif
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(y8_r8_c2[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccSafety, ArgDcc_safety, PARAMETERS_DCC_SAFETY)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image y8_r8_c2[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_distribution raw_hist[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    tivx_vpac_viss_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference safety_ref[TEST_NUM_NODE_INSTANCE][1] = {NULL};

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    #ifdef VPAC3L
    if(7 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(6 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/1936x1096_R_14b.raw";
        downshift_bits = 0;
    }
    else if(5 == arg_->dcc)
    {
        raw_params.width = 1600;
        raw_params.height = 1300;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 2312;
        sensor_name = SENSOR_OV2312_UB953_LI;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
        downshift_bits = 0;
    }
    else
    #else
    if(11 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(10 == arg_->dcc)
    {
        raw_params.width = 1920;
        raw_params.height = 1080;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/imx219-image-10b-D50.raw";
        downshift_bits = 0;
    }
    else if(9 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(8 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(7 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(6 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }
    else if(5 == arg_->dcc)
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/1936x1096_R_14b.raw";
        downshift_bits = 0;
    }
    else
    #endif
    if(4 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 820;
        sensor_name = SENSOR_ONSEMI_AR0820_UB953_LI;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/AR0820_001/AR0820_12bWDR_3840x2160_GRBG.raw";
        downshift_bits = 0;
    }
    else if(3 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 9702;
        sensor_name = UB9XX_RAW_TESTPAT;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/UB960/ub960_test_pattern_3840x2160.raw";
        downshift_bits = 0;
    }
    else if(2 == arg_->dcc)
    {
        raw_params.width = 2048;
        raw_params.height = 1280;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 233;
        sensor_name = SENSOR_ONSEMI_AR0233_UB953_MARS;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/AR0233_001/input1.raw";
        downshift_bits = 4;
    }
    else
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    #ifdef VPAC3L
    if(6 == arg_->dcc)
        raw_params.format[0].msb = 13;
    else if(5 == arg_->dcc)
        raw_params.format[0].msb = 9;
    else
    #else
    if(5 == arg_->dcc)
        raw_params.format[0].msb = 13;
    else
    #endif
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                if(arg_->op_fmt == 0U)
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
                }
                else if (arg_->op_fmt == 1U)
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
                }
                else
                {
                    ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
                }
                
                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
                params.fcp[0].mux_output0 = 0;
                params.fcp[0].mux_output1 = 0;
                if(arg_->op_fmt == 0U)
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                }
                else if(arg_->op_fmt == 1U)
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                }
                else
                {
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                }                
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
                else
                #endif
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
                params.bypass_glbce = arg_->bypass_glbce;
                params.bypass_nsf4 = arg_->bypass_nsf4;

                #ifdef VPAC3L
                if(7 == arg_->dcc)
                #else
                if(6 == arg_->dcc)
                #endif
                {
                    params.bypass_cac = 0;
                }
                else
                {
                    params.bypass_cac = 1;
                }

                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.bypass_pcid = 0;
                #endif

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 1;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        #ifdef VPAC3L
                        if(5 == arg_->dcc)
                            ae_awb_params.awb_valid = 0;
                        else
                        #endif
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                if(0 != arg_->dcc)
                {
                    #ifdef VPAC3L
                    if(7 == arg_->dcc)
                    #else
                    if(11 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CFA_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_cfa[DCC_VISS_IMX390_WDR_CFA_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CFADCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cfa, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(10 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_LSC_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_lsc[DCC_VISS_IMX390_WDR_LSC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_LSCDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_lsc, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(9 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CCMV_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_ccmv[DCC_VISS_IMX390_WDR_CCMV_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CCMVDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_ccmv, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(8 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_RFE_B1_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_rfe_b1[DCC_VISS_IMX390_WDR_RFE_B1_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_RFE_B1DCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_rfe_b1, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(7 == arg_->dcc)
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_GAMMA_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_gamma[DCC_VISS_IMX390_WDR_GAMMA_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_GAMMADCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_gamma, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else if(6 == arg_->dcc)
                    #endif
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_cac[DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CACDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cac, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    #ifdef VPAC3L
                    else if(6 == arg_->dcc)
                    #else
                    else if(5 == arg_->dcc)
                    #endif
                    {
                        /* Creating DCC */
                        dcc_buff_size = DCC_VISS_IMX390_WDR_14B_DCC_CFG_NUM_ELEM;

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                        
                        static uint8_t  dcc_viss_imx390_wdr_14b[DCC_VISS_IMX390_WDR_14B_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_14BDCC_CFG;
                        memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_14b, dcc_buff_size);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    else
                    {
                        /* Creating DCC */
                        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                        ASSERT(dcc_status == 0);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    }
                    
                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if(NULL != h3a_aew_af[i])
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                    }

                }
                #if defined (VPAC3) || defined (VPAC3L)
                /* Creating raw hist output */
                int nbins = 128U;
                int offset = 0U;
                int range = 128U;
                ASSERT_VX_OBJECT(raw_hist[i] = vxCreateDistribution(context, nbins, offset, range),
                    (enum vx_type_e)VX_TYPE_DISTRIBUTION);
                #endif

                ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, y8_r8_c2[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, raw_hist[i]), VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        for (i=0; i<TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                tivx_vpac_viss_safety_mech_prms_init(&safety_mechanism_params);

                if(0U != arg_->enable_safety_mechanism)
                {
                    switch (arg_->enable_safety_mechanism)
                    {
                        case 1:
                            /* safety_mechanism=reconfigReinitReg */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            break;
                        case 2:
                            /* safety_mechanism=statusRegValidate */
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            break;
                        case 3:
                            /* safety_mechanism=ConfigRegValidate */
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        case 4:
                            /* safety_mechanism=all */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        default:
                        /* Invalid value */
                        break;
                    }
                    #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined (SOC_AM62A) || defined (SOC_J722S)
                    for(uint32_t i=0U; i < 8U; i++)
                    {
                        if(vpac0_buffer_allocation_flags[i] == 0U)
                        {
                            /* 256KB allocation for each handle */
                            safety_mechanism_params.config_reg_readback_addr = VISS_VPAC0_READBACK_REG_ADDR + (i * 0x40000U);
                            vpac0_buffer_allocation_flags[i] = 1U;
                            break;
                        }
                    }
                    #endif
                    #if defined(SOC_J784S4) || defined(SOC_J742S2)
                    /* For J784S4/J742S2: VPAC1 (VPAC2_xxx targets) runs on MCU4_0 */
                    if ((arg_->target_string != NULL) && (strstr(arg_->target_string, "VPAC2") != NULL))
                    {
                        for(uint32_t i=0U; i < 8U; i++)
                        {
                            if(vpac1_buffer_allocation_flags[i] == 0U)
                            {
                                /* 256KB allocation for each handle */
                                safety_mechanism_params.config_reg_readback_addr = VISS_VPAC1_READBACK_REG_ADDR + (i * 0x40000U);
                                vpac1_buffer_allocation_flags[i] = 1U;
                                break;
                            }
                        }                        
                    }
                    #endif

                    ASSERT_VX_OBJECT(safety_mechanism_obj[i] = vxCreateUserDataObject(context,
                        "tivx_vpac_viss_safety_mechanism_params_t", sizeof(tivx_vpac_viss_safety_mechanism_params_t),
                        &safety_mechanism_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    safety_ref[i][0] = (vx_reference)safety_mechanism_obj[i];
                    VX_CALL(tivxNodeSendCommand(node[i], 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, safety_ref[i], 1));
                }
            }
        }

        VX_CALL(vxProcessGraph(graph));
        if(arg_->bypass_glbce == 0)
        {
            VX_CALL(vxProcessGraph(graph));
        }
        for (i=0; i<TEST_NUM_NODE_INSTANCE; i++)
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
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_dcc_out_", i, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_viss_output_image(file, y8_r8_c2[i]);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                if(arg_->op_fmt == 0U)
                {
                    checksum_expected = get_checksum(viss_checksums_luma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                else if(arg_->op_fmt == 1U)
                {
                    checksum_expected = get_checksum(viss_checksums_yuv422_uyvy_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                else
                {
                    checksum_expected = get_checksum(viss_checksums_yuv422_yuyv_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                }
                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                if(arg_->op_fmt == 0U)
                {
                    rect.end_x = width/2;
                    rect.end_y = height/2;
                    checksum_expected = get_checksum(viss_checksums_chroma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                    checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 1, rect);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    #endif
                    ASSERT(checksum_expected == checksum_actual);
                }
                
                if( (0 != arg_->dcc) && (NULL != h3a_aew_af[i]))
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    /* TIOVX-1247: Setting the cpu_id and channel_id to 0 indiscriminately in order to re-use the same checksums */
                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    printf("%zu\n", h3a_valid_size);
                    #endif
                    ASSERT(viss_checksums_h3a_ref[arg_->dcc] == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&y8_r8_c2[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                if(0 != arg_->dcc)
                {
                    VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                    VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
                }
                #if defined (VPAC3) || defined (VPAC3L)
                VX_CALL(vxReleaseDistribution(&raw_hist[i]));
                #endif
            }
        }

        VX_CALL(vxReleaseGraph(&graph));
        #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined (SOC_AM62A)|| defined (SOC_J722S)
        for(uint8_t i = 0; i < 8U; i++)
        {
            vpac0_buffer_allocation_flags[i] = 0U;
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            vpac1_buffer_allocation_flags[i] = 0U;
            #endif
        }
        #endif


        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                    ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(y8_r8_c2[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int width, height, exposures, mux0, mux1, mux2, mux3, mux4;
    vx_bool line_interleaved;
    char* target_string;
} Arg_mux;


#define ADD_LINE_FALSE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=false", __VA_ARGS__, vx_false_e))

#define ADD_LINE_TRUE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=true", __VA_ARGS__, vx_true_e))

#define ADD_MUX0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=5", __VA_ARGS__, 5))

#define ADD_MUX1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux1=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=5", __VA_ARGS__, 5))

#define ADD_MUX2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux2=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=5", __VA_ARGS__, 5))

#define ADD_MUX3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux3=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=5", __VA_ARGS__, 5))

#define ADD_MUX4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux4=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=5", __VA_ARGS__, 5))

/* TODO: Add DCC as option */
/* TODO: Add h3a_in as option */
/* TODO: Add chroma mode as option */
/* TODO: Alternate P12, U16 */
/* TODO: Negative tests w/ correct mux but incorrect val */
/* TODO: Fix failing scenarios:
   mux0 = 4, mux1 = 0 */

#define MUX_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_64x48, ADD_EXP1, ADD_MUX0, ADD_MUX1, ADD_MUX2, ADD_MUX3, ADD_MUX4, ADD_LINE_FALSE, ADD_SET_TARGET_PARAMETERS, ARG), \

TEST_WITH_ARG(tivxHwaVpacViss, testMux, Arg_mux,
    MUX_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        if (TIVX_VPAC_VISS_MUX2_YUV422 != arg_->mux2)
        {
            if (TIVX_VPAC_VISS_MUX0_Y12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX0_VALUE12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX0_NV12_P12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else
            {
                y12 = NULL;
            }

            if (TIVX_VPAC_VISS_MUX0_NV12_P12 == arg_->mux0)
            {
                uv12_c1 = NULL;
            }
            else
            {
                if (TIVX_VPAC_VISS_MUX1_UV12 == arg_->mux1)
                {
                    ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
                }
                else if (TIVX_VPAC_VISS_MUX1_C1 == arg_->mux1)
                {
                    ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
                }
                else
                {
                    uv12_c1 = NULL;
                }
            }
        }

        if (TIVX_VPAC_VISS_MUX2_Y8 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_RED == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_C2 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_VALUE8 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_NV12 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
        else
        {
            y8_r8_c2 = NULL;
        }

        if ((TIVX_VPAC_VISS_MUX2_NV12 == arg_->mux2) ||
            (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2))
        {
            uv8_g8_c3 = NULL;
        }
        else
        {
            if (TIVX_VPAC_VISS_MUX3_UV8 == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX3_GREEN == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX3_C3 == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else
            {
                uv8_g8_c3 = NULL;
            }
        }

        if (TIVX_VPAC_VISS_MUX4_BLUE == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX4_C4 == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX4_SAT == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else
        {
            s8_b8_c4 = NULL;
        }

        //ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = arg_->mux0;
        params.fcp[0].mux_output1 = arg_->mux1;
        params.fcp[0].mux_output2 = arg_->mux2;
        params.fcp[0].mux_output3 = arg_->mux3;
        params.fcp[0].mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.h3a_in = 0;

        if (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2)
        {
            params.fcp[0].chroma_mode = 1;
        }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (VX_SUCCESS == vxVerifyGraph(graph))
        {
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));

        if (NULL != s8_b8_c4)
        {
            VX_CALL(vxReleaseImage(&s8_b8_c4));
            ASSERT(s8_b8_c4 == 0);
        }

        if ( NULL != uv8_g8_c3 )
        {
            VX_CALL(vxReleaseImage(&uv8_g8_c3));
            ASSERT(uv8_g8_c3 == 0);
        }

        if (NULL != y8_r8_c2)
        {
            VX_CALL(vxReleaseImage(&y8_r8_c2));
            ASSERT(y8_r8_c2 == 0);
        }

        if ( NULL != uv12_c1 )
        {
            VX_CALL(vxReleaseImage(&uv12_c1));
            ASSERT(uv12_c1 == 0);
        }

        if ( NULL != y12 )
        {
            VX_CALL(vxReleaseImage(&y12));
            ASSERT(y12 == 0);
        }

        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);

        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacViss, testMuxNegative, Arg_mux,
    MUX_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = arg_->mux0;
        params.fcp[0].mux_output1 = arg_->mux1;
        params.fcp[0].mux_output2 = arg_->mux2;
        params.fcp[0].mux_output3 = arg_->mux3;
        params.fcp[0].mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (0 == isMuxValid(arg_->mux0, arg_->mux1, arg_->mux2, arg_->mux3, arg_->mux4))
        {
            EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingRaw, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_image y12_ref = NULL, uv12_c1_ref = NULL, y8_r8_c2_ref = NULL, uv8_g8_c3_ref = NULL, s8_b8_c4_ref = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram_ref = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_user_data_object h3a_aew_af_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual = 0;
    vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1280;
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/bayer_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        //VX_CALL(vxProcessGraph(graph));

        /* Check output */

#if CHECK_OUTPUT

        ASSERT_VX_OBJECT(y12_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1_ref = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3_ref = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram_ref = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(h3a_aew_af_ref = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ct_read_raw_image_to_vx_image(y12_ref, "tivx/out_y12.raw", 2);
        ct_read_raw_image_to_vx_image(uv12_c1_ref, "tivx/out_uv12.raw", 2);
        ct_read_raw_image_to_vx_image(y8_r8_c2_ref, "tivx/out_y8_ee.raw", 2);
        ct_read_raw_image_to_vx_image(uv8_g8_c3_ref, "tivx/out_uv8.raw", 2);
        ct_read_raw_image_to_vx_image(s8_b8_c4_ref, "tivx/out_s8.raw", 2);
        ct_read_hist(histogram_ref, "tivx/out_hist.raw", 4);
        ct_read_user_data_object(h3a_aew_af_ref, "tivx/out_h3a.raw", 4);

        ASSERT(ct_cmp_image2(y12, y12_ref) == 0);
        ASSERT(ct_cmp_image2(uv12_c1, uv12_c1_ref) == 0);
        ASSERT(ct_cmp_image2(y8_r8_c2, y8_r8_c2_ref) == 0);
        ASSERT(ct_cmp_image2(uv8_g8_c3, uv8_g8_c3_ref) == 0);
        ASSERT(ct_cmp_image2(s8_b8_c4, s8_b8_c4_ref) == 0);
        ASSERT(ct_cmp_hist(histogram, histogram_ref) == 0);

        {
            uint32_t *data_ptr;
            tivx_h3a_data_t *h3a_out;
            uint8_t *ptr, *ref_ptr;
            int32_t error = 0;
            vx_map_id map_id, map_id_ref;
            int32_t i;

            vxMapUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &map_id,
                (void *)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST, 0);
            h3a_out = (tivx_h3a_data_t*)data_ptr;
            ptr = (uint8_t *)&h3a_out->data;

            vxMapUserDataObject(h3a_aew_af_ref, 0, sizeof(tivx_h3a_data_t), &map_id_ref,
                (void *)&ref_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST, 0);

            for(i=0; i<h3a_out->size; i++)
            {
                if(ptr[i] != ref_ptr[i])
                {
                    error++;
                }
            }
            vxUnmapUserDataObject(h3a_aew_af, map_id);
            vxUnmapUserDataObject(h3a_aew_af_ref, map_id_ref);
            ASSERT(error == 0);
        }

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_ref));
        VX_CALL(vxReleaseDistribution(&histogram_ref));
        VX_CALL(vxReleaseImage(&s8_b8_c4_ref));
        VX_CALL(vxReleaseImage(&uv8_g8_c3_ref));
        VX_CALL(vxReleaseImage(&y8_r8_c2_ref));
        VX_CALL(vxReleaseImage(&uv12_c1_ref));
        VX_CALL(vxReleaseImage(&y12_ref));
#endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = raw_params.width;
        rect.end_y = raw_params.height;

        /* For visual verification */
        save_image_from_viss(y8_r8_c2, "output/out_y8");

        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(0xd45afbd4 == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int negative_test;
    int condition;
    char* target_string;
} ArgNegative;

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=bypass_glbce", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=bypass_nsf4", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_in", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_aewb_af_mode", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ee_mode", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=chroma_mode", __VA_ARGS__, 10)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_source_data", __VA_ARGS__, 11)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ae_valid", __VA_ARGS__, 12)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ae_converged", __VA_ARGS__, 13)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=awb_valid", __VA_ARGS__, 14)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=awb_converged", __VA_ARGS__, 15))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))
#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 4;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 5;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 5;
                }
                else
                {
                    params.fcp[0].mux_output0 = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 3;
                }
                else
                {
                    params.fcp[0].mux_output1 = 1;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output2 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output2 = 5;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                break;
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 3;
                }
                else
                {
                    params.fcp[0].mux_output3 = 3;
                }
                break;
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 3;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 4;
                }
                else
                {
                    params.fcp[0].mux_output4 = 4;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.bypass_glbce = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.bypass_glbce = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.bypass_glbce = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.bypass_glbce = 2;
                }
                else
                {
                    params.bypass_glbce = 2;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.bypass_nsf4 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.bypass_nsf4 = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.bypass_nsf4 = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.bypass_nsf4 = 2;
                }
                else
                {
                    params.bypass_nsf4 = 2;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.h3a_in = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.h3a_in = 3;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                break;
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 2;
                }
                else
                {
                    params.h3a_aewb_af_mode = 2;
                }
                break;
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 3;
                }
                else
                {
                    params.fcp[0].ee_mode = 3;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 2;
                }
                else
                {
                    params.fcp[0].chroma_mode = 2;
                }
                break;
            }
            case 11:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.h3a_source_data = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.h3a_source_data = 3;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                break;
            }
            case 12:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 2;
                }
                else
                {
                    ae_awb_params.ae_valid = 2;
                }
                break;
            }
            case 13:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 2;
                }
                else
                {
                    ae_awb_params.ae_converged = 2;
                }
                break;
            }
            case 14:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 2;
                }
                else
                {
                    ae_awb_params.awb_valid = 2;
                }
                break;
            }
            case 15:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 2;
                }
                else
                {
                    ae_awb_params.awb_converged = 2;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

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
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#ifdef VPAC3

typedef struct {
    const char* testName;
    int HV_fmt;
    int MV_fmt;
    int LseOutComb_bit;
    int MV_input;
    int HV_EE_enabled;
    int MV_EE_enabled;
    char* target_string;
} Arg_HV_MV;

static uint32_t HV_8_YUV420_checksum[2]             = {(uint32_t) 0x080d9c2d,(uint32_t) 0x3083ca4b};
static uint32_t HV_8_YUV420_cac_checksum[2]         = {(uint32_t) 0xd8b50905,(uint32_t) 0x07614df0};
static uint32_t HV_8_YUV420_EE_checksum             = (uint32_t) 0x7a89ab4a;
static uint32_t HV_8_YUV422_UYVY_checksum           = (uint32_t) 0x516d0dde;
static uint32_t HV_8_YUV422_YUYV_checksum           = (uint32_t) 0x434c9d11;
static uint32_t HV_8_YUV422_UYVY_cac_checksum       = (uint32_t) 0x308e5316;
static uint32_t HV_8_YUV422_YUYV_cac_checksum       = (uint32_t) 0x94b057ae;
static uint32_t HV_8_YUV422_UYVY_EE_checksum        = (uint32_t) 0x97adb3de;
static uint32_t HV_8_YUV422_YUYV_EE_checksum        = (uint32_t) 0x5c92ddb7;
static uint32_t HV_8_YUV422_UYVY_cac_EE_checksum    = (uint32_t) 0x7f890e16;
static uint32_t HV_8_YUV422_YUYV_cac_EE_checksum    = (uint32_t) 0xa0ff5269;
static uint32_t HV_12_YUV420_checksum[2]            = {(uint32_t) 0xebc5cc8f,(uint32_t) 0xefce0e44};
static uint32_t HV_12_YUV420_cac_checksum[2]        = {(uint32_t) 0xb1b10ca4,(uint32_t) 0xebf8696e};
static uint32_t HV_12_YUV420_EE_checksum            = (uint32_t) 0x1527c387;
static uint32_t MV_8_YUV420_rfe_checksum[2]         = {(uint32_t) 0xdca1c62f,(uint32_t) 0x05cbea74};
static uint32_t MV_8_YUV420_rfe_EE_checksum         = (uint32_t) 0xc11ea6b3;
static uint32_t MV_8_YUV422_UYVY_rfe_checksum       = (uint32_t) 0xf529309b;
static uint32_t MV_8_YUV422_YUYV_rfe_checksum       = (uint32_t) 0x2e9af183;
static uint32_t MV_8_YUV422_UYVY_rfe_EE_checksum    = (uint32_t) 0x766f219b;
static uint32_t MV_8_YUV422_YUYV_rfe_EE_checksum    = (uint32_t) 0x621c3774;
static uint32_t MV_12_YUV420_rfe_checksum[2]        = {(uint32_t) 0x80e080e4,(uint32_t) 0xd55de40d};
static uint32_t MV_12_YUV420_rfe_EE_checksum        = (uint32_t) 0x5f2505e2;
static uint32_t MV_8_YUV420_nsf4_checksum[2]        = {(uint32_t) 0xb019be0a,(uint32_t) 0x2ce1134c};
static uint32_t MV_8_YUV420_nsf4_EE_checksum        = (uint32_t) 0x4c520ecd;
static uint32_t MV_8_YUV422_UYVY_nsf4_checksum      = (uint32_t) 0xd8808df6;
static uint32_t MV_8_YUV422_UYVY_cac_checksum       = (uint32_t) 0xb7cc9e22;
static uint32_t MV_8_YUV422_YUYV_nsf4_checksum      = (uint32_t) 0x877d33e2;
static uint32_t MV_8_YUV422_YUYV_cac_checksum       = (uint32_t) 0xcf6aa0d3;
static uint32_t MV_8_YUV422_UYVY_nsf4_EE_checksum   = (uint32_t) 0x84c37bf6;
static uint32_t MV_8_YUV422_UYVY_cac_EE_checksum    = (uint32_t) 0xa4077b22;
static uint32_t MV_8_YUV422_YUYV_nsf4_EE_checksum   = (uint32_t) 0xb62976d0;
static uint32_t MV_8_YUV422_YUYV_cac_EE_checksum    = (uint32_t) 0x0b56dbb0;
static uint32_t MV_12_YUV420_nsf4_checksum[2]       = {(uint32_t) 0x67708e03,(uint32_t) 0xa7e42d9d};
static uint32_t MV_12_YUV420_nsf4_EE_checksum       = (uint32_t) 0xf22873dc;
static uint32_t MV_12_YUV420_cac_checksum[2]        = {(uint32_t) 0xf65f0fbc, (uint32_t) 0x2b4332e4};
static uint32_t MV_12_YUV420_cac_EE_checksum        = (uint32_t) 0xe96a520b;
static uint32_t MV_8_YUV420_cac_EE_checksum         = (uint32_t) 0x4d04a05c;
static uint32_t MV_8_YUV420_cac_checksum[2]         = {(uint32_t) 0xbc0f0a5b, (uint32_t)0xec68fa2b};

#define ADD_HV_FMT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/HV=YUV420_8bit", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/HV=YUV420_12bit", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/HV=YUV422_8bit_UYVY", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/HV=YUV422_8bit_YUYV", __VA_ARGS__, 3))
    
#define ADD_MV_FMT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MV=YUV420_8bit", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/MV=YUV420_12bit", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MV=YUV422_8bit_UYVY", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/MV=YUV422_8bit_YUYV", __VA_ARGS__, 3))

#define ADD_LSEOUTCOMB_BIT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LSEOUT_YUV12=HV", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/LSEOUT_YUV12=MV", __VA_ARGS__, 1))

#define ADD_MV_INPUT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MV_INP=RAWFE", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/MV_INP=NSF4", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MV_INP=GLBCE", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/MV_INP=CAC", __VA_ARGS__, 3))

#define ADD_HV_EE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/HV_EE=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/HV_EE=Y12", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/HV_EE=Y8", __VA_ARGS__, 2))

#define ADD_MV_EE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MV_EE=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/MV_EE=Y12", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MV_EE=Y8", __VA_ARGS__, 2))

#define MV_PARAMETERS \
    CT_GENERATE_PARAMETERS("testMV", ADD_HV_FMT, ADD_MV_FMT, ADD_LSEOUTCOMB_BIT, ADD_MV_INPUT, ADD_HV_EE, ADD_MV_EE, ADD_SET_TARGET_PARAMETERS, ARG), \

TEST_WITH_ARG(tivxHwaVpacViss, testHV_MV, Arg_HV_MV, MV_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual_HV_plane0 = 0, checksum_actual_HV_plane1 = 0, checksum_actual_MV_plane0 = 0, checksum_actual_MV_plane1 = 0;
    vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;
    
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    char *sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    uint32_t sensor_dcc_mode = 0;
    uint32_t sensor_dcc_id = 390;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        if(arg_->LseOutComb_bit == 1U){
            /* HV output format */
            if(arg_->HV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }

            /* MV output format */
            if(arg_->MV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }
        }
        else{
            /* HV output format */
            if(arg_->HV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }

            /* MV output format */
            if(arg_->MV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }
        }

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
            params.sensor_dcc_id = sensor_dcc_id;

            /* Enable EE on HV based on Input */
            if(arg_->HV_EE_enabled == 0U)
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
            }
            else if(arg_->HV_EE_enabled == 1U)
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y12;
            }
            else
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
            }

            if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
            {
                params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
                params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }
            else
            {
                params.fcp[0].mux_output0 = 0;  
                params.fcp[0].mux_output1 = 0;
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }

            if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
            {
                params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }
            else
            {
                params.fcp[1].mux_output0 = 0; 
                params.fcp[1].mux_output1 = 0;
                params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                params.fcp[1].mux_output3 = 0;
                params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }

            /* HV output FCP0 format */
            if(arg_->HV_fmt == 0U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }
            else if (arg_->HV_fmt == 1U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT1);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT1);
                }
            }
            else
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }

            /* MV output FCP1 format */
            if(arg_->MV_fmt == 0U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }
            else if(arg_->MV_fmt == 1U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT1);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT1);
                }
            }
            else
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }

            params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

            params.h3a_in = 3;
            params.h3a_aewb_af_mode = 0;
            params.fcp[0].chroma_mode = 0;
            /* Enable CAC only when MV input is from CAC*/
            if(3U == arg_->MV_input)
            {
                params.bypass_cac = 0;  /* CAC on */
            }
            else
            {
                params.bypass_cac = 1;  /* CAC off */
            }

            /* MV input config */
            switch(arg_->MV_input)
            {
                case 0U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_RFE; /* RAWFE --> FCP1 */
                    break;
                case 1U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_NSF4; /* NSF4 --> FCP1 */
                    break;
                case 2U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_GLBCE; /* GLBCE --> FCP1 */
                    break;
                case 3U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_CAC; /* CAC --> FCP1 */
                    break;
                default:
                    break;
            }

            /* Enable EE on MV based on input */
            if(arg_->MV_EE_enabled == 0U)
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
            }
            else if(arg_->MV_EE_enabled == 1U)
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP1_Y12;
            }
            else
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP1_Y8;
            }
            params.fcp[1].chroma_mode = 0;

            params.enable_ctx = 1;
            params.bypass_glbce = 0;
            params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        /* Creating DCC */
        if(3U == arg_->MV_input)
        {
            dcc_buff_size = DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM;
        }
        else
        {
            dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);
        }

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
        
        if(3U == arg_->MV_input)
        {
            static uint8_t dcc_viss_imx390_wdr_cac[DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CACDCC_CFG;
            memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cac, dcc_buff_size);
        }
        else
        {
            dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
            ASSERT(dcc_status == 0);
        }
            
        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, dcc_param_viss,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        if( /* Disable if output format is YUV422_8bit and EE is on Y12 */
            !(((arg_->HV_fmt == 2 || arg_->HV_fmt == 3) && (arg_->HV_EE_enabled == 1)) || ((arg_->MV_fmt == 2 || arg_->MV_fmt == 3) && (arg_->MV_EE_enabled == 1))) &&
            /* Disable if pipline length of HV and MV is same and EE is disabled on HV and enabled on MV
            (latency on MV cannot be more than HV) */
            !((((arg_->HV_EE_enabled == 0 || (arg_->HV_EE_enabled == 1 && arg_->HV_fmt != 1) || (arg_->HV_EE_enabled == 2 && arg_->HV_fmt == 1)) && ((arg_->MV_EE_enabled == 1 && arg_->MV_fmt == 1) || (arg_->MV_EE_enabled == 2 && arg_->MV_fmt != 1))) &&
               ((arg_->MV_input == 2) ||
                (arg_->MV_input == 1 && params.bypass_glbce == 1) ||
                (arg_->MV_input == 3 && params.bypass_glbce == 1 && params.bypass_nsf4 == 1) ||
                (arg_->MV_input == 0 && params.bypass_glbce == 1 && params.bypass_nsf4 && params.bypass_cac == 1)))))
        {
            /* Positive Test cases */
            VX_CALL(vxVerifyGraph(graph));
        }
        else
        {
            /* Negative test cases of HV-MV */
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }


        ct_read_raw_image(raw, "psdkra/app_single_cam/IMX390_001/input2.raw", 2, 0);

        if( /* Disable if output format is YUV422_8bit and EE is on Y12 */
            !(((arg_->HV_fmt == 2 || arg_->HV_fmt == 3) && (arg_->HV_EE_enabled == 1)) || ((arg_->MV_fmt == 2 || arg_->MV_fmt == 3) && (arg_->MV_EE_enabled == 1))) &&
            /* Disable if pipline length of HV and MV is same and EE is disabled on HV and enabled on MV
            (latency on MV cannot be more than HV) */
            !((((arg_->HV_EE_enabled == 0 || (arg_->HV_EE_enabled == 1 && arg_->HV_fmt != 1) || (arg_->HV_EE_enabled == 2 && arg_->HV_fmt == 1)) && ((arg_->MV_EE_enabled == 1 && arg_->MV_fmt == 1) || (arg_->MV_EE_enabled == 2 && arg_->MV_fmt != 1))) &&
               ((arg_->MV_input == 2) ||
                (arg_->MV_input == 1 && params.bypass_glbce == 1) ||
                (arg_->MV_input == 3 && params.bypass_glbce == 1 && params.bypass_nsf4 == 1) ||
                (arg_->MV_input == 0 && params.bypass_glbce == 1 && params.bypass_nsf4 && params.bypass_cac == 1)))))
        {
            VX_CALL(vxProcessGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }

        /* Checksum verification */
        if( /* Disable if output format is YUV422_8bit and EE is on Y12 */
            !(((arg_->HV_fmt == 2 || arg_->HV_fmt == 3) && (arg_->HV_EE_enabled == 1)) || ((arg_->MV_fmt == 2 || arg_->MV_fmt == 3) && (arg_->MV_EE_enabled == 1))) &&
            /* Disable if pipline length of HV and MV is same and EE is disabled on HV and enabled on MV
            (latency on MV cannot be more than HV) */
            !((((arg_->HV_EE_enabled == 0 || (arg_->HV_EE_enabled == 1 && arg_->HV_fmt != 1) || (arg_->HV_EE_enabled == 2 && arg_->HV_fmt == 1)) && ((arg_->MV_EE_enabled == 1 && arg_->MV_fmt == 1) || (arg_->MV_EE_enabled == 2 && arg_->MV_fmt != 1))) &&
               ((arg_->MV_input == 2) ||
                (arg_->MV_input == 1 && params.bypass_glbce == 1) ||
                (arg_->MV_input == 3 && params.bypass_glbce == 1 && params.bypass_nsf4 == 1) ||
                (arg_->MV_input == 0 && params.bypass_glbce == 1 && params.bypass_nsf4 && params.bypass_cac == 1)))))
        {
            if(arg_->LseOutComb_bit == 0U){
                if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_HV_plane1 = tivx_utils_simple_image_checksum(y12, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);
                }

                if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_MV_plane1 = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                }
            }
            else{
                if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                    
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_HV_plane1 = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                }
                
                if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_MV_plane1 = tivx_utils_simple_image_checksum(y12, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);
                }
            }

            #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("checksum_actual_HV_plane0 = 0x%08x\n", checksum_actual_HV_plane0);
                printf("checksum_actual_HV_plane1 = 0x%08x\n", checksum_actual_HV_plane1);
                printf("checksum_actual_MV_plane0 = 0x%08x\n", checksum_actual_MV_plane0);
                printf("checksum_actual_MV_plane1 = 0x%08x\n", checksum_actual_MV_plane1);
            #endif

            if(params.bypass_cac == 1U)
            {
                switch (arg_->HV_fmt)
                {
                /* HV YUV420 8 bit checksum validation*/
                case 0U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV420_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV420_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_8_YUV420_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV420 12 bit checksum validation*/
                case 1U:
                    if(arg_->HV_EE_enabled == 1)
                    {
                        ASSERT(HV_12_YUV420_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_12_YUV420_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_12_YUV420_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV422 UYVY 8 bit checksum validation*/
                case 2U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_UYVY_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                /* HV YUV422 YUYV 8 bit checksum validation*/
                case 3U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_YUYV_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                
                default:
                    break;
                }
            }
            else if(params.bypass_cac == 0U)
            {
                switch (arg_->HV_fmt)
                {
                /* HV YUV420 8 bit checksum validation CAC enabled*/
                case 0U:
                    if(arg_->HV_EE_enabled == 0)
                    {
                        ASSERT(HV_8_YUV420_cac_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_8_YUV420_cac_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV420 12 bit checksum validation CAC enabled*/
                case 1U:
                    if(arg_->HV_EE_enabled == 0)
                    {
                        ASSERT(HV_12_YUV420_cac_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_12_YUV420_cac_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV422 UYVY 8 bit checksum validation CAC enabled*/
                case 2U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_UYVY_cac_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_cac_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                /* HV YUV422 YUYV 8 bit checksum validation CAC enabled*/
                case 3U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_YUYV_cac_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_cac_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                
                default:
                    break;
                }
            }
            switch (arg_->MV_fmt)
            {
            /* MV YUV420 8 bit checksum validation*/
            case 0U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_rfe_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_rfe_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_nsf4_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_nsf4_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV420_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV420_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(HV_8_YUV420_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_cac_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_cac_checksum[1] == checksum_actual_MV_plane1);
                }
                break;
            /* MV YUV420 12 bit checksum validation*/
            case 1U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_rfe_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_rfe_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_nsf4_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_nsf4_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(HV_12_YUV420_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_12_YUV420_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(HV_12_YUV420_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_cac_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_cac_checksum[1] == checksum_actual_MV_plane1);
                }
                break;
            /* MV YUV422 UYVY 8 bit checksum validation*/
            case 2U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_rfe_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_nsf4_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(HV_8_YUV422_UYVY_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_cac_checksum == checksum_actual_MV_plane0);
                    }
                }
                break;
            /* MV YUV422 YUYV 8 bit checksum validation*/
            case 3U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_rfe_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_nsf4_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(HV_8_YUV422_YUYV_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_cac_checksum == checksum_actual_MV_plane0);
                    }
                }
                break;
            
            default:
                break;
            }
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}
typedef struct {
    const char* testName;
    int HV_fmt;
    int MV_fmt;
    int LseOutComb_bit;
    int MV_input;
    int HV_EE_enabled;
    int MV_EE_enabled;
    char* target_string;
    int enable_safety_mechanism;
} Arg_HV_MV_safety;

#define MV_PARAMETERS_SAFETY \
    CT_GENERATE_PARAMETERS("testMV", ADD_HV_FMT, ADD_MV_FMT, ADD_LSEOUTCOMB_BIT, ADD_MV_INPUT, ADD_HV_EE, ADD_MV_EE, ADD_SET_TARGET_PARAMETERS,ADD_SAFETY_MECHANISM, ARG), \

TEST_WITH_ARG(tivxHwaVpacViss, testHV_MV_safety, Arg_HV_MV_safety, MV_PARAMETERS_SAFETY)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual_HV_plane0 = 0, checksum_actual_HV_plane1 = 0, checksum_actual_MV_plane0 = 0, checksum_actual_MV_plane1 = 0;
    vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_viss_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj = NULL;
    vx_reference safety_ref[1];

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;
    
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    char *sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    uint32_t sensor_dcc_mode = 0;
    uint32_t sensor_dcc_id = 390;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        if(arg_->LseOutComb_bit == 1U){
            /* HV output format */
            if(arg_->HV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }

            /* MV output format */
            if(arg_->MV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }
        }
        else{
            /* HV output format */
            if(arg_->HV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->HV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }

            /* MV output format */
            if(arg_->MV_fmt == 0U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 1U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else if(arg_->MV_fmt == 2U)
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
            }
            else
            {
                ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
            }
        }

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
            params.sensor_dcc_id = sensor_dcc_id;

            /* Enable EE on HV based on Input */
            if(arg_->HV_EE_enabled == 0U)
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
            }
            else if(arg_->HV_EE_enabled == 1U)
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y12;
            }
            else
            {
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
            }

            if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
            {
                params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
                params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }
            else
            {
                params.fcp[0].mux_output0 = 0;  
                params.fcp[0].mux_output1 = 0;
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }

            if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
            {
                params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
                params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }
            else
            {
                params.fcp[1].mux_output0 = 0; 
                params.fcp[1].mux_output1 = 0;
                params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                params.fcp[1].mux_output3 = 0;
                params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            }

            /* HV output FCP0 format */
            if(arg_->HV_fmt == 0U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }
            else if (arg_->HV_fmt == 1U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT1);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT1);
                }
            }
            else
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }

            /* MV output FCP1 format */
            if(arg_->MV_fmt == 0U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }
            else if(arg_->MV_fmt == 1U)
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT1);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT0);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT1);
                }
            }
            else
            {
                if(arg_->LseOutComb_bit == 0U){
                    params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
                else{
                    params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
                    params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
                }
            }

            params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

            params.h3a_in = 3;
            params.h3a_aewb_af_mode = 0;
            params.fcp[0].chroma_mode = 0;
            /* Enable CAC only when MV input is from CAC*/
            if(3U == arg_->MV_input)
            {
                params.bypass_cac = 0;  /* CAC on */
            }
            else
            {
                params.bypass_cac = 1;  /* CAC off */
            }

            /* MV input config */
            switch(arg_->MV_input)
            {
                case 0U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_RFE; /* RAWFE --> FCP1 */
                    break;
                case 1U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_NSF4; /* NSF4 --> FCP1 */
                    break;
                case 2U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_GLBCE; /* GLBCE --> FCP1 */
                    break;
                case 3U:
                    params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_CAC; /* CAC --> FCP1 */
                    break;
                default:
                    break;
            }

            /* Enable EE on MV based on input */
            if(arg_->MV_EE_enabled == 0U)
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
            }
            else if(arg_->MV_EE_enabled == 1U)
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP1_Y12;
            }
            else
            {
                params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP1_Y8;
            }
            params.fcp[1].chroma_mode = 0;

            params.enable_ctx = 1;
            params.bypass_glbce = 0;
            params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        /* Creating DCC */
        if(3U == arg_->MV_input)
        {
            dcc_buff_size = DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM;
        }
        else
        {
            dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);
        }

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
        
        if(3U == arg_->MV_input)
        {
            static uint8_t dcc_viss_imx390_wdr_cac[DCC_VISS_IMX390_WDR_CAC_DCC_CFG_NUM_ELEM] = DCC_VISS_IMX390_WDR_CACDCC_CFG;
            memcpy(dcc_viss_buf, dcc_viss_imx390_wdr_cac, dcc_buff_size);
        }
        else
        {
            dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
            ASSERT(dcc_status == 0);
        }
            
        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, dcc_param_viss,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        ct_read_raw_image(raw, "psdkra/app_single_cam/IMX390_001/input2.raw", 2, 0);

        if( /* Disable if output format is YUV422_8bit and EE is on Y12 */
            !(((arg_->HV_fmt == 2 || arg_->HV_fmt == 3) && (arg_->HV_EE_enabled == 1)) || ((arg_->MV_fmt == 2 || arg_->MV_fmt == 3) && (arg_->MV_EE_enabled == 1))) &&
            /* Disable if pipline length of HV and MV is same and EE is disabled on HV and enabled on MV
            (latency on MV cannot be more than HV) */
            !((((arg_->HV_EE_enabled == 0 || (arg_->HV_EE_enabled == 1 && arg_->HV_fmt != 1) || (arg_->HV_EE_enabled == 2 && arg_->HV_fmt == 1)) && ((arg_->MV_EE_enabled == 1 && arg_->MV_fmt == 1) || (arg_->MV_EE_enabled == 2 && arg_->MV_fmt != 1))) &&
               ((arg_->MV_input == 2) ||
                (arg_->MV_input == 1 && params.bypass_glbce == 1) ||
                (arg_->MV_input == 3 && params.bypass_glbce == 1 && params.bypass_nsf4 == 1) ||
                (arg_->MV_input == 0 && params.bypass_glbce == 1 && params.bypass_nsf4 && params.bypass_cac == 1)))))
        {
            /* Positive Test cases */
            VX_CALL(vxVerifyGraph(graph));

            tivx_vpac_viss_safety_mech_prms_init(&safety_mechanism_params);
            if(0U != arg_->enable_safety_mechanism)
            {
                switch (arg_->enable_safety_mechanism)
                {
                    case 1:
                        /* safety_mechanism=reconfigReinitReg */
                        safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                        break;
                    case 2:
                        /* safety_mechanism=statusRegValidate */
                        safety_mechanism_params.enable_status_reg_validation = 1U;
                        break;
                    case 3:
                        /* safety_mechanism=ConfigRegValidate */
                        safety_mechanism_params.enable_readback_config_registers = 1U;        
                        break;
                    case 4:
                        /* safety_mechanism=all */
                        safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                        safety_mechanism_params.enable_status_reg_validation = 1U;
                        safety_mechanism_params.enable_readback_config_registers = 1U;                           
                        break;
                    default:
                    /* Invalid value */
                    break;
                }
                    #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
                    for(uint32_t i=0U; i < 8U; i++)
                    {
                        if(vpac0_buffer_allocation_flags[i] == 0U)
                        {
                            /* 256KB allocation for each handle */
                            safety_mechanism_params.config_reg_readback_addr = VISS_VPAC0_READBACK_REG_ADDR + (i * 0x40000U);
                            vpac0_buffer_allocation_flags[i] = 1U;
                            break;
                        }
                    }
                    #endif
                    #if defined(SOC_J784S4) || defined(SOC_J742S2)
                    /* For J784S4/J742S2: VPAC1 (VPAC2_xxx targets) runs on MCU4_0 */
                    if ((arg_->target_string != NULL) && (strstr(arg_->target_string, "VPAC2") != NULL))
                    {
                        for(uint32_t i=0U; i < 8U; i++)
                        {
                            if(vpac1_buffer_allocation_flags[i] == 0U)
                            {
                                /* 256KB allocation for each handle */
                                safety_mechanism_params.config_reg_readback_addr = VISS_VPAC1_READBACK_REG_ADDR + (i * 0x40000U);
                                vpac1_buffer_allocation_flags[i] = 1U;
                                break;
                            }
                        }                        
                    }
                    #endif

                ASSERT_VX_OBJECT(safety_mechanism_obj = vxCreateUserDataObject(context,
                    "tivx_vpac_viss_safety_mechanism_params_t", sizeof(tivx_vpac_viss_safety_mechanism_params_t),
                    &safety_mechanism_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                safety_ref[0] = (vx_reference)safety_mechanism_obj;
                VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1));
            }
            VX_CALL(vxProcessGraph(graph));
            if(1U == arg_->enable_safety_mechanism)
            {
                /* safety_mechanism=reconfigReinitReg */
                safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                VX_CALL(vxCopyUserDataObject(safety_mechanism_obj, 0, sizeof(tivx_vpac_viss_safety_mechanism_params_t), &safety_mechanism_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                safety_ref[0] = (vx_reference)safety_mechanism_obj;
                VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, safety_ref, 1));
            }
            VX_CALL(vxProcessGraph(graph));
            if(0U != arg_->enable_safety_mechanism)
            {
                VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj));
            }
        }
        else
        {
            /* Negative test cases of HV-MV */
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }

        /* Checksum verification */
        if( /* Disable if output format is YUV422_8bit and EE is on Y12 */
            !(((arg_->HV_fmt == 2 || arg_->HV_fmt == 3) && (arg_->HV_EE_enabled == 1)) || ((arg_->MV_fmt == 2 || arg_->MV_fmt == 3) && (arg_->MV_EE_enabled == 1))) &&
            /* Disable if pipline length of HV and MV is same and EE is disabled on HV and enabled on MV
            (latency on MV cannot be more than HV) */
            !((((arg_->HV_EE_enabled == 0 || (arg_->HV_EE_enabled == 1 && arg_->HV_fmt != 1) || (arg_->HV_EE_enabled == 2 && arg_->HV_fmt == 1)) && ((arg_->MV_EE_enabled == 1 && arg_->MV_fmt == 1) || (arg_->MV_EE_enabled == 2 && arg_->MV_fmt != 1))) &&
               ((arg_->MV_input == 2) ||
                (arg_->MV_input == 1 && params.bypass_glbce == 1) ||
                (arg_->MV_input == 3 && params.bypass_glbce == 1 && params.bypass_nsf4 == 1) ||
                (arg_->MV_input == 0 && params.bypass_glbce == 1 && params.bypass_nsf4 && params.bypass_cac == 1)))))
        {
            if(arg_->LseOutComb_bit == 0U){
                if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_HV_plane1 = tivx_utils_simple_image_checksum(y12, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);
                }

                if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_MV_plane1 = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                }
            }
            else{
                if(arg_->HV_fmt == 0 || arg_->HV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                    
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_HV_plane1 = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_HV_plane0 = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                }
                
                if(arg_->MV_fmt == 0 || arg_->MV_fmt == 1)
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);

                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width/2;
                    rect.end_y = raw_params.height/2;
                    checksum_actual_MV_plane1 = tivx_utils_simple_image_checksum(y12, 1, rect);
                }
                else
                {
                    rect.start_x = 0;
                    rect.start_y = 0;
                    rect.end_x = raw_params.width;
                    rect.end_y = raw_params.height;
                    checksum_actual_MV_plane0 = tivx_utils_simple_image_checksum(y12, 0, rect);
                }
            }

            #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("checksum_actual_HV_plane0 = 0x%08x\n", checksum_actual_HV_plane0);
                printf("checksum_actual_HV_plane1 = 0x%08x\n", checksum_actual_HV_plane1);
                printf("checksum_actual_MV_plane0 = 0x%08x\n", checksum_actual_MV_plane0);
                printf("checksum_actual_MV_plane1 = 0x%08x\n", checksum_actual_MV_plane1);
            #endif

            if(params.bypass_cac == 1U)
            {
                switch (arg_->HV_fmt)
                {
                /* HV YUV420 8 bit checksum validation*/
                case 0U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV420_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV420_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_8_YUV420_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV420 12 bit checksum validation*/
                case 1U:
                    if(arg_->HV_EE_enabled == 1)
                    {
                        ASSERT(HV_12_YUV420_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_12_YUV420_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_12_YUV420_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV422 UYVY 8 bit checksum validation*/
                case 2U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_UYVY_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                /* HV YUV422 YUYV 8 bit checksum validation*/
                case 3U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_YUYV_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                
                default:
                    break;
                }
            }
            else if(params.bypass_cac == 0U)
            {
                switch (arg_->HV_fmt)
                {
                /* HV YUV420 8 bit checksum validation CAC enabled*/
                case 0U:
                    if(arg_->HV_EE_enabled == 0)
                    {
                        ASSERT(HV_8_YUV420_cac_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_8_YUV420_cac_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV420 12 bit checksum validation CAC enabled*/
                case 1U:
                    if(arg_->HV_EE_enabled == 0)
                    {
                        ASSERT(HV_12_YUV420_cac_checksum[0] == checksum_actual_HV_plane0);
                    }
                    ASSERT(HV_12_YUV420_cac_checksum[1] == checksum_actual_HV_plane1);
                    break;
                /* HV YUV422 UYVY 8 bit checksum validation CAC enabled*/
                case 2U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_UYVY_cac_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_cac_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                /* HV YUV422 YUYV 8 bit checksum validation CAC enabled*/
                case 3U:
                    if(arg_->HV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV422_YUYV_cac_EE_checksum == checksum_actual_HV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_cac_checksum == checksum_actual_HV_plane0);
                    }
                    break;
                
                default:
                    break;
                }
            }
            switch (arg_->MV_fmt)
            {
            /* MV YUV420 8 bit checksum validation*/
            case 0U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_rfe_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_rfe_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_nsf4_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_nsf4_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(HV_8_YUV420_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV420_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(HV_8_YUV420_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2)
                    {
                        ASSERT(MV_8_YUV420_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV420_cac_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_8_YUV420_cac_checksum[1] == checksum_actual_MV_plane1);
                }
                break;
            /* MV YUV420 12 bit checksum validation*/
            case 1U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_rfe_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_rfe_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_nsf4_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_nsf4_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(HV_12_YUV420_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_12_YUV420_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(HV_12_YUV420_checksum[1] == checksum_actual_MV_plane1);
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 1)
                    {
                        ASSERT(MV_12_YUV420_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_12_YUV420_cac_checksum[0] == checksum_actual_MV_plane0);
                    }
                    ASSERT(MV_12_YUV420_cac_checksum[1] == checksum_actual_MV_plane1);
                }
                break;
            /* MV YUV422 UYVY 8 bit checksum validation*/
            case 2U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_rfe_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_nsf4_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(HV_8_YUV422_UYVY_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_UYVY_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_UYVY_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_UYVY_cac_checksum == checksum_actual_MV_plane0);
                    }
                }
                break;
            /* MV YUV422 YUYV 8 bit checksum validation*/
            case 3U:
                /* MV input from RFE*/
                if(arg_->MV_input == 0U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_rfe_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_rfe_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from NSF4*/
                else if(arg_->MV_input == 1U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_nsf4_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_nsf4_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from GLBCE*/
                else if(arg_->MV_input == 2U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(HV_8_YUV422_YUYV_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(HV_8_YUV422_YUYV_checksum == checksum_actual_MV_plane0);
                    }
                }
                /* MV input from CAC*/
                else if(arg_->MV_input == 3U)
                {
                    if(arg_->MV_EE_enabled == 2U)
                    {
                        ASSERT(MV_8_YUV422_YUYV_cac_EE_checksum == checksum_actual_MV_plane0);
                    }
                    else
                    {
                        ASSERT(MV_8_YUV422_YUYV_cac_checksum == checksum_actual_MV_plane0);
                    }
                }
                break;
            
            default:
                break;
            }
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
        for(uint8_t i = 0; i < 8U; i++)
        {
            vpac0_buffer_allocation_flags[i] = 0U;
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            vpac1_buffer_allocation_flags[i] = 0U;
            #endif
        }
        #endif
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);
        ASSERT(safety_mechanism_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif

#ifdef VPAC3L

typedef struct {
    const char* testName;
    int results_2a;
    int ir_format;
    int interp_method;
    char *target_string, *target_string_2;
} ArgDccIr;

typedef struct {
    const char* testName;
    int results_2a;
    int ir_format;
    int interp_method;
    int enable_safety_mechanism;
    char *target_string, *target_string_2;
} ArgDccIr_safety;

static uint32_t viss_checksums_ref_ir[9] = {
    (uint32_t) 0x8ba222d0, (uint32_t) 0x16a8c8d7, (uint32_t) 0x3b8cc80d,
    (uint32_t) 0x8ba222d0, (uint32_t) 0x16a8c8d7, (uint32_t) 0x3b8cc80d,
    (uint32_t) 0x1974762c, (uint32_t) 0xc86012bb, (uint32_t) 0x86350610
};

static uint32_t viss_checksums_ref_ir_color_diff[9] = {
    (uint32_t) 0xd037c3d9, (uint32_t) 0x11f3dae4, (uint32_t) 0x7864d40c,
    (uint32_t) 0xd037c3d9, (uint32_t) 0x11f3dae4, (uint32_t) 0x7864d40c,
    (uint32_t) 0x727b3243, (uint32_t) 0xd87c5682, (uint32_t) 0x650548be
};
static uint32_t viss_checksums_h3a_ref_ir = (uint32_t) 0xa18d1b8e;

#define ADD_IR_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ir_format=8b", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/ir_format=12b", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/ir_format=16b", __VA_ARGS__, 2))

#define ADD_INTERP_METHOD(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/interp_method=const_hue", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/interp_method=color_diff", __VA_ARGS__, 1))

#define PARAMETERS_DCC_IR \
    CT_GENERATE_PARAMETERS("cksm_ir", ADD_2A, ADD_IR_FORMAT, ADD_INTERP_METHOD, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

#define PARAMETERS_DCC_IR_SAFETY \
    CT_GENERATE_PARAMETERS("cksm_ir", ADD_2A, ADD_IR_FORMAT, ADD_INTERP_METHOD, ADD_SAFETY_MECHANISM, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccIr, ArgDccIr, PARAMETERS_DCC_IR)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image ir_op[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                if(0 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
                else if(1 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
                else if(2 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.bypass_pcid = 0;
                params.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
                params.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;

                if(0 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
                else if(1 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR12_P12;
                else if(2 == arg_->ir_format)
                    params.fcp[0].mux_output0 = 0;

                params.fcp[0].mux_output1 = 0;

                if(2 == arg_->ir_format)
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_IR12_U16;
                else
                    params.fcp[0].mux_output2 = 0;

                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 0;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                if(0u == arg_->interp_method)
                {
                    /* Creating DCC */
                    dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                    ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                        dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    VX_CALL(vxMapUserDataObject(
                        dcc_param_viss[i],
                        0,
                        dcc_buff_size,
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        0
                    ));
                    memset(dcc_viss_buf, 0xAB, dcc_buff_size);
                    
                    dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                    ASSERT(dcc_status == 0);

                    VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                    /* Done w/ DCC */
                }
                else
                {
                    /* Creating DCC */
                    dcc_buff_size = DCC_VISS_OV2312_DCC_COLOR_DIFF_CFG_NUM_ELEM;

                    ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                        dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    VX_CALL(vxMapUserDataObject(
                        dcc_param_viss[i],
                        0,
                        dcc_buff_size,
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        0
                    ));
                    memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                    static uint8_t  dcc_viss_ov2312_color_diff[DCC_VISS_OV2312_DCC_COLOR_DIFF_CFG_NUM_ELEM] = DCC_VISS_OV2312DCC_COLOR_DIFF_CFG;

                    memcpy(dcc_viss_buf, dcc_viss_ov2312_color_diff, dcc_buff_size);

                    VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                    /* Done w/ DCC */
                }


                /* Creating H3A output */
                ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                if(NULL != h3a_aew_af[i])
                {
                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                }

                if((0 == arg_->ir_format) || (1 == arg_->ir_format))
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], ir_op[i], NULL, NULL, NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }
                else if(2 == arg_->ir_format)
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, ir_op[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%d%s", ct_get_test_file_path(), "output/viss_dcc_ir_out_", i,(int)arg_->ir_format, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_output_ir_image(file, ir_op[i], arg_->ir_format);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                if(0u == arg_->interp_method)
                {
                    checksum_expected = viss_checksums_ref_ir[(3)*(arg_->results_2a) + arg_->ir_format];
                }
                else
                {
                    checksum_expected = viss_checksums_ref_ir_color_diff[(3)*(arg_->results_2a) + arg_->ir_format];
                }
                checksum_actual = tivx_utils_simple_image_checksum(ir_op[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                if(NULL != h3a_aew_af[i])
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    #endif
                    ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_ir_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&ir_op[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(ir_op[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccIrSafety, ArgDccIr_safety, PARAMETERS_DCC_IR_SAFETY)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image ir_op[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    tivx_vpac_viss_safety_mechanism_params_t safety_mechanism_params;
    vx_user_data_object safety_mechanism_obj[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference safety_ref[TEST_NUM_NODE_INSTANCE][1] = {NULL};

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                if(0 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
                else if(1 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
                else if(2 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.bypass_pcid = 0;
                params.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
                params.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;

                if(0 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
                else if(1 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR12_P12;
                else if(2 == arg_->ir_format)
                    params.fcp[0].mux_output0 = 0;

                params.fcp[0].mux_output1 = 0;

                if(2 == arg_->ir_format)
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_IR12_U16;
                else
                    params.fcp[0].mux_output2 = 0;

                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 0;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                if(0u == arg_->interp_method)
                {
                    /* Creating DCC */
                    dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                    ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                        dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    VX_CALL(vxMapUserDataObject(
                        dcc_param_viss[i],
                        0,
                        dcc_buff_size,
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        0
                    ));
                    memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                    dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                    ASSERT(dcc_status == 0);

                    VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                    /* Done w/ DCC */
                }
                else
                {
                    /* Creating DCC */
                    dcc_buff_size = DCC_VISS_OV2312_DCC_COLOR_DIFF_CFG_NUM_ELEM;

                    ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                        dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    VX_CALL(vxMapUserDataObject(
                        dcc_param_viss[i],
                        0,
                        dcc_buff_size,
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        0
                    ));
                    memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                    static uint8_t  dcc_viss_ov2312_color_diff[DCC_VISS_OV2312_DCC_COLOR_DIFF_CFG_NUM_ELEM] = DCC_VISS_OV2312DCC_COLOR_DIFF_CFG;

                    memcpy(dcc_viss_buf, dcc_viss_ov2312_color_diff, dcc_buff_size);

                    VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                    /* Done w/ DCC */
                }


                /* Creating H3A output */
                ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                if(NULL != h3a_aew_af[i])
                {
                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                }

                if((0 == arg_->ir_format) || (1 == arg_->ir_format))
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], ir_op[i], NULL, NULL, NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }
                else if(2 == arg_->ir_format)
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, ir_op[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        for (i=0; i<TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                tivx_vpac_viss_safety_mech_prms_init(&safety_mechanism_params);

                if(0U != arg_->enable_safety_mechanism)
                {
                    switch (arg_->enable_safety_mechanism)
                    {
                        case 1:
                            /* safety_mechanism=reconfigReinitReg */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            break;
                        case 2:
                            /* safety_mechanism=statusRegValidate */
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            break;
                        case 3:
                            /* safety_mechanism=ConfigRegValidate */
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        case 4:
                            /* safety_mechanism=all */
                            safety_mechanism_params.enable_reconfig_and_reinit_Reg = 1U;
                            safety_mechanism_params.enable_status_reg_validation = 1U;
                            safety_mechanism_params.enable_readback_config_registers = 1U;
                            break;
                        default:
                        /* Invalid value */
                        break;
                    }
                    #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined (SOC_AM62A) || defined (SOC_J722S)
                    for(uint32_t i=0U; i < 8U; i++)
                    {
                        if(vpac0_buffer_allocation_flags[i] == 0U)
                        {
                            /* 256KB allocation for each handle */
                            safety_mechanism_params.config_reg_readback_addr = VISS_VPAC0_READBACK_REG_ADDR + (i * 0x40000U);
                            vpac0_buffer_allocation_flags[i] = 1U;
                            break;
                        }
                    }
                    #endif
                    #if defined(SOC_J784S4) || defined(SOC_J742S2)
                    /* For J784S4/J742S2: VPAC1 (VPAC2_xxx targets) runs on MCU4_0 */
                    if ((arg_->target_string != NULL) && (strstr(arg_->target_string, "VPAC2") != NULL))
                    {
                        for(uint32_t i=0U; i < 8U; i++)
                        {
                            if(vpac1_buffer_allocation_flags[i] == 0U)
                            {
                                /* 256KB allocation for each handle */
                                safety_mechanism_params.config_reg_readback_addr = VISS_VPAC1_READBACK_REG_ADDR + (i * 0x40000U);
                                vpac1_buffer_allocation_flags[i] = 1U;
                                break;
                            }
                        }
                    }
                    #endif

                    ASSERT_VX_OBJECT(safety_mechanism_obj[i] = vxCreateUserDataObject(context,
                        "tivx_vpac_viss_safety_mechanism_params_t", sizeof(tivx_vpac_viss_safety_mechanism_params_t),
                        &safety_mechanism_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    safety_ref[i][0] = (vx_reference)safety_mechanism_obj[i];
                    VX_CALL(tivxNodeSendCommand(node[i], 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, safety_ref[i], 1));
                }
            }
        }

        VX_CALL(vxProcessGraph(graph));

        for (i=0; i<TEST_NUM_NODE_INSTANCE; i++)
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
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%d%s", ct_get_test_file_path(), "output/viss_dcc_ir_out_", i,(int)arg_->ir_format, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_output_ir_image(file, ir_op[i], arg_->ir_format);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                if(0u == arg_->interp_method)
                {
                    checksum_expected = viss_checksums_ref_ir[(3)*(arg_->results_2a) + arg_->ir_format];
                }
                else
                {
                    checksum_expected = viss_checksums_ref_ir_color_diff[(3)*(arg_->results_2a) + arg_->ir_format];
                }
                checksum_actual = tivx_utils_simple_image_checksum(ir_op[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                if(NULL != h3a_aew_af[i])
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    #endif
                    ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_ir_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&ir_op[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(ir_op[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        for(uint8_t i = 0; i < 8U; i++)
        {
            vpac0_buffer_allocation_flags[i] = 0U;
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            vpac1_buffer_allocation_flags[i] = 0U;
            #endif
        }

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;
} ArgDccRgbIr;

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    int enable_safety_mechanism;
    char *target_string;
} ArgDccRgbIr_safety;

#define PARAMETERS_DCC_RGB_IR \
    CT_GENERATE_PARAMETERS("cksm_rgb_ir", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

#define PARAMETERS_DCC_RGB_IR_SAFETY \
    CT_GENERATE_PARAMETERS("cksm_rgb_ir", ADD_SAFETY_MECHANISM, ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccRgbIr, ArgDccRgbIr, PARAMETERS_DCC_RGB_IR)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration_rgb = NULL, configuration_ir = NULL;
    vx_user_data_object ae_awb_result_rgb = NULL, ae_awb_result_ir = NULL;
    tivx_raw_image raw = NULL;
    vx_image y8_r8_c2 = NULL;
    vx_image ir_op = NULL;
    vx_user_data_object h3a_aew_af_rgb = NULL, h3a_aew_af_ir = NULL;
    char file_rgb[MAXPATHLENGTH];
    char file_ir[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params_rgb, params_ir;
    tivx_ae_awb_params_t ae_awb_params_rgb, ae_awb_params_ir;

    vx_graph graph_rgb = 0;
    vx_graph graph_ir = 0;
    vx_node node_rgb = NULL;
    vx_node node_ir = NULL;

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width, height, j;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph_rgb = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph_ir = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ir_op = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params_rgb);
        tivx_vpac_viss_params_init(&params_ir);

        params_rgb.bypass_pcid = 0;
        params_rgb.sensor_dcc_id = sensor_dcc_id;
        params_rgb.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_rgb.fcp[0].mux_output0 = 0;
        params_rgb.fcp[0].mux_output1 = 0;
        params_rgb.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params_rgb.fcp[0].mux_output3 = 0;
        params_rgb.fcp[0].mux_output4 = 3;
        params_rgb.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
        params_rgb.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_rgb.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        params_ir.bypass_pcid = 0;
        params_ir.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
        params_ir.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
        params_ir.sensor_dcc_id = sensor_dcc_id;
        params_ir.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_ir.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
        params_ir.fcp[0].mux_output1 = 0;
        params_ir.fcp[0].mux_output2 = 0;
        params_ir.fcp[0].mux_output3 = 0;
        params_ir.fcp[0].mux_output4 = 3;
        params_ir.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params_ir.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_ir.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        ASSERT_VX_OBJECT(configuration_rgb = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_ir = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_params input structure */
        tivx_ae_awb_params_init(&ae_awb_params_rgb);
        tivx_ae_awb_params_init(&ae_awb_params_ir);

        ae_awb_params_rgb.ae_valid = 1;
        ae_awb_params_rgb.exposure_time = 16666;
        ae_awb_params_rgb.analog_gain = 1030;
        ae_awb_params_rgb.awb_valid = 0;
        ae_awb_params_rgb.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_rgb.wb_gains[j] = 525;
            ae_awb_params_rgb.wb_offsets[j] = 2;
        }

        ae_awb_params_ir.ae_valid = 0;
        ae_awb_params_ir.exposure_time = 16666;
        ae_awb_params_ir.analog_gain = 1030;
        ae_awb_params_ir.awb_valid = 1;
        ae_awb_params_ir.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_ir.wb_gains[j] = 525;
            ae_awb_params_ir.wb_offsets[j] = 2;
        }

        ASSERT_VX_OBJECT(ae_awb_result_rgb = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(ae_awb_result_ir = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */

        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af_rgb = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(h3a_aew_af_ir = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af_rgb)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));
        }

        if(NULL != h3a_aew_af_ir)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));
        }

        ASSERT_VX_OBJECT(node_rgb = tivxVpacVissNode(graph_rgb, configuration_rgb, ae_awb_result_rgb, dcc_param_viss,
                                            raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                            h3a_aew_af_rgb, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node_ir = tivxVpacVissNode(graph_ir, configuration_ir, ae_awb_result_ir, dcc_param_viss,
                                            raw, ir_op, NULL, NULL, NULL, NULL,
                                            h3a_aew_af_ir, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_rgb, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxSetNodeTarget(node_ir, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);

        VX_CALL(vxVerifyGraph(graph_rgb));
        VX_CALL(vxVerifyGraph(graph_ir));

        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));
        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));

        snprintf(file_rgb, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_rgb_out.yuv");
        snprintf(file_ir, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_ir_out.yuv");
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_viss_output_image(file_rgb, y8_r8_c2);
        write_output_ir_image(file_ir, ir_op, 0);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_expected = viss_checksums_luma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        checksum_expected = viss_checksums_ref_ir[6];
        checksum_actual = tivx_utils_simple_image_checksum(ir_op, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        rect.end_x = width/2;
        rect.end_y = height/2;
        checksum_expected = viss_checksums_chroma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        #ifndef SOC_J722S

        if(NULL != h3a_aew_af_rgb)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_rgb, VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_rgb, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref[5] == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_rgb, "output/viss_dcc_h3a_rgbir_rgb_out.bin");
            #endif
        }

        if(NULL != h3a_aew_af_ir)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_ir, VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_ir, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_ir, "output/viss_dcc_h3a_rgbir_ir_out.bin");
            #endif
        }
        #endif
        VX_CALL(vxReleaseNode(&node_rgb));
        VX_CALL(vxReleaseNode(&node_ir));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&ir_op));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration_rgb));
        VX_CALL(vxReleaseUserDataObject(&configuration_ir));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_rgb));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_ir));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_rgb));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_ir));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        VX_CALL(vxReleaseGraph(&graph_rgb));
        VX_CALL(vxReleaseGraph(&graph_ir));

        ASSERT(graph_rgb == 0);
        ASSERT(graph_ir == 0);

        ASSERT(node_rgb == 0);
        ASSERT(node_ir == 0);
        ASSERT(h3a_aew_af_rgb == 0);
        ASSERT(h3a_aew_af_ir == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(ir_op == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result_rgb == 0);
        ASSERT(ae_awb_result_ir == 0);
        ASSERT(configuration_rgb == 0);
        ASSERT(configuration_ir == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccRgbIrSafety, ArgDccRgbIr_safety, PARAMETERS_DCC_RGB_IR_SAFETY)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration_rgb = NULL, configuration_ir = NULL;
    vx_user_data_object ae_awb_result_rgb = NULL, ae_awb_result_ir = NULL;
    tivx_raw_image raw = NULL;
    vx_image y8_r8_c2 = NULL;
    vx_image ir_op = NULL;
    vx_user_data_object h3a_aew_af_rgb = NULL, h3a_aew_af_ir = NULL;
    char file_rgb[MAXPATHLENGTH];
    char file_ir[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params_rgb, params_ir;
    tivx_ae_awb_params_t ae_awb_params_rgb, ae_awb_params_ir;

    vx_graph graph_rgb = 0;
    vx_graph graph_ir = 0;
    vx_node node_rgb = NULL;
    vx_node node_ir = NULL;

    tivx_raw_image_create_params_t raw_params;

    tivx_vpac_viss_safety_mechanism_params_t safety_mechanism_params_rgb, safety_mechanism_params_ir;
    vx_user_data_object safety_mechanism_obj_rgb = NULL, safety_mechanism_obj_ir = NULL;
    vx_reference safety_ref_rgb = NULL, safety_ref_ir = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width, height, j;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph_rgb = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph_ir = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ir_op = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params_rgb);
        tivx_vpac_viss_params_init(&params_ir);

        params_rgb.bypass_pcid = 0;
        params_rgb.sensor_dcc_id = sensor_dcc_id;
        params_rgb.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_rgb.fcp[0].mux_output0 = 0;
        params_rgb.fcp[0].mux_output1 = 0;
        params_rgb.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params_rgb.fcp[0].mux_output3 = 0;
        params_rgb.fcp[0].mux_output4 = 3;
        params_rgb.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
        params_rgb.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_rgb.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        params_ir.bypass_pcid = 0;
        params_ir.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
        params_ir.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
        params_ir.sensor_dcc_id = sensor_dcc_id;
        params_ir.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_ir.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
        params_ir.fcp[0].mux_output1 = 0;
        params_ir.fcp[0].mux_output2 = 0;
        params_ir.fcp[0].mux_output3 = 0;
        params_ir.fcp[0].mux_output4 = 3;
        params_ir.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params_ir.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_ir.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        ASSERT_VX_OBJECT(configuration_rgb = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_ir = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_params input structure */
        tivx_ae_awb_params_init(&ae_awb_params_rgb);
        tivx_ae_awb_params_init(&ae_awb_params_ir);

        ae_awb_params_rgb.ae_valid = 1;
        ae_awb_params_rgb.exposure_time = 16666;
        ae_awb_params_rgb.analog_gain = 1030;
        ae_awb_params_rgb.awb_valid = 0;
        ae_awb_params_rgb.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_rgb.wb_gains[j] = 525;
            ae_awb_params_rgb.wb_offsets[j] = 2;
        }

        ae_awb_params_ir.ae_valid = 0;
        ae_awb_params_ir.exposure_time = 16666;
        ae_awb_params_ir.analog_gain = 1030;
        ae_awb_params_ir.awb_valid = 1;
        ae_awb_params_ir.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_ir.wb_gains[j] = 525;
            ae_awb_params_ir.wb_offsets[j] = 2;
        }

        ASSERT_VX_OBJECT(ae_awb_result_rgb = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(ae_awb_result_ir = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */

        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af_rgb = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(h3a_aew_af_ir = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af_rgb)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));
        }

        if(NULL != h3a_aew_af_ir)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));
        }

        ASSERT_VX_OBJECT(node_rgb = tivxVpacVissNode(graph_rgb, configuration_rgb, ae_awb_result_rgb, dcc_param_viss,
                                            raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                            h3a_aew_af_rgb, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node_ir = tivxVpacVissNode(graph_ir, configuration_ir, ae_awb_result_ir, dcc_param_viss,
                                            raw, ir_op, NULL, NULL, NULL, NULL,
                                            h3a_aew_af_ir, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_rgb, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxSetNodeTarget(node_ir, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);

        VX_CALL(vxVerifyGraph(graph_rgb));
        VX_CALL(vxVerifyGraph(graph_ir));

        tivx_vpac_viss_safety_mech_prms_init(&safety_mechanism_params_rgb);
        tivx_vpac_viss_safety_mech_prms_init(&safety_mechanism_params_ir);

        if(0U != arg_->enable_safety_mechanism)
        {
            switch (arg_->enable_safety_mechanism)
            {
                case 1:
                    /* safety_mechanism=reconfigReinitReg */
                    safety_mechanism_params_rgb.enable_reconfig_and_reinit_Reg = 1U;
                    safety_mechanism_params_ir.enable_reconfig_and_reinit_Reg = 1U;
                    break;
                case 2:
                    /* safety_mechanism=statusRegValidate */
                    safety_mechanism_params_rgb.enable_status_reg_validation = 1U;
                    safety_mechanism_params_ir.enable_status_reg_validation = 1U;
                    break;
                case 3:
                    /* safety_mechanism=ConfigRegValidate */
                    safety_mechanism_params_rgb.enable_readback_config_registers = 1U;
                    safety_mechanism_params_ir.enable_readback_config_registers = 1U;
                    break;
                case 4:
                    /* safety_mechanism=all */
                    safety_mechanism_params_rgb.enable_reconfig_and_reinit_Reg = 1U;
                    safety_mechanism_params_rgb.enable_status_reg_validation = 1U;
                    safety_mechanism_params_rgb.enable_readback_config_registers = 1U;
                    safety_mechanism_params_ir.enable_reconfig_and_reinit_Reg = 1U;
                    safety_mechanism_params_ir.enable_status_reg_validation = 1U;
                    safety_mechanism_params_ir.enable_readback_config_registers = 1U;
                    break;
                default:
                /* Invalid value */
                break;
            }
            #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined (SOC_AM62A) || defined (SOC_J722S)
            for(uint32_t i=0U; i < 8U; i++)
            {
                if(vpac0_buffer_allocation_flags[i] == 0U)
                {
                    /* 256KB allocation for RGB node */
                    safety_mechanism_params_rgb.config_reg_readback_addr = VISS_VPAC0_READBACK_REG_ADDR + (i * 0x40000U);
                    vpac0_buffer_allocation_flags[i] = 1U;
                    break;
                }
            }
            for(uint32_t i=0U; i < 8U; i++)
            {
                if(vpac0_buffer_allocation_flags[i] == 0U)
                {
                    /* 256KB allocation for IR node */
                    safety_mechanism_params_ir.config_reg_readback_addr = VISS_VPAC0_READBACK_REG_ADDR + (i * 0x40000U);
                    vpac0_buffer_allocation_flags[i] = 1U;
                    break;
                }
            }
            #endif

            ASSERT_VX_OBJECT(safety_mechanism_obj_rgb = vxCreateUserDataObject(context,
                "tivx_vpac_viss_safety_mechanism_params_t", sizeof(tivx_vpac_viss_safety_mechanism_params_t),
                &safety_mechanism_params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            safety_ref_rgb = (vx_reference)safety_mechanism_obj_rgb;
            VX_CALL(tivxNodeSendCommand(node_rgb, 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, &safety_ref_rgb, 1));

            ASSERT_VX_OBJECT(safety_mechanism_obj_ir = vxCreateUserDataObject(context,
                "tivx_vpac_viss_safety_mechanism_params_t", sizeof(tivx_vpac_viss_safety_mechanism_params_t),
                &safety_mechanism_params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            safety_ref_ir = (vx_reference)safety_mechanism_obj_ir;
            VX_CALL(tivxNodeSendCommand(node_ir, 0, TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM, &safety_ref_ir, 1));
        }

        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));
        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));

        if(0U != arg_->enable_safety_mechanism)
        {
            VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj_rgb));
            VX_CALL(vxReleaseUserDataObject(&safety_mechanism_obj_ir));
        }

        snprintf(file_rgb, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_rgb_out.yuv");
        snprintf(file_ir, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_ir_out.yuv");
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_viss_output_image(file_rgb, y8_r8_c2);
        write_output_ir_image(file_ir, ir_op, 0);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_expected = viss_checksums_luma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        checksum_expected = viss_checksums_ref_ir[6];
        checksum_actual = tivx_utils_simple_image_checksum(ir_op, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        rect.end_x = width/2;
        rect.end_y = height/2;
        checksum_expected = viss_checksums_chroma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        #ifndef SOC_J722S

        if(NULL != h3a_aew_af_rgb)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_rgb, VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_rgb, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref[5] == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_rgb, "output/viss_dcc_h3a_rgbir_rgb_out.bin");
            #endif
        }

        if(NULL != h3a_aew_af_ir)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_ir, VX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_ir, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_ir, "output/viss_dcc_h3a_rgbir_ir_out.bin");
            #endif
        }
        #endif
        VX_CALL(vxReleaseNode(&node_rgb));
        VX_CALL(vxReleaseNode(&node_ir));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&ir_op));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration_rgb));
        VX_CALL(vxReleaseUserDataObject(&configuration_ir));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_rgb));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_ir));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_rgb));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_ir));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        VX_CALL(vxReleaseGraph(&graph_rgb));
        VX_CALL(vxReleaseGraph(&graph_ir));

        ASSERT(graph_rgb == 0);
        ASSERT(graph_ir == 0);

        ASSERT(node_rgb == 0);
        ASSERT(node_ir == 0);
        ASSERT(h3a_aew_af_rgb == 0);
        ASSERT(h3a_aew_af_ir == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(ir_op == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result_rgb == 0);
        ASSERT(ae_awb_result_ir == 0);
        ASSERT(configuration_rgb == 0);
        ASSERT(configuration_ir == 0);
        ASSERT(dcc_param_viss == 0);

        for(uint8_t i = 0; i < 8U; i++)
        {
            vpac0_buffer_allocation_flags[i] = 0U;
            #if defined(SOC_J784S4) || defined(SOC_J742S2)
            vpac1_buffer_allocation_flags[i] = 0U;
            #endif
        }

        tivxHwaUnLoadKernels(context);
    }
}
#endif

TEST_WITH_ARG(tivxHwaVpacViss, testErrorInterrupts, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj;
    vx_enum scalar_out_err_stat;
    vx_reference err_ref = 0;
    vx_reference param_ref = 0;

    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        /* Disable all events by default */
        params.enable_error_events = TIVX_VPAC_VISS_WDTIMER_ERR;
        /* Configure WDTimer with 128K cycle timeout */
        params.wdtimer_timeout_cycles = TIVX_VPAC_VISS_WDTIMER_128K_CYCLES;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        /* Enable watchdog timer error events */
        params.enable_error_events = TIVX_VPAC_VISS_WDTIMER_ERR;
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        vx_reference param_ref = (vx_reference)configuration;
        tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u);

        /* Run first frame with watchdog timer error interrupts enabled */
        VX_CALL(vxProcessGraph(graph));

        /* Get error interrupt status values */
        ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
        VX_TYPE_UINT32 , &scalar_out_err_stat),
        (enum vx_type_e)VX_TYPE_SCALAR);

        err_ref = (vx_reference)scalar_err_stat_obj;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_ERR_STATUS, &err_ref, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("VISS error status value is %d\n", scalar_out_err_stat);
        #endif
        /* Checks whether error interrupt was detected.
        *  No error is detected during normal operation. */
        ASSERT(scalar_out_err_stat == 0);

        /* Call the control command for maximum coverage */
        params.enable_error_events = 0;
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        param_ref = (vx_reference)configuration;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u));

        /* Run second frame with all error interrupts enabled, including the watchdog timer timeout errors
        *  which are disabled by default. */
        params.enable_error_events = TIVX_VPAC_VISS_ALL_ERR_EVENTS;
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        param_ref = (vx_reference)configuration;
        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_ENABLE_ERROR_EVENTS, &param_ref, 1u));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_ERR_STATUS, &err_ref, 1u));
        VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat,
                                VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        #if defined TEST_GET_ERROR_INTERRUPT_VALUES
        printf("VISS error status value is %d\n", scalar_out_err_stat);
        #endif
        /* Checks whether error interrupt was detected.
        *  No error is detected during normal operation. */
        ASSERT(scalar_out_err_stat == 0);
        VX_CALL(vxReleaseScalar(&scalar_err_stat_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static uint32_t expected_psa_values[4] = {
0x854c621f, 
0x9bc8c371, 
0x73d234ae,
0x29e05356
};
TEST_WITH_ARG(tivxHwaVpacViss, testPsaSignValue, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[6] = {0};
    vx_user_data_object psa_obj;
    int i;
    tivx_vpac_viss_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)raw, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);*/
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);*/

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        params.enable_psa = 1u;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_viss_output_image(file, y8_r8_c2);

        ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_viss_psa_timestamp_data_t", sizeof(tivx_vpac_viss_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);
        
        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
            
        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_viss_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* Compare PSA signature values */
        for (i = 0; i < 4; i++) 
        {
            ASSERT(psa_status.psa_values[i] == expected_psa_values[i]);
        }
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));
        /*VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));*/
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        /*VX_CALL(vxReleaseImage(&uv12_c1));*/
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#ifndef VPAC3L
/*
 * Test name: testSl2Firewall
 *
 * Purpose: Test VISS with SL2 firewall protection enabled
 *
 * Test Details: This test is identical to testPsaSignValue but with SL2 firewall
 *               protection enabled. Since firewall is transparent to output data,
 *               it reuses the same PSA signature values for validation.
 *
 * Pass Criteria:
 *    - PSA signature values match expected values
 *    - No error events occur
 *    - Firewall protection is successfully configured and doesn't affect output
 */
TEST_WITH_ARG(tivxHwaVpacViss, testSl2Firewall, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[6] = {0};
    vx_user_data_object psa_obj;
    int i;
    tivx_vpac_viss_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;
    vx_status status;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;
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

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)raw, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);*/
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);*/

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        params.enable_psa = 1u;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_viss_output_image(file, y8_r8_c2);

        ASSERT_VX_OBJECT(psa_obj =
        vxCreateUserDataObject(context, "tivx_vpac_viss_psa_timestamp_data_t", sizeof(tivx_vpac_viss_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);

        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_PSA_STATUS,
            psa_refs, 1u));

        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_viss_psa_timestamp_data_t),
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        /* Compare PSA signature values */
        for (i = 0; i < 4; i++)
        {
            ASSERT(psa_status.psa_values[i] == expected_psa_values[i]);
        }
        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));
        /*VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));*/
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        /*VX_CALL(vxReleaseImage(&uv12_c1));*/
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

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


typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;
    int fcp_selected;
} Arg_rgb;

#ifdef VPAC3
#define SELECT_FCP_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_selected=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_selected=1", __VA_ARGS__, 1))
#else 
#define SELECT_FCP_MODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_selected=0", __VA_ARGS__, 0))
#endif

#define SET_NODE_TARGET_RGB_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, SELECT_FCP_MODE, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessing_rgb, Arg_rgb, SET_NODE_TARGET_RGB_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    uint32_t checksum_actual = 0, checksum_expected = 0;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);

        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(arg_->fcp_selected == 0)
        {
            params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
            params.fcp[0].mux_output0 = 0;
            params.fcp[0].mux_output1 = 0;
            params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_RED;
            params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_GREEN;
            params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_BLUE;
            params.fcp[0].chroma_mode = 0;
        }
        else
        {
            #ifdef VPAC3
            params.fcp[1].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
            params.fcp[1].mux_output0 = 0;
            params.fcp[1].mux_output1 = 0;
            params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_RED;
            params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX3_GREEN;
            params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_BLUE;
            params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT0);
            params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT1);
            params.output_fcp_mapping[2] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT2);
            params.output_fcp_mapping[3] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT3);
            params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP1,TIVX_VPAC_VISS_FCP_OUT4);
            params.fcp[1].chroma_mode = 0;
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_RFE; /* RAWFE --> FCP1 */
            #endif
        }

        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested
        params.h3a_aewb_af_mode = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, NULL, NULL, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("y8_r8_c2 checksum values:0x%08x\n", checksum_actual);
        save_image_from_viss(y8_r8_c2, "output/out_r8");
        #endif
        ASSERT((uint32_t)0x1fd04a62 == checksum_actual);
        
        checksum_actual = tivx_utils_simple_image_checksum(uv8_g8_c3, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("uv8_g8_c3 checksum values:0x%08x\n", checksum_actual);
        save_image_from_viss(uv8_g8_c3, "output/out_g8");
        #endif
        ASSERT((uint32_t)0x252e5711 == checksum_actual);

        
        checksum_actual = tivx_utils_simple_image_checksum(s8_b8_c4, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("s8_b8_c4 checksum values:0x%08x\n", checksum_actual);
        save_image_from_viss(s8_b8_c4, "output/out_b8");
        #endif
        ASSERT((uint32_t)0x0e0b43bf == checksum_actual);


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
/* Test case for check the Input Format  */
static uint32_t msb_checksum             = (uint32_t) 0x621c3774;
typedef struct {
    const char* testName;
    int msb;
    char* target_string;
} ArgMsb;

#define ADD_MSB(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/msb=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/msb=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/msb=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/msb=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/msb=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/msb=5", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/msb=6", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/msb=8", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/msb=9", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/msb=10", __VA_ARGS__, 10)), \
    CT_EXPAND(nextmacro(testArgName "/msb=11", __VA_ARGS__, 11)), \
    CT_EXPAND(nextmacro(testArgName "/msb=12", __VA_ARGS__, 12)), \
    CT_EXPAND(nextmacro(testArgName "/msb=13", __VA_ARGS__, 13)), \
    CT_EXPAND(nextmacro(testArgName "/msb=14", __VA_ARGS__, 15))

#define PARAMETERS_MSB \
    CT_GENERATE_PARAMETERS("testMsb", ADD_MSB, ADD_SET_TARGET_PARAMETERS, ARG)
TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testMsb, ArgMsb,
    PARAMETERS_MSB
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = arg_->msb;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = sensor_dcc_id;                                                    
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */


                   /* Creating H3A output */
                   ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                       (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                   if(NULL != h3a_aew_af)
                   {
                       VX_CALL(vxMapUserDataObject(h3a_aew_af,
                           0,
                           sizeof(tivx_h3a_data_t),
                           &dcc_viss_buf_map_id,
                           (void **)&dcc_viss_buf,
                           (vx_enum)VX_WRITE_ONLY,
                           (vx_enum)VX_MEMORY_TYPE_HOST,
                           0
                           ));

                       memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                       VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
                   }
                   

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

                
               /* for checksum */     
               snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_msb_out_", arg_->msb ,".yuv");
               #if defined(TEST_VISS_CHECKSUM_LOGGING)
               write_viss_output_image(file, y8_r8_c2);
               #endif

               rect.start_x = 0;
               rect.start_y = 0;
               rect.end_x = width;
               rect.end_y = height;

               checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
               #if defined(TEST_VISS_CHECKSUM_LOGGING)
               printf(" checksum_actual=  0x%08x\n", checksum_actual);
               #endif
               ASSERT( msb_checksum == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case to check pixel format */
static uint32_t pixel_checksum[] = {(uint32_t) 0x624a92cd , (uint32_t) 0x621c3774};       
typedef struct {
    const char* testName;
    int pixel;
    char* target_string;
} ArgPixel;

#define ADD_PIXEL(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/pixel=TIVX_RAW_IMAGE_16_BIT", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/pixel=TIVX_RAW_IMAGE_P12_BIT", __VA_ARGS__, 2)),

#define PARAMETERS_PIXEL \
    CT_GENERATE_PARAMETERS("testPixel", ADD_PIXEL, ADD_SET_TARGET_PARAMETERS, ARG)


TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testPixel, ArgPixel,
    PARAMETERS_PIXEL
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].msb = 11;
    raw_params.format[1].msb = 7;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    switch (arg_->pixel)
    {
    case 1:
        raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
        raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_8_BIT;
        break;
    case 2:
        raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
        raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
        raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        break;
    default:
        break;
    }

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.sensor_dcc_id = sensor_dcc_id;

        params.output_fcp_mapping[4u] = 1;  // added for 1559
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
         dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

         ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
             dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

         VX_CALL(vxMapUserDataObject(
             dcc_param_viss,
             0,
             dcc_buff_size,
             &dcc_viss_buf_map_id,
             (void **)&dcc_viss_buf,
             VX_WRITE_ONLY,
             VX_MEMORY_TYPE_HOST,
             0
         ));
         memset(dcc_viss_buf, 0xAB, dcc_buff_size);

         dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
         ASSERT(dcc_status == 0);

         VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
         /* Done w/ DCC */


                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if(NULL != h3a_aew_af)
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af,
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
                    }
                    

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        if (arg_->pixel == 1)
        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        /* for checksum */     
        snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_pixel_out_",arg_->pixel, ".yuv");
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_viss_output_image(file, y8_r8_c2);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        if(arg_->pixel == 1)
        {
            checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("y8_r8_c2 checksum values:0x%08x\n", checksum_actual);
            #endif
            ASSERT(pixel_checksum[1] == checksum_actual);
        }
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for checking mux format */


typedef struct {
    const char* testName;
    int fcp_map0;
    int fcp_map1;
    int fcp_map2;
    int fcp_map3;
    int fcp_map4;
    char* target_string;
} ArgMuxFormat;

#define ADD_FCP_MAP0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=3", __VA_ARGS__, 3))
    
#define ADD_FCP_MAP1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=3", __VA_ARGS__, 3))

#define ADD_FCP_MAP2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=3", __VA_ARGS__, 3))

#define ADD_FCP_MAP3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=3", __VA_ARGS__, 3))

#define ADD_FCP_MAP4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/fcp_map0=1", __VA_ARGS__, 1))
    
#define PARAMETERS_MUX_FORMAT \
    CT_GENERATE_PARAMETERS("testMuxFormat", ADD_FCP_MAP0, ADD_FCP_MAP1, ADD_FCP_MAP2, ADD_FCP_MAP3, ADD_FCP_MAP4, ADD_SET_TARGET_PARAMETERS, ARG)

typedef struct {
    const char* testName;
    int output_fmt;
    char *target_string, *target_string_2;
} ArgOutput;


#define PARAMETERS_OUT \
    CT_GENERATE_PARAMETERS("out", ADD_OP_FMT, ADD_SET_TARGET_PARAMETERS, ARG)
    
TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testMuxFormat, ArgMuxFormat,PARAMETERS_MUX_FORMAT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object dcc_param_viss = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    char file[MAXPATHLENGTH];
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 0;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;
    
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    switch (arg_->fcp_map0)
    {
    case 0:
        params.fcp[0].mux_output0 = 0;
        break;
    #if defined (VPAC3)
    case 1:
        params.fcp[1].mux_output0 = 0;
        break;
    #endif
    case 2:
        params.fcp[0].mux_output2 = 0;
        break;
    #if defined (VPAC3)
    case 3:
        params.fcp[1].mux_output2 = 0;
        break;
    #endif
    default:
        break;
    }
    
    switch (arg_->fcp_map1)
    {
    case 0:
        params.fcp[0].mux_output1 = 0;
        break;
    #if defined (VPAC3)
    case 1:
        params.fcp[1].mux_output1 = 0;
        break;
    #endif
    case 2:
        params.fcp[0].mux_output3 = 0;
        break;
    #if defined (VPAC3)
    case 3:
        params.fcp[1].mux_output3 = 0;
        break;
    #endif
    default:
        break;
    }
    
    switch (arg_->fcp_map2)
    {
    case 0:
        params.fcp[0].mux_output0 = 0;
        break;
    #if defined (VPAC3)
    case 1:
        params.fcp[1].mux_output0 = 0;
        break;
    #endif
    case 2:
        params.fcp[0].mux_output2 = 0;
        break;
    #if defined (VPAC3)
    case 3:
        params.fcp[1].mux_output2 = 0;
        break;
    #endif
    default:
        break;
    }

    switch (arg_->fcp_map3)
    {
    case 0:
        params.fcp[0].mux_output1 = 0;
        break;
    #if defined (VPAC3)
    case 1:
        params.fcp[1].mux_output1 = 0;
        break;
    #endif
    case 2:
        params.fcp[0].mux_output3 = 0;
        break;
    #if defined (VPAC3)
    case 3:
        params.fcp[1].mux_output3 = 0;
        break;
    #endif
    default:
        break;
    }
    
    switch (arg_->fcp_map4)
    {
    case 0:
        params.fcp[0].mux_output4 = 0;
        break;
    #if defined (VPAC3)
    case 1:
        params.fcp[1].mux_output4 = 0;
        break;
    #endif
    default:
        break;
    }   
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.sensor_dcc_id = sensor_dcc_id;
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

         /* Creating DCC */
         dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

         ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
             dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

         VX_CALL(vxMapUserDataObject(
             dcc_param_viss,
             0,
             dcc_buff_size,
             &dcc_viss_buf_map_id,
             (void **)&dcc_viss_buf,
             VX_WRITE_ONLY,
             VX_MEMORY_TYPE_HOST,
             0
         ));
         memset(dcc_viss_buf, 0xAB, dcc_buff_size);

         dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
         ASSERT(dcc_status == 0);

         VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
         /* Done w/ DCC */


         /* Creating H3A output */
         ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

             if(NULL != h3a_aew_af)
            {
                     VX_CALL(vxMapUserDataObject(h3a_aew_af,
                        0,
                        sizeof(tivx_h3a_data_t),
                         &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
                    }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s%s", ct_get_test_file_path(), "output/viss_mux_out_", ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_viss_output_image(file, y8_r8_c2);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf(" checksum_actual=  0x%08x\n", checksum_actual);
                #endif
                ASSERT(msb_checksum == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(dcc_param_viss == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case to create tivx_h3a_data_t data object */

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testH3A_AEW, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;
    tivx_h3a_aew_config h3a_aew;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_h3a_data_init(&h3a_data);
        tivx_h3a_aew_config_init(&h3a_aew);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);  
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));     
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* tivxAddKernelVpacVissValidate function histogram0_numBins = 255 negative case rawhist 127 */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testHist, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram1 = NULL;
    vx_distribution raw_hist = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    
    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        
#if defined (VPAC3)
        params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
#endif
        params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
        params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
        
        params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_cac = 1;
        
        params.fcp1_config = TIVX_VPAC_VISS_FCP1_DISABLED;	// MV_Input is set to 0 for covering negative case
        
#if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[1].chroma_mode = 0;
#endif        

        params.enable_ctx = 1;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 255, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(histogram1 = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(raw_hist = vxCreateDistribution(context, 127, 0, 128), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, histogram1, raw_hist), VX_TYPE_NODE);
	    ASSERT_NO_FAILURE(vxVerifyGraph(graph));
	
	    VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseDistribution(&raw_hist));
        VX_CALL(vxReleaseDistribution(&histogram1));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(raw_hist == 0);
        ASSERT(histogram1 == 0);
        ASSERT(histogram == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case to check node_cmd_id */
typedef struct {
    const char* testName;
    int node_cmd_id;
    char* target_string;
} Argnode_cmd_id;

#define ADD_NODE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/node_cmd_id=TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/node_cmd_id=TIVX_VPAC_VISS_CMD_GET_ERR_STATUS", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/node_cmd_id=0", __VA_ARGS__, 2))

    #define PARAMETERS_NODE_ID \
    CT_GENERATE_PARAMETERS("testnode_cmd_id", ADD_NODE, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testnode_cmd_id, Argnode_cmd_id,
    PARAMETERS_NODE_ID
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[6] = {0};
    vx_user_data_object psa_obj;
    int i;
    tivx_vpac_viss_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)raw, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        params.enable_psa = 1u;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);
        #endif
        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_viss_output_image(file, y8_r8_c2);
        #endif

        ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_viss_psa_timestamp_data_t", sizeof(tivx_vpac_viss_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);
        
        psa_refs[0] = (vx_reference) psa_obj;
        switch (arg_->node_cmd_id)
        {
        case 0:
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS,
            psa_refs, 1u));
            break;
        case 1:
            break;
        case 2:
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, 0,
            psa_refs, 1u));
            break;
        default:
            break;
        }
        
            
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_viss_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testDccTarget, ArgOutput, PARAMETERS_OUT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image y8_r8_c2[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_distribution raw_hist[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
                
                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
                params.fcp[0].mux_output0 = 0;
                params.fcp[0].mux_output1 = 0;

                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                    
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;

                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
                params.bypass_glbce = 1;
                params.bypass_nsf4 = 1;

                    params.bypass_cac = 1;

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        /* Creating DCC */
                        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                        ASSERT(dcc_status == 0);

                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    
                    
                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if(NULL != h3a_aew_af[i])
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                    }

                
                #if defined (VPAC3) || defined (VPAC3L)
                /* Creating raw hist output */
                int nbins = 128U;
                int offset = 0U;
                int range = 128U;
                ASSERT_VX_OBJECT(raw_hist[i] = vxCreateDistribution(context, nbins, offset, range),
                    (enum vx_type_e)VX_TYPE_DISTRIBUTION);
                #endif

                ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, y8_r8_c2[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, raw_hist[i]), VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_dcc_out_", i, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_viss_output_image(file, y8_r8_c2[i]);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf(" checksum_actual=  0x%08x\n", checksum_actual);
                #endif
                ASSERT(0x2e9af183 == checksum_actual);
                
            }
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&y8_r8_c2[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));

                    VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                    VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
  
                #if defined (VPAC3) || defined (VPAC3L)
                VX_CALL(vxReleaseDistribution(&raw_hist[i]));
                #endif
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(y8_r8_c2[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
                #if defined (VPAC3) || defined (VPAC3L)
                ASSERT(&raw_hist[i]);
                #endif
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacViss, tesVpacVissGetErrStatus, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_scalar scalar_obj;
    vx_enum scalar_out;
    vx_reference refs[5] = {0};
    vx_scalar scalar_err_stat_obj, scalar_wdtimer_err_stat_obj;
    vx_enum scalar_out_err_stat, scalar_out_wdtimer_err_stat;
    vx_reference err_refs[5] = {0};
    vx_reference wd_err_refs[5] = {0};
    vx_event_t event = { 0 };

    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        /* Disable all events by default */
        params.enable_error_events = 0;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        /* enable events generation */
        vxEnableEvents(context);
        /* Register an error event for the node */
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxRegisterEvent((vx_reference)node, VX_EVENT_NODE_ERROR, 0, VISS_NODE_ERROR_EVENT));

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        
        VX_CALL(vxProcessGraph(graph));

        /* Note: since we are not blocking and there are no events, this returns VX_FAILURE per spec
         * and prints an error message saying that there are no events in the queue */
        vxWaitEvent(context, &event, vx_true_e);
        err_refs[0] = (vx_reference)scalar_err_stat_obj;

            /* Get error interrupt status values */
            ASSERT_VX_OBJECT(scalar_err_stat_obj = vxCreateScalar(context,
            VX_TYPE_UINT32 , &scalar_out_err_stat),
            (enum vx_type_e)VX_TYPE_SCALAR);
        
            err_refs[0] = (vx_reference)scalar_err_stat_obj;
            ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_ERR_STATUS,
                err_refs, 1u));
            VX_CALL(vxCopyScalar(scalar_err_stat_obj, &scalar_out_err_stat, VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST));
            #if defined TEST_GET_ERROR_INTERRUPT_VALUES
            printf("VISS error status value is %d\n", scalar_out_err_stat);
            #endif
            ASSERT(scalar_out_err_stat == 0);
            VX_CALL(vxReleaseScalar(&scalar_err_stat_obj)); 

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxDisableEvents(context));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case to cover negative cases for output image height/width and raw image size mismatch - PARAMETER RELATIONSHIP CHECKING */

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeDimension, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeParamDimensionCheck, ArgOutput, PARAMETERS_OUT)
{
   vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        if(arg_->output_fmt == 0)
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        }
        else if(arg_->output_fmt == 1)
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        

        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);



        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* test case for Parameter output_fcp_mapping  should be in range [0-3] */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeParams, SetTarget_Arg,SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;
    tivx_h3a_aew_config h3a_aew;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;


    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 0;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        params.output_fcp_mapping[0u] = 4; 
        params.output_fcp_mapping[1u] = 4; 
        params.output_fcp_mapping[2u] = 4; 
        params.output_fcp_mapping[3u] = 4; 
        params.output_fcp_mapping[4u] = 2; 
        params.fcp1_config = 5;
    #if defined (VPAC3L)
        params.enable_ir_op = 2;
        params.enable_bayer_op = 2;
        params.bypass_pcid = 2;
    #endif
        params.bypass_cac = 2;
        params.bypass_dwb = 2;
        params.bypass_glbce = 2;
        params.bypass_nsf4 = 2;
    #if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
    #endif
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                NULL, histogram, NULL, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));    
         
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


/* test case for hitting output0_fmt output1_fmt output2_fmt output0_fmt*/
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeOutput_fm, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
    
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativedataobject, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 12;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        
        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE,vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE,vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT(NULL == ( tivxVpacVissNode(graph, NULL, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                NULL, NULL, NULL, NULL) ));

        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeConfig, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
    
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeawbParams, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeaewafParams, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


/* Test case for Output Format negtive case check */

#define ADD_OUTPUT_FMT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=TIVX_DF_IMAGE_NV12_P12", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=VX_DF_IMAGE_NV12", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/op_fmt=VX_DF_IMAGE_YUYV", __VA_ARGS__, 2))

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeOutput, ArgOutput, PARAMETERS_OUT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;
    

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        if(arg_->output_fmt == 0)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        }
        else if(arg_->output_fmt == 1)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;
        params.output_fcp_mapping[0] = 0;
        params.output_fcp_mapping[2] = 0;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
    
}


/* test case for TIVX_VPAC_VISS_FCP1_DISABLED */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeMVINP, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    
    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        
#if defined (VPAC3)
        params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
#endif
        params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
        params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
        
        params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_cac = 1;
        
        params.fcp1_config = TIVX_VPAC_VISS_FCP1_DISABLED;	/* MV_Input is set to 0 for covering negative case */

#if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;        
        params.fcp[1].chroma_mode = 0;
#endif
        params.enable_ctx = 1;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);
	    ASSERT_NO_FAILURE(vxVerifyGraph(graph));
	
	    VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}



TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeOUT, ArgOutput, PARAMETERS_OUT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        if(arg_->output_fmt == 0)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        }
        else if(arg_->output_fmt == 1)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

 /* Test case for covering Default case of mux mapping  */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeMuxFormat, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 0;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    /* Default case for mux mapping */
    
    params.output_fcp_mapping[0u] = 4; 
    params.output_fcp_mapping[1u] = 4; 
    params.output_fcp_mapping[2u] = 4; 
    params.output_fcp_mapping[3u] = 4; 
    params.output_fcp_mapping[4u] = 2; 
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, NULL, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        ASSERT_NO_FAILURE(vxProcessGraph(graph));
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#if defined (VPAC3) || defined (VPAC3L)
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testnegativeOutFormat, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_image y12_ref = NULL, uv12_c1_ref = NULL, y8_r8_c2_ref = NULL, uv8_g8_c3_ref = NULL, s8_b8_c4_ref = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram_ref = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_user_data_object h3a_aew_af_ref = NULL;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual = 0;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1280;
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12 ;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 0;
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph));


        VX_CALL(vxProcessGraph(graph));


        /* For visual verification */

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif

/* tivxAddKernelVpacVissValidate function histogram0_numBins = 255 negative case rawhist 127 */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeHistCheck, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram1 = NULL;
    vx_distribution raw_hist = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    
    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        
#if defined (VPAC3)
        params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
#endif
        params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
        params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
        
        params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_cac = 1;
        
        params.fcp1_config = TIVX_VPAC_VISS_FCP1_DISABLED;	// MV_Input is set to 0 for covering negative case
        
#if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;        
        params.fcp[1].chroma_mode = 0;
#endif
        params.enable_ctx = 1;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 255, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(histogram1 = vxCreateDistribution(context, 255, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(raw_hist = vxCreateDistribution(context, 127, 0, 128), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, histogram1, raw_hist), VX_TYPE_NODE);
	    ASSERT_NO_FAILURE(vxVerifyGraph(graph));
	
	    VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseDistribution(&raw_hist));
        VX_CALL(vxReleaseDistribution(&histogram1));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(raw_hist == 0);
        ASSERT(histogram1 == 0);
        ASSERT(histogram == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for negative mux0 and mux4 check  */

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeMux, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
   vx_context context = context_->vx_context_;
   vx_user_data_object configuration = NULL;
   vx_user_data_object ae_awb_result = NULL;
   tivx_raw_image raw = NULL;
   vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
   vx_distribution histogram = NULL;
   vx_user_data_object h3a_aew_af = NULL;

   tivx_vpac_viss_params_t params;
   tivx_ae_awb_params_t ae_awb_params;
   tivx_h3a_data_t h3a_data;

   vx_graph graph = 0;
   vx_node node = 0;

   tivx_raw_image_create_params_t raw_params;
   raw_params.width = 128;
   raw_params.height = 128;
   raw_params.num_exposures = 3;
   raw_params.line_interleaved = vx_false_e;
   raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
   raw_params.format[0].msb = 11;
   raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
   raw_params.format[1].msb = 7;
   raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
   raw_params.format[2].msb = 11;
   raw_params.meta_height_before = 5;
   raw_params.meta_height_after = 0;

   ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

   {
       tivxHwaLoadKernels(context);
       CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

       ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

       tivx_vpac_viss_params_init(&params);
       ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                           sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

       tivx_ae_awb_params_init(&ae_awb_params);
       ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                           sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
       tivx_h3a_data_init(&h3a_data);
       ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                           sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

       params.fcp[0].mux_output0 = 3U;
       params.fcp[0].mux_output1 = 0U;
       params.fcp[0].mux_output2 = 0U;
       params.fcp[0].mux_output3 = 0U;
       params.fcp[0].mux_output4 = 2U;
       params.bypass_glbce = 0U;
       params.bypass_nsf4 = 0U;
       params.h3a_in = 0U;
       params.h3a_aewb_af_mode = 0U;
       params.fcp[0].ee_mode = 0U;
       params.fcp[0].chroma_mode = 0U;

#if defined (VPAC3)
       params.fcp[1].mux_output0 = 4U;
       params.fcp[1].mux_output2= 5U;
#endif
       params.output_fcp_mapping[0] = 0U; 
       params.output_fcp_mapping[1] = 0U; 
       params.output_fcp_mapping[2] = 0U; 

       VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
       VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
       VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

       ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);


       ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                               raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                               h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

       VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

       ASSERT_NO_FAILURE(vxVerifyGraph(graph));

       VX_CALL(vxReleaseNode(&node));
       VX_CALL(vxReleaseGraph(&graph));
       VX_CALL(vxReleaseDistribution(&histogram));
       VX_CALL(vxReleaseImage(&s8_b8_c4));
       VX_CALL(vxReleaseImage(&uv8_g8_c3));
       VX_CALL(vxReleaseImage(&y8_r8_c2));
       VX_CALL(vxReleaseImage(&uv12_c1));
       VX_CALL(vxReleaseImage(&y12));
       VX_CALL(tivxReleaseRawImage(&raw));
       VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
       VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
       VX_CALL(vxReleaseUserDataObject(&configuration));

       ASSERT(node == 0);
       ASSERT(graph == 0);
       ASSERT(h3a_aew_af == 0);
       ASSERT(histogram == 0);
       ASSERT(s8_b8_c4 == 0);
       ASSERT(uv8_g8_c3 == 0);
       ASSERT(y8_r8_c2 == 0);
       ASSERT(uv12_c1 == 0);
       ASSERT(y12 == 0);
       ASSERT(raw == 0);
       ASSERT(ae_awb_result == 0);
       ASSERT(configuration == 0);

       tivxHwaUnLoadKernels(context);
   }
}


/* Test case for negative h3a_in and h3a_data check*/
typedef struct
{
    const char* testName;
    int hin,hdata;
    char *target_string;

} Set_Arg_h3a;

#define ADD_hin(testArgName, nextmacro, ...) \
   CT_EXPAND(nextmacro(testArgName "/hin=1", __VA_ARGS__, 0)), \
   CT_EXPAND(nextmacro(testArgName "/hin=2", __VA_ARGS__, 1))

#define ADD_hdata(testArgName, nextmacro, ...) \
   CT_EXPAND(nextmacro(testArgName "/hdata=1", __VA_ARGS__, 0)), \
   CT_EXPAND(nextmacro(testArgName "/hdata=2", __VA_ARGS__, 1))

#define SET_H3A_PARAMETERS \
    CT_GENERATE_PARAMETERS("testNegativeParamh3a", ADD_hin, ADD_hdata, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeParamh3a, Set_Arg_h3a, SET_H3A_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 3U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;      

        if(0==arg_->hin)
        {
            params.h3a_in = 1U;
        }
        if(1==arg_->hin)
        {
            params.h3a_in = 2U;
        }

        if(0==arg_->hdata)
        {
            ae_awb_params.h3a_source_data = 1U;
        }
        if(1==arg_->hdata)
        {
            ae_awb_params.h3a_source_data = 2U;
        }
    

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for tivxAddKernelVpacVissValidate function when histogram1_numBins = 256 */

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeHist1Check, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram1 = NULL;
    vx_distribution raw_hist = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    
    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;   
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
#if defined (VPAC3)
        params.fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[1].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
#endif
        params.output_fcp_mapping[0] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT2);
        params.output_fcp_mapping[1] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT3);
        
        params.output_fcp_mapping[4] = TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0,TIVX_VPAC_VISS_FCP_OUT4);

        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_cac = 1;
        
        params.fcp1_config = TIVX_VPAC_VISS_FCP1_DISABLED;	// MV_Input is set to 0 for covering negative case
#if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[1].chroma_mode = 0;
#endif
        params.enable_ctx = 1;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(histogram1 = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(raw_hist = vxCreateDistribution(context, 127, 0, 128), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, histogram1, raw_hist), VX_TYPE_NODE);
	    ASSERT_NO_FAILURE(vxVerifyGraph(graph));
	
	    VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseDistribution(&raw_hist));
        VX_CALL(vxReleaseDistribution(&histogram1));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(raw_hist == 0);
        ASSERT(histogram1 == 0);
        ASSERT(histogram == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


/* Test case for fcp_of_0 != fcp_of_2 check  */
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativefcp_of_0_2, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
   vx_context context = context_->vx_context_;
   vx_user_data_object configuration = NULL;
   vx_user_data_object ae_awb_result = NULL;
   tivx_raw_image raw = NULL;
   vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
   vx_distribution histogram = NULL;
   vx_user_data_object h3a_aew_af = NULL;

   tivx_vpac_viss_params_t params;
   tivx_ae_awb_params_t ae_awb_params;
   tivx_h3a_data_t h3a_data;

   vx_graph graph = 0;
   vx_node node = 0;

   tivx_raw_image_create_params_t raw_params;
   raw_params.width = 128;
   raw_params.height = 128;
   raw_params.num_exposures = 3;
   raw_params.line_interleaved = vx_false_e;
   raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
   raw_params.format[0].msb = 11;
   raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
   raw_params.format[1].msb = 7;
   raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
   raw_params.format[2].msb = 11;
   raw_params.meta_height_before = 5;
   raw_params.meta_height_after = 0;

   ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

   {
       tivxHwaLoadKernels(context);
       CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

       ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

       tivx_vpac_viss_params_init(&params);
       ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                           sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

       tivx_ae_awb_params_init(&ae_awb_params);
       ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                           sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
       tivx_h3a_data_init(&h3a_data);
       ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                           sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                                                            
       params.fcp[0].mux_output0 = 3U;
       params.fcp[0].mux_output1 = 0U;
       params.fcp[0].mux_output2 = 0U;
       params.fcp[0].mux_output3 = 0U;
       params.fcp[0].mux_output4 = 2U;
       params.bypass_glbce = 0U;
       params.bypass_nsf4 = 0U;
       params.h3a_in = 0U;
       params.h3a_aewb_af_mode = 0U;
       params.fcp[0].ee_mode = 0U;
       params.fcp[0].chroma_mode = 0U;
#if defined (VPAC3)
       params.fcp[1].mux_output0 = 4U;
       params.fcp[1].mux_output2= 5U;
#endif
       params.output_fcp_mapping[0] = 0U; 
       params.output_fcp_mapping[2] = 1U; 

       VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
       VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
       VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

       ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);


       ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                               raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                               h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

       VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

       ASSERT_NO_FAILURE(vxVerifyGraph(graph));

       VX_CALL(vxReleaseNode(&node));
       VX_CALL(vxReleaseGraph(&graph));
       VX_CALL(vxReleaseDistribution(&histogram));
       VX_CALL(vxReleaseImage(&s8_b8_c4));
       VX_CALL(vxReleaseImage(&uv8_g8_c3));
       VX_CALL(vxReleaseImage(&y8_r8_c2));
       VX_CALL(vxReleaseImage(&uv12_c1));
       VX_CALL(vxReleaseImage(&y12));
       VX_CALL(tivxReleaseRawImage(&raw));
       VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
       VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
       VX_CALL(vxReleaseUserDataObject(&configuration));

       ASSERT(node == 0);
       ASSERT(graph == 0);
       ASSERT(h3a_aew_af == 0);
       ASSERT(histogram == 0);
       ASSERT(s8_b8_c4 == 0);
       ASSERT(uv8_g8_c3 == 0);
       ASSERT(y8_r8_c2 == 0);
       ASSERT(uv12_c1 == 0);
       ASSERT(y12 == 0);
       ASSERT(raw == 0);
       ASSERT(ae_awb_result == 0);
       ASSERT(configuration == 0);

       tivxHwaUnLoadKernels(context);
   }
}

/* Test case for negative bypass_glbce and bypass_nsf4 check*/
typedef struct
{
    const char* testName;
    int fcp1config,check;
    char *target_string;

} Set_fcp1;

#define ADD_fcp1config(testArgName, nextmacro, ...) \
   CT_EXPAND(nextmacro(testArgName "/fcp1config=TIVX_VPAC_VISS_FCP1_INPUT_NSF4", __VA_ARGS__, 0)), \
   CT_EXPAND(nextmacro(testArgName "/fcp1config=TIVX_VPAC_VISS_FCP1_INPUT_CAC", __VA_ARGS__, 1)), \
   CT_EXPAND(nextmacro(testArgName "/fcp1config=TIVX_VPAC_VISS_FCP1_INPUT_RFE", __VA_ARGS__, 2))

   #define ADD_Check(testArgName, nextmacro, ...) \
   CT_EXPAND(nextmacro(testArgName "/check=FALSE", __VA_ARGS__, 0)), \
   CT_EXPAND(nextmacro(testArgName "/check=TRUE", __VA_ARGS__, 1))  

#define SET_FCP1_PARAMETERS \
    CT_GENERATE_PARAMETERS("testNegativefcp1_config", ADD_fcp1config, ADD_Check, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativefcp1_config, Set_fcp1, SET_FCP1_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 3U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
#if defined (VPAC3)
        params.fcp[1].ee_mode = 4U;
#endif
        params.fcp[0].chroma_mode = 0U;      
        params.fcp1_config = TIVX_VPAC_VISS_FCP1_DISABLED;
        params.output_fcp_mapping[0] = 1U; 

        switch(arg_->fcp1config)
        {
        case 0:
        {
            if(0==arg_->check)
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_NSF4;
            params.bypass_glbce = 0U;
            }
            else
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_NSF4;
            params.bypass_glbce = 1U;  
            }
            break;
        }
        case 1:
        {
            if(0==arg_->check)
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_CAC;
            params.bypass_glbce = 1U;
            params.bypass_nsf4 = 0U;
            }
            else
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_CAC;
            params.bypass_glbce = 1U;
            params.bypass_nsf4 = 1U;  
            }
            break;
        }

        case 2:
        {
            if(0==arg_->check)
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_RFE;
            params.bypass_glbce = 1U;
            params.bypass_nsf4 = 0U;
            }
            else
            {
            params.fcp1_config = TIVX_VPAC_VISS_FCP1_INPUT_RFE;
            params.bypass_glbce = 1U;
            params.bypass_nsf4 = 1U;  
            }
            break;
        }

        default:
        break;
        }
  
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for chroma_mode check*/

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeMuxChrome, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;
    tivx_h3a_aew_config h3a_aew;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 256;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;


    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        params.fcp[0].mux_output2 = 0;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = 3;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = 2;

        params.output_fcp_mapping[0u] = 4; 
        params.output_fcp_mapping[1u] = 4; 
        params.output_fcp_mapping[2u] = 4; 
        params.output_fcp_mapping[3u] = 4; 
        params.output_fcp_mapping[4u] = 2; 
        params.fcp1_config = 5;
    #if defined (VPAC3L)
        params.enable_ir_op = 2;
        params.enable_bayer_op = 2;
        params.bypass_pcid = 2;
    #endif
        params.bypass_cac = 2;
        params.bypass_dwb = 2;
        params.bypass_glbce = 2;
        params.bypass_nsf4 = 2;
       
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                NULL, histogram, NULL, NULL), VX_TYPE_NODE);
                                                ASSERT_NO_FAILURE(vxVerifyGraph(graph));
                                                ASSERT_NO_FAILURE(vxProcessGraph(graph));
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));    
         
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case for dcc_param_name check*/

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeDccParam, ArgOutput, PARAMETERS_OUT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image y8_r8_c2[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_distribution raw_hist[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss_user";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
 
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
 
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};
 
    tivx_raw_image_create_params_t raw_params;
 
    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }
 
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
 
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;
 
    {
        vx_uint32 width = 0, height = 0, i = 0, j = 0;
 
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
 
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
 
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
 
                ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
                
                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);
 
                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
                params.fcp[0].mux_output0 = 0;
                params.fcp[0].mux_output1 = 0;
 
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
                    
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
 
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
                params.bypass_glbce = 1;
                params.bypass_nsf4 = 1;
 
                    params.bypass_cac = 1;
 
                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
                        /* Creating DCC */

                        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);
                                               
                        ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
                        VX_CALL(vxMapUserDataObject(
                            dcc_param_viss[i],
                            0,
                            dcc_buff_size,
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST,
                            0
                        ));
                        memset(dcc_viss_buf, 0xAB, dcc_buff_size);
 
                        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                        ASSERT(dcc_status == 0);
 
                        VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                        /* Done w/ DCC */
                    
                    
                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
                    if(NULL != h3a_aew_af[i])
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));
 
                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));
 
                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                    }
 
                
                #if defined (VPAC3) || defined (VPAC3L)
                /* Creating raw hist output */
                int nbins = 128U;
                int offset = 0U;
                int range = 128U;
                ASSERT_VX_OBJECT(raw_hist[i] = vxCreateDistribution(context, nbins, offset, range),
                    (enum vx_type_e)VX_TYPE_DISTRIBUTION);
                #endif
 
                ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, y8_r8_c2[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, raw_hist[i]), VX_TYPE_NODE);
 
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
        ASSERT_NO_FAILURE(vxProcessGraph(graph));
 
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&y8_r8_c2[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
 
                    VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                    VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
  
                #if defined (VPAC3) || defined (VPAC3L)
                VX_CALL(vxReleaseDistribution(&raw_hist[i]));
                #endif
            }
        }
 
        VX_CALL(vxReleaseGraph(&graph));
 
        ASSERT(graph == 0);
 
        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(y8_r8_c2[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
                #if defined (VPAC3) || defined (VPAC3L)
                ASSERT(&raw_hist[i]);
                #endif
            }
        }
 
        tivxHwaUnLoadKernels(context);
    }
}


/* Test case for negative size check  */

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    int neg_size;
    char *target_string;
} Negative_Size;


#define ADD_NEGATIVE_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/neg_size=viss", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=aeawb", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/neg_size=h3adata", __VA_ARGS__, 2))

#define NEGATIVESIZE_PARAMETERS \
    CT_GENERATE_PARAMETERS("testNegativeSize", ADD_NEGATIVE_SIZE, ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeSizeCheck, Negative_Size, NEGATIVESIZE_PARAMETERS)
{
   vx_context context = context_->vx_context_;
   vx_user_data_object configuration = NULL;
   vx_user_data_object ae_awb_result = NULL;
   tivx_raw_image raw = NULL;
   vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
   vx_distribution histogram = NULL;
   vx_user_data_object h3a_aew_af = NULL;

   tivx_vpac_viss_params_t params;
   tivx_ae_awb_params_t ae_awb_params;
   tivx_h3a_data_t h3a_data;

   vx_graph graph = 0;
   vx_node node = 0;

   tivx_raw_image_create_params_t raw_params;
   raw_params.width = 128;
   raw_params.height = 128;
   raw_params.num_exposures = 3;
   raw_params.line_interleaved = vx_false_e;
   raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
   raw_params.format[0].msb = 11;
   raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
   raw_params.format[1].msb = 7;
   raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
   raw_params.format[2].msb = 11;
   raw_params.meta_height_before = 5;
   raw_params.meta_height_after = 0;

   ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

   {
       tivxHwaLoadKernels(context);
       CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

       ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
       ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       
       ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 127, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 127, 127, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
       ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

       tivx_vpac_viss_params_init(&params);
       
       tivx_ae_awb_params_init(&ae_awb_params);
       
       tivx_h3a_data_init(&h3a_data);
       
       switch(arg_->neg_size)
       {
        case 0:
        {
            ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                sizeof(uint32_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT); 
            ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                    sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                        sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(uint32_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                        
            break;
        }
        case 1:
        {
            ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                sizeof(uint32_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                    sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                        sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(uint32_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                        
            break;         
        }
        case 2:
        {
            ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                sizeof(uint32_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT); 
            ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                    sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                        sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                        
            VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(uint32_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                        
            break;           
        } 
       default:
       {
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT); 
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                    sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    
        break;
       }
      }
      
      ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
      ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                               raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                               h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

       ASSERT_NO_FAILURE(vxVerifyGraph(graph));

       VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

       VX_CALL(vxReleaseNode(&node));
       VX_CALL(vxReleaseGraph(&graph));
       VX_CALL(vxReleaseDistribution(&histogram));
       VX_CALL(vxReleaseImage(&s8_b8_c4));
       VX_CALL(vxReleaseImage(&uv8_g8_c3));
       VX_CALL(vxReleaseImage(&y8_r8_c2));
       VX_CALL(vxReleaseImage(&uv12_c1));
       VX_CALL(vxReleaseImage(&y12));
       VX_CALL(tivxReleaseRawImage(&raw));
       VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
       VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
       VX_CALL(vxReleaseUserDataObject(&configuration));

       ASSERT(node == 0);
       ASSERT(graph == 0);
       ASSERT(h3a_aew_af == 0);
       ASSERT(histogram == 0);
       ASSERT(s8_b8_c4 == 0);
       ASSERT(uv8_g8_c3 == 0);
       ASSERT(y8_r8_c2 == 0);
       ASSERT(uv12_c1 == 0);
       ASSERT(y12 == 0);
       ASSERT(raw == 0);
       ASSERT(ae_awb_result == 0);
       ASSERT(configuration == 0);

       tivxHwaUnLoadKernels(context);
   }
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativeImage, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_RGB565), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
    
}

/*tivxAddKernelVpacVissValidate function cover VX_DF_IMAGE_S16 for VX_TYPE_IMAGE negative*/
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testMuxFormatOutNegative, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_image y12_ref = NULL, uv12_c1_ref = NULL, y8_r8_c2_ref = NULL, uv8_g8_c3_ref = NULL, s8_b8_c4_ref = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram_ref = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_user_data_object h3a_aew_af_ref = NULL;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual = 0;
    // vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1280;
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_C1 ;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_C3;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, NULL, histogram, NULL), VX_TYPE_NODE);

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/bayer_1280x720.raw", 2, 0);

        ASSERT_NO_FAILURE(vxProcessGraph(graph));
        save_image_from_viss(y8_r8_c2, "output/out_y8");


        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testConfigNegative, ArgOutput, PARAMETERS_OUT)
{
   vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        if(arg_->output_fmt == 0)
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        }
        else if(arg_->output_fmt == 1)
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else
        {
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        

        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t) + 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t) + 2, &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);



        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
// for covering default arams.fcp1_config
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testNegativefcpdefault, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_image y12_ref = NULL, y8_r8_c2_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    
    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;
    
    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
        
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            
       params.output_fcp_mapping[4u] = 1; 
  
     params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;

#if defined (VPAC3)
        params.fcp[1].mux_output4 = 2;
#endif
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_cac = 1;
        
        params.fcp1_config = 5;

#if defined (VPAC3)
        params.fcp[1].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[1].chroma_mode = 0;
#endif
        params.enable_ctx = 1;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;


        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        
        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);
	    ASSERT_NO_FAILURE(vxVerifyGraph(graph));
	
	    VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov, testPsaSignValue_negative, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[6] = {0};
    vx_user_data_object psa_obj;
    int i;
    tivx_vpac_viss_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)raw, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        params.enable_psa = 1u;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_viss_psa_timestamp_data_t", sizeof(tivx_vpac_viss_psa_timestamp_data_t) + 1, NULL),
        VX_TYPE_USER_DATA_OBJECT);
        
        psa_refs[0] = (vx_reference) psa_obj;
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_GET_PSA_STATUS,
            psa_refs, 1u));
            
        VX_CALL(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_viss_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/*
@brief Tests edge enhancer mode with Y12 output
Verifies that the VISS node correctly processes edge enhancement
when configured for Y12 output format.
Coverage: tivxVpacVissSetFcpConfig - EE_on_Y12 branch
*/

TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testEdgeEnhancerMode, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y8_r8_c2 = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0;
    uint32_t checksum_pc = (uint32_t) 0x2e9af183;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = sensor_dcc_id;                                                    
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y12;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */


        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
        }
                   

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

                
        /* for checksum */   
        snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_msb_out_", 0 ,".yuv");  
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_viss_output_image(file, y8_r8_c2);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf(" checksum_actual=  0x%08x\n", checksum_actual);
        #endif
        ASSERT( checksum_pc == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/*
@brief Tests edge enhancer mode with Y12 output
Verifies that the VISS node correctly processes edge enhancement
when configured for Y12 output format and
data format is FVID2_DF_YUV422I_UYVY
Coverage: tivxVpacVissSetFcpConfig - FVID2_DF_YUV422I_UYVY branch
*/

TEST_WITH_ARG(tivxHwaVpacVissPositiveCov, testEdgeEnhancerModeDataFormat, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image  y8_r8_c2 = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0;
    uint32_t checksum_pc = (uint32_t) 0xdca1c62f;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = sensor_dcc_id;                                                    
        params.fcp[0].ee_mode = 1;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.bypass_cac = 1;
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */


        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
        }
                   

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

                
        /* for checksum */     
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_viss_output_image(file, y8_r8_c2);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf(" checksum_actual=  0x%08x\n", checksum_actual);
        #endif
        
        ASSERT( checksum_pc == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/* Test case to cover else case branch coverage for the if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[outID].fmt.dataFormat) ||
                 (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[outID].fmt.dataFormat) ) condition from tivxVpacVissSetFcpConfig function - vx_vpac_viss_target_drv.c */

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov2, testNegativeedgeEnhancerMode, Argnode_cmd_id, PARAMETERS_NODE_ID
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, y8_r8_c2 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    vx_reference psa_refs [5] = {0};
    uint32_t psa_values[6] = {0};
    vx_user_data_object psa_obj;
    int i;
    tivx_vpac_viss_psa_timestamp_data_t psa_status;
    uint64_t input_timestamp = 0;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        input_timestamp = SET_TIME_STAMP_VALUE;
        VX_CALL(tivxSetReferenceAttribute((vx_reference)raw, TIVX_REFERENCE_TIMESTAMP,
        &input_timestamp, sizeof(input_timestamp)));

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        params.enable_psa = 1u;
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = 1;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);
        #endif
        VX_CALL(vxProcessGraph(graph));
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_viss_output_image(file, y8_r8_c2);
        #endif

        ASSERT_VX_OBJECT(psa_obj = 
        vxCreateUserDataObject(context, "tivx_vpac_viss_psa_timestamp_data_t", sizeof(tivx_vpac_viss_psa_timestamp_data_t), NULL),
        VX_TYPE_USER_DATA_OBJECT);
        
        psa_refs[0] = (vx_reference) psa_obj;
        switch (arg_->node_cmd_id)
        {
        case 0:
        ASSERT_EQ_VX_STATUS(VX_SUCCESS,
            tivxNodeSendCommand(node, 0u, TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS,
            psa_refs, 1u));
            break;
        case 1:
            break;
        case 2:
        ASSERT_EQ_VX_STATUS(VX_FAILURE,
            tivxNodeSendCommand(node, 0u, 0,
            psa_refs, 1u));
            break;
        default:
            break;
        }
        
            
        ASSERT_NO_FAILURE(vxCopyUserDataObject(psa_obj, 0, sizeof(tivx_vpac_viss_psa_timestamp_data_t), 
            &psa_status, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

        #if defined TEST_PRINT_TIME_STAMP_VALUES
        printf("PSA Timestamp = %" PRIu64 "\n", psa_status.timestamp);
        #endif
        VX_CALL(vxReleaseUserDataObject(&psa_obj));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

/*
@brief Tests edge enhancer mode with Y8 output
Verifies that the VISS node correctly processes edge enhancement
when configured for Y8 output format.
Coverage: tivxVpacVissSetFcpConfig - EE_on_Y8 branch
*/

TEST_WITH_ARG(tivxHwaVpacVissNegativeCov2, testEdgeEnhancerModeY8, SetTarget_Arg,
    SET_NODE_TARGET_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y8_r8_c2 = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;
    vx_rectangle_t rect;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.meta_height_after = 4;
    sensor_dcc_id = 390;
    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
    downshift_bits = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = sensor_dcc_id;                                                    
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_FCP0_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.bypass_cac = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */


        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af, dcc_viss_buf_map_id));
        }
                   

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, dcc_param_viss,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);
        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}
/*
@brief Negative test case coverage 
Coverage: tivxAddKernelVpacVissValidate - params.enable_ir_op == TIVX_VPAC_VISS_IR_ENABLE
*/
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov2, testNegativeIrEnable, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;
    tivx_h3a_aew_config h3a_aew;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 256;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;


    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        params.fcp[0].mux_output2 = 0;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = 3;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = 2;

        params.output_fcp_mapping[0u] = 4; 
        params.output_fcp_mapping[1u] = 4; 
        params.output_fcp_mapping[2u] = 4; 
        params.output_fcp_mapping[3u] = 4; 
        params.output_fcp_mapping[4u] = 2; 
        params.fcp1_config = 5;
    #if defined (VPAC3L)
        params.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
        params.enable_bayer_op = 2;
        params.bypass_pcid = 2;
    #endif
        params.bypass_cac = 2;
        params.bypass_dwb = 2;
        params.bypass_glbce = 2;
        params.bypass_nsf4 = 2;
       
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                NULL, histogram, NULL, NULL), VX_TYPE_NODE);
                                                ASSERT_NO_FAILURE(vxVerifyGraph(graph));
                                                ASSERT_NO_FAILURE(vxProcessGraph(graph));
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));    
         
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
/*
@brief Negative test case coverage for TIVX_DF_IMAGE_P12 
Coverage: tivxAddKernelVpacVissValidate - 
*/
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov2, testNegativeImgP12, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;
    tivx_h3a_aew_config h3a_aew;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 256;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;


    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        params.fcp[0].mux_output2 = 0;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = 3;
        params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = 2;

        params.output_fcp_mapping[0u] = 4; 
        params.output_fcp_mapping[1u] = 4; 
        params.output_fcp_mapping[2u] = 4; 
        params.output_fcp_mapping[3u] = 4; 
        params.output_fcp_mapping[4u] = 2; 
        params.fcp1_config = 5;
    #if defined (VPAC3L)
        params.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
        params.enable_bayer_op = 2;
        params.bypass_pcid = 2;
    #endif
        params.bypass_cac = 2;
        params.bypass_dwb = 2;
        params.bypass_glbce = 2;
        params.bypass_nsf4 = 2;
       
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                NULL, histogram, NULL, NULL), VX_TYPE_NODE);
                                                ASSERT_NO_FAILURE(vxVerifyGraph(graph));
                                                ASSERT_NO_FAILURE(vxProcessGraph(graph));
        
        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));    
         
        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}
 
/*
@brief Negative test case coverage for Fcp Mapping 
Coverage: tivxAddKernelVpacVissValidate - (fcp_of_0 == fcp_of_2) && (0 != chroma_mux_status[fcp_of_0])
*/
TEST_WITH_ARG(tivxHwaVpacVissNegativeCov2, testNegativeOutput_fcp, ArgOutput, PARAMETERS_OUT)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;
    

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        if(arg_->output_fmt == 0)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        }
        else if(arg_->output_fmt == 1)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        if(arg_->output_fmt == 0)
        {
            params.fcp[0].chroma_mode = 0U;
            params.output_fcp_mapping[0] = 0;
            params.output_fcp_mapping[2] = 1;
        }
        else if(arg_->output_fmt == 1)
        {
            params.fcp[0].chroma_mode = 2U;
            params.output_fcp_mapping[0] = 0;
            params.output_fcp_mapping[2] = 0;        
        }
        else
        {
            params.fcp[0].chroma_mode = 0U;
            params.output_fcp_mapping[0] = 0;
            params.output_fcp_mapping[2] = 0;
        }
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NO_FAILURE(vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
    
}

#if defined(BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS)
#define testMuxNegative testMuxNegative
#else
#define testMuxNegative DISABLED_testMuxNegative
#endif
TESTCASE_TESTS(tivxHwaVpacViss,
               testNodeCreation,
               #ifndef VPAC3L
               testGraphProcessing,
               #endif
               testGraphProcessingFile,
               testGraphProcessing_rgb,
               #ifndef VPAC3L
               testGraphProcessingRaw,
               #endif
               testGraphProcessingFileDcc,
               #ifndef x86_64
               testGraphProcessingFileDccSafety,
               #endif
               #ifdef VPAC3L
               testGraphProcessingFileDccIr,
               testGraphProcessingFileDccIrSafety,
               testGraphProcessingFileDccRgbIr,
               testGraphProcessingFileDccRgbIrSafety,
               #endif
               #ifdef VPAC3
               testHV_MV,
               #ifndef x86_64
               testHV_MV_safety,
               #endif
               #endif
               #ifndef x86_64
               testErrorInterrupts,
               testPsaSignValue,
               tesVpacVissGetErrStatus,
               #ifndef VPAC3L
               testSl2Firewall,
               #endif
               #endif
               testNegativeGraph
               )
TESTCASE_TESTS(tivxHwaVpacVissPositiveCov,
               testMsb,
               testPixel,
               testMuxFormat,
               testDccTarget,
               testEdgeEnhancerMode,
               testEdgeEnhancerModeDataFormat
               )
TESTCASE_TESTS(tivxHwaVpacVissNegativeCov,
               testNegativeDimension,               
               testNegativeParamDimensionCheck,
               testNegativeParams,
               testNegativeOutput_fm,
               testNegativedataobject,
               testNegativeConfig,
               testNegativeawbParams,
               testNegativeaewafParams,
               testNegativeOutput,
               testNegativeMVINP,
               testNegativeOUT,
               testNegativeMuxFormat, 
               testNegativeHistCheck,
               testNegativeMux,
               testNegativeParamh3a,
               testNegativeHist1Check,
               testNegativefcp_of_0_2,
               testNegativefcp1_config,
               testNegativeMuxChrome,
               testNegativeDccParam,
               testNegativeSizeCheck,
               testNegativeImage,
               testMuxFormatOutNegative,
               testConfigNegative,
               testNegativefcpdefault,
               testPsaSignValue_negative,
               #if defined(VPAC3) || defined(VPAC3L)
               testnegativeOutFormat,
               #endif
               testH3A_AEW,
               testHist,
               testnode_cmd_id
                              )            
TESTCASE_TESTS(tivxHwaVpacVissNegativeCov2,
                testNegativeIrEnable,
                testNegativeImgP12,
                testNegativeOutput_fcp,
                testNegativeedgeEnhancerMode,
                testEdgeEnhancerModeY8
                )
#endif /* BUILD_VPAC_VISS */
