/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#include "app_common.h"
#include "app_scaler_module.h"
#include "app_pre_proc_module.h"
#include "app_tidl_module.h"
#include "app_post_proc_module.h"
#include "app_img_mosaic_module.h"
#include "app_display_module.h"
#include "app_test.h"
#include "app_dump_output.h"

#ifdef APP_TIDL_TRACE_DUMP
#include <tivx_utils_tidl_trace.h>
#endif

/* #define WRITE_INTERMEDIATE_OUTPUTS */

#ifndef x86_64
#define APP_ENABLE_PIPELINE_FLOW
#endif

#define APP_BUFFER_Q_DEPTH   (2)
#define APP_PIPELINE_DEPTH   (6)

typedef struct {
    vx_object_array arr[APP_MAX_BUFQ_DEPTH];

    vx_int32 width;
    vx_int32 height;

} InputObj;

typedef struct {

    ScalerObj  scalerObj;
    PreProcObj preProcObj;

    TIDLObj tidlObj;

    PostProcObj postProcObj;
    ImgMosaicObj imgMosaicObj;

    DisplayObj displayObj;

    InputObj input;

    /* Arrray of 0th index reference of input.arr */
    vx_image input_images[APP_MAX_BUFQ_DEPTH];

    vx_char input_file_path[APP_MAX_FILE_PATH];
    vx_char output_file_path[APP_MAX_FILE_PATH];

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    vx_int32 en_out_img_write;
    vx_int32 test_mode;
    vx_int32 test_case;

    vx_int32 start_frame;
    vx_int32 num_frames;

    vx_uint32 delay_in_msecs;
    vx_uint32 num_iterations;
    vx_uint32 is_interactive;
    vx_uint32 cpu_core_id;
    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t pipeline;

    int32_t enqueueCnt;
    int32_t dequeueCnt;
    int32_t dump_output;

} AppObj;

AppObj gAppObj;

/* Update the color map as required, the lenth of array should be less than or equal to, TIVX_PIXEL_VIZ_MAX_CLASS */
/* The format is R, G, B */
static const vx_uint8 color_map[5][3] = {{152,251,152},{0,130,180},{220,20,60},{3,3,251},{190,153,153}};

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
void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_pipeline_params_defaults(AppObj *obj);
void app_find_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
#ifdef APP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);
#else
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id);
#endif

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL Demo - Semantic Segmentation (c) Texas Instruments Inc. 2020\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ========================="
    "\n TIDL Demo - Semantic Segmentation"
    "\n ========================="
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
    while(!obj->stop_task && (status == VX_SUCCESS))
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
    while(obj->stop_task_done==0)
    {
         tivxTaskWaitMsecs(100);
    }

    tivxTaskDelete(&obj->task);
}

