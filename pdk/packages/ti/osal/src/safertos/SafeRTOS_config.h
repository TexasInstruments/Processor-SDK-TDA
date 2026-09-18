/*
 *  Copyright ( C ) 2022 - 2023 Texas Instruments Incorporated
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
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __SAFERTOS_CFG__
#define __SAFERTOS_CFG__

#if defined (BUILD_C7X) || defined (BUILD_MCU)
/* Minimal size for Task's stacks. */
#if defined (BUILD_C7X)
#define configMINIMAL_STACK_SIZE        ( 16 * 1024 )
#elif defined (BUILD_MCU)
#define configTIMER_STACK_SIZE_WITH_FPU         ( 16 * 1024U )
#define configMINIMAL_STACK_SIZE_WITH_FPU       ( 1024U )
#define configMINIMAL_STACK_SIZE_WITH_NO_FPU    ( 512U )
#define configINCLUDE_RUNTIMESTATS          ( 1 )
#endif

/* System tick rate. */
#define configCPU_CLOCK_HZ              ( 1000 * 1000 * 1000 )
#define configSYSTICK_CLOCK_HZ          ( 19200000 )
#define configTICK_RATE_HZ              ( ( portTickType ) 1000UL )
/* This macro calculates the number of ticks per millisecond.
 * NOTE: This integer calculation is only correct for values of
 * configTICK_RATE_HZ less than or equal to 1000 that are also divisors
 * of 1000.
 */
#define configTICK_RATE_MS              ( ( portTickType ) 1000UL / configTICK_RATE_HZ )

/* The user configuration for the timer module. */
#define configTIMER_TASK_PRIORITY       ( 2U )
#define configTIMER_QUEUE_LENGTH        ( 10U )
#endif

#endif
