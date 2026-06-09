/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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
 *  \defgroup SAFETY_CHECKERS_PM PM Safety Checkers Library
 *  \ingroup  PM SAFETY CHECKERS
 */

/**
 *  \file     safety_checkers_soc.h
 *
 *  \brief    This file contains J722S specific data structures for safety checkers
 *
 */

#ifndef SAFETY_CHECKERS_SOC_H_
#define SAFETY_CHECKERS_SOC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "safety_checkers_pm_soc.h"
#include "safety_checkers_rm_soc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup SAFETY_CHECKERS_PM
 *
 * @defgroup SAFETY_CHECKERS_PM_SOC_MACROS  SAFETY_CHECKERS_PM PM safety checkers SOC specific macro definition
 *  @{
 */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief PLL and PSC base addresses */
#define SAFETY_CHECKERS_PM_PSC_BASE_ADDRESS                       (0x400000UL)
#define SAFETY_CHECKERS_PM_PLL_CFG_BASE_ADDRESS                   (0x680000UL)
#define SAFETY_CHECKERS_PM_MCU_PLL_CFG_BASE_ADDRESS               (0x4040000UL)

/** \brief PD STAT and MD STAT registers details for PSC */
#define SAFETY_CHECKERS_PM_WKUP_PD_STAT_NUM                       (0x02U)
#define SAFETY_CHECKERS_PM_WKUP_MD_STAT_NUM                       (0x0BU)
#define SAFETY_CHECKERS_PM_PD_STAT_NUM                            (0x16U)
#define SAFETY_CHECKERS_PM_MD_STAT_NUM                            (0x60U)

/** \brief PLL register details */
#define SAFETY_CHECKERS_PM_PLL0_LENGTH                            (0xA8U)
#define SAFETY_CHECKERS_PM_PLL1_LENGTH                            (0x9CU)
#define SAFETY_CHECKERS_PM_PLL2_LENGTH                            (0xA8U)
#define SAFETY_CHECKERS_PM_PLL5_LENGTH                            (0x8CU)
#define SAFETY_CHECKERS_PM_PLL6_LENGTH                            (0x84U)
#define SAFETY_CHECKERS_PM_PLL7_LENGTH                            (0x88U)
#define SAFETY_CHECKERS_PM_PLL8_LENGTH                            (0x84U)
#define SAFETY_CHECKERS_PM_PLL12_LENGTH                           (0x84U)
#define SAFETY_CHECKERS_PM_PLL15_LENGTH                           (0x90U)
#define SAFETY_CHECKERS_PM_PLL16_LENGTH                           (0x84U)
#define SAFETY_CHECKERS_PM_PLL17_LENGTH                           (0x84U)
#define SAFETY_CHECKERS_PM_PLL18_LENGTH                           (0x84U)
#define SAFETY_CHECKERS_PM_MCU_PLL0_LENGTH                        (0x9CU)

/** \brief Maximum number of firewalls that can be accessed */
#define TIFS_CHECKER_FWL_MAX_NUM                                  (0x14U)

/**
 * \brief  Total register dump size for PSC.
 *         This has been calculated by the addition of PD STAT and MD STAT registers.
 */
#define SAFETY_CHECKERS_PM_PSC_REGDUMP_SIZE                       (SAFETY_CHECKERS_PM_WKUP_PD_STAT_NUM + \
                                                                   SAFETY_CHECKERS_PM_WKUP_MD_STAT_NUM + \
                                                                   SAFETY_CHECKERS_PM_PD_STAT_NUM + \
                                                                   SAFETY_CHECKERS_PM_MD_STAT_NUM)

/**
 * \brief  Total register dump size for PLL.
 *         This has been calculated by iterating through each element in SafetyCheckers_PmPllData
 *         array. Within the loop, another loop calculates the total size by incrementing
 *         totalLength until the length indicator matches the corresponding SafetyCheckers_PLL_regOffset
 *         in the array. The final result is the total size of register offsets of each PLLs
 *         stored in the totalLength variable.
 */
#define SAFETY_CHECKERS_PM_PLL_REGDUMP_SIZE                       (179U)

