/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include <utils/draw2d/include/draw2d.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_get.h>
#include <utils/grpx/include/app_grpx.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>
#include <utils/ipc/include/app_ipc.h>
#include <TI/tivx_event.h>

#include "app_common.h"
#include "app_sensor_module.h"
#include "app_capture_module.h"
#include "app_viss_module.h"
#include "app_aewb_module.h"
#include "app_ldc_module.h"
#include "app_scaler_module.h"
#include "app_pre_proc_module_queued.h"
#include "app_tidl_module.h"
#include "app_dd_module_queued.h"
#include "app_img_mosaic_module.h"
#include "app_display_module.h"

#define APP_BUFFER_Q_DEPTH         (4)
#define APP_PIPELINE_DEPTH         (7)
#define APP_MAIN_GRAPH_NUM_PARAMS  (8)

/* Enable Reciprocal Comparison monitoring for the R5F (AEWB) node in normal operation. */
#define APP_TEST_OPT_R5F_RC        (1 << 1)

/* Enable Reciprocal Comparison monitoring for the A72 (PreProc) node in normal operation. */
#define APP_TEST_OPT_A72_RC        (1 << 2)

/* Enable Reciprocal Comparison monitoring for the C7X (PostProc) node in normal operation. */
#define APP_TEST_OPT_C7X_RC        (1 << 3)

/* Enable Program Sequence Monitoring for the R5F (AEWB) node to verify timing and execution order under normal conditions. */
#define APP_TEST_OPT_R5F_PSM       (1 << 4)

/* Enable Program Sequence Monitoring for the A72 (PreProc) node to verify timing and execution order under normal conditions. */
#define APP_TEST_OPT_A72_PSM       (1 << 5)

/* Enable Program Sequence Monitoring for the C7X (PostProc) node to verify timing and execution order under normal conditions. */
#define APP_TEST_OPT_C7X_PSM       (1 << 6)

/* Enable all safety mechanisms (RC and PSM) for all three cores (default configuration). */
#define APP_TEST_OPT_DEFAULT       (0x7F)

/* Enable safety mechanisms (Reciprocal Comparison and Program Sequence Monitoring) in the application. */
#define SAFETY_ENABLED

#ifdef SAFETY_ENABLED
    #define SAFETY_ENABLED_RUN(expr) do { expr; } while (0)
    #define APP_NAME_SUFFIX "Safety enabled"
#else
    #define SAFETY_ENABLED_RUN(expr) do { } while (0)
    #define APP_NAME_SUFFIX "Safety disabled"
#endif

/* Number of frames to skip between runtime verification checks (default: every 5 frames). */
#define DEFAULT_CHECK_INTERVAL     (5U)

/* Maximum allowed execution time (nano seconds) for the AEWB node based on vx_perf_t.tmp measurement. */
#define DEFAULT_AEWB_MAX_TIME      (7000000U)

/* Maximum allowed execution time (nano seconds) for the pre-processing node based on vx_perf_t.tmp measurement. */
#define DEFAULT_PREPROC_MAX_TIME   (8000000U)

/* Maximum allowed execution time (nano seconds) for the post-processing node based on vx_perf_t.tmp measurement. */
#define DEFAULT_POSTPROC_MAX_TIME  (4000000U)

typedef enum
{
    MSC_OUT_MAIN_PREPROC = 0,
    MSC_OUT_VERIFIER_PREPROC,
    MSC_OUT_MAX
} ScalerOutput;

typedef struct {

    SensorObj         sensorObj;
    CaptureObj        captureObj;
    VISSObj           vissObj;
    AEWBObj           aewbObj;
    LDCObj            ldcObj;
    ScalerObj         scalerObj;
    PreProcObj        preProcObj;
    TIDLObj           tidlObj;
    DrawDetectionsObj drawDetObj;
    ImgMosaicObj      imgMosaicObj;
    DisplayObj        displayObj;

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    vx_uint32 is_interactive;
    vx_uint32 cpu_core_id;

    vx_uint32 num_frames_to_run;

    vx_uint32 num_frames_to_write;
    vx_uint32 num_frames_to_skip;

    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t pipeline;

    int32_t enqueueCnt;
    int32_t dequeueCnt;

#ifdef SAFETY_ENABLED

    AEWBObj           aewbObjVerifier;
    PreProcObj        preProcObjVerifier;
    DrawDetectionsObj drawDetObjVerifier;

    vx_graph   graphAewbVerifier;
    vx_graph   graphPreProcVerifier;
    vx_graph   graphDrawDetVerifier;

    tivx_task  monitoring_task;
    tivx_task  aewb_verifier_task;
    tivx_task  preproc_verifier_task;
    tivx_task  drawdet_verifier_task;
    tivx_mutex aewb_mutex;
    tivx_mutex preproc_mutex;
    tivx_mutex postproc_mutex;
    tivx_event evt_aewb_v_run;
    tivx_event evt_preproc_v_run;
    tivx_event evt_drawdet_v_run;

    vx_uint32  aewb_verifier_in_idx;
    vx_uint32  aewb_verifier_graph_ready;
    vx_uint32  preproc_verifier_in_idx;
    vx_uint32  preproc_verifier_graph_ready;
    vx_uint32  drawdet_verifier_in_idx;
    vx_uint32  drawdet_verifier_graph_ready;

    vx_image   dd_in_img;
    vx_object_array aewb_verifier_h3a_stats[APP_BUFFER_Q_DEPTH];

    vx_user_data_object aewb_out_copy;
    vx_tensor preproc_out_copy;
    vx_image drawdet_out_copy;

    vx_uint64 aewb_max_time;
    vx_uint64 preproc_max_time;
    vx_uint64 postproc_max_time;
    vx_uint32 check_interval;
    vx_uint32 test_option;
#endif
} AppObj;

AppObj gAppObj;

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[]);
static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static vx_status app_run_graph_interactive(AppObj *obj);
static void app_delete_graph(AppObj *obj);
static void app_default_param_set(AppObj *obj);
static void app_update_param_set(AppObj *obj);
static void update_draw_detections_defaults(AppObj *obj, DrawDetectionsObj *drawDetObj);
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_pipeline_params_defaults(AppObj *obj);
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_uint32 frame_id);
static vx_status app_pipeup_graphs(AppObj *obj);

#ifdef SAFETY_ENABLED
static void app_print_test_options(uint32_t test_option);
static vx_status app_dequeue_viss_aewb_from_main_graph(AppObj *obj, vx_user_data_object *h3a_stats, vx_user_data_object *aewb_results);
static vx_status app_reenqueue_viss_aewb_to_main_graph(AppObj *obj, vx_user_data_object h3a_stats, vx_user_data_object aewb_results);
static vx_status app_dequeue_scaler_preproc_from_main_graph(AppObj *obj, vx_image *scaler_output, vx_tensor *preproc_output);
static vx_status app_enqueue_scaler_preproc_to_main_graph(AppObj *obj, vx_image *scaler_output, vx_tensor preproc_output);
static vx_status app_dequeue_tidl_drawdet_from_main_graph(AppObj *obj, vx_tensor *tidl_output, vx_image *draw_det_output);
static vx_status app_enqueue_tidl_drawdet_to_main_graph(AppObj *obj, vx_tensor *tidl_output, vx_image draw_det_output);
static vx_status app_init_aewb_verifier_input(AppObj *obj);
static vx_status app_copy_h3a_data(vx_user_data_object src, vx_user_data_object dst);
static vx_status app_copy_aewb_outputs(vx_user_data_object src_aewb_results, vx_user_data_object dst_aewb_results);
static vx_status app_copy_tensor_u8(vx_tensor src, vx_tensor dst);
static vx_status app_verify_nv12_pair(vx_image src, vx_image dst, vx_uint32 *width, vx_uint32 *height);
static vx_status app_copy_image_nv12(vx_image src, vx_image dst);
static vx_status app_compare_aewb_outputs(vx_user_data_object aewb_results, vx_user_data_object aewb_verifier_results);
static vx_status app_compare_tensors_u8(vx_tensor tensor_a, vx_tensor tensor_b);
static vx_status app_compare_images_nv12(vx_image image_a, vx_image image_b);
static void app_verify_aewb_outputs(AppObj *obj);
static void app_verify_preproc_outputs(AppObj *obj);
static void app_verify_drawdet_outputs(AppObj *obj);
static vx_status app_verify_program_sequence(AppObj *obj);
static vx_status app_schedule_aewb_verifier(AppObj *obj, vx_user_data_object h3a_stats, vx_user_data_object aewb_results, vx_uint32 frame_id);
static vx_status app_schedule_preproc_verifier(AppObj *obj, vx_image scaler_img, vx_tensor preproc_main, vx_uint32 frame_id);
static vx_status app_schedule_drawdet_verifier(AppObj *obj, vx_tensor tidl_out_tensor, vx_image scaler_img, vx_image drawdet_main_out, vx_uint32 frame_id);
static vx_status app_pipeup_inputs_aewb_verifier_graph(AppObj *obj, vx_user_data_object h3a_stats);
static vx_status app_pipeup_inputs_preproc_verifier_graph(AppObj *obj, vx_image scaler_img);
static vx_status app_pipeup_inputs_drawdet_verifier_graph(AppObj *obj, vx_tensor tidl_out_tensor, vx_image scaler_img);
static vx_tensor app_create_tensor_from_exemplar(vx_context context, vx_tensor exemplar);
static vx_status register_events(AppObj *obj);
static void app_monitoring_task(void *app_var);
static void app_run_aewb_verifier_task(void *app_var);
static void app_run_preproc_verifier_task(void *app_var);
static void app_run_drawdet_verifier_task(void *app_var);
static vx_status app_monitoring_tasks_create(AppObj *obj);
static void app_monitoring_tasks_delete(AppObj *obj);
static vx_status app_delete_graph_safety(AppObj *obj);
static void app_parse_cfg_file_safety(AppObj *obj, vx_char *token, vx_char *s);
static vx_status app_init_safety(AppObj *obj);
static vx_status app_deinit_safety(AppObj *obj);
static vx_status app_create_verifier_graphs(AppObj *obj);
static vx_status app_verify_verifier_graphs(AppObj *obj);
static vx_status app_wait_verifier_graphs(AppObj *obj, vx_status status);
static void app_update_param_set_safety(AppObj *obj);

static void app_print_test_options(uint32_t test_option)
{
    const char *en = "Enabled";
    const char *dis = "Disabled";

    printf("\nR5F - RC: %s PSM: %s\n",
           ((test_option & APP_TEST_OPT_R5F_RC)  != 0u) ? en : dis,
           ((test_option & APP_TEST_OPT_R5F_PSM) != 0u) ? en : dis);

    printf("A72 - RC: %s PSM: %s\n",
           ((test_option & APP_TEST_OPT_A72_RC)  != 0u) ? en : dis,
           ((test_option & APP_TEST_OPT_A72_PSM) != 0u) ? en : dis);

    printf("C7X - RC: %s PSM: %s\n",
           ((test_option & APP_TEST_OPT_C7X_RC)  != 0u) ? en : dis,
           ((test_option & APP_TEST_OPT_C7X_PSM) != 0u) ? en : dis);
}

static vx_status app_init_aewb_verifier_input(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q = 0;

    if (obj == NULL)
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_user_data_object h3a_stats = vxCreateUserDataObject(obj->context,
                                                               "tivx_h3a_data_t",
                                                               sizeof(tivx_h3a_data_t),
                                                               NULL);
        status = vxGetStatus((vx_reference)h3a_stats);

        if (status == VX_SUCCESS)
        {
            char name[128];

            for (q = 0; q < (vx_int32)APP_BUFFER_Q_DEPTH; q++)
            {
                obj->aewb_verifier_h3a_stats[q] =
                    vxCreateObjectArray(obj->context,
                                        (vx_reference)h3a_stats,
                                        1); /* num cameras enabled */

                status = vxGetStatus((vx_reference)obj->aewb_verifier_h3a_stats[q]);
                if (status == VX_SUCCESS)
                {
                    (void)snprintf(name, sizeof(name), "viss_h3a_stats_arr_q%d", (int)q);
                    (void)vxSetReferenceName((vx_reference)obj->aewb_verifier_h3a_stats[q], name);
                }
                else
                {
                    APP_PRINT("Unable to create h3a stats object array!\n");
                }

                if (status != VX_SUCCESS)
                {
                    break;
                }
            }
        }
        else
        {
            APP_PRINT("Unable to create h3a stats exemplar!\n");
        }

        if (h3a_stats != NULL)
        {
            (void)vxReleaseUserDataObject(&h3a_stats);
        }
    }

    return status;
}

static vx_status app_copy_h3a_data(vx_user_data_object src, vx_user_data_object dst)
{
    vx_status status = VX_SUCCESS;
    vx_status unmap_status = VX_SUCCESS;
    vx_map_id dst_map_id = 0;
    tivx_h3a_data_t *pdst = NULL;
    vx_bool is_mapped = vx_false_e;


    if (src == NULL || dst == NULL)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    status = vxMapUserDataObject(dst,
                                 0,
                                 sizeof(tivx_h3a_data_t),
                                 &dst_map_id,
                                 (void **)&pdst,
                                 VX_READ_AND_WRITE,
                                 VX_MEMORY_TYPE_HOST,
                                 0);
    if (status == VX_SUCCESS)
    {
        is_mapped = vx_true_e;

        status = vxCopyUserDataObject(src,
                                      0,
                                      sizeof(tivx_h3a_data_t),
                                      pdst,
                                      VX_READ_ONLY,
                                      VX_MEMORY_TYPE_HOST);

        if (status == VX_SUCCESS)
        {
            /* Avoid sending results to VISS from verifier AEWB node by setting invalid cpu_id */
            pdst->cpu_id = APP_IPC_CPU_INVALID;
        }
    }

    if (is_mapped == vx_true_e)
    {
        unmap_status = vxUnmapUserDataObject(dst, dst_map_id);

        if (status == VX_SUCCESS)
        {
            status = unmap_status;
        }
    }

    return status;
}

