/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include "fisheye_angle_table.h"

#include "avp_common.h"
#include "avp_scaler_module.h"
#include "avp_dof_pyramid_module.h"
#include "avp_dof_proc_module.h"
#include "avp_dof_viz_module.h"
#include "avp_pre_proc_module.h"
#include "avp_tidl_module.h"
#include "avp_post_proc_module.h"
#include "avp_draw_detections_module.h"
#include "avp_img_mosaic_module.h"
#include "avp_display_module.h"
#include "avp_test.h"
#if defined(APP_CTOOLS_ENABLED) && !defined(x86_64)
#include "ctools_wrapper.h"
#endif

#ifndef x86_64
#define AVP_ENABLE_PIPELINE_FLOW
#endif

#define AVP_BUFFER_Q_DEPTH   (2)
#define AVP_PIPELINE_DEPTH   (7)

typedef struct {

    ScalerObj  scalerObj;

    DofPreProcObj dofPreProcObj;
    PreProcObj preProcObj;

    PyramidObj pyramidObj;
    DofProcObj dofProcObj;
    DofVizObj dofVizObj;

    TIDLObj odTIDLObj;
    TIDLObj pcTIDLObj;

    PostProcObj odPostProcObj;
    PostProcObj pcPostProcObj;

    DrawDetectionsObj psdDrawDetectionsObj;
    DrawDetectionsObj vdDrawDetectionsObj;

    ImgMosaicObj imgMosaicObj;

    DisplayObj displayObj;

    vx_char input_file_path[APP_MAX_FILE_PATH];
    vx_char output_file_path[APP_MAX_FILE_PATH];

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    vx_int32 en_out_img_write;

    vx_float32 viz_th_pd;
    vx_float32 viz_th_vd;

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

    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t enable_psd;
    int32_t enable_vd;
    int32_t enable_sem_seg;

    int32_t pipeline;

    int32_t enqueueCnt;
    int32_t dequeueCnt;

} AppObj;

AppObj gAppObj;

static vx_int32 g_frame_switch = 0;

static const vx_uint8 motion_segment_color_map[2][3] = {{0,0,0},{128,64,128}};
static const vx_uint8 semantic_segment_color_map[5][3] = {{152,251,152},{0,130,180},{220,20,60},{3,3,251},{190,153,153}};
static const vx_uint8 depth_segment_color_map[63][3] ={{255,10,0},{255,20,0},{255,30,0},{255,40,0},{255,50,0},{255,60,0},{255,70,0},{255,80,0},
                                                       {255,91,0},{255,101,0},{255,111,0},{255,121,0},{255,131,0},{255,141,0},{255,151,0},{255,161,0},
                                                       {255,172,0},{255,182,0},{255,192,0},{255,202,0},{255,212,0},{255,222,0},{255,232,0},{255,242,0},
                                                       {255,252,0},{238,255,0},{218,255,0},{198,255,0},{178,255,0},{157,255,0},{137,255,0},{117,255,0},
                                                       {97,255,0},{76,255,0},{56,255,0},{36,255,0},{16,255,0},{0,250,4},{0,230,24},{0,210,44},
                                                       {0,190,64},{0,170,84},{0,149,105},{0,129,125},{0,109,145},{0,89,165},{0,68,186},{0,48,206},
                                                       {0,28,226},{0,8,246},{8,0,255},{21,0,255},{35,0,255},{48,0,255},{62,0,255},{75,0,255},
                                                       {89,0,255},{102,0,255},{116,0,255},{129,0,255},{143,0,255},{156,0,255},{170,0,255}
                                                      };

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
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_pipeline_params_defaults(AppObj *obj);
static void app_find_object_array_index(vx_object_array object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
static void set_img_mosaic_frame1(AppObj *obj, ImgMosaicObj *imgMosaicObj);
static void set_img_mosaic_frame2(AppObj *obj, ImgMosaicObj *imgMosaicObj);

#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
#ifdef AVP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);
#else
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id);
#endif

static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL AVP Demo 3 - (c) Texas Instruments 2018\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n ========================="
    "\n Demo : TIDL AVP Demo 3"
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
    while((!obj->stop_task) && (status == VX_SUCCESS))
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
    if(status != VX_SUCCESS)
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
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_avp3");
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
    snprintf(obj->odTIDLObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->odTIDLObj.network_file_path,APP_MAX_FILE_PATH, ".");

    snprintf(obj->pcTIDLObj.config_file_path,APP_MAX_FILE_PATH, ".");
    snprintf(obj->pcTIDLObj.network_file_path,APP_MAX_FILE_PATH, ".");

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
            if(strcmp(token, "tidl_pc_config")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->pcTIDLObj.config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_pc_network")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    strcpy(obj->pcTIDLObj.network_file_path, token);
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
                    obj->scalerObj.input.width  = width;

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->scalerObj.input.height = height;
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

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if(token[strlen(token)-1] == '\n')
                        token[strlen(token)-1]=0;

                        height =  atoi(token);
                        obj->scalerObj.output.height  = height;
                    }
                }
            }
            else
            if(strcmp(token, "viz_th")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->viz_th_pd = atof(token);

                    token = strtok(NULL, s);
                    if(token != NULL)
                    {
                        if (token[strlen(token)-1] == '\n')
                        token[strlen(token)-1]=0;

                        obj->viz_th_vd = atof(token);
                    }
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
                int32_t i;
                for(i = 0; i < TIVX_PIXEL_VIZ_MAX_CLASS*4; i++)
                {
                    token = strtok(NULL, s);
                    if(token != NULL){
                        obj->valid_region[i>>2][i&0x3] = atoi(token);
                    }else{
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
            if(strcmp(token, "enable_psd")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_psd = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_vd")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->enable_vd = atoi(token);
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
            if(strcmp(token, "enable_temporal_predicton_flow_vector")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->dofProcObj.enable_temporal_predicton_flow_vector = atoi(token);
                }
            }
            else
            if(strcmp(token, "enable_flow_vec_delay_obj")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->dofProcObj.enable_flow_vec_delay_obj = atoi(token);
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
            if(strcmp(token, "confidence_threshold_value")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->dofVizObj.confidence_threshold_value = atoi(token);
                }
            }
        }
    }

    if (obj->dofProcObj.enable_temporal_predicton_flow_vector == 0)
    {
        obj->dofProcObj.enable_flow_vec_delay_obj = 0;
    }

    if (obj->test_mode == 1)
    {
        obj->is_interactive = 0;
        /* display_option must be set to 1 in order for the checksums
            to come out correctly */
        obj->displayObj.display_option = 1;
        obj->num_iterations = 1;
        obj->num_frames = (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])) + TEST_BUFFER;
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
        obj->num_frames = (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])) + TEST_BUFFER;
    }

    #ifdef x86_64
    obj->displayObj.display_option = 0;
    obj->is_interactive = 0;
    #endif

    return;
}

