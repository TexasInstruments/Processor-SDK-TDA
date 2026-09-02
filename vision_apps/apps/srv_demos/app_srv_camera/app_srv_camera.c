/*
 *  Copyright (c) Texas Instruments Incorporated 2019
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(LINUX) || defined(QNX)
#include <sys/stat.h>
#endif
#include <assert.h>

#include <TI/tivx.h>
#include <TI/j7_imaging_aewb.h>
#include <TI/video_io_kernels.h>
#include <TI/video_io_capture.h>
#include <TI/video_io_display.h>

#include <VX/vxu.h>
#include <VX/vx_khr_pipelining.h>
#include "math.h"
#include "tivx_utils_file_rd_wr.h"
#include <limits.h>
#include <TI/tivx_srv.h>
#include "srv_bowl_lut_gen_applib/srv_bowl_lut_gen_applib.h"
#include "lens_distortion_correction.h"
#include "srv_common.h"
#include <TI/tivx_task.h>
#include <render.h>
#include <tivx_utils_graph_perf.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <app_init.h>
#include "app_common.h"
#include "app_test.h"

#include <utils/sensors/include/app_sensors.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/iss/include/app_iss.h>
#include <utils/grpx/include/app_grpx.h>

#include <iss_sensors.h>
#include <iss_sensor_if.h>

#define MAX_ABS_FILENAME          (1024u)
#define MAXPATHLENGTH             (512u)

#define SV_OUT_DISPLAY_HEIGHT   (1080)
#define SV_OUT_DISPLAY_WIDTH    (1080)
#define SV_SUBSAMPLE            (4)
#define SV_XYZLUT3D_SIZE        (SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE) * (SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) *3
#define SV_GPULUT_SIZE          ((2 + SV_OUT_DISPLAY_WIDTH/SV_SUBSAMPLE) * (2 + SV_OUT_DISPLAY_HEIGHT/SV_SUBSAMPLE)) *7


#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

#define MAX_NUM_BUF                         (8u)
#define NUM_CAPT_CHANNELS                   (4U)
#define CHANNEL_SWITCH_FRAME_COUNT          (300u)
#define NUM_BUFS                            (4u)
#define PIPE_DEPTH                          (4u)
#define NUM_INTERMEDIATE_BUFS               (3u)
#define CAPT_INST_ID                        (0U)

#define DISPLAY_WIDTH_2MP                   (1920u)
#define DISPLAY_HEIGHT_2MP                  (1080u)

typedef struct {

    /* config options */
    vx_uint32 inWidth;
    vx_uint32 inHeight;
    vx_uint32 outWidth;
    vx_uint32 outHeight;
    uint32_t num_views;

    int32_t offsetXleft;
    int32_t offsetXright;
    int32_t offsetYfront;
    int32_t offsetYback;

    float camx[MAX_SRV_VIEWS];
    float camy[MAX_SRV_VIEWS];
    float camz[MAX_SRV_VIEWS];
    float targetx[MAX_SRV_VIEWS];
    float targety[MAX_SRV_VIEWS];
    float targetz[MAX_SRV_VIEWS];
    float anglex[MAX_SRV_VIEWS];
    float angley[MAX_SRV_VIEWS];
    float anglez[MAX_SRV_VIEWS];

    /* OpenVX Types */
    vx_context context;
    vx_graph graph_gpu_lut;

    /* GPU LUT Applib API objects */
    vx_user_data_object         in_config;
    vx_user_data_object         in_calmat_object;
    vx_user_data_object         in_offset_object;
    vx_user_data_object         in_lens_param_object;
    vx_array                    out_gpulut_array;

    /* GPU LUT Applib params */
    srv_bowl_lut_gen_handle srv_handle;
    srv_bowl_lut_gen_createParams create_params;

    /* OpenGL SRV node objects */
    vx_node srv_node;
    vx_object_array srv_views_array;
    vx_image output_image[MAX_NUM_BUF];
    vx_user_data_object param_obj;
    vx_user_data_object srv_views;

    uint32_t is_enable_gui;
    uint32_t is_interactive;
    uint32_t test_mode;
    uint32_t displayNodeGraphParamNum;

    uint32_t is_enable_yuyv;
    tivx_task task;
    uint32_t stop_task;
    uint32_t stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;


    /* Multicam objects */
    tivx_raw_image sample_raw_img;
    tivx_raw_image raw_img;
    vx_object_array capt_frames[MAX_NUM_BUF];
    vx_user_data_object display_param_obj;
    vx_graph graph;
    vx_node displayNode;
    vx_node captureNode;
    vx_user_data_object capture_param_obj;
    vx_user_data_object switch_ch_obj;
    tivx_display_select_channel_params_t channel_prms;
    vx_reference refs[1];
    tivx_raw_image fs_test_raw_image;

    /* VISS Objects */
    vx_image sample_nv12_img;
    vx_image viss_nv12_out_img;
    vx_node vissNode;
    vx_node node_aewb;
    vx_user_data_object viss_configuration;
    vx_distribution histogram;
    vx_object_array histogram_arr;
    vx_object_array ae_awb_result_arr;
    vx_user_data_object ae_awb_result;
    vx_object_array viss_out_frames;
    vx_user_data_object dcc_param_viss;

    /* Aewb objects */
    vx_object_array h3a_aew_af_arr;
    vx_user_data_object h3a_aew_af;
    tivx_aewb_config_t aewb_cfg;
    tivx_ae_awb_params_t ae_awb_params;
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;
    vx_object_array aewb_config_array;

    uint32_t cam_dcc_id;
    char *sensor_name;
    vx_uint8 sensor_sel;
} AppObj;

AppObj gAppObj;

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[]);
static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static int app_create_graphs(AppObj *obj);
static int app_create_graph_gpu_lut(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_run_graph_gpu_lut(AppObj *obj);
static void app_delete_graph(AppObj *obj);
tivx_raw_image read_test_image_raw(vx_context context,
                             IssSensor_Info *sensorInfo, char raw_image_fname[],
                             vx_int32 *bytes_read);
vx_status app_send_test_frame(vx_node cap_node, tivx_raw_image raw_img);
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif

static char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
static vx_uint8 num_sensors_found;
static IssSensor_CreateParams sensorParams;

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,
    vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static char *get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static void app_run_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;

    appPerfStatsResetAll();

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

static const char menu[] = {
    "\n"
    "\n =========================="
    "\n Demo : Integrated SRV"
    "\n =========================="
    "\n"
    "\n p: Print performance statistics"
    "\n"
    "\n e: Export performance statistics"
    "\n"
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};


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
        printf("ERROR: Unable to create task\n");
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
                    appPerfPointPrint(&obj->total_perf);
                    printf("\n");
                    appPerfPointPrintFPS(&obj->total_perf);
                    appPerfPointReset(&obj->total_perf);
                    printf("\n");
                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "apps_srv_demos_app_srv_camera");
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

#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{

  appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

  if(update_type == 0)
  {
    Draw2D_FontPrm sHeading;
    Draw2D_FontPrm sLabel;

    sHeading.fontIdx = 4;
    Draw2D_drawString(handle, 615, 5, "360 Degree Surround View", &sHeading);

    sLabel.fontIdx = 2;
    Draw2D_drawString(handle, 1375, 5, " Input: 4CH 1920x1080 (2MP) @ 30fps", &sLabel);

    sLabel.fontIdx = 2;
    Draw2D_drawString(handle, 1375, 35, "Output: 1920x1080 (2MP) @ 30fps", &sLabel);

    // Add Graphic Model info
    sLabel.fontIdx = 2;
    Draw2D_drawString(handle, 1375, 65, "Graphics Model Triangles: 213.3K", &sLabel);
    Draw2D_drawString(handle, 1375, 95, "Graphics Model Vertices: 640K", &sLabel);
  }

  return;
}
#endif