static vx_status app_run_graph_interactive(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint32_t done = 0;
    char ch;
    FILE *fp;
    app_perf_point_t *perf_arr[1];

    status = app_run_task_create(obj);
    if(status!=0)
    {
        printf("app_tidl: ERROR: Unable to create task\n");
    }
    else
    {
        appPerfStatsResetAll();
        while(!done && (status == VX_SUCCESS))
        {
            printf(menu);
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
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_seg");
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
    snprintf(obj->tidlObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->tidlObj.network_file_path,APP_MAX_FILE_PATH, ".");

    snprintf(obj->input_file_path,APP_MAX_FILE_PATH, ".");

}

static void app_parse_cfg_file(AppObj *obj, vx_char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    vx_char line_str[1024];
    vx_char *token;

    if(fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
        exit(0);
    }

    while(fgets(line_str, sizeof(line_str), fp)!=NULL)
    {
        vx_char s[]=" \t";

        if (strchr(line_str, '#'))
        {
            continue;
        }

        /* get the first token */
        token = strtok(line_str, s);
        if(token != NULL)
        {
            if(strcmp(token, "tidl_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->tidlObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_network")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->tidlObj.network_file_path, token);
                  /* for testing if relevant */
                  if(strstr(obj->tidlObj.network_file_path, "u16") != NULL)
                  {
                    obj->test_case = 1;
                  }
                  else
                  {
                    obj->test_case = 0;
                  }
                }
            }
            else
            if(strcmp(token, "input_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->input_file_path, token);
                }
            }
            else
            if(strcmp(token, "output_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->output_file_path, token);
                }
            }
            else
            if(strcmp(token, "start_frame")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  obj->start_frame = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_frames")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  obj->num_frames = atoi(token);
                }
            }
            else
            if(strcmp(token, "in_size")==0)
            {
                vx_int32 width, height;
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  width =  atoi(token);
                  obj->input.width  = width;

                  token = strtok(NULL, s);
                  if(token != NULL)
                  {
                    if (token[strlen(token)-1] == '\n')
                      token[strlen(token)-1]=0;

                    height =  atoi(token);
                    obj->input.height = height;
                  }
                }
            }
            else
            if(strcmp(token, "dl_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                  width =  atoi(token);
                  obj->scalerObj.output[0].width   = width;

                  token = strtok(NULL, s);
                  if(token != NULL)
                  {
                    if(token[strlen(token)-1] == '\n')
                      token[strlen(token)-1]=0;

                    height =  atoi(token);
                    obj->scalerObj.output[0].height  = height;
                  }
                }
            }
            else
            if(strcmp(token, "out_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                  width =  atoi(token);
                  obj->scalerObj.output[1].width   = width;

                  token = strtok(NULL, s);
                  if(token != NULL)
                  {
                    if(token[strlen(token)-1] == '\n')
                      token[strlen(token)-1]=0;

                    height =  atoi(token);
                    obj->scalerObj.output[1].height  = height;
                  }
                }
            }
            else
            if(strcmp(token, "num_classes")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->postProcObj.viz_params.num_classes[0] = atoi(token);
                }
            }
            else
            if(strcmp(token, "en_out_img_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  obj->en_out_img_write = atoi(token);
                }
            }
            else
            if(strcmp(token, "display_option")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  obj->displayObj.display_option = atoi(token);
                }
            }
            else
            if(strcmp(token, "delay_in_msecs")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  obj->delay_in_msecs = atoi(token);
                  if(obj->delay_in_msecs > 2000)
                    obj->delay_in_msecs = 2000;
                }
            }
            else
            if(strcmp(token, "num_iterations")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  obj->num_iterations = atoi(token);
                  if(obj->num_iterations == 0)
                    obj->num_iterations = 1;
                }
            }
            else
            if(strcmp(token, "is_interactive")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  obj->is_interactive = atoi(token);
                  if(obj->is_interactive > 1)
                    obj->is_interactive = 1;
                }
            }
            else
            if(strcmp(token, "cpu_core_id")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->cpu_core_id = atoi(token);
                }
            }
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;
    vx_bool set_test_mode = vx_false_e;
    vx_bool dump_output = vx_false_e;

    app_set_cfg_default(obj);

    if(argc==1)
    {
        app_show_usage(argc, argv);
        exit(0);
    }

    for(i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "--cfg")==0)
        {
            i++;
            if(i>=argc)
            {
                app_show_usage(argc, argv);
            }
            app_parse_cfg_file(obj, argv[i]);
        }
        else
        if(strcmp(argv[i], "--help")==0)
        {
            app_show_usage(argc, argv);
            exit(0);
        }
        else
        if(strcmp(argv[i], "--test")==0)
        {
            set_test_mode = vx_true_e;
        }
        else
        if(strcmp(argv[i], "--dump")==0)
        {
            dump_output = vx_true_e;
        }
    }

    if (set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->is_interactive = 0;
        obj->displayObj.display_option = 1;
        obj->num_iterations = 1;
        obj->num_frames = sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0]) + TEST_BUFFER + 1;
    }

    if (dump_output == vx_true_e)
    {
        obj->dump_output = 1;
        obj->num_iterations = 1;
        obj->is_interactive = 0;
        obj->displayObj.display_option = 1;
        obj->num_frames = PIPELINE_TOTAL_NO_OF_FRAMES + 1;
        APP_PRINTF("Output dumping is enabled\n");
    }
    else
    {
        obj->dump_output = 0;
        APP_PRINTF("Output dumping is disabled\n");
    }

    #ifdef x86_64
    obj->displayObj.display_option = 0;
    obj->is_interactive = 0;
    #endif

    return;
}

