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

#include <edgeai_dl_pre_proc_armv8_utils.h>
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

void dlPreProcess_RGB_image
(
    dlPreProcessImageParams *prms
)
{
    uint32_t skip_mean_scale = 0;
    uint32_t pos_x = 0, pos_y = 0;
    uint32_t ch_offset = prms->channel_pitch;

    if(prms->mean[0]==0 && prms->mean[1]==0 && prms->mean[2]==0 &&
       prms->scale[0]==1 && prms->scale[1]==1 && prms->scale[2]==1)
    {
        skip_mean_scale = 1;
    }

    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC)
    {
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 1 */
            /* Input is RGB, Output is RGB (NHWC) */
            if(prms->tensor_data_type == 0x002)
            /* prms->tensor_data_type == VX_TYPE_INT8
               VX_TYPE_INT8            = 0x002 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;
                            int8x16x3_t RGB_op;

                            RGB_ip = vld3q_u8(pIn+offset);

                            RGB_op.val[0] = vreinterpretq_s8_u8(RGB_ip.val[0]);
                            RGB_op.val[1] = vreinterpretq_s8_u8(RGB_ip.val[1]);
                            RGB_op.val[2] = vreinterpretq_s8_u8(RGB_ip.val[2]);

                            vst3q_s8(pOut+offset, RGB_op);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int8_t)R;
                            pOut[offset + 1] = (int8_t)G;
                            pOut[offset + 2] = (int8_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int8x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 to int8x8 */
                            RGB_op.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f))));
                            RGB_op.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f))));
                            RGB_op.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f))));

                            vst3_s8(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (int8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            /* prms->tensor_data_type == VX_TYPE_UINT8
               VX_TYPE_UINT8            = 0x003 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;

                            RGB_ip = vld3q_u8(pIn+offset);
                            vst3q_u8(pOut+offset, RGB_ip);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = R;
                            pOut[offset + 1] = G;
                            pOut[offset + 2] = B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint8x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 to uint8x8 */
                            RGB_op.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f))));
                            RGB_op.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f))));
                            RGB_op.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f))));

                            vst3_u8(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (uint8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            /* prms->tensor_data_type == VX_TYPE_INT16
               VX_TYPE_INT16           = 0x004 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            int16x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            RGB_op.val[0] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[0]));
                            RGB_op.val[1] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[1]));
                            RGB_op.val[2] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[2]));

                            vst3q_s16(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int16_t)R;
                            pOut[offset + 1] = (int16_t)G;
                            pOut[offset + 2] = (int16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int16x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 */
                            RGB_op.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f)));
                            RGB_op.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f)));
                            RGB_op.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f)));

                            vst3q_s16(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (int16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            /* prms->tensor_data_type == VX_TYPE_UINT16
               VX_TYPE_UINT16          = 0x005 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            RGB_op.val[0] = vmovl_u8(RGB_ip.val[0]);
                            RGB_op.val[1] = vmovl_u8(RGB_ip.val[1]);
                            RGB_op.val[2] = vmovl_u8(RGB_ip.val[2]);

                            vst3q_u16(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint16_t)R;
                            pOut[offset + 1] = (uint16_t)G;
                            pOut[offset + 2] = (uint16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint16x8x3_t RGB_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 */
                            RGB_op.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f)));
                            RGB_op.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f)));
                            RGB_op.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f)));

                            vst3q_u16(pOut+offset, RGB_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (uint16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            /* prms->tensor_data_type == VX_TYPE_INT32
               VX_TYPE_INT32           = 0x006 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            int32x4x3_t RGB_op1;
                            int32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two int16x4 to int32x4 */
                            RGB_op1.val[0] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(R)));
                            RGB_op2.val[0] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(R)));
                            RGB_op1.val[1] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(G)));
                            RGB_op2.val[1] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(G)));
                            RGB_op1.val[2] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(B)));
                            RGB_op2.val[2] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(B)));

                            vst3q_s32(pOut+offset, RGB_op2);
                            vst3q_s32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int32_t)R;
                            pOut[offset + 1] = (int32_t)G;
                            pOut[offset + 2] = (int32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int32x4x3_t RGB_op1;
                            int32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 */
                            RGB_op1.val[0] = vcvtq_s32_f32(R_f);
                            RGB_op2.val[0] = vcvtq_s32_f32(R_s);
                            RGB_op1.val[1] = vcvtq_s32_f32(G_f);
                            RGB_op2.val[1] = vcvtq_s32_f32(G_s);
                            RGB_op1.val[2] = vcvtq_s32_f32(B_f);
                            RGB_op2.val[2] = vcvtq_s32_f32(B_s);

                            vst3q_s32(pOut+offset, RGB_op2);
                            vst3q_s32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (int32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            /* prms->tensor_data_type == VX_TYPE_UINT32
               VX_TYPE_UINT32            = 0x007 */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            uint32x4x3_t RGB_op1;
                            uint32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 */
                            RGB_op1.val[0] = vmovl_u16(vget_high_u16(R));
                            RGB_op2.val[0] = vmovl_u16(vget_low_u16(R));
                            RGB_op1.val[1] = vmovl_u16(vget_high_u16(G));
                            RGB_op2.val[1] = vmovl_u16(vget_low_u16(G));
                            RGB_op1.val[2] = vmovl_u16(vget_high_u16(B));
                            RGB_op2.val[2] = vmovl_u16(vget_low_u16(B));

                            vst3q_u32(pOut+offset, RGB_op2);
                            vst3q_u32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint32_t)R;
                            pOut[offset + 1] = (uint32_t)G;
                            pOut[offset + 2] = (uint32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint32x4x3_t RGB_op1;
                            uint32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 */
                            RGB_op1.val[0] = vcvtq_u32_f32(R_f);
                            RGB_op2.val[0] = vcvtq_u32_f32(R_s);
                            RGB_op1.val[1] = vcvtq_u32_f32(G_f);
                            RGB_op2.val[1] = vcvtq_u32_f32(G_s);
                            RGB_op1.val[2] = vcvtq_u32_f32(B_f);
                            RGB_op2.val[2] = vcvtq_u32_f32(B_s);

                            vst3q_u32(pOut+offset, RGB_op2);
                            vst3q_u32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (uint32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (uint32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            /* prms->tensor_data_type == VX_TYPE_FLOAT32
               VX_TYPE_FLOAT32         = 0x00A */
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4x3_t RGB_op1;
                            float32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 to two float32x4 */
                            RGB_op1.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            RGB_op2.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            RGB_op1.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            RGB_op2.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            RGB_op1.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            RGB_op2.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            vst3q_f32(pOut+offset, RGB_op2);
                            vst3q_f32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[offset + 0] = (float)R;
                            pOut[offset + 1] = (float)G;
                            pOut[offset + 2] = (float)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            float32x4x3_t RGB_op1;
                            float32x4x3_t RGB_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            RGB_op1.val[0] = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB_op2.val[0] = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB_op1.val[1] = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            RGB_op2.val[1] = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            RGB_op1.val[2] = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            RGB_op2.val[2] = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            vst3q_f32(pOut+offset, RGB_op2);
                            vst3q_f32(pOut+offset+(3*4), RGB_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[offset + 0] = (float)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (float)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 2] = (float)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR)
        {
            /* Case 2 */
            /* Input is RGB, Output is BGR (NHWC) */
            if(prms->tensor_data_type == 0x002)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;
                            int8x16x3_t BGR_op;

                            RGB_ip = vld3q_u8(pIn+offset);

                            BGR_op.val[2] = vreinterpretq_s8_u8(RGB_ip.val[0]);
                            BGR_op.val[1] = vreinterpretq_s8_u8(RGB_ip.val[1]);
                            BGR_op.val[0] = vreinterpretq_s8_u8(RGB_ip.val[2]);

                            vst3q_s8(pOut+offset, BGR_op);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int8_t)R;
                            pOut[offset + 1] = (int8_t)G;
                            pOut[offset + 0] = (int8_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int8x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 to int8x8 */
                            BGR_op.val[2] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f))));
                            BGR_op.val[1] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f))));
                            BGR_op.val[0] = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f))));

                            vst3_s8(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (int8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;
                            uint8x16x3_t BGR_op;

                            RGB_ip = vld3q_u8(pIn+offset);

                            BGR_op.val[2] = RGB_ip.val[0];
                            BGR_op.val[1] = RGB_ip.val[1];
                            BGR_op.val[0] = RGB_ip.val[2];

                            vst3q_u8(pOut+offset, BGR_op);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = R;
                            pOut[offset + 1] = G;
                            pOut[offset + 0] = B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint8x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 to uint8x8 */
                            BGR_op.val[2] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f))));
                            BGR_op.val[1] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f))));
                            BGR_op.val[0] = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f))));

                            vst3_u8(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (uint8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            int16x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            BGR_op.val[2] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[0]));
                            BGR_op.val[1] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[1]));
                            BGR_op.val[0] = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[2]));

                            vst3q_s16(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int16_t)R;
                            pOut[offset + 1] = (int16_t)G;
                            pOut[offset + 0] = (int16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int16x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 */
                            BGR_op.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f)));
                            BGR_op.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f)));
                            BGR_op.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f)));

                            vst3q_s16(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (int16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            BGR_op.val[2] = vmovl_u8(RGB_ip.val[0]);
                            BGR_op.val[1] = vmovl_u8(RGB_ip.val[1]);
                            BGR_op.val[0] = vmovl_u8(RGB_ip.val[2]);

                            vst3q_u16(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint16_t)R;
                            pOut[offset + 1] = (uint16_t)G;
                            pOut[offset + 0] = (uint16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint16x8x3_t BGR_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 */
                            BGR_op.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f)));
                            BGR_op.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f)));
                            BGR_op.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f)));

                            vst3q_u16(pOut+offset, BGR_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (uint16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            int32x4x3_t BGR_op1;
                            int32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two int16x4 to int32x4 */
                            BGR_op1.val[2] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(R)));
                            BGR_op2.val[2] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(R)));
                            BGR_op1.val[1] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(G)));
                            BGR_op2.val[1] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(G)));
                            BGR_op1.val[0] = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(B)));
                            BGR_op2.val[0] = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(B)));

                            vst3q_s32(pOut+offset, BGR_op2);
                            vst3q_s32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int32_t)R;
                            pOut[offset + 1] = (int32_t)G;
                            pOut[offset + 0] = (int32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int32x4x3_t BGR_op1;
                            int32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 */
                            BGR_op1.val[2] = vcvtq_s32_f32(R_f);
                            BGR_op2.val[2] = vcvtq_s32_f32(R_s);
                            BGR_op1.val[1] = vcvtq_s32_f32(G_f);
                            BGR_op2.val[1] = vcvtq_s32_f32(G_s);
                            BGR_op1.val[0] = vcvtq_s32_f32(B_f);
                            BGR_op2.val[0] = vcvtq_s32_f32(B_s);

                            vst3q_s32(pOut+offset, BGR_op2);
                            vst3q_s32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (int32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (int32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            uint32x4x3_t BGR_op1;
                            uint32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 */
                            BGR_op1.val[2] = vmovl_u16(vget_high_u16(R));
                            BGR_op2.val[2] = vmovl_u16(vget_low_u16(R));
                            BGR_op1.val[1] = vmovl_u16(vget_high_u16(G));
                            BGR_op2.val[1] = vmovl_u16(vget_low_u16(G));
                            BGR_op1.val[0] = vmovl_u16(vget_high_u16(B));
                            BGR_op2.val[0] = vmovl_u16(vget_low_u16(B));

                            vst3q_u32(pOut+offset, BGR_op2);
                            vst3q_u32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint32_t)R;
                            pOut[offset + 1] = (uint32_t)G;
                            pOut[offset + 0] = (uint32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint32x4x3_t BGR_op1;
                            uint32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 */
                            BGR_op1.val[2] = vcvtq_u32_f32(R_f);
                            BGR_op2.val[2] = vcvtq_u32_f32(R_s);
                            BGR_op1.val[1] = vcvtq_u32_f32(G_f);
                            BGR_op2.val[1] = vcvtq_u32_f32(G_s);
                            BGR_op1.val[0] = vcvtq_u32_f32(B_f);
                            BGR_op2.val[0] = vcvtq_u32_f32(B_s);

                            vst3q_u32(pOut+offset, BGR_op2);
                            vst3q_u32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (uint32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (uint32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (uint32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4x3_t BGR_op1;
                            float32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 to two float32x4 */
                            BGR_op1.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            BGR_op2.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            BGR_op1.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            BGR_op2.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            BGR_op1.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            BGR_op2.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            vst3q_f32(pOut+offset, BGR_op2);
                            vst3q_f32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[offset + 2] = (float)R;
                            pOut[offset + 1] = (float)G;
                            pOut[offset + 0] = (float)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        /* In this case output is simply a copy of input, so use the input stride_y */
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            float32x4x3_t BGR_op1;
                            float32x4x3_t BGR_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            BGR_op1.val[2] = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR_op2.val[2] = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR_op1.val[1] = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            BGR_op2.val[1] = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            BGR_op1.val[0] = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            BGR_op2.val[0] = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            vst3q_f32(pOut+offset, BGR_op2);
                            vst3q_f32(pOut+offset+(3*4), BGR_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[offset + 2] = (float)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[offset + 1] = (float)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[offset + 0] = (float)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
    }
    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW)
    {
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 3 */
            /* Input is RGB Output is RGB (NCHW) */
            if(prms->tensor_data_type == 0x002)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;
                            int8x16_t R_op, G_op, B_op;

                            RGB_ip = vld3q_u8(pIn+offset);

                            R_op = vreinterpretq_s8_u8(RGB_ip.val[0]);
                            G_op = vreinterpretq_s8_u8(RGB_ip.val[1]);
                            B_op = vreinterpretq_s8_u8(RGB_ip.val[2]);

                            vst1q_s8(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1q_s8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s8(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (int8_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (int8_t)G;
                            pOut[(ch_offset * 2) + pos_x] = (int8_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int8x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 to int8x8 */
                            R_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f))));
                            G_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f))));
                            B_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f))));

                            vst1_s8(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1_s8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1_s8(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (int8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (int8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;

                            RGB_ip = vld3q_u8(pIn+offset);

                            vst1q_u8(pOut + (ch_offset * 0) + pos_x, RGB_ip.val[0]);
                            vst1q_u8(pOut + (ch_offset * 1) + pos_x, RGB_ip.val[1]);
                            vst1q_u8(pOut + (ch_offset * 2) + pos_x, RGB_ip.val[2]);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = R;
                            pOut[(ch_offset * 1) + pos_x] = G;
                            pOut[(ch_offset * 2) + pos_x] = B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint8x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 to uint8x8 */
                            R_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f))));
                            G_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f))));
                            B_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f))));

                            vst1_u8(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1_u8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1_u8(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (uint8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (uint8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            int16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            R_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[0]));
                            G_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[1]));
                            B_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[2]));

                            vst1q_s16(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1q_s16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s16(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (int16_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (int16_t)G;
                            pOut[(ch_offset * 2) + pos_x] = (int16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 */
                            R_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f)));
                            G_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f)));
                            B_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f)));

                            vst1q_s16(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1q_s16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s16(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (int16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (int16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            R_op = vmovl_u8(RGB_ip.val[0]);
                            G_op = vmovl_u8(RGB_ip.val[1]);
                            B_op = vmovl_u8(RGB_ip.val[2]);

                            vst1q_u16(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1q_u16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_u16(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (uint16_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (uint16_t)G;
                            pOut[(ch_offset * 2) + pos_x] = (uint16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 */
                            R_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f)));
                            G_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f)));
                            B_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f)));

                            vst1q_u16(pOut + (ch_offset * 0) + pos_x, R_op);
                            vst1q_u16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_u16(pOut + (ch_offset * 2) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (uint16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (uint16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            int32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two int16x4 to int32x4 */
                            R_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(R)));
                            R_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(R)));
                            G_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(G)));
                            G_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(G)));
                            B_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(B)));
                            B_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(B)));

                            vst1q_s32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 0] = (int32_t)R;
                            pOut[offset + 1] = (int32_t)G;
                            pOut[offset + 2] = (int32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 */
                            R_op1 = vcvtq_s32_f32(R_f);
                            R_op2 = vcvtq_s32_f32(R_s);
                            G_op1 = vcvtq_s32_f32(G_f);
                            G_op2 = vcvtq_s32_f32(G_s);
                            B_op1 = vcvtq_s32_f32(B_f);
                            B_op2 = vcvtq_s32_f32(B_s);

                            vst1q_s32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (int32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (int32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            uint32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 */
                            R_op1 = vmovl_u16(vget_high_u16(R));
                            R_op2 = vmovl_u16(vget_low_u16(R));
                            G_op1 = vmovl_u16(vget_high_u16(G));
                            G_op2 = vmovl_u16(vget_low_u16(G));
                            B_op1 = vmovl_u16(vget_high_u16(B));
                            B_op2 = vmovl_u16(vget_low_u16(B));

                            vst1q_u32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (uint32_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (uint32_t)G;
                            pOut[(ch_offset * 2) + pos_x] = (uint32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 */
                            R_op1 = vcvtq_u32_f32(R_f);
                            R_op2 = vcvtq_u32_f32(R_s);
                            G_op1 = vcvtq_u32_f32(G_f);
                            G_op2 = vcvtq_u32_f32(G_s);
                            B_op1 = vcvtq_u32_f32(B_f);
                            B_op2 = vcvtq_u32_f32(B_s);

                            vst1q_u32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (uint32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (uint32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 to two float32x4 */
                            R_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            vst1q_f32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (float)R;
                            pOut[(ch_offset * 1) + pos_x] = (float)G;
                            pOut[(ch_offset * 2) + pos_x] = (float)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            float32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_op1 = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_op2 = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_op1 = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_op2 = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_op1 = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_op2 = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            vst1q_f32(pOut + (ch_offset * 0) + pos_x, R_op2);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x + 4, R_op1);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x, B_op2);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_offset * 0) + pos_x] = (float)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (float)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 2) + pos_x] = (float)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR)
        {
            /* Case 4 */
            /* Input is RGB Output is BGR (NCHW) */
            if(prms->tensor_data_type == 0x002)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;
                            int8x16_t R_op, G_op, B_op;

                            RGB_ip = vld3q_u8(pIn+offset);

                            R_op = vreinterpretq_s8_u8(RGB_ip.val[0]);
                            G_op = vreinterpretq_s8_u8(RGB_ip.val[1]);
                            B_op = vreinterpretq_s8_u8(RGB_ip.val[2]);

                            vst1q_s8(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1q_s8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s8(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (int8_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (int8_t)G;
                            pOut[(ch_offset * 0) + pos_x] = (int8_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int8x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 to int8x8 */
                            R_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f))));
                            G_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f))));
                            B_op = vmovn_s16(vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f))));

                            vst1_s8(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1_s8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1_s8(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (int8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (int8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 4) << 4); pos_x+=16)
                        {
                            uint8x16x3_t RGB_ip;

                            RGB_ip = vld3q_u8(pIn+offset);

                            vst1q_u8(pOut + (ch_offset * 2) + pos_x, RGB_ip.val[0]);
                            vst1q_u8(pOut + (ch_offset * 1) + pos_x, RGB_ip.val[1]);
                            vst1q_u8(pOut + (ch_offset * 0) + pos_x, RGB_ip.val[2]);

                            offset += 3*16;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = R;
                            pOut[(ch_offset * 1) + pos_x] = G;
                            pOut[(ch_offset * 0) + pos_x] = B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint8x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 to uint8x8 */
                            R_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f))));
                            G_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f))));
                            B_op = vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f))));

                            vst1_u8(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1_u8(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1_u8(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint8_t R = pIn[offset + 0];
                            uint8_t G = pIn[offset + 1];
                            uint8_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (uint8_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint8_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (uint8_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            int16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            R_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[0]));
                            G_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[1]));
                            B_op = vreinterpretq_s16_u16(vmovl_u8(RGB_ip.val[2]));

                            vst1q_s16(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1q_s16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s16(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (int16_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (int16_t)G;
                            pOut[(ch_offset * 0) + pos_x] = (int16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 to int16x4 and combine to int16x8 */
                            R_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R_s)), vmovn_s32(vcvtq_s32_f32(R_f)));
                            G_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G_s)), vmovn_s32(vcvtq_s32_f32(G_f)));
                            B_op = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B_s)), vmovn_s32(vcvtq_s32_f32(B_f)));

                            vst1q_s16(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1q_s16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_s16(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int16_t R = pIn[offset + 0];
                            int16_t G = pIn[offset + 1];
                            int16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (int16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (int16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            R_op = vmovl_u8(RGB_ip.val[0]);
                            G_op = vmovl_u8(RGB_ip.val[1]);
                            B_op = vmovl_u8(RGB_ip.val[2]);

                            vst1q_u16(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1q_u16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_u16(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (uint16_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (uint16_t)G;
                            pOut[(ch_offset * 0) + pos_x] = (uint16_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint16x8_t R_op, G_op, B_op;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 to uint16x4 and combine to uint16x8 */
                            R_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R_s)), vmovn_u32(vcvtq_u32_f32(R_f)));
                            G_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G_s)), vmovn_u32(vcvtq_u32_f32(G_f)));
                            B_op = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B_s)), vmovn_u32(vcvtq_u32_f32(B_f)));

                            vst1q_u16(pOut + (ch_offset * 2) + pos_x, R_op);
                            vst1q_u16(pOut + (ch_offset * 1) + pos_x, G_op);
                            vst1q_u16(pOut + (ch_offset * 0) + pos_x, B_op);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint16_t R = pIn[offset + 0];
                            uint16_t G = pIn[offset + 1];
                            uint16_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (uint16_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint16_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (uint16_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            int32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two int16x4 to int32x4 */
                            R_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(R)));
                            R_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(R)));
                            G_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(G)));
                            G_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(G)));
                            B_op1 = vmovl_s16(vreinterpret_s16_u16(vget_high_u16(B)));
                            B_op2 = vmovl_s16(vreinterpret_s16_u16(vget_low_u16(B)));

                            vst1q_s32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[offset + 2] = (int32_t)R;
                            pOut[offset + 1] = (int32_t)G;
                            pOut[offset + 0] = (int32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            int32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to int32x4 */
                            R_op1 = vcvtq_s32_f32(R_f);
                            R_op2 = vcvtq_s32_f32(R_s);
                            G_op1 = vcvtq_s32_f32(G_f);
                            G_op2 = vcvtq_s32_f32(G_s);
                            B_op1 = vcvtq_s32_f32(B_f);
                            B_op2 = vcvtq_s32_f32(B_s);

                            vst1q_s32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_s32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_s32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_s32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            int32_t R = pIn[offset + 0];
                            int32_t G = pIn[offset + 1];
                            int32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (int32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (int32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (int32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            uint32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 */
                            R_op1 = vmovl_u16(vget_high_u16(R));
                            R_op2 = vmovl_u16(vget_low_u16(R));
                            G_op1 = vmovl_u16(vget_high_u16(G));
                            G_op2 = vmovl_u16(vget_low_u16(G));
                            B_op1 = vmovl_u16(vget_high_u16(B));
                            B_op2 = vmovl_u16(vget_low_u16(B));

                            vst1q_u32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (uint32_t)R;
                            pOut[(ch_offset * 1) + pos_x] = (uint32_t)G;
                            pOut[(ch_offset * 0) + pos_x] = (uint32_t)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            uint32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_f = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_s = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_f = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_s = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_f = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_s = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            /* From float32x4 to uint32x4 */
                            R_op1 = vcvtq_u32_f32(R_f);
                            R_op2 = vcvtq_u32_f32(R_s);
                            G_op1 = vcvtq_u32_f32(G_f);
                            G_op2 = vcvtq_u32_f32(G_s);
                            B_op1 = vcvtq_u32_f32(B_f);
                            B_op2 = vcvtq_u32_f32(B_s);

                            vst1q_u32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_u32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_u32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_u32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            uint32_t R = pIn[offset + 0];
                            uint32_t G = pIn[offset + 1];
                            uint32_t B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (uint32_t)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (uint32_t)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (uint32_t)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint32x4 to two float32x4 */
                            R_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_op1 = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_op2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            vst1q_f32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (float)R;
                            pOut[(ch_offset * 1) + pos_x] = (float)G;
                            pOut[(ch_offset * 0) + pos_x] = (float)B;

                            offset += 3;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y++)
                    {
                        uint8_t *pIn  = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        float *pOut = (float *)prms->out_tensor_target_ptr + (pos_y * prms->output_dimensions[0]);
                        uint32_t offset;

                        offset = 0;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for(pos_x = 0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8)
                        {
                            uint8x8x3_t RGB_ip;
                            uint16x8_t R, G, B;
                            float32x4_t R_f, R_s, G_f, G_s, B_f, B_s;
                            float32x4_t R_op1, G_op1, B_op1, R_op2, G_op2, B_op2;

                            RGB_ip = vld3_u8(pIn+offset);

                            /* 8x8 to 16x8 */
                            R = vmovl_u8(RGB_ip.val[0]);
                            G = vmovl_u8(RGB_ip.val[1]);
                            B = vmovl_u8(RGB_ip.val[2]);

                            /* uint16x8 to two uint16x4 to two uint16x4 to float32x4 */
                            R_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(R)));
                            R_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(R)));
                            G_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(G)));
                            G_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(G)));
                            B_f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(B)));
                            B_s = vcvtq_f32_u32(vmovl_u16(vget_low_u16(B)));

                            /* RGB = (RGB - mean) * scale */
                            R_op1 = vmulq_n_f32(vsubq_f32(R_f, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R_op2 = vmulq_n_f32(vsubq_f32(R_s, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G_op1 = vmulq_n_f32(vsubq_f32(G_f, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            G_op2 = vmulq_n_f32(vsubq_f32(G_s, vdupq_n_f32(prms->mean[1])), prms->scale[1]);
                            B_op1 = vmulq_n_f32(vsubq_f32(B_f, vdupq_n_f32(prms->mean[2])), prms->scale[2]);
                            B_op2 = vmulq_n_f32(vsubq_f32(B_s, vdupq_n_f32(prms->mean[2])), prms->scale[2]);

                            vst1q_f32(pOut + (ch_offset * 2) + pos_x, R_op2);
                            vst1q_f32(pOut + (ch_offset * 2) + pos_x + 4, R_op1);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x, G_op2);
                            vst1q_f32(pOut + (ch_offset * 1) + pos_x + 4, G_op1);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x, B_op2);
                            vst1q_f32(pOut + (ch_offset * 0) + pos_x + 4, B_op1);

                            offset += 3*8;
                        }
#else
                        pos_x = 0;
#endif
                        for(; pos_x < prms->input_width; pos_x++)
                        {
                            float R = pIn[offset + 0];
                            float G = pIn[offset + 1];
                            float B = pIn[offset + 2];

                            pOut[(ch_offset * 2) + pos_x] = (float)((R - prms->mean[0]) * prms->scale[0]);
                            pOut[(ch_offset * 1) + pos_x] = (float)((G - prms->mean[1]) * prms->scale[1]);
                            pOut[(ch_offset * 0) + pos_x] = (float)((B - prms->mean[2]) * prms->scale[2]);

                            offset += 3;
                        }
                    }
                }
            }
        }
    }
}

