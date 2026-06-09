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
 *  \brief This application implements a monitoring and reporting interface
 *         for the MSMC Null Slave endpoint to handle accesses that cannot
 *         be sent to their intended destination.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/board/board.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/arch/cslr64.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#include <ti/csl/soc/j721s2/src/cslr_mcu_r5fss0_baseaddress.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#include <ti/csl/soc/j784s4/src/cslr_mcu_r5fss0_baseaddress.h>
#endif
#include <ti/csl/arch/r5/interrupt.h>
#include <ti/csl/src/ip/msmc/cslr_msmc.h>

#include <ip_fma_r5f.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Data Fault Status  Register function */
_DEFINE_COPROCR_READ_FUNC(dfsr, p15, 0, c5, c0, 0)
/**< Data Fault Address Register function */
_DEFINE_COPROCR_READ_FUNC(dfar, p15, 0, c6, c0, 0)

/**< Macros for DFSR bit fields */
#define DFSR_SD_BIT_MASK                        (0x1000U)
#define DFSR_SD_BIT_SHIFT                       (12U)
#define DFSR_RW_BIT_MASK                        (0x800U)
#define DFSR_RW_BIT_SHIFT                       (11U)

#define MSMC_INT_INTERRUPT_WAIT_DELAY           (10U)
#define MSMC_INT_INTERRUPT_WAIT_TIMEOUT         (3000u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief MSMC Null Slave endpoint info object.
 *
 * This structure captures the debug metadata  populated in the NULL_SLV_STAT reporting
 * hardware when a command is routed to the Null Slave due to a faulted response status
 */
typedef struct
{
    /**< Fault Address (caddress): Address where the error occurred.
     *   Address that some master tried to access. */
    uint64_t addr;
    /**< Privilege Level (cpriv): Indicates execution mode of master.
     *   0 = User mode, 1 = Privileged/Supervisor mode. */
    uint32_t priv;
    /**< Security (csecure): Indicates security state of transaction.
     *   1 = Secure access, 0 = Non-secure access. */
    uint32_t secure;
    /**< Emulation Debug (cemudbg): Indicates if the transaction was
     *   initiated by an emulation/debug read or write. 1 = error is
     *   caused by debuger. */
    uint32_t emu;
    /**< Memory Type (cmemtype): Type of memory being accessed. */
    uint32_t memtype;
    /**< Type of operation that was attempted. */
    uint32_t opcode;
    /**< ID of the master who initiated erroneous transaction. */
    uint32_t privid;
    /**< Path the transaction took. */
    uint32_t routeid;
    /**< Transaction size in bytes. */
    uint32_t bytecnt;
} IpFma_MsmcNullSlaveErrorInfo;

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
    IpFma_MsmcNullSlaveErrorInfo errorInfo; /**< Global MSMC Null Slave endpoint info object. */
} MsmcIntApp_DataAbortExptnHandlerArgs;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Structure containing the Exception Handlers. */
extern CSL_R5ExptnHandlers gExptnHandlers;

/* Data abort exception handler arguments. */
static MsmcIntApp_DataAbortExptnHandlerArgs gDataAbortExptnHandlerArgs;

/* Indicates if the data abort exception was invoked. */
bool gDataAbortExptnInvoked = false;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   This function retrieves the registers populated when a command is
 *          routed to the null slave due to an unexecutable access.
 *
 * \param   info        Pointer to the MSMC null slave endpoint  info  object
 *                      containing information on the cause of an uncompleted
 *                      access.
 *
 * \retval  none.
 */
static void IpFma_Msmc_GetNullSlaveErrorInfo(IpFma_MsmcNullSlaveErrorInfo *info);

/**
 * \brief   This function is wrapper function used to print message on
 *          respective consoles.
 *
 * \param   info        Pointer to the MSMC null slave endpoint  info  object
 *                      containing information on the cause of an uncompleted
 *                      access.
 *
 * \retval  none.
 */
static void IpFma_Msmc_PrintNullSlaveErrorInfo(IpFma_MsmcNullSlaveErrorInfo *info);

/**
 * \brief Registers a custom application-level handler for Data Abort exceptions.
 *
 * By default, the R5F runtime environment provides a weak Data Abort handler
 * that typically enters an infinite loop, providing no diagnostic info.
 * This function overwrites the 'dabtExptnHandler' member within the global
 * CSL_R5ExptnHandlers structure. When a Data Abort occurs, the low-level
 * hardware vector dispatcher will call this registered function instead of
 * the default, allowing for post-mortem analysis of the MSMC fault registers.
 *
 * \param dataAbortExptnHandler         Pointer to the user-defined exception handler function.
 * \param dataAbortExptnHandlerArgs     Pointer to the user-defined exception handler arguments.
 *
 * \retval  none.
 */
