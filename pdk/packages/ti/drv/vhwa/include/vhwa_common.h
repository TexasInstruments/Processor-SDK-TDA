/*
 *   Copyright (c) Texas Instruments Incorporated 2018
 *   All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 *  \defgroup DRV_VHWA_MODULE VHWA Driver
 *
 * The Vision Hardware Accelerator (VHWA) provides the logic to interface
 * for MSC, NF, LDC, VISS, DOF and SDE accelerators.
 * This is VHWA FVID2 driver documentation.
 */
/**
 *  \ingroup  DRV_VHWA_MODULE
 *  \defgroup COMMON_IMP Common
 *            Common configuration/API
 *
 *  @{
 */
/**
 *  \file   vhwa_common.h
 *
 *  \brief  File containning defination for common configuration/API
 *          used across all VHWA modules.
 */

#ifndef VHWA_CFG_COMMON_H_
#define VHWA_CFG_COMMON_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
/* Used to get FVID2 Base Driver ID */
#include <vhwa_vpac_abstraction_layer.h>



#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor VHWA_DRV_IDs
 *  \name   VHWA FVID2 Driver IDs
 *  \brief  FVID2 Driver IDs for VHWA Drivers
 *
 *  @{
 */
/** \brief FVID2 Driver Id for LDC Memory to Memory. */
#define FVID2_VHWA_M2M_LDC_DRV_ID        (FVID2_VHWA_DRV_BASE + 0x00U)

/** \brief FVID2 Driver Id for NF Memory to Memory. */
#define FVID2_VHWA_M2M_NF_DRV_ID         (FVID2_VHWA_M2M_LDC_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for MSC Memory to Memory. */
#define FVID2_VHWA_M2M_MSC_DRV_ID        (FVID2_VHWA_M2M_NF_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for VISS Memory to Memory. */
#define FVID2_VHWA_M2M_VISS_DRV_ID       (FVID2_VHWA_M2M_MSC_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for DOF Memory to Memory. */
#define FVID2_VHWA_M2M_DOF_DRV_ID        (FVID2_VHWA_M2M_VISS_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for SDE Memory to Memory. */
#define FVID2_VHWA_M2M_SDE_DRV_ID        (FVID2_VHWA_M2M_DOF_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for VISS-OTF Memory to Memory. */
#define FVID2_VHWA_M2M_VISS_OTF_DRV_ID   (FVID2_VHWA_M2M_SDE_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for Flex-connect Memory to Memory. */
#define FVID2_VHWA_M2M_FC_DRV_ID        (FVID2_VHWA_M2M_VISS_OTF_DRV_ID + 0x01U)

/** \brief FVID2 Driver Id for BIST driver. */
#define FVID2_VHWA_M2M_BIST_DRV_ID     (FVID2_VHWA_M2M_FC_DRV_ID + 0x01U)

/** @} */


#define VHWA_IOCTL_COMMON_IOCTL_BASE    (FVID2_VHWA_DRV_IOCTL_BASE)
#define VHWA_IOCTL_COMMON_IOCTL_NUM     (0x0000000AU)
#define VHWA_IOCTL_COMMON_IOCTL_END     (VHWA_IOCTL_COMMON_IOCTL_BASE +\
                                             VHWA_IOCTL_COMMON_IOCTL_NUM - \
                                             1U)

#define VHWA_IOCTL_NF_IOCTL_BASE        (VHWA_IOCTL_COMMON_IOCTL_END)
/**< NF Ioctl base */
#define VHWA_IOCTL_NF_IOCTL_NUM         (0x0000000AU)
/**< Number of control commands for NF */
#define VHWA_IOCTL_NF_IOCTL_END         (VHWA_IOCTL_NF_IOCTL_BASE + \
                                             VHWA_IOCTL_NF_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_LDC_IOCTL_BASE       (VHWA_IOCTL_NF_IOCTL_END)