/**
 * \brief  Total register dump size for RM.
 *         This has been calculated by iterating through each element in gSafetyCheckers_RmRegData
 *         array. Within the loop, calculates the total talLength by multiplying the regNum and regArrayLen
 *         in the array. The final result is the total size of register array for RM module registers.
 */
#define SAFETY_CHECKERS_RM_REGDUMP_SIZE                           (3471U)

/** \brief RM IR module base addresses */
#define SAFETY_CHECKERS_RM_BA0_IR						          (CSL_TIMESYNC_EVENT_INTROUTER0_INTR_ROUTER_CFG_BASE)
#define SAFETY_CHECKERS_RM_BA1_IR						          (CSL_WKUP_MCU_GPIOMUX_INTROUTER0_INTR_ROUTER_CFG_BASE)
#define SAFETY_CHECKERS_RM_BA2_IR						          (CSL_MAIN_GPIOMUX_INTROUTER0_INTR_ROUTER_CFG_BASE)

/** \brief Formula input of IR module to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_IR_REG0_NUM	                          (26U)
#define SAFETY_CHECKERS_RM_IR_REG1_NUM	                          (16U)
#define SAFETY_CHECKERS_RM_IR_REG2_NUM	                          (36U)

/** \brief Number of registers in IR register group */
#define SAFETY_CHECKERS_RM_IR_SUBMOD0_NUM						  (1U)

/** \brief RM IAIMAP module base addresses */
#define SAFETY_CHECKERS_RM_BA0_IA_IMAP                            (CSL_DMASS0_INTAGGR_IMAP_BASE)
#define SAFETY_CHECKERS_RM_BA1_IA_IMAP                            (CSL_DMASS1_INTAGGR_IMAP_BASE)

/** \brief Formula input of IAIMAP module to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_REG0_IA_IMAP					          (1536U)
#define SAFETY_CHECKERS_RM_REG1_IA_IMAP					          (216U)

/** \brief Number of registers in IAIMAP register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_IA_IMAP					      (1U)

/** \brief RM RA module base addresses */
#define SAFETY_CHECKERS_RM_BA0_RA                                 (CSL_DMASS0_BCDMA_RING_BASE)
#define SAFETY_CHECKERS_RM_BA1_RA                                 (CSL_DMASS0_PKTDMA_RING_BASE)
#define SAFETY_CHECKERS_RM_BA2_RA                                 (CSL_DMASS0_RINGACC_CFG_BASE)
#define SAFETY_CHECKERS_RM_BA3_RA                                 (CSL_DMASS1_BCDMA_RING_BASE)

/** \brief Formula input of RA module to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_RA_REG0_NUM						      (82U)
#define SAFETY_CHECKERS_RM_RA_REG1_NUM						      (150U)
#define SAFETY_CHECKERS_RM_RA_REG2_NUM						      (20U)
#define SAFETY_CHECKERS_RM_RA_REG3_NUM						      (40U)

/** \brief Number of registers in RA register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_RA						      (3U)
#define SAFETY_CHECKERS_RM_RA_SUBMOD1						      (5U)

/** \brief RM UDMA TX module base addresses */
#define SAFETY_CHECKERS_RM_BA0_UDMA_TX                    		  (CSL_DMASS0_BCDMA_TCHAN_BASE)
#define SAFETY_CHECKERS_RM_BA1_UDMA_TX                    		  (CSL_DMASS0_PKTDMA_TCHAN_BASE)
#define SAFETY_CHECKERS_RM_BA2_UDMA_TX                    		  (CSL_DMASS1_BCDMA_TCHAN_BASE)

/** \brief Formula input of UDMA TX to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_REG0_UDMA_TX                           (25U)
#define SAFETY_CHECKERS_RM_REG1_UDMA_TX                           (29U)
#define SAFETY_CHECKERS_RM_REG2_UDMA_TX                           (8U)

/** \brief Number of registers in UDMA TX register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_UDMA_TX                   	  (5U)

/** \brief RM UDMA RX module base addresses */
#define SAFETY_CHECKERS_RM_BA0_UDMA_RX                    		  (CSL_DMASS0_BCDMA_RCHAN_BASE)
#define SAFETY_CHECKERS_RM_BA1_UDMA_RX                    		  (CSL_DMASS0_PKTDMA_RCHAN_BASE)
#define SAFETY_CHECKERS_RM_BA2_UDMA_RX                    		  (CSL_DMASS1_BCDMA_RCHAN_BASE)