static void read_calmat_file( svCalmat_t *calmat, const char*fileName)
{
    char file[MAXPATHLENGTH];
    uint32_t cnt;
    FILE* f = 0;
    size_t sz;
    uint32_t  read_size;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    printf ("Reading calmat file \n");

    if (!fileName)
    {
        printf("calmat file name not specified\n");
        return;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", test_data_path, fileName);
    if (sz > MAXPATHLENGTH)
    {
        return;
    }

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Cant open calmat file: %s\n", fileName);
        return;
    }

    read_size = fread((int8_t *)&calmat->numCameras,sizeof(uint8_t),4,f);
    if (read_size != 4)
    {
        printf("Incorrect Bytes read from calmat file: %s\n", fileName);
        fclose(f);
        return;
    }

    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        read_size = fread((int8_t *)&calmat->calMatSize[cnt],sizeof(uint8_t),4,f);
        if (read_size != 4)
        {
            printf("Incorrect Bytes read from calmat file: %s\n", fileName);
            fclose(f);
            return;
        }
    }

    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        APP_PRINTF("Calmat size for cnt %d = %d \n",cnt,calmat->calMatSize[cnt]);
    }

    /* Set Pointer ahead by 128 bytes to skip over metadata */
    fseek(f,128,SEEK_SET);

    /* Read calmat per camera */
    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        read_size = fread((int8_t *)&calmat->calMatBuf + 48*cnt,sizeof(uint8_t),calmat->calMatSize[cnt],f);
        if (read_size != calmat->calMatSize[cnt])
        {
            printf("Incorrect Bytes read from calmat file: %s\n", fileName);
            fclose(f);
            return;
        }
    }

    for (cnt=0;cnt<calmat->numCameras;cnt++) {
        APP_PRINTF("For Camera = %d Ref calmat[0] = %d Ref Calmat[11] = %d \n",cnt,calmat->calMatBuf[12*cnt+0],calmat->calMatBuf[12*cnt +11]);
    }

    printf ("file read completed \n");
    fclose(f);

}

static void read_lut_file(ldc_lensParameters *ldcParams, const char*fileName)
{
    char file[MAXPATHLENGTH];
    uint32_t  read_size;
    FILE* f = 0;
    size_t sz;
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = get_test_file_path();

    if (!fileName)
    {
        printf("lut file name not specified\n");
        return;
    }

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

#if defined(LINUX) || defined(QNX)
    {
        struct stat s;
        if (stat(test_data_path, &s))
        {
            printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
            test_data_path = failsafe_test_data_path;
        }
    }
#endif

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", test_data_path, fileName);
    if (sz > MAXPATHLENGTH)
    {
        return;
    }

    f = fopen(file, "rb");
    if (!f)
    {
        printf("Can't open LUT file: %s\n", fileName);
        return;
    }

    read_size = fread((uint8_t *)ldcParams,sizeof(uint8_t),sizeof(ldc_lensParameters),f);
    if (read_size != sizeof(ldc_lensParameters))
    {
        printf("Incorrect Bytes read from  LUT file: %s\n", fileName);
        fclose(f);
        return;
    }

    fclose(f);
}

static LDC_status LDC_Init(LensDistortionCorrection* ldc,
                        dtype distCenterX, dtype distCenterY, dtype distFocalLength,
                        dtype *lut_d2u, vx_int32 lut_d2u_length, dtype lut_d2u_step,
                        dtype *lut_u2d, vx_int32 lut_u2d_length, dtype lut_u2d_step)
{
    /*FLOATING TYPE*/
    /*distortion center*/
    ldc->distCenterX = distCenterX;
    ldc->distCenterY = distCenterY;
    ldc->distFocalLength = distFocalLength;
    ldc->distFocalLengthInv = 1/ldc->distFocalLength;
    /*ldc look-up table parameters*/
    ldc->lut_d2u_indMax = lut_d2u_length-1;
    ldc->lut_d2u_step = lut_d2u_step;
    ldc->lut_u2d_indMax = lut_u2d_length - 1;
    ldc->lut_u2d_step = lut_u2d_step;

    ldc->lut_d2u_stepInv = 1/ldc->lut_d2u_step;
    ldc->lut_u2d_stepInv = 1/ldc->lut_u2d_step;


    /*ldc look-up table pointers*/
    memcpy (ldc->lut_d2u, (uint8_t *)lut_d2u,(sizeof(dtype)*LDC_D2U_TABLE_MAX_LENGTH) );
    memcpy (ldc->lut_u2d, (uint8_t *)lut_u2d,(sizeof(dtype)*LDC_U2D_TABLE_MAX_LENGTH) );

    return LDC_STATUS_OK;
}


