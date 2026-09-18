/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include "app_single_cam_main.h"
#include <utils/iss/include/app_iss.h>
#if defined(SOC_AM62A)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "app_test.h"
#if defined(SOC_AM62A)
#pragma GCC diagnostic pop
#endif
#include <TI/hwa_vpac_msc.h>
#include <TI/video_io_kernels.h>
#include <TI/video_io_capture.h>
#if defined (LDRA_COVERAGE)
#include <kernels/coverage_files/include/ldra_remote_core_coverage_main.h>
#endif

#if defined(A72) || defined(A53)
#if defined(LINUX)
/*ITT server is supported only in target mode and only on Linux*/
#include <itt_server.h>
#endif
#endif

#if defined(SOC_AM62A) && defined(QNX)
/*AM62A: QNX to use screen package for displaying frames on A53*/
#include <screen/screen.h>
#include <time.h>
screen_context_t screen_ctx = NULL;
screen_window_t screen_win = NULL;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
/*AM62A: Linux to use DRM/KMS for displaying frames on A53 with zero-copy*/
#include <utils/drm_wrapper/include/drm_wrapper.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <time.h>
#define  DRM_FORMAT_NV12                0x3231564E
#endif

static char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];
static vx_uint8 num_sensors_found;
static IssSensor_CreateParams sensorParams;

#define NUM_CAPT_CHANNELS   (4u)

#ifdef _APP_DEBUG_
static char *app_get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static void app_update_debug_cache_and_dump(AppObj *obj,
                                             vx_bool is_yuv_cam_input,
                                             tivx_raw_image raw_ref,
                                             vx_image cap_yuv_ref,
                                             vx_image y8_r8_c2_ref,
                                             vx_user_data_object h3a_ref,
                                             vx_image scaler_out_ref,
                                             vx_image ldc_out_ref);
