/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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
 *  \file  main_baremetal.c
 *
 *  \brief This application is software test  of error codes for invalid
 *         transactions. If there is fault in a transaction that affects
 *         it's addressability in the memory map, the  interconnect will
 *         terminate the transaction in null slave. The addressing error
 *         code will be returned in  read  or write status signal. There
 *         will be interrupt when such invalid transaction occurs and SW
 *         can determine the exact transaction that caused the interrupt.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/board/board.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/arch/cslr64.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/csl/arch/r5/interrupt.h>
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#include <ti/csl/soc/j721s2/src/cslr_mcu_r5fss0_baseaddress.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#include <ti/csl/soc/j784s4/src/cslr_mcu_r5fss0_baseaddress.h>
#endif

#include <ip_fma_r5f.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Data Fault Status  Register function */
_DEFINE_COPROCR_READ_FUNC(dfsr, p15, 0, c5, c0, 0)
/**< Data Fault Address Register function */
_DEFINE_COPROCR_READ_FUNC(dfar, p15, 0, c6, c0, 0)

/**< Macros for DFSR bit fields */
#define DFSR_SD_BIT_MASK        (0x1000U)
#define DFSR_SD_BIT_SHIFT       (12U)
#define DFSR_RW_BIT_MASK        (0x800U)
#define DFSR_RW_BIT_SHIFT       (11U)

#define CBASS_ERR_PID                           (0x00)
#define CBASS_ERR_DESTINATION_ID                (0x04)
#define CBASS_ERR_EXCEPTION_LOGGING_HEADER0     (0x24)
#define CBASS_ERR_EXCEPTION_LOGGING_HEADER1     (0x28)
#define CBASS_ERR_EXCEPTION_LOGGING_DATA0       (0x2C)
#define CBASS_ERR_EXCEPTION_LOGGING_DATA1       (0x30)
#define CBASS_ERR_EXCEPTION_LOGGING_DATA2       (0x34)
#define CBASS_ERR_EXCEPTION_LOGGING_DATA3       (0x38)
#define CBASS_ERR_ERR_INTR_RAW_STAT             (0x50)
#define CBASS_ERR_ERR_INTR_ENABLED_STAT         (0x54)
#define CBASS_ERR_ERR_INTR_ENABLE_SET           (0x58)
#define CBASS_ERR_ERR_INTR_ENABLE_CLR           (0x5C)
#define CBASS_ERR_EOI                           (0x60)

#define CBASS_GLB_PID_OFFSET                        (0x00U)  /**< Revision Register */
#define CBASS_GLB_DESTINATION_ID_OFFSET             (0x04U)  /**< Destination ID Register */
#define CBASS_GLB_EXCEPTION_LOGGING_CONTROL_OFFSET  (0x20U)  /**< Exception Logging Control Register */
#define CBASS_GLB_EXCEPTION_LOGGING_HEADER0_OFFSET  (0x24U)  /**< Exception Logging Header 0 Register */
#define CBASS_GLB_EXCEPTION_LOGGING_HEADER1_OFFSET  (0x28U)  /**< Exception Logging Header 1 Register */
#define CBASS_GLB_EXCEPTION_LOGGING_DATA0_OFFSET    (0x2CU)  /**< Exception Logging Data 0 Register */
#define CBASS_GLB_EXCEPTION_LOGGING_DATA1_OFFSET    (0x30U)  /**< Exception Logging Data 1 Register */
#define CBASS_GLB_EXCEPTION_LOGGING_DATA2_OFFSET    (0x34U)  /**< Exception Logging Data 2 Register */
#define CBASS_GLB_EXCEPTION_LOGGING_DATA3_OFFSET    (0x38U)  /**< Exception Logging Data 3 Register */
#define CBASS_GLB_EXCEPTION_PEND_SET_OFFSET         (0x40U)  /**< Exception Logging Pending Set Register */
#define CBASS_GLB_EXCEPTION_PEND_CLEAR_OFFSET       (0x44U)  /**< Exception Logging Pending Clear Register */

/* ========================================================================== */
/*                                  Structures                                */
/* ========================================================================== */

typedef struct
{
    uint32_t pid;
    uint32_t destination_id;
    uint32_t exception_logging_header0;
    uint32_t exception_logging_header1;
    uint32_t exception_logging_data0;
    uint32_t exception_logging_data1;
    uint32_t exception_logging_data2;
    uint32_t exception_logging_data3;
    uint32_t err_intr_raw_stat;
    uint32_t err_intr_enabled_stat;
    uint32_t err_intr_enable_set;
    uint32_t err_intr_enable_clr;
    uint32_t eio;
} CbassErr_Regs;

