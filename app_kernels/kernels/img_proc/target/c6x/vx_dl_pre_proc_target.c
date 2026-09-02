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
#include "tivx_kernels_target_utils.h"

#include <tivx_dl_pre_proc_host.h>

#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>

#include "vx_dma_transfers.h"

#define DL_PRE_PROC_8BIT_UNSIGNED_MIN (   0.0f)
#define DL_PRE_PROC_8BIT_UNSIGNED_MAX ( 255.0f)
#define DL_PRE_PROC_8BIT_SIGNED_MIN   (-128.0f)
#define DL_PRE_PROC_8BIT_SIGNED_MAX   ( 127.0f)

#define DL_PRE_PROC_16BIT_UNSIGNED_MIN (   0.0f)
#define DL_PRE_PROC_16BIT_UNSIGNED_MAX ( 255.0f)
#define DL_PRE_PROC_16BIT_SIGNED_MIN   (-128.0f)
#define DL_PRE_PROC_16BIT_SIGNED_MAX   ( 127.0f)

#define DL_PRE_PROC_32BIT_UNSIGNED_MIN (   0.0f)
#define DL_PRE_PROC_32BIT_UNSIGNED_MAX ( 255.0f)
#define DL_PRE_PROC_32BIT_SIGNED_MIN   (-128.0f)
#define DL_PRE_PROC_32BIT_SIGNED_MAX   ( 127.0f)

#define DL_PRE_PROC_FLOAT_MIN (   0.0f)
#define DL_PRE_PROC_FLOAT_MAX ( 255.0f)

#define EXEC_PIPELINE_STAGE1(x) ((x) & 0x00000001)
#define EXEC_PIPELINE_STAGE2(x) ((x) & 0x00000002)
#define EXEC_PIPELINE_STAGE3(x) ((x) & 0x00000004)

#define J721E_C66X_DSP_L2_SRAM_SIZE  (224 * 1024) /* size in bytes */

typedef struct {
    tivxDLPreProcParams dlParams;

    sTransferGroup inTfrs;
    sTransferGroup outTfrs;

    vx_uint32 tile_width;
    vx_uint32 tile_height;
    vx_uint32 num_sets;

    uint8_t *pL2;
    uint32_t l2_heap_id;
    uint64_t l2_global_base;
    uint32_t alloc_size;

} tivxDLPreProcKernelParams;

