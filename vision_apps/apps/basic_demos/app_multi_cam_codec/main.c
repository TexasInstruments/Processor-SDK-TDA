/*
 *
 * Copyright (c) 2020-2025 Texas Instruments Incorporated
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
#include <utils/hwa/include/app_hwa_hlos.h>
#include <utils/codec_wrapper/include/codec_wrapper.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/video_io_kernels.h>

#include "app_common.h"
#include "app_sensor_module.h"
#include "app_capture_module.h"
#include "app_viss_module.h"
#include "app_aewb_module.h"
#include "app_display_module.h"
#include "app_test.h"

#include "multi_cam_codec_ldc_module.h"
#include "multi_cam_codec_scaler_module.h"
#include "multi_cam_codec_img_mosaic_module.h"

#define APP_BUFFER_Q_DEPTH   (2)
#define CODEC_ENC_BUFQ_DEPTH   (2)

#if defined(QNX)
#define CODEC_DEC_BUFQ_DEPTH   (4)
#else
#define CODEC_DEC_BUFQ_DEPTH   (0)
#endif

#define APP_ENC_BUFFER_Q_DEPTH   (APP_BUFFER_Q_DEPTH + CODEC_ENC_BUFQ_DEPTH + 2)
#define APP_DEC_BUFFER_Q_DEPTH   (APP_BUFFER_Q_DEPTH + CODEC_DEC_BUFQ_DEPTH)

#if defined(QNX)
#define CODEC_DEC_BUFQ_SIZE   (3)
#else
#define CODEC_DEC_BUFQ_SIZE   (APP_DEC_BUFFER_Q_DEPTH + 1)
#endif

#define COPY_BUFFER_Q_DEPTH   (1)

#define CAPTURE_PIPELINE_DEPTH   (5)
#define DISPLAY_PIPELINE_DEPTH   (2)

// #define DEC_WRITE_TO_FILE

typedef struct {

    vx_object_array arr[APP_MODULES_MAX_BUFQ_DEPTH];
    vx_image        img[APP_MODULES_MAX_BUFQ_DEPTH];

    void           *data_ptr[APP_MODULES_MAX_BUFQ_DEPTH][CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES];
    vx_map_id       map_id[APP_MODULES_MAX_BUFQ_DEPTH][CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES];
    vx_uint32       plane_sizes[CODEC_MAX_NUM_PLANES];

    vx_int32        bufq_depth;
    vx_int32        num_channels;
    vx_int32        num_planes;

    vx_int32        width;
    vx_int32        height;
    vx_df_image     format;

    vx_int32        graph_parameter_index;

} AppGraphParamRefPool;

typedef struct {

    SensorObj     sensorObj;
    CaptureObj    captureObj;
    VISSObj       vissObj;
    AEWBObj       aewbObj;
    LDCObj        ldcObj;
    ScalerObj     scalerObj;
    TIOVXImgMosaicModuleObj  imgMosaicObj;
    DisplayObj    displayObj;

    app_codec_wrapper_params_t codec_pipe_params;

    AppGraphParamRefPool enc_pool;
    AppGraphParamRefPool dec_pool;

    vx_char output_file_path[APP_MAX_FILE_PATH];

    /* OpenVX references */
    vx_context context;
    vx_graph   capture_graph;
    vx_graph   display_graph;
    vx_object_array ldc_out_arr_q[APP_MODULES_MAX_BUFQ_DEPTH];

    vx_int32 en_out_img_write;
    vx_int32 test_mode;

    vx_uint32 is_interactive;

    vx_uint32 num_codec_bufs;
    vx_uint32 num_ch;

    vx_uint32 num_frames_to_run;

    vx_uint32 num_frames_to_write;
    vx_uint32 num_frames_to_skip;

    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;
    vx_uint32 EOS;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t enable_viss;
    int32_t enable_aewb;
    int32_t enable_mosaic;
    int32_t encode;
    int32_t decode;
    int32_t downscale;

    vx_uint32 ldc_enq_id;
    vx_uint32 appsrc_push_id;
    vx_uint32 mosaic_enq_id;
    vx_uint32 appsink_pull_id;
    vx_uint32 capture_id;
    vx_uint32 display_id;

    int32_t write_file;

    vx_uint32 enable_configure_hwa_freq;
    vx_uint32 hwa_freq_config;

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
static void app_querry_param_set(AppObj *obj);
static void app_update_param_set(AppObj *obj);
static void app_pipeline_params_defaults(AppObj *obj);
static vx_int32 calc_grid_size(vx_uint32 ch);
static void set_img_mosaic_params(TIOVXImgMosaicModuleObj *imgMosaicObj, vx_uint32 in_width, vx_uint32 in_height, vx_int32 numCh);
static vx_status map_vx_object_arr(vx_object_array in_arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_map_id map_id[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_int32 num_channels);
static vx_status unmap_vx_object_arr(vx_object_array in_arr, vx_map_id map_id[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_int32 num_channels);
static vx_status capture_encode(AppObj* obj, vx_int32 frame_id);
#if defined(DEC_WRITE_TO_FILE)
static vx_status decode_display(AppObj* obj, vx_int32 frame_id, FILE *fp);
static vx_status write_vx_object_arr(vx_object_array in_arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], FILE *fp);
#else
static vx_status decode_display(AppObj* obj, vx_int32 frame_id);
#endif
#if defined(LINUX)
static vx_status delete_array_image_buffers(vx_object_array arr);
#endif
static vx_status assign_array_image_buffers(vx_object_array arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_uint32 sizes[CODEC_MAX_NUM_PLANES]);

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" Camera Demo - (c) Texas Instruments 2020\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ========================="
    "\n Demo : Camera Demo"
    "\n ========================="
    "\n"
    "\n s: Save CSIx, VISS and LDC outputs"
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
    while((!obj->stop_task) && (status == VX_SUCCESS))
    {
        status = app_run_graph(obj);
    }
    obj->stop_task_done = 1;
    if ( obj->EOS )
    {
        printf("\nEOS: app_run_task() exiting. Choose x to exit app:\n");
        printf(menu);
    }
}

static int32_t app_run_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    vx_status status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_task;
    params.app_var = obj;

    obj->stop_task_done = 0;
    obj->stop_task = 0;
    obj->EOS = 0;

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
    if(status == VX_FAILURE)
    {
        printf("app_tidl: ERROR: Unable to create task\n");
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
                    if (1 == obj->encode) {status = tivx_utils_graph_perf_print(obj->capture_graph);}
                    if (1 == obj->decode) {status = tivx_utils_graph_perf_print(obj->display_graph);}
                    appPerfPointPrint(&obj->fileio_perf);
                    appPerfPointPrint(&obj->total_perf);
                    printf("\n");
                    appPerfPointPrintFPS(&obj->total_perf);
                    appPerfPointReset(&obj->total_perf);
                    printf("\n");
                    appCodecPrintStats();
                    printf("\n");

                    if (1 == obj->encode)
                    {
                        vx_reference refs[1];
                        refs[0] = (vx_reference)obj->captureObj.raw_image_arr[0];
                        if (status == VX_SUCCESS)
                        {
                            status = tivxNodeSendCommand(obj->captureObj.node, 0u,
                                        TIVX_CAPTURE_PRINT_STATISTICS,
                                        refs, 1u);
                        }
                    }
                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "basic_demos_app_multi_cam_codec");
                    if (NULL != fp)
                    {
                        appPerfStatsExportAll(fp, perf_arr, 1);
                        if (status == VX_SUCCESS)
                        {
                            status = tivx_utils_graph_perf_export(fp, obj->capture_graph);
                        }
                        if (status == VX_SUCCESS)
                        {
                            status = tivx_utils_graph_perf_export(fp, obj->display_graph);
                        }
                        appPerfStatsExportCloseFile(fp);
                        appPerfStatsResetAll();
                    }
                    else
                    {
                        printf("fp is null\n");
                    }
                    break;
                case 's':
                    obj->write_file = 1;
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
    snprintf(obj->vissObj.output_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->ldcObj.output_file_path,APP_MAX_FILE_PATH, ".");

    obj->captureObj.en_out_capture_write = 0;
    obj->vissObj.en_out_viss_write = 0;
    obj->ldcObj.en_out_ldc_write = 0;

    obj->num_frames_to_write = 0;
    obj->num_frames_to_skip = 0;
}