#endif //_APP_DEBUG_

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

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

    while(1)
    {
        vx_image source_image;

        /* SYNCHRONIZED ACCESS: Wait for graph task to provide completed buffer */
        if(vx_true_e == obj->scaler_enable)
        {
            struct timespec ts;
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
        }
        else
        {
            /* Fallback: no scaler or old behavior (may still have tearing) */
            source_image = obj->display_image;
        }

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
        if(vx_true_e == obj->scaler_enable)
        {
            sem_post(&obj->disp_frame_done_sem);
        }

        /* Post frame to display */
        err = screen_post_window(screen_win, screen_buf[0], 0, NULL, 0);
        if(err != 0) {
            printf("[QNX Screen] Failed to post window\n");
            break;
        }

        if(obj->stop_screen_task == 1)
        {
            break;
        }
    }

    /* Unblock the graph task if it is blocked on disp_frame_done_sem during exit */
    if(vx_true_e == obj->scaler_enable)
    {
        sem_post(&obj->disp_frame_done_sem);
    }

    return err;
}
#endif

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
 * \brief Linux DRM display task function with double-buffered canvases
 *
 * Two NV12 canvases are allocated and registered as DRM framebuffers
 * (buf_idx 0 and 1).  Each iteration blits the latest TIOVX output into
 * the back canvas, then flips to it.  Combined with the VBlank-gated
 * page_flip_pending wait in appDrmWrapperRender, this ensures the DSS
 * has finished scanning out a canvas before it is overwritten.
 *
 * When the pipeline image is smaller than the display resolution the
 * canvas is sized to the display and the image is centered (letterbox).
 * When sizes match the canvas equals the image size and the blit is a
 * full-frame copy.
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
    vx_image canvas[2] = {NULL, NULL}; /* Double-buffered canvases for DRM */
    uint32_t canvas_w, canvas_h;       /* Canvas dimensions (>= image size) */
    int needs_letterbox = 0;
    uint32_t disp_w = 1920;
    uint32_t disp_h = 1080;
    uint32_t i;

    /* Query display image properties */
    vx_image display_image = obj->display_image;
    vxQueryImage(display_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(display_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(display_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    /* Map TIOVX format to DRM format */
    if (VX_DF_IMAGE_NV12 == df)
    {
        drm_pix_format = DRM_FORMAT_NV12;
    }
    else if (TIVX_DF_IMAGE_NV12_P12 == df)
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
        /* Fill borders with black (Y=0, UV=128) */
        app_drm_clear_nv12(canvas[i], canvas_w, canvas_h);
        /* Pre-populate with current pipeline frame so first modeset
         * has valid pixel content */
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

    /* Register both canvases; buf_idx 0 triggers the initial modeset */
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

    /*
     * Main display loop — fully synchronised with the TIOVX pipeline.
     *
     * Per-iteration sequence:
     *   1. sem_wait(disp_frame_ready_sem): block until the graph task has
     *      dequeued a completed scaler output buffer and stored it in
     *      obj->drm_frame.  This buffer is guaranteed fully written by
     *      TIOVX — no source-side read/write race.
     *   2. appDrmWrapperWaitFlipDone: block until the previous DRM page flip
     *      has completed (VBlank fired) so canvas[buf_idx] is the retired
     *      back buffer and safe to overwrite.
     *   3. Blit obj->drm_frame → canvas[buf_idx].
     *   4. sem_post(disp_frame_done_sem): tell the graph task the buffer has
     *      been consumed and may be re-enqueued to TIOVX.
     *   5. Submit the page flip to canvas[buf_idx].
     *   6. Advance buf_idx.
     */
    buf_idx = 0;
    while (1)
    {
        vx_image src_frame;

        if (vx_true_e == obj->scaler_enable)
        {
            /*
             * Scaler output is a graph parameter: wait for the graph task to
             * dequeue a guaranteed-complete frame before blitting.
             */
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;  /* 1-second timeout so stop_drm_task is polled */
            if (sem_timedwait(&obj->disp_frame_ready_sem, &ts) != 0)
            {
                if (obj->stop_drm_task == 1) { break; }
                continue;
            }
            src_frame = obj->drm_frame;
        }
        else
        {
            /* No scaler graph parameter: fall back to reading display_image
             * directly (source-side race possible but display works).      */
            src_frame = display_image;
        }

        /* Ensure back canvas is no longer being scanned out */
        appDrmWrapperWaitFlipDone(drm_handle);

        /* Blit completed frame into the safe back buffer */
        app_drm_letterbox_nv12(canvas[buf_idx], src_frame,
                               canvas_w, canvas_h, width, height);

        if (vx_true_e == obj->scaler_enable)
        {
            /* Signal graph task: blit done, buffer may be re-enqueued */
            sem_post(&obj->disp_frame_done_sem);
        }

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

    /* Unblock the graph task if it is blocked on disp_frame_done_sem */
    if (vx_true_e == obj->scaler_enable)
    {
        sem_post(&obj->disp_frame_done_sem);
    }

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

vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint8 count = 0;
    char ch = 0xFF;
    vx_uint8 selectedSensor = 0xFF;
    vx_uint8 detectedSensors[ISS_SENSORS_MAX_CHANNEL];
#if defined(A72) || defined(A53)
#if defined(LINUX)
    /* ITT server is supported only in target mode and only on Linux */
    status = itt_server_init((void*)obj, (void*)save_debug_images, (void*)appSingleCamUpdateVpacDcc);
    if(status != 0)
    {
        printf("Warning : Failed to initialize ITT server. Live tuning will not work \n");
    }
#endif
#endif

    for(count=0;count<ISS_SENSORS_MAX_CHANNEL;count++)
    {
        detectedSensors[count] = 0xFF;
    }

    for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
    {
        sensor_list[count] = NULL;
    }

    obj->stop_task = 0;
    obj->stop_task_done = 0;

#ifdef _APP_DEBUG_
    obj->debug_last_frame_valid = vx_false_e;
    if (tivxMutexCreate(&obj->debug_last_frame_mutex) != VX_SUCCESS)
    {
        printf("Warning : Failed to create debug_last_frame_mutex. save_debug_images() will not work \n");
    }
#endif

#if defined(SOC_AM62A) && defined(QNX)
    obj->stop_screen_task = 0;
    obj->stop_screen_task_done = 0;
#endif

#if defined(SOC_AM62A) && defined(LINUX)
    obj->stop_drm_task = 0;
    obj->stop_drm_task_done = 0;
    obj->drm_handle = NULL;
    obj->drm_frame  = NULL;
#endif

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Common initialization for synchronization */
    sem_init(&obj->disp_frame_ready_sem, 0, 0);
    sem_init(&obj->disp_frame_done_sem,  0, 0);
#endif

#if defined(SOC_AM62A) && defined(QNX)
    obj->screen_frame = NULL;
#endif

    obj->selectedCam = 0xFF;

    if(status == VX_SUCCESS)
    {
        obj->context = vxCreateContext();
        status = vxGetStatus((vx_reference) obj->context);
    }

    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxVideoIOLoadKernels(obj->context);
        tivxImagingLoadKernels(obj->context);
        APP_PRINTF("tivxImagingLoadKernels done\n");
    }

    /*memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);*/
    for(count=0;count<ISS_SENSORS_MAX_SUPPORTED_SENSOR;count++)
    {
        availableSensorNames[count][0] = '\0';
        sensor_list[count] = availableSensorNames[count];
    }

    if(status == VX_SUCCESS)
    {
        status = appEnumerateImageSensor(sensor_list, &num_sensors_found);
    }

    if(obj->is_interactive && !obj->file_read_enable)
    {
        selectedSensor = 0xFF;
        obj->selectedCam = 0xFF;
        while(obj->selectedCam == 0xFF)
        {
            printf("Select camera port index 0-%d : ", ISS_SENSORS_MAX_CHANNEL-1);
            ch = getchar();
            obj->selectedCam = ch - '0';

            if(obj->selectedCam >= ISS_SENSORS_MAX_CHANNEL)
            {
                printf("Invalid entry %c. Please choose between 0 and %d \n", ch, ISS_SENSORS_MAX_CHANNEL-1);
                obj->selectedCam = 0xFF;
            }

            while ((obj->selectedCam != 0xFF) && (selectedSensor > (num_sensors_found-1)))
            {
                printf("%d registered sensor drivers\n", num_sensors_found);
                for(count=0;count<num_sensors_found;count++)
                {
                    printf("%c : %s \n", count+'a', sensor_list[count]);
                }

                printf("Select a sensor above or press '0' to autodetect the sensor : ");
                ch = getchar();
                if(ch == '0')
                {
                    uint8_t num_sensors_detected = 0;
                    uint32_t channel_mask = (1<<obj->selectedCam);

                    status = appDetectImageSensor(detectedSensors, &num_sensors_detected, channel_mask);
                    if(0 == status)
                    {
                        selectedSensor = detectedSensors[obj->selectedCam];
                        if(selectedSensor > ISS_SENSORS_MAX_SUPPORTED_SENSOR)
                        {
                            printf("No sensor detected at port %d. Please select another port \n", obj->selectedCam);
                            obj->selectedCam = 0xFF;
                            selectedSensor = 0xFF;
                        }
                    }
                    else
                    {
                        printf("sensor detection at port %d returned error . Please try again \n", obj->selectedCam);
                        obj->selectedCam = 0xFF;
                        selectedSensor = 0xFF;
                    }
                }
                else
                {
                    selectedSensor = ch - 'a';
                    if(selectedSensor > (num_sensors_found-1))
                    {
                        printf("Invalid selection %c. Try again \n", ch);
                    }
                }
            }
        }

        obj->sensor_name = sensor_list[selectedSensor];
        printf("Sensor selected : %s\n", obj->sensor_name);

        #if !defined(SOC_FAMILY_TDA5)
        #ifdef VPAC3
        /* Selection for MV enable */
        ch = 0xFF;
        fflush (stdin);
        while ((ch != '0') && (ch != '1'))
        {
            fflush (stdin);
            printf ("Dual FCP enable for MV Selection Yes(1)/No(0) : ");
            ch = getchar();
        }

        obj->vpac3_dual_fcp_enable = ch - '0';
        #endif

        #if defined(VPAC3) || defined(VPAC3L)
        /* Selection for CAC enable */
        ch = 0xFF;
        fflush (stdin);
        while ((ch != '0') && (ch != '1'))
        {
            fflush (stdin);
            printf ("CAC Selection Yes(1)/No(0) : ");
            ch = getchar();
        }

        obj->cac_enable = ch - '0';
        #endif
        #endif

    }
    else
    {
        selectedSensor = obj->sensor_sel;
        obj->sensor_name = sensor_list[selectedSensor];
        obj->selectedCam = 0;
        if(selectedSensor > (num_sensors_found-1))
        {
            printf("Invalid sensor selection %d \n", selectedSensor);
            return VX_FAILURE;
        }
    }

    ch = 0xFF;
    fflush (stdin);
    while ((ch != '0') && (ch != '1'))
    {
        fflush (stdin);
        printf ("LDC Selection Yes(1)/No(0) : ");
        ch = getchar();
    }

    obj->ldc_enable = ch - '0';

    obj->sensor_wdr_mode = 0;

    obj->table_width = LDC_TABLE_WIDTH;
    obj->table_height = LDC_TABLE_HEIGHT;
    obj->ds_factor = LDC_DS_FACTOR;

    /* Display initialization HV*/
    memset(&obj->display_params, 0, sizeof(tivx_display_params_t));
    obj->display_params.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
    obj->display_params.pipeId = 0; // TODO: Need to clean this up ultimately when merging back to mainline
    obj->display_params.outHeight = 1080;
    obj->display_params.outWidth = 1920;
    obj->display_params.posX = 0;
    obj->display_params.posY = 0;

#ifdef VPAC3
    /* YUV8 output from dual CC for HV and MV */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        /* Display Modification for HV*/
        obj->display_params.outWidth = 960;

        /* Display initialization MV*/
        memset(&obj->display_params_MV, 0, sizeof(tivx_display_params_t));
        obj->display_params_MV.opMode = TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE;
        obj->display_params_MV.pipeId = 2;
        obj->display_params_MV.outHeight = 1080;
        obj->display_params_MV.outWidth = 960;
        obj->display_params_MV.posX = 960;
        obj->display_params_MV.posY = 0;
    }
#endif

    obj->scaler_enable = vx_false_e;

    appPerfPointSetName(&obj->total_perf , "TOTAL");

    return status;
}

vx_status app_deinit(AppObj *obj)
{
    vx_status status = VX_FAILURE;

#ifdef _APP_DEBUG_
    tivxMutexDelete(&obj->debug_last_frame_mutex);
#endif

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Common cleanup for synchronization */
    sem_destroy(&obj->disp_frame_ready_sem);
    sem_destroy(&obj->disp_frame_done_sem);
#endif

    tivxHwaUnLoadKernels(obj->context);
    APP_PRINTF("tivxHwaUnLoadKernels done\n");

    tivxVideoIOUnLoadKernels(obj->context);
    APP_PRINTF("tivxVideoIOUnLoadKernels done\n");

    tivxImagingUnLoadKernels(obj->context);
    APP_PRINTF("tivxImagingUnLoadKernels done\n");

    status = vxReleaseContext(&obj->context);
    if(VX_SUCCESS == status)
    {
        APP_PRINTF("vxReleaseContext done\n");
    }
    else
    {
        printf("Error: vxReleaseContext returned 0x%x \n", status);
    }
    return status;
}

/*
 * Graph,
 *           viss_config
 *               |
 *               v
 * input_img -> VISS -----> LDC -----> output_img
 *
 */

vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    int32_t sensor_init_status = -1;
    obj->configuration = NULL;
    obj->raw = NULL;
    obj->y12 = NULL;
    obj->uv12_c1 = NULL;
    obj->y8_r8_c2 = NULL;
    obj->uv8_g8_c3 = NULL;
    obj->s8_b8_c4 = NULL;
    obj->histogram = NULL;
    obj->h3a_aew_af = NULL;
#ifdef VPAC3
    obj->display_image = NULL;
    obj->display_image_MV = NULL;
#endif

    unsigned int image_width = obj->width_in;
    unsigned int image_height = obj->height_in;

    tivx_raw_image raw_image = 0;
    vx_user_data_object capture_config;
    vx_uint8 num_capture_frames = 1;
    tivx_capture_params_t local_capture_config;
    uint32_t buf_id;
    const vx_char capture_user_data_object_name[] = "tivx_capture_params_t";
    uint32_t sensor_features_enabled = 0;
    uint32_t sensor_features_supported = 0;
    uint32_t sensor_wdr_enabled = 0;
    uint32_t sensor_exp_control_enabled = 0;
    uint32_t sensor_gain_control_enabled = 0;

    vx_bool yuv_cam_input = vx_false_e;
    vx_image viss_out_image = NULL;
#ifdef VPAC3
    vx_image viss_out_image_MV = NULL;
#endif
    vx_image ldc_in_image = NULL;
    vx_image capt_yuv_image = NULL;

    uint32_t channel_mask = (1<<obj->selectedCam);

    /* On AM62A Linux/QNX the scaler output is a second graph parameter so
     * the app can receive the exact completed buffer from TIOVX.        */
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    vx_uint32 params_list_depth = 2;
#else
    vx_uint32 params_list_depth = 1;
#endif
    if(obj->test_mode == 1)
    {
        params_list_depth++;
    }
    /* Fixed-size to avoid VLA; 6 covers all combinations (capture/raw +
     * AM62A scaler + test_mode display + y8_r8_c2 + h3a_aew_af + ldc_out).
     * params_list_depth is recomputed below from the actual number of
     * parameters registered (graph_parameter_num), so this initial value
     * is unused once _APP_DEBUG_ adds the 3 extra debug parameters.      */
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[6];

    printf("Querying %s \n", obj->sensor_name);
    memset(&sensorParams, 0, sizeof(sensorParams));
    status = appQueryImageSensor(obj->sensor_name, &sensorParams);
    if(VX_SUCCESS != status)
    {
        printf("appQueryImageSensor returned %d\n", status);
        return status;
    }

    if(sensorParams.sensorInfo.raw_params.format[0].pixel_container == VX_DF_IMAGE_UYVY)
    {
        yuv_cam_input = vx_true_e;
        printf("YUV Input selected. VISS and AEWB nodes will be bypassed. \n");
    }


    /*
    Check for supported sensor features.
    It is upto the application to decide which features should be enabled.
    This demo app enables WDR, DCC and 2A if the sensor supports it.
    */
    sensor_features_supported = sensorParams.sensorInfo.features;

    if(vx_false_e == yuv_cam_input)
    {
        if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
        {
            APP_PRINTF("WDR mode is supported \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
            sensor_wdr_enabled = 1;
            obj->sensor_wdr_mode = 1;
        }else
        {
            APP_PRINTF("WDR mode is not supported. Defaulting to linear \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
            sensor_wdr_enabled = 0;
            obj->sensor_wdr_mode = 0;
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

        if(ISS_SENSOR_FEATURE_CFG_UC1 == (sensor_features_supported & ISS_SENSOR_FEATURE_CFG_UC1))
        {
            APP_PRINTF("CMS Usecase is supported \n");
            sensor_features_enabled |= ISS_SENSOR_FEATURE_CFG_UC1;
        }

        switch(sensorParams.sensorInfo.aewbMode)
        {
            case ALGORITHMS_ISS_AEWB_MODE_NONE:
                obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
                obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_DISABLED;
                break;
            case ALGORITHMS_ISS_AEWB_MODE_AWB:
                obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
                obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_AUTO;
                break;
            case ALGORITHMS_ISS_AEWB_MODE_AE:
                obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_AUTO;
                obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_DISABLED;
                break;
            case ALGORITHMS_ISS_AEWB_MODE_AEWB:
                obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_AUTO;
                obj->aewb_cfg.awb_mode = ALGORITHMS_ISS_AWB_AUTO;
                break;
        }
        if(obj->aewb_cfg.ae_mode == ALGORITHMS_ISS_AE_DISABLED)
        {
            if(sensor_exp_control_enabled || sensor_gain_control_enabled )
            {
                obj->aewb_cfg.ae_mode = ALGORITHMS_ISS_AE_MANUAL;
            }
        }

        APP_PRINTF("obj->aewb_cfg.ae_mode = %d\n", obj->aewb_cfg.ae_mode);
        APP_PRINTF("obj->aewb_cfg.awb_mode = %d\n", obj->aewb_cfg.awb_mode);

    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
        APP_PRINTF("Sensor DCC is enabled \n");
    }else
    {
        APP_PRINTF("Sensor DCC is NOT enabled \n");
    }

    APP_PRINTF("Sensor width = %d\n", sensorParams.sensorInfo.raw_params.width);
    APP_PRINTF("Sensor height = %d\n", sensorParams.sensorInfo.raw_params.height);
    APP_PRINTF("Sensor DCC ID = %d\n", sensorParams.dccId);
    APP_PRINTF("Sensor Supported Features = 0x%x\n", sensor_features_supported);
    APP_PRINTF("Sensor Enabled Features = 0x%x\n", sensor_features_enabled);
    if (!obj->file_read_enable)
    {
        sensor_init_status = appInitImageSensor(obj->sensor_name, sensor_features_enabled, channel_mask);/*Mask = 1 for camera # 0*/
        if(0 != sensor_init_status)
        {
            /* Not returning failure because application may be waiting for
                error/test frame */
            printf("Error initializing sensor %s \n", obj->sensor_name);
        }
    }

    image_width     = sensorParams.sensorInfo.raw_params.width;
    image_height    = sensorParams.sensorInfo.raw_params.height;
    obj->cam_dcc_id = sensorParams.dccId;
    obj->width_in = image_width;
    obj->height_in = image_height;

/*
Assuming same dataformat for all exposures.
This may not be true for staggered HDR. WIll be handled later
    for(count = 0;count<raw_params.num_exposures;count++)
    {
        memcpy(&(raw_params.format[count]), &(sensorProperties.sensorInfo.dataFormat), sizeof(tivx_raw_image_format_t));
    }
*/

/*
Sensor driver does not support metadata yet.
*/

    APP_PRINTF("Creating graph \n");

    obj->graph = vxCreateGraph(obj->context);
    if(status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference) obj->graph);
    }
    APP_ASSERT(vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1));

    APP_PRINTF("Initializing params for capture node \n");

    /* Setting to num buf of capture node */
    obj->num_cap_buf = NUM_BUFS;

    if (obj->file_read_enable)
    {
        if (vx_true_e == yuv_cam_input)
        {
            printf("Error: file_read_enable is only supported for RAW sensors (not YUV camera input)\n");
            return VX_FAILURE;
        }

        obj->capture_node = NULL;
        /* allocate multiple raw image refs to allow pipelining of graph */
        for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
        {
            if(status == VX_SUCCESS)
            {
                obj->raw_frames[buf_id] = tivxCreateRawImage(obj->context, &sensorParams.sensorInfo.raw_params);
                status = vxGetStatus((vx_reference) obj->raw_frames[buf_id]);
            }
        }
        obj->raw = obj->raw_frames[0];

        status = app_create_viss(obj, sensor_wdr_enabled);
        if(VX_SUCCESS == status)
        {
            vxSetNodeTarget(obj->node_viss, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1);
#ifndef _APP_DEBUG_
            tivxSetNodeParameterNumBufByIndex(obj->node_viss, 6u, obj->num_cap_buf);
#endif
        }
        else
        {
            printf("app_create_viss failed \n");
            return -1;
        }

        /* AEWB result is not consumed by VISS in file-read mode (VISS is
         * created with ae_awb_result = NULL), so the AEWB node is skipped
         * entirely to avoid running it against canned frames for no
         * functional benefit. */
        viss_out_image = obj->y8_r8_c2;
#ifdef VPAC3
        /* Populate viss_out_image with image created */
        if (obj->vpac3_dual_fcp_enable == 1U)
        {
            viss_out_image_MV = obj->y12;
        }
#endif
        ldc_in_image = viss_out_image;
    }
    else
    {
        if(vx_false_e == yuv_cam_input)
        {
            raw_image = tivxCreateRawImage(obj->context, &sensorParams.sensorInfo.raw_params);

            /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
            for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
            {
                if(status == VX_SUCCESS)
                {
                    obj->cap_frames[buf_id] = vxCreateObjectArray(obj->context, (vx_reference)raw_image, num_capture_frames);
                    status = vxGetStatus((vx_reference) obj->cap_frames[buf_id]);
                }
            }
        }
        else
        {
            capt_yuv_image = vxCreateImage(
                                    obj->context,
                                    sensorParams.sensorInfo.raw_params.width,
                                    sensorParams.sensorInfo.raw_params.height,
                                    VX_DF_IMAGE_UYVY
                             );

            /* allocate Input and Output refs, multiple refs created to allow pipelining of graph */
            for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
            {
                if(status == VX_SUCCESS)
                {
                    obj->cap_frames[buf_id] = vxCreateObjectArray(obj->context, (vx_reference)capt_yuv_image, num_capture_frames);
                    status = vxGetStatus((vx_reference) obj->cap_frames[buf_id]);
                }
            }
        }

        /* Config initialization */
        tivx_capture_params_init(&local_capture_config);

        local_capture_config.timeout = 33;
        local_capture_config.timeoutInitial = 500;

#if defined(SOC_AM62A)
        local_capture_config.numInst  = 1U;/* Single instance for AM62A */
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        local_capture_config.numInst  = 3U;/* Configure three instances */
#elif defined(SOC_J721S2) || defined(SOC_J722S) || defined(SOC_J721E)
        local_capture_config.numInst  = 2U;/* Configure both instances */
#elif defined(SOC_TDA54)
        local_capture_config.numInst  = 1U;/* Configure one instances */
#endif
        local_capture_config.numCh = 1U;/* Single cam. Only 1 channel enabled */
        {
            vx_uint8 ch, id, lane, q;
            for(id = 0; id < local_capture_config.numInst; id++)
            {
                local_capture_config.instId[id]                       = id;
                #if defined(SOC_FAMILY_J7) || defined(SOC_FAMILY_AM)
                local_capture_config.instCfg[id].enableCsiv2p0Support = (uint32_t)vx_true_e;
                #elif defined(SOC_FAMILY_TDA5)
                local_capture_config.instCfg[id].phyMode              = TIVX_CAPTURE_DPHY_MODE;
                #endif
                local_capture_config.instCfg[id].numDataLanes         = sensorParams.sensorInfo.numDataLanes;
                local_capture_config.instCfg[id].laneBandSpeed        = sensorParams.sensorInfo.csi_laneBandSpeed;

                for (lane = 0; lane < local_capture_config.instCfg[id].numDataLanes; lane++)
                {
                    local_capture_config.instCfg[id].dataLanesMap[lane] = lane + 1;
                }
                for (q = 0; q < NUM_CAPT_CHANNELS; q++)
                {
                    ch = (NUM_CAPT_CHANNELS-1)* id + q;
                    local_capture_config.chVcNum[ch]   = q;
                    local_capture_config.chInstMap[ch] = id;
                }
            }
        }

        local_capture_config.chInstMap[0] = obj->selectedCam/NUM_CAPT_CHANNELS;
        local_capture_config.chVcNum[0]   = obj->selectedCam%NUM_CAPT_CHANNELS;

        capture_config = vxCreateUserDataObject(obj->context, capture_user_data_object_name, sizeof(tivx_capture_params_t), &local_capture_config);
        APP_PRINTF("capture_config = 0x%p \n", capture_config);

        APP_PRINTF("Creating capture node \n");
        obj->capture_node = tivxCaptureNode(obj->graph, capture_config, obj->cap_frames[0]);
        APP_PRINTF("obj->capture_node = 0x%p \n", obj->capture_node);

        if(status == VX_SUCCESS)
        {
            status = vxReleaseUserDataObject(&capture_config);
        }
        if(status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(obj->capture_node, VX_TARGET_STRING, TIVX_TARGET_CAPTURE2);
        }

        if(vx_false_e == yuv_cam_input)
        {
            obj->raw = (tivx_raw_image)vxGetObjectArrayItem(obj->cap_frames[0], 0);
            if(status == VX_SUCCESS)
            {
                status = tivxReleaseRawImage(&raw_image);
            }
#ifdef _APP_DEBUG_


            obj->fs_test_raw_image = tivxCreateRawImage(obj->context, &(sensorParams.sensorInfo.raw_params));

            if (NULL != obj->fs_test_raw_image)
            {
                if(status == VX_SUCCESS)
                {
                    status = read_test_image_raw(NULL, obj->fs_test_raw_image, obj->test_mode);
                }
                else
                {
                    status = tivxReleaseRawImage(&obj->fs_test_raw_image);
                    obj->fs_test_raw_image = NULL;
                }
            }
#endif //_APP_DEBUG_

            status = app_create_viss(obj, sensor_wdr_enabled);
            if(VX_SUCCESS == status)
            {
            vxSetNodeTarget(obj->node_viss, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1);
#ifndef _APP_DEBUG_
            tivxSetNodeParameterNumBufByIndex(obj->node_viss, 6u, obj->num_cap_buf);
#endif
            }
            else
            {
                printf("app_create_viss failed \n");
                return -1;
            }

            status = app_create_aewb(obj, sensor_wdr_enabled);
            if(VX_SUCCESS != status)
            {
                printf("app_create_aewb failed \n");
                return -1;
            }
            viss_out_image = obj->y8_r8_c2;
#ifdef VPAC3
        /* Populate viss_out_image with image created */
        if (obj->vpac3_dual_fcp_enable == 1U)
        {
            viss_out_image_MV = obj->y12;
        }
#endif
            ldc_in_image = viss_out_image;
        }
        else
        {
            obj->capt_yuv_image = (vx_image)vxGetObjectArrayItem(obj->cap_frames[0], 0);
            ldc_in_image = obj->capt_yuv_image;
            vxReleaseImage(&capt_yuv_image);
        }
    }

    if (obj->ldc_enable)
    {
        printf ("Enabling LDC \n");
        status = app_create_ldc(obj, ldc_in_image);

        if(status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(obj->node_ldc, VX_TARGET_STRING, TIVX_TARGET_VPAC_LDC1);
        }
        else
        {
            printf("app_create_ldc returned error \n");
            return status;
        }
#ifndef _APP_DEBUG_
        if(status == VX_SUCCESS)
        {
            status = tivxSetNodeParameterNumBufByIndex(obj->node_ldc, 7u, obj->num_cap_buf);
        }
#endif

        /*Check if resizing is needed for display*/
        if((obj->table_width >= obj->display_params.outWidth) && (obj->table_height >= obj->display_params.outHeight))
        {
            vx_uint16 scaler_out_w, scaler_out_h;
            obj->scaler_enable = vx_true_e;
            appIssGetResizeParams(obj->table_width, obj->table_height, obj->display_params.outWidth, obj->display_params.outHeight, &scaler_out_w, &scaler_out_h);
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
            /* Allocate one image per pipeline slot; all are exposed as a
             * graph output parameter so we can dequeue the exact buffer
             * TIOVX just finished writing (eliminates source-side tearing). */
            for(buf_id = 0; buf_id < obj->num_cap_buf; buf_id++)
            {
                obj->scaler_out_imgs[buf_id] = vxCreateImage(obj->context,
                                                              scaler_out_w,
                                                              scaler_out_h,
                                                              VX_DF_IMAGE_NV12);
            }
            obj->scaler_out_img = obj->scaler_out_imgs[0];
#else
            obj->scaler_out_img = vxCreateImage(obj->context, scaler_out_w, scaler_out_h, VX_DF_IMAGE_NV12);
#endif
            obj->scalerNode = tivxVpacMscScaleNode(obj->graph, obj->ldc_out, obj->scaler_out_img, NULL, NULL, NULL, NULL);
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
            /* On AM62A Linux/QNX the scaler output is managed as a graph
             * parameter below; skip internal buffer count management.   */
            if(status == VX_SUCCESS)
            {
                status = tivxSetNodeParameterNumBufByIndex(obj->scalerNode, 1u, obj->num_cap_buf);
            }
#endif
            obj->display_params.outHeight = scaler_out_h;
#ifdef VPAC3
        /* HV display scaler height modification */
        if (obj->vpac3_dual_fcp_enable == 1U)
        {
            obj->display_params.outHeight = (scaler_out_h)*2;
        }
#endif
            obj->display_params.outWidth = scaler_out_w;
            obj->display_image = obj->scaler_out_img;
        }else /*No resize needed*/
        {
            obj->scaler_enable = vx_false_e;
            obj->display_image = obj->ldc_out;

            /* MSC can only downsize. If ldc resolution is lower,
               display resolution must be set accordingly
            */
            obj->display_params.outWidth = obj->table_width;
            obj->display_params.outHeight = obj->table_height;
        }
    }
    else /*ldc_enable*/
    {
        if(NULL != obj->capt_yuv_image)
        {
            /*MSC does not support YUV422 input*/
            obj->scaler_enable = vx_false_e;
        }
        else
        {
            if ((image_width >= obj->display_params.outWidth) && (image_height >= obj->display_params.outHeight))
            {
                obj->scaler_enable = vx_true_e;
            }
            else
            {
                obj->scaler_enable = vx_false_e;
                /* MSC can only downsize. If viss resolution is lower,
                   display resolution must be set accordingly
                */
                obj->display_params.outWidth = image_width;
                obj->display_params.outHeight = image_height;
            }
        }

        if(vx_true_e == obj->scaler_enable)
        {
            vx_uint16 scaler_out_w, scaler_out_h;
            appIssGetResizeParams(image_width, image_height, obj->display_params.outWidth, obj->display_params.outHeight, &scaler_out_w, &scaler_out_h);
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
            for(buf_id = 0; buf_id < obj->num_cap_buf; buf_id++)
            {
                obj->scaler_out_imgs[buf_id] = vxCreateImage(obj->context,
                                                              scaler_out_w,
                                                              scaler_out_h,
                                                              VX_DF_IMAGE_NV12);
            }
            obj->scaler_out_img = obj->scaler_out_imgs[0];
#else
            obj->scaler_out_img = vxCreateImage(obj->context, scaler_out_w, scaler_out_h, VX_DF_IMAGE_NV12);
#endif
            obj->scalerNode = tivxVpacMscScaleNode(obj->graph, ldc_in_image, obj->scaler_out_img, NULL, NULL, NULL, NULL);
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
            if(status == VX_SUCCESS)
            {
                status = tivxSetNodeParameterNumBufByIndex(obj->scalerNode, 1u, obj->num_cap_buf);
            }
#endif
            if(status == VX_SUCCESS)
            {
                status = vxSetNodeTarget(obj->scalerNode, VX_TARGET_STRING, TIVX_TARGET_VPAC_MSC1);
            }
            obj->display_params.outHeight = scaler_out_h;
#ifdef VPAC3
            /* HV display scaler height modification */
            if (obj->vpac3_dual_fcp_enable == 1U)
            {
                obj->display_params.outHeight = (scaler_out_h)*2;
            }
#endif
            obj->display_params.outWidth = scaler_out_w;
            obj->display_image = obj->scaler_out_img;
        }
        else
        {
            obj->display_image = ldc_in_image;
        }
    }
#ifdef VPAC3
            /* Populate Display image MV with viss_out_image_MV */
            if (obj->vpac3_dual_fcp_enable == 1U)
            {
                obj->display_image_MV = viss_out_image_MV;
            }
#endif

    if(NULL == obj->display_image)
    {
        printf("Error : Display input is uninitialized \n");
        return VX_FAILURE;
    }
    else
    {
        obj->display_params.posX = (1920U - obj->display_params.outWidth)/2;
        obj->display_params.posY = (1080U - obj->display_params.outHeight)/2;
#ifdef VPAC3
        /* HV position modification for MV display */
        if (obj->vpac3_dual_fcp_enable == 1U)
        {
            obj->display_params.posX = 0U;
            obj->display_params.posY = 0U;
        }
#endif
#if !(defined(SOC_AM62A))
        obj->display_param_obj = vxCreateUserDataObject(obj->context, "tivx_display_params_t", sizeof(tivx_display_params_t), &obj->display_params);
        obj->displayNode = tivxDisplayNode(obj->graph, obj->display_param_obj, obj->display_image);
#endif
    }

#if !(defined(SOC_AM62A))
#ifdef VPAC3
    /* Check if display_image_MV is not NULL and create display node */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        if(NULL == obj->display_image_MV)
        {
            printf("Error : Display MV input is uninitialized \n");
            return VX_FAILURE;
        }
        else
        {
            obj->display_params_MV.posX = 960U;
            obj->display_params_MV.posY = 0U;
            obj->display_param_MV_obj = vxCreateUserDataObject(obj->context, "tivx_display_params_t", sizeof(tivx_display_params_t), &obj->display_params_MV);
            obj->displayNode_MV = tivxDisplayNode(obj->graph, obj->display_param_MV_obj, obj->display_image_MV);
        }
    }