static vx_status app_copy_aewb_outputs(vx_user_data_object src_aewb_results, vx_user_data_object dst_aewb_results)
{
    vx_status status = VX_SUCCESS;

    void *map_ptr_src = NULL;
    vx_map_id map_id_src = 0U;

    void *map_ptr_dst = NULL;
    vx_map_id map_id_dst = 0U;

    status = vxMapUserDataObject(src_aewb_results,
                                 0,
                                 sizeof(tivx_ae_awb_params_t),
                                 &map_id_src,
                                 &map_ptr_src,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 0);

    if (status == VX_SUCCESS)
    {
        status = vxMapUserDataObject(dst_aewb_results,
                                     0,
                                     sizeof(tivx_ae_awb_params_t),
                                     &map_id_dst,
                                     &map_ptr_dst,
                                     VX_WRITE_ONLY,
                                     VX_MEMORY_TYPE_HOST,
                                     0);
    }

    if ((status == VX_SUCCESS) &&
        (map_ptr_src != NULL) &&
        (map_ptr_dst != NULL))
    {
        (void)memcpy(map_ptr_dst,
                     map_ptr_src,
                     sizeof(tivx_ae_awb_params_t));
    }
    else if (status == VX_SUCCESS)
    {
        status = VX_FAILURE;
    }

    if (map_ptr_dst != NULL)
    {
        (void)vxUnmapUserDataObject(dst_aewb_results, map_id_dst);
    }

    if (map_ptr_src != NULL)
    {
        (void)vxUnmapUserDataObject(src_aewb_results, map_id_src);
    }

    return status;
}

