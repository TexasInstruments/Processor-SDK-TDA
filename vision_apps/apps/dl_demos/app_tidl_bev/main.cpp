/*
 * Copyright (c) 2026 Texas Instruments Incorporated
 * All rights reserved.
 */

#include <utils/draw2d/include/draw2d.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_get.h>
#include <utils/grpx/include/app_grpx.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>
#include <TI/hwa_kernels.h>

#include "app_common.h"
#include <app_modules_types.h>
#include <app_pipeline_config.h>
#include <app_scaler_module.hpp>
#include <app_pre_proc_module.hpp>
#include <app_tidl_module.hpp>
#include "bev_post_proc_module.h"
#include <app_img_mosaic_module.hpp>
#include <app_display_module.hpp>
#include "config/app_bev_cfg_params.h"
#include <app_init.h>

/* #define WRITE_INTERMEDIATE_OUTPUTS */

static char bev_img_labels[TIVX_DL_BEV_POST_PROC_MAX_NUM_CLASSNAMES][TIVX_DL_BEV_POST_PROC_MAX_SIZE_CLASSNAME] = {""};
static int32_t bev_label_offset[TIVX_DL_BEV_POST_PROC_MAX_NUM_CLASSNAMES] = {0};

#define LIDAR_XYCORD_PER_FRAME
#define APP_BUFFER_Q_DEPTH   (7)
#define APP_PIPELINE_DEPTH   (5)


#ifdef LIDAR_XYCORD_PER_FRAME
static vx_status readTensorInput(char* file_name, vx_object_array tensor_arr, vx_int32 num_ch);
static void app_find_lidar_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
#endif

/**
 * \brief Application object for the BEV demo with 6-batch processing and LiDAR
 *
 * Modern C++ structure following od_v2/avp2_v2 pattern while preserving
 * all custom BEV functionality (6-batch processing, LiDAR integration)
 */
class AppObj {
public:
    AppGraphObj graphObj;

    ScalerObj  scalerObj;
    ScalerObj  scalerObj_b2;
    ScalerObj  scalerObj_b3;
    ScalerObj  scalerObj_b4;
    ScalerObj  scalerObj_b5;
    ScalerObj  scalerObj_b6;
    ScalerObj  scalerObj_lidar_canvas;

    PreProcObj preProcObj;

    TIDLObj tidlObj;

    vx_tensor xycord_pipeline_refs[APP_BUFFER_Q_DEPTH] = {NULL};

    BEVPostProcObj postProcObj;
    BEVPostProcObj postProcObj_cam;

    ImgMosaicObj imgMosaicObj;

    DisplayObj displayObj;

    AppBevCfgParams cfgParams;

    /* ---- Methods ---- */
    vx_status setup_graph();
    vx_status create_graph();

    static vx_status run_one_frame_sequential(void *context, vx_int32 frame_id);
    static vx_status run_one_frame_pipeline(void *context, vx_int32 frame_id);

#ifndef x86_64
    static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
};

static AppObj app;

