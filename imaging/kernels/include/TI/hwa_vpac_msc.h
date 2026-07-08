/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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

#ifndef HWA_VPAC_MSC_H_
#define HWA_VPAC_MSC_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The MSC kernels in this kernel extension.
 */

/*! \brief vpac_msc scaler kernel name
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME "com.ti.hwa.vpac_msc_multi_scale"

/*! \brief vpac_msc pyramid kernel name
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME   "com.ti.hwa.vpac_msc_pyramid"

/*! \brief vpac_msc scaler2 kernel name
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_KERNEL_VPAC_MSC_MULTI_SCALE2_NAME "com.ti.hwa.vpac_msc_multi_scale2"

/*********************************
 *      VPAC_MSC Control Commands
 *********************************/

/*! \brief Control Command to set MSC Filter Coefficients
 *         User data object tivx_vpac_msc_coefficients_t is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_COEFF                    (0x20000000u)

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
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS             (0x20000001u)

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
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS            (0x20000002u)

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
 *         Node: Applies only to \ref tivxVpacMscScaleNode
 *
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS            (0x20000003u)

/*! \brief Control Command to get the combined error status
 *         Returns the combined error status of the last processed frame,
 *         incorporating error interrupts and watchdog timer error interrupt. 
 *         A reference to vx_scalar is passed as an argument with 
 *         this control command.
 *         The node returns a bit-mask of the error status in the 
 *         U32 variable of vx_scalar. 
 *
 *         - Error interrupt status bits (0–29) are directly 
 *           taken from the msc_obj->err_stat variable.
 *         - If watchdog timer error is detected in msc0 (bit 24 in wd_err_status),
 *           this is flagged by setting bit 30 in the returned error status.
 *         - If watchdog timer error is detected in msc1 (bit 25 in wd_err_status),
 *           this is flagged by setting bit 31 in the returned error status. 
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_GET_ERR_STATUS                    (0x20000004u)

/*! \brief Control Command to retrieve the PSA status
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_GET_PSA_STATUS                    (0x20000005u)

/*! \brief Control Command to enable/disable error events
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS               (0x20000006u)

/* MSC: Add command for enabling safety mechanism */
#define TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM           (0x20000007u)

/*********************************
 *      VPAC_MSC Defines
 *********************************/

/*!
 * \defgroup group_vision_function_vpac_msc_enums Enumerations
 * \brief Enumerations for VPAC MSC configuration parameters and error events
 * \ingroup group_vision_function_vpac_msc
 * @{*/

/*! Maximum number of Multi Phase Coefficient Set */
#define TIVX_VPAC_MSC_MAX_MP_COEFF_SET                (4U)

/*! Maximum number of Single Phase Coefficient Set */
#define TIVX_VPAC_MSC_MAX_SP_COEFF_SET                (2U)

/*! Maximum number of Scaler output supported by Scaler,
 *  Used by the sim target kernel */
#define TIVX_VPAC_MSC_MAX_OUTPUT                      (10U)

/*! Maximum number of filter taps supported in Scaler */
#define TIVX_VPAC_MSC_MAX_TAP                         (5U)

/*! 32 Phase coefficients for Scaler */
#define TIVX_VPAC_MSC_32_PHASE_COEFF                  (32U)

/*! Autocompute: Allow the node to autocompute instead of specifying */
#define TIVX_VPAC_MSC_AUTOCOMPUTE                     (0xFFFFFFFFU)

/*! Specify the 32 phase gaussian interpolation filter coeficients */
#define TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE (0)

/*! \see tivx_vpac_msc_input_params_t::yc_mode */
#define TIVX_VPAC_MSC_MODE_LUMA_ONLY        (0U)

/*! \see tivx_vpac_msc_input_params_t::yc_mode */
#define TIVX_VPAC_MSC_MODE_CHROMA_ONLY      (1U)

/*! 420SP UV chroma only mode */
#define TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY      (1U)

/*! 420SP VU chroma only mode */
#define TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY      (2U)