void IpFma_Msmc_UpdateDataAbortExptnHandler(void* dataAbortExptnHandler, MsmcIntApp_DataAbortExptnHandlerArgs* dataAbortExptnHandlerArgs);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void IpFma_Msmc_UpdateDataAbortExptnHandler(void* dataAbortExptnHandler, MsmcIntApp_DataAbortExptnHandlerArgs* dataAbortExptnHandlerArgs)
{
    gExptnHandlers.dabtExptnHandler = dataAbortExptnHandler;
    gExptnHandlers.dabtExptnHandlerArgs = dataAbortExptnHandlerArgs;
}

void IpFma_Msmc_DataAbortExptnHandler(MsmcIntApp_DataAbortExptnHandlerArgs* args)
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

    /**
     * Transactions that reach the null slave are recorded into global
     * MSMC Null Slave endpoint info object for later debug/viewing
     */
    IpFma_Msmc_GetNullSlaveErrorInfo(&args->errorInfo);

    /* Flag that a Data Abort exception has occurred */
    gDataAbortExptnInvoked = true;
}

static void IpFma_Msmc_GetNullSlaveErrorInfo(IpFma_MsmcNullSlaveErrorInfo *info)
{
    if (info != NULL)
    {
        // Get Null Slave Status 0
        uint64_t stat0 = ( ((uint64_t)CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + 0xA004)) << 32) | \
                                      CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + 0xA000);

        // Get Null Slave Status 1
        uint64_t stat1 = ( ((uint64_t)CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + 0xA00C)) << 32) | \
                                      CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + 0xA008);

        // Get Null Slave Status 0 - 63:0 ADDR (Address)
        info->addr      = (stat0 & CSL_MSMC_CFGS0_NULL_SLV_STAT0_ADDR_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT0_ADDR_SHIFT;

        // Get Null Slave Status 1 - 53:52 PRIV (Privilege)
        info->priv      = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_PRIV_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_PRIV_SHIFT);

        // Get Null Slave Status 1 - 48 SECURE (Secure)
        info->secure    = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_SECURE_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_SECURE_SHIFT);

        // Get Null Slave Status 1 - 44 EMU (Emulation)
        info->emu       = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_EMU_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_EMU_SHIFT);

        // Get Null Slave Status 1 - 41:40 MEMTYPE (Memory Type)
        info->memtype   = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_MEMTYPE_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_MEMTYPE_SHIFT);

        // Get Null Slave Status 1 - 37:32 OPCODE (Opcode)
        info->opcode    = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_OPCODE_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_OPCODE_SHIFT);

        // Get Null Slave Status 1 - 31:24 PRIVID (Priv ID)
        info->privid    = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_PRIVID_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_PRIVID_SHIFT);

        // Get Null Slave Status 1 - 23:12 ROUTEID (Route ID)
        info->routeid   = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_ROUTEID_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_ROUTEID_SHIFT);

        // Get Null Slave Status 1 - 9:0 BYTECNT (Byte Count)
        info->bytecnt   = (uint32_t)((stat1 & CSL_MSMC_CFGS0_NULL_SLV_STAT1_BYTECNT_MASK) >> CSL_MSMC_CFGS0_NULL_SLV_STAT1_BYTECNT_SHIFT);
    }
}

static void IpFma_Msmc_PrintNullSlaveErrorInfo(IpFma_MsmcNullSlaveErrorInfo *info)
{
    if (NULL != info)
    {
        UART_printf("\n|--------- INTERRUPT ANALYSIS ---------|");
        UART_printf("\n| SMESTAT  (Combined)     0x%08X", CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMESTAT));
        UART_printf("\n| SMIESTAT (Enabled)      0x%08X", CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMIESTAT));
        UART_printf("\n| SMIRSTAT (Triggered)    0x%08X", CSL_REG32_RD(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMIRSTAT));
        UART_printf("\n|--------------------------------------|\n");

        UART_printf("\n|---------- MSMC NULL SLAVE ERROR REPORT ----------|\n");

        /* ADDR: Full 64-bit address where the error was detected */
        UART_printf("| Fault Address                : 0x%08x%08x\n", (uint32_t)(info->addr >> 32), (uint32_t)(info->addr));

        /* PRIVID: Master identificator (core/perifpheral) */
        UART_printf("| Master identificator PRIVID  : 0x%x = %d\n", info->privid, info->privid);

        /* ROUTEID: Internal SoC path identificator */
        UART_printf("| Route identificator  ROUTEID : 0x%x\n", info->routeid);

        /* OPCODE: Operation type (Read/Write/Atomic...) */
        UART_printf("| Operation type (r/w) OPCODE  : 0x%x\n", info->opcode);

        /* BYTECNT: Number of bytes in transaction */
        UART_printf("| Bytes in transaction BYTECNT : %u bytes\n", info->bytecnt);

        /* MEMTYPE: Memory space type */
        UART_printf("| Memory Type MEMTYPE          : %u\n", info->memtype);

        /* PRIV: Privilege */
        UART_printf("| Privilege   PRIV             : %u\n", info->priv);

        /* SECURE: Secure */
        UART_printf("| Secure      SECURE           : %u\n", info->secure);

        /* EMU: Emulation */
        UART_printf("| Emulation   EMU              : %u\n", info->emu);

        UART_printf("|--------------------------------------------------|\n");
    }
}

