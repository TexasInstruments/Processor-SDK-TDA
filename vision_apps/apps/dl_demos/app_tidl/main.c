/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include "itidl_ti.h"
#include "app_common.h"
#include "app_test.h"
#include <TI/hwa_kernels.h>
#include <TI/dl_kernels.h>
#include <TI/video_io_kernels.h>
#include <TI/video_io_display.h>

#if defined(SOC_AM62A) && defined(QNX)
#include <screen/screen.h>
#endif

#if defined(SOC_AM62A) && defined(LINUX) && !defined(x86_64)
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#endif

/*
 * define this macro to enable TIDL intermediate layer traces on target
 */

#undef APP_TIDL_TRACE_DUMP

/*
 * This is the size of trace buffer allocated in host memory and
 * shared with target.
 */
#define TIVX_TIDL_TRACE_DATA_SIZE  (128 * 1024 * 1024)

extern const char imgnet_labels[1001][256];
static const char *tensor_num_str[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};

#ifdef APP_DEBUG
#define APP_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define APP_PRINTF(f_, ...)
#endif

#if defined(SOC_AM62A) && defined(QNX)
/*AM62A: QNX to use screen package for displaying frames on A53*/
screen_context_t screen_ctx = NULL;
screen_window_t screen_win = NULL;
#endif

typedef struct {

    /* config options */
    char tidl_config_file_path[APP_MAX_FILE_PATH];
    char tidl_network_file_path[APP_MAX_FILE_PATH];
    char input_file_path[APP_MAX_FILE_PATH];
    char input_file_list[APP_MAX_FILE_PATH];
    char output_file_path[APP_MAX_FILE_PATH];
    char ti_logo_file_path[APP_MAX_FILE_PATH];

    uint32_t num_input_tensors;
    uint32_t num_output_tensors;

    /* Input image params */
    vx_df_image df_image;
    void *data_ptr;
    tivx_utils_bmp_image_params_t imgParams;
    vx_uint32 img_width;
    vx_uint32 img_height;
    vx_uint32 img_stride;

    sTIDL_IOBufDesc_t   ioBufDesc;

    uint8_t *pInPlanes;
    uint8_t *pOutPlanes;

    uint8_t  *pDisplayBuf888;
    uint16_t *pDisplayBuf565;

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;
    vx_kernel  kernel;
    vx_node    tidl_node;

    vx_user_data_object  config;
    vx_user_data_object  network;
    vx_user_data_object  createParams;
    vx_user_data_object  inArgs;
    vx_user_data_object  outArgs;
    vx_user_data_object  traceData;

    vx_tensor  input_tensors[APP_MAX_TENSORS];
    vx_tensor  output_tensors[APP_MAX_TENSORS];

    vx_graph   disp_graph;
    vx_node disp_node;
    vx_image disp_image;

    vx_user_data_object disp_params_obj;
    tivx_display_params_t disp_params;
    vx_rectangle_t disp_rect;
    vx_imagepatch_addressing_t image_addr;

    vx_graph   disp_graph2;
    vx_node disp_node2;
    vx_image disp_image2;

    vx_user_data_object disp_params_obj2;
    tivx_display_params_t disp_params2;
    vx_rectangle_t disp_rect2;
    vx_imagepatch_addressing_t image_addr2;

    vx_size in_tensor_data_type[APP_MAX_TENSORS];

    vx_uint32 display_option;
    vx_uint32 use_dual_display;
    vx_uint32 delay_in_msecs;
    vx_uint32 num_iterations;

    Draw2D_Handle  pHndl;

    uint32_t is_interactive;
    vx_int32 test_mode;
    vx_int32 test_case;

    tivx_task task;
    uint32_t stop_task;
    uint32_t stop_task_done;

#if defined(SOC_AM62A) && defined(QNX)
    tivx_task screen_task;
    uint32_t stop_screen_task;
    uint32_t stop_screen_task_done;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    void *drm_handle;
    tivx_task drm_task;
    uint32_t stop_drm_task;
    uint32_t stop_drm_task_done;
    uint32_t drm_buf_idx;
#endif

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

} AppObj;

AppObj gAppObj;

static int app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[]);
static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static vx_status app_run_graph_interactive(AppObj *obj);
static void app_delete_graph(AppObj *obj);

static vx_user_data_object readConfig(AppObj *obj, vx_context context, char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors);
static vx_user_data_object readNetwork(vx_context context, char *network_file);
static vx_user_data_object setCreateParams(vx_context context);
static vx_user_data_object setInArgs(vx_context context);
static vx_user_data_object setOutArgs(vx_context context);
static void createInputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors);
static void createOutputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *output_tensors);
static vx_status readInput(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file);
static void displayOutput(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, char *output_file);
static vx_size getTensorDataType(vx_int32 tidl_type);
#ifdef APP_WRITE_PRE_PROC_OUTPUT
static vx_status writePreProcOutput(char* file_name, vx_tensor output);
#endif
static uint32_t num_params;
static uint32_t max_params;

