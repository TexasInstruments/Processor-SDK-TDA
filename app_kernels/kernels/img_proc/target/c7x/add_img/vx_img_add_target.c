/*
*
* Copyright (c) 2017 - 2026 Texas Instruments Incorporated
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

/**
 * \file app_c7x_target_kernel_img_add.c Target Kernel implementation for Phase to RGB conversion function
 *
 *  This file shows a sample implementation of a target kernel function.
 *
 *  To implement a target kernel the below top level interface functions are implemented
 *  - vxTutorialAddTargetKernelPhaseRgb() : Registers target kernel to TIOVX target framework
 *  - vxTutorialRemoveTargetKernelPhaseRgb() : Un-Registers target kernel from TIOVX target framework
 *
 *  When registering a target kernel, the following callback function are implemented and registered with the TIOVX framework
 *  - app_c7x_target_kernel_img_add() : kernel execute/run function
 *  - app_c7x_target_kernel_img_addCreate() : kernel init function
 *  - app_c7x_target_kernel_img_addDelete() : kernel deinit function
 *  - app_c7x_target_kernel_img_addControl(): kernel control function
 *
 *  When working with target kernel
 *  - vxTutorialAddTargetKernelPhaseRgb() MUST be called during TIOVX target framework system init
 *     - This is done by using function tivxRegisterTutorialTargetKernels() in \ref vx_tutorial_target_kernel.c
 *  - vxTutorialRemoveTargetKernelPhaseRgb() MUST be called during TIOVX target framework system deinit
 *     - This is done by using function tivxUnRegisterTutorialTargetKernels() in \ref vx_tutorial_target_kernel.c
 *
 *  When registering a target kernel a unique name MUST be used to register the
 *  kernel on target side and HOST side.
 *
 *  Follow the comments for the different functions in the file to understand how a user/target kernel is implemented.
 */

#include "stdio.h"

#include <TI/tivx.h>
#include <TI/tivx_target_kernel.h>
#include <TI/tivx_img_proc.h>
#include "VXLIB_add_i8u_i8u_o8u_cn.h"

#include <utils/udma/include/app_udma_utils.h>

/* #define ENABLE_DMA_KERNEL_PRINT_STATS */

#if defined(x86_64) || defined(SOC_AM62A) || defined(SOC_J722S)
#undef USE_HW_DMA
#else
#define USE_HW_DMA
#endif

#define EXEC_PIPELINE_STAGE1(x) ((x) & 0x00000001)
#define EXEC_PIPELINE_STAGE2(x) ((x) & 0x00000002)
#define EXEC_PIPELINE_STAGE3(x) ((x) & 0x00000004)

/**
 * \brief Target kernel handle [static global]
 */
static tivx_target_kernel vx_img_add_target_kernel = NULL;

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

}DMAObj;

typedef struct {

    DMAObj dmaObjSrc0;
    DMAObj dmaObjSrc1;
    DMAObj dmaObjDst;

    vx_uint32 blkWidth;
    vx_uint32 blkHeight;
    vx_uint32 remHeight;
    vx_uint32 numSets;
    vx_uint32 req_size;
    vx_uint32 avail_size;

    uint8_t *pL2;
    uint32_t l2_heap_id;

} tivxC7xKernelParams;

static vx_status dma_create(DMAObj *dmaObj, vx_size transfer_type, vx_uint32 dma_ch)
{
    vx_status status = VX_SUCCESS;

    dmaObj->transfer_type = transfer_type;

    memset(&dmaObj->tfrPrms, 0, sizeof(app_udma_copy_nd_prms_t));

    dmaObj->icnt1_next = 0;
    dmaObj->icnt2_next = 0;
    dmaObj->icnt3_next = 0;

    if(transfer_type == DATA_COPY_DMA)
    {
#if defined(USE_HW_DMA)
        dmaObj->udmaChHdl = appUdmaCopyNDGetHandle(dma_ch);
#else
        dmaObj->udmaChHdl = NULL;
#endif
    }
    else
    {
        dmaObj->udmaChHdl = NULL;
    }

    return status;
}

