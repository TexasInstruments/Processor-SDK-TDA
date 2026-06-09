/*
 *  Copyright (C) 2024-25 Texas Instruments Incorporated
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
 */

/**
 *  \file  PmuP_armv7.h
 *
 *  \brief
 *  Header file containing various enumerations, structure definitions and function
 *  declarations for the ARM R5 PMU IP.
 */

#ifndef PMUP_ARMV7R_H
#define PMUP_ARMV7R_H

/**
 *
 * \defgroup PmuP_ARM_R5_PMU_API R5 PMU API
 *
 * Provides the APIs to configure/read Performance Monitor Unit counters
 *
 */

/**
 *  \defgroup PmuP_ARM_R5_PMU_FUNCTION  R5 PMU Functions
 *  \ingroup PmuP_ARM_R5_PMU_API
 */

/**
 *  \defgroup PmuP_ARM_R5_PMU_ENUM R5 PMU Enumerated Data Types
 *  \ingroup PmuP_ARM_R5_PMU_API
 */

/**
 *  \defgroup PmuP_ARM_R5_PMU_MACROS R5 PMU Macros
 *  \ingroup PmuP_ARM_R5_PMU_API
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/**
 *  \addtogroup PmuP_ARM_R5_PMU_ENUM
 *
 *  @{
 */

/** PMU Event types */
#define PmuP_EVENT_TYPE_SWINC                                         (0U)
#define PmuP_EVENT_TYPE_ICACHE_MISS                                   (0x01U)
#define PmuP_EVENT_TYPE_DCACHE_MISS                                   (0x03U)
#define PmuP_EVENT_TYPE_DCACHE_ACCESS                                 (0x04U)
#define PmuP_EVENT_TYPE_D_RD                                          (0x06U)
#define PmuP_EVENT_TYPE_D_WR                                          (0x07U)
#define PmuP_EVENT_TYPE_I_X                                           (0x08U)
#define PmuP_EVENT_TYPE_PI_X                                          (0x5EU)
#define PmuP_EVENT_TYPE_EXCEPTION                                     (0x09U)
#define PmuP_EVENT_TYPE_EXCEPTION_RET                                 (0x0AU)
#define PmuP_EVENT_TYPE_CID_CHANGE                                    (0x0BU)
#define PmuP_EVENT_TYPE_SW_PC                                         (0x0CU)
#define PmuP_EVENT_TYPE_B_IMMEDIATE                                   (0x0DU)
#define PmuP_EVENT_TYPE_PROC_RET                                      (0x0EU)
#define PmuP_EVENT_TYPE_UNALIGNED_ACCESS                              (0x0FU)
#define PmuP_EVENT_TYPE_BRANCH_TAKEN                                  (0x10U)
#define PmuP_EVENT_TYPE_BRANCH_PRED                                   (0x12U)
#define PmuP_EVENT_TYPE_ICACHE_STALL                                  (0x40U)
#define PmuP_EVENT_TYPE_DCACHE_STALL                                  (0x41U)
#define PmuP_EVENT_TYPE_DCACHE_WB                                     (0x42U)
#define PmuP_EVENT_TYPE_MEM_REQ                                       (0x43U)
#define PmuP_EVENT_TYPE_LSU_BUSY_STALL                                (0x44U)
#define PmuP_EVENT_TYPE_SB_DRAIN                                      (0x45U)
#define PmuP_EVENT_TYPE_FIQ_DISABLED_CYCLES                           (0x46U)
#define PmuP_EVENT_TYPE_IRQ_DISABLED_CYCLES                           (0x47U)
#define PmuP_EVENT_TYPE_ETMEXTOUTM0                                   (0x48U)
#define PmuP_EVENT_TYPE_ETMEXTOUTM1                                   (0x49U)
#define PmuP_EVENT_TYPE_ICACHE_TAG_CECC                               (0x4AU)
#define PmuP_EVENT_TYPE_ICACHE_DATA_CECC                              (0x4BU)
#define PmuP_EVENT_TYPE_DCACHE_TAG_CECC                               (0x4CU)
#define PmuP_EVENT_TYPE_DCACHE_DATA_CECC                              (0x4DU)
#define PmuP_EVENT_TYPE_TCM_FECC_PF                                   (0x4EU)
#define PmuP_EVENT_TYPE_TCM_FECC_LS                                   (0x4FU)
#define PmuP_EVENT_TYPE_SB_MERGE                                      (0x50U)
#define PmuP_EVENT_TYPE_LSU_SB_STALL                                  (0x51U)
#define PmuP_EVENT_TYPE_LSU_QF_STALL                                  (0x52U)
#define PmuP_EVENT_TYPE_INT_DIV                                       (0x53U)
#define PmuP_EVENT_TYPE_INT_DIV_STALL                                 (0x54U)
#define PmuP_EVENT_TYPE_PLD_LINEFILL                                  (0x55U)
#define PmuP_EVENT_TYPE_PLD_NO_LINEFILL                               (0x56U)
#define PmuP_EVENT_TYPE_NONCACHEABLE_ACCESS                           (0x57U)
#define PmuP_EVENT_TYPE_ICACHE_ACCESS                                 (0x58U)
#define PmuP_EVENT_TYPE_SB_ATTR                                       (0x59U)
#define PmuP_EVENT_TYPE_DUAL_ISSUE_CASE_A                             (0x5AU)
#define PmuP_EVENT_TYPE_DUAL_ISSUE_CASE_B                             (0x5BU)
#define PmuP_EVENT_TYPE_DUAL_ISSUE_CASE_OTHER                         (0x5CU)
#define PmuP_EVENT_TYPE_DOUBLE_FP                                     (0x5DU)
#define PmuP_EVENT_TYPE_DCACHE_DATA_FECC                              (0x60U)
#define PmuP_EVENT_TYPE_DCACHE_TAG_FECC                               (0x61U)
#define PmuP_EVENT_TYPE_LIVELOCK                                      (0x62U)
#define PmuP_EVENT_TYPE_ATCM_MB_ECC                                   (0x64U)
#define PmuP_EVENT_TYPE_B0TCM_MB_ECC                                  (0x65U)
#define PmuP_EVENT_TYPE_B1TCM_MB_ECC                                  (0x66U)
#define PmuP_EVENT_TYPE_ATCM_SB_ECC                                   (0x67U)
#define PmuP_EVENT_TYPE_B0TCM_SB_ECC                                  (0x68U)
#define PmuP_EVENT_TYPE_B1TCM_SB_ECC                                  (0x69U)
#define PmuP_EVENT_TYPE_TCM_CECC_LS                                   (0x6AU)
#define PmuP_EVENT_TYPE_TCM_CECC_PF                                   (0x6BU)
#define PmuP_EVENT_TYPE_TCM_FECC_AXI                                  (0x6CU)
#define PmuP_EVENT_TYPE_TCM_CECC_AXI                                  (0x6DU)
#define PmuP_EVENT_TYPE_CORRECTABLE_EVENTS                            (0x6EU)
#define PmuP_EVENT_TYPE_FATAL_EVENTS                                  (0x6FU)
#define PmuP_EVENT_TYPE_CORRECTABLE_BUS_FAULTS                        (0x70U)
#define PmuP_EVENT_TYPE_FATAL_BUS_FAULTS                              (0x71U)
#define PmuP_EVENT_TYPE_ACP_DCACHE_ACCESS                             (0x72U)
#define PmuP_EVENT_TYPE_ACP_DCACHE_INVALIDATE                         (0x73U)
#define PmuP_EVENT_TYPE_CYCLE_CNT                                     (0xFF)

