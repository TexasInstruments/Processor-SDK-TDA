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


#include <stdio.h>
#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include <tivx_sfm_host.h>
#include "tiadalg_interface.h"
#include <utils/udma/include/app_udma.h>
#include <utils/udma/include/app_udma_utils.h>
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include "tiadalg_interface.h"
#include "tivx_kernels_target_utils.h"


#ifndef x86_64
//#include "c7x.h"

#if defined(SOC_J722S)
#include <kernel/dpl/HwiP.h>
#else
#include <ti/osal/HwiP.h>
#endif

#define DISABLE_INTERRUPTS_DURING_PROCESS
#endif

typedef struct
{
    IVISION_BufDesc     inBufDesc[SFM_TI_IN_BUFDESC_TOTAL];
    IVISION_BufDesc     outBufDesc[SFM_TI_OUT_BUFDESC_TOTAL];

    IVISION_BufDesc    *inBufDescList[SFM_TI_IN_BUFDESC_TOTAL];
    IVISION_BufDesc    *outBufDescList[SFM_TI_OUT_BUFDESC_TOTAL];

    IVISION_InBufs      inBufs;
    IVISION_OutBufs     outBufs;

    SFM_TI_InArgs        *inArgs;
    SFM_TI_OutArgs       *outArgs;

    SFM_TI_CreateParams  createParams;

    tivxSFMParams        params;

    void                *algHandle;

} tivxSFMObj;

static tivx_target_kernel vx_sfm_target_kernel = NULL;

static void tivxSFMFreeMem(tivxSFMObj *sfmObj);

static int32_t SFM_AllocNetInputMem(IVISION_BufDesc *BufDescList, int32_t totYPixs, uint16_t inNumTracks)
{
  /* Currently only one input buffer supported */
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].numPlanes                          = 1;
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].frameROI.topLeft.x    = 0;
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].frameROI.topLeft.y    = 0;
  /* This has to be width + horizontal padding */
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].width = totYPixs*sizeof(uint32_t);
  /* This has to be numCh * (height + vertical padding) */
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].height= 1;
  /* This has to be just width */
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].frameROI.width = totYPixs*sizeof(uint32_t);
  /* This has to be just height */
  BufDescList[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].frameROI.height = 1;

  /* Currently only one input buffer supported */
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].numPlanes                          = 1;
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].frameROI.topLeft.x    = 0;
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].frameROI.topLeft.y    = 0;
  /* This has to be width + horizontal padding */
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].width                 = totYPixs;
  /* This has to be numCh * (height + vertical padding) */
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].height                = 1;
  /* This has to be just width */
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].frameROI.width        = totYPixs;
  /* This has to be just height */
  BufDescList[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].frameROI.height       = 1;

  return SFM_TI_IN_BUFDESC_TOTAL;
}

static int32_t SFM_AllocNetOutputMem(IVISION_BufDesc *BufDescList, int32_t totYPixs, uint16_t inNumTracks)
{
  /* Currently only one output buffer supported */
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].numPlanes                          = 1;
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].frameROI.topLeft.x    = 0;
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].frameROI.topLeft.y    = 0;
  /* This requires output width + horizontal padding */
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].width = inNumTracks*sizeof(SFM_TI_output);
  /* This requires numOutCh * (output height + vertial padding) */
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].height= 1;
  /* This requires just output width */
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].frameROI.width = inNumTracks*sizeof(SFM_TI_output);
  /* This requires just output height */
  BufDescList[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].frameROI.height        = 1;

  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].numPlanes                          = 1;
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].frameROI.topLeft.x    = 0;
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].frameROI.topLeft.y    = 0;
  /* This requires output width + horizontal padding */
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].width = totYPixs;
  /* This requires numOutCh * (output height + vertial padding) */
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].height= 1;
  /* This requires just output width */
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].frameROI.width = totYPixs;
  /* This requires just output height */
  BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].frameROI.height= 1;

  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].numPlanes                          = 1;
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].frameROI.topLeft.x    = 0;
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].frameROI.topLeft.y    = 0;
  /* This requires output width + horizontal padding */
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].width = (totYPixs>>1);
  /* This requires numOutCh * (output height + vertial padding) */
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].height= 1;
  /* This requires just output width */
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].frameROI.width = (totYPixs>>1);
  /* This requires just output height */
  BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].frameROI.height= 1;

  memcpy(&BufDescList[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER], &BufDescList[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER], sizeof(IVISION_BufDesc));

  memcpy(&BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER], &BufDescList[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER], sizeof(IVISION_BufDesc));

  return SFM_TI_OUT_BUFDESC_TOTAL;
}

