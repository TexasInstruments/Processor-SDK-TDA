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
#include "TI/tivx_sample.h"
#include "VX/vx.h"
#include "tivx_sample_kernels_priv.h"
#include "tivx_kernel_opengl_mosaic.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "gpu_render1x1.h"
#include "gpu_render2x2.h"
#include <app_gl_egl_utils.h>
#include <utils/mem/include/app_mem.h>
#if defined(x86_64)
#include <ti/vxlib/vxlib.h>
#endif

/* #define APP_OPENGL_MOSAIC_DEBUG */

#define MAX_TILES   (4)

typedef struct
{
    tivx_opengl_mosaic_params_t createArgs;
    /**< placeholder to store the create parameters */

    void *eglWindowObj;
    /**< EGL object information */

    GpuRender1x1_Obj render1x1Obj;
    /**< 1x1 rendering prgram obj */

    GpuRender2x2_Obj render2x2Obj;
    /**< 2x2 rendering prgram obj */

    #if defined(x86_64)
    uint8_t *intermediate_addr[MAX_TILES];

    uint32_t intermediate_addr_size;
    #endif

} TivxOpenGLMosaicParams;

static tivx_target_kernel vx_opengl_mosaic_target_kernel = NULL;

static vx_status VX_CALLBACK tivxOpenglMosaicProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxOpenglMosaicCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxOpenglMosaicDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxOpenglMosaicProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    TivxOpenGLMosaicParams *mosaicParams = NULL;
    tivx_obj_desc_object_array_t *input_desc;
    tivx_obj_desc_image_t *img_input_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    vx_uint32 i;
    tivx_obj_desc_image_t *output_desc;
    vx_uint32 size  = 0;

    if ( (num_params != TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void *)&mosaicParams,
                                                    &size);
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX];

        if(VX_DF_IMAGE_RGBX != output_desc->format)
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        void *input_target_ptr[TIVX_OBJECT_ARRAY_MAX_ITEMS][TIVX_OBJECT_ARRAY_MAX_ITEMS] = {NULL};
        void *output_target_ptr;
        vx_uint32 j;

        tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_items);

        for(i=0; i<input_desc->num_items; i++)
        {
            for (j= 0; j < img_input_desc[i]->planes; j++)
            {
                input_target_ptr[i][j] = NULL;
            }
        }

        tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_items);
        for(i=0; i<input_desc->num_items; i++)
        {
            for (j= 0; j < img_input_desc[i]->planes; j++)
            {
                input_target_ptr[i][j] = tivxMemShared2TargetPtr(&img_input_desc[i]->mem_ptr[j]);
            }
        }

        output_target_ptr = tivxMemShared2TargetPtr(
           &output_desc->mem_ptr[0]);

        /* call kernel processing function */
        {
            GLuint texYuv[8] = {0};
            app_egl_tex_prop_t texProp[4] = {0}, renderTexProp;
            void *pEglWindowObj = mosaicParams->eglWindowObj;
            unsigned int i, numWindows;

            numWindows = 1;
            if(mosaicParams->createArgs.renderType==TIVX_KERNEL_OPENGL_MOSAIC_TYPE_2x2)
            {
                numWindows = MAX_TILES;
            }

            for (i = 0; i<numWindows; i++)
            {
                texProp[i].width             = img_input_desc[0]->imagepatch_addr[0].dim_x;
                texProp[i].height            = img_input_desc[0]->imagepatch_addr[0].dim_y;
                texProp[i].pitch[0]          = img_input_desc[0]->imagepatch_addr[0].stride_y;
                texProp[i].pitch[1]          = img_input_desc[0]->imagepatch_addr[1].stride_y;
                texProp[i].dataFormat        = APP_EGL_DF_NV12;
            }

            texProp[0].dmaBufFdOffset[0] = img_input_desc[0]->mem_ptr[0].dma_buf_fd_offset;
            texProp[0].dmaBufFdOffset[1] = img_input_desc[0]->mem_ptr[1].dma_buf_fd_offset;

            if (numWindows > 1)
            {
                for (i = 1; i < MAX_TILES; i++)
                {
                    for (j = 0; j < img_input_desc[i]->planes; j++)
                    {
                        texProp[i].dmaBufFdOffset[j] = img_input_desc[i]->mem_ptr[j].dma_buf_fd_offset;
                    }
                }
            }

            #if defined(x86_64)
            VXLIB_bufParams2D_t input_dim[TIVX_OBJECT_ARRAY_MAX_ITEMS], intermediate_dim;

            intermediate_dim.dim_x     = img_input_desc[0]->imagepatch_addr[0].dim_x;
            intermediate_dim.dim_y     = img_input_desc[0]->imagepatch_addr[0].dim_y;
            intermediate_dim.stride_y  = 3*img_input_desc[0]->imagepatch_addr[0].dim_x;
            intermediate_dim.data_type = VXLIB_UINT24;

            /* All buf params are the same, so only using the first */
            tivxInitBufParams(img_input_desc[0], (VXLIB_bufParams2D_t*)&input_dim);

            status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)input_target_ptr[0][0], &input_dim[0],
                    (uint8_t *)input_target_ptr[0][1], &input_dim[1], (uint8_t *)mosaicParams->intermediate_addr[0], &intermediate_dim, 0,
                    img_input_desc[0]->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));

            /* convert to RGB for PC emulation mode*/
            if (4 == numWindows)
            {
                status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)input_target_ptr[1][0], &input_dim[0],
                    (uint8_t *)input_target_ptr[1][1], &input_dim[1], (uint8_t *)mosaicParams->intermediate_addr[1], &intermediate_dim, 0,
                    img_input_desc[0]->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));

                status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)input_target_ptr[2][0], &input_dim[0],
                    (uint8_t *)input_target_ptr[2][1], &input_dim[1], (uint8_t *)mosaicParams->intermediate_addr[2], &intermediate_dim, 0,
                    img_input_desc[0]->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));

                status = VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)input_target_ptr[3][0], &input_dim[0],
                    (uint8_t *)input_target_ptr[3][1], &input_dim[1], (uint8_t *)mosaicParams->intermediate_addr[3], &intermediate_dim, 0,
                    img_input_desc[0]->color_space - VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_COLOR_SPACE));
            }

            #endif

            #if defined(x86_64)

            texProp[0].width      = intermediate_dim.dim_x;
            texProp[0].height     = intermediate_dim.dim_y;
            texProp[0].pitch[0]   = intermediate_dim.stride_y;
            texProp[0].pitch[1]   = intermediate_dim.stride_y;
            texProp[0].dataFormat = APP_EGL_DF_RGB;
            #endif

            for (i = 0; i < MAX_TILES; i++)
            {
                for (j = 0; j < img_input_desc[i]->planes; j++)
                {
                    texProp[i].dmaBufFd[j] = img_input_desc[i]->mem_ptr[j].dma_buf_fd;
                    texProp[i].bufAddr[j]  = input_target_ptr[i][j];
                }
            }

            for (i = 0; i<numWindows; i++)
            {
                #if defined(x86_64)
                texYuv[i] = appEglWindowGetTexYuv(pEglWindowObj, &texProp[0]);
                #else
                texYuv[i] = appEglWindowGetTexYuv(pEglWindowObj, &texProp[i]);
                #endif
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
            renderTexProp.bufAddr[0]           = output_target_ptr;

            appEglBindFrameBuffer(mosaicParams->eglWindowObj, &renderTexProp);

            glViewport(0,
                0,
                renderTexProp.width,
                renderTexProp.height);

            switch(mosaicParams->createArgs.renderType)
            {
                default:
                case TIVX_KERNEL_OPENGL_MOSAIC_TYPE_1x1:
                    gpuRender1x1_renderFrame(
                        &mosaicParams->render1x1Obj,
                        mosaicParams->eglWindowObj,
                        texYuv[0]
                        );
                break;
                case TIVX_KERNEL_OPENGL_MOSAIC_TYPE_2x2:
                    gpuRender2x2_renderFrame(
                        &mosaicParams->render2x2Obj,
                        mosaicParams->eglWindowObj,
                        texYuv,
                        4
                        );
                break;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glFinish();
            appEglCheckGlError("glFinish");
            appEglSwap(mosaicParams->eglWindowObj);
        }
        /* kernel processing function complete */
    }
    return status;
}


