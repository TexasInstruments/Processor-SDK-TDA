/*
 *  Copyright (c) Texas Instruments Incorporated 2024-2025
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
 *  \file boot_app_main.c
 *
 *  \brief File containing main function implementation for boot app
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/sciclient/sciserver_tirtos.h>
#include "boot_app_priv.h"

#if defined(SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_power.h>
#elif defined(SOC_J7200)
#include <ti/board/src/j7200_evm/include/board_power.h>
#elif defined(SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_power.h>
#include <ti/board/src/j721s2_evm/include/board_ecu.h>
#elif defined(SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_power.h>
#include <ti/board/src/j784s4_evm/include/board_ecu.h>
#elif defined(SOC_J742S2)
#include <ti/board/src/j742s2_evm/include/board_power.h>
#include <ti/board/src/j742s2_evm/include/board_ecu.h>
#endif

#if defined(SAFETY_CHECKER_LOOP_ENABLED)
#if defined(SOC_J721E)
#include <soc/j721e/safety_checkers_regcfg.h>
#elif defined(SOC_J7200)
#include <soc/j7200/safety_checkers_regcfg.h>
#elif defined(SOC_J721S2)
#include <soc/j721s2/safety_checkers_regcfg.h>
#elif defined(SOC_J784S4) || defined (SOC_J742S2)
#include <soc/j784s4/safety_checkers_regcfg.h>
#endif
#endif

#if defined(BOOT_MMCSD) || defined(BOOT_EMMC_UDA)
#include "boot_app_mmcsd.h"
#elif defined(BOOT_OSPI)
#include "boot_app_ospi.h"
#if defined(CAN_RESP_TASK_ENABLED)
#include <ti/boot/sbl/soc/k3/sbl_qos.h>
#include "boot_app_can.h"
#endif
#endif
#if defined(SDL_SAFETY_TASK_ENABLED)
#include <osal/sdl_osal.h>
#include "bist.h"
#if !defined(SOC_J742S2)
#include "vtm.h"
#include "pok.h"
#include "tog.h"
#endif
#if defined(ECC_ENABLED)
#include "ecc.h"
#endif
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Test application stack size */
#define BOOT_APP_TASK_STACK               (10U * 1024U)

/* Task Priority Levels, CAN Task has the highest priority,
 * followed by the safety tasks as it is recommended to test them before the Boot Task */
#if defined(SDL_SAFETY_TASK_ENABLED)
#define BOOT_APP_BIST_TASK_PRIORITY      (9)
#if defined(SOC_J784S4)
#define BOOT_APP_VTM_TASK_PRIORITY       (8)
#define BOOT_APP_POK_TASK_PRIORITY       (6)
#define BOOT_APP_TOG_TASK_PRIORITY       (7)
#endif
#endif
#if defined(CAN_RESP_TASK_ENABLED)
#define BOOT_APP_CAN_TASK_PRIORITY        (10)
#endif
#define BOOT_APP_BOOT_TASK_PRIORITY       (5)

/* Uncomment the following for debug logs */
/* #define UART_PRINT_DEBUG */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined(SDL_SAFETY_TASK_ENABLED)
#if defined(SOC_J784S4)
/* Stack for the TOG task */
static uint8_t gBootAppTogStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppTogTask;
/* Start and End time for TOG task */
static uint64_t gBootAppTogTimeStart, gBootAppTogTimeFinish;
/* Semaphore to indicate TOG Task completion */
static SemaphoreP_Handle gBootAppTogCompletedSem = NULL;

/* Stack for the POK task */
static uint8_t gBootAppPokStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppPokTask;
/* Start and End time for POK task */
static uint64_t gBootAppPokTimeStart, gBootAppPokTimeFinish;
/* Semaphore to indicate POK Task completion */
static SemaphoreP_Handle gBootAppPokCompletedSem = NULL;

/* Stack for the VTM task */
static uint8_t gBootAppVtmStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppVtmTask;
/* Start and End time for VTM task */
static uint64_t gBootAppVtmTimeStart, gBootAppVtmTimeFinish;
/* Semaphore to indicate VTM Task completion */
static SemaphoreP_Handle gBootAppVtmCompletedSem = NULL;
#endif
/* Stack for the BIST task */
static uint8_t gBootAppBistStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppBistTask;
/* Start and End time for BIST task */
static uint64_t gBootAppBistTimeStart, gBootAppBistTimeFinish;
/* Semaphore to indicate BIST Task completion */
static SemaphoreP_Handle gBootAppBistCompletedSem = NULL;
#endif

/* Stack for the CAN response task */
#if defined(CAN_RESP_TASK_ENABLED)
static uint8_t gBootAppCanStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppCanTask;
#endif

/* Stack for the Boot task */
static uint8_t gBootAppTaskStack[BOOT_APP_TASK_STACK] __attribute__((aligned(32)));
TaskP_Handle gBootAppTask;
static uint64_t gBootAppTimeStart, gBootAppTimeFinish;

#if defined(SAFETY_CHECKER_LOOP_ENABLED)
volatile uint32_t gBootAppTimerIsrCount = 0;
#endif

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 * \brief  This function initializes SCI Server to process RM/PM Requests by other cores
 *
 * \param  None
 *
 * \return CSL_PASS for success, CSL_EFAIL for failure
 */
static uint32_t BootApp_setupSciServer(void);

