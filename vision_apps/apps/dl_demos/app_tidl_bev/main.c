/*
 *
 * Copyright (c) 2025-2026 Texas Instruments Incorporated
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
#include <TI/hwa_kernels.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>

#include "bev_common.h"
#include "bev_scaler_module.h"
#include "bev_pre_proc_module.h"
#include "bev_tidl_module.h"
#include "bev_post_proc_module.h"
#include "bev_img_mosaic_module.h"
#include "bev_display_module.h"
#include "bev_test.h"
#include "bev_post_proc_module.h"

#ifndef x86_64
#define AVP_ENABLE_PIPELINE_FLOW
//#define WRITE_INTERMEDIATE_OUTPUTS
#endif

#define LIDAR_XYCORD_PER_FRAME
#define AVP_BUFFER_Q_DEPTH   (7)
#define AVP_PIPELINE_DEPTH   (5)

typedef struct {

    ScalerObj  scalerObj;
    ScalerObj  scalerObj_b2;
    ScalerObj  scalerObj_b3;
    ScalerObj  scalerObj_b4;
    ScalerObj  scalerObj_b5;
    ScalerObj  scalerObj_b6;
    ScalerObj  scalerObj_lidar_canvas;

    PreProcObj preProcObj;
    PreProcObj preProcObj_b2;
    PreProcObj preProcObj_b3;
    PreProcObj preProcObj_b4;
    PreProcObj preProcObj_b5;
    PreProcObj preProcObj_b6;


    TIDLObj TIDLObj;

    BEVPostProcObj BEVPostProcObj;
    BEVPostProcObj BEVPostProcObj_cam;

    ImgMosaicObj imgMosaicObj;

    DisplayObj displayObj;

    vx_char input_file_path[APP_MAX_FILE_PATH];
    vx_char output_file_path[APP_MAX_FILE_PATH];

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    vx_image   background;
    vx_int32 en_out_img_write;

    vx_float32 viz_th_BEV;

    vx_int32 num_classes[TIVX_PIXEL_VIZ_MAX_CLASS];
    vx_int32 valid_region[TIVX_PIXEL_VIZ_MAX_CLASS][4];

    vx_int32 ip_rgb_or_yuv;
    vx_int32 op_rgb_or_yuv;

    vx_int32 start_frame;
    vx_int32 num_frames;

    vx_uint32 delay_in_msecs;
    vx_uint32 num_iterations;
    vx_uint32 is_interactive;
    vx_uint32 test_mode;
    vx_uint32 enable_gui;
    vx_uint32 BATCH_ENABLED;


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

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[]);
static int app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static vx_status app_run_graph_interactive(AppObj *obj);
static void app_delete_graph(AppObj *obj);
static void app_default_param_set(AppObj *obj);
static void app_update_param_set(AppObj *obj);
static vx_status readTensorInput(char* file_name, vx_object_array img_arr, int32_t ch_num);
#ifdef AVP_ENABLE_PIPELINE_FLOW
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_find_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
#ifdef LIDAR_XYCORD_PER_FRAME
static void app_find_lidar_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
#endif
#endif
static void app_pipeline_params_defaults(AppObj *obj);
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
#ifdef AVP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);
#else
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id);
#endif

static char img_labels[TIVX_DL_BEV_POST_PROC_MAX_NUM_CLASSNAMES][TIVX_DL_BEV_POST_PROC_MAX_SIZE_CLASSNAME] = {""};
static int32_t label_offset3[TIVX_DL_BEV_POST_PROC_MAX_NUM_CLASSNAMES] = {0};

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL BEV Demo  - (c) Texas Instruments 2025\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ========================="
    "\n Demo : TIDL FAST BEV Demo "
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
    vx_status status;
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
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_bev");
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
    snprintf(obj->TIDLObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->TIDLObj.network_file_path,APP_MAX_FILE_PATH, ".");

    snprintf(obj->TIDLObj.XYCordinate_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->BEVPostProcObj.Lidar_2_img_file_path,APP_MAX_FILE_PATH, ".");
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
            if(strcmp(token, "tidl_bev_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->TIDLObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_bev_network")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->TIDLObj.network_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_BEV_XYCordinate")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->TIDLObj.XYCordinate_file_path, token);
                    strcpy(obj->BEVPostProcObj.Lidar_2_img_file_path, token);
                    
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
            if(strcmp(token, "batch_enable")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->BATCH_ENABLED = atoi(token);
                }
            }
            else
            if(strcmp(token, "lidar_points")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->BEVPostProcObj.lidar_points[0] = atoi(token);
                    //obj->BEVPostProcObj_cam.lidar_points[0] = atoi(token);
                }
            }
            else
            if(strcmp(token, "lidar2cam_points")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->BEVPostProcObj_cam.lidar2cam_points[0] = atoi(token);
                }
            }
            if(strcmp(token, "xycord_size")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->TIDLObj.xycord_size[0] = atoi(token);
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
                    obj->scalerObj.input.width  = width;
                    obj->scalerObj_b2.input.width  = width;
                    obj->scalerObj_b3.input.width  = width;
                    obj->scalerObj_b4.input.width  = width;
                    obj->scalerObj_b5.input.width  = width;
                    obj->scalerObj_b6.input.width  = width;
                    obj->scalerObj_lidar_canvas.input.width  = width;
                    
                    
                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->scalerObj.input.height = height;
                        obj->scalerObj_b2.input.height = height;
                        obj->scalerObj_b3.input.height = height;
                        obj->scalerObj_b4.input.height = height;
                        obj->scalerObj_b5.input.height = height;
                        obj->scalerObj_b6.input.height = height;
                        obj->scalerObj_lidar_canvas.input.height = 1080;
                        
                           
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
                    obj->scalerObj.output.width   = width;
                    obj->scalerObj.output1.width   = width;
                    obj->scalerObj_b2.output.width   = width;
                    obj->scalerObj_b2.output1.width   = width;
                    obj->scalerObj_b3.output.width   = width;
                    obj->scalerObj_b3.output1.width   = width;
                    obj->scalerObj_b4.output.width   = width;
                    obj->scalerObj_b4.output1.width   = width;
                    obj->scalerObj_b5.output.width   = width;
                    obj->scalerObj_b5.output1.width   = width;
                    obj->scalerObj_b6.output.width   = width;
                    obj->scalerObj_b6.output1.width   = width;
                    obj->scalerObj_lidar_canvas.output.width    = 1920;
                    obj->scalerObj_lidar_canvas.output1.width   = 1920;
                    
                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->scalerObj.output.height  = height;
                        obj->scalerObj.output1.height  = height;
                        obj->scalerObj_b2.output.height  = height;
                        obj->scalerObj_b2.output1.height  = height;
                        obj->scalerObj_b3.output.height  = height;
                        obj->scalerObj_b3.output1.height  = height;
                        obj->scalerObj_b4.output.height  = height;
                        obj->scalerObj_b4.output1.height  = height;
                        obj->scalerObj_b5.output.height  = height;
                        obj->scalerObj_b5.output1.height  = height;
                        obj->scalerObj_b6.output.height  = height;
                        obj->scalerObj_b6.output1.height  = height;
                        obj->scalerObj_lidar_canvas.output.height   = 600;
                        obj->scalerObj_lidar_canvas.output1.height  = 600;
                                                
                    }
                }
            }
            else
            if(strcmp(token, "viz_th")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->viz_th_BEV = atof(token);
                }
            }
            else
            if(strcmp(token, "num_classes")==0)
            {
                vx_int32 i;

                for(i = 0; i < TIVX_PIXEL_VIZ_MAX_CLASS; i++)
                {
                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        obj->num_classes[i] = atoi(token);
                    }
                    else
                    {
                        break;
                    }
                }
                for(;i < TIVX_PIXEL_VIZ_MAX_CLASS; i++)
                {
                    obj->num_classes[i] = 0;
                }
            }
            else
            if(strcmp(token, "valid_region")==0)
            {
                vx_int32 i;
                for(i = 0; i < TIVX_PIXEL_VIZ_MAX_CLASS*4; i++)
                {
                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        obj->valid_region[i>>2][i&0x3] = atoi(token);
                    }
                    else
                    {
                        break;
                    }
                }
                for(;i < TIVX_PIXEL_VIZ_MAX_CLASS*4; i++)
                {
                    obj->valid_region[i>>2][i&0x3] = -1;
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
            if(strcmp(token, "ip_rgb_or_yuv")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->ip_rgb_or_yuv = atoi(token);
                }
            }
            else
            if(strcmp(token, "op_rgb_or_yuv")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->op_rgb_or_yuv = atoi(token);
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
                    {
                        obj->delay_in_msecs = 2000;
                    }
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
                    {
                        obj->num_iterations = 1;
                    }
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
                    {
                        obj->is_interactive = 1;
                    }
                }
            }
            else
            if(strcmp(token, "test_mode")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->test_mode = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_gui")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_gui = atoi(token);
                    if(obj->enable_gui==0)
                    {
                        /* if GUI is disable, this is most likely a dual display scenario, move display to pipe 0 in this case */
                        obj->displayObj.display_pipe_id = 0;
                    }
                }
            }
        }
    }
    if (obj->test_mode == 1)
    {
        obj->is_interactive = 0;
        /* display_option must be set to 1 in order for the checksums
                to come out correctly */
        obj->displayObj.display_option = 1;
        obj->num_iterations = 1;
        /* if testing, just run the number of frames that are found in the expected
            checksums + a BUFFER to maintain data integrity */
        obj->num_frames = sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])
                            + TEST_BUFFER;
    }
    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;
    vx_bool set_test_mode = vx_false_e;

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
    }
    if (set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->is_interactive = 0;
        obj->displayObj.display_option = 1;
        obj->num_iterations = 1;
        /* if testing, just run the number of frames that are found in the expected
            checksums + a BUFFER to maintain data integrity */
        obj->num_frames = sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])
                            + TEST_BUFFER;
    }

    #ifdef x86_64
    obj->displayObj.display_option = 0;
    obj->is_interactive = 0;
    #endif

    return;
}