typedef struct
{
    uint32_t pid;
    uint32_t destination_id;
    uint32_t exception_logging_control;
    uint32_t exception_logging_header0;
    uint32_t exception_logging_header1;
    uint32_t exception_logging_data0;
    uint32_t exception_logging_data1;
    uint32_t exception_logging_data2;
    uint32_t exception_logging_data3;
    uint32_t exception_pend_set;
    uint32_t exception_pend_clear;
} CbassGlb_Regs;

/**
 *  \brief Data Abort Exception Handler Args
 */
typedef struct
{
    uint32_t cause;                         /**< Cause of the exception, 0 - read, 1 - write. */
    uint32_t type;                          /**< Type of the exception. */
    uint32_t externalAbortCause;            /**< Cause of the external abort(Only valid if the abort is external). */
    uint32_t address;                       /**< Address on which the read/write operation caused an exception. */
    uint32_t auxiliaryFaultStatusReg;       /**< Auxiliary Data Fault Status Register (ADFSR). */
    CbassGlb_Regs lb_mcu;                   /**< CbassGlb_Regs registers. */
    CbassGlb_Regs lb_wkup;                  /**< CbassGlb_Regs registers. */
    CbassErr_Regs err_mcu;                  /**< CbassErr_Regs registers. */
    CbassErr_Regs err_wkup;                 /**< CbassErr_Regs registers. */
} Cbat4App_DataAbortExptnHandlerArgs;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Structure containing the Exception Handlers. */
extern CSL_R5ExptnHandlers gExptnHandlers;

/* Data abort exception handler arguments. */
static Cbat4App_DataAbortExptnHandlerArgs gDataAbortExptnHandlerArgs;

/* Indicates if the data abort exception was invoked. */
bool gDataAbortExptnInvoked = false;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void IpFma_Cba_read_registers(CbassGlb_Regs* glb_mcu,
                                     CbassGlb_Regs* glb_wkup,
                                     CbassErr_Regs* err_mcu,
                                     CbassErr_Regs* err_wkup)
{
    glb_mcu->pid                        = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_PID_OFFSET);
    glb_mcu->destination_id             = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_DESTINATION_ID_OFFSET);
    glb_mcu->exception_logging_control  = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_CONTROL_OFFSET);
    glb_mcu->exception_logging_header0  = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_HEADER0_OFFSET);
    glb_mcu->exception_logging_header1  = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_HEADER1_OFFSET);
    glb_mcu->exception_logging_data0    = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA0_OFFSET);
    glb_mcu->exception_logging_data1    = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA1_OFFSET);
    glb_mcu->exception_logging_data2    = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA2_OFFSET);
    glb_mcu->exception_logging_data3    = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA3_OFFSET);
    glb_mcu->exception_pend_set         = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_PEND_SET_OFFSET);
    glb_mcu->exception_pend_clear       = CSL_REG32_RD(CSL_MCU_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_PEND_CLEAR_OFFSET);

    glb_wkup->pid                        = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_PID_OFFSET);
    glb_wkup->destination_id             = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_DESTINATION_ID_OFFSET);
    glb_wkup->exception_logging_control  = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_CONTROL_OFFSET);
    glb_wkup->exception_logging_header0  = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_HEADER0_OFFSET);
    glb_wkup->exception_logging_header1  = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_HEADER1_OFFSET);
    glb_wkup->exception_logging_data0    = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA0_OFFSET);
    glb_wkup->exception_logging_data1    = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA1_OFFSET);
    glb_wkup->exception_logging_data2    = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA2_OFFSET);
    glb_wkup->exception_logging_data3    = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_LOGGING_DATA3_OFFSET);
    glb_wkup->exception_pend_set         = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_PEND_SET_OFFSET);
    glb_wkup->exception_pend_clear       = CSL_REG32_RD(CSL_WKUP_CBASS0_GLB_BASE + CBASS_GLB_EXCEPTION_PEND_CLEAR_OFFSET);

    err_mcu->pid                         = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_PID);
    err_mcu->destination_id              = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_DESTINATION_ID);
    err_mcu->exception_logging_header0   = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_HEADER0);
    err_mcu->exception_logging_header1   = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_HEADER1);
    err_mcu->exception_logging_data0     = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA0);
    err_mcu->exception_logging_data1     = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA1);
    err_mcu->exception_logging_data2     = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA2);
    err_mcu->exception_logging_data3     = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA3);
    err_mcu->err_intr_raw_stat           = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_RAW_STAT);
    err_mcu->err_intr_enabled_stat       = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLED_STAT);
    err_mcu->err_intr_enable_set         = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLE_SET);
    err_mcu->err_intr_enable_clr         = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLE_CLR);
    err_mcu->eio                         = CSL_REG32_RD(CSL_MCU_CBASS0_ERR_BASE + CBASS_ERR_EOI);

    err_wkup->pid                         = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_PID);
    err_wkup->destination_id              = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_DESTINATION_ID);
    err_wkup->exception_logging_header0   = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_HEADER0);
    err_wkup->exception_logging_header1   = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_HEADER1);
    err_wkup->exception_logging_data0     = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA0);
    err_wkup->exception_logging_data1     = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA1);
    err_wkup->exception_logging_data2     = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA2);
    err_wkup->exception_logging_data3     = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EXCEPTION_LOGGING_DATA3);
    err_wkup->err_intr_raw_stat           = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_RAW_STAT);
    err_wkup->err_intr_enabled_stat       = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLED_STAT);
    err_wkup->err_intr_enable_set         = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLE_SET);
    err_wkup->err_intr_enable_clr         = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_ERR_INTR_ENABLE_CLR);
    err_wkup->eio                         = CSL_REG32_RD(CSL_WKUP_CBASS0_ERR_BASE + CBASS_ERR_EOI);
}