static void app_parse_cfg_file(AppObj *obj, vx_char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    vx_char line_str[1024];
    vx_char *token;

    if(fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
        exit(1);
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
            if(strcmp(token, "sensor_index")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->sensorObj.sensor_index = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_frames_to_run")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->num_frames_to_run = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_error_detection")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->captureObj.enable_error_detection = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_configure_hwa_freq")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_configure_hwa_freq = atoi(token);
                }
            }
            else
            if(strcmp(token, "hwa_freq_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->hwa_freq_config = atoi(token);
                }
            }
            else
            if(strcmp(token, "en_out_img_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->en_out_img_write = atoi(token);
                    if(obj->en_out_img_write > 1)
                        obj->en_out_img_write = 1;
                }
            }
            else
            if(strcmp(token, "en_out_capture_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->captureObj.en_out_capture_write = atoi(token);
                    if(obj->captureObj.en_out_capture_write > 1)
                        obj->captureObj.en_out_capture_write = 1;
                }
            }
            else
            if(strcmp(token, "en_out_viss_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->vissObj.en_out_viss_write = atoi(token);
                    if(obj->vissObj.en_out_viss_write > 1)
                        obj->vissObj.en_out_viss_write = 1;
                }
            }
            else
            if(strcmp(token, "en_out_ldc_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->ldcObj.en_out_ldc_write = atoi(token);
                    if(obj->ldcObj.en_out_ldc_write > 1)
                        obj->ldcObj.en_out_ldc_write = 1;
                }
            }
            else
            if(strcmp(token, "output_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->captureObj.output_file_path, token);
                    strcpy(obj->vissObj.output_file_path, token);
                    strcpy(obj->ldcObj.output_file_path, token);
                    strcpy(obj->output_file_path, token);
                }
            }
            else
            if(strcmp(token, "display_option")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->displayObj.display_option = atoi(token);
                    if(obj->displayObj.display_option > 1)
                        obj->displayObj.display_option = 1;
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

            /*
                num_cameras_enabled from cfg file is ignored
                Instead channel_mask is read. num_cameras_enabled = number of 1 bits in mask
                For e.g. channel_mask = 15 (0x0F) indicates that first 4 cameras are enabled
            */
            if(strcmp(token, "channel_mask")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->sensorObj.ch_mask = atoi(token);
                }
            }
            /* Add option to enable dowscaling to 720p if num_channels>1 */
            else
            if(strcmp(token, "en_downscale")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->downscale = atoi(token);
                    if(obj->downscale > 1)
                    {
                        obj->downscale = 1;
                    }
                }
            }
            else
            if(strcmp(token, "en_encode")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->encode = atoi(token);
                    if(obj->encode > 1)
                    {
                        obj->encode = 1;
                    }
                }
            }
            else
            if(strcmp(token, "en_decode")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->decode = atoi(token);
                    if(obj->decode > 1)
                    {
                        obj->decode = 1;
                    }
                }
            }
            else
            if(strcmp(token, "usecase_option")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->sensorObj.usecase_option = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_frames_to_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->num_frames_to_write = atoi(token);
                }
            }
            else
            if(strcmp(token, "num_frames_to_skip")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->num_frames_to_skip = atoi(token);
                }
            }
            else
            if(strcmp(token, "test_mode")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->test_mode = atoi(token);
                    obj->captureObj.test_mode = atoi(token);
                }
            }
        }
    }

    fclose(fp);

}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;
    vx_int16 num_test_cams = 0xFF, sensor_override = 0xFF;
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
            /* check to see if there is another argument following --test */
            if (argc > i+1)
            {
                num_test_cams = atoi(argv[i+1]);
                /* increment i again to avoid this arg */
                i++;
            }
        }
        else
        if(strcmp(argv[i], "--sensor")==0)
        {
            /* check to see if there is another argument following --sensor */
            if (argc > i+1)
            {
                sensor_override = atoi(argv[i+1]);
                /* increment i again to avoid this arg */
                i++;
            }
        }
    }

    if (set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->captureObj.test_mode = 1;
        obj->is_interactive = 0;
        obj->enable_configure_hwa_freq = 0;
        obj->hwa_freq_config = 0;
        obj->sensorObj.is_interactive = 0;
        /* set the number of test cams from cmd line */
        if (num_test_cams != 0xFF)
        {
            obj->sensorObj.num_cameras_enabled = num_test_cams;
        }
        if (sensor_override != 0xFF)
        {
            obj->sensorObj.sensor_index = sensor_override;
        }
    }
#if defined(SOC_AM62A) && defined(QNX)
    if ( obj->is_interactive )
    {
        obj->sensorObj.is_interactive = 1;
    }
#endif

    return;
}

vx_int32 app_multi_cam_codec_main(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;

    AppObj *obj = &gAppObj;

    /*Optional parameter setting*/
    app_default_param_set(obj);

    /*Config parameter reading*/
    app_parse_cmd_line_args(obj, argc, argv);

    /* Querry sensor parameters */
    status = app_querry_sensor(&obj->sensorObj);
    if(1 == obj->sensorObj.sensor_out_format)
    {
        printf("YUV Input selected. VISS, AEWB and Mosaic nodes will be bypassed. \n");
        obj->enable_viss = 0;
        obj->enable_aewb = 0;
        obj->enable_mosaic = 0;
    }
    else
    {
        obj->enable_viss = 1;
        obj->enable_aewb = 1;
        obj->enable_mosaic = 1;
    }

    /* Querry App params : encode, decode, num_channels */
    if ( obj->is_interactive )
    {
        app_querry_param_set(obj);
    }

    /*Update of parameters read from config files or from user input*/
    app_update_param_set(obj);

    if (status == VX_SUCCESS)
    {
        status = app_init(obj);
    }
    if(status == VX_SUCCESS)
    {
        APP_PRINTF("App Init Done! \n");

        status = app_create_graph(obj);

        if(status == VX_SUCCESS)
        {
            APP_PRINTF("App Create Graph Done! \n");

            status = app_verify_graph(obj);

            if(status == VX_SUCCESS)
            {
                APP_PRINTF("App Verify Graph Done! \n");

                if (status == VX_SUCCESS)
                {
                    APP_PRINTF("App Send Error Frame Done! \n");
                    if(obj->is_interactive)
                    {
                        status = app_run_graph_interactive(obj);
                    }
                    else
                    {
                        status = app_run_graph(obj);
                    }
                }
            }
        }

        APP_PRINTF("App Run Graph Done! \n");
    }

    app_delete_graph(obj);

    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);

    APP_PRINTF("App De-init Done! \n");

    if(obj->test_mode == 1)
    {
        if((vx_false_e == test_result) || (status != VX_SUCCESS))
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

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    if (1U == obj->enable_configure_hwa_freq)
    {
        APP_PRINTF("Configuring VPAC frequency!\n");
        if (0U == obj->hwa_freq_config)
        {
            APP_PRINTF("Configuring VPAC frequency to 650 MHz and DMPAC to 520 MHz!\n");
            status = appVhwaConfigureFreq(APP_HWA_CONFIGURE_FREQ_VPAC_650_DMPAC_520);
            APP_PRINTF("Configuring VPAC frequency done!\n");
        }
        else if (1U == obj->hwa_freq_config)
        {
            APP_PRINTF("Configuring VPAC frequency to 720 MHz and DMPAC to 480 MHz!\n");
            status = appVhwaConfigureFreq(APP_HWA_CONFIGURE_FREQ_VPAC_720_DMPAC_480);
            APP_PRINTF("Configuring VPAC frequency done!\n");
        }
        else
        {
            APP_PRINTF("Invalid option for VPAC frequency configuration!\n");
        }
    }

    if (status == VX_SUCCESS)
    {
        /* Create OpenVx Context */
        obj->context = vxCreateContext();
        status = vxGetStatus((vx_reference)obj->context);
        APP_PRINTF("Creating context done!\n");
    }

    if (status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxImagingLoadKernels(obj->context);
        tivxFileIOLoadKernels(obj->context);
        APP_PRINTF("Kernel loading done!\n");
    }

    /* Initialize modules */
    if ((1 == obj->encode) && (status == VX_SUCCESS))
    {
        app_init_sensor(&obj->sensorObj, "sensor_obj");
    }

    if ((1 == obj->encode) && (status == VX_SUCCESS))
    {
        APP_PRINTF("Sensor init done!\n");
        status = app_init_capture(obj->context, &obj->captureObj, &obj->sensorObj, "capture_obj", APP_ENC_BUFFER_Q_DEPTH);
    }

    if((1 == obj->encode) && (1 == obj->enable_viss) && (status == VX_SUCCESS))
    {
        status = app_init_viss(obj->context, &obj->vissObj, &obj->sensorObj, "viss_obj", obj->num_ch);
        APP_PRINTF("VISS init done!\n");
    }

    if((1 == obj->encode) && (1 == obj->enable_aewb) && (status == VX_SUCCESS))
    {
        status = app_init_aewb(obj->context, &obj->aewbObj, &obj->sensorObj, "aewb_obj", 0, obj->num_ch);
        APP_PRINTF("AEWB init done!\n");
    }

    if ((1 == obj->encode) && (status == VX_SUCCESS))
    {
        status = app_init_ldc(obj->context, &obj->ldcObj, &obj->sensorObj, "ldc_obj");
        APP_PRINTF("LDC init done!\n");
    }

    if((1 == obj->encode) && (obj->downscale == 1) && (status == VX_SUCCESS))
    {
        vx_image ldc_out_img = vxCreateImage(obj->context, obj->ldcObj.table_width, obj->ldcObj.table_height, VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)ldc_out_img);
        vx_int32 q;
        if(status == VX_SUCCESS)
        {
            for(q = 0; q < APP_ENC_BUFFER_Q_DEPTH; q++)
            {
                obj->ldc_out_arr_q[q] = vxCreateObjectArray(obj->context, (vx_reference)ldc_out_img, obj->num_ch);
                status = vxGetStatus((vx_reference)obj->ldc_out_arr_q[q]);
                if(status != VX_SUCCESS)
                {
                    printf("[APP_INIT]: Unable to create image object array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "ldc_out_arr_q_%d", q);

                    vxSetReferenceName((vx_reference)obj->ldc_out_arr_q[q], name);
                }
            }
            vxReleaseImage(&ldc_out_img);
        }
        else
        {
            printf("[APP_INIT]: Unable to create ldc_out_img\n");
        }
    }

    if((1 == obj->encode) && (obj->downscale == 1) && (status == VX_SUCCESS))
    {
        status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", obj->num_ch, 1);
        APP_PRINTF("Scaler init done!\n");
    }

    if((1 == obj->encode) && (status == VX_SUCCESS))
    {
        vx_image intermediate_img = vxCreateImage(obj->context, obj->enc_pool.width, obj->enc_pool.height, VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)intermediate_img);
        vx_int32 q;
        if(status == VX_SUCCESS)
        {
            for(q = 0; q < obj->enc_pool.bufq_depth; q++)
            {
                obj->enc_pool.arr[q] = vxCreateObjectArray(obj->context, (vx_reference)intermediate_img, obj->num_ch);
                obj->enc_pool.img[q] = (vx_image)vxGetObjectArrayItem(obj->enc_pool.arr[q], 0);
                status = vxGetStatus((vx_reference)obj->enc_pool.arr[q]);
                if(status != VX_SUCCESS)
                {
                    printf("[APP_INIT]: Unable to create image object array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "enc_pool.arr_%d", q);

                    vxSetReferenceName((vx_reference)obj->enc_pool.arr[q], name);
                }
            }
            vxReleaseImage(&intermediate_img);
        }
        else
        {
            printf("[APP_INIT]: Unable to create intermediate_img\n");
        }
    }

    if((1 == obj->decode) && (status == VX_SUCCESS))
    {
        vx_image intermediate_img = vxCreateImage(obj->context, obj->dec_pool.width, obj->dec_pool.height, VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)intermediate_img);
        vx_int32 q;
        if(status == VX_SUCCESS)
        {
            for(q = 0; q < obj->dec_pool.bufq_depth; q++)
            {
                obj->dec_pool.arr[q] = vxCreateObjectArray(obj->context, (vx_reference)intermediate_img, obj->num_ch);
                status = vxGetStatus((vx_reference)obj->dec_pool.arr[q]);
                if(status != VX_SUCCESS)
                {
                    printf("[APP_INIT]: Unable to create image object array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "dec_pool.arr_%d", q);

                    vxSetReferenceName((vx_reference)obj->dec_pool.arr[q], name);
                }
            }
            vxReleaseImage(&intermediate_img);
        }
        else
        {
            printf("[APP_INIT]: Unable to create intermediate_img\n");
        }
    }

    if((1 == obj->decode) && (obj->enable_mosaic == 1) && (status == VX_SUCCESS))
    {
        status = tiovx_img_mosaic_module_init(obj->context, &obj->imgMosaicObj);
        APP_PRINTF("Img Mosaic init done!\n");
    }

    if((1 == obj->decode) && (status == VX_SUCCESS))
    {
        status = app_init_display(obj->context, &obj->displayObj, "display_obj");
        APP_PRINTF("Display init done!\n");
    }

    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");
    return status;
}