vx_int32 app_tidl_bev_main(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;

    AppObj *obj = &gAppObj;

    /*Optional parameter setting*/
    app_default_param_set(obj);
    APP_PRINTF("App set default params Done! \n");

    /*Config parameter reading*/
    app_parse_cmd_line_args(obj, argc, argv);
    APP_PRINTF("App Parse User params Done! \n");

    /*Update of parameters are config file read*/
    app_update_param_set(obj);
    APP_PRINTF("App Update Params Done! \n");
    //tivx_set_debug_zone(VX_ZONE_INFO);

    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    if(status == VX_SUCCESS)
    {
        app_create_graph(obj);
        APP_PRINTF("App Create Graph Done! \n");
    }
    if(status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
        APP_PRINTF("App Verify Graph Done! \n");
    }
    if(status == VX_SUCCESS)
    {
        if(obj->is_interactive)
        {
            status = app_run_graph_interactive(obj);
        }
        else
        {
            if (app_run_graph(obj) != VX_SUCCESS)
            {
                test_result = vx_false_e;
            }
        }

        APP_PRINTF("App Run Graph Done! \n");
    }
    else
    {
        test_result = vx_false_e;
    }

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");
    if (obj->test_mode == 1)
    {
        if((test_result == vx_false_e) || (status != VX_SUCCESS))
        {
            printf("\n\nTEST FAILED\n\n");
            /* in the case that checksums changed, print a new checksums_expected
                array */
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
#ifdef AVP_ENABLE_PIPELINE_FLOW
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}
#endif
static vx_status fill_background_image(vx_image background)
{
    vx_status status = VX_SUCCESS;

    Draw2D_Handle  handle;
    Draw2D_BufInfo draw2dBufInfo;

    Draw2D_FontPrm sHeading;
    Draw2D_FontPrm sLabel;
    Draw2D_BmpPrm  bmpPrm;

    status = vxGetStatus((vx_reference)background);

    if(status == VX_SUCCESS)
    {
        vx_rectangle_t rect;
        vx_imagepatch_addressing_t luma_addr;
        vx_imagepatch_addressing_t cbcr_addr;
        vx_map_id luma_map_id;
        vx_map_id cbcr_map_id;
        void * luma_ptr;
        void * cbcr_ptr;
        vx_uint32  img_width;
        vx_uint32  img_height;

        vxQueryImage(background, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(background, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height;
        status = vxMapImagePatch(background,
                                &rect,
                                0,
                                &luma_map_id,
                                &luma_addr,
                                &luma_ptr,
                                VX_WRITE_ONLY,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);

        memset(luma_ptr, 0, (img_width * img_height));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height / 2;

        status = vxMapImagePatch(background,
                                &rect,
                                1,
                                &cbcr_map_id,
                                &cbcr_addr,
                                &cbcr_ptr,
                                VX_WRITE_ONLY,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);

        memset(cbcr_ptr, 128, (img_width * img_height/2));

        draw2dBufInfo.bufAddr[0]  = luma_ptr;
        draw2dBufInfo.bufAddr[1]  = cbcr_ptr;
        draw2dBufInfo.bufWidth    = img_width;
        draw2dBufInfo.bufHeight   = img_height;
        draw2dBufInfo.bufPitch[0] = luma_addr.stride_y;
        draw2dBufInfo.bufPitch[1] = cbcr_addr.stride_y;
        draw2dBufInfo.dataFormat  = DRAW2D_DF_YUV420SP_UV;
        draw2dBufInfo.transperentColor = 0x00000000;
        draw2dBufInfo.transperentColorFormat = DRAW2D_DF_BGR16_565;

        status = Draw2D_create(&handle);

        if(status==0)
        {
            if (luma_ptr != NULL && cbcr_ptr != NULL)
            {
                Draw2D_setBufInfo(handle, &draw2dBufInfo);

                bmpPrm.bmpIdx = DRAW2D_BMP_IDX_TI_LOGO_2;

                Draw2D_drawBmp(handle, 20, 0, &bmpPrm);

                sHeading.fontIdx = 10;

                Draw2D_drawString(handle, 560, 5, "Analytics for Auto Valet Parking", &sHeading);

                sLabel.fontIdx = 12;
                Draw2D_drawString(handle, 340, 90, "Left  camera", &sLabel);
                Draw2D_drawString(handle, 920, 90, "Front camera", &sLabel);
                Draw2D_drawString(handle, 1520,90, "Right camera", &sLabel);

                sLabel.fontIdx = 13;
                Draw2D_drawString(handle, 20, 244, "Semantic", &sLabel);
                Draw2D_drawString(handle, 0 , 264, "Segmentation", &sLabel);
                Draw2D_drawString(handle, 0 , 284, "  (768x384) ", &sLabel);

                Draw2D_drawString(handle, 20, 526, "Parking", &sLabel);
                Draw2D_drawString(handle, 35, 546, "Spot", &sLabel);
                Draw2D_drawString(handle, 10, 566, "Detection", &sLabel);
                Draw2D_drawString(handle, 10, 586, "(768x384)", &sLabel);

                Draw2D_drawString(handle, 25, 828, "Vehicle", &sLabel);
                Draw2D_drawString(handle, 15, 848, "Detection", &sLabel);
                Draw2D_drawString(handle, 15, 868, "(768x384)", &sLabel);

                sLabel.fontIdx = 12;
                Draw2D_drawString(handle, 340, 1000, "Camera resolution : 1280 x 720 (1 MP) , Analytics resolution : 768 x 384 (0.3 MP)", &sLabel);

                sLabel.fontIdx = 12;
                Draw2D_drawString(handle, 596, 1040, "Analytics performance : 3 CH, 3 algorithm, 15 FPS", &sLabel);
            }

            Draw2D_delete(handle);
        }

        vxUnmapImagePatch(background, luma_map_id);
        vxUnmapImagePatch(background, cbcr_map_id);
    }

    return (status);
}

static vx_status app_init(AppObj *obj)
{
    int status = VX_SUCCESS;
    app_grpx_init_prms_t grpx_prms;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxHwaLoadKernels(obj->context);
    tivxVideoIOLoadKernels(obj->context);
    tivxImgProcLoadKernels(obj->context);
    tivxTIDLLoadKernels(obj->context);

    /* Create a background image for display */
    obj->background = vxCreateImage(obj->context, DISPLAY_WIDTH, DISPLAY_HEIGHT ,VX_DF_IMAGE_NV12);
    status = vxGetStatus((vx_reference)obj->background);

    /* Initialize modules */
    if(status == VX_SUCCESS)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", AVP_BUFFER_Q_DEPTH,(NUM_CH+5));
    }
    
    APP_PRINTF("app_init_scaler_1st success\n");
    if(status == VX_SUCCESS)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj_b2, "scaler_obj_b2", AVP_BUFFER_Q_DEPTH,NUM_CH);
    }

    if(status == VX_SUCCESS)
    {   
        status = app_init_scaler(obj->context, &obj->scalerObj_b3, "scaler_obj_b3", AVP_BUFFER_Q_DEPTH,NUM_CH);
    }

    if(status == VX_SUCCESS)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj_b4, "scaler_obj_b4", AVP_BUFFER_Q_DEPTH,NUM_CH);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj_b5, "scaler_obj_b5", AVP_BUFFER_Q_DEPTH,NUM_CH);
        status = app_init_scaler(obj->context, &obj->scalerObj_b6, "scaler_obj_b6", AVP_BUFFER_Q_DEPTH,NUM_CH);
        status = app_init_scaler(obj->context, &obj->scalerObj_lidar_canvas, "scaler_obj_lidar_canvas", AVP_BUFFER_Q_DEPTH,NUM_CH);
    }
     
    APP_PRINTF("app_init_scaler all 6 success\n");
    
    /* Initialize TIDL first to get tensor I/O information from network */
    if(status == VX_SUCCESS)
    {
        #if defined (SOC_J784S4)
        obj->TIDLObj.core_id = 2;
        #else
        obj->TIDLObj.core_id = 0;
        #endif
        status = app_init_tidl_bev(obj->context, &obj->TIDLObj, "od_tidl_obj_bev",AVP_BUFFER_Q_DEPTH);
    }
    APP_PRINTF("app_init_tidl_bev success\n");
    if(status == VX_SUCCESS)
    {
        status = app_update_pre_proc(obj->context, &obj->preProcObj, obj->TIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_pre_proc(obj->context, &obj->preProcObj, "pre_proc_obj");
    }
    
    if(status == VX_SUCCESS)
    {
        
        status = app_update_bev_post_proc(obj->context, &obj->BEVPostProcObj, obj->TIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        
        status = app_update_bev_cam_post_proc(obj->context, &obj->BEVPostProcObj_cam, obj->TIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        
        status = app_init_bev_post_proc(obj->context , &obj->BEVPostProcObj, "bev_post_proc_obj", AVP_BUFFER_Q_DEPTH, NUM_CH);
    }
    if(status == VX_SUCCESS)
    {
        
        status = app_init_bev_cam_post_proc(obj->context , &obj->BEVPostProcObj_cam, "bev_post_proc_obj_cam", AVP_BUFFER_Q_DEPTH,(NUM_CH+5));
    }
    
    if(status == VX_SUCCESS)
    {
        status = app_init_img_mosaic(obj->context, &obj->imgMosaicObj, AVP_BUFFER_Q_DEPTH);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_display(obj->context, &obj->displayObj, "display_obj");
    }

    if(status == VX_SUCCESS)
    {
        appPerfPointSetName(&obj->total_perf , "TOTAL");
        appPerfPointSetName(&obj->fileio_perf, "FILEIO");
    }

    #ifndef x86_64
    if(obj->displayObj.display_option == 1 && obj->enable_gui)
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
    app_deinit_scaler(&obj->scalerObj, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_b2, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_b3, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_b4, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_b5, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_b6, AVP_BUFFER_Q_DEPTH);
    app_deinit_scaler(&obj->scalerObj_lidar_canvas, AVP_BUFFER_Q_DEPTH);

    app_deinit_pre_proc(&obj->preProcObj);
    app_deinit_tidl_bev(&obj->TIDLObj, AVP_BUFFER_Q_DEPTH);

    app_deinit_bev_post_proc(obj->context,&obj->BEVPostProcObj,AVP_BUFFER_Q_DEPTH);
    APP_PRINTF("Post proc deinit done!\n");
    app_deinit_bev_cam_post_proc(obj->context,&obj->BEVPostProcObj_cam,AVP_BUFFER_Q_DEPTH);
    app_deinit_img_mosaic(&obj->imgMosaicObj, AVP_BUFFER_Q_DEPTH);

    app_deinit_display(&obj->displayObj);

    #ifndef x86_64
    if(obj->displayObj.display_option == 1 && obj->enable_gui)
    {
        appGrpxDeInit();
    }
    #endif

    /* Release reference to background image */
    vxReleaseImage(&obj->background);

    tivxTIDLUnLoadKernels(obj->context);
    tivxImgProcUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    app_delete_scaler(&obj->scalerObj);
    app_delete_scaler(&obj->scalerObj_b2);
    app_delete_scaler(&obj->scalerObj_b3);   
    app_delete_scaler(&obj->scalerObj_b4);
    app_delete_scaler(&obj->scalerObj_b5);
    app_delete_scaler(&obj->scalerObj_b6);
    app_delete_scaler(&obj->scalerObj_lidar_canvas);

    app_delete_pre_proc(&obj->preProcObj);

    app_delete_tidl_bev(&obj->TIDLObj);
    app_delete_bev_post_proc(&obj->BEVPostProcObj);
    app_delete_bev_post_proc(&obj->BEVPostProcObj_cam);
    app_delete_img_mosaic(&obj->imgMosaicObj);

    app_delete_display(&obj->displayObj);

#ifdef APP_TIVX_LOG_RT_ENABLE
    tivxLogRtTraceExportToFile("app_tidl_bev.bin");
    tivxLogRtTraceDisable(obj->graph);
#endif

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
#ifdef AVP_ENABLE_PIPELINE_FLOW
    vx_int32 list_depth = ((obj->en_out_img_write == 1) || (obj->test_mode == 1)) ? 2 : 1;

        list_depth++;
        list_depth++;        
        list_depth++;            
        list_depth++;
        list_depth++;
        list_depth++;
#ifdef LIDAR_XYCORD_PER_FRAME
        list_depth++; /*for XY Cordinate Input*/
        list_depth++; /*for Lidar Input*/
        list_depth++; /*for camLidar Input*/
#endif

    APP_PRINTF("list_depth of graph_parameters_queue_params_list[ --%d ]\n",list_depth);
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[list_depth];
    vx_int32 graph_parameter_index;
#endif

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);
    vxSetReferenceName((vx_reference)obj->graph, "bev_graph");

    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, "TIVX_TARGET_VPAC_MSC1");
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_b2, "TIVX_TARGET_VPAC_MSC2");
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_b3, "TIVX_TARGET_VPAC_MSC1");       
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_b4, "TIVX_TARGET_VPAC_MSC2");           
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_b5, "TIVX_TARGET_VPAC_MSC1");
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_b6, "TIVX_TARGET_VPAC_MSC2");
    app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj_lidar_canvas, "TIVX_TARGET_VPAC_MSC1");
    
    
    APP_PRINTF("App Create Graph 6 Scaler's Done\n");
    
    
        /*Modified BEV*/
    app_create_graph_pre_proc_4D(obj->graph, &obj->preProcObj, obj->scalerObj.output.arr[0], obj->scalerObj_b2.output.arr[0], 
    obj->scalerObj_b3.output.arr[0], obj->scalerObj_b4.output.arr[0], 
    obj->scalerObj_b5.output.arr[0], obj->scalerObj_b6.output.arr[0],
    "4D_pre_proc_node");
    APP_PRINTF("App Create Graph 4D Pre-Proc Done \n");

    status = app_create_graph_tidl_4D(obj->context, obj->graph, &obj->TIDLObj, obj->preProcObj.output_tensor_arr);
    if (VX_SUCCESS == status)
    {
        APP_PRINTF("App Create Graph TIDL 4D Done \n");
    }

    status = app_create_graph_bev_post_proc(obj->graph, &obj->BEVPostProcObj, obj->TIDLObj.output_tensor_arr_batch[0], obj->scalerObj_lidar_canvas.output.arr[0] );
        
    if (VX_SUCCESS == status)
    {
        APP_PRINTF("App Create Graph BEV Post Processing Done \n");
    }
    status = app_create_graph_bev_cam_post_proc(obj->graph, &obj->BEVPostProcObj_cam, obj->TIDLObj.output_tensor_arr_batch[0], obj->scalerObj.output1.arr[0] );
        
    if (VX_SUCCESS == status)
    {
        APP_PRINTF("App Create Graph BEV Post Processing for cam Space Done \n");
    }
    
    
    vx_int32 idx = 0;

    if(VX_SUCCESS == status)
    {
        obj->imgMosaicObj.input_arr[idx++] = obj->BEVPostProcObj_cam.output_arr[0];               
        obj->imgMosaicObj.input_arr[idx++] = obj->BEVPostProcObj.output_arr[0];          
                    
    }

    obj->imgMosaicObj.num_inputs = idx;

    if(obj->enable_gui==0)
    {
        app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, obj->background);
        APP_PRINTF("Mosaic Graph Creation completed with background\n");
    }
    else
    {
        app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);
        APP_PRINTF("Mosaic Graph Creation completed without background\n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);
    }