#endif

    if(status == VX_SUCCESS)
    {
        status = vxSetNodeTarget(obj->displayNode, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1);
        APP_PRINTF("Display Set Target done\n");
    }
#ifdef VPAC3
    /* Check status of MV node creation */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        if(status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(obj->displayNode_MV, VX_TARGET_STRING, TIVX_TARGET_DISPLAY1);
            APP_PRINTF("Display MV Set Target done\n");
        }
    }
#endif
#endif /* #if !(defined(SOC_AM62A) && defined(QNX)) */

    {
        int graph_parameter_num = 0;

        if (!obj->file_read_enable)
        {
            /* input @ node index 1, becomes graph parameter 0 */
            add_graph_parameter_by_node_index(obj->graph, obj->capture_node, 1);

            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->cap_frames[0]);
            graph_parameter_num++;
        }
        else
        {
            /* VISS raw input @ node index 3, becomes graph parameter 0 */
            add_graph_parameter_by_node_index(obj->graph, obj->node_viss, 3);

            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->raw_frames[0]);
            graph_parameter_num++;
        }

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* Scaler output becomes graph parameter 1.  By managing these buffers
         * as graph parameters we can dequeue the exact image TIOVX just
         * finished writing, which eliminates the source-side read/write race
         * that causes tearing when blitting from display_image.             */
        if(vx_true_e == obj->scaler_enable)
        {
            add_graph_parameter_by_node_index(obj->graph, obj->scalerNode, 1);
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list =
                    (vx_reference*)&(obj->scaler_out_imgs[0]);
            graph_parameter_num++;
        }