static tivx_target_kernel vx_dl_pre_proc_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDLPreProcCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    /* tivx_set_debug_zone(VX_ZONE_INFO); */

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    tivxDLPreProcKernelParams * kernelParams = NULL;

    kernelParams = tivxMemAlloc(sizeof(tivxDLPreProcKernelParams), TIVX_MEM_EXTERNAL);
    if (NULL == kernelParams)
    {
        status = VX_FAILURE;
    }
    else
    {
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr;

        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[2];

        tivx_obj_desc_tensor_t *out_tensor_desc;
        void *out_tensor_target_ptr;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_INPUT_IMAGE_IDX];
        in_image_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_image_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_image_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_image_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_image_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        VX_PRINT(VX_ZONE_INFO, "Image channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[0].step_y);

        if(in_image_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[1].step_y);
        }

        VX_PRINT(VX_ZONE_INFO, "Tensor input \n");
        VX_PRINT(VX_ZONE_INFO, "stride[0] = %d\n", out_tensor_desc->stride[0]);
        VX_PRINT(VX_ZONE_INFO, "stride[1] = %d\n", out_tensor_desc->stride[1]);
        VX_PRINT(VX_ZONE_INFO, "stride[2] = %d\n", out_tensor_desc->stride[2]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[0] = %d\n", out_tensor_desc->dimensions[0]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[1] = %d\n", out_tensor_desc->dimensions[1]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[2] = %d\n", out_tensor_desc->dimensions[2]);

        memcpy(&kernelParams->dlParams, config_target_ptr, sizeof(tivxDLPreProcParams));

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

        vx_df_image image_format = in_img_desc->format;
        vx_uint32 channel_order = kernelParams->dlParams.channel_order;

        sTransferGroup *inTfrs = (sTransferGroup *)&kernelParams->inTfrs;
        sTransferGroup *outTfrs = (sTransferGroup *)&kernelParams->outTfrs;
        vx_uint32 in, out;

        /* Input is assumed to be only of type 8-bit */
        if(image_format == VX_DF_IMAGE_NV12)
        {
            inTfrs->num_transfers = 2;

            inTfrs->blockObj[0].blk_width  = in_img_desc->imagepatch_addr[0].dim_x;
            inTfrs->blockObj[0].blk_height = 2;
            inTfrs->blockObj[0].num_elems  = 1;
            inTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);

            inTfrs->blockObj[1].blk_width  = in_img_desc->imagepatch_addr[1].dim_x;
            inTfrs->blockObj[1].blk_height = 1;
            inTfrs->blockObj[1].num_elems  = 1; /*Special case where CbCR is adjacent */
            inTfrs->blockObj[1].num_bytes  = sizeof(vx_uint8);
        }
        else if (image_format == VX_DF_IMAGE_RGB)
        {
            inTfrs->num_transfers = 1;

            inTfrs->blockObj[0].blk_width = in_img_desc->imagepatch_addr[0].dim_x;
            inTfrs->blockObj[0].blk_height = 2;
            inTfrs->blockObj[0].num_elems  = 3;
            inTfrs->blockObj[0].num_bytes  = sizeof(vx_uint8);
        }

        if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NCHW)
        {
            outTfrs->num_transfers = out_tensor_desc->number_of_dimensions;

            for(out = 0; out < outTfrs->num_transfers; out++)
            {
                /* Output width is same as input width */
                outTfrs->blockObj[out].blk_width  = in_img_desc->imagepatch_addr[0].dim_x;
                outTfrs->blockObj[out].blk_height = 2;
                outTfrs->blockObj[out].num_elems  = 1;
                outTfrs->blockObj[out].num_bytes  = out_tensor_desc->stride[0];
            }
        }
        else if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NHWC)
        {
            outTfrs->num_transfers = 1;

            /* Output width is same as input width */
            outTfrs->blockObj[0].blk_width  = in_img_desc->imagepatch_addr[0].dim_x;
            outTfrs->blockObj[0].blk_height = 2;
            outTfrs->blockObj[0].num_elems  = out_tensor_desc->number_of_dimensions;
            outTfrs->blockObj[0].num_bytes  = out_tensor_desc->stride[0];
        }

        vx_uint32 in_size, out_size, total_size;
        vx_uint32 max_height, rem_height, mblk;
        vx_uint32 num_sets;

        /* This logic tries to find optimum block height which is a
           multiple of 2. It is assumed that DMA will fetch a minimum of
           Width x block_height */
        total_size = 0;
        mblk = 1;
        rem_height = 0;
        in_size = 0;
        out_size = 0;
        num_sets = 0;
        max_height = in_img_desc->imagepatch_addr[0].dim_y;

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

            total_size = in_size + out_size;
            rem_height = max_height % (mblk * 2);
            num_sets = max_height / (mblk * 2);

            VX_PRINT(VX_ZONE_INFO, "mblk = %d\n", mblk);
            VX_PRINT(VX_ZONE_INFO, "num_sets = %d\n", num_sets);
            VX_PRINT(VX_ZONE_INFO, "rem_height = %d\n", rem_height);
            VX_PRINT(VX_ZONE_INFO, "in_size = %d\n", in_size);
            VX_PRINT(VX_ZONE_INFO, "out_size = %d\n", out_size);
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
            VX_PRINT(VX_ZONE_INFO, "block->blk_width[%d] = %d\n", in, block->blk_width);
            VX_PRINT(VX_ZONE_INFO, "block->blk_height[%d] = %d\n", in, block->blk_height);
            VX_PRINT(VX_ZONE_INFO, "block->num_elems[%d] = %d\n", in, block->num_elems);
            VX_PRINT(VX_ZONE_INFO, "block->num_bytes[%d] = %d\n", in, block->num_bytes);
        }

        kernelParams->tile_width  = inTfrs->blockObj[0].blk_width;
        kernelParams->tile_height = inTfrs->blockObj[0].blk_height;

        VX_PRINT(VX_ZONE_INFO, "Selected tile_width = %d\n", kernelParams->tile_width);
        VX_PRINT(VX_ZONE_INFO, "Selected tile_height = %d\n", kernelParams->tile_height);

        for(out = 0; out < outTfrs->num_transfers; out++)
        {
            sTransferBlock *block = &outTfrs->blockObj[out];
            block->blk_height = block->blk_height * mblk;
            VX_PRINT(VX_ZONE_INFO, "block->blk_width[%d] = %d\n", out, block->blk_width);
            VX_PRINT(VX_ZONE_INFO, "block->blk_height[%d] = %d\n", out, block->blk_height);
            VX_PRINT(VX_ZONE_INFO, "block->num_elems[%d] = %d\n", out, block->num_elems);
            VX_PRINT(VX_ZONE_INFO, "block->num_bytes[%d] = %d\n", out, block->num_bytes);
        }

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

        if((vx_status)VX_SUCCESS == status)
        {
            vx_uint32 in, out, ch = 0;

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

        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(in_image_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_image_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxDLPreProcKernelParams));
    }

    /* tivx_clr_debug_zone(VX_ZONE_INFO); */

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLPreProcDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDLPreProcKernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            sTransferGroup *inTfrs = (sTransferGroup *)&prms->inTfrs;
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

            tivxMemFree(prms, sizeof(tivxDLPreProcKernelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static inline void convertYCbCrToRGB(int32_t Y, int32_t Cb, int32_t Cr, float *R, float *G, float *B, float min_val, float max_val)
{
    Y  = Y - 16;
    Cb = Cb - 128;
    Cr = Cr - 128;

    float R_ = (float)((298*Y + 409*Cr) >> 8);
    float G_ = (float)((298*Y - 100*Cb - 208*Cr) >> 8);
    float B_ = (float)((298*Y + 516*Cb) >> 8);

    if(R_ < min_val) { R_ = min_val; }
    if(R_ > max_val) { R_ = max_val; }

    if(G_ < min_val) { G_ = min_val; }
    if(G_ > max_val) { G_ = max_val; }

    if(B_ < min_val) { B_ = min_val; }
    if(B_ > max_val) { B_ = max_val; }

    *R = R_;
    *G = G_;
    *B = B_;
}

static void dl_pre_proc_kernel
(
    void *in_image_target_ptr[2],
    void *out_tensor_target_ptr,
    vx_df_image image_format,
    uint32_t tensor_format,
    uint32_t channel_order,
    uint32_t tensor_data_type,
    uint32_t tile_width,
    uint32_t tile_height,
    vx_float32 dl_mean[3],
    vx_float32 dl_scale[3]
)
{
    uint32_t pos_x, pos_y;

    /* tivx_set_debug_zone(VX_ZONE_INFO); */

    VX_PRINT(VX_ZONE_INFO, "in_image_target_ptr[0] = 0x%08X \n", in_image_target_ptr[0]);
    VX_PRINT(VX_ZONE_INFO, "in_image_target_ptr[1] = 0x%08X \n", in_image_target_ptr[1]);
    VX_PRINT(VX_ZONE_INFO, "out_tensor_target_ptr = 0x%08X \n", out_tensor_target_ptr);
    VX_PRINT(VX_ZONE_INFO, "image_format = %d\n", image_format);
    VX_PRINT(VX_ZONE_INFO, "tensor_format = %d\n", tensor_format);
    VX_PRINT(VX_ZONE_INFO, "channel_order = %d\n", channel_order);
    VX_PRINT(VX_ZONE_INFO, "tensor_data_type = %d\n", tensor_data_type);
    VX_PRINT(VX_ZONE_INFO, "tile_width = %d\n", tile_width);
    VX_PRINT(VX_ZONE_INFO, "tile_height = %d\n", tile_height);

    /* tivx_clr_debug_zone(VX_ZONE_INFO); */

    if(image_format == VX_DF_IMAGE_RGB)
    {
        if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NHWC)
        {
            if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB)
            {
                /* Case 1 */
                /* Input is RGB, Output is RGB (NHWC) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R = pIn[offset + 0] * 1.0f;
                            float G = pIn[offset + 1] * 1.0f;
                            float B = pIn[offset + 2] * 1.0f;

                            pOut[offset + 0] = (float)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (float)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (float)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            else if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_BGR)
            {
                /* Case 2 */
                /* Input is RGB, Output is BGR (NHWC) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)out_tensor_target_ptr + (pos_y * tile_width * 3);
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R = pIn[offset + 0] * 1.0f;
                            float G = pIn[offset + 1] * 1.0f;
                            float B = pIn[offset + 2] * 1.0f;

                            pOut[offset + 2] = (float)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (float)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (float)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
        else if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NCHW)
        {
            if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB)
            {
                /* Case 3 */
                /* Input is RGB Output is RGB (NCHW) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int8_t *pOut = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint8_t *pOut = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int16_t *pOut = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint16_t *pOut = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int32_t *pOut = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint32_t *pOut = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        float *pOut = (float *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_stride * 0) + pos_x] = (float)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (float)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (float)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            else if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_BGR)
            {
                /* Case 4 */
                /* Input is RGB Output is BGR (NCHW) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int8_t *pOut = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint8_t *pOut = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int16_t *pOut = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint16_t *pOut = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        int32_t *pOut = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        uint32_t *pOut = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width * 3);
                        float *pOut = (float *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;
                        uint32_t offset;

                        offset = 0;
                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_stride * 2) + pos_x] = (float)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (float)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (float)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
    }
    else if(image_format == VX_DF_IMAGE_NV12)
    {
        if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NHWC)
        {
            if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB)
            {
                /* Case 1 */
                /* Input is NV12, Output is RGB (NHWC) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    int8_t *pOut = (int8_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[offset + 0] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    uint8_t *pOut = (uint8_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[offset + 0] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    int16_t *pOut = (int16_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_SIGNED_MIN, DL_PRE_PROC_16BIT_SIGNED_MAX);

                            pOut[offset + 0] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    uint16_t *pOut = (uint16_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_UNSIGNED_MIN, DL_PRE_PROC_16BIT_UNSIGNED_MAX);

                            pOut[offset + 0] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    int32_t *pOut = (int32_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_SIGNED_MIN, DL_PRE_PROC_32BIT_SIGNED_MAX);

                            pOut[offset + 0] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    uint32_t *pOut = (uint32_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_UNSIGNED_MIN, DL_PRE_PROC_32BIT_UNSIGNED_MAX);

                            pOut[offset + 0] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 2] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    float *pOut = (float *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_FLOAT_MIN, DL_PRE_PROC_FLOAT_MAX);

                            pOut[offset + 0] = (R - dl_mean[0]) * dl_scale[0];
                            pOut[offset + 1] = (G - dl_mean[1]) * dl_scale[1];
                            pOut[offset + 2] = (B - dl_mean[2]) * dl_scale[2];

                            offset += 3;
                        }
                    }
                }
            }
            else if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_BGR)
            {
                /* Case 2 */
                /* Input is NV12,  Output is BGR (NHWC) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    int8_t *pOut = (int8_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[offset + 2] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    uint8_t *pOut = (uint8_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[offset + 2] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    int16_t *pOut = (int16_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_SIGNED_MIN, DL_PRE_PROC_16BIT_SIGNED_MAX);

                            pOut[offset + 2] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    uint16_t *pOut = (uint16_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_UNSIGNED_MIN, DL_PRE_PROC_16BIT_UNSIGNED_MAX);

                            pOut[offset + 2] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    int32_t *pOut = (int32_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_SIGNED_MIN, DL_PRE_PROC_32BIT_SIGNED_MAX);

                            pOut[offset + 2] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    uint32_t *pOut = (uint32_t *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_UNSIGNED_MIN, DL_PRE_PROC_32BIT_UNSIGNED_MAX);

                            pOut[offset + 2] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[offset + 0] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);

                            offset += 3;
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    float *pOut = (float *)out_tensor_target_ptr;
                    uint32_t offset;

                    offset = 0;
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_FLOAT_MIN, DL_PRE_PROC_FLOAT_MAX);

                            pOut[offset + 2] = (R - dl_mean[0]) * dl_scale[0];
                            pOut[offset + 1] = (G - dl_mean[1]) * dl_scale[1];
                            pOut[offset + 0] = (B - dl_mean[2]) * dl_scale[2];

                            offset += 3;
                        }
                    }
                }
            }
        }
        else if(channel_order == TIVX_DL_PRE_PROC_CHANNEL_ORDER_NCHW)
        {
            if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB)
            {
                /* Case 3 */
                /* Input is NV12, Output is RGB (NCHW) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int8_t *pOut   = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint8_t *pOut  = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int16_t *pOut  = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_SIGNED_MIN, DL_PRE_PROC_16BIT_SIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint16_t *pOut  = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_UNSIGNED_MIN, DL_PRE_PROC_16BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int32_t *pOut  = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_SIGNED_MIN, DL_PRE_PROC_32BIT_SIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint32_t *pOut  = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_UNSIGNED_MIN, DL_PRE_PROC_32BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 2) + pos_x] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        float   *pOut  = (float   *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_FLOAT_MIN, DL_PRE_PROC_FLOAT_MAX);

                            pOut[(ch_stride * 0) + pos_x] = (R - dl_mean[0]) * dl_scale[0];
                            pOut[(ch_stride * 1) + pos_x] = (G - dl_mean[1]) * dl_scale[1];
                            pOut[(ch_stride * 2) + pos_x] = (B - dl_mean[2]) * dl_scale[2];
                        }
                    }
                }
            }
            else if(tensor_format == TIVX_DL_PRE_PROC_TENSOR_FORMAT_BGR)
            {
                /* Case 4 */
                /* Input is NV12, Output is BGR (NCHW) */
                if(tensor_data_type == VX_TYPE_INT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int8_t *pOut   = (int8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (int8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int8_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT8)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint8_t *pOut  = (uint8_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_8BIT_UNSIGNED_MIN, DL_PRE_PROC_8BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (uint8_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint8_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint8_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int16_t *pOut  = (int16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_SIGNED_MIN, DL_PRE_PROC_16BIT_SIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (int16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int16_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT16)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint16_t *pOut  = (uint16_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_16BIT_UNSIGNED_MIN, DL_PRE_PROC_16BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (uint16_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint16_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint16_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_INT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        int32_t *pOut  = (int32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_SIGNED_MIN, DL_PRE_PROC_32BIT_SIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (int32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (int32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (int32_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_UINT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        uint32_t *pOut  = (uint32_t *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_32BIT_UNSIGNED_MIN, DL_PRE_PROC_32BIT_UNSIGNED_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (uint32_t)((R - dl_mean[0]) * dl_scale[0]);
                            pOut[(ch_stride * 1) + pos_x] = (uint32_t)((G - dl_mean[1]) * dl_scale[1]);
                            pOut[(ch_stride * 0) + pos_x] = (uint32_t)((B - dl_mean[2]) * dl_scale[2]);
                        }
                    }
                }
                else if(tensor_data_type == VX_TYPE_FLOAT32)
                {
                    for(pos_y = 0; pos_y < tile_height; pos_y++)
                    {
                        uint8_t *pY    = (uint8_t *)in_image_target_ptr[0] + (pos_y * tile_width);
                        uint8_t *pCbCr = (uint8_t *)in_image_target_ptr[1] + ((pos_y >> 1) * tile_width);
                        float   *pOut  = (float   *)out_tensor_target_ptr + (pos_y * tile_width);
                        uint32_t ch_stride = tile_width * tile_height;

                        for(pos_x = 0; pos_x < tile_width; pos_x++)
                        {
                            float R, G, B;

                            int32_t Y  = pY[pos_x];
                            int32_t Cb = pCbCr[((pos_x>>1)<<1)];
                            int32_t Cr = pCbCr[((pos_x>>1)<<1) + 1];

                            convertYCbCrToRGB(Y, Cb, Cr, &R, &G, &B, DL_PRE_PROC_FLOAT_MIN, DL_PRE_PROC_FLOAT_MAX);

                            pOut[(ch_stride * 2) + pos_x] = (R - dl_mean[0]) * dl_scale[0];
                            pOut[(ch_stride * 1) + pos_x] = (G - dl_mean[1]) * dl_scale[1];
                            pOut[(ch_stride * 0) + pos_x] = (B - dl_mean[2]) * dl_scale[2];
                        }
                    }
                }
            }
        }
    }
}

static vx_status dl_pre_proc_pipeline_blocks
(
    tivxDLPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void *pInData[2][2],
    void *pOutData[2]
)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 pipeup = 2;
    vx_uint32 pipedown = 2;
    vx_uint32 exec_cmd = 1;
    vx_uint32 pipeline = exec_cmd;
    vx_uint32 ping_npong = 0;

    vx_uint32 blk, in, out;

    tivxDLPreProcParams *dlParams = (tivxDLPreProcParams *)&prms->dlParams;
    sTransferGroup *inTfrs  = (sTransferGroup *)&prms->inTfrs;
    sTransferGroup *outTfrs = (sTransferGroup *)&prms->outTfrs;

    vx_df_image image_format = in_img_desc->format;
    uint32_t tensor_format = dlParams->tensor_format;
    uint32_t channel_order = dlParams->channel_order;
    uint32_t tensor_data_type = out_tensor_desc->data_type;

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

            void *pIn[2];
            void *pOut;

            pIn[0] = pInData[0][ping_npong];
            pIn[1] = pInData[1][ping_npong];
            pOut = pOutData[ping_npong];

            dl_pre_proc_kernel
            (
                pIn,
                pOut,
                image_format,
                tensor_format,
                channel_order,
                tensor_data_type,
                prms->tile_width,
                prms->tile_height,
                dlParams->mean,
                dlParams->scale
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

static vx_status dl_pre_proc_process
(
    tivxDLPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void *in_image_target_ptr[2],
    void *out_tensor_target_ptr
)
{
    vx_status status = VX_SUCCESS;
    void *pIn[2][2];
    void *pOut[2];

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
    }

    pOut[0] = (void *)pL2Base; /* ping instance */
    pOut[1] = (void *)((uint8_t *)pL2Base + (outTfrs->num_transfers         *
                                            outTfrs->blockObj[0].blk_width  *
                                            outTfrs->blockObj[0].blk_height *
                                            outTfrs->blockObj[0].num_elems  *
                                            outTfrs->blockObj[0].num_bytes)); /* pong instance */

    for(out = 0; out < outTfrs->num_transfers; out++)
    {
        app_udma_copy_nd_prms_t *tfrPrms = (app_udma_copy_nd_prms_t *)&outTfrs->dmaObj[out].tfrPrms;
        vx_uint32 blk_width  = outTfrs->blockObj[out].blk_width;
        vx_uint32 blk_height = outTfrs->blockObj[out].blk_height;
        vx_uint32 num_elems  = outTfrs->blockObj[out].num_elems;
        vx_uint32 num_bytes  = outTfrs->blockObj[out].num_bytes;

        tfrPrms->copy_mode    = 2;
        void *pCh = (void *)((uint8_t *)pL2Base + (out * blk_width * blk_height * num_bytes));

        if(outTfrs->dmaObj[out].transfer_type == DATA_COPY_DMA)
        {
            tfrPrms->src_addr    = ((uintptr_t)pCh + prms->l2_global_base);
        }
        else
        {
            tfrPrms->src_addr    = (uintptr_t)pCh;
        }
        tfrPrms->dest_addr    = (uint64_t)((uint8_t *)out_tensor_target_ptr + (out * in_img_desc->imagepatch_addr[0].dim_y * num_bytes * blk_width));

        tfrPrms->icnt0     = blk_width * num_elems;
        tfrPrms->icnt1     = blk_height;
        tfrPrms->icnt2     = 2; /* Double buffer */
        tfrPrms->icnt3     = prms->num_sets / 2;
        tfrPrms->dim1      = blk_width * num_elems * num_bytes;
        tfrPrms->dim2      = blk_height * tfrPrms->dim1 * outTfrs->num_transfers;
        tfrPrms->dim3      = 0;

        tfrPrms->dicnt0    = tfrPrms->icnt0;
        tfrPrms->dicnt1    = tfrPrms->icnt1;
        tfrPrms->dicnt2    = 2;
        tfrPrms->dicnt3    = prms->num_sets / 2;
        tfrPrms->ddim1     = tfrPrms->dim1;
        tfrPrms->ddim2     = (blk_height * tfrPrms->ddim1);
        tfrPrms->ddim3     = (tfrPrms->ddim2 * 2);

        tfrPrms->eltype = num_bytes;

        dma_init(&outTfrs->dmaObj[out]);
    }

    dl_pre_proc_pipeline_blocks
    (
        prms,
        in_img_desc,
        out_tensor_desc,
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


static vx_status VX_CALLBACK tivxKernelDLPreProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxDLPreProcKernelParams *prms = NULL;
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
            (sizeof(tivxDLPreProcKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[2];

        tivx_obj_desc_tensor_t *out_tensor_desc;
        void *out_tensor_target_ptr;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_INPUT_IMAGE_IDX];
        in_image_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_image_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_image_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_image_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_image_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        VX_PRINT(VX_ZONE_INFO, "Image channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[0].step_y);

        if(in_image_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[1].step_y);
        }

        VX_PRINT(VX_ZONE_INFO, "Tensor input \n");
        VX_PRINT(VX_ZONE_INFO, "stride[0] = %d\n", out_tensor_desc->stride[0]);
        VX_PRINT(VX_ZONE_INFO, "stride[1] = %d\n", out_tensor_desc->stride[1]);
        VX_PRINT(VX_ZONE_INFO, "stride[2] = %d\n", out_tensor_desc->stride[2]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[0] = %d\n", out_tensor_desc->dimensions[0]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[1] = %d\n", out_tensor_desc->dimensions[1]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[2] = %d\n", out_tensor_desc->dimensions[2]);

        status = dl_pre_proc_process(prms, in_img_desc, out_tensor_desc, (void **)in_image_target_ptr, out_tensor_target_ptr);

        /* Write DL pre proc operation here */
        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_image_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_image_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
    }

    return (status);
}

void tivxAddTargetKernelDLPreProc()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = tivxKernelsTargetUtilsAssignTargetNameDsp(target_name);
    }

    if( (vx_status)VX_SUCCESS == status)
    {
        vx_dl_pre_proc_target_kernel = tivxAddTargetKernelByName
                                        (
                                            TIVX_KERNEL_DL_PRE_PROC_NAME,
                                            target_name,
                                            tivxKernelDLPreProcProcess,
                                            tivxKernelDLPreProcCreate,
                                            tivxKernelDLPreProcDelete,
                                            NULL,
                                            NULL
                                        );
    }
}

void tivxRemoveTargetKernelDLPreProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dl_pre_proc_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dl_pre_proc_target_kernel = NULL;
    }
}
