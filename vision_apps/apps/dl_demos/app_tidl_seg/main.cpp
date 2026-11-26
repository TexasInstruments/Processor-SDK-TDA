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

#ifdef APP_TIDL_TRACE_DUMP
#include <tivx_utils_tidl_trace.h>
#endif

#include "app_common.h"
#include "app_pipeline_config.h"
#include "app_pipeline_config.h"
#include "app_modules.h"
#include "app_scaler_module.hpp"
#include "app_pre_proc_module.hpp"
#include "app_tidl_module.hpp"
#include "app_img_mosaic_module.hpp"
#include "app_display_module.hpp"
#include "app_post_proc_module.hpp"
#include "config/app_seg_cfg_params.h"
#include <app_init.h>

/* #define WRITE_INTERMEDIATE_OUTPUTS */

#define APP_BUFFER_Q_DEPTH   (2)
#define APP_PIPELINE_DEPTH   (6)

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

class AppObj {
public:
    /* Core graph object */
    AppGraphObj graphObj;

    /* Module-specific objects */
    ScalerObj    scalerObj;
    PreProcObj   preProcObj;
    TIDLObj      tidlObj;
    PostProcObj  postProcObj;
    ImgMosaicObj imgMosaicObj;
    DisplayObj   displayObj;

    /* Demo-specific config params */
    AppSegCfgParams cfgParams;

    /* Segmentation color map pointers (persistent storage) */
    uint8_t *color_map_ptrs[256];

    vx_status parse_cmd_line_args(vx_int32 argc, vx_char *argv[]);
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
    this->graphObj.dot_export_name = "vx_app_tidl_seg";

    /* File I/O paths */
    this->graphObj.input_file_path  = this->cfgParams.input_file_path;
    this->graphObj.output_file_path = this->cfgParams.output_file_path;

    /* Graph run parameters */
    this->graphObj.start_frame          = this->cfgParams.start_frame;
    this->graphObj.num_frames           = this->cfgParams.num_frames;
    this->graphObj.delay_in_msecs       = this->cfgParams.delay_in_msecs;
    this->graphObj.num_iterations       = this->cfgParams.num_iterations;
    this->graphObj.is_interactive       = this->cfgParams.is_interactive;
    this->graphObj.enable_pipeline_flow = this->cfgParams.enable_pipeline_flow;
    this->graphObj.dump_output          = this->cfgParams.dump_output;
    this->graphObj.buffer_q_depth       = APP_BUFFER_Q_DEPTH;
    this->graphObj.perf_export_name = "dl_demos_app_tidl_seg";
    this->graphObj.demo_name = "TIDL Demo - Semantic Segmentation";

    /* Interactive mode: run indefinitely until user presses 'x' */
    if (this->cfgParams.is_interactive) {
        this->graphObj.num_iterations = 1000000000;
    }

    /* Set the run_one_frame callback */
    this->graphObj.run_one_frame_context = this;
    this->graphObj.run_one_frame = this->cfgParams.enable_pipeline_flow ?
                                    AppObj::run_one_frame_pipeline : AppObj::run_one_frame_sequential;

    /* Initialize graph context */
    vx_status status = this->graphObj.init();
    if (status != VX_SUCCESS) return status;
    APP_PRINTF("Graph Initialize Done! \n");

    /* Scaler config + register */
    this->scalerObj.input_width  = this->cfgParams.in_width;
    this->scalerObj.input_height = this->cfgParams.in_height;
    this->scalerObj.bufq_depth   = APP_BUFFER_Q_DEPTH;
    this->scalerObj.num_outputs  = 2;
    this->scalerObj.output[0].width  = this->cfgParams.dl_width;
    this->scalerObj.output[0].height = this->cfgParams.dl_height;
    this->scalerObj.output[1].width  = this->cfgParams.out_width;
    this->scalerObj.output[1].height = this->cfgParams.out_height;
    if (this->graphObj.addNode(&this->scalerObj) == NULL) return VX_FAILURE;

