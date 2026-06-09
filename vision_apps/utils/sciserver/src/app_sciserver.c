/*
 *
 * Copyright (c) 2018-26 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <stdio.h>

#if !defined(MCU_PLUS_SDK)
#include <sciclient/sciclient.h>
#include <sciclient/sciserver_tirtos.h>
#else
#include <device_manager/sciclient.h>
#include <device_manager/sciserver_tirtos.h>
#include <SystemP.h>
#endif

#if defined(SOC_AM62A) || defined(SOC_J722S)
#if !defined(MCU_PLUS_SDK)
#include <ti/board/board.h>
#include <uart/UART.h>
#include <uart/UART_stdio.h>
#else
#include <kernel/dpl/DebugP.h>
#include <drivers/hw_include/csl_types.h>
#if !defined(THREADX)
extern     void vTaskDelete( void* xTaskToDelete );
#endif
#endif
#endif

/* High Priority for SCI Server - must be higher than Low priority task */
#if defined THREADX
#define SETUP_SCISERVER_TASK_PRI_HIGH   (26)
#else
#define SETUP_SCISERVER_TASK_PRI_HIGH   (5)
#endif
/*
 * Low Priority for SCI Server - must be higher than IPC echo test tasks
 * to prevent delay in handling Sciserver requests when test is performing
 * multicore ping/pong.
 */
#if defined THREADX
#define SETUP_SCISERVER_TASK_PRI_LOW    (27)
#else
#define SETUP_SCISERVER_TASK_PRI_LOW    (4)

#endif

#if defined(SOC_AM62A) || defined(SOC_J722S)
/* Stack size allocated for the sciserver task */
#define SCISERVER_TASK_STACK_SIZE                   (2U*1024U)

/* Stack memory alignment requirement for the sciserver task */
#define SCISERVER_TASK_STACK_ALIGNMENT              (32)

/* Stack buffers for user high and low priority tasks */
static uint8_t __attribute__((aligned(SCISERVER_TASK_STACK_ALIGNMENT))) gUserHiTaskStack[SCISERVER_TASK_STACK_SIZE];
static uint8_t __attribute__((aligned(SCISERVER_TASK_STACK_ALIGNMENT))) gUserLoTaskStack[SCISERVER_TASK_STACK_SIZE];
#endif

int32_t appSciserverSciclientInit()
{
    int32_t retVal = CSL_PASS;
    Sciclient_ConfigPrms_t  clientParams;

#if !defined(MCU_PLUS_SDK)
#if defined(SOC_AM62A)
    Board_initCfg   boardCfg;
#endif

    retVal = Sciclient_configPrmsInit(&clientParams);

    if(retVal==CSL_PASS)
    {
        retVal = Sciclient_boardCfgParseHeader(
                (uint8_t *) SCISERVER_COMMON_X509_HEADER_ADDR,
                &clientParams.inPmPrms, &clientParams.inRmPrms);
    }

#if defined(SOC_AM62A)
    if(retVal==CSL_PASS)
    {
        boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
        Board_init(boardCfg);
    }
#endif

    if(retVal==CSL_PASS)
    {
        retVal = Sciclient_init(&clientParams);
    }
#else
#if defined(SOC_AM62A)
#ifdef CPU_c7x_1
/* Only for C7x sysconfig is not used now.
 * For R5, sci client init is done by sysconfig generated files
 */
    int32_t retVal = CSL_PASS;
    Sciclient_ConfigPrms_t  clientParams;

    retVal = Sciclient_configPrmsInit(&clientParams);

    if(retVal==CSL_PASS)
    {
        retVal = Sciclient_boardCfgParseHeader(
                (uint8_t *) SCISERVER_COMMON_X509_HEADER_ADDR,
                &clientParams.inPmPrms, &clientParams.inRmPrms);
    }

    if(retVal==CSL_PASS)
    {
        retVal = Sciclient_init(&clientParams);
    }

#endif
#endif
#endif

    return retVal;
}

int32_t appSciserverSciclientDeInit()
{
    int32_t retVal = 0;

    retVal = Sciclient_deinit();
    if(retVal!=0)
    {
        #if defined(SOC_AM62A) || defined(SOC_J722S)

        #if !defined(MCU_PLUS_SDK)
        UART_printf("SCICLIENT: ERROR: Sciclient deinit failed !!!\n");
        #else
        DebugP_log("SCICLIENT: ERROR: Sciclient deinit failed !!!\n");
        #endif

        #else
        printf("SCICLIENT: ERROR: Sciclient deinit failed !!!\n");
        #endif
    }

    return retVal;
}

void appSciserverInit(void* arg0, void* arg1)
{

    int32_t retVal = CSL_PASS;
    Sciserver_TirtosCfgPrms_t serverParams;

    #if defined(SOC_AM62A) || defined(SOC_J722S)
    const char *version_str = NULL;
    const char *rmpmhal_version_str = NULL;
    #endif

    retVal = Sciserver_tirtosInitPrms_Init(&serverParams);

    serverParams.taskPriority[SCISERVER_TASK_USER_LO] = SETUP_SCISERVER_TASK_PRI_LOW;
    serverParams.taskPriority[SCISERVER_TASK_USER_HI] = SETUP_SCISERVER_TASK_PRI_HIGH;
    #if defined(SOC_AM62A) || defined(SOC_J722S)
    serverParams.hiTaskStack    =   gUserHiTaskStack;
    serverParams.loTaskStack    =   gUserLoTaskStack;
    serverParams.taskStackSize  =   SCISERVER_TASK_STACK_SIZE;
    #endif
    if(retVal==CSL_PASS)
    {
        retVal = Sciserver_tirtosInit(&serverParams);
    }

    #if defined(SOC_AM62A) || defined(SOC_J722S)
    version_str = Sciserver_getVersionStr();
    rmpmhal_version_str = Sciserver_getRmPmHalVersionStr();
    #if !defined(MCU_PLUS_SDK)
    UART_printf("##DM Built On: %s %s\r\n", __DATE__, __TIME__);
    UART_printf("##Sciserver Version: %s\r\n", version_str);
    UART_printf("##RM_PM_HAL Version: %s\r\n", rmpmhal_version_str);
    #else
    DebugP_log("##DM Built On: %s %s\r\n", __DATE__, __TIME__);
    DebugP_log("##Sciserver Version: %s\r\n", version_str);
    DebugP_log("##RM_PM_HAL Version: %s\r\n", rmpmhal_version_str);
    #endif

    if (retVal == CSL_PASS)
    {
    #if !defined(MCU_PLUS_SDK)
        UART_printf("##Starting Sciserver..... PASSED\r\n");

    #else
        DebugP_log("##Starting Sciserver..... PASSED\r\n");
    #endif
    }
    else
    {
    #if !defined(MCU_PLUS_SDK)
        UART_printf("Starting Sciserver..... FAILED\r\n");
    #else
        DebugP_log("Starting Sciserver..... FAILED\r\n");
    #endif
    }
    #endif

    #if defined(MCU_PLUS_SDK) && !defined(THREADX)
    vTaskDelete( NULL );
    #endif
}

void appSciserverDeInit()
{
    Sciserver_tirtosDeinit();

    return;
}
