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
#include <TI/j7_kernels_imaging_aewb.h>
#include <TI/tivx_fileio_kernels.h>

#include "app_common.h"
#include "app_pipeline_config.h"
#include "app_modules.h"
#include "app_sensor_module.hpp"
#include "app_capture_module.hpp"
#include "app_viss_module.hpp"
#include "app_aewb_module.hpp"
#include "app_ldc_module.hpp"
#include "app_scaler_module.hpp"
#include "app_pre_proc_module.hpp"
#include "app_tidl_module.hpp"
#include "app_post_proc_module.hpp"
#include "app_img_mosaic_module.hpp"
#include "app_display_module.hpp"
#include "config/app_seg_cam_cfg_params.h"
#include <app_init.h>
#include <app_camera_pipeline_setup.h>

/* Color map for semantic segmentation visualization */
/* Format: YUV (each class gets one color) */
static uint8_t color_map_data[256][3] = {
    {152, 251, 152}, // class 0
    {0,   130, 180}, // class 1
    {220, 20,  60},  // class 2
    {3,   3,   251}, // class 3
    {190, 153, 153}, // class 4
    /* Rest initialized to 0 */
};

#define APP_BUFFER_Q_DEPTH  (4)

class AppObj {
public:
    /* Core graph object — owns context, graph, module_list, pipeline
     * state, perf points, file paths, task control, etc. */
    AppGraphObj graphObj;

    /* Camera pipeline modules */
    SensorObj    sensorObj;
    CaptureObj   captureObj;
    VISSObj      vissObj;
    AEWBObj      aewbObj;
    LDCObj       ldcObj;
    ScalerObj    scalerObj;

    /* DL pipeline modules */
    PreProcObj   preProcObj;
    TIDLObj      tidlObj;
    PostProcObj  postProcObj;
    ImgMosaicObj imgMosaicObj;
    DisplayObj   displayObj;

    /* Demo-specific config params */
    AppSegCamCfgParams cfgParams;

    /* Segmentation color map pointers (persistent storage) */
    uint8_t *color_map_ptrs[256];

    vx_status setup_graph();
    vx_status create_graph();

#ifndef x86_64
    static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif

    /* These are static so they match RunOneFrameFn (void *context, vx_int32).
     * The framework calls them via function pointer; inside, the void*
     * is cast back to AppObj*. */
    static vx_status run_one_frame_pipeline(void *context, vx_int32 frame_id);
};

static AppObj app;

/* ========================================================================== */
/*                          AppObj::setup_graph()                           */
/* ========================================================================== */

