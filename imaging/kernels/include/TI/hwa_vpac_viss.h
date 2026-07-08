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

#ifndef HWA_VPAC_VISS_H_
#define HWA_VPAC_VISS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#include <TI/hwa_vpac_viss_fcp.h>

#ifdef VPAC3
#include <TI/hwa_vpac_viss3.h>
#elif defined (VPAC3L)
#include <TI/hwa_vpac_viss3l.h>
#else
#include <TI/hwa_vpac_viss1.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS kernels in this kernel extension.
 */

/*! \brief vpac_viss kernel name
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_KERNEL_VPAC_VISS_NAME     "com.ti.hwa.vpac_viss"

/*********************************
 *      VPAC_VISS Control Commands
 *********************************/

/*! \brief Control Command to set the DCC (Dynamic Camera Configuration)
 *         information to the given VISS Node.
 *
 *         Viss node gets the pointer to DCC buffer containing
 *         VISS configuration. It uses DCC parser to parse and
 *         map DCC parameters into VISS configuration and sets it
 *         in the driver.
 *
 *         Note that if the module is bypassed in tivx_vpac_viss_params_t
 *         during node creating, the parameter will not be set for
 *         this module, even through DCC profile has config for
 *         the same module.
 *
 *         User data object containing DCC buffer is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS                   (0x30000000u)
/*! \brief Control Command to get the combined error status
 *         Returns the combined error status of the last processed frame,
 *         incorporating error interrupts and watchdog timer error interrupt. 
 *         A reference to vx_scalar is passed as an argument with 
 *         this control command.
 *         The node returns a bit-mask of the error status in the 
 *         U32 variable of vx_scalar. 
 *
 *         - Error interrupt status bits (0–29) are directly 
 *           taken from the viss_obj->err_stat variable.
 *         - If watchdog timer error is detected (bit 20 in wd_err_status),
 *           this is flagged by setting bit 30 in the returned error status. 
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_GET_ERR_STATUS                    (0x30000001u)

/*! \brief Control Command to retrieve the PSA signature values 
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_GET_PSA_STATUS                    (0x30000002u)

#if defined (LDRA_COVERAGE)
/*! \brief Control Command to stop the LDRA coverage data capture for the VPAC modules
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_COVERAGE_END                    (0x30000003u)
#endif /* LDRA_COVERAGE */

/*! \brief Control Command to enable/disable error events
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_ENABLE_ERROR_EVENTS               (0x30000004u)
/*! \brief Control Command to enable VPAC VISS safety mechanism
 *         User data object tivx_vpac_viss_safety_mechanism_params_t is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_CMD_ENABLE_SAFETY_MECHANISM   (0x30000005u)

/*********************************
 *      VPAC_VISS Defines
 *********************************/

/*! \brief Maximum H3A number of bytes in statistics data array
 *
 *   Allocating 24KB buffer which is large enough for 32x32 windows
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES            (24576U)

/*! \brief The H3A output memory address alignment
 *
 *   The start address of the tivx_h3a_data_t::data buffer must be aligned
 *   to this value, so the tivx_h3a_data_t::resv field is used
 *   along with this value to properly pad the structure to enable this
 *   alignment.  \see tivx_h3a_data_t::resv
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN  (64U)

/*!
 * \defgroup group_vision_function_vpac_viss_h3a Enumerations
 * \brief Enumerations for configuration parameters in VPAC VISS structures
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW0     (0U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW1     (1U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_RAW2     (2U)
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_LSC      (3U)
#ifdef VPAC3L
/*! \see tivx_vpac_viss_params_t::h3a_in and tivx_ae_awb_params_t::h3a_source_data and tivx_h3a_data_t::h3a_source_data */
#define TIVX_VPAC_VISS_H3A_IN_PCID     (4U)
#endif
/*! \see tivx_vpac_viss_params_t::h3a_mode and tivx_h3a_data_t::aew_af_mode */
#define TIVX_VPAC_VISS_H3A_MODE_AEWB   (0U)
/*! \see tivx_vpac_viss_params_t::h3a_mode and tivx_h3a_data_t::aew_af_mode */
#define TIVX_VPAC_VISS_H3A_MODE_AF     (1U)
/*@}*/