/*! 422SP UV chroma only mode */
#define TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY      (3U)

/*! 422SP VU chroma only mode */
#define TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY      (4U)

/*! default value of divison factor */
#define TIVX_VPAC_MSC_INIT_DIV_FACTOR       (1U)

/*! default value of max downscaling factor */
#define TIVX_VPAC_MSC_DEFAULT_MAX_DS_FACTOR       (2U)

/*! Error on Input VBUSM Read interface */
#define TIVX_VPAC_MSC_VBUSM_RD_ERR                   (0x4U)

/*!Error on SL2 VBSUM Write interface */
#define TIVX_VPAC_MSC_SL2_WR_ERR                     (0x8U)

/*! Watchdog Timeout Error for MSC0 */
#define TIVX_VPAC_MSC0_WDTIMER_ERR                   (0x40000000U)

/*! Watchdog Timeout Error for MSC1 */
#define TIVX_VPAC_MSC1_WDTIMER_ERR                   (0x80000000U)

/*! Macro to enable all MSC0 error events */
#define TIVX_VPAC_MSC0_ALL_ERR_EVENTS ( \
    TIVX_VPAC_MSC_VBUSM_RD_ERR | \
    TIVX_VPAC_MSC_SL2_WR_ERR | \
    TIVX_VPAC_MSC0_WDTIMER_ERR \
)

/*! Macro to enable all MSC1 error events */
#define TIVX_VPAC_MSC1_ALL_ERR_EVENTS ( \
    TIVX_VPAC_MSC_VBUSM_RD_ERR | \
    TIVX_VPAC_MSC_SL2_WR_ERR | \
    TIVX_VPAC_MSC1_WDTIMER_ERR \
)
/*! \brief Disable Watchdog Timer (default) */
#define TIVX_VPAC_MSC_WDTIMER_DISABLE                (0u)
/*! \brief Enable Watchdog Timer */
#define TIVX_VPAC_MSC_WDTIMER_ENABLE                 (1u)
/*! \brief Watchdog Timer timeout: 64K cycles (default) */
#define TIVX_VPAC_MSC_WDTIMER_64K_CYCLES             (0u)
/*! \brief Watchdog Timer timeout: 128K cycles */
#define TIVX_VPAC_MSC_WDTIMER_128K_CYCLES            (1u)

/** \brief Maximum possible MSC iteration for a simgle request (Luma + Chroma) */
#define TIVX_VPAC_MSC_MAX_COMP                   (2u)

/*@}*/

/*********************************
 *      VPAC_MSC STRUCTURES
 *********************************/

/*!
 * \brief The coefficients input data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! 2 Sets of Single phase coefficient set 0, signed 10-bit */
    int32_t  single_phase[TIVX_VPAC_MSC_MAX_SP_COEFF_SET][TIVX_VPAC_MSC_MAX_TAP];
    /*! 4 Sets of Multi phase coefficient, signed 10-bit */
    int32_t  multi_phase[TIVX_VPAC_MSC_MAX_MP_COEFF_SET]
        [TIVX_VPAC_MSC_MAX_TAP*TIVX_VPAC_MSC_32_PHASE_COEFF];
} tivx_vpac_msc_coefficients_t;

/*!
 * \brief The configuration data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Single-phase horizontal filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient
     *         set 0 (specified using 'horz_coef_sel')
     */
    uint32_t  horz_coef_src;
    /*! Single-phase horizontal filter coefficient selection
     *      if (horz_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient
     *          set 0 [Range (0-31)]
     */
    uint32_t  horz_coef_sel;
    /*! Single-phase vertical filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient
     *         set 0 (specified using 'vert_coef_sel')
     */
    uint32_t  vert_coef_src;
    /*! Single-phase vertical filter coefficient selection
     *      if (vert_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient
     *          set 0 [Range (0-31)]
     */
    uint32_t  vert_coef_sel;
} tivx_vpac_msc_single_phase_params_t;