#ifdef AVP_ENABLE_PIPELINE_FLOW
    /* Scalar Node - input is in Index 0 */
    graph_parameter_index = 0;
    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 0);
    obj->scalerObj.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj.input_images[0];
    graph_parameter_index++;

    
    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_b2.node, 0);
    obj->scalerObj_b2.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_b2.input_images[0];
    graph_parameter_index++;

    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_b3.node, 0);
    obj->scalerObj_b3.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_b3.input_images[0];
    graph_parameter_index++;

                
    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_b4.node, 0);
    obj->scalerObj_b4.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_b4.input_images[0];
    graph_parameter_index++;


    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_b5.node, 0);
    obj->scalerObj_b5.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_b5.input_images[0];
    graph_parameter_index++;

    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_b6.node, 0);
    obj->scalerObj_b6.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_b6.input_images[0];
    graph_parameter_index++;

    add_graph_parameter_by_node_index(obj->graph, obj->scalerObj_lidar_canvas.node, 0);
    obj->scalerObj_lidar_canvas.graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
    graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj_lidar_canvas.input_images[0];
    graph_parameter_index++;

#ifdef LIDAR_XYCORD_PER_FRAME
                        add_graph_parameter_by_node_index(obj->graph, obj->TIDLObj.node, 7);
                        obj->TIDLObj.graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->TIDLObj.input_xycord[0];
                        graph_parameter_index++;
                        
                        add_graph_parameter_by_node_index(obj->graph, obj->BEVPostProcObj.node, 3);
                        obj->BEVPostProcObj.graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->BEVPostProcObj.input_lidar[0];
                        graph_parameter_index++;

                        add_graph_parameter_by_node_index(obj->graph, obj->BEVPostProcObj_cam.node, 3);
                        obj->BEVPostProcObj_cam.graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
                        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->BEVPostProcObj_cam.input_lidar2cam[0];
                        graph_parameter_index++;