static void app_show_usage(int argc, char* argv[])
{
    printf("\n");
    printf(" SRV Demo - (c) Texas Instruments 2019\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static void app_set_cfg_default(AppObj *obj)
{
    obj->test_mode = 0;
    obj->is_interactive = 1;
    obj->is_enable_gui = 1;
    obj->is_enable_yuyv = 0;
    #if defined(x86_64)
    obj->is_interactive = 0;
    #endif
}

static void app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;
    uint32_t camx_idx = 0, camy_idx = 0, camz_idx = 0;
    uint32_t targetx_idx = 0, targety_idx = 0, targetz_idx = 0;
    uint32_t anglex_idx = 0, angley_idx = 0, anglez_idx = 0;

    if(fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]\n", cfg_file_name);
        exit(0);
    }

    while(fgets(line_str, sizeof(line_str), fp)!=NULL)
    {
        char s[]=" \t";

        if (strchr(line_str, '#'))
        {
            continue;
        }

        /* get the first token */
        token = strtok(line_str, s);

        if (NULL != token)
        {
            if(strcmp(token, "sensor_index")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->sensor_sel = atoi(token);
                }
            }
            else if(strcmp(token, "offsetXleft")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetXleft = atoi(token);
                }
            }
            else if(strcmp(token, "offsetXright")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetXright = atoi(token);
                }
            }
            else if(strcmp(token, "offsetYfront")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetYfront = atoi(token);
                }
            }
            else if(strcmp(token, "offsetYback")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->offsetYback = atoi(token);
                }
            }
            else if(strcmp(token, "num_views")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->num_views = atoi(token);
                    if (obj->num_views > MAX_SRV_VIEWS)
                    {
                        printf("Config file num_views argument exceeds MAX_SRV_VIEWS = %d\n", MAX_SRV_VIEWS);
                        exit(0);
                    }
                }
            }
            else if(strcmp(token, "camx")==0)
            {
                if (obj->num_views < (camx_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camx[camx_idx] = atof(token);
                    APP_PRINTF("obj->camx[%d] = %f\n", camx_idx, obj->camx[camx_idx]);
                    camx_idx++;
                }
            }
            else if(strcmp(token, "camy")==0)
            {
                if (obj->num_views < (camy_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camy[camy_idx] = atof(token);
                    APP_PRINTF("obj->camy[%d] = %f\n", camy_idx, obj->camy[camy_idx]);
                    camy_idx++;
                }
            }
            else if(strcmp(token, "camz")==0)
            {
                if (obj->num_views < (camz_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->camz[camz_idx] = atof(token);
                    APP_PRINTF("obj->camz[%d] = %f\n", camz_idx, obj->camz[camz_idx]);
                    camz_idx++;
                }
            }
            else if(strcmp(token, "targetx")==0)
            {
                if (obj->num_views < (targetx_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targetx[targetx_idx] = atof(token);
                    APP_PRINTF("obj->targetx[%d] = %f\n", targetx_idx, obj->targetx[targetx_idx]);
                    targetx_idx++;
                }
            }
            else if(strcmp(token, "targety")==0)
            {
                if (obj->num_views < (targety_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targety[targety_idx] = atof(token);
                    APP_PRINTF("obj->targety[%d] = %f\n", targety_idx, obj->targety[targety_idx]);
                    targety_idx++;
                }
            }
            else if(strcmp(token, "targetz")==0)
            {
                if (obj->num_views < (targetz_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->targetz[targetz_idx] = atof(token);
                    APP_PRINTF("obj->targetz[%d] = %f\n", targetz_idx, obj->targetz[targetz_idx]);
                    targetz_idx++;
                }
            }
            else if(strcmp(token, "anglex")==0)
            {
                if (obj->num_views < (anglex_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->anglex[anglex_idx] = atof(token);
                    APP_PRINTF("obj->anglex[%d] = %f\n", anglex_idx, obj->anglex[anglex_idx]);
                    anglex_idx++;
                }
            }
            else if(strcmp(token, "angley")==0)
            {
                if (obj->num_views < (angley_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->angley[angley_idx] = atof(token);
                    APP_PRINTF("obj->angley[%d] = %f\n", angley_idx, obj->angley[angley_idx]);
                    angley_idx++;
                }
            }
            else if(strcmp(token, "anglez")==0)
            {
                if (obj->num_views < (anglez_idx+1))
                {
                    printf("Mismatch between number of views and view parameters!!!\n");
                    exit(0);
                }

                token = strtok(NULL, s);
                if (NULL != token)
                {
                    obj->anglez[anglez_idx] = atof(token);
                    APP_PRINTF("obj->anglez[%d] = %f\n", anglez_idx, obj->anglez[anglez_idx]);
                    anglez_idx++;
                }
            }
            else
            if(strcmp(token, "is_interactive")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    obj->is_interactive = atoi(token);
                    if(obj->is_interactive > 1)
                        obj->is_interactive = 1;
                }
            }
            else
            if(strcmp(token, "is_enable_yuyv")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    obj->is_enable_yuyv = atoi(token);
                    if(obj->is_enable_yuyv > 1)
                        obj->is_enable_yuyv = 1;
                }
            }
            else
            if(strcmp(token, "is_enable_gui")==0)
            {
                token = strtok(NULL, s);
                if (NULL != token)
                {
                    token[strlen(token)-1]=0;
                    obj->is_enable_gui = atoi(token);
                    if(obj->is_enable_gui > 1)
                        obj->is_enable_gui = 1;
                }
            }
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    int i;
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
    if(set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->is_interactive = 0;
    }
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_FAILURE;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 count = 0;
    vx_uint8 selectedSensor = 0xFF;

    app_grpx_init_prms_t grpx_prms;

    obj->stop_task = 0;
    obj->stop_task_done = 0;

    appInit();

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    tivxSrvLoadKernels(obj->context);
    tivxHwaLoadKernels(obj->context);
    tivxVideoIOLoadKernels(obj->context);
    tivxImagingLoadKernels(obj->context);
    APP_PRINTF("tivxImagingLoadKernels done\n");

    if(obj->is_enable_gui)
    {
        appGrpxInitParamsInit(&grpx_prms, obj->context);
        grpx_prms.draw_callback = app_draw_graphics;
        appGrpxInit(&grpx_prms);
    }

    memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
    for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
    {
        sensor_list[count] = availableSensorNames[count];
    }
    status = appEnumerateImageSensor(sensor_list, &num_sensors_found);
    if(VX_SUCCESS != status)
    {
        printf("appCreateImageSensor returned %d\n", status);
        return status;
    }
    else
    {
        selectedSensor = obj->sensor_sel;
        if(selectedSensor > (num_sensors_found-1))
        {
            printf("Invalid sensor selection %d \n", selectedSensor);
            return VX_FAILURE;
        }
    }
    obj->sensor_name = sensor_list[selectedSensor];
    printf("Sensor selected : %s\n", obj->sensor_name);

    appPerfPointSetName(&obj->total_perf , "TOTAL");

    return status;
}

static vx_status app_create_capture(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    tivx_capture_params_t capture_params;
    uint32_t buf_id, loop_id;

    /* Creating exemplar to use for raw image */
    obj->sample_raw_img = tivxCreateRawImage(obj->context, &(sensorParams.sensorInfo.raw_params));
    if (vxGetStatus((vx_reference)obj->sample_raw_img) != VX_SUCCESS)
    {
        APP_PRINTF("sample_raw_img create failed\n");
    }

    /* Creating object arrays to be used as output to capture node */
    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        obj->capt_frames[buf_id] =
            vxCreateObjectArray(obj->context,
            (vx_reference)obj->sample_raw_img, NUM_CAPT_CHANNELS);
        if (vxGetStatus((vx_reference)obj->capt_frames[buf_id]) != VX_SUCCESS)
        {
            APP_PRINTF("obj->capt_frames[buf_id] create failed\n");
            return VX_FAILURE;
        }
    }

    tivxReleaseRawImage(&obj->sample_raw_img);

    /* Capture initialization */
    tivx_capture_params_init(&capture_params);
    capture_params.numInst                          = 1U;
    capture_params.numCh                            = NUM_CAPT_CHANNELS;
    capture_params.instId[0U]                       = CAPT_INST_ID;
    capture_params.instCfg[0U].enableCsiv2p0Support = (uint32_t)vx_true_e;
    capture_params.instCfg[0U].numDataLanes         = sensorParams.sensorInfo.numDataLanes;
    if(obj->test_mode == 1)
    {
        capture_params.timeout = 33;
        capture_params.timeoutInitial = 500;
    }
    for (loop_id=0U; loop_id<capture_params.instCfg[0U].numDataLanes; loop_id++)
    {
        capture_params.instCfg[0U].dataLanesMap[loop_id] = sensorParams.sensorInfo.dataLanesMap[loop_id];
    }

    obj->capture_param_obj =
        vxCreateUserDataObject(obj->context, "tivx_capture_params_t" ,
        sizeof(tivx_capture_params_t), &capture_params);
    if (vxGetStatus((vx_reference)obj->capture_param_obj) != VX_SUCCESS)
    {
        APP_PRINTF("capture_param_obj create failed\n");
        return VX_FAILURE;
    }

    obj->captureNode = tivxCaptureNode(obj->graph, obj->capture_param_obj, obj->capt_frames[0]);
    if (vxGetStatus((vx_reference)obj->captureNode) != VX_SUCCESS)
    {
        APP_PRINTF("captureNode create failed\n");
        return VX_FAILURE;
    }
    vxSetNodeTarget(obj->captureNode, VX_TARGET_STRING,
        TIVX_TARGET_CAPTURE1);
    vxSetReferenceName((vx_reference)obj->captureNode, "Capture_node");

    return status;
}

/* Creating viss and aewb nodes */
static vx_status app_create_viss_aewb(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    tivx_vpac_viss_params_t viss_params;
    tivx_ae_awb_params_t ae_awb_params;
    uint32_t loop_id;

    /* Sensor Params */
    uint32_t sensor_features_enabled = 0;
    uint32_t sensor_features_supported = 0;
    uint32_t sensor_dcc_enabled = 0;
    uint32_t sensor_wdr_enabled = 0;
    uint32_t sensor_exp_control_enabled = 0;
    uint32_t sensor_gain_control_enabled = 0;

    /* DCC Params */
    vx_size dcc_buff_size;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    uint8_t * dcc_viss_buf;
    vx_map_id dcc_viss_buf_map_id;
    vx_distribution histogram_exemplar;
    vx_user_data_object ae_awb_result_exemplar;
    vx_bool viss_prms_replicate[] =
        {vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e,
         vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e};
    vx_bool aewb_prms_replicate[] =
        {vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e};

    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    vx_map_id dcc_2a_buf_map_id;
    uint8_t * dcc_2a_buf;
    vx_user_data_object h3a_aew_af_exemplar;
    vx_user_data_object aewb_config_exemplar;

    /*
    Check for supported sensor features.
    It is upto the application to decide which features should be enabled.
    This demo app enables WDR, DCC and 2A if the sensor supports it.
    */
    sensor_features_supported = sensorParams.sensorInfo.features;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        APP_PRINTF("WDR mode is supported \n");
        sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
        sensor_wdr_enabled = 1;
    }else
    {
        APP_PRINTF("WDR mode is not supported. Defaulting to linear \n");
        sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
        sensor_wdr_enabled = 0;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        APP_PRINTF("Expsoure control is supported \n");
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
        sensor_exp_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        APP_PRINTF("Gain control is supported \n");
        sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
        sensor_gain_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
        sensor_dcc_enabled = 1;
    }else
    {
        sensor_dcc_enabled = 0;
    }

    APP_PRINTF("Sensor width = %d\n", sensorParams.sensorInfo.raw_params.width);
    APP_PRINTF("Sensor height = %d\n", sensorParams.sensorInfo.raw_params.height);
    APP_PRINTF("Sensor DCC ID = %d\n", sensorParams.dccId);
    APP_PRINTF("Sensor Supported Features = 0x%x\n", sensor_features_supported);
    APP_PRINTF("Sensor Enabled Features = 0x%x\n", sensor_features_enabled);
    appInitImageSensor(obj->sensor_name, sensor_features_enabled, 0xF);/*Mask = 0xF for 4 cameras*/

    /* Allocate sample NV12 image, using which object array of NV12
     * would be created */
    if (0 == obj->is_enable_yuyv)
    {
        obj->sample_nv12_img =
            vxCreateImage(obj->context, obj->inWidth, obj->inHeight,
            VX_DF_IMAGE_NV12);
    }
    else
    {
        obj->sample_nv12_img =
            vxCreateImage(obj->context, obj->inWidth, obj->inHeight,
            VX_DF_IMAGE_YUYV);
    }

    if (vxGetStatus((vx_reference)obj->sample_nv12_img) != VX_SUCCESS)
    {
        APP_PRINTF("sample_nv12_img create failed\n");
        return VX_FAILURE;
    }

    /* Allocate object array for the output frames */
    obj->viss_out_frames = vxCreateObjectArray(obj->context,
        (vx_reference)obj->sample_nv12_img, NUM_CAPT_CHANNELS);
    if (vxGetStatus((vx_reference)obj->viss_out_frames) != VX_SUCCESS)
    {
        APP_PRINTF("viss_out_frames create failed\n");
        return VX_FAILURE;
    }

    obj->viss_nv12_out_img = (vx_image)vxGetObjectArrayItem(obj->viss_out_frames, 0);
    if (vxGetStatus((vx_reference)obj->viss_nv12_out_img) != VX_SUCCESS)
    {
        APP_PRINTF("viss_nv12_out_img create failed\n");
        return VX_FAILURE;
    }

    /* Sample image is no longer required */
    vxReleaseImage(&obj->sample_nv12_img);

    memset(&viss_params, 0, sizeof(tivx_vpac_viss_params_t));

    obj->viss_configuration =
            vxCreateUserDataObject(obj->context, "tivx_vpac_viss_params_t",
            sizeof(tivx_vpac_viss_params_t), NULL);
    if (vxGetStatus((vx_reference)obj->viss_configuration) != VX_SUCCESS)
    {
        APP_PRINTF("viss_configuration create failed\n");
        return VX_FAILURE;
    }

    /* VISS Initialize parameters */
    tivx_vpac_viss_params_init(&viss_params);

    viss_params.sensor_dcc_id = obj->cam_dcc_id;
    viss_params.use_case = 0;
    viss_params.fcp[0].ee_mode = 0;
    viss_params.fcp[0].mux_output0 = 0;
    viss_params.fcp[0].mux_output1 = 0;
    if (0 == obj->is_enable_yuyv)
    {
        viss_params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
    }
    else
    {
        viss_params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
    }
    viss_params.fcp[0].mux_output3 = 0;
    viss_params.fcp[0].mux_output4 = 3;
    viss_params.bypass_nsf4 = 1;
    viss_params.h3a_in = 3;
    viss_params.h3a_aewb_af_mode = 0;
    viss_params.fcp[0].chroma_mode = 0;
    viss_params.enable_ctx = 1;
    if(sensor_wdr_enabled == 1)
    {
        viss_params.bypass_glbce = 0;
    }else
    {
        viss_params.bypass_glbce = 1;
    }


    /* Create h3a_aew_af output buffer (uninitialized) */
    h3a_aew_af_exemplar = vxCreateUserDataObject(obj->context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    if (vxGetStatus((vx_reference)h3a_aew_af_exemplar) != VX_SUCCESS)
    {
        APP_PRINTF("h3a_aew_af create failed\n");
        return VX_FAILURE;
    }

    obj->h3a_aew_af_arr = vxCreateObjectArray(obj->context,
        (vx_reference)h3a_aew_af_exemplar, NUM_CAPT_CHANNELS);
    if (vxGetStatus((vx_reference)obj->h3a_aew_af_arr) != VX_SUCCESS)
    {
        APP_PRINTF("h3a_aew_af_arr create failed\n");
        return VX_FAILURE;
    }

    obj->h3a_aew_af = (vx_user_data_object) vxGetObjectArrayItem(obj->h3a_aew_af_arr, 0);
    if (vxGetStatus((vx_reference)obj->h3a_aew_af) != VX_SUCCESS)
    {
        APP_PRINTF("h3a_aew_af create failed\n");
        return VX_FAILURE;
    }

    vxReleaseUserDataObject(&h3a_aew_af_exemplar);

    vxCopyUserDataObject(obj->viss_configuration, 0,
        sizeof(tivx_vpac_viss_params_t), &viss_params, VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST);

    /* Create/Configure ae_awb_result input structure */
    memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
    ae_awb_result_exemplar =
        vxCreateUserDataObject(obj->context, "tivx_ae_awb_params_t",
        sizeof(tivx_ae_awb_params_t), NULL);
    if (vxGetStatus((vx_reference)ae_awb_result_exemplar) != VX_SUCCESS)
    {
        APP_PRINTF("ae_awb_result_exemplar create failed\n");
        return VX_FAILURE;
    }

    obj->ae_awb_result_arr = vxCreateObjectArray(obj->context,
        (vx_reference)ae_awb_result_exemplar, NUM_CAPT_CHANNELS);
    if (vxGetStatus((vx_reference)obj->ae_awb_result_arr) != VX_SUCCESS)
    {
        APP_PRINTF("obj->ae_awb_result_arr create failed\n");
        return VX_FAILURE;
    }

    obj->ae_awb_result = (vx_user_data_object)vxGetObjectArrayItem(obj->ae_awb_result_arr, 0);
    if (vxGetStatus((vx_reference)obj->ae_awb_result) != VX_SUCCESS)
    {
        APP_PRINTF("obj->ae_awb_result create failed\n");
        return VX_FAILURE;
    }

    vxReleaseUserDataObject(&ae_awb_result_exemplar);

    /* Get sample RAW Image */
    obj->raw_img = (tivx_raw_image) vxGetObjectArrayItem(obj->capt_frames[0], 0);

    if(sensor_dcc_enabled)
    {
        dcc_buff_size = appIssGetDCCSizeVISS(obj->sensor_name, sensor_wdr_enabled);

        obj->dcc_param_viss = vxCreateUserDataObject(
            obj->context,
            (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size,
            NULL
        );
        vxMapUserDataObject(
                obj->dcc_param_viss,
                0,
                dcc_buff_size,
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );

        status = appIssGetDCCBuffVISS(obj->sensor_name, sensor_wdr_enabled, dcc_viss_buf, dcc_buff_size);
        if(status != VX_SUCCESS)
        {
            printf("Error getting VISS DCC buffer \n");
            return VX_FAILURE;
        }
        vxUnmapUserDataObject(obj->dcc_param_viss, dcc_viss_buf_map_id);
    }else
    {
        obj->dcc_param_viss = NULL;
    }

    histogram_exemplar = vxCreateDistribution(obj->context, 256, 0, 256);

    obj->histogram_arr = vxCreateObjectArray(obj->context,
            (vx_reference)histogram_exemplar, NUM_CAPT_CHANNELS);
    if (vxGetStatus((vx_reference)obj->histogram_arr) != VX_SUCCESS)
    {
        APP_PRINTF("histogram_arr create failed\n");
        return VX_FAILURE;
    }

    obj->histogram = (vx_distribution)vxGetObjectArrayItem(obj->histogram_arr, 0);
    if (vxGetStatus((vx_reference)obj->histogram) != VX_SUCCESS)
    {
        APP_PRINTF("histogram create failed\n");
        return VX_FAILURE;
    }

    vxReleaseDistribution(&histogram_exemplar);

    obj->vissNode = tivxVpacVissNode(
                         obj->graph,
                         obj->viss_configuration,
                         NULL,
                         obj->dcc_param_viss,
                         obj->raw_img,
                         NULL,
                         NULL,
                         obj->viss_nv12_out_img,
                         NULL,
                         NULL,
                         obj->h3a_aew_af,
                         NULL, NULL, NULL);
    if (vxGetStatus((vx_reference)obj->vissNode) != VX_SUCCESS)
    {
        APP_PRINTF("vissNode create failed\n");
        return VX_FAILURE;
    }

    tivxSetNodeParameterNumBufByIndex(obj->vissNode, 6u, NUM_INTERMEDIATE_BUFS);
    tivxSetNodeParameterNumBufByIndex(obj->vissNode, 9u, NUM_INTERMEDIATE_BUFS);

    vxSetReferenceName((vx_reference)obj->vissNode, "VISS_Processing");
    vxSetNodeTarget(obj->vissNode, VX_TARGET_STRING,
        TIVX_TARGET_VPAC_VISS1);
    vxReplicateNode(obj->graph, obj->vissNode, viss_prms_replicate, 13u);
    obj->aewb_cfg.sensor_dcc_id = obj->cam_dcc_id;
    obj->aewb_cfg.sensor_img_format = 0;
    obj->aewb_cfg.sensor_img_phase = 3;
    if(sensor_exp_control_enabled || sensor_gain_control_enabled )
    {
        obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_AUTO;
    }
    else
    {
        obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
    }
    obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_AUTO;
    obj->aewb_cfg.awb_num_skip_frames = 9;
    obj->aewb_cfg.ae_num_skip_frames = 9;
    obj->aewb_cfg.channel_id = 0;

    if(sensor_dcc_enabled)
    {
        dcc_buff_size = appIssGetDCCSize2A(obj->sensor_name, sensor_wdr_enabled);

        obj->dcc_param_2a = vxCreateUserDataObject(
            obj->context,
            (const vx_char*)&dcc_2a_user_data_object_name,
            dcc_buff_size,
            NULL
        );

        vxMapUserDataObject(
            obj->dcc_param_2a,
            0,
            dcc_buff_size,
            &dcc_2a_buf_map_id,
            (void **)&dcc_2a_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );

        status = appIssGetDCCBuff2A(obj->sensor_name, sensor_wdr_enabled,  dcc_2a_buf, dcc_buff_size);
        if(status != VX_SUCCESS)
        {
            printf("Error getting 2A DCC buffer \n");
            return VX_FAILURE;
        }

        vxUnmapUserDataObject(obj->dcc_param_2a, dcc_2a_buf_map_id);
    }else
    {
        obj->dcc_param_2a = NULL;
    }

    /* Separate config for AEWB nodes to pass unique channel ID */
    aewb_config_exemplar = vxCreateUserDataObject(obj->context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &obj->aewb_cfg);
    obj->aewb_config_array = vxCreateObjectArray(obj->context, (vx_reference)aewb_config_exemplar, NUM_CAPT_CHANNELS);
    vxReleaseUserDataObject(&aewb_config_exemplar);
    for(loop_id=0;loop_id<NUM_CAPT_CHANNELS;loop_id++)
    {
        aewb_config_exemplar = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_config_array, loop_id);
        obj->aewb_cfg.channel_id = loop_id;
        vxCopyUserDataObject(aewb_config_exemplar, 0, sizeof(tivx_aewb_config_t), &obj->aewb_cfg, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        vxReleaseUserDataObject(&aewb_config_exemplar);
    }
    obj->aewb_config = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_config_array, 0);

    obj->node_aewb = tivxAewbNode(obj->graph,
                                  obj->aewb_config,
                                  obj->histogram,
                                  obj->h3a_aew_af,
                                  NULL,
                                  obj->ae_awb_result,
                                  obj->dcc_param_2a);
    vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU2_0);

    if(NULL != obj->node_aewb)
        vxSetReferenceName((vx_reference)obj->node_aewb, "2A_AlgNode");
    else
    {
        APP_PRINTF("tivxAewbNode returned NULL \n");
        return VX_FAILURE;
    }
    APP_PRINTF("AEWB Set Reference done\n");

    vxReplicateNode(obj->graph, obj->node_aewb, aewb_prms_replicate, 6u);
    tivxSetNodeParameterNumBufByIndex(obj->node_aewb, 4u, NUM_BUFS);

    return status;
}

/* Creating display node */
static vx_status app_create_display(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 posX, posY, pipeId;
    tivx_display_params_t display_params;

    posX      = 0;
    posY      = 0;
    pipeId    = 0;

    /* Display initialization */
    memset(&display_params, 0, sizeof(tivx_display_params_t));
    display_params.opMode=TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
    display_params.pipeId=pipeId;
    display_params.outWidth=obj->outWidth;
    display_params.outHeight=obj->outHeight;
    display_params.posX=posX;
    display_params.posY=posY;

    obj->display_param_obj =
        vxCreateUserDataObject(obj->context, "tivx_display_params_t",
        sizeof(tivx_display_params_t), &display_params);
    if (vxGetStatus((vx_reference)obj->display_param_obj) != VX_SUCCESS)
    {
        APP_PRINTF("display_param_obj create failed\n");
        return VX_FAILURE;
    }

    obj->displayNode =
        tivxDisplayNode(obj->graph, obj->display_param_obj,obj->output_image[0]);
    if (vxGetStatus((vx_reference)obj->display_param_obj) != VX_SUCCESS)
    {
        APP_PRINTF("displayNode create failed\n");
    }

    vxSetNodeTarget(obj->displayNode, VX_TARGET_STRING,
        TIVX_TARGET_DISPLAY1);
    vxSetReferenceName((vx_reference)obj->displayNode, "Display_node");

    /* Create User Data object for channel switching */
    obj->channel_prms.active_channel_id = 0;
    obj->switch_ch_obj = vxCreateUserDataObject(obj->context,
        "tivx_display_select_channel_params_t",
        sizeof(tivx_display_select_channel_params_t), &obj->channel_prms);
    if (vxGetStatus((vx_reference)obj->switch_ch_obj) != VX_SUCCESS)
    {
        APP_PRINTF("switch_ch_obj create failed\n");
    }

    obj->refs[0] = (vx_reference)obj->switch_ch_obj;

    return status;
}

/*
 * Graph,
 *           viss_config
 *               |
 *               v
 * input_img -> VISS -> SRV -> Display
 *
 */
static vx_status app_create_graphs(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    int graph_parameter_num = 0;
    int graph_parameters_list_depth = 1;
    if(obj->test_mode == 1)
    {
        graph_parameters_list_depth = 2;
    }
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[graph_parameters_list_depth];

    APP_PRINTF("Querying %s \n", obj->sensor_name);
    memset(&sensorParams, 0, sizeof(sensorParams));
    status = appQueryImageSensor(obj->sensor_name, &sensorParams);
    if(VX_SUCCESS != status)
    {
        printf("appQueryImageSensor returned %d\n", status);
        return status;
    }

    obj->outWidth  = DISPLAY_WIDTH_2MP;
    obj->outHeight = DISPLAY_HEIGHT_2MP;

    obj->inWidth    = sensorParams.sensorInfo.raw_params.width;
    obj->inHeight   = sensorParams.sensorInfo.raw_params.height;
    obj->cam_dcc_id = sensorParams.dccId;

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_CAPTURE1)) &&
        (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1)) )
    {
        obj->graph = vxCreateGraph(obj->context);
        if (vxGetStatus((vx_reference)obj->graph) != VX_SUCCESS)
        {
            APP_PRINTF("graph create failed\n");
            return VX_FAILURE;
        }
        status = vxSetReferenceName((vx_reference)obj->graph, "3DSRV");

        if (VX_SUCCESS == status)
        {
            status = app_create_capture(obj);
        }

        if (status != VX_SUCCESS)
        {
            APP_PRINTF("app_create_capture failed\n");
            return VX_FAILURE;
        }

        if (VX_SUCCESS == status)
        {
            status = app_create_viss_aewb(obj);
        }

        if (status != VX_SUCCESS)
        {
            APP_PRINTF("app_create_viss_aewb failed\n");
            return VX_FAILURE;
        }

        if (VX_SUCCESS == status)
        {
            status = app_create_graph_gpu_lut(obj);
        }

        if (status != VX_SUCCESS)
        {
            APP_PRINTF("app_create_graph_gpu_lut failed\n");
            return VX_FAILURE;
        }

        if (VX_SUCCESS == status)
        {
            status = app_create_display(obj);
        }

        if (status != VX_SUCCESS)
        {
            APP_PRINTF("app_create_display failed\n");
            return VX_FAILURE;
        }

        add_graph_parameter_by_node_index(obj->graph, obj->captureNode, 1);

        /* Set graph schedule config such that graph parameter @ index 0 is
         * enqueuable */
        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list =
            (vx_reference*)&obj->capt_frames[0];
        graph_parameter_num++;

        if(obj->test_mode == 1)
        {
            add_graph_parameter_by_node_index(obj->graph, obj->displayNode, 1);
            /* Set graph schedule config such that graph parameter @ index 0 is
            * enqueuable */
            obj->displayNodeGraphParamNum = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = NUM_BUFS;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&obj->output_image[0];
            graph_parameter_num++;
        }

        if(status == VX_SUCCESS)
        {
            status = tivxSetGraphPipelineDepth(obj->graph, PIPE_DEPTH);
        }
        /* Schedule mode auto is used, here we don't need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        if(status == VX_SUCCESS)
        {
            status = vxSetGraphScheduleConfig(obj->graph,
                            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                            graph_parameters_list_depth,
                            graph_parameters_queue_params_list
                            );
        }
        if(status == VX_SUCCESS)
        {
            APP_PRINTF("app_linux_opengl_integrated_srv: Verifying graph 2  ... .\n");
            status = vxVerifyGraph(obj->graph);
            APP_ASSERT(status == VX_SUCCESS);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxExportGraphToDot(obj->graph, ".", "integrated_srv_graph");
            APP_PRINTF("app_linux_opengl_integrated_srv: Verifying graph 2... Done\n");
        }
        APP_PRINTF("vxSetGraphScheduleConfig done\n");
        APP_PRINTF("app_create_graph exiting\n");
    }
    else
    {
        APP_PRINTF("app_create_graph failed: appropriate cores not enabled\n");
        status = VX_FAILURE;
    }

    if (obj->test_mode == 1)
    {
        vx_int32 bytes_read = 0;
        obj->fs_test_raw_image = read_test_image_raw(obj->context, &(sensorParams.sensorInfo),
                                                "/opt/vision_apps/test_data/psdkra/app_single_cam/IMX390_001/input2.raw",
                                                &bytes_read);
        APP_PRINTF("%d bytes were read by read_error_image_raw()\n", bytes_read);
        if(obj->fs_test_raw_image == NULL)
        {
            printf("read_error_image_raw returned a null pointer - test case failed\n");
            status = VX_FAILURE;
        }
        if((bytes_read <= 0) && (status == VX_SUCCESS))
        {
            status = tivxReleaseRawImage(&obj->fs_test_raw_image);
            obj->fs_test_raw_image = NULL;
        }
        if(status == VX_SUCCESS)
        {
            status = vxVerifyGraph(obj->graph);
        }
        if((status == VX_SUCCESS) && (NULL != obj->fs_test_raw_image) && (NULL != obj->captureNode))
        {
            status = app_send_test_frame(obj->captureNode, obj->fs_test_raw_image);
        }
    }
    return status;
}


static int app_create_graph_gpu_lut(AppObj *obj)
{
    vx_uint32 i;
    vx_uint32 num_cameras;
    srv_coords_t local_srv_coords;
    tivx_srv_params_t params;
    vx_status status = VX_SUCCESS;

    /* Applib data */
    svGpuLutGen_t               in_params;
    svACCalmatStruct_t          in_calmat;
    svGeometric_t               in_offset;
    ldc_lensParameters          lens_params;
    svLdcLut_t                  in_lens_params;
    svCalmat_t                  calmat_file;

    num_cameras = NUM_CAPT_CHANNELS;

    read_calmat_file(&calmat_file, "psdkra/srv/srv_app/CALMAT.BIN");

    obj->graph_gpu_lut = vxCreateGraph(obj->context);

    memset(&params, 0, sizeof(tivx_srv_params_t));
    params.cam_bpp = 24;

    memset(&params, 0, sizeof(tivx_srv_params_t));
    params.cam_bpp = 24;

    /* Populating data objects */
    memset(&in_params, 0, sizeof(svGpuLutGen_t));
    obj->in_config = vxCreateUserDataObject(obj->context, "svGpuLutGen_t",
                                       sizeof(svGpuLutGen_t), NULL);
    APP_ASSERT_VALID_REF(obj->in_config);

    memset(&in_calmat, 0, sizeof(svACCalmatStruct_t));
    obj->in_calmat_object = vxCreateUserDataObject(obj->context, "svACCalmatStruct_t",
                                       sizeof(svACCalmatStruct_t), NULL);
    APP_ASSERT_VALID_REF(obj->in_calmat_object);

    memset(&in_offset, 0, sizeof(svGeometric_t));
    obj->in_offset_object = vxCreateUserDataObject(obj->context, "svGeometric_t",
                                       sizeof(svGeometric_t), NULL);
    APP_ASSERT_VALID_REF(obj->in_offset_object);

    memset(&in_lens_params, 0, sizeof(svLdcLut_t));
    obj->in_lens_param_object = vxCreateUserDataObject(obj->context, "svLdcLut_t",
                                       sizeof(svLdcLut_t), NULL);
    APP_ASSERT_VALID_REF(obj->in_lens_param_object);

    obj->out_gpulut_array = vxCreateArray(obj->context, VX_TYPE_UINT16, SV_GPULUT_SIZE);
    APP_ASSERT_VALID_REF(obj->out_gpulut_array);

    obj->param_obj = vxCreateUserDataObject(obj->context, "tivx_srv_params_t", sizeof(tivx_srv_params_t), &params);
    APP_ASSERT_VALID_REF(obj->param_obj);

    obj->srv_views = vxCreateUserDataObject(obj->context, "srv_coords_t", sizeof(srv_coords_t), &local_srv_coords);
    APP_ASSERT_VALID_REF(obj->srv_views);

    obj->srv_views_array = vxCreateObjectArray(obj->context, (vx_reference)obj->srv_views, obj->num_views);
    APP_ASSERT_VALID_REF(obj->srv_views_array);

    vxReleaseUserDataObject(&obj->srv_views);

    for (i = 0; i < obj->num_views; i++)
    {
        local_srv_coords.camx =     obj->camx[i];
        local_srv_coords.camy =     obj->camy[i];
        local_srv_coords.camz =     obj->camz[i];
        local_srv_coords.targetx =  obj->targetx[i];
        local_srv_coords.targety =  obj->targety[i];
        local_srv_coords.targetz =  obj->targetz[i];
        local_srv_coords.anglex =   obj->anglex[i];
        local_srv_coords.angley =   obj->angley[i];
        local_srv_coords.anglez =   obj->anglez[i];

        obj->srv_views = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->srv_views_array, i);
        if(status == VX_SUCCESS)
        {
            status = vxCopyUserDataObject(obj->srv_views, 0, sizeof(srv_coords_t), &local_srv_coords, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
        if(status == VX_SUCCESS)
        {
            status = vxReleaseUserDataObject(&obj->srv_views);
        }
    }

    /* Populate input data */
    in_params.SVInCamFrmHeight   = obj->inHeight;
    in_params.SVInCamFrmWidth    = obj->inWidth;
    in_params.SVOutDisplayHeight = SV_OUT_DISPLAY_HEIGHT;
    in_params.SVOutDisplayWidth  = SV_OUT_DISPLAY_WIDTH;
    in_params.numCameras         = num_cameras;
    in_params.subsampleratio     = SV_SUBSAMPLE;
    in_params.useWideBowl        = 1;

    in_offset.offsetXleft     = obj->offsetXleft;
    in_offset.offsetXright    = obj->offsetXright;
    in_offset.offsetYfront    = obj->offsetYfront;
    in_offset.offsetYback     = obj->offsetYback;

    for (int idx=0;idx<48;idx++) {
        in_calmat.outcalmat[idx] = calmat_file.calMatBuf[idx];
    }

    /* Read Lens file */
    read_lut_file(&lens_params,"psdkra/srv/srv_app/LENS.BIN" );

    /* Initialize all the 4 channel ldc luts within in_params    */

    for (int i=0;i<4; i++) {
        LDC_Init(&in_lens_params.ldc[i],
             lens_params.ldcLUT_distortionCenters[i*2],
             lens_params.ldcLUT_distortionCenters[i*2+1],
             lens_params.ldcLUT_focalLength,
             lens_params.ldcLUT_D2U_table,
             lens_params.ldcLUT_D2U_length,
             lens_params.ldcLUT_D2U_step,
             lens_params.ldcLUT_U2D_table,
             lens_params.ldcLUT_U2D_length,
             lens_params.ldcLUT_U2D_step);
    }

    /* Copying to user data objects */
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_config, 0, sizeof(svGpuLutGen_t), &in_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_calmat_object, 0, sizeof(svACCalmatStruct_t), &in_calmat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_offset_object, 0, sizeof(svGeometric_t), &in_offset, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    if(status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(obj->in_lens_param_object, 0, sizeof(svLdcLut_t), &in_lens_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    }
    /* Creating applib */
    obj->create_params.vxContext = obj->context;
    obj->create_params.vxGraph   = obj->graph_gpu_lut;

    /* Data object */
    obj->create_params.in_config    = obj->in_config;
    obj->create_params.in_calmat    = obj->in_calmat_object;
    obj->create_params.in_offset    = obj->in_offset_object;
    obj->create_params.in_ldclut    = obj->in_lens_param_object;
    obj->create_params.out_gpulut3d = obj->out_gpulut_array;

    obj->srv_handle = srv_bowl_lut_gen_create(&obj->create_params);

    /* Creating objects for OpenGL node */
    for(int i=0; i < NUM_BUFS; i++)
    {
        if(status == VX_SUCCESS)
        {
            obj->output_image[i] = vxCreateImage(obj->context, obj->outWidth, obj->outHeight, VX_DF_IMAGE_RGBX);
            status = vxGetStatus((vx_reference) obj->output_image[i]);
        }
    }

    obj->srv_node = tivxGlSrvNode(obj->graph, obj->param_obj, obj->viss_out_frames, obj->srv_views_array, obj->out_gpulut_array, obj->output_image[0]);
    if (vxGetStatus((vx_reference)obj->srv_node) != VX_SUCCESS)
    {
        APP_PRINTF("srv_node create failed\n");
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        status = vxSetNodeTarget(obj->srv_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    }
    if(status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->srv_node, "OpenGL_SRV_Node");
    }
    if(status == VX_SUCCESS)
    {
        status = tivxSetNodeParameterNumBufByIndex(obj->srv_node, 4u, NUM_INTERMEDIATE_BUFS);
    }
    APP_PRINTF("app_linux_opengl_integrated_srv: Verifying graph 1  ... .\n");
    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph_gpu_lut);
    }

    APP_PRINTF("app_linux_opengl_integrated_srv: Verifying graph 1... Done\n");
    return status;
}