/** \brief Formula input of UDMA RX to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_REG0_UDMA_RX                    		  (25U)
#define SAFETY_CHECKERS_RM_REG1_UDMA_RX                    		  (24U)
#define SAFETY_CHECKERS_RM_REG2_UDMA_RX                    		  (32U)

/** \brief Number of registers in UDMA RX register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_UDMA_RX                   	  (4U)

/** \brief RM UDMA FLOW module base addresses */
#define SAFETY_CHECKERS_RM_BA0_UDMA_FLW                  		  (CSL_DMASS0_PKTDMA_RFLOW_BASE)

/** \brief Formula input of UDMA FLOW to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_REG0_UDMA_FLW                  		  (51U)

/** \brief Number of registers in UDMA FLOW register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_UDMA_FLW                 	  (1U)

/** \brief RM UDMA GCFG module base addresses */
#define SAFETY_CHECKERS_RM_BA0_UDMA_GCFG                  		  (CSL_DMASS0_BCDMA_GCFG_BASE)
#define SAFETY_CHECKERS_RM_BA1_UDMA_GCFG                  		  (CSL_DMASS0_PKTDMA_GCFG_BASE)
#define SAFETY_CHECKERS_RM_BA2_UDMA_GCFG                  		  (CSL_DMASS1_BCDMA_GCFG_BASE)

/** \brief Formula input of UDMA GCFG to read relevant registers from register group */
#define SAFETY_CHECKERS_RM_REG0_UDMA_GCFG                  		  (1U)
#define SAFETY_CHECKERS_RM_REG1_UDMA_GCFG                  		  (1U)

/** \brief Number of registers in UDMA GCFG register group */
#define SAFETY_CHECKERS_RM_SUBMOD0_UDMA_GCFG                 	  (13U)
#define SAFETY_CHECKERS_RM_SUBMOD1_UDMA_GCFG                 	  (14U)

/** @} */

/**
 * @ingroup SAFETY_CHECKERS_PM
 *
 * @defgroup SAFETY_CHECKERS_PM_GLOBAL_VAR  SAFETY_CHECKERS_PM global variables declarations
 *  @{
 */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**
 *
 * \brief     This defines the array holding register offset values for the each PLL's
 *
 */

static uint32_t gSafetyCheckers_PmPllRegOffset0 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU,
 0x90U, 0x94U, 0x98U, 0x9cU, 0xA0U, 0xA4U, 0xA8U};

static uint32_t gSafetyCheckers_PmPllRegOffset1 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU,
 0x90U, 0x94U, 0x98U, 0x9cU};

static uint32_t gSafetyCheckers_PmPllRegOffset2 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU,
 0x90U, 0x94U, 0x98U, 0x9cU, 0xA0U, 0xA4U, 0xA8U};

static uint32_t gSafetyCheckers_PmPllRegOffset5 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU};

static uint32_t gSafetyCheckers_PmPllRegOffset6 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmPllRegOffset7 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U};

static uint32_t gSafetyCheckers_PmPllRegOffset8 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmPllRegOffset12 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmPllRegOffset15 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU,
 0x90U};

static uint32_t gSafetyCheckers_PmPllRegOffset16 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmPllRegOffset17 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmPllRegOffset18 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U};

static uint32_t gSafetyCheckers_PmMcuPllRegOffset0 [] =
{0x00U, 0x08U, 0x20U, 0x24U, 0x30U, 0x34U, 0x38U,
 0x40U, 0x44U, 0x60U, 0x80U, 0x84U, 0x88U, 0x8cU,
 0x90U, 0x94U, 0x98U, 0x9cU};

/** @} */

