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
#include <TI/video_io_display.h>
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
#include "app_post_proc_module.hpp"
#include "app_img_mosaic_module.hpp"
#include "app_display_module.hpp"
#include "config/app_tidl_cfg_params.h"
#include <app_init.h>

/* ImageNet class labels (1001 entries, 64 bytes each) defined in
 * imagenet_class_labels.c. Stride now matches TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME. */
extern "C" {
    extern const char imgnet_labels[1001][TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME];
}

/* #define WRITE_INTERMEDIATE_OUTPUTS */

#define APP_BUFFER_Q_DEPTH   (2)
#define APP_PIPELINE_DEPTH   (6)

class AppObj {
public:
    /* Core graph object — owns context, graph, input, module_list, pipeline
     * state, perf points, file paths, task control, etc. */
    AppGraphObj graphObj;

    /* Module-specific objects */
    ScalerObj    scalerObj;      // First scaler: 1024x512 → 512x256
    ScalerObj    scaler2Obj;     // Second scaler: 512x256 → 224x224
    PreProcObj   preProcObj;
    TIDLObj      tidlObj;
    PostProcObj  postProcObj;
    ImgMosaicObj imgMosaicObj;
    DisplayObj   displayObj;

    /* Demo-specific config params */
    AppTidlCfgParams cfgParams;

    vx_status setup_graph();
    vx_status create_graph();

    /* These are static so they match RunOneFrameFn (void *context, vx_int32).
     * The framework calls them via function pointer; inside, the void*
     * is cast back to AppObj*. */
    static vx_status run_one_frame_sequential(void *context, vx_int32 frame_id);
    static vx_status run_one_frame_pipeline(void *context, vx_int32 frame_id);

    #ifndef x86_64
    static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
    #endif
};

static AppObj app;