static vx_status app_copy_tensor_u8(vx_tensor src, vx_tensor dst)
{
    vx_status status = VX_SUCCESS;

    vx_size nd = 0U;
    vx_size dims[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size start[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size end[APP_MAX_TENSOR_DIMS] = {0U};

    vx_map_id map_id_src = 0U;
    vx_map_id map_id_dst = 0U;

    vx_size stride_src[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size stride_dst[APP_MAX_TENSOR_DIMS] = {0U};

    void *ptr_src = NULL;
    void *ptr_dst = NULL;

    vx_size bytes = 1U;

    status = vxQueryTensor(src, VX_TENSOR_NUMBER_OF_DIMS, &nd, sizeof(nd));

    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(src, VX_TENSOR_DIMS, dims, nd * sizeof(vx_size));
    }

    if (status == VX_SUCCESS)
    {
        for (vx_size d = 0U; d < nd; d++)
        {
            start[d] = 0U;
            end[d] = dims[d];
            bytes *= dims[d];
        }
    }

    if (status == VX_SUCCESS)
    {
        status = tivxMapTensorPatch(src,
                                   nd,
                                   start,
                                   end,
                                   &map_id_src,
                                   stride_src,
                                   &ptr_src,
                                   VX_READ_ONLY,
                                   VX_MEMORY_TYPE_HOST);
    }

    if (status == VX_SUCCESS)
    {
        status = tivxMapTensorPatch(dst,
                                   nd,
                                   start,
                                   end,
                                   &map_id_dst,
                                   stride_dst,
                                   &ptr_dst,
                                   VX_WRITE_ONLY,
                                   VX_MEMORY_TYPE_HOST);
    }

    if (status == VX_SUCCESS)
    {
        (void)memcpy(ptr_dst, ptr_src, bytes);
    }

    if (ptr_dst != NULL)
    {
        (void)tivxUnmapTensorPatch(dst, map_id_dst);
    }

    if (ptr_src != NULL)
    {
        (void)tivxUnmapTensorPatch(src, map_id_src);
    }

    return status;
}

static vx_status app_verify_nv12_pair(vx_image src, vx_image dst, vx_uint32 *width, vx_uint32 *height)
{
    vx_status status = VX_SUCCESS;
    vx_df_image fmt_s = 0;
    vx_df_image fmt_d = 0;
    vx_uint32 w_s = 0;
    vx_uint32 h_s = 0;
    vx_uint32 w_d = 0;
    vx_uint32 h_d = 0;

    if ((src == NULL) || (dst == NULL) || (width == NULL) || (height == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(src, VX_IMAGE_FORMAT, &fmt_s, sizeof(fmt_s));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(dst, VX_IMAGE_FORMAT, &fmt_d, sizeof(fmt_d));
    }
    if ((status == VX_SUCCESS) && ((fmt_s != VX_DF_IMAGE_NV12) || (fmt_d != VX_DF_IMAGE_NV12)))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(src, VX_IMAGE_WIDTH, &w_s, sizeof(w_s));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(src, VX_IMAGE_HEIGHT, &h_s, sizeof(h_s));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(dst, VX_IMAGE_WIDTH, &w_d, sizeof(w_d));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(dst, VX_IMAGE_HEIGHT, &h_d, sizeof(h_d));
    }
    if ((status == VX_SUCCESS) && ((w_s != w_d) || (h_s != h_d)))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        *width = w_s;
        *height = h_s;
    }

    return status;
}

static vx_status app_copy_image_nv12(vx_image src, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0;
    vx_uint32 height = 0;

    status = app_verify_nv12_pair(src, dst, &width, &height);

    for (vx_uint32 plane = 0; (status == VX_SUCCESS) && (plane < 2U); plane++)
    {
        vx_rectangle_t rect;
        vx_map_id map_id_src = 0;
        vx_map_id map_id_dst = 0;
        vx_imagepatch_addressing_t addr_src;
        vx_imagepatch_addressing_t addr_dst;
        void *ptr_src = NULL;
        void *ptr_dst = NULL;
        vx_bool mapped_src = vx_false_e;
        vx_bool mapped_dst = vx_false_e;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = (plane == 0U) ? width : (width / 2U);
        rect.end_y = (plane == 0U) ? height : (height / 2U);

        status = vxMapImagePatch(src, &rect, plane, &map_id_src,
                                 &addr_src, &ptr_src,
                                 VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
        if (status == VX_SUCCESS)
        {
            mapped_src = vx_true_e;
        }

        if (status == VX_SUCCESS)
        {
            status = vxMapImagePatch(dst, &rect, plane, &map_id_dst,
                                     &addr_dst, &ptr_dst,
                                     VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);
            if (status == VX_SUCCESS)
            {
                mapped_dst = vx_true_e;
            }
        }

        if (status == VX_SUCCESS)
        {
            vx_uint32 rows = (vx_uint32)(rect.end_y - rect.start_y);
            vx_size width_px = (vx_size)(rect.end_x - rect.start_x);
            vx_size row_bytes_src = width_px * (vx_size)addr_src.stride_x;
            vx_size row_bytes_dst = width_px * (vx_size)addr_dst.stride_x;

            if (row_bytes_src != row_bytes_dst)
            {
                status = VX_FAILURE;
            }
            else
            {
                for (vx_uint32 y = 0; y < rows; y++)
                {
                    uint8_t *s = (uint8_t *)ptr_src + (vx_size)y * (vx_size)addr_src.stride_y;
                    uint8_t *d = (uint8_t *)ptr_dst + (vx_size)y * (vx_size)addr_dst.stride_y;
                    memcpy(d, s, row_bytes_src);
                }
            }
        }

        if (mapped_dst == vx_true_e)
        {
            (void)vxUnmapImagePatch(dst, map_id_dst);
        }
        if (mapped_src == vx_true_e)
        {
            (void)vxUnmapImagePatch(src, map_id_src);
        }
    }

    return status;
}

static vx_status app_compare_aewb_outputs(vx_user_data_object aewb_results, vx_user_data_object aewb_verifier_results)
{
    vx_status status = VX_SUCCESS;

    void *map_ptr = NULL;
    vx_map_id map_id = 0U;

    void *map_ptr_verifier = NULL;
    vx_map_id map_id_verifier = 0U;

    status = vxMapUserDataObject(aewb_results,
                                 0,
                                 sizeof(tivx_ae_awb_params_t),
                                 &map_id,
                                 &map_ptr,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 0);

    if (status == VX_SUCCESS)
    {
        status = vxMapUserDataObject(aewb_verifier_results,
                                     0,
                                     sizeof(tivx_ae_awb_params_t),
                                     &map_id_verifier,
                                     &map_ptr_verifier,
                                     VX_READ_ONLY,
                                     VX_MEMORY_TYPE_HOST,
                                     0);
    }

    if ((status == VX_SUCCESS) &&
        (map_ptr != NULL) &&
        (map_ptr_verifier != NULL))
    {
        if (memcmp(map_ptr,
                   map_ptr_verifier,
                   sizeof(tivx_ae_awb_params_t)) != 0)
        {
            status = VX_FAILURE;
        }
    }
    else if (status == VX_SUCCESS)
    {
        status = VX_FAILURE;
    }

    if (map_ptr_verifier != NULL)
    {
        (void)vxUnmapUserDataObject(aewb_verifier_results, map_id_verifier);
    }

    if (map_ptr != NULL)
    {
        (void)vxUnmapUserDataObject(aewb_results, map_id);
    }

    return status;
}

static vx_status app_compare_tensors_u8(vx_tensor tensor_a, vx_tensor tensor_b)
{
    vx_status status = VX_SUCCESS;

    vx_size nd_main = 0U;
    vx_size nd_ver  = 0U;

    vx_size dims_main[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size dims_ver[APP_MAX_TENSOR_DIMS]  = {0U};

    vx_size start[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size end[APP_MAX_TENSOR_DIMS]   = {0U};

    vx_map_id map_id_main = 0U;
    vx_map_id map_id_ver  = 0U;

    vx_size stride_main[APP_MAX_TENSOR_DIMS] = {0U};
    vx_size stride_ver[APP_MAX_TENSOR_DIMS]  = {0U};

    void *ptr_main = NULL;
    void *ptr_ver  = NULL;

    vx_bool mapped_main = vx_false_e;
    vx_bool mapped_ver  = vx_false_e;

    vx_size total_elems = 0U;
    vx_size bytes       = 0U;

    if ((tensor_a == NULL) || (tensor_b == NULL))
    {
        printf("NULL tensor(s)\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(tensor_a, VX_TENSOR_NUMBER_OF_DIMS, &nd_main, sizeof(nd_main));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(tensor_b, VX_TENSOR_NUMBER_OF_DIMS, &nd_ver, sizeof(nd_ver));
    }

    if ((status == VX_SUCCESS) &&
        ((nd_main == 0U) || (nd_main > (vx_size)APP_MAX_TENSOR_DIMS) || (nd_main != nd_ver)))
    {
        printf("dim mismatch nd_main=%u nd_ver=%u\n",
               (unsigned)nd_main, (unsigned)nd_ver);
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(tensor_a, VX_TENSOR_DIMS, dims_main, nd_main * sizeof(vx_size));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(tensor_b, VX_TENSOR_DIMS, dims_ver, nd_ver * sizeof(vx_size));
    }

    if (status == VX_SUCCESS)
    {
        for (vx_size d = 0U; d < nd_main; d++)
        {
            if (dims_main[d] != dims_ver[d])
            {
                printf("dims mismatch at d=%u main=%u verifier=%u\n",
                       (unsigned)d, (unsigned)dims_main[d], (unsigned)dims_ver[d]);
                status = VX_FAILURE;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        total_elems = 1U;
        for (vx_size d = 0U; d < nd_main; d++)
        {
            if (dims_main[d] == 0U)
            {
                total_elems = 0U;
            }
            else if (total_elems != 0U)
            {
                total_elems *= dims_main[d];
            }
        }

        bytes = total_elems;

        for (vx_size d = 0U; d < nd_main; d++)
        {
            start[d] = 0U;
            end[d] = dims_main[d];
        }
    }

    if (status == VX_SUCCESS)
    {
        status = tivxMapTensorPatch(tensor_a,
                                   nd_main,
                                   start,
                                   end,
                                   &map_id_main,
                                   stride_main,
                                   &ptr_main,
                                   VX_READ_ONLY,
                                   VX_MEMORY_TYPE_HOST);

        if (status == VX_SUCCESS)
        {
            mapped_main = vx_true_e;
        }
        else
        {
            printf("tivxMapTensorPatch(main) failed (%d)\n",
                   (int)status);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = tivxMapTensorPatch(tensor_b,
                                   nd_main,
                                   start,
                                   end,
                                   &map_id_ver,
                                   stride_ver,
                                   &ptr_ver,
                                   VX_READ_ONLY,
                                   VX_MEMORY_TYPE_HOST);

        if (status == VX_SUCCESS)
        {
            mapped_ver = vx_true_e;
        }
        else
        {
            printf("тivxMapTensorPatch(verifier) failed (%d)\n",
                   (int)status);
        }
    }

    if ((status == VX_SUCCESS) && (ptr_main != NULL) && (ptr_ver != NULL))
    {
        if (memcmp(ptr_main, ptr_ver, bytes) != 0)
        {
            printf("FAIL (buffers differ)\n");
            status = VX_FAILURE;
        }
    }
    else if (status == VX_SUCCESS)
    {
        printf("NULL mapped ptr(s)\n");
        status = VX_FAILURE;
    }

    if (mapped_ver == vx_true_e)
    {
        (void)tivxUnmapTensorPatch(tensor_b, map_id_ver);
    }
    if (mapped_main == vx_true_e)
    {
        (void)tivxUnmapTensorPatch(tensor_a, map_id_main);
    }

    return status;
}

static vx_status app_compare_images_nv12(vx_image image_a, vx_image image_b)
{
    vx_status status = VX_SUCCESS;

    vx_df_image format_a = 0;
    vx_df_image format_b = 0;

    vx_uint32 width_a = 0U;
    vx_uint32 height_a = 0U;

    vx_uint32 width_b = 0U;
    vx_uint32 height_b = 0U;

    vx_rectangle_t rect = {0};
    vx_map_id map_id_a_y = 0U;
    vx_map_id map_id_a_uv = 0U;
    vx_map_id map_id_b_y = 0U;
    vx_map_id map_id_b_uv = 0U;

    vx_imagepatch_addressing_t addr_a_y;
    vx_imagepatch_addressing_t addr_a_uv;
    vx_imagepatch_addressing_t addr_b_y;
    vx_imagepatch_addressing_t addr_b_uv;

    void *ptr_a_y = NULL;
    void *ptr_a_uv = NULL;
    void *ptr_b_y = NULL;
    void *ptr_b_uv = NULL;

    vx_bool mapped_a_y = vx_false_e;
    vx_bool mapped_a_uv = vx_false_e;
    vx_bool mapped_b_y = vx_false_e;
    vx_bool mapped_b_uv = vx_false_e;

    if ((image_a == NULL) || (image_b == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_a, VX_IMAGE_FORMAT, &format_a, sizeof(format_a));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_b, VX_IMAGE_FORMAT, &format_b, sizeof(format_b));
    }

    if ((status == VX_SUCCESS) && ((format_a != VX_DF_IMAGE_NV12) || (format_b != VX_DF_IMAGE_NV12)))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_a, VX_IMAGE_WIDTH, &width_a, sizeof(width_a));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_a, VX_IMAGE_HEIGHT, &height_a, sizeof(height_a));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_b, VX_IMAGE_WIDTH, &width_b, sizeof(width_b));
    }
    if (status == VX_SUCCESS)
    {
        status = vxQueryImage(image_b, VX_IMAGE_HEIGHT, &height_b, sizeof(height_b));
    }

    if ((status == VX_SUCCESS) && ((width_a != width_b) || (height_a != height_b) || (width_a == 0U) || (height_a == 0U)))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        rect.start_x = 0U;
        rect.start_y = 0U;
        rect.end_x = width_a;
        rect.end_y = height_a;

        status = vxMapImagePatch(image_a,
                                 &rect,
                                 0U,
                                 &map_id_a_y,
                                 &addr_a_y,
                                 &ptr_a_y,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 VX_NOGAP_X);
        if (status == VX_SUCCESS)
        {
            mapped_a_y = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxMapImagePatch(image_a,
                                 &rect,
                                 1U,
                                 &map_id_a_uv,
                                 &addr_a_uv,
                                 &ptr_a_uv,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 VX_NOGAP_X);
        if (status == VX_SUCCESS)
        {
            mapped_a_uv = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxMapImagePatch(image_b,
                                 &rect,
                                 0U,
                                 &map_id_b_y,
                                 &addr_b_y,
                                 &ptr_b_y,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 VX_NOGAP_X);
        if (status == VX_SUCCESS)
        {
            mapped_b_y = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxMapImagePatch(image_b,
                                 &rect,
                                 1U,
                                 &map_id_b_uv,
                                 &addr_b_uv,
                                 &ptr_b_uv,
                                 VX_READ_ONLY,
                                 VX_MEMORY_TYPE_HOST,
                                 VX_NOGAP_X);
        if (status == VX_SUCCESS)
        {
            mapped_b_uv = vx_true_e;
        }
    }

    if ((status == VX_SUCCESS) && (ptr_a_y != NULL) && (ptr_b_y != NULL))
    {
        vx_uint32 y;

        for (y = 0U; y < height_a; y++)
        {
            const vx_uint8 *row_a = (const vx_uint8 *)ptr_a_y + (y * addr_a_y.stride_y);
            const vx_uint8 *row_b = (const vx_uint8 *)ptr_b_y + (y * addr_b_y.stride_y);

            if (memcmp(row_a, row_b, (size_t)width_a) != 0)
            {
                status = VX_FAILURE;
                break;
            }
        }
    }
    else if (status == VX_SUCCESS)
    {
        status = VX_FAILURE;
    }

    if ((status == VX_SUCCESS) && (ptr_a_uv != NULL) && (ptr_b_uv != NULL))
    {
        vx_uint32 y;
        vx_uint32 uv_height = height_a / 2U;
        vx_uint32 uv_width = width_a;

        for (y = 0U; y < uv_height; y++)
        {
            const vx_uint8 *row_a = (const vx_uint8 *)ptr_a_uv + (y * addr_a_uv.stride_y);
            const vx_uint8 *row_b = (const vx_uint8 *)ptr_b_uv + (y * addr_b_uv.stride_y);

            if (memcmp(row_a, row_b, (size_t)uv_width) != 0)
            {
                status = VX_FAILURE;
                break;
            }
        }
    }
    else if (status == VX_SUCCESS)
    {
        status = VX_FAILURE;
    }

    if (mapped_b_uv == vx_true_e)
    {
        (void)vxUnmapImagePatch(image_b, map_id_b_uv);
    }
    if (mapped_b_y == vx_true_e)
    {
        (void)vxUnmapImagePatch(image_b, map_id_b_y);
    }
    if (mapped_a_uv == vx_true_e)
    {
        (void)vxUnmapImagePatch(image_a, map_id_a_uv);
    }
    if (mapped_a_y == vx_true_e)
    {
        (void)vxUnmapImagePatch(image_a, map_id_a_y);
    }

    return status;
}

static void app_verify_aewb_outputs(AppObj *obj)
{
    vx_status status;
    vx_status compare_status = VX_SUCCESS;

    uint32_t num_refs = 0U;
    vx_user_data_object aewb_results_verifier = NULL;
    static uint32_t frame_id = 0U;
    frame_id++;

    status = vxGraphParameterDequeueDoneRef(obj->graphAewbVerifier,
                                            obj->aewbObjVerifier.graph_parameter_out_index,
                                            (vx_reference *)&aewb_results_verifier,
                                            1,
                                            &num_refs);

    if ((status == VX_SUCCESS) && (num_refs == 1U))
    {
        if ((frame_id % obj->check_interval) == 0U)
        {
            tivxMutexLock(obj->aewb_mutex);
            compare_status = app_compare_aewb_outputs(obj->aewb_out_copy, aewb_results_verifier);
            tivxMutexUnlock(obj->aewb_mutex);

            if (compare_status != VX_SUCCESS)
            {
                /* Application-specific error handling can be implemented here */
                printf("ERROR: AEWB node comparison failed.\n");
            }
        }

        (void)vxGraphParameterEnqueueReadyRef(obj->graphAewbVerifier,
                                              obj->aewbObjVerifier.graph_parameter_out_index,
                                              (vx_reference *)&aewb_results_verifier,
                                              1);
    }
}

static void app_verify_preproc_outputs(AppObj *obj)
{
    vx_status status;
    vx_status compare_status;

    uint32_t num_refs = 0U;
    vx_tensor preproc_out_verifier = NULL;

    status = vxGraphParameterDequeueDoneRef(obj->graphPreProcVerifier,
                                            obj->preProcObjVerifier.graph_parameter_out_index,
                                            (vx_reference *)&preproc_out_verifier,
                                            1,
                                            &num_refs);

    if ((status == VX_SUCCESS) && (num_refs == 1U))
    {
        tivxMutexLock(obj->preproc_mutex);
        compare_status = app_compare_tensors_u8(obj->preproc_out_copy,
                                               preproc_out_verifier);
        tivxMutexUnlock(obj->preproc_mutex);

        if (compare_status != VX_SUCCESS)
        {
            /* Application-specific error handling can be implemented here */
            printf("ERROR: PreProc node comparison failed.\n");
        }

        (void)vxGraphParameterEnqueueReadyRef(obj->graphPreProcVerifier,
                                              obj->preProcObjVerifier.graph_parameter_out_index,
                                              (vx_reference *)&preproc_out_verifier,
                                              1);
    }
}

static void app_verify_drawdet_outputs(AppObj *obj)
{
    vx_status status;
    vx_status compare_status;

    uint32_t num_refs = 0U;
    vx_image drawdet_out_verifier = NULL;

    status = vxGraphParameterDequeueDoneRef(obj->graphDrawDetVerifier,
                                            obj->drawDetObjVerifier.graph_parameter_out_image_index,
                                            (vx_reference *)&drawdet_out_verifier,
                                            1,
                                            &num_refs);
    if ((status == VX_SUCCESS) && (num_refs == 1U))
    {
        tivxMutexLock(obj->postproc_mutex);
        compare_status = app_compare_images_nv12(obj->drawdet_out_copy,
                                                 drawdet_out_verifier);
        tivxMutexUnlock(obj->postproc_mutex);

        if (compare_status != VX_SUCCESS)
        {
            /* Application-specific error handling can be implemented here */
            printf("ERROR: DrawDet node comparison failed.\n");
        }

        (void)vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                              obj->drawDetObjVerifier.graph_parameter_out_image_index,
                                              (vx_reference *)&drawdet_out_verifier,
                                              1);
    }
}

static vx_status app_verify_program_sequence(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_perf_t perf_aewb;
    vx_perf_t perf_preproc;
    vx_perf_t perf_drawdet;
    vx_bool aewb_en;
    vx_bool preproc_en;
    vx_bool drawdet_en;

    aewb_en = (((obj->test_option & APP_TEST_OPT_R5F_PSM) != 0u) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e);
    preproc_en = (((obj->test_option & APP_TEST_OPT_A72_PSM) != 0u) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e);
    drawdet_en = (((obj->test_option & APP_TEST_OPT_C7X_PSM) != 0u) ? (vx_bool)vx_true_e : (vx_bool)vx_false_e);

    if (aewb_en != (vx_bool)vx_false_e)
    {
        status = vxQueryNode(obj->aewbObj.node,
                             VX_NODE_PERFORMANCE,
                             &perf_aewb,
                             sizeof(perf_aewb));
    }

    if ((status == VX_SUCCESS) && (preproc_en != (vx_bool)vx_false_e))
    {
        status = vxQueryNode(obj->preProcObj.node,
                             VX_NODE_PERFORMANCE,
                             &perf_preproc,
                             sizeof(perf_preproc));
    }

    if ((status == VX_SUCCESS) && (drawdet_en != (vx_bool)vx_false_e))
    {
        status = vxQueryNode(obj->drawDetObj.node,
                             VX_NODE_PERFORMANCE,
                             &perf_drawdet,
                             sizeof(perf_drawdet));
    }

    if (status == VX_SUCCESS)
    {
        if (aewb_en != (vx_bool)vx_false_e)
        {
            if (perf_aewb.tmp > obj->aewb_max_time)
            {
                status = VX_FAILURE;
                printf("ERROR: AEWB tmp too large: %llu\n",
                       (unsigned long long)perf_aewb.tmp);
            }
        }

        if ((status == VX_SUCCESS) && (preproc_en != (vx_bool)vx_false_e))
        {
            if (perf_preproc.tmp > obj->preproc_max_time)
            {
                status = VX_FAILURE;
                printf("ERROR: PREPROC tmp too large: %llu\n",
                       (unsigned long long)perf_preproc.tmp);
            }
        }

        if ((status == VX_SUCCESS) && (drawdet_en != (vx_bool)vx_false_e))
        {
            if (perf_drawdet.tmp > obj->postproc_max_time)
            {
                status = VX_FAILURE;
                printf("ERROR: DRAWDET tmp too large: %llu\n",
                       (unsigned long long)perf_drawdet.tmp);
            }
        }

        if (status == VX_SUCCESS)
        {
            uint64_t prev_end = 0u;
            vx_bool have_prev = (vx_bool)vx_false_e;

            if (aewb_en != (vx_bool)vx_false_e)
            {
                if (!(perf_aewb.beg < perf_aewb.end))
                {
                    status = VX_FAILURE;
                    printf("ERROR: AEWB timestamps invalid\n");
                }
                else
                {
                    prev_end = perf_aewb.end;
                    have_prev = (vx_bool)vx_true_e;
                }
            }

            if ((status == VX_SUCCESS) && (preproc_en != (vx_bool)vx_false_e))
            {
                if (!(perf_preproc.beg < perf_preproc.end))
                {
                    status = VX_FAILURE;
                    printf("ERROR: PREPROC timestamps invalid\n");
                }
                else
                {
                    if ((have_prev != (vx_bool)vx_false_e) && !(prev_end < perf_preproc.beg))
                    {
                        status = VX_FAILURE;
                        printf("ERROR: PREPROC timestamp order invalid\n");
                    }
                    else
                    {
                        prev_end = perf_preproc.end;
                        have_prev = (vx_bool)vx_true_e;
                    }
                }
            }

            if ((status == VX_SUCCESS) && (drawdet_en != (vx_bool)vx_false_e))
            {
                if (!(perf_drawdet.beg < perf_drawdet.end))
                {
                    status = VX_FAILURE;
                    printf("ERROR: DRAWDET timestamps invalid\n");
                }
                else
                {
                    if ((have_prev != (vx_bool)vx_false_e) && !(prev_end < perf_drawdet.beg))
                    {
                        status = VX_FAILURE;
                        printf("ERROR: DRAWDET timestamp order invalid\n");
                    }
                }
            }

            if (status != VX_SUCCESS)
            {
                printf("ERROR: node timestamp order invalid\n");
            }
        }
    }

    return status;
}

static vx_status app_schedule_aewb_verifier(AppObj *obj,
                                            vx_user_data_object h3a_stats,
                                            vx_user_data_object aewb_results,
                                            vx_uint32 frame_id)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    AEWBObj *aewbObjVerifier = &obj->aewbObjVerifier;

    if (obj->aewb_verifier_graph_ready == 0)
    {
        status = app_pipeup_inputs_aewb_verifier_graph(obj, h3a_stats);
    }
    else
    {
        vx_user_data_object h3a_stats_input = NULL;

        status = vxGraphParameterDequeueDoneRef(obj->graphAewbVerifier,
                                                aewbObjVerifier->graph_parameter_in_index,
                                                (vx_reference*)&h3a_stats_input,
                                                1,
                                                &num_refs);

        if (status == VX_SUCCESS)
        {
            status = app_copy_h3a_data(h3a_stats, h3a_stats_input);
        }

        if (status == VX_SUCCESS)
        {
            tivxMutexLock(obj->aewb_mutex);
            status = app_copy_aewb_outputs(aewb_results, obj->aewb_out_copy);
            tivxMutexUnlock(obj->aewb_mutex);
        }

        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graphAewbVerifier,
                                                     aewbObjVerifier->graph_parameter_in_index,
                                                     (vx_reference*)&h3a_stats_input,
                                                     1);
        }
    }

    return status;
}

static vx_status app_schedule_preproc_verifier(AppObj *obj,
                                                vx_image scaler_img,
                                                vx_tensor preproc_main,
                                                vx_uint32 frame_id)
{
    vx_status status = VX_SUCCESS;

    uint32_t num_refs = 0U;

    PreProcObj *ppv = NULL;

    if ((obj == NULL) || (scaler_img == NULL) || (preproc_main == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        ppv = &obj->preProcObjVerifier;

        if (obj->preproc_verifier_graph_ready == 0)
        {
            status = app_pipeup_inputs_preproc_verifier_graph(obj, scaler_img);
        }
        else
        {
            if ((frame_id % obj->check_interval) == 0U)
            {
                vx_image preproc_in_img = NULL;

                status = vxGraphParameterDequeueDoneRef(obj->graphPreProcVerifier,
                                                ppv->graph_parameter_in_index,
                                                (vx_reference *)&preproc_in_img,
                                                1,
                                                &num_refs);
                if (status == VX_SUCCESS)
                {
                    status = app_copy_image_nv12(scaler_img, preproc_in_img);
                }

                if (status == VX_SUCCESS)
                {
                    tivxMutexLock(obj->preproc_mutex);
                    status = app_copy_tensor_u8(preproc_main, obj->preproc_out_copy);
                    tivxMutexUnlock(obj->preproc_mutex);
                }

                if (status == VX_SUCCESS)
                {
                    status = vxGraphParameterEnqueueReadyRef(obj->graphPreProcVerifier,
                                                        ppv->graph_parameter_in_index,
                                                        (vx_reference *)&preproc_in_img,
                                                        1);
                }
            }
        }
    }

    return status;
}

static vx_status app_schedule_drawdet_verifier(AppObj *obj,
                                                vx_tensor tidl_out_tensor,
                                                vx_image scaler_img,
                                                vx_image drawdet_main_out,
                                                vx_uint32 frame_id)
{
    vx_status status = VX_SUCCESS;

    uint32_t num_refs = 0U;

    DrawDetectionsObj *ddv = NULL;

    if ((obj == NULL) || (tidl_out_tensor == NULL) || (scaler_img == NULL) || (drawdet_main_out == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        ddv = &obj->drawDetObjVerifier;

        if (obj->drawdet_verifier_graph_ready == 0)
        {
            status = app_pipeup_inputs_drawdet_verifier_graph(obj, tidl_out_tensor, scaler_img);
        }
        else
        {
            if ((frame_id % obj->check_interval) == 0U)
            {
                vx_tensor dd_in_tensor = NULL;
                vx_image  dd_in_img = NULL;
                status = vxGraphParameterDequeueDoneRef(obj->graphDrawDetVerifier,
                                                        ddv->graph_parameter_in_tensor_index,
                                                        (vx_reference *)&dd_in_tensor,
                                                        1,
                                                        &num_refs);
                if (status == VX_SUCCESS)
                {
                    status = app_copy_tensor_u8(tidl_out_tensor, dd_in_tensor);
                }

                if (status == VX_SUCCESS)
                {
                    num_refs = 0U;

                    status = vxGraphParameterDequeueDoneRef(obj->graphDrawDetVerifier,
                                                            ddv->graph_parameter_in_image_index,
                                                            (vx_reference *)&dd_in_img,
                                                            1,
                                                            &num_refs);
                }

                if (status == VX_SUCCESS)
                {
                    status = app_copy_image_nv12(scaler_img, dd_in_img);
                }

                if (status == VX_SUCCESS)
                {
                    tivxMutexLock(obj->postproc_mutex);
                    status = app_copy_image_nv12(drawdet_main_out, obj->drawdet_out_copy);
                    tivxMutexUnlock(obj->postproc_mutex);
                }

                if (status == VX_SUCCESS)
                {
                    status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                             ddv->graph_parameter_in_tensor_index,
                                                             (vx_reference *)&dd_in_tensor,
                                                             1);
                }

                if (status == VX_SUCCESS)
                {
                    status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                             ddv->graph_parameter_in_image_index,
                                                             (vx_reference *)&dd_in_img,
                                                             1);
                }
            }
        }
    }

    return status;
}

static vx_status app_pipeup_inputs_aewb_verifier_graph(AppObj *obj, vx_user_data_object h3a_stats)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object aewb_results_verifier = NULL;
    uint32_t num_refs = 0U;

    vx_user_data_object h3a_stats_input =
        obj->aewbObjVerifier.h3a_stats[obj->aewb_verifier_in_idx];

    obj->aewb_verifier_in_idx++;
    if (obj->aewb_verifier_in_idx == APP_BUFFER_Q_DEPTH)
    {
        obj->aewb_verifier_graph_ready = 1;
    }

    status = app_copy_h3a_data(h3a_stats, h3a_stats_input);

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphAewbVerifier,
                                                 obj->aewbObjVerifier.graph_parameter_in_index,
                                                 (vx_reference*)&h3a_stats_input,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterDequeueDoneRef(obj->graphAewbVerifier,
                                                obj->aewbObjVerifier.graph_parameter_out_index,
                                                (vx_reference*)&aewb_results_verifier,
                                                1,
                                                &num_refs);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphAewbVerifier,
                                                 obj->aewbObjVerifier.graph_parameter_out_index,
                                                 (vx_reference*)&aewb_results_verifier,
                                                 1);
    }

    return status;
}

