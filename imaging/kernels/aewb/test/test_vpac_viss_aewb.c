/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/hwa_kernels.h>
#include <TI/hwa_vpac_viss.h>
#include <TI/j7_imaging_aewb.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <utils/ipc/include/app_ipc.h>

#include <iss_sensors.h>
#include <iss_sensor_if.h>
#include <itt_server_remote/include/itt_srvr_remote.h>

#define APP_MAX_FILE_PATH           (512u)

/*
 * On AM62A, MCU2_0 is not available; MCU1_0 is the equivalent target
 */
#if defined(SOC_AM62A)
#define TIVX_TARGET_MCU  TIVX_TARGET_MCU1_0
#else
#define TIVX_TARGET_MCU  TIVX_TARGET_MCU2_0
#endif

#define ADD_SIZE_64x48(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=64x48", __VA_ARGS__, 64, 48))

TESTCASE(tivxHwaVpacAewb, CT_VXContext, ct_setup_vx_context, 0)

static vx_int32 read_dcc_file(char * file_name, uint8_t * buf, uint32_t num_bytes)
{
    vx_uint32 num_bytes_read_from_file;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    num_bytes_read_from_file = fread(buf, sizeof(uint8_t), num_bytes, fp);
    fclose(fp);

    return num_bytes_read_from_file;
}

static vx_int32 get_dcc_file_size(char * file_name)
{
    vx_uint32 num_bytes;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    num_bytes = ftell(fp);
    fclose(fp);

    return num_bytes;
}

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,
    vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/*
 * Utility API to export graph information to file for debug and visualization
 */
static vx_status export_graph_to_file(vx_graph graph, char *filename_prefix)
{
    size_t sz = 0;
    char filepath[MAXPATHLENGTH];

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/output", ct_get_test_file_path());
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
    return tivxExportGraphToDot(graph, filepath, filename_prefix);
}


#define MAX_NUM_BUF                         (8u)
#define NUM_CAPT_CHANNELS                   (4U)
#define NUM_BUFS                            (4u)

TEST(tivxHwaVpacAewb, testSingleChannel)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