static void app_deinit(AppObj *obj)
{
    if (1 == obj->encode)
    {
        app_deinit_sensor(&obj->sensorObj);
        APP_PRINTF("Sensor deinit done!\n");

        app_deinit_capture(&obj->captureObj, APP_ENC_BUFFER_Q_DEPTH);
        APP_PRINTF("Capture deinit done!\n");

        if(1 == obj->enable_viss)
        {
            app_deinit_viss(&obj->vissObj);
            APP_PRINTF("VISS deinit done!\n");
        }

        if(1 == obj->enable_aewb)
        {
            app_deinit_aewb(&obj->aewbObj);
            APP_PRINTF("AEWB deinit done!\n");
        }

        app_deinit_ldc(&obj->ldcObj);
        APP_PRINTF("LDC deinit done!\n");

        if (obj->downscale == 1)
        {
            for(vx_int32 i = 0; i < APP_ENC_BUFFER_Q_DEPTH; i++)
            {
                vxReleaseObjectArray(&obj->ldc_out_arr_q[i]);
            }

            app_deinit_scaler(&obj->scalerObj);
            APP_PRINTF("Scaler deinit done!\n");
        }

        for(vx_int32 i = 0; i < obj->enc_pool.bufq_depth; i++)
        {
            vxReleaseObjectArray(&obj->enc_pool.arr[i]);
            vxReleaseImage(&obj->enc_pool.img[i]);
        }
    }

    if (1 == obj->decode)
    {
        for(vx_int32 i = 0; i < obj->dec_pool.bufq_depth; i++)
        {
            vxReleaseObjectArray(&obj->dec_pool.arr[i]);
        }

        if(obj->enable_mosaic == 1)
        {
            tiovx_img_mosaic_module_deinit(&obj->imgMosaicObj);
            APP_PRINTF("Img Mosaic deinit done!\n");
        }

        app_deinit_display(&obj->displayObj);
        APP_PRINTF("Display deinit done!\n");
    }

    tivxHwaUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxImagingUnLoadKernels(obj->context);
    tivxFileIOUnLoadKernels(obj->context);
    APP_PRINTF("Kernels unload done!\n");

    vxReleaseContext(&obj->context);
    APP_PRINTF("Release context done!\n");
}

