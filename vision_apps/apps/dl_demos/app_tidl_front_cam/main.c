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

#include <utils/draw2d/include/draw2d.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_get.h>
#include <utils/grpx/include/app_grpx.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/hwa_kernels.h>
#include <TI/video_io_kernels.h>
#include <TI/dl_kernels.h>

#include "app_common.h"
#include "app_sensor_module.h"
#include "app_capture_module.h"
#include "app_viss_module.h"
#include "app_aewb_module.h"
#include "app_ldc_module.h"
#include "app_scaler_module.h"
#include "app_pre_proc_module.h"
#include "app_tidl_module.h"
#include "app_post_proc_module.h"
#include "app_img_mosaic_module.h"
#include "app_display_module.h"

#if defined(APP_CTOOLS_ENABLED) && !defined(x86_64)
#include <ctools_wrapper.h>
#endif

#ifdef QNX
#include "omx_encode_module.h"
#endif

#if SOC_J722S 
#define APP_BUFFER_Q_DEPTH      (5u)
#else
#define APP_BUFFER_Q_DEPTH      (6u)
#endif
#define APP_PIPELINE_DEPTH      (10u)
#define APP_ENC_BUFFER_Q_DEPTH  (6u)

/*
 * Macros for RGB to YUV color space conversion and value clipping.
 *
 * CLIP(X):
 *   Ensures the input value X is within the 0-255 range.
 *   If X > 255, returns 255.
 *   If X < 0, returns 0.
 *   Otherwise, returns X.
 *
 * RGB2Y(R, G, B):
 *   Converts RGB values to the Y (luminance) component of YUV.
 *   Uses the formula: Y = CLIP(((66*R + 129*G + 25*B + 128) >> 8) + 16)
 *
 * RGB2U(R, G, B):
 *   Converts RGB values to the U (chrominance) component of YUV.
 *   Uses the formula: U = CLIP(((-38*R - 74*G + 112*B + 128) >> 8) + 128)
 *
 * RGB2V(R, G, B):
 *   Converts RGB values to the V (chrominance) component of YUV.
 *   Uses the formula: V = CLIP(((112*R - 94*G - 18*B + 128) >> 8) + 128)
 */
#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define RGB2Y(R, G, B) CLIP(( ( 66*(R) + 129*(G) + 25*(B) + 128) >>8 ) + 16)
#define RGB2U(R, G, B) CLIP(( ( -38*(R) - 74*(G) + 112*(B) + 128) >>8) + 128)
#define RGB2V(R, G, B) CLIP((( 112*(R) - 94*(G) - 18*(B) + 128) >> 8) + 128)

typedef struct {
    SensorObj           sensorObj;
    CaptureObj          captureObj;
    VISSObj             vissObj;
    AEWBObj             aewbObj;
    LDCObj              ldcObj;
    ScalerObj           scalerObj;
    PreProcObj          odPreProcObj;
    PreProcObj          sgPreProcObj;    
    TIDLObj             odTIDLObj;
    TIDLObj             sgTIDLObj;
    PostProcObj         odPostProcObj;
    PostProcObj         sgPostProcObj;
    ImgMosaicObj        imgMosaicObj;
    DisplayObj          displayObj;

    /* Encode references */
    vx_char             encode_output_file_path[APP_MAX_FILE_PATH];
#ifdef QNX
    omxEncodeCfg        omx_encode_cfg;
    omxEncodeHandle     *omx_encode_handle;
    Buf                 encode_buf[APP_ENC_BUFFER_Q_DEPTH];
    vx_int32            buf_index;
#endif

    /* OpenVX references */
    vx_context          context;
    vx_graph            graph;

    vx_int32            en_out_img_write;

    vx_int32            ip_rgb_or_yuv;
    vx_int32            op_rgb_or_yuv;
    
    vx_uint32           num_frames_to_write;
    vx_uint32           num_frames_to_skip;

    vx_uint32           delay_in_msecs;
    vx_uint32           num_iterations;
    vx_uint32           is_interactive;
    vx_uint32           test_mode;
    vx_uint32           enable_gui;

    tivx_task           app_task;
    tivx_task           load_cpu_task[NUM_CPU_CORES];
    volatile vx_uint32  stop_app_task;
    volatile vx_uint32  stop_app_task_done;
    volatile vx_uint32  stop_load_cpu_task;
    volatile vx_uint32  stop_load_cpu_task_done;

    tivx_mutex          load_cpu_task_mutex;

    vx_uint32           mosaic_window_height;
    vx_uint32           mosaic_window_width;

    app_perf_point_t    total_perf;
    app_perf_point_t    fileio_perf;
    app_perf_point_t    draw_perf;

    int32_t             enable_od;
    int32_t             enable_sem_seg;

    int32_t             pipeline_exec;

    int32_t             pipeline;
    int32_t             enqueueCnt;
    int32_t             dequeueCnt;
    
    int32_t             write_file;

    int32_t             enable_capture;
    int32_t             enable_viss;
    int32_t             enable_aewb;
    int32_t             enable_ldc;
    int32_t             enable_scaler;
    int32_t             enable_mosaic;
    int32_t             enable_encode;
    int32_t             enable_cpu_load;
} AppObj;

AppObj gAppObj;
static vx_bool test_result = vx_true_e;

static char img_labels[TIVX_DL_POST_PROC_MAX_NUM_CLASSNAMES][TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME] = {""};
static int32_t label_offset3[TIVX_DL_POST_PROC_MAX_NUM_CLASSNAMES] = {0};

/* RGB Color Map for Semantic Segmentation */
static uint8_t rgb_color_map[26][3] = {{255,0,0},{0,255,0},{73,102,92},
                                       {255,255,0},{0,255,255},{0,99,245},
                                       {255,127,0},{0,255,100},{235,117,117},
                                       {242,15,109},{118,194,167},{255,0,255},
                                       {231,200,255},{255,186,239},{0,110,0},
                                       {0,0,255},{110,0,0},{110,110,0},
                                       {100,0,50},{90,60,0},{255,255,255} ,
                                       {170,0,255},{204,255,0},{78,69,128},
                                       {133,133,74},{0,0,110}};

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
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);
static void app_pipeline_params_defaults(AppObj *obj);
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
static vx_status load_cpu_task_create_and_start(AppObj *obj);
static void load_cpu_task_stop_and_delete(AppObj *obj);

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL Front Cam Demo - (c) Texas Instruments 2025\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static char menu[] = {
    "\n"
    "\n ========================="
    "\n Demo : TIDL Front Cam"
    "\n ========================="
    "\n"
    "\n p: Print performance statistics"
    "\n"
    "\n e: Export performance statistics"
    "\n"
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};