TEST(tivxHwaVpacAewb, testMultiChannel)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_object_array capt_frames[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_arr;
    vx_object_array ae_awb_result_arr;


    /* VISS Output */
    vx_image sample_nv12_img;
    vx_image viss_nv12_out_img;
    vx_object_array viss_out_frames;
    vx_user_data_object h3a_aew_af_exemplar;
    vx_object_array h3a_aew_af_arr;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config_exemplar;
    vx_object_array aewb_config_array;
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    vx_bool viss_prms_replicate[] =
        {vx_false_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
         vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
    vx_bool aewb_prms_replicate[] =
        {vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_false_e};

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        ASSERT_VX_OBJECT(raw[0] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* Allocate frames */
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(capt_frames[buf_id] =
                vxCreateObjectArray(context,
                (vx_reference)raw[0], NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);
        }
        tivxReleaseRawImage(&raw[0]);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = (tivx_raw_image) vxGetObjectArrayItem(capt_frames[buf_id], 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        /* Allocate sample NV12 image, using which object array of NV12
         * would be created */
        ASSERT_VX_OBJECT(sample_nv12_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Allocate object array for the output frames */
        ASSERT_VX_OBJECT(viss_out_frames = vxCreateObjectArray(context,
            (vx_reference)sample_nv12_img, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            (vx_image)vxGetObjectArrayItem(viss_out_frames, 0), VX_TYPE_IMAGE);

        /* Sample image is no longer required */
        VX_CALL(vxReleaseImage(&sample_nv12_img));

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af_exemplar = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(h3a_aew_af_arr = vxCreateObjectArray(context,
            (vx_reference)h3a_aew_af_exemplar, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(h3a_aew_af = (vx_user_data_object) vxGetObjectArrayItem(h3a_aew_af_arr, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_exemplar));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(ae_awb_result_arr = vxCreateObjectArray(context,
            (vx_reference)ae_awb_result, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(delay_arr = vxCreateDelay(context, (vx_reference)(ae_awb_result_arr), 2), VX_TYPE_DELAY);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        vx_object_array tmp_obj_arr_1;
        vx_object_array tmp_obj_arr_0;

        vx_user_data_object tmp_user_data_object_1;
        vx_user_data_object tmp_user_data_object_0;

        ASSERT_VX_OBJECT(tmp_obj_arr_1 = (vx_object_array)vxGetReferenceFromDelay(delay_arr, -1), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(tmp_obj_arr_0 = (vx_object_array)vxGetReferenceFromDelay(delay_arr, 0), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(tmp_user_data_object_1 = (vx_user_data_object)vxGetObjectArrayItem(tmp_obj_arr_1, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(tmp_user_data_object_0 = (vx_user_data_object)vxGetObjectArrayItem(tmp_obj_arr_0, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(tmp_user_data_object_1, 0, sizeof(tivx_aewb_config_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                (vx_user_data_object)tmp_user_data_object_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);
        vxReplicateNode(graph, vissNode, viss_prms_replicate, 13u);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        /*Seperate config for AEWB nodes to pass unique channel ID*/
        ASSERT_VX_OBJECT(aewb_config_exemplar = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(aewb_config_array = vxCreateObjectArray(context, (vx_reference)aewb_config_exemplar, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(vxReleaseUserDataObject(&aewb_config_exemplar));

        for(loop_id=0;loop_id<NUM_CAPT_CHANNELS;loop_id++)
        {
            ASSERT_VX_OBJECT(aewb_config_exemplar = (vx_user_data_object)vxGetObjectArrayItem(aewb_config_array, loop_id), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            aewb_cfg.channel_id = loop_id;
            VX_CALL(vxCopyUserDataObject(aewb_config_exemplar, 0, sizeof(tivx_aewb_config_t), &aewb_cfg, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxReleaseUserDataObject(&aewb_config_exemplar));
        }
        ASSERT_VX_OBJECT(aewb_config = (vx_user_data_object)vxGetObjectArrayItem(aewb_config_array, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 tmp_user_data_object_1,
                                 tmp_user_data_object_0,
                                 dcc_param_2a), VX_TYPE_NODE);
        vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU);
        vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode");

        vxReplicateNode(graph, node_aewb, aewb_prms_replicate, 6u);

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        tivxSetGraphPipelineDepth(graph, NUM_BUFS);

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_arr));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_multi_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<2; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&tmp_user_data_object_1));
        VX_CALL(vxReleaseUserDataObject(&tmp_user_data_object_0));
        VX_CALL(vxReleaseDelay(&delay_arr));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
            VX_CALL(vxReleaseObjectArray(&capt_frames[buf_id]));
        }
        VX_CALL(vxReleaseObjectArray(&ae_awb_result_arr));
        VX_CALL(vxReleaseObjectArray(&viss_out_frames));
        VX_CALL(vxReleaseObjectArray(&h3a_aew_af_arr));
        VX_CALL(vxReleaseObjectArray(&aewb_config_array));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

TEST(tivxHwaVpacAewb, testMultiChannelNullH3A)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_object_array capt_frames[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_arr;
    vx_object_array ae_awb_result_arr;

    uint32_t i;

    /* VISS Output */
    vx_image sample_nv12_img;
    vx_image viss_nv12_out_img;
    vx_object_array viss_out_frames;
    vx_user_data_object h3a_aew_af_exemplar;
    vx_object_array h3a_aew_af_arr;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config_exemplar;
    vx_object_array aewb_config_array;
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;


    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_params;
    vx_bool viss_prms_replicate[] =
        {vx_false_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
         vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
    vx_bool aewb_prms_replicate[] =
        {vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_false_e};

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        ASSERT_VX_OBJECT(raw[0] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        /* Allocate frames */
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(capt_frames[buf_id] =
                vxCreateObjectArray(context,
                (vx_reference)raw[0], NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);
        }
        tivxReleaseRawImage(&raw[0]);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = (tivx_raw_image) vxGetObjectArrayItem(capt_frames[buf_id], 0), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        /* Allocate sample NV12 image, using which object array of NV12
         * would be created */
        ASSERT_VX_OBJECT(sample_nv12_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Allocate object array for the output frames */
        ASSERT_VX_OBJECT(viss_out_frames = vxCreateObjectArray(context,
            (vx_reference)sample_nv12_img, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            (vx_image)vxGetObjectArrayItem(viss_out_frames, 0), VX_TYPE_IMAGE);

        /* Sample image is no longer required */
        VX_CALL(vxReleaseImage(&sample_nv12_img));

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af_exemplar = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(h3a_aew_af_arr = vxCreateObjectArray(context,
            (vx_reference)h3a_aew_af_exemplar, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        for (i = 0; i < NUM_CAPT_CHANNELS; i++)
        {
            ASSERT_VX_OBJECT(h3a_aew_af = (vx_user_data_object) vxGetObjectArrayItem(h3a_aew_af_arr, i), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            memset(&h3a_params, 0, sizeof(tivx_h3a_data_t));
#if defined(SOC_AM62A)
            h3a_params.cpu_id = APP_IPC_CPU_MCU1_0;
#else
            h3a_params.cpu_id = APP_IPC_CPU_MCU2_0;
#endif

            VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

            VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        }

        ASSERT_VX_OBJECT(h3a_aew_af = (vx_user_data_object) vxGetObjectArrayItem(h3a_aew_af_arr, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_exemplar));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(ae_awb_result_arr = vxCreateObjectArray(context,
            (vx_reference)ae_awb_result, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(delay_arr = vxCreateDelay(context, (vx_reference)(ae_awb_result_arr), 2), VX_TYPE_DELAY);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        vx_object_array tmp_obj_arr_1;
        vx_object_array tmp_obj_arr_0;

        vx_user_data_object tmp_user_data_object_1;
        vx_user_data_object tmp_user_data_object_0;

        ASSERT_VX_OBJECT(tmp_obj_arr_1 = (vx_object_array)vxGetReferenceFromDelay(delay_arr, -1), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(tmp_obj_arr_0 = (vx_object_array)vxGetReferenceFromDelay(delay_arr, 0), VX_TYPE_OBJECT_ARRAY);

        ASSERT_VX_OBJECT(tmp_user_data_object_1 = (vx_user_data_object)vxGetObjectArrayItem(tmp_obj_arr_1, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(tmp_user_data_object_0 = (vx_user_data_object)vxGetObjectArrayItem(tmp_obj_arr_0, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(tmp_user_data_object_1, 0, sizeof(tivx_aewb_config_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                (vx_user_data_object)tmp_user_data_object_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                NULL, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);
        vxReplicateNode(graph, vissNode, viss_prms_replicate, 13u);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        /*Seperate config for AEWB nodes to pass unique channel ID*/
        ASSERT_VX_OBJECT(aewb_config_exemplar = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(aewb_config_array = vxCreateObjectArray(context, (vx_reference)aewb_config_exemplar, NUM_CAPT_CHANNELS), VX_TYPE_OBJECT_ARRAY);
        VX_CALL(vxReleaseUserDataObject(&aewb_config_exemplar));

        for(loop_id=0;loop_id<NUM_CAPT_CHANNELS;loop_id++)
        {
            ASSERT_VX_OBJECT(aewb_config_exemplar = (vx_user_data_object)vxGetObjectArrayItem(aewb_config_array, loop_id), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            aewb_cfg.channel_id = loop_id;
            VX_CALL(vxCopyUserDataObject(aewb_config_exemplar, 0, sizeof(tivx_aewb_config_t), &aewb_cfg, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxReleaseUserDataObject(&aewb_config_exemplar));
        }
        ASSERT_VX_OBJECT(aewb_config = (vx_user_data_object)vxGetObjectArrayItem(aewb_config_array, 0), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 tmp_user_data_object_1,
                                 tmp_user_data_object_0,
                                 dcc_param_2a), VX_TYPE_NODE);
        vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU);
        vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode");

        vxReplicateNode(graph, node_aewb, aewb_prms_replicate, 6u);

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        tivxSetGraphPipelineDepth(graph, NUM_BUFS);

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_arr));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_multi_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<2; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&tmp_user_data_object_1));
        VX_CALL(vxReleaseUserDataObject(&tmp_user_data_object_0));
        VX_CALL(vxReleaseDelay(&delay_arr));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
            VX_CALL(vxReleaseObjectArray(&capt_frames[buf_id]));
        }
        VX_CALL(vxReleaseObjectArray(&ae_awb_result_arr));
        VX_CALL(vxReleaseObjectArray(&viss_out_frames));
        VX_CALL(vxReleaseObjectArray(&h3a_aew_af_arr));
        VX_CALL(vxReleaseObjectArray(&aewb_config_array));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}
/* Test case with invalid size for tivx_aewb_config_t  */
TEST(tivxHwaVpacAewb, testNegativeconfiguration_size)
 {
     vx_context context = context_->vx_context_;
     uint32_t buf_id, loop_id, num_refs;
 
     /* VISS Input */
     tivx_raw_image raw[MAX_NUM_BUF];
     vx_user_data_object configuration = NULL;
     vx_user_data_object ae_awb_result;
     vx_delay delay_2a_res;
 
 
     /* VISS Output */
     vx_image viss_nv12_out_img;
     vx_user_data_object h3a_aew_af = NULL;
 
     /* AEWB Objects */
     vx_user_data_object aewb_config;
     vx_user_data_object dcc_param_2a;
 
     vx_user_data_object ae_aewb_delay_1;
 
     vx_size dcc_buff_size;
     char dcc_viss_file_path[APP_MAX_FILE_PATH];
     const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
     vx_map_id dcc_2a_buf_map_id;
     uint8_t * dcc_2a_buf;
     tivx_aewb_config_t aewb_cfg;
     tivx_vpac_viss_params_t params;
     tivx_ae_awb_params_t ae_awb_params;
 
     vx_graph graph = 0;
     vx_node vissNode = 0, node_aewb = 0;
 
     tivx_raw_image_create_params_t raw_params;
     raw_params.width = 1920;
     raw_params.height = 1080;
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
 
 
 
     if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
     {
         vx_uint32 width, height;
 
         tivxHwaLoadKernels(context);
 
         tivxImagingLoadKernels(context);
 
         for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
         {
             ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
         }
 
         VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
         VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
 
         ASSERT_VX_OBJECT(viss_nv12_out_img =
             vxCreateImage(context, width, height,
             VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
 
         /* Create/Configure configuration input structure */
         memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
         tivx_vpac_viss_params_init(&params);
         ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                             sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         params.sensor_dcc_id = 390;
         params.use_case = 0;
         params.fcp[0].ee_mode = 0;
         params.fcp[0].mux_output0 = 0;
         params.fcp[0].mux_output1 = 0;
         params.fcp[0].mux_output2 = 4;
         params.fcp[0].mux_output3 = 0;
         params.fcp[0].mux_output4 = 3;
         params.bypass_nsf4 = 1;
         params.h3a_in = 3;
         params.h3a_aewb_af_mode = 0;
         params.fcp[0].chroma_mode = 0;
         params.bypass_glbce = 1;
 
         VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
 
         ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         /* Create/Configure ae_awb_result input structure */
         memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
         ASSERT_VX_OBJECT(ae_awb_result =
             vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
             sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);
 
         ASSERT_VX_OBJECT(ae_aewb_delay_1 =
             (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
 
         ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
         ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                 ae_aewb_delay_1, NULL,
                                                 raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                 h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
         tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
         tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);
 
         vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
         vxSetNodeTarget(vissNode, VX_TARGET_STRING,
             TIVX_TARGET_VPAC_VISS1);
 
         aewb_cfg.sensor_dcc_id = 390;
         aewb_cfg.sensor_img_format = 0;
         aewb_cfg.sensor_img_phase = 3;
         aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
         aewb_cfg.awb_num_skip_frames = 9;
         aewb_cfg.ae_num_skip_frames = 9;
         aewb_cfg.channel_id = 0;
 
         ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t)+1, &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         /* Creating DCC */
         snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
         dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);
 
         dcc_param_2a = vxCreateUserDataObject(
             context,
             (const vx_char*)&dcc_2a_user_data_object_name,
             dcc_buff_size,
             NULL
          );
 
         vxMapUserDataObject(
             dcc_param_2a,
             0,
             dcc_buff_size,
             &dcc_2a_buf_map_id,
             (void **)&dcc_2a_buf,
             VX_WRITE_ONLY,
             VX_MEMORY_TYPE_HOST,
             0
         );
 
         memset(dcc_2a_buf, 0xAB, dcc_buff_size);
 
         read_dcc_file(
             dcc_viss_file_path,
             dcc_2a_buf,
             dcc_buff_size
         );
 
         vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);
 
         ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                  aewb_config,
                                  NULL,
                                  h3a_aew_af,
                                  ae_aewb_delay_1,
                                  (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                  dcc_param_2a), VX_TYPE_NODE);
         VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
         VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));
 
         VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));
 
         VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));
 
         vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];
 
         int graph_parameter_num = 0;
         add_graph_parameter_by_node_index(graph, vissNode, 3);
 
         graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
         graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
         graph_parameters_queue_params_list[graph_parameter_num].refs_list =
             (vx_reference*)&raw[0];
         graph_parameter_num++;
 
         VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));
 
         /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
          * Graph gets scheduled automatically as refs are enqueued to it
          */
         vxSetGraphScheduleConfig(graph,
                 VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                 1,
                 graph_parameters_queue_params_list
                 );
 
         VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));
 
         ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
 
         export_graph_to_file(graph, "viss_aewb_single_channel");
 
         /* Enqueue buf for pipe up but don't trigger graph execution */
         for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
         {
             graph_parameter_num = 0;
             vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                 (vx_reference*)&raw[buf_id], 1);
             graph_parameter_num++;
         }
 
         /* Need to trigger again since display holds on to a buffer */
         graph_parameter_num = 0;
         vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
             (vx_reference*)&raw[NUM_BUFS-1], 1);
         graph_parameter_num++;
 
         for(loop_id=0; loop_id<4; loop_id++)
         {
             tivx_raw_image raw;
 
             graph_parameter_num = 0;
 
             /* Get output reference, waits until a frame is available */
             vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                 (vx_reference*)&raw, 1, &num_refs);
             graph_parameter_num++;
 
             graph_parameter_num = 0;
 
             vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
             graph_parameter_num++;
         }
 
         vxWaitGraph(graph);
 
         VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
         VX_CALL(vxReleaseImage(&viss_nv12_out_img));
         VX_CALL(vxReleaseUserDataObject(&aewb_config));
         VX_CALL(vxReleaseUserDataObject(&configuration));
         VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
         VX_CALL(vxReleaseDelay(&delay_2a_res));
         VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
         for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
         {
             VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
         }
         VX_CALL(vxReleaseNode(&vissNode));
         VX_CALL(vxReleaseNode(&node_aewb));
         VX_CALL(vxReleaseGraph(&graph));
 
         ASSERT(vissNode == 0);
         ASSERT(graph == 0);
         ASSERT(h3a_aew_af == 0);
         ASSERT(viss_nv12_out_img == 0);
         ASSERT(configuration == 0);
 
         tivxHwaUnLoadKernels(context);
         tivxImagingUnLoadKernels(context);
     }
 }
/* Test case with dcc_param_name not equalto dcc_2a*/
 TEST(tivxHwaVpacAewb, testNegative_dcc_param)
 {
     vx_context context = context_->vx_context_;
     uint32_t buf_id, loop_id, num_refs;
 
     /* VISS Input */
     tivx_raw_image raw[MAX_NUM_BUF];
     vx_user_data_object configuration = NULL;
     vx_user_data_object ae_awb_result;
     vx_delay delay_2a_res;
 
 
     /* VISS Output */
     vx_image viss_nv12_out_img;
     vx_user_data_object h3a_aew_af = NULL;
 
     /* AEWB Objects */
     vx_user_data_object aewb_config;
     vx_user_data_object dcc_param_2a;
 
     vx_user_data_object ae_aewb_delay_1;
 
     vx_size dcc_buff_size;
     char dcc_viss_file_path[APP_MAX_FILE_PATH];
     const vx_char dcc_2a_user_data_object_name[] = "dcc_3a";
     vx_map_id dcc_2a_buf_map_id;
     uint8_t * dcc_2a_buf;
     tivx_aewb_config_t aewb_cfg;
     tivx_vpac_viss_params_t params;
     tivx_ae_awb_params_t ae_awb_params;
 
     vx_graph graph = 0;
     vx_node vissNode = 0, node_aewb = 0;
 
     tivx_raw_image_create_params_t raw_params;
     raw_params.width = 1920;
     raw_params.height = 1080;
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
 
 
 
     if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
     {
         vx_uint32 width, height;
 
         tivxHwaLoadKernels(context);
 
         tivxImagingLoadKernels(context);
 
         for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
         {
             ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
         }
 
         VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
         VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));
 
         ASSERT_VX_OBJECT(viss_nv12_out_img =
             vxCreateImage(context, width, height,
             VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
 
         /* Create/Configure configuration input structure */
         memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
         tivx_vpac_viss_params_init(&params);
         ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                             sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         params.sensor_dcc_id = 390;
         params.use_case = 0;
         params.fcp[0].ee_mode = 0;
         params.fcp[0].mux_output0 = 0;
         params.fcp[0].mux_output1 = 0;
         params.fcp[0].mux_output2 = 4;
         params.fcp[0].mux_output3 = 0;
         params.fcp[0].mux_output4 = 3;
         params.bypass_nsf4 = 1;
         params.h3a_in = 3;
         params.h3a_aewb_af_mode = 0;
         params.fcp[0].chroma_mode = 0;
         params.bypass_glbce = 1;
 
         VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
 
         ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         /* Create/Configure ae_awb_result input structure */
         memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
         ASSERT_VX_OBJECT(ae_awb_result =
             vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
             sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);
 
         ASSERT_VX_OBJECT(ae_aewb_delay_1 =
             (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
 
         ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
 
         ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                 ae_aewb_delay_1, NULL,
                                                 raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                 h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
         tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
         tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);
 
         vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
         vxSetNodeTarget(vissNode, VX_TARGET_STRING,
             TIVX_TARGET_VPAC_VISS1);
 
         aewb_cfg.sensor_dcc_id = 390;
         aewb_cfg.sensor_img_format = 0;
         aewb_cfg.sensor_img_phase = 3;
         aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
         aewb_cfg.awb_num_skip_frames = 9;
         aewb_cfg.ae_num_skip_frames = 9;
         aewb_cfg.channel_id = 0;
 
         ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
 
         /* Creating DCC */
         snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
         dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);
 
         dcc_param_2a = vxCreateUserDataObject(
             context,
             (const vx_char*)&dcc_2a_user_data_object_name,
             dcc_buff_size,
             NULL
          );
 
         vxMapUserDataObject(
             dcc_param_2a,
             0,
             dcc_buff_size,
             &dcc_2a_buf_map_id,
             (void **)&dcc_2a_buf,
             VX_WRITE_ONLY,
             VX_MEMORY_TYPE_HOST,
             0
         );
 
         memset(dcc_2a_buf, 0xAB, dcc_buff_size);
 
         read_dcc_file(
             dcc_viss_file_path,
             dcc_2a_buf,
             dcc_buff_size
         );
 
         vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);
 
         ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                  aewb_config,
                                  NULL,
                                  h3a_aew_af,
                                  ae_aewb_delay_1,
                                  (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                  dcc_param_2a), VX_TYPE_NODE);
         VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
         VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));
 
         VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));
 
         VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));
 
         vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];
 
         int graph_parameter_num = 0;
         add_graph_parameter_by_node_index(graph, vissNode, 3);
 
         graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
         graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
         graph_parameters_queue_params_list[graph_parameter_num].refs_list =
             (vx_reference*)&raw[0];
         graph_parameter_num++;
 
         VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));
 
         /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
          * Graph gets scheduled automatically as refs are enqueued to it
          */
         vxSetGraphScheduleConfig(graph,
                 VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                 1,
                 graph_parameters_queue_params_list
                 );
 
         VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));
 
         ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
 
         export_graph_to_file(graph, "viss_aewb_single_channel");
 
         /* Enqueue buf for pipe up but don't trigger graph execution */
         for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
         {
             graph_parameter_num = 0;
             vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                 (vx_reference*)&raw[buf_id], 1);
             graph_parameter_num++;
         }
 
         /* Need to trigger again since display holds on to a buffer */
         graph_parameter_num = 0;
         vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
             (vx_reference*)&raw[NUM_BUFS-1], 1);
         graph_parameter_num++;
 
         for(loop_id=0; loop_id<4; loop_id++)
         {
             tivx_raw_image raw;
 
             graph_parameter_num = 0;
 
             /* Get output reference, waits until a frame is available */
             vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                 (vx_reference*)&raw, 1, &num_refs);
             graph_parameter_num++;
 
             graph_parameter_num = 0;
 
             vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
             graph_parameter_num++;
         }
 
         vxWaitGraph(graph);
 
         VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
         VX_CALL(vxReleaseImage(&viss_nv12_out_img));
         VX_CALL(vxReleaseUserDataObject(&aewb_config));
         VX_CALL(vxReleaseUserDataObject(&configuration));
         VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
         VX_CALL(vxReleaseDelay(&delay_2a_res));
         VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
         for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
         {
             VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
         }
         VX_CALL(vxReleaseNode(&vissNode));
         VX_CALL(vxReleaseNode(&node_aewb));
         VX_CALL(vxReleaseGraph(&graph));
 
         ASSERT(vissNode == 0);
         ASSERT(graph == 0);
         ASSERT(h3a_aew_af == 0);
         ASSERT(viss_nv12_out_img == 0);
         ASSERT(configuration == 0);
 
         tivxHwaUnLoadKernels(context);
         tivxImagingUnLoadKernels(context);
     }
 }
