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
 *  \defgroup IP_FMA_R5F_APP_UTILS 
 */

/**
 *  \ingroup  IP_FMA_R5F_APP_UTILS
 *  \defgroup IP_FMA_R5F_APP_UTILS_INTERFACE interface for app utility functions.
 *
 *  @{
 */

/**
 *  \file     ip_fma_r5f_app_utils.h
 *
 *  \brief    R5F application utility functions
 *
 */

#ifndef IP_FMA_R5F_APP_UTILS_
#define IP_FMA_R5F_APP_UTILS_

/*	==========================================================================	*/
/*	                            Include Files                                 	*/
/*	==========================================================================	*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ip_fma_common.h>

/*	==========================================================================	*/
/*	                        Macros & Typedefs                            	*/
/*	==========================================================================	*/

/*	==========================================================================	*/
/*	                        Structure Declarations                            	*/
/*	==========================================================================	*/

/*	==========================================================================	*/
/*	                         Function Declarations                            	*/
/*	==========================================================================	*/

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
void IpFma_R5f_PrintRegReadStatus(IpFma_Status regReadStatus, const char* registerName);

/**
 *  \brief Initializes the application by setting the configuration.
 *
 *  This is a initialization function that sets the board configuration.
 *  It also configures the board by enabling UART module needed to print log
 *  messages to the user via UART console. This is used so the tests can
 *  print results back to us.
 *
 *  \return  BOARD_SOK in case of success or appropriate error code.
 */
static int32_t R5fApp_Init(void);

/**
 *  \brief R5fApp_Run
 *
 *  This function executes the application code.
 */
static void R5fApp_Run(void);

/*	==========================================================================	*/
/*	                      Static Function Definitions                         	*/
/*	==========================================================================	*/

#ifdef __cplusplus
}
#endif

#endif /*	#ifndef IP_FMA_R5F_APP_UTILS_	*/

/** @} */