#endif

        if(obj->test_mode == 1)
        {
            add_graph_parameter_by_node_index(obj->graph, obj->displayNode, 1);
            /* set graph schedule config such that graph parameter @ index 0 is enqueuable */
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = 1;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->display_image);
            graph_parameter_num++;
        }

#ifdef _APP_DEBUG_
        if (vx_false_e == yuv_cam_input)
        {
            /* y8_r8_c2: VISS output2 @ node index 6 */
            add_graph_parameter_by_node_index(obj->graph, obj->node_viss, 6);
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->y8_r8_c2_frames[0]);
            obj->y8_r8_c2_graph_param_idx = graph_parameter_num;
            graph_parameter_num++;

            /* h3a_aew_af: VISS h3a_output @ node index 9 */
            add_graph_parameter_by_node_index(obj->graph, obj->node_viss, 9);
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->h3a_aew_af_frames[0]);
            obj->h3a_aew_af_graph_param_idx = graph_parameter_num;
            graph_parameter_num++;
        }
        else
        {
            obj->y8_r8_c2_graph_param_idx = -1;
            obj->h3a_aew_af_graph_param_idx = -1;
        }

        if (obj->ldc_enable)
        {
            /* ldc_out: LDC out0_img @ node index 7 */
            add_graph_parameter_by_node_index(obj->graph, obj->node_ldc, 7);
            graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = obj->num_cap_buf;
            graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&(obj->ldc_out_frames[0]);
            obj->ldc_out_graph_param_idx = graph_parameter_num;
            graph_parameter_num++;
        }
        else
        {
            obj->ldc_out_graph_param_idx = -1;
        }