    /* TIDL config + register (before PreProc/PostProc) */
    this->tidlObj.config_file_path  = this->cfgParams.tidl_config;
    this->tidlObj.network_file_path = this->cfgParams.tidl_network;
    if (this->graphObj.addNode(&this->tidlObj) == NULL) return VX_FAILURE;

    /* PreProc register */
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->preProcObj.tidl_io_buf_desc = *this->tidlObj.getIOBufDesc();
    if (this->graphObj.addNode(&this->preProcObj) == NULL) return VX_FAILURE;

    /* Wire: preProc takes scaler output[0]; tidl takes preProc output tensors */
    this->preProcObj.input_arr[0] = this->scalerObj.output[0].arr;
    for (vx_int32 i = 0; i < this->tidlObj.num_input_tensors; i++) {
        this->tidlObj.input_tensor_arr[i] = this->preProcObj.output_tensor_arr[i];
    }

    /* PostProc config + register (after TIDL) */
    /* Setup segmentation-specific parameters */
    this->postProcObj.params.task_type = TIVX_DL_POST_PROC_SEGMENTATION_TASK_TYPE;

    /* Setup segmentation parameters */
    sTIDL_IOBufDesc_t *ioBufDesc = &this->tidlObj.ioBufDesc;
    this->postProcObj.params.ss_prms.ioBufDesc = ioBufDesc;
    this->postProcObj.params.ss_prms.inDataWidth = ioBufDesc->outWidth[0];
    this->postProcObj.params.ss_prms.inDataHeight = ioBufDesc->outHeight[0];
    this->postProcObj.params.ss_prms.alpha = 0.4f;  /* Blending factor for overlay */

    /* Setup color map for segmentation (use persistent member storage) */
    for (int i = 0; i < 256; i++) {
        this->color_map_ptrs[i] = color_map_data[i];
    }
    this->postProcObj.params.ss_prms.YUVColorMap = this->color_map_ptrs;
    this->postProcObj.params.ss_prms.MaxColorClass = this->cfgParams.num_classes;

    this->postProcObj.output_img_width  = this->cfgParams.out_width;
    this->postProcObj.output_img_height = this->cfgParams.out_height;
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->postProcObj.ioBufDesc = *this->tidlObj.getIOBufDesc();

    if (this->graphObj.addNode(&this->postProcObj) == NULL) return VX_FAILURE;

    /* Wire: postProc takes tidl output + scaler output[1] */
    for (int i = 0; i < this->tidlObj.ioBufDesc.numOutputBuf; i++)
    {
        this->postProcObj.input_tensor_arr[i] = this->tidlObj.output_tensor_arr[i];
    }
    this->postProcObj.input_arr = this->scalerObj.output[1].arr;

    /* ImgMosaic config + register */
    this->imgMosaicObj.params.windows[0].width  = this->cfgParams.out_width;
    this->imgMosaicObj.params.windows[0].height = this->cfgParams.out_height;
    this->imgMosaicObj.bufq_depth = APP_BUFFER_Q_DEPTH;
    if (this->graphObj.addNode(&this->imgMosaicObj) == NULL) return VX_FAILURE;

    /* Wire: mosaic takes postProc output */
    {
        vx_int32 idx = 0;
        this->imgMosaicObj.input_arr[idx++] = this->postProcObj.output_image_arr;
        this->imgMosaicObj.num_inputs = idx;
    }

    /* Display config + register */
    this->displayObj.display_option = this->cfgParams.display_option;
    if (this->graphObj.addNode(&this->displayObj) == NULL) return VX_FAILURE;

    /* Wire: display takes mosaic output */
    this->displayObj.input_image = this->imgMosaicObj.output_image[0];

    APP_PRINTF("All nodes registered and initialized! \n");

    return VX_SUCCESS;
}