static vx_status app_pipeup_inputs_preproc_verifier_graph(AppObj *obj, vx_image scaler_img)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    PreProcObj *ppv = NULL;
    vx_image input_img = NULL;
    vx_tensor preproc_out_verifier = NULL;

    if ((obj == NULL) || (scaler_img == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        ppv = &obj->preProcObjVerifier;

        input_img = ppv->input_img[obj->preproc_verifier_in_idx];

        obj->preproc_verifier_in_idx++;
        if (obj->preproc_verifier_in_idx == APP_BUFFER_Q_DEPTH)
        {
            obj->preproc_verifier_graph_ready = 1;
        }

        status = app_copy_image_nv12(scaler_img, input_img);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphPreProcVerifier,
                                                 ppv->graph_parameter_in_index,
                                                 (vx_reference *)&input_img,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterDequeueDoneRef(obj->graphPreProcVerifier,
                                                ppv->graph_parameter_out_index,
                                                (vx_reference *)&preproc_out_verifier,
                                                1,
                                                &num_refs);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphPreProcVerifier,
                                                 ppv->graph_parameter_out_index,
                                                 (vx_reference *)&preproc_out_verifier,
                                                 1);
    }

    return status;
}

static vx_status app_pipeup_inputs_drawdet_verifier_graph(AppObj *obj,
                                                          vx_tensor tidl_out_tensor,
                                                          vx_image scaler_img)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    DrawDetectionsObj *ddv = NULL;

    vx_tensor input_tensor = NULL;
    vx_image  input_img = NULL;

    vx_image  dd_out_verifier = NULL;

    if ((obj == NULL) || (tidl_out_tensor == NULL) || (scaler_img == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        ddv = &obj->drawDetObjVerifier;

        input_tensor = ddv->input_tensors[obj->drawdet_verifier_in_idx];
        input_img    = ddv->input_images[obj->drawdet_verifier_in_idx];

        obj->drawdet_verifier_in_idx++;
        if (obj->drawdet_verifier_in_idx == APP_BUFFER_Q_DEPTH)
        {
            obj->drawdet_verifier_graph_ready = 1;
        }

        status = app_copy_tensor_u8(tidl_out_tensor, input_tensor);
    }

    if (status == VX_SUCCESS)
    {
        status = app_copy_image_nv12(scaler_img, input_img);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                 ddv->graph_parameter_in_tensor_index,
                                                 (vx_reference *)&input_tensor,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                 ddv->graph_parameter_in_image_index,
                                                 (vx_reference *)&input_img,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterDequeueDoneRef(obj->graphDrawDetVerifier,
                                                ddv->graph_parameter_out_image_index,
                                                (vx_reference *)&dd_out_verifier,
                                                1,
                                                &num_refs);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                 ddv->graph_parameter_out_image_index,
                                                 (vx_reference *)&dd_out_verifier,
                                                 1);
    }

    return status;
}

static vx_tensor app_create_tensor_from_exemplar(vx_context context, vx_tensor exemplar)
{
    vx_status status = VX_SUCCESS;

    vx_size nd = 0U;
    vx_size dims[APP_MAX_TENSOR_DIMS] = {0U};
    vx_enum data_type = 0;

    vx_tensor tensor = NULL;

    status = vxQueryTensor(exemplar, VX_TENSOR_NUMBER_OF_DIMS, &nd, sizeof(nd));

    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(exemplar, VX_TENSOR_DIMS, dims, nd * sizeof(vx_size));
    }

    if (status == VX_SUCCESS)
    {
        status = vxQueryTensor(exemplar, VX_TENSOR_DATA_TYPE, &data_type, sizeof(data_type));
    }

    if (status == VX_SUCCESS)
    {
        tensor = vxCreateTensor(context, nd, dims, data_type, 0);
    }

    return tensor;
}

static vx_status register_events(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxEnableEvents(obj->context);

    if (status == VX_SUCCESS)
    {
        status = vxRegisterEvent((vx_reference)obj->preProcObjVerifier.node,
                                 VX_EVENT_NODE_COMPLETED,
                                 0,
                                 0);
    }
    if (status == VX_SUCCESS)
    {
        status = vxRegisterEvent((vx_reference)obj->aewbObjVerifier.node,
                                 VX_EVENT_NODE_COMPLETED,
                                 0,
                                 0);
    }
    if (status == VX_SUCCESS)
    {
        status = vxRegisterEvent((vx_reference)obj->drawDetObjVerifier.node,
                                 VX_EVENT_NODE_COMPLETED,
                                 0,
                                 0);
    }
    if (status == VX_SUCCESS)
    {
        status = vxRegisterEvent((vx_reference)obj->graph,
                                 VX_EVENT_GRAPH_COMPLETED,
                                 0,
                                 0);
    }

    return status;
}

static void app_monitoring_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_event_t event;
    vx_status status;

    while (1)
    {
        status = vxWaitEvent(obj->context, &event, vx_false_e);

        if (status != VX_SUCCESS)
        {
            continue;
        }

        if (event.type == VX_EVENT_NODE_COMPLETED)
        {
            if (event.event_info.node_completed.node == obj->aewbObjVerifier.node)
            {
                if (obj->aewb_verifier_graph_ready == 1)
                {
                    status = tivxEventPost(obj->evt_aewb_v_run);
                }
            }
            if (event.event_info.node_completed.node == obj->preProcObjVerifier.node)
            {
                if (obj->preproc_verifier_graph_ready == 1)
                {
                    status = tivxEventPost(obj->evt_preproc_v_run);
                }
            }
            if (event.event_info.node_completed.node == obj->drawDetObjVerifier.node)
            {
                if (obj->drawdet_verifier_graph_ready == 1)
                {
                    status = tivxEventPost(obj->evt_drawdet_v_run);
                }
            }
        }
        else if (event.type == VX_EVENT_GRAPH_COMPLETED)
        {
            if (event.event_info.graph_completed.graph == obj->graph)
            {
                (void)app_verify_program_sequence(obj);
            }
        }
    }
}

static void app_run_aewb_verifier_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status;

    while (1)
    {
        status = tivxEventWait(obj->evt_aewb_v_run, VX_TIMEOUT_WAIT_FOREVER);

        if (status == VX_SUCCESS)
        {
            app_verify_aewb_outputs(obj);
        }
    }
}

static void app_run_preproc_verifier_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status;

    while (1)
    {
        status = tivxEventWait(obj->evt_preproc_v_run, VX_TIMEOUT_WAIT_FOREVER);

        if (status == VX_SUCCESS)
        {
            app_verify_preproc_outputs(obj);
        }
    }
}

static void app_run_drawdet_verifier_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status;

    while (1)
    {
        status = tivxEventWait(obj->evt_drawdet_v_run, VX_TIMEOUT_WAIT_FOREVER);

        if (status == VX_SUCCESS)
        {
            app_verify_drawdet_outputs(obj);
        }
    }
}

static vx_status app_monitoring_tasks_create(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    tivx_task_create_params_t params;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_monitoring_task;
    params.app_var   = obj;

    status = tivxTaskCreate(&obj->monitoring_task, &params);

    if (status == VX_SUCCESS)
    {
        tivxTaskSetDefaultCreateParams(&params);
        params.task_main = app_run_aewb_verifier_task;
        params.app_var   = obj;

        status = tivxTaskCreate(&obj->aewb_verifier_task, &params);
    }

    if (status == VX_SUCCESS)
    {
        tivxTaskSetDefaultCreateParams(&params);
        params.task_main = app_run_preproc_verifier_task;
        params.app_var   = obj;

        status = tivxTaskCreate(&obj->preproc_verifier_task, &params);
    }

    if (status == VX_SUCCESS)
    {
        tivxTaskSetDefaultCreateParams(&params);
        params.task_main = app_run_drawdet_verifier_task;
        params.app_var   = obj;

        status = tivxTaskCreate(&obj->drawdet_verifier_task, &params);
    }

    return status;
}

static void app_monitoring_tasks_delete(AppObj *obj)
{
    vx_status status;

    status = tivxTaskDelete(&obj->monitoring_task);
    if (status != VX_SUCCESS)
    {
        printf("Error deleting monitoring_task: %d\n", status);
    }

    status = tivxTaskDelete(&obj->aewb_verifier_task);
    if (status != VX_SUCCESS)
    {
        printf("Error deleting aewb_verifier_task: %d\n", status);
    }

    status = tivxTaskDelete(&obj->preproc_verifier_task);
    if (status != VX_SUCCESS)
    {
        printf("Error deleting preproc_verifier_task: %d\n", status);
    }

    status = tivxTaskDelete(&obj->drawdet_verifier_task);
    if (status != VX_SUCCESS)
    {
        printf("Error deleting drawdet_verifier_task: %d\n", status);
    }
}

static vx_status app_delete_graph_safety(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_status temp_status = VX_SUCCESS;
    vx_int32 q;

    for (q = 0; q < APP_BUFFER_Q_DEPTH; q++)
    {
        temp_status = vxReleaseObjectArray(&obj->aewb_verifier_h3a_stats[q]);
        if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
        {
            status = temp_status;
        }
    }

    temp_status = vxReleaseTensor(&obj->preproc_out_copy);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = vxReleaseUserDataObject(&obj->aewb_out_copy);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = vxReleaseImage(&obj->dd_in_img);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = vxReleaseImage(&obj->drawdet_out_copy);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    app_delete_aewb(&obj->aewbObjVerifier);
    app_delete_pre_proc_queued(&obj->preProcObjVerifier);
    app_delete_dd_queued(&obj->drawDetObjVerifier);

    temp_status = vxReleaseGraph(&obj->graphAewbVerifier);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = vxReleaseGraph(&obj->graphPreProcVerifier);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = vxReleaseGraph(&obj->graphDrawDetVerifier);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    return status;
}

