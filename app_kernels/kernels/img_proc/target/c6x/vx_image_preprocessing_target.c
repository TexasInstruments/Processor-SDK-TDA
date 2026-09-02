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



#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"

#include <tivx_img_preprocessing_host.h>
#include "tiadalg_interface.h"
#include <utils/udma/include/app_udma_utils.h>
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>

#ifndef x86_64
#include <ti/osal/HwiP.h>
//#define DISABLE_INTERRUPTS_DURING_PROCESS
#endif

#define EXEC_PIPELINE_STAGE1(x) ((x) & 0x00000001)
#define EXEC_PIPELINE_STAGE2(x) ((x) & 0x00000002)
#define EXEC_PIPELINE_STAGE3(x) ((x) & 0x00000004)

#define J721E_C66X_DSP_L2_SRAM_SIZE  (224 * 1024) /* size in bytes */

static tivx_target_kernel vx_img_preproc_target_kernel = NULL;

typedef enum {
    DATA_COPY_CPU = 0,
    DATA_COPY_DMA = 1,
    DATA_COPY_DEFAULT = DATA_COPY_CPU

} DataCopyStyle;

typedef struct {

    app_udma_ch_handle_t udmaChHdl;
    app_udma_copy_nd_prms_t tfrPrms;

    vx_uint32 transfer_type;

    vx_uint32 icnt1_next;
    vx_uint32 icnt2_next;
    vx_uint32 icnt3_next;

    vx_uint32 dma_ch;

}DMAObj;

typedef struct {
    tivxImgPreProcParams nodeParams;

    DMAObj dmaObjRGB;
    DMAObj dmaObjY;
    DMAObj dmaObjC;
    DMAObj dmaObjR;
    DMAObj dmaObjG;
    DMAObj dmaObjB;

    vx_uint32 blkWidth;
    vx_uint32 blkHeight;
    vx_uint32 remHeight;
    vx_uint32 numSets;
    vx_uint32 req_size;
    vx_uint32 avail_size;

    uint8_t *pL2;
    uint32_t l2_heap_id;
    uint64_t l2_global_base;

} tivxImgPreProcKernelParams;

static vx_status img_proc_pipeline_blocks_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    vx_uint8 *pYL2[2],
    vx_uint8 *pCL2[2],
    void *pOutL2[2],
    uint32_t blk_width,
    uint32_t blk_height,
    uint32_t blk_stride,
    uint32_t num_sets,
    int32_t data_type
);

static vx_status img_proc_pipeline_blocks_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    vx_uint8 *pRGBL2[2],
    void *pOutL2[2],
    uint32_t blk_width,
    uint32_t blk_height,
    uint32_t blk_stride,
    uint32_t num_sets,
    int32_t data_type
);

static vx_status img_proc_execute_8bit_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
);

static vx_status img_proc_execute_8bit_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
);

static vx_status img_proc_execute_16bit_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
);

static vx_status img_proc_execute_16bit_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
);

static vx_status dma_create(DMAObj *dmaObj, vx_size transfer_type, vx_uint32 dma_ch)
{
    vx_status status = VX_SUCCESS;

    dmaObj->transfer_type = transfer_type;

    memset(&dmaObj->tfrPrms, 0, sizeof(app_udma_copy_nd_prms_t));

    dmaObj->icnt1_next = 0;
    dmaObj->icnt2_next = 0;
    dmaObj->icnt3_next = 0;

    dmaObj->dma_ch = dma_ch;

    if(transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        dmaObj->udmaChHdl = appUdmaCopyNDGetHandle(dma_ch);
#endif
    }
    else
    {
        dmaObj->udmaChHdl = NULL;
    }

    return status;
}

static vx_status dma_transfer_trigger(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDTrigger(dmaObj->udmaChHdl);
#endif
    }
    else
    {
        app_udma_copy_nd_prms_t *tfrPrms;
        vx_uint32 icnt1, icnt2, icnt3;
        vx_uint32 num_bytes = 1;

        tfrPrms = (app_udma_copy_nd_prms_t *)&dmaObj->tfrPrms;

        /* This is for case where for every trigger ICNT0 * ICNT1 bytes get transferred */
        icnt3 = dmaObj->icnt3_next;
        icnt2 = dmaObj->icnt2_next;
        icnt1 = dmaObj->icnt1_next;

        /* As C66 is a 32b processor, address will be truncated to use only lower 32b address */
        /* So user is responsible for providing correct address */
#ifndef x86_64
        vx_uint8 *pSrcNext = (vx_uint8 *)((uint32_t)tfrPrms->src_addr + (icnt3 * tfrPrms->dim3) + (icnt2 * tfrPrms->dim2));
        vx_uint8 *pDstNext = (vx_uint8 *)((uint32_t)tfrPrms->dest_addr + (icnt3 * tfrPrms->ddim3) + (icnt2 * tfrPrms->ddim2));
#else
        vx_uint8 *pSrcNext = (vx_uint8 *)(tfrPrms->src_addr + (icnt3 * tfrPrms->dim3) + (icnt2 * tfrPrms->dim2));
        vx_uint8 *pDstNext = (vx_uint8 *)(tfrPrms->dest_addr + (icnt3 * tfrPrms->ddim3) + (icnt2 * tfrPrms->ddim2));
#endif

        if((tfrPrms->eltype == 1) || (tfrPrms->eltype == 0))
        {
            /* Indicate 1 byte per element for transferring 8bit data */
            num_bytes = 1;
        }
        else if(tfrPrms->eltype == 2)
        {
            /* Indicate 2 bytes per element for transferring 16bit data */
            num_bytes = 2;
        }
        else if(tfrPrms->eltype == 3)
        {
            /* Indicate 3 bytes per element for transferring 24bit data */
            num_bytes = 3;
        }
        else if(tfrPrms->eltype == 4)
        {
            /* Indicate 4 bytes per element for transferring 32bit data */
            num_bytes = 4;
        }

        for(icnt1 = 0; icnt1 < tfrPrms->icnt1; icnt1++)
        {
            memcpy(pDstNext, pSrcNext, (tfrPrms->icnt0 * num_bytes));

            pSrcNext += tfrPrms->dim1;
            pDstNext += tfrPrms->ddim1;
        }

        icnt2++;

        if(icnt2 == tfrPrms->icnt2)
        {
            icnt2 = 0;
            icnt3++;
        }

        dmaObj->icnt3_next = icnt3;
        dmaObj->icnt2_next = icnt2;
    }

    return status;
}

static vx_status dma_transfer_wait(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDWait(dmaObj->udmaChHdl);
#endif
    }

    return status;
}

static vx_status dma_init(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDInit(dmaObj->udmaChHdl, &dmaObj->tfrPrms);
#endif
    }
    else
    {
        dmaObj->icnt1_next = 0;
        dmaObj->icnt2_next = 0;
        dmaObj->icnt3_next = 0;
    }

    return status;
}

static vx_status dma_deinit(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        appUdmaCopyNDDeinit(dmaObj->udmaChHdl);
#endif
    }
    else
    {
        dmaObj->icnt1_next = 0;
        dmaObj->icnt2_next = 0;
        dmaObj->icnt3_next = 0;
    }
    return status;
}

