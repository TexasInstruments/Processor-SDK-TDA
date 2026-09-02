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
#include <assert.h>

#define TOTAL_COLOR_CONVERSIONS 8

/*
    NV12_op0_plane0,  NV12_op0_plane1,  RGB_op1_plane0,  RGB_op2_plane0,
    I420_op3_plane0,  I420_op3_plane1,  I420_op3_plane2, I420_op4_plane0,
    I420_op4_plane1,  I420_op4_plane2,  NV12_op5_plane0, NV12_op5_plane1,
    NV12_op6_plane0,  NV12_op6_plane1,  NV12_op7_plane0, NV12_op7_plane1
*/
static uint32_t output_checksums_expected[16] = {
    (uint32_t) 0x6b37c0a9, (uint32_t) 0x53ea2624, (uint32_t) 0x4371ad5f, (uint32_t) 0x4371ad5f,
    (uint32_t) 0xc1dbfe15, (uint32_t) 0x9906ac15, (uint32_t) 0x7532f31a, (uint32_t) 0xc1dbfe15,
    (uint32_t) 0x9906ac15, (uint32_t) 0x7532f31a, (uint32_t) 0xc1dbfe15, (uint32_t) 0x156aa476,
    (uint32_t) 0xc1dbfe15, (uint32_t) 0x156aa476, (uint32_t) 0xc1dbfe15, (uint32_t) 0x156aa476
};

static vx_status validate_out_image_cksm(vx_image out_img, vx_int32 width, vx_int32 height, vx_int32 outColorFormat, int outColorFormatIdx)
{
    vx_status cksm_status = VX_SUCCESS;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    int outChecksumIdx = outColorFormatIdx;
    vx_rectangle_t rect;

    if(outColorFormatIdx == 1)
    {
        outChecksumIdx += 1;
    }
    else if(outColorFormatIdx == 2)
    {
        outChecksumIdx += 1;
    }
    else if(outColorFormatIdx == 3)
    {
        outChecksumIdx += 1;
    }
    else if(outColorFormatIdx == 4)
    {
        outChecksumIdx += 3;
    }
    else if(outColorFormatIdx == 5)
    {
        outChecksumIdx += 5;
    }
    else if(outColorFormatIdx == 6)
    {
        outChecksumIdx += 6;
    }
    else if(outColorFormatIdx == 7)
    {
        outChecksumIdx += 7;
    }

    if(outColorFormat == VX_DF_IMAGE_IYUV)
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(out_img, 0, rect);
        checksum_expected = output_checksums_expected[outChecksumIdx];

        if(checksum_actual != checksum_expected)
        {
            APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
            cksm_status = VX_SUCCESS;
        }

        if(cksm_status == VX_SUCCESS)
        {
            rect.end_x = width/2;
            rect.end_y = height/2;

            checksum_actual = tivx_utils_simple_image_checksum(out_img, 1, rect);
            checksum_expected = output_checksums_expected[outChecksumIdx+1];

            if(checksum_actual != checksum_expected)
            {
                APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
                cksm_status = VX_SUCCESS;
            }
        }

        if(cksm_status == VX_SUCCESS)
        {
            rect.end_x = width/2;
            rect.end_y = height/2;

            checksum_actual = tivx_utils_simple_image_checksum(out_img, 2, rect);
            checksum_expected = output_checksums_expected[outChecksumIdx+2];

            if(checksum_actual != checksum_expected)
            {
                APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
                cksm_status = VX_SUCCESS;
            }
        }
    }
    else if(outColorFormat == VX_DF_IMAGE_NV12)
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(out_img, 0, rect);
        checksum_expected = output_checksums_expected[outChecksumIdx];

        if(checksum_actual != checksum_expected)
        {
            APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
            cksm_status = VX_SUCCESS;
        }

        if(cksm_status == VX_SUCCESS)
        {
            rect.end_x = width;
            rect.end_y = height/2;

            checksum_actual = tivx_utils_simple_image_checksum(out_img, 1, rect);
            checksum_expected = output_checksums_expected[outChecksumIdx+1];

            if(checksum_actual != checksum_expected)
            {
                APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
                cksm_status = VX_SUCCESS;
            }
        }
    }
    else if(outColorFormat == VX_DF_IMAGE_RGB)
    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_actual = tivx_utils_simple_image_checksum(out_img, 0, rect);
        checksum_expected = output_checksums_expected[outChecksumIdx];

        if(checksum_actual != checksum_expected)
        {
            APP_KERNELS_APP_ERROR("checksum_actual = 0x%08x, checksum_expected = 0x%08x \n", checksum_actual, checksum_expected);
            cksm_status = VX_SUCCESS;
        }
    }

    return cksm_status;
}