vx_status AppObj::setup_graph()
{
    vx_status status = VX_SUCCESS;

    /* ---- File I/O paths ---- */
    this->graphObj.input_file_path  = this->cfgParams.input_file_path;
    this->graphObj.output_file_path = this->cfgParams.output_file_path;

    /* ---- Graph run parameters ---- */
    this->graphObj.start_frame          = this->cfgParams.start_frame;
    this->graphObj.num_frames           = this->cfgParams.num_frames;
    this->graphObj.delay_in_msecs       = this->cfgParams.delay_in_msecs;
    this->graphObj.num_iterations       = this->cfgParams.num_iterations;
    this->graphObj.is_interactive       = this->cfgParams.is_interactive;
    this->graphObj.enable_pipeline_flow = this->cfgParams.enable_pipeline_flow;
    this->graphObj.dump_output          = this->cfgParams.dump_output;
    this->graphObj.buffer_q_depth       = APP_BUFFER_Q_DEPTH;

    this->graphObj.perf_export_name = "dl_demos_app_tidl_bev";
    this->graphObj.dot_export_name = "vx_app_tidl_bev";
    this->graphObj.demo_name = "TIDL Demo - BEV";

    /* Set the run_one_frame callback */
    this->graphObj.run_one_frame_context = this;
    this->graphObj.run_one_frame = this->cfgParams.enable_pipeline_flow ? AppObj::run_one_frame_pipeline : AppObj::run_one_frame_sequential;

    status = this->graphObj.init();
    if(status!= VX_SUCCESS){
        return status;
    }

    this->graphObj.buffer_q_depth = APP_BUFFER_Q_DEPTH;

    /* Configure all 6 batch scalers */
    ScalerObj* batch_scalers[] = {&this->scalerObj, &this->scalerObj_b2, &this->scalerObj_b3,
                                   &this->scalerObj_b4, &this->scalerObj_b5, &this->scalerObj_b6};

    for (int i = 0; i < 6; i++) {
        batch_scalers[i]->input_width = this->cfgParams.in_width;
        batch_scalers[i]->input_height = this->cfgParams.in_height;
        batch_scalers[i]->num_outputs = 2;
        for (int j = 0; j < batch_scalers[i]->num_outputs; j++) {
            batch_scalers[i]->output[j].width = this->cfgParams.dl_width;
            batch_scalers[i]->output[j].height = this->cfgParams.dl_height;
        }
    }

    /* LiDAR canvas scaler */
    this->scalerObj_lidar_canvas.input_width = this->cfgParams.in_width;
    this->scalerObj_lidar_canvas.input_height = 1080;
    this->scalerObj_lidar_canvas.num_outputs = 2;
    for (int i = 0; i < this->scalerObj_lidar_canvas.num_outputs; i++) {
        this->scalerObj_lidar_canvas.output[i].width = 1920;
        this->scalerObj_lidar_canvas.output[i].height = 600;
    }

    /* Configure TIDL paths */
    this->tidlObj.config_file_path = this->cfgParams.tidl_config;
    this->tidlObj.network_file_path = this->cfgParams.tidl_network;

    /* Enable XYCoordinate support for BEV */
#ifdef LIDAR_XYCORD_PER_FRAME
    this->tidlObj.num_cameras = 6;  /* BEV uses 6 cameras */
    this->tidlObj.enable_xycord = vx_true_e;
    this->tidlObj.xycord_size[0] = this->cfgParams.xycord_size;
#endif

    /* Configure PostProc */
    this->postProcObj.Lidar_2_img_file_path = this->cfgParams.tidl_xycordinate;
    this->postProcObj.lidar_points[0] = this->cfgParams.lidar_points;
    this->postProcObj.is_camera_mode = false;
    /* PostProc output dimensions match lidar_canvas scaler output */
    this->postProcObj.output_img_width = 1920;
    this->postProcObj.output_img_height = 600;
    this->postProcObj.params.od_prms.scaleX = this->postProcObj.output_img_width / 110.0;
    this->postProcObj.params.od_prms.scaleY = this->postProcObj.output_img_height / 110.0;
    this->postProcObj.params.od_prms.viz_th = this->cfgParams.viz_th;
    this->postProcObj.params.od_prms.formatter[0] = 0;
    this->postProcObj.params.od_prms.formatter[1] = 1;
    this->postProcObj.params.od_prms.formatter[2] = 2;
    this->postProcObj.params.od_prms.formatter[3] = 3;
    this->postProcObj.params.od_prms.formatter[4] = 5;
    this->postProcObj.params.od_prms.formatter[5] = 4;
    this->postProcObj.params.od_prms.labelIndexOffset = 0;
    this->postProcObj.params.od_prms.labelOffset = bev_label_offset;
    this->postProcObj.params.od_prms.classnames = bev_img_labels;

    this->postProcObj_cam.lidar2cam_points[0] = this->cfgParams.lidar2cam_points;
    this->postProcObj_cam.is_camera_mode = true;
    /* PostProc_cam output dimensions match first scaler output1 */
    this->postProcObj_cam.output_img_width = this->cfgParams.dl_width;
    this->postProcObj_cam.output_img_height = this->cfgParams.dl_height;
    this->postProcObj_cam.params.od_prms.scaleX = this->postProcObj_cam.output_img_width / 108.0;
    this->postProcObj_cam.params.od_prms.scaleY = this->postProcObj_cam.output_img_height / 55.0;
    this->postProcObj_cam.params.od_prms.viz_th = this->cfgParams.viz_th;
    this->postProcObj_cam.params.od_prms.formatter[0] = 0;
    this->postProcObj_cam.params.od_prms.formatter[1] = 1;
    this->postProcObj_cam.params.od_prms.formatter[2] = 2;
    this->postProcObj_cam.params.od_prms.formatter[3] = 3;
    this->postProcObj_cam.params.od_prms.formatter[4] = 5;
    this->postProcObj_cam.params.od_prms.formatter[5] = 4;
    this->postProcObj_cam.params.od_prms.labelIndexOffset = 0;
    this->postProcObj_cam.params.od_prms.labelOffset = bev_label_offset;
    this->postProcObj_cam.params.od_prms.classnames = bev_img_labels;

    /* ImgMosaic */
    this->imgMosaicObj.out_width = 1920;
    this->imgMosaicObj.out_height = 1080;
    this->imgMosaicObj.num_inputs = 2;  /* 2 inputs: camera postproc + lidar postproc */
    /* Configure MSC instance - use MSC1 as MSC0 is used by scalers */
    this->imgMosaicObj.params.num_msc_instances = 1;
    this->imgMosaicObj.params.msc_instance = 1;
    this->imgMosaicObj.params.clear_count = 8;

    struct {
        vx_int32 startX, startY, width, height, input_select, channel_select;
    } window_configs[] = {
        /* Camera back center - top middle */
        {641, 0, 639, 238, 0, 5},
        /* Camera right - top right */
        {1280, 0, 640, 238, 0, 2},
        /* Camera front left - top left */
        {0, 0, 640, 238, 0, 3},
        /* Camera front center - middle center */
        {641, 239, 639, 238, 0, 0},
        /* Camera left - middle left */
        {0, 239, 640, 238, 0, 1},
        /* Camera front right - middle right */
        {1280, 239, 640, 238, 0, 4},
        /* BEV output - bottom large */
        {300, 480, 1500, 600, 1, 0}
    };

    for (vx_int32 i = 0; i < 7; i++) {
        this->imgMosaicObj.params.windows[i].startX = window_configs[i].startX;
        this->imgMosaicObj.params.windows[i].startY = window_configs[i].startY;
        this->imgMosaicObj.params.windows[i].width = window_configs[i].width;
        this->imgMosaicObj.params.windows[i].height = window_configs[i].height;
        this->imgMosaicObj.params.windows[i].input_select = window_configs[i].input_select;
        this->imgMosaicObj.params.windows[i].channel_select = window_configs[i].channel_select;
    }

    this->imgMosaicObj.params.num_windows = 7;

    /* background_image will be created automatically in mosaic initModule if NULL */

    /* Display */
    this->displayObj.display_option = this->cfgParams.display_option;

    /* Register batch scalers (first scaler always added, rest based on batch_enable) */
    for (int i = 0; i < 6; i++) {
        if (i == 0 || this->cfgParams.batch_enable >= i) {
            batch_scalers[i]->num_ch = (i == 0) ? (NUM_CH + 5) : NUM_CH;  /* First scaler has 6 channels */
            batch_scalers[i]->bufq_depth = APP_BUFFER_Q_DEPTH;
            this->graphObj.addNode(batch_scalers[i]);
        }
    }
    this->scalerObj_lidar_canvas.num_ch = NUM_CH;
    this->scalerObj_lidar_canvas.bufq_depth = APP_BUFFER_Q_DEPTH;
    this->graphObj.addNode(&this->scalerObj_lidar_canvas);

    /* Register TIDL module (must be before PreProc/PostProc) */
    this->tidlObj.bufq_depth = APP_BUFFER_Q_DEPTH;
    this->graphObj.addNode(&this->tidlObj);

    /* Register PreProc module */
    this->preProcObj.num_images = 6;
    this->preProcObj.num_ch = 1;  /* 6 cameras already packed into one 4D tensor's batch dim; no node replication for BEV */
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->preProcObj.tidl_io_buf_desc = *this->tidlObj.getIOBufDesc();
    this->graphObj.addNode(&this->preProcObj);

    /* Register PostProc modules */
    this->postProcObj.num_ch = NUM_CH;
    this->postProcObj.bufq_depth = APP_BUFFER_Q_DEPTH;
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->postProcObj.tidl_config = this->tidlObj.config;
    this->graphObj.addNode(&this->postProcObj);

    this->postProcObj_cam.num_ch = NUM_CH + 5;
    this->postProcObj_cam.bufq_depth = APP_BUFFER_Q_DEPTH;
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->postProcObj_cam.tidl_config = this->tidlObj.config;
    this->graphObj.addNode(&this->postProcObj_cam);

    /* Register ImgMosaic and Display modules */
    this->imgMosaicObj.bufq_depth = APP_BUFFER_Q_DEPTH;
    this->graphObj.addNode(&this->imgMosaicObj);
    this->graphObj.addNode(&this->displayObj);

    /* ---- Setup connections ---- */

    /* Set PreProc inputs (6-batch) */
    this->preProcObj.input_arr[0] = this->scalerObj.output[0].arr;
    this->preProcObj.input_arr[1] = this->scalerObj_b2.output[0].arr;
    this->preProcObj.input_arr[2] = this->scalerObj_b3.output[0].arr;
    this->preProcObj.input_arr[3] = this->scalerObj_b4.output[0].arr;
    this->preProcObj.input_arr[4] = this->scalerObj_b5.output[0].arr;
    this->preProcObj.input_arr[5] = this->scalerObj_b6.output[0].arr;

    /* Set TIDL input tensors
     * input_tensor_arr[0] = Pre-proc output (6-batch images)
     * input_tensor_arr[1] = XYCoordinate (auto-connected by module) */
    this->tidlObj.input_tensor_arr[0] = this->preProcObj.output_tensor_arr[0];

    /* Set PostProc inputs */
    this->postProcObj.input_tensor_arr = this->tidlObj.output_tensor_arr[0];
    this->postProcObj.input_image_arr = this->scalerObj_lidar_canvas.output[0].arr;  /* Fixed: use lidar_canvas for non-camera mode */
    this->postProcObj_cam.input_tensor_arr = this->tidlObj.output_tensor_arr[0];
    this->postProcObj_cam.input_image_arr = this->scalerObj.output[1].arr;

    /* Set Mosaic inputs */
    vx_int32 idx = 0;
    this->imgMosaicObj.input_arr[idx++] = this->postProcObj_cam.output_arr[0];
    this->imgMosaicObj.input_arr[idx++] = this->postProcObj.output_arr[0];

    /* Set Display input */
    this->displayObj.input_image = this->imgMosaicObj.output_image[0];

    APP_PRINTF("All nodes registered and initialized!\n");

    return status;
}