static void app_run_graph_gpu_lut(AppObj *obj)
{
    #ifdef APP_DEBUG_SRV
    printf("app_linux_opengl_integrated_srv: Running graph 1 ...\n");
    #endif
    vxProcessGraph(obj->graph_gpu_lut);
    #ifdef APP_DEBUG_SRV
    printf("app_linux_opengl_integrated_srv: Running graph 1 ... Done\n");
    #endif
    return;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_refs, buf_id;
    int graph_parameter_num = 0;
    uint32_t iteration = 0;

    if(NULL == obj->sensor_name)
    {
        printf("sensor name is NULL \n");
        status = VX_FAILURE;
    }
    if(status == VX_SUCCESS)
    {
        status = appStartImageSensor(obj->sensor_name, 0xF);
        APP_PRINTF("Sensor start status = %d\n", status);
    }

    /* Enqueue buf for pipe up but don't trigger graph execution */
    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        graph_parameter_num = 0;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num,
                       (vx_reference*)&obj->capt_frames[buf_id], 1);
        }
        graph_parameter_num++;
    }
    if(obj->test_mode == 1)
    {
        for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
        {
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->displayNodeGraphParamNum,
                                (vx_reference*)&obj->output_image[buf_id], 1);
            }
        }
    }

    /* wait for graph instances to complete, compare output and
     * recycle data buffers, schedule again */
    vx_uint32 actual_checksum = 0;
    while(status == VX_SUCCESS)
    {
        vx_object_array out_capture_frames;
        vx_image test_image;
        graph_parameter_num = 0;

        appPerfPointBegin(&obj->total_perf);

        /* Get output reference, waits until a frame is available */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num,
                            (vx_reference*)&out_capture_frames, 1, &num_refs);
        }
        graph_parameter_num++;

        if(obj->test_mode == 1)
        {
            /* Get output reference, waits until a frame is available */
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(obj->graph, obj->displayNodeGraphParamNum,
                                                        (vx_reference*)&test_image, 1, &num_refs);
            }
            printf("test iteration: %d of %d\n", iteration, TEST_BUFFER+1);
            if( (status == VX_SUCCESS) && (iteration > TEST_BUFFER) )
            {
                if(app_test_check_image(test_image, checksums_expected[0][0], &actual_checksum) == vx_false_e)
                {
                    test_result = vx_false_e;
                }
                populate_gatherer(0, 0, actual_checksum);
                obj->stop_task = 1;
            }
            /* Get output reference, waits until a frame is available */
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->displayNodeGraphParamNum,
                                (vx_reference*)&test_image, 1);
            }
        }

        graph_parameter_num = 0;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1);
        }
        graph_parameter_num++;

        appPerfPointEnd(&obj->total_perf);

        if(iteration==100)
        {
            /* after first 'n' iteration reset performance stats */
            appPerfStatsResetAll();
        }

        iteration++;

        if((obj->stop_task) || (status != VX_SUCCESS))
        {
            break;
        }
    }
    /* ensure all graph processing is complete */
    vxWaitGraph(obj->graph);

    /* Dequeue buf for pipe down */
    num_refs = 0xFF;
    graph_parameter_num = 0;
    while((num_refs > 0) && (status == VX_SUCCESS))
    {
        vx_object_array out_capture_frames;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterCheckDoneRef(obj->graph, graph_parameter_num, &num_refs);
        }
        if(num_refs > 0)
        {
            APP_PRINTF("Dequeue capture\n");
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(
                                            obj->graph,
                                            graph_parameter_num,
                                            (vx_reference*)&out_capture_frames,
                                            1,
                                            &num_refs);
            }
        }
    }
    num_refs = 0xFF;
    while((num_refs > 0) && (obj->test_mode == 1) && (status == VX_SUCCESS))
    {
        vx_image out_image;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterCheckDoneRef(obj->graph, obj->displayNodeGraphParamNum, &num_refs);
        }
        if(num_refs > 0)
        {
            APP_PRINTF("Dequeue display \n");
            if(status == VX_SUCCESS)
            {
                status = vxGraphParameterDequeueDoneRef(
                                            obj->graph,
                                            obj->displayNodeGraphParamNum,
                                            (vx_reference*)&out_image,
                                            1,
                                            &num_refs);
            }
        }
    }
    return status;
}

