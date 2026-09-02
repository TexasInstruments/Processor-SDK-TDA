/******************************************************************************
Copyright (c) 2024 Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 license under copyrights and patents it now or hereafter owns or controls to
 make,  have made, use, import, offer to sell and sell ("Utilize") this software
 subject to the terms herein.  With respect to the foregoing patent license,
 such license is granted  solely to the extent that any such patent is necessary
 to Utilize the software alone.  The patent license shall not apply to any
 combinations which include this software, other than combinations with devices
 manufactured by or for TI ("TI Devices").  No hardware patent is licensed
 hereunder.

 Redistributions must preserve existing copyright notices and reproduce this
 license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

 * No reverse engineering, decompilation, or disassembly of this software
   is permitted with respect to any software provided in binary form.

 * Any redistribution and use are licensed by TI for use only with TI Devices.

 * Nothing shall obligate TI to provide you with source code for the software
   licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution of
 the source code are permitted provided that the following conditions are met:

 * Any redistribution and use of the source code, including any resulting
   derivative works, are licensed by TI for use only with TI Devices.

 * Any redistribution and use of any object code compiled from the source code
   and any resulting derivative works, are licensed by TI for use only with TI
   Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this software
 without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
/**
 *  \file ov2312_linear_1600x1300_30FPS_setup.h
 *
 *  \brief OV2312 sensor settings for 1600x1300 frame size in linear mode.
 *
 */

#ifndef OV2312_LINEAR_1600x1300_SETUP_H_
#define OV2312_LINEAR_1600x1300_SETUP_H_