vx_status AppObj::create_graph()
{
    vx_status status = this->graphObj.createGraph();

    if (this->cfgParams.enable_pipeline_flow && status == VX_SUCCESS)
    {
        PipelineConfig pipelineCfg(APP_PIPELINE_DEPTH);


        /* 7 Scaler inputs */
        pipelineCfg.addGraphParam(this->scalerObj.node, 0,
                                  (vx_reference*)&this->scalerObj.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_b2.node, 0,
                                  (vx_reference*)&this->scalerObj_b2.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_b2.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_b3.node, 0,
                                  (vx_reference*)&this->scalerObj_b3.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_b3.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_b4.node, 0,
                                  (vx_reference*)&this->scalerObj_b4.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_b4.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_b5.node, 0,
                                  (vx_reference*)&this->scalerObj_b5.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_b5.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_b6.node, 0,
                                  (vx_reference*)&this->scalerObj_b6.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_b6.graph_parameter_index);

        pipelineCfg.addGraphParam(this->scalerObj_lidar_canvas.node, 0,
                                  (vx_reference*)&this->scalerObj_lidar_canvas.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj_lidar_canvas.graph_parameter_index);

#ifdef LIDAR_XYCORD_PER_FRAME
        /* TIDL XYCoordinate input - extract tensors fresh for pipeline (stored in AppObj) */
        for (int q = 0; q < APP_BUFFER_Q_DEPTH; q++)
        {
            this->xycord_pipeline_refs[q] = (vx_tensor)vxGetObjectArrayItem(this->tidlObj.input_xycord_arr[q], 0);
        }
        pipelineCfg.addGraphParam(this->tidlObj.node, 7,
                                  (vx_reference*)&this->xycord_pipeline_refs[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->tidlObj.graph_parameter_index);

        /* PostProc lidar inputs */
        pipelineCfg.addGraphParam(this->postProcObj.node, 3,
                                  (vx_reference*)&this->postProcObj.input_lidar[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->postProcObj.graph_parameter_index);

        pipelineCfg.addGraphParam(this->postProcObj_cam.node, 3,
                                  (vx_reference*)&this->postProcObj_cam.input_lidar2cam[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->postProcObj_cam.graph_parameter_index);
#endif

        /* ImgMosaic output */
        vx_bool mosaic_param_en = ((this->cfgParams.en_out_img_write == 1) || (this->cfgParams.dump_output == 1))
                                  ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
        vx_bool mosaic_buf_en   = (mosaic_param_en == (vx_bool)vx_true_e)
                                  ? (vx_bool)vx_false_e : (vx_bool)vx_true_e;
        pipelineCfg.addGraphParam(this->imgMosaicObj.node, 1,
                                  (vx_reference*)&this->imgMosaicObj.output_image[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->imgMosaicObj.graph_parameter_index,
                                  mosaic_param_en);

        /* Per-node buffer depths */
        pipelineCfg.addNodeBuf(this->scalerObj.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_b2.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_b2.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_b3.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_b3.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_b4.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_b4.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_b5.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_b5.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_b6.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_b6.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->scalerObj_lidar_canvas.node, /*output1_index=*/1, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->scalerObj_lidar_canvas.node, /*output2_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->preProcObj.node, /*output_tensor_index=*/7, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->tidlObj.node, /*output_tensor_index=*/8, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->postProcObj.node, /*lidar_pointcloud_tensor_index=*/2, /*buffer_depth=*/8);
        pipelineCfg.addNodeBuf(this->postProcObj_cam.node, /*lidar_to_cam_tensor_index=*/2, /*buffer_depth=*/8);

        pipelineCfg.addNodeBuf(this->imgMosaicObj.node, /*output_image_index=*/1, /*buffer_depth=*/8, mosaic_buf_en);

        status = pipelineCfg.setup(this->graphObj.graph);
    }

    return status;
}

vx_status AppObj::run_one_frame_sequential(void *context, vx_int32 frame_id)
{
    AppObj *obj = (AppObj *)context;
    vx_status status = VX_SUCCESS;

    vx_char input_file_name_bc[APP_MAX_FILE_PATH];
    vx_char input_file_name_lc[APP_MAX_FILE_PATH];
    vx_char input_file_name_frc[APP_MAX_FILE_PATH];
    vx_char input_file_name_flc[APP_MAX_FILE_PATH];
    vx_char input_file_name_rc[APP_MAX_FILE_PATH];
    vx_char input_file_name_fc[APP_MAX_FILE_PATH];
    vx_char input_file_name7[APP_MAX_FILE_PATH];
#ifdef LIDAR_XYCORD_PER_FRAME
    vx_char input_file_namexycord[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2bc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2fc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2frc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2flc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2rc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2lc[APP_MAX_FILE_PATH];
#endif

    ScalerObj *scalerObj = &obj->scalerObj;
    ScalerObj *scalerObj_b2 = &obj->scalerObj_b2;
    ScalerObj *scalerObj_b3 = &obj->scalerObj_b3;
    ScalerObj *scalerObj_b4 = &obj->scalerObj_b4;
    ScalerObj *scalerObj_b5 = &obj->scalerObj_b5;
    ScalerObj *scalerObj_b6 = &obj->scalerObj_b6;
    ScalerObj *scalerObj_lidar_canvas = &obj->scalerObj_lidar_canvas;

#ifdef LIDAR_XYCORD_PER_FRAME
    TIDLObj *TIDLObj = &obj->tidlObj;
    BEVPostProcObj *BEVPostProcObj_cam = &obj->postProcObj_cam;
    BEVPostProcObj *BEVPostProcObj = &obj->postProcObj;
#endif

    snprintf(input_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_bc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_lc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_frc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_flc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_rc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_fc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name7, APP_MAX_FILE_PATH, "%s/black_image.yuv", obj->cfgParams.input_file_path.c_str());
#ifdef LIDAR_XYCORD_PER_FRAME
    snprintf(input_file_namexycord, APP_MAX_FILE_PATH, "%s/%010d_xy.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr, APP_MAX_FILE_PATH, "%s/%010d_ldr.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2lc, APP_MAX_FILE_PATH, "%s/%010d_ldr2lc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2frc, APP_MAX_FILE_PATH, "%s/%010d_ldr2frc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2flc, APP_MAX_FILE_PATH, "%s/%010d_ldr2flc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2bc, APP_MAX_FILE_PATH, "%s/%010d_ldr2bc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2rc, APP_MAX_FILE_PATH, "%s/%010d_ldr2rc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_nameldr2fc, APP_MAX_FILE_PATH, "%s/%010d_ldr2fc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
#endif

    appPerfPointBegin(&obj->graphObj.fileio_perf);
    readScalerInput(input_file_name_bc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name_lc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, 1);
    readScalerInput(input_file_name_frc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, 2);
    readScalerInput(input_file_name_flc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, 3);
    readScalerInput(input_file_name_rc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, 4);
    readScalerInput(input_file_name_fc, scalerObj->input_arr[0], APP_MODULES_READ_CHANNEL, 5);
    readScalerInput(input_file_name_lc, scalerObj_b2->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name_frc, scalerObj_b3->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name_flc, scalerObj_b4->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name_rc, scalerObj_b5->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name_fc, scalerObj_b6->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
    readScalerInput(input_file_name7, scalerObj_lidar_canvas->input_arr[0], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
#ifdef LIDAR_XYCORD_PER_FRAME
    readTensorInput(input_file_namexycord, TIDLObj->input_xycord_arr[0], (NUM_CH-1));
    readTensorInput(input_file_nameldr,BEVPostProcObj->input_lidar_arr[0],(NUM_CH-1));
    readTensorInput(input_file_nameldr2bc,BEVPostProcObj_cam->input_lidar2cam_arr[0],(NUM_CH-1));
    readTensorInput(input_file_nameldr2lc,BEVPostProcObj_cam->input_lidar2cam_arr[0],1);
    readTensorInput(input_file_nameldr2frc,BEVPostProcObj_cam->input_lidar2cam_arr[0],2);
    readTensorInput(input_file_nameldr2flc,BEVPostProcObj_cam->input_lidar2cam_arr[0],3);
    readTensorInput(input_file_nameldr2rc,BEVPostProcObj_cam->input_lidar2cam_arr[0],4);
    readTensorInput(input_file_nameldr2fc,BEVPostProcObj_cam->input_lidar2cam_arr[0],5);
#endif
    APP_PRINTF("App Scalar Reading done!\n");

    appPerfPointEnd(&obj->graphObj.fileio_perf);

    APP_PRINTF("App Reading Input Done!\n");

#ifdef x86_64
    APP_PRINTF("Processing files ...... \n %s\n %s\n %s\n %s\n %s\n %s\n",
            input_file_name_bc,input_file_name_fc,
            input_file_name_lc,input_file_name_rc,
            input_file_name_frc,input_file_name_flc);
#endif

    status = vxProcessGraph(obj->graphObj.graph);

#ifdef x86_64
    APP_PRINTF("Done!\n");
#endif   

    if((VX_SUCCESS == status) && (obj->cfgParams.en_out_img_write == 1))
    {
        vx_char output_file_name[APP_MAX_FILE_PATH];

        APP_PRINTF("App Writing Outputs Start...\n");

#ifdef WRITE_INTERMEDIATE_OUTPUTS
        vx_char output_file_name_bc[APP_MAX_FILE_PATH];
        vx_char output_file_name_lc[APP_MAX_FILE_PATH];
        vx_char output_file_name_flc[APP_MAX_FILE_PATH];
        vx_char output_file_name_frc[APP_MAX_FILE_PATH];
        vx_char output_file_name_rc[APP_MAX_FILE_PATH];
        vx_char output_file_name_fc[APP_MAX_FILE_PATH];

         snprintf(output_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_704x256_bc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_bc, scalerObj->output[0].arr);

         snprintf(output_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_704x256_lc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_lc, scalerObj_b2->output[0].arr);
         snprintf(output_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_704x256_flc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_flc, scalerObj_b3->output[0].arr);
         snprintf(output_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_704x256_frc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_frc, scalerObj_b4->output[0].arr);
         snprintf(output_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_704x256_rc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_rc, scalerObj_b5->output[0].arr);
         snprintf(output_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_704x256_fc.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         writeScalerOutput(output_file_name_fc, scalerObj_b6->output[0].arr);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/pre_proc_output_%010d", obj->cfgParams.output_file_path.c_str(), frame_id);
        writePreProcOutput(output_file_name, obj->preProcObj.output_tensor_arr);
#endif

         snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->cfgParams.output_file_path.c_str(), frame_id);
         obj->imgMosaicObj.writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);

        APP_PRINTF("App Writing Outputs Done!\n");
    }

    return status;
}
vx_status AppObj::run_one_frame_pipeline(void *context, vx_int32 frame_id)
{
    AppObj *obj = (AppObj *)context;
    vx_status status = VX_SUCCESS;
    vx_char input_file_name_bc[APP_MAX_FILE_PATH];
    vx_char input_file_name_lc[APP_MAX_FILE_PATH];
    vx_char input_file_name_frc[APP_MAX_FILE_PATH];
    vx_char input_file_name_flc[APP_MAX_FILE_PATH];
    vx_char input_file_name_rc[APP_MAX_FILE_PATH];
    vx_char input_file_name_fc[APP_MAX_FILE_PATH];
    vx_char input_file_name7[APP_MAX_FILE_PATH];
#ifdef LIDAR_XYCORD_PER_FRAME
    vx_char input_file_namexycord[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2bc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2fc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2frc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2flc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2rc[APP_MAX_FILE_PATH];
    vx_char input_file_nameldr2lc[APP_MAX_FILE_PATH];
#endif

    vx_int32 obj_array_idx = -1;
    vx_int32 obj_array_idx_b2 = -1;
    vx_int32 obj_array_idx_b3 = -1;
    vx_int32 obj_array_idx_b4 = -1;
    vx_int32 obj_array_idx_b5 = -1;
    vx_int32 obj_array_idx_b6 = -1;
    vx_int32 obj_array_idx_b7 = -1;
#ifdef LIDAR_XYCORD_PER_FRAME
    vx_int32 obj_array_idx_xycord = -1;
    vx_int32 obj_array_idx_ldr = -1;
    vx_int32 obj_array_idx_camldr = -1;
#endif

    ScalerObj    *scalerObj    = &obj->scalerObj;
    ScalerObj    *scalerObj_b2    = &obj->scalerObj_b2;
    ScalerObj    *scalerObj_b3    = &obj->scalerObj_b3;
    ScalerObj    *scalerObj_b4    = &obj->scalerObj_b4;
    ScalerObj    *scalerObj_b5    = &obj->scalerObj_b5;
    ScalerObj    *scalerObj_b6    = &obj->scalerObj_b6;
    ScalerObj    *scalerObj_lidar_canvas    = &obj->scalerObj_lidar_canvas;
#ifdef LIDAR_XYCORD_PER_FRAME
    TIDLObj *TIDLObj = &obj->tidlObj;
    BEVPostProcObj *BEVPostProcObj_cam = &obj->postProcObj_cam;
    BEVPostProcObj *BEVPostProcObj = &obj->postProcObj;
#endif

    ImgMosaicObj *imgMosaicObj = &obj->imgMosaicObj;

    snprintf(input_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_bc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_lc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_frc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_flc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_rc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_fc.nv12", obj->cfgParams.input_file_path.c_str(), frame_id);
    snprintf(input_file_name7, APP_MAX_FILE_PATH, "%s/black_image.yuv", obj->cfgParams.input_file_path.c_str());
#ifdef LIDAR_XYCORD_PER_FRAME
        snprintf(input_file_namexycord, APP_MAX_FILE_PATH, "%s/%010d_xy.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2lc, APP_MAX_FILE_PATH, "%s/%010d_ldr2lc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2frc, APP_MAX_FILE_PATH, "%s/%010d_ldr2frc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2flc, APP_MAX_FILE_PATH, "%s/%010d_ldr2flc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2bc, APP_MAX_FILE_PATH, "%s/%010d_ldr2bc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2rc, APP_MAX_FILE_PATH, "%s/%010d_ldr2rc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr2fc, APP_MAX_FILE_PATH, "%s/%010d_ldr2fc.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
        snprintf(input_file_nameldr, APP_MAX_FILE_PATH, "%s/%010d_ldr.bin", obj->cfgParams.input_file_path.c_str(), frame_id);
#endif

    if(obj->graphObj.pipeline < 0)
    {
        /* Enqueue outpus */
        if(((obj->cfgParams.en_out_img_write == 1) || (obj->graphObj.dump_output == 1))
            && (status == VX_SUCCESS))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->graphObj.enqueueCnt], 1);
        }

        appPerfPointBegin(&obj->graphObj.fileio_perf);
        /* Read input */
        readScalerInput(input_file_name_bc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name_lc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, 1);
        readScalerInput(input_file_name_frc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, 2);
        readScalerInput(input_file_name_flc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, 3);
        readScalerInput(input_file_name_rc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, 4);
        readScalerInput(input_file_name_fc, scalerObj->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, 5);
        readScalerInput(input_file_name_lc, scalerObj_b2->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name_frc, scalerObj_b3->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name_flc, scalerObj_b4->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name_rc, scalerObj_b5->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name_fc, scalerObj_b6->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
        readScalerInput(input_file_name7, scalerObj_lidar_canvas->input_arr[obj->graphObj.enqueueCnt], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
#ifdef LIDAR_XYCORD_PER_FRAME
            readTensorInput(input_file_namexycord, TIDLObj->input_xycord_arr[obj->graphObj.enqueueCnt], (NUM_CH-1));
            readTensorInput(input_file_nameldr,BEVPostProcObj->input_lidar_arr[obj->graphObj.enqueueCnt],(NUM_CH-1));
            readTensorInput(input_file_nameldr2bc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],(NUM_CH-1));
            readTensorInput(input_file_nameldr2lc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],1);
            readTensorInput(input_file_nameldr2frc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],2);
            readTensorInput(input_file_nameldr2flc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],3);
            readTensorInput(input_file_nameldr2rc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],4);
            readTensorInput(input_file_nameldr2fc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->graphObj.enqueueCnt],5);

#endif
        appPerfPointEnd(&obj->graphObj.fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj->graph_parameter_index, (vx_reference*)&obj->scalerObj.input_images[obj->graphObj.enqueueCnt], 1);
        }
            APP_PRINTF("Enque -%d - Success Scaler Image 1 !\n",obj->graphObj.enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&obj->scalerObj_b2.input_images[obj->graphObj.enqueueCnt], 1);

            APP_PRINTF("Enque -%d - Success Scaler Image 2 !\n",obj->graphObj.enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&obj->scalerObj_b3.input_images[obj->graphObj.enqueueCnt], 1);
            APP_PRINTF("Enque -%d - Success Scaler Image 3 !\n",obj->graphObj.enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&obj->scalerObj_b4.input_images[obj->graphObj.enqueueCnt], 1);
            APP_PRINTF("Enque -%d - Success Scaler Image 4 !\n",obj->graphObj.enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&obj->scalerObj_b5.input_images[obj->graphObj.enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&obj->scalerObj_b6.input_images[obj->graphObj.enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&obj->scalerObj_lidar_canvas.input_images[obj->graphObj.enqueueCnt], 1);
#ifdef LIDAR_XYCORD_PER_FRAME
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&obj->postProcObj.input_lidar[obj->graphObj.enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&obj->postProcObj_cam.input_lidar2cam[obj->graphObj.enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, TIDLObj->graph_parameter_index, (vx_reference*)&obj->xycord_pipeline_refs[obj->graphObj.enqueueCnt], 1);
#endif
            if(status == VX_SUCCESS)
            {
                APP_PRINTF("Enque -%d - Success Scaler Image 5, 6 & 7 !\n",obj->graphObj.enqueueCnt);
                APP_PRINTF("Enque -%d - Success lidar Tensor !\n",obj->graphObj.enqueueCnt);
            }
        obj->graphObj.enqueueCnt++;
        obj->graphObj.enqueueCnt   = (obj->graphObj.enqueueCnt  >= APP_BUFFER_Q_DEPTH)? 0 : obj->graphObj.enqueueCnt;
        obj->graphObj.pipeline++;
    }

    if(obj->graphObj.pipeline >= 0)
    {
        vx_image scaler_input_image;
        vx_image scaler_input_image2;
        vx_image scaler_input_image3;
        vx_image scaler_input_image4;
        vx_image scaler_input_image5;
        vx_image scaler_input_image6;
        vx_image scaler_input_image7;
        vx_image mosaic_output_image;
#ifdef LIDAR_XYCORD_PER_FRAME
        vx_tensor bev_tidl_input_xycord;
        vx_tensor bev_post_proc_input_lidar;
        vx_tensor bev_post_proc_input_cam_lidar;
#endif

        uint32_t num_refs;

        /* Dequeue input */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1, &num_refs);
        }

        APP_PRINTF("deque -%d - Success Scaler Image 1 !\n",obj->graphObj.enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&scaler_input_image2, 1, &num_refs);
        APP_PRINTF("deque -%d - Success Scaler Image 2 !\n",obj->graphObj.enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&scaler_input_image3, 1, &num_refs);
        APP_PRINTF("deque -%d - Success Scaler Image 3 !\n",obj->graphObj.enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&scaler_input_image4, 1, &num_refs);
        APP_PRINTF("deque -%d - Success Scaler Image 4 !\n",obj->graphObj.enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&scaler_input_image5, 1, &num_refs);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&scaler_input_image6, 1, &num_refs);
        status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&scaler_input_image7, 1, &num_refs);
#ifdef LIDAR_XYCORD_PER_FRAME
            status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, TIDLObj->graph_parameter_index, (vx_reference*)&bev_tidl_input_xycord, 1, &num_refs);
            status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&bev_post_proc_input_cam_lidar, 1, &num_refs);
            status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&bev_post_proc_input_lidar, 1, &num_refs);