static void app_parse_cfg_file_safety(AppObj *obj, vx_char *token, vx_char *s)
{
    if (strcmp(token, "aewb_max_time") == 0)
    {
        token = strtok(NULL, s);
        if (token != NULL)
        {
            obj->aewb_max_time = strtoull(token, NULL, 10);
        }
    }
    else if (strcmp(token, "preproc_max_time") == 0)
    {
        token = strtok(NULL, s);
        if (token != NULL)
        {
            obj->preproc_max_time = strtoull(token, NULL, 10);
        }
    }
    else if (strcmp(token, "postproc_max_time") == 0)
    {
        token = strtok(NULL, s);
        if (token != NULL)
        {
            obj->postproc_max_time = strtoull(token, NULL, 10);
        }
    }
    else if (strcmp(token, "check_interval" )== 0)
    {
        token = strtok(NULL, s);
        if (token != NULL)
        {
            obj->check_interval = atoi(token);
        }
    }
    else if (strcmp(token, "test_option") == 0)
    {
        token = strtok(NULL, s);
        if (token != NULL)
        {
            int value = atoi(token);

            if ((value >= 1) && (value <= 6))
            {
                obj->test_option = (uint32_t)(1u << value);
            }
            else
            {
                obj->test_option = APP_TEST_OPT_DEFAULT;
            }
        }
    }
}

static vx_status app_init_safety(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = tivxMutexCreate(&obj->aewb_mutex);

    if (status == VX_SUCCESS)
    {
        status = tivxMutexCreate(&obj->preproc_mutex);
    }

    if (status == VX_SUCCESS)
    {
        status = tivxMutexCreate(&obj->postproc_mutex);
    }

    if (status == VX_SUCCESS)
    {
        status = app_init_aewb_verifier_input(obj);
    }

    if (status == VX_SUCCESS)
    {
        status = app_init_aewb_queued(obj->context,
                                      &obj->aewbObjVerifier,
                                      &obj->sensorObj,
                                      0,
                                      obj->sensorObj.num_cameras_enabled,
                                      APP_BUFFER_Q_DEPTH);
    }

    if (status == VX_SUCCESS)
    {
        status = app_init_pre_proc_queued(obj->context,
                                          &obj->preProcObjVerifier,
                                          "pre_proc_verifier_obj");
    }

    if (status == VX_SUCCESS)
    {
        status = app_update_pre_proc_queued(obj->context,
                                            &obj->preProcObjVerifier,
                                            obj->tidlObj.config,
                                            APP_BUFFER_Q_DEPTH,
                                            obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].arr[0]);
    }

    if (status == VX_SUCCESS)
    {
        status = app_update_dd_queued(&obj->drawDetObjVerifier, obj->tidlObj.config);
    }

    if (status == VX_SUCCESS)
    {
        status = app_init_dd_queued(obj->context,
                                    &obj->drawDetObjVerifier,
                                    "draw_detections_ver_obj",
                                    obj->sensorObj.num_cameras_enabled,
                                    APP_BUFFER_Q_DEPTH,
                                    vx_true_e,
                                    obj->tidlObj.output_tensor_q[0][0]);
    }

    if (status == VX_SUCCESS)
    {
        obj->dd_in_img = vxCreateImage(obj->context,
                                       obj->drawDetObj.params.width,
                                       obj->drawDetObj.params.height,
                                       VX_DF_IMAGE_NV12);

        if (NULL == obj->dd_in_img)
        {
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        obj->aewb_out_copy = vxCreateUserDataObject(obj->context,
                                                    "tivx_ae_awb_params_t",
                                                    sizeof(tivx_ae_awb_params_t),
                                                    NULL);

        if (NULL == obj->aewb_out_copy)
        {
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        obj->preproc_out_copy = app_create_tensor_from_exemplar(obj->context,
                                                                obj->preProcObj.output_tensor[0][0]);

        if (NULL == obj->preproc_out_copy)
        {
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        obj->drawdet_out_copy = vxCreateImage(obj->context,
                                              obj->drawDetObj.params.width,
                                              obj->drawDetObj.params.height,
                                              VX_DF_IMAGE_NV12);

        if (NULL == obj->drawdet_out_copy)
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

static vx_status app_deinit_safety(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_status temp_status = VX_SUCCESS;

    app_deinit_aewb_queued(&obj->aewbObjVerifier);
    app_deinit_pre_proc_queued(&obj->preProcObjVerifier);
    app_deinit_dd_queued(&obj->drawDetObjVerifier);

    temp_status = tivxMutexDelete(&obj->aewb_mutex);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = tivxMutexDelete(&obj->preproc_mutex);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = tivxMutexDelete(&obj->postproc_mutex);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = tivxEventDelete(&obj->evt_aewb_v_run);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = tivxEventDelete(&obj->evt_preproc_v_run);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    temp_status = tivxEventDelete(&obj->evt_drawdet_v_run);
    if ((status == VX_SUCCESS) && (VX_SUCCESS != temp_status))
    {
        status = temp_status;
    }

    return status;
}

static vx_status app_create_verifier_graphs(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_graph_parameter_queue_params_t graph_aewb_parameters_queue_params_list[2];
    vx_graph_parameter_queue_params_t graph_preproc_parameters_queue_params_list[2];
    vx_graph_parameter_queue_params_t graph_drawdet_parameters_queue_params_list[3];

    obj->graphAewbVerifier = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graphAewbVerifier);

    if (status == VX_SUCCESS)
    {
        (void)vxSetReferenceName((vx_reference)obj->graphAewbVerifier, "app_aewb_verifier_graph");
        APP_PRINT("Graph AEWB verifier create done!\n");
    }

    if (status == VX_SUCCESS)
    {
        obj->graphPreProcVerifier = vxCreateGraph(obj->context);
        status = vxGetStatus((vx_reference)obj->graphPreProcVerifier);

        if (status == VX_SUCCESS)
        {
            (void)vxSetReferenceName((vx_reference)obj->graphPreProcVerifier, "app_preproc_verifier_graph");
            APP_PRINT("Graph PreProc verifier create done!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        obj->graphDrawDetVerifier = vxCreateGraph(obj->context);
        status = vxGetStatus((vx_reference)obj->graphDrawDetVerifier);

        if (status == VX_SUCCESS)
        {
            (void)vxSetReferenceName((vx_reference)obj->graphDrawDetVerifier, "app_drawdet_verifier_graph");
            APP_PRINT("Graph DrawDet verifier create done!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_aewb_queued(obj->graphAewbVerifier,
                                              &obj->aewbObjVerifier,
                                              &obj->aewb_verifier_h3a_stats[0],
                                              TIVX_TARGET_MCU2_1);
        APP_PRINT("AEWB verifier graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_pre_proc_queued(obj->graphPreProcVerifier,
                                                  &obj->preProcObjVerifier,
                                                  obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].img[0],
                                                  "pre_proc_verifier_node",
                                                  TIVX_TARGET_MPU_1);
        APP_PRINT("Pre proc verifier graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_dd_queued(obj->graphDrawDetVerifier,
                                            &obj->drawDetObjVerifier,
                                            obj->tidlObj.output_tensor_arr_q[0][0],
                                            obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].arr[0],
                                            TIVX_TARGET_DSP_C7_1);
        APP_PRINT("Draw detections verifier graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        add_graph_parameter_by_node_index(obj->graphAewbVerifier, obj->aewbObjVerifier.node, 2);
        obj->aewbObjVerifier.graph_parameter_in_index = 0;
        graph_aewb_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_aewb_parameters_queue_params_list[0].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_aewb_parameters_queue_params_list[0].refs_list = (vx_reference *)&obj->aewbObjVerifier.h3a_stats[0];

        add_graph_parameter_by_node_index(obj->graphAewbVerifier, obj->aewbObjVerifier.node, 4);
        obj->aewbObjVerifier.graph_parameter_out_index = 1;
        graph_aewb_parameters_queue_params_list[1].graph_parameter_index = 1;
        graph_aewb_parameters_queue_params_list[1].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_aewb_parameters_queue_params_list[1].refs_list = (vx_reference *)&obj->aewbObjVerifier.aewb_output[0];

        vxSetGraphScheduleConfig(obj->graphAewbVerifier,
                                 VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                                 2,
                                 graph_aewb_parameters_queue_params_list);

        tivxSetGraphPipelineDepth(obj->graphAewbVerifier, 1);

        add_graph_parameter_by_node_index(obj->graphPreProcVerifier, obj->preProcObjVerifier.node, 1);
        obj->preProcObjVerifier.graph_parameter_in_index = 0;
        graph_preproc_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_preproc_parameters_queue_params_list[0].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_preproc_parameters_queue_params_list[0].refs_list = (vx_reference *)&obj->preProcObjVerifier.input_img[0];

        add_graph_parameter_by_node_index(obj->graphPreProcVerifier, obj->preProcObjVerifier.node, 2);
        obj->preProcObjVerifier.graph_parameter_out_index = 1;
        graph_preproc_parameters_queue_params_list[1].graph_parameter_index = 1;
        graph_preproc_parameters_queue_params_list[1].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_preproc_parameters_queue_params_list[1].refs_list = (vx_reference *)&obj->preProcObjVerifier.output_tensor[0][0];

        vxSetGraphScheduleConfig(obj->graphPreProcVerifier,
                                 VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                                 2,
                                 graph_preproc_parameters_queue_params_list);

        tivxSetGraphPipelineDepth(obj->graphPreProcVerifier, 1);

        add_graph_parameter_by_node_index(obj->graphDrawDetVerifier, obj->drawDetObjVerifier.node, 1);
        obj->drawDetObjVerifier.graph_parameter_in_tensor_index = 0;
        graph_drawdet_parameters_queue_params_list[0].graph_parameter_index = 0;
        graph_drawdet_parameters_queue_params_list[0].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_drawdet_parameters_queue_params_list[0].refs_list =
            (vx_reference *)&obj->drawDetObjVerifier.input_tensors[0];

        add_graph_parameter_by_node_index(obj->graphDrawDetVerifier, obj->drawDetObjVerifier.node, 2);
        obj->drawDetObjVerifier.graph_parameter_in_image_index = 1;
        graph_drawdet_parameters_queue_params_list[1].graph_parameter_index = 1;
        graph_drawdet_parameters_queue_params_list[1].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_drawdet_parameters_queue_params_list[1].refs_list =
            (vx_reference *)&obj->drawDetObjVerifier.input_images[0];

        add_graph_parameter_by_node_index(obj->graphDrawDetVerifier, obj->drawDetObjVerifier.node, 3);
        obj->drawDetObjVerifier.graph_parameter_out_image_index = 2;
        graph_drawdet_parameters_queue_params_list[2].graph_parameter_index = 2;
        graph_drawdet_parameters_queue_params_list[2].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_drawdet_parameters_queue_params_list[2].refs_list =
            (vx_reference *)&obj->drawDetObjVerifier.output_images[0];

        vxSetGraphScheduleConfig(obj->graphDrawDetVerifier,
                                 VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                                 3,
                                 graph_drawdet_parameters_queue_params_list);

        tivxSetGraphPipelineDepth(obj->graphDrawDetVerifier, 1);
    }

    return status;
}

static vx_status app_verify_verifier_graphs(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graphAewbVerifier);

    if (status == VX_SUCCESS)
    {
        APP_PRINT("Graph AEWB verify SUCCESS!\n");
    }
    else
    {
        APP_PRINT("Graph AEWB verify FAILURE! (%d)\n", (int)status);
    }

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graphPreProcVerifier);

        if (status == VX_SUCCESS)
        {
            APP_PRINT("Graph PreProc verify SUCCESS!\n");
        }
        else
        {
            APP_PRINT("Graph PreProc verify FAILURE! (%d)\n", (int)status);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graphDrawDetVerifier);

        if (status == VX_SUCCESS)
        {
            APP_PRINT("Graph DrawDet verify SUCCESS!\n");
        }
        else
        {
            APP_PRINT("Graph DrawDet verify FAILURE! (%d)\n", (int)status);
        }
    }

    return status;
}

static vx_status app_wait_verifier_graphs(AppObj *obj, vx_status status)
{
    vx_status wait_status = VX_SUCCESS;

    if (obj->test_option == APP_TEST_OPT_R5F_RC)
    {
        wait_status = vxWaitGraph(obj->graphAewbVerifier);
        if (wait_status != VX_SUCCESS)
        {
            APP_PRINT("Error waiting for AEWB verifier graph completion: %d\n", wait_status);

            if (status == VX_SUCCESS)
            {
                status = wait_status;
            }
        }
    }

    if (obj->test_option == APP_TEST_OPT_A72_RC)
    {
        wait_status = vxWaitGraph(obj->graphPreProcVerifier);
        if (wait_status != VX_SUCCESS)
        {
            APP_PRINT("Error waiting for PreProc verifier graph completion: %d\n", wait_status);

            if (status == VX_SUCCESS)
            {
                status = wait_status;
            }
        }
    }

    if (obj->test_option == APP_TEST_OPT_C7X_RC)
    {
        wait_status = vxWaitGraph(obj->graphDrawDetVerifier);
        if (wait_status != VX_SUCCESS)
        {
            APP_PRINT("Error waiting for DrawDet verifier graph completion: %d\n", wait_status);

            if (status == VX_SUCCESS)
            {
                status = wait_status;
            }
        }
    }

    return status;
}

static void app_update_param_set_safety(AppObj *obj)
{
    if (obj->check_interval == 0U)
    {
        obj->check_interval = DEFAULT_CHECK_INTERVAL;
    }

    if (obj->aewb_max_time == 0U)
    {
        obj->aewb_max_time = DEFAULT_AEWB_MAX_TIME;
    }

    if (obj->preproc_max_time == 0U)
    {
        obj->preproc_max_time = DEFAULT_PREPROC_MAX_TIME;
    }

    if (obj->postproc_max_time == 0U)
    {
        obj->postproc_max_time = DEFAULT_POSTPROC_MAX_TIME;
    }

    update_draw_detections_defaults(obj, &obj->drawDetObjVerifier);
}


static vx_status app_dequeue_viss_aewb_from_main_graph(AppObj *obj,
                                                      vx_user_data_object *h3a_stats,
                                                      vx_user_data_object *aewb_results)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    if ((obj == NULL) || (h3a_stats == NULL) || (aewb_results == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                obj->vissObj.graph_parameter_index,
                                                (vx_reference *)h3a_stats,
                                                1,
                                                &num_refs);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                obj->aewbObj.graph_parameter_out_index,
                                                (vx_reference *)aewb_results,
                                                1,
                                                &num_refs);
    }

    return status;
}

