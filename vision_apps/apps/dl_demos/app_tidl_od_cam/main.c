/*
 *
 * Copyright (c) 2020-2026 Texas Instruments Incorporated
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
#include "app_sensor_module.h"
#include "app_capture_module.h"
#include "app_viss_module.h"
#include "app_aewb_module.h"
#include "app_ldc_module.h"
#include "app_scaler_module.h"
#include "app_pre_proc_module.h"
#include "app_tidl_module.h"
#include "app_draw_detections_module.h"
#include "app_img_mosaic_module.h"
#include "app_display_module.h"
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
#include "app_post_proc_module.h"
#endif
#if defined(SOC_AM62A) && defined(QNX)
#include <screen/screen.h>
#endif

#if defined(SOC_AM62A) && defined(LINUX)
/*AM62A: Linux to use DRM/KMS for displaying frames on A53 with zero-copy*/
#include <utils/drm_wrapper/include/drm_wrapper.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <unistd.h>
#define  DRM_FORMAT_NV12                0x3231564E
#endif

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
#include <time.h>
#include <semaphore.h>
#endif

#define APP_BUFFER_Q_DEPTH   (4)
#define APP_PIPELINE_DEPTH   (7)

#if defined(SOC_AM62A) && defined(QNX)
/*AM62A: QNX to use screen package for displaying frames on A53*/
screen_context_t screen_ctx = NULL;
screen_window_t screen_win = NULL;
#endif

typedef struct {

    SensorObj         sensorObj;
    CaptureObj        captureObj;
    VISSObj           vissObj;
    AEWBObj           aewbObj;
    LDCObj            ldcObj;
    ScalerObj         scalerObj;
    PreProcObj        preProcObj;
    TIDLObj           tidlObj;
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    DrawDetectionsObj drawDetectionsObj;
#else
    PostProcObj       postProcObj;
#endif
    ImgMosaicObj      imgMosaicObj;
    DisplayObj        displayObj;

    vx_char input_file_path[APP_MAX_FILE_PATH];
    vx_char output_file_path[APP_MAX_FILE_PATH];

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;

    vx_uint32 is_interactive;
    vx_uint32 cpu_core_id;

    vx_uint32 num_frames_to_run;

    vx_uint32 num_frames_to_write;
    vx_uint32 num_frames_to_skip;

    tivx_task task;
    vx_uint32 stop_task;
    vx_uint32 stop_task_done;

    app_perf_point_t total_perf;
    app_perf_point_t fileio_perf;
    app_perf_point_t draw_perf;

    int32_t pipeline;

    int32_t enqueueCnt;
    int32_t dequeueCnt;

    int32_t write_file;

#if defined(SOC_AM62A) && defined(QNX)
    /* QNX-specific display task variables */
    tivx_task screen_task;
    uint32_t stop_screen_task;
    uint32_t stop_screen_task_done;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    /* Linux-specific DRM variables */
    void *drm_handle;
    tivx_task drm_task;
    uint32_t stop_drm_task;
    uint32_t stop_drm_task_done;
#endif

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Common synchronization variables for AM62A Linux and QNX to eliminate screen tearing.
     * Both platforms use graph parameters and semaphores for synchronized buffer access. */
    sem_t disp_frame_ready_sem;  /* Graph task → Display task: frame ready */
    sem_t disp_frame_done_sem;   /* Display task → Graph task: copy done   */
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    /* Linux: Pointer to TIOVX-completed frame the DRM task should blit from */
    vx_image drm_frame;
#endif

#if defined(SOC_AM62A) && defined(QNX)
    /* QNX: Pointer to TIOVX-completed frame the screen task should copy from */
    vx_image screen_frame;
#endif

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
static void app_update_param_set(AppObj *obj);
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);
static void app_pipeline_params_defaults(AppObj *obj);
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);
#endif
#endif

static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id);

#if defined(SOC_AM62A) && defined(LINUX)
/**
 * \brief Copy a smaller NV12 image into the center of a larger NV12 canvas
 *
 * The canvas is pre-filled with black (Y=0, UV=128). This function copies
 * the source image into the center, creating a letterboxed output.
 */
