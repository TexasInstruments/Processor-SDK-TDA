/*
*
* Copyright (c) 2021-2026 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <TI/hwa_vpac_msc.h>

#include <tivx_kernels_target_utils.h>
#include <vx_kernels_hwa_target.h>
#include <tivx_img_mosaic_host.h>
#include "scaler_core.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TIVX_IMAGE_MOSAIC_MAX_PLANES    (2u) /* one plane for Y, one plane for C */

#define TIVX_IMAGE_MOSAIC_MSC_MAX_INST  (2u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* Mosaic MSC internal object */
typedef struct
{
    msc_config config;

    uint16_t *src16[2];
    uint16_t *dst16[2];

    uint32_t crop_width;
    uint32_t crop_height;

    /* State from user commands to override auto mode or not */
    uint32_t user_init_phase_x;
    uint32_t user_init_phase_y;
    uint32_t user_offset_x;
    uint32_t user_offset_y;
    uint32_t user_crop_start_x;
    uint32_t user_crop_start_y;

} tivxImgMosaicMscInstObj;

typedef struct
{
    tivxImgMosaicMscInstObj inst_obj[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

    uint32_t buffer_size_in;
    uint32_t buffer_size_out;
    uint16_t *src16[2];
    uint16_t *dst16[2];

    tivxImgMosaicWindow    *tmp_win[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    uint32_t                tmp_in_ch[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    uint32_t                tmp_ch_sel[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

    tivx_obj_desc_object_array_t *in_arr_desc[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    tivx_obj_desc_image_t        *in_img_desc[TIVX_IMAGE_MOSAIC_MSC_MAX_INST][TIVX_OBJECT_ARRAY_MAX_ITEMS];

    vx_uint32               max_msc_instances;
    vx_uint32               msc_instance;

    /** Number of times to clear the output buffer */
    vx_uint32               clear_count;

} tivxImgMosaicMscObj;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* Interface API Functions */
static vx_status VX_CALLBACK tivxKernelImgMosaicMscCreate(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg);
static vx_status VX_CALLBACK tivxKernelImgMosaicMscDelete(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg);
static vx_status VX_CALLBACK tivxKernelImgMosaicMscProcess(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg);

/* Local Functions */
static void tivxKernelImgMosaicMscFreeMem(tivxImgMosaicMscObj *msc_obj);
static void tivxVpacMscScaleInitParams(Scaler_Config *settings);
static vx_status tivxVpacMscScaleUpdateOutputSettings(tivxImgMosaicMscInstObj *prms, uint32_t ow, uint32_t oh, uint32_t cnt, uint32_t h_divider, vx_df_image format);

static vx_status tivxKernelImgMosaicMscSimProcess(
    tivxImgMosaicMscInstObj *inst_obj,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window,
    const tivx_obj_desc_image_t* out_img_desc);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_imgMosaicMsc1_kernel = NULL;
static tivx_target_kernel vx_imgMosaicMsc2_kernel = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelImgMosaicMsc(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    /* Both scalars are used, but the target used it always MSC0 */
    strncpy(target_name, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME);
    vx_imgMosaicMsc1_kernel = tivxAddTargetKernelByName(
                                 TIVX_KERNEL_IMG_MOSAIC_NAME,
                                 target_name,
                                 tivxKernelImgMosaicMscProcess,
                                 tivxKernelImgMosaicMscCreate,
                                 tivxKernelImgMosaicMscDelete,
                                 NULL,
                                 NULL);
    if(NULL == vx_imgMosaicMsc1_kernel)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Add Img Mosaic MSC TargetKernel\n");
    }

    strncpy(target_name, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_MAX_NAME);
    vx_imgMosaicMsc2_kernel = tivxAddTargetKernelByName(
                                 TIVX_KERNEL_IMG_MOSAIC_NAME,
                                 target_name,
                                 tivxKernelImgMosaicMscProcess,
                                 tivxKernelImgMosaicMscCreate,
                                 tivxKernelImgMosaicMscDelete,
                                 NULL,
                                 NULL);
    if(NULL == vx_imgMosaicMsc2_kernel)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Add Img Mosaic MSC TargetKernel\n");
    }
}

void tivxRemoveTargetKernelImgMosaicMsc(void)
{
    vx_status status;

    status = tivxRemoveTargetKernel(vx_imgMosaicMsc1_kernel);
    if(status == VX_SUCCESS)
    {
        vx_imgMosaicMsc1_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Img Mosaic MSC TargetKernel\n");
    }
    status = tivxRemoveTargetKernel(vx_imgMosaicMsc2_kernel);
    if(status == VX_SUCCESS)
    {
        vx_imgMosaicMsc2_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Img Mosaic MSC TargetKernel\n");
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxKernelImgMosaicMscCreate(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    uint32_t                          i;
    tivx_obj_desc_user_data_object_t *config_desc;
    void                             *config_target_ptr;
    tivx_obj_desc_image_t            *out_img_desc;
    tivx_obj_desc_object_array_t     *input_image_arr_desc;
    tivx_obj_desc_image_t            *in_img_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivxImgMosaicParams              *params;
    vx_int32                          win;
    vx_int32                          num_inputs;
    tivxImgMosaicMscObj              *msc_obj = NULL;

    for(i = 0U; i < num_params; i++)
    {
        if ((NULL == obj_desc[i]) && (i != TIVX_IMG_MOSAIC_HOST_BACKGROUND_IMAGE_IDX))
        {
            status = VX_FAILURE;
            break;
        }
    }

    if(VX_SUCCESS == status)
    {
        /* Alloc mosaic object */
        msc_obj = tivxMemAlloc(sizeof(tivxImgMosaicMscObj), TIVX_MEM_EXTERNAL);
        if(NULL == msc_obj)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            tivxSetTargetKernelInstanceContext(kernel, msc_obj,  sizeof(tivxImgMosaicMscObj));
            memset(msc_obj, 0x0, sizeof(tivxImgMosaicMscObj));
        }
    }

    if(VX_SUCCESS == status)
    {
        /* 0 - config, 1 - output image, 2 - background image, 3 onwards is array of inputs */
        num_inputs = num_params - TIVX_IMG_MOSAIC_BASE_PARAMS;

        config_desc = (tivx_obj_desc_user_data_object_t *)
            obj_desc[TIVX_IMG_MOSAIC_HOST_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(
            config_target_ptr,
            config_desc->mem_size,
            VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        params = (tivxImgMosaicParams *) config_target_ptr;
        if(params->num_windows > TIVX_IMG_MOSAIC_MAX_WINDOWS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "Num windows %d greater than supported max %d\n",
                params->num_windows, TIVX_IMG_MOSAIC_MAX_WINDOWS);
            status = VX_FAILURE;
        }

        out_img_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_IMG_MOSAIC_HOST_OUTPUT_IMAGE_IDX];
        if(VX_SUCCESS == status)
        {
            vx_uint32 maxInPixels = 0;
            vx_uint32 maxOutPixels = 0;

            for(win = 0U; win < params->num_windows; win++)
            {
                tivxImgMosaicWindow *window =
                    (tivxImgMosaicWindow *) &params->windows[win];
                vx_int32 in = window->input_select;
                vx_int32 ch = window->channel_select;
                vx_int32 outPixels = window->width * window->height;
                vx_int32 inPixels;

                if(maxOutPixels < outPixels)
                {
                    maxOutPixels = outPixels;
                }

                if(in > num_inputs)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "Input ID %d greater than num_inputs %d\n",
                        in, num_inputs);
                    status = VX_FAILURE;
                    break;
                }

                input_image_arr_desc = (tivx_obj_desc_object_array_t *)
                    obj_desc[TIVX_IMG_MOSAIC_INPUT_START_IDX + in];
                tivxGetObjDescList(
                    input_image_arr_desc->obj_desc_id,
                    (tivx_obj_desc_t **) &in_img_desc[0U],
                    input_image_arr_desc->num_items);

                inPixels = in_img_desc[0U]->width * in_img_desc[0U]->height;
                if(maxInPixels < inPixels)
                {
                    maxInPixels = inPixels;
                }

                if(ch > input_image_arr_desc->num_items)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "Channel ID %d greater than num_items %d\n",
                        ch, input_image_arr_desc->num_items);
                    status = VX_FAILURE;
                    break;
                }

                if(((window->startX + window->width) > out_img_desc->width) ||
                   ((window->startY + window->height) > out_img_desc->height))
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "Window %d, does not fit within output image!\n", win);
                    status = VX_FAILURE;
                }
            }
            msc_obj->buffer_size_in  = maxInPixels*2U;
            msc_obj->buffer_size_out = maxOutPixels*2U;
        }

        VX_PRINT(VX_ZONE_INFO, "num_msc_instance = %d \n", params->num_msc_instances);
        VX_PRINT(VX_ZONE_INFO, "msc_instance = %d \n", params->msc_instance);

        if (params->num_msc_instances < TIVX_IMAGE_MOSAIC_MSC_MAX_INST)
        {
            msc_obj->max_msc_instances = params->num_msc_instances;
        }
        else
        {
            msc_obj->max_msc_instances = TIVX_IMAGE_MOSAIC_MSC_MAX_INST;
        }

        if (params->msc_instance > TIVX_IMAGE_MOSAIC_MSC_MAX_INST)
        {
            msc_obj->msc_instance = 0;
        }
        else
        {
            msc_obj->msc_instance = params->msc_instance;
        }

        msc_obj->clear_count= params->clear_count;

        tivxMemBufferUnmap(
            config_target_ptr,
            config_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    if(VX_SUCCESS == status)
    {
        for (i = 0u; (i < 2u) && (VX_SUCCESS == status); i ++)
        {
            msc_obj->src16[i] = tivxMemAlloc(msc_obj->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == msc_obj->src16[i])
            {
                VX_PRINT(VX_ZONE_ERROR, "Input Buffer Alloc Error\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
        for (i = 0u; (i < 2u) && (VX_SUCCESS == status); i ++)
        {
            msc_obj->dst16[i] = tivxMemAlloc(msc_obj->buffer_size_out, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == msc_obj->dst16[i])
            {
                VX_PRINT(VX_ZONE_ERROR, "Output Buffer Alloc Error\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }
    }

    if(VX_SUCCESS == status)
    {
        for (i = 0u; i < msc_obj->max_msc_instances; i ++)
        {
            msc_obj->inst_obj[i].src16[0] = msc_obj->src16[0];
            msc_obj->inst_obj[i].src16[1] = msc_obj->src16[1];
            msc_obj->inst_obj[i].dst16[0] = msc_obj->dst16[0];
            msc_obj->inst_obj[i].dst16[1] = msc_obj->dst16[1];
            tivxVpacMscScaleInitParams(&msc_obj->inst_obj[i].config.settings);
        }
    }

    /* Free the allocated memory incase of error */
    if(VX_SUCCESS != status)
    {
        if(msc_obj != NULL)
        {
            tivxKernelImgMosaicMscFreeMem(msc_obj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelImgMosaicMscDelete(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status            status = VX_SUCCESS;
    tivxImgMosaicMscObj *msc_obj = NULL;
    uint32_t             i;
    vx_uint32            size;

    for(i = 0U; i < num_params; i++)
    {
        if ((NULL == obj_desc[i]) && (i != TIVX_IMG_MOSAIC_HOST_BACKGROUND_IMAGE_IDX))
        {
            status = VX_FAILURE;
            break;
        }
    }

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&msc_obj, &size);
    if ((VX_SUCCESS != status) || (NULL == msc_obj) ||  (sizeof(tivxImgMosaicMscObj) != size))
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxKernelImgMosaicMscFreeMem(msc_obj);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelImgMosaicMscProcess(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *config_desc;
    void                             *config_target_ptr;
    tivx_obj_desc_image_t            *out_img_desc;
    void                             *output_image_target_ptr[2U] = {NULL};
    vx_int32                          win, in, ch, msc_cnt;
    vx_int32                          num_inputs;
    tivxImgMosaicParams              *params;
    tivxImgMosaicMscObj              *msc_obj = NULL;
    vx_uint32                         size, num_inst;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&msc_obj, &size);
    if((VX_SUCCESS != status) || (NULL == msc_obj) ||  (sizeof(tivxImgMosaicMscObj) != size))
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        /* Map config data - index 0 */
        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_IMG_MOSAIC_HOST_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        params = (tivxImgMosaicParams *) config_target_ptr;

        /* Memset output buffers for initial count */
        if(msc_obj->clear_count > 0U)
        {
            /* Map output image luma buffer */
            out_img_desc = (tivx_obj_desc_image_t *) obj_desc[TIVX_IMG_MOSAIC_HOST_OUTPUT_IMAGE_IDX];
            output_image_target_ptr[0U] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[0U]);
            tivxMemBufferMap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

            /* Map output image CbCr buffer */
            if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
            {
                output_image_target_ptr[1U] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[1U]);
                tivxMemBufferMap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }

            if (NULL != obj_desc[TIVX_IMG_MOSAIC_HOST_BACKGROUND_IMAGE_IDX])
            {
                tivx_obj_desc_image_t *background_img_desc;
                void *backgroud_image_target_ptr[2U];

                /* Map background luma buffer */
                background_img_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_IMG_MOSAIC_HOST_BACKGROUND_IMAGE_IDX];
                backgroud_image_target_ptr[0U] = tivxMemShared2TargetPtr(&background_img_desc->mem_ptr[0U]);
                tivxMemBufferMap(backgroud_image_target_ptr[0U], background_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                /* Map background CbCr buffer */
                if(background_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
                {
                    backgroud_image_target_ptr[1U] = tivxMemShared2TargetPtr(&background_img_desc->mem_ptr[1U]);
                    tivxMemBufferMap(backgroud_image_target_ptr[1U], background_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
                }

                /* Copy luma data */
                if ((output_image_target_ptr[0U] != NULL) && (backgroud_image_target_ptr[0U] != NULL))
                {
                    /* Copy only if the sizes match */
                    if(out_img_desc->mem_size[0U] == background_img_desc->mem_size[0U])
                    {
                        memcpy(output_image_target_ptr[0U], backgroud_image_target_ptr[0U], out_img_desc->mem_size[0U]);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Background luma size %d and output luma size %d do not match!\n", background_img_desc->mem_size[0U], out_img_desc->mem_size[0U]);
                    }
                }

                /* Copy cbcr data */
                if ((output_image_target_ptr[1U] != NULL) && (backgroud_image_target_ptr[1U] != NULL))
                {
                    /* Copy only if the sizes match */
                    if(out_img_desc->mem_size[1U] == background_img_desc->mem_size[1U])
                    {
                        memcpy(output_image_target_ptr[1U], backgroud_image_target_ptr[1U], out_img_desc->mem_size[1U]);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Background cbcr size %d and output cbcr size %d do not match!\n", background_img_desc->mem_size[1U], out_img_desc->mem_size[1U]);
                    }
                }

                /* Unmap background luma buffer */
                tivxMemBufferUnmap(backgroud_image_target_ptr[0U], background_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                /* Unmap background cbcr buffer */
                if(background_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
                {
                    tivxMemBufferUnmap(backgroud_image_target_ptr[1U], background_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
                }
            }
            else
            {
                memset(output_image_target_ptr[0U], 0U, out_img_desc->mem_size[0U]);

                if(output_image_target_ptr[1U] != NULL)
                {
                    memset(output_image_target_ptr[1U], 0x80U, out_img_desc->mem_size[1U]);
                }
            }

            /* unmap output buffer */
            tivxMemBufferUnmap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
            {
                tivxMemBufferUnmap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }

            msc_obj->clear_count--;
        }

        /* Map output image luma buffer */
        out_img_desc = (tivx_obj_desc_image_t *) obj_desc[TIVX_IMG_MOSAIC_HOST_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0U] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[0U]);
        tivxMemBufferMap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        /* Map output image CbCr buffer */
        if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
        {
            output_image_target_ptr[1U] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[1U]);
            tivxMemBufferMap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }

        /* 0 - config, 1 - output image, 2 - background image, 3 onwards is array of inputs */
        num_inputs = num_params - TIVX_IMG_MOSAIC_BASE_PARAMS;

        /* Perform scale operation for each of the output mosaic window */
        for(win = 0U; win < params->num_windows; win += msc_obj->max_msc_instances)
        {
            if ((params->num_windows - win) < msc_obj->max_msc_instances)
            {
                num_inst = params->num_windows - win;
            }
            else
            {
                num_inst = msc_obj->max_msc_instances;
            }

            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                msc_obj->tmp_win[msc_cnt] =
                    (tivxImgMosaicWindow *) &params->windows[win + msc_cnt];
                in = msc_obj->tmp_win[msc_cnt]->input_select;
                ch = msc_obj->tmp_win[msc_cnt]->channel_select;

                if (in < num_inputs)
                {
                    msc_obj->in_arr_desc[msc_cnt] = (tivx_obj_desc_object_array_t *)
                        obj_desc[TIVX_IMG_MOSAIC_INPUT_START_IDX + in];

                    tivxGetObjDescList(
                        msc_obj->in_arr_desc[msc_cnt]->obj_desc_id,
                        (tivx_obj_desc_t **) &msc_obj->in_img_desc[msc_cnt][0U],
                        msc_obj->in_arr_desc[msc_cnt]->num_items);

                    status = tivxKernelImgMosaicMscSimProcess(
                        &msc_obj->inst_obj[msc_cnt],
                        msc_obj->in_img_desc[msc_cnt][ch],
                        msc_obj->tmp_win[msc_cnt], out_img_desc);

                    if (VX_SUCCESS != status)
                    {
                        break;
                    }

                    msc_obj->tmp_in_ch[msc_cnt] = in;
                    msc_obj->tmp_ch_sel[msc_cnt] = ch;
                }
            }

            if (VX_SUCCESS != status)
            {
                break;
            }
        }

        /* unmap config buffer */
        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        /* unmap output buffer */
        tivxMemBufferUnmap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
    }

    return (status);
}

/* ========================================================================== */
/*                              Local Functions                               */
/* ========================================================================== */

static void tivxKernelImgMosaicMscFreeMem(tivxImgMosaicMscObj *msc_obj)
{
    uint32_t i;

    if (NULL != msc_obj)
    {
        for( i = 0; i < 2; i++)
        {
            if (NULL != msc_obj->src16[i])
            {
                tivxMemFree(msc_obj->src16[i], msc_obj->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
                msc_obj->src16[i] = NULL;
            }
            if (NULL != msc_obj->dst16[i])
            {
                tivxMemFree(msc_obj->dst16[i], msc_obj->buffer_size_out, (vx_enum)TIVX_MEM_EXTERNAL);
                msc_obj->dst16[i] = NULL;
            }
        }

        for (i = 0u; i < msc_obj->max_msc_instances; i ++)
        {
            msc_obj->inst_obj[i].src16[0] = NULL;
            msc_obj->inst_obj[i].src16[1] = NULL;
            msc_obj->inst_obj[i].dst16[0] = NULL;
            msc_obj->inst_obj[i].dst16[1] = NULL;
        }

        tivxMemFree(msc_obj, sizeof(tivxImgMosaicMscObj), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */


static vx_status tivxKernelImgMosaicMscSimProcess(
    tivxImgMosaicMscInstObj *inst_obj,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window,
    const tivx_obj_desc_image_t* out_img_desc)
{
    vx_status           status = VX_SUCCESS;
    uint32_t            plane_cnt, i;
    vx_uint32           startX, startY;
    uint32_t ow;
    uint32_t oh;
    unsigned short *imgInput[2];
    unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;

        for (i=0; (i < in_img_desc->planes) && (VX_SUCCESS == status); i++)
        {
            src_target_ptr = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[i]);
            tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, in_img_desc->mem_size[i],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            /* C-model supports only 12-bit in uint16_t container
             * So we may need to translate.  In HW, VPAC_LSE does this
             */
            lse_reformat_in(in_img_desc, src_target_ptr, inst_obj->src16[i], i, 0);

            tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, in_img_desc->mem_size[i],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if(window->width > in_img_desc->imagepatch_addr[0U].dim_x)
        {
            /* Upscaling not supported - set MSC width same as input */
            ow = in_img_desc->imagepatch_addr[0U].dim_x;
        }
        else
        {
            ow = window->width;
        }
        if(window->height > in_img_desc->imagepatch_addr[0U].dim_y)
        {
            /* Upscaling not supported - set MSC height same as input */
            oh = in_img_desc->imagepatch_addr[0U].dim_y;
        }
        else
        {
            oh = window->height;
        }

        inst_obj->user_init_phase_x = 0;//TIVX_VPAC_MSC_AUTOCOMPUTE;
        inst_obj->user_init_phase_y = 0;//TIVX_VPAC_MSC_AUTOCOMPUTE;
        inst_obj->user_offset_x =     0;//TIVX_VPAC_MSC_AUTOCOMPUTE;
        inst_obj->user_offset_y =     0;//TIVX_VPAC_MSC_AUTOCOMPUTE;

        /* Crop setting - same as input size */
        if(window->enable_roi == 0)
        {
            inst_obj->user_crop_start_x = 0U;
            inst_obj->user_crop_start_y = 0U;
            inst_obj->crop_width = in_img_desc->imagepatch_addr[0U].dim_x;
            inst_obj->crop_height = in_img_desc->imagepatch_addr[0U].dim_y;
        }
        else
        {
            inst_obj->user_crop_start_x = window->roiStartX;
            inst_obj->user_crop_start_y = window->roiStartY;
            inst_obj->crop_width = window->roiWidth;
            inst_obj->crop_height = window->roiHeight;

        }
        status = tivxVpacMscScaleUpdateOutputSettings(inst_obj, ow, oh, 0, 1, in_img_desc->format);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        inst_obj->config.settings.G_inWidth[0] = in_img_desc->imagepatch_addr[0].dim_x;
        inst_obj->config.settings.G_inHeight[0] = in_img_desc->imagepatch_addr[0].dim_y;

        /* Is it enough to set for just 1 pipe in host-emulation mode? */
        inst_obj->config.settings.unitParams[0].uvMode = 0;

        imgInput[0] = inst_obj->src16[0];
        imgOutput[0] = inst_obj->dst16[0];

        scaler_top_processing(imgInput, imgOutput, &inst_obj->config.settings);
    }

    if (((vx_status)VX_SUCCESS == status) && (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_NV12))
    {
        status = tivxVpacMscScaleUpdateOutputSettings(inst_obj, ow, oh/2, 0, 2, in_img_desc->format);

        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->config.settings.G_inWidth[0] = in_img_desc->imagepatch_addr[1].dim_x;
            inst_obj->config.settings.G_inHeight[0] = in_img_desc->imagepatch_addr[1].dim_y / in_img_desc->imagepatch_addr[1].step_y;

            /* Is it enough to set for just 1 pipe in host-emulation mode? */
            inst_obj->config.settings.unitParams[0].uvMode = 1;

            imgInput[0] = inst_obj->src16[1];
            imgOutput[0] = inst_obj->dst16[1];

            scaler_top_processing(imgInput, imgOutput, &inst_obj->config.settings);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Adjust start X/Y so that we center the input when input size is less
         * than the window size */
        startX = window->startX + ((window->width  - ow)  >> 1U);
        startY = window->startY + ((window->height - oh) >> 1U);
        startX = ((startX >> 1U) << 1U);    /* Make it even for YUV420SP */
        startY = ((startY >> 1U) << 1U);    /* Make it even for YUV420SP */

        for(plane_cnt = 0U; plane_cnt < out_img_desc->planes; plane_cnt++)
        {
            uint64_t    temp, out_addr;
            tivx_obj_desc_image_t stub_in, stub_out;

            stub_in.valid_roi.start_x = 0;
            stub_in.valid_roi.start_y = 0;

            memcpy(&stub_out, out_img_desc, sizeof(tivx_obj_desc_image_t));
            stub_out.imagepatch_addr[plane_cnt].dim_x = window->width;
            stub_out.imagepatch_addr[plane_cnt].dim_y = window->height;

            /* Map output image luma buffer */
            temp = (uint64_t)tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[plane_cnt]);

            /* Manipulate output overlay pointer to point to right offset
             * to do scaling */
            if(0U == plane_cnt)
            {
                out_addr = temp +
                    (startY * out_img_desc->imagepatch_addr[0U].stride_y) +
                    startX;
                lse_reformat_out((const tivx_obj_desc_image_t *)&stub_in, (const tivx_obj_desc_image_t *)&stub_out, (void *)out_addr,
                    inst_obj->dst16[plane_cnt], 12, 0);
            }
            else
            {
               out_addr = temp +
                    ((startY >> 1U) * out_img_desc->imagepatch_addr[1U].stride_y) +
                    startX;
               lse_reformat_out((const tivx_obj_desc_image_t *)&stub_in, (const tivx_obj_desc_image_t *)&stub_out, (void *)out_addr,
                    inst_obj->dst16[plane_cnt], 12, 1);
            }
        }
    }
    return (status);
}

static void tivxVpacMscScaleInitParams(Scaler_Config *settings)
{
    uint32_t i;
    uint32_t weight;
    /* Coefficients for Bilinear Interpolation */
    for(i=0; i<32; i++)
    {
        weight = i<<2;
        settings->coef_mp[0].matrix[i][0] = 0;
        settings->coef_mp[0].matrix[i][1] = 0;
        settings->coef_mp[0].matrix[i][2] = 256-weight;
        settings->coef_mp[0].matrix[i][3] = weight;
        settings->coef_mp[0].matrix[i][4] = 0;
    }
    for(i=0; i<32; i++)
    {
        weight = (i+32)<<2;
        settings->coef_mp[1].matrix[i][0] = 0;
        settings->coef_mp[1].matrix[i][1] = 0;
        settings->coef_mp[1].matrix[i][2] = 256-weight;
        settings->coef_mp[1].matrix[i][3] = weight;
        settings->coef_mp[1].matrix[i][4] = 0;
    }
    /* Coefficients for Nearest Neighbor */
    for(i=0; i<32; i++)
    {
        settings->coef_mp[2].matrix[i][0] = 0;
        settings->coef_mp[2].matrix[i][1] = 0;
        settings->coef_mp[2].matrix[i][2] = 256;
        settings->coef_mp[2].matrix[i][3] = 0;
        settings->coef_mp[2].matrix[i][4] = 0;
    }
    for(i=0; i<32; i++)
    {
        settings->coef_mp[3].matrix[i][0] = 0;
        settings->coef_mp[3].matrix[i][1] = 0;
        settings->coef_mp[3].matrix[i][2] = 0;
        settings->coef_mp[3].matrix[i][3] = 256;
        settings->coef_mp[3].matrix[i][4] = 0;
    }

    /* Be default, 5-tap filter */
    settings->cfg_Kernel[0].Sz_height = 5;
    settings->cfg_Kernel[0].Tpad_sz = 2;
    settings->cfg_Kernel[0].Bpad_sz = 2;

    /* Initializing all scaler outputs to defaults */
    for (i = 0u; i < SCALER_NUM_PIPES; i ++)
    {
        settings->unitParams[i].threadMap = 0;
        settings->unitParams[i].coefShift = 8;

        settings->unitParams[i].filter_mode = 1;

        settings->unitParams[i].phase_mode = 0;
        settings->unitParams[i].hs_coef_sel = 0;
        settings->unitParams[i].vs_coef_sel = 0;

        settings->unitParams[i].x_offset = 0;
        settings->unitParams[i].y_offset = 0;
    }
}


static vx_status tivxVpacMscScaleUpdateOutputSettings(tivxImgMosaicMscInstObj *prms, uint32_t ow, uint32_t oh, uint32_t cnt, uint32_t h_divider, vx_df_image format)
{
    vx_status status = (vx_status)VX_SUCCESS;
    float temp_horzAccInit, temp_vertAccInit;
    uint32_t int_horzAccInit, int_vertAccInit;
    uint32_t temp_cropStartX, temp_cropStartY;
    uint32_t iw = prms->crop_width;
    uint32_t ih = prms->crop_height/h_divider;
    uint32_t hzScale = (4096*iw+(ow>>1))/ow;
    uint32_t vtScale = (4096*ih+(oh>>1))/oh;

    if(hzScale > 16384U)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Output %d: max horizontal downscale exceeded, limit is 1/4\n", cnt);
        status = (vx_status)VX_FAILURE;
    }

    if(vtScale > 16384U)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Output %d: max vertical downscale exceeded, limit is 1/4\n", cnt);
        status = (vx_status)VX_FAILURE;
    }

    prms->config.settings.unitParams[cnt].outWidth = ow;
    prms->config.settings.unitParams[cnt].outHeight = oh;
    prms->config.settings.unitParams[cnt].hzScale = hzScale;
    prms->config.settings.unitParams[cnt].vtScale = vtScale;

    if((TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_offset_x) ||
       (TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_init_phase_x))
    {
        temp_horzAccInit = (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        int_horzAccInit = (uint32_t)temp_horzAccInit;
        temp_cropStartX = 0;
        if(int_horzAccInit > 4095U)
        {
            int_horzAccInit -= 4096U;
            temp_cropStartX = 1U;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_init_phase_x)
        {
            prms->config.settings.unitParams[cnt].initPhaseX = int_horzAccInit;
        }
        else
        {
            prms->config.settings.unitParams[cnt].initPhaseX = prms->user_init_phase_x;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_offset_x)
        {
            prms->config.settings.unitParams[cnt].x_offset = prms->user_crop_start_x + temp_cropStartX;
        }
        else
        {
            prms->config.settings.unitParams[cnt].x_offset = prms->user_crop_start_x + prms->user_offset_x;
        }

        /* TIOVX-1129: If NV12, x_offset should be an even number to not flip the chroma channels */
        if ((format == (vx_df_image)VX_DF_IMAGE_NV12) && ((prms->config.settings.unitParams[cnt].x_offset & 1U) == 1U))
        {
            prms->config.settings.unitParams[cnt].x_offset--;
            prms->config.settings.unitParams[cnt].initPhaseX = 4095U;
        }
    }
    else
    {
        prms->config.settings.unitParams[cnt].initPhaseX = prms->user_init_phase_x;
        prms->config.settings.unitParams[cnt].x_offset = prms->user_crop_start_x + prms->user_offset_x;
    }

    if((TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_offset_y) ||
       (TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_init_phase_y))
    {
        temp_vertAccInit = (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        int_vertAccInit = (uint32_t)temp_vertAccInit;
        temp_cropStartY = 0;
        if(int_vertAccInit > 4095U)
        {
            int_vertAccInit -= 4096U;
            temp_cropStartY = 1U;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_init_phase_y)
        {
            prms->config.settings.unitParams[cnt].initPhaseY = int_vertAccInit;
        }
        else
        {
            prms->config.settings.unitParams[cnt].initPhaseY = prms->user_init_phase_y;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == prms->user_offset_y)
        {
            prms->config.settings.unitParams[cnt].y_offset = prms->user_crop_start_y + temp_cropStartY;
        }
        else
        {
            prms->config.settings.unitParams[cnt].y_offset = prms->user_crop_start_y + prms->user_offset_y;
        }
    }
    else
    {
        prms->config.settings.unitParams[cnt].initPhaseY = prms->user_init_phase_y;
        prms->config.settings.unitParams[cnt].y_offset = prms->user_crop_start_y + prms->user_offset_y;
    }

    return status;
}

