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



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_host_utils.h"
#include <TI/tivx_img_proc.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>

#include <TI/j7_tidl.h>
#include <tivx_utils_file_rd_wr.h>

#include <utils/draw2d/include/draw2d.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_get.h>
#include <utils/grpx/include/app_grpx.h>
#include <VX/vx_khr_pipelining.h>

#include "itidl_ti.h"
#include "tiadalg_interface.h"

#include "app_common.h"
#include "app_scaler_module.h"
#include "app_pre_proc_module.h"
#include "app_tidl_module.h"
#include "app_pose_calc_module.h"
#include "app_pose_viz_module.h"
#include "app_img_mosaic_module.h"
#include "app_display_module.h"
#include "app_dump_output.h"

/*Maximum tensor for TIDL and is made dependent maximum input tensor for visualization (or TIDL output)*/
#define TIDL_NODE_MAX_TENSORS        (2)
#define APP_MAX_TENSOR_DIMS          (4u)
#define TIDL_NODE_MAX_PARAMS         (16u)

#ifndef x86_64
#define APP_ENABLE_PIPELINE_FLOW
#endif

#define ENABLE_DISPLAY

typedef struct {
    vx_object_array arr[APP_MAX_BUFQ_DEPTH];

    vx_int32 width;
    vx_int32 height;

} InputObj;

typedef struct {

    ScalerObj   scalerObj;
    PreProcObj  preProcObj;
    TIDLObj     tidlObj;
    PoseCalcObj poseCalcObj;
    PoseVizObj  poseVizObj;
    ImgMosaicObj imgMosaicObj;
    DisplayObj  displayObj;

    InputObj input;

    /* Arrray of 0th index reference of input.arr */
    vx_image input_images[APP_MAX_BUFQ_DEPTH];

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    char input_file_path[APP_MAX_FILE_PATH];
    char output_file_path[APP_MAX_FILE_PATH];

    char lo_res_desc_head_name[APP_MAX_FILE_PATH];
    char hi_res_score_head_name[APP_MAX_FILE_PATH];

    vx_uint32 win_pos_x;
    vx_uint32 win_pos_y;

    vx_int32 start_frame;
    vx_int32 num_frames;
    vx_int32 skip_frames;

    vx_int32 raw_tidl_op;

    vx_int32 en_out_img_write;
    vx_int32 dump_output;

    vx_uint32 delay_in_msecs;
    vx_uint32 num_iterations;
    vx_uint32 is_interactive;

    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t pipeline;

    int32_t enqueueCnt;
    int32_t dequeueCnt;
} AppObj;

AppObj gAppObj;
static void app_parse_cfg_file(AppObj *obj, vx_char *cfg_file_name);
static void app_show_usage(vx_int32 argc, vx_char* argv[]);
static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[]);
static vx_int32 app_init(AppObj *obj);
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

#ifdef ENABLE_DISPLAY
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
#endif

#ifdef APP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);
#else
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id);
#endif


static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" Visual Localization Demo - (c) Texas Instruments 2020\n");
    printf(" =====================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ========================="
    "\n Visual Localization Demo "
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

    while(!obj->stop_task)
    {
        app_run_graph(obj);
    }
    obj->stop_task_done = 1;
}