vx_status app_tidl_seg_main(vx_int32 argc, vx_char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    /*Optional parameter setting*/
    app_default_param_set(obj);
    APP_PRINTF("Default param set! \n");

    /*Config parameter reading*/
    app_parse_cmd_line_args(obj, argc, argv);
    APP_PRINTF("Parsed user params! \n");

    /*Update of parameters are config file read*/
    app_update_param_set(obj);
    APP_PRINTF("Updated user params! \n");

    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    if(status == VX_SUCCESS)
    {
      status = app_create_graph(obj);
      APP_PRINTF("App Create Graph Done! \n");
    }
    if(status == VX_SUCCESS)
    {
      status = app_verify_graph(obj);
      APP_PRINTF("App Verify Graph Done! \n");
    }
    if(obj->is_interactive && (status == VX_SUCCESS))
    {
      status = app_run_graph_interactive(obj);
    }
    else
    if (status == VX_SUCCESS)
    {
      status = app_run_graph(obj);
    }

    APP_PRINTF("App Run Graph Done! \n");

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");
    if (obj->test_mode)
    {
        if ((test_result == vx_false_e) || (status != VX_SUCCESS))
        {
            printf("\n\nTEST FAILED\n\n");
            print_new_checksum_structs();
            status = (status == VX_SUCCESS) ? VX_FAILURE : status;
        }
        else
        {
            printf("\n\nTEST PASSED\n\n");
        }
    }

    return status;
}

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    app_grpx_init_prms_t grpx_prms;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);
    if(status == VX_SUCCESS)
    {
      tivxHwaLoadKernels(obj->context);
      tivxVideoIOLoadKernels(obj->context);
      tivxImgProcLoadKernels(obj->context);
      tivxTIDLLoadKernels(obj->context);

    }

    /* Create Input OpenVx object */
    vx_image input  = vxCreateImage(obj->context, obj->input.width, obj->input.height, VX_DF_IMAGE_NV12);
    status = vxGetStatus((vx_reference)input);
    if(status == VX_SUCCESS)
    {
        vx_int32 q;
        for(q = 0; q < APP_BUFFER_Q_DEPTH; q++)
        {
            obj->input.arr[q]     = vxCreateObjectArray(obj->context, (vx_reference)input, NUM_CH);
            obj->input_images[q]  = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[q], 0);
        }
        vxReleaseImage(&input);
    }

    if(status == VX_SUCCESS)
    {
      /* Initialize modules */
      status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", NUM_CH, 2);
      APP_PRINTF("Scaler Init Done! \n");
    }

    /* Initialize TIDL first to get tensor I/O information from network */
    if(status == VX_SUCCESS)
    {
      #if defined (SOC_J784S4)
      obj->tidlObj.core_id = obj->cpu_core_id;
      #elif defined (SOC_J722S)
      if(obj->cpu_core_id >0)
      {
        obj->tidlObj.core_id = 1;
      }
      #else
      obj->tidlObj.core_id = 0;
      #endif

      status = app_init_tidl(obj->context, &obj->tidlObj, "tidl_obj", 1);
      APP_PRINTF("TIDL Init Done! \n");
    }

    /* Update pre-proc parameters with TIDL config before calling init */
    if(status == VX_SUCCESS)
    {
      status = app_update_pre_proc(obj->context, &obj->preProcObj, obj->tidlObj.config);
      APP_PRINTF("Pre Proc Update Done! \n");
    }
    if(status == VX_SUCCESS)
    {
      status = app_init_pre_proc(obj->context, &obj->preProcObj, "pre_proc_obj");
      APP_PRINTF("Pre Proc Init Done! \n");
    }
    /* Update post-proc parameters with TIDL config before calling init */
    if(status == VX_SUCCESS)
    {
      status = app_update_post_proc(&obj->postProcObj, obj->tidlObj.config);
      APP_PRINTF("Post Proc Update Done! \n");
    }
    if(status == VX_SUCCESS)
    {
      status = app_init_post_proc(obj->context, &obj->postProcObj, "post_proc_obj");
      APP_PRINTF("Post Proc Init Done! \n");
    }
    if(status == VX_SUCCESS)
    {
      status = app_init_img_mosaic(obj->context, &obj->imgMosaicObj, "sw_mosaic_obj", APP_BUFFER_Q_DEPTH);
      APP_PRINTF("Img Mosaic Init Done! \n");
    }
    if(status == VX_SUCCESS)
    {
      status = app_init_display(obj->context, &obj->displayObj, "display_obj");
      APP_PRINTF("Display Init Done! \n");
    }
    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");

    #ifndef x86_64
    if(obj->displayObj.display_option == 1)
    {
      appGrpxInitParamsInit(&grpx_prms, obj->context);
      grpx_prms.draw_callback = app_draw_graphics;
      appGrpxInit(&grpx_prms);
    }
    #endif

    return status;
}