#if defined(SOC_AM62A) && defined(QNX)
int32_t app_run_screen(AppObj *obj)
{
    int32_t err = 0;
    int usage = SCREEN_USAGE_READ | SCREEN_USAGE_WRITE;
    int screenFormat = SCREEN_FORMAT_RGBA4444;
    screen_context_t screen_ctx = NULL;
    screen_window_t screen_win = NULL;
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1 = NULL, *data_ptr2 = NULL;
    vx_uint32 num_bytes_per_4pixels;
    vx_uint32 imgaddr_width, imgaddr_height, imgaddr_stride;
    uint32_t i;

    /* connect to screen */
    err = screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
    if(err != 0) {
        printf("Failed to create screen context\n");
    }

    /* create a window */
    err = screen_create_window(&screen_win, screen_ctx);
    if(err != 0) {
        printf("Failed to create screen window\n");
    }

    err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    if(err != 0) {
        printf("Failed to set usage property\n");
    }
    err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &screenFormat);
    if(err != 0) {
        printf("Failed to set format prpoerty\n");
    }

    /* create screen buffers */
    int nbuffers = 2;
    err = screen_create_window_buffers(screen_win, nbuffers);
    if(err != 0) {
        printf("Failed to create window buffer\n");
    }

    while(1) {
        int buffer_size[2];
        err = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size);
        if(err != 0) {
            printf("Failed to get window buffer size\n");
        }

        screen_buffer_t screen_buf[2];
        err = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_buf);
        if(err != 0) {
            printf("Failed to get window buffer\n");
        }

        /* obtain pointers to the buffers */
        void *ptr1 = NULL;
        err = screen_get_buffer_property_pv(screen_buf[0], SCREEN_PROPERTY_POINTER, (void **)&ptr1);
        if(err != 0) {
            printf("Failed to get buffer pointer\n");
        }

        int buf_stride1 = 0;
        err = screen_get_buffer_property_iv(screen_buf[0], SCREEN_PROPERTY_STRIDE, &buf_stride1);
        if(err != 0) {
           printf("Failed to get buffer stride1\n");
        }

        /* copy frames from OVX buffer to screen buffer*/
        vx_image display_image = obj->disp_image;
        vxQueryImage(display_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(display_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(display_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        if(VX_DF_IMAGE_NV12 == df)
        {
            num_bytes_per_4pixels = 4;
        }
        else if(TIVX_DF_IMAGE_NV12_P12 == df)
        {
            num_bytes_per_4pixels = 6;
        }
        else
        {
            num_bytes_per_4pixels = 8;
        }

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        vxMapImagePatch(display_image,
            &rect,
            0,
            &map_id1,
            &image_addr,
            &data_ptr1,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(!data_ptr1) {
            printf("data_ptr1 is NULL \n");
            return -1;
        }

        imgaddr_width  = image_addr.dim_x;
        imgaddr_height = image_addr.dim_y;
        imgaddr_stride = image_addr.stride_y;

        for(i=0;i<height;i++)
        {
            memcpy(ptr1, data_ptr1, imgaddr_width*num_bytes_per_4pixels/4);
            data_ptr1 += imgaddr_stride;
            ptr1 += (buf_stride1);
        }
        vxUnmapImagePatch(display_image, map_id1);

        if(VX_DF_IMAGE_NV12 == df || TIVX_DF_IMAGE_NV12_P12 == df)
        {
            vxMapImagePatch(display_image,
                &rect,
                1,
                &map_id2,
                &image_addr,
                &data_ptr2,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                VX_NOGAP_X
                );

            if(!data_ptr2)
            {
                printf("data_ptr2 is NULL \n");
                return -1;
            }

            imgaddr_width  = image_addr.dim_x;
            imgaddr_height = image_addr.dim_y;
            imgaddr_stride = image_addr.stride_y;

            for(i=0;i<imgaddr_height/2;i++)
            {
                memcpy(ptr1, data_ptr2, imgaddr_width*num_bytes_per_4pixels/4);
                data_ptr2 += imgaddr_stride;
                ptr1 += buf_stride1;
            }
            vxUnmapImagePatch(display_image, map_id2);
        }

        err = screen_post_window(screen_win, screen_buf[0], 0, NULL, 0);
        if(err != 0) {
            printf("Failed to post window\n");
        }

        if(obj->stop_screen_task == 1) {
            break;
        }
    }

    return err;
}
#endif

int app_tidl_main(int argc, char* argv[])
{
    int status = 0;

    AppObj *obj = &gAppObj;

    status = app_parse_cmd_line_args(obj, argc, argv);
    if(status == VX_SUCCESS)
    {
        status = app_init(obj);
    }
    if(status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
    }
    if(status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
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
            if(status == VX_FAILURE)
            {
                printf("Error processing graph!\n");
            }
        }
    }
    app_delete_graph(obj);
    app_deinit(obj);

    if(obj->test_mode == 1)
    {
        if((test_result == vx_false_e) || (status != VX_SUCCESS))
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

static int app_init(AppObj *obj)
{

    int status = 0;
#if !(defined(SOC_AM62A))
    int useVideo = 0;
#endif

#if defined(SOC_AM62A) && defined(QNX)
    obj->stop_screen_task = 0;
    obj->stop_screen_task_done = 0;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    obj->stop_drm_task = 0;
    obj->stop_drm_task_done = 0;
    obj->drm_handle = NULL;
#endif

    uint32_t num_input_tensors = 0;
    uint32_t num_output_tensors = 0;

    APP_PRINTF("app_tidl: Init ... \n");

    obj->context = vxCreateContext();
    APP_ASSERT_VALID_REF(obj->context);

    /* Create a vx_array object and read the config data*/
    obj->config = readConfig(obj, obj->context, &obj->tidl_config_file_path[0], &num_input_tensors, &num_output_tensors);
    APP_ASSERT_VALID_REF(obj->config)

    /* Save a copy of number of input/output tensors required as per config */
    obj->num_input_tensors  = num_input_tensors;
    obj->num_output_tensors = num_output_tensors;

    /* Create a vx_tensor object and read the network data */
    obj->network = readNetwork(obj->context, &obj->tidl_network_file_path[0]);
    APP_ASSERT_VALID_REF(obj->network)

    obj->createParams = setCreateParams(obj->context);
    APP_ASSERT_VALID_REF(obj->createParams)

    obj->inArgs = setInArgs(obj->context);
    APP_ASSERT_VALID_REF(obj->inArgs)

    obj->outArgs = setOutArgs(obj->context);
    APP_ASSERT_VALID_REF(obj->outArgs)

#ifdef APP_TIDL_TRACE_DUMP
    obj->traceData = vxCreateUserDataObject(obj->context, "TIDL_traceData", TIVX_TIDL_TRACE_DATA_SIZE, NULL);
    APP_ASSERT_VALID_REF(obj->traceData)
#endif

    obj->kernel = tivxAddKernelTIDL(obj->context, num_input_tensors, num_output_tensors);
    APP_ASSERT_VALID_REF(obj->kernel)

#if !(defined(SOC_AM62A))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        obj->disp_image = vxCreateImage(obj->context, DISPLAY_WIDTH, DISPLAY_HEIGHT, VX_DF_IMAGE_RGBX);
        APP_ASSERT_VALID_REF(obj->disp_image)

        obj->image_addr.dim_x = DISPLAY_WIDTH;
        obj->image_addr.dim_y = DISPLAY_HEIGHT;
        obj->image_addr.stride_x = 4; /* RGBX */
        obj->image_addr.stride_y = DISPLAY_WIDTH * 4;
        obj->image_addr.scale_x = VX_SCALE_UNITY;
        obj->image_addr.scale_y = VX_SCALE_UNITY;
        obj->image_addr.step_x = 1;
        obj->image_addr.step_y = 1;

        obj->disp_rect.start_x = 0;
        obj->disp_rect.start_y = 0;
        obj->disp_rect.end_x = DISPLAY_WIDTH;
        obj->disp_rect.end_y = DISPLAY_HEIGHT;

        memset(&obj->disp_params, 0, sizeof(tivx_display_params_t));

        obj->disp_params.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        obj->disp_params.pipeId = 0;
        obj->disp_params.outWidth = DISPLAY_WIDTH;
        obj->disp_params.outHeight = DISPLAY_HEIGHT;
        obj->disp_params.posX = (1920-DISPLAY_WIDTH)/2;
        obj->disp_params.posY = (1080-DISPLAY_HEIGHT)/2;

        obj->disp_params_obj = vxCreateUserDataObject(obj->context, "tivx_display_params_t", sizeof(tivx_display_params_t), &obj->disp_params);
        APP_ASSERT_VALID_REF(obj->disp_params_obj)
#if !(defined(SOC_AM62A))
        useVideo = 1;
#endif
    }

#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2)) && (obj->display_option == 1))
        {
            obj->disp_image2 = vxCreateImage(obj->context, DISPLAY_WIDTH, DISPLAY_HEIGHT, VX_DF_IMAGE_RGBX);
            APP_ASSERT_VALID_REF(obj->disp_image2)

            obj->image_addr2.dim_x = DISPLAY_WIDTH;
            obj->image_addr2.dim_y = DISPLAY_HEIGHT;
            obj->image_addr2.stride_x = 4; /* RGB */
            obj->image_addr2.stride_y = DISPLAY_WIDTH * 4;
            obj->image_addr2.scale_x = VX_SCALE_UNITY;
            obj->image_addr2.scale_y = VX_SCALE_UNITY;
            obj->image_addr2.step_x = 1;
            obj->image_addr2.step_y = 1;

            obj->disp_rect2.start_x = 0;
            obj->disp_rect2.start_y = 0;
            obj->disp_rect2.end_x = DISPLAY_WIDTH;
            obj->disp_rect2.end_y = DISPLAY_HEIGHT;

            memset(&obj->disp_params2, 0, sizeof(tivx_display_params_t));

            obj->disp_params2.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
            obj->disp_params2.pipeId = 1;
            obj->disp_params2.outWidth = DISPLAY_WIDTH;
            obj->disp_params2.outHeight = DISPLAY_HEIGHT;
            obj->disp_params2.posX = (1920-DISPLAY_WIDTH)/2;
            obj->disp_params2.posY = (1080-DISPLAY_HEIGHT)/2;

            obj->disp_params_obj2 = vxCreateUserDataObject(obj->context, "tivx_display_params_t", sizeof(tivx_display_params_t), &obj->disp_params2);
            APP_ASSERT_VALID_REF(obj->disp_params_obj2)
            useVideo = 1;
        }
    }

    if(1 == useVideo)
    {
        tivxVideoIOLoadKernels(obj->context);
    }
#endif

    tivxTIDLLoadKernels(obj->context);

    obj->pInPlanes = tivxMemAlloc(MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3, TIVX_MEM_EXTERNAL);
    if(obj->pInPlanes == NULL) {
        printf("app_tidl: ERROR: Unable to allocate memory for inPlanes, size = %d\n", MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3);
        status = -1;
    }

    obj->pOutPlanes = tivxMemAlloc(MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3, TIVX_MEM_EXTERNAL);
    if(obj->pOutPlanes == NULL) {
        printf("app_tidl: ERROR: Unable to allocate memory for outPlanes, size = %d\n", MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3);
        status = -1;
    }

    obj->pDisplayBuf888 = tivxMemAlloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * 4, TIVX_MEM_EXTERNAL);
    if(obj->pDisplayBuf888 == NULL) {
        printf("app_tidl: ERROR: Unable to allocate memory for displayBuf888, size = %d\n", DISPLAY_WIDTH * DISPLAY_HEIGHT * 3);
        status = -1;
    }

    obj->pDisplayBuf565 = tivxMemAlloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t), TIVX_MEM_EXTERNAL);
    if(obj->pDisplayBuf565 == NULL) {
        printf("app_tidl: ERROR: Unable to allocate memory for displayBuf565, size = %ld\n", DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
        status = -1;
    }

    {
      Draw2D_BufInfo sBufInfo;
      Draw2D_LinePrm sLinePrm;
      Draw2D_FontPrm sTop5Prm;

      char banner_file[APP_MAX_FILE_PATH];

      snprintf(banner_file, APP_MAX_FILE_PATH, "%s/ti_logo.bmp", obj->ti_logo_file_path);

      Draw2D_create(&obj->pHndl);

      if(obj->pHndl != NULL)
      {
          sBufInfo.bufWidth    = DISPLAY_WIDTH;
          sBufInfo.bufHeight   = DISPLAY_HEIGHT;
          sBufInfo.bufPitch[0] = DISPLAY_WIDTH * 2;
          sBufInfo.dataFormat = DRAW2D_DF_BGR16_565;
          sBufInfo.transperentColor = 0;
          sBufInfo.transperentColorFormat = DRAW2D_DF_BGR16_565;

          sBufInfo.bufAddr[0] = (uint8_t *)obj->pDisplayBuf565;

          Draw2D_setBufInfo(obj->pHndl, &sBufInfo);

          Draw2D_clearBuf(obj->pHndl);

          Draw2D_insertBmp(obj->pHndl, banner_file, 0, 0);

          sLinePrm.lineColor = RGB888_TO_RGB565(255, 255, 255);
          sLinePrm.lineSize  = 3;
          sLinePrm.lineColorFormat = DRAW2D_DF_BGR16_565;

          /* Draw a vertial line */
          Draw2D_drawLine(obj->pHndl, DISPLAY_WIDTH/2, 0, DISPLAY_WIDTH/2, DISPLAY_HEIGHT, &sLinePrm);

          /* green color heading */
          Draw2D_setFontColor(RGB888_TO_RGB565(0, 255, 0), RGB888_TO_RGB565(0, 0, 0), RGB888_TO_RGB565(0, 0, 0));

          sTop5Prm.fontIdx = 0;
          Draw2D_drawString(obj->pHndl, (DISPLAY_WIDTH/2) + 40, 140, "Top 5 classes", &sTop5Prm);

          Draw2D_resetFontColor();
      }
    }

    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->draw_perf  , "DRAW");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");

    APP_PRINTF("app_tidl: Init ... Done.\n");

    return status;
}

static void app_deinit(AppObj *obj)
{
    APP_PRINTF("app_tidl: De-init ... \n");

    tivxTIDLUnLoadKernels(obj->context);

#if !(defined(SOC_AM62A))
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        tivxVideoIOUnLoadKernels(obj->context);
    }

    vxRemoveKernel(obj->kernel);

    vxReleaseContext(&obj->context);

    tivxMemFree(obj->pInPlanes, MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3, TIVX_MEM_EXTERNAL);
    tivxMemFree(obj->pOutPlanes, MAX_IMG_WIDTH * MAX_IMG_HEIGHT * 3, TIVX_MEM_EXTERNAL);

    tivxMemFree(obj->pDisplayBuf888, DISPLAY_WIDTH * DISPLAY_HEIGHT * 4, TIVX_MEM_EXTERNAL);
    tivxMemFree(obj->pDisplayBuf565 , DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t), TIVX_MEM_EXTERNAL);


    {
        Draw2D_delete(obj->pHndl);
    }

    APP_PRINTF("app_tidl: De-init ... Done.\n");
}

static void app_delete_graph(AppObj *obj)
{
    uint32_t id;

    APP_PRINTF("app_tidl: Delete ... \n");

#if !(defined(SOC_AM62A))
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        vxReleaseNode(&obj->disp_node);
        vxReleaseGraph(&obj->disp_graph);
    }

