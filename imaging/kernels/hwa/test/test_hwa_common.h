/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <TI/hwa_vpac_msc.h>
#include <math.h>
#include "tivx_utils_file_rd_wr.h"
#include <utils/hwa/include/app_hwa.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>

void ct_teardown_hwa_kernels(void/*vx_context*/ **context_);
void ct_read_raw_image_to_vx_image(vx_image image, const char* fileName, uint16_t file_byte_pack);
void ct_write_image2(vx_image image, const char* fileName);
vx_status readNV12Input(char* file_name, vx_image in_img);
vx_status readNV12P12Input(char* file_name, vx_image in_img);
vx_int32 write_output_image_fp(FILE * fp, vx_image out_image);
vx_int32 write_output_image_nv12_8bit(char * file_name, vx_image out);
vx_int32 write_output_image_fp_nv12_p12(FILE * fp, vx_image out_image);
vx_int32 write_output_image_nv12_p12(char * file_name, vx_image out);

/* VISS common functions */

void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack, uint16_t downshift_bits);
void ct_write_user_data_object(vx_user_data_object user_data_object, const char* fileName);
vx_status save_image_from_viss(vx_image y8, char *filename_prefix);

void ct_read_hist(vx_distribution hist, const char* fileName, uint16_t file_byte_pack);
void ct_read_user_data_object(vx_user_data_object user_data_object, const char* fileName, uint16_t file_byte_pack);
vx_int32 ct_cmp_image2(vx_image image, vx_image image_ref);
vx_int32 ct_cmp_hist(vx_distribution hist, vx_distribution hist_ref);
vx_int32 get_dcc_file_size(char * file_name);
vx_int32 read_dcc_file(char * file_name, uint8_t * buf, uint32_t num_bytes);
CT_Image raw_read_image(const char* fileName, int width, int height);
vx_int32 write_viss_output_image_fp(FILE * fp, vx_image out_image);
vx_int32 write_viss_output_image(char * file_name, vx_image out);
uint8_t isMuxValid(int mux0, int mux1, int mux2, int mux3, int mux4);
vx_int32 write_output_ir_image(char * file_name, vx_image out, vx_uint32 ir_format);

/* MSC common functions */

void scale_set_output_params(tivx_vpac_msc_output_params_t *params, uint32_t interpolation, uint32_t iw, uint32_t ih, uint32_t ow, uint32_t oh);
void img_scale_set_coeff(tivx_vpac_msc_coefficients_t *coeff, uint32_t interpolation);
CT_Image scale_generate_random(const char* fileName, int width, int height);
CT_Image scale_generate_gradient_2x2(const char* fileName, int width, int height);
CT_Image scale_generate_gradient_16x16(const char* fileName, int width, int height);
CT_Image scale_generate_pattern3x3(const char* fileName, int width, int height);
CT_Image scale_read_image(const char* fileName, int width, int height);
vx_int32 ct_image_get_pixel_8u(CT_Image img, int x, int y, vx_border_t border);
int scale_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border);
int scale_check_pixel_exact(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border);
void scale_validate(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact);
void scale_check(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact);
void img_dst_size_generator_1_1(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_1_3(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_2_1(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_3_1(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_4_1(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_5_1(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_SCALE_PYRAMID_ORB(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_SCALE_NEAR_UP(int width, int height, int* dst_width, int* dst_height);
void img_dst_size_generator_SCALE_NEAR_DOWN(int width, int height, int* dst_width, int* dst_height);
vx_status save_image_from_msc(vx_image y8, char *filename_prefix);
vx_status readUYVYInput(char* file_name, vx_image in_img);
vx_int32 write_uyvy_output_image_fp(FILE * fp, vx_image out_image);
vx_int32 write_uyvy_output_image(char * file_name, vx_image out);

#if defined(LDRA_COVERAGE)
int32_t imaging_vpac_coverage_start();
int32_t imaging_vpac_coverage_end();
#if !defined(SOC_AM62A)
int32_t imaging_dmpac_coverage_start();
int32_t imaging_dmpac_coverage_end();
#endif /* LDRA_COVERAGE */
#endif