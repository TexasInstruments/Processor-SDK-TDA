/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include <unistd.h>
#include "TI/tivx.h"
#include "TI/tivx_srv.h"
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_gl_srv.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <app_gl_egl_utils.h>
#include "utils/perf_stats/include/app_perf_stats.h"
#include <utils/mem/include/app_mem.h>
#include <render.h>
#include <car.h>
#if defined(x86_64)
#include <ti/vxlib/vxlib.h>
#endif

#define MAX_CAMERAS (4)

typedef struct
{
    void *eglWindowObj;
    /**< EGL object information */

    render_state_t render3DSRVObj;
    /**< 3D SRV rendering prgram obj */

    #if defined(x86_64)
    uint8_t *intermediate_addr[TIVX_OBJECT_ARRAY_MAX_ITEMS];

    uint32_t intermediate_addr_size;
    #endif
} tivxGlSrvParams;

static tivx_target_kernel vx_gl_srv_target_kernel = NULL;

static vx_status VX_CALLBACK tivxGlSrvProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGlSrvCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxGlSrvDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxOpenglGlSrvEglInfo(
       tivxGlSrvParams *glSrvParams,
       VXLIB_bufParams2D_t input_dim,
       VXLIB_bufParams2D_t output_dim,
       tivx_srv_params_t *params,
       srv_coords_t *srv_views[],
       uint32_t num_srv_views);