#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2) && (obj->display_option == 1))
        {
            vxReleaseNode(&obj->disp_node2);
            vxReleaseGraph(&obj->disp_graph2);
        }
    }
#endif

    #ifdef APP_TIVX_LOG_RT_ENABLE
    tivxLogRtTraceExportToFile("app_tidl.bin");
    tivxLogRtTraceDisable(obj->graph);
    #endif

    vxReleaseNode(&obj->tidl_node);
    vxReleaseGraph(&obj->graph);

    vxReleaseUserDataObject(&obj->config);
    vxReleaseUserDataObject(&obj->network);

    vxReleaseUserDataObject(&obj->createParams);
    vxReleaseUserDataObject(&obj->inArgs);
    vxReleaseUserDataObject(&obj->outArgs);

#ifdef APP_TIDL_TRACE_DUMP
    vxReleaseUserDataObject(&obj->traceData);
#endif

    for(id = 0; id < obj->num_input_tensors; id++)
    {
        vxReleaseTensor(&obj->input_tensors[id]);
    }

    for(id = 0; id < obj->num_output_tensors; id++)
    {
        vxReleaseTensor(&obj->output_tensors[id]);
    }

#if !(defined(SOC_AM62A))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        vxReleaseImage(&obj->disp_image);
        vxReleaseUserDataObject(&obj->disp_params_obj);
    }

#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2)) && (obj->display_option == 1))
        {
            vxReleaseImage(&obj->disp_image2);
            vxReleaseUserDataObject(&obj->disp_params_obj2);
        }
    }
#endif

    APP_PRINTF("app_tidl: Delete ... Done.\n");
}

static void app_show_usage(int argc, char* argv[])
{
    printf("\n");
    printf(" TIDL Demo - (c) Texas Instruments 2018-2026\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static void app_set_cfg_default(AppObj *obj)
{
    snprintf(obj->tidl_config_file_path,APP_MAX_FILE_PATH, "test_data/tivx/tidl_models/mobilenetv1/config.bin");
    snprintf(obj->tidl_network_file_path,APP_MAX_FILE_PATH, "test_data/tivx/tidl_models/mobilenetv1/network.bin");
    snprintf(obj->input_file_path,APP_MAX_FILE_PATH, "test_data/psdkra/app_tidl");
    snprintf(obj->input_file_list,APP_MAX_FILE_PATH, "test_data/psdkra/app_tidl/names.txt");
    snprintf(obj->output_file_path,APP_MAX_FILE_PATH, "app_tidl_out");
    snprintf(obj->ti_logo_file_path,APP_MAX_FILE_PATH, "test_data/tivx/tidl_models/");
    obj->display_option = 1;
    obj->delay_in_msecs = 0;
    obj->num_iterations = 1;
    obj->is_interactive = 0;
    obj->test_mode      = 0;
}

static int app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;

    if(fp==NULL)
    {
        printf("app_tidl: ERROR: Unable to open config file [%s]\n", cfg_file_name);
        return -1;
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

        if(token != NULL)
        {
            if(strcmp(token, "tidl_config_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->tidl_config_file_path, token);
                }
            }
            else
            if(strcmp(token, "tidl_network_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->tidl_network_file_path, token);
                  /* for testing if relevant */
                  if(strstr(obj->tidl_network_file_path, "u16") != NULL)
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
            if(strcmp(token, "ti_logo_file_path")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->ti_logo_file_path, token);
                }
            }
            else
            if(strcmp(token, "input_file_list")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  strcpy(obj->input_file_list, token);
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
            if(strcmp(token, "display_option")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  obj->display_option = atoi(token);
                  if(obj->display_option > 1)
                      obj->display_option = 1;
                }
            }
            else
            if(strcmp(token, "delay")==0)
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
            if(strcmp(token, "use_dual_display")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                  token[strlen(token)-1]=0;
                  obj->use_dual_display = atoi(token);
                }
            }
        }
        if (obj->test_mode == 1)
        {
          obj->is_interactive = 0;
          /* display_option must be set to 1 in order for the checksums
              to come out correctly */
#ifndef x86_64
          printf("Turning display option on ... \n");
          obj->display_option = 1;
#else
          printf("Turning display option off ... \n");
          obj->display_option = 0;
#endif
        }
    }

    fclose(fp);

    return 0;
}

static int app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    int i;
    vx_bool set_test_mode = vx_false_e;

    app_set_cfg_default(obj);

    if(argc==1)
    {
        app_show_usage(argc, argv);
        return -1;
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
            return -1;
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
        obj->display_option = 1;
        obj->delay_in_msecs = 100;
    }

    #ifdef x86_64
    printf("Turning display option off ... \n");
    obj->display_option = 0;
    obj->is_interactive = 0;
    #endif

    return 0;
}

static void initParam(vx_reference params[], uint32_t _max_params)
{
   num_params  = 0;
   max_params = _max_params;
}

static void addParam(vx_reference params[], vx_reference obj)
{
   APP_ASSERT(num_params <= max_params);

   params[num_params] = obj;

   num_params++;
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_reference params[APP_TIDL_MAX_PARAMS];
    uint32_t i;

    APP_PRINTF("app_tidl: Creating graph ... \n");

    /* Create OpenVx Graph */
    obj->graph = vxCreateGraph(obj->context);
    APP_ASSERT_VALID_REF(obj->graph)
    vxSetReferenceName((vx_reference)obj->graph, "app_tidl_graph");

    /* Create array of input tensors */
    createInputTensors(obj, obj->context, obj->config, obj->input_tensors);

    /* Create array of output tensors */
    createOutputTensors(obj, obj->context, obj->config, obj->output_tensors);

    /* Initialize param array */
    initParam(params, APP_TIDL_MAX_PARAMS);

    /* The 1st param MUST be config */
    addParam(params, (vx_reference)obj->config);

    /* The 2nd param MUST be network */
    addParam(params, (vx_reference)obj->network);

    /* The 3rd param MUST be create params */
    addParam(params, (vx_reference)obj->createParams);

    /* The 4th param MUST be inArgs */
    addParam(params, (vx_reference)obj->inArgs);

    /* The 5th param MUST be outArgs */
    addParam(params, (vx_reference)obj->outArgs);

#ifdef APP_TIDL_TRACE_DUMP
    addParam(params, (vx_reference)obj->traceData);
#else
/* The 6th param MUST be NULL if trace data dump is not enabled */
    addParam(params, (vx_reference)NULL);
#endif

    /* Create TIDL Node */
    obj->tidl_node = tivxTIDLNode(obj->graph, obj->kernel, params, obj->input_tensors, obj->output_tensors);
    APP_ASSERT_VALID_REF(obj->tidl_node)

    vxSetNodeTarget(obj->tidl_node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);

#if !(defined(SOC_AM62A))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        /* Create OpenVx Graph */
        obj->disp_graph = vxCreateGraph(obj->context);
        APP_ASSERT_VALID_REF(obj->disp_graph)
        vxSetReferenceName((vx_reference)obj->disp_graph, "Display");

#if !(defined(SOC_AM62A))
        obj->disp_node = tivxDisplayNode(obj->disp_graph, obj->disp_params_obj, obj->disp_image);
        APP_ASSERT_VALID_REF(obj->disp_node)

        vxSetNodeTarget(obj->disp_node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1);
#endif
    }
#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2)) && (obj->display_option == 1))
        {
            /* Create OpenVx Graph */
            obj->disp_graph2 = vxCreateGraph(obj->context);
            APP_ASSERT_VALID_REF(obj->disp_graph2)
            vxSetReferenceName((vx_reference)obj->disp_graph2, "Display2");

            obj->disp_node2 = tivxDisplayNode(obj->disp_graph2, obj->disp_params_obj2, obj->disp_image2);
            APP_ASSERT_VALID_REF(obj->disp_node2)

            vxSetNodeTarget(obj->disp_node2, VX_TARGET_STRING, TIVX_TARGET_DISPLAY2);
        }
    }
#endif

    /* Set names for diferent OpenVX objects */
    vxSetReferenceName((vx_reference)obj->config, "Config");
    vxSetReferenceName((vx_reference)obj->network, "Network");
    vxSetReferenceName((vx_reference)obj->createParams, "CreateParams");
    vxSetReferenceName((vx_reference)obj->inArgs, "InArgs");
    vxSetReferenceName((vx_reference)obj->outArgs, "OutArgs");

    for(i = 0; i < obj->num_input_tensors; i++) {
        char tensor_name[] = "InputTensor_";
        char ref_name[64];
        snprintf(ref_name, 64, "%s%s", tensor_name, tensor_num_str[i]);
        vxSetReferenceName((vx_reference)obj->input_tensors[i], ref_name);
    }

    for(i = 0; i < obj->num_output_tensors; i++) {
        char tensor_name[] = "OutputTensor_";
        char ref_name[64];
        snprintf(ref_name, 64, "%s%s", tensor_name, tensor_num_str[i]);
        vxSetReferenceName((vx_reference)obj->output_tensors[i], ref_name);
    }

    vxSetReferenceName((vx_reference)obj->kernel, "TIDLKernel");
    vxSetReferenceName((vx_reference)obj->tidl_node, "TIDLNode");

#if !(defined(SOC_AM62A))
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1) && (obj->display_option == 1))
#else
    if (obj->display_option == 1)
#endif
    {
        vxSetReferenceName((vx_reference)obj->disp_params_obj, "DisplayParams");
        vxSetReferenceName((vx_reference)obj->disp_node, "DisplayNode");
    }

#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2) && (obj->display_option == 1))
        {
            vxSetReferenceName((vx_reference)obj->disp_params_obj2, "DisplayParams2");
            vxSetReferenceName((vx_reference)obj->disp_node2, "DisplayNode2");
        }
    }
#endif

    APP_PRINTF("app_tidl: Creating graph ... Done.\n");

    return status;
}

static void app_run_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;
    vx_status status = VX_SUCCESS;
    appPerfStatsCpuLoadResetAll();

    if(!obj->stop_task)
    {
        status = app_run_graph(obj);
        if(status == VX_FAILURE)
        {
            printf("Error processing graph!\n");
        }
        obj->stop_task = 1;
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

#if defined(SOC_AM62A) && defined(QNX)
static void app_run_screen_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;

    app_run_screen(obj);

    obj->stop_screen_task_done = 1;
}

