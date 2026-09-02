/*
 *  Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
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

/**
 * \file main.c
 *
 * \brief DRM Wrapper Test Application
 *
 * This application tests the DRM wrapper functionality by:
 * 1. Creating a TIOVX NV12 image with a color bar test pattern
 * 2. Displaying it using the drm_wrapper APIs
 * 3. Animating the pattern to verify continuous rendering
 *
 * This allows validation of the DRM display path on AM62A Linux
 * without requiring the CSIRX camera driver.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <utils/drm_wrapper/include/drm_wrapper.h>
#include <app_init.h>
#include <utils/perf_stats/include/app_perf_stats.h>

#define APP_DRM_TEST_WIDTH      (1920u)
#define APP_DRM_TEST_HEIGHT     (1080u)
#define APP_DRM_TEST_NUM_BUFS   (2u)

#define APP_DRM_TEST_LOG(fmt, ...) \
    printf("[DRM_TEST] " fmt "\n", ##__VA_ARGS__)

#define APP_DRM_TEST_ERROR(fmt, ...) \
    printf("[DRM_TEST][ERROR] " fmt "\n", ##__VA_ARGS__)

/** \brief Application context structure */
typedef struct {
    vx_context context;
    vx_image   images[APP_DRM_TEST_NUM_BUFS];
    app_drm_wrapper_handle_t *drm_handle;
    volatile int stop_flag;
} AppDrmTestObj;

static AppDrmTestObj g_app_obj;

/**
 * \brief Signal handler for graceful exit
 */
static void app_signal_handler(int sig)
{
    APP_DRM_TEST_LOG("Received signal %d, exiting...", sig);
    g_app_obj.stop_flag = 1;
}

/**
 * \brief Fill NV12 image with color bar test pattern
 *
 * Creates 8 vertical color bars: White, Yellow, Cyan, Green, Magenta, Red, Blue, Black
 *
 * \param image     TIOVX image to fill
 * \param frame_num Frame number for animation offset
 */