/*
 * Application main
 */
int main(void)
{
    int32_t retVal = CSL_PASS;

    /* Register a custom Data Abort exception handler */
    IpFma_Msmc_UpdateDataAbortExptnHandler((exptnHandlerPtr) IpFma_Msmc_DataAbortExptnHandler, &gDataAbortExptnHandlerArgs);

    /* Board initialization */
    Board_initCfg boardCfg = BOARD_INIT_MODULE_CLOCK |
                             BOARD_INIT_PINMUX_CONFIG |
                             BOARD_INIT_UNLOCK_MMR |
                             BOARD_INIT_UART_STDIO;

    if (BOARD_SOK == Board_init(boardCfg))
    {
        // Clear interrupt raw status register
        CSL_REG64_WR(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMIRC, 0x01u);

        // Enable NULL SLAVE
        CSL_REG64_WR(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMIEWS, 0x01u);

        // Invalid transaction - Invalid Memory Range (ADDRESSING ERROR)
        uint32_t invalidTransactionAddress = CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE + CSL_MSMC_CFGS0_SMIEWS;
        CSL_REG8_WR(invalidTransactionAddress, 0xff);

        /* Wait for data abort exception */
        uint32_t timeOutCnt = 0U;
        bool exceptionInvoked = true;
        do
        {
            Board_delay(MSMC_INT_INTERRUPT_WAIT_DELAY);
            timeOutCnt += MSMC_INT_INTERRUPT_WAIT_DELAY;
            if (timeOutCnt > MSMC_INT_INTERRUPT_WAIT_TIMEOUT)
            {
                UART_printf("\r\n Timed out while waiting for the interrupt.");
                exceptionInvoked = false;
                break;
            }
        } while (false == gDataAbortExptnInvoked);

        if (true == exceptionInvoked)
        {
            bool exceptionInvokedForValidReasons  = true;
            if ((invalidTransactionAddress == gDataAbortExptnHandlerArgs.address) && \
                (invalidTransactionAddress == (uint32_t) gDataAbortExptnHandlerArgs.errorInfo.addr))
            {
                UART_printf("\n Data abort exception was invoked on address %x!", gDataAbortExptnHandlerArgs.address);
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was not invoked on correct address %x!", gDataAbortExptnHandlerArgs.address);
                exceptionInvokedForValidReasons  = false;
            }

            if (0x08 == gDataAbortExptnHandlerArgs.type)
            {
                UART_printf("\n Data abort exception was invoked due to Synchronous External Abort");
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was not invoked due to Synchronous External Abort!");
                exceptionInvokedForValidReasons  = false;
            }

            if (0U == gDataAbortExptnHandlerArgs.cause)
            {
                UART_printf("\n Data abort exception was invoked due to read operation!");
                exceptionInvokedForValidReasons  = false;
            }
            else
            {
                UART_printf("\n ERROR: Data abort exception was invoked due to write operation!");
            }

            if (0 == gDataAbortExptnHandlerArgs.externalAbortCause)
            {
                UART_printf("\n AXI Decode error (DECERR) or AHB  error caused the abort\n");
            }
            else
            {
                UART_printf("\n ERROR: AXI Slave  error (SLVERR) or unsupported exclusive access\n");
                exceptionInvokedForValidReasons  = false;
            }

            IpFma_Msmc_PrintNullSlaveErrorInfo(&gDataAbortExptnHandlerArgs.errorInfo);

            if (true == exceptionInvokedForValidReasons )
            {
                UART_printf("All tests have passed!!\n");
            }
        }
    }
    else
    {
        UART_printf("\n ERROR: Board was not initialized correctly!!");
    }

    return retVal;
}