static void app_drm_letterbox_nv12(vx_image canvas, vx_image src,
                                    vx_uint32 canvas_w, vx_uint32 canvas_h,
                                    vx_uint32 src_w, vx_uint32 src_h)
{
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_map_id src_map, dst_map;
    void *src_ptr = NULL, *dst_ptr = NULL;
    vx_uint32 offset_x, offset_y;
    vx_uint32 row;

    offset_x = (canvas_w - src_w) / 2;
    offset_y = (canvas_h - src_h) / 2;

    /* Ensure offsets are even (NV12 chroma subsampling requirement) */
    offset_x &= ~1u;
    offset_y &= ~1u;

    /* Copy Y plane */
    rect.start_x = 0; rect.start_y = 0;
    rect.end_x = src_w; rect.end_y = src_h;
    vxMapImagePatch(src, &rect, 0, &src_map, &src_addr, &src_ptr,
                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    rect.end_x = canvas_w; rect.end_y = canvas_h;
    vxMapImagePatch(canvas, &rect, 0, &dst_map, &dst_addr, &dst_ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    if (src_ptr != NULL && dst_ptr != NULL)
    {
        for (row = 0; row < src_h; row++)
        {
            uint8_t *s = (uint8_t *)src_ptr + row * src_addr.stride_y;
            uint8_t *d = (uint8_t *)dst_ptr + (offset_y + row) * dst_addr.stride_y + offset_x;
            memcpy(d, s, src_w);
        }
    }

    vxUnmapImagePatch(src, src_map);
    vxUnmapImagePatch(canvas, dst_map);

    /* Copy UV plane */
    rect.start_x = 0; rect.start_y = 0;
    rect.end_x = src_w; rect.end_y = src_h;
    vxMapImagePatch(src, &rect, 1, &src_map, &src_addr, &src_ptr,
                    VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    rect.end_x = canvas_w; rect.end_y = canvas_h;
    vxMapImagePatch(canvas, &rect, 1, &dst_map, &dst_addr, &dst_ptr,
                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

    if (src_ptr != NULL && dst_ptr != NULL)
    {
        vx_uint32 uv_src_h = src_h / 2;
        vx_uint32 uv_offset_y = offset_y / 2;

        for (row = 0; row < uv_src_h; row++)
        {
            uint8_t *s = (uint8_t *)src_ptr + row * src_addr.stride_y;
            uint8_t *d = (uint8_t *)dst_ptr + (uv_offset_y + row) * dst_addr.stride_y + offset_x;
            memcpy(d, s, src_w);
        }
    }

    vxUnmapImagePatch(src, src_map);
    vxUnmapImagePatch(canvas, dst_map);
}

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

    /* Y plane: fill with 0 (black) */
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
 * \brief Linux DRM display task function
 *
 * Implements synchronized display using DRM/KMS similar to app_single_cam.
 * Uses double-buffered canvases and semaphore-based synchronization with
 * the graph task to eliminate screen tearing.
 */
int32_t app_run_drm_display(AppObj *obj)
{
    int32_t status = 0;
    app_drm_wrapper_cfg_t drm_cfg;
    app_drm_wrapper_handle_t *drm_handle = NULL;
    vx_uint32 width, height;
    vx_df_image df;
    uint32_t drm_pix_format;
    uint32_t buf_idx = 0;
    vx_image canvas[2] = {NULL, NULL};
    uint32_t canvas_w, canvas_h;
    int needs_letterbox = 0;
    uint32_t disp_w = 1920;
    uint32_t disp_h = 1080;
    uint32_t i;
    vx_image display_image;

    /* Query mosaic output image properties */
    display_image = obj->imgMosaicObj.output_image[0];
    vxQueryImage(display_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(display_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(display_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    printf("[DRM] DEBUG: display_image dimensions = %u x %u, format = 0x%x\n", width, height, df);

    /* Map TIOVX format to DRM format */
    if (VX_DF_IMAGE_NV12 == df)
    {
        drm_pix_format = DRM_FORMAT_NV12;
    }
    else
    {
        printf("[DRM] Unsupported pixel format 0x%x, defaulting to NV12\n", df);
        drm_pix_format = DRM_FORMAT_NV12;
    }

    /* Query actual display resolution from DRM CRTC */
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

    needs_letterbox = (width < disp_w || height < disp_h);
    canvas_w = needs_letterbox ? disp_w : width;
    canvas_h = needs_letterbox ? disp_h : height;

    if (needs_letterbox)
    {
        printf("[DRM] Letterbox mode: %ux%u centered in %ux%u\n",
               width, height, canvas_w, canvas_h);
    }

    /* Allocate two canvases for double buffering */
    for (i = 0; i < 2u; i++)
    {
        canvas[i] = vxCreateImage(obj->context, canvas_w, canvas_h,
                                  VX_DF_IMAGE_NV12);
        if (canvas[i] == NULL)
        {
            printf("[DRM] Failed to create canvas[%u]\n", i);
            goto cleanup_canvas;
        }
        app_drm_clear_nv12(canvas[i], canvas_w, canvas_h);
        printf("[DRM] DEBUG: letterboxing canvas[%u] (%u x %u) with image (%u x %u)\n", i, canvas_w, canvas_h, width, height);
        app_drm_letterbox_nv12(canvas[i], display_image,
                               canvas_w, canvas_h, width, height);
    }

    /* Initialize DRM wrapper */
    drm_cfg.width      = canvas_w;
    drm_cfg.height     = canvas_h;
    drm_cfg.pix_format = drm_pix_format;
    drm_cfg.bufq_depth = 2;

    drm_handle = appDrmWrapperCreate(&drm_cfg);
    if (drm_handle == NULL)
    {
        printf("[DRM] Failed to create DRM wrapper\n");
        goto cleanup_canvas;
    }

    obj->drm_handle = (void *)drm_handle;

    /* Register both canvases */
    for (i = 0; i < 2u; i++)
    {
        status = appDrmWrapperRegisterBuffer(drm_handle, canvas[i], i);
        if (status < 0)
        {
            printf("[DRM] Failed to register canvas[%u]\n", i);
            appDrmWrapperDelete(drm_handle);
            obj->drm_handle = NULL;
            goto cleanup_canvas;
        }
    }

    printf("[DRM] DRM display initialized: %ux%u, double-buffered\n",
           canvas_w, canvas_h);

    /* Main display loop - synchronized with TIOVX pipeline */
    buf_idx = 0;
    while (1)
    {
        vx_image src_frame;
        struct timespec ts;

        /* Wait for graph task to provide completed buffer */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        if (sem_timedwait(&obj->disp_frame_ready_sem, &ts) != 0)
        {
            if (obj->stop_drm_task == 1) { break; }
            continue;
        }

        src_frame = obj->drm_frame;

        /* Ensure back canvas is no longer being scanned out */
            appDrmWrapperWaitFlipDone(drm_handle);

        /* Blit completed frame into the safe back buffer */
        app_drm_letterbox_nv12(canvas[buf_idx], src_frame,
                               canvas_w, canvas_h, width, height);

        /* Signal graph task: blit done, buffer may be re-enqueued */
        sem_post(&obj->disp_frame_done_sem);

        status = appDrmWrapperRender(drm_handle, buf_idx);
        if (status < 0)
        {
           printf("[DRM] Render failed\n");
           break;
        }

        buf_idx = (buf_idx + 1u) % 2u;

        if (obj->stop_drm_task == 1)
        {
            break;
        }
    }

    /* Unblock the graph task if it is blocked */
    sem_post(&obj->disp_frame_done_sem);

cleanup_canvas:
    for (i = 0; i < 2u; i++)
    {
        if (canvas[i] != NULL)
        {
            vxReleaseImage(&canvas[i]);
            canvas[i] = NULL;
        }
    }

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

#if defined(SOC_AM62A) && defined(QNX)
int32_t app_run_screen(AppObj *obj)
{
    int32_t err = 0;
    int usage = SCREEN_USAGE_READ | SCREEN_USAGE_WRITE;
    int screenFormat = SCREEN_FORMAT_NV12;
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
        printf("[QNX Screen] Failed to create screen context\n");
        return err;
    }

    /* create a window */
    err = screen_create_window(&screen_win, screen_ctx);
    if(err != 0) {
        printf("[QNX Screen] Failed to create screen window\n");
        return err;
    }

    err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    if(err != 0) {
        printf("[QNX Screen] Failed to set usage property\n");
        return err;
    }
    err = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &screenFormat);
    if(err != 0) {
        printf("[QNX Screen] Failed to set format property\n");
        return err;
    }

    /* create screen buffers */
    int nbuffers = 2;
    err = screen_create_window_buffers(screen_win, nbuffers);
    if(err != 0) {
        printf("[QNX Screen] Failed to create window buffer\n");
        return err;
    }

    printf("[QNX Screen] Display initialized\n");

    while(1) {
        vx_image source_image;
        struct timespec ts;

        /* SYNCHRONIZED ACCESS: Wait for graph task to provide completed buffer */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;  /* 1-second timeout */

        /* Wait for signal from graph task: "frame ready" */
        if (sem_timedwait(&obj->disp_frame_ready_sem, &ts) != 0)
        {
            /* Timeout - check if we should exit */
            if(obj->stop_screen_task == 1) {
                break;
            }
            continue;  /* Try again */
        }

        /* Graph task has provided a completed buffer */
        source_image = obj->screen_frame;

        /* Get screen buffer properties */
        int buffer_size[2];
        err = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size);
        if(err != 0) {
            printf("[QNX Screen] Failed to get window buffer size\n");
            break;
        }

        screen_buffer_t screen_buf[2];
        err = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_buf);
        if(err != 0) {
            printf("[QNX Screen] Failed to get window buffer\n");
            break;
        }

        /* obtain pointers to the buffers */
        void *ptr1 = NULL;
        err = screen_get_buffer_property_pv(screen_buf[0], SCREEN_PROPERTY_POINTER, (void **)&ptr1);
        if(err != 0) {
            printf("[QNX Screen] Failed to get buffer pointer\n");
            break;
        }

        int buf_stride1 = 0;
        err = screen_get_buffer_property_iv(screen_buf[0], SCREEN_PROPERTY_STRIDE, &buf_stride1);
        if(err != 0) {
           printf("[QNX Screen] Failed to get buffer stride\n");
           break;
        }

        /* Query source image properties */
        vxQueryImage(source_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(source_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(source_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

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
        rect.end_x = width;
        rect.end_y = height;

        if (0 == strcmp(obj->sensorObj.sensor_name, SENSOR_OV2312_UB953_LI))
        {
            /*OV2312 raw frame has 1600 width, moving the frame to middle of 1920 width display*/
            ptr1 += (1920-1600)/2;
            rect.end_x = 1600;
            memset(ptr1, 0x00000000, buffer_size[0]*buffer_size[1]);
        }

        /* Copy Y plane from TIOVX buffer to screen buffer */
        vxMapImagePatch(source_image,
            &rect,
            0,  /* Plane 0 = Y */
            &map_id1,
            &image_addr,
            &data_ptr1,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(!data_ptr1)
        {
            printf("[QNX Screen] data_ptr1 is NULL\n");
            break;
        }

        imgaddr_width  = image_addr.dim_x;
        imgaddr_height = image_addr.dim_y;
        imgaddr_stride = image_addr.stride_y;

        for(i=0; i<height; i++)
        {
            memcpy(ptr1, data_ptr1, imgaddr_width*num_bytes_per_4pixels/4);
            data_ptr1 += imgaddr_stride;
            ptr1 += buf_stride1;
        }
        vxUnmapImagePatch(source_image, map_id1);

        /* Copy UV plane if NV12 format */
        if(VX_DF_IMAGE_NV12 == df || TIVX_DF_IMAGE_NV12_P12 == df)
        {
            vxMapImagePatch(source_image,
                &rect,
                1,  /* Plane 1 = UV */
                &map_id2,
                &image_addr,
                &data_ptr2,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST,
                VX_NOGAP_X
                );

            if(!data_ptr2)
            {
                printf("[QNX Screen] data_ptr2 is NULL\n");
                break;
            }

            imgaddr_width  = image_addr.dim_x;
            imgaddr_height = image_addr.dim_y;
            imgaddr_stride = image_addr.stride_y;

            for(i=0; i<imgaddr_height/2; i++)
            {
                memcpy(ptr1, data_ptr2, imgaddr_width*num_bytes_per_4pixels/4);
                data_ptr2 += imgaddr_stride;
                ptr1 += buf_stride1;
            }
            vxUnmapImagePatch(source_image, map_id2);
        }

        /* Signal graph task: copy done, buffer can be re-enqueued */
        sem_post(&obj->disp_frame_done_sem);

        /* Post frame to display */
        err = screen_post_window(screen_win, screen_buf[0], 0, NULL, 0);
        if(err != 0) {
            printf("[QNX Screen] Failed to post window\n");
            break;
        }

        if(obj->stop_screen_task == 1) {
            break;
        }
    }

    /* Unblock the graph task if it is blocked on disp_frame_done_sem during exit */
    sem_post(&obj->disp_frame_done_sem);

    return err;
}
#endif


static void app_show_usage(vx_int32 argc, vx_char* argv[])
{
    printf("\n");
    printf(" TIDL Demo - Camera based Object Detection (c) Texas Instruments Inc. 2020\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

static const char menu[] = {
    "\n"
    "\n =========================================="
    "\n TIDL Demo - Camera based Object Detection"
    "\n =========================================="
#ifdef APP_WRITE_INTERMEDIATE_OUTPUTS
    "\n"
    "\n s: Save intermediate outputs"
#endif
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
                    fp = appPerfStatsExportOpenFile(".", "dl_demos_app_tidl_od_cam");
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
#ifdef APP_WRITE_INTERMEDIATE_OUTPUTS
                case 's':
                    obj->write_file = 1;
                    break;
#endif
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
#if defined(SOC_AM62A) && defined(LINUX)
        app_run_drm_task_delete(obj);
#endif
    }
    return status;
}

static void app_set_cfg_default(AppObj *obj)
{
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
        exit(-1);
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
                obj->sensorObj.is_interactive = obj->is_interactive;
            }
            else
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
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
            else
            if(strcmp(token, "viz_th")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->drawDetectionsObj.params.viz_th = atof(token);
                }
            }
            else
            if(strcmp(token, "num_classes")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    obj->drawDetectionsObj.params.num_classes = atoi(token);
                }
            }
#endif
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
            if(strcmp(token, "cpu_core_id")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->cpu_core_id = atoi(token);
                }
            }
#ifdef APP_WRITE_INTERMEDIATE_OUTPUTS
            else
            if(strcmp(token, "num_frames_to_run")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->num_frames_to_run = atoi(token);
                }
            }
            else
            if(strcmp(token, "en_out_capture_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
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
                    token[strlen(token)-1]=0;
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
                    token[strlen(token)-1]=0;
                    obj->ldcObj.en_out_ldc_write = atoi(token);
                    if(obj->ldcObj.en_out_ldc_write > 1)
                    obj->ldcObj.en_out_ldc_write = 1;
                }
            }
            else
            if(strcmp(token, "en_out_scaler_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->scalerObj.en_out_scaler_write = atoi(token);
                    if(obj->scalerObj.en_out_scaler_write > 1)
                        obj->scalerObj.en_out_scaler_write = 1;
                }
            }
            else
            if(strcmp(token, "en_out_pre_proc_write")==0)
            {
                token = strtok(NULL, s);
                if(token != NULL)
                {
                    token[strlen(token)-1]=0;
                    obj->preProcObj.en_out_pre_proc_write = atoi(token);
                    if(obj->preProcObj.en_out_pre_proc_write > 1)
                        obj->preProcObj.en_out_pre_proc_write = 1;
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
                    strcpy(obj->scalerObj.output_file_path, token);
                    strcpy(obj->preProcObj.output_file_path, token);
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
#endif
        }
    }

    fclose(fp);
}

static void app_parse_cmd_line_args(AppObj *obj, vx_int32 argc, vx_char *argv[])
{
    vx_int32 i;

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
            break;
        }
        else
        if(strcmp(argv[i], "--help")==0)
        {
            app_show_usage(argc, argv);
            exit(0);
        }
    }

    #ifdef x86_64
    obj->displayObj.display_option = 0;
    obj->is_interactive = 0;
    #endif

    return;
}

vx_status app_tidl_od_cam_main(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;

    AppObj *obj = &gAppObj;

    /*Optional parameter setting*/
    app_default_param_set(obj);
    APP_PRINTF("Default param set! \n");

    /*Config parameter reading*/
    app_parse_cmd_line_args(obj, argc, argv);
    APP_PRINTF("Parsed user params! \n");

    /* Querry sensor parameters */
    app_querry_sensor(&obj->sensorObj);
    APP_PRINTF("Sensor params queried! \n");

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

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    app_grpx_init_prms_t grpx_prms;
#endif
    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);
    APP_PRINTF("Creating context done!\n");
    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxImagingLoadKernels(obj->context);
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        tivxEdgeaiImgProcLoadKernels(obj->context);
#else
        tivxImgProcLoadKernels(obj->context);
#endif
        tivxTIDLLoadKernels(obj->context);
        tivxFileIOLoadKernels(obj->context);
    }
    APP_PRINTF("Kernel loading done!\n");

    /* Initialize modules */

    app_init_sensor(&obj->sensorObj, "sensor_obj");
    APP_PRINTF("Sensor init done!\n");

    app_init_capture(obj->context, &obj->captureObj, &obj->sensorObj, "capture_obj", APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Capture init done!\n");

    app_init_viss(obj->context, &obj->vissObj, &obj->sensorObj, "viss_obj", obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("VISS init done!\n");

    app_init_aewb(obj->context, &obj->aewbObj, &obj->sensorObj, "aewb_obj", 0, obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("AEWB init done!\n");

    app_init_ldc(obj->context, &obj->ldcObj, &obj->sensorObj, "ldc_obj", obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("LDC init done!\n");

#if (defined(SOC_AM62A) && (defined(QNX) || defined(LINUX)))
    /* This acts as an image pad for the post_proc module*/
    obj->scalerObj.output[0].width  = obj->ldcObj.table_width;
    obj->scalerObj.output[0].height = obj->ldcObj.table_height;

    /*For Object Detection, TIDL expects input image of resolution 416x416*/
    obj->scalerObj.output[1].width  = 416;
    obj->scalerObj.output[1].height = 416;
#endif

    printf("Scaler output1 width   = %d\n", obj->scalerObj.output[0].width);
    printf("Scaler output1 height  = %d\n", obj->scalerObj.output[0].height);
    printf("Scaler output2 width   = %d\n", obj->scalerObj.output[1].width);
    printf("Scaler output2 height  = %d\n", obj->scalerObj.output[1].height);

    app_init_scaler(obj->context, &obj->scalerObj, "scaler_obj", obj->sensorObj.num_cameras_enabled, 2);
    APP_PRINTF("Scaler init done!\n");

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
    /* Initialize TIDL first to get tensor I/O information from network */
    app_init_tidl(obj->context, &obj->tidlObj, "tidl_obj", obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("TIDL Init Done! \n");

    /* Update pre-proc parameters with TIDL config before calling init */
    app_update_pre_proc(obj->context, &obj->preProcObj, obj->tidlObj.config, obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("Pre Proc Update Done! \n");

    app_init_pre_proc(obj->context, &obj->preProcObj, "pre_proc_obj");
    APP_PRINTF("Pre Proc Init Done! \n");

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    /* Update ioBufDesc in draw detections object */
    app_update_draw_detections(&obj->drawDetectionsObj, obj->tidlObj.config);
    APP_PRINTF("Draw detections Update Done! \n");

    app_init_draw_detections(obj->context, &obj->drawDetectionsObj, "draw_detections_obj", obj->sensorObj.num_cameras_enabled);
    APP_PRINTF("Draw Detections Init Done! \n");
#else
    /* Update post-proc parameters with TIDL config before calling init */
    app_update_post_proc(obj->context, &obj->postProcObj, obj->tidlObj.config);
    APP_PRINTF("Post Proc Update Done! \n");

    status = app_init_post_proc(obj->context, &obj->postProcObj, "post_proc_obj", obj->sensorObj.num_cameras_enabled, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Post Proc Init Done! \n");
#endif
    app_init_img_mosaic(obj->context, &obj->imgMosaicObj, "img_mosaic_obj", APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Img Mosaic init done!\n");

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    app_init_display(obj->context, &obj->displayObj, "display_obj");
    APP_PRINTF("Display init done!\n");

    #ifndef x86_64
    if(obj->displayObj.display_option == 1)
    {
        appGrpxInitParamsInit(&grpx_prms, obj->context);
        grpx_prms.draw_callback = app_draw_graphics;
        appGrpxInit(&grpx_prms);
    }
    #endif
#endif

    appPerfPointSetName(&obj->total_perf , "TOTAL");
    appPerfPointSetName(&obj->fileio_perf, "FILEIO");

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Initialize synchronization semaphores for both Linux and QNX */
    sem_init(&obj->disp_frame_ready_sem, 0, 0);
    sem_init(&obj->disp_frame_done_sem,  0, 0);
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    obj->drm_handle = NULL;
    obj->drm_frame = NULL;
    obj->stop_drm_task = 0;
    obj->stop_drm_task_done = 0;
#endif

#if defined(SOC_AM62A) && defined(QNX)
    obj->screen_frame = NULL;
#endif

    return status;
}

static void app_deinit(AppObj *obj)
{
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Cleanup synchronization semaphores for both Linux and QNX */
    sem_destroy(&obj->disp_frame_ready_sem);
    sem_destroy(&obj->disp_frame_done_sem);
#endif

    app_deinit_sensor(&obj->sensorObj);
    APP_PRINTF("Sensor deinit done!\n");

    app_deinit_capture(&obj->captureObj, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Capture deinit done!\n");

    app_deinit_viss(&obj->vissObj);
    APP_PRINTF("VISS deinit done!\n");

    app_deinit_aewb(&obj->aewbObj);
    APP_PRINTF("AEWB deinit done!\n");

    app_deinit_ldc(&obj->ldcObj);
    APP_PRINTF("LDC deinit done!\n");

    app_deinit_scaler(&obj->scalerObj);
    APP_PRINTF("Scaler deinit done!\n");

    app_deinit_pre_proc(&obj->preProcObj);
    APP_PRINTF("Pre proc deinit done!\n");

    app_deinit_tidl(&obj->tidlObj);
    APP_PRINTF("TIDL deinit done!\n");

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    app_deinit_post_proc(obj->context, &obj->postProcObj, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Post proc deinit done!\n");
#else
    app_deinit_draw_detections(&obj->drawDetectionsObj);
    APP_PRINTF("Draw detections deinit done!\n");
#endif

    app_deinit_img_mosaic(&obj->imgMosaicObj, APP_BUFFER_Q_DEPTH);
    APP_PRINTF("Img Mosaic deinit done!\n");
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    app_deinit_display(&obj->displayObj);
    APP_PRINTF("Display deinit done!\n");

    #ifndef x86_64
    if(obj->displayObj.display_option == 1)
    {
        appGrpxDeInit();
    }
    #endif
#endif
    tivxTIDLUnLoadKernels(obj->context);
    tivxHwaUnLoadKernels(obj->context);
    tivxVideoIOUnLoadKernels(obj->context);
    tivxImagingUnLoadKernels(obj->context);
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    tivxEdgeaiImgProcUnLoadKernels(obj->context);
#else
    tivxImgProcUnLoadKernels(obj->context);
#endif
    tivxFileIOUnLoadKernels(obj->context);
    APP_PRINTF("Kernels unload done!\n");

    vxReleaseContext(&obj->context);
    APP_PRINTF("Release context done!\n");
}

static void app_delete_graph(AppObj *obj)
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

    app_delete_pre_proc(&obj->preProcObj);
    APP_PRINTF("Pre Proc delete done!\n");

    app_delete_tidl(&obj->tidlObj);
    APP_PRINTF("TIDL delete done!\n");

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    app_delete_post_proc(&obj->postProcObj);
    APP_PRINTF("Post Proc delete done!\n");
#else
    app_delete_draw_detections(&obj->drawDetectionsObj);
    APP_PRINTF("Post Proc delete done!\n");
#endif

    app_delete_img_mosaic(&obj->imgMosaicObj);
    APP_PRINTF("Img Mosaic delete done!\n");
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    app_delete_display(&obj->displayObj);
    APP_PRINTF("Display delete done!\n");
#endif
    vxReleaseGraph(&obj->graph);
    APP_PRINTF("Graph delete done!\n");
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);
    vxSetReferenceName((vx_reference)obj->graph, "app_tidl_od_cam_graph");
    APP_PRINTF("Graph create done!\n");

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_capture(obj->graph, &obj->captureObj);
        APP_PRINTF("Capture graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_viss(obj->graph, &obj->vissObj, obj->captureObj.raw_image_arr[0], TIVX_TARGET_VPAC_VISS1);
        APP_PRINTF("VISS graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_aewb(obj->graph, &obj->aewbObj, obj->vissObj.h3a_stats_arr);
        APP_PRINTF("AEWB graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_ldc(obj->graph, &obj->ldcObj, obj->vissObj.output_arr, TIVX_TARGET_VPAC_LDC1);
        APP_PRINTF("LDC graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
        app_create_graph_scaler(obj->context, obj->graph, &obj->scalerObj, obj->ldcObj.output_arr);
        APP_PRINTF("Scaler graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
        app_create_graph_pre_proc(obj->graph, &obj->preProcObj, obj->scalerObj.output[0].arr);
#else
        app_create_graph_pre_proc(obj->graph, &obj->preProcObj, obj->scalerObj.output[1].arr);
#endif
        APP_PRINTF("Pre proc graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
        app_create_graph_tidl(obj->context, obj->graph, &obj->tidlObj, obj->preProcObj.output_tensor_arr);
        APP_PRINTF("TIDL graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /*obj->scalerObj.output[0].arr gives the required image pad and obj->tidlObj.output_tensor_arr[0] acts as the required tensor
      pad for the post_proc module*/
        status = app_create_graph_post_proc(obj->graph, &obj->postProcObj, obj->tidlObj.out_args_arr, obj->tidlObj.output_tensor_arr[0], obj->scalerObj.output[0].arr);
        APP_PRINTF("Post proc graph done!\n");
#else
        app_create_graph_draw_detections(obj->graph, &obj->drawDetectionsObj, obj->tidlObj.output_tensor_arr[0], obj->scalerObj.output[1].arr);
        APP_PRINTF("Draw detections graph done!\n");
#endif
    }

    vx_int32 idx = 0;
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    obj->imgMosaicObj.input_arr[idx++] = obj->drawDetectionsObj.output_image_arr;
#else
    obj->imgMosaicObj.input_arr[idx++] = obj->scalerObj.output[0].arr;
#endif
    obj->imgMosaicObj.num_inputs = idx;

    if(status == VX_SUCCESS)
    {
        status = app_create_graph_img_mosaic(obj->graph, &obj->imgMosaicObj, NULL);
        APP_PRINTF("Img Mosaic graph done!\n");
    }

    if(status == VX_SUCCESS)
    {
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
        status = app_create_graph_display(obj->graph, &obj->displayObj, obj->imgMosaicObj.output_image[0]);
        APP_PRINTF("Display graph done!\n");
#endif
    }

    if(status == VX_SUCCESS)
    {
        graph_parameter_index = 0;
        add_graph_parameter_by_node_index(obj->graph, obj->captureObj.node, 1);
        obj->captureObj.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->captureObj.raw_image_arr[0];
        graph_parameter_index++;

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* For AM62A (Linux and QNX): Expose postProc output (parameter index 2) as graph parameter
         * to enable synchronized access for display and eliminate screen tearing */
        add_graph_parameter_by_node_index(obj->graph, obj->postProcObj.node, 2);
        obj->postProcObj.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFFER_Q_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->postProcObj.results[0];
        graph_parameter_index++;
#endif

        if(status == VX_SUCCESS)
        {
            status = vxSetGraphScheduleConfig(obj->graph,
                                VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                                graph_parameter_index,
                                graph_parameters_queue_params_list);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetGraphPipelineDepth(obj->graph, APP_PIPELINE_DEPTH);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 6, APP_BUFFER_Q_DEPTH);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->vissObj.node, 9, APP_BUFFER_Q_DEPTH);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->aewbObj.node, 4, APP_BUFFER_Q_DEPTH);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->ldcObj.node, 7, APP_BUFFER_Q_DEPTH);
        }

        /*This output is accessed slightly later in the pipeline by mosaic node so queue depth is larger */
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 1, 6);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->scalerObj.node, 2, 6);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->preProcObj.node, 2, APP_BUFFER_Q_DEPTH);
        }
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->tidlObj.node, 7, APP_BUFFER_Q_DEPTH);
        }

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->drawDetectionsObj.node, 3, APP_BUFFER_Q_DEPTH);
        }
#endif

        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->imgMosaicObj.node, 1, APP_BUFFER_Q_DEPTH);
        }

        APP_PRINTF("Pipeline params setup done!\n");
    }

    return status;
}