/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP0            (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP1            (1U)

/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT0        (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT1        (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT2        (1U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT3        (1U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT4        (0U)

/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_DISABLED    (0U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_RFE   (1U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_CAC   (2U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_NSF4  (3U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_GLBCE (4U)

/*! Macro to set the output_fcp_mapping
    \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_MAP_FCP_OUTPUT(fcp, output)   (output<<1 | fcp)

/*!
 * \defgroup group_vision_function_vpac_viss_wdtimer Enumerations
 * \brief Enumerations for VPAC VISS Watchdog Timer configuration
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \brief Disable Watchdog Timer (default) */
#define TIVX_VPAC_VISS_WDTIMER_DISABLE      (0u)
/*! \brief Enable Watchdog Timer */
#define TIVX_VPAC_VISS_WDTIMER_ENABLE       (1u)
/*! \brief Watchdog Timer timeout: 64K cycles (default) */
#define TIVX_VPAC_VISS_WDTIMER_64K_CYCLES   (0u)
/*! \brief Watchdog Timer timeout: 128K cycles */
#define TIVX_VPAC_VISS_WDTIMER_128K_CYCLES  (1u)
/*@}*/

/*!
 * \defgroup group_vision_function_vpac_viss_error Enumerations
 * \brief Enumerations for VPAC VISS error events
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \brief Config read or write memory acccess occurred during functional operation and
 *        likely corrupted functional opertion. VISS merges all config error sources from
 *        RawFE and refer to RawFE spec for the entire error source list */
#define TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR                 (0x1U)
/*! H3A AEW interrupt */
#define TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR               (0x2U)
/*! H3A AF interrupt */
#define TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR                (0x4U)
/*! H3A interrupt */
#define TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR               (0x8U)
/*! H3A output buffer overflow */
#define TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR   (0x10U)
/*! VBUSP diagnostic read access of, while NSF data using RAM for functional purpose */
#define TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR         (0x20U)
/*! Horzontal Blanking too short between lines */
#define TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR              (0x40U)
/*! Vertical Blanking too short between frames */
#define TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR              (0x80U)
/*! Either non-shadowed registers written or statastic memories are accessed during active window */
#define TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR                 (0x100U)
/*! GLBCE started filtering. This interrupt is issued at the rising edge of filtering signal */
#define TIVX_VPAC_VISS_GLBCE_FILT_START_INTR              (0x200U)
/*! GLBCE ended filtering. This interrupt is issued at the falling edge of filtering signal */
#define TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR               (0x400U)
/*! Generated when delayed HS/HE signals does not match with derived signals from GLBCE core.*/
#define TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR               (0x800U)
/*! Generated when delayed VS/VE signals does not match with derived signals from GLBCE core */
#define TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR               (0x1000U)
/*! This interrupt is issued, if there is a data input while filtering is high */
#define TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR                  (0x2000U)
/*! Either non-shadowed registers written or line memories are accessed during active window */
#define TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR                  (0x4000U)
/*! Configuration access to registers/memories has corrupted functional operation. 
 * Configuration read or write memory acccess occurred during functional operation.
 * Merged independent error sources at VISS. Refer to Section VISS Flexible Color
   Processing (FCP) for all sources. */
#define TIVX_VPAC_VISS_FCC_CFG_ERR_INTR                   (0x8000U)
/*! FIFO overflow on FIFO for Y12 LSE I/F. Merged all FCC output overflow error
 * sources at VISS. Refer to Section VISS Flexible Color Processing (FCP) for all sources. */
#define TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR             (0x10000U)
/*! Host was not able to read the entire histogram mem between VS-VE window
 * (triggered when the first access to histogram has been performed but the last has
 * not been performed). */
#define TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR             (0x20000U)
/*! Configuration happened to EE regions causing corruption during frame processing */
#define TIVX_VPAC_VISS_EE_CFG_ERR                         (0x40000U)
/*! EE horizontal synchronization FIFO overflow interrupt */
#define TIVX_VPAC_VISS_EE_SYNCOVF_ERR                     (0x80000U)
/*! LSE frame done interrupt */
#define TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR               (0x100000U)
/*! Set whenever there is an error response on VBUSM read command for any input channel. */
#define TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR                (0x200000U)
/*! Set whenever there is an error response on VBUSM write command for any output channel. */
#define TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR                (0x400000U)
/*! Set whenever one of the following input frame errors is detected at VPORT_INPUT */
#define TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR                (0x800000U)
/**<brief Output Frame Start (from VISS top level) */
#define TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR          (0x1000000U)
#if defined(VPAC3) || defined(VPAC3L)
/**<brief x,y pixel position has reached the start of rawfe pipeline interrupt*/
#define TIVX_VPAC_VISS_RAWFE_X_Y_POINTER                  (0x2000000U)
/**<brief Config read/write access happened to CAC MeshLUT when CAC data is active 
 * (active window - IN_VS to OUT_VE)*/
#define TIVX_VPAC_VISS_CR_CFG_ERR                         (0x4000000U)
/**<brief Triggered when the first access to statastics memory has been performed 
 * but the last has not been performed*/
#endif
#if defined(VPAC3L)
#define TIVX_VPAC_VISS_RAWFE_DPC_STATS_READ_ERR           (0x8000000U)
#endif
/*! Watchdog Timeout Error for VISS */
#define TIVX_VPAC_VISS_WDTIMER_ERR                       (0x40000000U)

/*! Macro to enable all VISS error events (including platform-specific) */
#if defined(VPAC1)
#define TIVX_VPAC_VISS_ALL_ERR_EVENTS ( \
    TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR | \
    TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR | \
    TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR | \
    TIVX_VPAC_VISS_EE_CFG_ERR | \
    TIVX_VPAC_VISS_EE_SYNCOVF_ERR | \
    TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR | \
    TIVX_VPAC_VISS_WDTIMER_ERR \
)
#define TIVX_VPAC_VISS_ALL_ERR_AND_INFO_EVENTS ( \
    TIVX_VPAC_VISS_ALL_ERR_EVENTS | \
    TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_START_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR | \
    TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR | \
    TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR \
)
#elif defined(VPAC3)
#define TIVX_VPAC_VISS_ALL_ERR_EVENTS ( \
    TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR | \
    TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR | \
    TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR | \
    TIVX_VPAC_VISS_EE_CFG_ERR | \
    TIVX_VPAC_VISS_EE_SYNCOVF_ERR | \
    TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR | \
    TIVX_VPAC_VISS_CR_CFG_ERR | \
    TIVX_VPAC_VISS_WDTIMER_ERR \
)
#define TIVX_VPAC_VISS_ALL_ERR_AND_INFO_EVENTS ( \
    TIVX_VPAC_VISS_ALL_ERR_EVENTS | \
    TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_START_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR | \
    TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR | \
    TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR | \
    TIVX_VPAC_VISS_RAWFE_X_Y_POINTER \
)
#elif defined(VPAC3L)
#define TIVX_VPAC_VISS_ALL_ERR_EVENTS ( \
    TIVX_VPAC_VISS_RAWFE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_BUF_OVRFLOW_PULSE_INTR | \
    TIVX_VPAC_VISS_NSF4V_LINEMEM_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_HBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_NSF4V_VBLANK_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_HSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VSYNC_ERR_INTR | \
    TIVX_VPAC_VISS_GLBCE_VP_ERR_INTR | \
    TIVX_VPAC_VISS_FCFA_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_CFG_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_OUTIF_OVF_ERR_INTR | \
    TIVX_VPAC_VISS_FCC_HIST_READ_ERR_INTR | \
    TIVX_VPAC_VISS_EE_CFG_ERR | \
    TIVX_VPAC_VISS_EE_SYNCOVF_ERR | \
    TIVX_VPAC_VISS_LSE_SL2_RD_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_SL2_WR_ERR_INTR | \
    TIVX_VPAC_VISS_LSE_CAL_VP_ERR_INTR | \
    TIVX_VPAC_VISS_CR_CFG_ERR | \
    TIVX_VPAC_VISS_RAWFE_DPC_STATS_READ_ERR | \
    TIVX_VPAC_VISS_WDTIMER_ERR \
)
#define TIVX_VPAC_VISS_ALL_ERR_AND_INFO_EVENTS ( \
    TIVX_VPAC_VISS_ALL_ERR_EVENTS | \
    TIVX_VPAC_VISS_RAWFE_AEW_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_AF_PULSE_INTR | \
    TIVX_VPAC_VISS_RAWFE_H3A_PULSE_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_START_INTR | \
    TIVX_VPAC_VISS_GLBCE_FILT_DONE_INTR | \
    TIVX_VPAC_VISS_LSE_FR_DONE_EVT_INTR | \
    TIVX_VPAC_VISS_LSE_OUT_FR_START_EVT_INTR | \
    TIVX_VPAC_VISS_RAWFE_X_Y_POINTER \
)
#endif

/*@}*/


/*********************************
 *      VPAC_VISS STRUCTURES
 *********************************/

/*!
 * \brief Data corresponding to results of 2A algorithm
 *
 *  \details This is the data structure used for the \a ae_awb_result parameter of
 *  \ref tivxVpacVissNode.  This is used by the VISS to index into the
 *  appropriate photospace in the DCC database.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Indicates the source data corresponding to this data:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW0     | H3A Input is from RAW0           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW1     | H3A Input is from RAW1           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW2     | H3A Input is from RAW2           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_LSC      | H3A Input is from LSC            |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_PCID     | H3A Input is from PCID           |
     *  \ref TIVX_VPAC_VISS_H3A_IN_PCID is applicable for only VPAC3L
     */
    uint32_t                    h3a_source_data;
    /*! Measured in micro seconds (us) */
    uint32_t                    exposure_time;
    /*! Analog Gain */
    uint32_t                    analog_gain;
    /*! Is AE output valid? */
    uint32_t                    ae_valid;
    /*! Is AE converged? */
    uint32_t                    ae_converged;
    /*! Digital Gain */
    uint32_t                    digital_gain;
    /*! White Balance Gains */
    uint32_t                    wb_gains[4];
    /*! White Balance Offsets */
    int32_t                     wb_offsets[4];
    /*! Color Temperature (K) */
    uint32_t                    color_temperature;
    /*! Is AWB output valid? */
    uint32_t                    awb_valid;
    /*! Is AWB converged? */
    uint32_t                    awb_converged;
} tivx_ae_awb_params_t;

/*!
 * \brief H3A AEW configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \details This is included in the \ref tivx_h3a_data_t H3A output if the
 * AEWB mode of H3A was enabled.  This is included in case
 * the 2A algorithm needs to know the configuration that the VISS used for the
 * H3A AEWB engine.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct
{
    /*! AEW Window Height */
    uint16_t                    aewwin1_WINH;
    /*! AEW Window Width */
    uint16_t                    aewwin1_WINW;
    /*! AEW Vertical Count */
    uint16_t                    aewwin1_WINVC;
    /*! AEW Horizontal Count */
    uint16_t                    aewwin1_WINHC;
    /*! AEW Subwindow Vertical Increment Value */
    uint16_t                    aewsubwin_AEWINCV;
    /*! AEW Subwindow Horizontal Increment Value */
    uint16_t                    aewsubwin_AEWINCH;
} tivx_h3a_aew_config;

/*!
 * \brief The \a h3a_output data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \details This is used for the \a h3a_output parameter of \ref tivxVpacVissNode
 *
 * \note Must be allocated at 64 byte boundary.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Indicates the contents of this buffer:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AEWB   | AEWB                             |
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AF     | AF                               |
     */
    uint32_t                    aew_af_mode;
    /*! Indicates the source data corresponding to this data:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW0     | H3A Input is from RAW0           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW1     | H3A Input is from RAW1           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW2     | H3A Input is from RAW2           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_LSC      | H3A Input is from LSC            |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_PCID     | H3A Input is from PCID           |
     *  \ref TIVX_VPAC_VISS_H3A_IN_PCID is applicable for only VPAC3L
     */
    uint32_t                    h3a_source_data;
    /*! If aew_af_mode == TIVX_VPAC_VISS_H3A_MODE_AEWB, this is the aew HW configuration used */
    tivx_h3a_aew_config         aew_config;
    /*! Identifier for cpu ID that the VISS node is running on.
     *  Currently used only for notifying AEWB node which cpu to send the update to based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                    cpu_id;
    /*! Identifier for camera channel ID.
     *  Currently used only for notifying AEWB node which channel to update based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                    channel_id;
    /*! Total used size of the data buffer in bytes */
    uint32_t                    size;
    /*! Reserved dummy field to make data to be 64 byte aligned */
    uint32_t                    resv[(TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN-(sizeof(tivx_h3a_aew_config)+20U))/4U ];
    /*! Payload of the AEW or AF data */
    uint8_t                     data[TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES];
} tivx_h3a_data_t;