vx_status AppObj::create_graph()
{
    vx_status status = VX_SUCCESS;

    status = this->graphObj.createGraph();

    if (this->cfgParams.enable_pipeline_flow && status == VX_SUCCESS)
    {
        vx_bool en_mosaic_out   = ((this->cfgParams.en_out_img_write == 1) || (this->cfgParams.dump_output == 1))
                                  ? (vx_bool)vx_true_e : (vx_bool)vx_false_e;
        vx_bool mosaic_param_en = en_mosaic_out;
        vx_bool mosaic_buf_en   = (en_mosaic_out == (vx_bool)vx_true_e)
                                  ? (vx_bool)vx_false_e : (vx_bool)vx_true_e;

        PipelineConfig pipelineCfg(APP_PIPELINE_DEPTH);

        /* Graph parameters (queued inputs/outputs) */
        pipelineCfg.addGraphParam(this->scalerObj.node, 0,
                                  (vx_reference*)&this->scalerObj.input_images[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->scalerObj.graph_parameter_index);

        pipelineCfg.addGraphParam(this->imgMosaicObj.node, 1,
                                  (vx_reference*)&this->imgMosaicObj.output_image[0],
                                  APP_BUFFER_Q_DEPTH,
                                  &this->imgMosaicObj.graph_parameter_index,
                                  mosaic_param_en);

        /* Per-node buffer depths */
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output1_index=*/1, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output2_index=*/2, /*buffer_depth=*/6);
        pipelineCfg.addNodeBuf(this->preProcObj.node,   /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->tidlObj.node,      /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->postProcObj.node,  /*output_image_index=*/2, /*buffer_depth=*/APP_BUFFER_Q_DEPTH);
        pipelineCfg.addNodeBuf(this->imgMosaicObj.node, /*output_image_index=*/1, /*buffer_depth=*/4, mosaic_buf_en);

        status = pipelineCfg.setup(this->graphObj.graph);
    }

    return status;
}

vx_status AppObj::run_one_frame_sequential(void *context, vx_int32 frame_id)
{
    AppObj *self = (AppObj*)context;
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];

#ifdef WRITE_INTERMEDIATE_OUTPUTS
    ScalerObj *scalerPtr = &self->scalerObj;
#endif

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv",
             self->graphObj.input_file_path.c_str(), frame_id);
    appPerfPointBegin(&self->graphObj.fileio_perf);
    readScalerInput(input_file_name, self->scalerObj.input_arr[0], APP_MODULES_READ_FILE, 0);
    appPerfPointEnd(&self->graphObj.fileio_perf);
    APP_PRINTF("App Reading Input Done!\n");

#ifdef x86_64
    APP_PRINTF("Processing file %s ...", input_file_name);
#endif

    status = vxProcessGraph(self->graphObj.graph);

#ifdef x86_64
    APP_PRINTF("Done!\n");
#endif

    if ((VX_SUCCESS == status) && (self->cfgParams.en_out_img_write == 1))
    {
        vx_char output_file_name[APP_MAX_FILE_PATH];

        APP_PRINTF("App Writing Outputs Start...\n");

#ifdef APP_TIDL_TRACE_DUMP
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_",
                 self->graphObj.output_file_path.c_str(), frame_id);
        vx_user_data_object trace_data = (vx_user_data_object)vxGetObjectArrayItem(
                                          self->tidlObj.trace_data_arr, 0);
        tivx_utils_tidl_trace_write(trace_data, output_file_name);
        vxReleaseUserDataObject(&trace_data);
#endif

#ifdef WRITE_INTERMEDIATE_OUTPUTS
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_scaler_%dx%d.yuv",
                 self->graphObj.output_file_path.c_str(), frame_id,
                 self->scalerObj.output[0].width, self->scalerObj.output[0].height);
        writeScalerOutput(output_file_name, self->scalerObj.output[0].arr);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/pre_proc_output_%010d",
                 self->graphObj.output_file_path.c_str(), frame_id);
        self->preProcObj.writeOutput(output_file_name);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/tidl_output_%010d",
                 self->graphObj.output_file_path.c_str(), frame_id);
        self->tidlObj.writeOutput(output_file_name);