/**
 * @ingroup SAFETY_CHECKERS_PM
 *
 * @defgroup SAFETY_CHECKERS_PM_STRUCTURE  SAFETY_CHECKERS_PM PM safety checkers SOC specific structure declaration
 *  @{
 */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/** 
 *
 * \brief Structure defines PLL register base address and the total length of registers
 *
 */
static SafetyCheckers_PmPllData gSafetyCheckers_PmPllData[] =
{
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(0),  gSafetyCheckers_PmPllRegOffset0, SAFETY_CHECKERS_PM_PLL0_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(1),  gSafetyCheckers_PmPllRegOffset1, SAFETY_CHECKERS_PM_PLL1_LENGTH},
	{SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(2),  gSafetyCheckers_PmPllRegOffset2, SAFETY_CHECKERS_PM_PLL2_LENGTH},
	{SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(5),  gSafetyCheckers_PmPllRegOffset5, SAFETY_CHECKERS_PM_PLL5_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(6),  gSafetyCheckers_PmPllRegOffset6, SAFETY_CHECKERS_PM_PLL6_LENGTH},
	{SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(7),  gSafetyCheckers_PmPllRegOffset7, SAFETY_CHECKERS_PM_PLL7_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(8),  gSafetyCheckers_PmPllRegOffset8, SAFETY_CHECKERS_PM_PLL8_LENGTH},
	{SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(12), gSafetyCheckers_PmPllRegOffset12, SAFETY_CHECKERS_PM_PLL12_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(15), gSafetyCheckers_PmPllRegOffset15, SAFETY_CHECKERS_PM_PLL15_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(16), gSafetyCheckers_PmPllRegOffset16, SAFETY_CHECKERS_PM_PLL16_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(17), gSafetyCheckers_PmPllRegOffset17, SAFETY_CHECKERS_PM_PLL17_LENGTH},
    {SAFETY_CHECKERS_PM_PLL_BASE_ADDRESS(18), gSafetyCheckers_PmPllRegOffset18, SAFETY_CHECKERS_PM_PLL18_LENGTH},

    {SAFETY_CHECKERS_PM_MCU_PLL_BASE_ADDRESS(0), gSafetyCheckers_PmMcuPllRegOffset0, SAFETY_CHECKERS_PM_MCU_PLL0_LENGTH},
};

/** 
 *
 * \brief Structure defines PSC register base address and the total length of registers
 *
 */
static SafetyCheckers_PmPscData gSafetyCheckers_PmPscData[] =
{
       {SAFETY_CHECKERS_PM_WKUP_PSC_BASE_ADDRESS,   SAFETY_CHECKERS_PM_WKUP_PD_STAT_NUM, SAFETY_CHECKERS_PM_WKUP_MD_STAT_NUM},
       {SAFETY_CHECKERS_PM_PSC_BASE_ADDRESS,        SAFETY_CHECKERS_PM_PD_STAT_NUM,      SAFETY_CHECKERS_PM_MD_STAT_NUM},
};

/** 
 *    
 * \brief Structure defines RM module register base address and the total length of registers
 *
 */