static void app_delete_graph(AppObj *obj)
{
    if (1 == obj->encode)
    {
        app_delete_capture(&obj->captureObj);
        APP_PRINTF("Capture delete done!\n");

        app_delete_viss(&obj->vissObj);
        APP_PRINTF("VISS delete done!\n");

        app_delete_aewb(&obj->aewbObj);
        APP_PRINTF("AEWB delete done!\n");

        app_delete_ldc(&obj->ldcObj);
        APP_PRINTF("LDC delete done!\n");

        app_delete_scaler(&obj->scalerObj);
        APP_PRINTF("Scaler delete done!\n");
    }

    if (1 == obj->decode)
    {
        tiovx_img_mosaic_module_delete(&obj->imgMosaicObj);
        APP_PRINTF("Img Mosaic delete done!\n");

        app_delete_display(&obj->displayObj);
        APP_PRINTF("Display delete done!\n");
    }

    if (1 == obj->encode) {vxReleaseGraph(&obj->capture_graph);}
    if (1 == obj->decode) {vxReleaseGraph(&obj->display_graph);}
    APP_PRINTF("Graph delete done!\n");

    if ( obj->encode || obj->decode )
    {
        appCodecDeInit();
        APP_PRINTF("Codec Pipeline delete done!\n");
    }
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    if (1 == obj->encode)
    {
        obj->capture_graph = vxCreateGraph(obj->context);
        status = vxGetStatus((vx_reference)obj->capture_graph);
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName((vx_reference)obj->capture_graph, "capture_graph");
            APP_PRINTF("capture_graph create done!\n");
        }
    }

    if (1 == obj->decode)
    {
        if (status == VX_SUCCESS)
        {
            obj->display_graph = vxCreateGraph(obj->context);
            status = vxGetStatus((vx_reference)obj->display_graph);
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName((vx_reference)obj->display_graph, "display_graph");
            APP_PRINTF("display_graph create done!\n");
        }
    }

    if (1 == obj->encode)
    {
        if(status == VX_SUCCESS)
        {
            status = app_create_graph_capture(obj->capture_graph, &obj->captureObj);
            APP_PRINTF("Capture graph done!\n");
        }

        if(1 == obj->enable_viss)
        {
            if(status == VX_SUCCESS)
            {
                status = app_create_graph_viss(obj->capture_graph, &obj->vissObj, obj->captureObj.raw_image_arr[0], TIVX_TARGET_VPAC_VISS1);
                APP_PRINTF("VISS graph done!\n");
            }
        }

        if(1 == obj->enable_aewb)
        {
            if(status == VX_SUCCESS)
            {
                status = app_create_graph_aewb(obj->capture_graph, &obj->aewbObj, obj->vissObj.h3a_stats_arr);

                APP_PRINTF("AEWB graph done!\n");
            }
        }

        vx_object_array ldc_in_arr;
        if(1 == obj->enable_viss)
        {
            ldc_in_arr = obj->vissObj.output_arr;
        }
        else
        {
            ldc_in_arr = obj->captureObj.raw_image_arr[0];
        }
        if (1 == obj->downscale)
        {
            obj->ldcObj.output_arr = obj->ldc_out_arr_q[0];
        }
        else
        {
            obj->ldcObj.output_arr = obj->enc_pool.arr[0];
        }
        if (status == VX_SUCCESS)
        {
            status = app_create_graph_ldc(obj->capture_graph, &obj->ldcObj, ldc_in_arr);
            APP_PRINTF("LDC graph done!\n");
        }

        if (1 == obj->downscale && status == VX_SUCCESS)
        {
            obj->scalerObj.output[0].width = obj->enc_pool.width;
            obj->scalerObj.output[0].height = obj->enc_pool.height;
            obj->scalerObj.output[0].arr = obj->enc_pool.arr[0];
            status = app_create_graph_scaler(obj->context, obj->capture_graph, &obj->scalerObj, obj->ldc_out_arr_q[0]);
            APP_PRINTF("Scaler graph done!\n");
        }
    }

    if (1 == obj->decode)
    {
        vx_image display_in_image;
        if(obj->enable_mosaic == 1)
        {
            vx_object_array mosaic_input_arr[1];
            mosaic_input_arr[0] = obj->dec_pool.arr[0];

            if(status == VX_SUCCESS)
            {
                status = tiovx_img_mosaic_module_create(obj->display_graph, &obj->imgMosaicObj, NULL, mosaic_input_arr, TIVX_TARGET_VPAC_MSC1);
                APP_PRINTF("Img Mosaic graph done!\n");
            }
            display_in_image = obj->imgMosaicObj.output_image[0];
        }
        else
        {
            display_in_image = (vx_image)vxGetObjectArrayItem(obj->captureObj.raw_image_arr[0], 0);
        }

        if(status == VX_SUCCESS)
        {
            status = app_create_graph_display(obj->display_graph, &obj->displayObj, display_in_image);
            APP_PRINTF("Display graph done!\n");
        }
    }

    if(status == VX_SUCCESS)
    {
        if (1 == obj->encode)
        {
            vx_graph_parameter_queue_params_t capt_graph_parameters_queue_params_list[2] = {0};
            vx_int32 capt_graph_parameter_index = 0;
            add_graph_parameter_by_node_index(obj->capture_graph, obj->captureObj.node, 1);
            obj->captureObj.graph_parameter_index = capt_graph_parameter_index;
            capt_graph_parameters_queue_params_list[capt_graph_parameter_index].graph_parameter_index = capt_graph_parameter_index;
            capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list_size = APP_ENC_BUFFER_Q_DEPTH;
            capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list = (vx_reference*)&obj->captureObj.raw_image_arr[0];
            capt_graph_parameter_index++;

            if (1 == obj->downscale)
            {
                add_graph_parameter_by_node_index(obj->capture_graph, obj->scalerObj.node, 1);
                obj->scalerObj.graph_parameter_index = capt_graph_parameter_index;
                obj->enc_pool.graph_parameter_index = capt_graph_parameter_index;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].graph_parameter_index = capt_graph_parameter_index;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list_size = obj->enc_pool.bufq_depth;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list = (vx_reference*)&obj->enc_pool.img[0];
                capt_graph_parameter_index++;
            }
            else
            {
                add_graph_parameter_by_node_index(obj->capture_graph, obj->ldcObj.node, 7);
                obj->ldcObj.graph_parameter_index = capt_graph_parameter_index;
                obj->enc_pool.graph_parameter_index = capt_graph_parameter_index;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].graph_parameter_index = capt_graph_parameter_index;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list_size = obj->enc_pool.bufq_depth;
                capt_graph_parameters_queue_params_list[capt_graph_parameter_index].refs_list = (vx_reference*)&obj->enc_pool.img[0];
                capt_graph_parameter_index++;
            }

            status = vxSetGraphScheduleConfig(obj->capture_graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                    capt_graph_parameter_index,
                    capt_graph_parameters_queue_params_list);
        }

        if (1 == obj->decode)
        {
            vx_graph_parameter_queue_params_t disp_graph_parameters_queue_params_list[2] = {0};
            vx_int32 disp_graph_parameter_index = 0;
            add_graph_parameter_by_node_index(obj->display_graph, obj->imgMosaicObj.node, 3);
            obj->imgMosaicObj.inputs[0].graph_parameter_index = disp_graph_parameter_index;
            obj->dec_pool.graph_parameter_index = disp_graph_parameter_index;
            disp_graph_parameters_queue_params_list[disp_graph_parameter_index].graph_parameter_index = disp_graph_parameter_index;
            disp_graph_parameters_queue_params_list[disp_graph_parameter_index].refs_list_size = obj->dec_pool.bufq_depth;
            disp_graph_parameters_queue_params_list[disp_graph_parameter_index].refs_list = (vx_reference*)&obj->dec_pool.arr[0];
            disp_graph_parameter_index++;

            if((obj->en_out_img_write == 1) || (obj->test_mode == 1))
            {
                add_graph_parameter_by_node_index(obj->display_graph, obj->imgMosaicObj.node, 1);
                obj->imgMosaicObj.output_graph_parameter_index = disp_graph_parameter_index;
                disp_graph_parameters_queue_params_list[disp_graph_parameter_index].graph_parameter_index = disp_graph_parameter_index;
                disp_graph_parameters_queue_params_list[disp_graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
                disp_graph_parameters_queue_params_list[disp_graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.output_image[0];
                disp_graph_parameter_index++;
            }

            status = vxSetGraphScheduleConfig(obj->display_graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                    disp_graph_parameter_index,
                    disp_graph_parameters_queue_params_list);
        }

        if ((1 == obj->encode) && (status == VX_SUCCESS))
        {
            status = tivxSetGraphPipelineDepth(obj->capture_graph, CAPTURE_PIPELINE_DEPTH);
        }
        if ((1 == obj->decode) && (status == VX_SUCCESS))
        {
            status = tivxSetGraphPipelineDepth(obj->display_graph, DISPLAY_PIPELINE_DEPTH);
        }
        if((1 == obj->encode) && (obj->enable_viss == 1) && (status == VX_SUCCESS))
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 6, APP_ENC_BUFFER_Q_DEPTH);

            if (status == VX_SUCCESS)
            {
                status = tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 9, APP_ENC_BUFFER_Q_DEPTH);
            }
        }
        if((1 == obj->encode) && (obj->enable_aewb == 1) && (status == VX_SUCCESS))
        {
            if (status == VX_SUCCESS)
            {
                status = tivxSetNodeParameterNumBufByIndex(obj->aewbObj.node, 4, APP_ENC_BUFFER_Q_DEPTH);
            }
        }
        if((1 == obj->encode) && (obj->downscale == 1) && (status == VX_SUCCESS))
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->ldcObj.node, 7, APP_ENC_BUFFER_Q_DEPTH);
        }
        if((1 == obj->decode) && (obj->enable_mosaic == 1) && (status == VX_SUCCESS))
        {
            if(!((obj->en_out_img_write == 1) || (obj->test_mode == 1)))
            {
                status = tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, APP_BUFFER_Q_DEPTH);
                APP_PRINTF("Pipeline params setup done!\n");
            }
        }
    }

    obj->codec_pipe_params.appEncode = obj->encode;
    obj->codec_pipe_params.appDecode = obj->decode;

    if ( obj->encode || obj->decode )
    {
        if(status == VX_SUCCESS)
        {
            status = appCodecInit(&obj->codec_pipe_params);
            APP_PRINTF("Codec Pipeline done!\n");
        }
    }

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    if ( obj->encode )
    {
        status = vxVerifyGraph(obj->capture_graph);
    }

    if(status == VX_SUCCESS)
    {
        if ( obj->encode )
        {
            APP_PRINTF("Capture Graph verify done!\n");
        }
        if ( obj->decode )
        {
            status = vxVerifyGraph(obj->display_graph);
        }
    }
    if(status == VX_SUCCESS)
    {
        if ( obj->decode )
        {
            APP_PRINTF("Display Graph verify done!\n");
        }
    }

    if(VX_SUCCESS == status)
    {
        if ( obj->encode )
        {
            status = tivxExportGraphToDot(obj->capture_graph,".", "vx_app_multi_cam_capture");
        }
    }
    if(VX_SUCCESS == status)
    {
        if ( obj->decode )
        {
            status = tivxExportGraphToDot(obj->display_graph,".", "vx_app_multi_cam_display");
        }
    }

    if (((obj->captureObj.enable_error_detection) || (obj->test_mode)) && (status == VX_SUCCESS))
    {
        if ( obj->encode )
        {
            status = app_send_error_frame(&obj->captureObj);
        }
    }

    if ( obj->encode )
    {
        for (vx_int8 buf_id=0; buf_id<obj->enc_pool.bufq_depth; buf_id++)
        {
            if(VX_SUCCESS == status)
            {
                status = map_vx_object_arr(obj->enc_pool.arr[buf_id], obj->enc_pool.data_ptr[buf_id], obj->enc_pool.map_id[buf_id], obj->enc_pool.num_channels);
            }
        }

        if(VX_SUCCESS == status)
        {
            status = appCodecSrcInit(obj->enc_pool.data_ptr);
            if(VX_SUCCESS == status)
            {
                APP_PRINTF("\nappCodecSrcInit Done!\n");
            }
            else
            {
                APP_ERROR("\nappCodecSrcInit Failed!\n");
            }
        }

        for (vx_int8 buf_id=0; buf_id<obj->enc_pool.bufq_depth; buf_id++)
        {
            if(VX_SUCCESS == status)
            {
                status = unmap_vx_object_arr(obj->enc_pool.arr[buf_id], obj->enc_pool.map_id[buf_id], obj->enc_pool.num_channels);
            }
        }
    }

    if ( obj->decode )
    {
        for (vx_int8 buf_id=0; buf_id<obj->dec_pool.bufq_depth; buf_id++)
        {
            if(VX_SUCCESS == status)
            {
#if defined(QNX)
                status = map_vx_object_arr(obj->dec_pool.arr[buf_id], obj->dec_pool.data_ptr[buf_id], obj->dec_pool.map_id[buf_id], obj->dec_pool.num_channels);
#endif
#if defined(LINUX)
                status = delete_array_image_buffers(obj->dec_pool.arr[buf_id]);
#endif
            }
        }

        if(VX_SUCCESS == status)
        {
            status = appCodecSinkInit(obj->dec_pool.data_ptr);
            if(VX_SUCCESS == status)
            {
                APP_PRINTF("\nappCodecSinkInit Done!\n");
            }
            else
            {
                APP_ERROR("\nappCodecSinkInit Failed!\n");
            }
        }
    #if defined(QNX)
        for (vx_int8 buf_id=0; buf_id<obj->dec_pool.bufq_depth; buf_id++)
        {
            if(VX_SUCCESS == status)
            {
                status = unmap_vx_object_arr(obj->dec_pool.arr[buf_id], obj->dec_pool.map_id[buf_id], obj->dec_pool.num_channels);
            }
        }
    #endif
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

#if defined(DEC_WRITE_TO_FILE)
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id, FILE *fp)
#else
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id)
#endif
{
    vx_status status = VX_SUCCESS;

    appPerfPointBegin(&obj->total_perf);

        /* enc_pool buffer recycling */
        if (status==VX_SUCCESS && (obj->encode==1))
        {
            status = capture_encode(obj, frame_id);
        }

    #if defined(QNX)
        /* dec_pool buffer recycling */
        if (status==VX_SUCCESS && (obj->decode==1) && frame_id>=obj->dec_pool.bufq_depth)
        {
        #if defined(DEC_WRITE_TO_FILE)
            status = decode_display(obj,frame_id-obj->dec_pool.bufq_depth, fp);
        #else
            status = decode_display(obj,frame_id-obj->dec_pool.bufq_depth);
        #endif
        }
    #else
        /* dec_pool buffer recycling */
        if (status==VX_SUCCESS && (obj->decode==1) && frame_id>=obj->enc_pool.bufq_depth)
        {
        #if defined(DEC_WRITE_TO_FILE)
            status = decode_display(obj,frame_id-obj->enc_pool.bufq_depth, fp);
        #else
            status = decode_display(obj,frame_id-obj->enc_pool.bufq_depth);
        #endif
        }
    #endif

    appPerfPointEnd(&obj->total_perf);
    return status;
}