/**< LDC Ioctl base */
#define VHWA_IOCTL_LDC_IOCTL_NUM        (0x0000000AU)
/**< Number of control commands for LDC */
#define VHWA_IOCTL_LDC_IOCTL_END        (VHWA_IOCTL_LDC_IOCTL_BASE + \
                                             VHWA_IOCTL_LDC_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_MSC_IOCTL_BASE       (VHWA_IOCTL_LDC_IOCTL_END)
/**< MSC Ioctl base */
#define VHWA_IOCTL_MSC_IOCTL_NUM        (0x0000000AU)
/**< Number of control commands for MSC */
#define VHWA_IOCTL_MSC_IOCTL_END        (VHWA_IOCTL_MSC_IOCTL_BASE + \
                                             VHWA_IOCTL_MSC_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_RFE_IOCTL_BASE       (VHWA_IOCTL_MSC_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_RFE_IOCTL_NUM        (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_RFE_IOCTL_END       (VHWA_IOCTL_RFE_IOCTL_BASE + \
                                             VHWA_IOCTL_RFE_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_FCP_IOCTL_BASE       (VHWA_IOCTL_RFE_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_FCP_IOCTL_NUM        (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_FCP_IOCTL_END       (VHWA_IOCTL_FCP_IOCTL_BASE + \
                                             VHWA_IOCTL_FCP_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_NSF4_IOCTL_BASE      (VHWA_IOCTL_FCP_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_NSF4_IOCTL_NUM       (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_NSF4_IOCTL_END       (VHWA_IOCTL_NSF4_IOCTL_BASE + \
                                             VHWA_IOCTL_NSF4_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_GLBCE_IOCTL_BASE      (VHWA_IOCTL_NSF4_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_GLBCE_IOCTL_NUM       (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_GLBCE_IOCTL_END       (VHWA_IOCTL_GLBCE_IOCTL_BASE + \
                                             VHWA_IOCTL_GLBCE_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_H3A_IOCTL_BASE        (VHWA_IOCTL_GLBCE_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_H3A_IOCTL_NUM         (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_H3A_IOCTL_END         (VHWA_IOCTL_H3A_IOCTL_BASE + \
                                               VHWA_IOCTL_H3A_IOCTL_NUM -  \
                                               1U)
#define VHWA_IOCTL_CAC_IOCTL_BASE      (VHWA_IOCTL_H3A_IOCTL_END)
/**< VISS Ioctl base */
#define VHWA_IOCTL_CAC_IOCTL_NUM       (0x0000000AU)
/**< Number of control commands for VISS */
#define VHWA_IOCTL_CAC_IOCTL_END       (VHWA_IOCTL_CAC_IOCTL_BASE + \
                                             VHWA_IOCTL_CAC_IOCTL_NUM -  \
                                             1U)

#define VHWA_IOCTL_DOF_IOCTL_BASE        (VHWA_IOCTL_CAC_IOCTL_END)
/**< DOF Ioctl base */
#define VHWA_IOCTL_DOF_IOCTL_NUM         (0x0000000AU)
/**< Number of control commands for DOF */
#define VHWA_IOCTL_DOF_IOCTL_END         (VHWA_IOCTL_DOF_IOCTL_BASE + \
                                               VHWA_IOCTL_DOF_IOCTL_NUM -  \
                                               1U)

#define VHWA_IOCTL_SDE_IOCTL_BASE        (VHWA_IOCTL_DOF_IOCTL_BASE)
/**< SDE Ioctl base */
#define VHWA_IOCTL_SDE_IOCTL_NUM         (0x0000000AU)
/**< Number of control commands for SDE */
#define VHWA_IOCTL_SDE_IOCTL_END         (VHWA_IOCTL_SDE_IOCTL_BASE + \
                                               VHWA_IOCTL_SDE_IOCTL_NUM -  \
                                               1U)

#define VHWA_IOCTL_M2M_MSC_IOCTL_BASE    (VHWA_IOCTL_DOF_IOCTL_END)
/**< MSC M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_MSC_IOCTL_NUM     (0x00000008U)
/**< Number of control commands for MSC M2M Driver */
#define VHWA_IOCTL_M2M_MSC_IOCTL_END     (VHWA_IOCTL_M2M_MSC_IOCTL_BASE + \
                                               VHWA_IOCTL_M2M_MSC_IOCTL_NUM -  \
                                               1U)
/**< Last control command for MSC M2M Driver */

#define VHWA_IOCTL_M2M_LDC_IOCTL_BASE    (VHWA_IOCTL_M2M_MSC_IOCTL_END)
/**< LDC M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_LDC_IOCTL_NUM     (0x00000008U)
/**< Number of control commands for LDC M2M Driver */
#define VHWA_IOCTL_M2M_LDC_IOCTL_END     (VHWA_IOCTL_M2M_LDC_IOCTL_BASE + \
                                               VHWA_IOCTL_M2M_LDC_IOCTL_NUM -  \
                                               1U)
/**< Last control command for LDC M2M Driver */

#define VHWA_IOCTL_M2M_VISS_IOCTL_BASE   (VHWA_IOCTL_M2M_LDC_IOCTL_END)
/**< VISS M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_VISS_IOCTL_NUM    (0x0000000AU)
/**< Number of control commands for VISS M2M Driver */
#define VHWA_IOCTL_M2M_VISS_IOCTL_END    (VHWA_IOCTL_M2M_VISS_IOCTL_BASE + \
                                             VHWA_IOCTL_M2M_VISS_IOCTL_NUM -  \
                                               1U)
/**< Last control command for VISS M2M Driver */

#define VHWA_IOCTL_M2M_NF_IOCTL_BASE     (VHWA_IOCTL_M2M_VISS_IOCTL_END)
/**< NF M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_NF_IOCTL_NUM      (0x00000008U)
/**< Number of control commands for NF M2M Driver */
#define VHWA_IOCTL_M2M_NF_IOCTL_END      (VHWA_IOCTL_M2M_NF_IOCTL_BASE + \
                                             VHWA_IOCTL_M2M_NF_IOCTL_NUM -  \
                                               1U)
/**< Last control command for NF M2M Driver */

#define VHWA_IOCTL_M2M_DOF_IOCTL_BASE    (VHWA_IOCTL_M2M_NF_IOCTL_END)
/**< DOF M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_DOF_IOCTL_NUM     (0x00000010U)
/**< Number of control commands for DOF M2M Driver */
#define VHWA_IOCTL_M2M_DOF_IOCTL_END     (VHWA_IOCTL_M2M_DOF_IOCTL_BASE + \
                                             VHWA_IOCTL_M2M_DOF_IOCTL_NUM -  \
                                               1U)
/**< Last control command for DOF M2M Driver */

#define VHWA_IOCTL_M2M_SDE_IOCTL_BASE    (VHWA_IOCTL_M2M_DOF_IOCTL_END)
/**< SDE M2M Driver Ioctl base */
#define VHWA_IOCTL_M2M_SDE_IOCTL_NUM     (0x00000008U)
/**< Number of control commands for SDE M2M Driver */
#define VHWA_IOCTL_M2M_SDE_IOCTL_END     (VHWA_IOCTL_M2M_SDE_IOCTL_BASE + \
                                             VHWA_IOCTL_M2M_SDE_IOCTL_NUM -  \
                                               1U)
/**< Last control command for SDE M2M Driver */

#define VHWA_IOCTL_FC_IOCTL_BASE        (VHWA_IOCTL_M2M_SDE_IOCTL_END)
/**< Flex-connect Driver Ioctl base */
#define VHWA_IOCTL_FC_IOCTL_NUM         (0x00000010U)
/**< Number of control commands for Flex-connect Driver */
#define VHWA_IOCTL_FC_IOCTL_END         (VHWA_IOCTL_FC_IOCTL_BASE + \
                                             VHWA_IOCTL_FC_IOCTL_NUM -  \
                                               1U)
/**< Last control command for Flex-connect Driver */

#define VHWA_IOCTL_FC_VISS_IOCTL_BASE    (VHWA_IOCTL_FC_IOCTL_END)
/**< Flex-connect Driver Ioctl base */
#define VHWA_IOCTL_FC_VISS_IOCTL_NUM     (0x0000004U)
/**< Number of control commands for Flex-connect Driver */
#define VHWA_IOCTL_FC_VISS_IOCTL_END     (VHWA_IOCTL_FC_VISS_IOCTL_BASE + \
                                             VHWA_IOCTL_FC_VISS_IOCTL_NUM -  \
                                               1U)
/**< Last control command for VISS used by Flex-connect Driver */

#define VHWA_IOCTL_FC_MSC_IOCTL_BASE    (VHWA_IOCTL_FC_VISS_IOCTL_END)
/**< Flex-connect Driver Ioctl base */
#define VHWA_IOCTL_FC_MSC_IOCTL_NUM     (0x0000004U)
/**< Number of control commands for Flex-connect Driver */
#define VHWA_IOCTL_FC_MSC_IOCTL_END     (VHWA_IOCTL_FC_MSC_IOCTL_BASE + \
                                             VHWA_IOCTL_FC_MSC_IOCTL_NUM -  \
                                               1U)
/**< Last control command for MSC used by Flex-connect Driver */

/**
 *  \anchor VHWA_IrqNum
 *  \name   VHWA Output IRQ Numbers
 *  \brief  VHWA VPAC module outputs 6 IRQs and DMPAC outputs 2 IRQs.
 *          These Irq numbers should be passed as init parameters
 *          along with the core irq number to enable them in INTD module.
 *
 *  @{
 */
#define VHWA_M2M_IRQ_NUM_0                               (0u)
#define VHWA_M2M_IRQ_NUM_1                               (1u)
#define VHWA_M2M_IRQ_NUM_2                               (2u)
#define VHWA_M2M_IRQ_NUM_3                               (3u)
#define VHWA_M2M_IRQ_NUM_4                               (4u)
#define VHWA_M2M_IRQ_NUM_5                               (5u)

#define VHWA_M2M_DMPAC_IRQ_NUM_0                         (0u)
#define VHWA_M2M_DMPAC_IRQ_NUM_1                         (1u)

#if !defined(MCU_PLUS_SDK)
/** \brief Log enable for LDC module */
#define VhwaLdcTrace                            ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for DOF module */
#define VhwaDofTrace                            ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for SDE module */
#define VhwaSdeTrace                            ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for NF module */
#define VhwaNfTrace                             ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for MSC module */
#define VhwaMscTrace                            ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for VISS module */
#define VhwaVissTrace                           ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for FC module */
#define VhwaFcTrace                            ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
/** \brief Log enable for BIST module */
#define VhwaM2mBistTrace                      ((uint32_t) GT_ERR   |\
                                                (uint32_t) GT_TraceState_Enable)
#endif
/** @} */

/* ========================================================================== */
/*                        Hardware IP PID Definitions                         */
/* ========================================================================== */

/**
 *  \anchor VHWA_HwIpPid
 *  \name   VHWA Hardware IP Product IDs
 *  \brief  Product IDs for VHWA hardware IP blocks.
 *  @{
 */

/** \brief Product ID for VISS IP */
#if defined(SOC_J722S) || defined(SOC_AM62A)
#define VHWA_HW_PID_VISS      (0x64C40200U)
/** \brief Product ID for LDC IP */
#define VHWA_HW_PID_LDC       (0x64C60002U)
#else
#define VHWA_HW_PID_LDC       (0x64C60001U)
#define VHWA_HW_PID_VISS      (0x64C40100U)
#endif
/** \brief Product ID for MSC IP */
#define VHWA_HW_PID_MSC       (0x64C20100U)
/** \brief Product ID for NF IP */
#define VHWA_HW_PID_NF        (0x64C81900U)
/** \brief Product ID for DOF IP */
#define VHWA_HW_PID_DOF       (0x64900100U)
/** \brief Product ID for SDE IP */
#define VHWA_HW_PID_SDE       (0x64840100U)
/** \brief Product ID for VPAC IP */
#if defined(SOC_J722S) || defined(SOC_AM62A)
#define VHWA_HW_PID_VPAC      (0x64C00400U)
#else
#define VHWA_HW_PID_VPAC      (0x64C00300U)
#endif
/** \brief Product ID for DMPAC IP */
#if defined(SOC_J722S)
#define VHWA_HW_PID_DMPAC     (0x64800200U)  /* Different PID for J722S */
#else
#define VHWA_HW_PID_DMPAC     (0x64801100U)  /* Original PID for J721S2/J784S4 */
#endif
#define VHWA_HW_PID_H3A       (0x4D010000U)


/** @} */


/**
 *  \anchor VHWA_WdTimeOutCycles
 *  \name   VHWA WDTimeout cycles
 *  \brief  Enum to select WD Timeout cycles.
 *
 *  @{
 */
#define VHWA_WDTIMEOUT_64K_CYCLES                       (0u)
#define VHWA_WDTIMEOUT_128K_CYCLES                      (1u)
/** @} */


/** MACRO to be defined to start all modules together */
// #define VHWA_USE_PIPELINE_COMMON_ENABLE (1U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**< Function pointer for getting time stamp */
typedef uint64_t (*Vhwa_GetTimeStamp) (void);

/**
 *  struct Vhwa_LutConfig
 *  \brief Companding Lut configuration Structure
 *         LUT based companding is used at many places in RAWFE
 *         This structure is common and used for configuring these Luts.
 */
typedef struct
{
    uint32_t                    enable;
    /**< Flag to enable Lut */
    uint32_t                    inputBits;
    /**< Number of input bits */
    uint32_t                    clip;
    /**< Output Clip value */
    uint32_t                   *tableAddr;
    /**< Pointer to the Lut Address,
         must not be null if enable is set to UTRUE */
} Vhwa_LutConfig;

/**
 *  struct Vhwa_FocoPrms
 *  \brief These are advanced parameters adn should not be modified in most cases.
 *          Used by DMPAC module
 */
typedef struct
{
    uint32_t shiftM1;
    /**< Shift count */
    uint32_t dir;
    /**< Shift direction */
    uint32_t round;
    /**< Round */
} Vhwa_FocoPrms;

/**
 *  struct Vhwa_HtsLimiter
 *  \brief Bandwith limiter configuration for HTS.
 *         This is used to slowed down the HTS by introducing clock cycles
 *         between internal start signals.
 *         These are advanced parameters and should not be
 *         modified in most cases.
 */
typedef struct
{
    uint32_t                    enableBwLimit;
    /**< Enable Bandwidth Limiter */
    uint32_t                    cycleCnt;
    /**< Average Cycle count between successive Thread Start */
    uint32_t                    tokenCnt;
    /**< Max Token count to create average BW */
} Vhwa_HtsLimiter;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 *  \brief Function to width of image in Bytes.
 *
 *  \param width    Width of Image in Pixels
 *  \param ccsf     Image Storeage format. Ref to #Fvid2_ColorCompStorageFmt.
 *                  Only Following format are supported
 *                  - FVID2_CCSF_BITS8_PACKED
 *                  - FVID2_CCSF_BITS12_PACKED
 *                  - FVID2_CCSF_BITS8_UNPACKED16_MSB_ALIGNED
 *                  - FVID2_CCSF_BITS8_UNPACKED16
 *                  - FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED
 *                  - FVID2_CCSF_BITS12_UNPACKED16
 *
 *  \return Image width in Bytes
 */
uint32_t Vhwa_calcHorzSizeInBytes(uint32_t width, uint32_t ccsf);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /** VHWA_CFG_COMMON_H_ */
 /** @} */