vx_status AppObj::setup_graph()
{
    this->graphObj.dot_export_name = "vx_app_tidl";

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
    this->graphObj.perf_export_name     = "dl_demos_app_tidl";
    this->graphObj.demo_name            = "TIDL Demo - Image Classification";

    /* Interactive mode: run indefinitely until user presses 'x' */
    if (this->cfgParams.is_interactive) {
        this->graphObj.num_iterations = 1000000000;
    }

    /* Set the run_one_frame callback */
    this->graphObj.run_one_frame_context = this;
    this->graphObj.run_one_frame = this->cfgParams.enable_pipeline_flow
                                   ? AppObj::run_one_frame_pipeline
                                   : AppObj::run_one_frame_sequential;

    /* ---- Initialize graph context ---- */
    vx_status status = this->graphObj.init();
    if (status != VX_SUCCESS) return status;
    APP_PRINTF("Graph Initialize Done! \n");

    /* ---- Scaler1 config + register: 1024x512 → 512x256 (2x downscale) ---- */
    vx_int32 intermediate_width = 512;
    vx_int32 intermediate_height = 256;

    this->scalerObj.input_width  = this->cfgParams.in_width;
    this->scalerObj.input_height = this->cfgParams.in_height;
    this->scalerObj.bufq_depth   = APP_BUFFER_Q_DEPTH;
    this->scalerObj.num_outputs  = 2;
    this->scalerObj.output[0].width  = intermediate_width;   // Intermediate size for scaler2
    this->scalerObj.output[0].height = intermediate_height;
    this->scalerObj.output[1].width  = this->cfgParams.out_width;
    this->scalerObj.output[1].height = this->cfgParams.out_height;

    if (this->graphObj.addNode(&this->scalerObj) == NULL) return VX_FAILURE;

    /* ---- Scaler2 config + register: 512x256 → 224x224 (2.28x downscale) ---- */
    this->scaler2Obj.input_width  = intermediate_width;
    this->scaler2Obj.input_height = intermediate_height;
    this->scaler2Obj.bufq_depth   = APP_BUFFER_Q_DEPTH;
    this->scaler2Obj.num_outputs  = 1;
    this->scaler2Obj.external_input = vx_true_e;  /* Input from scaler1, don't create/manage */
    this->scaler2Obj.output[0].width  = this->cfgParams.dl_width;
    this->scaler2Obj.output[0].height = this->cfgParams.dl_height;

    if (this->graphObj.addNode(&this->scaler2Obj) == NULL) return VX_FAILURE;

    /* ---- TIDL config + register ---- */
    this->tidlObj.config_file_path  = this->cfgParams.tidl_config;
    this->tidlObj.network_file_path = this->cfgParams.tidl_network;
    if (this->graphObj.addNode(&this->tidlObj) == NULL) return VX_FAILURE;

    /* ---- PreProc register ---- */
    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->preProcObj.tidl_io_buf_desc = *this->tidlObj.getIOBufDesc();
    if (this->graphObj.addNode(&this->preProcObj) == NULL) return VX_FAILURE;

    /* Wire: scaler1 output[0] feeds scaler2 input */
    this->scaler2Obj.input_arr[0] = this->scalerObj.output[0].arr;

    /* Wire: preProc takes scaler2 output[0]; tidl takes preProc output tensors */
    this->preProcObj.input_arr[0] = this->scaler2Obj.output[0].arr;
    for (vx_int32 i = 0; i < this->tidlObj.num_input_tensors; i++) {
        this->tidlObj.input_tensor_arr[i] = this->preProcObj.output_tensor_arr[i];
    }

    /* ---- PostProc config + register ---- */
    this->postProcObj.num_ch = 1;
    this->postProcObj.output_img_width = this->cfgParams.out_width;
    this->postProcObj.output_img_height = this->cfgParams.out_height;

    /* MUST configure BEFORE addNode() so module doesn't overwrite */
    /* Classification mode */
    this->postProcObj.params.task_type = TIVX_DL_POST_PROC_CLASSIFICATION_TASK_TYPE;
    this->postProcObj.params.oc_prms.num_top_results = 5;
    this->postProcObj.params.oc_prms.labelOffset = 1;  /* ImageNet has 1000 classes, offset by 1 */
    this->postProcObj.params.oc_prms.classnames = (char (*)[TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME])imgnet_labels;

    /* CHANGE-APPLIED: direct TIDL wiring, no getModuleByType lookup */
    this->postProcObj.ioBufDesc = *this->tidlObj.getIOBufDesc();

    if (this->graphObj.addNode(&this->postProcObj) == NULL) {
        return VX_FAILURE;
    }

    /* Wire: postProc takes TIDL output tensor and scaler output[1] for overlay */
    for (int i = 0; i < this->tidlObj.ioBufDesc.numOutputBuf; i++)
    {
        this->postProcObj.input_tensor_arr[i] = this->tidlObj.output_tensor_arr[i];
    }
    this->postProcObj.input_arr = this->scalerObj.output[1].arr;

    /* ---- ImgMosaic config + register ---- */
    this->imgMosaicObj.num_inputs = 1;
    this->imgMosaicObj.out_width = this->cfgParams.out_width;
    this->imgMosaicObj.out_height = this->cfgParams.out_height;
    this->imgMosaicObj.bufq_depth = APP_BUFFER_Q_DEPTH;

    /* Single window covering entire output (passthrough) */
    this->imgMosaicObj.params.windows[0].startX = 0;
    this->imgMosaicObj.params.windows[0].startY = 0;
    this->imgMosaicObj.params.windows[0].width = this->cfgParams.out_width;
    this->imgMosaicObj.params.windows[0].height = this->cfgParams.out_height;
    this->imgMosaicObj.params.windows[0].input_select = 0;
    this->imgMosaicObj.params.windows[0].channel_select = 0;
    this->imgMosaicObj.params.num_windows = 1;
    this->imgMosaicObj.params.clear_count = 4;

    if (this->graphObj.addNode(&this->imgMosaicObj) == NULL) {
        return VX_FAILURE;
    }

    /* Wire: imgMosaic takes postProc output */
    this->imgMosaicObj.input_arr[0] = this->postProcObj.output_image_arr;

    /* ---- Display config + register ---- */
    this->displayObj.display_option = this->cfgParams.display_option;
    this->displayObj.out_width      = this->cfgParams.out_width;
    this->displayObj.out_height     = this->cfgParams.out_height;

    if (this->graphObj.addNode(&this->displayObj) == NULL) {
        return VX_FAILURE;
    }

    /* Wire: display takes imgMosaic output (with labels drawn) */
    this->displayObj.input_image = this->imgMosaicObj.output_image[0];

    APP_PRINTF("All nodes registered and initialized! \n");

    return VX_SUCCESS;
}

vx_status AppObj::create_graph()
{
    vx_status status = this->graphObj.createGraph();

    /* Pipeline configuration (now supported with Scaler/PreProc) */
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

        /* Node buffer depths */
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output1_index=*/1, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->scalerObj.node,    /*output2_index=*/2, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->preProcObj.node,   /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->tidlObj.node,      /*output_tensor_index=*/7, /*buffer_depth=*/2);
        pipelineCfg.addNodeBuf(this->postProcObj.node,  /*output_image_index=*/2, /*buffer_depth=*/APP_BUFFER_Q_DEPTH);
        pipelineCfg.addNodeBuf(this->imgMosaicObj.node, /*output_image_index=*/1, /*buffer_depth=*/4, mosaic_buf_en);

        status = pipelineCfg.setup(this->graphObj.graph);
        if (status != VX_SUCCESS) {
            return status;
        }
        APP_PRINTF("Pipeline configuration done!\n");
    }

    return status;
}