static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graph);

    if(status == VX_SUCCESS)
    {
        APP_PRINTF("Grapy verify SUCCESS!\n");
    }
    else
    {
        APP_PRINTF("Grapy verify FAILURE!\n");
        status = VX_FAILURE;
    }

    #if 1
    if(VX_SUCCESS == status)
    {
        status = tivxExportGraphToDot(obj->graph,".", "vx_app_tidl_od_cam");
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


static vx_status app_run_graph_for_one_frame_pipeline(AppObj *obj, vx_int32 frame_id)
{
    vx_status status = VX_SUCCESS;

    appPerfPointBegin(&obj->total_perf);

    CaptureObj *captureObj = &obj->captureObj;
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    PostProcObj *postProcObj = &obj->postProcObj;
#endif

    if(obj->pipeline <= 0)
    {
        /* Enqueue outputs */
        /* Enqueue inputs during pipeup dont execute */
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, postProcObj->graph_parameter_index, (vx_reference*)&postProcObj->results[obj->enqueueCnt], 1);
        }
#endif
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, captureObj->graph_parameter_index, (vx_reference*)&captureObj->raw_image_arr[obj->enqueueCnt], 1);
        }
        obj->enqueueCnt++;
        obj->enqueueCnt   = (obj->enqueueCnt  >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->pipeline++;
    }


    if(obj->pipeline > 0)
    {
        vx_image capture_input_image;
        uint32_t num_refs;

        /* Dequeue input */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, captureObj->graph_parameter_index, (vx_reference*)&capture_input_image, 1, &num_refs);
        }