static vx_status capture_encode(AppObj* obj, vx_int32 frame_id)
{
    APP_PRINTF("\ncapture_encode: frame %d beginning\n", frame_id);
    vx_status status = VX_SUCCESS;

    CaptureObj *captureObj = &obj->captureObj;
    AppGraphParamRefPool *enc_pool = &obj->enc_pool;

    vx_object_array capture_input_arr;
    vx_reference ldc_output_arr;
    uint32_t num_refs;

    if ( frame_id >= APP_ENC_BUFFER_Q_DEPTH )
    {
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->capture_graph, captureObj->graph_parameter_index, (vx_reference*)&capture_input_arr, 1, &num_refs);
        }
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->capture_graph, enc_pool->graph_parameter_index, (vx_reference*)&ldc_output_arr, 1, &num_refs);
        }

        if ( frame_id >= enc_pool->bufq_depth )
        {
            if (status == VX_SUCCESS && obj->encode==1)
            {
                status = appCodecDeqAppSrc(obj->ldc_enq_id);
            }
            if(status==VX_SUCCESS)
            {
                status = unmap_vx_object_arr(enc_pool->arr[obj->ldc_enq_id], enc_pool->map_id[obj->ldc_enq_id], obj->num_ch);
            }
        }

        if(status==VX_SUCCESS)
        {
            status = map_vx_object_arr(enc_pool->arr[obj->appsrc_push_id], enc_pool->data_ptr[obj->appsrc_push_id], enc_pool->map_id[obj->appsrc_push_id], obj->num_ch);
        }
        if(status==VX_SUCCESS && obj->encode==1)
        {
            status = appCodecEnqAppSrc(obj->appsrc_push_id);
        }
        obj->appsrc_push_id++;
        obj->appsrc_push_id     = (obj->appsrc_push_id  >= enc_pool->bufq_depth)? 0 : obj->appsrc_push_id;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->capture_graph, captureObj->graph_parameter_index, (vx_reference*)&captureObj->raw_image_arr[obj->capture_id], 1);
    }
    if (status == VX_SUCCESS)
    {
        status = vxGraphParameterEnqueueReadyRef(obj->capture_graph, enc_pool->graph_parameter_index, (vx_reference*)&enc_pool->img[obj->ldc_enq_id], 1);
    }
    obj->ldc_enq_id++;
    obj->ldc_enq_id         = (obj->ldc_enq_id  >= enc_pool->bufq_depth)? 0 : obj->ldc_enq_id;
    obj->capture_id++;
    obj->capture_id         = (obj->capture_id  >= APP_ENC_BUFFER_Q_DEPTH)? 0 : obj->capture_id;

    return status;
}

#if defined(DEC_WRITE_TO_FILE)
static vx_status decode_display(AppObj* obj, vx_int32 frame_id, FILE *fp)
#else
static vx_status decode_display(AppObj* obj, vx_int32 frame_id)
#endif
{
    APP_PRINTF("\ndecode_display: frame %d beginning\n", frame_id);
    vx_status status = VX_SUCCESS;

    TIOVXImgMosaicModuleObj *imgMosaicObj = &obj->imgMosaicObj;
    AppGraphParamRefPool *dec_pool = &obj->dec_pool;

    /* checksum_actual is the checksum determined by the realtime test
        checksum_expected is the checksum that is expected to be the pipeline output */
    uint32_t checksum_actual = 0;

    /* This is the number of frames required for the pipeline AWB and AE algorithms to stabilize
        (note that 15 is only required for the 6-8 camera use cases - others converge quicker) */
    uint8_t stability_frame = 15;

    vx_object_array mosaic_input_arr;
    vx_image mosaic_output_image;
    uint32_t num_refs;

    int8_t pull_status = -2;


    if ( frame_id >= dec_pool->bufq_depth )
    {
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->display_graph, imgMosaicObj->inputs[0].graph_parameter_index, (vx_reference*)&mosaic_input_arr, 1, &num_refs);
            if (obj->decode==1)
            {
                appCodecEnqAppSink((obj->appsink_pull_id + 1) % obj->num_codec_bufs);
            }
        }
        if (status == VX_SUCCESS && obj->decode==1)
        {
            pull_status = appCodecDeqAppSink(obj->appsink_pull_id);
            if (pull_status == 1)
            {
                obj->EOS=1;
                obj->stop_task=1;
                APP_PRINTF("\nCODEC=> EOS Received\n");
                goto exit;
            }
            else if (pull_status != 0)
            {
                goto exit;
            }
        }
        if((obj->en_out_img_write == 1) || (obj->test_mode == 1))
        {
            vx_char output_file_name[APP_MAX_FILE_PATH];

            /* Dequeue output */
            if (status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(obj->display_graph, imgMosaicObj->output_graph_parameter_index, (vx_reference*)&mosaic_output_image, 1, &num_refs);
            }
            if ((status == VX_SUCCESS) && (obj->test_mode == 1) && (frame_id > TEST_BUFFER))
            {
                /* calculate the checksum of the mosaic output */

                if ((app_test_check_image(mosaic_output_image, checksums_expected[obj->sensorObj.sensor_index][obj->num_ch-1],
                                        &checksum_actual) != vx_true_e) && (frame_id > stability_frame))
                {
                    test_result = vx_false_e;
                    /* in case test fails and needs to change */
                    populate_gatherer(obj->sensorObj.sensor_index, obj->num_ch-1, checksum_actual);
                }
            }

            if (obj->en_out_img_write == 1) {
                appPerfPointBegin(&obj->fileio_perf);
                snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_%dx%d.yuv", obj->output_file_path, (frame_id - APP_BUFFER_Q_DEPTH), imgMosaicObj->out_width, imgMosaicObj->out_height);
                if (status == VX_SUCCESS)
                {
                    /* TODO: Correct checksums not added yet.
                     * status = writeMosaicOutput(output_file_name, mosaic_output_image);
                     */
                }
                appPerfPointEnd(&obj->fileio_perf);
            }
        }

        if(status==VX_SUCCESS && obj->decode==1)
        {
            status = assign_array_image_buffers(
                            mosaic_input_arr,
                            dec_pool->data_ptr[obj->appsink_pull_id],
                            dec_pool->plane_sizes);
        }

        if((obj->en_out_img_write == 1) || (obj->test_mode == 1))
        {
            if (status == VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->display_graph, imgMosaicObj->output_graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->display_id], 1);
            }
        }
        if (status == VX_SUCCESS)
        {
        #if defined(DEC_WRITE_TO_FILE)
            write_vx_object_arr((vx_reference*)&mosaic_input_arr, dec_pool->data_ptr[obj->appsink_pull_id], fp);
        #endif
            status = vxGraphParameterEnqueueReadyRef(obj->display_graph, imgMosaicObj->inputs[0].graph_parameter_index, (vx_reference*)&mosaic_input_arr, 1);
        }
    }
    else
    {
        if (status == VX_SUCCESS && obj->decode==1)
        {
            pull_status = appCodecDeqAppSink(obj->appsink_pull_id);
            if (pull_status == 1)
            {
                obj->EOS=1;
                obj->stop_task=1;
                APP_PRINTF("\nCODEC=> EOS Received\n");
                goto exit;
            }
            else if (pull_status != 0)
            {
                goto exit;
            }
        }

        if(status==VX_SUCCESS && obj->decode==1)
        {
            status = assign_array_image_buffers(
                            dec_pool->arr[obj->mosaic_enq_id],
                            dec_pool->data_ptr[obj->appsink_pull_id],
                            dec_pool->plane_sizes);
        }

        if((obj->en_out_img_write == 1) || (obj->test_mode == 1))
        {
            if (status == VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->display_graph, imgMosaicObj->output_graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->display_id], 1);
            }
        }
        if (status == VX_SUCCESS)
        {
        #if defined(DEC_WRITE_TO_FILE)
            write_vx_object_arr(dec_pool->arr[obj->mosaic_enq_id], dec_pool->data_ptr[obj->appsink_pull_id], fp);
        #endif
            status = vxGraphParameterEnqueueReadyRef(obj->display_graph, imgMosaicObj->inputs[0].graph_parameter_index, (vx_reference*)&dec_pool->arr[obj->mosaic_enq_id], 1);
        }
    }

    obj->display_id++;
    obj->display_id         = (obj->display_id  >= APP_BUFFER_Q_DEPTH)? 0 : obj->display_id;
    obj->mosaic_enq_id++;
    obj->mosaic_enq_id      = (obj->mosaic_enq_id  >= dec_pool->bufq_depth)? 0 : obj->mosaic_enq_id;
    obj->appsink_pull_id++;
    obj->appsink_pull_id    = (obj->appsink_pull_id  >= obj->num_codec_bufs)? 0 : obj->appsink_pull_id;