vx_int32 app_tidl_avp_main(vx_int32 argc, vx_char* argv[])
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
        APP_PRINTF("App Verify Graph Done! %d\n", status);
    }
    if(status == VX_SUCCESS)
    {
        if(obj->is_interactive)
        {
            status = app_run_graph_interactive(obj);
        }
        else
        {
            status = app_run_graph(obj);
        }

        APP_PRINTF("App Run Graph Done! \n");
    }

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");
    if (obj->test_mode)
    {
        if((test_result == vx_false_e) || (status != VX_SUCCESS))
        {
            printf("\nTEST FAILED\n");
            /* in the case that checksums changed, print a new checksums_expected
                array */
            print_new_checksum_structs();
            status = (status == VX_SUCCESS) ? VX_FAILURE : status;
        }
        else
        {
            printf("\nTEST PASSED\n");
        }
    }

    return status;
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

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    app_grpx_init_prms_t grpx_prms;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();

    status = vxGetStatus((vx_reference)obj->context);
    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxImgProcLoadKernels(obj->context);
        tivxTIDLLoadKernels(obj->context);
    }
    /* Initialize modules */
    if(status == VX_SUCCESS)
    {
        status = app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", AVP_BUFFER_Q_DEPTH);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_pyramid(obj->context, &obj->pyramidObj, "pyramid_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_dof_proc(obj->context, &obj->dofProcObj, "dof_proc_obj");
    }
    /* Initialize TIDL first to get tensor I/O information from network */
    if(status == VX_SUCCESS)
    {
        #if defined (SOC_J784S4)
        obj->odTIDLObj.core_id = 2;
        #else
        obj->odTIDLObj.core_id = 0;
        #endif
        status = app_init_tidl_od(obj->context, &obj->odTIDLObj, "od_tidl_obj");
    }
    if(status == VX_SUCCESS)
    {
        #if defined (SOC_J784S4)
        obj->pcTIDLObj.core_id = 3;
        #else
        obj->pcTIDLObj.core_id = 0;
        #endif
        status = app_init_tidl_pc(obj->context, &obj->pcTIDLObj, "pc_tidl_obj");
    }

    /* For a multi-task network the input tensor at 0th index is dof planes and 1st index is image */
    if(status == VX_SUCCESS)
    {
        status = app_update_dof_pre_proc(obj->context, &obj->dofPreProcObj, obj->odTIDLObj.config, 0);
    }
    if(status == VX_SUCCESS)
    {
        status = app_update_pre_proc(obj->context, &obj->preProcObj, obj->odTIDLObj.config, 0);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_dof_pre_proc(obj->context, &obj->dofPreProcObj, "dof_pre_proc_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_pre_proc(obj->context, &obj->preProcObj, "pre_proc_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_update_post_proc_od(&obj->odPostProcObj, obj->odTIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        status = app_update_post_proc_pc(&obj->pcPostProcObj, obj->pcTIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_post_proc_od(obj->context, &obj->odPostProcObj, "od_post_proc_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_post_proc_pc(obj->context, &obj->pcPostProcObj, "pc_post_proc_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_update_draw_detections(&obj->psdDrawDetectionsObj, obj->odTIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        status = app_update_draw_detections(&obj->vdDrawDetectionsObj, obj->odTIDLObj.config);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_draw_detections(obj->context, &obj->psdDrawDetectionsObj, "psd_draw_detections_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_draw_detections(obj->context, &obj->vdDrawDetectionsObj, "vd_draw_detections_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_dof_viz(obj->context, &obj->dofVizObj, "dof_viz_obj");
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_img_mosaic(obj->context, &obj->imgMosaicObj, AVP_BUFFER_Q_DEPTH);
    }
    if(status == VX_SUCCESS)
    {
        status = app_init_display(obj->context, &obj->displayObj, "display_obj");
    }
    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");

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

    app_deinit_pyramid(&obj->pyramidObj);

    app_deinit_dof_proc(&obj->dofProcObj);

    app_deinit_dof_pre_proc(&obj->dofPreProcObj);

    app_deinit_pre_proc(&obj->preProcObj);

    app_deinit_tidl_od(&obj->odTIDLObj);

    app_deinit_tidl_pc(&obj->pcTIDLObj);

    app_deinit_post_proc_od(&obj->odPostProcObj);

    app_deinit_post_proc_pc(&obj->pcPostProcObj);

    app_deinit_draw_detections(&obj->psdDrawDetectionsObj);

    app_deinit_draw_detections(&obj->vdDrawDetectionsObj);

    app_deinit_dof_viz(&obj->dofVizObj);

    app_deinit_img_mosaic(&obj->imgMosaicObj, AVP_BUFFER_Q_DEPTH);

    app_deinit_display(&obj->displayObj);

    #ifndef x86_64
    if(obj->displayObj.display_option == 1 && obj->enable_gui)
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

    app_delete_dof_proc(&obj->dofProcObj);

    app_delete_pyramid(&obj->pyramidObj);

    app_delete_dof_pre_proc(&obj->dofPreProcObj);

    app_delete_pre_proc(&obj->preProcObj);

    app_delete_tidl_od(&obj->odTIDLObj);

    app_delete_tidl_pc(&obj->pcTIDLObj);

    app_delete_post_proc_od(&obj->odPostProcObj);

    app_delete_post_proc_pc(&obj->pcPostProcObj);

    app_delete_draw_detections(&obj->psdDrawDetectionsObj);

    app_delete_draw_detections(&obj->vdDrawDetectionsObj);

    app_delete_dof_viz(&obj->dofVizObj);

    app_delete_img_mosaic(&obj->imgMosaicObj);

    app_delete_display(&obj->displayObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->graph, "avp3_graph");
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj);
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_pyramid(obj->graph, &obj->pyramidObj, obj->scalerObj.output.arr[0], DOF_PYRAMID_START_FROM_PREVIOUS);
    }
    if(status == VX_SUCCESS)
    {
        status = vxRegisterAutoAging(obj->graph, obj->pyramidObj.pyramid_delay);
    }
    if((obj->dofProcObj.enable_flow_vec_delay_obj == 1) && (status == VX_SUCCESS))
    {
        status = vxRegisterAutoAging(obj->graph, obj->dofProcObj.flow_vector_field_delay);
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_dof_proc(obj->graph, &obj->dofProcObj, obj->pyramidObj.pyramid_delay);
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_dof_pre_proc(obj->graph, &obj->dofPreProcObj, obj->dofProcObj.flow_vector_field_array);
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_pre_proc(obj->graph, &obj->preProcObj, obj->scalerObj.output.arr[0]);
    }
    if((obj->enable_psd == 1) || (obj->enable_vd == 1))
    {
        if(VX_SUCCESS == status)
        {
            status = app_create_graph_tidl_od(obj->context, obj->graph, &obj->odTIDLObj, obj->preProcObj.output_tensor_arr);
        }
    }

    if(obj->enable_sem_seg == 1)
    {
        if(VX_SUCCESS == status)
        {
            status = app_create_graph_tidl_pc(obj->context, obj->graph, &obj->pcTIDLObj, obj->dofPreProcObj.output_tensor_arr, obj->preProcObj.output_tensor_arr);
        }
    }

    if((obj->enable_psd == 1) || (obj->enable_vd == 1))
    {
        app_create_graph_post_proc_od(obj->graph, &obj->odPostProcObj, obj->odTIDLObj.output_tensor_arr[0]);
    }

    if(obj->enable_sem_seg == 1)
    {
        app_create_graph_post_proc_pc(obj->graph, &obj->pcPostProcObj, obj->scalerObj.output.arr[0], obj->pcTIDLObj.out_args_arr, obj->pcTIDLObj.output_tensor_arr);
    }

    if((obj->enable_psd == 1) && (status == VX_SUCCESS))
    {
        status = app_create_graph_draw_detections(obj->graph, &obj->psdDrawDetectionsObj, obj->odPostProcObj.kp_tensor_arr, obj->odPostProcObj.kp_valid_arr, obj->odTIDLObj.output_tensor_arr[0], obj->scalerObj.output.arr[0]);
    }

    if((obj->enable_vd == 1) && (status == VX_SUCCESS))
    {
        status = app_create_graph_draw_detections(obj->graph, &obj->vdDrawDetectionsObj, obj->odPostProcObj.kp_tensor_arr, obj->odPostProcObj.kp_valid_arr, obj->odTIDLObj.output_tensor_arr[0], obj->scalerObj.output.arr[0]);
    }

    if((obj->dofProcObj.enable_flow_vec_delay_obj == 1) && (status == VX_SUCCESS))
    {
        status = app_create_graph_dof_viz(obj->graph,
                                            &obj->dofVizObj,
                                            obj->dofProcObj.flow_vector_field_delay,
                                            NULL);
    }
    else
    if(status == VX_SUCCESS)
    {
        status = app_create_graph_dof_viz(obj->graph,
                                            &obj->dofVizObj,
                                            NULL,
                                            obj->dofProcObj.flow_vector_field_array);
    }

    vx_int32 idx = 0;
    if(obj->enable_psd == 1)
    {
        obj->imgMosaicObj.input_arr[idx++] = obj->psdDrawDetectionsObj.output_image_arr; //PSD output
    }
    if(obj->enable_vd == 1)
    {
        obj->imgMosaicObj.input_arr[idx++] = obj->vdDrawDetectionsObj.output_image_arr; //VD output
    }
    if(obj->enable_sem_seg == 1)
    {
        obj->imgMosaicObj.input_arr[idx++] = obj->pcPostProcObj.output_image_arr[1]; //Seg output
        obj->imgMosaicObj.input_arr[idx++] = obj->pcPostProcObj.output_image_arr[2]; //Motion output
        obj->imgMosaicObj.input_arr[idx++] = obj->pcPostProcObj.output_image_arr[0]; //Depth output
    }
    obj->imgMosaicObj.input_arr[idx++] = obj->dofVizObj.flow_vector_field_image_array; //DOF output

    obj->imgMosaicObj.num_inputs = idx;

    app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);

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
                                            graph_parameter_index,
                                            graph_parameters_queue_params_list);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetGraphPipelineDepth(obj->graph, AVP_PIPELINE_DEPTH);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 6);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->pyramidObj.node, 1, 2);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->dofProcObj.node, 8, 2);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->dofVizObj.node, 2, 4);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->dofPreProcObj.node, 2, 2);
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 2, 5);
    }
    if((obj->enable_psd == 1) || (obj->enable_vd == 1))
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->odTIDLObj.node, 7, 3);
        }
    }

    if(obj->enable_sem_seg == 1)
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcTIDLObj.node, 8, 2);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcTIDLObj.node, 9, 2);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcTIDLObj.node, 10, 2);
        }
    }

    if((obj->enable_psd == 1) || (obj->enable_vd == 1))
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->odPostProcObj.node, 5, 2);
        }
    }

    if(obj->enable_psd == 1)
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->psdDrawDetectionsObj.node, 5, 2);
        }
    }

    if(obj->enable_vd == 1)
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->vdDrawDetectionsObj.node, 5, 2);
        }
    }

    if(obj->enable_sem_seg == 1)
    {
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcPostProcObj.node, 6, 4);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcPostProcObj.node, 7, 4);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->pcPostProcObj.node, 8, 4);
        }
    }
    if(status == VX_SUCCESS)
    {
        if(!((obj->en_out_img_write == 1) || (obj->test_mode == 1)))
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
    if(status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_avp3");
    }
    #endif

    scalerObj = &obj->scalerObj;

    refs[0] = (vx_reference)scalerObj->coeff_obj;
    if(status == VX_SUCCESS)
    {
        status = tivxNodeSendCommand(scalerObj->node, 0u,
                                    TIVX_VPAC_MSC_CMD_SET_COEFF,
                                    refs, 1u);
    }
    APP_PRINTF("App Send MSC Command Done!\n");

    if(status != VX_SUCCESS)
    {
        printf("MSC: Node send command failed!\n");
    }

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

    return status;
}