#endif

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_%dx%d.yuv",
                 self->graphObj.output_file_path.c_str(), frame_id,
                 self->cfgParams.out_width, self->cfgParams.out_height);
        self->imgMosaicObj.writeMosaicOutput(output_file_name, self->imgMosaicObj.output_image[0]);

        APP_PRINTF("App Writing Outputs Done!\n");
    }
    return status;
}

vx_status AppObj::run_one_frame_pipeline(void *context, vx_int32 frame_id)
{
    AppObj *self = (AppObj*)context;
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];
    vx_int32 obj_array_idx = -1;

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv",
             self->graphObj.input_file_path.c_str(), frame_id);

    if (self->graphObj.pipeline < 0)
    {
        /* Enqueue outputs */
        if (((self->cfgParams.en_out_img_write == 1) || (self->graphObj.dump_output == 1)) && (status == VX_SUCCESS))
        {
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                                                     self->imgMosaicObj.graph_parameter_index,
                                                     (vx_reference*)&self->imgMosaicObj.output_image[self->graphObj.enqueueCnt],
                                                     1);
        }
        appPerfPointBegin(&self->graphObj.fileio_perf);
        /* Read input */
        if (status == VX_SUCCESS)
        {
            status = readScalerInput(input_file_name, self->scalerObj.input_arr[self->graphObj.enqueueCnt],
                                    APP_MODULES_READ_FILE, 0);
        }
        appPerfPointEnd(&self->graphObj.fileio_perf);
        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                                                     self->scalerObj.graph_parameter_index,
                                                     (vx_reference*)&self->scalerObj.input_images[self->graphObj.enqueueCnt],
                                                     1);
        }
        self->graphObj.enqueueCnt++;
        self->graphObj.enqueueCnt = (self->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.enqueueCnt;
        self->graphObj.pipeline++;
    }

    if (self->graphObj.pipeline >= 0)
    {
        vx_image scaler_input_image;
        vx_image mosaic_output_image;
        uint32_t num_refs;

        /* Dequeue input */
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(self->graphObj.graph,
                                                    self->scalerObj.graph_parameter_index,
                                                    (vx_reference*)&scaler_input_image,
                                                    1, &num_refs);
        }
        if (self->cfgParams.en_out_img_write == 1)
        {
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output */
            if (status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(self->graphObj.graph,
                                                        self->imgMosaicObj.graph_parameter_index,
                                                        (vx_reference*)&mosaic_output_image,
                                                        1, &num_refs);
            }
            if (status == VX_SUCCESS)
            {
                APP_PRINTF("App Writing Outputs Start...\n");
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_%dx%d.yuv",
                         self->graphObj.output_file_path.c_str(), (frame_id - APP_BUFFER_Q_DEPTH),
                         self->cfgParams.out_width, self->cfgParams.out_height);
                status = self->imgMosaicObj.writeMosaicOutput(output_file_name, mosaic_output_image);
                APP_PRINTF("App Writing Outputs Done!\n");
            }
            /* Enqueue output */
            if (status == VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                                                         self->imgMosaicObj.graph_parameter_index,
                                                         (vx_reference*)&mosaic_output_image,
                                                         1);
            }
        }

        /* Offline output dumping (--dump): write the mosaic output only for the
         * fixed set of target frame indices tracked by AppGraphObj::shouldDumpFrame(). */
        if (self->graphObj.dump_output == 1)
        {
            vx_bool dump_this_frame = self->graphObj.shouldDumpFrame(frame_id);
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output (skip if already dequeued above by en_out_img_write) */
            if ((self->cfgParams.en_out_img_write != 1) && (status == VX_SUCCESS))
            {
                status = vxGraphParameterDequeueDoneRef(self->graphObj.graph,
                                                        self->imgMosaicObj.graph_parameter_index,
                                                        (vx_reference*)&mosaic_output_image,
                                                        1, &num_refs);
            }
            if ((status == VX_SUCCESS) && (dump_this_frame == (vx_bool)vx_true_e))
            {
                printf("Dumping output for frame id : %d\n", frame_id);
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_%dx%d.yuv",
                         self->graphObj.output_file_path.c_str(), (frame_id - APP_BUFFER_Q_DEPTH),
                         self->cfgParams.out_width, self->cfgParams.out_height);
                status = self->imgMosaicObj.writeMosaicOutput(output_file_name, mosaic_output_image);
                printf("App Dumping Outputs Done!\n");
            }
            /* Enqueue output back (skip if already enqueued above by en_out_img_write) */
            if ((self->cfgParams.en_out_img_write != 1) && (status == VX_SUCCESS))
            {
                status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                                                         self->imgMosaicObj.graph_parameter_index,
                                                         (vx_reference*)&mosaic_output_image,
                                                         1);
            }
        }

        appPerfPointBegin(&self->graphObj.fileio_perf);

        if (status == VX_SUCCESS)
        {
            AppGraphObj::findObjectArrayIndex(self->scalerObj.input_arr, (vx_reference)scaler_input_image,
                                             APP_BUFFER_Q_DEPTH, &obj_array_idx);
        }
        if ((obj_array_idx != -1) && (status == VX_SUCCESS))
        {
            status = readScalerInput(input_file_name, self->scalerObj.input_arr[obj_array_idx],
                                    APP_MODULES_READ_FILE, 0);
        }
        appPerfPointEnd(&self->graphObj.fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(self->graphObj.graph,
                                                     self->scalerObj.graph_parameter_index,
                                                     (vx_reference*)&scaler_input_image,
                                                     1);
        }
        self->graphObj.enqueueCnt++;
        self->graphObj.dequeueCnt++;

        self->graphObj.enqueueCnt = (self->graphObj.enqueueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.enqueueCnt;
        self->graphObj.dequeueCnt = (self->graphObj.dequeueCnt >= APP_BUFFER_Q_DEPTH) ? 0 : self->graphObj.dequeueCnt;
    }

    APP_PRINTF("App Process Graph Done!\n");

    return status;
}