/* Test case to cover tivxAewbControl function when node_cmd_id = ISS_CMD_SET_DCC_PARAMS*/
 TEST(tivxHwaVpacAewb, testControl)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;
    vx_reference ref[1];

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        ref[0] = (vx_reference)dcc_param_2a;
        tivxNodeSendCommand(node_aewb, 0, ISS_CMD_SET_DCC_PARAMS, ref, 1);

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}
/* Test case to cover nodePrms->p_ae_params->mode = ALGORITHMS_ISS_AE_AUTO in tivxAewbProcess function*/
TEST(tivxHwaVpacAewb, testNodePrmsModeAE_AUTO)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_AUTO;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover nodePrms->p_ae_params->mode = ALGORITHMS_ISS_AE_MANUAL in tivxAewbProcess function*/
TEST(tivxHwaVpacAewb, testNodePrmsModeAE_MANUAL)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_MANUAL;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover nodePrms->p_ae_params->mode = ALGORITHMS_ISS_AWB_MANUAL in tivxAewbProcess function*/
TEST(tivxHwaVpacAewb, testNodePrmsModeAWB_MANUAL)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AWB_MANUAL;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover if(1 == ae_awb_result_target_ptr->ae_valid) and if(1 == ae_awb_result_target_ptr->awb_valid)*/
TEST(tivxHwaVpacAewb, testae_awb_params)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));

        ae_awb_params.ae_valid = 1;
        ae_awb_params.awb_valid = 1;


        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover if(1 == ae_awb_result_target_ptr->ae_valid) and if(1 == ae_awb_result_target_ptr->awb_valid) and ae_mode is ALGORITHMS_ISS_AE_MANUAL*/