#if defined(SOC_AM62A) && defined(QNX)
        /* Dequeue postProc output (display frame) - for QNX this is vx_image */
        vx_image postproc_out_frame;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, postProcObj->graph_parameter_index, (vx_reference*)&postproc_out_frame, 1, &num_refs);
        }

        /* Hand the completed frame to the display task and wait until it
         * has finished copying before we re-enqueue the buffer. */
        if(status == VX_SUCCESS)
        {
            obj->screen_frame = postproc_out_frame;
            sem_post(&obj->disp_frame_ready_sem);
            sem_wait(&obj->disp_frame_done_sem);
        }
#elif defined(SOC_AM62A) && defined(LINUX)
        /* Dequeue postProc output (display frame) - for Linux this is vx_image */
        vx_image postproc_out_frame;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, postProcObj->graph_parameter_index, (vx_reference*)&postproc_out_frame, 1, &num_refs);
        }

        /* Hand the completed frame to the DRM display task and wait until it
         * has finished copying before we re-enqueue the buffer. */
        if(status == VX_SUCCESS)
        {
            obj->drm_frame = postproc_out_frame;
            sem_post(&obj->disp_frame_ready_sem);
            sem_wait(&obj->disp_frame_done_sem);
        }
#endif

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* Re-enqueue postProc output buffer only after display task has finished
         * reading it (sem_wait above returned). */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, postProcObj->graph_parameter_index, (vx_reference*)&postproc_out_frame, 1);
        }