static vx_status app_fill_test_pattern(vx_image image, uint32_t frame_num)
{
    vx_status status = VX_SUCCESS;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t y_addr, uv_addr;
    vx_map_id y_map_id, uv_map_id;
    uint8_t *y_ptr = NULL;
    uint8_t *uv_ptr = NULL;
    vx_uint32 width, height;
    uint32_t x, y;

    /* YUV values for 8 color bars (Y, U, V) */
    static const uint8_t color_bars[8][3] = {
        {235, 128, 128},  /* White */
        {210, 16,  146},  /* Yellow */
        {170, 166, 16},   /* Cyan */
        {145, 54,  34},   /* Green */
        {107, 202, 222},  /* Magenta */
        {82,  90,  240},  /* Red */
        {41,  240, 110},  /* Blue */
        {16,  128, 128},  /* Black */
    };

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    /* Map Y plane */
    status = vxMapImagePatch(image, &rect, 0, &y_map_id, &y_addr, (void **)&y_ptr,
                             VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    if (status != VX_SUCCESS || y_ptr == NULL)
    {
        APP_DRM_TEST_ERROR("Failed to map Y plane");
        return VX_FAILURE;
    }

    /* Map UV plane */
    status = vxMapImagePatch(image, &rect, 1, &uv_map_id, &uv_addr, (void **)&uv_ptr,
                             VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    if (status != VX_SUCCESS || uv_ptr == NULL)
    {
        APP_DRM_TEST_ERROR("Failed to map UV plane");
        vxUnmapImagePatch(image, y_map_id);
        return VX_FAILURE;
    }

    /* Animation offset - shift pattern horizontally */
    uint32_t offset = (frame_num * 4) % width;

    /* Fill Y plane */
    for (y = 0; y < height; y++)
    {
        uint8_t *y_row = y_ptr + (y * y_addr.stride_y);
        for (x = 0; x < width; x++)
        {
            uint32_t bar_idx = ((x + offset) * 8 / width) % 8;
            y_row[x] = color_bars[bar_idx][0];
        }
    }

    /* Fill UV plane (interleaved, half height) */
    for (y = 0; y < height / 2; y++)
    {
        uint8_t *uv_row = uv_ptr + (y * uv_addr.stride_y);
        for (x = 0; x < width / 2; x++)
        {
            uint32_t bar_idx = (((x * 2) + offset) * 8 / width) % 8;
            uv_row[x * 2 + 0] = color_bars[bar_idx][1]; /* U */
            uv_row[x * 2 + 1] = color_bars[bar_idx][2]; /* V */
        }
    }

    vxUnmapImagePatch(image, uv_map_id);
    vxUnmapImagePatch(image, y_map_id);

    return VX_SUCCESS;
}

/**
 * \brief Initialize the application
 */
static vx_status app_init(AppDrmTestObj *obj)
{
    vx_status status = VX_SUCCESS;
    app_drm_wrapper_cfg_t drm_cfg;
    uint32_t i;

    APP_DRM_TEST_LOG("Initializing application...");

    memset(obj, 0, sizeof(AppDrmTestObj));
    obj->stop_flag = 0;

    /* Initialize app_init framework */
    status = appInit();
    if (status != 0)
    {
        APP_DRM_TEST_ERROR("appInit failed");
        return VX_FAILURE;
    }

    /* Create OpenVX context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference)obj->context);
    if (status != VX_SUCCESS)
    {
        APP_DRM_TEST_ERROR("Failed to create OpenVX context");
        return status;
    }

    /* Create NV12 images */
    for (i = 0; i < APP_DRM_TEST_NUM_BUFS; i++)
    {
        obj->images[i] = vxCreateImage(obj->context,
                                       APP_DRM_TEST_WIDTH,
                                       APP_DRM_TEST_HEIGHT,
                                       VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)obj->images[i]);
        if (status != VX_SUCCESS)
        {
            APP_DRM_TEST_ERROR("Failed to create image %u", i);
            return status;
        }
    }

    /* Fill first image with test pattern */
    status = app_fill_test_pattern(obj->images[0], 0);
    if (status != VX_SUCCESS)
    {
        APP_DRM_TEST_ERROR("Failed to fill test pattern");
        return status;
    }

    /* Initialize DRM wrapper */
    appDrmWrapperInitCfg(&drm_cfg);
    drm_cfg.width = APP_DRM_TEST_WIDTH;
    drm_cfg.height = APP_DRM_TEST_HEIGHT;
    drm_cfg.pix_format = 0x3231564E; /* DRM_FORMAT_NV12 */
    drm_cfg.bufq_depth = APP_DRM_TEST_NUM_BUFS;

    obj->drm_handle = appDrmWrapperCreate(&drm_cfg);
    if (obj->drm_handle == NULL)
    {
        APP_DRM_TEST_ERROR("Failed to create DRM wrapper");
        return VX_FAILURE;
    }

    /* Register buffers with DRM */
    for (i = 0; i < APP_DRM_TEST_NUM_BUFS; i++)
    {
        status = appDrmWrapperRegisterBuffer(obj->drm_handle, obj->images[i], i);
        if (status < 0)
        {
            APP_DRM_TEST_ERROR("Failed to register buffer %u", i);
            return VX_FAILURE;
        }
    }

    APP_DRM_TEST_LOG("Initialization complete: %ux%u, %u buffers",
                     APP_DRM_TEST_WIDTH, APP_DRM_TEST_HEIGHT, APP_DRM_TEST_NUM_BUFS);

    return VX_SUCCESS;
}

/**
 * \brief Run the display loop
 */
static vx_status app_run(AppDrmTestObj *obj)
{
    vx_status status = VX_SUCCESS;
    uint32_t frame_count = 0;
    uint32_t buf_idx = 0;
    int32_t drm_status;

    APP_DRM_TEST_LOG("Starting display loop (press Ctrl+C to exit)...");

    while (!obj->stop_flag)
    {
        /* Update test pattern for animation */
        status = app_fill_test_pattern(obj->images[buf_idx], frame_count);
        if (status != VX_SUCCESS)
        {
            APP_DRM_TEST_ERROR("Failed to update test pattern");
            break;
        }

        /* Render frame */
        drm_status = appDrmWrapperRender(obj->drm_handle, buf_idx);
        if (drm_status < 0)
        {
            APP_DRM_TEST_ERROR("Render failed");
            status = VX_FAILURE;
            break;
        }

        frame_count++;

        /* Toggle buffer for double buffering */
        buf_idx = (buf_idx + 1) % APP_DRM_TEST_NUM_BUFS;

        /* Print FPS every 100 frames */
        if ((frame_count % 100) == 0)
        {
            APP_DRM_TEST_LOG("Rendered %u frames", frame_count);
        }

        /* Small delay to control frame rate (~30 fps) */
        usleep(33000);
    }

    APP_DRM_TEST_LOG("Display loop ended after %u frames", frame_count);

    return status;
}

/**
 * \brief Cleanup and deinitialize the application
 */
static void app_deinit(AppDrmTestObj *obj)
{
    uint32_t i;

    APP_DRM_TEST_LOG("Cleaning up...");

    /* Delete DRM wrapper */
    if (obj->drm_handle != NULL)
    {
        appDrmWrapperDelete(obj->drm_handle);
        obj->drm_handle = NULL;
    }

    /* Release images */
    for (i = 0; i < APP_DRM_TEST_NUM_BUFS; i++)
    {
        if (obj->images[i] != NULL)
        {
            vxReleaseImage(&obj->images[i]);
        }
    }

    /* Release context */
    if (obj->context != NULL)
    {
        vxReleaseContext(&obj->context);
    }

    /* Deinitialize app_init framework */
    appDeInit();

    APP_DRM_TEST_LOG("Cleanup complete");
}

/**
 * \brief Print usage information
 */
static void app_print_usage(const char *prog_name)
{
    printf("\n");
    printf("DRM Wrapper Test Application\n");
    printf("============================\n");
    printf("\n");
    printf("Usage: %s [options]\n", prog_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help    Show this help message\n");
    printf("\n");
    printf("This application displays an animated color bar test pattern\n");
    printf("using the DRM/KMS display framework with zero-copy DMA-BUF.\n");
    printf("\n");
    printf("Press Ctrl+C to exit.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    vx_status status;

    /* Parse arguments */
    if (argc > 1)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            app_print_usage(argv[0]);
            return 0;
        }
    }

    printf("\n");
    printf("=========================================\n");
    printf("  DRM Wrapper Test Application\n");
    printf("  AM62A Linux - Zero-Copy Display Test\n");
    printf("=========================================\n");
    printf("\n");

    /* Setup signal handlers */
    signal(SIGINT, app_signal_handler);
    signal(SIGTERM, app_signal_handler);

    /* Initialize */
    status = app_init(&g_app_obj);
    if (status != VX_SUCCESS)
    {
        APP_DRM_TEST_ERROR("Initialization failed");
        app_deinit(&g_app_obj);
        return -1;
    }

    /* Run display loop */
    status = app_run(&g_app_obj);

    /* Cleanup */
    app_deinit(&g_app_obj);

    if (status == VX_SUCCESS)
    {
        APP_DRM_TEST_LOG("Test completed successfully");
        return 0;
    }
    else
    {
        APP_DRM_TEST_ERROR("Test failed");
        return -1;
    }
}