#endif   

    if((obj->en_out_img_write == 1) || (obj->test_mode == 1))
    {
        add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 1);
        obj->imgMosaicObj.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = AVP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.output_image[0];
        graph_parameter_index++;
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetGraphScheduleConfig(obj->graph,
                        VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                        list_depth,
                        graph_parameters_queue_params_list);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetGraphPipelineDepth(obj->graph, AVP_PIPELINE_DEPTH);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, 8);
    }
    
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b2.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b2.node, 2, 8);
    }   
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b3.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b3.node, 2, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b4.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b4.node, 2, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b5.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b5.node, 2, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b6.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_b6.node, 2, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_lidar_canvas.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj_lidar_canvas.node, 2, 8);
    }
    if(status == VX_SUCCESS)
    {
        APP_PRINTF("Set Node Parameter Scalar Done \n");
    }

    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 7, 8);
    }
    if(status == VX_SUCCESS)
    {
        APP_PRINTF("Set Node Parameter Pre Proc Done \n");
    }

    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->TIDLObj.node, 8, 8);
    }

    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->BEVPostProcObj.node, 2,8);
        status = tivxSetNodeParameterNumBufByIndex(obj->BEVPostProcObj_cam.node, 2,8);
        status = tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, 8);
    }
    if(status == VX_SUCCESS)
    {
        if(!((obj->en_out_img_write == 1) || (obj->test_mode == 1)))
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, 8);
        }
    }

#endif
    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    ScalerObj *scalerObj;
    ScalerObj *scalerObj_b2;
    ScalerObj *scalerObj_b3;
    ScalerObj *scalerObj_b4;
    ScalerObj *scalerObj_b5;
    ScalerObj *scalerObj_b6;
    ScalerObj *scalerObj_lidar_canvas;
    
    vx_reference refs[1];
    vx_reference refs_b2[1];
    vx_reference refs_b3[1];
    vx_reference refs_b4[1];
    vx_reference refs_b5[1];
    vx_reference refs_b6[1];
    vx_reference refs_b7[1];

    
    
    status = vxVerifyGraph(obj->graph);
    APP_PRINTF("App Verify Graph Done!\n");

    if(status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_bev");
    }
#ifdef APP_TIVX_LOG_RT_ENABLE
    if(VX_SUCCESS == status)
    {
        tivxLogRtTraceEnable(obj->graph);
    }
