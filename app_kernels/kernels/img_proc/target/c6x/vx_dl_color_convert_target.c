/*
*
* Copyright (c) 2022-2026 Texas Instruments Incorporated
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
#include <tivx_kernels_target_utils.h>
#include <utils/udma/include/app_udma_utils.h>
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include <ti/vxlib/vxlib.h>

#include <tivx_dl_color_convert_host.h>
#include "vx_dma_transfers.h"

#define EXEC_PIPELINE_STAGE1(x) ((x) & 0x00000001)
#define EXEC_PIPELINE_STAGE2(x) ((x) & 0x00000002)
#define EXEC_PIPELINE_STAGE3(x) ((x) & 0x00000004)

#define J721E_C66X_DSP_L2_SRAM_SIZE  (224 * 1024) /* size in bytes */

typedef struct {
    sTransferGroup inTfrs;
    sTransferGroup outTfrs;

    VXLIB_bufParams2D_t vxlib_src[TIVX_IMAGE_MAX_PLANES];
    VXLIB_bufParams2D_t vxlib_dst[TIVX_IMAGE_MAX_PLANES];

    vx_uint32 tile_width;
    vx_uint32 tile_height;
    vx_uint32 num_sets;

    uint8_t *pL2;
    uint8_t *pScratch;
    uint32_t l2_heap_id;
    uint64_t l2_global_base;
    uint32_t alloc_size;
    uint32_t scratch_size;

} tivxDLColorConvertKernelParams;