#ifndef AVP_ENABLE_PIPELINE_FLOW
static vx_status app_run_graph_for_one_frame_sequential(AppObj *obj, vx_int32 frame_id)
{
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];

    ScalerObj *scalerObj = &obj->scalerObj;

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);

    appPerfPointBegin(&obj->fileio_perf);

    readScalerInput(input_file_name, scalerObj->input.arr[0], NUM_CH);

    appPerfPointEnd(&obj->fileio_perf);

#ifdef APP_DEBUG
    printf("App Reading Input Done!\n");
#endif

#ifdef x86_64
    printf("Processing file %s ...", input_file_name);
#endif

    status = vxProcessGraph(obj->graph);

#ifdef x86_64
    printf("Done!\n");
#endif

#ifdef APP_DEBUG
    printf("App Process Graph Done!\n");
#endif

    if((VX_SUCCESS == status) && (obj->en_out_img_write == 1))
    {
        vx_char output_file_name[APP_MAX_FILE_PATH];

#ifdef APP_DEBUG
        printf("App Writing Outputs Start...\n");
#endif

#ifdef WRITE_INTERMEDIATE_OUTPUTS

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/%010d_768x384.yuv", obj->output_file_path, frame_id);
        writeScalerOutput(output_file_name, scalerObj->output.arr[0]);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/dof_flow_%010d", obj->output_file_path, frame_id);
        writeDofVizOutput(output_file_name, obj->dofVizObj.flow_vector_field_image_array);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/dof_pre_proc_output_%010d", obj->output_file_path, frame_id);
        writePreProcOutput(output_file_name, obj->dofPreProcObj.output_tensor_arr);

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/pre_proc_output_%010d", obj->output_file_path, frame_id);
        writePreProcOutput(output_file_name, obj->preProcObj.output_tensor_arr);

        if(obj->enable_psd == 1)
        {
            writeTIDLOutput(obj->odTIDLObj.output_tensor_arr[0], "Parking Spot Detection");
        }

        if(obj->enable_vd == 1)
        {
            writeTIDLOutput(obj->odTIDLObj.output_tensor_arr[0], "Vehicle Detection");
        }

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, frame_id);
        writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);

        if(obj->enable_psd == 1)
        {
            snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/psd_output_%010d", obj->output_file_path, frame_id);
            writePostProcOutput(output_file_name, obj->psdDrawDetectionsObj.output_image_arr);
        }

        if(obj->enable_vd == 1)
        {
            snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/vd_output_%010d", obj->output_file_path, frame_id);
            writePostProcOutput(output_file_name, obj->vdDrawDetectionsObj.output_image_arr);
        }

        if(obj->enable_sem_seg == 1)
        {
            snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/sem_seg_output_%010d_768x384.yuv", obj->output_file_path, frame_id);
            writeScalerOutput(output_file_name, obj->pcPostProcObj.output_image_arr);
        }