static void IpFma_Cba_print_registers(const CbassGlb_Regs* glb_mcu,
                                      const CbassGlb_Regs* glb_wkup,
                                      CbassErr_Regs* err_mcu,
                                      CbassErr_Regs* err_wkup)
{
    UART_printf("\n|-------------- MCU_CBASS_GLB Register Dump --------------|\n");
    UART_printf("| pid                        = 0x%08x\n", glb_mcu->pid);
    UART_printf("| destination_id             = 0x%08x\n", glb_mcu->destination_id);
    UART_printf("| exception_logging_control  = 0x%08x\n", glb_mcu->exception_logging_control);
    UART_printf("| exception_logging_header0  = 0x%08x\n", glb_mcu->exception_logging_header0);
    UART_printf("| exception_logging_header1  = 0x%08x\n", glb_mcu->exception_logging_header1);
    UART_printf("| exception_logging_data0    = 0x%08x\n", glb_mcu->exception_logging_data0);
    UART_printf("| exception_logging_data1    = 0x%08x\n", glb_mcu->exception_logging_data1);
    UART_printf("| exception_logging_data2    = 0x%08x\n", glb_mcu->exception_logging_data2);
    UART_printf("| exception_logging_data3    = 0x%08x\n", glb_mcu->exception_logging_data3);
    UART_printf("| exception_pend_set         = 0x%08x\n", glb_mcu->exception_pend_set);
    UART_printf("| exception_pend_clear       = 0x%08x\n", glb_mcu->exception_pend_clear);
    UART_printf("|-----------------------------------------------------|\n");

    UART_printf("\n|-------------- WKUP_CBASS_GLB Register Dump --------------|\n");
    UART_printf("| pid                        = 0x%08x\n", glb_wkup->pid);
    UART_printf("| destination_id             = 0x%08x\n", glb_wkup->destination_id);
    UART_printf("| exception_logging_control  = 0x%08x\n", glb_wkup->exception_logging_control);
    UART_printf("| exception_logging_header0  = 0x%08x\n", glb_wkup->exception_logging_header0);
    UART_printf("| exception_logging_header1  = 0x%08x\n", glb_wkup->exception_logging_header1);
    UART_printf("| exception_logging_data0    = 0x%08x\n", glb_wkup->exception_logging_data0);
    UART_printf("| exception_logging_data1    = 0x%08x\n", glb_wkup->exception_logging_data1);
    UART_printf("| exception_logging_data2    = 0x%08x\n", glb_wkup->exception_logging_data2);
    UART_printf("| exception_logging_data3    = 0x%08x\n", glb_wkup->exception_logging_data3);
    UART_printf("| exception_pend_set         = 0x%08x\n", glb_wkup->exception_pend_set);
    UART_printf("| exception_pend_clear       = 0x%08x\n", glb_wkup->exception_pend_clear);
    UART_printf("|-----------------------------------------------------|\n");

    UART_printf("\n|-------------- MCU_CBASS_ERR Register Dump --------------|\n");
    UART_printf("| pid                        = 0x%08x\n", err_mcu->pid);
    UART_printf("| destination_id             = 0x%08x\n", err_mcu->destination_id);
    UART_printf("| exception_logging_header0  = 0x%08x\n", err_mcu->exception_logging_header0);
    UART_printf("| exception_logging_header1  = 0x%08x\n", err_mcu->exception_logging_header1);
    UART_printf("| exception_logging_data0    = 0x%08x\n", err_mcu->exception_logging_data0);
    UART_printf("| exception_logging_data1    = 0x%08x\n", err_mcu->exception_logging_data1);
    UART_printf("| exception_logging_data2    = 0x%08x\n", err_mcu->exception_logging_data2);
    UART_printf("| exception_logging_data3    = 0x%08x\n", err_mcu->exception_logging_data3);
    UART_printf("| err_intr_raw_stat          = 0x%08x\n", err_mcu->err_intr_raw_stat);
    UART_printf("| err_intr_enabled_stat      = 0x%08x\n", err_mcu->err_intr_enabled_stat);
    UART_printf("| err_intr_enable_set        = 0x%08x\n", err_mcu->err_intr_enable_set);
    UART_printf("| err_intr_enable_clr        = 0x%08x\n", err_mcu->err_intr_enable_clr);
    UART_printf("| eio                        = 0x%08x\n", err_mcu->eio);
    UART_printf("|-----------------------------------------------------|\n");

    UART_printf("\n|-------------- WKUP_CBASS_ERR Register Dump --------------|\n");
    UART_printf("| pid                        = 0x%08x\n", err_wkup->pid);
    UART_printf("| destination_id             = 0x%08x\n", err_wkup->destination_id);
    UART_printf("| exception_logging_header0  = 0x%08x\n", err_wkup->exception_logging_header0);
    UART_printf("| exception_logging_header1  = 0x%08x\n", err_wkup->exception_logging_header1);
    UART_printf("| exception_logging_data0    = 0x%08x\n", err_wkup->exception_logging_data0);
    UART_printf("| exception_logging_data1    = 0x%08x\n", err_wkup->exception_logging_data1);
    UART_printf("| exception_logging_data2    = 0x%08x\n", err_wkup->exception_logging_data2);
    UART_printf("| exception_logging_data3    = 0x%08x\n", err_wkup->exception_logging_data3);
    UART_printf("| err_intr_raw_stat          = 0x%08x\n", err_wkup->err_intr_raw_stat);
    UART_printf("| err_intr_enabled_stat      = 0x%08x\n", err_wkup->err_intr_enabled_stat);
    UART_printf("| err_intr_enable_set        = 0x%08x\n", err_wkup->err_intr_enable_set);
    UART_printf("| err_intr_enable_clr        = 0x%08x\n", err_wkup->err_intr_enable_clr);
    UART_printf("| eio                        = 0x%08x\n", err_wkup->eio);
    UART_printf("|-----------------------------------------------------|\n");
}