/*!
 * \brief The multi_phase data structure in the configuration data
 *        structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Multi-phase Mode, 0: 64 phases, 1: 32 phases */
    uint32_t  phase_mode;
    /*! Multi-phase horizontal coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint32_t  horz_coef_sel;
    /*! Multi-phase vertical coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint32_t  vert_coef_sel;
    /*! Multi-phase initial horizontal resize phase (U12Q12):
     *  Used to align center tap if filter to appropriate input for first output [Range (0-4095, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_output_params_t::offset_x
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  init_phase_x;
    /*! Multi-phase initial vertical resize phase (U12Q12):
     *  Used to align center tap if filter to appropriate input for first output [Range (0-4095, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_output_params_t::offset_y
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  init_phase_y;
} tivx_vpac_msc_multi_phase_params_t;

/*!
 * \brief The crop config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Source region of interest X offset [Range (0-width)] */
    uint32_t  crop_start_x;
    /*! Source region of interest Y offset [Range (0-height)] */
    uint32_t  crop_start_y;
    /*! Source region of interest X width [Range (0-(width-startx))] */
    uint32_t  crop_width;
    /*! Source region of interest Y height [Range (0-(height-starty))] */
    uint32_t  crop_height;
} tivx_vpac_msc_crop_params_t;

/*!
 * \brief The output config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Integer type of input and output frame data,
     *  0: Unsigned 12-bit, 1: Signed 12-bit */
    uint32_t  signed_data;
    /*! 0: Single Phase Filter (integer ratios: 1x, 1/2x, 1/4x);
     *  1: Multi-phase scaling filter */
    uint32_t  filter_mode;
    /*! Sets the fractional bit precision of the 10-bit filter
     *  coefficient [Range (5-9)] */
    uint32_t  coef_shift;
    /*! Filter output saturation mdoe,
     *   0: [0..4095] clipping;
     *   1: [-2048..2047] clip followed by + 2048 */
    uint32_t  saturation_mode;
    /*! Source X offset: used to align center tap if filter to appropriate input for first output [Range (0,1, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_multi_phase_params_t::init_phase_x
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  offset_x;
    /*! Source Y offset: used to align center tap if filter to appropriate input for first output [Range (0,1, or TIVX_VPAC_MSC_AUTOCOMPUTE))]
     *  \see tivx_vpac_msc_multi_phase_params_t::init_phase_y
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  offset_y;
    /*! Optional: When input 16-bit unpacked, alignment of 12-bit pixel,
     *  0: LSB, 1:MSB */
    uint32_t  output_align_12bit;

    /*! Optional: When 'filter_mode' == 0: Single-phase */
    tivx_vpac_msc_single_phase_params_t single_phase;
    /*! Optional: When 'filter_mode' == 1: Multi-phase */
    tivx_vpac_msc_multi_phase_params_t  multi_phase;
} tivx_vpac_msc_output_params_t;