static vx_status tivxOpenglGlSrvEglInfo(
       tivxGlSrvParams *glSrvParams,
       VXLIB_bufParams2D_t input_dim,
       VXLIB_bufParams2D_t output_dim,
       tivx_srv_params_t *params,
       srv_coords_t *srv_views[],
       uint32_t num_srv_views)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    glSrvParams->eglWindowObj = appEglWindowOpen();

    if (NULL != glSrvParams->eglWindowObj)
    {
        glSrvParams->render3DSRVObj.screen_width = output_dim.dim_x;
        glSrvParams->render3DSRVObj.screen_height = output_dim.dim_y;
        glSrvParams->render3DSRVObj.cam_width = input_dim.dim_x;
        glSrvParams->render3DSRVObj.cam_height = input_dim.dim_y;
        glSrvParams->render3DSRVObj.blendLUT3D = NULL;
        glSrvParams->render3DSRVObj.cam_bpp = params->cam_bpp;
        for (i = 0; i < num_srv_views; i++)
        {
            glSrvParams->render3DSRVObj.srv_views[i] = *srv_views[i];
        }

        glSrvParams->render3DSRVObj.num_srv_views = num_srv_views;
        status = render_setup(&glSrvParams->render3DSRVObj);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "GL SRV: ERROR: Couldn't open egl window! \r\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxGlSrvProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivx_obj_desc_object_array_t *input_desc;
    tivx_obj_desc_image_t *img_input_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    vx_uint32 i, j;
    tivx_obj_desc_object_array_t *srv_views_desc;
    tivx_obj_desc_user_data_object_t *user_object_srv_views_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_array_t *galign_lut_desc;
    tivx_obj_desc_image_t *output_desc;
    tivxGlSrvParams *glSrvParams = NULL;
    vx_uint32 size  = 0;
    uint64_t                cur_time;

    if ( (num_params != TIVX_KERNEL_GL_SRV_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX];
        srv_views_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_SRV_VIEWS_IDX];
        galign_lut_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_GALIGN_LUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GL_SRV_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void *)&glSrvParams,
                                                    &size);
    }

    if(VX_SUCCESS == status)
    {
        void *input_target_ptr[TIVX_OBJECT_ARRAY_MAX_ITEMS][TIVX_IMAGE_MAX_PLANES];
        void *output_target_ptr;
        void *galign_target_ptr;
        uint8_t *input_addr[TIVX_OBJECT_ARRAY_MAX_ITEMS][TIVX_IMAGE_MAX_PLANES];
        void *srv_views_target_ptr[TIVX_OBJECT_ARRAY_MAX_ITEMS];
        srv_coords_t *srv_views[TIVX_OBJECT_ARRAY_MAX_ITEMS] = {NULL};
        uint32_t num_srv_views = 0;

        tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_items);

        for(i=0; i<input_desc->num_items; i++)
        {
            for(j=0; j < img_input_desc[i]->planes; j++)
            {
                input_target_ptr[i][j] = tivxMemShared2TargetPtr(&img_input_desc[i]->mem_ptr[j]);
                tivxSetPointerLocation(img_input_desc[i], &input_target_ptr[i][j], &input_addr[i][j]);
            }
        }

        if( srv_views_desc != NULL)
        {
            num_srv_views = srv_views_desc->num_items;
            tivxGetObjDescList(srv_views_desc->obj_desc_id, (tivx_obj_desc_t**)user_object_srv_views_desc, srv_views_desc->num_items);
            for(i=0; i<srv_views_desc->num_items; i++)
            {
                srv_views_target_ptr[i] = tivxMemShared2TargetPtr(&user_object_srv_views_desc[i]->mem_ptr);
                srv_views[i] = (srv_coords_t *)srv_views_target_ptr[i];
            }
            glSrvParams->render3DSRVObj.num_srv_views = num_srv_views;
            for(i=0; i<srv_views_desc->num_items; i++)
            {
                glSrvParams->render3DSRVObj.srv_views[i] = *srv_views[i];
            }
        }

        if( galign_lut_desc != NULL)
        {
            galign_target_ptr = tivxMemShared2TargetPtr(&galign_lut_desc->mem_ptr);
            glSrvParams->render3DSRVObj.LUT3D = (void *)galign_target_ptr;
        }

        output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);

        {
            VXLIB_bufParams2D_t input_dim[TIVX_OBJECT_ARRAY_MAX_ITEMS], output_dim;
            uint8_t *output_addr = NULL;

            /* All buf params are the same, so only using the first */
            tivxInitBufParams(img_input_desc[0], (VXLIB_bufParams2D_t*)&input_dim);

            tivxInitBufParams(output_desc, &output_dim);
            tivxSetPointerLocation(output_desc, &output_target_ptr, &output_addr);

            #if defined(x86_64)
            VXLIB_bufParams2D_t intermediate_dim;

            intermediate_dim.dim_x     = input_dim[0].dim_x;
            intermediate_dim.dim_y     = input_dim[0].dim_y;
            intermediate_dim.stride_y  = 3*input_dim[0].dim_x;
            intermediate_dim.data_type = VXLIB_UINT24;
            /* convert to RGB for PC emulation mode*/
            for(i=0; i<input_desc->num_items; i++)
            {
                status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)input_addr[i][0], &input_dim[0],
                    (uint8_t *)input_addr[i][1], &input_dim[1], (uint8_t *)glSrvParams->intermediate_addr[i], &intermediate_dim, 0,
                    img_input_desc[0]->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
            }

            #endif
            GLuint texYuv[8] = {0};
            app_egl_tex_prop_t texProp[4], renderTexProp;
            void *pEglWindowObj = glSrvParams->eglWindowObj;

            #if defined(x86_64)

            for (i = 0; i < input_desc->num_items; i++)
            {
                texProp[i].width      = intermediate_dim.dim_x;
                texProp[i].height     = intermediate_dim.dim_y;
                texProp[i].pitch[0]   = intermediate_dim.stride_y;
                texProp[i].pitch[1]   = intermediate_dim.stride_y;
                texProp[i].dataFormat = APP_EGL_DF_RGB;
                texProp[i].dmaBufFd[0] = (int64_t)glSrvParams->intermediate_addr[i];
            }
            #else

            for (i = 0; i < input_desc->num_items; i++)
            {
                texProp[i].width             = input_dim[0].dim_x;
                texProp[i].height            = input_dim[0].dim_y;
                texProp[i].pitch[0]          = img_input_desc[i]->imagepatch_addr[0].stride_y;
                texProp[i].pitch[1]          = img_input_desc[i]->imagepatch_addr[1].stride_y;
                if (VX_DF_IMAGE_YUYV == img_input_desc[i]->format)
                {
                    texProp[i].dataFormat        = APP_EGL_DF_YUYV;
                }
                else if (VX_DF_IMAGE_UYVY == img_input_desc[i]->format)
                {
                    texProp[i].dataFormat        = APP_EGL_DF_UYVY;
                }
                else
                {
                    texProp[i].dataFormat        = APP_EGL_DF_NV12;
                }
                texProp[i].dmaBufFdOffset[0] = img_input_desc[i]->mem_ptr[0].dma_buf_fd_offset;
                texProp[i].dmaBufFdOffset[1] = img_input_desc[i]->mem_ptr[1].dma_buf_fd_offset;
            }

            texProp[0].dmaBufFd[0] = img_input_desc[0]->mem_ptr[0].dma_buf_fd;
            texProp[0].dmaBufFd[1] = img_input_desc[0]->mem_ptr[1].dma_buf_fd;
            texProp[1].dmaBufFd[0] = img_input_desc[1]->mem_ptr[0].dma_buf_fd;
            texProp[1].dmaBufFd[1] = img_input_desc[1]->mem_ptr[1].dma_buf_fd;
            texProp[2].dmaBufFd[0] = img_input_desc[2]->mem_ptr[0].dma_buf_fd;
            texProp[2].dmaBufFd[1] = img_input_desc[2]->mem_ptr[1].dma_buf_fd;
            texProp[3].dmaBufFd[0] = img_input_desc[3]->mem_ptr[0].dma_buf_fd;
            texProp[3].dmaBufFd[1] = img_input_desc[3]->mem_ptr[1].dma_buf_fd;

            texProp[0].bufAddr[0] = input_target_ptr[0][0];
            texProp[0].bufAddr[1] = input_target_ptr[0][1];
            texProp[1].bufAddr[0] = input_target_ptr[1][0];
            texProp[1].bufAddr[1] = input_target_ptr[1][1];
            texProp[2].bufAddr[0] = input_target_ptr[2][0];
            texProp[2].bufAddr[1] = input_target_ptr[2][1];
            texProp[3].bufAddr[0] = input_target_ptr[3][0];
            texProp[3].bufAddr[1] = input_target_ptr[3][1];
            #endif

            for (i = 0; i < input_desc->num_items; i++)
            {
                texYuv[i] = appEglWindowGetTexYuv(pEglWindowObj, &texProp[i]);
            }

            /* bind framebuffer for this draw
             * we have only one buffer
             */
            renderTexProp.width          = output_desc->imagepatch_addr[0].dim_x;
            renderTexProp.height         = output_desc->imagepatch_addr[0].dim_y;
            renderTexProp.pitch[0]       = output_desc->imagepatch_addr[0].stride_y;
            renderTexProp.pitch[1]       = output_desc->imagepatch_addr[0].stride_y;
            renderTexProp.dataFormat     = APP_EGL_DF_RGBX;

            renderTexProp.dmaBufFd[0]       = output_desc->mem_ptr[0].dma_buf_fd;
            renderTexProp.dmaBufFdOffset[0] = output_desc->mem_ptr[0].dma_buf_fd_offset;
            renderTexProp.bufAddr[0]        = output_target_ptr;
            renderTexProp.bufAddr[1]        = NULL;
            cur_time = tivxPlatformGetTimeInUsecs();

            appEglBindFrameBuffer(glSrvParams->eglWindowObj, &renderTexProp);
            appEglCheckGlError("appEglBindFrameBuffer");

            #if defined(x86_64)
            render_renderFrame(
                        &glSrvParams->render3DSRVObj,
                        glSrvParams->eglWindowObj,
                        texYuv
                        );
            #else
            render_renderFrame(
                        &glSrvParams->render3DSRVObj,
                        glSrvParams->eglWindowObj,
                        texYuv
                        );
            #endif

            appEglCheckGlError("render_renderFrame");
            glFinish();
            appEglCheckGlError("glFinish");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            appEglCheckGlError("glBindFramebuffer");
            appEglSwap(glSrvParams->eglWindowObj);

            cur_time = tivxPlatformGetTimeInUsecs() - cur_time;


            appPerfStatsHwaUpdateLoad(APP_PERF_HWA_GPU,
                cur_time,
                output_desc->imagepatch_addr[0].dim_x*output_desc->imagepatch_addr[0].dim_y /* pixels processed */
                );

            #if defined(x86_64)
            /* Buffer copy from GPU bufer to OVX buffer */
            {
                GLint x = 0;
                GLint y = 0;
                GLsizei width = output_desc->imagepatch_addr[0].dim_x;
                GLsizei height = output_desc->imagepatch_addr[0].dim_y;
                GLenum format = GL_RGBA;
                GLenum type = GL_UNSIGNED_BYTE;
                void *pixels = output_target_ptr;

                #ifdef APP_OPENGL_MOSAIC_DEBUG
                printf("APP OpenGL Mosaic: buffer copy - started !!!\n");
                #endif
                glReadPixels(x, y, width, height, format, type, pixels);
                appEglCheckGlError("glReadPixels");
                #ifdef APP_OPENGL_MOSAIC_DEBUG
                printf("APP OpenGL Mosaic: buffer copy - completed !!!\n");
                #endif
            }
            #endif
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxGlSrvCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxGlSrvParams *glSrvParams = NULL;
    tivx_obj_desc_object_array_t *input_desc;
    tivx_obj_desc_array_t *galign_lut_desc;
    tivx_obj_desc_user_data_object_t *obj_desc_configuration;
    tivx_obj_desc_image_t *img_input_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_object_array_t *srv_views_desc;
    tivx_obj_desc_user_data_object_t *user_object_srv_views_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_image_t *output_desc;
    tivx_srv_params_t *params;

    if ( (num_params != TIVX_KERNEL_GL_SRV_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        glSrvParams = tivxMemAlloc(sizeof(tivxGlSrvParams), TIVX_MEM_EXTERNAL);
        if(NULL != glSrvParams)
        {
            memset(glSrvParams, 0, sizeof(tivxGlSrvParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "GL SRV: ERROR: Couldn't allocate memory! \r\n");
            status = VX_ERROR_NO_MEMORY;
        }

        if(VX_SUCCESS == status)
        {
            void *srv_views_target_ptr[TIVX_OBJECT_ARRAY_MAX_ITEMS];
            void *config_target_ptr;
            void *galign_target_ptr;
            VXLIB_bufParams2D_t input_dim, output_dim;
            uint32_t i, num_srv_views = 0;
            srv_coords_t *srv_views[TIVX_OBJECT_ARRAY_MAX_ITEMS];

            obj_desc_configuration = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_GL_SRV_CONFIGURATION_IDX];
            input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX];
            srv_views_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_SRV_VIEWS_IDX];
            galign_lut_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_GALIGN_LUT_IDX];
            output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GL_SRV_OUTPUT_IDX];

            tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_items);

            config_target_ptr = tivxMemShared2TargetPtr(&obj_desc_configuration->mem_ptr);

            tivxInitBufParams(output_desc, &output_dim);

            tivxInitBufParams(img_input_desc[0], (VXLIB_bufParams2D_t*)&input_dim);

            #if defined(x86_64)
            /* Allocating an RGB image corresponding to the size of the input */
            glSrvParams->intermediate_addr_size = 3*input_dim.dim_x*input_dim.dim_y;
            for (i = 0; i < input_desc->num_items; i++)
            {
                glSrvParams->intermediate_addr[i] = tivxMemAlloc(glSrvParams->intermediate_addr_size, TIVX_MEM_EXTERNAL);
            }
            #endif
            if(VX_SUCCESS == status)
            {
                for (i = 0; i < TIVX_OBJECT_ARRAY_MAX_ITEMS; i++)
                {
                    srv_views[i] = NULL;
                }

                if( srv_views_desc != NULL)
                {
                    num_srv_views = srv_views_desc->num_items;
                    tivxGetObjDescList(srv_views_desc->obj_desc_id, (tivx_obj_desc_t**)user_object_srv_views_desc, srv_views_desc->num_items);
                    for(i=0; i<srv_views_desc->num_items; i++)
                    {
                        srv_views_target_ptr[i] = tivxMemShared2TargetPtr(&user_object_srv_views_desc[i]->mem_ptr);
                        srv_views[i] = (srv_coords_t *)srv_views_target_ptr[i];
                    }
                }

                if( galign_lut_desc != NULL)
                {
                    galign_target_ptr = tivxMemShared2TargetPtr(&galign_lut_desc->mem_ptr);
                    glSrvParams->render3DSRVObj.LUT3D = (void *)galign_target_ptr;
                }
                else
                {
                    glSrvParams->render3DSRVObj.LUT3D = NULL;
                }

                params = (tivx_srv_params_t *)config_target_ptr;

                status = tivxOpenglGlSrvEglInfo(glSrvParams, input_dim, output_dim,
                             params, srv_views, num_srv_views);
            }
        }

        if(VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel,
                                               glSrvParams,
                                               sizeof(glSrvParams));
        }
        else
        {
            if(NULL != glSrvParams)
            {
                tivxMemFree(glSrvParams, sizeof(glSrvParams), TIVX_MEM_EXTERNAL);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxGlSrvDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    #if defined(x86_64)
    uint32_t i;
    tivx_obj_desc_object_array_t *input_desc;
    #endif
    tivxGlSrvParams *glSrvParams = NULL;
    vx_uint32 size  = 0;

    if ( (num_params != TIVX_KERNEL_GL_SRV_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_GL_SRV_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **) &glSrvParams,
                                                    &size);
        #if defined(x86_64)
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_GL_SRV_INPUT_IDX];
        for (i = 0; i < input_desc->num_items; i++)
        {
            tivxMemFree(glSrvParams->intermediate_addr[i], glSrvParams->intermediate_addr_size, TIVX_MEM_EXTERNAL);
        }
        #endif

        if(VX_SUCCESS != status)
        {
            status = ReleaseView();
        }

        if(VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "GL SRV: ERROR: Could not obtain SRV kernel instance context!\r\n");
        }

        if((VX_SUCCESS == status) && (NULL != glSrvParams))
        {
            status = appEglWindowClose(glSrvParams->eglWindowObj);

            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "GL SRV: ERROR - appEglWindowClose failed! \r\n");
            }

            if((NULL != glSrvParams) && (sizeof(tivxGlSrvParams) == size))
            {
                tivxMemFree(glSrvParams, sizeof(tivxGlSrvParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

void tivxAddTargetKernelGlSrv(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_MPU_0 )
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_gl_srv_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_GL_SRV_NAME,
                            target_name,
                            tivxGlSrvProcess,
                            tivxGlSrvCreate,
                            tivxGlSrvDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelGlSrv(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_gl_srv_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_gl_srv_target_kernel = NULL;
    }
}