/*!
 * \brief Safety parameters structure for VPAC VISS.
 *
 * This structure can be used to configure safety-related features
 * such as error event monitoring and status register validation for the
 * VISS module.
 *
 * The following safety mechanisms can be enabled by setting the individual
 * parameters:
 * - enable_reconfig_and_reinit_Reg: Enables the reconfiguration and
 *   reinitialization of the registers safety mechanism feature.
 * - enable_status_reg_validation: Enables the status register validation
 *   safety mechanism feature.
 * - enable_readback_config_registers: Enables the readback of
 *   configuration registers for safety validation.
 *
 * Operation modes for safety flags:
 * - 0 = disabled: The feature is turned off.
 * - 1 = one-shot: The feature operates once per trigger.
 * - 2 = continuous: The feature operates continuously until stopped.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {

    /* This flag will be used to control the reconfig of Registers safety mechanism feature */
    uint32_t enable_reconfig_and_reinit_Reg;

    /* This flag will be used to control the status register validation safety mechanism feature */
    uint32_t enable_status_reg_validation;

    /* This flag will be used to enable configuration register readback validation safety mechanism feature */
    uint32_t enable_readback_config_registers;

    /* 32-bit address of the configuration register readback data */
    uint32_t config_reg_readback_addr;

} tivx_vpac_viss_safety_mechanism_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_vpac_viss
 */
void tivxRegisterHwaTargetVpacVissKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_vpac_viss
 */
void tivxUnRegisterHwaTargetVpacVissKernels(void);

/*!
 * \brief Function to initialize VISS Parameters
 *
 * \param prms  [in] Pointer to VISS parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_vpac_viss_params_init(tivx_vpac_viss_params_t *prms);

/*!
 * \brief Function to initialize H3A data Parameters
 *
 * \param prms  [in] Pointer to H3A data parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_h3a_data_init(tivx_h3a_data_t *prms);

/*!
 * \brief Function to initialize AEWB Output Parameters
 *        These parameters come from the AEWB algorithm.
 *
 * \param prms  [in] Pointer to AEWB output parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_ae_awb_params_init(tivx_ae_awb_params_t *prms);

/*!
 * \brief Function to initialize H3A aew Config
 *
 * \param prms  [in] Pointer to H3A aew config
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_h3a_aew_config_init(tivx_h3a_aew_config *prms);

#if !defined(VPAC3L)
/*!
 * \brief Function to initialize VISS safety mechanism parameters with default values
 *
 * \param prms  [IN] Pointer to VISS safety mechanism parameter structure
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_vpac_viss_safety_mech_prms_init(
    tivx_vpac_viss_safety_mechanism_params_t *prms);

#endif

/*! \brief [Graph] Creates a VPAC_VISS Node.
 * \ingroup group_vision_function_vpac_viss
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacVissNode(vx_graph  graph,
                                      vx_user_data_object   configuration,
                                      vx_user_data_object   ae_awb_result,
                                      vx_user_data_object   dcc_buf,
                                      tivx_raw_image        raw,
                                      vx_image              output0,
                                      vx_image              output1,
                                      vx_image              output2,
                                      vx_image              output3,
                                      vx_image              output4,
                                      vx_user_data_object   h3a_output,
                                      vx_distribution       histogram0,
                                      vx_distribution       histogram1,
                                      vx_distribution       raw_histogram);

#ifdef __cplusplus
}
#endif

#endif /* HWA_VPAC_VISS_H_ */

