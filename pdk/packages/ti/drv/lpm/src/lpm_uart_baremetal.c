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
 * \file lpm_uart_baremetal.c
 *
 * \brief Baremetal UART debug print implementation for lpm_stub.
 *
 * Provides a minimal direct-register UART driver for printing debug messages
 * from lpm_stub code during low power entry.
 * The entire implementation is compiled only when -DLPM_DEBUG is defined.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ti/csl/soc.h>
#include <ti/csl/src/ip/uart/V1/hw_uart.h>
#include <ti/csl/src/ip/uart/V1/uart.h>
#include <ti/drv/lpm/src/lpm_uart_baremetal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Combined mask for both the TX shift register empty (TX_SR_E) and TX FIFO
 * empty (TX_FIFO_E) bits in UART_LSR. Both must be set before writing a new
 * byte to ensure the transmitter is fully idle.
 */
#define LPM_UART_LSR_TX_EMPTY_MASK  (UART_LSR_TX_SR_E_MASK | UART_LSR_TX_FIFO_E_MASK)

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

#if defined(LPM_DEBUG)

/**
 * \brief  Writes a byte on MCU_UART0, configuring the port on first use.
 *
 * This function is based on UART_putc from ti/drv/uart/src/UART_stdio.c.
 *
 * \param  byteTx The byte to be transmitted.
 *
 */
static void Lpm_uartPutC(uint8_t byteTx);

/**
 * \brief  Writes data from a specified buffer onto the transmitter
 *         FIFO of UART.
 *
 * \param  str  Pointer to a buffer to transmit.
 * \param  len  Number of bytes to be transmitted.
 *
 */
static void Lpm_uartPutS(const char *str, uint32_t len);

/**
 * \brief Writes a value in hex format to the UART output buffer.
 *
 * This function is a stripped down version of UART_convertVal from
 * ti/drv/uart/src/UART_stdio.c. Only the hexadecimal support is kept.
 *
 * \param ulValue  The value of the resolved variable passed as argument.
 * \param ulPos    The buffer position of the printed value.
 * \param ulCount  Total number of characters to be printed.
 * \param cFill    The char to be placed between numbers.
 * \param pcBuf    Buffer storing all the numbers.
 *
 * \return 0.
 *
 */
