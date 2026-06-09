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
 *  \defgroup IP_FMA_R5F_CPU3 R5F MPU Fault handler
 */

/**
 *  \ingroup  IP_FMA_R5F_CPU3
 *  \defgroup IP_FMA_R5F_CPU3_INTERFACE MPU fault handler interface.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f_cpu3.h
 *
 *  \brief    MPU fault handler API.
 *
 */

#ifndef IP_FMA_R5F_CPU3_
#define IP_FMA_R5F_CPU3_

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

/* Fault status types */
#define FAULT_STATUS_TYPE_ALIGNMENT         (1U)
#define FAULT_STATUS_TYPE_BACKGROUND        (0U)
#define FAULT_STATUS_TYPE_PERMISSION        (13U)

/* Fault status causes */
#define FAULT_STATUS_CAUSE_READ             (0U)
#define FAULT_STATUS_CAUSE_WRITE            (1U)

/**
 *  \brief R5F Data Abort Exception Handler Args
 */
typedef struct
{
    uint32_t cause;                         /**< Cause of the exception, 0 - read, 1 - write. */
    uint32_t type;                          /**< Type of the exception. */
    uint32_t externalAbortCause;            /**< Cause of the external abort(Only valid if the abort is external). */
    uint32_t address;                       /**< Address on which the read/write operation caused an exception. */
    uint32_t auxiliaryFaultStatusReg;       /**< Auxiliary Data Fault Status Register (ADFSR). */
} IpFma_R5fDataAbortExptnHandlerArgs;

/**
 *  \brief R5F Prefetch Abort Exception Handler Args
 */
typedef struct
{
    uint32_t type;                      /**< Type of the exception. */
    uint32_t externalAbortCause;        /**< Cause of the external abort(Only valid if the abort is external). */
    uint32_t address;                   /**< Address on which the prefetch operation caused an exception. */
    uint32_t auxiliaryFaultStatusReg;   /**< Auxiliary Instruction Fault Status Register (AIFSR). */
} IpFma_R5fPrefetchAbortExptnHandlerArgs;

/*	==========================================================================	*/
/*	                        Structure Declarations                            	*/
/*	==========================================================================	*/

/*	==========================================================================	*/
/*	                         Function Declarations                            	*/
/*	==========================================================================	*/

/**
 * \brief   This function is executed in case "data abort" exception is invoked.
 *
 *  This is a "naked" wrapper function written in assembly, it calls the real data abort exception handler.
 *  It is used to pass SPSR(Saved Program Status Register) and LR(Link Register) pointers in stack to the exception handler.
 *  It is necessary because when the C functions are called they mangle the stack by creating function stack space.
 *  Then inside the exception handler the LR is modified based on the instruction.
 *
 * \param	args   Data abort handler args
 *
 */
void IpFma_R5f_DataAbortExptnHandler(IpFma_R5fDataAbortExptnHandlerArgs* args);

/**
 * \brief   This function is executed in case "data abort" exception is invoked.
 *
 * \param	args                    Data abort handler args.
 * \param	spsrRegPtr              Saved program status register stack pointer
 * \param	failedInstrAddressPtr   Failed instruction address stack pointer
 *
 */
void IpFma_R5f_DataAbortExptnHandlerCallback(IpFma_R5fDataAbortExptnHandlerArgs* args,
                                             uint32_t* spsrRegPtr,
                                             uint32_t* failedInstrAddressPtr);

/**
 * \brief   This function is executed in case "prefetch abort" exception is invoked.
 *
 * \param	args   Prefetch abort handler args.
 *
 */
void IpFma_R5f_PrefetchAbortExptnHandler(IpFma_R5fPrefetchAbortExptnHandlerArgs* args);

/**
 * \brief   This function updates the "data abort" handler.
 *
 * \param   handlers                    Structure which contains all the handlers and their arguments.
 * \param	dataAbortExptnHandler       This function is executed in case "data abort" exception is invoked.
 * \param   dataAbortExptnHandlerArgs   These arguments are passed to the "data abort" exception handler to write exception details into.
 *
 */
void IpFma_R5f_UpdateDataAbortExptnHandler(CSL_R5ExptnHandlers* handlers, void* dataAbortExptnHandler,
                                           IpFma_R5fDataAbortExptnHandlerArgs* dataAbortExptnHandlerArgs);

/**
 * \brief   This function updates the "prefetch abort" handler.
 *
 * \param   handlers                    Structure which contains all the handlers and their arguments.
 * \param	prefetchAbortExptnHandler   This function is executed in case "prefetch abort" exception is invoked.
 * \param   dataAbortExptnHandlerArgs   These arguments are passed to the "prefetch abort" exception handler to write exception details into.
 *
 */
void IpFma_R5f_UpdatePrefetchAbortExptnHandler(CSL_R5ExptnHandlers* handlers, void* prefetchAbortExptnHandler,
                                               IpFma_R5fPrefetchAbortExptnHandlerArgs* prefetchAbortExptnHandlerArgs);

/**
 * \brief   Gets exception invoked counter variable.
 */
uint32_t IpFma_R5f_GetExceptionInvocationCount();

/*	==========================================================================	*/
/*	                      Static Function Definitions                         	*/
/*	==========================================================================	*/

_DEFINE_COPROCR_READ_FUNC(sctlr, p15, 0, c1, c0, 0)	    /*  System control register	 */
_DEFINE_COPROCR_WRITE_FUNC(sctlr, p15, 0, c1, c0, 0)	/*  System control register	 */

/*	MPU memory region programming registers	*/
_DEFINE_COPROCR_WRITE_FUNC(rbar, p15, 0, c6, c1, 0)	    /*	Region Base Address Registers  */
_DEFINE_COPROCR_WRITE_FUNC(rser, p15, 0, c6, c1, 2)	    /*	Region Size and Enable Registers  */
_DEFINE_COPROCR_WRITE_FUNC(racr, p15, 0, c6, c1, 4)	    /*	Region Access Control Registers	 */
_DEFINE_COPROCR_WRITE_FUNC(rgnr, p15, 0, c6, c2, 0)	    /*	Region Number Register	*/

#ifdef __cplusplus
}
#endif

#endif /*	#ifndef IP_FMA_R5F_CPU3_	*/

/** @} */
