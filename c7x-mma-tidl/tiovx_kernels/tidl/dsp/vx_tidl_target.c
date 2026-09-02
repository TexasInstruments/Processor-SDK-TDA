/*
*
* Copyright (c) 2018-2025 Texas Instruments Incorporated
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
#include <TI/dl_kernels.h>
#include <TI/j7_tidl.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include "itidl_ti.h"
#include "tivx_tidl_trace.h"
#include "tivx_kernels_target_utils.h"
#include "tidl_custom.h"
#include "app_fileio.h"
#include <app_udma.h>
#include "../../rt/inc/tidl_rt_force_negative_test.h"
#include "tivx_tidl_kernels.h"

#ifndef x86_64
#include "c7x.h"
#include <HwiP.h>
#if defined(SOC_AM62A) || defined(SOC_J722S)
#if !defined(MCU_PLUS_SDK)
#include <ti/kernel/freertos/portable/TI_CGT/c7x/Cache.h>
#else
#include <kernel/nortos/dpl/c75/CacheP_c75.h>
#endif
#endif

#if defined(SOC_TDA54) && !defined(HOST_EMULATION)
#include "udma_standalone/udma.h"
#include "hal/Dru/v0/include/Dru_Hal.h"
#define CSL_MSMC_DRU0_CFG_BASE (0x6AC00000UL)
/* Central DRU base address 0x6AC00000UL*/
#define UDMA_UTC_BASE_DRU0 (CSL_MSMC_DRU0_CFG_BASE)
Dru_Hal_InstanceType druHalInstance;
#endif

/* #define DISABLE_PREEMPTION */

#endif

#ifdef x86_64
static uint32_t numCreateCalls = 0;
#endif

/* #define TIVX_TIDL_TARGET_DEBUG */

/* #define TIDL_COPY_NETWORK_BUF */