static int32_t app_screen_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    int32_t status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_screen_task;
    params.app_var = obj;

    obj->stop_screen_task_done = 0;
    obj->stop_screen_task = 0;

    status = tivxTaskCreate(&obj->screen_task, &params);

    return status;
}

static void app_run_screen_task_delete(AppObj *obj)
{
    while(obj->stop_screen_task_done==0)
    {
         tivxTaskWaitMsecs(100);
    }

    screen_destroy_window(screen_win);
    screen_destroy_context(screen_ctx);
    tivxTaskDelete(&obj->screen_task);
}
#endif

#if defined(SOC_AM62A) && defined(LINUX) && !defined(x86_64)

/**
 * \brief Fill an NV12 image with black (Y=0, UV=128)
 */
static void app_drm_clear_nv12(vx_image image, vx_uint32 w, vx_uint32 h)
{
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr;
    vx_map_id map;
    void *ptr = NULL;
    vx_uint32 row;

    rect.start_x = 0; rect.start_y = 0;
    rect.end_x = w; rect.end_y = h;

    /* Y plane: fill with 0 (black luma) */
    vxMapImagePatch(image, &rect, 0, &map, &addr, &ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    if (ptr != NULL)
    {
        for (row = 0; row < h; row++)
        {
            memset((uint8_t *)ptr + row * addr.stride_y, 0, w);
        }
    }
    vxUnmapImagePatch(image, map);

    /* UV plane: fill with 128 (neutral chroma) */
    vxMapImagePatch(image, &rect, 1, &map, &addr, &ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    if (ptr != NULL)
    {
        for (row = 0; row < h / 2; row++)
        {
            memset((uint8_t *)ptr + row * addr.stride_y, 128, w);
        }
    }
    vxUnmapImagePatch(image, map);
}

/**
 * \brief Blit a RGB source image centered into an NV12 canvas
 *
 * Converts RGB (3 bytes/pixel, VX_DF_IMAGE_RGB) to NV12 (YUV 4:2:0)
 * while centering the source in the canvas. AM62A DSS (tidss) only
 * supports NV12 on its available plane, so all DRM display goes through
 * this format regardless of the TIOVX display image format.
 */
static void app_drm_blit_rgb_to_nv12(vx_image canvas, vx_image src,
                                       vx_uint32 canvas_w, vx_uint32 canvas_h,
                                       vx_uint32 src_w, vx_uint32 src_h)
{
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t src_addr, y_addr, uv_addr;
    vx_map_id src_map, y_map, uv_map;
    void *src_ptr = NULL, *y_ptr = NULL, *uv_ptr = NULL;
    vx_uint32 offset_x, offset_y;
    vx_uint32 row, col;

    /* Offsets must be even for NV12 chroma subsampling alignment */
    offset_x = ((canvas_w - src_w) / 2) & ~1u;
    offset_y = ((canvas_h - src_h) / 2) & ~1u;

    /* Map source RGB plane */
    rect.start_x = 0; rect.start_y = 0;
    rect.end_x = src_w; rect.end_y = src_h;
    vxMapImagePatch(src, &rect, 0, &src_map, &src_addr, &src_ptr,
                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    /* Map canvas Y plane */
    rect.end_x = canvas_w; rect.end_y = canvas_h;
    vxMapImagePatch(canvas, &rect, 0, &y_map, &y_addr, &y_ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    /* Map canvas UV plane */
    vxMapImagePatch(canvas, &rect, 1, &uv_map, &uv_addr, &uv_ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    if (src_ptr != NULL && y_ptr != NULL && uv_ptr != NULL)
    {
        for (row = 0; row < src_h; row++)
        {
            uint8_t *s  = (uint8_t *)src_ptr + row * src_addr.stride_y;
            uint8_t *dy = (uint8_t *)y_ptr   + (offset_y + row) * y_addr.stride_y + offset_x;
            uint8_t *uv = (uint8_t *)uv_ptr  + ((offset_y / 2) + (row / 2)) * uv_addr.stride_y + offset_x;

            for (col = 0; col < src_w; col++)
            {
                uint8_t r = s[col * 3 + 0];
                uint8_t g = s[col * 3 + 1];
                uint8_t b = s[col * 3 + 2];

                /* BT.601 full-range RGB -> YUV */
                dy[col] = (uint8_t)(( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16;

                /* Write UV for top-left pixel of each 2x2 block */
                if ((row % 2 == 0) && (col % 2 == 0))
                {
                    uv[col]     = (uint8_t)((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128; /* U/Cb */
                    uv[col + 1] = (uint8_t)((112 * r -  94 * g -  18 * b + 128) >> 8) + 128; /* V/Cr */
                }
            }
        }
    }

    vxUnmapImagePatch(src, src_map);
    vxUnmapImagePatch(canvas, y_map);
    vxUnmapImagePatch(canvas, uv_map);
}

/**
 * \brief Linux DRM display task function using zero-copy via DMA-BUF
 *
 * Creates an NV12 canvas at the display resolution (DRM_FORMAT_NV12,
 * the only format supported by AM62A DSS tidss on its available plane)
 * and blits the RGB display image into it each frame, centering it with
 * black borders when the image is smaller than the display resolution.
 */
int32_t app_run_drm_display(AppObj *obj)
{
    int32_t status = 0;
    app_drm_wrapper_cfg_t drm_cfg;
    app_drm_wrapper_handle_t *drm_handle = NULL;
    vx_uint32 width, height;
    uint32_t buf_idx = 0;
    vx_image canvas = NULL;
    uint32_t disp_w = 1920;
    uint32_t disp_h = 1080;

    /* Query display image properties */
    vx_image display_image = obj->disp_image;
    vxQueryImage(display_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(display_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    /*
     * Query the actual display resolution from the DRM CRTC mode.
     */
    {
        int tmp_fd;
        drmModeCrtcPtr crtc;

        appDrmWrapperInitCfg(&drm_cfg);

        tmp_fd = drmOpen("tidss", NULL);
        if (tmp_fd >= 0)
        {
            crtc = drmModeGetCrtc(tmp_fd, drm_cfg.crtc_id);
            if (crtc != NULL && crtc->mode_valid)
            {
                disp_w = crtc->mode.hdisplay;
                disp_h = crtc->mode.vdisplay;
                drmModeFreeCrtc(crtc);
            }
            close(tmp_fd);
        }

        printf("[DRM] Display resolution: %ux%u, Image resolution: %ux%u\n",
               disp_w, disp_h, width, height);
    }

    if (width < disp_w || height < disp_h)
    {
        printf("[DRM] Letterbox mode: %ux%u centered in %ux%u\n",
               width, height, disp_w, disp_h);
    }

    /*
     * Always create an NV12 canvas at display resolution.
     * AM62A DSS (tidss) only supports DRM_FORMAT_NV12 on its available
     * plane, so the RGB display image is converted to NV12 on each blit.
     */
    canvas = vxCreateImage(obj->context, disp_w, disp_h, VX_DF_IMAGE_NV12);
    if (canvas == NULL)
    {
        printf("[DRM] Failed to create NV12 canvas\n");
        return -1;
    }

    app_drm_clear_nv12(canvas, disp_w, disp_h);

    drm_cfg.width      = disp_w;
    drm_cfg.height     = disp_h;
    drm_cfg.pix_format = DRM_FORMAT_NV12;
    drm_cfg.bufq_depth = 1;

    drm_handle = appDrmWrapperCreate(&drm_cfg);
    if (drm_handle == NULL)
    {
        printf("[DRM] Failed to create DRM wrapper\n");
        vxReleaseImage(&canvas);
        return -1;
    }

    obj->drm_handle = (void *)drm_handle;

    /* Initial blit before registering so the buffer has valid content */
    app_drm_blit_rgb_to_nv12(canvas, display_image, disp_w, disp_h, width, height);

    status = appDrmWrapperRegisterBuffer(drm_handle, canvas, 0);
    if (status < 0)
    {
        printf("[DRM] Failed to register buffer\n");
        appDrmWrapperDelete(drm_handle);
        obj->drm_handle = NULL;
        vxReleaseImage(&canvas);
        return -1;
    }

    printf("[DRM] DRM display initialized: %ux%u (NV12)\n", disp_w, disp_h);

    /* Main display loop */
    while (1)
    {
        app_drm_blit_rgb_to_nv12(canvas, display_image, disp_w, disp_h, width, height);

        status = appDrmWrapperRender(drm_handle, buf_idx);
        if (status < 0)
        {
            printf("[DRM] Render failed\n");
            break;
        }

        if (obj->stop_drm_task == 1)
        {
            break;
        }
    }

    vxReleaseImage(&canvas);

    return status;
}

static void app_run_drm_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;

    app_run_drm_display(obj);

    obj->stop_drm_task_done = 1;
}

static int32_t app_drm_task_create(AppObj *obj)
{
    tivx_task_create_params_t params;
    int32_t status;

    tivxTaskSetDefaultCreateParams(&params);
    params.task_main = app_run_drm_task;
    params.app_var = obj;

    obj->stop_drm_task_done = 0;
    obj->stop_drm_task = 0;
    obj->drm_handle = NULL;

    status = tivxTaskCreate(&obj->drm_task, &params);

    return status;
}

static void app_run_drm_task_delete(AppObj *obj)
{
    while (obj->stop_drm_task_done == 0)
    {
        tivxTaskWaitMsecs(100);
    }

    /* Clean up DRM resources */
    if (obj->drm_handle != NULL)
    {
        appDrmWrapperDelete((app_drm_wrapper_handle_t *)obj->drm_handle);
        obj->drm_handle = NULL;
    }

    tivxTaskDelete(&obj->drm_task);
}
#endif

static const char menu[] = {
    "\n"
    "\n ================================="
    "\n Demo : TIDL Object Classification"
    "\n ================================="
    "\n"
    "\n p: Print performance statistics"
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
    if(status != VX_SUCCESS)
    {
        printf("app_tidl: ERROR: Unable to create task\n");
    }
    else
    {
        appPerfStatsResetAll();
        while((!done) && (status == VX_SUCCESS) && (!obj->stop_task_done))
        {
            printf(menu);
            ch = getchar();
            printf("\n");

            switch(ch)
            {
                case 'p':
                    appPerfStatsPrintAll();
                    if(status == VX_SUCCESS)
                    {
                        status = tivx_utils_graph_perf_print(obj->graph);
                    }
                    if(status == VX_SUCCESS)
                    {
                        status = tivx_utils_graph_perf_print(obj->disp_graph);
                    }
                    appPerfPointPrint(&obj->fileio_perf);
                    appPerfPointPrint(&obj->draw_perf);
                    appPerfPointPrint(&obj->total_perf);
                    printf("\n");
                    appPerfPointPrintFPS(&obj->total_perf);
                    printf("\n");
                    break;
                case 'e':
                    perf_arr[0] = &obj->total_perf;
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl");
                    if((NULL != fp) && (status == VX_SUCCESS))
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
#if defined(SOC_AM62A) && defined(QNX)
                    obj->stop_screen_task = 1;
#endif
#if defined(SOC_AM62A) && defined(LINUX)
                    obj->stop_drm_task = 1;
#endif
                    done = 1;
                    break;
            }
        }
        app_run_task_delete(obj);
#if defined(SOC_AM62A) && defined(QNX)
        app_run_screen_task_delete(obj);
#endif
#if defined(SOC_AM62A) && defined(LINUX) && !defined(x86_64)
        app_run_drm_task_delete(obj);
#endif
    }
    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    APP_PRINTF("app_tidl: Verifying graph ... \n");

    /* Verify the TIDL Graph */
    status = vxVerifyGraph(obj->graph);
    if(status != VX_SUCCESS)
    {
        printf("app_tidl: ERROR: Verifying graph ... Failed !!!\n");
        return status;
    }

    APP_PRINTF("app_tidl: Verifying graph ... Done.\n");

#if !(defined(SOC_AM62A))
    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (obj->display_option == 1))
#else
    if(obj->display_option == 1)
#endif
    {
        APP_PRINTF("app_tidl: Verifying display graph ... \n");

        /* Verify the TIDL Graph */
        status = vxVerifyGraph(obj->disp_graph);
        if(status != VX_SUCCESS)
        {
            printf("app_tidl: ERROR: Verifying display graph ... Failed !!!\n");
            return status;
        }

        APP_PRINTF("app_tidl: Verifying display graph for display 1 ... Done.\n");
    }

#if !(defined(SOC_AM62A))
    if (obj->use_dual_display == 1)
    {
        if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2)) && (obj->display_option == 1))
        {
            APP_PRINTF("app_tidl: Verifying display graph ... \n");

            /* Verify the TIDL Graph */
            status = vxVerifyGraph(obj->disp_graph2);
            if(status != VX_SUCCESS)
            {
                printf("app_tidl: ERROR: Verifying display for display 2 ... Failed !!!\n");
                return status;
            }

            APP_PRINTF("app_tidl: Verifying display graph ... Done.\n");
        }
    }
#endif

    /* wait a while for prints to flush */
    tivxTaskWaitMsecs(100);

#if 1
    if(status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl");
        APP_ASSERT(status==VX_SUCCESS);
    }
#endif

    #ifdef APP_TIVX_LOG_RT_ENABLE
    tivxLogRtTraceEnable(obj->graph);
    #endif

    return status;
}

static vx_status app_run_graph_for_one_frame(AppObj *obj, char *curFileName, vx_uint32 counter)
{
    vx_status status = VX_SUCCESS;

    vx_char input_file_name[APP_MAX_FILE_PATH];
    vx_char output_file_name[APP_MAX_FILE_PATH];
    vx_uint32 tensor_actual_checksum = 0, display_actual_checksum = 0;

    appPerfPointBegin(&obj->total_perf);

    snprintf(input_file_name, APP_MAX_FILE_PATH-1, "%s/%s",
          obj->input_file_path,
          curFileName
          );

    APP_PRINTF("app_tidl: Reading input file %s ... ", input_file_name);

    appPerfPointBegin(&obj->fileio_perf);

    /* Read input from file and poplulate the input tensors */
    if(status == VX_SUCCESS)
    {
        status = readInput(obj, obj->context, obj->config, obj->input_tensors, &input_file_name[0]);
    }
    appPerfPointEnd(&obj->fileio_perf);

    APP_PRINTF("Done!\n");

#ifdef APP_WRITE_PRE_PROC_OUTPUT
    vx_char pre_proc_file_name[APP_MAX_FILE_PATH];

    snprintf(pre_proc_file_name, APP_MAX_FILE_PATH-1, "%s/%s",
          obj->output_file_path,
          "pre_proc_out"
          );

    writePreProcOutput(pre_proc_file_name, obj->input_tensors[0]);
#endif

    APP_PRINTF("app_tidl: Running Graph ... ");
    /* Execute the network */
    if(status == VX_SUCCESS)
    {
        status = vxProcessGraph(obj->graph);
    }
    APP_PRINTF("Done!\n");

    if(status == VX_SUCCESS)
    {
        snprintf(output_file_name, APP_MAX_FILE_PATH-1, "%s/%s",
              obj->output_file_path,
              curFileName
              );

        appPerfPointBegin(&obj->draw_perf);

        /* Display the output */
        displayOutput(obj, obj->config, obj->output_tensors, &output_file_name[0]);


        appPerfPointEnd(&obj->draw_perf);

#if !(defined(SOC_AM62A))
        if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY1)) && (obj->display_option == 1)) {
#else
        if(obj->display_option == 1) {
#endif
            /* At this point, the output is ready to copy the updated buffer */
            if(status == VX_SUCCESS)
            {
                status = vxCopyImagePatch(obj->disp_image,
                                        &obj->disp_rect,
                                        0,
                                        &obj->image_addr,
                                        (void *)obj->pDisplayBuf888,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST
                                        );
            }
            APP_PRINTF("app_tidl: Running display graph ... \n");
            /* Execute the display graph */
#if !(defined(SOC_AM62A))
            if(status == VX_SUCCESS)
            {
                status = vxProcessGraph(obj->disp_graph);
            }
            APP_PRINTF("app_tidl: Running display graph ... Done.\n");
#endif
        }

#if !(defined(SOC_AM62A))
        if (obj->use_dual_display == 1)
        {
            if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY2)) && (obj->display_option == 1)) {
                /* At this point, the output is ready to copy the updated buffer */
                if(status == VX_SUCCESS)
                {
                    status = vxCopyImagePatch(obj->disp_image2,
                                            &obj->disp_rect2,
                                            0,
                                            &obj->image_addr2,
                                            (void *)obj->pDisplayBuf888,
                                            VX_WRITE_ONLY,
                                            VX_MEMORY_TYPE_HOST
                                            );
                }
                APP_PRINTF("app_tidl: Running display graph 2... \n");
                /* Execute the display graph */
                if(status == VX_SUCCESS)
                {
                    status = vxProcessGraph(obj->disp_graph2);
                }
                APP_PRINTF("app_tidl: Running display graph ... Done.\n");
            }
        }
#endif

        /* Check that you are within the first n frames, where n is the number
            of samples in the checksums_expected */
        if ((obj->test_mode == 1) &&
            (counter < ((sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0]))-1)))
        {
            /* numOutputbuf is 1 here, but for the sake of generalizing
                in the future, the loop will remain */
            if(vx_false_e == app_test_check_tensor(obj->output_tensors[0],
                                                    getTensorDataType(obj->ioBufDesc.outElementType[0]),
                                                    obj->ioBufDesc.outWidth[0], obj->ioBufDesc.outPadL[0],
                                                    obj->ioBufDesc.outPadR[0], obj->ioBufDesc.outHeight[0],
                                                    obj->ioBufDesc.outPadT[0], obj->ioBufDesc.outPadB[0],
                                                    obj->ioBufDesc.outNumChannels[0], checksums_expected[obj->test_case*2][counter],
                                                    &tensor_actual_checksum))
            {
                test_result = vx_false_e;
            }
            /* in case test fails and needs to change */
            populate_gatherer(0 + (obj->test_case*2), counter, tensor_actual_checksum);
#ifndef x86_64
            if(vx_false_e == app_test_check_image(obj->disp_image, checksums_expected[1+(2*obj->test_case)][counter],
                                                    &display_actual_checksum))
            {
                test_result = vx_false_e;
            }
            populate_gatherer(1 + (obj->test_case*2), counter, display_actual_checksum);
#endif

        }
    }

    appPerfPointEnd(&obj->total_perf);