static vx_status VX_CALLBACK tivxKernelSFMProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxSFMObj *sfmObj;
    uint32_t i, size;

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    uint32_t oldIntState;
    #endif

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    /* disabling interrupts when doing SFM processing
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
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&sfmObj, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == sfmObj) ||  (sizeof(tivxSFMObj) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t *config_desc;
        tivx_obj_desc_user_data_object_t *in_args_desc;
        tivx_obj_desc_user_data_object_t *out_args_desc;
        tivx_obj_desc_image_t *in_img_desc;
        tivx_obj_desc_image_t *flow_vec_desc;
        tivx_obj_desc_image_t *out_img_ptcld_desc;
        tivx_obj_desc_image_t *out_img_og_desc;
        tivx_obj_desc_user_data_object_t *out_feat_desc;

        void *config_target_ptr;
        void *in_args_target_ptr  = NULL;
        void *out_args_target_ptr =  NULL;
        void *in_img_target_ptr[2];
        void *flow_vec_target_ptr[2];
        void *out_img_ptcld_target_ptr[2];
        void *out_img_og_target_ptr[2];
        void *out_feat_target_ptr =  NULL;

        /* Use this access SFM params */
        config_desc   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(config_target_ptr, config_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        /* Use this access in_args */
        in_args_desc   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_IN_ARGS_IDX];
        in_args_target_ptr = tivxMemShared2TargetPtr(&in_args_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(in_args_target_ptr, in_args_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        SFM_TI_InArgs  *inArgs  = (SFM_TI_InArgs  *)in_args_target_ptr;

        /* Use this access out_args */
        out_args_desc   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_OUT_ARGS_IDX];
        out_args_target_ptr = tivxMemShared2TargetPtr(&out_args_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(out_args_target_ptr, out_args_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        SFM_TI_OutArgs  *outArgs = (SFM_TI_OutArgs  *)out_args_target_ptr;

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SFM_INPUT_IMAGE_IDX];
        in_img_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img_target_ptr[0], in_img_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        sfmObj->inBufDesc[SFM_TI_IN_BUFDESC_IN_LUMA_IMG_BUFFER].bufPlanes[0].buf = in_img_target_ptr[0];

        flow_vec_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SFM_FLOW_VECTOR_IDX];
        flow_vec_target_ptr[0]  = tivxMemShared2TargetPtr(&flow_vec_desc->mem_ptr[0]);
        tivxMemBufferMap(flow_vec_target_ptr[0], flow_vec_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        flow_vec_target_ptr[1]  = NULL;
        sfmObj->inBufDesc[SFM_TI_IN_BUFDESC_IN_DOF_BUFFER].bufPlanes[0].buf = flow_vec_target_ptr[0];

        /*Point cloud visulaization output image buffer */
        out_img_ptcld_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SFM_OUTPUT_PTCLD_IMAGE_IDX];
        out_img_ptcld_target_ptr[0]  = tivxMemShared2TargetPtr(&out_img_ptcld_desc->mem_ptr[0]);
        tivxMemBufferMap(out_img_ptcld_target_ptr[0], out_img_ptcld_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        out_img_ptcld_target_ptr[1]  = NULL;
        sfmObj->outBufDesc[SFM_TI_OUT_BUFDESC_LUMA_PTCLD_BUFFER].bufPlanes[0].buf = out_img_ptcld_target_ptr[0];

        if(out_img_ptcld_desc->mem_ptr[1].shared_ptr != 0)
        {
            out_img_ptcld_target_ptr[1]  = tivxMemShared2TargetPtr(&out_img_ptcld_desc->mem_ptr[1]);
            tivxMemBufferMap(out_img_ptcld_target_ptr[1], out_img_ptcld_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
            sfmObj->outBufDesc[SFM_TI_OUT_BUFDESC_CHROMA_PTCLD_BUFFER].bufPlanes[0].buf = out_img_ptcld_target_ptr[1];
        }

        /*Occupancy grid visulaization output image buffer */
        out_img_og_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SFM_OUTPUT_OCPGRD_IMAGE_IDX];
        out_img_og_target_ptr[0]  = tivxMemShared2TargetPtr(&out_img_og_desc->mem_ptr[0]);
        tivxMemBufferMap(out_img_og_target_ptr[0], out_img_og_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        out_img_og_target_ptr[1]  = NULL;
        sfmObj->outBufDesc[SFM_TI_OUT_BUFDESC_LUMA_OCPGD_BUFFER].bufPlanes[0].buf = out_img_og_target_ptr[0];

        if(out_img_og_desc->mem_ptr[1].shared_ptr != 0)
        {
            out_img_og_target_ptr[1]  = tivxMemShared2TargetPtr(&out_img_og_desc->mem_ptr[1]);
            tivxMemBufferMap(out_img_og_target_ptr[1], out_img_og_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
            sfmObj->outBufDesc[SFM_TI_OUT_BUFDESC_CHROMA_OCPGD_BUFFER].bufPlanes[0].buf = out_img_og_target_ptr[1];
        }

        /*Point cloud raw data output buffer */
        out_feat_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_OUTPUT_FEATURE_IDX];
        if(out_feat_desc != NULL){
            out_feat_target_ptr = tivxMemShared2TargetPtr(&out_feat_desc->mem_ptr);
            tivxMemBufferMap(out_feat_target_ptr, out_feat_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_AND_WRITE);
        }

        sfmObj->outBufDesc[SFM_TI_OUT_BUFDESC_FEATURE_PLANES].bufPlanes[0].buf = out_feat_target_ptr;

        outArgs->iVisionOutArgs.size  = sizeof(SFM_TI_OutArgs);
        inArgs->iVisionInArgs.size    = sizeof(SFM_TI_InArgs);

        inArgs->maxRansacItr      = 45;
        inArgs->maxTriangItr      = 3;
        inArgs->fMatrixPrunEn     = 1;
        inArgs->fMatrixInTh       = 4;
        inArgs->pointPruneAngle   = 2.0;
        inArgs->fMatrixCalcMethod = 0;
        inArgs->flowConfThr       = 103;
        inArgs->flowInvalidPadX   = 0;
        inArgs->flowInvalidPadY   = 0;
        inArgs->reserved0         = 0x0;

        memcpy(inArgs->camExtPrm, ((tivxSFMParams*)config_target_ptr)->camera_pose, 12*sizeof(float));

        if(((tivxSFMParams*)config_target_ptr)->camera_pose[15] == 111.0f) // check for marker flag for first frame or for reset
        {
            inArgs->reset = 0x1;
        }
        else
        {
            inArgs->reset = 0x0;
        }

        status = tivxAlgiVisionProcess
                (
                    sfmObj->algHandle,
                    &sfmObj->inBufs,
                    &sfmObj->outBufs,
                    (IVISION_InArgs  *)inArgs,
                    (IVISION_OutArgs *)outArgs,
                    1
                );

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        tivxMemBufferUnmap(in_args_target_ptr, in_args_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        tivxMemBufferUnmap(out_args_target_ptr, out_args_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_img_target_ptr[0], in_img_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);
        tivxMemBufferUnmap(flow_vec_target_ptr[0], flow_vec_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_READ_ONLY);

        tivxMemBufferUnmap(out_img_ptcld_target_ptr[0], out_img_ptcld_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        if (out_img_ptcld_target_ptr[1] != NULL)
        {
          tivxMemBufferUnmap(out_img_ptcld_target_ptr[1], out_img_ptcld_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        }

        tivxMemBufferUnmap(out_img_og_target_ptr[0], out_img_og_desc->mem_size[0], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        if (out_img_og_target_ptr[1] != NULL)
        {
          tivxMemBufferUnmap(out_img_og_target_ptr[1], out_img_og_desc->mem_size[1], TIVX_MEMORY_TYPE_DMA, VX_WRITE_ONLY);
        }

        if(out_feat_desc != NULL){
          tivxMemBufferUnmap(out_feat_target_ptr, out_feat_desc->mem_size, TIVX_MEMORY_TYPE_DMA, VX_READ_AND_WRITE);
        }
    }

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    HwiP_restore(oldIntState);
    #endif

    return (status);
}
#ifdef x86_64
/* Udma_init for target flow is done part of App Common Init
   Udma is not used in in Host emulation mode of other module, but TIDL
   Has flows which uses UDMA. So intilizing here Specific to TIDL Init.
   This flow is controlled via flowCtrl in create Params
*/
#include <udma/udma.h>
static struct Udma_DrvObj  x86udmaDrvObj;

uint64_t tiadalgVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
    return (uint64_t)virtAddr;
}
void * tiadalgX86UdmaInit( void)
{
    static uint8_t firstCall = 1;
    if(firstCall)
    {
        Udma_InitPrms initPrms;
        UdmaInitPrms_init(UDMA_INST_ID_MAIN_0, &initPrms);
        initPrms.printFxn = NULL;
        initPrms.skipGlobalEventReg = 1;
        initPrms.virtToPhyFxn = tiadalgVirtToPhyAddrConversion;
        Udma_init(&x86udmaDrvObj, &initPrms);
        firstCall = 0;
    }
    return &x86udmaDrvObj;
}
#endif

static vx_status VX_CALLBACK tivxKernelSFMCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  uint16_t num_params,
  void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_user_data_object_t *config;
    tivx_obj_desc_user_data_object_t *createParams;

    tivxSFMObj *sfmObj = NULL;

    void *config_target_ptr = NULL;
    void *create_params_target_ptr = NULL;

    uint32_t i;

    #ifdef TIVX_SFM_TARGET_DEBUG
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        /* IMPORTANT! Config data is assumed to be available at index 0 */
        config    = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_CONFIG_IDX];

        /* IMPORTANT! Create params is assumed to be available at index 2 */
        createParams   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_SFM_CREATE_PARAMS_IDX];

        sfmObj = tivxMemAlloc(sizeof(tivxSFMObj), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != sfmObj)
        {
            memset(sfmObj, 0, sizeof(tivxSFMObj));
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          config_target_ptr = tivxMemShared2TargetPtr(&config->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

          memcpy(&sfmObj->params, config_target_ptr, sizeof(tivxSFMParams));

        }

        if ((vx_status)VX_SUCCESS == status)
        {
          create_params_target_ptr = tivxMemShared2TargetPtr(&createParams->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

          memcpy(&sfmObj->createParams, create_params_target_ptr, sizeof(SFM_TI_CreateParams));
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivx_mem_stats l1_stats;
            tivx_mem_stats l2_stats;
            tivx_mem_stats l3_stats;

            /* reset scratch heap offset to zero by doing a dummy free */
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L1);
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L3);

            tivxMemStats(&l1_stats, (vx_enum)TIVX_MEM_INTERNAL_L1);
            tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemStats(&l3_stats, (vx_enum)TIVX_MEM_INTERNAL_L3);

            VX_PRINT(VX_ZONE_INFO, "L1 = %d KB, L2 = %d KB, L3 = %d KB\n",
                l1_stats.free_size/1024,
                l2_stats.free_size/1024,
                l3_stats.free_size/1024
                );

#ifdef x86_64
            sfmObj->createParams.udmaDrvObj = tiadalgX86UdmaInit();
#else
            sfmObj->createParams.udmaDrvObj = appUdmaGetObj();
#endif

            if ((vx_status)VX_SUCCESS == status)
            {
                sfmObj->algHandle = tivxAlgiVisionCreate
                                    (
                                        &SFM_TI_VISION_FXNS,
                                        (IALG_Params *)(&sfmObj->createParams)
                                    );

                if (NULL == sfmObj->algHandle)
                {
                    status = (vx_status)VX_FAILURE;
                }

                sfmObj->inBufs.size     = sizeof(sfmObj->inBufs);
                sfmObj->outBufs.size    = sizeof(sfmObj->outBufs);

                sfmObj->inBufs.bufDesc  = sfmObj->inBufDescList;
                sfmObj->outBufs.bufDesc = sfmObj->outBufDescList;

                /* Call the functions with appropiate arguments */
                sfmObj->inBufs.numBufs  = SFM_AllocNetInputMem(sfmObj->inBufDesc,
                                                               sfmObj->createParams.imgWidth*sfmObj->createParams.imgHeight,
                                                               sfmObj->createParams.maxNumTracks);

                sfmObj->outBufs.numBufs = SFM_AllocNetOutputMem(sfmObj->outBufDesc,
                                                                sfmObj->createParams.imgWidth*sfmObj->createParams.imgHeight,
                                                                sfmObj->createParams.maxNumTracks);

                for(i = 0; i < sfmObj->inBufs.numBufs; i++)
                {
                    sfmObj->inBufDescList[i]     = &sfmObj->inBufDesc[i];
                }
                for(i = 0; i < sfmObj->outBufs.numBufs; i++)
                {
                    sfmObj->outBufDescList[i]     = &sfmObj->outBufDesc[i];
                }
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, sfmObj,  sizeof(tivxSFMObj));
        }
        else
        {
            if (NULL != sfmObj)
            {
                tivxSFMFreeMem(sfmObj);
            }
        }
    }

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    VX_PRINT(VX_ZONE_WARNING, "All Interrupts DISABLED during SFM process\n");
    #endif

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSFMDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxSFMObj *sfmObj = NULL;

    for (i = 0U; i < num_params; i ++)
    {
        if(NULL == obj_desc[i])
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&sfmObj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != sfmObj) && (sizeof(tivxSFMObj) == size))
        {
            if (sfmObj->algHandle)
            {
                tivxAlgiVisionDelete(sfmObj->algHandle);
            }
            tivxSFMFreeMem(sfmObj);
        }
    }

    #ifdef TIVX_SFM_TARGET_DEBUG
    tivx_clr_debug_zone(VX_ZONE_INFO);
    #endif

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSFMControl(
    tivx_target_kernel_instance kernel, uint32_t node_cmd_id,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

void tivxAddTargetKernelSFM()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == TIVX_CPU_ID_DSP_C7_1)
    {
        strncpy(target_name, TIVX_TARGET_DSP_C7_1, TIVX_TARGET_MAX_NAME);

        vx_sfm_target_kernel = tivxAddTargetKernelByName
                                (
                                  TIVX_KERNEL_SFM_NAME,
                                  target_name,
                                  tivxKernelSFMProcess,
                                  tivxKernelSFMCreate,
                                  tivxKernelSFMDelete,
                                  tivxKernelSFMControl,
                                  NULL
                                );
    }
}


void tivxRemoveTargetKernelSFM()
{
    tivxRemoveTargetKernel(vx_sfm_target_kernel);
}

static void tivxSFMFreeMem(tivxSFMObj *sfmObj)
{
    if (NULL != sfmObj)
    {
        tivxMemFree(sfmObj, sizeof(tivxSFMObj), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

int32_t tivxKernelSFMLog(const char * format, va_list va_args_ptr)
{
    static char buf[1024];

    vsnprintf(buf, 1024, format, va_args_ptr);

    printf(buf);

    return 0;
}