#endif

    scalerObj = &obj->scalerObj;
    scalerObj_b2 = &obj->scalerObj_b2;
    scalerObj_b3 = &obj->scalerObj_b3;
    scalerObj_b4 = &obj->scalerObj_b4;
    scalerObj_b5 = &obj->scalerObj_b5;
    scalerObj_b6 = &obj->scalerObj_b6;
    scalerObj_lidar_canvas = &obj->scalerObj_lidar_canvas;

    refs[0] = (vx_reference)scalerObj->coeff_obj;
    
    refs_b2[0] = (vx_reference)scalerObj_b2->coeff_obj;

    refs_b3[0] = (vx_reference)scalerObj_b3->coeff_obj;
       
    refs_b4[0] = (vx_reference)scalerObj_b4->coeff_obj;
                    
    refs_b5[0] = (vx_reference)scalerObj_b5->coeff_obj;
                     
    refs_b6[0] = (vx_reference)scalerObj_b6->coeff_obj;
                    
    refs_b7[0] = (vx_reference)scalerObj_lidar_canvas->coeff_obj;

    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj->node, 0u,
                                    TIVX_VPAC_MSC_CMD_SET_COEFF,
                                    refs, 1u);
        APP_PRINTF("App Send MSC Command Done for scalar 1st batch!\n");
    }
    if(status != VX_SUCCESS)
    {
        printf("MSC: Node send command failed for batch 1!\n");
    }
    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj_b2->node, 0u,
                                TIVX_VPAC_MSC_CMD_SET_COEFF,
                                refs_b2, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 2!\n");
    }

    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj_b3->node, 0u,
                            TIVX_VPAC_MSC_CMD_SET_COEFF,
                            refs_b3, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 3!\n");
    }
    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj_b4->node, 0u,
                        TIVX_VPAC_MSC_CMD_SET_COEFF,
                        refs_b4, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 4!\n");
    }       
    if(status == VX_SUCCESS)
    {       
        status = tivxNodeSendCommand(scalerObj_b5->node, 0u,
                    TIVX_VPAC_MSC_CMD_SET_COEFF,
                    refs_b5, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 5!\n");
    }
    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj_b6->node, 0u,
                    TIVX_VPAC_MSC_CMD_SET_COEFF,
                    refs_b6, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 6!\n");
    }
    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj_lidar_canvas->node, 0u,
                    TIVX_VPAC_MSC_CMD_SET_COEFF,
                    refs_b7, 1u);
        APP_PRINTF("App Send MSC Command Done for Scalar batch 7!\n");
    }
    
    if(status != VX_SUCCESS)
    {
        printf("MSC: Node send command failed for batch2!\n");
    }

    /* Fill background image */
    if((status == VX_SUCCESS) && (obj->enable_gui == 0))
    {
        fill_background_image(obj->background);
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

#ifndef AVP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id)
{
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
    ScalerObj *scalerObj_b2;
    ScalerObj *scalerObj_b3;
    ScalerObj *scalerObj_b4;
    ScalerObj *scalerObj_b5;
    ScalerObj *scalerObj_b6;
    ScalerObj *scalerObj_lidar_canvas;

    scalerObj_b2 = &obj->scalerObj_b2;
    scalerObj_b3 = &obj->scalerObj_b3;
    scalerObj_b4 = &obj->scalerObj_b4;
    scalerObj_b5 = &obj->scalerObj_b5;
    scalerObj_b6 = &obj->scalerObj_b6;
    scalerObj_lidar_canvas = &obj->scalerObj_lidar_canvas;

#ifdef LIDAR_XYCORD_PER_FRAME
    TIDLObj *TIDLObj = &obj->TIDLObj;
    BEVPostProcObj *BEVPostProcObj_cam = &obj->BEVPostProcObj_cam;
    BEVPostProcObj *BEVPostProcObj = &obj->BEVPostProcObj;
#endif

    //snprintf(input_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);
    snprintf(input_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_bc.nv12", obj->input_file_path, frame_id);
    
        snprintf(input_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_lc.nv12", obj->input_file_path, frame_id);
        snprintf(input_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_frc.nv12", obj->input_file_path, frame_id);
        snprintf(input_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_flc.nv12", obj->input_file_path, frame_id);
        snprintf(input_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_rc.nv12", obj->input_file_path, frame_id);
        snprintf(input_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_fc.nv12", obj->input_file_path, frame_id);
        snprintf(input_file_name7, APP_MAX_FILE_PATH, "%s/black_image.yuv", obj->input_file_path);
        //snprintf(input_file_name7, APP_MAX_FILE_PATH, "%s/%010d_6.yuv", obj->input_file_path, frame_id);
#ifdef LIDAR_XYCORD_PER_FRAME
        snprintf(input_file_namexycord, APP_MAX_FILE_PATH, "%s/%010d_xy.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr, APP_MAX_FILE_PATH, "%s/%010d_ldr.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2lc, APP_MAX_FILE_PATH, "%s/%010d_ldr2lc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2frc, APP_MAX_FILE_PATH, "%s/%010d_ldr2frc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2flc, APP_MAX_FILE_PATH, "%s/%010d_ldr2flc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2bc, APP_MAX_FILE_PATH, "%s/%010d_ldr2bc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2rc, APP_MAX_FILE_PATH, "%s/%010d_ldr2rc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2fc, APP_MAX_FILE_PATH, "%s/%010d_ldr2fc.bin", obj->input_file_path, frame_id);
#endif
    appPerfPointBegin(&obj->fileio_perf);

    readScalerInput(input_file_name_bc, scalerObj->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name_lc, scalerObj->input.arr[0], 1);
    readScalerInput(input_file_name_frc, scalerObj->input.arr[0], 2);
    readScalerInput(input_file_name_flc, scalerObj->input.arr[0], 3);
    readScalerInput(input_file_name_rc, scalerObj->input.arr[0], 4);
    readScalerInput(input_file_name_fc, scalerObj->input.arr[0], 5);
    readScalerInput(input_file_name_lc, scalerObj_b2->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name_frc, scalerObj_b3->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name_flc, scalerObj_b4->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name_rc, scalerObj_b5->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name_fc, scalerObj_b6->input.arr[0], (NUM_CH-1));
    readScalerInput(input_file_name7, scalerObj_lidar_canvas->input.arr[0], (NUM_CH-1));
#ifdef LIDAR_XYCORD_PER_FRAME
    readTensorInput(input_file_namexycord,TIDLObj->input_xycord_arr[0],(NUM_CH-1));
    readTensorInput(input_file_nameldr,BEVPostProcObj->input_lidar_arr[0],(NUM_CH-1));
    readTensorInput(input_file_nameldr2bc,BEVPostProcObj_cam->input_lidar2cam_arr[0],(NUM_CH-1));
    readTensorInput(input_file_nameldr2lc,BEVPostProcObj_cam->input_lidar2cam_arr[0],1);
    readTensorInput(input_file_nameldr2frc,BEVPostProcObj_cam->input_lidar2cam_arr[0],2);
    readTensorInput(input_file_nameldr2flc,BEVPostProcObj_cam->input_lidar2cam_arr[0],3);
    readTensorInput(input_file_nameldr2rc,BEVPostProcObj_cam->input_lidar2cam_arr[0],4);
    readTensorInput(input_file_nameldr2fc,BEVPostProcObj_cam->input_lidar2cam_arr[0],5);
#endif
    APP_PRINTF("App Scalar Reading done!\n");

    appPerfPointEnd(&obj->fileio_perf);

    APP_PRINTF("App Reading Input Done!\n");

#ifdef x86_64
    printf("Processing files ...... \n %s\n %s\n %s\n %s\n %s\n %s\n",
            input_file_name_bc,input_file_name_fc,
            input_file_name_lc,input_file_name_rc,
            input_file_name_frc,input_file_name_flc);
#endif

    status = vxProcessGraph(obj->graph);

#ifdef x86_64
    printf("Done!\n");
#endif

    APP_PRINTF("App Process Graph Done!\n");

    if((VX_SUCCESS == status) && (obj->en_out_img_write == 1))
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

         snprintf(output_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_704x256_bc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_bc, scalerObj->output.arr[0]);

         snprintf(output_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_704x256_lc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_lc, scalerObj_b2->output.arr[0]);
         snprintf(output_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_704x256_flc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_flc, scalerObj_b3->output.arr[0]);
         snprintf(output_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_704x256_frc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_frc, scalerObj_b4->output.arr[0]);
         snprintf(output_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_704x256_rc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_rc, scalerObj_b5->output.arr[0]);
         snprintf(output_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_704x256_fc.yuv", obj->output_file_path, frame_id);
         writeScalerOutput(output_file_name_fc, scalerObj_b6->output.arr[0]);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/pre_proc_output_%010d", obj->output_file_path, frame_id);
        writePreProcOutput(output_file_name, obj->preProcObj.output_tensor_arr);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/tidl_output_%010d", obj->output_file_path, frame_id);
        writeTIDLOutput(output_file_name, &obj->TIDLObj);



#endif

         snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, frame_id);
         writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);

        APP_PRINTF("App Writing Outputs Done!\n");
    }
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
    /* actual_checksum is the checksum determined by the realtime test */
    uint32_t actual_checksum = 0;

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
    TIDLObj *TIDLObj = &obj->TIDLObj;
    BEVPostProcObj *BEVPostProcObj_cam = &obj->BEVPostProcObj_cam;
    BEVPostProcObj *BEVPostProcObj = &obj->BEVPostProcObj; 
#endif

    ImgMosaicObj *imgMosaicObj = &obj->imgMosaicObj;

    snprintf(input_file_name_bc, APP_MAX_FILE_PATH, "%s/%010d_bc.nv12", obj->input_file_path, frame_id);
    
    snprintf(input_file_name_lc, APP_MAX_FILE_PATH, "%s/%010d_lc.nv12", obj->input_file_path, frame_id);
    snprintf(input_file_name_frc, APP_MAX_FILE_PATH, "%s/%010d_frc.nv12", obj->input_file_path, frame_id);
    snprintf(input_file_name_flc, APP_MAX_FILE_PATH, "%s/%010d_flc.nv12", obj->input_file_path, frame_id);
    snprintf(input_file_name_rc, APP_MAX_FILE_PATH, "%s/%010d_rc.nv12", obj->input_file_path, frame_id);
    snprintf(input_file_name_fc, APP_MAX_FILE_PATH, "%s/%010d_fc.nv12", obj->input_file_path, frame_id);
    snprintf(input_file_name7, APP_MAX_FILE_PATH, "%s/black_image.yuv", obj->input_file_path);
#ifdef LIDAR_XYCORD_PER_FRAME
        snprintf(input_file_namexycord, APP_MAX_FILE_PATH, "%s/%010d_xy.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2lc, APP_MAX_FILE_PATH, "%s/%010d_ldr2lc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2frc, APP_MAX_FILE_PATH, "%s/%010d_ldr2frc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2flc, APP_MAX_FILE_PATH, "%s/%010d_ldr2flc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2bc, APP_MAX_FILE_PATH, "%s/%010d_ldr2bc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2rc, APP_MAX_FILE_PATH, "%s/%010d_ldr2rc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr2fc, APP_MAX_FILE_PATH, "%s/%010d_ldr2fc.bin", obj->input_file_path, frame_id);
        snprintf(input_file_nameldr, APP_MAX_FILE_PATH, "%s/%010d_ldr.bin", obj->input_file_path, frame_id);
#endif
    
    if(obj->pipeline < 0)
    {
        /* Enqueue outpus */
        if(((obj->en_out_img_write == 1) || (obj->test_mode == 1))
            && (status == VX_SUCCESS))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->enqueueCnt], 1);
        }

        appPerfPointBegin(&obj->fileio_perf);
        /* Read input */
        readScalerInput(input_file_name_bc, scalerObj->input.arr[obj->enqueueCnt], (NUM_CH-1));
        readScalerInput(input_file_name_lc, scalerObj->input.arr[obj->enqueueCnt], 1);
        readScalerInput(input_file_name_frc, scalerObj->input.arr[obj->enqueueCnt], 2);
        readScalerInput(input_file_name_flc, scalerObj->input.arr[obj->enqueueCnt], 3);
        readScalerInput(input_file_name_rc, scalerObj->input.arr[obj->enqueueCnt], 4);
        readScalerInput(input_file_name_fc, scalerObj->input.arr[obj->enqueueCnt], 5);
        readScalerInput(input_file_name_lc, scalerObj_b2->input.arr[obj->enqueueCnt], (NUM_CH-1));
        readScalerInput(input_file_name_frc, scalerObj_b3->input.arr[obj->enqueueCnt], (NUM_CH-1));    
        readScalerInput(input_file_name_flc, scalerObj_b4->input.arr[obj->enqueueCnt], (NUM_CH-1));        
        readScalerInput(input_file_name_rc, scalerObj_b5->input.arr[obj->enqueueCnt], (NUM_CH-1));
        readScalerInput(input_file_name_fc, scalerObj_b6->input.arr[obj->enqueueCnt], (NUM_CH-1));
        readScalerInput(input_file_name7, scalerObj_lidar_canvas->input.arr[obj->enqueueCnt], (NUM_CH-1));
#ifdef LIDAR_XYCORD_PER_FRAME
            readTensorInput(input_file_namexycord,TIDLObj->input_xycord_arr[obj->enqueueCnt],(NUM_CH-1));
            readTensorInput(input_file_nameldr,BEVPostProcObj->input_lidar_arr[obj->enqueueCnt],(NUM_CH-1));
            readTensorInput(input_file_nameldr2bc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],(NUM_CH-1));
            readTensorInput(input_file_nameldr2lc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],1);
            readTensorInput(input_file_nameldr2frc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],2);
            readTensorInput(input_file_nameldr2flc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],3);
            readTensorInput(input_file_nameldr2rc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],4);
            readTensorInput(input_file_nameldr2fc,BEVPostProcObj_cam->input_lidar2cam_arr[obj->enqueueCnt],5);
