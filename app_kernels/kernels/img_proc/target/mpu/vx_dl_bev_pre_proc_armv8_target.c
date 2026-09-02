/*
*
* Copyright (c) 2025-2026 Texas Instruments Incorporated
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

#include <TI/tivx_img_proc_kernels.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"
#include <tivx_dl_bev_pre_proc_armv8_host.h>

#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>



#define DL_PRE_PROC_MAX_KERNELS 4

#include <stdint.h>

#define MAX_TENSOR_DIMS (4)

/* Supported tensor formats in dl-pre-proc */
#define DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB   (0)
#define DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR   (1)

/* Supported channel ordering in dl-pre-proc */
#define DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW   (0)
#define DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC   (1)


/*  ###########################
    UTILS FOR DL_PRE_PROC_ARMV8
    ###########################
*/
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
#include <arm_neon.h>
#endif

/*  ############################
    MACROS FOR DL_PRE_PROC_ARMV8
    ############################
*/

#define CLIP_UNSIGNED(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define CLIP_SIGNED(X) ( (X) > 127 ? 127 : (X) < -128 ? -128 : X)

#define YUV2R(Y, U, V) (Y + ((25802 * V) >> 14) - 201)
#define YUV2G(Y, U, V) (Y - ((3068 * U + 7669 * V) >> 14) + 83)
#define YUV2B(Y, U, V) (Y + ((30402 * U) >> 14) - 237)



/*  ###########################
    UTILS FOR DL_PRE_PROC_ARMV8
    ###########################
*/

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
static inline int16x8_t dlPreProc_yuv2rgb_vtr(uint8x8_t y, int16x8_t calc)
{
    int16x8_t y_tmp, rgb_tmp;

    /* Widening from 8x8 to 16x8
       and then uint16x8 to int16x8 */
    y_tmp = vreinterpretq_s16_u16(vmovl_u8(y));

    /* r = y + calc_v */
    rgb_tmp = vaddq_s16(y_tmp, calc);

    return rgb_tmp;
}
#endif

typedef struct
{
    /* Channel ordering, 0-NCHW, 1-NHWC */
    uint32_t channel_order;

    /* Tensor format, 0-RGB, 1-BGR */
    uint32_t tensor_format;

    /*
    These are VX data type enums used for below
    mentioned data formats. Corresponding values
    must be used for using that particular data
    type.
    VX_TYPE_INT8            = 0x002
    VX_TYPE_UINT8           = 0x003
    VX_TYPE_INT16           = 0x004
    VX_TYPE_UINT16          = 0x005
    VX_TYPE_INT32           = 0x006
    VX_TYPE_UINT32          = 0x007
    VX_TYPE_FLOAT32         = 0x00A
    */
    uint32_t tensor_data_type;

    uint32_t input_width;
    uint32_t input_height;
    uint32_t in_stride_y;

    /* Input image data pointers of two planes */
    void*    in_img_target_ptr[2];
    
    /*##################Taking 6 Vx Images and Creating 4 Dimension Tensor- BEV Change
    #############################################################*/
    void*    in_img2_target_ptr[2];
    void*    in_img3_target_ptr[2];
    void*    in_img4_target_ptr[2];
    void*    in_img5_target_ptr[2];
    void*    in_img6_target_ptr[2];

    /*#############################################################################*/
    uint32_t output_dimensions[MAX_TENSOR_DIMS];
    void*    out_tensor_target_ptr;
    uint32_t number_of_dimensions ;
    float    mean[4];
    float    scale[4];
    /*
    BEV Change
    float    mean[MAX_TENSOR_DIMS];
    float    scale[MAX_TENSOR_DIMS];
    */

    uint32_t channel_pitch;

}dl4DPreProcessImageParams;

void dlPreProcess_4D_RGB_image
(
    dl4DPreProcessImageParams *prms
);

void dlPreProcess_4D_NV12_image
(
    dl4DPreProcessImageParams *prms
);



static tivx_target_kernel vx_dl_pre_proc_armv8_target_kernel[DL_PRE_PROC_MAX_KERNELS] = {NULL};