/*!
 * \brief The input config data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! kernel size [Range (3 - 5)] */
    uint32_t  kern_sz;
    /*! 0: Off, 1: vertical skip input lines (LSE) */
    uint32_t  src_ln_inc_2;
    /*! Optional: When input 16-bit unpacked,
     *  alignment of 12-bit pixel, 0: LSB, 1:MSB */
    uint32_t  input_align_12bit;
    /*! When input format is NV12 and output format is U8,
     *  this flag indicates which plane(chroma or luma) is scaled for all outputs
     *  0: Luma only mode, 1: Chroma Only mode 
     *  \ref TIVX_VPAC_MSC_MODE_LUMA_ONLY, TIVX_VPAC_MSC_MODE_CHROMA_ONLY macros */
    uint32_t yc_mode;
    /*! max_ds_factor is the pyramid node max down scaling factor. 
     *  If max_ds_factor = 2, Pyramid node downscales the input image within the range from 1x to 1/2x in a single pass. 
     *  If max_ds_factor = 4, Pyramid node downscales the input image within the range from 1x to 1/4x in a single pass. 
     *  Although the hardware supports a maximum downscaling factor of 4, 
     *  this variable allows downscale the input image within range from 1x to 1/2x when set to 2.
     *  The quality diminishes when downscaling beyond 2 in single pass. However, 
     *  if downscaling beyond 2 is needed, setting it to 4 reduces hardware passes, improving speed. 
     *  So, it's a trade-off between speed and quality. 
     *  This parameter is only used for pyramid nodes, not for other MSC nodes */
    uint32_t max_ds_factor;
    #if defined(VPAC3) || defined(VPAC3L)
    /*! This flag indicates, process the input either simultaneously or sequentially 
     *  0: Sequential process of input (scale Y and UV planes one after another), 
     *  1: Simultaneous processing of input (scale both Y and UV planes at the same time) */
    uint32_t is_enable_simul_processing;
    /*! When in_img0 is not NV12, this flag indicates which plane(chroma or luma) is provided
     * 0: Luma only mode, 1: Chroma Only mode
     * \ref TIVX_VPAC_MSC_MODE_LUMA_ONLY, TIVX_VPAC_MSC_MODE_CHROMA_ONLY macros
     * in_img0_yc_mode must be luma only (in_img0_yc_mode = 0) */
    uint32_t in_img0_yc_mode;
    /*! When in_img1 is not NULL, this flag indicates which plane(chroma or luma) is provided
     * 0: Luma only mode, 1: Chroma Only mode
     * \ref TIVX_VPAC_MSC_MODE_LUMA_ONLY, TIVX_VPAC_MSC_MODE_CHROMA_ONLY macros */
    uint32_t in_img1_yc_mode;
    #endif
    /*!
     * This flag is a 32-bit bitmask used to specify which error events 
     * should be reported. Each bit in this mask corresponds to a specific error type 
     * generated by the hardware. Setting a bit to 1 enables reporting for that error, 
     * while setting it to 0 disables it.
     *
     * ### How to Enable Error Events
     * Use the predefined error constants to configure enable_error_events:
     * 
     * - \ref TIVX_VPAC_MSC_VBUSM_RD_ERR
     * - \ref TIVX_VPAC_MSC_SL2_WR_ERR
     * - \ref TIVX_VPAC_MSC0_WDTIMER_ERR
     * - \ref TIVX_VPAC_MSC1_WDTIMER_ERR
     * 
     * To enable specific errors, combine the relevant error bits using a bitwise OR. 
     * For example, to monitor TIVX_VPAC_MSC_VBUSM_RD_ERR and TIVX_VPAC_MSC0_WDTIMER_ERR, set:
     * 
     * @code
     * enable_error_events = TIVX_VPAC_MSC_VBUSM_RD_ERR | TIVX_VPAC_MSC0_WDTIMER_ERR;
     * @endcode
     */
    uint32_t enable_error_events;
    /*!
     *  This flag is used to control the retrieval of the PSA signature. When set to 1,
     *  the PSA signature is retrieved; when set to 0, it is not.
     */
    uint32_t enable_psa;
    /*!
     *  This field configures the watchdog timer timeout cycles.
     *  Watchdog timer is enabled when \ref TIVX_VPAC_MSC0_WDTIMER_ERR is set in enable_error_events.
     *  Set to 0 for 64K cycles (default), set to 1 for 128K cycles.
     */
    uint32_t wdtimer_timeout_cycles;
} tivx_vpac_msc_input_params_t;

/**
 * @brief Structure to hold PSA timestamp data.
 *
 * This structure contains PSA values and a timestamp.
 */
typedef struct {
    /**
     * @brief 2D array of PSA values.
     *
     * Array of 2 rows and 10 columns to store PSA values.
     */
    uint32_t psa_values[TIVX_VPAC_MSC_MAX_COMP][TIVX_VPAC_MSC_MAX_OUTPUT];

    /**
     * @brief Timestamp value.
     *
     * 64-bit unsigned integer to store the timestamp value.
     */
    uint64_t timestamp;
} tivx_vpac_msc_psa_timestamp_data_t;