static void app_deinit(AppObj *obj)
{
    vx_int32 q;

    for(q = 0; q < APP_BUFFER_Q_DEPTH; q++)
    {
        vxReleaseObjectArray(&obj->input.arr[q]);
        vxReleaseImage(&obj->input_images[q]);
    }

    app_deinit_scaler(&obj->scalerObj);

    app_deinit_pre_proc(&obj->preProcObj);

    app_deinit_tidl(&obj->tidlObj);

    app_deinit_post_proc(&obj->postProcObj);

    app_deinit_img_mosaic(&obj->imgMosaicObj, APP_BUFFER_Q_DEPTH);

    app_deinit_display(&obj->displayObj);

    #ifndef x86_64
    if(obj->displayObj.display_option == 1)
    {
      appGrpxDeInit();
    }
    #endif

    tivxTIDLUnLoadKernels(obj->context);
    tivxImgProcUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    app_delete_scaler(&obj->scalerObj);

    app_delete_pre_proc(&obj->preProcObj);

    app_delete_tidl(&obj->tidlObj);

    app_delete_post_proc(&obj->postProcObj);

    app_delete_img_mosaic(&obj->imgMosaicObj);

    app_delete_display(&obj->displayObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
#ifdef APP_ENABLE_PIPELINE_FLOW
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_int32 graph_parameter_index;
#endif
    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);
    vxSetReferenceName((vx_reference)obj->graph, "OpenVxGraph");

    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, obj->input.arr[0]);
    app_create_graph_pre_proc(obj->graph, &obj->preProcObj, obj->scalerObj.output[0].arr);

    app_create_graph_tidl(obj->context, obj->graph, &obj->tidlObj, obj->preProcObj.output_tensor_arr);

    app_create_graph_post_proc(obj->graph, &obj->postProcObj, obj->scalerObj.output[1].arr, obj->tidlObj.out_args_arr, obj->tidlObj.output_tensor_arr[0]);

    vx_int32 idx = 0;
    obj->imgMosaicObj.input_arr[idx++] = obj->postProcObj.output_image_arr;
    obj->imgMosaicObj.num_inputs = idx;

    app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);

    app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);

#ifdef APP_ENABLE_PIPELINE_FLOW
    /* Scalar Node - input is in Index 0 */
    graph_parameter_index = 0;
    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 0);
    obj->scalerObj.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->input_images[0];
    graph_parameter_index++;

    if ((obj->en_out_img_write == 1) || (obj->test_mode == 1) || (obj->dump_output == 1))
    {
      add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 1);
      obj->imgMosaicObj.graph_parameter_index = graph_parameter_index;
      graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
      graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
      graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.output_image[0];
      graph_parameter_index++;
    }

    vxSetGraphScheduleConfig(obj->graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            graph_parameter_index,
            graph_parameters_queue_params_list);

    tivxSetGraphPipelineDepth(obj->graph, APP_PIPELINE_DEPTH);

    tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 2);
    tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, 6);

    tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 2, 2);

    tivxSetNodeParameterNumBufByIndex(obj->tidlObj.node, 7, 2);

    tivxSetNodeParameterNumBufByIndex(obj->postProcObj.node, 4, 2);

    if(status == VX_SUCCESS)
    {
        if(!((obj->en_out_img_write == 1) || (obj->test_mode == 1) || (obj->dump_output == 1)))
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, 4);
        }
    }