/**
 * \brief  Function to set board muxes for Ethernet firmware if needed
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_ethMuxConfig(void);

/**
 * \brief  This function initializes PMU counter
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_armR5PmuCntrInit(void);

/**
 * \brief  This function calculates Time in Microseconds
 *
 * \param pmuCntrVal PMU counter value
 *
 * \return Time in microseconds
 */
static uint32_t BootApp_getTimeInMicroSec(uint32_t pmuCntrVal);

/**
 * \brief  Function to load boot images and boot cores
 *
 * \param  None
 *
 * \return None
 */
static uint32_t BootApp_loadImg(void);

/**
 * \brief  Function to request stage cores
 *
 * \param  stageNum stage number for which cores are requested
 *
 * \return CSL_PASS for success, CSL_EFAIL for failure
 */
static int32_t BootApp_requestCores(uint8_t stageNum);

/**
 * \brief  Function to release stage cores
 *
 * \param  stageNum stage number for which cores are released
 *
 * \return CSL_PASS for success, CSL_EFAIL for failure
 */
static int32_t BootApp_releaseCores(uint8_t stageNum);

/**
 * \brief  Boot App Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_bootTaskFxn(void* a0, void* a1);

#if defined(SAFETY_CHECKER_LOOP_ENABLED)
/**
 * \brief  Safety loop execution for PM, RM, and TIFS safety checker libraries
 *
 * \param  None
 *
 * \return CSL_PASS for success, CSL_EFAIL for failure
 */
static int32_t BootApp_safetyCheckerLoop(void);
#endif

#if defined(SDL_SAFETY_TASK_ENABLED)
#if defined(SOC_J784S4)
/**
 * \brief  TOG Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_togTaskFxn(void* a0, void* a1);

/**
 * \brief  POK Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_pokTaskFxn(void* a0, void* a1);

/**
 * \brief  VTM Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_vtmTaskFxn(void* a0, void* a1);
#endif
/**
 * \brief  BIST Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_bistTaskFxn(void* a0, void* a1);
#endif

#if defined(CAN_RESP_TASK_ENABLED)
/**
 * \brief  CAN Response Task Function
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_canTaskFxn(void* a0, void* a1);

/**
 * \brief  Function to configure Main Domain if SBL_USE_MCU_DOMAIN_ONLY is enabled.
 *         - Initialize Board Cfgs, PLLs, Module Clock for Main Domain and DDR
 *         - QoS settings
 *         - Change GTC Parent
 *
 * \param  None
 *
 * \return None
 */
static void BootApp_mainDomainSetup(void);
#endif

/**
 * \brief   Function to shutdown the pmic.
 * 
 * \param   None.
 * 
 * \return  Success or failure.
*/
int32_t BootApp_pmicShutdown(void);

/* Function Pointer used while reading data from the storage. */
extern int32_t  (*fp_readData)(void *dstAddr, void *srcAddr, uint32_t length);
extern void     (*fp_seek)(void *srcAddr, uint32_t location);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t main(void)
{
    TaskP_Params bootTaskParams;

#if defined(CAN_RESP_TASK_ENABLED)
    TaskP_Params canRespTaskParams;
    /* Take MCAN transceivers out of STB mode to the NORMAL Mode */
    BootApp_canEnableTransceivers();
#endif

    OS_init();

    BootApp_armR5PmuCntrInit();

    UART_printf("MCU R5F App started at %d usecs\r\n", BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM)));

#if defined(SDL_SAFETY_TASK_ENABLED)
#if defined(SOC_J784S4)
    /* Initialize the TOG task params */
    TaskP_Params togTaskParams;
    TaskP_Params_init(&togTaskParams);
    togTaskParams.priority       = BOOT_APP_TOG_TASK_PRIORITY;
    togTaskParams.stack          = gBootAppTogStack;
    togTaskParams.stacksize      = sizeof (gBootAppTogStack);
    gBootAppTogTask = TaskP_create(&BootApp_togTaskFxn, &togTaskParams);
    if (NULL == gBootAppTogTask)
    {
        OS_stop();
    }

    /* Initialize the POK task params */
    TaskP_Params pokTaskParams;
    TaskP_Params_init(&pokTaskParams);
    pokTaskParams.priority       = BOOT_APP_POK_TASK_PRIORITY;
    pokTaskParams.stack          = gBootAppPokStack;
    pokTaskParams.stacksize      = sizeof (gBootAppPokStack);
    gBootAppPokTask = TaskP_create(&BootApp_pokTaskFxn, &pokTaskParams);
    if (NULL == gBootAppPokTask)
    {
        OS_stop();
    }

    /* Initialize the VTM task params */
    TaskP_Params vtmTaskParams;
    TaskP_Params_init(&vtmTaskParams);
    vtmTaskParams.priority       = BOOT_APP_VTM_TASK_PRIORITY;
    vtmTaskParams.stack          = gBootAppVtmStack;
    vtmTaskParams.stacksize      = sizeof (gBootAppVtmStack);
    gBootAppVtmTask = TaskP_create(&BootApp_vtmTaskFxn, &vtmTaskParams);
    if (NULL == gBootAppVtmTask)
    {
        OS_stop();
    }
#endif
    /* Initialize the BIST task params */
    TaskP_Params bistTaskParams;
    TaskP_Params_init(&bistTaskParams);
    bistTaskParams.priority       = BOOT_APP_BIST_TASK_PRIORITY;
    bistTaskParams.stack          = gBootAppBistStack;
    bistTaskParams.stacksize      = sizeof (gBootAppBistStack);
    gBootAppBistTask = TaskP_create(&BootApp_bistTaskFxn, &bistTaskParams);
    if (NULL == gBootAppBistTask)
    {
        OS_stop();
    }