#define DELAY_MS	0x1U
/* OV2312 recommended setting 1600X1300 RAW10 MIPI 30FPS */
#define OV2312_LINEAR_1600x1300_30FPS_CONFIG   \
{   \
		{0x0103, 0x01, DELAY_MS}, \
		{0x0100, 0x00, DELAY_MS}, \
		{0x010c, 0x02, DELAY_MS}, \
		{0x010b, 0x01, DELAY_MS}, \
		{0x0300, 0x01, DELAY_MS}, \
		{0x0302, 0x32, DELAY_MS}, \
		{0x0303, 0x00, DELAY_MS}, \
		{0x0304, 0x03, DELAY_MS}, \
		{0x0305, 0x02, DELAY_MS}, \
		{0x0306, 0x01, DELAY_MS}, \
		{0x030d, 0x5a, DELAY_MS}, \
		{0x030e, 0x04, DELAY_MS}, \
		{0x3001, 0x02, DELAY_MS}, \
		{0x3004, 0x00, DELAY_MS}, \
		{0x3005, 0x00, DELAY_MS}, \
		{0x3006, 0x0a, DELAY_MS}, \
		{0x3011, 0x0d, DELAY_MS}, \
		{0x3014, 0x04, DELAY_MS}, \
		{0x301c, 0xf0, DELAY_MS}, \
		{0x3020, 0x20, DELAY_MS}, \
		{0x302c, 0x00, DELAY_MS}, \
		{0x302d, 0x00, DELAY_MS}, \
		{0x302e, 0x00, DELAY_MS}, \
		{0x302f, 0x03, DELAY_MS}, \
		{0x3030, 0x10, DELAY_MS}, \
		{0x303f, 0x03, DELAY_MS}, \
		{0x3103, 0x00, DELAY_MS}, \
		{0x3106, 0x08, DELAY_MS}, \
		{0x31ff, 0x01, DELAY_MS}, \
		{0x3501, 0x05, DELAY_MS}, \
		{0x3502, 0x7c, DELAY_MS}, \
		{0x3506, 0x00, DELAY_MS}, \
		{0x3507, 0x00, DELAY_MS}, \
		{0x3620, 0x67, DELAY_MS}, \
		{0x3633, 0x78, DELAY_MS}, \
		{0x3662, 0x65, DELAY_MS}, \
		{0x3664, 0xb0, DELAY_MS}, \
		{0x3666, 0x70, DELAY_MS}, \
		{0x3670, 0x68, DELAY_MS}, \
		{0x3674, 0x10, DELAY_MS}, \
		{0x3675, 0x00, DELAY_MS}, \
		{0x367e, 0x90, DELAY_MS}, \
		{0x3680, 0x84, DELAY_MS}, \
		{0x36a2, 0x04, DELAY_MS}, \
		{0x36a3, 0x80, DELAY_MS}, \
		{0x36b0, 0x00, DELAY_MS}, \
		{0x3700, 0x35, DELAY_MS}, \
		{0x3704, 0x39, DELAY_MS}, \
		{0x370a, 0x50, DELAY_MS}, \
		{0x3712, 0x00, DELAY_MS}, \
		{0x3713, 0x02, DELAY_MS}, \
		{0x3778, 0x00, DELAY_MS}, \
		{0x379b, 0x01, DELAY_MS}, \
		{0x379c, 0x10, DELAY_MS}, \
		{0x3800, 0x00, DELAY_MS}, \
		{0x3801, 0x00, DELAY_MS}, \
		{0x3802, 0x00, DELAY_MS}, \
		{0x3803, 0x00, DELAY_MS}, \
		{0x3804, 0x06, DELAY_MS}, \
		{0x3805, 0x4f, DELAY_MS}, \
		{0x3806, 0x05, DELAY_MS}, \
		{0x3807, 0x23, DELAY_MS}, \
		{0x3808, 0x06, DELAY_MS}, \
		{0x3809, 0x40, DELAY_MS}, \
		{0x380a, 0x05, DELAY_MS}, \
		{0x380b, 0x14, DELAY_MS}, \
		{0x380c, 0x03, DELAY_MS}, \
		{0x380d, 0xa8, DELAY_MS}, \
		{0x380e, 0x05, DELAY_MS}, \
		{0x380f, 0x88, DELAY_MS}, \
		{0x3810, 0x00, DELAY_MS}, \
		{0x3811, 0x08, DELAY_MS}, \
		{0x3812, 0x00, DELAY_MS}, \
		{0x3813, 0x08, DELAY_MS}, \
		{0x3814, 0x11, DELAY_MS}, \
		{0x3815, 0x11, DELAY_MS}, \
		{0x3816, 0x00, DELAY_MS}, \
		{0x3817, 0x01, DELAY_MS}, \
		{0x3818, 0x00, DELAY_MS}, \
		{0x3819, 0x05, DELAY_MS}, \
		{0x382b, 0x5a, DELAY_MS}, \
		{0x382c, 0x0a, DELAY_MS}, \
		{0x382d, 0xf8, DELAY_MS}, \
		{0x3881, 0x44, DELAY_MS}, \
		{0x3882, 0x02, DELAY_MS}, \
		{0x3883, 0x8c, DELAY_MS}, \
		{0x3885, 0x07, DELAY_MS}, \
		{0x389d, 0x03, DELAY_MS}, \
		{0x38a6, 0x00, DELAY_MS}, \
		{0x38a7, 0x01, DELAY_MS}, \
		{0x38b3, 0x07, DELAY_MS}, \
		{0x38b1, 0x00, DELAY_MS}, \
		{0x38e5, 0x02, DELAY_MS}, \
		{0x38e7, 0x00, DELAY_MS}, \
		{0x38e8, 0x00, DELAY_MS}, \
		{0x3910, 0xff, DELAY_MS}, \
		{0x3911, 0xff, DELAY_MS}, \
		{0x3912, 0x08, DELAY_MS}, \
		{0x3913, 0x00, DELAY_MS}, \
		{0x3914, 0x00, DELAY_MS}, \
		{0x3915, 0x00, DELAY_MS}, \
		{0x391c, 0x00, DELAY_MS}, \
		{0x3920, 0xff, DELAY_MS}, \
		{0x3921, 0x80, DELAY_MS}, \
		{0x3922, 0x00, DELAY_MS}, \
		{0x3923, 0x00, DELAY_MS}, \
		{0x3924, 0x05, DELAY_MS}, \
		{0x3925, 0x00, DELAY_MS}, \
		{0x3926, 0x00, DELAY_MS}, \
		{0x3927, 0x00, DELAY_MS}, \
		{0x3928, 0x1a, DELAY_MS}, \
		{0x392d, 0x03, DELAY_MS}, \
		{0x392e, 0xa8, DELAY_MS}, \
		{0x392f, 0x08, DELAY_MS}, \
		{0x4001, 0x00, DELAY_MS}, \
		{0x4003, 0x40, DELAY_MS}, \
		{0x4008, 0x04, DELAY_MS}, \
		{0x4009, 0x1b, DELAY_MS}, \
		{0x400c, 0x04, DELAY_MS}, \
		{0x400d, 0x1b, DELAY_MS}, \
		{0x4010, 0xf4, DELAY_MS}, \
		{0x4011, 0x00, DELAY_MS}, \
		{0x4016, 0x00, DELAY_MS}, \
		{0x4017, 0x04, DELAY_MS}, \
		{0x4042, 0x11, DELAY_MS}, \
		{0x4043, 0x70, DELAY_MS}, \
		{0x4045, 0x00, DELAY_MS}, \
		{0x4409, 0x5f, DELAY_MS}, \
		{0x4509, 0x00, DELAY_MS}, \
		{0x450b, 0x00, DELAY_MS}, \
		{0x4600, 0x00, DELAY_MS}, \
		{0x4601, 0x80, DELAY_MS}, \
		{0x4708, 0x09, DELAY_MS}, \
		{0x470c, 0x81, DELAY_MS}, \
		{0x4710, 0x06, DELAY_MS}, \
		{0x4711, 0x00, DELAY_MS}, \
		{0x4800, 0x00, DELAY_MS}, \
		{0x481f, 0x30, DELAY_MS}, \
		{0x4837, 0x14, DELAY_MS}, \
		{0x4f00, 0x00, DELAY_MS}, \
		{0x4f07, 0x00, DELAY_MS}, \
		{0x4f08, 0x03, DELAY_MS}, \
		{0x4f09, 0x08, DELAY_MS}, \
		{0x4f0c, 0x05, DELAY_MS}, \
		{0x4f0d, 0xb4, DELAY_MS}, \
		{0x4f10, 0x00, DELAY_MS}, \
		{0x4f11, 0x00, DELAY_MS}, \
		{0x4f12, 0x07, DELAY_MS}, \
		{0x4f13, 0xe2, DELAY_MS}, \
		{0x5000, 0x9f, DELAY_MS}, \
		{0x5001, 0x20, DELAY_MS}, \
		{0x5026, 0x00, DELAY_MS}, \
		{0x5c00, 0x00, DELAY_MS}, \
		{0x5c01, 0x2c, DELAY_MS}, \
		{0x5c02, 0x00, DELAY_MS}, \
		{0x5c03, 0x7f, DELAY_MS}, \
		{0x5e00, 0x00, DELAY_MS}, \
		{0x5e01, 0x41, DELAY_MS}, \
		{0x38b1, 0x02, DELAY_MS}, \
		{0x0100, 0x01, DELAY_MS}, \
		{0x3006, 0x08, DELAY_MS}, /* Strobe control  */ \
		{0x3004, 0x02, DELAY_MS}, \
		{0x3007, 0x02, DELAY_MS}, \
		{0x301c, 0x20, DELAY_MS}, \
		{0x3020, 0x20, DELAY_MS}, \
		{0x3025, 0x02, DELAY_MS}, \
		{0x382c, 0x0a, DELAY_MS}, \
		{0x382d, 0xf8, DELAY_MS}, \
		{0x3920, 0xff, DELAY_MS}, \
		{0x3921, 0x00, DELAY_MS}, \
		{0x3923, 0x00, DELAY_MS}, \
		{0x3924, 0x00, DELAY_MS}, \
		{0x3925, 0x00, DELAY_MS}, \
		{0x3926, 0x00, DELAY_MS}, \
	  	{0x3927, 0x00, DELAY_MS}, \
		{0x3928, 0x80, DELAY_MS}, \
		{0x392b, 0x00, DELAY_MS}, \
		{0x392c, 0x00, DELAY_MS}, \
		{0x392d, 0x03, DELAY_MS}, \
		{0x392e, 0xa8, DELAY_MS}, \
		{0x392f, 0x0b, DELAY_MS}, \
		{0x38b3, 0x07, DELAY_MS}, \
		{0x3885, 0x07, DELAY_MS}, \
		{0x382b, 0x3a, DELAY_MS}, \
		{0x3670, 0x68, DELAY_MS}, \
		{0x301C, 0xF0, DELAY_MS}, /* AB mode - Group auto switch example setting */ \
		{0x3209, 0x01, DELAY_MS}, \
		{0x320A, 0x01, DELAY_MS}, \
		{0x320B, 0x00, DELAY_MS}, \
		{0x320C, 0x00, DELAY_MS}, \
		{0x3208, 0x01, DELAY_MS},  /* Group A (IR Dominant VC1) */ \
		{0x3501, (0x0090U >> 8U) & 0xffU, DELAY_MS}, \
		{0x3502, 0x0090U & 0xffU, DELAY_MS}, \
		{0x3508, 0x01, DELAY_MS}, \
		{0x3509, 0x00, DELAY_MS}, \
		{0x350a, 0x01, DELAY_MS}, \
		{0x350b, 0x00, DELAY_MS}, \
		{0x3920, 0xff, DELAY_MS}, /* IR Strobe duty cycle */\
		{0x3927, (0x0090U >> 8U) & 0xffU, DELAY_MS}, \
		{0x3928, 0x0090U & 0xffU, DELAY_MS}, \
		{0x3929, ((0x0588U - 0x0090U - 7U) >> 8U) & 0xffU, DELAY_MS}, \
		{0x392a, (0x0588U - 0x0090U - 7U) & 0xffU, DELAY_MS}, \
		{0x4813, 0x00, DELAY_MS}, /* VC ID*/\
		{0x3208, 0x11, DELAY_MS}, \
		{0x3208, 0x00, DELAY_MS}, /* Group B (RGB Dominant VC0) */\
		{0x3501, 0x05, DELAY_MS}, \
		{0x3502, 0x7c, DELAY_MS}, \
		{0x3508, 0x01, DELAY_MS}, \
		{0x3509, 0x00, DELAY_MS}, \
		{0x350a, 0x01, DELAY_MS}, \
		{0x350b, 0x00, DELAY_MS}, \
		{0x3920, 0x00, DELAY_MS}, \
		{0x4813, 0x01, DELAY_MS}, /* VC ID*/\
		{0x3208, 0x10, DELAY_MS}, \
		{0x320D, 0x00, DELAY_MS}, /* Auto mode switch between group0 and group1 ;setting to switch */ \
		{0x320D, 0x31, DELAY_MS}, \
		{0x3208, 0xA0, DELAY_MS}, \
}
#define OV2312_LINEAR_CONFIG_SIZE (217)

static I2cParams ov2312LinearConfig30fps[OV2312_LINEAR_CONFIG_SIZE] =
    OV2312_LINEAR_1600x1300_30FPS_CONFIG;

#endif /* #ifndef OV2312_LINEAR_1688x1388_SETUP_H_  */