static SafetyCheckers_RmRegData gSafetyCheckers_RmRegData[] =
{
	{SAFETY_CHECKERS_RM_BA0_IR, SAFETY_CHECKERS_RM_IR_SUBMOD0_NUM, SAFETY_CHECKERS_RM_IR_REG0_NUM,SAFETY_CHECKERS_RM_REG_HEX4, {0X4U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA1_IR, SAFETY_CHECKERS_RM_IR_SUBMOD0_NUM, SAFETY_CHECKERS_RM_IR_REG1_NUM,SAFETY_CHECKERS_RM_REG_HEX4, {0X4U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA2_IR, SAFETY_CHECKERS_RM_IR_SUBMOD0_NUM, SAFETY_CHECKERS_RM_IR_REG2_NUM,SAFETY_CHECKERS_RM_REG_HEX4, {0X4U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	
	{SAFETY_CHECKERS_RM_BA0_IA_IMAP, SAFETY_CHECKERS_RM_SUBMOD0_IA_IMAP, SAFETY_CHECKERS_RM_REG0_IA_IMAP, SAFETY_CHECKERS_RM_REG_HEX8, {0X0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA1_IA_IMAP, SAFETY_CHECKERS_RM_SUBMOD0_IA_IMAP, SAFETY_CHECKERS_RM_REG1_IA_IMAP, SAFETY_CHECKERS_RM_REG_HEX8, {0X0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
    
	{SAFETY_CHECKERS_RM_BA0_RA, SAFETY_CHECKERS_RM_SUBMOD0_RA, SAFETY_CHECKERS_RM_RA_REG0_NUM, SAFETY_CHECKERS_RM_REG_HEX100, {0x40U, 0x44U, 0x48U,0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA1_RA, SAFETY_CHECKERS_RM_SUBMOD0_RA, SAFETY_CHECKERS_RM_RA_REG1_NUM, SAFETY_CHECKERS_RM_REG_HEX100, {0x40U, 0x44U, 0x48U,0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA2_RA, SAFETY_CHECKERS_RM_RA_SUBMOD1, SAFETY_CHECKERS_RM_RA_REG2_NUM, SAFETY_CHECKERS_RM_REG_HEX100, {0x40U, 0x44U, 0x48U, 0x4CU, 0x50U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA3_RA, SAFETY_CHECKERS_RM_SUBMOD0_RA, SAFETY_CHECKERS_RM_RA_REG3_NUM, SAFETY_CHECKERS_RM_REG_HEX100, {0x40U, 0x44U, 0x48U,0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},

	{SAFETY_CHECKERS_RM_BA0_UDMA_TX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_TX, SAFETY_CHECKERS_RM_REG0_UDMA_TX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x70U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}}, 
	{SAFETY_CHECKERS_RM_BA1_UDMA_TX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_TX, SAFETY_CHECKERS_RM_REG1_UDMA_TX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x70U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA2_UDMA_TX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_TX, SAFETY_CHECKERS_RM_REG2_UDMA_TX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x70U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	
	{SAFETY_CHECKERS_RM_BA0_UDMA_RX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_RX, SAFETY_CHECKERS_RM_REG0_UDMA_RX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA1_UDMA_RX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_RX, SAFETY_CHECKERS_RM_REG1_UDMA_RX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA2_UDMA_RX, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_RX, SAFETY_CHECKERS_RM_REG2_UDMA_RX, SAFETY_CHECKERS_RM_REG_HEX100, {0x0U, 0x64U, 0x68U, 0x80U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},

	{SAFETY_CHECKERS_RM_BA0_UDMA_FLW, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_FLW, SAFETY_CHECKERS_RM_REG0_UDMA_FLW, SAFETY_CHECKERS_RM_REG_HEX40, {0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},  
	
	{SAFETY_CHECKERS_RM_BA0_UDMA_GCFG, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_GCFG, SAFETY_CHECKERS_RM_REG0_UDMA_GCFG, SAFETY_CHECKERS_RM_REG_HEX0, {0x0U, 0x04U, 0X08U, 0x10U, 0x20U, 0x24U, 0x28U, 0x2CU, 0x30U, 0x60U, 0x64U, 0x78U, 0x7CU, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA1_UDMA_GCFG, SAFETY_CHECKERS_RM_SUBMOD1_UDMA_GCFG, SAFETY_CHECKERS_RM_REG1_UDMA_GCFG, SAFETY_CHECKERS_RM_REG_HEX0, {0x0U, 0x04U, 0X08U, 0x10U, 0x20U, 0x24U, 0x28U, 0x2CU, 0x30U, 0x60U, 0x64U, 0x78U, 0x7CU, 0x88U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
	{SAFETY_CHECKERS_RM_BA2_UDMA_GCFG, SAFETY_CHECKERS_RM_SUBMOD0_UDMA_GCFG, SAFETY_CHECKERS_RM_REG0_UDMA_GCFG, SAFETY_CHECKERS_RM_REG_HEX0, {0x0U, 0x04U, 0X08U, 0x10U, 0x20U, 0x24U, 0x28U, 0x2CU, 0x30U, 0x60U, 0x64U, 0x78U, 0x7CU, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U}},
};

/** @} */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef SAFETY_CHECKERS_SOC_H_ */
