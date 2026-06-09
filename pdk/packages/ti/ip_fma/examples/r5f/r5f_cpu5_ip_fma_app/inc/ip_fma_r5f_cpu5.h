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
 *  \defgroup IP_FMA_R5F_CPU5 R5F MPU Fault handler
 */

/**
 *  \ingroup  IP_FMA_R5F_CPU5
 *  \defgroup IP_FMA_R5F_CPU5_INTERFACE MPU fault handler interface.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f_cpu5.h
 *
 *  \brief    Undefined exception handler API.
 *
 */

#ifndef IP_FMA_R5F_CPU5_
#define IP_FMA_R5F_CPU5_

/*	==========================================================================	*/
/*	                            Include Files                                 	*/
/*	==========================================================================	*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ip_fma_common.h>
#include <ti/csl/arch/r5/interrupt.h>

/*	==========================================================================	*/
/*	                        Macros & Typedefs                            	*/
/*	==========================================================================	*/

/* CP10/CP11 FPU full access permission bits (CPACR[23:20]) */
#define R5F_APP_CPACR_FPU_ACCESS_PERMISSION_MASK        ((uint32_t)0xFU << 20U)

/* Enable division by zero to generate Undefined Instruction exception (SCTLR.DZ) */
#define R5F_APP_SCTLR_REG_DZ_MASK                       ((uint32_t)0x1U << 19U)

/**
 *  \brief R5F Undefined Instruction Exception Handler Args
 *
 * Populated by IpFma_R5f_UndefInstrExptnHandlerCallback each time
 * an undefined instruction exception is taken.
 *
 */
typedef struct
{
    IpFma_R5f_InstructionSet instrSet;  /**< Which instruction set mode was used to execute the undef instr. */
    uint32_t undefInstrEncoding;        /**< Undefined instruction encoding. */
    uint32_t address;                   /**< Address on which the prefetch operation caused an exception. */
} IpFma_R5f_UndefInstrExptnHandlerArgs;

/*	==========================================================================	*/
/*	                        Structure Declarations                            	*/
/*	==========================================================================	*/

/*	==========================================================================	*/
/*	                         Function Declarations                            	*/
/*	==========================================================================	*/

/**
 * \brief   This function is executed in case "Undefined Instruction" exception is invoked.
 *
 *  This is a "naked" wrapper function written in assembly, it calls the real undefined exception handler.
 *  It is used to pass SPSR(Saved Program Status Register) and LR(Link Register) pointers in stack to the exception handler.
 *  It is necessary because when the C functions are called they mangle the stack by creating function stack space.
 *  Then inside the exception handler the LR and SPSR are modified based on the undefined instruction.
 *
 * \param	args   Undefined exception handler args
 *
 */
void IpFma_R5f_UndefInstrExptnHandler(IpFma_R5f_UndefInstrExptnHandlerArgs* args);

/**
 * \brief   "Undefined Instruction" exception handler.
 *
 *  This function handles the undefined exception by either fixing the issue that caused the exception
 *  or by skipping the instruction and also logging the exception details.
 *
 * \param	args                    Undefined exception handler args
 * \param	spsrRegPtr              Saved program status register stack pointer
 * \param	linkRegisterPtr         Undefined instruction address stack pointer
 *
 */
void IpFma_R5f_UndefInstrExptnHandlerCallback(IpFma_R5f_UndefInstrExptnHandlerArgs* args,
                                                    uint32_t* spsrRegPtr,
                                                    uint32_t* linkRegisterPtr);

/**
 * \brief   This function updates the R5F exceptions handlers.
 *
 * \param   handlers                            Structure which contains all the handlers and their arguments.
 * \param	undefInstrExptnHandler        This function is executed in case undefined exception is invoked.
 * \param   undefInstrExptnHandlerArgs    These arguments are passed to the undefined exception handler to write exception details into.
 *
 */
void IpFma_R5f_UpdateUndefInstrExptnHandler(CSL_R5ExptnHandlers* handlers,
                                                  void* undefInstrExptnHandler,
                                                  IpFma_R5f_UndefInstrExptnHandlerArgs* undefInstrExptnHandlerArgs);

/**
 * \brief   Gets exception invoked counter variable.
 */
uint32_t IpFma_R5f_GetExceptionInvocationCount();

/*	==========================================================================	*/
/*	                      Static Function Definitions                         	*/
/*	==========================================================================	*/

_DEFINE_COPROCR_READ_FUNC(sctlr, p15, 0, c1, c0, 0)	    /* System control register */
_DEFINE_COPROCR_WRITE_FUNC(sctlr, p15, 0, c1, c0, 0)	/* System control register */

_DEFINE_COPROCR_READ_FUNC(cpacr, p15, 0, c1, c0, 2)	    /* Coprocessor Access Control Register	*/
_DEFINE_COPROCR_WRITE_FUNC(cpacr, p15, 0, c1, c0, 2)	/* Coprocessor Access Control Register	*/

#ifdef __cplusplus
}
#endif

#endif /*	#ifndef IP_FMA_R5F_CPU5_	*/

/** @} */