typedef struct
{
    IVISION_BufDesc     inBufDesc[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc     outBufDesc[TIDL_MAX_ALG_OUT_BUFS];

    IVISION_BufDesc    *inBufDescList[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc    *outBufDescList[TIDL_MAX_ALG_OUT_BUFS];

    IVISION_InBufs      inBufs;
    IVISION_OutBufs     outBufs;

    TIDL_InArgs         *inArgs;
    TIDL_outArgs        *outArgs;

    TIDL_CreateParams   createParams;

    tivxTIDLJ7Params    tidlParams;

    void                *tidlNet;
    vx_uint32            netSize;

    void                *algHandle;

    tivxTIDLTraceDataManager mgr;

} tivxTIDLObj;


#define TIDL_MAX_TARGETS (8U * C7XMMA_COUNT)

static char target_name[TIDL_MAX_TARGETS][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_DSP_C7_1_PRI_1,
    TIVX_TARGET_DSP_C7_1_PRI_2,
    TIVX_TARGET_DSP_C7_1_PRI_3,
    TIVX_TARGET_DSP_C7_1_PRI_4,
    TIVX_TARGET_DSP_C7_1_PRI_5,
    TIVX_TARGET_DSP_C7_1_PRI_6,
    TIVX_TARGET_DSP_C7_1_PRI_7,
    TIVX_TARGET_DSP_C7_1_PRI_8,
#if (C7XMMA_COUNT > 1U)
    TIVX_TARGET_DSP_C7_2_PRI_1,
    TIVX_TARGET_DSP_C7_2_PRI_2,
    TIVX_TARGET_DSP_C7_2_PRI_3,
    TIVX_TARGET_DSP_C7_2_PRI_4,
    TIVX_TARGET_DSP_C7_2_PRI_5,
    TIVX_TARGET_DSP_C7_2_PRI_6,
    TIVX_TARGET_DSP_C7_2_PRI_7,
    TIVX_TARGET_DSP_C7_2_PRI_8,
#endif
#if (C7XMMA_COUNT > 2U)
    TIVX_TARGET_DSP_C7_3_PRI_1,
    TIVX_TARGET_DSP_C7_3_PRI_2,
    TIVX_TARGET_DSP_C7_3_PRI_3,
    TIVX_TARGET_DSP_C7_3_PRI_4,
    TIVX_TARGET_DSP_C7_3_PRI_5,
    TIVX_TARGET_DSP_C7_3_PRI_6,
    TIVX_TARGET_DSP_C7_3_PRI_7,
    TIVX_TARGET_DSP_C7_3_PRI_8,
#endif
#if (C7XMMA_COUNT > 3U)
    TIVX_TARGET_DSP_C7_4_PRI_1,
    TIVX_TARGET_DSP_C7_4_PRI_2,
    TIVX_TARGET_DSP_C7_4_PRI_3,
    TIVX_TARGET_DSP_C7_4_PRI_4,
    TIVX_TARGET_DSP_C7_4_PRI_5,
    TIVX_TARGET_DSP_C7_4_PRI_6,
    TIVX_TARGET_DSP_C7_4_PRI_7,
    TIVX_TARGET_DSP_C7_4_PRI_8,
#endif
};

static tivx_target_kernel vx_tidl_target_kernel[TIDL_MAX_TARGETS] = {NULL};

/* OpenVX Node callbacks */
static vx_status VX_CALLBACK tivxKernelTIDLCreate(tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLProcess(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLDelete(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLControl(tivx_target_kernel_instance kernel,
    uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

/* TIDL App function callbacks */
#if !defined(DISABLE_PREEMPTION)
#if !defined(HOST_EMULATION)
static int32_t TIDL_lockInterrupts(void);
static void TIDL_unlockInterrupts(int32_t oldIntState);
#endif
#endif

static int32_t tivxKernelTIDLLog(const char * format, va_list va_args_ptr);
// static int32_t tivxKernelTIDLDumpToFile(const char * fileName, void * addr, int32_t size, void * tracePtr);

/* File Private functions */
static vx_status testChecksum(void *dataPtr, const uint8_t *refQC, vx_int32 data_size, uint32_t loc);

static int32_t tidl_AllocNetInputMem(IVISION_BufDesc *BufDescList, const sTIDL_IOBufDesc_t *pConfig);
static int32_t tidl_AllocNetOutputMem(IVISION_BufDesc *BufDescList, const sTIDL_IOBufDesc_t *pConfig);

#if !defined(DISABLE_PREEMPTION)
/*
 * Following static lock/unlock functions passed as function pointers to TIDL to internally
 * disable and enable interrupts around critical section.
 */
#if !defined(HOST_EMULATION)
static int32_t TIDL_lockInterrupts(void)
{
    int32_t oldIntState = 0;
#ifndef x86_64
    oldIntState = (int32_t)HwiP_disable();
#endif
    return oldIntState;
}

static void TIDL_unlockInterrupts(int32_t oldIntState)
{
#ifndef x86_64
    HwiP_restore((uintptr_t)oldIntState);
#endif
}
#endif
#endif

static int32_t tidl_AllocNetInputMem(IVISION_BufDesc *BufDescList, const sTIDL_IOBufDesc_t *pConfig)
{
  int32_t numBuffs = 0;

  /* Currently only one input buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numInputBuf; numBuffs++)
  {
    BufDescList[numBuffs].numPlanes                          = 1;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

    /* This has to be width + horizontal padding */
    BufDescList[numBuffs].bufPlanes[0].width                 = (uint32_t)(pConfig->inWidth[numBuffs]) + (uint32_t)(pConfig->inPadL[numBuffs]) + (uint32_t)(pConfig->inPadR[numBuffs]);
    /* This has to be numCh * (height + vertical padding) */
    BufDescList[numBuffs].bufPlanes[0].height                = (uint32_t)(pConfig->inNumChannels[numBuffs]) * (((uint32_t)(pConfig->inHeight[numBuffs])  + (uint32_t)(pConfig->inPadT[numBuffs]) + (uint32_t)(pConfig->inPadB[numBuffs])));
    /* This has to be just width */
    BufDescList[numBuffs].bufPlanes[0].frameROI.width        = (uint32_t)(pConfig->inWidth[numBuffs]);
    /* This has to be just height */
    BufDescList[numBuffs].bufPlanes[0].frameROI.height       = (uint32_t)(pConfig->inHeight[numBuffs]);

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].reserved[0]                        = (uint32_t)(pConfig->inDataId[numBuffs]);

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].bufferId                           = pConfig->inDataId[numBuffs];
  }
  return numBuffs;
}

static int32_t tidl_AllocNetOutputMem(IVISION_BufDesc *BufDescList, const sTIDL_IOBufDesc_t *pConfig)
{
  int32_t numBuffs = 0;

  /* Currently only one output buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numOutputBuf; numBuffs++)
  {
      BufDescList[numBuffs].numPlanes                          = 1;
      BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
      BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

      /* This requires output width + horizontal padding */
      BufDescList[numBuffs].bufPlanes[0].width                 = (uint32_t)(pConfig->outWidth[numBuffs]) + (uint32_t)(pConfig->outPadL[numBuffs]) + (uint32_t)(pConfig->outPadR[numBuffs]);
      /* This requires numOutCh * (output height + vertial padding) */
      BufDescList[numBuffs].bufPlanes[0].height                = (uint32_t)(pConfig->outNumChannels[numBuffs]) * ((uint32_t)(pConfig->outHeight[numBuffs]) + (uint32_t)(pConfig->outPadT[numBuffs]) + (uint32_t)(pConfig->outPadB[numBuffs]));
      /* This requires just output width */
      BufDescList[numBuffs].bufPlanes[0].frameROI.width        = (uint32_t)(pConfig->outWidth[numBuffs]);
      /* This requires just output height */
      BufDescList[numBuffs].bufPlanes[0].frameROI.height       = (uint32_t)(pConfig->outHeight[numBuffs]);

      /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
      BufDescList[numBuffs].reserved[0]                        = (uint32_t)(pConfig->outDataId[numBuffs]);

      /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
      BufDescList[numBuffs].bufferId                           = pConfig->outDataId[numBuffs];
  }
  return numBuffs;
}

static vx_status VX_CALLBACK tivxKernelTIDLProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    VX_PRINT(VX_ZONE_INFO, "DSP tivxKernelTIDLProcess -- num_params = %d, kernel = %p \n", num_params, kernel);

    tivxTIDLObj *tidlObj;
    uint32_t i, size;

    for (i = 0U; i < num_params; i ++)
    {
        /* The parameter at i == 5 is optional and is used to provide a buffer for trace data */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Except trace data object, all other parameters are mandatory and are
        validated before hand by the Validate Callback of the kernel.
        Refer to the function tivxKernelTIDLValidate.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This is follow up of the earlier check,
    all parameters are mandatory and are checked and validated before hand by the
    Validate Callback of the kernel. Refer to the function tivxKernelTIDLValidate.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tidlObj, &size);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        The TIDL kernel is already registered and validated in the context of
        the kernel and this function is called only when the kernel is not in context
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (((vx_status)VX_SUCCESS != status) || (NULL == tidlObj) ||  (sizeof(tivxTIDLObj) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Since the TIDL kernel is already registered and validated in the context,
    tidlObj cannot be NULL.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivx_obj_desc_tensor_t *inTensor;
        tivx_obj_desc_tensor_t *outTensor;
        tivx_obj_desc_user_data_object_t *inArgs;
        tivx_obj_desc_user_data_object_t *outArgs;
        tivx_obj_desc_user_data_object_t *traceData;

        void *in_tensor_target_ptr;
        void *out_tensor_target_ptr;
        void *in_args_target_ptr;
        void *out_args_target_ptr;
        void *trace_data_target_ptr = NULL;

        #if !defined(HOST_EMULATION)
        int32_t oldIntState = 0;
        if(tidlObj->createParams.maxPreEmptDelay == FLT_MAX){
          oldIntState = TIDL_lockInterrupts();
        }
        #endif
    
        /* IMPORTANT! inArgs is assumed to be available at index 3 */
        inArgs   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX];

        in_args_target_ptr = tivxMemShared2TargetPtr(&inArgs->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(in_args_target_ptr, inArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tidlObj->inArgs = in_args_target_ptr;

        /* IMPORTANT! outArgs is assumed to be available at index 4 */
        outArgs  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX];

        out_args_target_ptr = tivxMemShared2TargetPtr(&outArgs->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(out_args_target_ptr, outArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        tidlObj->outArgs = out_args_target_ptr;

        tivxTIDLTraceDataClear(&tidlObj->mgr);

        traceData  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX];
        if((tidlObj->createParams.traceWriteLevel > 0) && (traceData != NULL))
        {
          trace_data_target_ptr = tivxMemShared2TargetPtr(&traceData->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(trace_data_target_ptr, traceData->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

          tivxTIDLTraceDataInit(&tidlObj->mgr, trace_data_target_ptr, traceData->mem_size);
        }

        /* Idx 0 - config data,
           Idx 1 - network data,
           Idx 2 - create parameters,
           Idx 3 - inArgs,
           Idx 4 - outArgs,
           Idx 5 - traceData,
           Idx 6 - input tensor */
        uint32_t in_tensor_idx = TIVX_KERNEL_TIDL_IN_FIRST_TENSOR;

        /* Idx N - output tensors, where N = Idx 2 + number of input tensors */
        uint32_t out_tensor_idx = 0U;
        uint32_t id;

        /*
         * FORCE NEGATIVE TEST START
         *
         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INVALID_INPUT_TYPE
         * forcefully changes obj_desc type to invalid to test else branch
         */
        if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INVALID_INPUT_TYPE)
        {
            obj_desc[in_tensor_idx]->type = (uint16_t)TIVX_OBJ_DESC_INVALID; // 0xFFFF - Invalid type
            VX_PRINT(VX_ZONE_INFO, "DSP: Force negative test - setting invalid input type\n");
        }
        /*
         * FORCE NEGATIVE TEST END
         */

        if (obj_desc[in_tensor_idx]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
        {
            /*
                Currently, this approach will not work if #I/P tensor >32 (TIDL_MAX_ALG_IN_BUFS) and #O/P tensors > 128 (TIDL_MAX_ALG_OUT_BUFS)
                As, this limit is set from iVision side, so need to change this macro value to test beyond that.
            */
            tivx_obj_desc_object_array_t * inputArrayDesc;
            tivx_obj_desc_object_array_t * outputArrayDesc;
            tivx_obj_desc_t *inputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
            tivx_obj_desc_t *outputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
            uint32_t inputElementSize, outputElementSize;

            inputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_INPUT_IDX];
            outputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_OUTPUT_IDX];
            inputElementSize = inputArrayDesc->num_items;
            outputElementSize = outputArrayDesc->num_items;

            tivxGetObjDescList(inputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)inputTensorDescList, inputElementSize);
            tivxGetObjDescList(outputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)outputTensorDescList, outputElementSize);

            // Extracting input tensors
            for(id = 0U; id < inputElementSize; id++)
            {
                inTensor = (tivx_obj_desc_tensor_t*)inputTensorDescList[id];
                in_tensor_target_ptr = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                tidlObj->inBufDesc[id].bufPlanes[0].buf = in_tensor_target_ptr;
            }
            // Extracting output tensors
            out_tensor_idx = 0U;
            for(id = 0U; id < outputElementSize; id++)
            {
                outTensor = (tivx_obj_desc_tensor_t*)outputTensorDescList[out_tensor_idx + id];
                out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                tidlObj->outBufDesc[id].bufPlanes[0].buf = out_tensor_target_ptr;
            }
        }
        else if (obj_desc[in_tensor_idx]->type == (uint16_t)TIVX_OBJ_DESC_TENSOR)
        {
            out_tensor_idx = in_tensor_idx + tidlObj->inBufs.numBufs;
            for(id = 0; id < tidlObj->inBufs.numBufs; id++) {
                inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
                in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                tidlObj->inBufDesc[id].bufPlanes[0].buf = in_tensor_target_ptr;
            }

            for(id = 0; id < tidlObj->outBufs.numBufs; id++) {
                outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
                out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                tidlObj->outBufDesc[id].bufPlanes[0].buf = out_tensor_target_ptr;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Input type not supported by TIDL\n");
            status = (vx_status)VX_FAILURE;
        }

#if defined(SOC_AM62A) || defined(SOC_J722S)
#ifndef x86_64
#if !defined(MCU_PLUS_SDK)
        Cache_wbInvL1dAll();
#else
        CacheP_wbInvAll(CacheP_TYPE_L1D);
#endif
#endif
#endif
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        The input and output tesors are already validated for sanity during create time, hence tensor mapping is always expected to go through.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxAlgiVisionProcess
                        (
                        tidlObj->algHandle,
                        &tidlObj->inBufs,
                        &tidlObj->outBufs,
                        (IVISION_InArgs  *)tidlObj->inArgs,
                        (IVISION_OutArgs *)tidlObj->outArgs,
                        tidlObj->tidlParams.optimize_ivision_activation
                        );
        }

#if defined(SOC_AM62A) || defined(SOC_J722S)
#ifndef x86_64
#if !defined(MCU_PLUS_SDK)
        Cache_wbInvL1dAll();
#else
        CacheP_wbInvAll(CacheP_TYPE_L1D);
#endif
#endif
#endif

        tivxCheckStatus(&status, tivxMemBufferUnmap(in_args_target_ptr, inArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(out_args_target_ptr, outArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        if((tidlObj->createParams.traceWriteLevel > 0) && (traceData != NULL))
        {
           tivxTIDLTraceWriteEOB(&tidlObj->mgr);

           tivxCheckStatus(&status, tivxMemBufferUnmap(trace_data_target_ptr, traceData->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }

        if (obj_desc[in_tensor_idx]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
        {
            tivx_obj_desc_object_array_t * inputArrayDesc;
            tivx_obj_desc_object_array_t * outputArrayDesc;
            tivx_obj_desc_t *inputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
            tivx_obj_desc_t *outputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
            uint32_t inputElementSize, outputElementSize;

            inputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_INPUT_IDX];
            outputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_OUTPUT_IDX];
            inputElementSize = inputArrayDesc->num_items;
            outputElementSize = outputArrayDesc->num_items;

            tivxGetObjDescList(inputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)inputTensorDescList, inputElementSize);
            tivxGetObjDescList(outputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)outputTensorDescList, outputElementSize);

            /* Unmapping input tensors */
            for(id = 0U; id < inputElementSize; id++)
            {
                inTensor = (tivx_obj_desc_tensor_t *)inputTensorDescList[id];
                in_tensor_target_ptr = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
            /* Unmapping output tensors */
            for(id = 0U; id < outputElementSize; id++)
            {
                outTensor = (tivx_obj_desc_tensor_t *)outputTensorDescList[id];
                out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
        }
        else{
            for(id = 0; id < tidlObj->inBufs.numBufs; id++) {
                inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
                in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
    
            for(id = 0; id < tidlObj->outBufs.numBufs; id++) {
                outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
                out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
                tivxCheckStatus(&status, tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
        }
        #if !defined(HOST_EMULATION)
        if(tidlObj->createParams.maxPreEmptDelay == FLT_MAX){
          TIDL_unlockInterrupts(oldIntState);
        }
        #endif    
    }
    return (status);
}


static int32_t tivxKernelTIDLLog(const char * format, va_list va_args_ptr)
{
    static char buf[1024];
    (void)vsnprintf(buf, 1024, format, va_args_ptr);
    #ifdef TIVX_TIDL_TARGET_DEBUG
    /* Added for better debugging, VX_PRINT sends all logs via ipc,
       allowing to see logs of TIDL_VISION_FXNS called via tivxAlgiVision*
    */
    VX_PRINT(VX_ZONE_INFO, " %s", (char *)buf);
    #else
    (void)printf("%s\n", (char *)buf);
    #endif
    return 0;
}

#ifdef x86_64
/* Udma_init for target flow is done part of App Common Init
   Udma is not used in in Host emulation mode of other module, but TIDL
   Has flows which uses UDMA. So intilizing here Specific to TIDL Init.
   This flow is controlled via flowCtrl in create Params
*/
#if defined(SOC_TDA54) || defined(SOC_AM62A) || defined (SOC_J722S)
#include <udma_standalone/udma.h>
#else
#include <udma/udma.h>
#endif
static struct Udma_DrvObj  x86udmaDrvObj[TIDL_MAX_NUM_CORES];
static uint64_t tidlVirtToPhyAddrConversion(const void *virtAddr, uint32_t chNum, void *appData);
static void tidlX86Printf(const char *str);
static void * tidlX86UdmaInit( int32_t coreId);
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static uint64_t tidlVirtToPhyAddrConversion.* <function end>
<justification start> The function is just registered as callback and not used
<justification end> */
static uint64_t tidlVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
    return (uint64_t)virtAddr;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> static void tidlX86Printf.* <function end>
<justification start> The function is just registered as callback and not used
<justification end> */
static void tidlX86Printf(const char *str)
{
}
static void * tidlX86UdmaInit( int32_t coreId)
{
    Udma_InitPrms initPrms;
    (void)UdmaInitPrms_init(UDMA_INST_ID_MAIN_0, &initPrms);

    #if defined SOC_J722S
    if(coreId == 1)
    {
        (void)UdmaInitPrms_init(UDMA_INST_ID_MAIN_1, &initPrms);
    }
    #endif
    initPrms.printFxn = &tidlX86Printf;
    #if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    initPrms.skipGlobalEventReg = 1;
    #endif
    initPrms.virtToPhyFxn = tidlVirtToPhyAddrConversion;
#if defined(SOC_J784S4) || defined(SOC_J742S2) /*need to check on this later */
    int32_t utcId;
    switch (coreId) {
            case 0:
            utcId = UDMA_UTC_ID_C7X_MSMC_DRU4;
            break;
            case 1:
            utcId = UDMA_UTC_ID_C7X_MSMC_DRU5;
            break;
#if defined(SOC_J784S4)
            case 2:
            utcId = UDMA_UTC_ID_C7X_MSMC_DRU6;
            break;
            case 3:
            utcId = UDMA_UTC_ID_C7X_MSMC_DRU7;
            break;
#endif
            default:
            utcId = UDMA_UTC_ID_C7X_MSMC_DRU4;
            break;
    }
    /* For host emulation we udma driver reads utcId based on build for different cores, given
    that for host emulation we have single build this creates problem. Currently we udma driver
    will allocate resource for all the cores based on core 0, which results into allocation failures
    of other cores. To avoid this update the resorces allocated to other cores using core 0 resources */
    initPrms.rmInitPrms.numUtcCh[utcId] = initPrms.rmInitPrms.numUtcCh[UDMA_UTC_ID_C7X_MSMC_DRU4];
#endif

    (void)Udma_init(&x86udmaDrvObj[coreId], &initPrms);

    return &x86udmaDrvObj[coreId];
}
#endif

static vx_status VX_CALLBACK tivxKernelTIDLControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    VX_PRINT(VX_ZONE_INFO, "DSP tivxKernelTIDLControl -- num_params = %d, kernel = %p \n", num_params, kernel);
    tivxTIDLObj *tidlObj;
    uint32_t size, i;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tidlObj, &size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == tidlObj) ||  (sizeof(tivxTIDLObj) != size))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {

        switch (node_cmd_id)
        {
            case TIVX_TIDL_CMD_GET_C7X_PTRS:
            {
                VX_PRINT(VX_ZONE_INFO, "Running TIDL control for TIVX_TIDL_CMD_GET_C7X_PTRS\n");
                tivx_obj_desc_user_data_object_t * memrec_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[0];
                void * memrec_target_ptr = tivxMemShared2TargetPtr(&memrec_desc->mem_ptr);
                (void)tivxMemBufferMap(memrec_target_ptr, memrec_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

                status = tivxAlgiVisionControl
                        (
                            tidlObj->algHandle,
                            (IALG_Cmd)node_cmd_id,
                            NULL,
                            (IALG_Params *)(memrec_target_ptr)
                        );
                VX_PRINT(VX_ZONE_INFO, "TIDL control complete for TIVX_TIDL_CMD_GET_C7X_PTRS\n");

                TIDL_controlGetArgs * controlArgs = (TIDL_controlGetArgs *)memrec_target_ptr;

                /*************************** Get required pointers from DSP **************************/

                /* Get physical ptr for sync buffer  --- TIDL Memrec -- ALG_PERSIST_SYNC_MEMREC */
                void * virtPtr = controlArgs->ctrlGetArgs.syncBufferPtr[TIDL_Virtual];
                void * physPtr = (void *)tivxMemHost2SharedPtr((uint64_t)virtPtr, (vx_enum)TIVX_MEM_EXTERNAL_PERSISTENT_NON_CACHEABLE);
                controlArgs->ctrlGetArgs.syncBufferPtr[TIDL_Physical] = physPtr;
                /*Get Physical Pointer for nc Scratch L1 buffer -- TIDL Memrec -- ALG_SCRATCH_L1_MEM_MEMREC */
                virtPtr = controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L1];
                physPtr = virtPtr;
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L1] = physPtr;
                /*Get Physical Pointer for nc Scratch L2 buffer -- TIDL Memrec -- ALG_SCRATCH_L2_MEM_MEMREC */
                virtPtr = controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L2];
                physPtr = virtPtr;
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L2] = physPtr;
                /*Get Physical Pointer for nc Scratch MSMC buffer -- TIDL Memrec -- ALG_SCRATCH_L3_MEM_MEMREC */
                virtPtr = controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_MSMC];
                physPtr = virtPtr;
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_MSMC] = physPtr;
                /*Get Physical Pointer for nc Scratch DDR buffer -- TIDL Memrec -- ALG_SCRATCH_DATA_BUFF_EXT_MEMREC */
                virtPtr = controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR];
                physPtr = (void *)tivxMemHost2SharedPtr((uint64_t)virtPtr, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR] = physPtr;
                /*Get Physical Pointer for nc persistent DDR_PERSIST buffer -- TIDL Memrec -- ALG_LAYERS_PARAMS_BUFF_MEMREC */
                virtPtr = controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST];
                physPtr = (void *)tivxMemHost2SharedPtr((uint64_t)virtPtr, (vx_enum)TIVX_MEM_EXTERNAL);
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST] = physPtr;
                tivxCheckStatus(&status, tivxMemBufferUnmap(memrec_target_ptr, memrec_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                /*****************************************************************************************/

                /***************************** Info prints for all the get pointers *************/
                VX_PRINT(VX_ZONE_INFO, "GET :: Sync buf: Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.syncBufferPtr[TIDL_Virtual] , controlArgs->ctrlGetArgs.syncBufferPtr[TIDL_Physical]);
                VX_PRINT(VX_ZONE_INFO, "GET :: L1 : Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L1] , controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L1]);
                VX_PRINT(VX_ZONE_INFO, "GET :: L2 : Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L2] , controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L2]);
                VX_PRINT(VX_ZONE_INFO, "GET :: MSMC : Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_MSMC] , controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_MSMC]);
                VX_PRINT(VX_ZONE_INFO, "GET :: DDR : Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR] , controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR]);
                VX_PRINT(VX_ZONE_INFO, "GET :: DDR_persist : Virtual -- %p Physical %p \n",
                controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST] , controlArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST]);
                /********************************************************************************/

                break;
            }
            case TIVX_TIDL_CMD_SET_C7X_PTRS:
            {
                VX_PRINT(VX_ZONE_INFO, "Running TIDL control for TIVX_TIDL_CMD_SET_C7X_PTRS\n");
                /* Copy over the pointers from obj_desc to handle for TIDL alloc to access */
                tivx_obj_desc_user_data_object_t * memrec_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[0];
                void * memrec_target_ptr = tivxMemShared2TargetPtr(&memrec_desc->mem_ptr);
                (void)tivxMemBufferMap(memrec_target_ptr, memrec_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
                /* Get virtual sync ptr for current DSP core */
                TIDL_controlSetArgs * controlArgs = (TIDL_controlSetArgs *)memrec_target_ptr;
                tivx_shared_mem_ptr_t memStruct;

                /*************************** Set required pointers for DSP **************************/

                for(i = 0; i < TIDL_MAX_NUM_CORES; i++)
                {
                    /* Get virtual ptr for sync buffer */
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].syncBufferPtr[TIDL_Physical];
                    void * virtPtr = (void *)tivxMemShared2TargetPtr(&memStruct);
                    controlArgs->ctrlSetArgs[i].syncBufferPtr[TIDL_Virtual] = virtPtr;
                    /*Get virtual Pointer for nc Scratch L1 buffer*/
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_L1];
                    virtPtr = (void *)memStruct.shared_ptr;
                    controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Virtual][TIDL_L1] = virtPtr;
                    /*Get virtual Pointer for nc Scratch L2 buffer*/
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_L2];
                    virtPtr = (void *)memStruct.shared_ptr;
                    controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Virtual][TIDL_L2] = virtPtr;
                    /*Get virtual Pointer for nc Scratch L3 buffer*/
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_MSMC];
                    virtPtr = (void *)memStruct.shared_ptr;
                    controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Virtual][TIDL_MSMC] = virtPtr;
                    /*Get virtual Pointer for nc Scratch DDR buffer*/
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_DDR];
                    virtPtr = (void *)tivxMemShared2TargetPtr(&memStruct);
                    controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Virtual][TIDL_DDR] = virtPtr;
                    /*Get virtual Pointer for nc persistent DDR buffer*/
                    memStruct.shared_ptr = (uint64_t)controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST];
                    virtPtr = (void *)tivxMemShared2TargetPtr(&memStruct);
                    controlArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST] = virtPtr;
                }
                /*************************************************************************************/

                /***************************** Info prints for all the set pointers *************/
                VX_PRINT(VX_ZONE_INFO, "SET :: Sync buf: Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].syncBufferPtr[TIDL_Virtual] , controlArgs->ctrlSetArgs[1].syncBufferPtr[TIDL_Virtual],
                controlArgs->ctrlSetArgs[0].syncBufferPtr[TIDL_Physical], controlArgs->ctrlSetArgs[1].syncBufferPtr[TIDL_Physical]);
                VX_PRINT(VX_ZONE_INFO, "SET :: L1 : Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Virtual][TIDL_L1] , controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Virtual][TIDL_L1],
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Physical][TIDL_L1], controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Physical][TIDL_L1]);
                VX_PRINT(VX_ZONE_INFO, "SET :: L2 : Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Virtual][TIDL_L2] , controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Virtual][TIDL_L2],
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Physical][TIDL_L2], controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Physical][TIDL_L2]);
                VX_PRINT(VX_ZONE_INFO, "SET :: MSMC : Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Virtual][TIDL_MSMC] , controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Virtual][TIDL_MSMC],
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Physical][TIDL_MSMC], controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Physical][TIDL_MSMC]);
                VX_PRINT(VX_ZONE_INFO, "SET :: DDR : Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Virtual][TIDL_DDR] , controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Virtual][TIDL_DDR],
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Physical][TIDL_DDR], controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Physical][TIDL_DDR]);
                VX_PRINT(VX_ZONE_INFO, "SET :: DDR_persist : Virtual -- %p %p Physical %p %p\n",
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST] , controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST],
                controlArgs->ctrlSetArgs[0].ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST], controlArgs->ctrlSetArgs[1].ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST]);
                /********************************************************************************/

                status = tivxAlgiVisionControl
                        (
                            tidlObj->algHandle,
                            (IALG_Cmd)node_cmd_id,
                            (IALG_Params *)(memrec_target_ptr),
                            NULL
                        );

                tivxCheckStatus(&status, tivxMemBufferUnmap(memrec_target_ptr, memrec_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                VX_PRINT(VX_ZONE_INFO, "TIDL control complete for TIVX_TIDL_CMD_SET_C7X_PTRS\n");
                break;
            }
            case TIVX_TIDL_CMD_INIT:
            {
                VX_PRINT(VX_ZONE_INFO, "Running TIDL control for TIVX_TIDL_CMD_INIT \n");
                status = tivxAlgiVisionControl
                        (
                            tidlObj->algHandle,
                            (IALG_Cmd)node_cmd_id,
                            NULL,
                            NULL
                        );
                VX_PRINT(VX_ZONE_INFO, "TIDL control complete for TIVX_TIDL_CMD_INIT\n");
                break;
            }
            case TIVX_TIDL_CODE_COVERAGE_UPLOAD:
            {
                VX_PRINT(VX_ZONE_INFO, "Running TIDL control for  TIVX_TIDL_CODE_COVERAGE_UPLOAD\n");
                status = tivxAlgiVisionControl
                        (
                            tidlObj->algHandle,
                            (IALG_Cmd)node_cmd_id,
                            NULL,
                            NULL
                        );
                VX_PRINT(VX_ZONE_INFO, "TIDL control complete for TIVX_TIDL_CODE_COVERAGE_UPLOAD\n");
                break;
            }
            default:
                break;
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxKernelTIDLCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  uint16_t num_params,
  void *priv_arg
)
{
#if !defined(HOST_EMULATION)
    int32_t oldIntState;
    oldIntState = TIDL_lockInterrupts();
#endif
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_user_data_object_t *config;
    tivx_obj_desc_user_data_object_t *network;
    tivx_obj_desc_user_data_object_t *createParams;

    tivxTIDLObj *tidlObj = NULL;

    void *config_target_ptr = NULL;
    void *network_target_ptr = NULL;
    void *create_params_target_ptr = NULL;

    uint32_t i;

    #ifdef TIVX_TIDL_TARGET_DEBUG
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

    VX_PRINT(VX_ZONE_INFO, "DSP tivxKernelTIDLCreate -- num_params = %d\n", num_params);

    for (i = 0U; i < num_params; i ++)
    {
        /* The parameter at i == 5 is optional and is used to provide a buffer for trace data */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Can't be true in due to Validate Callback of kernel, all the params are
        checked and validated in the Validate Callback of the kernel.
        Refer tivxKernelTIDLValidate function.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This is a follow up check,
    Can't be true in due to Validate Callback of kernel, all the params are
    checked and validated in the Validate Callback of the kernel.
    Refer tivxKernelTIDLValidate function.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }

    #if defined(SOC_J784S4) || defined(SOC_AM62A) || defined (SOC_J722S) || defined (SOC_TDA54) || defined(SOC_J742S2)
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Mem reset is always expected to be successful,
    appMemResetScratchHeap handles TIVX_MEM_EXTERNAL_SCRATCH as valid region.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE);
    }
    /* LDRA_JUSTIFY_END */
    #endif

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Mem reset is always expected to be successful,
    appMemResetScratchHeap handles TIVX_MEM_EXTERNAL_SCRATCH and
    TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE as valid region.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* IMPORTANT! Config data is assumed to be available at index 0 */
        config    = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];

        /* IMPORTANT! Network data is assumed to be available at index 1 */
        network   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];

        /* IMPORTANT! Create params is assumed to be available at index 2 */
        createParams   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];

        tidlObj = tivxMemAlloc((uint32_t)(sizeof(tivxTIDLObj)), (vx_enum)TIVX_MEM_EXTERNAL);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Before create callback is being called, all of shm is not used/allocated
        by other objects, so tidlObj cannot be NULL.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (NULL != tidlObj)
        /* LDRA_JUSTIFY_END */
        {
            (void)memset(tidlObj, 0, sizeof(tivxTIDLObj));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Before create callback is being called, all of shm is not used/allocated
        by other objects, so tidlObj cannot be NULL.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        tivx Mem alloc is always expected to be successful,and hence 
        tidlObj cannot be NULL. Hence status is never VX_ERROR_NO_MEMORY
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
          config_target_ptr = tivxMemShared2TargetPtr(&config->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

          (void)memcpy((void *)&tidlObj->tidlParams, (void *)config_target_ptr, sizeof(tivxTIDLJ7Params));

          if(tidlObj->tidlParams.compute_config_checksum == 1U)
          {
            status = testChecksum(&tidlObj->tidlParams.ioBufDesc, &tidlObj->tidlParams.config_checksum[0], (int32_t)(sizeof(sTIDL_IOBufDesc_t)), 0U);
          }
        }
        if ((vx_status)VX_SUCCESS == status)
        {
          tidlObj->tidlParams.ioBufDesc.numInputBuf = tidlObj->tidlParams.ioBufDesc.numInputBuf / tidlObj->tidlParams.ioBufDesc.numVirtualCores;
          tidlObj->tidlParams.ioBufDesc.numOutputBuf = tidlObj->tidlParams.ioBufDesc.numOutputBuf / tidlObj->tidlParams.ioBufDesc.numVirtualCores;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            #ifdef TIDL_COPY_NETWORK_BUF
            tidlObj->tidlNet = tivxMemAlloc(network->mem_size, (vx_enum)TIVX_MEM_EXTERNAL);
            tidlObj->netSize = network->mem_size;
            if (NULL == tidlObj->tidlNet)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            #else
            tidlObj->tidlNet = NULL;
            tidlObj->netSize = 0;
            #endif
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          network_target_ptr = tivxMemShared2TargetPtr(&network->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(network_target_ptr, network->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

          #ifdef TIDL_COPY_NETWORK_BUF
          (void)memcpy((void *)tidlObj->tidlNet, (void *)network_target_ptr, network->mem_size);
          #else
          tidlObj->tidlNet = network_target_ptr;
          tidlObj->netSize = network->mem_size;
          #endif

          if(tidlObj->tidlParams.compute_network_checksum == 1U)
          {
            sTIDL_Network_t *pNet = (sTIDL_Network_t *)network_target_ptr;
            uint8_t *pPerfInfo = (uint8_t *)network_target_ptr + pNet->dataFlowInfo;

            VX_PRINT(VX_ZONE_INFO, "tidlObj->netSize = %d\n", tidlObj->netSize);
            VX_PRINT(VX_ZONE_INFO, "pNet->dataFlowInfo = %d \n", pNet->dataFlowInfo);

            status = testChecksum(pPerfInfo, &tidlObj->tidlParams.network_checksum[0], (int32_t)tidlObj->netSize - (int32_t)pNet->dataFlowInfo, 0U);
          }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          create_params_target_ptr = tivxMemShared2TargetPtr(&createParams->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            (void)memcpy((void *)&tidlObj->createParams, (void *)create_params_target_ptr, sizeof(TIDL_CreateParams));
#if defined (HOST_EMULATION)
            tidlObj->createParams.tracePtr = NULL;
#else
            tidlObj->createParams.tracePtr = (void *)&tidlObj->mgr;
#endif
            #if defined(DISABLE_PREEMPTION)
            VX_PRINT(VX_ZONE_INFO, "Preemption is disabled\n");
            #else
#if defined (HOST_EMULATION)
            tidlObj->createParams.pFxnLock = NULL;
            tidlObj->createParams.pFxnUnLock = NULL;
#else
            if(tidlObj->createParams.maxPreEmptDelay != FLT_MAX)
            {
              tidlObj->createParams.pFxnLock = TIDL_lockInterrupts;
              tidlObj->createParams.pFxnUnLock = TIDL_unlockInterrupts;
            }
#endif
            status = tivxGetTargetKernelTargetId(kernel, &tidlObj->createParams.targetPriority);
            VX_PRINT(VX_ZONE_INFO, "Enabling preemption\n");
            #endif
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivx_mem_stats l1_stats;
            tivx_mem_stats l2_stats;
            tivx_mem_stats l3_stats;

            /* reset scratch heap offset to zero by doing a dummy free */
#ifdef x86_64
            /* PC emulation maintains a global offset for allocating L1, L2 and
                L3 memories. Mem Free sets this global offset to 0. Doing mem
                free for each core results in same L1, L2 and L3 ptrs allocated
                across all cores. So, do reset only for 1st core to enable
                different ptrs for each of the cores */
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Inference mode can be high throughput or low latency only in
            multicore cases
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            #endif
            if(numCreateCalls == 0U)
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_END */
            #endif
            {
#endif
                (void)tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L1);
                (void)tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
                (void)tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L3);
#ifdef x86_64
            }
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Inference mode can be high throughput or low latency only in
            multicore cases
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            #endif
            if((tidlObj->tidlParams.ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeHighThroughput) ||
                (tidlObj->tidlParams.ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
            {
                numCreateCalls++;
            }
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_END */
            #endif
#endif
            tivxMemStats(&l1_stats, (vx_enum)TIVX_MEM_INTERNAL_L1);
            tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemStats(&l3_stats, (vx_enum)TIVX_MEM_INTERNAL_L3);

            VX_PRINT(VX_ZONE_INFO, "L1 = %u KB, L2 = %u KB, L3 = %u KB\n",
                l1_stats.free_size/1024U,
                l2_stats.free_size/1024U,
                l3_stats.free_size/1024U
                );
#ifdef x86_64
            tidlObj->createParams.udmaDrvObj = tidlX86UdmaInit((int32_t)tidlObj->createParams.coreId);
#elif defined(SOC_TDA54)
            Dru_Hal_InitParamsType druHalInitParams;
            // Udma_init also uses central dru 
            druHalInitParams.HwDruRegsPtr = (Dru_Hal_Regs *) UDMA_UTC_BASE_DRU0;
            druHalInitParams.UserArgsPtr = NULL;
            Dru_Hal_Init(&druHalInstance, &druHalInitParams);
            tidlObj->createParams.udmaDrvObj = &druHalInstance;
#else
            tidlObj->createParams.udmaDrvObj = appUdmaGetObj();
#endif

            tidlObj->createParams.net = (sTIDL_Network_t *)tidlObj->tidlNet;

            tidlObj->createParams.TIDLVprintf = tivxKernelTIDLLog;

            // tidlObj->createParams.TIDLWriteBinToFile = tivxKernelTIDLDumpToFile;
            tidlObj->createParams.TIDLWriteBinToFile = appWriteBinToFile;
            tidlObj->createParams.TIDLReadBinFromFile = appReadBinFromFile;
            tidlObj->createParams.TIDL_CustomLayerProcess = TIDL_customLayerProcess;
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Inference mode can be high throughput or low latency only in
            multicore cases
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            #endif
            if((tidlObj->tidlParams.ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeDefault) ||
                (tidlObj->tidlParams.ioBufDesc.inferenceMode == (int32_t)TIDL_inferenceModeHighThroughput))
            #if defined(SOC_J721S2) || defined(SOC_AM62A)
            /* LDRA_JUSTIFY_END */
            #endif
            {
                /* Both these modes are equivalent to single core inference from core TIDL point of view
                Target setting for the node is already done one level above these kernels */
                tidlObj->createParams.coreStartIdx = tidlObj->createParams.coreId;
            }

            if(TIDL_NET_VERSION_FW_ACTIVE != tidlObj->createParams.net->netVersion)
            {
                VX_PRINT(VX_ZONE_ERROR, "Network version - 0x%08X, Expected version - 0x%08X\n",
                    tidlObj->createParams.net->netVersion,
                    TIDL_NET_VERSION_FW_ACTIVE
                );

                status = (vx_status)VX_FAILURE;
            }
            if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INIT_OPTEXTMEM)
            {
                tidlObj->createParams.optimiseExtMem = TIDL_OptimiseExtMemL0;
            }
            if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_UTILS_FORCE_PADEDBUF_ZERO)
            {
                tidlObj->createParams.isInbufsPaded = 0;
            }
            if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_ALLOC_FORCE_VPRINTF_NULL)
            {
                tidlObj->createParams.TIDLVprintf = NULL;
            }
            if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_ALLOC_FORCE_DMACREATEOBJ_NULL)
            {
                tidlObj->createParams.udmaDrvObj = NULL;
            }
            if(tidlObj->createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PREEMPTION_DISABLED)
            {
               tidlObj->createParams.pFxnLock = NULL;
               tidlObj->createParams.pFxnUnLock = NULL;
            }
            if ((vx_status)VX_SUCCESS == status)
            {

                tidlObj->algHandle = tivxAlgiVisionCreate
                                    (
                                        &TIDL_VISION_FXNS,
                                        (IALG_Params *)(&tidlObj->createParams)
                                    );

                if (NULL == tidlObj->algHandle)
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxAlgiVisionCreate returned NULL\n");
                    status = (vx_status)VX_FAILURE;
                }

                tidlObj->inBufs.size     = (uint32_t)(sizeof(tidlObj->inBufs));
                tidlObj->outBufs.size    = (uint32_t)(sizeof(tidlObj->outBufs));

                tidlObj->inBufs.bufDesc  = tidlObj->inBufDescList;
                tidlObj->outBufs.bufDesc = tidlObj->outBufDescList;

                tidlObj->inBufs.numBufs  = (uint32_t)tidl_AllocNetInputMem(tidlObj->inBufDesc, &tidlObj->tidlParams.ioBufDesc);
                tidlObj->outBufs.numBufs = (uint32_t)tidl_AllocNetOutputMem(tidlObj->outBufDesc, &tidlObj->tidlParams.ioBufDesc);

                for(i = 0; i < tidlObj->inBufs.numBufs; i++)
                {
                    tidlObj->inBufDescList[i]     = &tidlObj->inBufDesc[i];
                }
                for(i = 0; i < tidlObj->outBufs.numBufs; i++)
                {
                    tidlObj->outBufDescList[i]     = &tidlObj->outBufDesc[i];
                }
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(network_target_ptr, network->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

        if ((vx_status)VX_SUCCESS == status)
        {
            (void)tivxSetTargetKernelInstanceContext(kernel, tidlObj,  (uint32_t)(sizeof(tivxTIDLObj)));
        }
        else
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Since the TIDL kernel is already registered and validated in the context,
            tidlObj cannot be NULL.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (NULL != tidlObj)
            /* LDRA_JUSTIFY_END */
            {
                #ifdef TIDL_COPY_NETWORK_BUF
                if (NULL != tidlObj->tidlNet)
                {
                    (void)tivxMemFree(tidlObj->tidlNet, tidlObj->netSize, (vx_enum)TIVX_MEM_EXTERNAL);
                }
                #endif
                (void)tivxMemFree(tidlObj, (uint32_t)(sizeof(tivxTIDLObj)), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

#if !defined(HOST_EMULATION)
    TIDL_unlockInterrupts(oldIntState);
#endif
    return (status);
}

static vx_status VX_CALLBACK tivxKernelTIDLDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    VX_PRINT(VX_ZONE_INFO, "DSP tivxKernelTIDLDelete\n");

    uint32_t i;
    uint32_t size;
    tivxTIDLObj *tidlObj = NULL;

    for (i = 0U; i < num_params; i ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Expected to call this for TIDL kernel after inference, and
        since the TIDL kernel is already registered and validated in the context.
        Except trace data object, all other parameters are mandatory.
        Refer to the function tivxKernelTIDLValidate.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    This is a follow up check,
    Expected to call this for TIDL kernel after inference, and
    since the TIDL kernel is already registered and validated in the context.
    Except trace data object, all other parameters are mandatory.
    Refer to the function tivxKernelTIDLValidate.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tidlObj, &size);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Since the TIDL kernel is already registered and validated and is used
        for inference in the context, tidlObj cannot be NULL.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (NULL != tidlObj) && (sizeof(tivxTIDLObj) == size))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Safe programming ideally should not fail as algHandle is used during
            inference and should not be NULL.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (tidlObj->algHandle != NULL)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxAlgiVisionDelete(tidlObj->algHandle);
            }
            #ifdef TIDL_COPY_NETWORK_BUF
            if (NULL != tidlObj->tidlNet)
            {
                (void)tivxMemFree(tidlObj->tidlNet, tidlObj->netSize, (vx_enum)TIVX_MEM_EXTERNAL);
            }
            #endif
            (void)tivxMemFree(tidlObj, (uint32_t)(sizeof(tivxTIDLObj)), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    #ifdef TIVX_TIDL_TARGET_DEBUG
    tivx_clr_debug_zone(VX_ZONE_INFO);
    #endif
    return (status);
}


static vx_status testChecksum(void *dataPtr, const uint8_t *refQC, vx_int32 data_size, uint32_t loc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint8 qcData[TIVX_TIDL_J7_CHECKSUM_SIZE];
    int32_t match = 1;
    int32_t x;

    /* Get QC of config params passed from host to target */
    tivx_tidl_j7_get_qc(dataPtr, qcData, data_size);

    /* Print QC */
    for(x = 0; x < TIVX_TIDL_J7_CHECKSUM_SIZE; x++)
    {
      if(qcData[x] != refQC[x])
      {
        match = 0;
        break;
      }
    }
    if(match == 0)
    {
      VX_PRINT(VX_ZONE_ERROR, "Computing checksum at 0x%08X, size = %d\n", dataPtr,  data_size);
      VX_PRINT(VX_ZONE_ERROR, "QC code mismatch at %d \n", loc);
      status = (vx_status)VX_FAILURE;
    }
    else
    {
      VX_PRINT(VX_ZONE_INFO, "QC code match! \n");
    }

    return status;
}

/* Public Functions */

void tivxAddTargetKernelTIDL(void)
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    #if defined(SOC_J721S2) || defined(SOC_AM62A)
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    For single code the c7x can only be c7_1
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    #endif
    if ((self_cpu == (vx_enum)TIVX_CPU_ID_DSP_C7_1)
#if (C7XMMA_COUNT > 1U)
        || (self_cpu == (vx_enum)TIVX_CPU_ID_DSP_C7_2)
#endif
#if (C7XMMA_COUNT > 2U)
        || (self_cpu == (vx_enum)TIVX_CPU_ID_DSP_C7_3)
#endif
#if (C7XMMA_COUNT > 3U)
        || (self_cpu == (vx_enum)TIVX_CPU_ID_DSP_C7_4)
#endif
        )
    #if defined(SOC_J721S2) || defined(SOC_AM62A)
    /* LDRA_JUSTIFY_END */
    #endif
    {
        uint32_t i;

        for (i = 0; i < TIDL_MAX_TARGETS; i++)
        {
            vx_tidl_target_kernel[i] = tivxAddTargetKernelByName
                                    (
                                      TIVX_KERNEL_TIDL_NAME,
                                      target_name[i],
                                      tivxKernelTIDLProcess,
                                      tivxKernelTIDLCreate,
                                      tivxKernelTIDLDelete,
                                      tivxKernelTIDLControl,
                                      NULL
                                    );
        }
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tivxRemoveTargetKernelTIDL.* <function end>
<justification start>
Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
<justification end> */
void tivxRemoveTargetKernelTIDL(void)
{
    uint32_t i;

    for (i = 0; i < TIDL_MAX_TARGETS; i++)
    {
        (void)tivxRemoveTargetKernel(vx_tidl_target_kernel[i]);
    }
}