vx_status AppObj::setup_graph()
{
    vx_status status = VX_SUCCESS;

    // Initialize graph object
    status = this->graphObj.init();
    if (status != VX_SUCCESS)
    {
        APP_PRINTF("[APP-SEG-CAM] ERROR: Failed to initialize graph object!\n");
        return status;
    }

    // Load imaging kernels required for VISS, LDC, AEWB nodes
    tivxImagingLoadKernels(this->graphObj.context);
    tivxFileIOLoadKernels(this->graphObj.context);

    // Set graph configuration
    this->graphObj.start_frame           = 0;
    this->graphObj.num_frames            = this->cfgParams.num_frames_to_run;
    this->graphObj.num_iterations        = this->cfgParams.num_iterations;
    this->graphObj.is_interactive        = this->cfgParams.is_interactive;
    this->graphObj.delay_in_msecs        = this->cfgParams.delay_in_msecs;
    this->graphObj.enable_pipeline_flow  = 1;  // Camera always uses pipeline mode
    this->graphObj.buffer_q_depth        = APP_BUFFER_Q_DEPTH - 1;
    this->graphObj.perf_export_name      = "dl_demos_app_tidl_seg_cam";
    this->graphObj.demo_name             = "TIDL Camera Demo - Semantic Segmentation";
    this->graphObj.dot_export_name       = "vx_app_tidl_seg_cam";

    // For interactive mode, run indefinitely
    if (this->cfgParams.is_interactive)
    {
        this->graphObj.num_iterations = 1000000000;
    }

    // Set the run_one_frame callback
    this->graphObj.run_one_frame_context = this;
    this->graphObj.run_one_frame = AppObj::run_one_frame_pipeline;

    // Configure Sensor/Capture/VISS/AEWB/LDC camera front-end
    AppCameraPipelineCfg camCfg;
    camCfg.sensor_name             = this->cfgParams.sensor_name;
    camCfg.sensor_index            = this->cfgParams.sensor_index;
    camCfg.usecase_option          = this->cfgParams.usecase_option;
    camCfg.num_cameras_enabled     = this->cfgParams.num_cameras_enabled;
    camCfg.ch_mask                 = this->cfgParams.ch_mask;
    camCfg.enable_ldc              = this->cfgParams.enable_ldc;
    camCfg.is_interactive          = this->cfgParams.is_interactive;
    camCfg.enable_error_detection  = this->cfgParams.enable_error_detection;
    camCfg.en_out_capture_write    = this->cfgParams.en_out_capture_write;
    camCfg.output_file_path        = this->cfgParams.output_file_path;
    camCfg.en_out_viss_write       = this->cfgParams.en_out_viss_write;
    camCfg.en_out_ldc_write        = this->cfgParams.en_out_ldc_write;
    camCfg.bufq_depth              = APP_BUFFER_Q_DEPTH;

    status = app_setup_camera_pipeline_modules(this->sensorObj, this->captureObj,
                                               this->vissObj, this->aewbObj, this->ldcObj,
                                               camCfg);
    if (status != VX_SUCCESS)
    {
        APP_PRINTF("[APP-SEG-CAM] ERROR: Sensor query failed!\n");
        return status;
    }

    // Configure scaler module
    this->scalerObj.num_ch = this->sensorObj.num_cameras_enabled;
    this->scalerObj.num_outputs = 2;
    this->scalerObj.color_format = VX_DF_IMAGE_NV12;
    this->scalerObj.bufq_depth = APP_BUFFER_Q_DEPTH;
    // Input comes from LDC output — don't create separate input buffers
    this->scalerObj.external_input = vx_true_e;
    this->scalerObj.input_width  = this->ldcObj.table_width;
    this->scalerObj.input_height = this->ldcObj.table_height;
    this->scalerObj.en_out_scaler_write = this->cfgParams.en_out_scaler_write;
    strncpy(this->scalerObj.output_file_path, this->cfgParams.output_file_path.c_str(), APP_MAX_FILE_PATH-1);

    // output[0]: DL input size (feeds preProc → TIDL)
    this->scalerObj.output[0].width  = this->cfgParams.dl_width;
    this->scalerObj.output[0].height = this->cfgParams.dl_height;

    // output[1]: display size — full resolution (feeds ImgMosaic directly as overlay)
    this->scalerObj.output[1].width  = this->cfgParams.out_width;
    this->scalerObj.output[1].height = this->cfgParams.out_height;

    // Configure PreProc module
    this->preProcObj.num_ch = this->sensorObj.num_cameras_enabled;
    this->preProcObj.num_images = 1;

    // Configure TIDL module
    this->tidlObj.config_file_path = this->cfgParams.tidl_config;
    this->tidlObj.network_file_path = this->cfgParams.tidl_network;
    this->tidlObj.num_cameras = this->sensorObj.num_cameras_enabled;

    // Configure PostProc module (ioBufDesc filled after TIDL addNode below)
    this->postProcObj.num_ch = this->sensorObj.num_cameras_enabled;
    // Segmentation mode configuration
    this->postProcObj.params.task_type = TIVX_DL_POST_PROC_SEGMENTATION_TASK_TYPE;

    // Setup segmentation parameters
    sTIDL_IOBufDesc_t *ioBufDesc = &this->tidlObj.ioBufDesc;
    this->postProcObj.params.ss_prms.ioBufDesc = ioBufDesc;
    this->postProcObj.params.ss_prms.inDataWidth = ioBufDesc->outWidth[0];
    this->postProcObj.params.ss_prms.inDataHeight = ioBufDesc->outHeight[0];
    this->postProcObj.params.ss_prms.alpha = this->cfgParams.alpha;  /* Blending factor for overlay */

    /* Setup color map for segmentation (use persistent member storage) */
    for (int i = 0; i < 256; i++) {
        this->color_map_ptrs[i] = color_map_data[i];
    }
    this->postProcObj.params.ss_prms.YUVColorMap = this->color_map_ptrs;
    this->postProcObj.params.ss_prms.MaxColorClass = this->cfgParams.num_classes;

    this->postProcObj.output_img_width  = this->cfgParams.out_width;
    this->postProcObj.output_img_height = this->cfgParams.out_height;

    /* Configure ImgMosaic module — output is display size */
    vx_int32 num_cameras = this->sensorObj.num_cameras_enabled;
    this->imgMosaicObj.out_width  = DISPLAY_WIDTH;
    this->imgMosaicObj.out_height = DISPLAY_HEIGHT;
    this->imgMosaicObj.num_inputs = 1;
    this->imgMosaicObj.bufq_depth = APP_BUFFER_Q_DEPTH;

    app_layout_img_mosaic_windows(this->imgMosaicObj, num_cameras, /*clear_count=*/2);

    // Configure Display module
    this->displayObj.display_option = this->cfgParams.display_option;

    // Add modules to graph (camera pipeline first)
    if (this->graphObj.addNode(&this->sensorObj)   == NULL) return VX_FAILURE;
    if (this->graphObj.addNode(&this->captureObj)  == NULL) return VX_FAILURE;
    if (this->graphObj.addNode(&this->vissObj)     == NULL) return VX_FAILURE;
    if (this->graphObj.addNode(&this->aewbObj)     == NULL) return VX_FAILURE;
    if (this->graphObj.addNode(&this->ldcObj)      == NULL) return VX_FAILURE;
    if (this->graphObj.addNode(&this->scalerObj)   == NULL) return VX_FAILURE;

    // TIDL must be registered before PreProc/PostProc so ioBufDesc is populated
    APP_PRINTF("[SEG-CAM] adding tidl\n"); fflush(stdout);
    if (this->graphObj.addNode(&this->tidlObj)     == NULL) return VX_FAILURE;
    APP_PRINTF("[SEG-CAM] adding preProc\n"); fflush(stdout);
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->preProcObj.tidl_io_buf_desc = *this->tidlObj.getIOBufDesc();
    if (this->graphObj.addNode(&this->preProcObj)  == NULL) return VX_FAILURE;
    APP_PRINTF("[SEG-CAM] preProc OK\n"); fflush(stdout);

    // Wire preProc input: output[1] = DL size (quarter res)
    this->preProcObj.input_arr[0] = this->scalerObj.output[0].arr;
    for (vx_int32 i = 0; i < this->tidlObj.num_input_tensors; i++) {
        this->tidlObj.input_tensor_arr[i] = this->preProcObj.output_tensor_arr[i];
    }

    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->postProcObj.ioBufDesc = *this->tidlObj.getIOBufDesc();

    APP_PRINTF("[SEG-CAM] adding postProc\n"); fflush(stdout);
    if (this->graphObj.addNode(&this->postProcObj) == NULL) return VX_FAILURE;
    APP_PRINTF("[SEG-CAM] postProc OK\n"); fflush(stdout);

    // Wire postProc: TIDL output tensors + scaler output[1] as background (already at out_width x out_height)
    for (vx_int32 i = 0; i < this->tidlObj.num_output_tensors; i++) {
        this->postProcObj.input_tensor_arr[i] = this->tidlObj.output_tensor_arr[i];
    }
    this->postProcObj.input_arr = this->scalerObj.output[1].arr;

    if (this->graphObj.addNode(&this->imgMosaicObj) == NULL) return VX_FAILURE;

    this->imgMosaicObj.input_arr[0] = this->postProcObj.output_image_arr;

    if (this->graphObj.addNode(&this->displayObj)   == NULL) return VX_FAILURE;

    // Wire camera pipeline: capture → viss → aewb + ldc → scaler
    this->ldcObj.input_arr = this->vissObj.output_arr;
    this->scalerObj.input_arr[0] = this->ldcObj.output_arr;

    // Wire: display takes imgMosaic output
    this->displayObj.input_image = this->imgMosaicObj.output_image[0];

    APP_PRINTF("All nodes registered and initialized! \n");

    return VX_SUCCESS;
}