#ifdef APP_TIDL_TRACE_DUMP
    tivx_utils_tidl_trace_write(obj->traceData, curFileName);
#endif

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_char curFileName[APP_MAX_FILE_PATH];
    uint64_t cur_time;
    uint64_t max_frames = INT64_MAX;
    vx_uint32 counter = 0;
    FILE* test_case_file;
    uint32_t cur_iteration;

    printf("network file: %s\n", obj->tidl_network_file_path);
    printf("config  file: %s\n", obj->tidl_config_file_path);

#if defined(SOC_AM62A) && defined(QNX)
    status = app_screen_task_create(obj);
    if(status!=0)
    {
        printf("ERROR: Unable to create screen task\n");
    }
#endif
#if defined(SOC_AM62A) && defined(LINUX) && !defined(x86_64)
    status = app_drm_task_create(obj);
    if(status!=0)
    {
        printf("ERROR: Unable to create DRM display task\n");
    }
#endif

    if (obj->test_mode == 1)
    {
        /* This way if the application failed at any point up to here
            the test mode would return a failure */
        test_result = vx_true_e;
        obj->num_iterations = 1;
        max_frames = (sizeof(checksums_expected[0])/sizeof(checksums_expected[0][0])) + TEST_BUFFER;
    }
    for(cur_iteration=0; cur_iteration<obj->num_iterations; cur_iteration++)
    {
        printf("Iteration %d of %d ... \n", cur_iteration+1, obj->num_iterations);

        test_case_file =  fopen(obj->input_file_list,"r");
        if(test_case_file==NULL)
        {
            break;
        }
        while (fgets(curFileName, sizeof(curFileName), test_case_file) && (counter < max_frames))
        {
            curFileName[strlen(curFileName) - 1] = 0;

            cur_time = tivxPlatformGetTimeInUsecs();

            APP_PRINTF("Classifying input %s ...\n", curFileName);
            if(status == VX_SUCCESS)
            {
                status = app_run_graph_for_one_frame(obj, curFileName, counter++);
            }
            APP_PRINTF("Classifying input %s ...Done!\n", curFileName);

            cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
            /* convert to msecs */
            cur_time = cur_time/1000;

            if(cur_time < obj->delay_in_msecs)
            {
                tivxTaskWaitMsecs(obj->delay_in_msecs - cur_time);
            }

            /* user asked to stop processing */
            if(obj->stop_task || (status != VX_SUCCESS))
            {
                break;
            }
       }
       fclose(test_case_file);

       if(obj->stop_task || (status != VX_SUCCESS))
       {
           break;
       }
    }

    return status;
}