static int32_t Lpm_uartConvertVal(uint32_t ulValue, uint32_t ulPos,
                                  uint32_t ulCount, char cFill, char *pcBuf);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_uartDebugPrintf(const char *pcString, ...)
{
    va_list  vaArgP;
    const char *pStr = pcString;
    char    pcBuf[LPM_UART_CHAR_BUFFER_LEN], cFill;
    uint32_t ulIdx, ulValue, ulPos, ulCount;

    /* Start the variable args processing. */
    (void)va_start(vaArgP, pcString);

    /* Loop while there are more characters in the string. */
    while (*pStr != (char)0U)
    {
        /* Find the first non-% character, or the end of the string. */
        for (ulIdx = 0; ((pStr[ulIdx] != (char) '%') && (pStr[ulIdx] != (char) '\0')); ulIdx++)
        {
            /* Scan for a null terminator or character that is not '%' */
        }

        /* Write this portion of the string. */
        (void)Lpm_uartPutS(pStr, ulIdx);

        /* Skip the portion of the string that was written. */
        pStr += ulIdx;

        /* See if the next character is a %. */
        if (*pStr == (char) '%')
        {
            /* Skip the %. */
            pStr++;

            /* Set the digit count to zero, and the fill character to space
             * (i.e. to the defaults).
             */
            ulCount = 0;
            cFill   = (char) ' ';

            /* Determine how to handle the next character. */
            while((*pStr >= (char)'0') && (*pStr <= (char)'9'))
            {
                /* If this is a zero, and it is the first digit, then the
                 * fill character is a zero instead of a space.
                 */
                if ((pStr[-1] == (char) '0') && (ulCount == 0U))
                {
                    cFill = (char) '0';
                }

                /* Update the digit count. */
                ulCount *= 10u;
                ulCount += ((uint32_t)(*pStr)) - (uint32_t) '0';

                /* Get the next character. */
                pStr++;
            }
            switch (*pStr)
            {
                /* Handle the %x and %X commands.  Note that they are treated
                 * identically; i.e. %X will use lower case letters for a-f
                 * instead of the upper case letters is should use.  We also
                 * alias %p to %x.
                 */
                case (char) 'x':
                case (char) 'X':
                case (char) 'p':
                {
                    /* Get the value from the varargs. */
                    ulValue = (uint32_t)va_arg(vaArgP, uint32_t);

                    /* Reset the buffer position. */
                    ulPos = 0;

                    /* Determine the number of digits in the string version of
                     * the value.
                     */
                    (void)Lpm_uartConvertVal(ulValue, ulPos, ulCount, cFill, pcBuf);

                    break;
                }

                /* Handle all other commands. */
                default:
                {
                    /* Indicate an error. */
                    (void)Lpm_uartPutS("ERROR", LPM_UART_PRINTF_ERROR_LEN);

                    /* This command has been handled. */
                    break;
                }
            }
            pStr++;
        }
    }

    /* End the variable args processing. */
    va_end(vaArgP);
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void Lpm_uartPutC(uint8_t byteTx)
{
    /* Configure the UART once: 8 data bits, 1 stop bit, no parity. */
    static bool Lpm_uartConfigured = false;
    if (Lpm_uartConfigured == false)
    {
        CSL_REG32_WR_OFF(CSL_MCU_UART0_BASE, UART_LCR,
                         UART_FRAME_WORD_LENGTH_8 | UART_FRAME_NUM_STB_1 | UART_PARITY_NONE);
        Lpm_uartConfigured = true;
    }

    uint32_t lcrRegValue = CSL_REG32_RD_OFF(CSL_MCU_UART0_BASE, UART_LCR);

    /* Clear DIV_EN (bit 7) so the data registers map to THR/RHR rather than
     * the divisor latch, allowing normal transmit/receive operation.
     */
    CSL_REG32_WR_OFF(CSL_MCU_UART0_BASE, UART_LCR, lcrRegValue & ~CSL_UART_LCR_DIV_EN_MASK);

    /* Wait until both the TX FIFO and the TX shift register are empty before
     * loading the next byte, so that no character is overwritten in flight.
     */
    while ((CSL_REG32_RD_OFF(CSL_MCU_UART0_BASE, UART_LSR) & LPM_UART_LSR_TX_EMPTY_MASK) !=
            LPM_UART_LSR_TX_EMPTY_MASK)
    {
        /* Busy-wait until both the TX FIFO and the TX shift register are empty,
         * confirming the UART is ready to accept the next byte.
         */
    }

    /* Write the byte to the TX holding register, then restore the LCR. */
    CSL_REG32_WR_OFF(CSL_MCU_UART0_BASE, UART_THR, byteTx);
    CSL_REG32_WR_OFF(CSL_MCU_UART0_BASE, UART_LCR, lcrRegValue);
}

static void Lpm_uartPutS(const char *str, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len && str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            Lpm_uartPutC('\r');
        }
        Lpm_uartPutC(str[i]);
    }
}

static int32_t Lpm_uartConvertVal(uint32_t ulValue, uint32_t ulPos, uint32_t ulCount, char cFill, char *pcBuf)
{
    const char *const g_pcHex = "0123456789abcdef";
    uint32_t ulBase = LPM_UART_CHAR_BUFFER_LEN;
    uint32_t ulIdx;
    uint32_t count = ulCount;
    uint32_t pos = ulPos;

    ulIdx = 1u;
    while (ulIdx <= (ulValue / ulBase))
    {
        ulIdx *= ulBase;
        count--;
    }

    /* Provide additional padding at the beginning of the
     * string conversion if needed.
     */
    if ((count > 1u) && (count < LPM_UART_CHAR_BUFFER_LEN))
    {
        for (count--; count != 0U; count--)
        {
            pcBuf[pos] = cFill;
            pos++;
        }
    }

    /* Convert the value into a string. */
    for (; ulIdx != 0U; ulIdx /= ulBase)
    {
        pcBuf[pos] = g_pcHex[(ulValue / ulIdx) % ulBase];
        pos++;
    }

    /* Write the string. */
    (void)Lpm_uartPutS(pcBuf, pos);

    return 0;
}

#endif /* LPM_DEBUG */