static vx_status dma_delete(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    dmaObj->udmaChHdl = NULL;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#ifndef x86_64
        int32_t retVal = appUdmaCopyNDReleaseHandle(dmaObj->dma_ch);
        if(retVal != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to release DMA handle %d\n", dmaObj->dma_ch);
            status = VX_FAILURE;
        }
#endif
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelImgPreProcCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    tivxImgPreProcKernelParams * kernelParams = NULL;
    tivxImgPreProcParams * nodeParams = NULL;
    vx_status status = VX_SUCCESS;

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    VX_PRINT(VX_ZONE_WARNING, "All Interrupts DISABLED during ImgPreProc process\n");
    #endif

    tivx_obj_desc_array_t *node_params_array = (tivx_obj_desc_array_t*)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_CONFIG_IDX];
    void *node_params_array_target_ptr = tivxMemShared2TargetPtr(&node_params_array->mem_ptr);
    tivxMemBufferMap(node_params_array_target_ptr, node_params_array->mem_size,
        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    kernelParams = tivxMemAlloc(sizeof(tivxImgPreProcKernelParams), TIVX_MEM_EXTERNAL);

    if(kernelParams == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate memory for kernel object\n");
        status = VX_FAILURE;
    }
    if(status == VX_SUCCESS)
    {
        nodeParams = (tivxImgPreProcParams *)node_params_array_target_ptr;

        memcpy(&kernelParams->nodeParams, nodeParams, sizeof(tivxImgPreProcParams));

        tivx_obj_desc_image_t *in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_INPUT_IMAGE_IDX];

        vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
        vx_uint32 in_height = pIn->dim_y;
        vx_int32  in_stride = pIn->stride_y;

        vx_uint32 num_bytes = 1;

        /* Input is always 8bit, output can be either 8bit or 16bit */
        if(nodeParams->tidl_8bit_16bit_flag == 0)
        {
            num_bytes = 1;
        }
        else if(nodeParams->tidl_8bit_16bit_flag == 1)
        {
            num_bytes = 2;
        }

        vx_uint32 blk_width, rem_height, mblk_height;
        vx_uint32 in_size, out_size, total_size, req_size, avail_size, num_sets;
        blk_width = in_stride / sizeof(vx_uint8);

        tivx_mem_stats l2_stats;
        tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
        tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);

        avail_size = l2_stats.free_size;
        req_size = J721E_C66X_DSP_L2_SRAM_SIZE; /* Out of available 288KB - 64KB is cache and 224KB is SRAM */

        if(req_size > avail_size)
            req_size = avail_size;

        /* This logic tries to find optimum block height which is a
           multiple of 2. It is assumed that DMA will fetch a minimum of
           Width x block_height */
        total_size = 0;
        mblk_height = 1;
        rem_height = 0;
        in_size = 0;
        out_size = 0;
        num_sets = 0;
        while((total_size < req_size) && (rem_height == 0) && ((num_sets & 1) == 0))
        {
            kernelParams->blkWidth = blk_width;
            kernelParams->blkHeight = mblk_height;
            kernelParams->remHeight = rem_height;

            /* Increment mblk_height by 2 */
            mblk_height *= 2;

            if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
               nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
            {
                /* Here 1 RGB interleaved line produces 1 RGB planar line */
                in_size  = (blk_width * mblk_height);
                /* Here output can be 8bit or 16bit hence multiplied by num_bytes */
                out_size = (in_size * num_bytes);
            }
            else if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                    nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
            {
                /* For YUV420SP, 2 luma, 1 chroma lines will produce 2 RGB planar lines */
                in_size  = (blk_width * mblk_height) + (blk_width * (mblk_height >> 1));
                /* Here output can be 8bit or 16bit hence multiplied by num_bytes */
                out_size = (blk_width * mblk_height * 3 * num_bytes);
            }
            /* Double buffer inputs and outputs*/
            in_size = in_size * 2;
            out_size = out_size * 2;

            total_size = in_size + out_size;
            rem_height = in_height % mblk_height;
            num_sets = in_height / mblk_height;

        }

        num_sets = in_height / kernelParams->blkHeight;
        kernelParams->numSets = num_sets;
        kernelParams->avail_size = avail_size;
        kernelParams->req_size = req_size;

        if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
            nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
        {
            /* Here 1 RGB interleaved line produces 1 RGB planar line */
            in_size  = (kernelParams->blkWidth * kernelParams->blkHeight);
            out_size = (in_size * num_bytes);
        }
        else if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
        {
            /* For YUV420SP, 2 luma, 1 chroma lines will produce 2 RGB planar lines */
            in_size  = (kernelParams->blkWidth * kernelParams->blkHeight) + (kernelParams->blkWidth * (kernelParams->blkHeight >> 1));
            out_size = (kernelParams->blkWidth * kernelParams->blkHeight * 3 * num_bytes);
        }

        /* Double buffer inputs and outputs*/
        in_size = in_size * 2;
        out_size = out_size * 2;
        total_size = in_size + out_size;

        VX_PRINT(VX_ZONE_INFO, "blk_width = %d\n", kernelParams->blkWidth);
        VX_PRINT(VX_ZONE_INFO, "blk_height = %d\n", kernelParams->blkHeight);
        VX_PRINT(VX_ZONE_INFO, "num_sets = %d\n", kernelParams->numSets);
        VX_PRINT(VX_ZONE_INFO, "rem_height = %d\n", kernelParams->remHeight);
        VX_PRINT(VX_ZONE_INFO, "in_size = %d\n", in_size);
        VX_PRINT(VX_ZONE_INFO, "out_size = %d\n", out_size);
        VX_PRINT(VX_ZONE_INFO, "total_size = %d\n", total_size);
        VX_PRINT(VX_ZONE_INFO, "req_size = %d\n", req_size);
        VX_PRINT(VX_ZONE_INFO, "avail_size = %d\n", avail_size);
        VX_PRINT(VX_ZONE_INFO, "\n");

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
        kernelParams->pL2 = tivxMemAlloc(kernelParams->req_size, kernelParams->l2_heap_id);
        if(kernelParams->pL2 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate L2 scratch!\n");
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(node_params_array_target_ptr, node_params_array->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
            nodeParams->color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
        {
        /* UDMA get handles 0..7 UDMA block copy channels, 8..15 DRU channels via UDMA */
#ifdef x86_64
            dma_create(&kernelParams->dmaObjRGB, DATA_COPY_CPU, 0);
            dma_create(&kernelParams->dmaObjR, DATA_COPY_CPU, 1);
            dma_create(&kernelParams->dmaObjG, DATA_COPY_CPU, 2);
            dma_create(&kernelParams->dmaObjB, DATA_COPY_CPU, 3);
#else
            dma_create(&kernelParams->dmaObjRGB, DATA_COPY_DMA, 8);
            dma_create(&kernelParams->dmaObjR, DATA_COPY_DMA, 9);
            dma_create(&kernelParams->dmaObjG, DATA_COPY_DMA, 10);
            dma_create(&kernelParams->dmaObjB, DATA_COPY_DMA, 11);
#endif
        }
        else if(nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                nodeParams->color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
        {
        /* UDMA get handles 0..7 UDMA block copy channels, 8..15 DRU channels via UDMA */
#ifdef x86_64
            dma_create(&kernelParams->dmaObjY, DATA_COPY_CPU, 0);
            dma_create(&kernelParams->dmaObjC, DATA_COPY_CPU, 1);
            dma_create(&kernelParams->dmaObjR, DATA_COPY_CPU, 2);
            dma_create(&kernelParams->dmaObjG, DATA_COPY_CPU, 3);
            dma_create(&kernelParams->dmaObjB, DATA_COPY_CPU, 4);
#else
            dma_create(&kernelParams->dmaObjY, DATA_COPY_DMA, 8);
            dma_create(&kernelParams->dmaObjC, DATA_COPY_DMA, 9);
            dma_create(&kernelParams->dmaObjR, DATA_COPY_DMA, 10);
            dma_create(&kernelParams->dmaObjG, DATA_COPY_DMA, 11);
            dma_create(&kernelParams->dmaObjB, DATA_COPY_DMA, 12);
#endif
        }

        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxImgPreProcKernelParams));
    }
    return(status);
}

static vx_status VX_CALLBACK tivxKernelImgPreProcDelete(
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
        tivxImgPreProcKernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
                prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
            {
                dma_delete(&prms->dmaObjRGB);
                dma_delete(&prms->dmaObjR);
                dma_delete(&prms->dmaObjG);
                dma_delete(&prms->dmaObjB);
            }
            else if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                    prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
            {
                dma_delete(&prms->dmaObjY);
                dma_delete(&prms->dmaObjC);
                dma_delete(&prms->dmaObjR);
                dma_delete(&prms->dmaObjG);
                dma_delete(&prms->dmaObjB);
            }
            tivxMemFree(prms->pL2, prms->req_size, prms->l2_heap_id);

            tivxMemFree(prms, sizeof(tivxImgPreProcKernelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelImgPreProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    tivxImgPreProcKernelParams *prms = NULL;

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    uint32_t oldIntState;
    #endif

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    /* disabling interrupts when doing TIDL processing
     *
     * suspect some stability issue due to interrupt handling,
     * until stability issue is root caused disabling interrupts
     * */
    oldIntState = HwiP_disable();
    #endif

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
            (sizeof(tivxImgPreProcKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_array_t* configuration_desc;
        void * configuration_target_ptr;

        tivx_obj_desc_image_t *in_img_desc;
        void* in_image_target_ptr[2];

        tivx_obj_desc_tensor_t *out_tensor_desc;
        void *out_tensor_target_ptr;

        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_CONFIG_IDX];
        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_INPUT_IMAGE_IDX];
        in_image_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_image_target_ptr[0], in_img_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        in_image_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_image_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_image_target_ptr[1], in_img_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        }

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);

        if(prms->nodeParams.clear_count > 0)
        {
            memset(out_tensor_target_ptr, 0, out_tensor_desc->mem_size);
            prms->nodeParams.clear_count--;
        }

        if(prms->nodeParams.tidl_8bit_16bit_flag == 0)
        {
            if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
                prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
            {
                status = img_proc_execute_8bit_RgbiToRgbp(prms, in_img_desc, out_tensor_desc, (void **)in_image_target_ptr, out_tensor_target_ptr);
            }
            else if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                    prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
            {
                status = img_proc_execute_8bit_nv12ToRgbp(prms, in_img_desc, out_tensor_desc, (void **)in_image_target_ptr, out_tensor_target_ptr);
            }
        }
        else if(prms->nodeParams.tidl_8bit_16bit_flag == 1)
        {
            if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB ||
                prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
            {
                status = img_proc_execute_16bit_RgbiToRgbp(prms, in_img_desc, out_tensor_desc, (void **)in_image_target_ptr, out_tensor_target_ptr);
            }
            else if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB ||
                    prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
            {
                status = img_proc_execute_16bit_nv12ToRgbp(prms, in_img_desc, out_tensor_desc, (void **)in_image_target_ptr, out_tensor_target_ptr);
            }
        }

        tivxMemBufferUnmap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_image_target_ptr[0], in_img_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);

        if (in_image_target_ptr[1] != NULL)
        {
          tivxMemBufferUnmap(in_image_target_ptr[1], in_img_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        }
    }

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    HwiP_restore(oldIntState);
    #endif

    return (status);
}
void tivxAddTargetKernelImgPreProc()
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
        vx_img_preproc_target_kernel = tivxAddTargetKernelByName
                                        (
                                            TIVX_KERNEL_IMG_PREPROCESS_NAME,
                                            target_name,
                                            tivxKernelImgPreProcProcess,
                                            tivxKernelImgPreProcCreate,
                                            tivxKernelImgPreProcDelete,
                                            NULL,
                                            NULL
                                        );
    }
}

void tivxRemoveTargetKernelImgPreProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_img_preproc_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_img_preproc_target_kernel = NULL;
    }
}

static vx_status img_proc_execute_8bit_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
)
{
    vx_status status = VX_SUCCESS;

    vx_uint8 *pOutL2[2];

    vx_uint8 *pRGBL2[2];
    vx_uint8 *pRL2[2];
    vx_uint8 *pGL2[2];
    vx_uint8 *pBL2[2];

    vx_uint8 *pRGBDDR = NULL;
    vx_uint8 *pRDDR = NULL;
    vx_uint8 *pGDDR = NULL;
    vx_uint8 *pBDDR = NULL;

    pRGBL2[0] = NULL;
    pRL2[0] = NULL;
    pGL2[0] = NULL;
    pBL2[0] = NULL;

    pRGBL2[1] = NULL;
    pRL2[1] = NULL;
    pGL2[1] = NULL;
    pBL2[1] = NULL;

    vx_int32 data_type = TIADALG_DATA_TYPE_U08;

    vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
    vx_int32  in_img_stride = pIn->stride_y;

    /* Out width is same as In width, out stride will include left and right padding */
    vx_uint32 out_tensor_width  = pIn->dim_x;
    vx_int32  out_tensor_row_stride = out_tensor_desc->stride[1];
    vx_int32  out_tensor_ch_stride  = out_tensor_desc->stride[2];

    vx_uint32 blk_width   = prms->blkWidth;
    vx_uint32 blk_height  = prms->blkHeight;
    vx_uint32 blk_stride  = prms->blkWidth;
    vx_uint32 out_blk_stride = blk_width / pIn->stride_x;

    vx_int32 pad_left = prms->nodeParams.pad_pixel[0];
    vx_int32 pad_top  = prms->nodeParams.pad_pixel[1];

    vx_uint32 num_sets = prms->numSets;
    vx_uint32 start_offset;

    pRGBDDR = (vx_uint8 *)in_image_target_ptr[0];

    /* L2 Y and C ping instance */
    pRGBL2[0] = (vx_uint8 *)prms->pL2;
    pRGBL2[1] = (vx_uint8 *)pRGBL2[0] + (blk_stride * blk_height);

    if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB)
    {
        /* L2 RGB ping instance */
        pRL2[0] = (vx_uint8 *)pRGBL2[1] + (blk_stride * blk_height);
        pGL2[0] = (vx_uint8 *)pRL2[0] + (out_blk_stride * blk_height);
        pBL2[0] = (vx_uint8 *)pGL2[0] + (out_blk_stride * blk_height);

        /* L2 RGB pong instance */
        pRL2[1] = (vx_uint8 *)pBL2[0] + (out_blk_stride * blk_height);
        pGL2[1] = (vx_uint8 *)pRL2[1] + (out_blk_stride * blk_height);
        pBL2[1] = (vx_uint8 *)pGL2[1] + (out_blk_stride * blk_height);

        pOutL2[0] = pRL2[0];
        pOutL2[1] = pRL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pRDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pGDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pBDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

    }
    else if (prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
    {
        /* L2 BGR ping instance */
        pBL2[0] = (vx_uint8 *)pRGBL2[1] + (blk_stride * blk_height);
        pGL2[0] = (vx_uint8 *)pBL2[0] + (out_blk_stride * blk_height);
        pRL2[0] = (vx_uint8 *)pGL2[0] + (out_blk_stride * blk_height);

        /* L2 BGR pong instance */
        pBL2[1] = (vx_uint8 *)pRL2[0] + (out_blk_stride * blk_height);
        pGL2[1] = (vx_uint8 *)pBL2[1] + (out_blk_stride * blk_height);
        pRL2[1] = (vx_uint8 *)pGL2[1] + (out_blk_stride * blk_height);

        pOutL2[0] = pBL2[0];
        pOutL2[1] = pBL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pBDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pGDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pRDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;
    }

    app_udma_copy_nd_prms_t *tfrPrmsRGB = (app_udma_copy_nd_prms_t *)&prms->dmaObjRGB.tfrPrms;

    tfrPrmsRGB->copy_mode    = 2;
    if(prms->dmaObjRGB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsRGB->dest_addr    = ((uintptr_t)pRGBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsRGB->dest_addr    = (uintptr_t)pRGBL2[0];
    }
    tfrPrmsRGB->src_addr     = (uint64_t)pRGBDDR;

    tfrPrmsRGB->icnt0        = blk_width;
    tfrPrmsRGB->icnt1        = blk_height;
    tfrPrmsRGB->icnt2        = 2;
    tfrPrmsRGB->icnt3        = num_sets / 2;
    tfrPrmsRGB->dim1         = in_img_stride;
    tfrPrmsRGB->dim2         = (blk_height * in_img_stride);
    tfrPrmsRGB->dim3         = (blk_height * in_img_stride * 2);

    tfrPrmsRGB->dicnt0       = tfrPrmsRGB->icnt0;
    tfrPrmsRGB->dicnt1       = tfrPrmsRGB->icnt1;
    tfrPrmsRGB->dicnt2       = 2; /* Ping-pong */
    tfrPrmsRGB->dicnt3       = num_sets / 2;
    tfrPrmsRGB->ddim1        = blk_stride;
    tfrPrmsRGB->ddim2        = (blk_height * blk_stride);
    tfrPrmsRGB->ddim3        = 0;

    dma_init(&prms->dmaObjRGB);

    app_udma_copy_nd_prms_t *tfrPrmsR = (app_udma_copy_nd_prms_t *)&prms->dmaObjR.tfrPrms;

    tfrPrmsR->copy_mode    = 2;
    if(prms->dmaObjR.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsR->src_addr    = ((uintptr_t)pRL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsR->src_addr    = (uintptr_t)pRL2[0];
    }
    tfrPrmsR->dest_addr    = (uint64_t)pRDDR;

    tfrPrmsR->icnt0        = out_tensor_width;
    tfrPrmsR->icnt1        = blk_height;
    tfrPrmsR->icnt2        = 2; /* Double buffer */
    tfrPrmsR->icnt3        = num_sets / 2;
    tfrPrmsR->dim1         = out_blk_stride;
    tfrPrmsR->dim2         = (blk_height * out_blk_stride) * 3;
    tfrPrmsR->dim3         = 0;

    tfrPrmsR->dicnt0       = tfrPrmsR->icnt0;
    tfrPrmsR->dicnt1       = tfrPrmsR->icnt1;
    tfrPrmsR->dicnt2       = 2;
    tfrPrmsR->dicnt3       = num_sets / 2;
    tfrPrmsR->ddim1        = out_tensor_row_stride;
    tfrPrmsR->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsR->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjR);

    app_udma_copy_nd_prms_t *tfrPrmsG = (app_udma_copy_nd_prms_t *)&prms->dmaObjG.tfrPrms;

    tfrPrmsG->copy_mode    = 2;
    if(prms->dmaObjG.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsG->src_addr    = ((uintptr_t)pGL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsG->src_addr    = (uintptr_t)pGL2[0];
    }
    tfrPrmsG->dest_addr    = (uint64_t)pGDDR;

    tfrPrmsG->icnt0        = out_tensor_width;
    tfrPrmsG->icnt1        = blk_height;
    tfrPrmsG->icnt2        = 2; /* Double buffer */
    tfrPrmsG->icnt3        = num_sets / 2;
    tfrPrmsG->dim1         = out_blk_stride;
    tfrPrmsG->dim2         = (blk_height * out_blk_stride) * 3;
    tfrPrmsG->dim3         = 0;

    tfrPrmsG->dicnt0       = tfrPrmsG->icnt0;
    tfrPrmsG->dicnt1       = tfrPrmsG->icnt1;
    tfrPrmsG->dicnt2       = 2;
    tfrPrmsG->dicnt3       = num_sets / 2;
    tfrPrmsG->ddim1        = out_tensor_row_stride;
    tfrPrmsG->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsG->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjG);

    app_udma_copy_nd_prms_t *tfrPrmsB = (app_udma_copy_nd_prms_t *)&prms->dmaObjB.tfrPrms;

    tfrPrmsB->copy_mode    = 2;
    if(prms->dmaObjB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsB->src_addr    = ((uintptr_t)pBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsB->src_addr    = (uintptr_t)pBL2[0];
    }
    tfrPrmsB->dest_addr    = (uint64_t)pBDDR;

    tfrPrmsB->icnt0        = out_tensor_width;
    tfrPrmsB->icnt1        = blk_height;
    tfrPrmsB->icnt2        = 2; /* Double buffer */
    tfrPrmsB->icnt3        = num_sets / 2;
    tfrPrmsB->dim1         = out_blk_stride;
    tfrPrmsB->dim2         = (blk_height * out_blk_stride) * 3;
    tfrPrmsB->dim3         = 0;

    tfrPrmsB->dicnt0       = tfrPrmsB->icnt0;
    tfrPrmsB->dicnt1       = tfrPrmsB->icnt1;
    tfrPrmsB->dicnt2       = 2;
    tfrPrmsB->dicnt3       = num_sets / 2;
    tfrPrmsB->ddim1        = out_tensor_row_stride;
    tfrPrmsB->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsB->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjB);

    #if 0
    appUdmaCopyNDPrmsPrint(&prms->dmaObjY->tfrPrms, "Y");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjC->tfrPrms, "C");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjR->tfrPrms, "R");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjG->tfrPrms, "G");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjB->tfrPrms, "B");
    #endif

    status = img_proc_pipeline_blocks_RgbiToRgbp(prms,
                                                pRGBL2,
                                                (void **)pOutL2,
                                                out_blk_stride,
                                                blk_height,
                                                blk_stride,
                                                num_sets,
                                                data_type);

    dma_deinit(&prms->dmaObjRGB);
    dma_deinit(&prms->dmaObjR);
    dma_deinit(&prms->dmaObjG);
    dma_deinit(&prms->dmaObjB);

    return status;
}