static vx_status VX_CALLBACK tivxKernelDLPreProc4DArmv8Create
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    tivxDLPreProcArmv8Params4D * kernelParams = NULL;

    kernelParams = tivxMemAlloc(sizeof(tivxDLPreProcArmv8Params4D), TIVX_MEM_EXTERNAL);
    if (NULL == kernelParams)
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxDLPreProcArmv8Params4D));
    }
    
    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLPreProc4DArmv8Delete(
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
        tivxDLPreProcArmv8Params4D *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxDLPreProcArmv8Params4D), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLPreProc4DArmv8Process
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    VX_PRINT(VX_ZONE_INFO, "Entering Process Kernel\n");
    vx_status status = VX_SUCCESS;

    tivxDLPreProcArmv8Params4D *prms = NULL;
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
            (sizeof(tivxDLPreProcArmv8Params4D) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr;

        tivx_obj_desc_image_t *in_img_desc;
        void* in_img_target_ptr[2];

        tivx_obj_desc_tensor_t *out_tensor_desc;
        void *out_tensor_target_ptr;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE_IDX];
        in_img_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        /* ################################# 4D BEV Modification-Extracting 6 Vx_Images at Target side to create Tensor*/
        tivx_obj_desc_image_t *in_img2_desc;
        tivx_obj_desc_image_t *in_img3_desc;
        tivx_obj_desc_image_t *in_img4_desc;
        tivx_obj_desc_image_t *in_img5_desc;
        tivx_obj_desc_image_t *in_img6_desc; 

        void* in_img2_target_ptr[2];
        void* in_img3_target_ptr[2];
        void* in_img4_target_ptr[2];
        void* in_img5_target_ptr[2];
        void* in_img6_target_ptr[2];

        in_img2_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE2_IDX];
        in_img2_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img2_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img2_target_ptr[0], in_img2_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img2_target_ptr[1]  = NULL;
        if(in_img2_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img2_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img2_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img2_target_ptr[1], in_img2_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        in_img3_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE3_IDX];
        in_img3_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img3_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img3_target_ptr[0], in_img3_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img3_target_ptr[1]  = NULL;
        if(in_img3_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img3_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img3_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img3_target_ptr[1], in_img3_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        in_img4_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE4_IDX];
        in_img4_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img4_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img4_target_ptr[0], in_img4_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img4_target_ptr[1]  = NULL;
        if(in_img4_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img4_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img4_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img4_target_ptr[1], in_img4_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        in_img5_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE5_IDX];
        in_img5_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img5_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img5_target_ptr[0], in_img5_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img5_target_ptr[1]  = NULL;
        if(in_img5_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img5_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img5_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img5_target_ptr[1], in_img5_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        in_img6_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE6_IDX];
        in_img6_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img6_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img6_target_ptr[0], in_img6_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img6_target_ptr[1]  = NULL;
        if(in_img6_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img6_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img6_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img6_target_ptr[1], in_img6_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        /*#################################################################*/

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        
        /*VX_PRINT(VX_ZONE_INFO, "Image channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[1].step_y);
        }
        
        // ################################# 4D BEV Modification-Extracting 6 Vx_Images at Target side to create Tensor
        VX_PRINT(VX_ZONE_INFO, "Image2 channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img2_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img2_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img2_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img2_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img2_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img2_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image2 channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img2_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img2_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img2_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img2_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img2_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img2_desc->imagepatch_addr[1].step_y);
        }
        VX_PRINT(VX_ZONE_INFO, "Image3 channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img3_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img3_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img3_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img3_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img3_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img3_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image3 channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img3_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img3_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img3_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img3_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img3_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img3_desc->imagepatch_addr[1].step_y);
        }
        
        VX_PRINT(VX_ZONE_INFO, "Image4 channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img4_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img4_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img4_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img4_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img4_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img4_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image4 channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img4_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img4_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img4_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img4_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img4_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img4_desc->imagepatch_addr[1].step_y);
        }

        VX_PRINT(VX_ZONE_INFO, "Image5 channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img5_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img5_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img5_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img5_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img5_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img5_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image5 channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img5_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img5_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img5_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img5_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img5_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img5_desc->imagepatch_addr[1].step_y);
        }

        VX_PRINT(VX_ZONE_INFO, "Image6 channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img6_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img6_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img6_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img6_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img6_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img6_desc->imagepatch_addr[0].step_y);
        
        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image6 channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img6_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img6_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img6_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img6_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img6_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img6_desc->imagepatch_addr[1].step_y);
        }*/   
        /*#############################*/
        // VX_PRINT(VX_ZONE_INFO, "out_tensor_desc\n");
        // VX_PRINT(VX_ZONE_INFO, "stride[0] = %d\n", out_tensor_desc->stride[0]);
        // VX_PRINT(VX_ZONE_INFO, "stride[1] = %d\n", out_tensor_desc->stride[1]);
        // VX_PRINT(VX_ZONE_INFO, "stride[2] = %d\n", out_tensor_desc->stride[2]);
        // VX_PRINT(VX_ZONE_INFO, "dimensions[0] = %d\n", out_tensor_desc->dimensions[0]);
        // VX_PRINT(VX_ZONE_INFO, "dimensions[1] = %d\n", out_tensor_desc->dimensions[1]);
        // VX_PRINT(VX_ZONE_INFO, "dimensions[2] = %d\n", out_tensor_desc->dimensions[2]);
        // if (out_tensor_desc->number_of_dimensions == 4)
        // {
        //     VX_PRINT(VX_ZONE_INFO, "stride[3] = %d\n", out_tensor_desc->stride[3]);
        //     VX_PRINT(VX_ZONE_INFO, "dimensions[3] = %d\n", out_tensor_desc->dimensions[3]);
        // }
        

        tivxDLPreProcArmv8Params4D *dlParams = (tivxDLPreProcArmv8Params4D *)config_target_ptr;
        dl4DPreProcessImageParams params;

        params.channel_order            = dlParams->channel_order;
        params.tensor_format            = dlParams->tensor_format;
        params.tensor_data_type         = out_tensor_desc->data_type;
        params.input_width              = in_img_desc->imagepatch_addr[0].dim_x;
        params.input_height             = in_img_desc->imagepatch_addr[0].dim_y;
        params.in_stride_y              = in_img_desc->imagepatch_addr[0].stride_y;
        params.in_img_target_ptr[0]     = in_img_target_ptr[0];
        params.in_img_target_ptr[1]     = in_img_target_ptr[1];
        params.output_dimensions[0]     = out_tensor_desc->dimensions[0];
        params.output_dimensions[1]     = out_tensor_desc->dimensions[1];
        params.output_dimensions[2]     = out_tensor_desc->dimensions[2];
        params.out_tensor_target_ptr    = out_tensor_target_ptr;
        params.mean[0]                  = dlParams->mean[0];
        params.mean[1]                  = dlParams->mean[1];
        params.mean[2]                  = dlParams->mean[2];
        params.scale[0]                 = dlParams->scale[0];
        params.scale[1]                 = dlParams->scale[1];
        params.scale[2]                 = dlParams->scale[2];
        params.channel_pitch            = dlParams->channel_pitch;

        /*##################################################################################
        ----------------------- 4D BEV Modifications to add 6 images to params------------
        ################################################################*/
        params.in_img2_target_ptr[0]     = in_img2_target_ptr[0];
        params.in_img2_target_ptr[1]     = in_img2_target_ptr[1];
        params.in_img3_target_ptr[0]     = in_img3_target_ptr[0];
        params.in_img3_target_ptr[1]     = in_img3_target_ptr[1];
        params.in_img4_target_ptr[0]     = in_img4_target_ptr[0];
        params.in_img4_target_ptr[1]     = in_img4_target_ptr[1];
        params.in_img5_target_ptr[0]     = in_img5_target_ptr[0];
        params.in_img5_target_ptr[1]     = in_img5_target_ptr[1];
        params.in_img6_target_ptr[0]     = in_img6_target_ptr[0];
        params.in_img6_target_ptr[1]     = in_img6_target_ptr[1];
        
        
        if (out_tensor_desc->number_of_dimensions == 4)
        {
            params.output_dimensions[3]      = out_tensor_desc->dimensions[3];
            params.number_of_dimensions = out_tensor_desc->number_of_dimensions;
            params.mean[3]                  = dlParams->mean[3];
            params.scale[3]                 = dlParams->scale[3];

        }
        
        /*#######################################################################################*/

        vx_df_image image_format = in_img_desc->format;

        if(image_format == VX_DF_IMAGE_NV12)
        {

            VX_PRINT(VX_ZONE_INFO, "Entering NV12 PRE PROC\n");
            dlPreProcess_4D_NV12_image(&params);
            
        }

        /* Write DL pre proc operation here */
        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
         /*##################################################################################
        -----------------------BEV Change ---Modifications to add 6 images to params------------
        ################################################################*/
        tivxMemBufferUnmap(in_img2_target_ptr[0], in_img2_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img2_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img2_target_ptr[1], in_img2_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(in_img3_target_ptr[0], in_img3_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img3_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img3_target_ptr[1], in_img3_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(in_img4_target_ptr[0], in_img4_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img2_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img4_target_ptr[1], in_img4_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(in_img5_target_ptr[0], in_img5_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img3_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img5_target_ptr[1], in_img5_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(in_img6_target_ptr[0], in_img6_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img3_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img6_target_ptr[1], in_img6_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

         /*##################################################################################*/
    }
    VX_PRINT(VX_ZONE_INFO, "Completed Process Kernel\n");
    return (status);
}

void tivxAddTargetKernelDLPreProc4DArmv8()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[DL_PRE_PROC_MAX_KERNELS][TIVX_TARGET_MAX_NAME];

    strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if( (vx_status)VX_SUCCESS == status)
    {
        int i;

        for (i = 0; i < DL_PRE_PROC_MAX_KERNELS; i++)
        {
            vx_dl_pre_proc_armv8_target_kernel[i] = tivxAddTargetKernelByName
                                                    (
                                                        TIVX_KERNEL_DL_PRE_PROC_ARMV8_4D_NAME,
                                                        target_name[i],
                                                        tivxKernelDLPreProc4DArmv8Process,
                                                        tivxKernelDLPreProc4DArmv8Create,
                                                        tivxKernelDLPreProc4DArmv8Delete,
                                                        NULL,
                                                        NULL
                                                    );
        }
    }
}

void tivxRemoveTargetKernelDLPreProc4DArmv8()
{
    vx_status status = VX_SUCCESS;
    int i;

    for (i = 0; i < DL_PRE_PROC_MAX_KERNELS; i++)
    {
        status = tivxRemoveTargetKernel(vx_dl_pre_proc_armv8_target_kernel[i]);
        if (status == VX_SUCCESS)
        {
            vx_dl_pre_proc_armv8_target_kernel[i] = NULL;
        }
    }
}
void dlPreProcess_4D_NV12_image
(
    dl4DPreProcessImageParams *prms
)
{
    
    uint32_t skip_mean_scale = 0;
    uint32_t pos_x = 0, pos_y = 0, input_batch =1, batch_iteration = 0 ;
    uint32_t ch_offset = prms->channel_pitch;
    
    /*BEV Modification change*/
    uint32_t batch_offset = prms->channel_pitch * (prms->output_dimensions[2]-1); 


    if(prms->mean[0]==0 && prms->mean[1]==0 && prms->mean[2]==0 &&
       prms->scale[0]==1 && prms->scale[1]==1 && prms->scale[2]==1)
    {
        skip_mean_scale = 1;
    }
    //VX_PRINT(VX_ZONE_INFO, "prms->tensor format before modification = %d\n", prms->tensor_format);
    /*if((prms->tensor_data_type == VX_TYPE_INT8) || (prms->tensor_data_type == VX_TYPE_UINT8))
    {
        if(prms->tensor_format == 1)
            {
                prms->tensor_format = DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB; //RGB
            }
        else prms->tensor_format = DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR; //BGR
    }
    else if((prms->tensor_data_type == VX_TYPE_INT16) || (prms->tensor_data_type  == VX_TYPE_UINT16))
    {
        if(prms->tensor_format == 1)
           {
                prms->tensor_format = DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB; //RGB
           } 
        else prms->tensor_format = DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR; //BGR
    }*/
    VX_PRINT(VX_ZONE_INFO, "prms->channel_order= %d\n", prms->channel_order);
    VX_PRINT(VX_ZONE_INFO, "prms->tensor format after modification = %d\n", prms->tensor_format);
    VX_PRINT(VX_ZONE_INFO, "prms->tensor_data_type = %d\n", prms->tensor_data_type);
    VX_PRINT(VX_ZONE_INFO, "skip_mean_scale = %d\n", skip_mean_scale);
    
    if(prms->number_of_dimensions == 4)
    {
        input_batch = prms->output_dimensions[3];
        VX_PRINT(VX_ZONE_INFO, "input_batch = %d\n", input_batch);
    }    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC)
    {
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 1 */
            /* Input is NV12, Output is RGB (NHWC) */
            if(prms->tensor_data_type == 0x002)
            {   
                
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                /*#########################################################################*/
                if(skip_mean_scale)
                {
                    for(batch_iteration=0; batch_iteration < input_batch; batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {    
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

                            /*#######################################################*/
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f, U_s, V_f, V_s;
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int8x8x3_t RGB0, RGB1;

                            /* Load YUV values from both source pointers */
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = pos_x;

                                /* For 4 y, only 1 u and 1 v are needed */

                                Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                /* U_f will be used for first half of Y0 and Y1 */
                                int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                /* U_s will be used for second half of Y0 and Y1 */
                                u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                /* V_f will be used for first half of Y0 and Y1 */
                                int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                /* V_s will be used for second half of Y0 and Y1 */
                                v_ld = (int32_t) srskip_mean_scalec1RowPtr[src1ColIdxBytes+1+4];
                                V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                            }

                            /* Conversion from YUV to RGB */

                            /* r = y + (25802 * v >> 14) - 202 */
                            /* calc = (25802 * v >> 14) - 202 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                            /* Saturating narrow from int16 to uint8 to int8 */
                            RGB0.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                            /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                            tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                            vmulq_n_s32(V_f, 7669)),
                                                    14);
                            tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                            vmulq_n_s32(V_s, 7669)),
                                                    14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                            /* Saturating narrow from int16 to uint8 to int8 */
                            RGB0.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to int8 */
                            RGB0.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB0);
                                vst3_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB1);
                            }
                        }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = (int8_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = (int8_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = (int8_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = (int8_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = (int8_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = (int8_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = (int8_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = (int8_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = (int8_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = (int8_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = (int8_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = (int8_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B11;
                            }
                            }
                    }
                }
                
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for RGB interleaved data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H))));
                                RGB1.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H))));
                                RGB1.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H))));
                                RGB1.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H))));

                                /* Store RGB values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB0);
                                    vst3_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (int8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (int8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (int8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (int8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (int8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (int8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (int8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (int8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (int8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (int8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (int8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (int8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B11;
                            }
                        }
                    }
                
                }

                
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for RGB interleaved data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* Store RGB values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB0);
                                    vst3_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B11;
                            }
                        }
                    }
                }
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for RGB interleaved data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

    #if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H))));
                                RGB1.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H))));
                                RGB1.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H))));
                                RGB1.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H))));

                                /* Store RGB values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB0);
                                    vst3_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, RGB1);
                                }
                            }
    #else
                            pos_x = 0;
    #endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (uint8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (uint8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (uint8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (uint8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (uint8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (uint8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (uint8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (uint8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (uint8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (uint8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (uint8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (uint8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = B11;
                            }
                        }
                    }
                }
            }
        }
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR)
        {
            /* Case 2 */
            /* Input is NV12,  Output is BGR (NHWC) */
            if(prms->tensor_data_type == 0x002)
            {
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for BGR interleaved data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Store BGR values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR0);
                                    vst3_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = (int8_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = (int8_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = (int8_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = (int8_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = (int8_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = (int8_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = (int8_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = (int8_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = (int8_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = (int8_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = (int8_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = (int8_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R11;
                            }
                        }
                    }
                }
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for BGR interleaved data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H))));
                                BGR1.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H))));
                                BGR1.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H))));
                                BGR1.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H))));

                                /* Store BGR values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR0);
                                    vst3_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (int8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (int8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (int8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (int8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (int8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (int8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (int8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (int8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (int8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (int8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (int8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (int8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R11;
                            }
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for BGR interleaved data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* Store RGB values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR0);
                                    vst3_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R11;
                            }
                        }
                    }
                }
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for BGR interleaved data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H))));
                                BGR1.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H))));
                                BGR1.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H))));
                                BGR1.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H))));

                                /* Store BGR values */
                                {
                                    int dstColIdxBytes = (pos_x+0) * 3;
                                    vst3_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR0);
                                    vst3_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + dstColIdxBytes, BGR1);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int dstColIdxBytes  = pos_x * 3;
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (uint8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (uint8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (uint8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (uint8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (uint8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (uint8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (uint8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (uint8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (uint8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (uint8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (uint8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (uint8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+1] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+2] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+3] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+4] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + dstColIdxBytes+5] = R11;
                            }
                        }
                    }
                }
            }
        }
    }
    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW)
    {
        
        //VX_PRINT(VX_ZONE_INFO, "Entering NCHW NV12 PRE PROC\n");
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 3 */
            /* Input is NV12, Output is RGB (NCHW) */
            if(prms->tensor_data_type == 0x002)
            {   
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {

                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                RGB0.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                RGB1.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to int8 */
                                RGB0.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                RGB1.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                RGB0.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                RGB1.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Store RGB values */
                                {

                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;
    
                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = (int8_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = (int8_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = (int8_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = (int8_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = (int8_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = (int8_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = (int8_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = (int8_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = (int8_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = (int8_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = (int8_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = (int8_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B11;

                            }
                        }
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO, "Entering NON SKIPPING_MEAN_SCALE 002-NCHW NV12 PRE PRO\n");
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                                
                            }
                            else if(batch_iteration==1)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* dst for RGB data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H))));
                                RGB1.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H))));
                                RGB1.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                RGB0.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H))));
                                RGB1.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H))));

                                /* Store RGB values */
                                {
                                    //BEV Modifications 
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB1.val[2]);
                                    
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (int8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (int8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (int8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (int8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (int8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (int8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (int8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (int8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (int8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (int8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (int8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (int8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B11;
                            }
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                VX_PRINT(VX_ZONE_INFO, "Entering 003-NCHW NV12 PRE PROC\n");
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                { 
                    for(batch_iteration=0;batch_iteration<input_batch;batch_iteration++)
                    {  
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                            src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                            src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                            src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 */
                                RGB0.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                RGB1.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* Store RGB values */
                                {
                                    /* BEV Modifications */
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB1.val[2]);
                                    
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = CLIP_UNSIGNED(YUV2B(Y11,U,V));



                                /* BEV Modification */
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B11;
                                
                            }
                        }   
                    }
                         VX_PRINT(VX_ZONE_INFO, "Finished processing across all Batch's-SKIPPING_MEAN_SCALE 003-NCHW -RGB \n" );
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO, "Entering NON SKIPPING_MEAN_SCALE 003-NCHW NV12 PRE PROC\n");
                    for(batch_iteration=0;batch_iteration<input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* dst for RGB data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t RGB0, RGB1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H))));
                                RGB1.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H))));
                                RGB1.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                RGB0.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H))));
                                RGB1.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H))));

                                /* Store RGB values */
                                {
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, RGB1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (uint8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (uint8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (uint8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (uint8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (uint8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (uint8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (uint8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (uint8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (uint8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (uint8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (uint8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (uint8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = R11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = B11;
                            }
                        }
                    }
                }
            }
        }
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR)
        {
            /* Case 4 */
            /* Input is NV12, Output is BGR (NCHW) */
            if(prms->tensor_data_type == 0x002)
            {
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* dst for BGR data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[2] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[1] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to int8 */
                                BGR0.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                BGR1.val[0] = vreinterpret_s8_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Store BGR values */
                                {
                                    vst1_s8(dstRowPtr_0 + ((batch_offset * batch_iteration) + ch_offset * 0) + pos_x, BGR0.val[0]);
                                    vst1_s8(dstRowPtr_0 + ((batch_offset * batch_iteration) + ch_offset * 1) + pos_x, BGR0.val[1]);
                                    vst1_s8(dstRowPtr_0 + ((batch_offset * batch_iteration) + ch_offset * 2) + pos_x, BGR0.val[2]);
                                    vst1_s8(dstRowPtr_1 + ((batch_offset * batch_iteration) + ch_offset * 0) + pos_x, BGR1.val[0]);
                                    vst1_s8(dstRowPtr_1 + ((batch_offset * batch_iteration) + ch_offset * 1) + pos_x, BGR1.val[1]);
                                    vst1_s8(dstRowPtr_1 + ((batch_offset * batch_iteration) + ch_offset * 2) + pos_x, BGR1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = (int8_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = (int8_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = (int8_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = (int8_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = (int8_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = (int8_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = (int8_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = (int8_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = (int8_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = (int8_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = (int8_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = (int8_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 0) + pos_x + 0] = B00;
                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 2) + pos_x + 0] = R00;
                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 0) + pos_x + 1] = B01;
                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[((batch_offset * batch_iteration) + ch_offset * 2) + pos_x + 1] = R01;

                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 0) + pos_x + 0] = B10;
                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 2) + pos_x + 0] = R10;
                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 0) + pos_x + 1] = B11;
                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[((batch_offset * batch_iteration) + ch_offset * 2) + pos_x + 1] = R11;
                            }
                        }
                    }
                }
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* dst for BGR data */
                            int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                int8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H))));
                                BGR1.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H))));
                                BGR1.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 to int8x8 */
                                BGR0.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H))));
                                BGR1.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H))));

                                /* Store BGR values */
                                {
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                    vst1_s8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                    vst1_s8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                int8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                int8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (int8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (int8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (int8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (int8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (int8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (int8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (int8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (int8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (int8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (int8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (int8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (int8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R11;
                            }
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* dst for BGR data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[2] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[1] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 */
                                BGR0.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid));
                                BGR1.val[0] = vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid));

                                /* Store RGB values */
                                {
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00 = CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01 = CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10 = CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11 = CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00 = CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01 = CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10 = CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11 = CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00 = CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01 = CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10 = CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11 = CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R11;
                            }
                        }
                    }
                }
                else
                {
                    for(batch_iteration=0;batch_iteration < input_batch;batch_iteration++)
                    {
                        for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                        {
                            uint8_t* src0RowPtr_0 =NULL;
                            uint8_t* src0RowPtr_1 =NULL;
                            uint8_t* src1RowPtr =NULL;

                            if(batch_iteration==0)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y); 
                            }
                            else if(batch_iteration==1)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img2_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img2_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img2_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }
                            /* ----------------Modification to get src0 for Image3 for Ydata---*/
                            else if(batch_iteration==2)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img3_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img3_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img3_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image4 for Ydata---*/
                            else if(batch_iteration==3)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img4_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img4_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img4_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image5 for Ydata---*/
                            else if(batch_iteration==4)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img5_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img5_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img5_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }

                            /* ----------------Modification to get src0 for Image6 for Ydata---*/
                            else if(batch_iteration==5)
                            {
                                src0RowPtr_0 = (uint8_t *)prms->in_img6_target_ptr[0] + (pos_y * prms->in_stride_y);
                                src0RowPtr_1 = (uint8_t *)prms->in_img6_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);
                                src1RowPtr = (uint8_t *)prms->in_img6_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);
                            }


                            /* dst for BGR data */
                            uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                            uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                            for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                                uint8x8_t Y0, Y1;
                                int32x4_t U_f, U_s, V_f, V_s;
                                int16x4_t tmp_mid_f, tmp_mid_s;
                                int16x8_t tmp_mid;
                                uint8x8x3_t BGR0, BGR1;

                                /* Load YUV values from both source pointers */
                                {
                                    int src0ColIdxBytes = pos_x;
                                    int src1ColIdxBytes = pos_x;

                                    /* For 4 y, only 1 u and 1 v are needed */

                                    Y0 = vld1_u8(src0RowPtr_0 + src0ColIdxBytes);
                                    Y1 = vld1_u8(src0RowPtr_1 + src0ColIdxBytes);

                                    /* U_f will be used for first half of Y0 and Y1 */
                                    int32_t u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+0];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 0);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+2];
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 2);
                                    U_f = vld1q_lane_s32(&u_ld, U_f, 3);

                                    /* U_s will be used for second half of Y0 and Y1 */
                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+4];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 0);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 1);

                                    u_ld = (int32_t) src1RowPtr[src1ColIdxBytes+6];
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 2);
                                    U_s = vld1q_lane_s32(&u_ld, U_s, 3);

                                    /* V_f will be used for first half of Y0 and Y1 */
                                    int32_t v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+0];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 0);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+2];
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 2);
                                    V_f = vld1q_lane_s32(&v_ld, V_f, 3);

                                    /* V_s will be used for second half of Y0 and Y1 */
                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+4];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 0);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 1);

                                    v_ld = (int32_t) src1RowPtr[src1ColIdxBytes+1+6];
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 2);
                                    V_s = vld1q_lane_s32(&v_ld, V_s, 3);
                                }

                                /* Conversion from YUV to RGB */

                                /* r = y + (25802 * v >> 14) - 202 */
                                /* calc = (25802 * v >> 14) - 202 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(V_f, 25802), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(V_s, 25802), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(202));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t R0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t R1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t R0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R0)));
                                float32x4_t R0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R0)));
                                float32x4_t R1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R1)));
                                float32x4_t R1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R1)));

                                /* R = (R - mean) * scale */
                                R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H))));
                                BGR1.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H))));

                                /* g = y - ((3069 * u + 7669 * v)>>14) + 84 */
                                /* calc_V = 84 - ((3069 * u + 7669 * v)>>14) */
                                tmp_mid_f = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_f, 3069),
                                                                vmulq_n_s32(V_f, 7669)),
                                                        14);
                                tmp_mid_s = vshrn_n_s32(vaddq_s32(vmulq_n_s32(U_s, 3069),
                                                                vmulq_n_s32(V_s, 7669)),
                                                        14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(vdupq_n_s16(84), tmp_mid);

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t G0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t G1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t G0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G0)));
                                float32x4_t G0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G0)));
                                float32x4_t G1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G1)));
                                float32x4_t G1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G1)));

                                /* G = (G - mean) * scale */
                                G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H))));
                                BGR1.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H))));

                                /* b = y + (30402 * u >> 14) - 237 */
                                /* calc = (30402 * u >> 14) - 237 */
                                tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                                tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                                tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                                tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                                /* Saturating narrow from int16 to uint8 to uint16 */
                                uint16x8_t B0 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                                uint16x8_t B1 = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                                /* Moving to float for mean and scale opeartions
                                uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                                float32x4_t B0_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B0)));
                                float32x4_t B0_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B0)));
                                float32x4_t B1_F32H = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B1)));
                                float32x4_t B1_F32L = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B1)));

                                /* B = (B - mean) * scale */
                                B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                                B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                                /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 to uint8x8 */
                                BGR0.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H))));
                                BGR1.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H))));

                                /* Store BGR values */
                                {
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                    vst1_u8(dstRowPtr_0 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                    vst1_u8(dstRowPtr_1 + (batch_offset * batch_iteration) + (ch_offset * 2) + pos_x, BGR1.val[2]);
                                }
                            }