static vx_status app_reenqueue_viss_aewb_to_main_graph(AppObj *obj,
                                                       vx_user_data_object h3a_stats,
                                                       vx_user_data_object aewb_results)
{
    vx_status status = VX_SUCCESS;

    if ((obj == NULL) || (h3a_stats == NULL) || (aewb_results == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 obj->aewbObj.graph_parameter_out_index,
                                                 (vx_reference *)&aewb_results,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 obj->vissObj.graph_parameter_index,
                                                 (vx_reference *)&h3a_stats,
                                                 1);
    }

    return status;
}

static vx_status app_dequeue_scaler_preproc_from_main_graph(AppObj *obj,
                                                           vx_image *scaler_output,
                                                           vx_tensor *preproc_output)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    if ((obj == NULL) || (scaler_output == NULL) || (preproc_output == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        for (vx_uint32 out_idx = 0U; out_idx < (vx_uint32)MSC_OUT_MAX; out_idx++)
        {
            num_refs = 0U;

            status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                    obj->scalerObj.graph_parameter_idxs[out_idx],
                                                    (vx_reference *)&scaler_output[out_idx],
                                                    1,
                                                    &num_refs);

            if ((status != VX_SUCCESS) || (num_refs != 1U) || (scaler_output[out_idx] == NULL))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        num_refs = 0U;

        status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                obj->preProcObj.graph_parameter_out_index,
                                                (vx_reference *)preproc_output,
                                                1,
                                                &num_refs);

        if ((status != VX_SUCCESS) || (num_refs != 1U) || (*preproc_output == NULL))
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

static vx_status app_enqueue_scaler_preproc_to_main_graph(AppObj *obj,
                                                         vx_image *scaler_output,
                                                         vx_tensor preproc_output)
{
    vx_status status = VX_SUCCESS;

    if ((obj == NULL) || (scaler_output == NULL) || (preproc_output == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 obj->preProcObj.graph_parameter_out_index,
                                                 (vx_reference *)&preproc_output,
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        for (vx_uint32 out_idx = 0U; out_idx < (vx_uint32)MSC_OUT_MAX; out_idx++)
        {
            if (scaler_output[out_idx] == NULL)
            {
                status = VX_FAILURE;
                break;
            }

            status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                     obj->scalerObj.graph_parameter_idxs[out_idx],
                                                     (vx_reference *)&scaler_output[out_idx],
                                                     1);
            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    return status;
}

static vx_status app_dequeue_tidl_drawdet_from_main_graph(AppObj *obj,
                                                          vx_tensor *tidl_output,
                                                          vx_image *draw_det_output)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs = 0U;
    vx_int32 out_idx;

    if ((obj == NULL) || (tidl_output == NULL) || (draw_det_output == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        for (out_idx = 0; out_idx < (vx_int32)obj->tidlObj.num_output_tensors; out_idx++)
        {
            num_refs = 0U;

            status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                    obj->tidlObj.graph_parameter_idxs[out_idx],
                                                    (vx_reference *)&tidl_output[out_idx],
                                                    1,
                                                    &num_refs);

            if ((status != VX_SUCCESS) ||
                (num_refs != 1U) ||
                (tidl_output[out_idx] == NULL))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        num_refs = 0U;

        status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                obj->drawDetObj.graph_parameter_out_image_index,
                                                (vx_reference *)draw_det_output,
                                                1,
                                                &num_refs);

        if ((status != VX_SUCCESS) ||
            (num_refs != 1U) ||
            (*draw_det_output == NULL))
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

static vx_status app_enqueue_tidl_drawdet_to_main_graph(AppObj *obj,
                                                        vx_tensor *tidl_output,
                                                        vx_image draw_det_output)
{
    vx_status status = VX_SUCCESS;
    vx_int32 out_idx;

    if ((obj == NULL) || (tidl_output == NULL) || (draw_det_output == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        for (out_idx = 0; out_idx < (vx_int32)obj->tidlObj.num_output_tensors; out_idx++)
        {
            if (tidl_output[out_idx] == NULL)
            {
                status = VX_FAILURE;
                break;
            }

            status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                     obj->tidlObj.graph_parameter_idxs[out_idx],
                                                     (vx_reference *)&tidl_output[out_idx],
                                                     1);

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 obj->drawDetObj.graph_parameter_out_image_index,
                                                 (vx_reference *)&draw_det_output,
                                                 1);
    }

    return status;
}

#endif // SAFETY_ENABLED

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL Demo - (Safe) Camera based Object Detection (c) Texas Instruments Inc. 2026\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ================================================"
    "\n TIDL Demo - Camera based OD (" APP_NAME_SUFFIX ")"
    "\n ================================================"
    "\n"
    "\n p: Print performance statistics"
    "\n"
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};

static void app_run_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status = VX_SUCCESS;
    while (!obj->stop_task && (status == VX_SUCCESS))
    {
        status = app_run_graph(obj);
    }
    obj->stop_task_done = 1;
}

static vx_status app_run_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    vx_status status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_task;
    params.app_var = obj;

    obj->stop_task_done = 0;
    obj->stop_task = 0;

    status = tivxTaskCreate(&obj->task, &params);

    return status;
}

static void app_run_task_delete(AppObj *obj)
{
    while (obj->stop_task_done == 0)
    {
        tivxTaskWaitMsecs(100);
    }

    tivxTaskDelete(&obj->task);
    SAFETY_ENABLED_RUN(app_monitoring_tasks_delete(obj));
}

static vx_status app_run_graph_interactive(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint32_t done = 0;

    char ch;
    FILE *fp;
    app_perf_point_t *perf_arr[1];

    SAFETY_ENABLED_RUN(tivxEventCreate(&obj->evt_aewb_v_run));
    SAFETY_ENABLED_RUN(tivxEventCreate(&obj->evt_preproc_v_run));
    SAFETY_ENABLED_RUN(tivxEventCreate(&obj->evt_drawdet_v_run));

    status = app_run_task_create(obj);

    if (status != VX_SUCCESS)
    {
        printf("app_tidl: ERROR: Unable to create task\n");
    }
    else
    {
        appPerfStatsResetAll();
        while (!done && (status == VX_SUCCESS))
        {
            printf(menu);
            SAFETY_ENABLED_RUN(app_print_test_options(obj->test_option));
            ch = getchar();
            printf("\n");

            switch(ch)
            {
                case 'p':
                    appPerfStatsPrintAll();
                    status = tivx_utils_graph_perf_print(obj->graph);
                    appPerfPointPrint(&obj->fileio_perf);
                    appPerfPointPrint(&obj->total_perf);
                    printf("\n");
                    appPerfPointPrintFPS(&obj->total_perf);
                    appPerfPointReset(&obj->total_perf);
                    printf("\n");

                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_od_cam_safe");
                    if (NULL != fp)
                    {
                        appPerfStatsExportAll(fp, perf_arr, 1);
                        status = tivx_utils_graph_perf_export(fp, obj->graph);
                        appPerfStatsExportCloseFile(fp);
                        appPerfStatsResetAll();
                    }
                    else
                    {
                        printf("fp is null\n");
                    }
                    break;
                case 'x':
                    obj->stop_task = 1;
                    done = 1;
                    break;
            }
        }
        app_run_task_delete(obj);
    }
    return status;
}

static void app_set_cfg_default(AppObj *obj)
{
    snprintf(obj->captureObj.output_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->ldcObj.output_file_path,APP_MAX_FILE_PATH, ".");

    obj->captureObj.en_out_capture_write = 0;
    obj->ldcObj.en_out_ldc_write = 0;

    obj->num_frames_to_write = 0;
    obj->num_frames_to_skip = 0;

    snprintf(obj->tidlObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->tidlObj.network_file_path,APP_MAX_FILE_PATH, ".");
}

static void app_parse_cfg_file(AppObj *obj, vx_char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    vx_char line_str[1024];
    vx_char *token;

    if (fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
        exit(-1);
    }

    while (fgets(line_str, sizeof(line_str), fp)!=NULL)
    {
        vx_char s[]=" \t";

        if (strchr(line_str, '#'))
        {
            continue;
        }

        /* get the first token */
        token = strtok(line_str, s);
        if (token != NULL)
        {
            if (strcmp(token, "sensor_index")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    obj->sensorObj.sensor_index = atoi(token);
                }
            }
            else
            if (strcmp(token, "is_interactive")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->is_interactive = atoi(token);
                    if (obj->is_interactive > 1)
                    obj->is_interactive = 1;
                }
            }
            else
            if (strcmp(token, "tidl_config")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->tidlObj.config_file_path, token);
                }
            }
            else
            if (strcmp(token, "tidl_network")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->tidlObj.network_file_path, token);
                }
            }
            else
            if (strcmp(token, "dl_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if (token != NULL)
                {
                    width =  atoi(token);
                    obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].width   = width;

                    token = strtok(NULL, s);
                    if (token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].height  = height;
                    }
                }
            }
            else
            if (strcmp(token, "out_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if (token != NULL)
                {
                    width =  atoi(token);
                    obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].width   = width;

                    token = strtok(NULL, s);
                    if (token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].height  = height;
                    }
                }
            }
            else
            if (strcmp(token, "viz_th")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    obj->drawDetObj.params.viz_th = atof(token);
                    SAFETY_ENABLED_RUN(obj->drawDetObjVerifier.params.viz_th = atof(token));
                }
            }
            else
            if (strcmp(token, "num_classes")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    obj->drawDetObj.params.num_classes = atoi(token);
                    SAFETY_ENABLED_RUN(obj->drawDetObjVerifier.params.num_classes = atoi(token));
                }
            }
            else
            if (strcmp(token, "display_option")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    obj->displayObj.display_option = atoi(token);
                }
            }
            else
            if (strcmp(token, "cpu_core_id")==0)
            {
                token = strtok(NULL, s);
                if (token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->cpu_core_id = atoi(token);
                }
            }
            SAFETY_ENABLED_RUN(app_parse_cfg_file_safety(obj, token, s));
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;

    app_set_cfg_default(obj);

    if (argc==1)
    {
        app_show_usage(argc, argv);
        exit(0);
    }

    for (i=0; i<argc; i++)
    {
        if (strcmp(argv[i], "--cfg")==0)
        {
            i++;
            if (i>=argc)
            {
                app_show_usage(argc, argv);
            }
            app_parse_cfg_file(obj, argv[i]);
            break;
        }
        else
        if (strcmp(argv[i], "--help")==0)
        {
            app_show_usage(argc, argv);
            exit(0);
        }
    }

    return;
}