static int32_t app_run_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    int32_t status;

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
    vx_status status;
    uint32_t done = 0;
    char ch;
    FILE *fp;
    app_perf_point_t *perf_arr[1];

    status = app_run_task_create(obj);
    if(status!=0)
    {
        printf("app_tidl_vl: ERROR: Unable to create task\n");
    }
    else
    {
        appPerfStatsResetAll();
        while(!done)
        {
            printf(menu);
            ch = getchar();
            printf("\n");

            switch(ch)
            {
                case 'p':
                    appPerfStatsPrintAll();
                    tivx_utils_graph_perf_print(obj->graph);
                    appPerfPointPrint(&obj->fileio_perf);
                    appPerfPointPrint(&obj->total_perf);
                    printf("\n");
                    appPerfPointPrintFPS(&obj->total_perf);
                    appPerfPointReset(&obj->total_perf);
                    printf("\n");

                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_vl");
                    if (NULL != fp)
                    {
                        appPerfStatsExportAll(fp, perf_arr, 1);
                        tivx_utils_graph_perf_export(fp, obj->graph);
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
static void app_parse_cfg_file(AppObj *obj, vx_char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    vx_char line_str[1024];
    vx_char *token;
    vx_int32 i;

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
            if(strcmp(token, "tidl_config_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->tidlObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_network_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->tidlObj.network_file_path, token);
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
            if(strcmp(token, "top_view_img")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseVizObj.top_view_img, token);
                }
            }
            else
            if(strcmp(token, "input_voxel_info_file")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_voxel_info_file, token);
                }
            }
            else
            if(strcmp(token, "input_map_feat_pt_file")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_map_feat_pt_file, token);
                }
            }
            else
            if(strcmp(token, "input_map_feat_desc_file")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_map_feat_desc_file, token);
                }
            }
            else
            if(strcmp(token, "input_lens_dist_table_file")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_lens_dist_table_file, token);
                }
            }
            else
            if(strcmp(token, "lens_dist_table_size")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.lens_dist_table_size = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_voxels")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.num_voxels = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_map_feat")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.num_map_feat = atoi(token);
                }
            }
            else
            if(strcmp(token, "max_frame_feat")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.max_frame_feat = atoi(token);
                }
            }
            else
            if(strcmp(token, "max_map_feat")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.max_map_feat = atoi(token);
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
                    obj->scalerObj.output[0].width     = width;
                    obj->poseCalcObj.params.dl_width = width;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if(token[strlen(token)-1] == '\n')
                            token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->scalerObj.output[0].height  = height;
                        obj->poseCalcObj.params.dl_height  = height;
                    }
                }
            }
            else
            if(strcmp(token, "win_size")==0)
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
            if(strcmp(token, "out_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                    width =  atoi(token);
                    obj->poseCalcObj.params.img_width = width;
                    obj->poseVizObj.params.img_width  = width;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if(token[strlen(token)-1] == '\n')
                            token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->poseCalcObj.params.img_height = height;
                        obj->poseVizObj.params.img_height  = height;
                    }
                }
            }
            else
            if(strcmp(token, "win_pos")==0)
            {
                vx_int32 x, y;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                    x =  atoi(token);
                    obj->win_pos_x  = x;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if(token[strlen(token)-1] == '\n')
                            token[strlen(token)-1]=0;

                        y =  atoi(token);
                        obj->win_pos_y  = y;
                    }
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
            if(strcmp(token, "skip_frames")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->skip_frames = atoi(token);
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
            if(strcmp(token, "num_voxels")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.num_voxels = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_map_feat")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.num_map_feat = atoi(token);
                }
            }
            else
            if(strcmp(token, "init_est")==0)
            {
                for(i = 0; i < 3; i++)
                {
                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        obj->poseCalcObj.params.init_est[i] = atof(token);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            if(strcmp(token, "input_upsampling_weight")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_upsampling_weight, token);
                }
            }
            else
            if(strcmp(token, "input_upsampling_bias")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->poseCalcObj.input_upsampling_bias, token);
                }
            }
            else
            if(strcmp(token, "is_ip_fe")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.is_ip_fe = atoi(token);
                }
            }
            else
            if(strcmp(token, "score_th")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.score_th = atoi(token);
                }
            }
            else
            if(strcmp(token, "filter_scale_pw2")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.filter_scale_pw2 = atoi(token);
                }
            }
            else
            if(strcmp(token, "hi_res_desc_scale_pw2")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.hi_res_desc_scale_pw2 = atoi(token);
                }
            }
            else
            if(strcmp(token, "lo_res_desc_head_name")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->lo_res_desc_head_name, token);
                }
            }
            else
            if(strcmp(token, "hi_res_score_head_name")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->hi_res_score_head_name, token);
                }
            }
            else
            if(strcmp(token, "raw_tidl_op")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->raw_tidl_op = atoi(token);
                }
            }
            else
            if(strcmp(token, "pose_calc_skip_flag")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->poseCalcObj.params.skip_flag = atoi(token);
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
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;
    vx_bool dump_output = vx_false_e;


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
        if(strcmp(argv[i], "--dump")==0)
        {
            dump_output = vx_true_e;
        }
    }

    if (dump_output == vx_true_e)
    {
        obj->dump_output = 1;
        obj->num_iterations = 1;
        obj->is_interactive = 0;
        obj->start_frame = 0;
        obj->skip_frames = 1;
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
}