static vx_status img_proc_execute_8bit_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
)
{
    vx_status status = VX_SUCCESS;

    vx_uint8 *pOutL2[2];

    vx_uint8 *pYL2[2];
    vx_uint8 *pCL2[2];
    vx_uint8 *pRL2[2];
    vx_uint8 *pGL2[2];
    vx_uint8 *pBL2[2];

    vx_uint8 *pYDDR = NULL;
    vx_uint8 *pCDDR = NULL;
    vx_uint8 *pRDDR = NULL;
    vx_uint8 *pGDDR = NULL;
    vx_uint8 *pBDDR = NULL;

    pYL2[0] = NULL;
    pCL2[0] = NULL;
    pRL2[0] = NULL;
    pGL2[0] = NULL;
    pBL2[0] = NULL;

    pYL2[1] = NULL;
    pCL2[1] = NULL;
    pRL2[1] = NULL;
    pGL2[1] = NULL;
    pBL2[1] = NULL;

    vx_int32 data_type = TIADALG_DATA_TYPE_U08;

    vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
    vx_uint32 in_img_width  = pIn->dim_x;
    vx_int32  in_img_stride = pIn->stride_y;

    /* Out width is same as In width, out stride will include left and right padding */
    vx_uint32 out_tensor_width  = pIn->dim_x;
    vx_int32  out_tensor_row_stride = out_tensor_desc->stride[1];
    vx_int32  out_tensor_ch_stride  = out_tensor_desc->stride[2];

    vx_uint32 blk_width   = prms->blkWidth;
    vx_uint32 blk_height  = prms->blkHeight;
    vx_uint32 blk_stride  = prms->blkWidth;

    vx_int32 pad_left = prms->nodeParams.pad_pixel[0];
    vx_int32 pad_top  = prms->nodeParams.pad_pixel[1];

    vx_uint32 num_sets = prms->numSets;
    vx_uint32 start_offset;

    pYDDR = (vx_uint8 *)in_image_target_ptr[0];
    pCDDR = (vx_uint8 *)in_image_target_ptr[1];

    /* L2 Y and C ping instance */
    pYL2[0] = (vx_uint8 *)prms->pL2;
    pYL2[1] = (vx_uint8 *)pYL2[0] + (blk_stride * blk_height);

    /* L2 Y and C pong instance */
    pCL2[0] = (vx_uint8 *)pYL2[1] + (blk_stride * blk_height);
    pCL2[1] = (vx_uint8 *)pCL2[0] + (blk_stride * (blk_height >> 1));

    if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB)
    {
        /* L2 RGB ping instance */
        pRL2[0] = (vx_uint8 *)pCL2[1] + (blk_stride * (blk_height >> 1));
        pGL2[0] = (vx_uint8 *)pRL2[0] + (blk_stride * blk_height);
        pBL2[0] = (vx_uint8 *)pGL2[0] + (blk_stride * blk_height);

        /* L2 RGB pong instance */
        pRL2[1] = (vx_uint8 *)pBL2[0] + (blk_stride * blk_height);
        pGL2[1] = (vx_uint8 *)pRL2[1] + (blk_stride * blk_height);
        pBL2[1] = (vx_uint8 *)pGL2[1] + (blk_stride * blk_height);

        pOutL2[0] = pRL2[0];
        pOutL2[1] = pRL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pRDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pGDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pBDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

    }
    else if (prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
    {
        /* L2 BGR ping instance */
        pBL2[0] = (vx_uint8 *)pCL2[1] + (blk_stride * (blk_height >> 1));
        pGL2[0] = (vx_uint8 *)pBL2[0] + (blk_stride * blk_height);
        pRL2[0] = (vx_uint8 *)pGL2[0] + (blk_stride * blk_height);

        /* L2 BGR pong instance */
        pBL2[1] = (vx_uint8 *)pRL2[0] + (blk_stride * blk_height);
        pGL2[1] = (vx_uint8 *)pBL2[1] + (blk_stride * blk_height);
        pRL2[1] = (vx_uint8 *)pGL2[1] + (blk_stride * blk_height);

        pOutL2[0] = pBL2[0];
        pOutL2[1] = pBL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pBDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pGDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + pad_left;
        pRDDR = (vx_uint8 *)out_tensor_target_ptr + start_offset;
    }

    app_udma_copy_nd_prms_t *tfrPrmsY = (app_udma_copy_nd_prms_t *)&prms->dmaObjY.tfrPrms;

    tfrPrmsY->copy_mode    = 2;
    if(prms->dmaObjY.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsY->dest_addr    = ((uintptr_t)pYL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsY->dest_addr    = (uintptr_t)pYL2[0];
    }
    tfrPrmsY->src_addr     = (uint64_t)pYDDR;

    tfrPrmsY->icnt0        = in_img_width;
    tfrPrmsY->icnt1        = blk_height;
    tfrPrmsY->icnt2        = 2;
    tfrPrmsY->icnt3        = num_sets / 2;
    tfrPrmsY->dim1         = in_img_stride;
    tfrPrmsY->dim2         = (blk_height * in_img_stride);
    tfrPrmsY->dim3         = (blk_height * in_img_stride * 2);

    tfrPrmsY->dicnt0       = tfrPrmsY->icnt0;
    tfrPrmsY->dicnt1       = tfrPrmsY->icnt1;
    tfrPrmsY->dicnt2       = 2; /* Ping-pong */
    tfrPrmsY->dicnt3       = num_sets / 2;
    tfrPrmsY->ddim1        = blk_stride;
    tfrPrmsY->ddim2        = (blk_height * blk_stride);
    tfrPrmsY->ddim3        = 0;

    dma_init(&prms->dmaObjY);

    app_udma_copy_nd_prms_t *tfrPrmsC = (app_udma_copy_nd_prms_t *)&prms->dmaObjC.tfrPrms;

    tfrPrmsC->copy_mode    = 2;
    if(prms->dmaObjC.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsC->dest_addr    = ((uintptr_t)pCL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsC->dest_addr    = (uintptr_t)pCL2[0];
    }
    tfrPrmsC->src_addr     = (uint64_t)pCDDR;

    tfrPrmsC->icnt0        = in_img_width;
    tfrPrmsC->icnt1        = blk_height >> 1;
    tfrPrmsC->icnt2        = 2;
    tfrPrmsC->icnt3        = num_sets / 2;
    tfrPrmsC->dim1         = in_img_stride;
    tfrPrmsC->dim2         = ((blk_height >> 1) * in_img_stride);
    tfrPrmsC->dim3         = ((blk_height >> 1) * in_img_stride * 2);

    tfrPrmsC->dicnt0       = tfrPrmsC->icnt0;
    tfrPrmsC->dicnt1       = tfrPrmsC->icnt1;
    tfrPrmsC->dicnt2       = 2; /* Ping-pong */
    tfrPrmsC->dicnt3       = num_sets / 2;
    tfrPrmsC->ddim1        = blk_stride;
    tfrPrmsC->ddim2        = ((blk_height >> 1) * blk_stride);
    tfrPrmsC->ddim3        = 0;

    dma_init(&prms->dmaObjC);

    app_udma_copy_nd_prms_t *tfrPrmsR = (app_udma_copy_nd_prms_t *)&prms->dmaObjR.tfrPrms;

    tfrPrmsR->copy_mode    = 2;
    if(prms->dmaObjR.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsR->src_addr    = ((uintptr_t)pRL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsR->src_addr    = (uintptr_t)pRL2[0];
    }
    tfrPrmsR->dest_addr    = (uint64_t)pRDDR;

    tfrPrmsR->icnt0        = out_tensor_width;
    tfrPrmsR->icnt1        = blk_height;
    tfrPrmsR->icnt2        = 2; /* Double buffer */
    tfrPrmsR->icnt3        = num_sets / 2;
    tfrPrmsR->dim1         = blk_stride;
    tfrPrmsR->dim2         = (blk_height * blk_stride) * 3;
    tfrPrmsR->dim3         = 0;

    tfrPrmsR->dicnt0       = tfrPrmsR->icnt0;
    tfrPrmsR->dicnt1       = tfrPrmsR->icnt1;
    tfrPrmsR->dicnt2       = 2;
    tfrPrmsR->dicnt3       = num_sets / 2;
    tfrPrmsR->ddim1        = out_tensor_row_stride;
    tfrPrmsR->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsR->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjR);

    app_udma_copy_nd_prms_t *tfrPrmsG = (app_udma_copy_nd_prms_t *)&prms->dmaObjG.tfrPrms;

    tfrPrmsG->copy_mode    = 2;
    if(prms->dmaObjG.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsG->src_addr    = ((uintptr_t)pGL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsG->src_addr    = (uintptr_t)pGL2[0];
    }
    tfrPrmsG->dest_addr    = (uint64_t)pGDDR;

    tfrPrmsG->icnt0        = out_tensor_width;
    tfrPrmsG->icnt1        = blk_height;
    tfrPrmsG->icnt2        = 2; /* Double buffer */
    tfrPrmsG->icnt3        = num_sets / 2;
    tfrPrmsG->dim1         = blk_stride;
    tfrPrmsG->dim2         = (blk_height * blk_stride) * 3;
    tfrPrmsG->dim3         = 0;

    tfrPrmsG->dicnt0       = tfrPrmsG->icnt0;
    tfrPrmsG->dicnt1       = tfrPrmsG->icnt1;
    tfrPrmsG->dicnt2       = 2;
    tfrPrmsG->dicnt3       = num_sets / 2;
    tfrPrmsG->ddim1        = out_tensor_row_stride;
    tfrPrmsG->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsG->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjG);

    app_udma_copy_nd_prms_t *tfrPrmsB = (app_udma_copy_nd_prms_t *)&prms->dmaObjB.tfrPrms;

    tfrPrmsB->copy_mode    = 2;
    if(prms->dmaObjB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsB->src_addr    = ((uintptr_t)pBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsB->src_addr    = (uintptr_t)pBL2[0];
    }
    tfrPrmsB->dest_addr    = (uint64_t)pBDDR;

    tfrPrmsB->icnt0        = out_tensor_width;
    tfrPrmsB->icnt1        = blk_height;
    tfrPrmsB->icnt2        = 2; /* Double buffer */
    tfrPrmsB->icnt3        = num_sets / 2;
    tfrPrmsB->dim1         = blk_stride;
    tfrPrmsB->dim2         = (blk_height * blk_stride) * 3;
    tfrPrmsB->dim3         = 0;

    tfrPrmsB->dicnt0       = tfrPrmsB->icnt0;
    tfrPrmsB->dicnt1       = tfrPrmsB->icnt1;
    tfrPrmsB->dicnt2       = 2;
    tfrPrmsB->dicnt3       = num_sets / 2;
    tfrPrmsB->ddim1        = out_tensor_row_stride;
    tfrPrmsB->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsB->ddim3        = (blk_height * out_tensor_row_stride * 2);

    dma_init(&prms->dmaObjB);

    #if 0
    appUdmaCopyNDPrmsPrint(&prms->dmaObjY->tfrPrms, "Y");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjC->tfrPrms, "C");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjR->tfrPrms, "R");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjG->tfrPrms, "G");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjB->tfrPrms, "B");
    #endif

    status = img_proc_pipeline_blocks_nv12ToRgbp(prms,
                                                pYL2,
                                                pCL2,
                                                (void **)pOutL2,
                                                blk_width,
                                                blk_height,
                                                blk_stride,
                                                num_sets,
                                                data_type);

    dma_deinit(&prms->dmaObjY);
    dma_deinit(&prms->dmaObjC);
    dma_deinit(&prms->dmaObjR);
    dma_deinit(&prms->dmaObjG);
    dma_deinit(&prms->dmaObjB);

    return status;

}