static void app_run_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status = VX_SUCCESS;

    while(!obj->stop_app_task && (status == VX_SUCCESS))
    {
        status = app_run_graph(obj);
    }
    obj->stop_app_task_done = 1;
}

static vx_status app_run_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    vx_status status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_task;
    params.app_var = obj;

    obj->stop_app_task_done = 0;
    obj->stop_app_task = 0;

    status = tivxTaskCreate(&obj->app_task, &params);

    return status;
}

static void app_run_task_delete(AppObj *obj)
{
    while(obj->stop_app_task_done==0)
    {
         tivxTaskWaitMsecs(100);
    }

    tivxTaskDelete(&obj->app_task);
}

/**
 * @brief Run the graph interactively.
 *
 * This function runs the graph, but allows the user to interact with it.
 * The user can type 'p' to print out the current performance statistics,
 * 'e' to export the current performance statistics to a file, or 'x' to exit.
 *
 * @param[in] obj The AppObj struct containing the graph, input tensors, output tensors, etc.
 * @return A vx_status indicating whether the function was successful or not.
 */
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
                    vx_reference refs[1];
                    refs[0] = (vx_reference)obj->captureObj.raw_image_arr[0];
                    if (status == VX_SUCCESS)
                    {
                        status = tivxNodeSendCommand(obj->captureObj.node, 0u,
                                    TIVX_CAPTURE_PRINT_STATISTICS,
                                    refs, 1u);
                    }
                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_front_cam");
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
            #if defined(APP_CTOOLS_ENABLED) && !defined(x86_64)
                case 'c':
                    runCtoolsDdrBw();
                break;
            #endif
                case 'x':
                    obj->stop_app_task = 1;
                    obj->stop_load_cpu_task = 1;
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
    snprintf(obj->odTIDLObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->odTIDLObj.network_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->sgTIDLObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->sgTIDLObj.network_file_path,APP_MAX_FILE_PATH, ".");

    snprintf(obj->captureObj.output_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->vissObj.output_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->ldcObj.output_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->scalerObj.output_file_path,APP_MAX_FILE_PATH, ".");

    obj->captureObj.en_out_capture_write = 0;
    obj->vissObj.en_out_viss_write = 0;
    obj->ldcObj.en_out_ldc_write = 0;
    obj->scalerObj.en_out_scaler_write = 0;

    obj->num_frames_to_write = 0;
    obj->num_frames_to_skip = 0;

    snprintf(obj->encode_output_file_path,APP_MAX_FILE_PATH, "./front_cam_out_encode.h264");
}

/**
 * @brief parse the configuration file specified by cfg_file_name
 *        and set the configuration values in the obj accordingly
 *
 * @param[in] obj pointer to the AppObj structure
 * @param[in] cfg_file_name the name of the configuration file
 *
 * @note The configuration file is expected to be in the following format:
 *       param_name1 value1
 *       param_name2 value2
 *       ...
 *
 *       The supported parameters are:
 *       - tidl_od_config  : the path to the od object detection config file
 *       - tidl_od_network : the path to the od object detection network file
 *       - tidl_sg_config  : the path to the sg semantic segmentation config file
 *       - tidl_sg_network : the path to the sg semantic segmentation network file
 *       - encode_output_file_path : the path to the encode output file
 *       - in_size         : the size of the input image
 *       - input_stream_window_size : the size of the input stream window on mosaic
 *       - od_dl_size      : the size of the od model input
 *       - sg_dl_size      : the size of the sg model input
 *       - viz_th          : the threshold for visualization for object detection
 *       - alpha           : the alpha value for blending for segmentation
 *       - en_out_img_write: 1 if the output should be written to a file, 0 otherwise
 *       - ip_rgb_or_yuv   : 1 if the input is RGB, 0 if the input is YUV
 *       - op_rgb_or_yuv   : 1 if the output is RGB, 0 if the output is YUV
 *       - display_option  : 1 if the output should be displayed on the screen, 0 otherwise
 *       - delay_in_msecs  : the delay in milliseconds between each frame
 *       - num_iterations  : the number of iterations to run the application
 *       - is_interactive  : 1 if the application is interactive, 0 otherwise
 *       - test_mode       : 1 if the application is in test mode, 0 otherwise
 *       - enable_od       : 1 if the od module should be enabled, 0 otherwise
 *       - enable_sem_seg  : 1 if the sg module should be enabled, 0 otherwise
 *       - enable_gui      : 1 if the gui module should be enabled, 0 otherwise
 */
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
            if(strcmp(token, "tidl_od_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->odTIDLObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_od_network")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->odTIDLObj.network_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_sg_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->sgTIDLObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_sg_network")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->sgTIDLObj.network_file_path, token);
                }
            }
            else
            if(strcmp(token, "encode_output_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->encode_output_file_path, token);
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

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->scalerObj.input.height = height;
                    }
                }
            }
            else
            if(strcmp(token, "input_stream_window_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                    width =  atoi(token);
                    obj->mosaic_window_width = width;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->mosaic_window_height = height;
                    }
                }
            }
            else
            if(strcmp(token, "od_dl_size")==0)
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
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->scalerObj.output[1].height  = height;
                    }
                }
            }
            else
            if(strcmp(token, "sg_dl_size")==0)
            {
                vx_int32 width, height;

                token = strtok(NULL, s);
                if(token != NULL)
                {
                    width =  atoi(token);
                    obj->scalerObj.output[2].width   = width;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        {
                            token[strlen(token)-1]=0;
                        }
                        height =  atoi(token);
                        obj->scalerObj.output[2].height  = height;
                    }
                }
            }
            else
            if(strcmp(token, "viz_th")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->odPostProcObj.viz_th = atof(token);
                }
            }
            else
            if(strcmp(token, "alpha")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->sgPostProcObj.alpha = atof(token);
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
            if(strcmp(token, "enable_od")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_od = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_sem_seg")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_sem_seg = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_encode")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_encode = atoi(token);
                    if(obj->enable_encode > 1)
                    {
                        obj->enable_encode = 1;
                    }
                }
            }
            else
            if(strcmp(token, "enable_gui")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_gui = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_cpu_load")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_cpu_load = atoi(token);
                }
            }
        }
    }
    if (obj->test_mode == 1)
    {
        obj->delay_in_msecs = 0;
        obj->is_interactive = 0;
        obj->sensorObj.is_interactive = 0;
        obj->displayObj.display_option = 1;
        obj->num_iterations = 300;
    }
    fclose(fp);
}