#endif
        APP_PRINTF("deque -%d - Success Scaler Image 5,6,7 !\n",obj->graphObj.enqueueCnt);
        APP_PRINTF("deque -%d - Sucess Lidar Tensor !\n",obj->graphObj.enqueueCnt);

        if(((obj->cfgParams.en_out_img_write == 1)) && (status == VX_SUCCESS))
        {
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output */
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);
            }

            if (obj->cfgParams.en_out_img_write == 1 && (status == VX_SUCCESS))
            {
                APP_PRINTF("App Writing Outputs Start...\n");
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->cfgParams.output_file_path.c_str(), (frame_id - APP_BUFFER_Q_DEPTH));
                status = obj->imgMosaicObj.writeMosaicOutput(output_file_name, mosaic_output_image);
                APP_PRINTF("App Writing Outputs Done!\n");
            }

            /* Enqueue output */
            if(status ==  VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1);
            }
        }

        /* Offline output dumping (--dump): write the mosaic output only for the
         * fixed set of target frame indices tracked by AppGraphObj::shouldDumpFrame(). */
        if (obj->graphObj.dump_output == 1)
        {
            vx_bool dump_this_frame = obj->graphObj.shouldDumpFrame(frame_id);
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output (skip if already dequeued above by en_out_img_write) */
            if ((obj->cfgParams.en_out_img_write != 1) && (status == VX_SUCCESS))
            {
                status = vxGraphParameterDequeueDoneRef(obj->graphObj.graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);
            }
            if ((status == VX_SUCCESS) && (dump_this_frame == (vx_bool)vx_true_e))
            {
                printf("Dumping output for frame id : %d\n", frame_id);
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->cfgParams.output_file_path.c_str(), (frame_id - APP_BUFFER_Q_DEPTH));
                status = obj->imgMosaicObj.writeMosaicOutput(output_file_name, mosaic_output_image);
                printf("App Dumping Outputs Done!\n");
            }
            /* Enqueue output back (skip if already enqueued above by en_out_img_write) */
            if ((obj->cfgParams.en_out_img_write != 1) && (status == VX_SUCCESS))
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1);
            }
        }

        appPerfPointBegin(&obj->graphObj.fileio_perf);

        if (status == VX_SUCCESS)
        {
            AppGraphObj::findObjectArrayIndex(scalerObj->input_arr, (vx_reference)scaler_input_image, APP_BUFFER_Q_DEPTH, &obj_array_idx);
        }

        if((obj_array_idx != -1) && (status == VX_SUCCESS))
        {
            status = readScalerInput(input_file_name_bc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
            status = readScalerInput(input_file_name_lc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, 1);
            status = readScalerInput(input_file_name_frc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, 2);
            status = readScalerInput(input_file_name_flc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, 3);
            status = readScalerInput(input_file_name_rc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, 4);
            status = readScalerInput(input_file_name_fc, scalerObj->input_arr[obj_array_idx], APP_MODULES_READ_CHANNEL, 5);
        }
        if((obj->cfgParams.batch_enable >=1) && (status == VX_SUCCESS))
        {
            AppGraphObj::findObjectArrayIndex(scalerObj_b2->input_arr, (vx_reference)scaler_input_image2, APP_BUFFER_Q_DEPTH, &obj_array_idx_b2);

            if((obj_array_idx_b2 != -1) && (status == VX_SUCCESS))
            {
                status = readScalerInput(input_file_name_lc, scalerObj_b2->input_arr[obj_array_idx_b2], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
            }

            if((obj->cfgParams.batch_enable >=2) && (status == VX_SUCCESS))
            {
                AppGraphObj::findObjectArrayIndex(scalerObj_b3->input_arr, (vx_reference)scaler_input_image3, APP_BUFFER_Q_DEPTH, &obj_array_idx_b3);

                if((obj_array_idx_b3 != -1) && (status == VX_SUCCESS))
                {
                    status = readScalerInput(input_file_name_frc, scalerObj_b3->input_arr[obj_array_idx_b3], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
                }

                if((obj->cfgParams.batch_enable >=3) && (status == VX_SUCCESS))
                {
                    AppGraphObj::findObjectArrayIndex(scalerObj_b4->input_arr, (vx_reference)scaler_input_image4, APP_BUFFER_Q_DEPTH, &obj_array_idx_b4);

                    if((obj_array_idx_b4 != -1) && (status == VX_SUCCESS))
                    {
                        status = readScalerInput(input_file_name_flc, scalerObj_b4->input_arr[obj_array_idx_b4], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
                    }

                    if((obj->cfgParams.batch_enable >=5) && (status == VX_SUCCESS))
                    {
                        AppGraphObj::findObjectArrayIndex(scalerObj_b5->input_arr, (vx_reference)scaler_input_image5, APP_BUFFER_Q_DEPTH, &obj_array_idx_b5);

                        if((obj_array_idx_b5 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name_rc, scalerObj_b5->input_arr[obj_array_idx_b5], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
                        }

                        AppGraphObj::findObjectArrayIndex(scalerObj_b6->input_arr, (vx_reference)scaler_input_image6, APP_BUFFER_Q_DEPTH, &obj_array_idx_b6);
                        if((obj_array_idx_b6 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name_fc, scalerObj_b6->input_arr[obj_array_idx_b6], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
                        }
                        AppGraphObj::findObjectArrayIndex(scalerObj_lidar_canvas->input_arr, (vx_reference)scaler_input_image7, APP_BUFFER_Q_DEPTH, &obj_array_idx_b7);
                        if((obj_array_idx_b7 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name7, scalerObj_lidar_canvas->input_arr[obj_array_idx_b7], APP_MODULES_READ_CHANNEL, (NUM_CH-1));
                        }
#ifdef LIDAR_XYCORD_PER_FRAME
                        app_find_lidar_object_array_index(TIDLObj->input_xycord_arr, (vx_reference)bev_tidl_input_xycord, APP_BUFFER_Q_DEPTH, &obj_array_idx_xycord);
                        if((obj_array_idx_xycord != -1) && (status == VX_SUCCESS))
                        {
                            status = readTensorInput(input_file_namexycord, TIDLObj->input_xycord_arr[obj_array_idx_xycord], (NUM_CH-1));
                        }
                        app_find_lidar_object_array_index(BEVPostProcObj->input_lidar_arr, (vx_reference)bev_post_proc_input_lidar, APP_BUFFER_Q_DEPTH, &obj_array_idx_ldr);
                        if((obj_array_idx_ldr != -1) && (status == VX_SUCCESS))
                        {
                            status = readTensorInput(input_file_nameldr, BEVPostProcObj->input_lidar_arr[obj_array_idx_ldr], (NUM_CH-1));
                        }
                        app_find_lidar_object_array_index(BEVPostProcObj_cam->input_lidar2cam_arr, (vx_reference)bev_post_proc_input_cam_lidar, APP_BUFFER_Q_DEPTH, &obj_array_idx_camldr);
                        if((obj_array_idx_camldr != -1) && (status == VX_SUCCESS))
                        {
                            status = readTensorInput(input_file_nameldr2bc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],(NUM_CH-1));
                            status = readTensorInput(input_file_nameldr2lc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],1);
                            status = readTensorInput(input_file_nameldr2frc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],2);
                            status = readTensorInput(input_file_nameldr2flc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],3);
                            status = readTensorInput(input_file_nameldr2rc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],4);
                            status = readTensorInput(input_file_nameldr2fc,BEVPostProcObj_cam->input_lidar2cam_arr[obj_array_idx_camldr],5);

                        }
#endif
                    }
                }
            }
        }
        appPerfPointEnd(&obj->graphObj.fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

    /* Enqueue input - start execution */
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1);
        }
        if((obj->cfgParams.batch_enable >=1) && (status == VX_SUCCESS))
        {
            APP_PRINTF("enque -%d - Success Scaler Image 1!\n",obj->graphObj.enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&scaler_input_image2, 1);

            if((obj->cfgParams.batch_enable >=2) && (status == VX_SUCCESS))
            {
                APP_PRINTF("enque -%d - Success Scaler Image 2!\n",obj->graphObj.enqueueCnt);
                status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&scaler_input_image3, 1);

                if((obj->cfgParams.batch_enable >=3) && (status == VX_SUCCESS))
                {
                    APP_PRINTF("enque -%d - Success Scaler Image 3 !\n",obj->graphObj.enqueueCnt);
                    status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&scaler_input_image4, 1);
                    if((obj->cfgParams.batch_enable >=5) && (status == VX_SUCCESS))
                    {
                        APP_PRINTF("enque -%d - Success Scaler Image 4 !\n",obj->graphObj.enqueueCnt);
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&scaler_input_image5, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&scaler_input_image6, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&scaler_input_image7, 1);
#ifdef LIDAR_XYCORD_PER_FRAME
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, TIDLObj->graph_parameter_index, (vx_reference*)&obj->xycord_pipeline_refs[obj->graphObj.enqueueCnt], 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&bev_post_proc_input_cam_lidar, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graphObj.graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&bev_post_proc_input_lidar, 1);
#endif
                    }
                    APP_PRINTF("enque -%d - Success Scaler Image 5,6,7 !\n",obj->graphObj.enqueueCnt);
                    APP_PRINTF("enque -%d - Success Lidar Tensor !\n",obj->graphObj.enqueueCnt);
                }

            }

        }
        obj->graphObj.enqueueCnt++;
        obj->graphObj.dequeueCnt++;

        obj->graphObj.enqueueCnt = (obj->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->graphObj.enqueueCnt;
        obj->graphObj.dequeueCnt = (obj->graphObj.dequeueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->graphObj.dequeueCnt;

    }

    appPerfPointEnd(&obj->graphObj.total_perf);

    APP_PRINTF("App Process Graph Done!\n");

    return status;
}