#endif

#if defined(CAN_RESP_TASK_ENABLED) 
    /* Initialize the task params */
    TaskP_Params_init(&canRespTaskParams);
    canRespTaskParams.priority    = BOOT_APP_CAN_TASK_PRIORITY;
    canRespTaskParams.stack       = gBootAppCanStack;
    canRespTaskParams.stacksize   = sizeof (gBootAppCanStack);
    gBootAppCanTask = TaskP_create(&BootApp_canTaskFxn, &canRespTaskParams);
    if(NULL == gBootAppCanTask)
    {
        OS_stop();
    }
#endif

    /* Initialize the task params */
    TaskP_Params_init(&bootTaskParams);
    bootTaskParams.priority       = BOOT_APP_BOOT_TASK_PRIORITY;
    bootTaskParams.stack          = gBootAppTaskStack;
    bootTaskParams.stacksize      = sizeof (gBootAppTaskStack);

    gBootAppTask = TaskP_create(&BootApp_bootTaskFxn, &bootTaskParams);
    if (NULL == gBootAppTask)
    {
        OS_stop();
    }

    OS_start();    /* Does not return */
    return(0);
}

#if defined(MPU1_HLOS_BOOT_ENABLED)
/* Function to clean the MCU R5 cache for a given start address and given memory size */
void BootApp_McuDCacheClean(void *addr, uint32_t size)
{
    /* Invalidate by MVA */
    CSL_armR5CacheWbInv((const void *)addr, uint32_to_int32(size), BTRUE);
    return;
}
#endif

#if defined(SBL_ECU_BUILD_FC) || defined(SBL_ECU_BUILD_SRV) || defined(SBL_ECU_BUILD_AVP4)
/* *  \brief Function to get the Board ECU Type
 *
 *  \param None
 *
 *  \return Board ECU Type (BOARD_ECU_FC, BOARD_ECU_SRV, BOARD_ECU_AVP4, BOARD_ECU_NONE)
 */
uint32_t SBL_getBoardEcuType(void){
#if defined(SBL_ECU_BUILD_FC)
    return BOARD_ECU_FC;
#elif defined(SBL_ECU_BUILD_SRV)
    return BOARD_ECU_SRV;
#elif defined(SBL_ECU_BUILD_AVP4)
    return BOARD_ECU_AVP4;
#else
    return BOARD_ECU_NONE;
#endif
}
#endif

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static uint32_t BootApp_setupSciServer(void)
{
    Sciserver_TirtosCfgPrms_t appPrms;
    Sciclient_ConfigPrms_t clientPrms;
    uint32_t ret = CSL_PASS;

    appPrms.taskPriority[SCISERVER_TASK_USER_LO] = 1;
    appPrms.taskPriority[SCISERVER_TASK_USER_HI] = 4;

    /* Sciclient needs to be initialized before Sciserver. Sciserver depends on
     * Sciclient API to execute message forwarding */
    ret = Sciclient_configPrmsInit(&clientPrms);
    if (ret == CSL_PASS)
    {
        ret = Sciclient_init(&clientPrms);
    }

    if (ret == CSL_PASS)
    {
        ret = Sciserver_tirtosInit(&appPrms);
    }

    /* Set PMIC Shutdown Function Pointer */
    if (ret == CSL_PASS)
    {
        ret = Sciclient_setPmicShutdownCb(BootApp_pmicShutdown);
    }

    if (ret == CSL_PASS)
    {
        UART_printf("Starting Sciserver..... PASSED\r\n");
    }
    else
    {
        UART_printf("Starting Sciserver..... FAILED\r\n");
    }

    return ret;
}

static void BootApp_armR5PmuCntrInit(void)
{
    uint32_t val;

    /* Configure PMU with User mode access */
    CSL_armR5PmuCfg(0, 0, 1);
    
    /* Read and clear the cycle counter overflow */
    val = CSL_armR5PmuReadCntrOverflowStatus();
    val &= 0x80000000; /* Bitwise AND with Mask */
    CSL_armR5PmuClearCntrOverflowStatus(val);
    
    /* Configure PMU Cycle Counter */
    CSL_armR5PmuCfgCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM, CSL_ARM_R5_PMU_EVENT_TYPE_CYCLE_CNT);
    
    /* Enable PMU counter */
    CSL_armR5PmuEnableAllCntrs(1);
    CSL_armR5PmuEnableCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM, 1);
    
    return;
}

#if defined(SDL_SAFETY_TASK_ENABLED)
#if defined(SOC_J784S4)
static void BootApp_togTaskFxn(void* a0, void* a1)
{
    /* Initialize the Semaphore */
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    gBootAppTogCompletedSem = SemaphoreP_create(0, &semParams);
    if(NULL == gBootAppTogCompletedSem)
    {
        UART_printf("Semaphore create failed\r\n");
    }

    gBootAppTogTimeStart = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    BootApp_togFxn();

    gBootAppTogTimeFinish = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    UART_printf("MCU TOG Task started at %d usecs and finished at %d usecs\r\n", (uint32_t)gBootAppTogTimeStart, (uint32_t)gBootAppTogTimeFinish);

    /* Post semaphore after TOG task completion so other tasks could start execution */
    SemaphoreP_post(gBootAppTogCompletedSem);

    return;
}