/**
 * @brief Parse command line arguments.
 *
 * This function parses command line arguments and sets the corresponding members
 * of the AppObj structure. The recognized arguments are:
 *   --cfg <config_file> : specify the configuration file to use
 *   --help : show the usage message
 *   --test : enter test mode
 *
 * @param obj The AppObj structure to populate
 * @param argc The number of command line arguments
 * @param argv The command line arguments
 */
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
        obj->sensorObj.is_interactive = 0;
        obj->displayObj.display_option = 1;
        obj->num_iterations = 300;
    }
    return;
}

vx_int32 app_tidl_front_cam_main(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;

    AppObj *obj = &gAppObj;

    // Optional parameter setting
    app_default_param_set(obj);
    APP_PRINTF("App set default params Done! \n");

    // Config parameter reading
    app_parse_cmd_line_args(obj, argc, argv);
    APP_PRINTF("App Parse User params Done! \n");
    
    // Update of parameters are config file read
    app_update_param_set(obj);
    APP_PRINTF("App Update Params Done! \n");

    // Querry sensor parameters
    app_querry_sensor(&obj->sensorObj);
    APP_PRINTF("Sensor params queried! \n");

    // Initialize modules
    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    // Create and verify graph
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

    // Run graph in interactive or non-interactive mode
    if(status == VX_SUCCESS)
    {   
        if (obj->enable_cpu_load == 1)
        {
            // Initializes and launches a task to generate 50% dummy load on A72/A53 CPU cores
            load_cpu_task_create_and_start(obj);
        }

        if(obj->is_interactive)
        {
            status = app_run_graph_interactive(obj);
        }
        else if (app_run_graph(obj) != VX_SUCCESS)
        {
            test_result = vx_false_e;
        }
        APP_PRINTF("App Run Graph Done! \n");

        if (obj->enable_cpu_load == 1)
        {
            load_cpu_task_stop_and_delete(obj);
        }
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
 *
 * This API first gets the parameter from the node using the node parameter index,
 * then adds it to the graph, and finally releases the parameter.
 *
 * \param [in] graph The graph to add the parameter to.
 * \param [in] node The node from which to extract the parameter.
 * \param [in] node_parameter_index The index of the parameter to which to get a reference.
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/**
 * @brief Initializes various modules and components required for the application.
 *
 * This function creates an OpenVX context and loads necessary kernels for hardware
 * acceleration and image processing. It initializes modules such as sensor, capture,
 * VISS, AEWB (Auto Exposure White Balance), LDC (Lens Distortion Correction), Scaler,
 * and TIDL (Texas Instruments Deep Learning) components. Additionally, it sets up 
 * pre-processing and post-processing modules, image mosaics,and display configurations. 
 * Performance measurement points are also initialized.
 *
 * Conditional initialization for GUI and encoding components is performed based on
 * the input parameters. The function returns a status indicating the success or
 * failure of the initialization process.
 *
 * @param obj A pointer to the application object containing configuration and state.
 * @return vx_status VX_SUCCESS on successful initialization, or an error status on failure.
 */
static vx_status app_init(AppObj *obj)
{
    int status = VX_SUCCESS;
    app_grpx_init_prms_t grpx_prms;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference)obj->context);
    APP_PRINTF("Creating context done!\n");

    if (status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxEdgeaiImgProcLoadKernels(obj->context);
        tivxImagingLoadKernels(obj->context);
        tivxTIDLLoadKernels(obj->context);    
        tivxFileIOLoadKernels(obj->context);
        APP_PRINTF("Kernel loading done!\n");
    }

    /* Initialize modules */
    if(status == VX_SUCCESS && obj->enable_capture == 1)
    {
        status = app_init_sensor(&obj->sensorObj, "sensor_obj");
        APP_PRINTF("Sensor init done!\n");

        status = app_init_capture(obj->context, &obj->captureObj, &obj->sensorObj, "capture_obj", APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Capture init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_viss == 1)
    {
        status = app_init_viss(obj->context, &obj->vissObj, &obj->sensorObj, "viss_obj", obj->sensorObj.num_cameras_enabled);
        APP_PRINTF("VISS init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_aewb == 1)
    {
        status = app_init_aewb(obj->context, &obj->aewbObj, &obj->sensorObj, "aewb_obj", 0, obj->sensorObj.num_cameras_enabled);
        APP_PRINTF("AEWB init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_ldc == 1)
    {
        status = app_init_ldc(obj->context, &obj->ldcObj, &obj->sensorObj, "ldc_obj", obj->sensorObj.num_cameras_enabled);
        APP_PRINTF("LDC init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_scaler == 1)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Scaler init done!\n");
    }
    /* Initialize TIDL first to get tensor I/O information from network */
    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_init_tidl(obj->context, &obj->odTIDLObj, "od_tidl_obj", obj->sensorObj.num_cameras_enabled);
        APP_PRINTF("OD TIDL init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_init_tidl(obj->context, &obj->sgTIDLObj, "sg_tidl_obj", obj->sensorObj.num_cameras_enabled);
        APP_PRINTF("SG TIDL init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_update_pre_proc(obj->context, &obj->odPreProcObj, obj->odTIDLObj.config);
        APP_PRINTF("OD Pre Proc update done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_update_pre_proc(obj->context, &obj->sgPreProcObj, obj->sgTIDLObj.config);
        APP_PRINTF("SG Pre Proc update done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_init_pre_proc(obj->context, &obj->odPreProcObj, "od_pre_proc_obj");
        APP_PRINTF("OD Pre Proc init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_init_pre_proc(obj->context, &obj->sgPreProcObj, "sg_pre_proc_obj");
        APP_PRINTF("SG Pre Proc init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_update_post_proc(obj->context, &obj->odPostProcObj, obj->odTIDLObj.config);
        APP_PRINTF("OD Post Proc update done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_update_post_proc(obj->context, &obj->sgPostProcObj, obj->sgTIDLObj.config);
        APP_PRINTF("SG Post Proc update done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_init_post_proc_od(obj->context, &obj->odPostProcObj, "od_post_proc_obj");
        APP_PRINTF("OD Post Proc init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_init_post_proc_sg(obj->context, &obj->sgPostProcObj, "sg_post_proc_obj");
        APP_PRINTF("SG Post Proc init done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_mosaic == 1)
    {
        status = app_init_img_mosaic(obj->context, &obj->imgMosaicObj, "img_mosaic_obj", APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Mosaic init done!\n");
    }
    if(status == VX_SUCCESS && obj->displayObj.display_option == 1)
    {
        status = app_init_display(obj->context, &obj->displayObj, "display_obj");
        APP_PRINTF("Display init done!\n");
    }

    if(status == VX_SUCCESS)
    {
        appPerfPointSetName(&obj->total_perf , "TOTAL");
        appPerfPointSetName(&obj->fileio_perf, "FILEIO");
    }
    if(obj->displayObj.display_option == 1 && obj->enable_gui)
    {
        appGrpxInitParamsInit(&grpx_prms, obj->context);
        grpx_prms.draw_callback = app_draw_graphics;
        appGrpxInit(&grpx_prms);
    }

#ifdef QNX
    if((obj->enable_encode == 1) && (status == VX_SUCCESS))
    {
        omx_encode_init_cfg(&obj->omx_encode_cfg);
        obj->omx_encode_cfg.width = ENCODE_WIDTH;
        obj->omx_encode_cfg.height = ENCODE_HEIGHT;
        obj->omx_encode_cfg.bufq_depth = APP_ENC_BUFFER_Q_DEPTH;
        sprintf(obj->omx_encode_cfg.file, obj->encode_output_file_path);
        obj->omx_encode_handle = omx_encode_create_handle(&obj->omx_encode_cfg);
    }
#endif

    return status;
}

static void app_deinit(AppObj *obj)
{
    if(obj->enable_capture == 1)
    {
        app_deinit_sensor(&obj->sensorObj);
        APP_PRINTF("\nSensor deinit done!\n");
        app_deinit_capture(&obj->captureObj, APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Capture deinit done!\n");
    }
    if(obj->enable_viss == 1)
    {
        app_deinit_viss(&obj->vissObj);
        APP_PRINTF("VISS deinit done!\n");
    }
    if(obj->enable_aewb == 1)
    {
        app_deinit_aewb(&obj->aewbObj);
        APP_PRINTF("AEWB deinit done!\n");
    }
    if (obj->enable_ldc == 1)
    {
        app_deinit_ldc(&obj->ldcObj);
        APP_PRINTF("LDC deinit done!\n");
    }
    if (obj->enable_scaler == 1)
    {
        app_deinit_scaler(&obj->scalerObj, APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Scaler deinit done!\n");
    }
    if (obj->enable_od == 1)
    {
        app_deinit_pre_proc(&obj->odPreProcObj);
        APP_PRINTF("OD Pre Proc deinit done!\n");
        
        app_deinit_tidl(&obj->odTIDLObj);
        APP_PRINTF("OD TIDL  deinit done!\n");

        app_deinit_post_proc(&obj->odPostProcObj);
        APP_PRINTF("OD Post Proc deinit done!\n");

    }
    if (obj->enable_sem_seg == 1)
    {
        app_deinit_pre_proc(&obj->sgPreProcObj);
        APP_PRINTF("SG Pre Proc deinit done!\n");

        app_deinit_tidl(&obj->sgTIDLObj);
        APP_PRINTF("SG TIDL deinit done!\n");

        app_deinit_post_proc(&obj->sgPostProcObj);
        APP_PRINTF("SG Post Proc deinit done!\n");
    }
    if (obj->enable_mosaic == 1)
    {
        app_deinit_img_mosaic(&obj->imgMosaicObj, APP_BUFFER_Q_DEPTH);
        APP_PRINTF("Mosaic deinit done!\n");
    }
    if (obj->displayObj.display_option == 1)
    {
        app_deinit_display(&obj->displayObj);
        APP_PRINTF("Display deinit done!\n");
    }
    if(obj->displayObj.display_option == 1 && obj->enable_gui)
    {
        appGrpxDeInit();
    }

    tivxFileIOUnLoadKernels(obj->context);
    tivxTIDLUnLoadKernels(obj->context);
    tivxImagingUnLoadKernels(obj->context);
    tivxEdgeaiImgProcUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);
    APP_PRINTF("Kernels unload done!\n");

    vxReleaseContext(&obj->context);
    APP_PRINTF("Release context done!\n");
}

/**
 * @brief Deletes all the objects in the graph and releases the graph reference.
 *
 * @param[in] obj   Pointer to the AppObj structure.
 *
 * This function deletes the objects in the graph and releases the graph reference.
 * The delete functions for each object are called in the reverse order of their creation.
 * The graph reference is released at the end.
 */
static void app_delete_graph(AppObj *obj)
{    
    if(obj->enable_capture == 1)
    {
        app_delete_capture(&obj->captureObj);
        APP_PRINTF("\nCapture delete done!\n");
    }
    if(obj->enable_viss == 1)
    {
        app_delete_viss(&obj->vissObj);
        APP_PRINTF("VISS delete done!\n");
    }
    if(obj->enable_aewb == 1)
    {
        app_delete_aewb(&obj->aewbObj);
        APP_PRINTF("AEWB delete done!\n");
    }
    if (obj->enable_ldc == 1)
    {
        app_delete_ldc(&obj->ldcObj);
        APP_PRINTF("LDC delete done!\n");
    }
    if (obj->enable_scaler == 1)
    {
        app_delete_scaler(&obj->scalerObj);
        APP_PRINTF("Scaler delete done!\n");
    }
    if (obj->enable_od == 1){
        app_delete_pre_proc(&obj->odPreProcObj);
        APP_PRINTF("OD Pre Proc delete done!\n");

        app_delete_tidl(&obj->odTIDLObj);
        APP_PRINTF("OD TIDL delete done!\n");

        app_delete_post_proc(&obj->odPostProcObj);
        APP_PRINTF("OD Post Proc delete done!\n");
    }
    if (obj->enable_sem_seg == 1)
    {
        app_delete_pre_proc(&obj->sgPreProcObj);
        APP_PRINTF("SG Pre Proc delete done!\n");

        app_delete_tidl(&obj->sgTIDLObj);
        APP_PRINTF("SG TIDL delete done!\n");

        app_delete_post_proc(&obj->sgPostProcObj);
        APP_PRINTF("SG Post Proc delete done!\n");
    }
    if (obj->enable_mosaic == 1)
    {
        app_delete_img_mosaic(&obj->imgMosaicObj);
        APP_PRINTF("Img Mosaic delete done!\n");
    }
    if (obj->displayObj.display_option == 1)
    {
        app_delete_display(&obj->displayObj);
        APP_PRINTF("Display delete done!\n");
    }

#ifdef APP_TIVX_LOG_RT_ENABLE
    tivxLogRtTraceExportToFile("app_tidl_front_cam.bin");
    tivxLogRtTraceDisable(obj->graph);
#endif

    vxReleaseGraph(&obj->graph);
    APP_PRINTF("Graph delete done!\n");
}

/**
 * @brief Creates the OpenVX graph for the front camera application.
 *
 * This function constructs an OpenVX graph for processing images captured
 * from a front camera using various image processing and deep learning
 * components. The graph includes components such as capture, VISS, AEWB,
 * LDC, scaler, pre-processing, TIDL, post-processing, and image mosaic.
 *
 * The graph is configured based on the application's settings, which
 * determine the inclusion of each component. The function also sets up
 * the graph's scheduling and pipeline depth for efficient execution.
 *
 * @param[in] obj Pointer to the application object containing configuration
 *                and state.
 * 
 * @return vx_status VX_SUCCESS on successful graph creation, or an error
 *         status on failure.
 */
static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

#ifdef QNX
    vx_int32 list_depth = (obj->enable_encode == 1) ? 2 : 1;
#else
    vx_int32 list_depth = 1;
#endif
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[list_depth];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);
    vxSetReferenceName((vx_reference)obj->graph, "app_front_cam_graph");
    APP_PRINTF("Graph create done!\n");

    if(status == VX_SUCCESS && obj->enable_capture == 1)
    {
        status = app_create_graph_capture(obj->graph, &obj->captureObj);
        APP_PRINTF("Capture graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_viss == 1)
    {
        status = app_create_graph_viss(obj->graph, &obj->vissObj, obj->captureObj.raw_image_arr[0], TIVX_TARGET_VPAC_VISS1);
        APP_PRINTF("VISS graph done!\n");
    }
    
    if (status == VX_SUCCESS && obj->enable_aewb == 1)
    {
        status = app_create_graph_aewb(obj->graph, &obj->aewbObj, obj->vissObj.h3a_stats_arr);
        APP_PRINTF("AEWB graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_ldc == 1)
    {
        if(status == VX_SUCCESS)
        {
            status = app_create_graph_ldc(obj->graph, &obj->ldcObj, obj->vissObj.output_arr, TIVX_TARGET_VPAC_LDC1);
            APP_PRINTF("LDC graph done!\n");
        }
    }

    if(status == VX_SUCCESS && obj->enable_scaler == 1)
    {
        if(obj->enable_ldc == 1)
        {
            status = app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, obj->ldcObj.output_arr);
            APP_PRINTF("Scaler graph done!\n");
        }
        else
        {
            status = app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, obj->vissObj.output_arr);
            APP_PRINTF("Scaler graph done!\n");
        }
    }

    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_create_graph_pre_proc(obj->graph, &obj->odPreProcObj, obj->scalerObj.output[1].arr[0], "od_pre_proc_node");
        APP_PRINTF("OD Pre proc graph done!\n");
    }
    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_create_graph_pre_proc(obj->graph, &obj->sgPreProcObj, obj->scalerObj.output[2].arr[0], "sg_pre_proc_node");
        APP_PRINTF("SG Pre proc graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_create_graph_tidl(obj->context, obj->graph, &obj->odTIDLObj, obj->odPreProcObj.output_tensor_arr);
        APP_PRINTF("TIDL OD graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_create_graph_tidl(obj->context, obj->graph, &obj->sgTIDLObj, obj->sgPreProcObj.output_tensor_arr);
        APP_PRINTF("TIDL SG graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_od == 1)
    {
        status = app_create_graph_post_proc(obj->graph, &obj->odPostProcObj, obj->scalerObj.output[3].arr[0], obj->odTIDLObj.output_tensor_arr[0], "od_post_proc_node");
        APP_PRINTF("OD Post Proc graph done!\n");
    }

    if(status == VX_SUCCESS && obj->enable_sem_seg == 1)
    {
        status = app_create_graph_post_proc(obj->graph, &obj->sgPostProcObj, obj->scalerObj.output[3].arr[0], obj->sgTIDLObj.output_tensor_arr[0], "sg_post_proc_node");
        APP_PRINTF("SG Post Proc graph done!\n");
    }
    
    vx_int32 idx = 0;    
    if (status == VX_SUCCESS && obj->enable_mosaic == 1){
        if (obj->enable_capture == 1)
        {
            obj->imgMosaicObj.input_arr[idx++] = obj->scalerObj.output[0].arr[0]; // Camera Stream
        }

        if(obj->enable_od == 1)
        {
            obj->imgMosaicObj.input_arr[idx++] = obj->odPostProcObj.output_image_arr; // OD output
        }
        else{
            obj->imgMosaicObj.input_arr[idx++] = obj->scalerObj.output[0].arr[0]; // Camera Stream
        }

        if(obj->enable_sem_seg == 1)
        {
            obj->imgMosaicObj.input_arr[idx++] = obj->sgPostProcObj.output_image_arr; // Seg output
        }
        else{
            obj->imgMosaicObj.input_arr[idx++] = obj->scalerObj.output[0].arr[0]; // Camera Stream
        }

        obj->imgMosaicObj.num_inputs = idx;
        app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);
    }

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);
    }
    
    if((status == VX_SUCCESS) && (obj->pipeline_exec == 1))
    {
        graph_parameter_index = 0;
        if(obj->enable_capture == 1)
        {
            add_graph_parameter_by_node_index(obj->graph, obj->captureObj.node, 1);
            obj->captureObj.graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->captureObj.raw_image_arr[0];
            graph_parameter_index++;
        }
#ifdef QNX
        if(obj->enable_encode == 1)
        {
            add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 1);
            obj->imgMosaicObj.graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.output_image[0];
            graph_parameter_index++;
        }
#endif  
        vxSetGraphScheduleConfig(obj->graph,
                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                graph_parameter_index,
                graph_parameters_queue_params_list);

        tivxSetGraphPipelineDepth(obj->graph, APP_PIPELINE_DEPTH);
        if(obj->enable_viss == 1)
        {
            tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 6, APP_BUFFER_Q_DEPTH);
            tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 9, APP_BUFFER_Q_DEPTH);
        }
        
        if(obj->enable_aewb == 1)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->aewbObj.node, 4, APP_BUFFER_Q_DEPTH);
        }

        if(obj->enable_ldc == 1)
        {
            tivxSetNodeParameterNumBufByIndex(obj->ldcObj.node, 7, APP_BUFFER_Q_DEPTH);
        }   

        if (obj->enable_scaler == 1)
        {
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 4);
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, 4);
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 3, 4);
            tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 4, 4);
        }
        
        if(obj->enable_od == 1)
        {
            tivxSetNodeParameterNumBufByIndex(obj->odPreProcObj.node, 2, APP_BUFFER_Q_DEPTH);
            status = tivxSetNodeParameterNumBufByIndex(obj->odTIDLObj.node, 7, APP_BUFFER_Q_DEPTH);
            tivxSetNodeParameterNumBufByIndex(obj->odPostProcObj.node, 2, 4);
        }

        if(obj->enable_sem_seg == 1)
        {
            tivxSetNodeParameterNumBufByIndex(obj->sgPreProcObj.node, 2, APP_BUFFER_Q_DEPTH);
            status = tivxSetNodeParameterNumBufByIndex(obj->sgTIDLObj.node, 7, APP_BUFFER_Q_DEPTH);
            tivxSetNodeParameterNumBufByIndex(obj->sgPostProcObj.node, 2, 4);
        }
        

        APP_PRINTF("Pipeline params setup done!\n");
    }
    return status;
}

/**
 * @brief Verify the graph for errors and export it to a dot file
 *
 * This function verifies the graph for errors and exports it to a dot file.
 * It also enables real-time tracing and sends an error frame to the capture
 * object if error detection is enabled.
 *
 * @param obj The application object
 *
 * @returns A vx_status indicating success or failure
 */
static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graph);
    
    if(status == VX_SUCCESS)
    {
        APP_PRINTF("Graph verify SUCCESS!\n");
    }
    else
    {
        APP_PRINTF("Graph verify FAILURE!\n");
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_front_cam");
    }

#ifdef APP_TIVX_LOG_RT_ENABLE
    if(VX_SUCCESS == status)
    {
        tivxLogRtTraceEnable(obj->graph);
    }
#endif

    if(VX_SUCCESS == status)
    {
        if (obj->captureObj.enable_error_detection)
        {
            status = app_send_error_frame(&obj->captureObj);
            APP_PRINTF("App Send Error Frame Done! %d \n", obj->captureObj.enable_error_detection);
        }
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

/**
 * @brief Runs the graph for one frame
 *
 * This function is responsible for running the graph for one frame. It
 * enqueues the output and input buffers, and starts the graph execution.
 *
 * @param obj The application object
 * @param frame_id The current frame ID
 *
 * @returns A vx_status indicating success or failure
 */
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id)
{
    vx_status status = VX_SUCCESS;
    uint64_t cur_time;
    appPerfPointBegin(&obj->total_perf);
    cur_time = tivxPlatformGetTimeInUsecs();

    if(obj->pipeline <= 0)
    {
#ifdef QNX
        if (obj->enable_encode == 1){
            /* Enqueue outputs */
            vxGraphParameterEnqueueReadyRef(obj->graph, obj->imgMosaicObj.graph_parameter_index, (vx_reference*)&obj->imgMosaicObj.output_image[obj->enqueueCnt], 1);
        }
#endif
        /* Enqueue inputs during pipeup dont execute */
        vxGraphParameterEnqueueReadyRef(obj->graph, obj->captureObj.graph_parameter_index, (vx_reference*)&obj->captureObj.raw_image_arr[obj->enqueueCnt], 1);

        obj->enqueueCnt++;
        obj->enqueueCnt   = (obj->enqueueCnt  >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
    }

    if(obj->pipeline > 0)
    {
        vx_reference raw_image;
        uint32_t num_refs;

        /* Dequeue input */
        vxGraphParameterDequeueDoneRef(obj->graph, obj->captureObj.graph_parameter_index, (vx_reference*)&raw_image, 1, &num_refs);
    
#ifdef QNX
        vx_image mosaic_output_image;

        if (obj->enable_encode == 1)
        {    
            vxGraphParameterDequeueDoneRef(obj->graph, obj->imgMosaicObj.graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);
    
            obj->encode_buf[obj->buf_index].buf_index = obj->buf_index;
            obj->encode_buf[obj->buf_index].handle = (vx_reference)mosaic_output_image;

            if (obj->pipeline <= obj->omx_encode_cfg.bufq_depth)
            {
                omx_encode_enqueue_buf(obj->omx_encode_handle, &obj->encode_buf[obj->buf_index]);
                if (obj->pipeline == obj->omx_encode_cfg.bufq_depth) // need to run once only to start encode
                {
                    omx_encode_start(obj->omx_encode_handle);
                    APP_PRINTF("OMX encode started.\n");
                }
            }
            if (obj->pipeline >= obj->omx_encode_cfg.bufq_depth) {
                omx_encode_dqueue_buf(obj->omx_encode_handle);
                omx_encode_enqueue_buf(obj->omx_encode_handle, &obj->encode_buf[obj->buf_index]);
            }

            obj->buf_index++;
            obj->buf_index = (obj->buf_index >= obj->omx_encode_cfg.bufq_depth)? 0 : obj->buf_index;

            /* Enqueue input - start execution */
            vxGraphParameterEnqueueReadyRef(obj->graph, obj->imgMosaicObj.graph_parameter_index, (vx_reference*)&mosaic_output_image, 1);
        }
#endif
        /* Enqueue input - start execution */
        vxGraphParameterEnqueueReadyRef(obj->graph, obj->captureObj.graph_parameter_index, &raw_image, 1);

        obj->enqueueCnt++;
        obj->dequeueCnt++;

        obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->dequeueCnt = (obj->dequeueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->dequeueCnt;
    }

#if SOC_J722S
    // add delay to set 20 fps
    tivxTaskWaitMsecs(25);
#endif

    obj->pipeline++;
    appPerfPointEnd(&obj->total_perf);

    /**
     * Ensures that each iteration of the processing loop takes at least a predefined duration,
     * effectively controlling the minimum cycle time (or maximum frame rate).
     *
     * The variable `obj->delay_in_msecs` represents the target duration for one iteration,
     * not an additional delay. If the processing completes faster than this target,
     * the function waits for the remaining time to maintain a consistent iteration period.
     */
    cur_time = (tivxPlatformGetTimeInUsecs() - cur_time) / 1000;
    cur_time = cur_time/1000;  /* convert to msecs */
    
    if(cur_time < obj->delay_in_msecs)
    {
        tivxTaskWaitMsecs(obj->delay_in_msecs - cur_time);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    
    SensorObj *sensorObj = &obj->sensorObj;
    vx_int32 frame_id;

    app_pipeline_params_defaults(obj);

    if(obj->enable_capture == 1)
    {
        if('\0' == sensorObj->sensor_name[0])
        {
            printf("sensor name is NULL \n");
            return VX_FAILURE;
        }
        status = appStartImageSensor(sensorObj->sensor_name, ((1 << sensorObj->num_cameras_enabled) - 1));
    }
    APP_PRINTF("\n");
    for(frame_id = 0; frame_id < obj->num_iterations; frame_id++)
    {
#ifdef APP_WRITE_INTERMEDIATE_OUTPUTS
        if(obj->write_file == 1)
        {
            if(obj->captureObj.en_out_capture_write == 1)
            {
                app_send_cmd_capture_write_node(&obj->captureObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->ldcObj.en_out_ldc_write == 1)
            {
                app_send_cmd_ldc_write_node(&obj->ldcObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->scalerObj.en_out_scaler_write == 1)
            {
                app_send_cmd_scaler_write_node(&obj->scalerObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            obj->write_file = 0;
        }
#endif
        if(obj->pipeline_exec == 1)
        {
            APP_PRINTF("Processing Frame Number : %d\n", frame_id);
            app_run_graph_for_one_frame_pipeline(obj, frame_id);
        }

        /* user asked to stop processing */
        if(obj->stop_app_task)
            break;
    }
    if(obj->pipeline_exec == 1)
    {
        vxWaitGraph(obj->graph);
    }

    obj->stop_app_task = 1;

    if(obj->enable_capture == 1)
    {
        status = appStopImageSensor(obj->sensorObj.sensor_name, ((1 << sensorObj->num_cameras_enabled) - 1));
    }

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

static void set_post_proc_defaults_od(PostProcObj *postProcObj)
{
    postProcObj->output_img_width = 768;  
    postProcObj->output_img_height = 384;
    
    postProcObj->params.od_prms.scaleX = 1;
    postProcObj->params.od_prms.scaleY = 1;
    
    postProcObj->params.od_prms.formatter[0] = 0;
    postProcObj->params.od_prms.formatter[1] = 1;
    postProcObj->params.od_prms.formatter[2] = 2;
    postProcObj->params.od_prms.formatter[3] = 3;
    postProcObj->params.od_prms.formatter[4] = 5;
    postProcObj->params.od_prms.formatter[5] = 4;

    postProcObj->params.od_prms.labelIndexOffset = 0;

    postProcObj->params.od_prms.labelOffset = label_offset3;
    postProcObj->params.od_prms.classnames = img_labels;

    postProcObj->params.od_prms.viz_th = 0.5;

}

static void update_post_proc_params_od(AppObj *obj, PostProcObj *postProcObj)
{
    postProcObj->output_img_width = obj->scalerObj.output[3].width;  
    postProcObj->output_img_height = obj->scalerObj.output[3].height;

    postProcObj->params.od_prms.scaleX = postProcObj->output_img_width / 416.0;
    postProcObj->params.od_prms.scaleY = postProcObj->output_img_height / 416.0;
    
    postProcObj->params.od_prms.viz_th = postProcObj->viz_th;
}

static void set_post_proc_defaults_sg(PostProcObj *postProcObj)
{
    postProcObj->output_img_width = 768;
    postProcObj->output_img_height = 384;

    postProcObj->params.ss_prms.alpha = 0.5;
    
    uint32_t max_color_class = sizeof(rgb_color_map)/sizeof(rgb_color_map[0]);
    postProcObj->params.ss_prms.MaxColorClass = max_color_class;
    postProcObj->params.ss_prms.YUVColorMap = (uint8_t **)malloc(sizeof(uint8_t *) * max_color_class);

    for (uint32_t i = 0; i < max_color_class; i++)
    {
        postProcObj->params.ss_prms.YUVColorMap[i] = (uint8_t *) malloc(sizeof(uint8_t) * 3);
        uint8_t R = rgb_color_map[i][0];
        uint8_t G = rgb_color_map[i][1];
        uint8_t B = rgb_color_map[i][2];
        postProcObj->params.ss_prms.YUVColorMap[i][0] = RGB2Y(R,G,B);
        postProcObj->params.ss_prms.YUVColorMap[i][1] = RGB2U(R,G,B);
        postProcObj->params.ss_prms.YUVColorMap[i][2] = RGB2V(R,G,B);
    }

}

static void update_post_proc_params_sg(AppObj *obj, PostProcObj *postProcObj)
{
    postProcObj->output_img_width = obj->scalerObj.output[3].width;  
    postProcObj->output_img_height = obj->scalerObj.output[3].height;

    postProcObj->params.ss_prms.alpha = postProcObj->alpha;
}

static void set_img_mosaic_defaults(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx = 0;
    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->num_inputs   = 3;

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);
    // Input Stream
    imgMosaicObj->params.windows[idx].startX  = 133;
    imgMosaicObj->params.windows[idx].startY  = 260;
    imgMosaicObj->params.windows[idx].width   = obj->mosaic_window_width;
    imgMosaicObj->params.windows[idx].height  = obj->mosaic_window_height;
    imgMosaicObj->params.windows[idx].input_select   = 0; // Input Stream
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    // Object Detection
    imgMosaicObj->params.windows[idx].startX  = 1019;
    imgMosaicObj->params.windows[idx].startY  = 5;
    imgMosaicObj->params.windows[idx].width   = obj->mosaic_window_width;
    imgMosaicObj->params.windows[idx].height  = obj->mosaic_window_height;
    imgMosaicObj->params.windows[idx].input_select   = 1; // Object Detection
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    // Semantic Segmentation
    imgMosaicObj->params.windows[idx].startX  = 1019;
    imgMosaicObj->params.windows[idx].startY  = 447;
    imgMosaicObj->params.windows[idx].width   = obj->mosaic_window_width;
    imgMosaicObj->params.windows[idx].height  = obj->mosaic_window_height;
    imgMosaicObj->params.windows[idx].input_select   = 2; // Semantic Segmentation
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = 12;
}

static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 0;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->pipeline       = -APP_BUFFER_Q_DEPTH + 1;
    obj->enqueueCnt     = 0;
    obj->dequeueCnt     = 0;
}

static void set_sensor_defaults(SensorObj *sensorObj)
{
    strcpy(sensorObj->sensor_name, SENSOR_SONY_IMX728_UB971_SONY);

    sensorObj->sensor_index = 4; // IMX728-UB971_SONY
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
    sensorObj->is_interactive = 1;
}

#ifdef QNX
static void set_omx_encode_defaults(AppObj *obj)
{
    vx_int32 i;
    obj->buf_index = 0;

    for (i = 0; i < APP_ENC_BUFFER_Q_DEPTH; i++)
    {
        obj->encode_buf[i].num_channels = NUM_CH;
    }
}
#endif

static void update_scaler_params(AppObj *obj)
{    
    obj->scalerObj.output[0].width  = obj->mosaic_window_width;
    obj->scalerObj.output[3].width  = obj->mosaic_window_width;
    obj->scalerObj.output[0].height  = obj->mosaic_window_height;
    obj->scalerObj.output[3].height  = obj->mosaic_window_height;
}

/**
 * @brief Set default parameters for the application
 *
 * This function sets default parameters for the application. It is called
 * once at the beginning of the application. It sets default parameters for
 * the sensor, preprocessing, postprocessing, display, and other components
 * of the application.
 *
 * @param obj pointer to the application object
 */
static void app_default_param_set(AppObj *obj)
{
    set_sensor_defaults(&obj->sensorObj);

    set_pre_proc_defaults(&obj->odPreProcObj);
    set_pre_proc_defaults(&obj->sgPreProcObj);

    set_post_proc_defaults_od(&obj->odPostProcObj);
    set_post_proc_defaults_sg(&obj->sgPostProcObj);

    set_display_defaults(&obj->displayObj);
    app_pipeline_params_defaults(obj);

#ifdef QNX
    set_omx_encode_defaults(obj);
#endif

    obj->num_iterations = 1000000000;
    obj->delay_in_msecs = 0;
    obj->is_interactive = 1;
    obj->test_mode      = 0;
    obj->enable_gui     = 1;

    obj->enable_od      = 1;
    obj->enable_sem_seg = 1;

    obj->enable_capture   = 1;
    obj->enable_viss      = 1;
    obj->enable_aewb      = 1;
    obj->enable_ldc       = 1;
    obj->enable_scaler    = 1;
    obj->enable_mosaic    = 1;
    obj->enable_encode    = 1;
    obj->enable_cpu_load  = 1;
    
    obj->pipeline_exec    = 1;

    obj->captureObj.enable_error_detection = 1; /* enable by default */
    obj->write_file = 0;
}

static void app_update_param_set(AppObj *obj)
{    
    update_scaler_params(obj);

    update_post_proc_params_od(obj, &obj->odPostProcObj);
    update_post_proc_params_sg(obj, &obj->sgPostProcObj);

    set_img_mosaic_defaults(obj, &obj->imgMosaicObj);

    if(obj->is_interactive)
    {
        obj->num_iterations = 1000000000;
    }
    else{
        obj->sensorObj.is_interactive = 0;
        obj->sensorObj.ch_mask = 0x01;
        obj->sensorObj.enable_ldc = 1;
    }
    obj->enable_ldc = obj->sensorObj.enable_ldc;
}

static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        Draw2D_FontPrm sLabel;

        sHeading.fontIdx = 0;
        Draw2D_drawString(handle, 240, 120, "Front Camera Demo", &sHeading);

        sLabel.fontIdx = 2;
        Draw2D_drawString(handle, 415, 265, "Input Stream", &sLabel);
        Draw2D_drawString(handle, 1275, 10, "Object Detection", &sLabel);
        Draw2D_drawString(handle, 1226, 450, "Semantic Segmentation", &sLabel);
    }
    return;
}

/**
 * @brief The load_cpu_task function is a CPU-intensive task that is designed to simulate a load on the CPU.
 *
 * @param app_var The application object that contains the parameters and state of the application.
 */
static void load_cpu_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    
    // Perform a CPU-intensive calculation
    volatile vx_int64 x = 0;
    volatile vx_int64 i;
    while (!obj->stop_load_cpu_task) {
        
        // Perform a CPU-intensive calculation
        for (i = 0; i < 300000; i++) {
            x += i * i; 
        }

        // Sleep for a short period of time to avoid 100% CPU utilization
        tivxTaskWaitMsecs(1);
    }

    // Signal that the task is complete
    tivxMutexLock(obj->load_cpu_task_mutex);
    obj->stop_load_cpu_task_done++;
    tivxMutexUnlock(obj->load_cpu_task_mutex);

    APP_PRINTF("CPU load task done\n");
}

/**
 * @brief Creates and initializes CPU load tasks for each CPU core.
 *
 * This function sets up the parameters for the CPU load tasks,
 * initializes a mutex for synchronizing task completion status,
 * and creates a task for each available CPU core.
 *
 * @param obj Pointer to the application object containing configuration
 *            and state information.
 *
 * @return vx_status VX_SUCCESS if all tasks are successfully created,
 *         or an error status if task creation fails.
 */
static vx_status load_cpu_task_create_and_start(AppObj *obj)
{
    tivx_task_create_params_t params;
    vx_status status;

    // Set default parameters for task creation
    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = load_cpu_task;
    params.app_var = obj;
    params.priority = TIVX_TASK_PRI_LOWEST;

    // Initialize task control variables
    obj->stop_load_cpu_task_done = 0;
    obj->stop_load_cpu_task = 0;
    
    // Create a mutex for synchronizing task completion
    tivxMutexCreate(&obj->load_cpu_task_mutex);

    // Create a task for each CPU core
    int i = 0;
    for (i = 0; i < NUM_CPU_CORES; i++)
    {
        status = tivxTaskCreate(&obj->load_cpu_task[i], &params);
        APP_PRINTF("App %dth task created for CPU load\n", i);
    }
    
    return status; 
}

/**
 * @brief Deletes CPU load tasks.
 *
 * This function waits for all CPU load tasks to complete, deletes the
 * mutex used to synchronize task completion status, and then deletes
 * each of the CPU load tasks.
 *
 * @param obj Pointer to the application object containing configuration
 *            and state information.
 */
static void load_cpu_task_stop_and_delete(AppObj *obj)
{
    // stop the load cpu task
    obj->stop_load_cpu_task = 1;

    // Wait for all tasks to complete
    while (obj->stop_load_cpu_task_done != NUM_CPU_CORES)
    {
        tivxTaskWaitMsecs(100);
    }

    // Delete the mutex used to synchronize task completion status
    tivxMutexDelete(&obj->load_cpu_task_mutex);

    // Delete each of the CPU load tasks
    int i = 0;
    for (i = 0; i < NUM_CPU_CORES; i++)
    {
        tivxTaskDelete(&obj->load_cpu_task[i]);
        APP_PRINTF("App %dth task deleted for CPU load\n", i);
    }
}