vx_status AppObj::run_one_frame_sequential(void *context, vx_int32 frame_id)
{
    AppObj *self = (AppObj*)context;
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];

    /* Read YUV file into scaler input (like OD demo) */
    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%04d.yuv",
             self->graphObj.input_file_path.c_str(), frame_id + 1);

    appPerfPointBegin(&self->graphObj.fileio_perf);
    /* Read YUV input - Scaler and PreProc handle the rest */
    status = readScalerInput(input_file_name, self->scalerObj.input_arr[0], APP_MODULES_READ_FILE, 0);
    if (status != VX_SUCCESS)
    {
        return status;
    }
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

        /* Write ImgMosaic output (with labels drawn from PostProc) */
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%04d.yuv",
                 self->graphObj.output_file_path.c_str(), frame_id);
        self->imgMosaicObj.writeMosaicOutput(output_file_name, self->imgMosaicObj.output_image[0]);

#ifdef APP_TIDL_TRACE_DUMP
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_",
                 self->graphObj.output_file_path.c_str(), frame_id);
        vx_user_data_object trace_data = (vx_user_data_object)vxGetObjectArrayItem(
            self->tidlObj.trace_data_arr, 0);
        tivx_utils_tidl_trace_write(trace_data, output_file_name);
        vxReleaseUserDataObject(&trace_data);
#endif

#ifdef WRITE_INTERMEDIATE_OUTPUTS
        snprintf(output_file_name, APP_MAX_FILE_PATH,
                 "%s/tidl_output_%010d",
                 self->graphObj.output_file_path.c_str(), frame_id);
        self->tidlObj.writeOutput(output_file_name);
#endif

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

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%04d.yuv",
             self->graphObj.input_file_path.c_str(), frame_id + 1);

    /* Initial enqueue phase - fill the pipeline */
    if (self->graphObj.pipeline < 0)
    {
        /* Enqueue ImgMosaic output if writing enabled */
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
            status = readScalerInput(input_file_name,
                                    self->scalerObj.input_arr[self->graphObj.enqueueCnt],
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

    /* Steady-state pipeline operation */
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

        /* Dequeue, write, and re-enqueue ImgMosaic output if enabled */
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

            /* Write output */
            if (status == VX_SUCCESS)
            {
                APP_PRINTF("App Writing Outputs Start...\n");
                snprintf(output_file_name, APP_MAX_FILE_PATH,
                         "%s/mosaic_output_%04d.yuv",
                         self->graphObj.output_file_path.c_str(),
                         (frame_id - APP_BUFFER_Q_DEPTH));
                status = self->imgMosaicObj.writeMosaicOutput(output_file_name, mosaic_output_image);
                APP_PRINTF("App Writing Outputs Done!\n");
            }

            /* Enqueue output back */
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
                snprintf(output_file_name, APP_MAX_FILE_PATH,
                         "%s/mosaic_output_%04d.yuv",
                         self->graphObj.output_file_path.c_str(),
                         (frame_id - APP_BUFFER_Q_DEPTH));
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

        /* Find which buffer was dequeued */
        if (status == VX_SUCCESS)
        {
            AppGraphObj::findObjectArrayIndex(self->scalerObj.input_arr,
                                              (vx_reference)scaler_input_image,
                                              APP_BUFFER_Q_DEPTH, &obj_array_idx);
        }

        /* Read next input into the dequeued buffer */
        if ((obj_array_idx != -1) && (status == VX_SUCCESS))
        {
            status = readScalerInput(input_file_name,
                                    self->scalerObj.input_arr[obj_array_idx],
                                    APP_MODULES_READ_FILE, 0);
        }

        appPerfPointEnd(&self->graphObj.fileio_perf);
        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start next execution */
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


int main(int argc, char *argv[])
{
    int status = appInit();

    if (status == 0)
    {
        /* One call: CLI + .cfg file → cfgParams (defaults set by constructor) */
        app.cfgParams.parse(argc, argv);

        /* Wire cfgParams into module structs */
        status = app.setup_graph();
        APP_PRINTF("App setup_graph Done (config + init + addNode)!\n");
        assert(status == VX_SUCCESS);

        status = app.create_graph();
        APP_PRINTF("App Create Graph Done!\n");
        assert(status == VX_SUCCESS);

        #ifndef x86_64
        /* Initialize graphics */
        app_grpx_init_prms_t grpx_prms;
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

#ifndef x86_64
void AppObj::app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        /* Draw static title - keep it simple like app_tidl_od */
        Draw2D_FontPrm sHeading;
        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 400, 5, (char*)"TIDL - Image Classification Demo", &sHeading);
    }
}
#endif