static void BootApp_pokTaskFxn(void* a0, void* a1)
{
    /* Initialize the Semaphore */
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    gBootAppPokCompletedSem = SemaphoreP_create(0, &semParams);
    if(NULL == gBootAppPokCompletedSem)
    {
        UART_printf("Semaphore create failed\r\n");
    }

    gBootAppPokTimeStart = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    BootApp_pokFxn();

    gBootAppPokTimeFinish = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    UART_printf("MCU POK Task started at %d usecs and finished at %d usecs\r\n", (uint32_t)gBootAppPokTimeStart, (uint32_t)gBootAppPokTimeFinish);

    /* Post semaphore after POK task completion so other tasks could start execution */
    SemaphoreP_post(gBootAppPokCompletedSem);

    return;
}

static void BootApp_vtmTaskFxn(void* a0, void* a1)
{
    /* Initialize the Semaphore */
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    gBootAppVtmCompletedSem = SemaphoreP_create(0, &semParams);
    if(NULL == gBootAppVtmCompletedSem)
    {
        UART_printf("Semaphore create failed\r\n");
    }

    gBootAppVtmTimeStart = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    BootApp_vtmFxn();

    gBootAppVtmTimeFinish = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    UART_printf("MCU VTM Task started at %d usecs and finished at %d usecs\r\n", (uint32_t)gBootAppVtmTimeStart, (uint32_t)gBootAppVtmTimeFinish);

    /* Post semaphore after VTM task completion so other tasks could start execution */
    SemaphoreP_post(gBootAppVtmCompletedSem);

    return;
}
#endif
static void BootApp_bistTaskFxn(void* a0, void* a1)
{
#if !defined(CAN_RESP_TASK_ENABLED)
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO | BOARD_INIT_UNLOCK_MMR;
    Board_init(boardCfg);
#else
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UNLOCK_MMR;
    Board_init(boardCfg);
#endif

    /* Initialize the Semaphore */
    SemaphoreP_Params semParams;
    SemaphoreP_Params_init(&semParams);
    gBootAppBistCompletedSem = SemaphoreP_create(0, &semParams);
    if(NULL == gBootAppBistCompletedSem)
    {
        UART_printf("Semaphore create failed\r\n");
    }

    /* SDL osal wrapper */
    BootApp_osalWrapper();

    gBootAppBistTimeStart = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    BootApp_bistFxn();

    gBootAppBistTimeFinish = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

    UART_printf("MCU Bist Task started at %d usecs and finished at %d usecs\r\n", (uint32_t)gBootAppBistTimeStart, (uint32_t)gBootAppBistTimeFinish);
    
    /* Post semaphore after BIST task completion so other tasks could start execution */
    SemaphoreP_post(gBootAppBistCompletedSem);
    
    return;
}
#endif

static uint32_t BootApp_getTimeInMicroSec(uint32_t pmuCntrVal)
{
    uint64_t mcu_clk_freq = SBL_MCU1_CPU0_FREQ_HZ;
    uint32_t cycles_per_usec = (mcu_clk_freq / 1000000);
    return (pmuCntrVal/cycles_per_usec);
}

#if defined(CAN_RESP_TASK_ENABLED)
static void BootApp_canTaskFxn(void* a0, void* a1)
{
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);
    BootApp_canResponseTest();
    return;
}

