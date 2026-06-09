/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#ifndef HWA_VPAC_FC_H_
#define HWA_VPAC_FC_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include "TI/hwa_vpac_viss.h"
#include "TI/hwa_vpac_msc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The FC kernels in this kernel extension.
 */

/*! \brief vpac_fc kernel name
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_KERNEL_VPAC_FC_NAME "com.ti.hwa.vpac_fc"


/*********************************
 *      VPAC_FC Control Commands
 *********************************/

/*! \brief Control Command to set MSC Filter Coefficients
 *         User data object tivx_vpac_msc_coefficients_t is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_MSC_CMD_SET_COEFF                    (0x10000000u)

/*! \brief Control Command to set MSC input parameters.
 *
 *         These parameters are common for all scaler outputs.
 *         Used to configure/select the number of taps to be
 *         used for scaling operation, line increment by 2 to improve
 *         HW throughput for 1/2 scaling.
 *
 *         This control command uses pointer to structure
 *         tivx_vpac_msc_input_params_t as an input argument.
 *
 *         The index0 of the vx_reference passed to the control command is
 *         used to specify user object containing input parameters.
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_MSC_CMD_SET_INPUT_PARAMS             (0x10000001u)

/*! \brief Control Command to set MSC output scaling parameters.
 *
 *         This command takes an array of vx_references as an argument,
 *         where each index contains output parameters for the
 *         corresponding scaler. For example, the output parameters at
 *         the index0 contains output parameters for the scaler output0
 *         and so on. If the reference is set to null for a scaler, default
 *         scaler parameters or previously set/configured parameters
 *         are used for that scaler.
 *
 *         Used to configure/select number of phases for the
 *         scaling operation, the coefficients and other scaler parameters.
 *
 *         This command takes an array of user objects containing
 *         tivx_vpac_msc_output_params_t parameters..
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_MSC_CMD_SET_OUTPUT_PARAMS            (0x10000002u)

/*! \brief Control Command to set MSC output croping parameters.
 *
 *         This command takes an array of vx_references as an argument,
 *         where each index contains crop parameters for the
 *         corresponding scaler. For example, the crop parameters at
 *         the index0 contains crop parameters required by output0 and so on.
 *         The crop parameters are applied on the corrosponding input image.
 *         If the reference is set to null for a scaler, default
 *         scaler parameters or previously set/configured parameters
 *         are used for that scaler. (Default: no crop)
 *
 *         Used to configure/select number of phases for the
 *         scaling operation, the coefficients and other scaler parameters.
 *
 *         This command takes an array of user objects containing
 *         tivx_vpac_msc_crop_params_t parameters.
 *
 *         Node: Applies only to \ref tivxVpacFcVissMscNode
 *
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS            (0x10000003u)

/*! \brief Control Command to set the DCC (Dynamic Camera Configuration)
 *         information to the given VISS Node.
 *
 *         Viss node gets the pointer to DCC buffer containing
 *         VISS configuration. It uses DCC parser to parse and
 *         map DCC parameters into VISS configuration and sets it
 *         in the driver.
 *
 *         Note that if the module is bypassed in tivx_vpac_fc_viss_msc_params_t
 *         during node creating, the parameter will not be set for
 *         this module, even through DCC profile has config for
 *         the same module.
 *
 *         User data object containing DCC buffer is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_VISS_CMD_SET_DCC_PARAMS                   (0x10000004u)

/*! \brief Control Command to delete the flex-connect path
 *         created by the FC node.
 *
 *         This command is used to delete the flex-connect path
 *         created by the FC node. The command is used when the
 *         graph is deleted or when the graph is reconfigured.
 *
 *         This command does not take any arguments.
 *
 *  \ingroup group_vision_function_vpac_fc
 */
#define TIVX_VPAC_FC_DELETE_GRAPH                              (0x10000005u)

/*********************************
 *      VPAC_FC Defines
 *********************************/

#define TIVX_KERNEL_VPAC_FC_MAX_MSC_INPUT_THREADS    (4U)
#define TIVX_VPAC_FC_VISS_OUT0                      (0U)
#define TIVX_VPAC_FC_VISS_OUT1                      (1U)
#define TIVX_VPAC_FC_VISS_OUT2                      (2U)
#define TIVX_VPAC_FC_VISS_OUT3                      (3U)
#define TIVX_VPAC_FC_MSC_CH_INVALID                 (4U)
#define TIVX_VPAC_FC_MSC0                           (0U)
#define TIVX_VPAC_FC_MSC1                           (1U)
#define TIVX_VPAC_FC_MSC_TH_INVALID                 (2U)

/*********************************
 *      VPAC_FC STRUCTURES
 *********************************/

/*!
 * \brief The input config data structure used by the
 *        TIVX_KERNEL_VPAC_FC kernel.
 *
 * \ingroup group_vision_function_vpac_fc
 */ 