#endif                  
        appPerfPointEnd(&obj->fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

        /* Enqueue input - start execution */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&obj->scalerObj.input_images[obj->enqueueCnt], 1);
        }
            APP_PRINTF("Enque -%d - Success Scaler Image 1 !\n",obj->enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&obj->scalerObj_b2.input_images[obj->enqueueCnt], 1);
            
            APP_PRINTF("Enque -%d - Success Scaler Image 2 !\n",obj->enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&obj->scalerObj_b3.input_images[obj->enqueueCnt], 1);
            APP_PRINTF("Enque -%d - Success Scaler Image 3 !\n",obj->enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&obj->scalerObj_b4.input_images[obj->enqueueCnt], 1);
            APP_PRINTF("Enque -%d - Success Scaler Image 4 !\n",obj->enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&obj->scalerObj_b5.input_images[obj->enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&obj->scalerObj_b6.input_images[obj->enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&obj->scalerObj_lidar_canvas.input_images[obj->enqueueCnt], 1);
#ifdef LIDAR_XYCORD_PER_FRAME
            status = vxGraphParameterEnqueueReadyRef(obj->graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&obj->BEVPostProcObj.input_lidar[obj->enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&obj->BEVPostProcObj_cam.input_lidar2cam[obj->enqueueCnt], 1);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, TIDLObj->graph_parameter_index, (vx_reference*)&obj->TIDLObj.input_xycord[obj->enqueueCnt], 1);
#endif         
            if(status == VX_SUCCESS)
            {
                APP_PRINTF("Enque -%d - Success Scaler Image 5, 6 & 7 !\n",obj->enqueueCnt);
                APP_PRINTF("Enque -%d - Success lidar Tensor !\n",obj->enqueueCnt);
            } 
        obj->enqueueCnt++;
        obj->enqueueCnt   = (obj->enqueueCnt  >= AVP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->pipeline++;
    }

    if(obj->pipeline >= 0)
    {
        //tivx_set_debug_zone(VX_ZONE_INFO);
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
            status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1, &num_refs);
        }
        
        APP_PRINTF("deque -%d - Success Scaler Image 1 !\n",obj->enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&scaler_input_image2, 1, &num_refs);
        APP_PRINTF("deque -%d - Success Scaler Image 2 !\n",obj->enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&scaler_input_image3, 1, &num_refs); 
        APP_PRINTF("deque -%d - Success Scaler Image 3 !\n",obj->enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&scaler_input_image4, 1, &num_refs);
        APP_PRINTF("deque -%d - Success Scaler Image 4 !\n",obj->enqueueCnt);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&scaler_input_image5, 1, &num_refs);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&scaler_input_image6, 1, &num_refs);
        status = vxGraphParameterDequeueDoneRef(obj->graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&scaler_input_image7, 1, &num_refs);
#ifdef LIDAR_XYCORD_PER_FRAME
            status = vxGraphParameterDequeueDoneRef(obj->graph, TIDLObj->graph_parameter_index, (vx_reference*)&bev_tidl_input_xycord, 1, &num_refs);
            status = vxGraphParameterDequeueDoneRef(obj->graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&bev_post_proc_input_cam_lidar, 1, &num_refs);
            status = vxGraphParameterDequeueDoneRef(obj->graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&bev_post_proc_input_lidar, 1, &num_refs);