static void BootApp_mainDomainSetup()
{
    uint32_t       retVal;
    Sciclient_DefaultBoardCfgInfo_t boardCfgInfo;
    Board_initCfg  boardCfg;

    /* Sciclient Board Cfg set up for Main Domain */
    retVal = Sciclient_getDefaultBoardCfgInfo(&boardCfgInfo);
    if(CSL_PASS != retVal)
    {
        UART_printf("Sciclient get default Board Cfg... FAILED\r\n");
    }

    Sciclient_BoardCfgPrms_t bootAppBoardCfgPrms    = {
                                                       .boardConfigLow = (uint32_t)boardCfgInfo.boardCfgLow,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = boardCfgInfo.boardCfgLowSize,
                                                       .devGrp = DEVGRP_01
                                                      };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgPmPrms  = {
                                                       .boardConfigLow = (uint32_t)boardCfgInfo.boardCfgLowPm,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = boardCfgInfo.boardCfgLowPmSize,
                                                       .devGrp = DEVGRP_01
                                                      };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgRmPrms  = {
                                                       .boardConfigLow = (uint32_t)boardCfgInfo.boardCfgLowRm,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = boardCfgInfo.boardCfgLowRmSize,
                                                       .devGrp = DEVGRP_01
                                                      };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgSecPrms = {
                                                       .boardConfigLow = (uint32_t)boardCfgInfo.boardCfgLowSec,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = boardCfgInfo.boardCfgLowSecSize,
                                                       .devGrp = DEVGRP_01
                                                      };

    retVal = Sciclient_boardCfg(&bootAppBoardCfgPrms);
    if (retVal != CSL_PASS)
    {
         UART_printf("Sciclient_boardCfg() failed.\r\n");
    }
    retVal = Sciclient_boardCfgPm(&bootAppBoardCfgPmPrms);
    if (retVal != CSL_PASS)
    {
         UART_printf( "Sciclient_boardCfgPm() failed.\r\n");
    }
    retVal = Sciclient_boardCfgRm(&bootAppBoardCfgRmPrms);
    if (retVal != CSL_PASS)
    {
         UART_printf("Sciclient_boardCfgRm() failed.\r\n");
    }
    retVal = Sciclient_boardCfgSec(&bootAppBoardCfgSecPrms);
    if (retVal != CSL_PASS)
    {
         UART_printf("Sciclient_boardCfgSec() failed.\r\n");
    }

    /* Initialize PLLs, Clock for Main Domain and DDR */
    boardCfg = BOARD_INIT_PLL_MAIN | BOARD_INIT_MODULE_CLOCK_MAIN | BOARD_INIT_DDR;
    Board_init(boardCfg);

    /* SBL implements the SBL_SetQoS only for J721S2, J721E, J784S4, J742S2. */
#if !defined (SOC_J7200)
    SBL_SetQoS();
#endif

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined (SOC_J742S2)
    /* Change the GTC Parent to MAIN_PLL3_HSDIV1_CLKOUT
       Reason :
        - for J721S2
            - MAIN_PLL3 default frequency is 2 GHz
            - MAIN_PLL3_HSDIV1_CLKOUT, MAIN_PLL3_HSDIV0_CLKOUT has the same divider value of 8
        - for J784S4/J742S2
            - MAIN_PLL3 default frequency is 2.5 GHz
            - MAIN_PLL3_HSDIV1_CLKOUT, MAIN_PLL3_HSDIV0_CLKOUT has the same divider value of 10
        - By defalult MAIN_PLL3_HSDIV1_CLKOUT (first input parent of the GTC mux) is given as an input to the GTC
        - MAIN_PLL3_HSDIV0_CLKOUT is given as input to the CPSW2G RGMI. CPSW2G RGMI needs 250MHz and GTC needs 200 MHz
        - It is not possible to have 250 MHz for MAIN_PLL3_HSDIV0_CLKOUT (divider of 8 in case of J721S2 and 10 incase of J784S4/J742S2)
          and 200 MHz for MAIN_PLL3_HSDIV1_CLKOUT (divider of 8 in case of J721S2 and 10 incase of J784S4/J742S2) with the same MAIN_PLL3 frequency.
        - So change the parent of GTC clock to MAIN_PLL0_HSDIV6_CLKOUT */

    UART_printf("Setting GTC clock parent frequency.... \r\n");
    retVal = Sciclient_pmSetModuleClkParent(TISCI_DEV_GTC0,
                                            TISCI_DEV_GTC0_GTC_CLK,
                                            TISCI_DEV_GTC0_GTC_CLK_PARENT_POSTDIV3_16FFT_MAIN_0_HSDIVOUT6_CLK,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (CSL_PASS != retVal)
    {
        UART_printf("Failed to set GTC clock parent \r\n");
    }
    else
    {
        UART_printf("Setting GTC clock parent frequency....done \r\n");
    }
#endif

    return;
}
#endif

static void BootApp_bootTaskFxn(void* a0, void* a1)
{
#if defined(SDL_SAFETY_TASK_ENABLED)
#if defined(SOC_J784S4)
    /* Wait for the TOG task completion */
    SemaphoreP_pend(gBootAppTogCompletedSem, SemaphoreP_WAIT_FOREVER);

    /* Wait for the POK task completion */
    SemaphoreP_pend(gBootAppPokCompletedSem, SemaphoreP_WAIT_FOREVER);

    /* Wait for the VTM task completion */
    SemaphoreP_pend(gBootAppVtmCompletedSem, SemaphoreP_WAIT_FOREVER);
#endif
    /* Wait for the BIST task completion */
    SemaphoreP_pend(gBootAppBistCompletedSem, SemaphoreP_WAIT_FOREVER);
#endif

#if !defined(CAN_RESP_TASK_ENABLED) && !defined(SDL_SAFETY_TASK_ENABLED)
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);
#endif
    uint32_t ret = CSL_PASS;

    ret = BootApp_setupSciServer();

#if defined(SBL_ECU_BUILD_FC) || defined(SBL_ECU_BUILD_SRV) || defined(SBL_ECU_BUILD_AVP4) 
    UART_printf("ECU_BUILD is enabled ... BOARD_ECU_TYPE: %d\n", SBL_getBoardEcuType());
    if (CSL_PASS != Board_ecuInit(SBL_getBoardEcuType()))
    {
        UART_printf("Board_ecuInit ...FAILED \n");
    }
#endif

    if(ret != CSL_PASS)
    {
        UART_printf("Failed to setup sciserver for boot app\r\n");
    }
    else
    {
        UART_printf("MCU R5F App started at %d usecs\r\n", BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM)));

        gBootAppTimeStart = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

        BootApp_loadImg();

        gBootAppTimeFinish = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));

        UART_printf("MCU Boot Task started at %d usecs and finished at %d usecs\r\n", (uint32_t)gBootAppTimeStart, (uint32_t)gBootAppTimeFinish);
    }
    return;
}

#if defined(SAFETY_CHECKER_LOOP_ENABLED)
void BootApp_timerIsr(void *arg)
{
    gBootAppTimerIsrCount = 1;
}