#endif

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    ScalerObj *scalerObj;
    vx_reference refs[1];

    status = vxVerifyGraph(obj->graph);

    APP_PRINTF("App Verify Graph Done!\n");

    #if 1
    if(VX_SUCCESS == status)
    {
      status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_seg");
    }
    #endif

    scalerObj = &obj->scalerObj;

    refs[0] = (vx_reference)scalerObj->coeff_obj;
    if(VX_SUCCESS == status)
    {
      status = tivxNodeSendCommand(scalerObj->node, 0u,
                                  TIVX_VPAC_MSC_CMD_SET_COEFF,
                                  refs, 1u);
      APP_PRINTF("App Send MSC Command Done!\n");
    }

    if(VX_SUCCESS != status)
    {
        printf("MSC: Node send command failed!\n");
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

#ifndef APP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id)
{
  vx_status status = VX_SUCCESS;

  vx_char input_file_name[APP_MAX_FILE_PATH];

  ScalerObj *scalerObj = &obj->scalerObj;

  snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);

  appPerfPointBegin(&obj->total_perf);
  appPerfPointBegin(&obj->fileio_perf);

  readScalerInput(input_file_name, obj->input.arr[0], APP_MODULES_READ_FILE, 0);

  appPerfPointEnd(&obj->fileio_perf);

  APP_PRINTF("App Reading Input Done!\n");

  #ifdef x86_64
  printf("Processing file %s ...", input_file_name);
  #endif

  status = vxProcessGraph(obj->graph);

  #ifdef x86_64
  printf("Done!\n");
  #endif

  if((VX_SUCCESS == status) && (obj->en_out_img_write == 1))
  {
    vx_char output_file_name[APP_MAX_FILE_PATH];

    APP_PRINTF("App Writing Outputs Start...\n");

#ifdef APP_TIDL_TRACE_DUMP
    snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_", obj->output_file_path, frame_id);
    vx_user_data_object trace_data = (vx_user_data_object)vxGetObjectArrayItem(obj->tidlObj.trace_data_arr, 0);
    tivx_utils_tidl_trace_write(trace_data, output_file_name);
    vxReleaseUserDataObject(&trace_data);
#endif

#ifdef WRITE_INTERMEDIATE_OUTPUTS
    snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_scaler_%dx%d.yuv", obj->output_file_path, frame_id, obj->scalerObj.output[0].width, obj->scalerObj.output1.height);
    writeScalerOutput(output_file_name, scalerObj->output[0].arr);

    snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/pre_proc_output_%010d", obj->output_file_path, frame_id);
    writePreProcOutput(output_file_name, &obj->preProcObj);

    snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/tidl_output_%010d", obj->output_file_path, frame_id);
    writeTIDLOutput(output_file_name, &obj->tidlObj);
#endif

    snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, frame_id);
    writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);

    APP_PRINTF("App Writing Outputs Done!\n");
  }

  appPerfPointEnd(&obj->total_perf);
  if (obj->test_mode == 1)
  {
      if (frame_id-obj->start_frame < (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])))
      {
          uint32_t actual_checksum = 0;
          /* Check the mosaic output image which contains everything */
          if (vx_false_e == app_test_check_image(obj->imgMosaicObj.output_image[0], checksums_expected[0][frame_id-obj->start_frame],
                                                  &actual_checksum))
          {
              test_result = vx_false_e;
          }
          /* in case test fails and needs to change */
          populate_gatherer(0, frame_id-obj->start_frame, actual_checksum);
      }
  }
  return status;
}
#else
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id)
{
  vx_status status = VX_SUCCESS;

  vx_char input_file_name[APP_MAX_FILE_PATH];
  vx_int32 obj_array_idx = -1;
  vx_uint32 actual_checksum = 0;

  ScalerObj    *scalerObj    = &obj->scalerObj;
  ImgMosaicObj *imgMosaicObj = &obj->imgMosaicObj;

  snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);

  appPerfPointBegin(&obj->total_perf);

  if(obj->pipeline < 0)
  {
    /* Enqueue outpus */
    if ((obj->en_out_img_write == 1) || (obj->test_mode == 1) || (obj->dump_output == 1))
    {
      status = vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->enqueueCnt], 1);
    }

    appPerfPointBegin(&obj->fileio_perf);
    /* Read input */
    readScalerInput(input_file_name, obj->input.arr[obj->enqueueCnt], APP_MODULES_READ_FILE, 0);

    appPerfPointEnd(&obj->fileio_perf);

    APP_PRINTF("App Reading Input Done!\n");

    /* Enqueue input - start execution */
    if(status == VX_SUCCESS)
    {
      status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&obj->input_images[obj->enqueueCnt], 1);
    }

    obj->enqueueCnt++;
    obj->enqueueCnt   = (obj->enqueueCnt  >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
    obj->pipeline++;
  }

  if(obj->pipeline >= 0)
  {
    vx_image scaler_input_image;
    vx_image mosaic_output_image;
    uint32_t num_refs;

    /* Dequeue input */
    if(status == VX_SUCCESS)
    {
      status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1, &num_refs);
    }

    if((obj->en_out_img_write == 1) || (obj->test_mode == 1) || (obj->dump_output == 1))
    {
      vx_char output_file_name[APP_MAX_FILE_PATH];

      /* Dequeue output */
      if (status == VX_SUCCESS)
      {
        status = vxGraphParameterDequeueDoneRef(obj->graph, imgMosaicObj->graph_parameter_index,
                                                (vx_reference*)&mosaic_output_image, 1, &num_refs);
      }
      /* Check that you are within the first n frames, where n is the number
          of samples in the checksums_expected */
      if (status == VX_SUCCESS)
      {
          if (obj->test_mode == 1 && (frame_id < (obj->start_frame + obj->num_frames - TEST_BUFFER)))
          {
            vx_uint32 expected_idx = frame_id - obj->start_frame - 1;
            if (app_test_check_image(mosaic_output_image, checksums_expected[obj->test_case][expected_idx],
                                      &actual_checksum) == vx_false_e)
            {
              test_result = vx_false_e;
            }
            /* in case test fails and needs to change */
            populate_gatherer(obj->test_case, expected_idx, actual_checksum);
          }
      }
      if ((obj->en_out_img_write == 1) && (status == VX_SUCCESS))
      {
        APP_PRINTF("App Writing Outputs Start...\n");
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, (frame_id - APP_BUFFER_Q_DEPTH));
        status = writeMosaicOutput(output_file_name, mosaic_output_image);
        APP_PRINTF("App Writing Outputs Done!\n");
      }

      if ((obj->dump_output == 1) && (status == VX_SUCCESS))
      {
        for (int i = 0; i < NUMBER_OF_FRAMES_DUMPS; i++)
        {
          vx_uint32 expected_idx = frame_id - obj->start_frame - 1;

          if (gDumpOutputFramesInfo[i].frame_index == expected_idx && gDumpOutputFramesInfo[i].is_dumped == vx_false_e)
          {
            printf("Dumping output for frame index : %d\n", gDumpOutputFramesInfo[i].frame_index);
            snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, (frame_id - APP_BUFFER_Q_DEPTH));
            status = writeMosaicOutput(output_file_name, mosaic_output_image);
            gDumpOutputFramesInfo[i].is_dumped = vx_true_e;
            printf("App Dumping Outputs Done!\n");
          }
        }
      }

      /* Enqueue output */
      if(status == VX_SUCCESS)
      {
        status = vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index,
                                                 (vx_reference*)&mosaic_output_image, 1);
      }
    }

    appPerfPointBegin(&obj->fileio_perf);

    if (status == VX_SUCCESS)
    {
        app_find_object_array_index(obj->input.arr, (vx_reference)scaler_input_image, APP_BUFFER_Q_DEPTH, &obj_array_idx);
    }
    if((obj_array_idx != -1) && (status == VX_SUCCESS))
    {
      status = readScalerInput(input_file_name, obj->input.arr[obj_array_idx], APP_MODULES_READ_FILE, 0);
    }

    appPerfPointEnd(&obj->fileio_perf);

    APP_PRINTF("App Reading Input Done!\n");

    /* Enqueue input - start execution */
    if(status == VX_SUCCESS)
    {
      status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1);
    }

    obj->enqueueCnt++;
    obj->dequeueCnt++;

    obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
    obj->dequeueCnt = (obj->dequeueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->dequeueCnt;

  }

  appPerfPointEnd(&obj->total_perf);

  APP_PRINTF("App Process Graph Done!\n");

  return status;
}
#endif

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint64_t cur_time;

    vx_int32 frame_id = obj->start_frame;
    vx_int32 x = 0;