TEST(tivxHwaVpacAewb, testae_awb_params_ae_manual)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));

        ae_awb_params.ae_valid = 1;
        ae_awb_params.awb_valid = 1;


        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_MANUAL;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover tivxAewbControl function when node_cmd_id = ISS_CMD_SET_DCC_PARAMS+1 */
TEST(tivxHwaVpacAewb, testNegativeControl)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;
    vx_reference ref[1];

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        ref[0] = (vx_reference)dcc_param_2a;
        tivxNodeSendCommand(node_aewb, 0, ISS_CMD_SET_DCC_PARAMS+1, ref, 1);

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

/* Test case to cover nodePrms->p_ae_params->mode = ALGORITHMS_ISS_AWB_AUTO in tivxAewbProcess function*/
TEST(tivxHwaVpacAewb, testNodePrmsModeAWB_AUTO)
{
    vx_context context = context_->vx_context_;
    uint32_t buf_id, loop_id, num_refs;

    /* VISS Input */
    tivx_raw_image raw[MAX_NUM_BUF];
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result;
    vx_delay delay_2a_res;


    /* VISS Output */
    vx_image viss_nv12_out_img;
    vx_user_data_object h3a_aew_af = NULL;

    /* AEWB Objects */
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;

    vx_user_data_object ae_aewb_delay_1;

    vx_size dcc_buff_size;
    char dcc_viss_file_path[APP_MAX_FILE_PATH];
    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    tivx_aewb_config_t aewb_cfg;
    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node vissNode = 0, node_aewb = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1920;
    raw_params.height = 1080;
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



    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);

        tivxImagingLoadKernels(context);

        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            ASSERT_VX_OBJECT(raw[buf_id] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        }

        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw[0], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(viss_nv12_out_img =
            vxCreateImage(context, width, height,
            VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.sensor_dcc_id = 390;
        params.use_case = 0;
        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = 4;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.bypass_nsf4 = 1;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result =
            vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        delay_2a_res = vxCreateDelay(context, (vx_reference)(ae_awb_result), 2);

        ASSERT_VX_OBJECT(ae_aewb_delay_1 =
            (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, -1), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_aewb_delay_1, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(vissNode = tivxVpacVissNode(graph, configuration,
                                                ae_aewb_delay_1, NULL,
                                                raw[0], NULL, NULL, viss_nv12_out_img, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);
        tivxSetNodeParameterNumBufByIndex(vissNode, 6u, NUM_BUFS);
        tivxSetNodeParameterNumBufByIndex(vissNode, 9u, NUM_BUFS);

        vxSetReferenceName((vx_reference)vissNode, "VISS_Processing");
        vxSetNodeTarget(vissNode, VX_TARGET_STRING,
            TIVX_TARGET_VPAC_VISS1);

        aewb_cfg.sensor_dcc_id = 390;
        aewb_cfg.sensor_img_format = 0;
        aewb_cfg.sensor_img_phase = 3;
        aewb_cfg.ae_mode = ALGORITHMS_ISS_AWB_AUTO;
        aewb_cfg.awb_num_skip_frames = 9;
        aewb_cfg.ae_num_skip_frames = 9;
        aewb_cfg.channel_id = 0;

        ASSERT_VX_OBJECT(aewb_config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewb_cfg), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        snprintf(dcc_viss_file_path, APP_MAX_FILE_PATH, "%s/%s", ct_get_test_file_path(), "psdkra/app_single_cam/IMX390_001/dcc_2a.bin");
        dcc_buff_size = get_dcc_file_size(dcc_viss_file_path);

        dcc_param_2a = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
         );

        vxMapUserDataObject(
            dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        memset(dcc_2a_buf, 0xAB, dcc_buff_size);

        read_dcc_file(
            dcc_viss_file_path,
            dcc_2a_buf,
            dcc_buff_size
        );

        vxUnmapUserDataObject(dcc_param_2a, dcc_2a_buf_map_id);

        ASSERT_VX_OBJECT(node_aewb = tivxAewbNode(graph,
                                 aewb_config,
                                 NULL,
                                 h3a_aew_af,
                                 ae_aewb_delay_1,
                                 (vx_user_data_object)vxGetReferenceFromDelay(delay_2a_res, 0),
                                 dcc_param_2a), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU));
        VX_CALL(vxSetReferenceName((vx_reference)node_aewb, "2A_AlgNode"));

        VX_CALL(vxSetReferenceName((vx_reference)delay_2a_res, "delay_object"));

        VX_CALL(tivxSetNodeParameterNumBufByIndex(node_aewb, 4u, NUM_BUFS));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[3];

        int graph_parameter_num = 0;
        add_graph_parameter_by_node_index(graph, vissNode, 3);

        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&raw[0];
        graph_parameter_num++;

        VX_CALL(tivxSetGraphPipelineDepth(graph, NUM_BUFS));

        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        vxSetGraphScheduleConfig(graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                1,
                graph_parameters_queue_params_list
                );

        VX_CALL(vxRegisterAutoAging(graph, delay_2a_res));

        VX_CALL(vxVerifyGraph(graph));

        export_graph_to_file(graph, "viss_aewb_single_channel");

        /* Enqueue buf for pipe up but don't trigger graph execution */
        for(buf_id=0; buf_id<NUM_BUFS-1; buf_id++)
        {
            graph_parameter_num = 0;
            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
                (vx_reference*)&raw[buf_id], 1);
            graph_parameter_num++;
        }

        /* Need to trigger again since display holds on to a buffer */
        graph_parameter_num = 0;
        vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num,
            (vx_reference*)&raw[NUM_BUFS-1], 1);
        graph_parameter_num++;

        for(loop_id=0; loop_id<4; loop_id++)
        {
            tivx_raw_image raw;

            graph_parameter_num = 0;

            /* Get output reference, waits until a frame is available */
            vxGraphParameterDequeueDoneRef(graph, graph_parameter_num,
                (vx_reference*)&raw, 1, &num_refs);
            graph_parameter_num++;

            graph_parameter_num = 0;

            vxGraphParameterEnqueueReadyRef(graph, graph_parameter_num, (vx_reference*)&raw, 1);
            graph_parameter_num++;
        }

        vxWaitGraph(graph);

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseImage(&viss_nv12_out_img));
        VX_CALL(vxReleaseUserDataObject(&aewb_config));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_2a));
        VX_CALL(vxReleaseDelay(&delay_2a_res));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            VX_CALL(tivxReleaseRawImage(&raw[buf_id]));
        }
        VX_CALL(vxReleaseNode(&vissNode));
        VX_CALL(vxReleaseNode(&node_aewb));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(vissNode == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(viss_nv12_out_img == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
        tivxImagingUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVpacAewb,
               testSingleChannel,
               testMultiChannel,
               testMultiChannelNullH3A,
               testNegativeconfiguration_size,
               testNegative_dcc_param,
               testControl,
               testNodePrmsModeAE_AUTO,
               testNodePrmsModeAE_MANUAL,
               testae_awb_params,
               testae_awb_params_ae_manual,
               testNegativeControl,
               testNodePrmsModeAWB_MANUAL,
               testNodePrmsModeAWB_AUTO)
