/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#include <VX/vx.h>
#include <TI/tivx_img_proc.h>
#include <tivx_utils_checksum.h>
#include <vx_tiovx_kernels_utils.h>
#include <vx_dl_pre_proc_armv8_utils.h>
#include <assert.h>

#define NUM_DIMENSIONS 3
#define APP_KERNELS_MAX_TENSOR_DIMS 4

/*
    IN_RGB_NHWC_OUT_RGB_INT8, IN_RGB_NHWC_OUT_RGB_UINT8, IN_RGB_NHWC_OUT_RGB_INT16, IN_RGB_NHWC_OUT_RGB_UINT16, IN_RGB_NHWC_OUT_RGB_INT32, IN_RGB_NHWC_OUT_RGB_UINT32, IN_RGB_NHWC_OUT_RGB_FLOAT32,
    IN_RGB_NHWC_OUT_BGR_INT8, IN_RGB_NHWC_OUT_BGR_UINT8, IN_RGB_NHWC_OUT_BGR_INT16, IN_RGB_NHWC_OUT_BGR_UINT16, IN_RGB_NHWC_OUT_BGR_INT32, IN_RGB_NHWC_OUT_BGR_UINT32, IN_RGB_NHWC_OUT_BGR_FLOAT32,
    IN_RGB_NCHW_OUT_RGB_INT8, IN_RGB_NCHW_OUT_RGB_UINT8, IN_RGB_NCHW_OUT_RGB_INT16, IN_RGB_NCHW_OUT_RGB_UINT16, IN_RGB_NCHW_OUT_RGB_INT32, IN_RGB_NCHW_OUT_RGB_UINT32, IN_RGB_NCHW_OUT_RGB_FLOAT32,
    IN_RGB_NCHW_OUT_BGR_INT8, IN_RGB_NCHW_OUT_BGR_UINT8, IN_RGB_NCHW_OUT_BGR_INT16, IN_RGB_NCHW_OUT_BGR_UINT16, IN_RGB_NCHW_OUT_BGR_INT32, IN_RGB_NCHW_OUT_BGR_UINT32, IN_RGB_NCHW_OUT_BGR_FLOAT32,
    IN_NV12_NHWC_OUT_RGB_INT8, IN_NV12_NHWC_OUT_RGB_UINT8, IN_NV12_NHWC_OUT_RGB_INT16, IN_NV12_NHWC_OUT_RGB_UINT16, IN_NV12_NHWC_OUT_RGB_INT32, IN_NV12_NHWC_OUT_RGB_UINT32, IN_NV12_NHWC_OUT_RGB_FLOAT32,
    IN_NV12_NHWC_OUT_BGR_INT8, IN_NV12_NHWC_OUT_BGR_UINT8, IN_NV12_NHWC_OUT_BGR_INT16, IN_NV12_NHWC_OUT_BGR_UINT16, IN_NV12_NHWC_OUT_BGR_INT32, IN_NV12_NHWC_OUT_BGR_UINT32, IN_NV12_NHWC_OUT_BGR_FLOAT32,
    IN_NV12_NCHW_OUT_RGB_INT8, IN_NV12_NCHW_OUT_RGB_UINT8, IN_NV12_NCHW_OUT_RGB_INT16, IN_NV12_NCHW_OUT_RGB_UINT16, IN_NV12_NCHW_OUT_RGB_INT32, IN_NV12_NCHW_OUT_RGB_UINT32, IN_NV12_NCHW_OUT_RGB_FLOAT32,
    IN_NV12_NCHW_OUT_BGR_INT8, IN_NV12_NCHW_OUT_BGR_UINT8, IN_NV12_NCHW_OUT_BGR_INT16, IN_NV12_NCHW_OUT_BGR_UINT16, IN_NV12_NCHW_OUT_BGR_INT32, IN_NV12_NCHW_OUT_BGR_UINT32, IN_NV12_NCHW_OUT_BGR_FLOAT32,    
*/
static uint32_t output_tensor_checksums_expected[56] = {
    (uint32_t) 0x7ebaaa4a, (uint32_t) 0x7ebaaa4a, (uint32_t) 0x782e8554, (uint32_t) 0x782e8554, (uint32_t) 0x0737f89e, (uint32_t) 0x0737f89e, (uint32_t) 0xc4eafffd,
    (uint32_t) 0xb864710a, (uint32_t) 0xb864710a, (uint32_t) 0x782e8554, (uint32_t) 0x782e8554, (uint32_t) 0x0737f89e, (uint32_t) 0x0737f89e, (uint32_t) 0xc4eafffd,
    (uint32_t) 0x0d2c31ba, (uint32_t) 0x0d2c31ba, (uint32_t) 0x52cb3f08, (uint32_t) 0x52cb3f08, (uint32_t) 0x07378e77, (uint32_t) 0x07378e77, (uint32_t) 0x5355fe6c,
    (uint32_t) 0x0c2d30bb, (uint32_t) 0x0c2d30bb, (uint32_t) 0x52cb3d0a, (uint32_t) 0x52cb3d0a, (uint32_t) 0x07378c79, (uint32_t) 0x07378c79, (uint32_t) 0xcc57fe6c,
    (uint32_t) 0x7afe70b1, (uint32_t) 0x7afe70b1, (uint32_t) 0x94d78dd3, (uint32_t) 0x0c6c9427, (uint32_t) 0xac42e90d, (uint32_t) 0xad8b6820, (uint32_t) 0xa7ef2074,
    (uint32_t) 0x2662c4e6, (uint32_t) 0x2662c4e6, (uint32_t) 0x94d78dd3, (uint32_t) 0x0c6c9427, (uint32_t) 0xac42e90d, (uint32_t) 0xad8b6820, (uint32_t) 0xa7ef2074,
    (uint32_t) 0x36b0e718, (uint32_t) 0x36b0e718, (uint32_t) 0xf3e7cabb, (uint32_t) 0xf8a02eab, (uint32_t) 0x78e7238b, (uint32_t) 0x7a2f8ccb, (uint32_t) 0x7b703383,
    (uint32_t) 0xff78f510, (uint32_t) 0xff78f510, (uint32_t) 0x0c8ae2e1, (uint32_t) 0x19564f5b, (uint32_t) 0x78e75455, (uint32_t) 0x7a2fce31, (uint32_t) 0xb2903383,
};