typedef struct {

    /*
    *
    * |     val  | msc0_input0 | msc0_input1 | msc1_input0 | msc1_input1 |
    * |:--------:|:-----------:|:-----------:| :----------:|:-----------:|
    * | 0        | VISS_OUT0   | VISS_OUT0   | VISS_OUT0   | VISS_OUT0   |
    * | 1        | VISS_OUT1   | VISS_OUT1   | VISS_OUT1   | VISS_OUT1   |
    * | 2        | VISS_OUT2   | VISS_OUT2   | VISS_OUT2   | VISS_OUT2   |
    * | 3        | VISS_OUT3   | VISS_OUT3   | VISS_OUT3   | VISS_OUT3   |
    * | 4        | INVALID     | INVALID     | INVALID     | INVALID     |
    * | -------- | ----------  | ----------  | ----------  | ----------  |
    * 
    * Note:
    *   - YUV420, YUV422, Luma+Chroma uses output_0 or output_2
    *       - output_1 or output_3 is unusable respectively
    *   - Single plane Luma uses output_0 or output_2
    *   - Single plane Chroma uses output_1 or output_3
    *   - Any other value is INVALID
    *   - For VPAC3L devices VISS_OUT2 and VISS_OUT3 is enabled at 1.5 bytes per pixel for Flexconnect; VISS_OUT0 and VISS_OUT1 is enabled at 2 bytes per pixel
    *   - For VPAC1 devices, Flexconnect is supported with LUMA only outputs on MSC0; Chroma only outputs on MSC1 will be enabled in upcoming release
    */
    uint32_t     msc_in_thread_viss_out_map[TIVX_KERNEL_VPAC_FC_MAX_MSC_INPUT_THREADS];

    /*
    * | val | msc_output0 | msc_output1 | .. | msc_output7 | msc_output8 | msc_output9 |
    * |:---:|:-----------:|:-----------:|:--:|:-----------:|:-----------:|:-----------:|
    * | 0   | MSC_0       | MSC_0       | .. | MSC_0       | MSC_0       | MSC_0       |
    * | 1   | MSC_1       | MSC_1       | .. | MSC_1       | MSC_1       | MSC_1       |
    * | 2   | INVALID     | INVALID     | .. | INVALID     | INVALID     | INVALID     |* 
    * | ----| ----------- | ----------- | -- | ----------- | ----------- | ----------- |
    * 
    * Note:
    *   - A single YUV420 uses 2 MSC outputs
    *       - mapped to even outputs (0/2/4/6/8)
    *       - odd outputs (1/3/5/7/9) are unusable
    *   - Any other value is INVALID
    *   - For VPAC3L devices only MSC0 is enabled for Flexconnect
    *   - For VPAC1 devices, Flexconnect is supported only on MSC0; MSC1 will be enabled in upcoming release
    */
    uint32_t     msc_out_msc_in_map[TIVX_VPAC_MSC_MAX_OUTPUT];

    tivx_vpac_viss_params_t         tivxVissPrms;

} tivx_vpac_fc_viss_msc_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

void tivx_vpac_fc_params_init(tivx_vpac_fc_viss_msc_params_t *tivxFcParams);

/*!
 * \brief Function to register HWA Kernels on the vpac_fc Target
 * \ingroup group_vision_function_vpac_fc
 */
void tivxRegisterHwaTargetVpacFcKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_fc Target
 * \ingroup group_vision_function_vpac_fc
 */
void tivxUnRegisterHwaTargetVpacFcKernels(void);