#else
                            pos_x = 0;
#endif
                            /* Process remaining pixels */
                            for(; pos_x < prms->input_width; pos_x+=2)
                            {
                                int src0ColIdxBytes = pos_x;
                                int src1ColIdxBytes = 2 * (pos_x/2);
                                uint8_t R00, R01, R10, R11, G00, G01, G10, G11;
                                uint8_t B00, B01, B10, B11;
                                float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                                float B00_F, B01_F, B10_F, B11_F;

                                uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                                uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                                uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                                uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                                uint8_t U = src1RowPtr[src1ColIdxBytes];
                                uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                                R00_F = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                                R01_F = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                                R10_F = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                                R11_F = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                                G00_F = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                                G01_F = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                                G10_F = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                                G11_F = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                                B00_F = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                                B01_F = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                                B10_F = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                                B11_F = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                                R00 = (uint8_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                                R01 = (uint8_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                                R10 = (uint8_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                                R11 = (uint8_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                                G00 = (uint8_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                                G01 = (uint8_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                                G10 = (uint8_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                                G11 = (uint8_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                                B00 = (uint8_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                                B01 = (uint8_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                                B10 = (uint8_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                                B11 = (uint8_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R00;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G01;
                                dstRowPtr_0[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R01;

                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 0] = B10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 0] = G10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 0] = R10;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 0) + pos_x + 1] = B11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 1) + pos_x + 1] = G11;
                                dstRowPtr_1[(batch_offset * batch_iteration) + (ch_offset * 2) + pos_x + 1] = R11;
                            }
                        }
                    }
                }
            }
        }
    }
    

}