static vx_status validate_out_tensor_cksm(vx_tensor out_tensor, vx_size *tensor_sizes, vx_int32 data_type, int outTensorIdx)
{
    vx_status status = VX_SUCCESS;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_size start[APP_KERNELS_MAX_TENSOR_DIMS];
    vx_size output_strides[APP_KERNELS_MAX_TENSOR_DIMS];

    for(vx_uint32 i = 0; i < APP_KERNELS_MAX_TENSOR_DIMS; i++)
    {
        output_strides[i] = 0;
    }

    if((data_type == VX_TYPE_INT8) ||
        (data_type == VX_TYPE_UINT8))
    {
        output_strides[0] = sizeof(vx_int8);
    }
    else
    if((data_type == VX_TYPE_INT16) ||
        (data_type == VX_TYPE_UINT16))
    {
        output_strides[0] = sizeof(vx_int16);
    }
    else
    if((data_type == VX_TYPE_INT32) ||
        (data_type == VX_TYPE_UINT32) || 
        (data_type == VX_TYPE_FLOAT32))
    {
        output_strides[0] = sizeof(vx_float32);
    }

    start[0] = start[1] = start[2] = start[3] = 0;
    output_strides[1] = tensor_sizes[0] * output_strides[0];
    output_strides[2] = tensor_sizes[1] * output_strides[1];

    checksum_actual = tivx_utils_tensor_checksum(out_tensor, NUM_DIMENSIONS, start, tensor_sizes, output_strides);
    checksum_expected = output_tensor_checksums_expected[outTensorIdx];

    if(checksum_actual != checksum_expected)
    {
        APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
        status = VX_SUCCESS;
    }

    return status;
}