static void IpFma_Cba_UpdateDataAbortExptnHandler(void* dataAbortExptnHandler, Cbat4App_DataAbortExptnHandlerArgs* dataAbortExptnHandlerArgs)
{
    gExptnHandlers.dabtExptnHandler = dataAbortExptnHandler;
    gExptnHandlers.dabtExptnHandlerArgs = dataAbortExptnHandlerArgs;
}

static void IpFma_Cba_DataAbortExptnHandler(Cbat4App_DataAbortExptnHandlerArgs* args)
{
    // Read Data Fault Status Register
    uint32_t dfsrReg = IpFma_R5f_read_dfsr();
    // Read Data Fault Address Register
    uint32_t dfarReg = (uint32_t) IpFma_R5f_read_dfar();

    args->address = dfarReg;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bit [12] - SD (External abort type)
     * Distinguishes between an AXI Decode or Slave error on an external abort.
     * This bit is only valid for external aborts. For  all other aborts types
     * of abort, this bit is set to zero:
     *  0 = AXI Decode error (DECERR) or AHB  error caused the abort
     *  1 = AXI Slave  error (SLVERR) or unsupported exclusive access
     */
    uint8_t sd = (dfsrReg & DFSR_SD_BIT_MASK) >> DFSR_SD_BIT_SHIFT;
    args->externalAbortCause = sd;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bit [11] - RW
     * Indicates whether a read or write access caused an abort:
     *  0 = read  access caused the abort
     *  1 = write access caused the abort
     */
    uint8_t rw = (dfsrReg & DFSR_RW_BIT_MASK) >> DFSR_RW_BIT_SHIFT;
    args->cause = rw;

    /**
     * DFSR (Data Fault Status Register) – Bit assignments
     *
     * Bits [10, 3:0] - STATUS
     * Indicates the type of fault generated
     *
     * -------------------------------------------------------------------------
     * Fault Type                              FSR[10,3:0]    FAR
     * -------------------------------------------------------------------------
     * Alignment                                0b00001        Valid
     *
     * Background                               0b00000        Valid
     *
     * Permission                               0b01101        Valid
     *
     * Synchronous External Abort               0b01000        Valid
     *
     * Asynchronous External Abort              0b10110        Unpredictable
     *
     * Synchronous Parity or ECC Error          0b11001        Valid
     *
     * Asynchronous Parity or ECC Error         0b11000        Unpredictable
     *
     * Debug Event                              0b00010        Unchanged
     * -------------------------------------------------------------------------
     */
    uint8_t status = ((dfsrReg >> 6) & 0x10) | (dfsrReg & 0x0F);
    args->type = status;

    IpFma_Cba_read_registers(&args->lb_mcu, &args->lb_wkup, &args->err_mcu, &args->err_wkup);

    /* Flag that a Data Abort exception has occurred */
    gDataAbortExptnInvoked = true;
}

