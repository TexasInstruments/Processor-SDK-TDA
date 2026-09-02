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

#include <TI/tivx.h>
#include <TI/tivx_event.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <TI/hwa_vpac_msc.h>

#include <tivx_kernels_target_utils.h>
#include <tivx_img_mosaic_host.h>
#include <include/vhwa_m2mMsc.h>
#include <utils/mem/include/app_mem.h>

#include <utils/perf_stats/include/app_perf_stats.h>

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
    Vhwa_M2mMscCreatePrms   createArgs;
    Vhwa_M2mMscParams       msc_prms;
    Fvid2_Handle            handle;
    tivx_event              wait_for_compl;

    Fvid2_FrameList         inFrmList;
    Fvid2_FrameList         outFrmList;
    Fvid2_Frame             inFrm;
    Fvid2_Frame             outFrm;
    Fvid2_CbParams          cbPrms;

    uint32_t                msc_drv_inst_id;

    uint64_t                time;

    uint32_t                scIdx;
} tivxImgMosaicMscInstObj;

typedef struct
{
    tivxImgMosaicMscInstObj inst_obj[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

    Msc_Coeff               coeffCfg;

    tivxImgMosaicWindow    *tmp_win[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    uint32_t                tmp_in_ch[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    uint32_t                tmp_ch_sel[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

    tivx_obj_desc_object_array_t *in_arr_desc[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];
    tivx_obj_desc_image_t        *in_img_desc[TIVX_IMAGE_MOSAIC_MSC_MAX_INST][TIVX_OBJECT_ARRAY_MAX_ITEMS];

    app_perf_hwa_id_t       app_hwa_inst_id[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

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
static vx_status tivxKernelImgMosaicMscDrvCreate(
    tivxImgMosaicMscInstObj *inst_obj, uint32_t inst_id);
static vx_status tivxKernelImgMosaicMscDrvDelete(
    tivxImgMosaicMscInstObj *inst_obj);
static void tivxKernelImgMosaicMscDrvSetScCfg(
    Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window);
static int32_t tivxKernelImgMosaicMscDrvCompCb(
    Fvid2_Handle handle,
    void *appData);
static vx_status tivxKernelImgMosaicMscSetCoeffsCmd(tivxImgMosaicMscObj *msc_obj);
static void scale_set_coeff(
    tivx_vpac_msc_coefficients_t *coeff,
    uint32_t interpolation);

static vx_status tivxKernelImgMosaicMscDrvPrepare(
    tivxImgMosaicMscInstObj *inst_obj,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window,
    const tivx_obj_desc_image_t* out_img_desc);
static vx_status tivxKernelImgMosaicMscDrvSubmit(tivxImgMosaicMscInstObj *inst_obj);
static void tivxKernelImgMosaicMscDrvWait(tivxImgMosaicMscInstObj *inst_obj);
static vx_status tivxKernelImgMosaicMscDrvGetReq(tivxImgMosaicMscInstObj *inst_obj);


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
    char target_name[TIVX_IMAGE_MOSAIC_MSC_MAX_INST][TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    vx_status status = (vx_status)VX_SUCCESS;

    self_cpu = tivxGetSelfCpuId();

    if ((vx_enum)TIVX_CPU_ID_VPAC1 == self_cpu )
    {
        /* Both scalars are used, but the target used it always MSC0 */
        strncpy(target_name[0], TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_MAX_NAME);
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
    {
        /* Both scalars are used, but the target used it always MSC0 */
        strncpy(target_name[0], TIVX_TARGET_VPAC2_MSC1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_VPAC2_MSC2, TIVX_TARGET_MAX_NAME);
    }
    #endif
    #if defined(SOC_TDA54)
    else if ((vx_enum)TIVX_CPU_ID_VPAC3 == self_cpu)
    {
        /* Both scalars are used, but the target used it always MSC0 */
        strncpy(target_name[0], TIVX_TARGET_VPAC3_MSC1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_VPAC3_MSC2, TIVX_TARGET_MAX_NAME);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_imgMosaicMsc1_kernel = tivxAddTargetKernelByName(
                                     TIVX_KERNEL_IMG_MOSAIC_NAME,
                                     target_name[0],
                                     tivxKernelImgMosaicMscProcess,
                                     tivxKernelImgMosaicMscCreate,
                                     tivxKernelImgMosaicMscDelete,
                                     NULL,
                                     NULL);
        if(NULL == vx_imgMosaicMsc1_kernel)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Add Img Mosaic MSC TargetKernel\n");
        }

        vx_imgMosaicMsc2_kernel = tivxAddTargetKernelByName(
                                     TIVX_KERNEL_IMG_MOSAIC_NAME,
                                     target_name[1],
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
            for(win = 0U; win < params->num_windows; win++)
            {
                tivxImgMosaicWindow *window =
                    (tivxImgMosaicWindow *) &params->windows[win];
                vx_int32 in = window->input_select;
                vx_int32 ch = window->channel_select;

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
        vx_enum                 self_cpu;
        app_perf_hwa_id_t       app_hwa_inst_id_0 = 0U, app_hwa_inst_id_1 = 0U;
        uint32_t                inst_id_offset = 0U;

        self_cpu = tivxGetSelfCpuId();

        if ((vx_enum)TIVX_CPU_ID_VPAC1 == self_cpu)
        {
            app_hwa_inst_id_0   = APP_PERF_HWA_VPAC1_MSC0;
            app_hwa_inst_id_1   = APP_PERF_HWA_VPAC1_MSC1;
            inst_id_offset      = 0U;
        }
        #if defined(SOC_J784S4) || defined(SOC_J742S2)
        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
        {
            app_hwa_inst_id_0   = APP_PERF_HWA_VPAC2_MSC0;
            app_hwa_inst_id_1   = APP_PERF_HWA_VPAC2_MSC1;
            inst_id_offset = 2U;
        }
        #endif
        #if defined(SOC_TDA54)
        else if ((vx_enum)TIVX_CPU_ID_VPAC3 == self_cpu)
        {
            app_hwa_inst_id_0   = APP_PERF_HWA_VPAC3_MSC0;
            app_hwa_inst_id_1   = APP_PERF_HWA_VPAC3_MSC1;
            inst_id_offset = 2U;
        }
        #endif

        if (msc_obj->max_msc_instances == 2)
        {
            msc_obj->app_hwa_inst_id[0] = app_hwa_inst_id_0;
            msc_obj->app_hwa_inst_id[1] = app_hwa_inst_id_1;
        }
        else if(msc_obj->max_msc_instances == 1)
        {
            if(msc_obj->msc_instance == 0)
            {
                msc_obj->app_hwa_inst_id[0] = app_hwa_inst_id_0;
            }
            else if (msc_obj->msc_instance == 1)
            {
                msc_obj->app_hwa_inst_id[0] = app_hwa_inst_id_1;
            }
        }

        for (i = 0u; i < msc_obj->max_msc_instances; i ++)
        {
            if (0u == i)
            {
                msc_obj->inst_obj[i].scIdx = i;
            }
            else
            {
                msc_obj->inst_obj[i].scIdx = MSC_MAX_OUTPUT - 1U - i;
            }

            /* MSC driver create */
            status = tivxKernelImgMosaicMscDrvCreate(&msc_obj->inst_obj[i], i+inst_id_offset);
        }
    }

    if(VX_SUCCESS == status)
    {
        status = tivxKernelImgMosaicMscSetCoeffsCmd(msc_obj);
    }

    /* Free the allocated memory incase of error */
    if(VX_SUCCESS != status)
    {
        if(msc_obj != NULL)
        {
            for (i = 0u; i < msc_obj->max_msc_instances; i ++)
            {
                /* MSC driver delete */
                status |= tivxKernelImgMosaicMscDrvDelete(
                    &msc_obj->inst_obj[i]);
            }

            tivxMemFree(msc_obj, sizeof(tivxImgMosaicMscObj), TIVX_MEM_EXTERNAL);
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
        for (i = 0u; i < msc_obj->max_msc_instances; i ++)
        {
            status = tivxKernelImgMosaicMscDrvDelete(
                &msc_obj->inst_obj[i]);
        }

        tivxMemFree(msc_obj, sizeof(tivxImgMosaicMscObj), TIVX_MEM_EXTERNAL);
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
    tivx_obj_desc_image_t            *in_img_desc;
    void                             *output_image_target_ptr[2U];
    vx_int32                          win, in, ch, msc_cnt;
    vx_int32                          num_inputs;
    tivxImgMosaicParams              *params;
    tivxImgMosaicMscObj              *msc_obj = NULL;
    vx_uint32                         size, num_inst;
    uint32_t                         do_process[TIVX_IMAGE_MOSAIC_MSC_MAX_INST];

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
        tivxMemBufferMap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);

        /* Map output image CbCr buffer */
        if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
        {
            output_image_target_ptr[1U] = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[1U]);
            tivxMemBufferMap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
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

                    status = tivxKernelImgMosaicMscDrvPrepare(
                        &msc_obj->inst_obj[msc_cnt],
                        msc_obj->in_img_desc[msc_cnt][ch],
                        msc_obj->tmp_win[msc_cnt], out_img_desc);

                    if (VX_SUCCESS != status)
                    {
                        break;
                    }

                    msc_obj->tmp_in_ch[msc_cnt] = in;
                    msc_obj->tmp_ch_sel[msc_cnt] = ch;
                    do_process[msc_cnt] = 1;
                }
                else
                {
                    do_process[msc_cnt] = 0;
                }
            }
            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                if (do_process[msc_cnt])
                {
                    msc_obj->inst_obj[msc_cnt].time =
                        tivxPlatformGetTimeInUsecs();
                }
            }

            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                if (do_process[msc_cnt])
                {
                    status = tivxKernelImgMosaicMscDrvSubmit(
                        &msc_obj->inst_obj[msc_cnt]);
                }
            }

            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                if (do_process[msc_cnt])
                {
                    tivxKernelImgMosaicMscDrvWait(
                        &msc_obj->inst_obj[msc_cnt]);
                }
            }

            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                if (do_process[msc_cnt])
                {
                    status = tivxKernelImgMosaicMscDrvGetReq(
                        &msc_obj->inst_obj[msc_cnt]);
                }
            }

            for (msc_cnt = 0u; (msc_cnt < num_inst) &&
                    (VX_SUCCESS == status); msc_cnt ++)
            {
                if (do_process[msc_cnt])
                {
                    msc_obj->inst_obj[msc_cnt].time =
                        tivxPlatformGetTimeInUsecs() - msc_obj->inst_obj[msc_cnt].time;

                    ch = msc_obj->tmp_win[msc_cnt]->channel_select;

                    in_img_desc = msc_obj->in_img_desc[msc_cnt][ch];
                    size = (in_img_desc->imagepatch_addr[0].dim_x *
                        in_img_desc->imagepatch_addr[0].dim_y) +
                        (in_img_desc->imagepatch_addr[1].dim_x *
                        in_img_desc->imagepatch_addr[1].dim_y);

                    appPerfStatsHwaUpdateLoad(msc_obj->app_hwa_inst_id[msc_cnt],
                        msc_obj->inst_obj[msc_cnt].time, size/* pixels processed */);
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
        tivxMemBufferUnmap(output_image_target_ptr[0U], out_img_desc->mem_size[0U], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        if(out_img_desc->mem_ptr[1U].shared_ptr != (uint64_t)NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1U], out_img_desc->mem_size[1U], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        }
    }

    return (status);
}

/* ========================================================================== */
/*                              Local Functions                               */
/* ========================================================================== */

static vx_status tivxKernelImgMosaicMscDrvCreate(tivxImgMosaicMscInstObj *inst_obj, uint32_t inst_id)
{
    vx_status   status = VX_SUCCESS;

    /* Hard code to first instance */
    inst_obj->msc_drv_inst_id = inst_id;

    Vhwa_M2mMscCreatePrmsInit(&inst_obj->createArgs);
    status = tivxEventCreate(&inst_obj->wait_for_compl);
    if(VX_SUCCESS == status)
    {
        inst_obj->cbPrms.cbFxn   = tivxKernelImgMosaicMscDrvCompCb;
        inst_obj->cbPrms.appData = inst_obj;

        inst_obj->handle = Fvid2_create(
            FVID2_VHWA_M2M_MSC_DRV_ID,
            inst_obj->msc_drv_inst_id,
            &inst_obj->createArgs,
            NULL,
            &inst_obj->cbPrms);
        if(NULL == inst_obj->handle)
        {
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_create failed\n");
            status = VX_ERROR_NO_RESOURCES;
        }
        else
        {
            Fvid2Frame_init(&inst_obj->inFrm);
            Fvid2Frame_init(&inst_obj->outFrm);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
    }

    return (status);
}

static vx_status tivxKernelImgMosaicMscDrvDelete(tivxImgMosaicMscInstObj *inst_obj)
{
    vx_status   status = VX_SUCCESS;

    if(NULL != inst_obj->handle)
    {
        Fvid2_delete(inst_obj->handle, NULL);
        inst_obj->handle = NULL;
    }
    if(NULL != inst_obj->wait_for_compl)
    {
        tivxEventDelete(&inst_obj->wait_for_compl);
        inst_obj->wait_for_compl = NULL;
    }

    return (status);
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxKernelImgMosaicMscSetCoeffsCmd(tivxImgMosaicMscObj *msc_obj)
{
    vx_status                         status = VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          cnt;
    tivx_vpac_msc_coefficients_t      coeffs;
    Msc_Coeff                        *coeffCfg;

    scale_set_coeff(&coeffs, VX_INTERPOLATION_BILINEAR);

    coeffCfg = &msc_obj->coeffCfg;

    Msc_coeffInit(coeffCfg);

    for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
    {
        coeffCfg->spCoeffSet[cnt] = &coeffs.single_phase[cnt][0u];
    }

    for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
    {
        coeffCfg->mpCoeffSet[cnt] = &coeffs.multi_phase[cnt][0u];
    }

    if (VX_SUCCESS == status)
    {
        fvid2_status = Fvid2_control(msc_obj->inst_obj[0u].handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            coeffCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxKernelImgMosaicMscSetCoeffsCmd: Failed to create coefficients\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}


static vx_status tivxKernelImgMosaicMscDrvPrepare(
    tivxImgMosaicMscInstObj *inst_obj,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window,
    const tivx_obj_desc_image_t* out_img_desc)
{
    vx_status           status = VX_SUCCESS;
    Vhwa_M2mMscParams  *msc_prms;
    Fvid2_Format       *fmt;
    Msc_ScConfig       *sc_cfg;
    Fvid2_Frame        *frm;
    uint32_t            plane_cnt;
    vx_uint32           startX, startY;
    uint32_t            scIdx;

    msc_prms = &inst_obj->msc_prms;
    Vhwa_m2mMscParamsInit(msc_prms);

    /* Set input format */
    fmt = &msc_prms->inFmt;
    scIdx = inst_obj->scIdx;

    if(in_img_desc->format == VX_DF_IMAGE_U8 ||in_img_desc->format == VX_DF_IMAGE_U16)
    {
        fmt->dataFormat = FVID2_DF_LUMA_ONLY;
    }
    else
    {
        fmt->dataFormat = FVID2_DF_YUV420SP_UV;
    }

    if(in_img_desc->format == VX_DF_IMAGE_U16)
    {
        fmt->ccsFormat  = FVID2_CCSF_BITS12_UNPACKED16;
    }
    else
    {
        fmt->ccsFormat  = FVID2_CCSF_BITS8_PACKED;
    }

    fmt->width      = in_img_desc->imagepatch_addr[0U].dim_x;
    fmt->height     = in_img_desc->imagepatch_addr[0U].dim_y;
    fmt->pitch[0U]  = in_img_desc->imagepatch_addr[0U].stride_y;
    fmt->pitch[1U]  = in_img_desc->imagepatch_addr[1U].stride_y;

    /* Set output format */
    sc_cfg = &msc_prms->mscCfg.scCfg[scIdx];       /* Only one output used */
    tivxKernelImgMosaicMscDrvSetScCfg(sc_cfg, in_img_desc, window);
    fmt = &msc_prms->outFmt[scIdx];                /* Only one output used */

    if(out_img_desc->format == VX_DF_IMAGE_U8 ||in_img_desc->format == VX_DF_IMAGE_U16)
    {
        fmt->dataFormat = FVID2_DF_LUMA_ONLY;
    }
    else
    {
        fmt->dataFormat = FVID2_DF_YUV420SP_UV;
    }

    if(out_img_desc->format == VX_DF_IMAGE_U16)
    {
        fmt->ccsFormat  = FVID2_CCSF_BITS12_UNPACKED16;
    }
    else
    {
        fmt->ccsFormat  = FVID2_CCSF_BITS8_PACKED;
    }

    fmt->width      = sc_cfg->outWidth;
    fmt->height     = sc_cfg->outHeight;
    /* Use the output overlay pitch */
    fmt->pitch[0U]  = out_img_desc->imagepatch_addr[0U].stride_y;
    fmt->pitch[1U]  = out_img_desc->imagepatch_addr[1U].stride_y;

    status = Fvid2_control(
        inst_obj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS, msc_prms, NULL);
    if(FVID2_SOK != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params\n");
        status = VX_FAILURE;
    }
    else
    {
        status = VX_SUCCESS;
    }

    if(VX_SUCCESS == status)
    {
        frm = &inst_obj->inFrm;
        for(plane_cnt = 0U; plane_cnt < TIVX_IMAGE_MOSAIC_MAX_PLANES; plane_cnt++)
        {
            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
        }

        /* Adjust start X/Y so that we center the input when input size is less
         * than the window size */
        startX = window->startX + ((window->width  - sc_cfg->outWidth)  >> 1U);
        startY = window->startY + ((window->height - sc_cfg->outHeight) >> 1U);
        startX = ((startX >> 1U) << 1U);    /* Make it even for YUV420SP */
        startY = ((startY >> 1U) << 1U);    /* Make it even for YUV420SP */
        frm = &inst_obj->outFrm;
        for(plane_cnt = 0U; plane_cnt < TIVX_IMAGE_MOSAIC_MAX_PLANES; plane_cnt++)
        {
            uint64_t    temp;
            temp = tivxMemShared2PhysPtr(
                       out_img_desc->mem_ptr[plane_cnt].shared_ptr,
                       out_img_desc->mem_ptr[plane_cnt].mem_heap_region);
            /* Manipulate output overlay pointer to point to right offset
             * to do scaling */
            if(0U == plane_cnt)
            {
                frm->addr[plane_cnt] = temp +
                    (startY * out_img_desc->imagepatch_addr[0U].stride_y) +
                    startX;
            }
            else
            {
                frm->addr[plane_cnt] = temp +
                    ((startY >> 1U) * out_img_desc->imagepatch_addr[0U].stride_y) +
                    startX;
            }
        }
    }

    return (status);
}

static vx_status tivxKernelImgMosaicMscDrvSubmit(tivxImgMosaicMscInstObj *inst_obj)
{
    vx_status           status = VX_SUCCESS;

    Fvid2FrameList_init(&inst_obj->inFrmList);
    Fvid2FrameList_init(&inst_obj->outFrmList);

    /* Set up Framelist */
    inst_obj->inFrmList.numFrames = 1U;
    inst_obj->inFrmList.frames[0U] = &inst_obj->inFrm;
    inst_obj->outFrmList.numFrames = MSC_MAX_OUTPUT;
    inst_obj->outFrmList.frames[inst_obj->scIdx] = &inst_obj->outFrm;

    /* Submit MSC Request*/
    status = Fvid2_processRequest(
                 inst_obj->handle,
                 &inst_obj->inFrmList,
                 &inst_obj->outFrmList,
                 FVID2_TIMEOUT_FOREVER);
    if(FVID2_SOK != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
        status = VX_FAILURE;
    }

    return (status);
}

static void tivxKernelImgMosaicMscDrvWait(tivxImgMosaicMscInstObj *inst_obj)
{
    /* Wait for Frame Completion */
    tivxEventWait(inst_obj->wait_for_compl, VX_TIMEOUT_WAIT_FOREVER);
}

static vx_status tivxKernelImgMosaicMscDrvGetReq(tivxImgMosaicMscInstObj *inst_obj)
{
    vx_status           status = VX_SUCCESS;

    status = Fvid2_getProcessedRequest(
                 inst_obj->handle,
                 &inst_obj->inFrmList,
                 &inst_obj->outFrmList,
                 0U);
    if(FVID2_SOK != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
        /* status = VX_FAILURE; */
    }

    return (status);
}

static void tivxKernelImgMosaicMscDrvSetScCfg(
    Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivxImgMosaicWindow *window)
{
    sc_cfg->enable = UTRUE;
    if(window->width > in_img_desc->imagepatch_addr[0U].dim_x)
    {
        /* Upscaling not supported - set MSC width same as input */
        sc_cfg->outWidth = in_img_desc->imagepatch_addr[0U].dim_x;
    }
    else
    {
        sc_cfg->outWidth = window->width;
    }
    if(window->height > in_img_desc->imagepatch_addr[0U].dim_y)
    {
        /* Upscaling not supported - set MSC height same as input */
        sc_cfg->outHeight = in_img_desc->imagepatch_addr[0U].dim_y;
    }
    else
    {
        sc_cfg->outHeight = window->height;
    }
    /* Crop setting - same as input size */
    if(window->enable_roi == 0)
    {
        sc_cfg->inRoi.cropStartX = 0U;
        sc_cfg->inRoi.cropStartY = 0U;
        sc_cfg->inRoi.cropWidth  = in_img_desc->imagepatch_addr[0U].dim_x;
        sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0U].dim_y;
    }
    else
    {
        sc_cfg->inRoi.cropStartX = window->roiStartX;
        sc_cfg->inRoi.cropStartY = window->roiStartY;
        sc_cfg->inRoi.cropWidth  = window->roiWidth;
        sc_cfg->inRoi.cropHeight = window->roiHeight;
    }

    /* For Scale, multi phase coefficients are used. */
    sc_cfg->filtMode     = MSC_FILTER_MODE_MULTI_PHASE;
    sc_cfg->phaseMode    = MSC_PHASE_MODE_64PHASE;
    sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
    sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
    sc_cfg->coeffShift   = MSC_COEFF_SHIFT_8;
}

static void scale_set_coeff(
    tivx_vpac_msc_coefficients_t *coeff,
    uint32_t interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 256;
    coeff->single_phase[0][idx++] = 0;
    coeff->single_phase[0][idx++] = 0;
    idx = 0;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 256;
    coeff->single_phase[1][idx++] = 0;
    coeff->single_phase[1][idx++] = 0;

    if(VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 256-weight;
            coeff->multi_phase[0][idx++] = weight;
            coeff->multi_phase[0][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 256-weight;
            coeff->multi_phase[1][idx++] = weight;
            coeff->multi_phase[1][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 256-weight;
            coeff->multi_phase[2][idx++] = weight;
            coeff->multi_phase[2][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 256-weight;
            coeff->multi_phase[3][idx++] = weight;
            coeff->multi_phase[3][idx++] = 0;
        }
    }
    else /* STR_VX_INTERPOLATION_NEAREST_NEIGHBOR */
    {
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 256;
            coeff->multi_phase[0][idx++] = 0;
            coeff->multi_phase[0][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 0;
            coeff->multi_phase[1][idx++] = 256;
            coeff->multi_phase[1][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 256;
            coeff->multi_phase[2][idx++] = 0;
            coeff->multi_phase[2][idx++] = 0;
        }
        idx = 0;
        for(i = 0; i < 32; i++)
        {
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 0;
            coeff->multi_phase[3][idx++] = 256;
            coeff->multi_phase[3][idx++] = 0;
        }
    }
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

static int32_t tivxKernelImgMosaicMscDrvCompCb(Fvid2_Handle handle, void *appData)
{
    tivxImgMosaicMscInstObj *inst_obj = (tivxImgMosaicMscInstObj *) appData;

    if(NULL != inst_obj)
    {
        tivxEventPost(inst_obj->wait_for_compl);
    }

    return FVID2_SOK;
}