exit:
    return status;
}

#if defined(LINUX)
static vx_status delete_array_image_buffers(vx_object_array arr)
{
    vx_status status = VX_SUCCESS;
    vx_size num_ch, img_size;
    void* data_ptr[CODEC_MAX_NUM_PLANES];
    vx_uint32 sizes[CODEC_MAX_NUM_PLANES], num_planes;

    status = vxQueryObjectArray(arr, VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(num_ch));
    for (vx_uint32 ch = 0; status==VX_SUCCESS && ch<num_ch; ch++)
    {
        vx_image image = (vx_image)vxGetObjectArrayItem(arr, ch);

        if (status == VX_SUCCESS)
        {
            status = vxQueryImage(image, VX_IMAGE_SIZE, &img_size, sizeof(img_size));
        }
        if (status == VX_SUCCESS)
        {
            status = tivxReferenceExportHandle(
                (vx_reference)image,
                data_ptr,
                sizes,
                CODEC_MAX_NUM_PLANES,
                &num_planes);
        }
        if (status == VX_SUCCESS)
        {
            /* Free only the first plane_addr as the remaining ones were
                derrived in allocate_single_image_buffer */
            tivxMemFree(data_ptr[0], img_size, TIVX_MEM_EXTERNAL_SHARED);

            /* Mark the handle as NULL */
            vx_int32 p;
            for(p = 0; p < num_planes; p++)
            {
                data_ptr[p] = NULL;
            }

            /* Assign NULL handles to the OpenVx objects as it will avoid
                doing a tivxMemFree twice, once now and once during release */
            status = tivxReferenceImportHandle((vx_reference)image,
                                                (const void **)data_ptr,
                                                (const uint32_t *)sizes,
                                                num_planes);
        }

        vxReleaseReference((vx_reference*)&image);
    }
    return status;
}
#endif