static vx_status img_proc_execute_16bit_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
)
{
    vx_status status = VX_SUCCESS;

    vx_uint16 *pOutL2[2];

    vx_uint8 *pRGBL2[2];
    vx_uint16 *pRL2[2];
    vx_uint16 *pGL2[2];
    vx_uint16 *pBL2[2];

    vx_uint8 *pRGBDDR = NULL;
    vx_uint16 *pRDDR = NULL;
    vx_uint16 *pGDDR = NULL;
    vx_uint16 *pBDDR = NULL;

    pRGBL2[0] = NULL;
    pRL2[0] = NULL;
    pGL2[0] = NULL;
    pBL2[0] = NULL;

    pRGBL2[1] = NULL;
    pRL2[1] = NULL;
    pGL2[1] = NULL;
    pBL2[1] = NULL;

    vx_int32 data_type = TIADALG_DATA_TYPE_U16;

    vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
    vx_uint32 in_img_width  = pIn->dim_x;
    vx_int32  in_img_stride = pIn->stride_y;

    /* Out width is same as In width, out stride will include left and right padding */
    vx_uint32 out_tensor_width  = pIn->dim_x;
    vx_int32  out_tensor_row_stride = out_tensor_desc->stride[1];
    vx_int32  out_tensor_ch_stride  = out_tensor_desc->stride[2];

    vx_uint32 blk_width   = prms->blkWidth;
    vx_uint32 blk_height  = prms->blkHeight;
    vx_uint32 blk_stride  = prms->blkWidth;
    vx_uint32 out_blk_stride = blk_width / pIn->stride_x;

    vx_int32 pad_left = prms->nodeParams.pad_pixel[0];
    vx_int32 pad_top  = prms->nodeParams.pad_pixel[1];

    vx_uint32 num_sets = prms->numSets;
    vx_uint32 start_offset;

    pRGBDDR = (vx_uint8 *)in_image_target_ptr[0];

    /* L2 Y and C ping instance */
    pRGBL2[0] = (vx_uint8 *)prms->pL2;
    pRGBL2[1] = (vx_uint8 *)pRGBL2[0] + (blk_stride * blk_height);

    if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_RGB)
    {
        /* L2 RGB ping instance */
        pRL2[0] = (vx_uint16 *)((vx_uint8 *)pRGBL2[1] + (blk_stride * blk_height));
        pGL2[0] = (vx_uint16 *)pRL2[0] + (out_blk_stride * blk_height);
        pBL2[0] = (vx_uint16 *)pGL2[0] + (out_blk_stride * blk_height);

        /* L2 RGB pong instance */
        pRL2[1] = (vx_uint16 *)pBL2[0] + (out_blk_stride * blk_height);
        pGL2[1] = (vx_uint16 *)pRL2[1] + (out_blk_stride * blk_height);
        pBL2[1] = (vx_uint16 *)pGL2[1] + (out_blk_stride * blk_height);

        pOutL2[0] = pRL2[0];
        pOutL2[1] = pRL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pRDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pGDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pBDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

    }
    else if (prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_RGBINTERLEAVE_BGR)
    {
        /* L2 BGR ping instance */
        pBL2[0] = (vx_uint16 *)((vx_uint8 *)pRGBL2[1] + (blk_stride * blk_height));
        pGL2[0] = (vx_uint16 *)pBL2[0] + (out_blk_stride * blk_height);
        pRL2[0] = (vx_uint16 *)pGL2[0] + (out_blk_stride * blk_height);

        /* L2 BGR pong instance */
        pBL2[1] = (vx_uint16 *)pRL2[0] + (out_blk_stride * blk_height);
        pGL2[1] = (vx_uint16 *)pBL2[1] + (out_blk_stride * blk_height);
        pRL2[1] = (vx_uint16 *)pGL2[1] + (out_blk_stride * blk_height);

        pOutL2[0] = pBL2[0];
        pOutL2[1] = pBL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pBDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pGDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pRDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);
    }

    app_udma_copy_nd_prms_t *tfrPrmsRGB = (app_udma_copy_nd_prms_t *)&prms->dmaObjRGB.tfrPrms;

    tfrPrmsRGB->copy_mode    = 2;
    if(prms->dmaObjRGB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsRGB->dest_addr    = ((uintptr_t)pRGBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsRGB->dest_addr    = (uintptr_t)pRGBL2[0];
    }
    tfrPrmsRGB->src_addr     = (uint64_t)pRGBDDR;

    tfrPrmsRGB->icnt0        = in_img_width;
    tfrPrmsRGB->icnt1        = blk_height;
    tfrPrmsRGB->icnt2        = 2;
    tfrPrmsRGB->icnt3        = num_sets / 2;
    tfrPrmsRGB->dim1         = in_img_stride;
    tfrPrmsRGB->dim2         = (blk_height * in_img_stride);
    tfrPrmsRGB->dim3         = (blk_height * in_img_stride * 2);

    tfrPrmsRGB->dicnt0       = tfrPrmsRGB->icnt0;
    tfrPrmsRGB->dicnt1       = tfrPrmsRGB->icnt1;
    tfrPrmsRGB->dicnt2       = 2; /* Ping-pong */
    tfrPrmsRGB->dicnt3       = num_sets / 2;
    tfrPrmsRGB->ddim1        = blk_stride;
    tfrPrmsRGB->ddim2        = (blk_height * blk_stride);
    tfrPrmsRGB->ddim3        = 0;

    dma_init(&prms->dmaObjY);

    app_udma_copy_nd_prms_t *tfrPrmsR = (app_udma_copy_nd_prms_t *)&prms->dmaObjR.tfrPrms;

    /* Stride should be in elements not bytes */
    //out_tensor_row_stride = out_tensor_row_stride / sizeof(uint16_t);

    tfrPrmsR->copy_mode    = 2;
    if(prms->dmaObjR.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsR->src_addr    = ((uintptr_t)pRL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsR->src_addr    = (uintptr_t)pRL2[0];
    }
    tfrPrmsR->dest_addr    = (uint64_t)pRDDR;

    tfrPrmsR->icnt0        = out_tensor_width;
    tfrPrmsR->icnt1        = blk_height;
    tfrPrmsR->icnt2        = 2; /* Double buffer */
    tfrPrmsR->icnt3        = num_sets / 2;
    tfrPrmsR->dim1         = out_blk_stride * sizeof(uint16_t);
    tfrPrmsR->dim2         = (blk_height * out_blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsR->dim3         = 0;

    tfrPrmsR->dicnt0       = tfrPrmsR->icnt0;
    tfrPrmsR->dicnt1       = tfrPrmsR->icnt1;
    tfrPrmsR->dicnt2       = 2;
    tfrPrmsR->dicnt3       = num_sets / 2;
    tfrPrmsR->ddim1        = out_tensor_row_stride;
    tfrPrmsR->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsR->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsR->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjR);

    app_udma_copy_nd_prms_t *tfrPrmsG = (app_udma_copy_nd_prms_t *)&prms->dmaObjG.tfrPrms;

    tfrPrmsG->copy_mode    = 2;
    if(prms->dmaObjG.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsG->src_addr    = ((uintptr_t)pGL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsG->src_addr    = (uintptr_t)pGL2[0];
    }
    tfrPrmsG->dest_addr    = (uint64_t)pGDDR;

    tfrPrmsG->icnt0        = out_tensor_width;
    tfrPrmsG->icnt1        = blk_height;
    tfrPrmsG->icnt2        = 2; /* Double buffer */
    tfrPrmsG->icnt3        = num_sets / 2;
    tfrPrmsG->dim1         = out_blk_stride * sizeof(uint16_t);
    tfrPrmsG->dim2         = (blk_height * out_blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsG->dim3         = 0;

    tfrPrmsG->dicnt0       = tfrPrmsG->icnt0;
    tfrPrmsG->dicnt1       = tfrPrmsG->icnt1;
    tfrPrmsG->dicnt2       = 2;
    tfrPrmsG->dicnt3       = num_sets / 2;
    tfrPrmsG->ddim1        = out_tensor_row_stride;
    tfrPrmsG->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsG->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsG->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjG);

    app_udma_copy_nd_prms_t *tfrPrmsB = (app_udma_copy_nd_prms_t *)&prms->dmaObjB.tfrPrms;

    tfrPrmsB->copy_mode    = 2;
    if(prms->dmaObjB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsB->src_addr    = ((uintptr_t)pBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsB->src_addr    = (uintptr_t)pBL2[0];
    }
    tfrPrmsB->dest_addr    = (uint64_t)pBDDR;

    tfrPrmsB->icnt0        = out_tensor_width;
    tfrPrmsB->icnt1        = blk_height;
    tfrPrmsB->icnt2        = 2; /* Double buffer */
    tfrPrmsB->icnt3        = num_sets / 2;
    tfrPrmsB->dim1         = out_blk_stride * sizeof(uint16_t);
    tfrPrmsB->dim2         = (blk_height * out_blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsB->dim3         = 0;

    tfrPrmsB->dicnt0       = tfrPrmsB->icnt0;
    tfrPrmsB->dicnt1       = tfrPrmsB->icnt1;
    tfrPrmsB->dicnt2       = 2;
    tfrPrmsB->dicnt3       = num_sets / 2;
    tfrPrmsB->ddim1        = out_tensor_row_stride;
    tfrPrmsB->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsB->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsB->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjB);

    #if 0
    appUdmaCopyNDPrmsPrint(&prms->dmaObjRGB->tfrPrms, "RGB");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjR->tfrPrms, "R");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjG->tfrPrms, "G");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjB->tfrPrms, "B");
    #endif

    status = img_proc_pipeline_blocks_RgbiToRgbp(prms,
                                                pRGBL2,
                                                (void **)pOutL2,
                                                out_blk_stride,
                                                blk_height,
                                                blk_stride,
                                                num_sets,
                                                data_type);

    dma_deinit(&prms->dmaObjRGB);
    dma_deinit(&prms->dmaObjR);
    dma_deinit(&prms->dmaObjG);
    dma_deinit(&prms->dmaObjB);

    return status;
}