vx_int32 app_tidl_vl_main(vx_int32 argc, vx_char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;
    vx_int32 retval = 0;

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

    if (VX_SUCCESS == status)
    {
        status = app_create_graph(obj);
        APP_PRINTF("App Create Graph Done! \n");
    }

    if (VX_SUCCESS == status)
    {
        status = app_verify_graph(obj);
        APP_PRINTF("App Verify Graph Done! \n");
    }

    if (VX_SUCCESS == status)
    {
        if(obj->is_interactive)
        {
            app_run_graph_interactive(obj);
        }
        else
        {
            app_run_graph(obj);
        }

        APP_PRINTF("App Run Graph Done! \n");
    }

    if (VX_SUCCESS == status)
    {
        app_delete_graph(obj);
        APP_PRINTF("App Delete Graph Done! \n");

        app_deinit(obj);
        APP_PRINTF("App De-init Done! \n");
    }

    if (VX_SUCCESS != status)
    {
        APP_PRINTF("app_tidl_vl_main Failed! \n");
        retval = -1;
    }

    return retval;
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
static vx_int32 app_init(AppObj *obj)
{
    int status = VX_SUCCESS;
#ifdef ENABLE_DISPLAY
    app_grpx_init_prms_t grpx_prms;
#endif

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxHwaLoadKernels(obj->context);
    tivxVideoIOLoadKernels(obj->context);
    tivxImgProcLoadKernels(obj->context);
    tivxTIDLLoadKernels(obj->context);

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

    /* Initialize modules */
    status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", NUM_CH, 2);
    APP_PRINTF("Scaler Init Done! \n");

    if(status == VX_SUCCESS)
    {
        /* Initialize TIDL first to get tensor I/O information from network */
        status = app_init_tidl(obj->context, &obj->tidlObj, &obj->poseCalcObj.params,
                    obj->lo_res_desc_head_name,obj->hi_res_score_head_name,
                    "tidl_obj");

        APP_PRINTF("TIDL Init Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        /* Update pre-proc parameters with TIDL config before calling init */
        status = app_update_pre_proc(obj->context, &obj->preProcObj, obj->tidlObj.config);
        APP_PRINTF("Pre Proc Update Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_init_pre_proc(obj->context, &obj->preProcObj, "pre_proc_obj");
        APP_PRINTF("Pre Proc Init Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        /* Update post-proc parameters with TIDL config before calling init */
        status = app_update_pose_calc(obj->context, &obj->poseCalcObj);
        APP_PRINTF("Pose calc Update Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_init_pose_calc(obj->context, &obj->poseCalcObj, "pose_calc_obj");
        APP_PRINTF("Pose calc Init Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        /* Update post-proc parameters with TIDL config before calling init */
        status = app_update_pose_viz(obj->context, &obj->poseVizObj);
        APP_PRINTF("Pose viz Update Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_init_pose_viz(obj->context, &obj->poseVizObj, "pose_viz_obj", APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Pose viz Init Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_init_img_mosaic(obj->context, &obj->imgMosaicObj, "sw_mosaic_obj", APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Img Mosaic Init Done! \n");
    }

    if(status == VX_SUCCESS)
    {
    #ifdef ENABLE_DISPLAY
        status = app_init_display(obj->context, &obj->displayObj, "display_obj");
        APP_PRINTF("Display Init Done! \n");

        #ifndef x86_64
        if(obj->displayObj.display_option == 1)
        {
          appGrpxInitParamsInit(&grpx_prms, obj->context);
          grpx_prms.draw_callback = app_draw_graphics;
          appGrpxInit(&grpx_prms);
        }
        #endif
    #endif
    }

    if(status == VX_SUCCESS)
    {
        appPerfPointSetName(&obj->total_perf , "TOTAL");
        appPerfPointSetName(&obj->fileio_perf, "FILEIO");
    }

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
    APP_PRINTF("Pre proc deinit Done! \n");

    app_deinit_tidl(&obj->tidlObj);
    APP_PRINTF("TIDL deinit Done! \n");

    app_deinit_pose_calc(&obj->poseCalcObj);
    APP_PRINTF("Pose calc deinit Done! \n");

    app_deinit_pose_viz(&obj->poseVizObj, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Pose Viz deinit Done! \n");

    app_deinit_img_mosaic(&obj->imgMosaicObj, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Img Mosaic deinit Done! \n");

#ifdef ENABLE_DISPLAY
    app_deinit_display(&obj->displayObj);
    #ifndef x86_64
    if(obj->displayObj.display_option == 1)
    {
        appGrpxDeInit();
    }
    #endif
#endif
    tivxTIDLUnLoadKernels(obj->context);
    tivxImgProcUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
    APP_PRINTF("Context release Done! \n");
}

static void app_delete_graph(AppObj *obj)
{
    app_delete_scaler(&obj->scalerObj);

    app_delete_pre_proc(&obj->preProcObj);
    APP_PRINTF("Pre proc delete Done! \n");

    app_delete_tidl(&obj->tidlObj);
    APP_PRINTF("TIDL delete Done! \n");

    app_delete_pose_calc(&obj->poseCalcObj);
    APP_PRINTF("Pose calc delete Done! \n");

    app_delete_pose_viz(&obj->poseVizObj);
    APP_PRINTF("Pose Viz delete Done! \n");

    app_delete_img_mosaic(&obj->imgMosaicObj);
    APP_PRINTF("Img Mosaic delete Done! \n");

#ifdef ENABLE_DISPLAY
    app_delete_display(&obj->displayObj);
    APP_PRINTF("Display delete Done! \n");
#endif

    vxReleaseGraph(&obj->graph);
    APP_PRINTF("Graph release Done! \n");
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
    vxSetReferenceName((vx_reference)obj->graph, "app_tidl_vl_graph");

    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, obj->input.arr[0]);

    app_create_graph_pre_proc(obj->graph, &obj->preProcObj, obj->scalerObj.output[0].arr);
    APP_PRINTF("Pre Proc create graph Done! \n");

    app_create_graph_tidl(obj->graph, &obj->tidlObj, obj->preProcObj.output_tensor_arr);
    APP_PRINTF("TIDL create graph Done! \n");

    app_create_graph_pose_calc(obj->graph, &obj->poseCalcObj, obj->poseCalcObj.input_tensor_arr,
                                obj->tidlObj.output_tensor_arr, &obj->tidlObj.out_args_arr);
    APP_PRINTF("Pose Calc create graph Done! \n");

    app_create_graph_pose_viz(obj->graph, &obj->poseVizObj, obj->poseCalcObj.output_tensor_arr);
    APP_PRINTF("Pose Viz create graph Done! \n");

    vx_int32 idx = 0;
    obj->imgMosaicObj.input_arr[idx++] = obj->poseVizObj.output_image_arr[0];
    obj->imgMosaicObj.input_arr[idx++] = obj->scalerObj.output[1].arr;
    obj->imgMosaicObj.num_inputs = idx;

    app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);

#ifdef ENABLE_DISPLAY
    app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);
    APP_PRINTF("Pose Viz create graph Done! \n");
#endif

#ifdef APP_ENABLE_PIPELINE_FLOW
    /* Scalar Node - input is in Index 0 */
    graph_parameter_index = 0;
    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 0);
    obj->scalerObj.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->input_images[0];
    graph_parameter_index++;

    if ((obj->en_out_img_write == 1) || (obj->dump_output == 1))
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

    tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, APP_BUFFER_Q_DEPTH);
    tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, APP_MAX_BUFQ_DEPTH);

    tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 2, APP_BUFFER_Q_DEPTH);

    tivxSetNodeParameterNumBufByIndex(obj->tidlObj.node, 7, APP_BUFFER_Q_DEPTH);
    tivxSetNodeParameterNumBufByIndex(obj->tidlObj.node, 8, APP_BUFFER_Q_DEPTH);

    tivxSetNodeParameterNumBufByIndex(obj->poseCalcObj.node, 9, APP_BUFFER_Q_DEPTH);

    tivxSetNodeParameterNumBufByIndex(obj->poseVizObj.node, 3, APP_BUFFER_Q_DEPTH);

    if(!((obj->en_out_img_write == 1) || (obj->dump_output == 1)))
    {
        tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, 4);
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

    if(status == VX_SUCCESS)
    {
        APP_PRINTF("App Verify Graph Done!\n");
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_vl");
    }

    scalerObj = &obj->scalerObj;

    refs[0] = (vx_reference)scalerObj->coeff_obj;

    status = tivxNodeSendCommand(scalerObj->node, 0u,
                                 TIVX_VPAC_MSC_CMD_SET_COEFF,
                                 refs, 1u);

    APP_PRINTF("App Send MSC Command Done!\n");

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

    appPerfPointBegin(&obj->fileio_perf);

    readScalerInput(input_file_name, obj->input.arr[0], APP_MODULES_READ_FILE, NUM_CH);

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

#ifdef APP_TIDL_TRACE_DUMP
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_", obj->output_file_path, frame_id);
        vx_user_data_object trace_data = (vx_user_data_object)vxGetObjectArrayItem(obj->tidlObj.trace_data_arr, 0);
        tivx_utils_tidl_trace_write(trace_data, output_file_name);
        vxReleaseUserDataObject(&trace_data);
#endif
        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, frame_id);

        printf("Writing %s... ", output_file_name);
        writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);
        printf("Done!\n");

    }

    return status;
}
#else
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id)
{
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];
    vx_int32 obj_array_idx = -1;

    ScalerObj  *scalerObj  = &obj->scalerObj;
    ImgMosaicObj *imgMosaicObj = &obj->imgMosaicObj;

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);

    if(obj->pipeline < 0)
    {
        /* Enqueue outpus */
        if ((obj->en_out_img_write == 1) || (obj->dump_output == 1))
        {
            vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->enqueueCnt], 1);
        }

        appPerfPointBegin(&obj->fileio_perf);
        /* Read input */
        readScalerInput(input_file_name, obj->input.arr[obj->enqueueCnt], APP_MODULES_READ_FILE, 0);

        appPerfPointEnd(&obj->fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&obj->input_images[obj->enqueueCnt], 1);

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
        vxGraphParameterDequeueDoneRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1, &num_refs);

        if((obj->en_out_img_write == 1) || (obj->dump_output == 1))
        {
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output */
            vxGraphParameterDequeueDoneRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);

            if (obj->en_out_img_write == 1)
            {
                APP_PRINTF("App Writing Outputs Start...\n");

                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, (frame_id - APP_BUFFER_Q_DEPTH));
                writeMosaicOutput(output_file_name, mosaic_output_image);

                APP_PRINTF("App Writing Outputs Done!\n");
            }

            if (obj->dump_output == 1)
            {
                for (int i = 0; i < NUMBER_OF_FRAMES_DUMPS; i++)
                {
                    vx_uint32 expected_idx = frame_id - obj->start_frame - 1;

                    if (gDumpOutputFramesInfo[i].frame_index == expected_idx && gDumpOutputFramesInfo[i].is_dumped == vx_false_e)
                    {
                        printf("Dumping output for frame index : %d\n", gDumpOutputFramesInfo[i].frame_index);
                        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, (frame_id - APP_BUFFER_Q_DEPTH));
                        writeMosaicOutput(output_file_name, mosaic_output_image);
                        gDumpOutputFramesInfo[i].is_dumped = vx_true_e;
                        printf("App Dumping Outputs Done!\n");
                    }
                }
            }

            /* Enqueue output */
            vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1);
        }

        appPerfPointBegin(&obj->fileio_perf);

        app_find_object_array_index(obj->input.arr, (vx_reference)scaler_input_image, APP_BUFFER_Q_DEPTH, &obj_array_idx);
        if(obj_array_idx != -1) {
            readScalerInput(input_file_name, obj->input.arr[obj_array_idx], APP_MODULES_READ_FILE, 0);
        }

        appPerfPointEnd(&obj->fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1);

        obj->enqueueCnt++;
        obj->dequeueCnt++;

        obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->dequeueCnt = (obj->dequeueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->dequeueCnt;
    }

    APP_PRINTF("App Process Graph Done!\n");

    return status;
}
#endif

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint64_t t0, t1;

    vx_int32 frame_id = obj->start_frame;
    vx_int32 x = 0;

    readPoseVizBackgroundImage(obj->poseVizObj.top_view_img, obj->poseVizObj.bg_image);