static vx_status dma_delete(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

#if defined(USE_HW_DMA)
    status = appUdmaCopyDelete(dmaObj->udmaChHdl);
#endif

    return status;
}

static vx_status dma_init(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#if defined(USE_HW_DMA)
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
#if defined(USE_HW_DMA)
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

static vx_status dma_transfer_trigger(DMAObj *dmaObj)
{
    vx_status status = VX_SUCCESS;

    if(dmaObj->transfer_type == DATA_COPY_DMA)
    {
#if defined(USE_HW_DMA)
        appUdmaCopyNDTrigger(dmaObj->udmaChHdl);
#endif
    }
    else
    {
        app_udma_copy_nd_prms_t *tfrPrms;
        vx_uint32 icnt1, icnt2, icnt3;

        tfrPrms = (app_udma_copy_nd_prms_t *)&dmaObj->tfrPrms;

        /* This is for case where for every trigger ICNT0 * ICNT1 bytes get transferred */
        icnt3 = dmaObj->icnt3_next;
        icnt2 = dmaObj->icnt2_next;
        icnt1 = dmaObj->icnt1_next;

        vx_uint8 *pSrcNext = (vx_uint8 *)(tfrPrms->src_addr + (icnt3 * tfrPrms->dim3) + (icnt2 * tfrPrms->dim2));
        vx_uint8 *pDstNext = (vx_uint8 *)(tfrPrms->dest_addr + (icnt3 * tfrPrms->ddim3) + (icnt2 * tfrPrms->ddim2));

        for(icnt1 = 0; icnt1 < tfrPrms->icnt1; icnt1++)
        {
            memcpy(pDstNext, pSrcNext, tfrPrms->icnt0);

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
#if defined(USE_HW_DMA)
        appUdmaCopyNDWait(dmaObj->udmaChHdl);
#endif
    }

    return status;
}



static vx_status add_img_dma_setup_input_image(tivxC7xKernelParams *prms,
                             DMAObj *dmaObj,
                             vx_uint8 *pL2,
                             tivx_obj_desc_image_t *image_desc,
                             void *desc_target_ptr)
{
    vx_status status = VX_SUCCESS;

    app_udma_copy_nd_prms_t *tfrPrms = (app_udma_copy_nd_prms_t *)&dmaObj->tfrPrms;
    vx_imagepatch_addressing_t *pImage = (vx_imagepatch_addressing_t *)&image_desc->imagepatch_addr[0];
    vx_uint32 img_width  = pImage->dim_x;
    vx_int32  img_stride = pImage->stride_y;
    vx_uint8 *pDDR = NULL;

    pDDR = (vx_uint8 *)desc_target_ptr;

    tfrPrms->copy_mode    = 2;
    tfrPrms->eltype       = sizeof(uint8_t);

    tfrPrms->dest_addr    = (uintptr_t)pL2;
    #if defined(USE_HW_DMA)
    tfrPrms->src_addr     = tivxMemShared2PhysPtr((uint64_t)pDDR, TIVX_MEM_EXTERNAL);
    #else
    tfrPrms->src_addr     = (uint64_t)pDDR, TIVX_MEM_EXTERNAL;
    #endif

    tfrPrms->icnt0        = img_width;
    tfrPrms->icnt1        = prms->blkHeight;
    tfrPrms->icnt2        = 2;
    tfrPrms->icnt3        = prms->numSets / 2;
    tfrPrms->dim1         = img_stride;
    tfrPrms->dim2         = (prms->blkHeight * img_stride);
    tfrPrms->dim3         = (prms->blkHeight * img_stride * 2);

    tfrPrms->dicnt0       = tfrPrms->icnt0;
    tfrPrms->dicnt1       = tfrPrms->icnt1;
    tfrPrms->dicnt2       = 2; /* Ping-pong */
    tfrPrms->dicnt3       = prms->numSets / 2;
    tfrPrms->ddim1        = prms->blkWidth;
    tfrPrms->ddim2        = (prms->blkHeight * prms->blkWidth);
    tfrPrms->ddim3        = 0;

    dma_init(dmaObj);

    return status;
}

static vx_status add_img_dma_setup_output_image(tivxC7xKernelParams *prms,
                             DMAObj *dmaObj,
                             vx_uint8 *pL2,
                             tivx_obj_desc_image_t *image_desc,
                             void *desc_target_ptr)
{
    vx_status status = VX_SUCCESS;

    app_udma_copy_nd_prms_t *tfrPrms = (app_udma_copy_nd_prms_t *)&dmaObj->tfrPrms;
    vx_imagepatch_addressing_t *pImage = (vx_imagepatch_addressing_t *)&image_desc->imagepatch_addr[0];
    vx_uint32 img_width  = pImage->dim_x;
    vx_int32  img_stride = pImage->stride_y;
    vx_uint8 *pDDR = NULL;

    pDDR = (vx_uint8 *)desc_target_ptr;

    tfrPrms->copy_mode    = 2;
    tfrPrms->eltype       = sizeof(uint8_t);

    #if defined(USE_HW_DMA)
    tfrPrms->dest_addr    = tivxMemShared2PhysPtr((uint64_t)pDDR, TIVX_MEM_EXTERNAL);
    #else
    tfrPrms->dest_addr    = (uint64_t)pDDR, TIVX_MEM_EXTERNAL;
    #endif
    tfrPrms->src_addr     = (uintptr_t)pL2;

    tfrPrms->icnt0        = img_width;
    tfrPrms->icnt1        = prms->blkHeight;
    tfrPrms->icnt2        = 2;
    tfrPrms->icnt3        = prms->numSets / 2;
    tfrPrms->dim1         = img_stride;
    tfrPrms->dim2         = (prms->blkHeight * img_stride);
    tfrPrms->dim3         = 0;

    tfrPrms->dicnt0       = tfrPrms->icnt0;
    tfrPrms->dicnt1       = tfrPrms->icnt1;
    tfrPrms->dicnt2       = 2; /* Ping-pong */
    tfrPrms->dicnt3       = prms->numSets / 2;
    tfrPrms->ddim1        = prms->blkWidth;
    tfrPrms->ddim2        = (prms->blkHeight * prms->blkWidth);
    tfrPrms->ddim3        = (prms->blkHeight * img_stride * 2);

    dma_init(dmaObj);

    return status;
}


static vx_status add_img_dma_setup(tivxC7xKernelParams *prms,
                             tivx_obj_desc_image_t *src_desc0,
                             tivx_obj_desc_image_t *src_desc1,
                             tivx_obj_desc_image_t *dst_desc,
                             void *src_desc0_target_ptr,
                             void *src_desc1_target_ptr,
                             void *dst_desc_target_ptr)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 *pSrc0L2[2] = {NULL};
    vx_uint8 *pSrc1L2[2] = {NULL};
    vx_uint8 *pDstL2[2] = {NULL};

    /* Setting first image pointer to the base of L2 then the second buffer
       directly following the pointer location.  Repeating this pattern for
       all images */
    pSrc0L2[0] = (vx_uint8 *)prms->pL2;
    pSrc0L2[1] = (vx_uint8 *)pSrc0L2[0] + (prms->blkWidth * prms->blkHeight);

    pSrc1L2[0] = (vx_uint8 *)pSrc0L2[1] + (prms->blkWidth * prms->blkHeight);
    pSrc1L2[1] = (vx_uint8 *)pSrc1L2[0] + (prms->blkWidth * prms->blkHeight);

    pDstL2[0]  = (vx_uint8 *)pSrc1L2[1] + (prms->blkWidth * prms->blkHeight);
    pDstL2[1]  = (vx_uint8 *)pDstL2[0]  + (prms->blkWidth * prms->blkHeight);

    /* Setting up src0 */
    status = add_img_dma_setup_input_image(prms, &prms->dmaObjSrc0, pSrc0L2[0], src_desc0, src_desc0_target_ptr);

    if (VX_SUCCESS == status)
    {
        /* Setting up src1 */
        status = add_img_dma_setup_input_image(prms, &prms->dmaObjSrc1, pSrc1L2[0], src_desc1, src_desc1_target_ptr);

        if (VX_SUCCESS == status)
        {
            /* Setting up dst */
            status = add_img_dma_setup_output_image(prms, &prms->dmaObjDst, pDstL2[0], dst_desc, dst_desc_target_ptr);
        }
        else
        {
            printf("dma setup for src1 failed\n");
        }
    }
    else
    {
        printf("dma setup for src0 failed\n");
    }

    return status;
}

static vx_status add_img_pipeline_blocks(tivxC7xKernelParams *prms,
                             tivx_obj_desc_image_t *src_desc0,
                             tivx_obj_desc_image_t *src_desc1,
                             tivx_obj_desc_image_t *dst_desc,
                             void *src_desc0_target_ptr,
                             void *src_desc1_target_ptr,
                             void *dst_desc_target_ptr)
{
    vx_status status = VX_SUCCESS;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
    uint16_t overflow_policy;

    vx_uint32 pipeup = 2;
    vx_uint32 pipedown = 2;
    vx_uint32 exec_cmd = 1;
    vx_uint32 pipeline = exec_cmd;
    vx_uint32 ping_npong = 0;
    vx_uint32 blk;

    vx_uint8 *pSrc0L2[2] = {NULL};
    vx_uint8 *pSrc1L2[2] = {NULL};
    vx_uint8 *pDstL2[2] = {NULL};

    /* Setting first image pointer to the base of L2 then the second buffer
       directly following the pointer location.  Repeating this pattern for
       all images */
    pSrc0L2[0] = (vx_uint8 *)prms->pL2;
    pSrc0L2[1] = (vx_uint8 *)pSrc0L2[0] + (prms->blkWidth * prms->blkHeight);

    pSrc1L2[0] = (vx_uint8 *)pSrc0L2[1] + (prms->blkWidth * prms->blkHeight);
    pSrc1L2[1] = (vx_uint8 *)pSrc1L2[0] + (prms->blkWidth * prms->blkHeight);

    pDstL2[0]  = (vx_uint8 *)pSrc1L2[1] + (prms->blkWidth * prms->blkHeight);
    pDstL2[1]  = (vx_uint8 *)pDstL2[0]  + (prms->blkWidth * prms->blkHeight);

    /* Setting kernel processing parameters */
    overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;

    vxlib_src0.dim_x = prms->blkWidth;
    vxlib_src0.dim_y = prms->blkHeight;
    vxlib_src0.stride_y = prms->blkWidth;
    vxlib_src0.data_type = VXLIB_UINT8;

    vxlib_src1.dim_x = prms->blkWidth;
    vxlib_src1.dim_y = prms->blkHeight;
    vxlib_src1.stride_y = prms->blkWidth;
    vxlib_src1.data_type = VXLIB_UINT8;

    vxlib_dst.dim_x = prms->blkWidth;
    vxlib_dst.dim_y = prms->blkHeight;
    vxlib_dst.stride_y = prms->blkWidth;
    vxlib_dst.data_type = VXLIB_UINT8;

    for(blk = 0; blk < (prms->numSets + pipeup + pipedown); blk++)
    {
        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjSrc0);
            dma_transfer_trigger(&prms->dmaObjSrc1);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_trigger(&prms->dmaObjDst);
        }

        if(EXEC_PIPELINE_STAGE2(pipeline))
        {
            status = C7xVXLIB_add_i8u_i8u_o8u_cn(
                (uint8_t *)pSrc0L2[ping_npong],
                &vxlib_src0,
                (uint8_t *)pSrc1L2[ping_npong],
                &vxlib_src1,
                (uint8_t *)pDstL2[ping_npong],
                &vxlib_dst,
                overflow_policy);

            ping_npong = !ping_npong;
        }

        if(EXEC_PIPELINE_STAGE1(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjSrc0);
            dma_transfer_wait(&prms->dmaObjSrc1);
        }

        if(EXEC_PIPELINE_STAGE3(pipeline))
        {
            dma_transfer_wait(&prms->dmaObjDst);
        }

        if(blk == (prms->numSets - 1))
        {
            exec_cmd = 0;
        }

        pipeline = (pipeline << 1) | exec_cmd;
    }

    if(status!=VX_SUCCESS)
    {
        printf("Kernel processing failed !!!\n");
        status = VX_FAILURE;
    }

    return status;
}