static vx_status assign_array_image_buffers(vx_object_array arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_uint32 sizes[CODEC_MAX_NUM_PLANES])
{
    vx_status status = VX_SUCCESS;
    vx_size num_ch, num_planes = 1;
    void* empty_data_ptr[CODEC_MAX_NUM_PLANES] = {NULL};

    status = vxQueryObjectArray(arr, VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(num_ch));
    for (vx_uint32 ch = 0; status==VX_SUCCESS && ch<num_ch; ch++)
    {
        vx_image image = (vx_image)vxGetObjectArrayItem(arr, ch);

        if (status == VX_SUCCESS)
        {
            if (data_ptr == NULL)
            {
                status = tivxReferenceImportHandle(
                            (vx_reference)image,
                            (const void **)empty_data_ptr,
                            (const uint32_t *)sizes,
                            num_planes);
            }
            else
            {
                status = tivxReferenceImportHandle(
                            (vx_reference)image,
                            (const void **)data_ptr[ch],
                            (const uint32_t *)sizes,
                            num_planes);
            }
        }

        vxReleaseReference((vx_reference*)&image);
    }
    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = &obj->sensorObj;
    vx_int32 frame_id;
    int32_t ch_mask = obj->sensorObj.ch_mask;

    app_pipeline_params_defaults(obj);
    APP_PRINTF("app_pipeline_params_defaults returned\n");

    if('\0' == sensorObj->sensor_name[0])
    {
        printf("sensor name is NULL \n");
        return VX_FAILURE;
    }

    /* if test_mode is enabled, don't fail the program if the sensor init fails */
    if( obj->encode==1 ) {
        if( (obj->test_mode) || (obj->captureObj.enable_error_detection) )
        {
            appStartImageSensor(sensorObj->sensor_name, ch_mask);
        }
        else
        {
            status = appStartImageSensor(sensorObj->sensor_name, ch_mask);
            APP_PRINTF("appStartImageSensor returned with status: %d\n", status);
        }
    }

    if(0 == obj->enable_viss)
    {
        obj->vissObj.en_out_viss_write = 0;
    }

    if (obj->test_mode == 1) {
        /* The buffer allows AWB/AE algos to converge before checksums are calculated */
        obj->num_frames_to_run = TEST_BUFFER + 30;
    }

    if (status == VX_SUCCESS)
    {
        if ( obj->encode || obj->decode )
        {
            status = appCodecStart();
            APP_PRINTF("appCodecStart Done!\n");
        }
    }

#if defined(DEC_WRITE_TO_FILE)
    FILE *fp = fopen("/tmp/output_video_ovx.yuv", "w");
#endif

    for(frame_id = 0; frame_id < obj->num_frames_to_run; frame_id++)
    {
        if(obj->write_file == 1)
        {
            if((obj->captureObj.en_out_capture_write == 1) && (status == VX_SUCCESS))
            {
                status = app_send_cmd_capture_write_node(&obj->captureObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if((obj->vissObj.en_out_viss_write == 1) && (status == VX_SUCCESS))
            {
                status = app_send_cmd_viss_write_node(&obj->vissObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if((obj->ldcObj.en_out_ldc_write == 1) && (status == VX_SUCCESS))
            {
                status = app_send_cmd_ldc_write_node(&obj->ldcObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            obj->write_file = 0;
        }

        if (status == VX_SUCCESS)
        {
        #if defined(DEC_WRITE_TO_FILE)
            status = app_run_graph_for_one_frame_pipeline(obj, frame_id, fp);
        #else
            status = app_run_graph_for_one_frame_pipeline(obj, frame_id);
        #endif
        }

        /* user asked to stop processing or pulled EoS from CodecPipeline*/
        if(obj->stop_task)
          break;
    }

    for(uint8_t x=0; x<CODEC_ENC_BUFQ_DEPTH; x++){
        if ( obj->encode==1 )
        {
            appCodecDeqAppSrc(obj->ldc_enq_id);
        }
        if ( obj->encode==1 || obj->decode==0 )
        {
            unmap_vx_object_arr(obj->enc_pool.arr[obj->ldc_enq_id], obj->enc_pool.map_id[obj->ldc_enq_id], obj->num_ch);
            obj->ldc_enq_id++;
            obj->ldc_enq_id         = (obj->ldc_enq_id  >= obj->enc_pool.bufq_depth)? 0 : obj->ldc_enq_id;
        }
    }
    if ( obj->encode==1 )
    {
        APP_PRINTF("Pushing EoS to Codec Pipeline.\n");
        status = appCodecEnqEosAppSrc();
    }

    if ( obj->decode==1 )
    {
        for(uint8_t x=0; x<obj->dec_pool.bufq_depth - 1; x++)
        {
            vx_object_array mosaic_input_arr;
            uint32_t num_refs;

            vxGraphParameterDequeueDoneRef(obj->display_graph, obj->imgMosaicObj.inputs[0].graph_parameter_index, (vx_reference*)&mosaic_input_arr, 1, &num_refs);

            status = assign_array_image_buffers(
                        obj->dec_pool.arr[obj->mosaic_enq_id],
                        NULL,
                        obj->dec_pool.plane_sizes);

            obj->mosaic_enq_id++;
            obj->mosaic_enq_id      = (obj->mosaic_enq_id  >= obj->dec_pool.bufq_depth)? 0 : obj->mosaic_enq_id;
            obj->appsink_pull_id++;
            obj->appsink_pull_id    = (obj->appsink_pull_id  >= obj->num_codec_bufs)? 0 : obj->appsink_pull_id;

            appCodecEnqAppSink(obj->appsink_pull_id);
        }
    }
    if ( obj->encode==1 || obj->decode==1 )
    {
        appCodecStop();
        APP_PRINTF("appCodecStop Done!\n");
    }
#if defined(DEC_WRITE_TO_FILE)
    fclose(fp);
#endif

    if (status == VX_SUCCESS && (obj->encode==1 || obj->decode==0 ))
    {
        status = vxWaitGraph(obj->capture_graph);
    }
    if (status == VX_SUCCESS && (obj->encode==0 || obj->decode==1 ))
    {
        status = vxWaitGraph(obj->display_graph);
    }
    obj->stop_task = 1;

    if (obj->encode==1){
    if (status == VX_SUCCESS)
    {
        status = appStopImageSensor(obj->sensorObj.sensor_name, ch_mask);
    }
    }

    return status;
}

#if defined(DEC_WRITE_TO_FILE)
static vx_status write_vx_object_arr(vx_object_array in_arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], FILE *fp)
{
    vx_status status;
    vx_int32 ch;
    vx_map_id map_id[CODEC_MAX_NUM_PLANES];

    status = vxGetStatus((vx_reference)in_arr);

    for(ch = 0; status==VX_SUCCESS && ch<1; ch++)
    {
        vx_rectangle_t rect;
        vx_imagepatch_addressing_t image_addr;

        vx_uint32  img_width;
        vx_uint32  img_height;

        vx_image in_img = (vx_image)vxGetObjectArrayItem(in_arr,ch);

        vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height;

        /* MAP Luma */
        status = vxMapImagePatch(in_img,
                                &rect,
                                0,
                                &map_id[0],
                                &image_addr,
                                &data_ptr[ch][0],
                                VX_READ_AND_WRITE,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);
        if (status != VX_SUCCESS) {printf("map_obj_arr(): vxMap unsuccessful"); return(status);}

        /* Map CbCr */
        status = vxMapImagePatch(in_img,
                                &rect,
                                1,
                                &map_id[1],
                                &image_addr,
                                &data_ptr[ch][1],
                                VX_READ_AND_WRITE,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);
        if (status != VX_SUCCESS) {printf("copy_image(): vxMap unsuccessful"); return(status);}

        //write to file
        vx_int32 buf_len = 3133440;

        fwrite(data_ptr[ch][0], 1, buf_len, fp);

        /* UNMAP Luma */
        vxUnmapImagePatch(in_img, map_id[0]);

        /* UNMap CbCr */
        vxUnmapImagePatch(in_img, map_id[1]);

        vxReleaseReference((vx_reference*)&in_img);
    }
    return(status);
}
#endif

static vx_status map_vx_object_arr(vx_object_array in_arr, void* data_ptr[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_map_id map_id[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_int32 num_channels)
{
    vx_status status;
    vx_int32 ch;

    status = vxGetStatus((vx_reference)in_arr);

    for(ch = 0; status==VX_SUCCESS && ch<num_channels; ch++)
    {
        vx_rectangle_t rect;
        vx_imagepatch_addressing_t image_addr;

        vx_uint32  img_width;
        vx_uint32  img_height;

        vx_image in_img = (vx_image)vxGetObjectArrayItem(in_arr,ch);

        vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = img_width;
        rect.end_y = img_height;

        /* MAP Luma */
        status = vxMapImagePatch(in_img,
                                &rect,
                                0,
                                &map_id[ch][0],
                                &image_addr,
                                &data_ptr[ch][0],
                                VX_READ_AND_WRITE,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);
        if (status != VX_SUCCESS) {printf("map_obj_arr(): vxMap unsuccessful"); return(status);}

        /* Map CbCr */
        status = vxMapImagePatch(in_img,
                                &rect,
                                1,
                                &map_id[ch][1],
                                &image_addr,
                                &data_ptr[ch][1],
                                VX_READ_AND_WRITE,
                                VX_MEMORY_TYPE_HOST,
                                VX_NOGAP_X);
        if (status != VX_SUCCESS) {printf("map_obj_arr(): vxMap unsuccessful"); return(status);}

        vxReleaseReference((vx_reference*)&in_img);
    }
    return(status);
}

static vx_status unmap_vx_object_arr(vx_object_array in_arr, vx_map_id map_id[CODEC_MAX_NUM_CHANNELS][CODEC_MAX_NUM_PLANES], vx_int32 num_channels)
{
    vx_status status;
    vx_int32 ch;

    status = vxGetStatus((vx_reference)in_arr);

    for(ch = 0; status==VX_SUCCESS && ch<num_channels; ch++)
    {
        vx_image in_img = (vx_image)vxGetObjectArrayItem(in_arr,ch);

        /* UNMAP Luma */
        status = vxUnmapImagePatch(in_img, map_id[ch][0]);
        if (status != VX_SUCCESS) {printf("unmap_obj_arr(): vxUnmap unsuccessful"); return(status);}

        /* UNMap CbCr */
        status = vxUnmapImagePatch(in_img, map_id[ch][1]);
        if (status != VX_SUCCESS) {printf("unmap_obj_arr(): vxUnmap unsuccessful"); return(status);}

        vxReleaseReference((vx_reference*)&in_img);
    }
    return(status);
}

static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 1;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->capture_id     = 0;
    obj->display_id     = 0;
    obj->ldc_enq_id     = 0;
    obj->appsrc_push_id = 0;
    obj->mosaic_enq_id  = 0;
    obj->appsink_pull_id= 0;
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

    sensorObj->enable_ldc = 1;
    sensorObj->num_cameras_enabled = 1;
    sensorObj->ch_mask = 0x1;
    sensorObj->usecase_option = APP_SENSOR_FEATURE_CFG_UC0;
    sensorObj->is_interactive = 0;
}

static void set_ref_pool_defaults(AppGraphParamRefPool *poolObj)
{
    poolObj->width          = 1920;
    poolObj->height         = 1088;
    poolObj->format         = VX_DF_IMAGE_NV12;
    poolObj->num_planes     = 2;
    poolObj->num_channels   = 1;
    poolObj->bufq_depth     = 1;
    poolObj->plane_sizes[0] = poolObj->width*poolObj->height*3/2;
}

static void app_default_param_set(AppObj *obj)
{
    set_sensor_defaults(&obj->sensorObj);

    set_display_defaults(&obj->displayObj);

    set_ref_pool_defaults(&obj->enc_pool);
    set_ref_pool_defaults(&obj->dec_pool);

    app_pipeline_params_defaults(obj);

    obj->is_interactive = 1;
    obj->test_mode = 0;
    obj->write_file = 0;

    obj->encode     = 1;
    obj->decode     = 1;
    obj->downscale  = 0;
    obj->num_ch     = 1;

    obj->enc_pool.bufq_depth    = APP_ENC_BUFFER_Q_DEPTH;
    obj->dec_pool.bufq_depth    = APP_DEC_BUFFER_Q_DEPTH;
    obj->num_codec_bufs         = CODEC_DEC_BUFQ_SIZE;
}

static void app_querry_param_set(AppObj *obj)
{
    vx_char ch = 0;
    vx_bool encSelected = vx_false_e;
    vx_bool decSelected = vx_false_e;
    vx_int8 ret;
    vx_int8 num_cameras;
    obj->sensorObj.num_cameras_enabled = 0;

    while (encSelected != vx_true_e)
    {
        fflush (stdin);
        printf ("Capture->Encode Selection Yes(1)/No(0)\n");
        ch = getchar();
        obj->encode = ch - '0';

        if((obj->encode > 1) || (obj->encode < 0))
        {
            printf("Invalid selection %c. Try again \n", ch);
        }
        else
        {
            encSelected = vx_true_e;
        }
        ch = getchar();
    }
    while (decSelected != vx_true_e)
    {
        fflush (stdin);
        printf ("Decode->Display Selection Yes(1)/No(0)\n");
        ch = getchar();
        obj->decode = ch - '0';

        if((obj->decode > 1) || (obj->decode < 0))
        {
            printf("Invalid selection %c. Try again \n", ch);
        }
        else
        {
            decSelected = vx_true_e;
        }
        ch = getchar();
    }
    #if defined(QNX)
    if ((1==obj->encode) && (1==obj->decode))
    {
        printf("Invalid selections: Both encode and decode are not enabled in the same app for QNX\n");
        exit(0);
    }
    #endif
    while (obj->sensorObj.num_cameras_enabled == 0)
    {
        fflush(stdin);
        printf("Max number of cameras supported by sensor %s = %d \n", obj->sensorObj.sensor_name, obj->sensorObj.sensorParams.num_channels);
        printf("Please enter number of channels to be enabled \n");
        ret = scanf("%hhd", &num_cameras);
        while ((ch = getchar()) != '\n' && ch != EOF);
        obj->sensorObj.num_cameras_enabled = num_cameras;
        if((1==ret) && ((obj->sensorObj.num_cameras_enabled > obj->sensorObj.sensorParams.num_channels) || (obj->sensorObj.num_cameras_enabled <= 0)))
        {
            obj->sensorObj.num_cameras_enabled = 0;
            printf("Invalid selection %c. Try again \n", ch);
        }
    }
    obj->num_ch = obj->sensorObj.num_cameras_enabled;
    obj->sensorObj.ch_mask = (1<<obj->sensorObj.num_cameras_enabled) - 1;
}

static vx_int32 calc_grid_size(vx_uint32 ch)
{
    if(0==ch)
    {
        return -1;
    }
    else if(1==ch)
    {
        return 1;
    }
    else if(4>=ch)
    {
        return 2;
    }
    else if(9>=ch)
    {
        return 3;
    }
    else if(16>=ch)
    {
        return 4;
    }else
    {
        return -1;
    }
}

static void set_img_mosaic_params(TIOVXImgMosaicModuleObj *imgMosaicObj, vx_uint32 in_width, vx_uint32 in_height, vx_int32 numCh)
{
    vx_int32 idx, ch;
    vx_int32 grid_size = calc_grid_size(numCh);

    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->out_bufq_depth = APP_BUFFER_Q_DEPTH;
    imgMosaicObj->color_format = VX_DF_IMAGE_NV12;

    imgMosaicObj->num_inputs   = 1;
    imgMosaicObj->num_channels = numCh;

    imgMosaicObj->inputs[0].width = in_width;
    imgMosaicObj->inputs[0].height = in_height;
    imgMosaicObj->inputs[0].bufq_depth = APP_BUFFER_Q_DEPTH;
    imgMosaicObj->inputs[0].color_format = VX_DF_IMAGE_NV12;

    idx = 0;

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

    for(ch = 0; ch < numCh; ch++)
    {
        vx_int32 winX = ch%grid_size;
        vx_int32 winY = ch/grid_size;

        imgMosaicObj->params.windows[idx].startX  = (winX * (in_width/grid_size));
        imgMosaicObj->params.windows[idx].startY  = (winY * (in_height/grid_size));
        imgMosaicObj->params.windows[idx].width   = in_width/grid_size;
        imgMosaicObj->params.windows[idx].height  = in_height/grid_size;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = ch;
        idx++;
    }

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = APP_BUFFER_Q_DEPTH;
}

#if defined(LINUX)
static void construct_gst_strings(app_codec_wrapper_params_t* params, uint8_t srcType, uint8_t sinkType)
{
    int32_t i = 0;

    for (uint8_t ch = 0; ch < params->in_num_channels; ch++)
    {
        if (srcType == 0){
            snprintf(params->m_AppSrcNameArr[ch] , CODEC_MAX_LEN_ELEM_NAME, "myAppSrc%d" , ch);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"appsrc format=GST_FORMAT_TIME is-live=true do-timestamp=true block=false name=%s ! queue \n",params->m_AppSrcNameArr[ch]);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! video/x-raw, width=(int)%d, height=(int)%d, framerate=(fraction)30/1, format=(string)%s, interlace-mode=(string)progressive, colorimetry=(string)bt601 \n",
                                                                                            params->in_width, params->in_height, params->in_format);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! v4l2h264enc extra-controls=\"controls, frame_level_rate_control_enable=1, video_bitrate=10000000\"\n");
        }
        else if (srcType == 1){
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"multifilesrc location=/opt/vision_apps/test_data/psdkra/app_multi_cam_codec/test_video_1080p30.264 \n");
        }
        else if (srcType == 2){
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"videotestsrc is-live=true do-timestamp=true num-buffers=%d \n",1800);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! video/x-raw, width=(int)%d, height=(int)%d, framerate=(fraction)30/1, format=(string)%s, interlace-mode=(string)progressive, colorimetry=(string)bt601 \n",
                                                                                            params->in_width, params->in_height, params->in_format);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! v4l2h264enc extra-controls=\"controls, frame_level_rate_control_enable=1, video_bitrate=10000000\"\n");
        }

        i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! h264parse \n");

        if (sinkType == 0){
            snprintf(params->m_AppSinkNameArr[ch], CODEC_MAX_LEN_ELEM_NAME, "myAppSink%d", ch);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! v4l2h264dec capture-io-mode=dmabuf-import \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! video/x-raw, format=(string)%s, stride-x-align=64 \n",
                                                                                        params->out_format);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! tiovxmemalloc pool-size=15 \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! appsink name=%s drop=true wait-on-eos=false max-buffers=4\n",params->m_AppSinkNameArr[ch]);
        }
        else if (sinkType == 1){
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! mp4mux \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! filesink location=output_video_%d.mp4 \n", ch);
        }
        else if (sinkType == 2){
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! v4l2h264dec capture-io-mode=dmabuf-import \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! video/x-raw, format=(string)%s, stride-x-align=64 \n",
                                                                                        params->out_format);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! tiovxmemalloc pool-size=15 \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! fakesink \n");
        }
        else if (sinkType == 3){
            snprintf(params->m_AppSinkNameArr[ch], CODEC_MAX_LEN_ELEM_NAME, "myAppSink%d", ch);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! v4l2h264dec \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! video/x-raw, format=(string)%s \n",
                                                                                        params->out_format);
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! tiovxmemalloc pool-size=7 \n");
            i += snprintf(&params->m_cmdString[i], CODEC_MAX_LEN_CMD_STR-i,"! appsink name=%s drop=true wait-on-eos=false max-buffers=4\n",params->m_AppSinkNameArr[ch]);
        }
    }
}
#endif /* LINUX */