#endif 
        APP_PRINTF("deque -%d - Success Scaler Image 5,6,7 !\n",obj->enqueueCnt);
        APP_PRINTF("deque -%d - Sucess Lidar Tensor !\n",obj->enqueueCnt);
            
        if(((obj->en_out_img_write == 1) || (obj->test_mode == 1)) && (status == VX_SUCCESS))
        {
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output */
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);
            }
            /* Check that you are within the first n frames, where n is the number
                of samples in the checksums_expected */
            if ((obj->test_mode == 1) &&
                ((frame_id - obj->start_frame - 2) < (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0]))))
            {
                /* -2 is there because it takes 2 pipeline executions to get to normal execution */
                vx_uint32 expected_idx = frame_id - obj->start_frame - 2;
                /* Check the mosaic output image which contains everything */
                if (vx_false_e == app_test_check_image(mosaic_output_image, checksums_expected[0][expected_idx],
                                                        &actual_checksum))
                {
                    test_result = vx_false_e;
                }
                /* in case test fails and needs to change */
                populate_gatherer(0, expected_idx, actual_checksum);
            }
            if (obj->en_out_img_write == 1 && (status == VX_SUCCESS))
            {
                APP_PRINTF("App Writing Outputs Start...\n");
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, (frame_id - AVP_BUFFER_Q_DEPTH));
                status = writeMosaicOutput(output_file_name, mosaic_output_image);
                APP_PRINTF("App Writing Outputs Done!\n");
            }

            /* Enqueue output */
            if(status ==  VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&mosaic_output_image, 1);
            }
        }

        appPerfPointBegin(&obj->fileio_perf);

        if (status == VX_SUCCESS)
        {
            app_find_object_array_index(scalerObj->input.arr, (vx_reference)scaler_input_image, AVP_BUFFER_Q_DEPTH, &obj_array_idx);
        }
        
        if((obj_array_idx != -1) && (status == VX_SUCCESS))
        {
            status = readScalerInput(input_file_name_bc, scalerObj->input.arr[obj_array_idx], (NUM_CH-1));
            status = readScalerInput(input_file_name_lc, scalerObj->input.arr[obj_array_idx], 1);
            status = readScalerInput(input_file_name_frc, scalerObj->input.arr[obj_array_idx], 2);
            status = readScalerInput(input_file_name_flc, scalerObj->input.arr[obj_array_idx], 3);
            status = readScalerInput(input_file_name_rc, scalerObj->input.arr[obj_array_idx], 4);
            status = readScalerInput(input_file_name_fc, scalerObj->input.arr[obj_array_idx], 5);
        }
        if((obj->BATCH_ENABLED >=1) && (status == VX_SUCCESS))
        {
            app_find_object_array_index(scalerObj_b2->input.arr, (vx_reference)scaler_input_image2, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b2);

            if((obj_array_idx_b2 != -1) && (status == VX_SUCCESS))
            {
                status = readScalerInput(input_file_name_lc, scalerObj_b2->input.arr[obj_array_idx_b2], (NUM_CH-1));
            }

            if((obj->BATCH_ENABLED >=2) && (status == VX_SUCCESS))
            {
                app_find_object_array_index(scalerObj_b3->input.arr, (vx_reference)scaler_input_image3, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b3);

                if((obj_array_idx_b3 != -1) && (status == VX_SUCCESS))
                {
                    status = readScalerInput(input_file_name_frc, scalerObj_b3->input.arr[obj_array_idx_b3], (NUM_CH-1));
                }

                if((obj->BATCH_ENABLED >=3) && (status == VX_SUCCESS))
                {
                    app_find_object_array_index(scalerObj_b4->input.arr, (vx_reference)scaler_input_image4, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b4);

                    if((obj_array_idx_b4 != -1) && (status == VX_SUCCESS))
                    {
                        status = readScalerInput(input_file_name_flc, scalerObj_b4->input.arr[obj_array_idx_b4], (NUM_CH-1));
                    }

                    if((obj->BATCH_ENABLED >=5) && (status == VX_SUCCESS))
                    {
                        app_find_object_array_index(scalerObj_b5->input.arr, (vx_reference)scaler_input_image5, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b5);

                        if((obj_array_idx_b5 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name_rc, scalerObj_b5->input.arr[obj_array_idx_b5], (NUM_CH-1));
                        }
                            
                        app_find_object_array_index(scalerObj_b6->input.arr, (vx_reference)scaler_input_image6, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b6);
                        if((obj_array_idx_b6 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name_fc, scalerObj_b6->input.arr[obj_array_idx_b6], (NUM_CH-1));
                        }
                        app_find_object_array_index(scalerObj_lidar_canvas->input.arr, (vx_reference)scaler_input_image7, AVP_BUFFER_Q_DEPTH, &obj_array_idx_b7);
                        if((obj_array_idx_b7 != -1) && (status == VX_SUCCESS))
                        {
                            status = readScalerInput(input_file_name7, scalerObj_lidar_canvas->input.arr[obj_array_idx_b7], (NUM_CH-1));
                        }
#ifdef LIDAR_XYCORD_PER_FRAME
                        app_find_lidar_object_array_index(TIDLObj->input_xycord_arr, (vx_reference)bev_tidl_input_xycord, AVP_BUFFER_Q_DEPTH, &obj_array_idx_xycord);
                        if((obj_array_idx_xycord != -1) && (status == VX_SUCCESS))
                        {
                            status = readTensorInput(input_file_namexycord, TIDLObj->input_xycord_arr[obj_array_idx_xycord], (NUM_CH-1));
                        }
                        app_find_lidar_object_array_index(BEVPostProcObj->input_lidar_arr, (vx_reference)bev_post_proc_input_lidar, AVP_BUFFER_Q_DEPTH, &obj_array_idx_ldr);
                        if((obj_array_idx_ldr != -1) && (status == VX_SUCCESS))
                        {
                            status = readTensorInput(input_file_nameldr, BEVPostProcObj->input_lidar_arr[obj_array_idx_ldr], (NUM_CH-1));
                        }
                        app_find_lidar_object_array_index(BEVPostProcObj_cam->input_lidar2cam_arr, (vx_reference)bev_post_proc_input_cam_lidar, AVP_BUFFER_Q_DEPTH, &obj_array_idx_camldr);
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
        appPerfPointEnd(&obj->fileio_perf);

        APP_PRINTF("App Reading Input Done!\n");

    /* Enqueue input - start execution */
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&scaler_input_image, 1);
        }
        if((obj->BATCH_ENABLED >=1) && (status == VX_SUCCESS))
        {
            APP_PRINTF("enque -%d - Success Scaler Image 1!\n",obj->enqueueCnt);
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b2->graph_parameter_index, (vx_reference*)&scaler_input_image2, 1);
            
            if((obj->BATCH_ENABLED >=2) && (status == VX_SUCCESS))
            {
                APP_PRINTF("enque -%d - Success Scaler Image 2!\n",obj->enqueueCnt);
                status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b3->graph_parameter_index, (vx_reference*)&scaler_input_image3, 1);

                if((obj->BATCH_ENABLED >=3) && (status == VX_SUCCESS))
                {
                    APP_PRINTF("enque -%d - Success Scaler Image 3 !\n",obj->enqueueCnt);
                    status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b4->graph_parameter_index, (vx_reference*)&scaler_input_image4, 1);
                    if((obj->BATCH_ENABLED >=5) && (status == VX_SUCCESS))
                    {
                        APP_PRINTF("enque -%d - Success Scaler Image 4 !\n",obj->enqueueCnt);
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b5->graph_parameter_index, (vx_reference*)&scaler_input_image5, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_b6->graph_parameter_index, (vx_reference*)&scaler_input_image6, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj_lidar_canvas->graph_parameter_index, (vx_reference*)&scaler_input_image7, 1);
#ifdef LIDAR_XYCORD_PER_FRAME      
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, TIDLObj->graph_parameter_index, (vx_reference*)&bev_tidl_input_xycord, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, BEVPostProcObj_cam->graph_parameter_index, (vx_reference*)&bev_post_proc_input_cam_lidar, 1);
                        status = vxGraphParameterEnqueueReadyRef(obj->graph, BEVPostProcObj->graph_parameter_index, (vx_reference*)&bev_post_proc_input_lidar, 1);
#endif                    
                    }
                    APP_PRINTF("enque -%d - Success Scaler Image 5,6,7 !\n",obj->enqueueCnt);
                    APP_PRINTF("enque -%d - Success Lidar Tensor !\n",obj->enqueueCnt);
                }

            }
        
        }
        obj->enqueueCnt++;
        obj->dequeueCnt++;

        obj->enqueueCnt = (obj->enqueueCnt >= AVP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->dequeueCnt = (obj->dequeueCnt >= AVP_BUFFER_Q_DEPTH)? 0 : obj->dequeueCnt;

    }

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

#ifdef AVP_ENABLE_PIPELINE_FLOW
    app_pipeline_params_defaults(obj);
#endif
    for(x = 0; x < obj->num_iterations; x++)
    {
        for(frame_id = obj->start_frame; frame_id < (obj->start_frame + obj->num_frames); frame_id++)
        {
            APP_PRINTF("Running frame %d\n", frame_id);

            appPerfPointBegin(&obj->total_perf);

            cur_time = tivxPlatformGetTimeInUsecs();

            #ifdef AVP_ENABLE_PIPELINE_FLOW
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

            appPerfPointEnd(&obj->total_perf);

            APP_PRINTF("app_bev: Frame ID %d of %d ... Done.\n", frame_id, obj->start_frame + obj->num_frames);

            /* user asked to stop processing */
            if((obj->stop_task) || (status == VX_FAILURE))
            {
              break;
            }
        }

        printf("app_bev: Iteration %d of %d ... Done.\n", x, obj->num_iterations);
        appPerfPointPrintFPS(&obj->total_perf);
        appPerfPointReset(&obj->total_perf);
        if(x==0)
        {
            /* after first iteration reset performance stats */
            appPerfStatsResetAll();
        }

        /* user asked to stop processing */
        if((obj->stop_task) || (status == VX_FAILURE))
        {
          break;
        }
    }

#ifdef AVP_ENABLE_PIPELINE_FLOW
    vxWaitGraph(obj->graph);
#endif

    obj->stop_task = 1;

    return status;
}

static void set_pre_proc_defaults(PreProcObj *preProcObj)
{
    uint32_t i =0;

    preProcObj->params.skip_flag = 0;
    for(i = 0 ; i < 3; i++)
    {
        preProcObj->params.scale[i] = 1;
        preProcObj->params.mean[i] = 0;
    }
    preProcObj->params.channel_order = 0; /* 0-NCHW */

    for(i = 0 ; i < 4; i++)
    {
        preProcObj->params.crop[i] = 0;
    }
}