vx_status app_tidl_od_cam_safe_main(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;

    AppObj *obj = &gAppObj;

    /*Optional parameter setting*/
    app_default_param_set(obj);
    APP_PRINT("Default param set! \n");

    /*Config parameter reading*/
    app_parse_cmd_line_args(obj, argc, argv);
    APP_PRINT("Parsed user params! \n");

    /* Querry sensor parameters */
    app_querry_sensor(&obj->sensorObj);
    APP_PRINT("Sensor params queried! \n");

    /* Apply parameter defaults and derived settings after configuration is loaded */
    app_update_param_set(obj);
    APP_PRINT("Updated user params!\n");

    status = app_init(obj);
    APP_PRINT("App Init Done! \n");

    if (status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
        APP_PRINT("App Create Graph Done! \n");
    }
    if (status == VX_SUCCESS)
    {
        SAFETY_ENABLED_RUN(status = register_events(obj));
        SAFETY_ENABLED_RUN(APP_PRINT("App Register Events Done! \n"));
    }
    if (status == VX_SUCCESS)
    {
        SAFETY_ENABLED_RUN(status = app_monitoring_tasks_create(obj));
        SAFETY_ENABLED_RUN(APP_PRINT("App Create Monitoring Task Done! \n"));
    }
    if (status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
        APP_PRINT("App Verify Graph Done! \n");
    }
    if (obj->is_interactive && (status == VX_SUCCESS))
    {
        status = app_run_graph_interactive(obj);
    }
    else
    if (status == VX_SUCCESS)
    {
        status = app_run_graph(obj);
    }

    APP_PRINT("App Run Graph Done! \n");

    app_delete_graph(obj);
    APP_PRINT("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINT("App De-init Done! \n");

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    app_grpx_init_prms_t grpx_prms;
    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);
    APP_PRINT("Creating context done!\n");
    if (status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxImagingLoadKernels(obj->context);
        tivxImgProcLoadKernels(obj->context);
        tivxTIDLLoadKernels(obj->context);
        tivxFileIOLoadKernels(obj->context);
    }
    APP_PRINT("Kernel loading done!\n");

    /* Initialize modules */

    app_init_sensor(&obj->sensorObj, "sensor_obj");
    APP_PRINT("Sensor init done!\n");

    app_init_capture(obj->context, &obj->captureObj, &obj->sensorObj, "capture_obj", APP_BUFFER_Q_DEPTH);
    APP_PRINT("Capture init done!\n");

    app_init_viss_queued(obj->context, &obj->vissObj, &obj->sensorObj, "viss_obj", obj->sensorObj.num_cameras_enabled, APP_BUFFER_Q_DEPTH);
    APP_PRINT("VISS init done!\n");

    app_init_aewb_queued(obj->context, &obj->aewbObj, &obj->sensorObj, 0, obj->sensorObj.num_cameras_enabled, APP_BUFFER_Q_DEPTH);
    APP_PRINT("AEWB init done!\n");

    app_init_ldc(obj->context, &obj->ldcObj, &obj->sensorObj, "ldc_obj", obj->sensorObj.num_cameras_enabled);
    APP_PRINT("LDC init done!\n");

    printf("Scaler output1 width   = %d\n", obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].width);
    printf("Scaler output1 height  = %d\n", obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].height);
    printf("Scaler output2 width   = %d\n", obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].width);
    printf("Scaler output2 height  = %d\n", obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].height);

    app_init_scaler_queued(obj->context, &obj->scalerObj, "scaler_obj", obj->sensorObj.num_cameras_enabled, MSC_OUT_MAX, APP_BUFFER_Q_DEPTH);
    APP_PRINT("Scaler init done!\n");

    #if defined (SOC_J784S4)
    obj->tidlObj.core_id = obj->cpu_core_id;
    #else
    obj->tidlObj.core_id = 0;
    #endif
    /* Initialize TIDL first to get tensor I/O information from network */
    app_init_tidl_queued(obj->context, &obj->tidlObj, "tidl_obj", obj->sensorObj.num_cameras_enabled, APP_BUFFER_Q_DEPTH);
    APP_PRINT("TIDL Init Done! \n");

    /* Update pre-proc parameters with TIDL config before calling init */
    app_update_pre_proc_queued(obj->context, &obj->preProcObj, obj->tidlObj.config, APP_BUFFER_Q_DEPTH, obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].arr[0]);
    APP_PRINT("Pre Proc Update Done! \n");

    app_init_pre_proc_queued(obj->context, &obj->preProcObj, "pre_proc_obj");
    APP_PRINT("Pre Proc Init Done! \n");

    /* Update ioBufDesc in draw detections object */
    app_update_dd_queued(&obj->drawDetObj, obj->tidlObj.config);
    APP_PRINT("Draw detections Update Done! \n");

    app_init_dd_queued(obj->context, &obj->drawDetObj, "draw_detections_obj", obj->sensorObj.num_cameras_enabled, APP_BUFFER_Q_DEPTH, vx_false_e, obj->tidlObj.output_tensor_q[0][0]);
    APP_PRINT("Draw Detections Init Done! \n");

    app_init_img_mosaic(obj->context, &obj->imgMosaicObj, "img_mosaic_obj", APP_BUFFER_Q_DEPTH);
    APP_PRINT("Img Mosaic init done!\n");

    app_init_display(obj->context, &obj->displayObj, "display_obj");
    APP_PRINT("Display init done!\n");

    if (obj->displayObj.display_option == 1)
    {
        appGrpxInitParamsInit(&grpx_prms, obj->context);
        grpx_prms.draw_callback = app_draw_graphics;
        appGrpxInit(&grpx_prms);
    }

    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");

    SAFETY_ENABLED_RUN(
        if (app_init_safety(obj) == VX_SUCCESS)
        {
            APP_PRINT("App init safety done!");
            (void)0;
        }
        else
        {
            printf("ERROR: App init safety failed!");
        }
    );

    return status;
}

static void app_deinit(AppObj *obj)
{
    app_deinit_sensor(&obj->sensorObj);
    APP_PRINT("Sensor deinit done!\n");

    app_deinit_capture(&obj->captureObj, APP_BUFFER_Q_DEPTH);
    APP_PRINT("Capture deinit done!\n");

    app_deinit_viss_queued(&obj->vissObj);
    APP_PRINT("VISS deinit done!\n");

    app_deinit_aewb_queued(&obj->aewbObj);
    APP_PRINT("AEWB deinit done!\n");

    app_deinit_ldc(&obj->ldcObj);
    APP_PRINT("LDC deinit done!\n");

    app_deinit_scaler_queued(&obj->scalerObj);
    APP_PRINT("Scaler deinit done!\n");

    app_deinit_pre_proc_queued(&obj->preProcObj);
    APP_PRINT("Pre proc deinit done!\n");

    app_deinit_tidl_queued(&obj->tidlObj, APP_BUFFER_Q_DEPTH);
    APP_PRINT("TIDL deinit done!\n");

    app_deinit_dd_queued(&obj->drawDetObj);
    APP_PRINT("Draw detections deinit done!\n");

    app_deinit_img_mosaic(&obj->imgMosaicObj, APP_BUFFER_Q_DEPTH);
    APP_PRINT("Img Mosaic deinit done!\n");

    app_deinit_display(&obj->displayObj);
    APP_PRINT("Display deinit done!\n");

    if (obj->displayObj.display_option == 1)
    {
        appGrpxDeInit();
    }

    SAFETY_ENABLED_RUN(
        if (app_deinit_safety(obj) == VX_SUCCESS)
        {
            APP_PRINT("App deinit safety done!\n");
        }
        else
        {
            printf("ERROR: App deinit safety failed!");
        }
    );

    tivxTIDLUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxImagingUnLoadKernels(obj->context);
    tivxImgProcUnLoadKernels(obj->context);
    tivxFileIOUnLoadKernels(obj->context);
    APP_PRINT("Kernels unload done!\n");

    vxReleaseContext(&obj->context);
    APP_PRINT("Release context done!\n");
}


static void app_delete_graph(AppObj *obj)
{
    app_delete_capture(&obj->captureObj);
    APP_PRINT("Capture delete done!\n");

    app_delete_viss(&obj->vissObj);
    APP_PRINT("VISS delete done!\n");

    app_delete_aewb(&obj->aewbObj);
    APP_PRINT("AEWB delete done!\n");

    app_delete_ldc(&obj->ldcObj);
    APP_PRINT("LDC delete done!\n");

    app_delete_scaler(&obj->scalerObj);
    APP_PRINT("Scaler delete done!\n");

    app_delete_pre_proc_queued(&obj->preProcObj);
    APP_PRINT("Pre Proc delete done!\n");

    app_delete_tidl(&obj->tidlObj);
    APP_PRINT("TIDL delete done!\n");

    app_delete_dd_queued(&obj->drawDetObj);

    app_delete_img_mosaic(&obj->imgMosaicObj);
    APP_PRINT("Img Mosaic delete done!\n");

    app_delete_display(&obj->displayObj);
    APP_PRINT("Display delete done!\n");

    SAFETY_ENABLED_RUN(
        if (app_delete_graph_safety(obj) == VX_SUCCESS)
        {
            APP_PRINT("App delete graph safety done!\n");
        }
        else
        {
            printf("ERROR: App delete graph safety failed!");
        }
    );

    vxReleaseGraph(&obj->graph);
    APP_PRINT("Graphs delete done!\n");
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[APP_MAIN_GRAPH_NUM_PARAMS];

    vx_int32 graph_parameter_index = 0;

    if (obj == NULL)
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        obj->graph = vxCreateGraph(obj->context);
        status = vxGetStatus((vx_reference)obj->graph);
        if (status == VX_SUCCESS)
        {
            (void)vxSetReferenceName((vx_reference)obj->graph, "app_tidl_od_cam_safe_graph");
            APP_PRINT("Graph create done!\n");
        }
    }
    if (status == VX_SUCCESS)
    {
        SAFETY_ENABLED_RUN(status = app_create_verifier_graphs(obj));
        SAFETY_ENABLED_RUN(APP_PRINT("Verifier graph done!\n"));
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_capture(obj->graph, &obj->captureObj);
        APP_PRINT("Capture graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_viss_queued(obj->graph,
                                             &obj->vissObj,
                                             obj->captureObj.raw_image_arr[0],
                                             TIVX_TARGET_VPAC_VISS1);
        APP_PRINT("VISS graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_aewb_queued(obj->graph,
                                             &obj->aewbObj,
                                             &obj->vissObj.h3a_stats_arr_q[0],
                                             TIVX_TARGET_MCU2_0);
        APP_PRINT("AEWB graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_ldc(obj->graph,
                                      &obj->ldcObj,
                                      obj->vissObj.output_arr,
                                      TIVX_TARGET_VPAC_LDC1);
        APP_PRINT("LDC graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_scaler_queued(obj->context,
                                                obj->graph,
                                                &obj->scalerObj,
                                                obj->ldcObj.output_arr);
        APP_PRINT("Scaler graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_pre_proc_queued(obj->graph,
                                                  &obj->preProcObj,
                                                  obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].img[0],
                                                  "pre_proc_node",
                                                  TIVX_TARGET_MPU_0);
        APP_PRINT("Pre proc graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_tidl_queued(obj->context,
                                              obj->graph,
                                              &obj->tidlObj,
                                              obj->preProcObj.output_tensor_arr[0]);
        APP_PRINT("TIDL graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_dd_queued(obj->graph,
                                            &obj->drawDetObj,
                                            obj->tidlObj.output_tensor_arr_q[0][0],
                                            obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].arr[0],
                                            TIVX_TARGET_DSP2);
        APP_PRINT("Draw detections graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        vx_int32 idx = 0;
        obj->imgMosaicObj.input_arr[idx] = obj->drawDetObj.output_image_arr[0];
        idx++;
        obj->imgMosaicObj.num_inputs = idx;

        status = app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);
        APP_PRINT("Img Mosaic graph done!\n");
    }

    if (status == VX_SUCCESS)
    {
        status = app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);
        APP_PRINT("Display graph done!\n");
    }


    if (status == VX_SUCCESS)
    {
        /* ---------------------------------- Main graph parmaeters ---------------------------------- */
        graph_parameter_index = 0;
        add_graph_parameter_by_node_index(obj->graph, obj->captureObj.node, 1);
        obj->captureObj.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->captureObj.raw_image_arr[0];
        graph_parameter_index++;

#ifdef SAFETY_ENABLED

        add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 9);
        obj->vissObj.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.h3a_stats_q[0];
        graph_parameter_index++;

        add_graph_parameter_by_node_index(obj->graph, obj->aewbObj.node, 4);
        obj->aewbObj.graph_parameter_out_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->aewbObj.aewb_output[0];
        graph_parameter_index++;

        vx_int32 out_idx;
        vx_int32 node_param_idx;

        for (out_idx = 0; out_idx < MSC_OUT_MAX; out_idx++)
        {
            node_param_idx = out_idx + 1;

            if (obj->scalerObj.output_q[out_idx].img[0] != NULL)
            {
                add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, node_param_idx);

                obj->scalerObj.graph_parameter_idxs[out_idx] = graph_parameter_index;

                graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
                graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = obj->scalerObj.bufq_depth;
                graph_parameters_queue_params_list[graph_parameter_index].refs_list =
                    (vx_reference *)&obj->scalerObj.output_q[out_idx].img[0];

                graph_parameter_index++;
            }
        }

        add_graph_parameter_by_node_index(obj->graph, obj->preProcObj.node, 2);
        obj->preProcObj.graph_parameter_out_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->preProcObj.output_tensor[0][0];
        graph_parameter_index++;

        for (out_idx = 0; out_idx < (vx_int32)obj->tidlObj.num_output_tensors; out_idx++)
        {
            node_param_idx = (vx_int32)TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS +
                            (vx_int32)obj->tidlObj.num_input_tensors +
                            out_idx;

            add_graph_parameter_by_node_index(obj->graph, obj->tidlObj.node, node_param_idx);

            obj->tidlObj.graph_parameter_idxs[out_idx] = graph_parameter_index;

            graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list =
                (vx_reference *)&obj->tidlObj.output_tensor_q[out_idx][0];

            graph_parameter_index++;
        }

        add_graph_parameter_by_node_index(obj->graph, obj->drawDetObj.node, 3);
        obj->drawDetObj.graph_parameter_out_image_index = graph_parameter_index;

        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = obj->drawDetObj.bufq_depth;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list =
            (vx_reference *)&obj->drawDetObj.output_images[0];

        graph_parameter_index++;

#endif

        if (graph_parameter_index > APP_MAIN_GRAPH_NUM_PARAMS)
        {
            status = VX_FAILURE;
            printf("Error: Invalid number of graph params!");
        }

        if (status == VX_SUCCESS)
        {
            vxSetGraphScheduleConfig(obj->graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            graph_parameter_index,
            graph_parameters_queue_params_list);

            tivxSetGraphPipelineDepth(obj->graph, APP_PIPELINE_DEPTH);

            tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 6, APP_BUFFER_Q_DEPTH);

            tivxSetNodeParameterNumBufByIndex(obj->ldcObj.node, 7, APP_BUFFER_Q_DEPTH);

            tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, APP_BUFFER_Q_DEPTH);

#ifndef SAFETY_ENABLED

            tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 9, APP_BUFFER_Q_DEPTH);
            tivxSetNodeParameterNumBufByIndex(obj->aewbObj.node, 4, APP_BUFFER_Q_DEPTH);


            /*This output is accessed slightly later in the pipeline by mosaic node so queue depth is larger */
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 6);
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, 6);

            tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 2, APP_BUFFER_Q_DEPTH);

            tivxSetNodeParameterNumBufByIndex(obj->tidlObj.node, 7, APP_BUFFER_Q_DEPTH);

            tivxSetNodeParameterNumBufByIndex(obj->drawDetObj.node, 3, APP_BUFFER_Q_DEPTH);
#endif

            APP_PRINT("Pipeline params setup done!\n");
        }
    }

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    if (obj == NULL)
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph);

        if (status == VX_SUCCESS)
        {
            APP_PRINT("Graph verify SUCCESS!\n");
        }
        else
        {
            APP_PRINT("Graph verify FAILURE! (%d)\n", (int)status);
        }
    }

#if 1
    if (status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph, ".", "vx_app_tidl_od_cam_safe");

        if (status != VX_SUCCESS)
        {
            APP_PRINT("Graph export to DOT FAILURE! (%d)\n", (int)status);
        }
    }
#endif

    if (status == VX_SUCCESS)
    {
        if (obj->captureObj.enable_error_detection)
        {
            status = app_send_error_frame(&obj->captureObj);

            if (status != VX_SUCCESS)
            {
                APP_PRINT("App Send Error Frame FAILURE! (%d) enable=%d\n",
                           (int)status, obj->captureObj.enable_error_detection);
            }
            else
            {
                APP_PRINT("App Send Error Frame Done! %d\n",
                           obj->captureObj.enable_error_detection);
            }
        }
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    SAFETY_ENABLED_RUN(
        status = app_verify_verifier_graphs(obj);

        if (status == VX_SUCCESS)
        {
            APP_PRINT("App verify verifier graphs done!");
        }
        else
        {
            printf("ERROR: App verify verifier graphs failed!");
        }
    );

    return status;
}