static void app_delete_graph(AppObj *obj)
{
    vx_uint8 buf_id = 0;

    /* Deleting applib */
    srv_bowl_lut_gen_delete(obj->srv_handle);
    APP_PRINTF("releasing srv_applib done\n");

    vxReleaseUserDataObject(&obj->param_obj);
    APP_PRINTF("releasing param_obj done\n");

    vxReleaseObjectArray(&obj->srv_views_array);
    APP_PRINTF("releasing srv_views_array done\n");

    vxReleaseUserDataObject(&obj->in_config);
    APP_PRINTF("releasing in_config done\n");

    vxReleaseUserDataObject(&obj->in_calmat_object);
    APP_PRINTF("releasing in_calmat_object done\n");

    vxReleaseUserDataObject(&obj->in_offset_object);
    APP_PRINTF("releasing in_offset_object done\n");

    vxReleaseUserDataObject(&obj->in_lens_param_object);
    APP_PRINTF("releasing in_lens_param_object done\n");

    vxReleaseArray(&obj->out_gpulut_array);
    APP_PRINTF("releasing out_gpulut_array done\n");

    vxReleaseNode(&obj->srv_node);
    APP_PRINTF("releasing srv_node done\n");

    if(NULL != obj->node_aewb)
    {
        vxReleaseNode(&obj->node_aewb);
        APP_PRINTF("releasing node_aewb done\n");
    }

    if(NULL != obj->vissNode)
    {
        vxReleaseNode(&obj->vissNode);
        APP_PRINTF("releasing vissNode done\n");
    }

    vxReleaseNode(&obj->displayNode);
    APP_PRINTF("releasing graph done\n");

    vxReleaseNode(&obj->captureNode);
    APP_PRINTF("releasing captureNode done\n");

    vxReleaseUserDataObject(&obj->display_param_obj);
    APP_PRINTF("releasing display_param_obj done\n");

    vxReleaseUserDataObject(&obj->capture_param_obj);
    APP_PRINTF("releasing capture_param_obj done\n");

    vxReleaseUserDataObject(&obj->switch_ch_obj);
    APP_PRINTF("releasing switch_ch_obj done\n");

    tivxReleaseRawImage(&obj->raw_img);
    APP_PRINTF("releasing raw_img done\n");

    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseObjectArray(&obj->capt_frames[buf_id]);
        APP_PRINTF("releasing capt_frames[%d] done\n", buf_id);
    }
    for(buf_id=0; buf_id<NUM_BUFS; buf_id++)
    {
        vxReleaseImage(&obj->output_image[buf_id]);
        APP_PRINTF("releasing output_image[%d] done\n", buf_id);
    }

    vxReleaseObjectArray(&obj->viss_out_frames);
    APP_PRINTF("releasing viss_out_frames done\n");

    if (NULL != obj->ae_awb_result_arr)
    {
        vxReleaseObjectArray(&obj->ae_awb_result_arr);
        APP_PRINTF("releasing ae_awb_result_arr done\n");
    }

    if (NULL != obj->ae_awb_result)
    {
        vxReleaseUserDataObject(&obj->ae_awb_result);
        APP_PRINTF("releasing ae_awb_result done\n");
    }

    vxReleaseUserDataObject(&obj->viss_configuration);
    APP_PRINTF("releasing viss_configuration done\n");

    if(NULL != obj->histogram)
    {
        vxReleaseDistribution(&obj->histogram);
        APP_PRINTF("releasing histogram done\n");
    }

    if(NULL != obj->histogram_arr)
    {
        vxReleaseObjectArray(&obj->histogram_arr);
        APP_PRINTF("releasing histogram_arr done\n");
    }

    vxReleaseImage(&obj->viss_nv12_out_img);
    APP_PRINTF("releasing viss_nv12_out_img done\n");

    if(NULL != obj->dcc_param_viss)
    {
        vxReleaseUserDataObject(&obj->dcc_param_viss);
        APP_PRINTF("releasing VISS DCC Data Object done\n");
    }

    if(NULL != obj->h3a_aew_af)
    {
        vxReleaseUserDataObject(&obj->h3a_aew_af);
        APP_PRINTF("releasing h3a_aew_af done\n");
    }

    if(NULL != obj->h3a_aew_af_arr)
    {
        vxReleaseObjectArray(&obj->h3a_aew_af_arr);
        APP_PRINTF("releasing h3a_aew_af_arr done\n");
    }

    if(NULL != obj->aewb_config)
    {
        vxReleaseUserDataObject(&obj->aewb_config);
        APP_PRINTF("releasing aewb_config done\n");
    }

    if(NULL != obj->aewb_config_array)
    {
        vxReleaseObjectArray(&obj->aewb_config_array);
        APP_PRINTF("releasing aewb_config_array done\n");
    }

    if(NULL != obj->dcc_param_2a)
    {
        vxReleaseUserDataObject(&obj->dcc_param_2a);
        APP_PRINTF("releasing 2A DCC Data Object done\n");
    }

    vxReleaseGraph(&obj->graph_gpu_lut);
    APP_PRINTF("releasing graph_gpu_lut done\n");

    vxReleaseGraph(&obj->graph);
    APP_PRINTF("releasing graph done\n");

    return;
}