/* ========================================================================== */
/*                          AppObj::create_graph()                            */
/* ========================================================================== */

vx_status AppObj::create_graph()
{
    vx_status status = VX_SUCCESS;

    // Create the OpenVX graph
    status = this->graphObj.createGraph();
    if (status != VX_SUCCESS)
    {
        APP_PRINTF("[APP-SEG-CAM] ERROR: Failed to create graph!\n");
        return status;
    }

    // Pipeline configuration (conditionally on for segmentation apps)
    if (this->graphObj.enable_pipeline_flow && status == VX_SUCCESS)
    {
        PipelineConfig pipelineCfg(6); // APP_PIPELINE_DEPTH from app_tidl_seg

        // Graph parameters (queued inputs/outputs) for camera pipeline
        pipelineCfg.addGraphParam(this->captureObj.node, 1,
                                  (vx_reference*)&this->captureObj.raw_image_arr[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->captureObj.graph_parameter_index);

        // Node buffer depths
        pipelineCfg.addNodeBuf(this->vissObj.node,      /*output_image_index=*/6, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->vissObj.node,      /*h3a_stats_index=*/9, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->aewbObj.node,      /*aewb_output_index=*/4, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->ldcObj.node,       /*output_image_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output1_index=*/1, /*buffer_depth=*/6);
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output2_index=*/2, /*buffer_depth=*/6);
        pipelineCfg.addNodeBuf(this->preProcObj.node,   /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->tidlObj.node,      /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->postProcObj.node,  /*output_image_index=*/2, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->imgMosaicObj.node, /*output_image_index=*/1, /*buffer_depth=*/2);

        status = pipelineCfg.setup(this->graphObj.graph);
        if (status != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: Pipeline configuration failed!\n");
            return status;
        }
        APP_PRINTF("[APP-SEG-CAM] Pipeline configuration done!\n");
    }

    return status;
}