#ifdef APP_ENABLE_PIPELINE_FLOW
    app_pipeline_params_defaults(obj);
#endif

    for(x = 0; x < obj->num_iterations; x++)
    {
        for(frame_id = obj->start_frame; frame_id < (obj->start_frame + obj->num_frames); frame_id+=obj->skip_frames)
        {
            APP_PRINTF("Running frame %d\n", frame_id);

            appPerfPointBegin(&obj->total_perf);

            t0 = tivxPlatformGetTimeInUsecs();

            #ifdef APP_ENABLE_PIPELINE_FLOW
            status = app_run_graph_for_one_frame_pipeline(obj, frame_id);
            #else
            status = app_run_graph_for_one_frame_sequential(obj, frame_id);
            #endif

            t1 = tivxPlatformGetTimeInUsecs();
            /* convert to msecs */
            vx_uint32 cur_time = (t1 - t0)/1000;

            if(cur_time < obj->delay_in_msecs)
            {
                tivxTaskWaitMsecs(obj->delay_in_msecs - cur_time);
            }

            appPerfPointEnd(&obj->total_perf);

            APP_PRINTF("app_tidl_vl: Frame ID %d of %d ... Done.\n", frame_id, obj->start_frame + obj->num_frames);

            /* user asked to stop processing */
            if((obj->stop_task) || (status == VX_FAILURE))
            {
              break;
            }
        }

        printf("app_tidl_vl: Iteration %d of %d ... Done.\n", x, obj->num_iterations);

        if(obj->stop_task)
          break;
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

static void update_img_mosaic_defaults(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx = 0;
    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->num_inputs   = 2;

    imgMosaicObj->params.windows[idx].startX         = 0;
    imgMosaicObj->params.windows[idx].startY         = 56;
    imgMosaicObj->params.windows[idx].width          = 1920;
    imgMosaicObj->params.windows[idx].height         = 1024;
    imgMosaicObj->params.windows[idx].input_select   = 0;
    imgMosaicObj->params.windows[idx].channel_select = 0;
    imgMosaicObj->params.windows[idx].enable_roi     = 1;
    imgMosaicObj->params.windows[idx].roiStartX      = 128;
    imgMosaicObj->params.windows[idx].roiStartY      = 0;
    imgMosaicObj->params.windows[idx].roiWidth       = 1920;
    imgMosaicObj->params.windows[idx].roiHeight      = 1024;
    idx++;

    imgMosaicObj->params.windows[idx].startX         = obj->win_pos_x;
    imgMosaicObj->params.windows[idx].startY         = obj->win_pos_y;
    imgMosaicObj->params.windows[idx].width          = obj->scalerObj.output[1].width;
    imgMosaicObj->params.windows[idx].height         = obj->scalerObj.output[1].height;
    imgMosaicObj->params.windows[idx].input_select   = 1;
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = 4;
    imgMosaicObj->params.num_msc_instances = 1;
    imgMosaicObj->params.msc_instance = 1; /* Use MSC1 instance as MSC0 is used for scaler */
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

    set_pose_calc_defaults(&obj->poseCalcObj);

    set_pose_viz_defaults(&obj->poseVizObj);

    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->delay_in_msecs = 0;
    obj->num_iterations = 1;
    obj->is_interactive = 0;

    obj->win_pos_x = 32;
    obj->win_pos_y = 64;
}

static void app_update_param_set(AppObj *obj)
{
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
#ifdef ENABLE_DISPLAY
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        Draw2D_FontPrm sLabel;

        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 600, 5, "Visual Localization Demo", &sHeading);

        sLabel.fontIdx = 3;
        Draw2D_drawString(handle, 1322, 104, "Front camera", &sLabel);
    }

    return;
}
#endif
#endif