static void set_post_proc_defaults_BEV(BEVPostProcObj *postProcObj){

    postProcObj->output_img_width = 704;
    postProcObj->output_img_height = 256;

    postProcObj->params.od_prms.scaleX = 1;
    postProcObj->params.od_prms.scaleY = 1;

    postProcObj->params.od_prms.formatter[0] = 0;
    postProcObj->params.od_prms.formatter[1] = 1;
    postProcObj->params.od_prms.formatter[2] = 2;
    postProcObj->params.od_prms.formatter[3] = 3;
    postProcObj->params.od_prms.formatter[4] = 5;
    postProcObj->params.od_prms.formatter[5] = 4;

    postProcObj->params.task_type = TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE;

    postProcObj->params.od_prms.labelIndexOffset = 0;

    postProcObj->params.od_prms.labelOffset = label_offset3;

    postProcObj->params.od_prms.classnames = img_labels;

    postProcObj->params.od_prms.viz_th = 0.6;
}

static void update_post_proc_params_BEV(AppObj *obj, BEVPostProcObj *postProcObj)
{  
    postProcObj->output_img_width = obj->scalerObj_lidar_canvas.output.width;
    postProcObj->output_img_height = obj->scalerObj_lidar_canvas.output.height;
    postProcObj->params.od_prms.scaleX = postProcObj->output_img_width/110.0;
    postProcObj->params.od_prms.scaleY = postProcObj->output_img_height/110.0;
    postProcObj->params.od_prms.viz_th = obj->viz_th_BEV;
} 

static void update_post_proc_params_BEV_camera_sp(AppObj *obj, BEVPostProcObj *postProcObj)
{  
    postProcObj->output_img_width = obj->scalerObj.output.width;
    postProcObj->output_img_height = obj->scalerObj.output.height;
    postProcObj->params.od_prms.scaleX = postProcObj->output_img_width/108.0;
    postProcObj->params.od_prms.scaleY = postProcObj->output_img_height/55.0;
    postProcObj->params.od_prms.viz_th = obj->viz_th_BEV;
}

static void set_img_mosaic_defaults(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx = 0;
    vx_int32 in = 0;
    imgMosaicObj->out_width    = 1920;
    imgMosaicObj->out_height   = 1080;
    imgMosaicObj->num_inputs = 2;
            
        
        
    

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

    if(obj->enable_gui==0)
    {
        /* if GUI is disabled on HOST side then enable static overlay in SW mosaic node */

    }

    in = 0;
       
    if(NUM_CH == 1)
    {
    imgMosaicObj->params.windows[idx].startX  = 641;
    imgMosaicObj->params.windows[idx].startY  = 00;
    imgMosaicObj->params.windows[idx].width   = 639;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 5;
    idx++;

    imgMosaicObj->params.windows[idx].startX  = 1280;
    imgMosaicObj->params.windows[idx].startY  = 00;
    imgMosaicObj->params.windows[idx].width   = 640;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 2;
    idx++;

    imgMosaicObj->params.windows[idx].startX  = 00;
    imgMosaicObj->params.windows[idx].startY  = 00;
    imgMosaicObj->params.windows[idx].width   = 640;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 3;
    idx++;

    imgMosaicObj->params.windows[idx].startX  = 641;
    imgMosaicObj->params.windows[idx].startY  = 239;
    imgMosaicObj->params.windows[idx].width   = 639;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    imgMosaicObj->params.windows[idx].startX  = 00;
    imgMosaicObj->params.windows[idx].startY  = 239;
    imgMosaicObj->params.windows[idx].width   = 640;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 1;
    idx++;

    imgMosaicObj->params.windows[idx].startX  = 1280;
    imgMosaicObj->params.windows[idx].startY  = 239;
    imgMosaicObj->params.windows[idx].width   = 640;
    imgMosaicObj->params.windows[idx].height  = 238;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 4;
    idx++;

    in++;
                
    imgMosaicObj->params.windows[idx].startX  = 300;
    imgMosaicObj->params.windows[idx].startY  = 480;
    imgMosaicObj->params.windows[idx].width   = 1500;
    imgMosaicObj->params.windows[idx].height  = 600;
    imgMosaicObj->params.windows[idx].input_select   = in;
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    in++;
                
    }
                   
    
    APP_PRINTF("NUM_CH --%d \n", NUM_CH);
    APP_PRINTF("BATCH -IN --%d \n", in);
    APP_PRINTF("imgMosaicObj->params.num_windows -%d\n",idx);
    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = 8;
    imgMosaicObj->params.num_msc_instances = 1;
    imgMosaicObj->params.msc_instance = 1; /* Use MSC1 instance as MSC0 is used for scaler */

    APP_PRINTF("imgMosaicObj->params.clear_count -%d\n",imgMosaicObj->params.clear_count);
}

static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 0;
    #ifdef SOC_TDA54
    displayObj->display_pipe_id = 0;
    #else
    displayObj->display_pipe_id = 2;
    #endif
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->pipeline       = -AVP_BUFFER_Q_DEPTH;
    obj->enqueueCnt     = 0;
    obj->dequeueCnt     = 0;
}

static void app_default_param_set(AppObj *obj)
{
    set_pre_proc_defaults(&obj->preProcObj);

    set_post_proc_defaults_BEV(&obj->BEVPostProcObj);
    set_post_proc_defaults_BEV(&obj->BEVPostProcObj_cam);

    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->delay_in_msecs = 0;
    obj->num_iterations = 1;
    obj->is_interactive = 0;
    obj->test_mode      = 0;
    obj->enable_gui     = 1;

    vx_int32 i;

    for(i=0; i < TIVX_PIXEL_VIZ_MAX_CLASS; i++)
    {
        obj->valid_region[i][0] = 0;
        obj->valid_region[i][1] = 0;
        obj->valid_region[i][2] = 768 - 1;
        obj->valid_region[i][3] = 384 - 1;
    }
}

static void app_update_param_set(AppObj *obj)
{

    update_post_proc_params_BEV(obj, &obj->BEVPostProcObj);
    update_post_proc_params_BEV_camera_sp(obj, &obj->BEVPostProcObj_cam);

    set_img_mosaic_defaults(obj, &obj->imgMosaicObj);

    if(obj->is_interactive)
    {
        obj->num_iterations = 1000000000;
    }
}

#ifdef AVP_ENABLE_PIPELINE_FLOW
static void app_find_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx)
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

#ifdef LIDAR_XYCORD_PER_FRAME
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
#endif

#endif

#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        Draw2D_FontPrm sLabel;

        sHeading.fontIdx = 0;
        Draw2D_drawString(handle, 560, 5, "Analytics for BIRD EYE VIEW", &sHeading);

        sLabel.fontIdx = 2;
        Draw2D_drawString(handle, 340, 90, "Front left camera", &sLabel);
        Draw2D_drawString(handle, 920, 90, "Front camera", &sLabel);
        Draw2D_drawString(handle, 1520,90, "Front right camera", &sLabel);
        Draw2D_drawString(handle, 340, 300, "Left  camera", &sLabel);
        Draw2D_drawString(handle, 920, 300, "Back camera", &sLabel);
        Draw2D_drawString(handle, 1520,300, "Right camera", &sLabel);

        sLabel.fontIdx = 3;
        /*Draw2D_drawString(handle, 20, 244, "Semantic", &sLabel);
        Draw2D_drawString(handle, 0 , 264, "Segmentation", &sLabel);
        Draw2D_drawString(handle, 0 , 284, "  (768x384) ", &sLabel);

        Draw2D_drawString(handle, 20, 526, "Parking", &sLabel);
        Draw2D_drawString(handle, 35, 546, "Spot", &sLabel);
        Draw2D_drawString(handle, 10, 566, "Detection", &sLabel);
        Draw2D_drawString(handle, 10, 586, "(768x384)", &sLabel);*/

        Draw2D_drawString(handle, 25, 828, "Object", &sLabel);
        Draw2D_drawString(handle, 15, 848, "Detection", &sLabel);
        Draw2D_drawString(handle, 15, 868, "(704x256)", &sLabel);
    }
    return;
}
#endif

vx_status readTensorInput(char* file_name, vx_object_array tensor_arr, vx_int32 num_ch)
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
            printf("Unable to open file %s \n", file_name);
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
            vxReleaseTensor(&in_tensor);
        }
        fclose(fp);
        free(lidar_tensor_buffer);

    }

    return(status);
}