#endif

        params_list_depth = (vx_uint32)graph_parameter_num;

        if(status == VX_SUCCESS)
        {
            status = tivxSetGraphPipelineDepth(obj->graph, obj->num_cap_buf);
        }

        /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
         * Graph gets scheduled automatically as refs are enqueued to it
         */
        if(status == VX_SUCCESS)
        {
            status = vxSetGraphScheduleConfig(obj->graph,
                            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
                            params_list_depth,
                            graph_parameters_queue_params_list
                            );
        }
        APP_PRINTF("vxSetGraphScheduleConfig done\n");
    }

    if(status == VX_SUCCESS)
    {
        status = vxVerifyGraph(obj->graph);
    }

    if(vx_true_e == obj->scaler_enable)
    {
        tivx_vpac_msc_coefficients_t sc_coeffs;
        #if defined(VPAC3) || defined(VPAC3L)
            tivx_vpac_msc_input_params_t msc_input_params;
        #endif
        vx_reference refs[1];

        printf("Scaler is enabled\n");

        tivx_vpac_msc_coefficients_params_init(&sc_coeffs, VX_INTERPOLATION_BILINEAR);

        obj->sc_coeff_obj = vxCreateUserDataObject(obj->context, "tivx_vpac_msc_coefficients_t", sizeof(tivx_vpac_msc_coefficients_t), NULL);
        if(status == VX_SUCCESS)
        {
            status = vxCopyUserDataObject(obj->sc_coeff_obj, 0, sizeof(tivx_vpac_msc_coefficients_t), &sc_coeffs, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
        refs[0] = (vx_reference)obj->sc_coeff_obj;
        if(status == VX_SUCCESS)
        {
            status = tivxNodeSendCommand(obj->scalerNode, 0u, TIVX_VPAC_MSC_CMD_SET_COEFF, refs, 1u);
        }

        #if defined(VPAC3) || defined(VPAC3L)
            /* Configure MSC input parameters with simultaneous processing enabled */
            if(status == VX_SUCCESS)
            {
                vx_user_data_object msc_input_params_obj;

                tivx_vpac_msc_input_params_init(&msc_input_params);
                msc_input_params.is_enable_simul_processing = 1;

                msc_input_params_obj = vxCreateUserDataObject(obj->context, "tivx_vpac_msc_input_params_t",
                sizeof(tivx_vpac_msc_input_params_t), &msc_input_params);

                refs[0] = (vx_reference)msc_input_params_obj;
                status = tivxNodeSendCommand(obj->scalerNode, 0u, TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS, refs, 1u);

                vxReleaseUserDataObject(&msc_input_params_obj);

                if(status == VX_SUCCESS)
                {
                    printf("MSC simultaneous processing enabled\n");
                }
                else
                {
                    printf("Failed to enable MSC simultaneous processing, status=%d", status);
                }
            }
        #endif
    }
    else
    {
        printf("Scaler is disabled\n");
    }

    if(status == VX_SUCCESS)
    {
        status = tivxExportGraphToDot(obj->graph, ".", "single_cam_graph");
    }

#ifdef _APP_DEBUG_
    if(vx_false_e == yuv_cam_input)
    {
        if( (NULL != obj->fs_test_raw_image) && (NULL != obj->capture_node) && (status == VX_SUCCESS))
        {
            status = app_send_test_frame(obj->capture_node, obj->fs_test_raw_image);
        }
    }
#endif //_APP_DEBUG_

    APP_PRINTF("app_create_graph exiting\n");
    return status;
}

vx_status app_delete_graph(AppObj *obj)
{
    uint32_t buf_id;
    vx_status status = VX_SUCCESS;

    if(NULL != obj->capture_node)
    {
        APP_PRINTF("releasing capture node\n");
        status |= vxReleaseNode(&obj->capture_node);
    }

    if(NULL != obj->node_viss)
    {
        APP_PRINTF("releasing node_viss\n");
        status |= vxReleaseNode(&obj->node_viss);
    }

    if(NULL != obj->node_aewb)
    {
        APP_PRINTF("releasing node_aewb\n");
        status |= vxReleaseNode(&obj->node_aewb);
    }

    if(NULL != obj->displayNode)
    {
        APP_PRINTF("releasing displayNode\n");
        status |= vxReleaseNode(&obj->displayNode);
    }
#ifdef VPAC3
    /* Releasing MV display node */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        if(NULL != obj->displayNode_MV)
        {
            APP_PRINTF("releasing MV displayNode\n");
            status |= vxReleaseNode(&obj->displayNode_MV);
        }
    }
#endif

    if (!obj->file_read_enable)
    {
        status |= tivxReleaseRawImage(&obj->raw);
        APP_PRINTF("releasing raw image done\n");
    }
    else
    {
        for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
        {
            if(NULL != obj->raw_frames[buf_id])
            {
                APP_PRINTF("releasing raw_frame # %d\n", buf_id);
                status |= tivxReleaseRawImage(&(obj->raw_frames[buf_id]));
            }
        }
        obj->raw = NULL;
        APP_PRINTF("releasing raw_frames done\n");
    }

    for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
    {
       if(NULL != obj->cap_frames[buf_id])
       {
        APP_PRINTF("releasing cap_frame # %d\n", buf_id);
        status |= vxReleaseObjectArray(&(obj->cap_frames[buf_id]));
       }
    }

    for(buf_id=0; buf_id<obj->num_viss_out_buf; buf_id++)
    {
      if(NULL != obj->viss_out_luma[buf_id])
      {
        APP_PRINTF("releasing y8 buffer # %d\n", buf_id);
        status |= vxReleaseImage(&(obj->viss_out_luma[buf_id]));
      }
    }

    if(NULL != obj->capt_yuv_image)
    {
        APP_PRINTF("releasing capt_yuv_image\n");
        status |= vxReleaseImage(&obj->capt_yuv_image);
    }


    if(NULL != obj->y12)
    {
        APP_PRINTF("releasing y12\n");
        status |= vxReleaseImage(&obj->y12);
    }

    if(NULL != obj->uv12_c1)
    {
        APP_PRINTF("releasing uv12_c1\n");
        status |= vxReleaseImage(&obj->uv12_c1);
    }

    if(NULL != obj->s8_b8_c4)
    {
        APP_PRINTF("releasing s8_b8_c4\n");
        status |= vxReleaseImage(&obj->s8_b8_c4);
    }

    for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
    {
        if(NULL != obj->y8_r8_c2_frames[buf_id])
        {
            APP_PRINTF("releasing y8_r8_c2_frames[%d]\n", buf_id);
            status |= vxReleaseImage(&(obj->y8_r8_c2_frames[buf_id]));
        }
    }
    obj->y8_r8_c2 = NULL;

    if(NULL != obj->uv8_g8_c3)
    {
        APP_PRINTF("releasing uv8_g8_c3\n");
        status |= vxReleaseImage(&obj->uv8_g8_c3);
    }

    if(NULL != obj->histogram)
    {
        APP_PRINTF("releasing histogram\n");
        status |= vxReleaseDistribution(&obj->histogram);
    }

    if(NULL != obj->configuration)
    {
        APP_PRINTF("releasing configuration\n");
        status |= vxReleaseUserDataObject(&obj->configuration);

    }

    if (NULL != obj->ae_awb_result)
    {
        status |= vxReleaseUserDataObject(&obj->ae_awb_result);
        APP_PRINTF("releasing ae_awb_result done\n");
    }

    for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
    {
        if(NULL != obj->h3a_aew_af_frames[buf_id])
        {
            APP_PRINTF("releasing h3a_aew_af_frames[%d]\n", buf_id);
            status |= vxReleaseUserDataObject(&(obj->h3a_aew_af_frames[buf_id]));
        }
    }
    obj->h3a_aew_af = NULL;

    if(NULL != obj->aewb_config)
    {
        APP_PRINTF("releasing aewb_config\n");
        status |= vxReleaseUserDataObject(&obj->aewb_config);
    }

    if(NULL != obj->dcc_param_viss)
    {
        APP_PRINTF("releasing VISS DCC Data Object\n");
        status |= vxReleaseUserDataObject(&obj->dcc_param_viss);
    }

    if(NULL != obj->display_param_obj)
    {
        APP_PRINTF("releasing Display Param Data Object\n");
        status |= vxReleaseUserDataObject(&obj->display_param_obj);
    }
#ifdef VPAC3
    /* Releasing MV params object */
    if (obj->vpac3_dual_fcp_enable == 1U)
    {
        if(NULL != obj->display_param_MV_obj)
        {
            APP_PRINTF("releasing MV Display Param Data Object\n");
            status |= vxReleaseUserDataObject(&obj->display_param_MV_obj);
        }
    }
#endif

    if(NULL != obj->dcc_param_2a)
    {
        APP_PRINTF("releasing 2A DCC Data Object\n");
        status |= vxReleaseUserDataObject(&obj->dcc_param_2a);
    }

    if(NULL != obj->dcc_param_ldc)
    {
        APP_PRINTF("releasing LDC DCC Data Object\n");
        status |= vxReleaseUserDataObject(&obj->dcc_param_ldc);
    }

    if (obj->ldc_enable)
    {
        if (NULL != obj->mesh_img)
        {
            APP_PRINTF("releasing LDC Mesh Image \n");
            status |= vxReleaseImage(&obj->mesh_img);
        }

        for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
        {
            if (NULL != obj->ldc_out_frames[buf_id])
            {
                APP_PRINTF("releasing LDC Output Image[%d] \n", buf_id);
                status |= vxReleaseImage(&(obj->ldc_out_frames[buf_id]));
            }
        }
        obj->ldc_out = NULL;

        if (NULL != obj->mesh_params_obj)
        {
            APP_PRINTF("releasing LDC Mesh Parameters Object\n");
            status |= vxReleaseUserDataObject(&obj->mesh_params_obj);
        }

        if (NULL != obj->ldc_param_obj)
        {
            APP_PRINTF("releasing LDC Parameters Object\n");
            status |= vxReleaseUserDataObject(&obj->ldc_param_obj);
        }

        if (NULL != obj->region_params_obj)
        {
            APP_PRINTF("releasing LDC Region Parameters Object\n");
            status |= vxReleaseUserDataObject(&obj->region_params_obj);
        }

        if(NULL != obj->node_ldc)
        {
            APP_PRINTF("releasing LDC Node \n");
            status |= vxReleaseNode(&obj->node_ldc);
        }
    }
    if(vx_true_e == obj->scaler_enable)
    {
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        for(buf_id = 0; buf_id < obj->num_cap_buf; buf_id++)
        {
            if(NULL != obj->scaler_out_imgs[buf_id])
            {
                APP_PRINTF("releasing scaler_out_imgs[%u]\n", buf_id);
                status |= vxReleaseImage(&obj->scaler_out_imgs[buf_id]);
                obj->scaler_out_imgs[buf_id] = NULL;
            }
        }
        obj->scaler_out_img = NULL;
#else
        if (NULL != obj->scaler_out_img)
        {
            APP_PRINTF("releasing Scaler Output Image \n");
            status |= vxReleaseImage(&obj->scaler_out_img);
        }
#endif

        if(NULL != obj->scalerNode)
        {
            APP_PRINTF("releasing Scaler Node \n");
            status |= vxReleaseNode(&obj->scalerNode);
        }

        if (NULL != obj->sc_coeff_obj)
        {
            APP_PRINTF("release Scalar coefficient data object \n");
            status |= vxReleaseUserDataObject(&obj->sc_coeff_obj);
        }
    }

#ifdef _APP_DEBUG_
    if(NULL != obj->fs_test_raw_image)
    {
        APP_PRINTF("releasing test raw image buffer # %d\n", buf_id);
        status |= tivxReleaseRawImage(&obj->fs_test_raw_image);
    }
#endif

    APP_PRINTF("releasing graph\n");
    status |= vxReleaseGraph(&obj->graph);
    APP_PRINTF("releasing graph done\n");
    return status;
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

static vx_status app_run_graph_file_read(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    vx_uint32 frm_loop_cnt;
    uint32_t buf_id;
    uint32_t frame_counter = 0;
    uint32_t num_refs;
    tivx_raw_image out_raw_frame;
    char raw_image_fname[APP_MAX_FILE_PATH];
    uint32_t seq_idx;

#if defined(A72) || defined(A53)
#if defined(LINUX)
    appDccUpdatefromFS(obj->sensor_name, obj->sensor_wdr_mode,
                        obj->node_viss, 0,
                        obj->node_aewb, 0,
                        obj->node_ldc, 0,
                        obj->context);
#endif
#endif

    /* Pre-load and enqueue all raw buffers so TIOVX can start pipelining
     * immediately, mirroring the live-capture prefill in app_run_graph(). */
    for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
    {
        seq_idx = obj->file_read_start_seq;
        if (obj->file_read_seq_count > 0)
        {
            seq_idx += (frame_counter % obj->file_read_seq_count);
        }
        snprintf(raw_image_fname, APP_MAX_FILE_PATH, "%s/img_%04d.raw",
                 obj->file_read_dir, seq_idx);

        if (status == VX_SUCCESS)
        {
            if (read_test_image_raw(raw_image_fname, obj->raw_frames[buf_id], 2) < 0)
            {
                printf("Failed to read raw file %s\n", raw_image_fname);
                status = VX_FAILURE;
            }
        }
        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&(obj->raw_frames[buf_id]), 1);
        }
#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&(obj->y8_r8_c2_frames[buf_id]), 1);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&(obj->h3a_aew_af_frames[buf_id]), 1);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&(obj->ldc_out_frames[buf_id]), 1);
        }