static int32_t BootApp_safetyCheckerLoop()
{
    int32_t      retVal = CSL_PASS;

#if defined(SC_REGDUMP_ENABLE)
    uint32_t pscCnt = 0;
    uint32_t pllCnt = 0;
    uint32_t rmCnt = 0;
    uint32_t tifsCnt = 0, regCnt = 0;

    /* Delay print out of boot log to avoid prints by other tasks */
    TaskP_sleep(15000);

    /* Get the PSC register dump */
    retVal = SafetyCheckers_pmGetPscRegCfg(pm_pscRegCfg, SAFETY_CHECKERS_PM_PSC_REGDUMP_SIZE);

    if(retVal == CSL_PASS)
    {
        UART_printf("static uintptr_t  pm_pscRegCfg[] = {\n");
        for(pscCnt = 0; pscCnt < SAFETY_CHECKERS_PM_PSC_REGDUMP_SIZE; pscCnt++)
        {
            UART_printf("0x%08x,\n",(uint32_t)pm_pscRegCfg[pscCnt]);
        }
        UART_printf("};\n\n");
    }

    /* Get the PLL register dump */
    retVal = SafetyCheckers_pmGetPllRegCfg(pm_pllRegCfg, SAFETY_CHECKERS_PM_PLL_REGDUMP_SIZE);

    if(retVal == CSL_PASS)
    {
        UART_printf("static uintptr_t  pm_pllRegCfg[] = {\n");
        for(pllCnt = 0; pllCnt < SAFETY_CHECKERS_PM_PLL_REGDUMP_SIZE; pllCnt++)
        {
            UART_printf("0x%08x,\n",(uint32_t)pm_pllRegCfg[pllCnt]);
        }
        UART_printf("};\n\n");
    }

    /* Get the RM register dump */
    retVal = SafetyCheckers_rmGetRegCfg(rm_regCfg, SAFETY_CHECKERS_RM_REGDUMP_SIZE);

    if(retVal == CSL_PASS)
    {
        UART_printf("static uintptr_t rm_regCfg[] __attribute__((section(\".data_buffer\")));\n");
        UART_printf("static uintptr_t rm_regCfg[] __attribute__((aligned (4096)));\n");
        UART_printf("static uintptr_t rm_regCfg[] = {\n");
        for(rmCnt = 0; rmCnt < SAFETY_CHECKERS_RM_REGDUMP_SIZE; rmCnt++)
        {
            UART_printf("0x%08x,\n",(uint32_t)rm_regCfg[rmCnt]);
        }
        UART_printf("};\n\n");
    }

    retVal = SafetyCheckers_tifsReqFwlOpen();

    if(retVal == CSL_PASS)
    {
        /* Get the TIFS register dump */
        retVal = SafetyCheckers_tifsGetFwlCfg(tifs_fwlConfig, TIFS_CHECKER_FWL_MAX_NUM);
    }

    if(retVal == CSL_PASS)
    {
        UART_printf("SafetyCheckers_TifsFwlConfig tifs_fwlConfig[TIFS_CHECKER_FWL_MAX_NUM] = {\n");
        for(tifsCnt = 0; tifsCnt < TIFS_CHECKER_FWL_MAX_NUM; tifsCnt++)
        {
            UART_printf("{\n");
            UART_printf("   %dU,    /* fwlId */\n",(uint32_t)tifs_fwlConfig[tifsCnt].fwlId);
            UART_printf("   %dU,    /* numRegions */\n",(uint32_t)tifs_fwlConfig[tifsCnt].numRegions);
            UART_printf("   %dU,    /* maxNumRegions */\n",(uint32_t)tifs_fwlConfig[tifsCnt].maxNumRegions);
            UART_printf("   {      /* Firewall registers for a given region : {controlReg, privId0, privId1, privId2, startAddrLow, startAddrHigh, endAddrLow, endAddrHigh} */\n");

            for (regCnt =0; regCnt< (tifs_fwlConfig[tifsCnt].maxNumRegions); regCnt++)
            {
                UART_printf("       {");
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].controlReg);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].privId0);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].privId1);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].privId2);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].startAddrLow);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].startAddrHigh);
                UART_printf("0x%xU, ",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].endAddrLow);
                UART_printf("0x%xU",(uint32_t)tifs_fwlConfig[tifsCnt].fwlCfgPerRegion[regCnt].endAddrHigh);
                UART_printf("},\n");
            }
            UART_printf("   },\n");
            UART_printf("},\n");
        }
        UART_printf("};\n\n");
    }

    if(retVal != CSL_PASS)
    {
        UART_printf("Get register configuration dump failed !!!\r\n");
    }