vx_status app_kernels_dl_pre_proc_test(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;
    vx_graph graph;
    vx_node node;
    char input_filename[APP_KERNELS_APP_MAX_FILE_PATH], output_filename[APP_KERNELS_APP_MAX_FILE_PATH];
    vx_int32 width, height;
    vx_image in_img = NULL;
    vx_tensor out_tensor;
    tivxDLPreProcArmv8Params *dl_pre_proc_params, params;
    vx_user_data_object config;
    vx_map_id map_id;
    int outTensorIdx=0;
    vx_size tensor_sizes[NUM_DIMENSIONS];
    vx_int32 srcColorFormat[]   = {VX_DF_IMAGE_RGB, VX_DF_IMAGE_NV12};
    vx_int32 channelOrder[]     = {DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC, DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW};
    vx_int32 tensorFormat[]     = {DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB, DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR};
    vx_int32 tensorDataType[]   = {VX_TYPE_INT8, VX_TYPE_UINT8, VX_TYPE_INT16, VX_TYPE_UINT16, VX_TYPE_INT32, VX_TYPE_UINT32, VX_TYPE_FLOAT32};

    vx_context context = vxCreateContext();
    status = vxGetStatus((vx_reference) context);

    if(status == VX_SUCCESS)
    {
        tivxImgProcLoadKernels(context);
    }

    for(vx_uint32 i = 0; i < NUM_DIMENSIONS; i++)
    {
        tensor_sizes[i] = 0;
    }

    width = 640;
    height = 480;

    params.skip_flag = 0;

    params.scale[0] = 1.0;
    params.scale[1] = 1.0;
    params.scale[2] = 1.0;

    params.mean[0] = 0.0;
    params.mean[1] = 0.0;
    params.mean[2] = 0.0;

    params.crop[0] = 0;
    params.crop[1] = 0;
    params.crop[2] = 0;
    params.crop[3] = 0;

    for(int scf=0; scf<2; scf++)
    {
        if(status == VX_SUCCESS)
        {
            in_img = vxCreateImage(context, width, height, srcColorFormat[scf]);
            status = vxGetStatus((vx_reference)in_img);
        }

        sprintf(input_filename, "%s/raw_images/kernels_test/input_dl_pre_proc%d", APP_KERNELS_DATA_PATH, scf);
        appKernelsReadImage(input_filename, in_img);

        for(int co=0; co<2; co++)
        {
            params.channel_order = channelOrder[co];
            if(channelOrder[co] == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC)
            {
                tensor_sizes[0] = NUM_DIMENSIONS;
                tensor_sizes[1] = width;
                tensor_sizes[2] = height;
            }
            else if(channelOrder[co] == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW)
            {
                tensor_sizes[0] = width;
                tensor_sizes[1] = height;
                tensor_sizes[2] = NUM_DIMENSIONS;
            }

            for(int tf=0; tf<2; tf++)
            {
                params.tensor_format = tensorFormat[tf];

                for(int tdt=0; tdt<7; tdt++)
                {
                    out_tensor = vxCreateTensor(context, NUM_DIMENSIONS, tensor_sizes, tensorDataType[tdt], 0);

                    if(status == VX_SUCCESS)
                    {
                        graph = vxCreateGraph(context);
                        status = vxGetStatus((vx_reference)graph);
                    }

                    if(status == VX_SUCCESS)
                    {
                        config = vxCreateUserDataObject(context, "tivxDLPreProcArmv8Params", sizeof(tivxDLPreProcArmv8Params), NULL );
                        status = vxGetStatus((vx_reference) config);
                    }

                    if (VX_SUCCESS == status)
                    {
                        vxSetReferenceName((vx_reference)config, "dl_pre_proc_config");
                        vxMapUserDataObject(config, 0, sizeof(tivxDLPreProcArmv8Params), &map_id,
                                        (void **)&dl_pre_proc_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);
                        memcpy(dl_pre_proc_params, &params, sizeof(tivxDLPreProcArmv8Params));
                        vxUnmapUserDataObject(config, map_id);
                    }

                    if(status == VX_SUCCESS)
                    {
                        node = tivxDLPreProcArmv8Node(graph, config, in_img, out_tensor);
                        status = vxGetStatus((vx_reference) node);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxVerifyGraph(graph);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxProcessGraph(graph);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = validate_out_tensor_cksm(out_tensor, tensor_sizes, tensorDataType[tdt], outTensorIdx);
                        outTensorIdx++;
                    }

                    sprintf(output_filename, "%s/output/output_dl_pre_proc%02d", APP_KERNELS_DATA_PATH, (outTensorIdx%100));
                    appKernelsWriteTensor(output_filename, out_tensor);

                    if(status == VX_SUCCESS)
                    {
                        status = vxReleaseTensor(&out_tensor);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxReleaseNode(&node);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxReleaseUserDataObject(&config);
                    }

                    if(status == VX_SUCCESS)
                    {
                        status = vxReleaseGraph(&graph);
                    }

                    assert(out_tensor == 0);
                    assert(node == 0);
                    assert(graph == 0);

                }
            }
        }
        if(status == VX_SUCCESS)
        {
            status = vxReleaseImage(&in_img);
        }
        assert(in_img == 0);
    }

    if(status == VX_SUCCESS)
    {
        tivxImgProcUnLoadKernels(context);
        status = vxReleaseContext(&context);
    }

    return status;
}