static void app_deinit(AppObj *obj)
{
    APP_PRINTF("deinit app...\n");

    if(obj->is_enable_gui)
    {
        appGrpxDeInit();
        APP_PRINTF("appGrpxDeInit done\n");
    }

    tivxHwaUnLoadKernels(obj->context);
    APP_PRINTF("tivxHwaUnLoadKernels done\n");

    tivxVideoIOUnLoadKernels(obj->context);
    APP_PRINTF("tivxVideoIOUnLoadKernels done\n");

    tivxImagingUnLoadKernels(obj->context);
    APP_PRINTF("tivxImagingUnLoadKernels done\n");

    tivxSrvUnLoadKernels(obj->context);
    APP_PRINTF("tivxSrvUnLoadKernels done\n");

    vxReleaseContext(&obj->context);

    appDeInitImageSensor(obj->sensor_name);
    APP_PRINTF("sensor deinit done\n");

    appDeInit();
}

/* TODO: Use a common utils file */
vx_status app_linux_opengl_integrated_srv(int argc, char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    app_parse_cmd_line_args(obj, argc, argv);
    status = app_init(obj);

    if(VX_SUCCESS == status)
    {
        status = app_create_graphs(obj);

        if(VX_SUCCESS == status)
        {
            app_run_graph_gpu_lut(obj);

            if(VX_SUCCESS == status)
            {
                if(obj->is_interactive)
                {
                    status = app_run_graph_interactive(obj);
                }
                else
                {
                    status = app_run_graph(obj);
                }
            }

            status = appStopImageSensor(obj->sensor_name, 0xF);/*Mask = 0xF for 4 cameras*/
            app_delete_graph(obj);
        }
    }

    app_deinit(obj);

    if(obj->test_mode == 1)
    {
        if((test_result == vx_false_e) || (status != VX_SUCCESS))
        {
            status = (status == VX_SUCCESS) ? VX_FAILURE : status;
            printf("\n\nTEST FAILED\n\n");
            print_new_checksum_structs();
        }
        else
        {
            printf("\n\nTEST PASSED\n\n");
        }
    }

    return status;
}