static tivx_target_kernel vx_dl_color_convert_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDLColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelDLColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelDLColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelDLColorConvertCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* tivx_set_debug_zone(VX_ZONE_INFO); */

    if (num_params != TIVX_DL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        uint32_t i;

        for (i = 0U; i < TIVX_DL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    tivxDLColorConvertKernelParams * kernelParams = NULL;
    kernelParams = tivxMemAlloc(sizeof(tivxDLColorConvertKernelParams), TIVX_MEM_EXTERNAL);

    if (NULL == kernelParams)
    {
        status = VX_FAILURE;
    }
    else
    {
        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        tivx_obj_desc_image_t *out_img_desc;
        void* out_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        vx_int32 i;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_INPUT_IDX];
        for (i = 0; i < in_img_desc->planes; i++)
        {
            VX_PRINT(VX_ZONE_INFO, "Input channel %d\n", i);
            if(in_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                in_image_target_ptr[i]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[i]);
                tivxMemBufferMap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[i].dim_x);
                VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[i].dim_y);
                VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[i].stride_y);
                VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[i].stride_x);
                VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[i].step_x);
                VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[i].step_y);
            }
        }

        out_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_OUTPUT_IDX];
        for (i = 0; i < out_img_desc->planes; i++)
        {
            VX_PRINT(VX_ZONE_INFO, "Output channel %d\n", i);
            if(out_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                out_image_target_ptr[i]  = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[i]);
                tivxMemBufferMap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[i].dim_x);
                VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[i].dim_y);
                VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[i].stride_y);
                VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[i].stride_x);
                VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[i].step_x);
                VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[i].step_y);
            }
        }

        tivx_mem_stats l2_stats;
        tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
        tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);

        vx_uint32 req_size, avail_size;
        avail_size = l2_stats.free_size;
        req_size = J721E_C66X_DSP_L2_SRAM_SIZE; /* Out of available 288KB - 64KB is cache and 224KB is SRAM */

        VX_PRINT(VX_ZONE_INFO, "req_size = %d\n", req_size);
        VX_PRINT(VX_ZONE_INFO, "avail_size = %d\n", avail_size);

        if(req_size > avail_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "Setting req_size as avail_size = %d\n", avail_size);
            req_size = avail_size;
        }

        /* Amount of L2 to be allocated will be required size */
        kernelParams->alloc_size = req_size;
        kernelParams->l2_heap_id = TIVX_MEM_INTERNAL_L2; /* TIVX_MEM_INTERNAL_L2 or TIVX_MEM_EXTERNAL */
        kernelParams->l2_global_base = 0;

        if(kernelParams->l2_heap_id==TIVX_MEM_INTERNAL_L2)
        {
#ifndef x86_64
            if(appIpcGetSelfCpuId()==APP_IPC_CPU_C6x_1)
            {
                kernelParams->l2_global_base = 0x4D80000000;
            }
            else
            {
                kernelParams->l2_global_base = 0x4D81000000;
            }
#else
            kernelParams->l2_global_base = 0;
#endif
        }

        kernelParams->pL2 = tivxMemAlloc(kernelParams->alloc_size, kernelParams->l2_heap_id);
        VX_PRINT(VX_ZONE_INFO, "kernelParams->pL2 = 0x%08X\n", kernelParams->pL2);
        VX_PRINT(VX_ZONE_INFO, "kernelParams->l2_global_base = 0x%08X\n", kernelParams->l2_global_base);

        if(kernelParams->pL2 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate L2 scratch!\n");
            status = VX_FAILURE;
        }

        sTransferGroup *inTfrs = (sTransferGroup *)&kernelParams->inTfrs;
        sTransferGroup *outTfrs = (sTransferGroup *)&kernelParams->outTfrs;
        vx_uint32 in, out;

        if (in_img_desc->format == VX_DF_IMAGE_RGB)
        {
            inTfrs->num_transfers = 1;

            inTfrs->blockObj[0].blk_width = in_img_desc->imagepatch_addr[0].dim_x;
            inTfrs->blockObj[0].blk_height = 2;
            inTfrs->blockObj[0].num_elems  = 3;
            inTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);
        }
        else if ((in_img_desc->format == VX_DF_IMAGE_NV12) || (in_img_desc->format == VX_DF_IMAGE_NV21))
        {
            inTfrs->num_transfers = 2;

            inTfrs->blockObj[0].blk_width  = in_img_desc->imagepatch_addr[0].dim_x;
            inTfrs->blockObj[0].blk_height = 2;
            inTfrs->blockObj[0].num_elems  = 1;
            inTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);

            inTfrs->blockObj[1].blk_width  = (in_img_desc->imagepatch_addr[1].dim_x *
                                              in_img_desc->imagepatch_addr[1].stride_x) /
                                              in_img_desc->imagepatch_addr[1].step_x;
            inTfrs->blockObj[1].blk_height = 1;
            inTfrs->blockObj[1].num_elems  = 1; /*Special case where CbCR is adjacent */
            inTfrs->blockObj[1].num_bytes  = sizeof(vx_uint8);
        }
        else if (in_img_desc->format == VX_DF_IMAGE_IYUV)
        {
            inTfrs->num_transfers = 3;

            inTfrs->blockObj[0].blk_width  = in_img_desc->imagepatch_addr[0].dim_x;
            inTfrs->blockObj[0].blk_height = 2;
            inTfrs->blockObj[0].num_elems  = 1;
            inTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);

            inTfrs->blockObj[1].blk_width  = (in_img_desc->imagepatch_addr[1].dim_x *
                                              in_img_desc->imagepatch_addr[1].stride_x) /
                                              in_img_desc->imagepatch_addr[1].step_x;
            inTfrs->blockObj[1].blk_height = 1;
            inTfrs->blockObj[1].num_elems  = 1;
            inTfrs->blockObj[1].num_bytes  = sizeof(vx_uint8);

            inTfrs->blockObj[2].blk_width  = (in_img_desc->imagepatch_addr[2].dim_x *
                                              in_img_desc->imagepatch_addr[2].stride_x) /
                                              in_img_desc->imagepatch_addr[2].step_x;
            inTfrs->blockObj[2].blk_height = 1;
            inTfrs->blockObj[2].num_elems  = 1;
            inTfrs->blockObj[2].num_bytes  = sizeof(vx_uint8);
        }

        if (out_img_desc->format == VX_DF_IMAGE_RGB)
        {
            outTfrs->num_transfers = 1;

            outTfrs->blockObj[0].blk_width = out_img_desc->imagepatch_addr[0].dim_x;
            outTfrs->blockObj[0].blk_height = 2;
            outTfrs->blockObj[0].num_elems  = 3;
            outTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);
        }
        else if (out_img_desc->format == VX_DF_IMAGE_NV12)
        {
            outTfrs->num_transfers = 2;

            outTfrs->blockObj[0].blk_width  = out_img_desc->imagepatch_addr[0].dim_x;
            outTfrs->blockObj[0].blk_height = 2;
            outTfrs->blockObj[0].num_elems  = 1;
            outTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);

            outTfrs->blockObj[1].blk_width  = (out_img_desc->imagepatch_addr[1].dim_x *
                                               out_img_desc->imagepatch_addr[1].stride_x) /
                                               out_img_desc->imagepatch_addr[1].step_x;
            outTfrs->blockObj[1].blk_height = 1;
            outTfrs->blockObj[1].num_elems  = 1; /*Special case where CbCR is adjacent */
            outTfrs->blockObj[1].num_bytes  = sizeof(vx_uint8);
        }
        else if (out_img_desc->format == VX_DF_IMAGE_IYUV)
        {
            outTfrs->num_transfers = 3;

            outTfrs->blockObj[0].blk_width  = out_img_desc->imagepatch_addr[0].dim_x;
            outTfrs->blockObj[0].blk_height = 2;
            outTfrs->blockObj[0].num_elems  = 1;
            outTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);

            outTfrs->blockObj[1].blk_width  = (out_img_desc->imagepatch_addr[1].dim_x *
                                               out_img_desc->imagepatch_addr[1].stride_x) /
                                               out_img_desc->imagepatch_addr[1].step_x;
            outTfrs->blockObj[1].blk_height = 1;
            outTfrs->blockObj[1].num_elems  = 1;
            outTfrs->blockObj[1].num_bytes  = sizeof(vx_uint8);

            outTfrs->blockObj[2].blk_width  = (out_img_desc->imagepatch_addr[2].dim_x *
                                               out_img_desc->imagepatch_addr[2].stride_x) /
                                               out_img_desc->imagepatch_addr[2].step_x;
            outTfrs->blockObj[2].blk_height = 1;
            outTfrs->blockObj[2].num_elems  = 1;
            outTfrs->blockObj[2].num_bytes  = sizeof(vx_uint8);
        }

        vx_uint32 in_size, out_size, scratch_size, total_size;
        vx_uint32 max_height, rem_height, mblk;
        vx_uint32 num_sets;

        /* This logic tries to find optimum block height which is a
           multiple of 2. It is assumed that DMA will fetch a minimum of
           Width x block_height */
        total_size = 0;
        rem_height = 0;
        in_size = 0;
        out_size = 0;
        num_sets = 0;
        mblk = 1;
        max_height = in_img_desc->imagepatch_addr[0].dim_y;

        if ( (((vx_df_image)VX_DF_IMAGE_RGB == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format)) ||
             (((vx_df_image)VX_DF_IMAGE_RGB == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_IYUV == out_img_desc->format)) )
        {
            scratch_size = 4U * in_img_desc->imagepatch_addr[0].dim_x * sizeof(uint8_t);
        }
        else
        {
            scratch_size = 0;
        }

        kernelParams->scratch_size = scratch_size;

        /* Iteratively estimate an optimal block size to accommodate input/output blocks */
        do
        {
            /* Increment block height multiplier by 2 */
            mblk *= 2;

            in_size  = 0;
            for(in = 0; in < inTfrs->num_transfers; in++)
            {
                sTransferBlock *block = &inTfrs->blockObj[in];
                in_size += (block->blk_width * block->blk_height * mblk * block->num_elems * block->num_bytes);
            }

            out_size = 0;
            for(out = 0; out < outTfrs->num_transfers; out++)
            {
                sTransferBlock *block = &outTfrs->blockObj[out];
                out_size += (block->blk_width * block->blk_height * mblk * block->num_elems * block->num_bytes);
            }

            /* Double buffer inputs and outputs*/
            in_size = in_size * 2;
            out_size = out_size * 2;

            total_size = in_size + out_size + scratch_size;
            rem_height = max_height % (mblk * 2);
            num_sets = max_height / (mblk * 2);

            VX_PRINT(VX_ZONE_INFO, "mblk = %d\n", mblk);
            VX_PRINT(VX_ZONE_INFO, "num_sets = %d\n", num_sets);
            VX_PRINT(VX_ZONE_INFO, "rem_height = %d\n", rem_height);
            VX_PRINT(VX_ZONE_INFO, "in_size = %d\n", in_size);
            VX_PRINT(VX_ZONE_INFO, "out_size = %d\n", out_size);
            VX_PRINT(VX_ZONE_INFO, "scratch_size = %d\n", scratch_size);
            VX_PRINT(VX_ZONE_INFO, "total_size = %d\n", total_size);
        }
        while  ((total_size < req_size) && (rem_height == 0) && ((num_sets & 1) == 0));

        /* Upon exit, the mblk would have increased to a value which fails the condition,
           hence latch on the previous passing condition */

        mblk = mblk / 2;
        kernelParams->num_sets = max_height / (mblk * 2);

        VX_PRINT(VX_ZONE_INFO, "Selected mblk = %d\n", mblk);
        VX_PRINT(VX_ZONE_INFO, "Selected num_sets = %d\n", kernelParams->num_sets);

        /* Set the new block sizes */
        for(in = 0; in < inTfrs->num_transfers; in++)
        {
            sTransferBlock *block = &inTfrs->blockObj[in];
            block->blk_height = block->blk_height * mblk;
            VX_PRINT(VX_ZONE_INFO, "inBlock->blk_width[%d] = %d\n", in, block->blk_width);
            VX_PRINT(VX_ZONE_INFO, "inBlock->blk_height[%d] = %d\n", in, block->blk_height);
            VX_PRINT(VX_ZONE_INFO, "inBlock->num_elems[%d] = %d\n", in, block->num_elems);
            VX_PRINT(VX_ZONE_INFO, "inBlock->num_bytes[%d] = %d\n", in, block->num_bytes);
        }

        kernelParams->tile_width  = inTfrs->blockObj[0].blk_width;
        kernelParams->tile_height = inTfrs->blockObj[0].blk_height;

        VX_PRINT(VX_ZONE_INFO, "Selected tile_width = %d\n", kernelParams->tile_width);
        VX_PRINT(VX_ZONE_INFO, "Selected tile_height = %d\n", kernelParams->tile_height);

        for(out = 0; out < outTfrs->num_transfers; out++)
        {
            sTransferBlock *block = &outTfrs->blockObj[out];
            block->blk_height = block->blk_height * mblk;
            VX_PRINT(VX_ZONE_INFO, "outBlock->blk_width[%d] = %d\n", out, block->blk_width);
            VX_PRINT(VX_ZONE_INFO, "outBlock->blk_height[%d] = %d\n", out, block->blk_height);
            VX_PRINT(VX_ZONE_INFO, "outBlock->num_elems[%d] = %d\n", out, block->num_elems);
            VX_PRINT(VX_ZONE_INFO, "outBlock->num_bytes[%d] = %d\n", out, block->num_bytes);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            vx_uint32 ch = 0;

            for(in = 0; in < inTfrs->num_transfers; in++, ch++)
            {
#ifdef x86_64
                dma_create(&inTfrs->dmaObj[in], DATA_COPY_CPU, ch);
#else
                dma_create(&inTfrs->dmaObj[in], DATA_COPY_DMA, J721E_C66X_DRU_START_CHANNEL + ch);
#endif
            }

            for(out = 0; out < outTfrs->num_transfers; out++, ch++)
            {
#ifdef x86_64
                dma_create(&outTfrs->dmaObj[out], DATA_COPY_CPU, ch);
#else
                dma_create(&outTfrs->dmaObj[out], DATA_COPY_DMA, J721E_C66X_DRU_START_CHANNEL + ch);
#endif
            }
        }

        for (i = 0; i < in_img_desc->planes; i++)
        {
            if (in_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }

        for (i = 0; i < out_img_desc->planes; i++)
        {
            if (out_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }

        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxDLColorConvertKernelParams));
    }

    /* tivx_clr_debug_zone(VX_ZONE_INFO); */

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLColorConvertDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (num_params != TIVX_DL_COLOR_CONVERT_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        uint32_t i;

        for (i = 0U; i < TIVX_DL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDLColorConvertKernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            sTransferGroup *inTfrs  = (sTransferGroup *)&prms->inTfrs;
            sTransferGroup *outTfrs = (sTransferGroup *)&prms->outTfrs;
            vx_uint32 in, out;

            for(in = 0; in < inTfrs->num_transfers; in++)
            {
                dma_delete(&inTfrs->dmaObj[in]);
            }

            for(out = 0; out < outTfrs->num_transfers; out++)
            {
                dma_delete(&outTfrs->dmaObj[out]);
            }

            tivxMemFree(prms->pL2, prms->alloc_size, prms->l2_heap_id);

            tivxMemFree(prms, sizeof(tivxDLColorConvertKernelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status dl_color_convert_kernel
(
    tivxDLColorConvertKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc,
    void *pInData[TIVX_IMAGE_MAX_PLANES],
    void *pOutData[TIVX_IMAGE_MAX_PLANES]

)
{
    vx_status status = VX_SUCCESS;

    if (((vx_df_image)VX_DF_IMAGE_RGB == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        status = (vx_status)VXLIB_colorConvert_RGBtoNV12_i8u_o8u((uint8_t *)pInData[0],
            &prms->vxlib_src[0], (uint8_t *)pOutData[0], &prms->vxlib_dst[0], (uint8_t *)pOutData[1], &prms->vxlib_dst[1],
            prms->pScratch, prms->scratch_size);
    }
    else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == in_img_desc->format) ) &&
                ((vx_df_image)VX_DF_IMAGE_RGB == out_img_desc->format))
    {
        uint8_t u_pix = (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;
        vx_enum temp0 = (vx_enum)in_img_desc->color_space - VX_ENUM_BASE((vx_enum)VX_ID_KHRONOS, (vx_enum)VX_ENUM_COLOR_SPACE);
        status = (vx_status)VXLIB_colorConvert_NVXXtoRGB_i8u_o8u((uint8_t *)pInData[0], &prms->vxlib_src[0],
            (uint8_t *)pInData[1], &prms->vxlib_src[1], (uint8_t *)pOutData[0], &prms->vxlib_dst[0], u_pix, (uint8_t)temp0);
    }
    else if ( ( ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) || ((vx_df_image)VX_DF_IMAGE_NV21 == in_img_desc->format) ) &&
                ((vx_df_image)VX_DF_IMAGE_IYUV == out_img_desc->format))
    {
        uint8_t u_pix = (in_img_desc->format == (vx_df_image)VX_DF_IMAGE_NV12) ? 0U : 1U;

        status = (vx_status)VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u((uint8_t *)pInData[0], &prms->vxlib_src[0],
            (uint8_t *)pInData[1], &prms->vxlib_src[1], (uint8_t *)pOutData[0], &prms->vxlib_dst[0], (uint8_t *)pOutData[1],
            &prms->vxlib_dst[1], (uint8_t *)pOutData[2], &prms->vxlib_dst[2], u_pix);
    }
    else if (((vx_df_image)VX_DF_IMAGE_IYUV == in_img_desc->format) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
    {
        status = (vx_status)VXLIB_colorConvert_IYUVtoNV12_i8u_o8u((uint8_t *)pInData[0], &prms->vxlib_src[0],
            (uint8_t *)pInData[1], &prms->vxlib_src[1], (uint8_t *)pInData[2], &prms->vxlib_src[2],
            (uint8_t *)pOutData[0], &prms->vxlib_dst[0], (uint8_t *)pOutData[1], &prms->vxlib_dst[1]);
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VXLIB_SUCCESS != status)
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status dl_color_convert_pipeline_blocks
(
    tivxDLColorConvertKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc,
    void *pInData[TIVX_IMAGE_MAX_PLANES][2],
    void *pOutData[TIVX_IMAGE_MAX_PLANES][2]
)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 pipeup = 2;
    vx_uint32 pipedown = 2;
    vx_uint32 exec_cmd = 1;
    vx_uint32 pipeline = exec_cmd;
    vx_uint32 ping_npong = 0;

    vx_uint32 blk, in, out;

    sTransferGroup *inTfrs  = (sTransferGroup *)&prms->inTfrs;
    sTransferGroup *outTfrs = (sTransferGroup *)&prms->outTfrs;

    uint32_t num_sets = prms->num_sets;

    for(blk = 0; blk < (num_sets + pipeup + pipedown); blk++)
    {
        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            for(in = 0; in < inTfrs->num_transfers; in++)
            {
                dma_transfer_trigger(&inTfrs->dmaObj[in]);
            }
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            for(out = 0; out < outTfrs->num_transfers; out++)
            {
                dma_transfer_trigger(&outTfrs->dmaObj[out]);
            }
        }

        if(EXEC_PIPELINE_STAGE2(pipeline))
        {

            void *pIn[TIVX_IMAGE_MAX_PLANES];
            void *pOut[TIVX_IMAGE_MAX_PLANES];
            vx_int32 i;

            for(i = 0; i < TIVX_IMAGE_MAX_PLANES; i++)
            {
                pIn[i] = pInData[i][ping_npong];
                pOut[i] = pOutData[i][ping_npong];
            }

            dl_color_convert_kernel
            (
                prms,
                in_img_desc,
                out_img_desc,
                pIn,
                pOut
            );

            ping_npong = !ping_npong;
        }

        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            for(in = 0; in < inTfrs->num_transfers; in++)
            {
                dma_transfer_wait(&inTfrs->dmaObj[in]);
            }
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            for(out = 0; out < outTfrs->num_transfers; out++)
            {
                dma_transfer_wait(&outTfrs->dmaObj[out]);
            }
        }

        if(blk == (num_sets - 1))
        {
            exec_cmd = 0;
        }

        pipeline = (pipeline << 1) | exec_cmd;
    }

    return status;
}

static vx_status dl_color_convert_process
(
    tivxDLColorConvertKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc,
    void *in_image_target_ptr[TIVX_IMAGE_MAX_PLANES],
    void *out_image_target_ptr[TIVX_IMAGE_MAX_PLANES]
)
{
    vx_status status = VX_SUCCESS;
    void *pIn[TIVX_IMAGE_MAX_PLANES][2];
    void *pOut[TIVX_IMAGE_MAX_PLANES][2];

    sTransferGroup *inTfrs = (sTransferGroup *)&prms->inTfrs;
    sTransferGroup *outTfrs = (sTransferGroup *)&prms->outTfrs;
    vx_uint32 in, out;

    uint8_t *pL2Base = prms->pL2;

    for(in = 0; in < inTfrs->num_transfers; in++)
    {
        app_udma_copy_nd_prms_t *tfrPrms = (app_udma_copy_nd_prms_t *)&inTfrs->dmaObj[in].tfrPrms;
        vx_uint32 blk_width  = inTfrs->blockObj[in].blk_width;
        vx_uint32 blk_height = inTfrs->blockObj[in].blk_height;
        vx_uint32 num_elems  = inTfrs->blockObj[in].num_elems;
        vx_uint32 in_img_stride = in_img_desc->imagepatch_addr[in].stride_y;

        tfrPrms->copy_mode    = 2;

        tfrPrms->src_addr     = (uint64_t)in_image_target_ptr[in];

        pIn[in][0] = (void *)pL2Base; /* ping instance */
        pIn[in][1] = (void *)((uint8_t *)pL2Base + (blk_width * num_elems * blk_height)); /* pong instance */
        /* Increment the base pointer by 2 for double buffering */
        pL2Base += (2 * (blk_width * num_elems * blk_height));

        if(inTfrs->dmaObj[in].transfer_type == DATA_COPY_DMA)
        {
            tfrPrms->dest_addr    = ((uintptr_t)pIn[in][0] + prms->l2_global_base);
        }
        else
        {
            tfrPrms->dest_addr    = (uintptr_t)pIn[in][0];
        }

        tfrPrms->icnt0     = blk_width * num_elems;
        tfrPrms->icnt1     = blk_height;
        tfrPrms->icnt2     = 2;
        tfrPrms->icnt3     = prms->num_sets / 2;
        tfrPrms->dim1      = in_img_stride;
        tfrPrms->dim2      = (blk_height * in_img_stride);
        tfrPrms->dim3      = (blk_height * in_img_stride * 2);

        tfrPrms->dicnt0    = tfrPrms->icnt0;
        tfrPrms->dicnt1    = tfrPrms->icnt1;
        tfrPrms->dicnt2    = 2; /* Ping-pong */
        tfrPrms->dicnt3    = prms->num_sets / 2;
        tfrPrms->ddim1     = blk_width * num_elems;
        tfrPrms->ddim2     = (blk_height * blk_width * num_elems);
        tfrPrms->ddim3     = 0;

        dma_init(&inTfrs->dmaObj[in]);

        /* Setup input VXLIB params */
        VXLIB_bufParams2D_t *vxlibPrms = &prms->vxlib_src[in];

        vxlibPrms->dim_x = inTfrs->blockObj[in].blk_width;
        vxlibPrms->dim_y = inTfrs->blockObj[in].blk_height;
        if(in_img_desc->format == VX_DF_IMAGE_RGB)
        {
            vxlibPrms->stride_y = inTfrs->blockObj[in].blk_width * 3;
            vxlibPrms->data_type = (uint32_t)VXLIB_UINT24;
        }
        else
        {
            vxlibPrms->stride_y = inTfrs->blockObj[in].blk_width;
            vxlibPrms->data_type = (uint32_t)VXLIB_UINT8;
        }
    }

    /* Note that at this point, pL2Base might be moved based on the
       number of input transfers */
    for(out = 0; out < outTfrs->num_transfers; out++)
    {
        app_udma_copy_nd_prms_t *tfrPrms = (app_udma_copy_nd_prms_t *)&outTfrs->dmaObj[out].tfrPrms;
        vx_uint32 blk_width  = outTfrs->blockObj[out].blk_width;
        vx_uint32 blk_height = outTfrs->blockObj[out].blk_height;
        vx_uint32 num_elems  = outTfrs->blockObj[out].num_elems;
        vx_uint32 out_img_stride = out_img_desc->imagepatch_addr[out].stride_y;

        tfrPrms->copy_mode    = 2;

        tfrPrms->dest_addr     = (uint64_t)out_image_target_ptr[out];

        pOut[out][0] = (void *)pL2Base; /* ping instance */
        pOut[out][1] = (void *)((uint8_t *)pL2Base + (blk_width * num_elems * blk_height)); /* pong instance */
        /* Increment the base pointer by 2 for double buffering */
        pL2Base += (2 * (blk_width * num_elems * blk_height));

        if(outTfrs->dmaObj[out].transfer_type == DATA_COPY_DMA)
        {
            tfrPrms->src_addr    = ((uintptr_t)pOut[out][0] + prms->l2_global_base);
        }
        else
        {
            tfrPrms->src_addr    = (uintptr_t)pOut[out][0];
        }

        tfrPrms->icnt0     = blk_width * num_elems;
        tfrPrms->icnt1     = blk_height;
        tfrPrms->icnt2     = 2;
        tfrPrms->icnt3     = prms->num_sets / 2;
        tfrPrms->dim1      = blk_width * num_elems;
        tfrPrms->dim2      = (blk_height * blk_width * num_elems);
        tfrPrms->dim3      = 0;

        tfrPrms->dicnt0    = tfrPrms->icnt0;
        tfrPrms->dicnt1    = tfrPrms->icnt1;
        tfrPrms->dicnt2    = 2; /* Ping-pong */
        tfrPrms->dicnt3    = prms->num_sets / 2;
        tfrPrms->ddim1     = out_img_stride;
        tfrPrms->ddim2     = (blk_height * out_img_stride);
        tfrPrms->ddim3     = (blk_height * out_img_stride * 2);

        dma_init(&outTfrs->dmaObj[out]);

        /* Setup output VXLIB params */
        VXLIB_bufParams2D_t *vxlibPrms = &prms->vxlib_dst[out];

        vxlibPrms->dim_x = outTfrs->blockObj[out].blk_width;
        vxlibPrms->dim_y = outTfrs->blockObj[out].blk_height;
        if(out_img_desc->format == VX_DF_IMAGE_RGB)
        {
            vxlibPrms->stride_y = outTfrs->blockObj[out].blk_width * 3;
            vxlibPrms->data_type = (uint32_t)VXLIB_UINT24;
        }
        else
        {
            vxlibPrms->stride_y = outTfrs->blockObj[out].blk_width;
            vxlibPrms->data_type = (uint32_t)VXLIB_UINT8;
        }
    }

    /* Assign scratch pointer to updated L2 base address */
    prms->pScratch = pL2Base;

    dl_color_convert_pipeline_blocks
    (
        prms,
        in_img_desc,
        out_img_desc,
        pIn,
        pOut
    );

    for(in = 0; in < inTfrs->num_transfers; in++)
    {
        dma_deinit(&inTfrs->dmaObj[in]);
    }

    for(out = 0; out < outTfrs->num_transfers; out++)
    {
        dma_deinit(&outTfrs->dmaObj[out]);
    }

    return status;
}


static vx_status VX_CALLBACK tivxKernelDLColorConvertProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivxDLColorConvertKernelParams *prms = NULL;
    vx_int32 i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDLColorConvertKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        tivx_obj_desc_image_t *out_img_desc;
        void* out_image_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL, NULL, NULL};

        vx_int32 i;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_INPUT_IDX];
        for (i = 0; i < in_img_desc->planes; i++)
        {
            if(in_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                in_image_target_ptr[i]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[i]);
                tivxMemBufferMap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }

        out_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_COLOR_CONVERT_OUTPUT_IDX];
        for (i = 0; i < out_img_desc->planes; i++)
        {
            if(out_img_desc->mem_ptr[i].shared_ptr != 0)
            {
                out_image_target_ptr[i]  = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[i]);
                tivxMemBufferMap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }

        dl_color_convert_process
        (
            prms,
            in_img_desc,
            out_img_desc,
            in_image_target_ptr,
            out_image_target_ptr
        );

        for (i = 0; i < in_img_desc->planes; i++)
        {
            if (in_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(in_image_target_ptr[i], in_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }

        for (i = 0; i < out_img_desc->planes; i++)
        {
            if (out_image_target_ptr[i] != NULL)
            {
                tivxMemBufferUnmap(out_image_target_ptr[i], out_img_desc->mem_size[i], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }
    }

    tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxDLColorConvertKernelParams));

    return (status);
}

void tivxAddTargetKernelDLColorConvert(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_dl_color_convert_target_kernel = tivxAddTargetKernelByName (
                                            TIVX_KERNEL_DL_COLOR_CONVERT_NAME,
                                            target_name,
                                            tivxKernelDLColorConvertProcess,
                                            tivxKernelDLColorConvertCreate,
                                            tivxKernelDLColorConvertDelete,
                                            NULL,
                                            NULL);
    }
}

void tivxRemoveTargetKernelDLColorConvert(void)
{
    tivxRemoveTargetKernel(vx_dl_color_convert_target_kernel);
}