static vx_status img_proc_execute_16bit_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_tensor_t *out_tensor_desc,
    void **in_image_target_ptr,
    void *out_tensor_target_ptr
)
{
    vx_status status = VX_SUCCESS;

    vx_uint16 *pOutL2[2];

    vx_uint8 *pYL2[2];
    vx_uint8 *pCL2[2];
    vx_uint16 *pRL2[2];
    vx_uint16 *pGL2[2];
    vx_uint16 *pBL2[2];

    vx_uint8 *pYDDR = NULL;
    vx_uint8 *pCDDR = NULL;
    vx_uint16 *pRDDR = NULL;
    vx_uint16 *pGDDR = NULL;
    vx_uint16 *pBDDR = NULL;

    pYL2[0] = NULL;
    pCL2[0] = NULL;
    pRL2[0] = NULL;
    pGL2[0] = NULL;
    pBL2[0] = NULL;

    pYL2[1] = NULL;
    pCL2[1] = NULL;
    pRL2[1] = NULL;
    pGL2[1] = NULL;
    pBL2[1] = NULL;

    vx_int32 data_type = TIADALG_DATA_TYPE_U16;

    vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
    vx_uint32 in_img_width  = pIn->dim_x;
    vx_int32  in_img_stride = pIn->stride_y;

    /* Out width is same as In width, out stride will include left and right padding */
    vx_uint32 out_tensor_width  = pIn->dim_x;
    vx_int32  out_tensor_row_stride = out_tensor_desc->stride[1];
    vx_int32  out_tensor_ch_stride  = out_tensor_desc->stride[2];

    vx_uint32 blk_width   = prms->blkWidth;
    vx_uint32 blk_height  = prms->blkHeight;
    vx_uint32 blk_stride  = prms->blkWidth;

    vx_int32 pad_left = prms->nodeParams.pad_pixel[0];
    vx_int32 pad_top  = prms->nodeParams.pad_pixel[1];

    vx_uint32 num_sets = prms->numSets;
    vx_uint32 start_offset;

    pYDDR = (vx_uint8 *)in_image_target_ptr[0];
    pCDDR = (vx_uint8 *)in_image_target_ptr[1];

    /* L2 Y and C ping instance */
    pYL2[0] = (vx_uint8 *)prms->pL2;
    pYL2[1] = (vx_uint8 *)pYL2[0] + (blk_stride * blk_height);

    /* L2 Y and C pong instance */
    pCL2[0] = (vx_uint8 *)pYL2[1] + (blk_stride * blk_height);
    pCL2[1] = (vx_uint8 *)pCL2[0] + (blk_stride * (blk_height >> 1));


    if(prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_RGB)
    {
        /* L2 RGB ping instance */
        pRL2[0] = (vx_uint16 *)((vx_uint8 *)pCL2[1] + (blk_stride * (blk_height >> 1)));
        pGL2[0] = (vx_uint16 *)pRL2[0] + (blk_stride * blk_height);
        pBL2[0] = (vx_uint16 *)pGL2[0] + (blk_stride * blk_height);

        /* L2 RGB pong instance */
        pRL2[1] = (vx_uint16 *)pBL2[0] + (blk_stride * blk_height);
        pGL2[1] = (vx_uint16 *)pRL2[1] + (blk_stride * blk_height);
        pBL2[1] = (vx_uint16 *)pGL2[1] + (blk_stride * blk_height);

        pOutL2[0] = pRL2[0];
        pOutL2[1] = pRL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pRDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pGDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pBDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

    }
    else if (prms->nodeParams.color_conv_flag == TIADALG_COLOR_CONV_YUV420_BGR)
    {
        /* L2 BGR ping instance */
        pBL2[0] = (vx_uint16 *)((vx_uint8 *)pCL2[1] + (blk_stride * (blk_height >> 1)));
        pGL2[0] = (vx_uint16 *)pBL2[0] + (blk_stride * blk_height);
        pRL2[0] = (vx_uint16 *)pGL2[0] + (blk_stride * blk_height);

        /* L2 BGR pong instance */
        pBL2[1] = (vx_uint16 *)pRL2[0] + (blk_stride * blk_height);
        pGL2[1] = (vx_uint16 *)pBL2[1] + (blk_stride * blk_height);
        pRL2[1] = (vx_uint16 *)pGL2[1] + (blk_stride * blk_height);

        pOutL2[0] = pBL2[0];
        pOutL2[1] = pBL2[1];

        start_offset = (0 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pBDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (1 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pGDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

        start_offset = (2 * out_tensor_ch_stride) + (pad_top * out_tensor_row_stride) + (pad_left * sizeof(uint16_t));
        pRDDR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);
    }

    app_udma_copy_nd_prms_t *tfrPrmsY = (app_udma_copy_nd_prms_t *)&prms->dmaObjY.tfrPrms;

    tfrPrmsY->copy_mode    = 2;
    if(prms->dmaObjY.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsY->dest_addr    = ((uintptr_t)pYL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsY->dest_addr    = (uintptr_t)pYL2[0];
    }
    tfrPrmsY->src_addr     = (uint64_t)pYDDR;

    tfrPrmsY->icnt0        = in_img_width;
    tfrPrmsY->icnt1        = blk_height;
    tfrPrmsY->icnt2        = 2;
    tfrPrmsY->icnt3        = num_sets / 2;
    tfrPrmsY->dim1         = in_img_stride;
    tfrPrmsY->dim2         = (blk_height * in_img_stride);
    tfrPrmsY->dim3         = (blk_height * in_img_stride * 2);

    tfrPrmsY->dicnt0       = tfrPrmsY->icnt0;
    tfrPrmsY->dicnt1       = tfrPrmsY->icnt1;
    tfrPrmsY->dicnt2       = 2; /* Ping-pong */
    tfrPrmsY->dicnt3       = num_sets / 2;
    tfrPrmsY->ddim1        = blk_stride;
    tfrPrmsY->ddim2        = (blk_height * blk_stride);
    tfrPrmsY->ddim3        = 0;

    dma_init(&prms->dmaObjY);

    app_udma_copy_nd_prms_t *tfrPrmsC = (app_udma_copy_nd_prms_t *)&prms->dmaObjC.tfrPrms;

    tfrPrmsC->copy_mode    = 2;
    if(prms->dmaObjC.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsC->dest_addr    = ((uintptr_t)pCL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsC->dest_addr    = (uintptr_t)pCL2[0];
    }
    tfrPrmsC->src_addr     = (uint64_t)pCDDR;

    tfrPrmsC->icnt0        = in_img_width;
    tfrPrmsC->icnt1        = blk_height >> 1;
    tfrPrmsC->icnt2        = 2;
    tfrPrmsC->icnt3        = num_sets / 2;
    tfrPrmsC->dim1         = in_img_stride;
    tfrPrmsC->dim2         = ((blk_height >> 1) * in_img_stride);
    tfrPrmsC->dim3         = ((blk_height >> 1) * in_img_stride * 2);

    tfrPrmsC->dicnt0       = tfrPrmsC->icnt0;
    tfrPrmsC->dicnt1       = tfrPrmsC->icnt1;
    tfrPrmsC->dicnt2       = 2; /* Ping-pong */
    tfrPrmsC->dicnt3       = num_sets / 2;
    tfrPrmsC->ddim1        = blk_stride;
    tfrPrmsC->ddim2        = ((blk_height >> 1) * blk_stride);
    tfrPrmsC->ddim3        = 0;

    dma_init(&prms->dmaObjC);

    app_udma_copy_nd_prms_t *tfrPrmsR = (app_udma_copy_nd_prms_t *)&prms->dmaObjR.tfrPrms;

    /* Stride should be in elements not bytes */
    //out_tensor_row_stride = out_tensor_row_stride / sizeof(uint16_t);

    tfrPrmsR->copy_mode    = 2;
    if(prms->dmaObjR.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsR->src_addr    = ((uintptr_t)pRL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsR->src_addr    = (uintptr_t)pRL2[0];
    }
    tfrPrmsR->dest_addr    = (uint64_t)pRDDR;

    tfrPrmsR->icnt0        = out_tensor_width;
    tfrPrmsR->icnt1        = blk_height;
    tfrPrmsR->icnt2        = 2; /* Double buffer */
    tfrPrmsR->icnt3        = num_sets / 2;
    tfrPrmsR->dim1         = blk_stride * sizeof(uint16_t);
    tfrPrmsR->dim2         = (blk_height * blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsR->dim3         = 0;

    tfrPrmsR->dicnt0       = tfrPrmsR->icnt0;
    tfrPrmsR->dicnt1       = tfrPrmsR->icnt1;
    tfrPrmsR->dicnt2       = 2;
    tfrPrmsR->dicnt3       = num_sets / 2;
    tfrPrmsR->ddim1        = out_tensor_row_stride;
    tfrPrmsR->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsR->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsR->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjR);

    app_udma_copy_nd_prms_t *tfrPrmsG = (app_udma_copy_nd_prms_t *)&prms->dmaObjG.tfrPrms;

    tfrPrmsG->copy_mode    = 2;
    if(prms->dmaObjG.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsG->src_addr    = ((uintptr_t)pGL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsG->src_addr    = (uintptr_t)pGL2[0];
    }
    tfrPrmsG->dest_addr    = (uint64_t)pGDDR;

    tfrPrmsG->icnt0        = out_tensor_width;
    tfrPrmsG->icnt1        = blk_height;
    tfrPrmsG->icnt2        = 2; /* Double buffer */
    tfrPrmsG->icnt3        = num_sets / 2;
    tfrPrmsG->dim1         = blk_stride * sizeof(uint16_t);
    tfrPrmsG->dim2         = (blk_height * blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsG->dim3         = 0;

    tfrPrmsG->dicnt0       = tfrPrmsG->icnt0;
    tfrPrmsG->dicnt1       = tfrPrmsG->icnt1;
    tfrPrmsG->dicnt2       = 2;
    tfrPrmsG->dicnt3       = num_sets / 2;
    tfrPrmsG->ddim1        = out_tensor_row_stride;
    tfrPrmsG->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsG->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsG->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjG);

    app_udma_copy_nd_prms_t *tfrPrmsB = (app_udma_copy_nd_prms_t *)&prms->dmaObjB.tfrPrms;

    tfrPrmsB->copy_mode    = 2;
    if(prms->dmaObjB.transfer_type == DATA_COPY_DMA)
    {
        tfrPrmsB->src_addr    = ((uintptr_t)pBL2[0] + prms->l2_global_base);
    }
    else
    {
        tfrPrmsB->src_addr    = (uintptr_t)pBL2[0];
    }
    tfrPrmsB->dest_addr    = (uint64_t)pBDDR;

    tfrPrmsB->icnt0        = out_tensor_width;
    tfrPrmsB->icnt1        = blk_height;
    tfrPrmsB->icnt2        = 2; /* Double buffer */
    tfrPrmsB->icnt3        = num_sets / 2;
    tfrPrmsB->dim1         = blk_stride * sizeof(uint16_t);
    tfrPrmsB->dim2         = (blk_height * blk_stride) * 3 * sizeof(uint16_t);
    tfrPrmsB->dim3         = 0;

    tfrPrmsB->dicnt0       = tfrPrmsB->icnt0;
    tfrPrmsB->dicnt1       = tfrPrmsB->icnt1;
    tfrPrmsB->dicnt2       = 2;
    tfrPrmsB->dicnt3       = num_sets / 2;
    tfrPrmsB->ddim1        = out_tensor_row_stride;
    tfrPrmsB->ddim2        = (blk_height * out_tensor_row_stride);
    tfrPrmsB->ddim3        = (blk_height * out_tensor_row_stride * 2);

    /* Indicate 2 bytes per element for transferring 16bit data */
    tfrPrmsB->eltype = sizeof(uint16_t);

    dma_init(&prms->dmaObjB);

    #if 0
    appUdmaCopyNDPrmsPrint(&prms->dmaObjY->tfrPrms, "Y");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjC->tfrPrms, "C");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjR->tfrPrms, "R");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjG->tfrPrms, "G");
    appUdmaCopyNDPrmsPrint(&prms->dmaObjB->tfrPrms, "B");
    #endif

    status = img_proc_pipeline_blocks_nv12ToRgbp(prms,
                                                pYL2,
                                                pCL2,
                                                (void **)pOutL2,
                                                blk_width,
                                                blk_height,
                                                blk_stride,
                                                num_sets,
                                                data_type);

    dma_deinit(&prms->dmaObjY);
    dma_deinit(&prms->dmaObjC);
    dma_deinit(&prms->dmaObjR);
    dma_deinit(&prms->dmaObjG);
    dma_deinit(&prms->dmaObjB);

    return status;
}

static vx_status img_proc_pipeline_blocks_RgbiToRgbp
(
    tivxImgPreProcKernelParams *prms,
    vx_uint8 *pRGBL2[2],
    void *pOutL2[2],
    uint32_t blk_width,
    uint32_t blk_height,
    uint32_t blk_stride,
    uint32_t num_sets,
    int32_t data_type
)
{
    vx_status status = VX_SUCCESS;

    /* Padding is taken care by DMA, so kernel is given zero padding */
    vx_int32 pad_pixel_zero[4] = {0};

    vx_uint32 pipeup = 2;
    vx_uint32 pipedown = 2;
    vx_uint32 exec_cmd = 1;
    vx_uint32 pipeline = exec_cmd;
    vx_uint32 ping_npong = 0;

    vx_uint32 blk;

    for(blk = 0; blk < (num_sets + pipeup + pipedown); blk++)
    {

        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjRGB);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjR);
            dma_transfer_trigger(&prms->dmaObjG);
            dma_transfer_trigger(&prms->dmaObjB);
        }

        if(EXEC_PIPELINE_STAGE2(pipeline))
        {

            vx_uint8 *pInL2[2];

            pInL2[0] = pRGBL2[ping_npong];

#if 0
            VX_PRINT(VX_ZONE_ERROR, "[%s] [%d] [%d] [%d] [%d] [%d] [%f] %f]\n",
                     __FUNCTION__,
                     blk_width,
                     blk_height,
                     blk_stride,
                     data_type,
                     prms->nodeParams.color_conv_flag,
                     prms->nodeParams.scale_val,
                     prms->nodeParams.mean_pixel);
#endif

            status = tiadalg_image_preprocessing_c66
                    (
                        (void *)pInL2,
                        blk_width,
                        blk_height,
                        blk_stride,
                        data_type,
                        prms->nodeParams.color_conv_flag,
                        prms->nodeParams.scale_val,
                        prms->nodeParams.mean_pixel,
                        pad_pixel_zero,
                        (void *)pOutL2[ping_npong]
                    );

            ping_npong = !ping_npong;
        }

        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjRGB);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjR);
            dma_transfer_wait(&prms->dmaObjG);
            dma_transfer_wait(&prms->dmaObjB);
        }

        if(blk == (num_sets - 1))
        {
            exec_cmd = 0;
        }

        pipeline = (pipeline << 1) | exec_cmd;
    }

    if(status!=TIADALG_PROCESS_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[IMG_PROC] tiadalg failed !!!\n");
        status = VX_FAILURE;
    }

    return status;
}