vx_status app_kernels_dl_color_convert_test(vx_int32 argc, vx_char* argv[])
{
    vx_status status = VX_SUCCESS;
    vx_graph graph;
    vx_node node;
    char input_filename[APP_KERNELS_APP_MAX_FILE_PATH], output_filename[APP_KERNELS_APP_MAX_FILE_PATH];
    vx_int32 width, height;
    vx_int32 srcColorFormat[TOTAL_COLOR_CONVERSIONS] = {VX_DF_IMAGE_RGB, VX_DF_IMAGE_NV12, VX_DF_IMAGE_NV21, VX_DF_IMAGE_NV12, VX_DF_IMAGE_NV21, VX_DF_IMAGE_IYUV, VX_DF_IMAGE_UYVY, VX_DF_IMAGE_YUYV};
    vx_int32 dstColorFormat[TOTAL_COLOR_CONVERSIONS] = {VX_DF_IMAGE_NV12, VX_DF_IMAGE_RGB, VX_DF_IMAGE_RGB, VX_DF_IMAGE_IYUV, VX_DF_IMAGE_IYUV, VX_DF_IMAGE_NV12, VX_DF_IMAGE_NV12, VX_DF_IMAGE_NV12};

    vx_context context = vxCreateContext();
    status = vxGetStatus((vx_reference) context);

    if(status == VX_SUCCESS)
    {
        tivxImgProcLoadKernels(context);
    }

    width = 1920;
    height = 1080;

    for(int i=0; i<TOTAL_COLOR_CONVERSIONS; i++)
    {
        vx_image out_img, in_img;

        out_img = NULL;
        in_img = NULL;

        if(status == VX_SUCCESS)
        {
            in_img = vxCreateImage(context, width, height, srcColorFormat[i]);
            status = vxGetStatus((vx_reference)in_img);
        }

        if(status == VX_SUCCESS)
        {
            out_img  = vxCreateImage(context, width, height, dstColorFormat[i]);
            status = vxGetStatus((vx_reference)out_img);
        }

        if(status == VX_SUCCESS)
        {
            graph = vxCreateGraph(context);
            status = vxGetStatus((vx_reference)graph);
        }

        sprintf(input_filename, "%s/raw_images/kernels_test/input_dl_color_convert%d", APP_KERNELS_DATA_PATH, i);
        appKernelsReadImage(input_filename, in_img);

        if(status == VX_SUCCESS)
        {
            node = tivxDLColorConvertArmv8Node(graph, in_img, out_img);
            status = vxGetStatus((vx_reference)node);
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

        sprintf(output_filename, "%s/output/output_dl_color_convert%d", APP_KERNELS_DATA_PATH, i);
        appKernelsWriteImage(output_filename, out_img);

        if(status == VX_SUCCESS)
        {
            status = validate_out_image_cksm(out_img, width, height, dstColorFormat[i], i);
        }

        if(status == VX_SUCCESS)
        {
            status = vxReleaseImage(&in_img);
        }

        if(status == VX_SUCCESS)
        {
            status = vxReleaseImage(&out_img);
        }

        if(status == VX_SUCCESS)
        {
            status = vxReleaseNode(&node);
        }

        if(status == VX_SUCCESS)
        {
            status = vxReleaseGraph(&graph);
        }

        assert(in_img == 0);
        assert(out_img == 0);
        assert(node == 0);
        assert(graph == 0);
    }

    if(status == VX_SUCCESS)
    {
        tivxImgProcUnLoadKernels(context);
        status = vxReleaseContext(&context);
    }

    return status;
}