#endif

        snprintf(output_file_name, APP_MAX_FILE_PATH, "%s/mosaic_output_%010d_1920x1080.yuv", obj->output_file_path, frame_id);
        writeMosaicOutput(output_file_name, obj->imgMosaicObj.output_image[0]);

#ifdef APP_DEBUG
        printf("App Writing Outputs Done!\n");
#endif
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
    uint32_t actual_checksum = 0;

    /* This is the number of frames required for the pipeline AWB and AE algorithms to stabilize
        (note that 15 is only required for the 6-8 camera use cases - others converge quicker) */
    vx_char input_file_name[APP_MAX_FILE_PATH];
    vx_int32 obj_array_idx = -1;

    ScalerObj    *scalerObj    = &obj->scalerObj;
    ImgMosaicObj *imgMosaicObj = &obj->imgMosaicObj;

    snprintf(input_file_name, APP_MAX_FILE_PATH, "%s/%010d.yuv", obj->input_file_path, frame_id);

    if(obj->pipeline < 0)
    {
        /* Enqueue outpus */
        if (((obj->en_out_img_write == 1) || (obj->test_mode == 1)) && (status == VX_SUCCESS))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, imgMosaicObj->graph_parameter_index, (vx_reference*)&imgMosaicObj->output_image[obj->enqueueCnt], 1);
        }

        appPerfPointBegin(&obj->fileio_perf);
        /* Read input */
        if(status == VX_SUCCESS)
        {
            status = readScalerInput(input_file_name, scalerObj->input.arr[obj->enqueueCnt], NUM_CH);
        }

        appPerfPointEnd(&obj->fileio_perf);

        APP_PRINTF("App Reading Input Done! @ pipeline idx: %d\n", obj->pipeline);

        /* Enqueue input - start execution */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->graph_parameter_index, (vx_reference*)&obj->scalerObj.input_images[obj->enqueueCnt], 1);
        }

        obj->enqueueCnt++;
        obj->enqueueCnt   = (obj->enqueueCnt  >= AVP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
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
            if (obj->test_mode == 1 &&
                ((frame_id - obj->start_frame - 2) < (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0]))))
            {
                /* -2 is there because it takes 2 pipeline executions to get to normal execution */
                vx_uint32 expected_idx = frame_id - obj->start_frame - 2;

                /* Check the mosaic output image which contains everything */
                if (vx_false_e == app_test_check_image(mosaic_output_image, checksums_expected[0][expected_idx], &actual_checksum))
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

        if(status ==  VX_SUCCESS)
        {
            app_find_object_array_index(scalerObj->input.arr, (vx_reference)scaler_input_image, AVP_BUFFER_Q_DEPTH, &obj_array_idx);
        }
        if((obj_array_idx != -1) && (status == VX_SUCCESS))
        {
            status = readScalerInput(input_file_name, scalerObj->input.arr[obj_array_idx], NUM_CH);
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

    vx_int32 frame_id = obj->start_frame + 1;
    vx_int32 x = 0;
    APP_PRINTF("app_run_graph beginning\n");

#ifdef AVP_ENABLE_PIPELINE_FLOW
    app_pipeline_params_defaults(obj);
#endif
    int32_t g_switch_count = 0;
    APP_PRINTF("start_frame: %d, frame_id: %d, obj->num_frames: %d\n", obj->start_frame, frame_id, obj->num_frames);

    for(x = 0; x < obj->num_iterations; x++)
    {
        g_switch_count = 0;
        for(frame_id = (obj->start_frame + 1); frame_id < (obj->start_frame + obj->num_frames - 1); frame_id++)
        {
            APP_PRINTF("Running frame %d\n", frame_id);

            if(g_switch_count >=  ((obj->num_frames * 3) >> 2))
            {
                g_frame_switch = 1;
                set_img_mosaic_frame2(obj, &obj->imgMosaicObj);
            }
            else
            {
                g_frame_switch = 0;
                set_img_mosaic_frame1(obj, &obj->imgMosaicObj);
            }

            g_switch_count++;
            g_switch_count = (g_switch_count > obj->num_frames)? 0 : g_switch_count;
            if(status == VX_SUCCESS)
            {
              status = vxCopyUserDataObject(obj->imgMosaicObj.config, 0, sizeof(tivxImgMosaicParams),\
                                             &obj->imgMosaicObj.params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
            appPerfPointBegin(&obj->total_perf);

            cur_time = tivxPlatformGetTimeInUsecs();

            if(status == VX_SUCCESS)
            {
#ifdef AVP_ENABLE_PIPELINE_FLOW
                status = app_run_graph_for_one_frame_pipeline(obj, frame_id);
#else
                status = app_run_graph_for_one_frame_sequential(obj, frame_id);
#endif
            }
            cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
            /* convert to msecs */
            cur_time = cur_time/1000;

            if(cur_time < obj->delay_in_msecs)
            {
                tivxTaskWaitMsecs(obj->delay_in_msecs - cur_time);
            }

            appPerfPointEnd(&obj->total_perf);

            APP_PRINTF("app_avp3: Frame ID %d of %d ... Done.\n", frame_id, obj->start_frame + obj->num_frames);

            /* user asked to stop processing */
            if((obj->stop_task) || (status != VX_SUCCESS))
            {
                break;
            }
        }

        printf("app_avp3: Iteration %d of %d ... Done.\n", x, obj->num_iterations);
        appPerfPointPrintFPS(&obj->total_perf);
        appPerfPointReset(&obj->total_perf);
        if(x==0)
        {
            /* after first iteration reset performance stats */
            appPerfStatsResetAll();
        }

        if((obj->stop_task) || (status != VX_SUCCESS))
        {
            break;
        }
    }

#ifdef AVP_ENABLE_PIPELINE_FLOW
    if(status == VX_SUCCESS)
    {
        status = vxWaitGraph(obj->graph);
    }
#endif

    obj->stop_task = 1;

    return status;
}

static void set_pre_proc_defaults(PreProcObj *preProcObj)
{
    vx_int32 i;
    for(i = 0; i< 3 ; i++)
    {
        preProcObj->params.scale_val[i] = 1.0;
        preProcObj->params.mean_pixel[i] = 0.0;
    }

    preProcObj->params.ip_rgb_or_yuv = 1;
    preProcObj->params.color_conv_flag = TIADALG_COLOR_CONV_YUV420_BGR;
}

static void update_pre_proc_params(AppObj *obj, PreProcObj *preProcObj)
{
    preProcObj->params.ip_rgb_or_yuv = obj->ip_rgb_or_yuv;

    if(obj->ip_rgb_or_yuv == 0)
    {
        preProcObj->params.color_conv_flag = TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR;
    }
    else
    {
        preProcObj->params.color_conv_flag = TIADALG_COLOR_CONV_YUV420_BGR;
    }
}

static void set_dof_pre_proc_defaults(DofPreProcObj *dofPreProcObj)
{
    dofPreProcObj->params.width  = 768;
    dofPreProcObj->params.height = 384;

    dofPreProcObj->params.pad_pixel[0] = 0;
    dofPreProcObj->params.pad_pixel[1] = 0;
    dofPreProcObj->params.pad_pixel[2] = 0;
    dofPreProcObj->params.pad_pixel[3] = 0;

    dofPreProcObj->params.tidl_8bit_16bit_flag  = 0;
}

static void update_dof_pre_proc_params(AppObj *obj, DofPreProcObj *dofPreProcObj)
{
    dofPreProcObj->params.width  = obj->scalerObj.output.width;
    dofPreProcObj->params.height = obj->scalerObj.output.height;
}

static void set_post_proc_defaults_od(PostProcObj *postProcObj)
{
    /*Focal length is hard coded for the given camera property. It can be exposed by cfg*/
    postProcObj->params.num_max_det = 100;
    postProcObj->params.points_per_line = 8;
    postProcObj->params.num_keypoints = 4;
    postProcObj->params.focal_length = 311.833f;
    postProcObj->params.inter_center_x_fact = 10;
    postProcObj->params.inter_center_y_fact = 2;
    postProcObj->params.num_table_rows = ANGLE_TABLE_ROWS;
    postProcObj->params.output_buffer_offset = 0;
}

static void update_post_proc_params_od(AppObj *obj, PostProcObj *postProcObj)
{
    postProcObj->params.width     = obj->scalerObj.input.width;
    postProcObj->params.height    = obj->scalerObj.input.height;
    postProcObj->params.dl_width  = obj->scalerObj.output.width;
    postProcObj->params.dl_height = obj->scalerObj.output.height;
    postProcObj->params.center_x  = postProcObj->params.width/2;
    postProcObj->params.center_y  = postProcObj->params.height/2;
}

static void set_post_proc_defaults_pc(PostProcObj *postProcObj)
{
    int32_t i;

    postProcObj->params.width     = 1280;
    postProcObj->params.height    = 720;
    postProcObj->params.dl_width  = 768;
    postProcObj->params.dl_height = 384;

    postProcObj->viz_params.ip_rgb_or_yuv = 1;
    postProcObj->viz_params.op_rgb_or_yuv = 1;

    postProcObj->viz_params.num_input_tensors  = 2;
    postProcObj->viz_params.num_output_tensors = 3;

    postProcObj->viz_params.tidl_8bit_16bit_flag = 0;

    postProcObj->viz_params.num_classes[0] = 5; //5 classes for semantic segmentation
    postProcObj->viz_params.num_classes[1] = 2; //2 class - motion / no-motion
    postProcObj->viz_params.num_classes[2] = 63; //63 values - depth range

    for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++)
    {
        postProcObj->viz_params.valid_region[i][0] = 0;
        postProcObj->viz_params.valid_region[i][1] = 0;
        postProcObj->viz_params.valid_region[i][2] = postProcObj->params.dl_width - 1;
        postProcObj->viz_params.valid_region[i][3] = postProcObj->params.dl_height - 1;
    }
}

static void update_post_proc_params_pc(AppObj *obj, PostProcObj *postProcObj)
{
    vx_int32 i, j;

    postProcObj->params.width     = obj->scalerObj.output.width;
    postProcObj->params.height    = obj->scalerObj.output.height;
    postProcObj->params.dl_width  = obj->scalerObj.output.width;
    postProcObj->params.dl_height = obj->scalerObj.output.height;

    for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++)
    {
        postProcObj->viz_params.num_classes[i] = obj->num_classes[i];
    }

    postProcObj->viz_params.ip_rgb_or_yuv = obj->ip_rgb_or_yuv;
    postProcObj->viz_params.op_rgb_or_yuv = obj->op_rgb_or_yuv;

    for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++)
    {
        if(obj->valid_region[i][0] != -1)
            postProcObj->viz_params.valid_region[i][0] = obj->valid_region[i][0];
        if(obj->valid_region[i][1] != -1)
            postProcObj->viz_params.valid_region[i][1] = obj->valid_region[i][1];
        if(obj->valid_region[i][2] != -1)
            postProcObj->viz_params.valid_region[i][2] = obj->valid_region[i][2];
        if(obj->valid_region[i][3] != -1)
            postProcObj->viz_params.valid_region[i][3] = obj->valid_region[i][3];
    }

    for(i=0; i < TIVX_PIXEL_VIZ_MAX_TENSOR; i++)
    {
        /*Motion segmentation scenario*/
        if(postProcObj->viz_params.num_classes[i] == 2)
        {
            memcpy(postProcObj->viz_params.color_map[i], motion_segment_color_map, postProcObj->viz_params.num_classes[i]*3*sizeof(vx_uint8));
            postProcObj->viz_params.max_value[i] = postProcObj->viz_params.num_classes[i] - 1;
        }
        /*Semantic segmentation scenario*/
        else if(postProcObj->viz_params.num_classes[i] == 5)
        {
            memcpy(postProcObj->viz_params.color_map[i], semantic_segment_color_map, postProcObj->viz_params.num_classes[i]*3*sizeof(vx_uint8));
            postProcObj->viz_params.max_value[i] = postProcObj->viz_params.num_classes[i] - 1;
        }
        /*depth class*/
        else
        {
            float color_jump = 63.0/postProcObj->viz_params.num_classes[i];
            for(j = 0; j < postProcObj->viz_params.num_classes[i] ; j++)
            {
                postProcObj->viz_params.color_map[i][j][0] = depth_segment_color_map[(int)(j * color_jump)][0];
                postProcObj->viz_params.color_map[i][j][1] = depth_segment_color_map[(int)(j * color_jump)][1];
                postProcObj->viz_params.color_map[i][j][2] = depth_segment_color_map[(int)(j * color_jump)][2];
            }
            /*For depth network maximum value is 20m*/
            /*for depth this is different from num_classes. num_classes is used for visualization granuaility*/
            postProcObj->viz_params.max_value[i] = 20;
        }
    }

}