static vx_status add_img_dma_teardown(tivxC7xKernelParams *prms)
{
    vx_status status = VX_SUCCESS;

    dma_deinit(&prms->dmaObjSrc0);
    dma_deinit(&prms->dmaObjSrc1);
    dma_deinit(&prms->dmaObjDst);

    return status;
}

static vx_status add_img_execute(tivxC7xKernelParams *prms,
                             tivx_obj_desc_image_t *src_desc0,
                             tivx_obj_desc_image_t *src_desc1,
                             tivx_obj_desc_image_t *dst_desc,
                             void *src_desc0_target_ptr,
                             void *src_desc1_target_ptr,
                             void *dst_desc_target_ptr)
{
    vx_status status = VX_SUCCESS;

    status = add_img_dma_setup(prms, src_desc0, src_desc1, dst_desc, src_desc0_target_ptr, src_desc1_target_ptr, dst_desc_target_ptr);

    if (VX_SUCCESS == status)
    {
        status = add_img_pipeline_blocks(prms, src_desc0, src_desc1, dst_desc, src_desc0_target_ptr, src_desc1_target_ptr, dst_desc_target_ptr);

        if (VX_SUCCESS == status)
        {
            status = add_img_dma_teardown(prms);
        }
        else
        {
            printf("add_img_pipeline_blocks failed\n");
        }
    }
    else
    {
        printf("add_img_dma_setup failed\n");
    }

    return status;
}