#ifdef APP_ENABLE_PIPELINE_FLOW
    app_pipeline_params_defaults(obj);
#endif

    for(x = 0; x < obj->num_iterations; x++)
    {
        for(frame_id = obj->start_frame; frame_id < (obj->start_frame + obj->num_frames); frame_id++)
        {
            APP_PRINTF("Running frame %d\n", frame_id);

            vx_int32 count = 1;

            while ((count > 0) && (status == VX_SUCCESS))
            {
                cur_time = tivxPlatformGetTimeInUsecs();

                #ifdef APP_ENABLE_PIPELINE_FLOW
                status = app_run_graph_for_one_frame_pipeline(obj, frame_id);
                #else
                status = app_run_graph_for_one_frame_sequential(obj, frame_id);
                #endif

                cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
                /* convert to msecs */
                cur_time = cur_time/1000;

                if(cur_time < obj->delay_in_msecs)
                {
                    tivxTaskWaitMsecs(obj->delay_in_msecs - cur_time);
                }

                APP_PRINTF("app_tidl_seg: Frame ID %d of %d ... Done.\n", frame_id, obj->start_frame + obj->num_frames);

                count--;
            }
            /* user asked to stop processing */
            if(obj->stop_task)
            {
              break;
            }
        }

        APP_PRINTF("app_tidl_seg: Iteration %d of %d ... Done.\n", x, obj->num_iterations);

        if(obj->stop_task)
        {
          break;
        }
    }