/*
 * Application main
 */
int main(void)
{
    int32_t retVal = CSL_PASS;

    /* Register a custom Data Abort exception handler */
    IpFma_Cba_UpdateDataAbortExptnHandler((exptnHandlerPtr) IpFma_Cba_DataAbortExptnHandler, &gDataAbortExptnHandlerArgs);

    /* Board initialization */
    Board_initCfg boardCfg = BOARD_INIT_MODULE_CLOCK |
                             BOARD_INIT_PINMUX_CONFIG |
                             BOARD_INIT_UART_STDIO;

    if (BOARD_SOK == Board_init(boardCfg))
    {
        // Invalid transaction - address is not matched to any target (ADDRESSING ERROR)
        CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_UNALLOCATED0_BASE);

        if (true == gDataAbortExptnInvoked)
        {
            bool exceptionInvokedForValidReasons = true;
            if (CSL_COMPUTE_CLUSTER0_UNALLOCATED0_BASE == gDataAbortExptnHandlerArgs.address)
            {
                UART_printf("\n Data abort exception was invoked on address %x!", gDataAbortExptnHandlerArgs.address);
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was not invoked on correct address %x!", gDataAbortExptnHandlerArgs.address);
                exceptionInvokedForValidReasons = false;
            }

            if (0x08 == gDataAbortExptnHandlerArgs.type)
            {
                UART_printf("\n Data abort exception was invoked due to Synchronous External Abort");
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was not invoked due to Synchronous External Abort!");
                exceptionInvokedForValidReasons = false;
            }

            if (0U == gDataAbortExptnHandlerArgs.cause)
            {
                UART_printf("\n Data abort exception was invoked due to read operation");
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was not invoked due to read operation!");
                exceptionInvokedForValidReasons = false;
            }

            if (0 == gDataAbortExptnHandlerArgs.externalAbortCause)
            {
                UART_printf("\n ERROR: AXI Decode error (DECERR) or AHB  error caused the abort\n");
                exceptionInvokedForValidReasons = false;
            }
            else
            {
                UART_printf("\n AXI Slave error (SLVERR) or unsupported exclusive access\n");
            }

            IpFma_Cba_print_registers(&gDataAbortExptnHandlerArgs.lb_mcu,
                                      &gDataAbortExptnHandlerArgs.lb_wkup,
                                      &gDataAbortExptnHandlerArgs.err_mcu,
                                      &gDataAbortExptnHandlerArgs.err_wkup);

            if (true == exceptionInvokedForValidReasons)
            {
                UART_printf("All tests have passed!!\n");
            }
        }
        else
        {
            UART_printf("ERROR: Some tests have failed!!\n");
        }
    }
    else
    {
        UART_printf("\n ERROR: Board not initialized succesfully!!");
    }

    return retVal;
}