/*!
 * \brief Safety parameters structure for VPAC MSC.
 *
 * This structure can be used to configure safety-related features
 * such as error event monitoring and status register validation for the
 * MSC module.
 *
 * The following safety mechanisms can be enabled by setting the individual
 * parameters:
 * - VPAC12 and VPAC-F2 enable_reconfig_and_reinit_Reg: Enables the
 *   reconfiguration and reinitialization of the registers safety mechanism
 *   feature.
 * - VPAC10 enable_status_reg_validation: Enables the status register
 *   validation safety mechanism feature.
 * - VPAC7 enable_readback_config_registers: Enables the readback of
 *   configuration registers for safety validation.
 *
 * Operation modes for safety flags:
 * - 0 = disabled: The feature is turned off.
 * - 1 = one-shot: The feature operates once per trigger.
 * - 2 = continuous: The feature operates continuously until stopped.
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {

    /* This flag will be used to control the reconfig of MMR safety mechanism feature */
    /*
     * Note: The user/application must periodically trigger the control command
     * "TIVX_VPAC_MSC_CMD_SET_COEFF" along with "TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM".
     * This ensures that the MSC coefficients are reconfigured for every frame being processed.
     */
    uint32_t enable_reconfig_and_reinit_Reg;

    /* This flag will be used to control the status register validation safety mechanism feature */
    uint32_t enable_status_reg_validation;

    /* This flag will be used to control the readback of configuration registers for safety validation */
    uint32_t enable_readback_config_registers;

} tivx_vpac_msc_safety_mechanism_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_msc Target
 * \ingroup group_vision_function_vpac_msc
 */
void tivxRegisterHwaTargetVpacMscKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_msc Target
 * \ingroup group_vision_function_vpac_msc
 */
void tivxUnRegisterHwaTargetVpacMscKernels(void);