#ifdef APP_ENABLE_PIPELINE_FLOW
    vxWaitGraph(obj->graph);
#endif

    obj->stop_task = 1;

    return status;
}

static void set_scaler_defaults(ScalerObj *scalerObj)
{
    scalerObj->color_format = VX_DF_IMAGE_NV12;
}

static void set_pre_proc_defaults(PreProcObj *preProcObj)
{
    vx_int32 i;
    for(i = 0; i < 4; i++ )
    {
        preProcObj->params.pad_pixel[i] = 0;
    }

    for(i = 0; i< 3 ; i++){
      preProcObj->params.scale_val[i] = 1.0;
      preProcObj->params.mean_pixel[i] = 0.0;
    }

    preProcObj->params.ip_rgb_or_yuv = 1;
    preProcObj->params.color_conv_flag = TIADALG_COLOR_CONV_YUV420_BGR;

    /* Number of time to clear the output buffer before it gets reused */
    preProcObj->params.clear_count  = 4;
}

static void update_pre_proc_params(AppObj *obj, PreProcObj *preProcObj)
{

}

static void update_img_mosaic_defaults(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
  vx_int32 idx = 0;
  imgMosaicObj->out_width    = DISPLAY_WIDTH;
  imgMosaicObj->out_height   = DISPLAY_HEIGHT;
  imgMosaicObj->num_inputs   = 1;

  tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

  imgMosaicObj->params.windows[idx].startX  = 500;
  imgMosaicObj->params.windows[idx].startY  = 200;
  imgMosaicObj->params.windows[idx].width   = obj->scalerObj.output[1].width;
  imgMosaicObj->params.windows[idx].height  = obj->scalerObj.output[1].height;
  imgMosaicObj->params.windows[idx].input_select   = 0;
  imgMosaicObj->params.windows[idx].channel_select = 0;
  idx++;

  imgMosaicObj->params.num_windows  = idx;

  /* Number of time to clear the output buffer before it gets reused */
  imgMosaicObj->params.clear_count  = 4;
}