#ifndef x86_64
void AppObj::app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 550, 5, (char*)"TIDL - Semantic Segmentation Demo", &sHeading);
    }
}
#endif

int main(int argc, char *argv[])
{
    int status = appInit();

    if (status == 0)
    {
        /* Parse CLI + .cfg file */
        app.cfgParams.parse(argc, argv);

        /* Wire cfgParams into module structs */
        status = app.setup_graph();
        APP_PRINTF("App setup_graph Done (config + init + addNode)!\n");
        assert(status == VX_SUCCESS);

        status = app.create_graph();
        APP_PRINTF("App Create Graph Done!\n");
        assert(status == VX_SUCCESS);

        app_grpx_init_prms_t grpx_prms;
#ifndef x86_64
        /* Initialize graphics */
        appGrpxInitParamsInit(&grpx_prms, app.graphObj.context);
        grpx_prms.draw_callback = AppObj::app_draw_graphics;
        appGrpxInit(&grpx_prms);
#endif

        status = app.graphObj.verifyGraph();
        APP_PRINTF("App Verify Graph Done!\n");
        assert(status == VX_SUCCESS);

        status = app.graphObj.run(app.cfgParams.is_interactive);
        assert(status == VX_SUCCESS);

        APP_PRINTF("App Run Graph Done!\n");

#ifndef x86_64
        /* Deinitialize graphics */
        appGrpxDeInit();
#endif

        app.graphObj.deleteGraph();
        app.graphObj.deinit();

        appDeInit();
    }

    return status;
}