static vx_status img_proc_pipeline_blocks_nv12ToRgbp
(
    tivxImgPreProcKernelParams *prms,
    vx_uint8 *pYL2[2],
    vx_uint8 *pCL2[2],
    void *pOutL2[2],
    uint32_t blk_width,
    uint32_t blk_height,
    uint32_t blk_stride,
    uint32_t num_sets,
    int32_t data_type
)
{
    vx_status status = VX_SUCCESS;

    /* Padding is taken care by DMA, so kernel is given zero padding */
    vx_int32 pad_pixel_zero[4] = {0};

    vx_uint32 pipeup = 2;
    vx_uint32 pipedown = 2;
    vx_uint32 exec_cmd = 1;
    vx_uint32 pipeline = exec_cmd;
    vx_uint32 ping_npong = 0;

    vx_uint32 blk;

    for(blk = 0; blk < (num_sets + pipeup + pipedown); blk++)
    {
        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjY);
            dma_transfer_trigger(&prms->dmaObjC);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjR);
            dma_transfer_trigger(&prms->dmaObjG);
            dma_transfer_trigger(&prms->dmaObjB);
        }

        if(EXEC_PIPELINE_STAGE2(pipeline))
        {

            vx_uint8 *pInL2[2];

            pInL2[0] = pYL2[ping_npong];
            pInL2[1] = pCL2[ping_npong];

#if 0
            VX_PRINT(VX_ZONE_ERROR, "[%s] [%d] [%d] [%d] [%d] [%d] [%f] %f]\n",
                     __FUNCTION__,
                     blk_width,
                     blk_height,
                     blk_stride,
                     data_type,
                     prms->nodeParams.color_conv_flag,
                     prms->nodeParams.scale_val,
                     prms->nodeParams.mean_pixel);
#endif

            status = tiadalg_image_preprocessing_c66
                    (
                        (void *)pInL2,
                        blk_width,
                        blk_height,
                        blk_stride,
                        data_type,
                        prms->nodeParams.color_conv_flag,
                        prms->nodeParams.scale_val,
                        prms->nodeParams.mean_pixel,
                        pad_pixel_zero,
                        (void *)pOutL2[ping_npong]
                    );

            ping_npong = !ping_npong;
        }

        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjY);
            dma_transfer_wait(&prms->dmaObjC);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjR);
            dma_transfer_wait(&prms->dmaObjG);
            dma_transfer_wait(&prms->dmaObjB);
        }

        if(blk == (num_sets - 1))
        {
            exec_cmd = 0;
        }

        pipeline = (pipeline << 1) | exec_cmd;
    }

    if(status!=TIADALG_PROCESS_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[IMG_PROC] tiadalg failed !!!\n");
        status = VX_FAILURE;
    }

    return status;
}