/**
 * \brief Target kernel run function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK tivxKernelImgAddProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc0, *src_desc1, *dst_desc;
    tivxC7xKernelParams *prms = NULL;

    if ((num_params != APP_C7X_IMG_ADD_MAX_PARAMS)
        || (NULL == obj_desc[APP_C7X_IMG_ADD_IN0_IMG_IDX])
        || (NULL == obj_desc[APP_C7X_IMG_ADD_IN1_IMG_IDX])
        || (NULL == obj_desc[APP_C7X_IMG_ADD_OUT0_IMG_IDX])
        )
    {
        status = VX_FAILURE;
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxC7xKernelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(status==VX_SUCCESS)
    {
        void *src_desc0_target_ptr;
        void *src_desc1_target_ptr;
        void *dst_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        src_desc0 = (tivx_obj_desc_image_t *)obj_desc[APP_C7X_IMG_ADD_IN0_IMG_IDX];
        src_desc1 = (tivx_obj_desc_image_t *)obj_desc[APP_C7X_IMG_ADD_IN1_IMG_IDX];
        dst_desc  = (tivx_obj_desc_image_t *)obj_desc[APP_C7X_IMG_ADD_OUT0_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src_desc0_target_ptr = tivxMemShared2TargetPtr(&src_desc0->mem_ptr[0]);
        src_desc1_target_ptr = tivxMemShared2TargetPtr(&src_desc1->mem_ptr[0]);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(&dst_desc->mem_ptr[0]);

        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src_desc0_target_ptr,
            src_desc0->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(src_desc1_target_ptr,
            src_desc1->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        status = add_img_execute(prms, src_desc0, src_desc1, dst_desc, src_desc0_target_ptr, src_desc1_target_ptr, dst_desc_target_ptr);

        tivxMemBufferUnmap(src_desc0_target_ptr,
            src_desc0->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src_desc1_target_ptr,
            src_desc1->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

    }

    return (status);
}

/**
 * \brief Target kernel create function
 *
 * \param kernel [in] target kernel handle
 * \param param_obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK tivxKernelImgAddCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == param_obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxC7xKernelParams * kernelParams = NULL;

        kernelParams = (tivxC7xKernelParams *)tivxMemAlloc(sizeof(tivxC7xKernelParams), TIVX_MEM_EXTERNAL);

        if(kernelParams == NULL)
        {
            status = VX_FAILURE;
        }

        /* Below is added directly from img preproc, need to review */
        if(status == VX_SUCCESS)
        {
            tivx_mem_stats l2_stats;
            vx_uint32 blk_width, blk_height, rem_height, mblk_height;
            vx_uint32 in_size1, in_size2, out_size, total_size, req_size, avail_size, num_sets;

            tivx_obj_desc_image_t *in_img_desc  = (tivx_obj_desc_image_t *)param_obj_desc[APP_C7X_IMG_ADD_IN0_IMG_IDX];

            vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];
            vx_int32  in_width  = pIn->dim_x;
            vx_uint32 in_height = pIn->dim_y;
            vx_int32  in_stride = pIn->stride_y;

            vx_uint32 num_bytes = 1;
            blk_width = in_stride / sizeof(vx_uint8);

            /* Resetting L2 heap */
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);

            avail_size = l2_stats.free_size;

            req_size = 448 * 1024; /* Out of available 512KB - 64KB is cache and 448KB is SRAM */

            if(req_size > avail_size)
                req_size = avail_size;

            total_size = 0;
            mblk_height = 1;
            blk_height = mblk_height;
            rem_height = 0;

            in_size1 = 0;
            in_size2 = 0;
            out_size = 0;

            /* Using a block width equal to the width of the image.
               Calculating the block height in this loop by starting
               with a height of 1 then multiplying the height by 2
               through each loop.  The loop terminates when either
               the total size required exceeds the available L2
               or the height can longer be divided evenly into the
               total height of the image (so that we can have the
               same size blocks being used in the DMA) */
            while((total_size < req_size) && (rem_height == 0))
            {
                kernelParams->blkWidth = blk_width;
                kernelParams->blkHeight = blk_height;
                kernelParams->remHeight = rem_height;

                blk_height = mblk_height;
                mblk_height *= 2;

                in_size1  = (blk_width * mblk_height);
                in_size2  = (blk_width * mblk_height);
                out_size = (blk_width * mblk_height * num_bytes);
                /* Double buffer inputs and outputs*/
                in_size1 = in_size1 * 2;
                in_size2 = in_size2 * 2;
                out_size = out_size * 2;
                total_size = in_size1 + in_size2 + out_size;
                rem_height = in_height % mblk_height;
            }

            num_sets = in_height / kernelParams->blkHeight;
            kernelParams->numSets = num_sets;
            kernelParams->avail_size = avail_size;
            kernelParams->req_size = req_size;

            /* In_size for two U8 images will be 2 x blkWidth x blkHeight */
            in_size1  = (kernelParams->blkWidth * kernelParams->blkHeight);
            in_size2  = (kernelParams->blkWidth * kernelParams->blkHeight);
            /* Out_size for one U8 images will be blkWidth x blkHeight */
            out_size = (kernelParams->blkWidth * kernelParams->blkHeight * num_bytes);

            /* Double buffer inputs and outputs*/
            in_size1 = in_size1 * 2;
            in_size2 = in_size2 * 2;
            out_size = out_size * 2;
            total_size = in_size1 + in_size2 + out_size;

            #ifdef ENABLE_DMA_KERNEL_PRINT_STATS
            printf("blk_width = %d\n", kernelParams->blkWidth);
            printf("blk_height = %d\n", kernelParams->blkHeight);
            printf("num_sets = %d\n", kernelParams->numSets);
            printf("rem_height = %d\n", kernelParams->remHeight);
            printf("in_size1 = %d\n", in_size1);
            printf("in_size2 = %d\n", in_size2);
            printf("out_size = %d\n", out_size);
            printf("total_size = %d\n", total_size);
            printf("req_size = %d\n", req_size);
            printf("avail_size = %d\n", avail_size);
            printf("\n");
            #endif
        }

        if(status == VX_SUCCESS)
        {
            kernelParams->l2_heap_id = TIVX_MEM_INTERNAL_L2; /* TIVX_MEM_INTERNAL_L2 or TIVX_MEM_EXTERNAL */

            kernelParams->pL2 = (uint8_t *)tivxMemAlloc(kernelParams->req_size, kernelParams->l2_heap_id);
            if(kernelParams->pL2 == NULL)
            {
                printf("Unable to allocate L2 scratch!\n");
                status = VX_FAILURE;
            }
        }

        if(status == VX_SUCCESS)
        {
#if defined(USE_HW_DMA)
            dma_create(&kernelParams->dmaObjSrc0, DATA_COPY_DMA, 8);
            dma_create(&kernelParams->dmaObjSrc1, DATA_COPY_DMA, 9);
            dma_create(&kernelParams->dmaObjDst,  DATA_COPY_DMA, 10);
#else
            dma_create(&kernelParams->dmaObjSrc0, DATA_COPY_CPU, 0);
            dma_create(&kernelParams->dmaObjSrc1, DATA_COPY_CPU, 1);
            dma_create(&kernelParams->dmaObjDst,  DATA_COPY_CPU, 2);
#endif
            tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxC7xKernelParams));
        }
    }

    return status;
}