#endif
        frame_counter++;
    }

    /* Same convention as app_run_graph(): a few extra iterations are run to
     * drain the AEWB convergence/pipeline latency.                        */
    frm_loop_cnt = obj->num_frames_to_run;
    frm_loop_cnt += obj->num_cap_buf;

    if (obj->is_interactive)
    {
        frm_loop_cnt = 0xFFFFFFFF;
    }

    for(i = 0; i < frm_loop_cnt; i++)
    {
#ifdef _APP_DEBUG_
        vx_image out_y8_r8_c2 = NULL;
        vx_user_data_object out_h3a = NULL;
        vx_image out_ldc = NULL;
        uint32_t num_refs_debug;
#endif
        appPerfPointBegin(&obj->total_perf);

        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, 0, (vx_reference*)&out_raw_frame, 1, &num_refs);
        }

#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&out_y8_r8_c2, 1, &num_refs_debug);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&out_h3a, 1, &num_refs_debug);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&out_ldc, 1, &num_refs_debug);
        }

        if (status == VX_SUCCESS)
        {
            app_update_debug_cache_and_dump(obj, vx_false_e, out_raw_frame, NULL,
                            out_y8_r8_c2, out_h3a, NULL,
                            obj->ldc_enable ? out_ldc : NULL);
        }
#endif

        if (status == VX_SUCCESS)
        {
            seq_idx = obj->file_read_start_seq;
            if (obj->file_read_seq_count > 0)
            {
                seq_idx += (frame_counter % obj->file_read_seq_count);
            }
            snprintf(raw_image_fname, APP_MAX_FILE_PATH, "%s/img_%04d.raw",
                     obj->file_read_dir, seq_idx);

            if (read_test_image_raw(raw_image_fname, out_raw_frame, 2) < 0)
            {
                printf("Failed to read raw file %s\n", raw_image_fname);
                status = VX_FAILURE;
            }
        }
        frame_counter++;

        if (status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&out_raw_frame, 1);
        }

#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&out_y8_r8_c2, 1);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&out_h3a, 1);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&out_ldc, 1);
        }
#endif

        appPerfPointEnd(&obj->total_perf);

        if ((obj->stop_task) || (status != VX_SUCCESS))
        {
            break;
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxWaitGraph(obj->graph);
    }

    return status;
}

vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    vx_uint32 frm_loop_cnt;

    uint32_t buf_id;
    uint32_t num_refs_capture;
    vx_object_array out_capture_frames;
    int graph_parameter_num = 0;

    uint32_t channel_mask = (1<<obj->selectedCam);

    if(NULL == obj->sensor_name)
    {
        printf("sensor name is NULL \n");
        return VX_FAILURE;
    }

    if (obj->file_read_enable)
    {
        return app_run_graph_file_read(obj);
    }

    status = appStartImageSensor(obj->sensor_name, channel_mask);
    if(status < 0)
    {
        printf("Failed to start sensor %s \n", obj->sensor_name);
        if (NULL != obj->fs_test_raw_image)
        {
            printf("Defaulting to file test mode \n");
            status = 0;
        }
    }

    graph_parameter_num = 0;
    for(buf_id=0; buf_id<obj->num_cap_buf; buf_id++)
    {
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&(obj->cap_frames[buf_id]), 1);
        }
        /* in order for the graph to finish execution, the
            display still needs to be enqueued 4 times for testing */
        if((status == VX_SUCCESS) && (obj->test_mode == 1))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&(obj->display_image), 1);
        }
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* Pre-enqueue all scaler output buffers (graph parameter 1) so
         * TIOVX can start pipelining immediately.                       */
        if((status == VX_SUCCESS) && (vx_true_e == obj->scaler_enable))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 1,
                        (vx_reference*)&(obj->scaler_out_imgs[buf_id]), 1);
        }
#endif
#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&(obj->y8_r8_c2_frames[buf_id]), 1);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&(obj->h3a_aew_af_frames[buf_id]), 1);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&(obj->ldc_out_frames[buf_id]), 1);
        }
#endif
    }

    /*
        The application reads and  processes the same image "frm_loop_cnt" times
        The output may change because on VISS, parameters are updated every frame based on AEWB results
        AEWB result is avaialble after 1 frame and is applied after 2 frames
        Therefore, first 2 output images will have wrong colors
    */
    frm_loop_cnt = obj->num_frames_to_run;
    frm_loop_cnt += obj->num_cap_buf;

    if(obj->is_interactive)
    {
        /* in interactive mode loop for ever */
        frm_loop_cnt  = 0xFFFFFFFF;
    }

#if defined(A72) || defined(A53)
#if defined(LINUX)

    appDccUpdatefromFS(obj->sensor_name, obj->sensor_wdr_mode,
                        obj->node_viss, 0,
                        obj->node_aewb, 0,
                        obj->node_ldc, 0,
                        obj->context);
#endif
#endif

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
        printf("ERROR: Unable to create DRM display task\n");
    }
#endif

    for(i=0; i<frm_loop_cnt; i++)
    {
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        vx_image out_scaler_frame = NULL;
        uint32_t num_refs_scaler  = 0;
#else
        vx_image test_image;
#endif
#ifdef _APP_DEBUG_
        vx_image out_y8_r8_c2 = NULL;
        vx_user_data_object out_h3a = NULL;
        vx_image out_ldc = NULL;
        uint32_t num_refs_debug;
#endif
        appPerfPointBegin(&obj->total_perf);
        graph_parameter_num = 0;
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1, &num_refs_capture);
        }
        graph_parameter_num++;
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* Dequeue the completed scaler output for this pipeline iteration.
         * After this call the buffer is guaranteed fully written by TIOVX.*/
        if((status == VX_SUCCESS) && (vx_true_e == obj->scaler_enable))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num,
                        (vx_reference*)&out_scaler_frame, 1, &num_refs_scaler);
            graph_parameter_num++;

            /* Hand the completed frame to the display task and wait until it
             * has finished copying before we re-enqueue the buffer.     */
#if defined(LINUX)
            obj->drm_frame = out_scaler_frame;
#elif defined(QNX)
            obj->screen_frame = out_scaler_frame;
#endif
            sem_post(&obj->disp_frame_ready_sem);
            sem_wait(&obj->disp_frame_done_sem);
        }