static void set_draw_detection_defaults(DrawDetectionsObj *drawDetectionsObj)
{
    drawDetectionsObj->params.width     = 1280;
    drawDetectionsObj->params.height    = 720;
    drawDetectionsObj->params.dl_width  = 768;
    drawDetectionsObj->params.dl_height = 384;

    drawDetectionsObj->params.num_max_det = 100;
    drawDetectionsObj->params.points_per_line = 8;
    drawDetectionsObj->params.num_keypoints = 4;
    drawDetectionsObj->params.viz_th = 0.5F;

}

static void app_update_draw_detections_params(DrawDetectionsObj *drawDetectionsObj, vx_float32 viz_th)
{
    drawDetectionsObj->params.viz_th = viz_th;
}

static void set_img_mosaic_frame2(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx;
    vx_int32 in;
    imgMosaicObj->out_width    = 1920;
    imgMosaicObj->out_height   = 1080;
    imgMosaicObj->num_inputs   = NUM_ALGOS;

    in  = 0;
    idx = 0;
    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 5;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 5;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 5;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 3;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 3;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 3;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 4;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 4;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 4;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = 4;
}

static void set_img_mosaic_frame1(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx;
    vx_int32 in;
    imgMosaicObj->out_width    = 1920;
    imgMosaicObj->out_height   = 1080;
    imgMosaicObj->num_inputs   = NUM_ALGOS;

    in  = 0;
    idx = 0;
    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 2;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 2;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 120;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 2;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 412;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    {

        imgMosaicObj->params.windows[idx].startX  = 120;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 1;
        imgMosaicObj->params.windows[idx].channel_select = 2;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 700;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 1;
        imgMosaicObj->params.windows[idx].channel_select = 0;
        idx++;

        imgMosaicObj->params.windows[idx].startX  = 1280;
        imgMosaicObj->params.windows[idx].startY  = 704;
        imgMosaicObj->params.windows[idx].width   = 576;
        imgMosaicObj->params.windows[idx].height  = 288;
        imgMosaicObj->params.windows[idx].input_select   = 1;
        imgMosaicObj->params.windows[idx].channel_select = 1;
        idx++;

        in++;
    }

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = 4;
}