static vx_user_data_object readConfig(AppObj *obj, vx_context context, char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors)
{
    vx_status status = VX_SUCCESS;

    tivxTIDLJ7Params  *tidlParams = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;
    vx_user_data_object   config = NULL;
    vx_uint32 capacity;
    vx_map_id map_id;

    FILE *fp_config;
    vx_size read_count;

    APP_PRINTF("app_tidl: Reading config file %s ...\n", config_file);

    fp_config = fopen(config_file, "rb");

    if(fp_config == NULL)
    {
        printf("app_tidl: ERROR: Unable to open IO config file %s \n", config_file);

        return NULL;
    }

    fseek(fp_config, 0, SEEK_END);
    capacity = ftell(fp_config);
    fseek(fp_config, 0, SEEK_SET);

    if( capacity != sizeof(sTIDL_IOBufDesc_t) )
    {
        printf("app_tidl: ERROR: Config file size (%d bytes) does not match size of sTIDL_IOBufDesc_t (%d bytes)\n", capacity, (vx_uint32)sizeof(sTIDL_IOBufDesc_t));
        fclose(fp_config);
        return NULL;
    }


    /* Create a user struct type for handling config data*/
    config = vxCreateUserDataObject(context, "tivxTIDLJ7Params", sizeof(tivxTIDLJ7Params), NULL );

    status = vxGetStatus((vx_reference)config);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id,
                            (void **)&tidlParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(tidlParams == NULL)
            {
              printf("app_tidl: ERROR: Map of config object failed\n");
              fclose(fp_config);
              return NULL;
            }

            tivx_tidl_j7_params_init(tidlParams);

            ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

            read_count = fread(ioBufDesc, capacity, 1, fp_config);
            if(read_count != 1)
            {
              printf("app_tidl: ERROR: Unable to read config file\n");
            }
            fclose(fp_config);

            memcpy(&obj->ioBufDesc, ioBufDesc, capacity);

            *num_input_tensors  = ioBufDesc->numInputBuf;
            *num_output_tensors = ioBufDesc->numOutputBuf;
            if(status == VX_SUCCESS)
            {
                status = vxUnmapUserDataObject(config, map_id);
            }
        }
        else
        {
            fclose(fp_config);
        }
    }
    else
    {
        fclose(fp_config);
    }

    APP_PRINTF("app_tidl: Reading config file %s ... Done. %d bytes\n", config_file, (uint32_t)capacity);
    APP_PRINTF("app_tidl: Tensors, input = %d, output = %d\n", *num_input_tensors, *num_output_tensors);

    return config;
}

static vx_user_data_object readNetwork(vx_context context, char *network_file)
{
    vx_status status;

    vx_user_data_object  network;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void      *network_buffer = NULL;
    vx_size read_count;

    FILE *fp_network;

    APP_PRINTF("app_tidl: Reading network file %s ... \n", network_file);

    fp_network = fopen(network_file, "rb");

    if(fp_network == NULL)
    {
        printf("app_tidl: ERROR: Unable to open network file %s \n", network_file);

        return NULL;
    }
    fseek(fp_network, 0, SEEK_END);
    capacity = ftell(fp_network);
    fseek(fp_network, 0, SEEK_SET);

    network = vxCreateUserDataObject(context, "TIDL_network", capacity, NULL );

    status = vxGetStatus((vx_reference)network);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(network, 0, capacity, &map_id,
                        (void **)&network_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(network_buffer)
            {
                read_count = fread(network_buffer, capacity, 1, fp_network);
                if(read_count != 1)
                {
                    printf("app_tidl: ERROR: Unable to read network file\n");
                }
                fclose(fp_network);
            }
            else
            {
                printf("app_tidl: ERROR: Unable to allocate memory for reading network file of %d bytes\n", capacity);
                fclose(fp_network);
            }
            if(status == VX_SUCCESS)
            {
                status = vxUnmapUserDataObject(network, map_id);
            }
        }
        else
        {
            fclose(fp_network);
        }
    }
    else
    {
        fclose(fp_network);
    }

    APP_PRINTF("app_tidl: Reading network file %s ... Done. %d bytes\n", network_file, (uint32_t)capacity);

    return network;
}

static vx_user_data_object setCreateParams(vx_context context)
{
    vx_status status;

    vx_user_data_object  createParams;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    capacity = sizeof(TIDL_CreateParams);
    createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );

    status = vxGetStatus((vx_reference)createParams);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(createParams, 0, capacity, &map_id,
                        (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(createParams_buffer)
            {
                TIDL_CreateParams *prms = createParams_buffer;

                TIDL_createParamsInit(prms);

                prms->isInbufsPaded                 = 1;
                prms->quantRangeExpansionFactor     = 1.0;
                prms->quantRangeUpdateFactor        = 0.0;
#ifdef x86_64
                prms->flowCtrl                      = 1;
#endif
#ifdef APP_TIDL_TRACE_DUMP
                prms->traceLogLevel                 = 1;
                prms->traceWriteLevel               = 1;
                strcpy(prms->traceBaseName, "/tmp/tidl_trace_");
#else
                prms->traceLogLevel                 = 0;
                prms->traceWriteLevel               = 0;
#endif

            }
            else
            {
                printf("app_tidl: ERROR: Unable to allocate memory for create time params! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(createParams, map_id);
        }
    }

    return createParams;
}

static vx_user_data_object setInArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  inArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *inArgs_buffer = NULL;

    capacity = sizeof(TIDL_InArgs);
    inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)inArgs);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(inArgs, 0, capacity, &map_id,
                        (void **)&inArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(inArgs_buffer)
            {
              TIDL_InArgs *prms = inArgs_buffer;
              prms->iVisionInArgs.size         = sizeof(TIDL_InArgs);
              prms->iVisionInArgs.subFrameInfo = 0;
            }
            else
            {
                printf("app_tidl: Unable to allocate memory for inArgs! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(inArgs, map_id);
        }
    }

    return inArgs;
}

static vx_user_data_object setOutArgs(vx_context context)
{
    vx_status status;

    vx_user_data_object  outArgs;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;

    capacity = sizeof(TIDL_outArgs);
    outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );

    status = vxGetStatus((vx_reference)outArgs);

    if (VX_SUCCESS == status)
    {
        status = vxMapUserDataObject(outArgs, 0, capacity, &map_id,
                        (void **)&outArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (VX_SUCCESS == status)
        {
            if(outArgs_buffer)
            {
              TIDL_outArgs *prms = outArgs_buffer;
              prms->iVisionOutArgs.size         = sizeof(TIDL_outArgs);
            }
            else
            {
                printf("app_tidl: Unable to allocate memory for outArgs! %d bytes\n", capacity);
            }

            vxUnmapUserDataObject(outArgs, map_id);
        }
    }

    return outArgs;
}

static void createInputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors)
{
    vx_size   input_sizes[APP_MAX_TENSOR_DIMS];

    uint32_t id;

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < ioBufDesc->numInputBuf; id++) {

        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        vx_size data_type = getTensorDataType(ioBufDesc->inElementType[id]);

        if(data_type != VX_TYPE_INVALID)
          input_tensors[id] = vxCreateTensor(context, 3, input_sizes, data_type, 0);
    }

    return;
}

static void createOutputTensors(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *output_tensors)
{
    vx_size output_sizes[APP_MAX_TENSOR_DIMS];

    uint32_t id;

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < ioBufDesc->numOutputBuf; id++) {

        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = ioBufDesc->outNumChannels[id];

        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);

        if(data_type != VX_TYPE_INVALID)
        {
          output_tensors[id] = vxCreateTensor(context, 3, output_sizes, data_type, 0);
        }
    }
    return;
}
static void resizeImage(uint8_t *inImg, uint8_t *outImg, uint32_t inWidth, uint32_t inHeight, uint32_t inStride, uint32_t outWidth, uint32_t outHeight, uint32_t outStride)
{

  float xScale = (inWidth * 1.0f) / (outWidth * 1.0f);
  float yScale = (inHeight * 1.0f) / (outHeight * 1.0f);

  uint8_t border_constant_value = 128;

  int32_t srcOffsetX = 0;
  int32_t srcOffsetY = 0;
  int32_t dstOffsetX = 0;
  int32_t dstOffsetY = 0;

  int32_t x, y, ch;

  for( ch = 0; ch < 3; ch++) {
    uint8_t *pIn  = inImg + (ch * inStride * inHeight);
    uint8_t *pOut = outImg + (ch * outStride * outHeight);
    for( y = 0; y < outHeight; y++ ) {
      for( x = 0; x < outWidth; x++ ) {

        /* Apply scale factors to find input pixel for each output pixel */
        float src_x_f = ((float)(x+dstOffsetX) + 0.5f)*xScale - 0.5f;
        float src_y_f = ((float)(y+dstOffsetY) + 0.5f)*yScale - 0.5f;

        float xf = floorf(src_x_f);
        float yf = floorf(src_y_f);
        float dx = src_x_f - xf;
        float dy = src_y_f - yf;
        float a[4];

        int32_t src_x = (int32_t)(xf) - srcOffsetX;
        int32_t src_y = (int32_t)(yf) - srcOffsetY;

        uint8_t tl = 0;
        uint8_t tr = 0;
        uint8_t bl = 0;
        uint8_t br = 0;

        a[0] = (1.0f - dx) * (1.0f - dy);
        a[1] = (1.0f - dx) * (dy);
        a[2] = (dx)* (1.0f - dy);
        a[3] = (dx)* (dy);

        tl = (src_x < 0 || src_y < 0 || src_x > (inWidth-1) || src_y > (inHeight-1) ) ?
             border_constant_value :
             pIn[(src_y*inStride) + src_x];
        tr = ((src_x+1) < 0 || src_y < 0 || (src_x+1) > (inWidth-1) || src_y > (inHeight-1) ) ?
             border_constant_value :
             pIn[(src_y*inStride) + src_x + 1];
        bl = (src_x < 0 || (src_y+1) < 0 || src_x > (inWidth-1) || (src_y+1) > (inHeight-1) ) ?
             border_constant_value :
             pIn[((src_y+1)*inStride) + src_x];
        br = ((src_x+1) < 0 || (src_y+1) < 0 || (src_x+1) > (inWidth-1) || (src_y+1) > (inHeight-1) ) ?
             border_constant_value :
             pIn[((src_y+1)*inStride) + src_x + 1];

        pOut[(y*outStride) + x] = (uint8_t)(a[0]*tl + a[2]*tr + a[1]*bl + a[3]*br + 0.5f);
      }
    }
  }

  return;
}