#else
        if((status == VX_SUCCESS) && (obj->test_mode == 1))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, 1, (vx_reference*)&test_image, 1, &num_refs_capture);
        }
        if((obj->test_mode == 1) && (i > TEST_BUFFER) && (status == VX_SUCCESS))
        {
            vx_uint32 actual_checksum = 0;
            if(app_test_check_image(test_image, checksums_expected[obj->sensor_sel][0], &actual_checksum) == vx_false_e)
            {
                test_result = vx_false_e;
            }
            populate_gatherer(obj->sensor_sel, 0, actual_checksum);
        }
#endif
#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&out_y8_r8_c2, 1, &num_refs_debug);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&out_h3a, 1, &num_refs_debug);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterDequeueDoneRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&out_ldc, 1, &num_refs_debug);
        }

        if (status == VX_SUCCESS)
        {
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
            vx_image debug_scaler_ref = obj->scaler_enable ? out_scaler_frame : NULL;
#else
            vx_image debug_scaler_ref = NULL;
#endif
            if (NULL == obj->capt_yuv_image)
            {
                tivx_raw_image cap_raw = (tivx_raw_image)vxGetObjectArrayItem(out_capture_frames, 0);
                app_update_debug_cache_and_dump(obj, vx_false_e, cap_raw, NULL,
                                out_y8_r8_c2, out_h3a, debug_scaler_ref,
                                obj->ldc_enable ? out_ldc : NULL);
                tivxReleaseRawImage(&cap_raw);
            }
            else
            {
                vx_image cap_yuv = (vx_image)vxGetObjectArrayItem(out_capture_frames, 0);
                app_update_debug_cache_and_dump(obj, vx_true_e, NULL, cap_yuv,
                                NULL, NULL, debug_scaler_ref,
                                obj->ldc_enable ? out_ldc : NULL);
                vxReleaseImage(&cap_yuv);
            }
        }
#endif
        APP_PRINTF(" i %d...\n", i);
        graph_parameter_num = 0;
#if !(defined(SOC_AM62A) && (defined(LINUX) || defined(QNX)))
        if((status == VX_SUCCESS) && (obj->test_mode == 1))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&test_image, 1);
        }
#endif
        if(status == VX_SUCCESS)
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1);
        }
        graph_parameter_num++;
#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
        /* Re-enqueue scaler buffer only after display task has finished
         * reading it (sem_wait above returned).
         * Scaler output is always graph parameter index 1.              */
        if((status == VX_SUCCESS) && (vx_true_e == obj->scaler_enable) && (NULL != out_scaler_frame))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, 1,
                        (vx_reference*)&out_scaler_frame, 1);
        }
#endif
#ifdef _APP_DEBUG_
        if((status == VX_SUCCESS) && (obj->y8_r8_c2_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->y8_r8_c2_graph_param_idx,
                        (vx_reference*)&out_y8_r8_c2, 1);
        }
        if((status == VX_SUCCESS) && (obj->h3a_aew_af_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->h3a_aew_af_graph_param_idx,
                        (vx_reference*)&out_h3a, 1);
        }
        if((status == VX_SUCCESS) && (obj->ldc_out_graph_param_idx >= 0))
        {
            status = vxGraphParameterEnqueueReadyRef(obj->graph, obj->ldc_out_graph_param_idx,
                        (vx_reference*)&out_ldc, 1);
        }
#endif
        appPerfPointEnd(&obj->total_perf);

        if((obj->stop_task) || (status != VX_SUCCESS))
        {
            break;
        }
    }

#if defined(SOC_AM62A) && (defined(LINUX) || defined(QNX))
    /* Ensure the display task is not left blocking on disp_frame_done_sem
     * after the graph loop exits (e.g. on 'x' keypress).              */
    if(vx_true_e == obj->scaler_enable)
    {
        sem_post(&obj->disp_frame_ready_sem);
    }
#endif

    if(status == VX_SUCCESS)
    {
        status = vxWaitGraph(obj->graph);
    }
/* Dequeue buf for pipe down */
#if 0
    for(buf_id=0; buf_id<obj->num_cap_buf-2; buf_id++)
    {
        APP_PRINTF(" Dequeuing capture # %d...\n", buf_id);
        graph_parameter_num = 0;
        vxGraphParameterDequeueDoneRef(obj->graph, graph_parameter_num, (vx_reference*)&out_capture_frames, 1, &num_refs_capture);
        graph_parameter_num++;
    }
#endif
    if(status == VX_SUCCESS)
    {
        status = appStopImageSensor(obj->sensor_name, channel_mask);
    }
    return status;
}

static void app_run_task(void *app_var)
{
    AppObj *obj = (AppObj *)app_var;

    appPerfStatsCpuLoadResetAll();

    app_run_graph(obj);

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
    "\n Demo : Single Camera w/ 2A"
    "\n =========================="
    "\n"
    "\n p: Print performance statistics"
    "\n"
    "\n e: Export performance statistics"
#if defined(A72) || defined(A53)
#if defined(LINUX)
    "\n"
    "\n u: Update DCC from File System"
    "\n"
    "\n"
#endif
#endif
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};

static vx_status app_run_graph_interactive(AppObj *obj)
{
    vx_status status;
    uint32_t done = 0;
    char ch;
    FILE *fp;
    app_perf_point_t *perf_arr[1];
    uint32_t channel_mask = (1<<obj->selectedCam);

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
                    fp = appPerfStatsExportOpenFile(".", "basic_demos_app_single_cam");
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
#if defined(A72) || defined(A53)
#if defined(LINUX)
                case 'u':
                    appDccUpdatefromFS(obj->sensor_name, obj->sensor_wdr_mode,
                        obj->node_viss, 0,
                        obj->node_aewb, 0,
                        obj->node_ldc, 0,
                        obj->context);
                    break;
#endif
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

                default:
                    printf("Unsupported command %c\n", ch);
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
    if(status == VX_SUCCESS)
    {
        if (!obj->file_read_enable)
        {
            status = appStopImageSensor(obj->sensor_name, channel_mask);
        }
    }
    return status;
}

static void app_show_usage(int argc, char* argv[])
{
    printf("\n");
    printf(" Single Camera Demo - (c) Texas Instruments 2019\n");
    printf(" ========================================================\n");
    printf("\n");
    printf(" Usage,\n");
    printf("  %s --cfg <config file>\n", argv[0]);
    printf("\n");
}

#if defined(A72) || defined(A53)
#if defined(LINUX)
int appSingleCamUpdateVpacDcc(AppObj *obj, uint8_t* dcc_buf, uint32_t dcc_buf_size)
{
    int32_t status = 0;

    status = appUpdateVpacDcc(dcc_buf, dcc_buf_size, obj->context,
                obj->node_viss, 0,
                obj->node_aewb, 0,
                obj->node_ldc, 0
             );

    return status;
}
#endif
#endif

#ifdef _APP_DEBUG_
/* Writes the RAW/VISS/H3A/LDC/scaler debug images using the specific
 * buffer refs passed in -- never a persistent obj->raw/obj->y8_r8_c2/etc
 * pointer -- so the caller must supply refs that TIOVX has confirmed
 * complete (e.g. this iteration's dequeue result, or the mutex-guarded
 * "last completed frame" cache). Shared by the continuous per-frame
 * dump (app_update_debug_cache_and_dump()) and the on-demand ITT
 * snapshot (save_debug_images()). Any ref that doesn't apply to the
 * current pipeline configuration (e.g. ldc_out_ref when LDC is
 * disabled) may be passed as NULL and is skipped.                      */
static int app_write_debug_images(AppObj *obj,
                                   vx_bool is_yuv_cam_input,
                                   tivx_raw_image raw_ref,
                                   vx_image cap_yuv_ref,
                                   vx_image y8_r8_c2_ref,
                                   vx_user_data_object h3a_ref,
                                   vx_image scaler_out_ref,
                                   vx_image ldc_out_ref)
{
    int num_bytes_io = 0;
    int file_index = obj->debug_save_file_index;
    char raw_image_fname[MAX_FNAME];
    char yuv_image_fname[MAX_FNAME];
    char h3a_image_fname[MAX_FNAME];
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = app_get_test_file_path();

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

    if(vx_false_e == is_yuv_cam_input)
    {
        snprintf(raw_image_fname, MAX_FNAME, "%s/%s_%04d.raw", test_data_path, "img", file_index);
        printf("RAW file name %s \n", raw_image_fname);
        num_bytes_io = write_output_image_raw(raw_image_fname, raw_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to RAW file \n");
            return VX_FAILURE;
        }

        snprintf(yuv_image_fname, MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "img_viss", file_index);
        printf("YUV file name %s \n", yuv_image_fname);
        num_bytes_io = write_output_image_nv12_8bit(yuv_image_fname, y8_r8_c2_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to VISS NV12 file \n");
            return VX_FAILURE;
        }

        snprintf(h3a_image_fname, MAX_FNAME, "%s/%s_%04d.bin", test_data_path, "h3a", file_index);
        printf("H3A file name %s \n", h3a_image_fname);
        num_bytes_io = write_h3a_image(h3a_image_fname, h3a_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to H3A file \n");
            return VX_FAILURE;
        }

    }
    else
    {
        snprintf(raw_image_fname, MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "cap", file_index);
        printf("YUV file name %s \n", raw_image_fname);
        num_bytes_io = write_output_image_yuv422_8bit(raw_image_fname, cap_yuv_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to YUV file \n");
            return VX_FAILURE;
        }
    }

    if(NULL != scaler_out_ref)
    {
        snprintf(yuv_image_fname, MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "img_msc", file_index);
        printf("YUV file name %s \n", yuv_image_fname);
        num_bytes_io = write_output_image_nv12_8bit(yuv_image_fname, scaler_out_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to MSC NV12 file \n");
            return VX_FAILURE;
        }
    }

    if(NULL != ldc_out_ref)
    {
        snprintf(yuv_image_fname, MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "img_ldc", file_index);
        printf("YUV file name %s \n", yuv_image_fname);
        num_bytes_io = write_output_image_nv12_8bit(yuv_image_fname, ldc_out_ref);
        if(num_bytes_io < 0)
        {
            printf("Error writing to LDC NV12 file \n");
            return VX_FAILURE;
        }
    }

    obj->debug_save_file_index = file_index + 1;
    return file_index;
}