static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 0;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->pipeline       = -AVP_BUFFER_Q_DEPTH;
    obj->enqueueCnt     = 0;
    obj->dequeueCnt     = 0;
}

static void set_dof_pyramid_defaults(PyramidObj *pyramidObj)
{
    pyramidObj->width  = 768;
    pyramidObj->height = 384;
    pyramidObj->dof_levels = 4;
    pyramidObj->vx_df_pyramid = VX_DF_IMAGE_U8;
}


static void set_dof_proc_defaults(DofProcObj *dofProcObj)
{
    /* Valid combinations:
     *
     * 1) enable_temporal_predicton_flow_vector = 0. This will force enable_flow_vec_delay_obj
     *    to 0.
     * 2) enable_temporal_predicton_flow_vector = 1 && enable_flow_vec_delay_obj = 1
     *    - This enables external delay object and loops back the flow vector output
     *      to the input through the delay.
     * 3) enable_temporal_predicton_flow_vector = 1 && enable_flow_vec_delay_obj = 0
     *    - This disables the external delay object and based on the DOF node
     *      configuration parameter, flow_vector_internal_delay_num, an internal
     *      history mechanism will be setup. This internal mechanism will maintain
     *      a history of previous flow vectors to use.
     */
    dofProcObj->enable_temporal_predicton_flow_vector = 0;
    dofProcObj->enable_flow_vec_delay_obj = 0;

    dofProcObj->width  = 768;
    dofProcObj->height = 384;
}