void dlPreProcess_NV12_image
(
    dlPreProcessImageParams *prms
)
{
    uint32_t skip_mean_scale = 0;
    uint32_t pos_x = 0, pos_y = 0;
    uint32_t ch_offset = prms->output_dimensions[0] * prms->output_dimensions[1];

    if(prms->mean[0]==0 && prms->mean[1]==0 && prms->mean[2]==0 &&
       prms->scale[0]==1 && prms->scale[1]==1 && prms->scale[2]==1)
    {
        skip_mean_scale = 1;
    }

    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC)
    {
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 1 */
            /* Input is NV12, Output is RGB (NHWC) */
            if(prms->tensor_data_type == 0x002)
            {
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3_s8(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3_s8(dstRowPtr_1 + dstColIdxBytes, RGB1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_s8(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3_s8(dstRowPtr_1 + dstColIdxBytes, RGB1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_u8(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3_u8(dstRowPtr_1 + dstColIdxBytes, RGB1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_u8(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3_u8(dstRowPtr_1 + dstColIdxBytes, RGB1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_s16(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3q_s16(dstRowPtr_1 + dstColIdxBytes, RGB1);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int16_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int16_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int16_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int16_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int16_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int16_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int16_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int16_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int16_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int16_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int16_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int16_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H)));
                            RGB1.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H)));
                            RGB1.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H)));
                            RGB1.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H)));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_s16(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3q_s16(dstRowPtr_1 + dstColIdxBytes, RGB1);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_u16(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3q_u16(dstRowPtr_1 + dstColIdxBytes, RGB1);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H)));
                            RGB1.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H)));
                            RGB1.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H)));
                            RGB1.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H)));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_u16(dstRowPtr_0 + dstColIdxBytes, RGB0);
                                vst3q_u16(dstRowPtr_1 + dstColIdxBytes, RGB1);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;
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

                            R00 = (uint16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;
                            int32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[0] = vmovl_s16(vget_low_s16(RGB0.val[0]));
                            RGB01.val[0] = vmovl_s16(vget_high_s16(RGB0.val[0]));
                            RGB10.val[0] = vmovl_s16(vget_low_s16(RGB1.val[0]));
                            RGB11.val[0] = vmovl_s16(vget_high_s16(RGB1.val[0]));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[1] = vmovl_s16(vget_low_s16(RGB0.val[1]));
                            RGB01.val[1] = vmovl_s16(vget_high_s16(RGB0.val[1]));
                            RGB10.val[1] = vmovl_s16(vget_low_s16(RGB1.val[1]));
                            RGB11.val[1] = vmovl_s16(vget_high_s16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[2] = vmovl_s16(vget_low_s16(RGB0.val[2]));
                            RGB01.val[2] = vmovl_s16(vget_high_s16(RGB0.val[2]));
                            RGB10.val[2] = vmovl_s16(vget_low_s16(RGB1.val[2]));
                            RGB11.val[2] = vmovl_s16(vget_high_s16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int32_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int32_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int32_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int32_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int32_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int32_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int32_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int32_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int32_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int32_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int32_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int32_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[0] = vcvtq_s32_f32(R0_F32L);
                            RGB01.val[0] = vcvtq_s32_f32(R0_F32H);
                            RGB10.val[0] = vcvtq_s32_f32(R1_F32L);
                            RGB11.val[0] = vcvtq_s32_f32(R1_F32H);

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[1] = vcvtq_s32_f32(G0_F32L);
                            RGB01.val[1] = vcvtq_s32_f32(G0_F32H);
                            RGB10.val[1] = vcvtq_s32_f32(G1_F32L);
                            RGB11.val[1] = vcvtq_s32_f32(G1_F32H);

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[2] = vcvtq_s32_f32(B0_F32L);
                            RGB01.val[2] = vcvtq_s32_f32(B0_F32H);
                            RGB10.val[2] = vcvtq_s32_f32(B1_F32L);
                            RGB11.val[2] = vcvtq_s32_f32(B1_F32H);

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            uint32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[0] = vmovl_u16(vget_low_u16(RGB0.val[0]));
                            RGB01.val[0] = vmovl_u16(vget_high_u16(RGB0.val[0]));
                            RGB10.val[0] = vmovl_u16(vget_low_u16(RGB1.val[0]));
                            RGB11.val[0] = vmovl_u16(vget_high_u16(RGB1.val[0]));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[1] = vmovl_u16(vget_low_u16(RGB0.val[1]));
                            RGB01.val[1] = vmovl_u16(vget_high_u16(RGB0.val[1]));
                            RGB10.val[1] = vmovl_u16(vget_low_u16(RGB1.val[1]));
                            RGB11.val[1] = vmovl_u16(vget_high_u16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[2] = vmovl_u16(vget_low_u16(RGB0.val[2]));
                            RGB01.val[2] = vmovl_u16(vget_high_u16(RGB0.val[2]));
                            RGB10.val[2] = vmovl_u16(vget_low_u16(RGB1.val[2]));
                            RGB11.val[2] = vmovl_u16(vget_high_u16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[0] = vcvtq_u32_f32(R0_F32L);
                            RGB01.val[0] = vcvtq_u32_f32(R0_F32H);
                            RGB10.val[0] = vcvtq_u32_f32(R1_F32L);
                            RGB11.val[0] = vcvtq_u32_f32(R1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[1] = vcvtq_u32_f32(G0_F32L);
                            RGB01.val[1] = vcvtq_u32_f32(G0_F32H);
                            RGB10.val[1] = vcvtq_u32_f32(G1_F32L);
                            RGB11.val[1] = vcvtq_u32_f32(G1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[2] = vcvtq_u32_f32(B0_F32L);
                            RGB01.val[2] = vcvtq_u32_f32(B0_F32H);
                            RGB10.val[2] = vcvtq_u32_f32(B1_F32L);
                            RGB11.val[2] = vcvtq_u32_f32(B1_F32H);

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;
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

                            R00 = (uint32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                float *pOut = (float *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            float32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[0])));
                            RGB01.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[0])));
                            RGB10.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[0])));
                            RGB11.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[0])));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[1])));
                            RGB01.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[1])));
                            RGB10.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[1])));
                            RGB11.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[1])));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[2])));
                            RGB01.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[2])));
                            RGB10.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[2])));
                            RGB11.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[2])));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB interleaved data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            float32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB00.val[0] = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[0] = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[0] = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[0] = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            RGB00.val[1] = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[1] = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[1] = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[1] = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            RGB00.val[2] = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[2] = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[2] = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[2] = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes0, RGB00);
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes1, RGB01);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes0, RGB10);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes1, RGB11);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;
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

                            R00 = ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = R00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = B00;
                            dstRowPtr_0[dstColIdxBytes+3] = R01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = B01;

                            dstRowPtr_1[dstColIdxBytes+0] = R10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = B10;
                            dstRowPtr_1[dstColIdxBytes+3] = R11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = B11;
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
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_s8(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3_s8(dstRowPtr_1 + dstColIdxBytes, BGR1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_s8(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3_s8(dstRowPtr_1 + dstColIdxBytes, BGR1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_u8(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3_u8(dstRowPtr_1 + dstColIdxBytes, BGR1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst3_u8(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3_u8(dstRowPtr_1 + dstColIdxBytes, BGR1);
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

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t BGR0, BGR1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_s16(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3q_s16(dstRowPtr_1 + dstColIdxBytes, BGR1);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int16_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int16_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int16_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int16_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int16_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int16_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int16_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int16_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int16_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int16_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int16_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int16_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t BGR0, BGR1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H)));
                            BGR1.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H)));
                            BGR1.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H)));
                            BGR1.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H)));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_s16(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3q_s16(dstRowPtr_1 + dstColIdxBytes, BGR1);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t BGR0, BGR1;

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
                            BGR0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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
                            BGR0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            BGR0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_u16(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3q_u16(dstRowPtr_1 + dstColIdxBytes, BGR1);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t BGR0, BGR1;

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H)));
                            BGR1.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H)));
                            BGR1.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H)));
                            BGR1.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H)));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes = (pos_x+0) * 3;
                                vst3q_u16(dstRowPtr_0 + dstColIdxBytes, BGR0);
                                vst3q_u16(dstRowPtr_1 + dstColIdxBytes, BGR1);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;
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

                            R00 = (uint16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;
                            int32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[2] = vmovl_s16(vget_low_s16(RGB0.val[0]));
                            BGR01.val[2] = vmovl_s16(vget_high_s16(RGB0.val[0]));
                            BGR10.val[2] = vmovl_s16(vget_low_s16(RGB1.val[0]));
                            BGR11.val[2] = vmovl_s16(vget_high_s16(RGB1.val[0]));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[1] = vmovl_s16(vget_low_s16(RGB0.val[1]));
                            BGR01.val[1] = vmovl_s16(vget_high_s16(RGB0.val[1]));
                            BGR10.val[1] = vmovl_s16(vget_low_s16(RGB1.val[1]));
                            BGR11.val[1] = vmovl_s16(vget_high_s16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[0] = vmovl_s16(vget_low_s16(RGB0.val[2]));
                            BGR01.val[0] = vmovl_s16(vget_high_s16(RGB0.val[2]));
                            BGR10.val[0] = vmovl_s16(vget_low_s16(RGB1.val[2]));
                            BGR11.val[0] = vmovl_s16(vget_high_s16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int32_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int32_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int32_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int32_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int32_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int32_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int32_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int32_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int32_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int32_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int32_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int32_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[2] = vcvtq_s32_f32(R0_F32L);
                            BGR01.val[2] = vcvtq_s32_f32(R0_F32H);
                            BGR10.val[2] = vcvtq_s32_f32(R1_F32L);
                            BGR11.val[2] = vcvtq_s32_f32(R1_F32H);

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[1] = vcvtq_s32_f32(G0_F32L);
                            BGR01.val[1] = vcvtq_s32_f32(G0_F32H);
                            BGR10.val[1] = vcvtq_s32_f32(G1_F32L);
                            BGR11.val[1] = vcvtq_s32_f32(G1_F32H);

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[0] = vcvtq_s32_f32(B0_F32L);
                            BGR01.val[0] = vcvtq_s32_f32(B0_F32H);
                            BGR10.val[0] = vcvtq_s32_f32(B1_F32L);
                            BGR11.val[0] = vcvtq_s32_f32(B1_F32H);

                            /* Store BGR values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_s32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_s32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            uint32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[2] = vmovl_u16(vget_low_u16(RGB0.val[0]));
                            BGR01.val[2] = vmovl_u16(vget_high_u16(RGB0.val[0]));
                            BGR10.val[2] = vmovl_u16(vget_low_u16(RGB1.val[0]));
                            BGR11.val[2] = vmovl_u16(vget_high_u16(RGB1.val[0]));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[1] = vmovl_u16(vget_low_u16(RGB0.val[1]));
                            BGR01.val[1] = vmovl_u16(vget_high_u16(RGB0.val[1]));
                            BGR10.val[1] = vmovl_u16(vget_low_u16(RGB1.val[1]));
                            BGR11.val[1] = vmovl_u16(vget_high_u16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[0] = vmovl_u16(vget_low_u16(RGB0.val[2]));
                            BGR01.val[0] = vmovl_u16(vget_high_u16(RGB0.val[2]));
                            BGR10.val[0] = vmovl_u16(vget_low_u16(RGB1.val[2]));
                            BGR11.val[0] = vmovl_u16(vget_high_u16(RGB1.val[2]));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[2] = vcvtq_u32_f32(R0_F32L);
                            BGR01.val[2] = vcvtq_u32_f32(R0_F32H);
                            BGR10.val[2] = vcvtq_u32_f32(R1_F32L);
                            BGR11.val[2] = vcvtq_u32_f32(R1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[1] = vcvtq_u32_f32(G0_F32L);
                            BGR01.val[1] = vcvtq_u32_f32(G0_F32H);
                            BGR10.val[1] = vcvtq_u32_f32(G1_F32L);
                            BGR11.val[1] = vcvtq_u32_f32(G1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[0] = vcvtq_u32_f32(B0_F32L);
                            BGR01.val[0] = vcvtq_u32_f32(B0_F32H);
                            BGR10.val[0] = vcvtq_u32_f32(B1_F32L);
                            BGR11.val[0] = vcvtq_u32_f32(B1_F32H);

                            /* Store BGR values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_u32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_u32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;
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

                            R00 = (uint32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                float *pOut = (float *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            float32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[0])));
                            BGR01.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[0])));
                            BGR10.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[0])));
                            BGR11.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[0])));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[1])));
                            BGR01.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[1])));
                            BGR10.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[1])));
                            BGR11.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[1])));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[2])));
                            BGR01.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[2])));
                            BGR10.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[2])));
                            BGR11.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[2])));

                            /* Store BGR values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR interleaved data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0] * prms->output_dimensions[1]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0] * prms->output_dimensions[1]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            float32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            BGR00.val[2] = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[2] = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[2] = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[2] = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            BGR00.val[1] = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[1] = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[1] = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[1] = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            BGR00.val[0] = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[0] = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[0] = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[0] = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Store BGR values */
                            {
                                int dstColIdxBytes0 = (pos_x+0) * 3;
                                int dstColIdxBytes1 = (pos_x+0) * 3 + 3*4;
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes0, BGR00);
                                vst3q_f32(dstRowPtr_0 + dstColIdxBytes1, BGR01);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes0, BGR10);
                                vst3q_f32(dstRowPtr_1 + dstColIdxBytes1, BGR11);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;
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

                            R00 = ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[dstColIdxBytes+0] = B00;
                            dstRowPtr_0[dstColIdxBytes+1] = G00;
                            dstRowPtr_0[dstColIdxBytes+2] = R00;
                            dstRowPtr_0[dstColIdxBytes+3] = B01;
                            dstRowPtr_0[dstColIdxBytes+4] = G01;
                            dstRowPtr_0[dstColIdxBytes+5] = R01;

                            dstRowPtr_1[dstColIdxBytes+0] = B10;
                            dstRowPtr_1[dstColIdxBytes+1] = G10;
                            dstRowPtr_1[dstColIdxBytes+2] = R10;
                            dstRowPtr_1[dstColIdxBytes+3] = B11;
                            dstRowPtr_1[dstColIdxBytes+4] = G11;
                            dstRowPtr_1[dstColIdxBytes+5] = R11;
                        }
                    }
                }
            }
        }
    }
    if(prms->channel_order == DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW)
    {
        if(prms->tensor_format == DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB)
        {
            /* Case 3 */
            /* Input is NV12, Output is RGB (NCHW) */
            if(prms->tensor_data_type == 0x002)
            {
                int8_t *pOut = (int8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_s8(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_s8(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_u8(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_u8(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store RGB values */
                            {
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int16_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int16_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int16_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int16_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int16_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int16_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int16_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int16_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int16_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int16_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int16_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int16_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H)));
                            RGB1.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H)));
                            RGB1.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            RGB0.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H)));
                            RGB1.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H)));

                            /* Store RGB values */
                            {
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store RGB values */
                            {
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H)));
                            RGB1.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H)));
                            RGB1.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            RGB0.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H)));
                            RGB1.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H)));

                            /* Store RGB values */
                            {
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB0.val[0]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB0.val[1]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB0.val[2]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB1.val[0]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB1.val[1]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB1.val[2]);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;
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

                            R00 = (uint16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;
                            int32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[0] = vmovl_s16(vget_low_s16(RGB0.val[0]));
                            RGB01.val[0] = vmovl_s16(vget_high_s16(RGB0.val[0]));
                            RGB10.val[0] = vmovl_s16(vget_low_s16(RGB1.val[0]));
                            RGB11.val[0] = vmovl_s16(vget_high_s16(RGB1.val[0]));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[1] = vmovl_s16(vget_low_s16(RGB0.val[1]));
                            RGB01.val[1] = vmovl_s16(vget_high_s16(RGB0.val[1]));
                            RGB10.val[1] = vmovl_s16(vget_low_s16(RGB1.val[1]));
                            RGB11.val[1] = vmovl_s16(vget_high_s16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            RGB00.val[2] = vmovl_s16(vget_low_s16(RGB0.val[2]));
                            RGB01.val[2] = vmovl_s16(vget_high_s16(RGB0.val[2]));
                            RGB10.val[2] = vmovl_s16(vget_low_s16(RGB1.val[2]));
                            RGB11.val[2] = vmovl_s16(vget_high_s16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int32_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int32_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int32_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int32_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int32_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int32_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int32_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int32_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int32_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int32_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int32_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int32_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[0] = vcvtq_s32_f32(R0_F32L);
                            RGB01.val[0] = vcvtq_s32_f32(R0_F32H);
                            RGB10.val[0] = vcvtq_s32_f32(R1_F32L);
                            RGB11.val[0] = vcvtq_s32_f32(R1_F32H);

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[1] = vcvtq_s32_f32(G0_F32L);
                            RGB01.val[1] = vcvtq_s32_f32(G0_F32H);
                            RGB10.val[1] = vcvtq_s32_f32(G1_F32L);
                            RGB11.val[1] = vcvtq_s32_f32(G1_F32H);

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            RGB00.val[2] = vcvtq_s32_f32(B0_F32L);
                            RGB01.val[2] = vcvtq_s32_f32(B0_F32H);
                            RGB10.val[2] = vcvtq_s32_f32(B1_F32L);
                            RGB11.val[2] = vcvtq_s32_f32(B1_F32H);

                            /* Store RGB values */
                            {
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            uint32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[0] = vmovl_u16(vget_low_u16(RGB0.val[0]));
                            RGB01.val[0] = vmovl_u16(vget_high_u16(RGB0.val[0]));
                            RGB10.val[0] = vmovl_u16(vget_low_u16(RGB1.val[0]));
                            RGB11.val[0] = vmovl_u16(vget_high_u16(RGB1.val[0]));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[1] = vmovl_u16(vget_low_u16(RGB0.val[1]));
                            RGB01.val[1] = vmovl_u16(vget_high_u16(RGB0.val[1]));
                            RGB10.val[1] = vmovl_u16(vget_low_u16(RGB1.val[1]));
                            RGB11.val[1] = vmovl_u16(vget_high_u16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            RGB00.val[2] = vmovl_u16(vget_low_u16(RGB0.val[2]));
                            RGB01.val[2] = vmovl_u16(vget_high_u16(RGB0.val[2]));
                            RGB10.val[2] = vmovl_u16(vget_low_u16(RGB1.val[2]));
                            RGB11.val[2] = vmovl_u16(vget_high_u16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint32x4x3_t RGB00, RGB01, RGB10, RGB11;

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[0] = vcvtq_u32_f32(R0_F32L);
                            RGB01.val[0] = vcvtq_u32_f32(R0_F32H);
                            RGB10.val[0] = vcvtq_u32_f32(R1_F32L);
                            RGB11.val[0] = vcvtq_u32_f32(R1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[1] = vcvtq_u32_f32(G0_F32L);
                            RGB01.val[1] = vcvtq_u32_f32(G0_F32H);
                            RGB10.val[1] = vcvtq_u32_f32(G1_F32L);
                            RGB11.val[1] = vcvtq_u32_f32(G1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            RGB00.val[2] = vcvtq_u32_f32(B0_F32L);
                            RGB01.val[2] = vcvtq_u32_f32(B0_F32H);
                            RGB10.val[2] = vcvtq_u32_f32(B1_F32L);
                            RGB11.val[2] = vcvtq_u32_f32(B1_F32H);

                            /* Store RGB values */
                            {
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;
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

                            R00 = (uint32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                float *pOut = (float *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            float32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[0])));
                            RGB01.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[0])));
                            RGB10.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[0])));
                            RGB11.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[0])));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[1])));
                            RGB01.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[1])));
                            RGB10.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[1])));
                            RGB11.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[1])));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            RGB00.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[2])));
                            RGB01.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[2])));
                            RGB10.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[2])));
                            RGB11.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[2])));

                            /* Store RGB values */
                            {
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for RGB data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            float32x4x3_t RGB00, RGB01, RGB10, RGB11;

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
                            RGB00.val[0] = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[0] = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[0] = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[0] = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            RGB00.val[1] = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[1] = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[1] = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[1] = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            RGB00.val[2] = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB01.val[2] = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB10.val[2] = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            RGB11.val[2] = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Store RGB values */
                            {
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x, RGB01.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x, RGB01.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x, RGB01.val[2]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, RGB00.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, RGB00.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, RGB00.val[2]);

                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x, RGB11.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x, RGB11.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x, RGB11.val[2]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, RGB10.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, RGB10.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, RGB10.val[2]);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;
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

                            R00 = ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = R01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = B01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = R11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = B11;
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
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_s8(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_s8(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1_s8(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1_s8(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x003)
            {
                uint8_t *pOut = (uint8_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_u8(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint8_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint8_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
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
                                vst1_u8(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1_u8(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1_u8(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x004)
            {
                int16_t *pOut = (int16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t BGR0, BGR1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            BGR0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store BGR values */
                            {
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int16_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int16_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int16_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int16_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int16_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int16_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int16_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int16_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int16_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int16_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int16_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int16_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t BGR0, BGR1;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R0_F32L)), vmovn_s32(vcvtq_s32_f32(R0_F32H)));
                            BGR1.val[2] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(R1_F32L)), vmovn_s32(vcvtq_s32_f32(R1_F32H)));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G0_F32L)), vmovn_s32(vcvtq_s32_f32(G0_F32H)));
                            BGR1.val[1] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(G1_F32L)), vmovn_s32(vcvtq_s32_f32(G1_F32H)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 to two int16x4 combine to int16x8 */
                            BGR0.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B0_F32L)), vmovn_s32(vcvtq_s32_f32(B0_F32H)));
                            BGR1.val[0] = vcombine_s16(vmovn_s32(vcvtq_s32_f32(B1_F32L)), vmovn_s32(vcvtq_s32_f32(B1_F32H)));

                            /* Store BGR values */
                            {
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1q_s16(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1q_s16(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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
                            int16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int16_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x005)
            {
                uint16_t *pOut = (uint16_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t BGR0, BGR1;

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
                            BGR0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

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
                            BGR0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            BGR0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            BGR1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Store BGR values */
                            {
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint16_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint16_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint16_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint16_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint16_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t BGR0, BGR1;

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R0_F32L)), vmovn_u32(vcvtq_u32_f32(R0_F32H)));
                            BGR1.val[2] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(R1_F32L)), vmovn_u32(vcvtq_u32_f32(R1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G0_F32L)), vmovn_u32(vcvtq_u32_f32(G0_F32H)));
                            BGR1.val[1] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(G1_F32L)), vmovn_u32(vcvtq_u32_f32(G1_F32H)));

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

                            /* Converting two float32x4 to two uint32x4 to two uint16x4 combine to uint16x8 */
                            BGR0.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B0_F32L)), vmovn_u32(vcvtq_u32_f32(B0_F32H)));
                            BGR1.val[0] = vcombine_u16(vmovn_u32(vcvtq_u32_f32(B1_F32L)), vmovn_u32(vcvtq_u32_f32(B1_F32H)));

                            /* Store BGR values */
                            {
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR0.val[0]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR0.val[1]);
                                vst1q_u16(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR0.val[2]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR1.val[0]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR1.val[1]);
                                vst1q_u16(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR1.val[2]);
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
                            uint16_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint16_t B00, B01, B10, B11;
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

                            R00 = (uint16_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint16_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint16_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint16_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint16_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint16_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint16_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint16_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint16_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint16_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint16_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint16_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x006)
            {
                int32_t *pOut = (int32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int16x8x3_t RGB0, RGB1;
                            int32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[2] = vmovl_s16(vget_low_s16(RGB0.val[0]));
                            BGR01.val[2] = vmovl_s16(vget_high_s16(RGB0.val[0]));
                            BGR10.val[2] = vmovl_s16(vget_low_s16(RGB1.val[0]));
                            BGR11.val[2] = vmovl_s16(vget_high_s16(RGB1.val[0]));

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[1] = vmovl_s16(vget_low_s16(RGB0.val[1]));
                            BGR01.val[1] = vmovl_s16(vget_high_s16(RGB0.val[1]));
                            BGR10.val[1] = vmovl_s16(vget_low_s16(RGB1.val[1]));
                            BGR11.val[1] = vmovl_s16(vget_high_s16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            RGB0.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* int16x8 to int16x4 to int32x4 */
                            BGR00.val[0] = vmovl_s16(vget_low_s16(RGB0.val[2]));
                            BGR01.val[0] = vmovl_s16(vget_high_s16(RGB0.val[2]));
                            BGR10.val[0] = vmovl_s16(vget_low_s16(RGB1.val[2]));
                            BGR11.val[0] = vmovl_s16(vget_high_s16(RGB1.val[2]));

                            /* Store RGB values */
                            {
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (int32_t)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01 = (int32_t)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10 = (int32_t)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11 = (int32_t)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00 = (int32_t)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01 = (int32_t)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10 = (int32_t)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11 = (int32_t)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00 = (int32_t)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01 = (int32_t)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10 = (int32_t)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11 = (int32_t)CLIP_SIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        int32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        int32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            int32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t R0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t R1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t R0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R0)));
                            float32x4_t R0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R0)));
                            float32x4_t R1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(R1)));
                            float32x4_t R1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(R1)));

                            /* R = (R - mean) * scale */
                            R0_F32H = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R0_F32L = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32H = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            R1_F32L = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[2] = vcvtq_s32_f32(R0_F32L);
                            BGR01.val[2] = vcvtq_s32_f32(R0_F32H);
                            BGR10.val[2] = vcvtq_s32_f32(R1_F32L);
                            BGR11.val[2] = vcvtq_s32_f32(R1_F32H);

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

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t G0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t G1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t G0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G0)));
                            float32x4_t G0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G0)));
                            float32x4_t G1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(G1)));
                            float32x4_t G1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(G1)));

                            /* G = (G - mean) * scale */
                            G0_F32H = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G0_F32L = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32H = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            G1_F32L = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[1] = vcvtq_s32_f32(G0_F32L);
                            BGR01.val[1] = vcvtq_s32_f32(G0_F32H);
                            BGR10.val[1] = vcvtq_s32_f32(G1_F32L);
                            BGR11.val[1] = vcvtq_s32_f32(G1_F32H);

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to int8 to int16 */
                            int16x8_t B0 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            int16x8_t B1 = vmovl_s8(vqmovn_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* Moving to float for mean and scale opeartions
                            int16x8 to two int16x4 to two int 32x4 to float 32x4 */
                            float32x4_t B0_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B0)));
                            float32x4_t B0_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B0)));
                            float32x4_t B1_F32H = vcvtq_f32_s32(vmovl_s16(vget_high_s16(B1)));
                            float32x4_t B1_F32L = vcvtq_f32_s32(vmovl_s16(vget_low_s16(B1)));

                            /* B = (B - mean) * scale */
                            B0_F32H = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B0_F32L = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32H = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            B1_F32L = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Converting two float32x4 to two int32x4 */
                            BGR00.val[0] = vcvtq_s32_f32(B0_F32L);
                            BGR01.val[0] = vcvtq_s32_f32(B0_F32H);
                            BGR10.val[0] = vcvtq_s32_f32(B1_F32L);
                            BGR11.val[0] = vcvtq_s32_f32(B1_F32H);

                            /* Store BGR values */
                            {
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_s32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_s32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            int32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            int32_t B00, B01, B10, B11;
                            float R00_F, R01_F, R10_F, R11_F, G00_F, G01_F, G10_F, G11_F;
                            float B00_F, B01_F, B10_F, B11_F;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00_F = (float)CLIP_SIGNED(YUV2R(Y00,U,V));
                            R01_F = (float)CLIP_SIGNED(YUV2R(Y01,U,V));
                            R10_F = (float)CLIP_SIGNED(YUV2R(Y10,U,V));
                            R11_F = (float)CLIP_SIGNED(YUV2R(Y11,U,V));

                            G00_F = (float)CLIP_SIGNED(YUV2G(Y00,U,V));
                            G01_F = (float)CLIP_SIGNED(YUV2G(Y01,U,V));
                            G10_F = (float)CLIP_SIGNED(YUV2G(Y10,U,V));
                            G11_F = (float)CLIP_SIGNED(YUV2G(Y11,U,V));

                            B00_F = (float)CLIP_SIGNED(YUV2B(Y00,U,V));
                            B01_F = (float)CLIP_SIGNED(YUV2B(Y01,U,V));
                            B10_F = (float)CLIP_SIGNED(YUV2B(Y10,U,V));
                            B11_F = (float)CLIP_SIGNED(YUV2B(Y11,U,V));

                            R00 = (int32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (int32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (int32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (int32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (int32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (int32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (int32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (int32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (int32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (int32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (int32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (int32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x007)
            {
                uint32_t *pOut = (uint32_t *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            uint32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[2] = vmovl_u16(vget_low_u16(RGB0.val[0]));
                            BGR01.val[2] = vmovl_u16(vget_high_u16(RGB0.val[0]));
                            BGR10.val[2] = vmovl_u16(vget_low_u16(RGB1.val[0]));
                            BGR11.val[2] = vmovl_u16(vget_high_u16(RGB1.val[0]));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[1] = vmovl_u16(vget_low_u16(RGB0.val[1]));
                            BGR01.val[1] = vmovl_u16(vget_high_u16(RGB0.val[1]));
                            BGR10.val[1] = vmovl_u16(vget_low_u16(RGB1.val[1]));
                            BGR11.val[1] = vmovl_u16(vget_high_u16(RGB1.val[1]));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 */
                            BGR00.val[0] = vmovl_u16(vget_low_u16(RGB0.val[2]));
                            BGR01.val[0] = vmovl_u16(vget_high_u16(RGB0.val[2]));
                            BGR10.val[0] = vmovl_u16(vget_low_u16(RGB1.val[2]));
                            BGR11.val[0] = vmovl_u16(vget_high_u16(RGB1.val[2]));

                            /* Store BGR values */
                            {
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (uint32_t)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (uint32_t)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (uint32_t)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        uint32_t* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        uint32_t* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint32x4x3_t BGR00, BGR01, BGR10, BGR11;

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[2] = vcvtq_u32_f32(R0_F32L);
                            BGR01.val[2] = vcvtq_u32_f32(R0_F32H);
                            BGR10.val[2] = vcvtq_u32_f32(R1_F32L);
                            BGR11.val[2] = vcvtq_u32_f32(R1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[1] = vcvtq_u32_f32(G0_F32L);
                            BGR01.val[1] = vcvtq_u32_f32(G0_F32H);
                            BGR10.val[1] = vcvtq_u32_f32(G1_F32L);
                            BGR11.val[1] = vcvtq_u32_f32(G1_F32H);

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

                            /* Converting two float32x4 to two uint32x4 */
                            BGR00.val[0] = vcvtq_u32_f32(B0_F32L);
                            BGR01.val[0] = vcvtq_u32_f32(B0_F32H);
                            BGR10.val[0] = vcvtq_u32_f32(B1_F32L);
                            BGR11.val[0] = vcvtq_u32_f32(B1_F32H);

                            /* Store BGR values */
                            {
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_u32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_u32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            uint32_t R00, R01, R10, R11, G00, G01, G10, G11;
                            uint32_t B00, B01, B10, B11;
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

                            R00 = (uint32_t) ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = (uint32_t) ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = (uint32_t) ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = (uint32_t) ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = (uint32_t) ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = (uint32_t) ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = (uint32_t) ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = (uint32_t) ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = (uint32_t) ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = (uint32_t) ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = (uint32_t) ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = (uint32_t) ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
            if(prms->tensor_data_type == 0x00A)
            {
                float *pOut = (float *)prms->out_tensor_target_ptr;

                if(skip_mean_scale)
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            uint16x8x3_t RGB0, RGB1;
                            float32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            RGB0.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[0] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[0])));
                            BGR01.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[0])));
                            BGR10.val[2] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[0])));
                            BGR11.val[2] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[0])));

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
                            RGB0.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[1] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[0])));
                            BGR01.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[0])));
                            BGR10.val[1] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[0])));
                            BGR11.val[1] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[0])));

                            /* b = y + (30402 * u >> 14) - 237 */
                            /* calc = (30402 * u >> 14) - 237 */
                            tmp_mid_f = vshrn_n_s32(vmulq_n_s32(U_f, 30402), 14);
                            tmp_mid_s = vshrn_n_s32(vmulq_n_s32(U_s, 30402), 14);

                            tmp_mid = vcombine_s16(tmp_mid_f, tmp_mid_s);
                            tmp_mid = vsubq_s16(tmp_mid, vdupq_n_s16(237));

                            /* Saturating narrow from int16 to uint8 to uint16 */
                            RGB0.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y0, tmp_mid)));
                            RGB1.val[2] = vmovl_u8(vqmovun_s16(dlPreProc_yuv2rgb_vtr(Y1, tmp_mid)));

                            /* uint16x8 to uint16x4 to uint32x4 to float32x4 */
                            BGR00.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB0.val[2])));
                            BGR01.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB0.val[2])));
                            BGR10.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(RGB1.val[2])));
                            BGR11.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(RGB1.val[2])));

                            /* Store BGR values */
                            {
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;

                            uint8_t Y00 = src0RowPtr_0[src0ColIdxBytes+0];
                            uint8_t Y01 = src0RowPtr_0[src0ColIdxBytes+1];
                            uint8_t Y10 = src0RowPtr_1[src0ColIdxBytes+0];
                            uint8_t Y11 = src0RowPtr_1[src0ColIdxBytes+1];
                            uint8_t U = src1RowPtr[src1ColIdxBytes];
                            uint8_t V = src1RowPtr[src1ColIdxBytes+1];

                            R00 = (float)CLIP_UNSIGNED(YUV2R(Y00,U,V));
                            R01 = (float)CLIP_UNSIGNED(YUV2R(Y01,U,V));
                            R10 = (float)CLIP_UNSIGNED(YUV2R(Y10,U,V));
                            R11 = (float)CLIP_UNSIGNED(YUV2R(Y11,U,V));

                            G00 = (float)CLIP_UNSIGNED(YUV2G(Y00,U,V));
                            G01 = (float)CLIP_UNSIGNED(YUV2G(Y01,U,V));
                            G10 = (float)CLIP_UNSIGNED(YUV2G(Y10,U,V));
                            G11 = (float)CLIP_UNSIGNED(YUV2G(Y11,U,V));

                            B00 = (float)CLIP_UNSIGNED(YUV2B(Y00,U,V));
                            B01 = (float)CLIP_UNSIGNED(YUV2B(Y01,U,V));
                            B10 = (float)CLIP_UNSIGNED(YUV2B(Y10,U,V));
                            B11 = (float)CLIP_UNSIGNED(YUV2B(Y11,U,V));

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
                else
                {
                    for(pos_y = 0; pos_y < prms->input_height; pos_y+=2)
                    {
                        /* src0 for Y data */
                        uint8_t* src0RowPtr_0 = (uint8_t *)prms->in_img_target_ptr[0] + (pos_y * prms->in_stride_y);
                        uint8_t* src0RowPtr_1 = (uint8_t *)prms->in_img_target_ptr[0] + ((pos_y+1) * prms->in_stride_y);

                        /* src1 for UV interleaved data */
                        uint8_t* src1RowPtr = (uint8_t *)prms->in_img_target_ptr[1] + ((pos_y >> 1) * prms->in_stride_y);

                        /* dst for BGR data */
                        float* dstRowPtr_0 = pOut + (pos_y * prms->output_dimensions[0]);
                        float* dstRowPtr_1 = pOut + ((pos_y+1) * prms->output_dimensions[0]);

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
                        for( pos_x=0; pos_x < ((prms->input_width >> 3) << 3); pos_x+=8 ) {

                            uint8x8_t Y0, Y1;
                            int32x4_t U_f = vdupq_n_s32(0), U_s = vdupq_n_s32(0), V_f = vdupq_n_s32(0), V_s = vdupq_n_s32(0);
                            int16x4_t tmp_mid_f, tmp_mid_s;
                            int16x8_t tmp_mid;
                            float32x4x3_t BGR00, BGR01, BGR10, BGR11;

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
                            BGR00.val[2] = vmulq_n_f32(vsubq_f32(R0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[2] = vmulq_n_f32(vsubq_f32(R0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[2] = vmulq_n_f32(vsubq_f32(R1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[2] = vmulq_n_f32(vsubq_f32(R1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            BGR00.val[1] = vmulq_n_f32(vsubq_f32(G0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[1] = vmulq_n_f32(vsubq_f32(G0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[1] = vmulq_n_f32(vsubq_f32(G1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[1] = vmulq_n_f32(vsubq_f32(G1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

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
                            BGR00.val[0] = vmulq_n_f32(vsubq_f32(B0_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR01.val[0] = vmulq_n_f32(vsubq_f32(B0_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR10.val[0] = vmulq_n_f32(vsubq_f32(B1_F32H, vdupq_n_f32(prms->mean[0])), prms->scale[0]);
                            BGR11.val[0] = vmulq_n_f32(vsubq_f32(B1_F32L, vdupq_n_f32(prms->mean[0])), prms->scale[0]);

                            /* Store BGR values */
                            {
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x, BGR01.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x, BGR01.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x, BGR01.val[2]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 0) + pos_x + 4, BGR00.val[0]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 1) + pos_x + 4, BGR00.val[1]);
                                vst1q_f32(dstRowPtr_0 + (ch_offset * 2) + pos_x + 4, BGR00.val[2]);

                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x, BGR11.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x, BGR11.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x, BGR11.val[2]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 0) + pos_x + 4, BGR10.val[0]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 1) + pos_x + 4, BGR10.val[1]);
                                vst1q_f32(dstRowPtr_1 + (ch_offset * 2) + pos_x + 4, BGR10.val[2]);
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
                            float R00, R01, R10, R11, G00, G01, G10, G11;
                            float B00, B01, B10, B11;
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

                            R00 = ((R00_F - prms->mean[0])* prms->scale[0]);
                            R01 = ((R01_F - prms->mean[0])* prms->scale[0]);
                            R10 = ((R10_F - prms->mean[0])* prms->scale[0]);
                            R11 = ((R11_F - prms->mean[0])* prms->scale[0]);

                            G00 = ((G00_F - prms->mean[1])* prms->scale[1]);
                            G01 = ((G01_F - prms->mean[1])* prms->scale[1]);
                            G10 = ((G10_F - prms->mean[1])* prms->scale[1]);
                            G11 = ((G11_F - prms->mean[1])* prms->scale[1]);

                            B00 = ((B00_F - prms->mean[2])* prms->scale[2]);
                            B01 = ((B01_F - prms->mean[2])* prms->scale[2]);
                            B10 = ((B10_F - prms->mean[2])* prms->scale[2]);
                            B11 = ((B11_F - prms->mean[2])* prms->scale[2]);

                            dstRowPtr_0[(ch_offset * 0) + pos_x + 0] = B00;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 0] = G00;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 0] = R00;
                            dstRowPtr_0[(ch_offset * 0) + pos_x + 1] = B01;
                            dstRowPtr_0[(ch_offset * 1) + pos_x + 1] = G01;
                            dstRowPtr_0[(ch_offset * 2) + pos_x + 1] = R01;

                            dstRowPtr_1[(ch_offset * 0) + pos_x + 0] = B10;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 0] = G10;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 0] = R10;
                            dstRowPtr_1[(ch_offset * 0) + pos_x + 1] = B11;
                            dstRowPtr_1[(ch_offset * 1) + pos_x + 1] = G11;
                            dstRowPtr_1[(ch_offset * 2) + pos_x + 1] = R11;
                        }
                    }
                }
            }
        }
    }
}