/* Called once per iteration from app_run_graph()/app_run_graph_file_read(),
 * on the graph-execution thread, immediately after this iteration's
 * relevant dequeues complete and before re-enqueuing. Publishes the
 * just-dequeued buffer refs into the mutex-guarded "last completed
 * frame" cache (for save_debug_images()'s later on-demand use from the
 * ITT thread), and, if save_debug_images_enable is set, writes the
 * debug images immediately using these same just-dequeued refs.        */
static void app_update_debug_cache_and_dump(AppObj *obj,
                                             vx_bool is_yuv_cam_input,
                                             tivx_raw_image raw_ref,
                                             vx_image cap_yuv_ref,
                                             vx_image y8_r8_c2_ref,
                                             vx_user_data_object h3a_ref,
                                             vx_image scaler_out_ref,
                                             vx_image ldc_out_ref)
{
    tivxMutexLock(obj->debug_last_frame_mutex);

    obj->debug_last_raw = raw_ref;
    obj->debug_last_cap_yuv = cap_yuv_ref;
    obj->debug_last_y8_r8_c2 = y8_r8_c2_ref;
    obj->debug_last_h3a = h3a_ref;
    obj->debug_last_scaler_out = scaler_out_ref;
    obj->debug_last_ldc_out = ldc_out_ref;
    obj->debug_last_frame_valid = vx_true_e;

    if (obj->save_debug_images_enable)
    {
        (void)app_write_debug_images(obj, is_yuv_cam_input, raw_ref, cap_yuv_ref,
                        y8_r8_c2_ref, h3a_ref, scaler_out_ref, ldc_out_ref);
    }

    tivxMutexUnlock(obj->debug_last_frame_mutex);
}

/* On-demand snapshot for the ITT server callback (iss_raw_save /
 * iss_yuv_save). Reads the "last completed frame" cache that the
 * graph-execution thread publishes once per iteration, under
 * debug_last_frame_mutex, instead of touching obj->raw/obj->y8_r8_c2/
 * obj->h3a_aew_af/obj->ldc_out/obj->cap_frames[0]/obj->scaler_out_img
 * directly -- those are racy from any thread other than the graph
 * thread itself (see app_update_debug_cache_and_dump()).                */
int save_debug_images(AppObj *obj)
{
    int result;

    tivxMutexLock(obj->debug_last_frame_mutex);

    if (vx_false_e == obj->debug_last_frame_valid)
    {
        tivxMutexUnlock(obj->debug_last_frame_mutex);
        printf("save_debug_images: no completed frame available yet \n");
        return VX_FAILURE;
    }

    result = app_write_debug_images(obj,
                    (NULL != obj->capt_yuv_image) ? vx_true_e : vx_false_e,
                    obj->debug_last_raw,
                    obj->debug_last_cap_yuv,
                    obj->debug_last_y8_r8_c2,
                    obj->debug_last_h3a,
                    obj->scaler_enable ? obj->debug_last_scaler_out : NULL,
                    obj->ldc_enable ? obj->debug_last_ldc_out : NULL);

    tivxMutexUnlock(obj->debug_last_frame_mutex);

    return result;
}
#endif //_APP_DEBUG_

static void app_parse_cfg_file(AppObj *obj, char *cfg_file_name)
{
    FILE *fp = fopen(cfg_file_name, "r");
    char line_str[1024];
    char *token;

    if(fp==NULL)
    {
        printf("# ERROR: Unable to open config file [%s]. Switching to interactive mode\n", cfg_file_name);
        obj->is_interactive = 1;
    }
    else
    {
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
                        printf("sensor_selection = [%d]\n", obj->sensor_sel);
                    }
                }
                else
                if(strcmp(token, "ldc_enable")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->ldc_enable = atoi(token);
                        printf("ldc_enable = [%d]\n", obj->ldc_enable);
                    }
                }
                else
                if(strcmp(token, "num_frames_to_run")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->num_frames_to_run = atoi(token);
                        printf("num_frames_to_run = [%d]\n", obj->num_frames_to_run);
                    }
                }
                else
                if(strcmp(token, "is_interactive")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->is_interactive = atoi(token);
                        printf("is_interactive = [%d]\n", obj->is_interactive);
                    }
                }
                else
                if(strcmp(token, "file_read_enable")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->file_read_enable = atoi(token);
                        printf("file_read_enable = [%d]\n", obj->file_read_enable);
                    }
                }
                else
                #ifdef SOC_FAMILY_TDA5
                if(strcmp(token, "file_read_dir_tda5")==0)
                #else
                if(strcmp(token, "file_read_dir_tda4")==0)
                #endif
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        size_t len = strlen(token);
                        if (len > 0 && (token[len-1] == '\n' || token[len-1] == '\r'))
                        {
                            token[len-1] = '\0';
                        }
                        strncpy(obj->file_read_dir, token, APP_MAX_FILE_PATH-1);
                        obj->file_read_dir[APP_MAX_FILE_PATH-1] = '\0';
                        printf("file_read_dir = [%s]\n", obj->file_read_dir);
                    }
                }
                else
                if(strcmp(token, "file_read_seq_count")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->file_read_seq_count = atoi(token);
                        printf("file_read_seq_count = [%d]\n", obj->file_read_seq_count);
                    }
                }
                else
                if(strcmp(token, "file_read_start_seq")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->file_read_start_seq = atoi(token);
                        printf("file_read_start_seq = [%d]\n", obj->file_read_start_seq);
                    }
                }
                else
                if(strcmp(token, "save_debug_images_enable")==0)
                {
                    token = strtok(NULL, s);
                    if (NULL != token)
                    {
                        obj->save_debug_images_enable = atoi(token);
                        printf("save_debug_images_enable = [%d]\n", obj->save_debug_images_enable);
                    }
                }
                else
                {
                    APP_PRINTF("Invalid token [%s]\n", token);
                }
            }
        }

        fclose(fp);
    }

    if(obj->width_in<128)
        obj->width_in = 128;
    if(obj->height_in<128)
        obj->height_in = 128;
    if(obj->width_out<128)
        obj->width_out = 128;
    if(obj->height_out<128)
        obj->height_out = 128;

}

vx_status app_parse_cmd_line_args(AppObj *obj, int argc, char *argv[])
{
    vx_bool set_test_mode = vx_false_e;
    uint8_t sensor_override = 0xFF;
    app_set_cfg_default(obj);

    int i;
    if(argc==1)
    {
        app_show_usage(argc, argv);
        printf("Defaulting to interactive mode \n");
        obj->is_interactive = 1;
        return VX_SUCCESS;
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
            return VX_FAILURE;
        }
        else
        if(strcmp(argv[i], "--test")==0)
        {
            set_test_mode = vx_true_e;
        }
        else
        if(strcmp(argv[i], "--sensor")==0)
        {
            // check to see if there is another argument following --sensor
            if (argc > i+1)
            {
                sensor_override = atoi(argv[i+1]);
                // increment i again to avoid this arg
                i++;
            }
        }
    }

    if(set_test_mode == vx_true_e)
    {
        obj->test_mode = 1;
        obj->is_interactive = 0;
        obj->num_frames_to_run = NUM_FRAMES;

        /* default to first sensor in sensor enumaration list */
        obj->sensor_sel = 0;

        if (sensor_override != 0xFF)
        {
            obj->sensor_sel = sensor_override;
        }
    }
    return VX_SUCCESS;
}



#ifdef _APP_DEBUG_
vx_int32 write_output_image_nv12(char * file_name, vx_image out_nv12)
{
    FILE * fp = fopen(file_name, "wb");
    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out_nv12);
    fclose(fp);
    APP_PRINTF("%d bytes written to %s\n", len1, file_name);
    return len1;
}
#endif


AppObj gAppObj;

int app_single_cam_main(int argc, char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_FAILURE;
#if defined (LDRA_COVERAGE)
    imaging_vpac_coverage_start();
#endif
    status = app_parse_cmd_line_args(obj, argc, argv);
    if(VX_SUCCESS == status)
    {
        status = app_init(obj);
        if(VX_SUCCESS == status)
        {
            APP_PRINTF("app_init done\n");
            /* Not checking status because application may be waiting for
                error/test frame */
            app_create_graph(obj);
            if(VX_SUCCESS == status)
            {
                APP_PRINTF("app_create_graph done\n");
                if(obj->is_interactive)
                {
                    status = app_run_graph_interactive(obj);
                }
                else
                {
                    status = app_run_graph(obj);
                }
                if(VX_SUCCESS == status)
                {
                    APP_PRINTF("app_run_graph done\n");
                    status = app_delete_graph(obj);
                    if(VX_SUCCESS == status)
                    {
                        APP_PRINTF("app_delete_graph done\n");
                    }
                    else
                    {
                        printf("Error : app_delete_graph returned 0x%x \n", status);
                    }
                }
                else
                {
                    printf("Error : app_run_graph_xx returned 0x%x \n", status);
                }
            }
            else
            {
                printf("Error : app_create_graph returned 0x%x is_interactive =%d  \n", status, obj->is_interactive);
            }
        }
        else
        {
            printf("Error : app_init returned 0x%x \n", status);
        }
        status = app_deinit(obj);
        if(VX_SUCCESS == status)
        {
            APP_PRINTF("app_deinit done\n");
        }
        else
        {
            printf("Error : app_deinit returned 0x%x \n", status);
        }
        appDeInitImageSensor(obj->sensor_name);
    }
    else
    {
        printf("Error: app_parse_cmd_line_args returned 0x%x \n", status);
    }
    if(obj->test_mode == 1)
    {
        if((test_result == vx_false_e) || (status == VX_FAILURE))
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
#if defined (LDRA_COVERAGE)
    imaging_vpac_coverage_end();
#endif
    return status;
}

vx_status app_send_test_frame(vx_node cap_node, tivx_raw_image raw_img)
{
    vx_status status = VX_SUCCESS;

    status = tivxCaptureRegisterErrorFrame(cap_node, (vx_reference)raw_img);

    return status;
}