static void set_post_proc_defaults(PostProcObj *postProcObj)
{
  int32_t i;

  postProcObj->viz_params.ip_rgb_or_yuv = 1;
  postProcObj->viz_params.op_rgb_or_yuv = 1;

  postProcObj->viz_params.num_input_tensors  = 1;
  postProcObj->viz_params.num_output_tensors = 1;

  postProcObj->viz_params.tidl_8bit_16bit_flag = 0;

  postProcObj->viz_params.num_classes[0] = 5; //5 classes for semantic segmentation
  postProcObj->viz_params.num_classes[1] = 0;
  postProcObj->viz_params.num_classes[2] = 0;

  for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++){
    postProcObj->viz_params.valid_region[i][0] = 0;
    postProcObj->viz_params.valid_region[i][1] = 0;
    postProcObj->viz_params.valid_region[i][2] = 0;
    postProcObj->viz_params.valid_region[i][3] = 0;
  }
}

static void update_post_proc_params(AppObj *obj, PostProcObj *postProcObj)
{
  vx_int32 i;

  postProcObj->out_width  = obj->scalerObj.output[1].width;
  postProcObj->out_height = obj->scalerObj.output[1].height;

  memcpy(postProcObj->viz_params.color_map[0], color_map, postProcObj->viz_params.num_classes[0]*3*sizeof(vx_uint8));
  postProcObj->viz_params.max_value[0] = postProcObj->viz_params.num_classes[0] - 1;


  for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++){
    postProcObj->viz_params.valid_region[i][0] = 0;
    postProcObj->viz_params.valid_region[i][1] = 0;
    postProcObj->viz_params.valid_region[i][2] = obj->scalerObj.output[0].width  - 1;
    postProcObj->viz_params.valid_region[i][3] = obj->scalerObj.output[0].height - 1;
  }
}

static void set_display_defaults(DisplayObj *displayObj)
{
  displayObj->display_option = 0;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
  obj->pipeline       = -APP_BUFFER_Q_DEPTH;
  obj->enqueueCnt     = 0;
  obj->dequeueCnt     = 0;
}

static void app_default_param_set(AppObj *obj)
{
    set_scaler_defaults(&obj->scalerObj);

    set_pre_proc_defaults(&obj->preProcObj);

    set_post_proc_defaults(&obj->postProcObj);

    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->delay_in_msecs = 0;
    obj->num_iterations = 1;
    obj->is_interactive = 0;
    obj->test_mode      = 0;
}

static void app_update_param_set(AppObj *obj)
{
    update_pre_proc_params(obj, &obj->preProcObj);

    update_post_proc_params(obj, &obj->postProcObj);

    update_img_mosaic_defaults(obj, &obj->imgMosaicObj);

    if(obj->is_interactive)
    {
      obj->num_iterations = 1000000000;
    }
}

void app_find_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx)
{
  vx_int32 i;

  *array_idx = -1;
  for(i = 0; i < array_size; i++)
  {
    vx_image img_ref = (vx_image)vxGetObjectArrayItem((vx_object_array)object_array[i], 0);
    if(ref == (vx_reference)img_ref)
    {
      *array_idx = i;
      vxReleaseImage(&img_ref);
      break;
    }
    vxReleaseImage(&img_ref);
  }
}
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{

  appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

  if(update_type == 0)
  {
    Draw2D_FontPrm sHeading;

    sHeading.fontIdx = 4;
    Draw2D_drawString(handle, 560, 5, "TIDL - Semantic Segmentation Demo", &sHeading);

  }

  return;
}
#endif