static void set_codec_pipe_params(AppObj *obj)
{
    AppGraphParamRefPool *enc_pool = &obj->enc_pool;
    AppGraphParamRefPool *dec_pool = &obj->dec_pool;
    app_codec_wrapper_params_t *codec_pipe_params = &obj->codec_pipe_params;

#if defined(LINUX)
    uint8_t srcType     = 0;
    uint8_t sinkType    = 0;

    if (obj->encode==0) srcType = 1;
    if (obj->decode==0) sinkType = 1;
#endif /* LINUX */

    codec_pipe_params->in_width        = enc_pool->width;
    codec_pipe_params->in_height       = enc_pool->height;
    snprintf(codec_pipe_params->in_format,8,"NV12");
    codec_pipe_params->in_num_planes   = enc_pool->num_planes;
    codec_pipe_params->in_num_channels = enc_pool->num_channels;
    codec_pipe_params->in_buffer_depth = enc_pool->bufq_depth;

    codec_pipe_params->out_width        = dec_pool->width;
    codec_pipe_params->out_height       = dec_pool->height;
    snprintf(codec_pipe_params->out_format,8,"NV12");
    codec_pipe_params->out_num_planes   = dec_pool->num_planes;
    codec_pipe_params->out_num_channels = dec_pool->num_channels;
    codec_pipe_params->out_buffer_depth = dec_pool->bufq_depth;
#if defined(LINUX)
    construct_gst_strings(codec_pipe_params,srcType,sinkType);
#endif /* LINUX */
}

static void app_update_param_set(AppObj *obj)
{

    vx_uint16 resized_width, resized_height;
    appIssGetResizeParams(obj->sensorObj.image_width, obj->sensorObj.image_height, DISPLAY_WIDTH, DISPLAY_HEIGHT, &resized_width, &resized_height);

    /* Don't allow downscaling to 720p if single channel */
    if (obj->sensorObj.num_cameras_enabled == 1)
        obj->downscale = 0;

    set_img_mosaic_params(&obj->imgMosaicObj, resized_width, resized_height, obj->num_ch);

    obj->enc_pool.num_channels = obj->num_ch;
    obj->dec_pool.num_channels = obj->num_ch;

    if (obj->downscale == 1)
    {
        obj->enc_pool.width = 1280;
        obj->enc_pool.height = 720;
        obj->dec_pool.width = 1280;
        obj->dec_pool.height = 720;
    }
#if defined(SOC_J721E)
    else
    {
        /* decoder outputs 16 byte alligned buffers */
        obj->dec_pool.height = 1088;
    #if defined(QNX)
        obj->enc_pool.height = 1088;
    #endif /* QNX */
    }
#endif /* SOC_J721E */
#if defined(QNX) && !defined(SOC_J721E)
    else
    {
        /* decoder outputs 16 byte alligned buffers */
        obj->dec_pool.height = 1088;
    }
#endif /* QNX && !SOC_J721E */
#if defined(SOC_AM62A) && defined(QNX)
    if ((0 == strcmp(obj->sensorObj.sensor_name, SENSOR_OV2312_UB953_LI)))
    {
        obj->enc_pool.width = 1600;
        obj->enc_pool.height = 1300;
        obj->dec_pool.width = 1600;
        obj->dec_pool.height = 1300;
    }
#endif /* QNX && SOC_AM62A */
    obj->enc_pool.plane_sizes[0] = obj->enc_pool.width * obj->enc_pool.height * 3/2;
    obj->dec_pool.plane_sizes[0] = obj->dec_pool.width * obj->dec_pool.height * 3/2;

    set_codec_pipe_params(obj);
}