/**
 * Main entry point
 */
int main(int argc, char *argv[])
{
    int status = appInit();

    if (status == 0)
    {
        /* Parse configuration */
        app.cfgParams.parse(argc, argv);

        /* Wire cfgParams into module structs and initialize graph */
        status = app.setup_graph();
        APP_PRINTF("App setup_graph Done (config + init + addNode)!\n");
        assert(status == VX_SUCCESS);

        /* Create graph */
        status = app.create_graph();
        APP_PRINTF("App Create Graph Done!\n");
        assert(status == VX_SUCCESS);

#ifndef x86_64
        if(app.displayObj.display_option == 1 && app.cfgParams.enable_gui)
        {
            /* Initialize graphics */
            app_grpx_init_prms_t grpx_prms;
            appGrpxInitParamsInit(&grpx_prms, app.graphObj.context);
            grpx_prms.draw_callback = AppObj::app_draw_graphics;
            appGrpxInit(&grpx_prms);
        }
#endif

        /* Verify graph */
        status = app.graphObj.verifyGraph();
        APP_PRINTF("App Verify Graph Done!\n");
        assert(status == VX_SUCCESS);

        /* Run graph */
        status = app.graphObj.run(app.cfgParams.is_interactive);
        assert(status == VX_SUCCESS);

        APP_PRINTF("App Run Graph Done!\n");

#ifndef x86_64
        if(app.displayObj.display_option == 1 && app.cfgParams.enable_gui)
        {
            /* Deinitialize graphics */
            appGrpxDeInit();
        }
#endif

        app.graphObj.deleteGraph();
        app.graphObj.deinit();

        appDeInit();
    }

    return status;
}

