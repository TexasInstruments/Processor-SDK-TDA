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
#include <utils/udma/include/app_udma.h>
#include <utils/mem/include/app_mem.h>

static tivx_target_kernel vx_img_preproc_target_kernel = NULL;

static void memcpyC662d(uint8_t *pOut, uint8_t *pIn, uint32_t width, uint32_t height, uint32_t src_pitch, uint32_t dest_pitch)
{
    uint32_t i;
    for(i = 0; i < height; i++)
    {
        memcpy(pOut, pIn, width);
        pIn += src_pitch;
        pOut += dest_pitch;
    }
}

static vx_status VX_CALLBACK tivxKernelImgPreProcCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  vx_uint16 num_params,
  void *priv_arg
)
{
  tivxImgPreProcParams * prms = NULL;
  tivx_obj_desc_array_t *params_array = (tivx_obj_desc_array_t*)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_CONFIG_IDX];
  tivxImgPreProcParams * params;
  void *params_array_target_ptr;

  prms = tivxMemAlloc(sizeof(tivxImgPreProcParams), TIVX_MEM_EXTERNAL);

  params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

  tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
      VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

  params = (tivxImgPreProcParams *)params_array_target_ptr;

  // Right now Node config and kernel config is same.
  memcpy(prms,params, sizeof(tivxImgPreProcParams));

  tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
      VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

  tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxImgPreProcParams));

  return(VX_SUCCESS);
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
        tivxImgPreProcParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        tivxMemFree(prms, sizeof(tivxImgPreProcParams), TIVX_MEM_EXTERNAL);

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
    tivxImgPreProcParams *prms = NULL;
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
            (sizeof(tivxImgPreProcParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_array_t* configuration_desc;
        void * configuration_target_ptr;

        tivx_obj_desc_image_t *in_img_desc;
        void* in_tensor_target_ptr[2];

        tivx_obj_desc_tensor_t *out_img_desc;
        void *out_tensor_target_ptr;

        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_CONFIG_IDX];
        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_INPUT_IMAGE_IDX];
        vx_imagepatch_addressing_t *pIn = (vx_imagepatch_addressing_t *)&in_img_desc->imagepatch_addr[0];

        in_tensor_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_tensor_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_tensor_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_tensor_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_tensor_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_img_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_IMG_PREPROCESS_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        if((prms->ip_rgb_or_yuv == 0) || (prms->ip_rgb_or_yuv == 1) || (prms->ip_rgb_or_yuv == 2))
        {
            vx_int32 data_type = TIADALG_DATA_TYPE_U08;
            if(prms->tidl_8bit_16bit_flag == 0){
              data_type = TIADALG_DATA_TYPE_U08;
            }else if(prms->tidl_8bit_16bit_flag == 1){
              data_type = TIADALG_DATA_TYPE_U16;
            }
            status = tiadalg_image_preprocessing_c66
                     (
                        in_tensor_target_ptr,
                        in_img_desc->width,
                        in_img_desc->height,
                        pIn->stride_y,
                        data_type,
                        prms->color_conv_flag,
                        prms->scale_val,
                        prms->mean_pixel,
                        prms->pad_pixel,
                        out_tensor_target_ptr
                     );
            if(status!=TIADALG_PROCESS_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tiadalg failed !!!\n");
            }
        }
        else
        {
          uint8_t *pIn;
          uint8_t *pOut;

          int32_t  inWidth  = in_img_desc->width;
          int32_t  inHeight = in_img_desc->height;

          int32_t  outWidth  = in_img_desc->width + prms->pad_pixel[0] + prms->pad_pixel[2];
          int32_t  outHeight = in_img_desc->height + prms->pad_pixel[1] + prms->pad_pixel[3];

          pIn  = (uint8_t *)in_tensor_target_ptr[0];
          pOut = (uint8_t *)out_tensor_target_ptr + (prms->pad_pixel[1] * outWidth) + prms->pad_pixel[0];
          memcpyC662d(pOut, pIn, inWidth, inHeight, inWidth, outWidth);

          pIn  = (uint8_t *)in_tensor_target_ptr[0];
          pOut = (uint8_t *)out_tensor_target_ptr + (outWidth * outHeight) + (prms->pad_pixel[1] * outWidth) + prms->pad_pixel[0];
          memcpyC662d(pOut, pIn, inWidth, inHeight, inWidth, outWidth);

          pIn  = (uint8_t *)in_tensor_target_ptr[0];
          pOut = (uint8_t *)out_tensor_target_ptr + (outWidth * outHeight * 2) + (prms->pad_pixel[1] * outWidth) + prms->pad_pixel[0];
          memcpyC662d(pOut, pIn, inWidth, inHeight, inWidth, outWidth);
        }

        tivxMemBufferUnmap(out_tensor_target_ptr, out_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_tensor_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (in_tensor_target_ptr[1] != NULL){
          tivxMemBufferUnmap(in_tensor_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

    }

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