#endif

        /* Enqueue input - start execution */
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, captureObj->graph_parameter_index, (vx_reference*)&capture_input_image, 1);
        }
        obj->enqueueCnt++;
        obj->dequeueCnt++;

        obj->enqueueCnt = (obj->enqueueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->enqueueCnt;
        obj->dequeueCnt = (obj->dequeueCnt >= APP_BUFFER_Q_DEPTH)? 0 : obj->dequeueCnt;
    }

    appPerfPointEnd(&obj->total_perf);

    return status;
}


static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = &obj->sensorObj;
    vx_int32 frame_id;
    int32_t ch_mask = obj->sensorObj.ch_mask;

    app_pipeline_params_defaults(obj);

    if('\0' == sensorObj->sensor_name[0])
    {
        printf("sensor name is NULL \n");
        return VX_FAILURE;
    }
    status = appStartImageSensor(sensorObj->sensor_name, ch_mask);
    APP_PRINTF("appStartImageSensor returned with status: %d\n", status);

#if defined(SOC_AM62A) && defined(QNX)
    status = app_screen_task_create(obj);
    if(status!=0)
    {
        printf("ERROR: Unable to create screen task\n");
    }
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    status = app_drm_task_create(obj);
    if(status!=0)
    {
        printf("ERROR: Unable to create DRM task\n");
    }
