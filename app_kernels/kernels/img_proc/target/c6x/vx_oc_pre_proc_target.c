/*
*
* Copyright (c) 2020-2026 Texas Instruments Incorporated
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

#include <tivx_oc_pre_proc_host.h>

#include <utils/mem/include/app_mem.h>

static tivx_target_kernel vx_oc_pre_proc_target_kernel = NULL;

#define CLIP_255(x) ((x)<0?0:(((x)>255)?255:(x)))
static vx_size getTensorDataType(vx_int32 tidl_type);

static vx_status VX_CALLBACK tivxKernelOCPreProcCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    tivxOCPreProcParams * prms = NULL;

    prms = tivxMemAlloc(sizeof(tivxOCPreProcParams), TIVX_MEM_EXTERNAL);

    tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxOCPreProcParams));

    return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelOCPreProcDelete(
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
        tivxOCPreProcParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxOCPreProcParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelOCPreProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;
    tivxOCPreProcParams *prms = NULL;
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
            (sizeof(tivxOCPreProcParams) != size))
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

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_OC_PRE_PROC_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_OC_PRE_PROC_INPUT_IMAGE_IDX];
        in_img_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OC_PRE_PROC_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        tivxOCPreProcParams *params = (tivxOCPreProcParams *)config_target_ptr;
        sTIDL_IOBufDesc_t *ioBufDesc = &params->ioBufDesc;

        uint32_t in_width  = in_img_desc->imagepatch_addr[0].dim_x;
        uint32_t in_height = in_img_desc->imagepatch_addr[0].dim_y;
        uint32_t in_stride = in_img_desc->imagepatch_addr[0].stride_y;

        vx_int32 dl_width  = ioBufDesc->inWidth[0];
        vx_int32 dl_height = ioBufDesc->inHeight[0];

        vx_int32 out_row_stride  = out_tensor_desc->stride[1];
        vx_int32 out_ch_stride   = out_tensor_desc->stride[2];

        if((in_width >= dl_width) && (in_height >= dl_height))
        {
            vx_int32 start_x = (in_width - dl_width) / 2;
            vx_int32 start_y = (in_height - dl_height) / 2;

            vx_uint8 *pY = (vx_uint8 *)in_img_target_ptr[0] + (start_y * in_stride) + start_x;
            vx_uint8 *pC = (vx_uint8 *)in_img_target_ptr[1] + ((start_y >> 1) * in_stride) + ((start_x >> 1)<<1);

            vx_int32 start_offset;

            vx_size data_type = getTensorDataType(ioBufDesc->inElementType[0]);

            if((data_type == VX_TYPE_INT8) ||
                (data_type == VX_TYPE_UINT8))
            {
                vx_uint8 *pR = NULL;
                vx_uint8 *pG = NULL;
                vx_uint8 *pB = NULL;

                if(ioBufDesc->inDataFormat[0] == 1) /* RGB */
                {
                    start_offset = (0 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pR = (vx_uint8 *)out_tensor_target_ptr + start_offset;

                    start_offset = (1 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pG = (vx_uint8 *)out_tensor_target_ptr + start_offset;

                    start_offset = (2 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pB = (vx_uint8 *)out_tensor_target_ptr + start_offset;
                }
                else /* BGR */
                {
                    start_offset = (0 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pB = (vx_uint8 *)out_tensor_target_ptr + start_offset;

                    start_offset = (1 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pG = (vx_uint8 *)out_tensor_target_ptr + start_offset;

                    start_offset = (2 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pR = (vx_uint8 *)out_tensor_target_ptr + start_offset;
                }

                vx_int32 col, row;
                for(row = 0; row < dl_height; row++)
                {
                    for(col = 0; col < dl_width; col++)
                    {
                        vx_uint8 Y  = pY[(row * in_stride) + col];
                        vx_uint8 Cb = pC[((row>>1) * in_stride) + ((col>>1)<<1) + 1];
                        vx_uint8 Cr = pC[((row>>1) * in_stride) + ((col>>1)<<1)];

                        vx_int32 Y_  = Y - 16;
                        vx_int32 Cb_ = Cb - 128;
                        vx_int32 Cr_ = Cr - 128;

                        vx_int32 R = ((298 * Y_) + (409 * Cr_) + 128) >> 8;
                        vx_int32 G = ((298 * Y_) - (100 * Cb_) - (208 * Cr_) + 128) >> 8;
                        vx_int32 B = ((298 * Y_) + (516 * Cb_) + 128) >> 8;

                        pR[col] = (vx_uint8)CLIP_255(R);
                        pG[col] = (vx_uint8)CLIP_255(G);
                        pB[col] = (vx_uint8)CLIP_255(B);
                    }
                    pR += out_row_stride;
                    pG += out_row_stride;
                    pB += out_row_stride;
                }
            }
            if((data_type == VX_TYPE_INT16) ||
                (data_type == VX_TYPE_UINT16))
            {
                vx_uint16 *pR = NULL;
                vx_uint16 *pG = NULL;
                vx_uint16 *pB = NULL;

                if(ioBufDesc->inDataFormat[0] == 1) /* RGB */
                {
                    start_offset = (0 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

                    start_offset = (1 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pG = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

                    start_offset = (2 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pB = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);
                }
                else /* BGR */
                {
                    start_offset = (0 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pB = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

                    start_offset = (1 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pG = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);

                    start_offset = (2 * out_ch_stride) + (ioBufDesc->inPadT[0] * out_row_stride) + ioBufDesc->inPadL[0];
                    pR = (vx_uint16 *)((vx_uint8 *)out_tensor_target_ptr + start_offset);
                }

                vx_int32 col, row;
                for(row = 0; row < dl_height; row++)
                {
                    for(col = 0; col < dl_width; col++)
                    {
                        vx_uint8 Y  = pY[(row * in_stride) + col];
                        vx_uint8 Cb = pC[((row>>1) * in_stride) + ((col>>1)<<1) + 1];
                        vx_uint8 Cr = pC[((row>>1) * in_stride) + ((col>>1)<<1)];

                        vx_int32 Y_  = Y - 16;
                        vx_int32 Cb_ = Cb - 128;
                        vx_int32 Cr_ = Cr - 128;

                        vx_int32 R = ((298 * Y_) + (409 * Cr_) + 128) >> 8;
                        vx_int32 G = ((298 * Y_) - (100 * Cb_) - (208 * Cr_) + 128) >> 8;
                        vx_int32 B = ((298 * Y_) + (516 * Cb_) + 128) >> 8;

                        pR[col] = CLIP_255(R);
                        pG[col] = CLIP_255(G);
                        pB[col] = CLIP_255(B);
                    }
                    pR += (out_row_stride / sizeof(vx_uint16));
                    pG += (out_row_stride / sizeof(vx_uint16));
                    pB += (out_row_stride / sizeof(vx_uint16));
                }
            }
        }

        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    }

    return (status);
}
void tivxAddTargetKernelOCPreProc()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_oc_pre_proc_target_kernel = tivxAddTargetKernelByName
                                        (
                                            TIVX_KERNEL_OC_PRE_PROC_NAME,
                                            target_name,
                                            tivxKernelOCPreProcProcess,
                                            tivxKernelOCPreProcCreate,
                                            tivxKernelOCPreProcDelete,
                                            NULL,
                                            NULL
                                        );
    }
}

void tivxRemoveTargetKernelOCPreProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_oc_pre_proc_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_oc_pre_proc_target_kernel = NULL;
    }
}

static vx_size getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }

    return openvx_type;
}