/*! \brief [Graph] Creates a VPAC_FC Node.
 *
 * At a high level, VPAC FC converts RAW image sensor data into
 * processed YUV or RGB images.
 *
 * FC node will support 4 optional viss outputs (viss_output0 to viss_output3 in a future release)
 * and 10 optional msc outputs (msc_output0 to msc_output9).
 * At least one of the msc_outputs must be enabled.
 *
 * VISS Outputs(optional)
 * The resolution of all the image ports should have the same width
 * and height. The only exception to this is if the
 * - NV12 uses viss_output0 and/or viss_output2 with viss_output1 and/or viss_output3 not usable
 * - viss_output1 and/or viss_output3 is used to output chroma alone by selecting
 *      appropriate tivx_vpac_fc_viss_msc_params_t::tivx_vpac_viss_params_t::fcp::mux_output1 and
 *      tivx_vpac_fc_viss_msc_params_t::tivx_vpac_viss_params_t::fcp::mux_output3, then the height is
 *      half of the input for these 2 ports if the tivx_vpac_fc_viss_msc_params_t::tivx_vpac_viss_params_t::fcp::chroma_mode is
 *      selected as TIVX_VPAC_VISS_CHROMA_MODE_420.
 *  - viss_output0 and/or viss_output2 is used to output luma alone by selecting
 *      appropriate tivx_vpac_fc_viss_msc_params_t::tivx_vpac_viss_params_t::fcp::mux_output0 and
 *      tivx_vpac_fc_viss_msc_params_t::tivx_vpac_viss_params_t::fcp::mux_output2
 *
 * MSC Outputs(atleast one must be enabled)
 * This node should adhere to the following rules:
 *  - If multi_scale_outputs are NV12, a maximum of five NV12 outputs can be generated.
 *  they should be assigned sequentially from out0 to out3 without any gaps.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration             The input object of a single params structure of type
 *                                       <tt>\ref tivx_vpac_fc_viss_msc_params_t</tt>.
 *                                       These parameters essentially defines path inside
 *                                       VISS and are used to select output format.
 * \param [in] viss_ae_awb_result (optional)  The input object of a single params structure of
 *                                       type <tt>\ref tivx_ae_awb_params_t</tt>.
 *                                       Typically this input parameter would come
 *                                       from a 2A algorithm node.
 * \param [in] dcc_buf (optional)        DCC tuning database for the given sensor
 *                                       <tt>\ref vx_user_data_object </tt>
 * \param [in] viss_raw                       The RAW input image (can contain up to 3 exposures
 *                                       plus meta data) in P12 or U16 or U8 format.
 *                                       RAW Image at index 0 is used for single exposure
 *                                          processing and also for Linear mode processing.
 *                                       For two exposure WDR merge mode, RAW Image at
 *                                          index 0 is used for short exposure and image
 *                                          at index1 is used for long exposure.
 *                                       For three exposure WDR merge mode, RAW Image at
 *                                          index 0 is used for very short exposure,
 *                                          image at index1 for short exposure and image
 *                                          at index2 for long exposure.
 * \param [out] viss_out0 (optional)       Output0. This will be enabled in a future release
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 12bit or 8bit format, or
 *                                          Luma plane of the YUV420 frame in 12 or 8 bit, or
 *                                          Value from the HSV module, or
 *                                          Red color plane in 8bit or
 *                                          YUV422 (YUYV or UYVY) in 8bit format or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[0]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>.
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] viss_out1 (optional)       Output1. This will be enabled in a future release
 *                                       Typically this output is used to get
 *                                          Chroma plane of YUV420 frame in 12bit or 8bit format or
 *                                          One of the CFA output  in 12bit format
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[1]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output0 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] viss_out2 (optional)       Output2. This will be enabled in a future release
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 12bit or 8bit format, or
 *                                          Luma plane of the YUV420 frame in 12 or 8 bit, or
 *                                          Value from the HSV module, or
 *                                          Red color plane in 8bit or
 *                                          YUV422 (YUYV or UYVY) in 8bit format or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[2]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>.
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] viss_out3 (optional)       Output3. This will be enabled in a future release
 *                                       Typically this output is used to get
 *                                          Chroma plane of YUV420 frame in 12bit or 8bit format or
 *                                          One of the CFA output  in 12bit format
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[3]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output0 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] viss_h3a_output (optional)    AEWB/AF output.
 *                                       This output is used to get AEWB/AF output.
 *                                       User data object of type \ref tivx_h3a_data_t
 *                                       is used to AEWB/AF output.
 *                                       Only one of AEWB & AF can be enabled and
 *                                       outputted at a time.
 * \param [out] viss_histogram0 (optional)    The output FCP0 histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 * \param [out] viss_histogram1 (optional)    The output FCP1 histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 * \param [out] viss_raw_histogram (optional) The output raw data histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 128 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 128 x sizeof(uint32_t), which is sufficient
 *                                       for storing 128x22bit histogram.
 * \param [out] msc_scale_out0_img The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out1_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out2_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 *              format.
 * \param [out] msc_scale_out3_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out4_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out5_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out6_img (optional) The output image in
*              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out7_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out8_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] msc_scale_out9_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \see <tt>TIVX_KERNEL_VPAC_FC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_fc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *         Any possible errors preventing a successful creation should
 *         be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacFcVissMscNode(vx_graph  graph,
                                      vx_user_data_object   configuration,
                                      vx_user_data_object   viss_ae_awb_result,
                                      vx_user_data_object   dcc_buf,
                                      tivx_raw_image        viss_raw,
                                      vx_image              viss_out0,
                                      vx_image              viss_out1,
                                      vx_image              viss_out2,
                                      vx_image              viss_out3,
                                      vx_user_data_object   viss_h3a_out,
                                      vx_distribution       viss_histogram0,
                                      vx_distribution       viss_histogram1,
                                      vx_distribution       viss_raw_histogram,                                                                      
                                      vx_image              msc_scale_out0_img,
                                      vx_image              msc_scale_out1_img,
                                      vx_image              msc_scale_out2_img,
                                      vx_image              msc_scale_out3_img,
                                      vx_image              msc_scale_out4_img,
                                      vx_image              msc_scale_out5_img,
                                      vx_image              msc_scale_out6_img,
                                      vx_image              msc_scale_out7_img,
                                      vx_image              msc_scale_out8_img,
                                      vx_image              msc_scale_out9_img);

#ifdef __cplusplus
}
#endif

#endif /* HWA_VPAC_FC_H_ */