#else
    uint32_t      numInt = BOOT_APP_SAFETY_CHECKERS_TIMER_MAX_INTERRUPTS;
    TimerP_Params timerParams;
    TimerP_Handle handle;

    TimerP_Params_init(&timerParams);
    timerParams.runMode    = TimerP_RunMode_CONTINUOUS;
    timerParams.startMode  = TimerP_StartMode_USER;
    timerParams.periodType = TimerP_PeriodType_MICROSECS;
    timerParams.period     = BOOT_APP_SAFETY_CHECKERS_TIMER_PERIOD;

    handle = TimerP_create(BOOT_APP_SAFETY_CHECKERS_TIMER_ID, (TimerP_Fxn)&BootApp_timerIsr, &timerParams);

    TimerP_start(handle);

    while (numInt > 0)
    {
        if (gBootAppTimerIsrCount == 1)
        {
            gBootAppTimerIsrCount = 0U;

            /* validate PSC register config with current value */
            retVal = SafetyCheckers_pmVerifyPscRegCfg(pm_pscRegCfg, SAFETY_CHECKERS_PM_PSC_REGDUMP_SIZE);

            if(retVal == CSL_PASS)
            {
                /* validate PLL register config with current value */
                retVal = SafetyCheckers_pmVerifyPllRegCfg(pm_pllRegCfg, SAFETY_CHECKERS_PM_PLL_REGDUMP_SIZE);
            }

            if(retVal == CSL_PASS)
            {
                /* validate RM register config with current value */
                retVal = SafetyCheckers_rmVerifyRegCfg(rm_regCfg, SAFETY_CHECKERS_RM_REGDUMP_SIZE);
            }

            if(retVal == CSL_PASS)
            {
                retVal = SafetyCheckers_tifsReqFwlOpen();
                /* validate TIFS FW register config with current value */
                retVal = SafetyCheckers_tifsVerifyFwlCfg(tifs_fwlConfig, TIFS_CHECKER_FWL_MAX_NUM);
            }

            if(retVal == CSL_PASS)
            {
                retVal = SafetyCheckers_pmRegisterLock();
            }

            if(retVal != CSL_PASS)
            {
                TimerP_stop(handle);
                break;
            }

            numInt--;
        }
    }

    if(retVal == CSL_PASS)
    {
        UART_printf("Safety loop execution passed\r\n");
    }
    else
    {
        UART_printf("Safety loop execution failed !!!\r\n");
    }

    TimerP_delete(handle);

#endif

    return (retVal);
}
#endif

static uint32_t BootApp_loadImg(void)
{
    uint32_t       retVal;
    cpu_core_id_t  core_id;
    cpu_core_id_t  booted_core_ids[DSP2_C7X_ID];
    uint8_t        i, j;
    cpu_core_id_t *boot_array;
    uint8_t        num_cores_to_boot;
    uint8_t        num_booted_cores = 0;
    uint64_t       time_boot_core_finish[DSP2_C7X_ID];
#if defined(BOOT_OSPI)
    bool           isNandBootEnabled = BFALSE;

#if defined(CAN_RESP_TASK_ENABLED)
    /* To measure CAN response, sbl_boot_perf_cust_img is used which doesn't setup the Main Domain
     * and doesn't initialize DDR for faster Boot time.
     * Setting up Main Domain and initializing DDR before Booting cores */
    BootApp_mainDomainSetup();
#endif
#if defined(BOOT_OSPI_NAND)
    isNandBootEnabled = BTRUE;
#endif
    SBL_SPI_init();
    SBL_ospiInit(&boardHandle,isNandBootEnabled);
#endif

    /* Configure Muxes for Ethernet Firmware, if needed */
    BootApp_ethMuxConfig();

#if defined(BOOT_MMCSD) || defined(BOOT_EMMC_UDA)
    retVal = BootApp_mmcBootImageInit();
    if (retVal != CSL_PASS)
    {
        UART_printf("Failure during BootApp_mmcBootImageInit\r\n");
    }
#endif

#if defined(MPU1_HLOS_BOOT_ENABLED)
    /* Unlock control MMRs */
    retVal = Board_unlockMMR();
    if (retVal != CSL_PASS)
    {
        UART_printf("Board_unlockMMR ...FAILED \n");
    }
#endif

    /* Initialize the entry point array to SBL_INVALID_ENTRY_ADDR */
    for (core_id = MPU1_CPU0_ID; core_id < NUM_CORES; core_id ++)
    {
        (&gK3xx_evmEntry)->CpuEntryPoint[core_id] = SBL_INVALID_ENTRY_ADDR;
    }

    for (j = 0; j < NUM_BOOT_STAGES; j++)
    {

        retVal = BootApp_requestCores(j);

        if (retVal != CSL_PASS)
        {
            UART_printf("Failed to request all late cores in Stage %d\r\n", j);
            BootApp_releaseCores(j);
        } 
        else
        {
            UART_printf("Loading BootImage\r\n");

            #if defined(BOOT_MMCSD) || defined(BOOT_EMMC_UDA)
                retVal = BootApp_mmcsdBootStageImage(&gK3xx_evmEntry, mmcsd_main_domain_rtos_image_name[j]);
            #endif

            #if defined(BOOT_OSPI)
                retVal = BootApp_ospiBootStageImage(&gK3xx_evmEntry, ospi_main_domain_flash_rtos_images[j]);
            #endif

            UART_printf("BootImage completed, status = %d\r\n", retVal);

            if (retVal != CSL_PASS)
            {
                UART_printf("Failure during image copy and parsing\r\n");
            }
            else
            {
                retVal = BootApp_releaseCores(j);
                if (retVal != CSL_PASS)
                {
                    UART_printf("Failed to release all late cores\r\n");
                }
            }
        } /* if (retVal != CSL_PASS) */

        if (retVal == CSL_PASS)
        {
            /* Start the individual cores for the boot stage */
            num_cores_to_boot = num_cores_per_boot_stage[j];
            boot_array        = boot_array_stage[j];
            for (i = 0; i < num_cores_to_boot; i++)
            {
                core_id = boot_array[i];
                
                /* Try booting all cores other than the cluster running the SBL */
                if ((gK3xx_evmEntry.CpuEntryPoint[core_id] != SBL_INVALID_ENTRY_ADDR) &&
                    ((core_id != MCU1_CPU1_ID) && (core_id != MCU1_CPU0_ID)))
                {
                    SBL_SlaveCoreBoot(core_id, 0, &gK3xx_evmEntry, SBL_REQUEST_CORE);
                    UART_printf("SBL_SlaveCoreBoot completed for Core ID#%d, Entry point is 0x%x\r\n",
                                    core_id, gK3xx_evmEntry.CpuEntryPoint[core_id]);
                    booted_core_ids[num_booted_cores] = core_id;
                    time_boot_core_finish[num_booted_cores] = BootApp_getTimeInMicroSec(CSL_armR5PmuReadCntr(CSL_ARM_R5_PMU_CYCLE_COUNTER_NUM));
                    num_booted_cores++;
                }
            }
        } /* if (retVal == CSL_PASS) */
    } /* for (j = 0; j < NUM_BOOT_STAGES; j++) */

#if defined(BOOT_MMCSD) || defined(BOOT_EMMC_UDA)
#if !defined(ECC_ENABLED)
    BootApp_mmcBootImageDeInit();
#endif
#endif

#if defined(BOOT_OSPI)
    SBL_ospiClose(&boardHandle);
    BootApp_ospiLeaveConfigSPI(isNandBootEnabled);
#endif

    /* Delay print out of boot log to avoid prints by other tasks */
    TaskP_sleep(4000);

    if (retVal == CSL_PASS)
    {
        /* Print boot log, including all gathered timestamps */
        UART_printf("Boot App: Started at %d usec\r\n", (uint32_t)gBootAppTimeStart);

        UART_printf("Boot App: Total Num booted cores = %d\r\n", num_booted_cores);

        for (core_id = 0; core_id < num_booted_cores; core_id++)
        {
            UART_printf("Boot App: Booted Core ID #%d at %d usecs\r\n", booted_core_ids[core_id], (uint32_t)time_boot_core_finish[core_id]);
        }
    } /* if (retVal == CSL_PASS) */
    else
    {
        UART_printf("Boot App: Failure occurred in boot sequence\r\n");
    }

#if defined(SAFETY_CHECKER_LOOP_ENABLED)
    retVal = BootApp_safetyCheckerLoop();
#endif

#if defined(ECC_ENABLED)
    /* Add 1ms delay to fix SDL_MCU_R5FSS0_0 ECC in OSPI Bootmode */
    Osal_delay(1U);
    retVal = BootApp_eccFxn();
#endif

    return (retVal);
}