/* ========================================================================== */
/*                   AppObj::run_one_frame_pipeline()                         */
/* ========================================================================== */

vx_status AppObj::run_one_frame_pipeline(void *context, vx_int32 frame_id)
{
    AppObj *self = (AppObj *)context;
    vx_status status = VX_SUCCESS;
    uint32_t num_refs;

    appPerfPointBegin(&self->graphObj.total_perf);

    if (self->graphObj.pipeline < 0)
    {
        if (status == VX_SUCCESS)
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                         self->captureObj.graph_parameter_index,
                         (vx_reference*)&self->captureObj.raw_image_arr[self->graphObj.enqueueCnt], 1);
        self->graphObj.enqueueCnt++;
        self->graphObj.enqueueCnt = (self->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.enqueueCnt;
        self->graphObj.pipeline++;
    }

    if (self->graphObj.pipeline == 0)
    {
        if (status == VX_SUCCESS)
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                         self->captureObj.graph_parameter_index,
                         (vx_reference*)&self->captureObj.raw_image_arr[self->graphObj.enqueueCnt], 1);
        self->graphObj.enqueueCnt++;
        self->graphObj.enqueueCnt = (self->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.enqueueCnt;
        self->graphObj.pipeline++;
    }

    if (self->graphObj.pipeline > 0)
    {
        vx_image capture_input_image;

        if (status == VX_SUCCESS)
            status = vxGraphParameterDequeueDoneRef(self->graphObj.graph,
                         self->captureObj.graph_parameter_index,
                         (vx_reference*)&capture_input_image, 1, &num_refs);

        if (status == VX_SUCCESS)
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                         self->captureObj.graph_parameter_index,
                         (vx_reference*)&capture_input_image, 1);

        self->graphObj.enqueueCnt++;
        self->graphObj.dequeueCnt++;
        self->graphObj.enqueueCnt = (self->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.enqueueCnt;
        self->graphObj.dequeueCnt = (self->graphObj.dequeueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.dequeueCnt;
    }

    appPerfPointEnd(&self->graphObj.total_perf);
    return status;
}

/* ========================================================================== */
/*                                 main()                                     */
/* ========================================================================== */

int main(int argc, char *argv[])
{
    int status = appInit();

    if (status == 0)
    {
        vx_status vxStatus = VX_SUCCESS;

        // Parse config
        app.cfgParams.parse(argc, argv);

        // Update parameters
        vxStatus = app.setup_graph();
        if (vxStatus != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: setup_graph() failed!\n");
        }
        assert(vxStatus == VX_SUCCESS);

        // Create graph
        vxStatus = app.create_graph();
        if (vxStatus != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: create_graph() failed!\n");
        }
        assert(vxStatus == VX_SUCCESS);

#ifndef x86_64
        // Initialize graphics (if display option is enabled)
        if (app.cfgParams.display_option == 1)
        {
            app_grpx_init_prms_t grpx_prms;
            appGrpxInitParamsInit(&grpx_prms, app.graphObj.context);
            grpx_prms.draw_callback = AppObj::app_draw_graphics;
            appGrpxInit(&grpx_prms);
        }
#endif

        // Verify graph
        vxStatus = app.graphObj.verifyGraph();
        if (vxStatus != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: verifyGraph() failed!\n");
        }
        assert(vxStatus == VX_SUCCESS);

        // Start image sensor streaming
        vxStatus = app.sensorObj.startSensor();
        if (vxStatus != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: startSensor() failed!\n");
        }
        assert(vxStatus == VX_SUCCESS);

        // Run graph
        if (app.cfgParams.is_interactive)
        {
            vxStatus = app.graphObj.run(true);
        }
        else
        {
            vxStatus = app.graphObj.run(false);
        }

        app.sensorObj.stopSensor();

        if (vxStatus != VX_SUCCESS)
        {
            APP_PRINTF("[APP-SEG-CAM] ERROR: run() failed!\n");
        }
        assert(vxStatus == VX_SUCCESS);

#ifndef x86_64
        // Deinitialize graphics (if display option was enabled)
        if (app.cfgParams.display_option == 1)
        {
            appGrpxDeInit();
        }
#endif

        APP_PRINTF("[APP-SEG-CAM] Application completed successfully!\n");

        app.graphObj.deleteGraph();
        app.graphObj.deinit();

        appDeInit();
    }

    return status;
}

/* ========================================================================== */
/*                    Graphics Draw Callback                                  */
/* ========================================================================== */

#ifndef x86_64
void AppObj::app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if (update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 400, 5, (char*)"TIDL Camera Demo - Semantic Segmentation", &sHeading);
    }
}
#endif