/* @} */

#ifndef PMU_SECTION
#define PMU_SECTION __attribute__((section(".text.pmu")))
#endif

/**
 *  \addtogroup PmuP_ARM_R5_PMU_MACROS
 *  @{
 */
/** PMU Cycle count number */
#define PmuP_PMU_CYCLE_COUNTER_NUM                                    (31U)

/** PMU_CFG_CNTR_EVENT_TYPE */
#define PmuP_PMU_CFG_CNTR_EVENT_TYPE_MASK                             (0x000000FFU)
#define PmuP_PMU_CFG_CNTR_EVENT_TYPE_SHIFT                            (0x00000000U)
#define PmuP_PMU_CFG_CNTR_EVENT_TYPE_RESETVAL                         (0x00000000U)
#define PmuP_PMU_CFG_CNTR_EVENT_TYPE_MAX                              (0x000000FFU)

/* @} */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \addtogroup PmuP_ARM_R5_PMU_FUNCTION
 *  @{
 */

/**
 *  \brief Configure PMU counters
 *
 *  \param cycleCntDiv cycle count div
 *
 *  \param exportEvents export events
 *
 *  \param userEnable value 1 to enable
 *
 *  \return None
 */
void PmuP_Config( uint32_t cycleCntDiv, uint32_t exportEvents, uint32_t userEnable );

/**
 *  \brief Configure PMU counter
 *
 *  \param counterNumber counter number
 *
 *  \param eventType event type
 *
 *  \return None
 */
void PmuP_ConfigCounter( uint32_t counterNumber, uint32_t eventType );

/**
 *  \brief Enable all PMU counters
 *
 *  \param enable value 1 to enable
 *
 *  \return None
 */
void PmuP_EnableAllCounters( uint32_t enable );

/**
 *  \brief Reset PMU counters
 *
 *  \param None
 *
 *  \return None
 */
void PmuP_ResetCounters( void );

/**
 *  \brief Enable PMU counter
 *
 *  \param counterNumber Counter number
 *
 *  \param enable value 1 to enable
 *
 *  \return None
 */
void PmuP_enableCounter( uint32_t counterNumber, uint32_t enable );

/**
 *  \brief Read counter
 *
 *  \param counterNumber Counter number
 *
 *  \return Counter value
 */
uint32_t PmuP_ReadCounter( uint32_t counterNumber );

/**
 *  \brief Enable PMU counter overflow interrupt
 *
 *  \param counterNumber Counter number
 *
 *  \param enable value 1 to enable
 *
 *  \return None
 */
void PmuP_EnableCounterOverflowInterrupt( uint32_t counterNumber, uint32_t enable );

/**
 *  \brief Setup PMU
 *  This function writes the PMCR register.
 *
 *  \param setupFlags
 *          Bitwise OR of the following options.
 *
 *          PmuP_SETUP_FLAG_CYCLE_COUNTER_DIV64
 *          PmuP_SETUP_FLAG_CYCLE_COUNTER_RESET
 *          PmuP_SETUP_FLAG_EVENT_COUNTER_RESET
 *          PmuP_SETUP_FLAG_ENABLE_ALL_COUNTERS
 *
 *  \return None
 */
void PmuP_setup( uint32_t setupFlags );

/**
 *  \brief Enables PMU counter
 *  This function writes PMCNTENSET register to enable the counter.
 *
 *  \param counterMask
 *          Bitwise OR of the following options.
 *
 *          PmuP_COUNTER_MASK_CYCLE_COUNTER
 *          PmuP_COUNTER_MASK_ALL_COUNTERS
 *
 *  \return None
 */
void PmuP_enableCounters( uint32_t counterMask );

/**
 *  \brief Disables PMU counter
 *  This function writes PMCNTENCLR register to disable the counter.
 *
 *  \param counterMask
 *          Bitwise OR of the following options.
 *
 *          PmuP_COUNTER_MASK_CYCLE_COUNTER
 *          PmuP_COUNTER_MASK_ALL_COUNTERS
 *
 *  \return None
 */
void PmuP_disableCounters( uint32_t counterMask );

/**
 *  \brief Read the overflow status for all of the counters
 *  This function is used to read the overflow status for all of the counters.
 *  A bit-mask is returned where bits set to '1' indicate overflow occurred for
 *  the corresponding counter. See the 'Overflow Flag Status Register'
 *  description in the ARM R5 TRM for the format of this bitmask.
 *
 *  \param None
 *
 *  \return the value of PMOVSR register
 */
uint32_t PmuP_getOverflowStatus( void );

/**
 *  \brief Get the number of PMU counters supported
 *  This function is used to get the number of PMU counters supported.
 *
 *  \param None
 *
 *  \return The number of PMU counters supported
 */
extern uint32_t PmuP_GetNumCntrs( void );

/**
 *  \brief Read a PMU counter
 *  This function is used to read the specified PMU counter.
 *
 *  \param cntrNum      [IN]    Counter number (0..(PmuP_PmuGetNumCntrs()-1))
 *                              or PmuP_PMU_CYCLE_COUNTER_NUM)
 *
 *  \return The current count associated with the specified PMU counter
 */
extern uint32_t PmuP_ReadCntr( uint32_t cntrNum );

/**
 *  \brief Set a PMU counter
 *  This function is used to set the specified PMU counter.
 *
 *  \param cntrNum      [IN]    Counter number (0..(PmuP_PmuGetNumCntrs()-1))
 *                              or PmuP_PMU_CYCLE_COUNTER_NUM)
 *  \param cntrVal      [IN]    Counter Value to be set
 *
 *  \return None
 */
extern uint32_t PmuP_SetCntr( uint32_t cntrNum, uint32_t cntrVal);

/**
 *  \brief Clear the overflow flag for the specified counter(s)
 *  This function is used to clear the overflow flag for the specified
 *  counter(s). The cntrMask argument is a bit-mask where bits set to '1'
 *  indicate which counter(s)' overflow flag to clear. See the 'Overflow
 *  Flag Status Register' description in the ARM R5 TRM for the format of
 *  this bitmask.
 *
 *  \param cntrMask     [IN]    Bit-mask indicating which counter(s)' overflow
 *                      flag to clear
 *
 *  \return None
 */
extern void PmuP_ClearCntrOverflowStatus( uint32_t cntrMask );

/**
 *  \brief Reset the cycle counter to zero
 *  This function resets the cycle counter to zero.
 *
 *  \param None
 *
 *  \return None
 */
extern void PmuP_ResetCycleCnt( void );

/* \} */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Structure Declarations                   */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* PMUP_ARMV7R_H */