#ifdef LIDAR_XYCORD_PER_FRAME
/**
 * Helper function to find tensor object array index
 */
static void app_find_lidar_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx)
{
    vx_int32 i;

    *array_idx = -1;
    for(i = 0; i < array_size; i++)
    {
        vx_tensor tnsr_ref = (vx_tensor)vxGetObjectArrayItem((vx_object_array)object_array[i], 0);
        if(ref == (vx_reference)tnsr_ref)
        {
            *array_idx = i;
            vxReleaseTensor(&tnsr_ref);
            break;
        }
        vxReleaseTensor(&tnsr_ref);
    }
}

/**
 * Helper function to read tensor input from file
 */
static vx_status readTensorInput(char* file_name, vx_object_array tensor_arr, vx_int32 num_ch)
{
    vx_status status;
    status = vxGetStatus((vx_reference)tensor_arr);
    vx_size start[1];
    vx_size strides[1];

    vx_uint32  capacity;
    vx_size read_count;
    vx_size input_dims_num=1;
    vx_size input_dims[1];

    if(status == VX_SUCCESS)
    {
        FILE *fp = fopen(file_name,"rb");
        vx_size tensor_len;
        vx_int32 i;
        vx_int32 *lidar_tensor_buffer = NULL;
        vx_tensor   in_tensor;

        if(fp == NULL)
        {
            APP_PRINTF("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        vxQueryObjectArray(tensor_arr, VX_OBJECT_ARRAY_NUMITEMS, &tensor_len, sizeof(vx_size));

        for (i = 0; i < 1; i++)
        {
            fseek(fp, 0, SEEK_END);
            capacity = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            in_tensor = (vx_tensor)vxGetObjectArrayItem(tensor_arr, num_ch);

            if(status == VX_SUCCESS)
            {
                APP_PRINTF("Success:--read file %s - capacity- %u\n",file_name+33,capacity);
            }
            if(status != VX_SUCCESS)
            {
                APP_PRINTF("Fail:--read file %s -capacity- %u\n",file_name+33,capacity);
            }
            lidar_tensor_buffer = (vx_int32*)malloc(sizeof(vx_float32)*capacity);

            read_count = fread(lidar_tensor_buffer, capacity, 1, fp);
            if(read_count != 1)
            {
                APP_PRINTF("Unable to read file %s \n", file_name);
                return 0;
            }

            strides[0] =  sizeof(vx_int32);
            input_dims[0] = (capacity/strides[0]);
            start[0] = 0;

            status = vxCopyTensorPatch(in_tensor, input_dims_num, start, input_dims, strides, lidar_tensor_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        }
        fclose(fp);
        free(lidar_tensor_buffer);

    }

    return(status);
}
#endif /* LIDAR_XYCORD_PER_FRAME */

#ifndef x86_64
void AppObj::app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        Draw2D_FontPrm sLabel;

        sHeading.fontIdx = 0;
        Draw2D_drawString(handle, 560, 5, (char*)"Analytics for BIRD EYE VIEW", &sHeading);

        sLabel.fontIdx = 2;
        Draw2D_drawString(handle, 340, 90, (char*)"Front left camera", &sLabel);
        Draw2D_drawString(handle, 920, 90, (char*)"Front camera", &sLabel);
        Draw2D_drawString(handle, 1520,90, (char*)"Front right camera", &sLabel);
        Draw2D_drawString(handle, 340, 300, (char*)"Left  camera", &sLabel);
        Draw2D_drawString(handle, 920, 300, (char*)"Back camera", &sLabel);
        Draw2D_drawString(handle, 1520,300, (char*)"Right camera", &sLabel);

        sLabel.fontIdx = 3;

        Draw2D_drawString(handle, 25, 828, (char*)"Object", &sLabel);
        Draw2D_drawString(handle, 15, 848, (char*)"Detection", &sLabel);
        Draw2D_drawString(handle, 15, 868, (char*)"(704x256)", &sLabel);
    }
    return;
}
#endif