static vx_status readInput(AppObj *obj, vx_context context, vx_user_data_object config, vx_tensor *input_tensors, char *input_file)
{
    vx_status status = VX_SUCCESS;

    tivx_utils_bmp_image_params_t *imgParams;
    void      *input_buffer = NULL;
    int32_t    capacity;
    uint32_t   id;

    vx_map_id map_id_input;

    vx_size    start[APP_MAX_TENSOR_DIMS];
    vx_size    input_strides[APP_MAX_TENSOR_DIMS];
    vx_size    input_sizes[APP_MAX_TENSOR_DIMS];

    sTIDL_IOBufDesc_t *ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < ioBufDesc->numInputBuf; id++)
    {
        vx_size data_type = getTensorDataType(ioBufDesc->inElementType[id]);

        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        capacity = input_sizes[0] * input_sizes[1] * input_sizes[2];

        start[0] = start[1] = start[2] = 0;

        input_strides[0] = sizeof(vx_int8);

        if((data_type == VX_TYPE_INT8) ||
           (data_type == VX_TYPE_UINT8))
        {
            input_strides[0] = sizeof(vx_int8);
        }
        else if((data_type == VX_TYPE_INT16) ||
                (data_type == VX_TYPE_UINT16))
        {
            input_strides[0] = sizeof(vx_int16);
        }

        input_strides[1] = input_sizes[0] * input_strides[0];
        input_strides[2] = input_sizes[1] * input_strides[1];

        APP_PRINTF(" input_sizes[0] = %d, dim = %d padL = %d padR = %d\n", (uint32_t)input_sizes[0], ioBufDesc->inWidth[id], ioBufDesc->inPadL[id], ioBufDesc->inPadR[id]);
        APP_PRINTF(" input_sizes[1] = %d, dim = %d padT = %d padB = %d\n", (uint32_t)input_sizes[1], ioBufDesc->inHeight[id], ioBufDesc->inPadT[id], ioBufDesc->inPadB[id]);
        APP_PRINTF(" input_sizes[2] = %d, dim = %d \n", (uint32_t)input_sizes[2], ioBufDesc->inNumChannels[id]);

        if(status == VX_SUCCESS)
        {
            status = tivxMapTensorPatch(input_tensors[id], 3, start, input_sizes, &map_id_input, input_strides, &input_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
        if (VX_SUCCESS == status)
        {
            vx_df_image df_image;
            void *data_ptr = NULL;
            vx_uint32 img_width;
            vx_uint32 img_height;
            vx_uint32 img_stride;
            vx_int32 start_offset;
            vx_uint8 *pData;
            vx_uint8 *pInPlanes;
            vx_uint8 *pOutPlanes;
            vx_int32 i, j;

            APP_PRINTF("app_tidl: Reading bmp file ...\n" );

            imgParams = &obj->imgParams;
            status = tivx_utils_bmp_file_read(
                        input_file,
                        vx_false_e,
                        imgParams);

            if(status != VX_SUCCESS)
            {
                printf("app_tidl: Reading bmp file ... Failed !!!\n" );
            }
            else
            {
                img_width  = imgParams->width;
                img_height = imgParams->height;
                img_stride = imgParams->stride_y;
                df_image   = imgParams->format;
                data_ptr   = imgParams->data;

                APP_PRINTF("app_tidl: Reading bmp file ... Done.\n" );

                APP_PRINTF("app_tidl: Image Pre processing for image of size %d x %d (pitch = %d bytes)...\n", img_width, img_height, img_stride);

                APP_PRINTF("app_tidl: Deinterleaving data ...\n");

                /* Save image params which are used in displayOutput function */
                obj->img_width   = img_width;
                obj->img_height  = img_height;
                obj->img_stride  = img_stride;
                obj->data_ptr    = data_ptr;
                obj->df_image    = df_image;

                pInPlanes = (vx_uint8 *)obj->pInPlanes;
                pOutPlanes = (vx_uint8 *)obj->pOutPlanes;

                /* Deinterleave RGB to planar */
                uint32_t chOffset = img_width * img_height;
                for(i = 0; i < img_height; i ++)
                {
                    pData = (vx_uint8 *)data_ptr + (i * img_stride);
                    for(j = 0; j < img_width; j ++)
                    {
                        pInPlanes[(0 * chOffset) + (i * img_width) + j] = *pData++;
                        pInPlanes[(1 * chOffset) + (i * img_width) + j] = *pData++;
                        pInPlanes[(2 * chOffset) + (i * img_width) + j] = *pData++;
                    }
                }

                APP_PRINTF("app_tidl: Resizing image ...\n");

                /* Resize image to network input resolution */
                resizeImage(pInPlanes, pOutPlanes, img_width, img_height, img_width, 256, 256, 256);

                APP_PRINTF("app_tidl: Rearranging data ...\n");

                if((data_type == VX_TYPE_INT8) ||
                   (data_type == VX_TYPE_UINT8))
                {
                    vx_uint8 *pR = NULL;
                    vx_uint8 *pG = NULL;
                    vx_uint8 *pB = NULL;

                    /* Reset the input buffer, this will take care of padding requirement for TIDL */
                    memset(input_buffer, 0, capacity);

                    if(ioBufDesc->inDataFormat[id] == 1) /* RGB */
                    {
                        start_offset = (0 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pR = (vx_uint8 *)input_buffer + start_offset;

                        start_offset = (1 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pG = (vx_uint8 *)input_buffer + start_offset;

                        start_offset = (2 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pB = (vx_uint8 *)input_buffer + start_offset;
                    }
                    else if(ioBufDesc->inDataFormat[id] == 0) /* BGR */
                    {
                        start_offset = (0 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pB = (vx_uint8 *)input_buffer + start_offset;

                        start_offset = (1 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pG = (vx_uint8 *)input_buffer + start_offset;

                        start_offset = (2 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pR = (vx_uint8 *)input_buffer + start_offset;
                    }

                    chOffset = 256*256;

                    if ( (NULL != pR) && (NULL != pG) && (NULL != pB) )
                    {
                        /* Write image in network required format */
                        for(i = 0; i < ioBufDesc->inHeight[id]; i++)
                        {
                            //Center crop 224x224 image from 256x256 resized input
                            uint32_t offset = ((16 + i) * 256) + 16;

                            for(j = 0; j < ioBufDesc->inWidth[id]; j++)
                            {
                                pR[j] = pOutPlanes[(0 * chOffset) + offset + j];
                                pG[j] = pOutPlanes[(1 * chOffset) + offset + j];
                                pB[j] = pOutPlanes[(2 * chOffset) + offset + j];
                            }

                            pR += input_sizes[0];
                            pG += input_sizes[0];
                            pB += input_sizes[0];
                        }
                    }
                }
                else if((data_type == VX_TYPE_INT16) ||
                        (data_type == VX_TYPE_UINT16))
                {
                    vx_uint16 *pR = NULL;
                    vx_uint16 *pG = NULL;
                    vx_uint16 *pB = NULL;

                    /* Reset the input buffer, this will take care of padding requirement for TIDL */
                    memset(input_buffer, 0, capacity);

                    if(ioBufDesc->inDataFormat[id] == 1) /* RGB */
                    {
                        start_offset = (0 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pR = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);

                        start_offset = (1 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pG = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);

                        start_offset = (2 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pB = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);
                    }
                    else if(ioBufDesc->inDataFormat[id] == 0) /* BGR */
                    {
                        start_offset = (0 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pB = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);

                        start_offset = (1 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pG = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);

                        start_offset = (2 * input_strides[2]) + (ioBufDesc->inPadT[id] * input_strides[1]) + (ioBufDesc->inPadL[id] * input_strides[0]);
                        pR = (vx_uint16 *)((vx_uint8 *)input_buffer + start_offset);
                    }

                    chOffset = 256*256;

                    if ( (NULL != pR) && (NULL != pG) && (NULL != pB) )
                    {
                        /* Write image in network required format */
                        for(i = 0; i < ioBufDesc->inHeight[id]; i++)
                        {
                            //Center crop 224x224 image from 256x256 resized input
                            uint32_t offset = ((16 + i) * 256) + 16;

                            for(j = 0; j < ioBufDesc->inWidth[id]; j++)
                            {
                                pR[j] = pOutPlanes[(0 * chOffset) + offset + j];
                                pG[j] = pOutPlanes[(1 * chOffset) + offset + j];
                                pB[j] = pOutPlanes[(2 * chOffset) + offset + j];
                            }

                            pR += input_sizes[0];
                            pG += input_sizes[0];
                            pB += input_sizes[0];
                        }
                    }
                }
                APP_PRINTF("app_tidl: Image Pre processing ... Done.\n");
            }

            tivxUnmapTensorPatch(input_tensors[id], map_id_input);
        }
    }

    return status;
}

static void displayOutput(AppObj *obj, vx_user_data_object config, vx_tensor *output_tensors, char *output_file)
{
    vx_status status = VX_SUCCESS;

    vx_size output_sizes[APP_MAX_TENSOR_DIMS];

    int32_t id, i, j;

    sTIDL_IOBufDesc_t *ioBufDesc;

    Draw2D_FontPrm sClassPrm;

    ioBufDesc = &obj->ioBufDesc;

    for(id = 0; id < ioBufDesc->numOutputBuf; id++)
    {
        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[id]);

        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = ioBufDesc->outNumChannels[id];
        if(status == VX_SUCCESS)
        {
            status = vxGetStatus((vx_reference)output_tensors[id]);
        }
        if (VX_SUCCESS == status)
        {
            void *output_buffer;

            vx_map_id map_id_output;

            vx_size output_strides[APP_MAX_TENSOR_DIMS];
            vx_size start[APP_MAX_TENSOR_DIMS];

            start[0] = start[1] = start[2] = start[3] = 0;

            output_strides[0] = sizeof(vx_int8);

            if((data_type == VX_TYPE_INT8) ||
               (data_type == VX_TYPE_UINT8))
            {
                output_strides[0] = sizeof(vx_int8);
            }
            else if((data_type == VX_TYPE_INT16) ||
                    (data_type == VX_TYPE_UINT16))
            {
                output_strides[0] = sizeof(vx_int16);
            }
            else if(data_type == VX_TYPE_FLOAT32)
            {
                output_strides[0] = sizeof(vx_float32);
            }

            output_strides[1] = output_sizes[0] * output_strides[0];
            output_strides[2] = output_sizes[1] * output_strides[1];

            status = tivxMapTensorPatch(output_tensors[id], 3, start, output_sizes, &map_id_output, output_strides, &output_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

            if(status == VX_SUCCESS)
            {
                vx_uint32 classid[5] = {0};
                vx_uint32 label_offset;

                label_offset = 0;
                if(ioBufDesc->outWidth[id] == 1001)
                {
                    label_offset = 0;
                }
                else if(ioBufDesc->outWidth[id] == 1000)
                {
                    label_offset = 1;
                }

                APP_PRINTF("app_tidl: Finding top-5 ... \n");

                if(data_type == VX_TYPE_FLOAT32)
                {
                    float *pOut;
                    float score[5];

                    pOut = (float *)output_buffer + (ioBufDesc->outPadT[id] * output_sizes[0]) + ioBufDesc->outPadL[id];

                    for(i = 0; i < 5; i++)
                    {
                        score[i] = FLT_MIN;
                        classid[i] = (vx_uint32)-1;

                        for(j = 0; j < ioBufDesc->outWidth[id]; j++)
                        {
                            if(pOut[j] > score[i])
                            {
                                score[i] = pOut[j];
                                classid[i] = j;
                            }
                        }
                        if(classid[i] < ioBufDesc->outWidth[id] )
                        {
                            pOut[classid[i]] = FLT_MIN;
                        }
                        else
                        {
                            classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                        }
                    }
                    #ifdef APP_DEBUG
                    APP_PRINTF("app_tidl: Image classification Top-5 results: \n");
                    for(i = 0; i < 5; i++)
                    {
                        APP_PRINTF("app_tidl:  %s, class-id: %d, score: %f \n", (char *)&imgnet_labels[classid[i] + label_offset], classid[i], score[i]);
                    }
                    #endif
                }
                else if(data_type == VX_TYPE_INT16)
                {
                    vx_int16 *pOut;
                    vx_int16 score[5];

                    pOut = (vx_int16 *)output_buffer + (ioBufDesc->outPadT[id] * output_sizes[0]) + ioBufDesc->outPadL[id];

                    for(i = 0; i < 5; i++)
                    {
                        score[i] = INT16_MIN;
                        classid[i] = (vx_uint32)-1;

                        for(j = 0; j < ioBufDesc->outWidth[id]; j++)
                        {
                            if(pOut[j] > score[i])
                            {
                                score[i] = pOut[j];
                                classid[i] = j;
                            }
                        }
                        if(classid[i] < ioBufDesc->outWidth[id] )
                        {
                            pOut[classid[i]] = INT16_MIN;
                        }
                        else
                        {
                            classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                        }
                    }
                    #ifdef APP_DEBUG
                    APP_PRINTF("app_tidl: Image classification Top-5 results: \n");
                    for(i = 0; i < 5; i++)
                    {
                        APP_PRINTF("app_tidl:  %s, class-id: %d, score: %d \n", (char *)&imgnet_labels[classid[i] + label_offset], classid[i], score[i]);
                    }
                    #endif
                }
                else if(data_type == VX_TYPE_INT8)
                {
                    vx_int8 *pOut;
                    vx_int8 score[5];

                    pOut = (vx_int8 *)output_buffer + (ioBufDesc->outPadT[id] * output_sizes[0]) + ioBufDesc->outPadL[id];

                    for(i = 0; i < 5; i++)
                    {
                        score[i] = INT8_MIN;
                        classid[i] = (vx_uint32)-1;

                        for(j = 0; j < ioBufDesc->outWidth[id]; j++)
                        {
                            if(pOut[j] > score[i])
                            {
                                score[i] = pOut[j];
                                classid[i] = j;
                            }
                        }
                        if(classid[i] < ioBufDesc->outWidth[id] )
                        {
                            pOut[classid[i]] = INT8_MIN;
                        }
                        else
                        {
                            classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                        }
                    }
                    #ifdef APP_DEBUG
                    APP_PRINTF("app_tidl: Image classification Top-5 results: \n");
                    for(i = 0; i < 5; i++)
                    {
                        APP_PRINTF("app_tidl:  %s, class-id: %d, score: %d \n", (char *)&imgnet_labels[classid[i] + label_offset], classid[i], score[i]);
                    }
                    #endif
                }

                APP_PRINTF("app_tidl: Finding top-5 ... Done \n");

                Draw2D_clearRegion(obj->pHndl, (DISPLAY_WIDTH/2) + 40, 200, 600, 300);
                sClassPrm.fontIdx = 1;
                for(i = 0; i < 5; i++)
                {
                    Draw2D_drawString(obj->pHndl, (DISPLAY_WIDTH/2) + 40, 200 + (i * 40), (char *)&imgnet_labels[classid[i] + label_offset], &sClassPrm);
                }
            }
            tivxUnmapTensorPatch(output_tensors[id], map_id_output);
        }
    }

    {
      /* Convert RGB565 to RGB888 before writing output */
      for(i = 0, j = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++)
      {
        uint16_t RGB_565_val = obj->pDisplayBuf565[i];

        obj->pDisplayBuf888[j + 0] = (RGB_565_val & 0x1F) << 3;
        obj->pDisplayBuf888[j + 1] = ((RGB_565_val >> 5) & 0x3F) << 2;
        obj->pDisplayBuf888[j + 2] = ((RGB_565_val >> 11) & 0x1F) << 3;

        j  += 4;
      }

      if((obj->img_width <= (DISPLAY_WIDTH/2)) && (obj->img_height <= (DISPLAY_HEIGHT - 200)))
      {
         uint32_t startX = ((DISPLAY_WIDTH/2) / 2) - (obj->img_width / 2);
         uint32_t startY = ((DISPLAY_HEIGHT/2)  / 2) - (obj->img_height / 2);
         uint32_t imgOffset = 200;

         for(i = 0; i < obj->img_height; i++)
         {
            uint8_t *pOut = &obj->pDisplayBuf888[((imgOffset + startY + i) * DISPLAY_WIDTH * 4) + (startX * 4)];
            uint8_t *pIn  = obj->data_ptr + (i * obj->img_stride);

            for(j = 0; j < obj->img_width; j++)
            {
              *pOut++ = *pIn++;
              *pOut++ = *pIn++;
              *pOut++ = *pIn++;
              pOut++;
            }
         }
      }
    }

    if (obj->display_option == 0)
    {
        tivx_utils_bmp_write(output_file, obj->pDisplayBuf888, DISPLAY_WIDTH, DISPLAY_HEIGHT, (DISPLAY_WIDTH * 4), VX_DF_IMAGE_RGBX);
    }

    /* Release the bmp buffer created in readInput() */
    tivx_utils_bmp_read_release(&obj->imgParams);

    APP_PRINTF("app_tidl: Showing output ... Done.\n");
}

static vx_size getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }

    return openvx_type;
}
#ifdef APP_WRITE_PRE_PROC_OUTPUT
static vx_status writePreProcOutput(char* file_name, vx_tensor output)
{
    vx_status status = VX_SUCCESS;
    vx_size num_dims;
    vx_enum data_type;
    void *data_ptr;
    vx_map_id map_id;

    vx_size    start[APP_MAX_TENSOR_DIMS];
    vx_size    tensor_strides[APP_MAX_TENSOR_DIMS];
    vx_size    tensor_sizes[APP_MAX_TENSOR_DIMS];

    vxQueryTensor(output, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));

    if(num_dims != 3)
    {
      printf("Number of dims are != 3! exiting.. \n");
      return VX_FAILURE;
    }

    vxQueryTensor(output, (vx_enum)VX_TENSOR_DIMS, tensor_sizes, 3 * sizeof(vx_size));
    vxQueryTensor(output, (vx_enum)VX_TENSOR_DATA_TYPE, &data_type, sizeof(data_type));

    start[0] = start[1] = start[2] = 0;

    tensor_strides[0] = sizeof(vx_int8);

    if((data_type == VX_TYPE_INT8) ||
       (data_type == VX_TYPE_UINT8))
    {
        tensor_strides[0] = sizeof(vx_int8);
    }
    else if((data_type == VX_TYPE_INT16) ||
            (data_type == VX_TYPE_UINT16))
    {
        tensor_strides[0] = sizeof(vx_int16);
    }
    else if((data_type == VX_TYPE_FLOAT32))
    {
        tensor_strides[0] = sizeof(vx_float32);
    }

    tensor_strides[1] = tensor_strides[0] * tensor_strides[0];
    tensor_strides[2] = tensor_strides[1] * tensor_strides[1];

    status = tivxMapTensorPatch(output, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    if(VX_SUCCESS == status)
    {
      vx_char new_name[APP_MAX_FILE_PATH];
      snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d.rgb", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1]);

      FILE *fp = fopen(new_name, "wb");
      if(NULL == fp)
      {
        printf("Unable to open file %s for writing!\n", new_name);
        return VX_FAILURE;
      }

      fwrite(data_ptr, 1, num_dims * tensor_strides[2], fp);
      fclose(fp);

      tivxUnmapTensorPatch(output, map_id);
    }

  return(status);
}
#endif