static void BootApp_ethMuxConfig(void)
{
#if defined(SOC_J721E)
    Board_STATUS status;

    /* Ethernet config: set proper board muxes for J712E Eth. firmware
     * Set IO Expander to use RMII on GESI board */
    status = Board_control(BOARD_CTRL_CMD_SET_RMII_DATA_MUX, NULL);
    
    if (status != BOARD_SOK)
    {
        UART_printf("Board_control failed to configure RMII pins\r\n");
    }
    
    /* Enable CPSW9G MDIO mux */
    status = Board_control(BOARD_CTRL_CMD_SET_GESI_CPSW_MDIO_MUX, NULL);
    
    if (status != BOARD_SOK)
    {
        UART_printf("Board_control failed to configure CPSW9G MDIO mux\r\n");
    }
#endif
    return;
}

static int32_t BootApp_requestCores(uint8_t stageNum)
{
    uint32_t i;
    int32_t  status = CSL_EFAIL;
    uint8_t  stage  = stageNum;

    for (i = 0; i < MAX_CORES_PER_STAGE; i++)
    {
        if (sbl_late_slave_core_stages_info[stage][i].tisci_proc_id != SBL_INVALID_ID)
        {
        #if defined(UART_PRINT_DEBUG)
            UART_printf("Calling Sciclient_procBootRequestProcessor, ProcId 0x%x... \r\n",
                        sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
        #endif
            status = Sciclient_procBootRequestProcessor(sbl_late_slave_core_stages_info[stage][i].tisci_proc_id,
                                                        SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
            #if defined(UART_PRINT_DEBUG)
                UART_printf("Sciclient_procBootRequestProcessor, ProcId 0x%x...FAILED \r\n",
                            sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
            #endif
                break;
            }
        }
    }

    return (status);
}

static int32_t BootApp_releaseCores(uint8_t stageNum)
{
    uint32_t i;
    int32_t  status = CSL_EFAIL;
    uint8_t  stage  = stageNum;

    for (i = 0; i < MAX_CORES_PER_STAGE; i++)
    {
        if (sbl_late_slave_core_stages_info[stage][i].tisci_proc_id != SBL_INVALID_ID)
        {
        #if defined(UART_PRINT_DEBUG)
            UART_printf("Sciclient_procBootReleaseProcessor, ProcId 0x%x...\r\n",
                        sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
        #endif
            status = Sciclient_procBootReleaseProcessor(sbl_late_slave_core_stages_info[stage][i].tisci_proc_id,
                                                        TISCI_MSG_FLAG_AOP,
                                                        SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
                UART_printf("Sciclient_procBootReleaseProcessor, ProcId 0x%x...FAILED \r\n",
                            sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
                break;
            }
        }
    }

    return (status);
}

/* PMIC Shutdown Function */
int32_t BootApp_pmicShutdown(void)
{
    int32_t ret = CSL_PASS;
    Board_STATUS status = -1;

    status = Board_pmPowerOff(0x48);
    if (status != 0) {
        ret = CSL_EFAIL;
    }

#if defined(SOC_J721E)
    if (ret == CSL_PASS) {
        status = Board_pmPowerOff(0x4C);
        if (status != 0) {
            ret = CSL_EFAIL;
        }
    }
#endif

    return ret;
}