/* This function is largely taken from single_cam_app :
    this function takes a raw_image that is unpopulated
    and populates it with the path set below */
tivx_raw_image read_test_image_raw(vx_context context,
                             IssSensor_Info *sensorInfo, char raw_image_fname[],
                             vx_int32 *bytes_read)
{
    /* Taking the test data from the app_single_cam demo to save space in the repo */
    FILE * fp;
    vx_uint32 width, height, i;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    void *data_ptr;
    vx_uint32 num_bytes_per_pixel = 2; /*Supports only RAW 12b Unpacked format*/
    vx_uint32 num_bytes_read_from_file;
    tivx_raw_image raw_image = NULL;
    tivx_raw_image_format_t format;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;
    /* Provision for injecting RAW frame into the sensor */
    /* rect defines the outer bounds of the raw_image frame
        which will be defined */
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = sensorInfo->raw_params.width;
    rect.end_y = sensorInfo->raw_params.height;

    /* Nothing is being populated here - just an empty frame */
    raw_image = tivxCreateRawImage(context, &(sensorInfo->raw_params));

    APP_PRINTF("Reading test RAW image %s \n", raw_image_fname);
    fp = fopen(raw_image_fname, "rb");
    if(!fp)
    {
        printf("read_test_image_raw : Unable to open file %s\n", raw_image_fname);
        return NULL;
    }
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    tivxMapRawImagePatch(raw_image,
        &rect,
        0,
        &map_id,
        &image_addr,
        &data_ptr,
        VX_READ_AND_WRITE,
        VX_MEMORY_TYPE_HOST,
        TIVX_RAW_IMAGE_PIXEL_BUFFER
        );

    if(!data_ptr)
    {
        APP_PRINTF("data_ptr is NULL \n");
        fclose(fp);
        return NULL;
    }
    num_bytes_read_from_file = 0;

    imgaddr_width  = image_addr.dim_x;
    imgaddr_height = image_addr.dim_y;
    imgaddr_stride = image_addr.stride_y;

    for(i=0;i<imgaddr_height;i++)
    {
        num_bytes_read_from_file += fread(data_ptr, 1, imgaddr_width*num_bytes_per_pixel, fp);
        data_ptr += imgaddr_stride;
    }

    tivxUnmapRawImagePatch(raw_image, map_id);

    fclose(fp);
    APP_PRINTF("%d bytes read from %s\n", num_bytes_read_from_file, raw_image_fname);
    *bytes_read = num_bytes_read_from_file;
    return raw_image;
}

vx_status app_send_test_frame(vx_node cap_node, tivx_raw_image raw_img)
{
    vx_status status = VX_SUCCESS;

    status = tivxCaptureRegisterErrorFrame(cap_node, (vx_reference)raw_img);

    return status;
}