int32_t tivxOpenglMosaicProcessEglInfo(TivxOpenGLMosaicParams *mosaicParams)
{
    vx_status status = VX_SUCCESS;

    mosaicParams->eglWindowObj = appEglWindowOpen();

    if (NULL != mosaicParams->eglWindowObj)
    {
        switch(mosaicParams->createArgs.renderType)
        {
            default:
            case TIVX_KERNEL_OPENGL_MOSAIC_TYPE_1x1:
                status = gpuRender1x1_setup(&mosaicParams->render1x1Obj);
                break;
            case TIVX_KERNEL_OPENGL_MOSAIC_TYPE_2x2:
                status = gpuRender2x2_setup(&mosaicParams->render2x2Obj);
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "OpenGL Mosaic: ERROR: Couldn't open openGL window! \r\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxOpenglMosaicCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc_configuration;
    TivxOpenGLMosaicParams *mosaicParams = NULL;
    tivx_opengl_mosaic_params_t *params;
    void *mosaic_config_target_ptr;
    tivx_obj_desc_image_t *output_desc;

    #if defined(x86_64)
    tivx_obj_desc_object_array_t *input_desc;
    tivx_obj_desc_image_t *img_input_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    #endif

    if ( (num_params != TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        obj_desc_configuration = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX];
        #if defined(x86_64)
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX];
        #endif
        mosaicParams = tivxMemAlloc(sizeof(TivxOpenGLMosaicParams), TIVX_MEM_EXTERNAL);

        if(NULL != mosaicParams)
        {
            memset(&mosaicParams->createArgs, 0, sizeof(tivx_opengl_mosaic_params_t));
            memset(&mosaicParams->eglWindowObj, 0, sizeof(void));
            memset(&mosaicParams->render1x1Obj, 0, sizeof(GpuRender1x1_Obj));
            memset(&mosaicParams->render2x2Obj, 0, sizeof(GpuRender2x2_Obj));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "OpenGL Mosaic: ERROR: Couldn't allocate memory! \r\n");
            status = VX_ERROR_NO_MEMORY;
        }

        if(sizeof(tivx_opengl_mosaic_params_t) != obj_desc_configuration->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "OpenGL Mosaic: ERROR: Params size is not correct! \r\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        if(VX_SUCCESS == status)
        {
            mosaic_config_target_ptr = tivxMemShared2TargetPtr(&obj_desc_configuration->mem_ptr);

            tivxMemBufferMap(mosaic_config_target_ptr,
                             obj_desc_configuration->mem_size,
                             VX_MEMORY_TYPE_HOST,
                             VX_READ_ONLY);

            params = (tivx_opengl_mosaic_params_t *)mosaic_config_target_ptr;
            memcpy(&mosaicParams->createArgs, params, sizeof(*params));
        }

        #if defined(x86_64)
        int i;
        tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_items);

        /* Allocating an RGB image corresponding to the size of the input */
        mosaicParams->intermediate_addr_size = 3*img_input_desc[0]->imagepatch_addr[0].dim_x*img_input_desc[0]->imagepatch_addr[0].dim_y;
        for (i = 0; i < MAX_TILES; i++)
        {
            mosaicParams->intermediate_addr[i] = tivxMemAlloc(mosaicParams->intermediate_addr_size, TIVX_MEM_EXTERNAL);
        }
        #endif

        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX];

        if(VX_DF_IMAGE_RGBX != output_desc->format)
        {
            status = VX_FAILURE;
        }

        if(VX_SUCCESS == status)
        {
            status = tivxOpenglMosaicProcessEglInfo(mosaicParams);
        }

        if(VX_SUCCESS == status)
        {
            tivxMemBufferUnmap(mosaic_config_target_ptr,
                               obj_desc_configuration->mem_size,
                               VX_MEMORY_TYPE_HOST,
                               VX_READ_ONLY);
            tivxSetTargetKernelInstanceContext(kernel,
                                               mosaicParams,
                                               sizeof(TivxOpenGLMosaicParams));
        }
        else
        {
            if(NULL != mosaicParams)
            {
                tivxMemFree(mosaicParams, sizeof(TivxOpenGLMosaicParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxOpenglMosaicDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    TivxOpenGLMosaicParams *mosaicParams = NULL;
    vx_uint32 size  = 0;

    if ( (num_params != TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **) &mosaicParams,
                                                    &size);
        if(VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "OpenGL Mosaic: ERROR: Could not obtain Mosaic kernel instance context!\r\n");
        }
        if((NULL != mosaicParams) && (VX_SUCCESS == status))
        {
            #if defined(x86_64)
            vx_uint32 i;

            for (i = 0; i < MAX_TILES; i++)
            {
                tivxMemFree(mosaicParams->intermediate_addr[i], mosaicParams->intermediate_addr_size, TIVX_MEM_EXTERNAL);
            }
            #endif

            status = appEglWindowClose(mosaicParams->eglWindowObj);
            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "OpenGL Mosaic: ERROR - appEglWindowClose failed! \r\n");
            }
            tivxMemFree(mosaicParams, sizeof(TivxOpenGLMosaicParams), TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

void tivxAddTargetKernelOpenglMosaic(void)
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
        vx_opengl_mosaic_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_OPENGL_MOSAIC_NAME,
                            target_name,
                            tivxOpenglMosaicProcess,
                            tivxOpenglMosaicCreate,
                            tivxOpenglMosaicDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelOpenglMosaic(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_opengl_mosaic_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_opengl_mosaic_target_kernel = NULL;
    }
}