static void set_dof_viz_defaults(DofVizObj *dofVizObj)
{
    dofVizObj->confidence_threshold_value = 1;
    dofVizObj->file_format = APP_FILE_FORMAT_YUV;
    dofVizObj->width  = 768;
    dofVizObj->height = 384;
}

static void app_default_param_set(AppObj *obj)
{
    set_dof_pyramid_defaults(&obj->pyramidObj);
    set_dof_proc_defaults(&obj->dofProcObj);

    set_dof_pre_proc_defaults(&obj->dofPreProcObj);
    set_pre_proc_defaults(&obj->preProcObj);

    set_post_proc_defaults_od(&obj->odPostProcObj);
    set_post_proc_defaults_pc(&obj->pcPostProcObj);

    set_draw_detection_defaults(&obj->psdDrawDetectionsObj);
    obj->psdDrawDetectionsObj.params.num_classes = 2;
    obj->psdDrawDetectionsObj.params.class_id[0] = 1;
    obj->psdDrawDetectionsObj.params.class_id[1] = 2;

    set_draw_detection_defaults(&obj->vdDrawDetectionsObj);
    obj->vdDrawDetectionsObj.params.num_classes = 1;
    obj->vdDrawDetectionsObj.params.class_id[0] = 3;

    set_dof_viz_defaults(&obj->dofVizObj);

    tivxImgMosaicParamsSetDefaults(&obj->imgMosaicObj.params);

    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->delay_in_msecs = 0;
    obj->num_iterations = 1;
    obj->is_interactive = 0;
    obj->test_mode      = 0;
    obj->enable_gui     = 1;

    obj->enable_psd     = 1;
    obj->enable_vd      = 1;
    obj->enable_sem_seg = 1;

}