/*! \brief Creates a VPAC_MSC Node with multi-scale outputs (at most 5 outputs).
 * \param [in] graph The reference to the graph.
 * \param [in] in_img The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 *             <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 * \param [out] out0_img The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 * \param [out] out1_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 * \param [out] out2_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 * \param [out] out3_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 * \param [out] out4_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 *
 * \see <tt>TIVX_KERNEL_VPAC_MSC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_msc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscScaleNode(vx_graph graph,
                                      vx_image             in_img0,
                                      vx_image             out0_img,
                                      vx_image             out1_img,
                                      vx_image             out2_img,
                                      vx_image             out3_img,
                                      vx_image             out4_img);

/*! \brief Creates a VPAC_MSC Node with multi-scale outputs (at most 10 outputs).
 *  This node takes at most 2 single plane inputs and can simultaneously scale them across the outputs
 *  This node should adhere to the following rules:
 *  When in_img0 is NV12, 
 *  - in_img1 should be NULL.
 *  - If the outputs are also NV12, a maximum of five NV12 outputs can be generated.
 *  - is_enable_simul_processing flag can be set to either 0 or 1 to process the luma and chroma planes sequentially or simultaneously.
 *  When in_img1 is NULL, then all of the outputs map as scales of in_img0
 *  When in_img1 is not NULL, 
 *  - out0_img to out4_img map as scales of in_img0, and out5_img to out9_img map as scales of in_img1
 *  - the number of outputs should be even
 *  - if any output in the order out0, out1, out2, out3, or out4 is not null, the corresponding paired output out5, out6, out7, out8, or out9, respectively, should also not be null, and vice versa.
 *  - out0 to out4 and out5 to out9 must not have null gaps between non-null values. must both be non-null or both null
 *  - out0_img to out4_img should have the same format as in_img0, and 
 *  - out5_img to out9_img should have the same format as in_img1.
 *  - if in_img1_yc_mode is 0, the heights of in_img0 and in_img1 must be the same
 *  - if in_img1_yc_mode is 1, the height of in_img1 should be half the height of in_img0
 *  - regardless of the value of in_img1_yc_mode (0 or 1), in_img0 and in_img1 must have the same width.
 *  - is_enable_simul_processing flag should be set to 1 to process the both planes simultaneously, each with a different bit depth.
 *  All output images must be continuous. For example, if there are four outputs, 
 *  they should be assigned sequentially from out0 to out3 without any gaps.
 * \param [in] graph The reference to the graph.
 * \param [in] in_img0 The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 * \param [out] out0_img The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out1_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out2_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 *              format.
 * \param [out] out3_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out4_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [in] in_img1 (optional) The input image in
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or chorma),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *             format.
 * \param [out] out5_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out6_img (optional) The output image in
*              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out7_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out8_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 * \param [out] out9_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma or Chroma, based on value of \ref tivx_vpac_msc_input_params_t::yc_mode, when NV12 is input) or
 *                                           (8bit luma only when U8 is input) or (8bit luma or chroma, when input is non NV12),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma or chroma), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma or chroma)
 *              format.
 *
 * \see <tt>TIVX_KERNEL_VPAC_MSC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_msc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscScaleNode2(vx_graph graph,
                                      vx_image             in_img0,
                                      vx_image             out0_img,
                                      vx_image             out1_img,
                                      vx_image             out2_img,
                                      vx_image             out3_img,
                                      vx_image             out4_img,
                                      vx_image             in_img1,
                                      vx_image             out5_img,
                                      vx_image             out6_img,
                                      vx_image             out7_img,
                                      vx_image             out8_img,
                                      vx_image             out9_img);

/*! \brief Creates a VPAC_MSC Node with multi-scale pyramid output.
 *
 *         By default, a separable 5-tap gaussian filter is used with following
 *         coefficients for half scale pyramid : [ 16, 64, 96, 64, 16 ], and a 32 phase
 *         5-tap gaussian filter is used for non-half scale pyramids.
 *         These can be customized using \ref TIVX_VPAC_MSC_CMD_SET_COEFF command.
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_img The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 *             <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 * \param [out] out_pyramid The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt> (VX_DF_IMAGE_UYVY format support only for VPAC3 and VPAC3L)
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt> (VX_DF_IMAGE_YUYV format support only for VPAC3 and VPAC3L)
 *
 * \see <tt>TIVX_KERNEL_VPAC_MSC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_msc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscPyramidNode(vx_graph graph,
                                      vx_image             in_img,
                                      vx_pyramid           out_pyramid);


/*!
 * \brief Function to initialize MSC Coefficients Parameters
 *        This initializes Coefficients to default values.
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_COEFF
 *
 * \param coeff          [IN] Pointer to MSC coefficient structure
 * \param interpolation  [IN] Indicates interpolation method to initialize coefficients to
 *                            ( \ref VX_INTERPOLATION_BILINEAR or
 *                              \ref VX_INTERPOLATION_NEAREST_NEIGHBOR or
 *                              \ref TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE)
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_coefficients_params_init(tivx_vpac_msc_coefficients_t *coeff, vx_enum interpolation);

/*!
 * \brief Function to initialize MSC input Parameters
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS
 *
 * \param prms  [IN] Pointer to MSC input parameters
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_input_params_init(tivx_vpac_msc_input_params_t *prms);

/*!
 * \brief Function to initialize MSC output Parameters
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS
 *
 * \param prms  [IN] Pointer to MSC output parameters
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_output_params_init(tivx_vpac_msc_output_params_t *prms);

#if !defined(VPAC3L)
/*!
 * \brief Function to initialize MSC safety mechanism parameters with default values
 *
 * \param prms  [IN] Pointer to MSC safety mechanism parameter structure
 *
 * \ingroup group_vision_function_vpac_msc
 */

void tivx_vpac_msc_safety_mech_prms_init(tivx_vpac_msc_safety_mechanism_params_t *prms);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HWA_VPAC_MSC_H_ */