#endif

    for(frame_id = 0; frame_id < obj->num_frames_to_run; frame_id++)
    {
#ifdef APP_WRITE_INTERMEDIATE_OUTPUTS
        if(obj->write_file == 1)
        {
            if(obj->captureObj.en_out_capture_write == 1)
            {
                app_send_cmd_capture_write_node(&obj->captureObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->vissObj.en_out_viss_write == 1)
            {
                app_send_cmd_viss_write_node(&obj->vissObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->ldcObj.en_out_ldc_write == 1)
            {
                app_send_cmd_ldc_write_node(&obj->ldcObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->scalerObj.en_out_scaler_write == 1)
            {
                app_send_cmd_scaler_write_node(&obj->scalerObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            if(obj->preProcObj.en_out_pre_proc_write == 1)
            {
                app_send_cmd_pre_proc_write_node(&obj->preProcObj, frame_id, obj->num_frames_to_write, obj->num_frames_to_skip);
            }
            obj->write_file = 0;
        }
#endif
        app_run_graph_for_one_frame_pipeline(obj, frame_id);

        /* user asked to stop processing */
        if(obj->stop_task)
            break;
    }

    vxWaitGraph(obj->graph);

    obj->stop_task = 1;

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Ensure the display task is not left blocking on disp_frame_done_sem
     * after the graph loop exits (e.g. on 'x' keypress). */
    sem_post(&obj->disp_frame_done_sem);
#endif

#if defined(SOC_AM62A) && defined(QNX)
    obj->stop_screen_task = 1;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    obj->stop_drm_task = 1;
#endif

    status = appStopImageSensor(obj->sensorObj.sensor_name, ch_mask);

    return status;
}

static void set_display_defaults(DisplayObj *displayObj)
{
    displayObj->display_option = 1;
}

static void app_pipeline_params_defaults(AppObj *obj)
{
    obj->pipeline       = -APP_BUFFER_Q_DEPTH + 1;
    obj->enqueueCnt     = 0;
    obj->dequeueCnt     = 0;
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
    sensorObj->ch_mask = 1;
    sensorObj->enable_ldc = 1;
    sensorObj->num_cameras_enabled = 1;
    sensorObj->usecase_option = APP_SENSOR_FEATURE_CFG_UC0;
    sensorObj->is_interactive = 1;

}

static void set_scaler_defaults(ScalerObj *scalerObj)
{
    scalerObj->color_format = VX_DF_IMAGE_NV12;
}

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
static void set_pre_proc_defaults(PreProcObj *preProcObj)
{
    vx_int32 i;
    for(i = 0; i < 4; i++ )
    {
        preProcObj->params.pad_pixel[i] = 0;
    }

    for(i = 0; i< 3 ; i++)
    {
        preProcObj->params.scale_val[i] = 1.0;
        preProcObj->params.mean_pixel[i] = 0.0;
    }

    preProcObj->params.ip_rgb_or_yuv = 1; /* YUV-NV12 default */
    preProcObj->params.color_conv_flag = TIADALG_COLOR_CONV_YUV420_BGR;

    /* Number of time to clear the output buffer before it gets reused */
    preProcObj->params.clear_count  = 4;
}
#endif

static void app_default_param_set(AppObj *obj)
{
    set_sensor_defaults(&obj->sensorObj);

    set_scaler_defaults(&obj->scalerObj);

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    set_pre_proc_defaults(&obj->preProcObj);
#endif
    set_display_defaults(&obj->displayObj);

    app_pipeline_params_defaults(obj);

    obj->captureObj.enable_error_detection = 1; /* enable by default */
    obj->is_interactive = 1;
    obj->write_file = 0;
    obj->num_frames_to_run = 1000000000;
}

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
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
    }
    else
    {
        return -1;
    }
}

static void update_img_mosaic_defaults(ImgMosaicObj *imgMosaicObj, vx_uint32 in_width, vx_uint32 in_height, vx_int32 numCh)
{
    vx_int32 idx, ch;
    vx_int32 grid_size = calc_grid_size(numCh);
    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->num_inputs   = 1;

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

    idx = 0;
    for(ch = 0; ch < numCh; ch++)
    {
        vx_int32 startX, startY, winX, winY, winWidth, winHeight;

        winX = ch%grid_size;
        winY = ch/grid_size;

        if((in_width * grid_size) >= imgMosaicObj->out_width)
        {
            winWidth = imgMosaicObj->out_width / grid_size;
            startX = 0;
        }
        else
        {
            winWidth = in_width;
            startX = (imgMosaicObj->out_width - (in_width * grid_size)) / 2;
        }

        if((in_height * grid_size) >= imgMosaicObj->out_height)
        {
            winHeight = imgMosaicObj->out_height / grid_size;
            startY = 0;
        }
        else
        {
            winHeight = in_height;
            startY = (imgMosaicObj->out_height - (in_height * grid_size)) / 2;
        }

        imgMosaicObj->params.windows[idx].startX  = startX + (winWidth * winX);
        imgMosaicObj->params.windows[idx].startY  = startY + (winHeight * winY);
        imgMosaicObj->params.windows[idx].width   = winWidth;
        imgMosaicObj->params.windows[idx].height  = winHeight;
        imgMosaicObj->params.windows[idx].input_select   = 0;
        imgMosaicObj->params.windows[idx].channel_select = idx;
        idx++;
    }

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = APP_BUFFER_Q_DEPTH;
}

static void update_draw_detections_defaults(AppObj *obj, DrawDetectionsObj *drawDetectionsObj)
{
    vx_int32 i;

    drawDetectionsObj->params.width  = obj->scalerObj.output[1].width;
    drawDetectionsObj->params.height = obj->scalerObj.output[1].height;

    for(i = 0; i < drawDetectionsObj->params.num_classes; i++)
    {
        drawDetectionsObj->params.color_map[i][0] = (vx_uint8)(rand() % 256);
        drawDetectionsObj->params.color_map[i][1] = (vx_uint8)(rand() % 256);
        drawDetectionsObj->params.color_map[i][2] = (vx_uint8)(rand() % 256);
    }
}
#else
static void set_img_mosaic_params(AppObj *obj, ImgMosaicObj *imgMosaicObj)
{
    vx_int32 idx;

    imgMosaicObj->out_width    = DISPLAY_WIDTH;
    imgMosaicObj->out_height   = DISPLAY_HEIGHT;
    imgMosaicObj->num_inputs   = 1;

    tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

    idx = 0;

    imgMosaicObj->params.windows[idx].startX  = 100;
    imgMosaicObj->params.windows[idx].startY  = 200;
    imgMosaicObj->params.windows[idx].width   = obj->scalerObj.output[0].width;
    imgMosaicObj->params.windows[idx].height  = obj->scalerObj.output[0].height;
    imgMosaicObj->params.windows[idx].input_select   = 0;
    imgMosaicObj->params.windows[idx].channel_select = 0;
    idx++;

    imgMosaicObj->params.num_windows  = idx;

    /* Number of time to clear the output buffer before it gets reused */
    imgMosaicObj->params.clear_count  = APP_BUFFER_Q_DEPTH;
}
#endif

static void app_update_param_set(AppObj *obj)
{
    obj->sensorObj.sensor_index = 0; /* App works only for IMX390 2MP cameras */

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
    update_draw_detections_defaults(obj, &obj->drawDetectionsObj);
    update_img_mosaic_defaults(&obj->imgMosaicObj, obj->scalerObj.output[1].width, obj->scalerObj.output[1].height, obj->sensorObj.num_cameras_enabled);
#else
    set_img_mosaic_params(obj, &obj->imgMosaicObj);
#endif
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

#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
#ifndef x86_64
static void app_draw_graphics(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type)
{
    appGrpxDrawDefault(handle, draw2dBufInfo, update_type);

    if(update_type == 0)
    {
        Draw2D_FontPrm sHeading;

        sHeading.fontIdx = 4;
        Draw2D_drawString(handle, 580, 5, "TIDL - Object Detection Demo", &sHeading);
    }

    return;
}
#endif
#endif