static void app_update_param_set(AppObj *obj)
{
    update_dof_pre_proc_params(obj, &obj->dofPreProcObj);
    update_pre_proc_params(obj, &obj->preProcObj);

    update_post_proc_params_od(obj, &obj->odPostProcObj);
    update_post_proc_params_pc(obj, &obj->pcPostProcObj);

    app_update_draw_detections_params(&obj->psdDrawDetectionsObj, obj->viz_th_pd);
    app_update_draw_detections_params(&obj->vdDrawDetectionsObj, obj->viz_th_vd);

    set_img_mosaic_frame1(obj, &obj->imgMosaicObj);

    if(obj->is_interactive)
    {
        obj->num_iterations = 1000000000;
    }
}

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

#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{

    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;
        Draw2D_FontPrm sLabel;

        sHeading.fontIdx = 0;
        Draw2D_drawString(handle, 560, 5, "Analytics for Auto Valet Parking", &sHeading);

        sLabel.fontIdx = 2;
        Draw2D_drawString(handle, 340, 90, "Left camera", &sLabel);
        Draw2D_drawString(handle, 920, 90, "Front camera", &sLabel);
        Draw2D_drawString(handle, 1520, 90, "Right camera", &sLabel);
    }

    if(update_type == 1)
    {
        Draw2D_FontPrm sLabel;
        sLabel.fontIdx = 3;

        if(g_frame_switch == 0)
        {
            Draw2D_clearString(handle, 0, 204, 60, &sLabel);
            Draw2D_clearString(handle, 0, 224, 60, &sLabel);
            Draw2D_clearString(handle, 0, 244, 60, &sLabel);
            Draw2D_drawString(handle, 20, 224, "Semantic", &sLabel);
            Draw2D_drawString(handle, 0, 244, "Segmentation", &sLabel);

            Draw2D_clearString(handle, 0, 526, 60, &sLabel);
            Draw2D_clearString(handle, 0, 546, 60, &sLabel);
            Draw2D_clearString(handle, 0, 566, 60, &sLabel);
            Draw2D_drawString(handle, 20, 526, "Parking", &sLabel);
            Draw2D_drawString(handle, 35, 546, "Spot", &sLabel);
            Draw2D_drawString(handle, 10, 566, "Detection", &sLabel);

            Draw2D_clearString(handle, 0, 828, 60, &sLabel);
            Draw2D_clearString(handle, 0, 848, 60, &sLabel);
            Draw2D_drawString(handle, 25, 828, "Vehicle", &sLabel);
            Draw2D_drawString(handle, 15, 848, "Detection", &sLabel);
        }
        else
        {
            Draw2D_clearString(handle, 0, 204, 60, &sLabel);
            Draw2D_clearString(handle, 0, 224, 60, &sLabel);
            Draw2D_clearString(handle, 0, 244, 60, &sLabel);
            Draw2D_drawString(handle, 30, 204, "Dense", &sLabel);
            Draw2D_drawString(handle, 25, 224, "Optical", &sLabel);
            Draw2D_drawString(handle, 35, 244, "Flow", &sLabel);

            Draw2D_clearString(handle, 0, 526, 60, &sLabel);
            Draw2D_clearString(handle, 0, 546, 60, &sLabel);
            Draw2D_clearString(handle, 0, 566, 60, &sLabel);
            Draw2D_drawString(handle, 25, 526, "Motion", &sLabel);
            Draw2D_drawString(handle, 0, 546, "Segmentation", &sLabel);

            Draw2D_clearString(handle, 0, 828, 60, &sLabel);
            Draw2D_clearString(handle, 0, 848, 60, &sLabel);
            Draw2D_drawString(handle, 30, 828, "Depth", &sLabel);
            Draw2D_drawString(handle, 10, 848, "Estimation", &sLabel);
        }
    }

    return;
}
#endif
