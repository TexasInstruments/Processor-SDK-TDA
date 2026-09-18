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
 * \file lpm_uart_baremetal.h
 *
 * \brief Baremetal UART debug print API for lpm_stub.
 *
 * Pass -DLPM_DEBUG to the compiler to enable debug output on MCU_UART0.
 * Without -DLPM_DEBUG both macros expand to nothing.
 *
 */

#ifndef LPM_UART_BAREMETAL_H_
#define LPM_UART_BAREMETAL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define LPM_UART_CHAR_BUFFER_LEN                (16U)
#define LPM_UART_PRINTF_ERROR_LEN               (5U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if defined(LPM_DEBUG)

/**
 * A simple UART based printf function supporting \%p, \%x, and \%X.
 *
 * \param pcString is the format string.
 * \param ... are the optional arguments, which depend on the contents of the
 * format string.
 *
 * This function is a small version of UART_printf from
 * ti/drv/uart/src/UART_stdio.c.
 * Only the support of \%p, \%x and \%X was kept.
 *
 * \return None.
 */
void Lpm_uartDebugPrintf(const char *pcString, ...);

/**
 * \brief Verbose debug print — active only when LPM_DEBUG is defined.
 *        Maps directly to Lpm_uartDebugPrintf.
 */
#define Lpm_uartDebugFullPrintf Lpm_uartDebugPrintf

#else /* LPM_DEBUG not defined */

#define Lpm_uartDebugPrintf(...)
#define Lpm_uartDebugFullPrintf(...)

#endif /* LPM_DEBUG */

#endif /* LPM_UART_BAREMETAL_H_ */
