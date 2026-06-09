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
 *  \ingroup  IP_FMA_R5F_CPU3
 *  \defgroup IP_FMA_R5F_CPU3_IMPLEMENTATION R5F MPU fault handler API implementation.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f_cpu3.c
 *
 *  \brief    R5F MPU specific wrappers for fault handler.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ip_fma_r5f.h>
#include <ip_fma_r5f_app_utils.h>

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*	==========================================================================	*/
/*	                          Macros & Typedefs                               	*/
/*	==========================================================================	*/

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  \brief Simple utility function that prints whether the register read was successful or not.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerName  [IN] Name of the register.
 *  \param regReadStatus [IN] Value that the register is expected contain.
 *
 *  \return None.
 */
void IpFma_R5f_PrintRegReadStatus(IpFma_Status regReadStatus, const char* registerName)
{
    if (IPFMA_OK == regReadStatus)
    {
        UART_printf("\nSuccess! The registers %s have been read successfully.", registerName);
    }
    else
    {
        UART_printf("\nError! The registers %s have not been read successfully.", registerName);
    }
}

/** @} */