/**
 * \brief Target kernel delete function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK tivxKernelImgAddDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == param_obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxC7xKernelParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            status += dma_delete(&prms->dmaObjSrc0);
            status += dma_delete(&prms->dmaObjSrc1);
            status += dma_delete(&prms->dmaObjDst);

            tivxMemFree(prms->pL2, prms->req_size, prms->l2_heap_id);

            tivxMemFree(prms, sizeof(tivxC7xKernelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

/**
 * \brief Add target kernel to TIOVX framework
 *
 */
void tivxAddTargetKernelImgAdd(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    /**
     * - Get CPU ID of the running CPU
     *
     * Add kernel to target framework only if it is supported on this target
     * \code
     */
    self_cpu = tivxGetSelfCpuId();
    /** \endcode */

    if ((self_cpu == TIVX_CPU_ID_DSP_C7_1))
    {
        /**
         * - Find target name based on currently running CPU
         *
         * \code
         */
        strncpy(target_name, TIVX_TARGET_DSP_C7_1,
                TIVX_TARGET_MAX_NAME);
        /** \endcode */

        /**
         * - Register target kernel to TIOVX framework
         *
         * "TIVX_C7X_KERNEL_IMG_ADD_NAME" is the name of the target kernel.
         * See also \ref app_c7x_kernel.h
         *
         * This MUST match the name specified when registering the same kernel
         * on the HOST side.
         *
         * The registered target kernel handle is stored in a global variable.
         * This is used during app_c7x_target_kernel_img_add_unregister()
         *
         * \code
         */
        vx_img_add_target_kernel = tivxAddTargetKernelByName(
                    (char*)TIVX_C7X_KERNEL_IMG_ADD_NAME,
                    target_name,
                    tivxKernelImgAddProcess,
                    tivxKernelImgAddCreate,
                    tivxKernelImgAddDelete,
                    NULL,
                    NULL);
        /** \endcode */
    }
}

/**
 * \brief Remove target kernel from TIOVX framework
 *
 */
void tivxRemoveTargetKernelImgAdd(void)
{
    /**
     * - UnRegister target kernel from TIOVX framework
     *
     * The target kernel handle used is the one returned during
     * tivxAddTargetKernel()
     *
     * \code
     */
    tivxRemoveTargetKernel(vx_img_add_target_kernel);
    /** \endcode */
}