static vx_status app_pipeup_graphs(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    CaptureObj *captureObj = &obj->captureObj;


    vx_uint32 q = obj->enqueueCnt;

    status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                             captureObj->graph_parameter_index,
                                             (vx_reference*)&captureObj->raw_image_arr[q],
                                             1);

#ifdef SAFETY_ENABLED

    AEWBObj    *aewbObj    = &obj->aewbObj;
    VISSObj    *vissObj    = &obj->vissObj;

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 vissObj->graph_parameter_index,
                                                 (vx_reference*)&vissObj->h3a_stats_q[q],
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 aewbObj->graph_parameter_out_index,
                                                 (vx_reference*)&aewbObj->aewb_output[q],
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphAewbVerifier,
                                                 obj->aewbObjVerifier.graph_parameter_out_index,
                                                 (vx_reference*)&obj->aewbObjVerifier.aewb_output[q],
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        for (vx_int32 out_idx = 0; out_idx < MSC_OUT_MAX; out_idx++)
        {
            if (obj->scalerObj.output_q[out_idx].img[0] != NULL)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                         obj->scalerObj.graph_parameter_idxs[out_idx],
                                                         (vx_reference *)&obj->scalerObj.output_q[out_idx].img[q],
                                                         1);
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                    obj->preProcObj.graph_parameter_out_index,
                                                    (vx_reference *)&obj->preProcObj.output_tensor[0][q],
                                                    1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphPreProcVerifier,
                                                 obj->preProcObjVerifier.graph_parameter_out_index,
                                                 (vx_reference*)&obj->preProcObjVerifier.output_tensor[0][q],
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        for (vx_int32 out_idx = 0; out_idx < (vx_int32)obj->tidlObj.num_output_tensors; out_idx++)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                     obj->tidlObj.graph_parameter_idxs[out_idx],
                                                     (vx_reference *)&obj->tidlObj.output_tensor_q[out_idx][q],
                                                     1);
            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                 obj->drawDetObj.graph_parameter_out_image_index,
                                                 (vx_reference *)&obj->drawDetObj.output_images[q],
                                                 1);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->graphDrawDetVerifier,
                                                 obj->drawDetObjVerifier.graph_parameter_out_image_index,
                                                (vx_reference *)&obj->drawDetObjVerifier.output_images[q],
                                                1);
    }

#endif

    if (status == VX_SUCCESS)
    {
        obj->enqueueCnt++;
        obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : obj->enqueueCnt;
        obj->pipeline++;
    }

    return status;
}

static void app_update_pipeline_counters(AppObj *obj)
{
    obj->enqueueCnt++;
    obj->dequeueCnt++;

    obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : obj->enqueueCnt;
    obj->dequeueCnt = (obj->dequeueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : obj->dequeueCnt;
}

static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_uint32 frame_id)
{
    vx_status status = VX_SUCCESS;
    vx_status return_status = VX_SUCCESS;
    uint32_t num_refs = 0U;

    appPerfPointBegin(&obj->total_perf);

    if (obj == NULL)
    {
        status = VX_FAILURE;
        return_status = VX_FAILURE;
    }

    if ((status == VX_SUCCESS) && (obj->pipeline <= 0))
    {
        status = app_pipeup_graphs(obj);
        if (return_status == VX_SUCCESS)
        {
            return_status = status;
        }
    }

    if ((status == VX_SUCCESS) && (obj->pipeline > 0))
    {
        vx_object_array capture_input = NULL;

        status = vxGraphParameterDequeueDoneRef(obj->graph,
                                                obj->captureObj.graph_parameter_index,
                                                (vx_reference *)&capture_input,
                                                1,
                                                &num_refs);
        if (return_status == VX_SUCCESS)
        {
            return_status = status;
        }

#ifdef SAFETY_ENABLED

        vx_user_data_object h3a_stats = NULL;
        vx_user_data_object aewb_results = NULL;

        vx_image scaler_output[MSC_OUT_MAX] = { (vx_image)0 };
        vx_tensor preproc_output = NULL;

        vx_tensor tidl_output[APP_MODULES_MAX_TENSORS] = { (vx_tensor)0 };
        vx_image draw_det_output = NULL;

        if (status == VX_SUCCESS)
        {
            status = app_dequeue_viss_aewb_from_main_graph(obj, &h3a_stats, &aewb_results);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        if ((status == VX_SUCCESS) && ((obj->test_option & APP_TEST_OPT_R5F_RC) != 0u))
        {
            status = app_schedule_aewb_verifier(obj, h3a_stats, aewb_results, frame_id);
            if (status != VX_SUCCESS)
            {
                if (return_status == VX_SUCCESS)
                {
                    return_status = status;
                }
                status = VX_SUCCESS;
            }
        }

        if (status == VX_SUCCESS)
        {
            status = app_reenqueue_viss_aewb_to_main_graph(obj, h3a_stats, aewb_results);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        if (status == VX_SUCCESS)
        {
            status = app_dequeue_scaler_preproc_from_main_graph(obj,
                                                                scaler_output,
                                                                &preproc_output);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        if ((status == VX_SUCCESS) && ((obj->test_option & APP_TEST_OPT_A72_RC) != 0u))
        {
            status = app_schedule_preproc_verifier(obj,
                                                   scaler_output[MSC_OUT_VERIFIER_PREPROC],
                                                   preproc_output,
                                                   frame_id);
            if (status != VX_SUCCESS)
            {
                if (return_status == VX_SUCCESS)
                {
                    return_status = status;
                }
                status = VX_SUCCESS;
            }
        }

        if (status == VX_SUCCESS)
        {
            status = app_dequeue_tidl_drawdet_from_main_graph(obj,
                                                              tidl_output,
                                                              &draw_det_output);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        if ((status == VX_SUCCESS) && ((obj->test_option & APP_TEST_OPT_C7X_RC) != 0u))
        {
            status = app_schedule_drawdet_verifier(obj,
                                                   tidl_output[0],
                                                   scaler_output[MSC_OUT_VERIFIER_PREPROC],
                                                   draw_det_output,
                                                   frame_id);
            if (status != VX_SUCCESS)
            {
                if (return_status == VX_SUCCESS)
                {
                    return_status = status;
                }
                status = VX_SUCCESS;
            }
        }

        if (status == VX_SUCCESS)
        {
            status = app_enqueue_scaler_preproc_to_main_graph(obj,
                                                              scaler_output,
                                                              preproc_output);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        if (status == VX_SUCCESS)
        {
            status = app_enqueue_tidl_drawdet_to_main_graph(obj,
                                                            tidl_output,
                                                            draw_det_output);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

#endif

        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph,
                                                     obj->captureObj.graph_parameter_index,
                                                     (vx_reference *)&capture_input,
                                                     1);
            if (return_status == VX_SUCCESS)
            {
                return_status = status;
            }
        }

        app_update_pipeline_counters(obj);
    }

    appPerfPointEnd(&obj->total_perf);

    return return_status;
}


static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = &obj->sensorObj;
    vx_uint32 frame_id = 0;
    int32_t ch_mask = obj->sensorObj.ch_mask;

    app_pipeline_params_defaults(obj);

    if ('\0' == sensorObj->sensor_name[0])
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = appStartImageSensor(sensorObj->sensor_name, ch_mask);
        APP_PRINT("appStartImageSensor returned with status: %d\n", status);
    }

    if (status == VX_SUCCESS)
    {
        for (frame_id = 0; frame_id < obj->num_frames_to_run; frame_id++)
        {
            status = app_run_graph_for_one_frame_pipeline(obj, frame_id);

            /* user asked to stop processing */
            if (obj->stop_task)
            {
                break;
            }

            /* If something failed, stop submitting more work */
            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        vx_status wait_status = vxWaitGraph(obj->graph);
        if (wait_status != VX_SUCCESS)
        {
            APP_PRINT("Error waiting for main graph completion: %d", wait_status);
            status = wait_status;
        }
        SAFETY_ENABLED_RUN(
            status = app_wait_verifier_graphs(obj, status);

            if (status == VX_SUCCESS)
            {
                APP_PRINT("App wait verifier graphs done!");
            }
            else
            {
                printf("ERROR: App wait verifier graphs failed!");
            }
        );
    }

    obj->stop_task = 1;

    /* Always attempt to stop sensor (best effort), but don't overwrite earlier error */
    {
        vx_status stop_status = appStopImageSensor(obj->sensorObj.sensor_name, ch_mask);

        if (status == VX_SUCCESS)
        {
            status = stop_status;
        }
    }

    return status;
}


static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 1;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->pipeline              = -APP_BUFFER_Q_DEPTH + 1;
    obj->enqueueCnt            = 0;
    obj->dequeueCnt            = 0;
}

static void set_sensor_defaults(SensorObj *sensorObj)
{
    strcpy(sensorObj->sensor_name, SENSOR_SONY_IMX390_UB953_D3);

    sensorObj->num_sensors_found = 0;
    sensorObj->sensor_features_enabled = 0;
    sensorObj->sensor_features_supported = 0;
    sensorObj->sensor_dcc_enabled = 0;
    sensorObj->sensor_wdr_enabled = 0;
    sensorObj->sensor_exp_control_enabled = 0;
    sensorObj->sensor_gain_control_enabled = 0;
    sensorObj->ch_mask = 1;
    sensorObj->enable_ldc = 1;
    sensorObj->num_cameras_enabled = 1;
    sensorObj->usecase_option = APP_SENSOR_FEATURE_CFG_UC0;
    sensorObj->is_interactive = 0;
}

static void set_scaler_defaults(ScalerObj *scalerObj)
{
    scalerObj->color_format = VX_DF_IMAGE_NV12;
}

static void set_pre_proc_defaults(PreProcObj *preProcObj)
{
    uint32_t i =0;

    preProcObj->params.skip_flag = 0;
    for (i = 0 ; i < 3; i++)
    {
        preProcObj->params.scale[i] = 1;
        preProcObj->params.mean[i] = 0;
    }
    preProcObj->params.channel_order = 0; /* 0-NCHW */

    for (i = 0 ; i < 4; i++)
    {
        preProcObj->params.crop[i] = 0;
    }
}

static void app_default_param_set(AppObj *obj)
{
    set_sensor_defaults(&obj->sensorObj);

    set_scaler_defaults(&obj->scalerObj);

    set_pre_proc_defaults(&obj->preProcObj);
    SAFETY_ENABLED_RUN(set_pre_proc_defaults(&obj->preProcObjVerifier));

    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->captureObj.enable_error_detection = 1; /* enable by default */
    obj->is_interactive = 1;
    obj->num_frames_to_run = 1000000000;
}

static vx_int32 calc_grid_size(vx_uint32 ch)
{
    if (0 == ch)
    {
        return -1;
    }
    else if (1 == ch)
    {
        return 1;
    }
    else if (4 >= ch)
    {
        return 2;
    }
    else if (9 >= ch)
    {
        return 3;
    }
    else if (16 >= ch)
    {
        return 4;
    }
    else
    {
        return -1;
    }
}

static void update_img_mosaic_defaults(ImgMosaicObj *imgMosaicObj, vx_uint32 in_width, vx_uint32 in_height, vx_int32 numCh)
{
    vx_int32 idx, ch;
    vx_int32 grid_size = calc_grid_size(numCh);
    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->num_inputs   = 1;

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

    idx = 0;
    for(ch = 0; ch < numCh; ch++)
    {
        vx_int32 startX, startY, winX, winY, winWidth, winHeight;

        winX = ch%grid_size;
        winY = ch/grid_size;

        if ((in_width * grid_size) >= imgMosaicObj->out_width)
        {
            winWidth = imgMosaicObj->out_width / grid_size;
            startX = 0;
        }
        else
        {
            winWidth = in_width;
            startX = (imgMosaicObj->out_width - (in_width * grid_size)) / 2;
        }

        if ((in_height * grid_size) >= imgMosaicObj->out_height)
        {
            winHeight = imgMosaicObj->out_height / grid_size;
            startY = 0;
        }
        else
        {
            winHeight = in_height;
            startY = (imgMosaicObj->out_height - (in_height * grid_size)) / 2;
        }

        imgMosaicObj->params.windows[idx].startX  = startX + (winWidth * winX);
        imgMosaicObj->params.windows[idx].startY  = startY + (winHeight * winY);
        imgMosaicObj->params.windows[idx].width   = winWidth;
        imgMosaicObj->params.windows[idx].height  = winHeight;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = idx;
        idx++;
    }

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = APP_BUFFER_Q_DEPTH;
}

static void update_draw_detections_defaults(AppObj *obj, DrawDetectionsObj *drawDetObj)
{
    vx_int32 i;

    drawDetObj->params.width  = obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].width;
    drawDetObj->params.height = obj->scalerObj.output_q[MSC_OUT_MAIN_PREPROC].height;

    for (i = 0; i < drawDetObj->params.num_classes; i++)
    {
        drawDetObj->params.color_map[i][0] = (i * 70)  % 256;
        drawDetObj->params.color_map[i][1] = (i * 130) % 256;
        drawDetObj->params.color_map[i][2] = (i * 200) % 256;
    }
}

static void app_update_param_set(AppObj *obj)
{
    obj->sensorObj.sensor_index = 0;

    SAFETY_ENABLED_RUN(app_update_param_set_safety(obj));

    update_draw_detections_defaults(obj, &obj->drawDetObj);

    update_img_mosaic_defaults(&obj->imgMosaicObj,
                               obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].width,
                               obj->scalerObj.output_q[MSC_OUT_VERIFIER_PREPROC].height,
                               obj->sensorObj.num_cameras_enabled);
}

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if (update_type == 0)
    {
        Draw2D_FontPrm sHeading;

        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 380, 5, "TIDL - Object Detection Demo (" APP_NAME_SUFFIX ")", &sHeading);
    }

    return;
}
