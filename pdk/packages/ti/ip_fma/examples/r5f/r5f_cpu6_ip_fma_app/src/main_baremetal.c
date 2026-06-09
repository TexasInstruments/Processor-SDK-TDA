/*
 * Copyright (c) 2026 Texas Instruments Incorporated
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
 *  \file main_baremetal.c
 *
 *  \brief R5F CPU6 static registers periodic readback example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

#include <ip_fma_r5f.h>
#include <ip_fma_r5f_app_utils.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DELAY_MS                                  ((uint32_t)1000U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void R5fApp_PrintRegCompareStatus(IpFma_Status status, const char* registerName);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  This is an example of how to read and write to r5f static configuration registers.
 *  These registers as a set are called system control coprocessor.
 *  They can only be accessed through assembly code and majority of them require
 *  privileged mode to access if using a operating system.
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = R5fApp_Init();
    if (status != BOARD_SOK)
    {
        UART_printf("R5F APP not initialized successfully!");
    }
    else
    {
        R5fApp_Run();
    }

    return 0;
}

static void R5fApp_Run(void)
{
    /*  READING  */
    /**
     *  R5F registers are contained/grouped in structures which contain one or more registers.
     *  Every structure has a corresponding read and compare API.
     */
    IpFma_R5fCoreCtrlRegs r5fCoreCtrlRegsExp;
    IpFma_Status r5fCoreCtrlRegsStatus = IpFma_R5f_GetCoreCtrlRegs(&r5fCoreCtrlRegsExp);
    IpFma_R5f_PrintRegReadStatus(r5fCoreCtrlRegsStatus, "R5F Core Control");

    IpFma_R5fMpuRegionRegs r5fMpuRegionRegsExp[CSL_ARM_R5F_MPU_REGIONS_MAX];
    IpFma_Status r5fCoreMpuRegionRegsStatus = IpFma_R5f_GetMpuRegs(r5fMpuRegionRegsExp);
    IpFma_R5f_PrintRegReadStatus(r5fCoreMpuRegionRegsStatus, "MPU region");

    IpFma_R5fAtcmRegionReg atcmRegionRegExp;
    IpFma_Status atcmRegionRegStatus = IpFma_R5f_GetAtcmRegionReg(&atcmRegionRegExp);
    IpFma_R5f_PrintRegReadStatus(atcmRegionRegStatus, "R5F ATCM region");

    IpFma_R5fBtcmRegionReg btcmRegionRegExp;
    IpFma_Status btcmRegionRegStatus = IpFma_R5f_GetBtcmRegionReg(&btcmRegionRegExp);
    IpFma_R5f_PrintRegReadStatus(btcmRegionRegStatus, "R5F BTCM region");

    IpFma_R5fSlavePortCtrlReg slavePortControlRegExp;
    IpFma_Status slavePortControlRegStatus = IpFma_R5f_GetSlavePortCtrlReg(&slavePortControlRegExp);
    IpFma_R5f_PrintRegReadStatus(slavePortControlRegStatus, "R5F slave port control");

    IpFma_R5fCacheSizeSelReg cacheSizeSelRegExp;
    IpFma_Status cacheSizeSelRegStatus = IpFma_R5f_GetCacheSizeSelReg(&cacheSizeSelRegExp);
    IpFma_R5f_PrintRegReadStatus(cacheSizeSelRegStatus, "R5F cache size selection");

    IpFma_R5fPeriphIfRegionRegs periphIfRegionRegsExp;
    IpFma_Status periphIfRegionRegsStatus = IpFma_R5f_GetPeriphIfRegionRegs(&periphIfRegionRegsExp);
    IpFma_R5f_PrintRegReadStatus(periphIfRegionRegsStatus, "R5F peripheral interface region");

    IpFma_R5fThreadProcessIdsRegs threadProcessIdsRegsExp;
    IpFma_Status threadProcessIdsRegsStatus = IpFma_R5f_GetThreadProcessIdsRegs(&threadProcessIdsRegsExp);
    IpFma_R5f_PrintRegReadStatus(threadProcessIdsRegsStatus, "R5F thread and process ids");


    /*  These registers initial values are modified only to demonstrate register difference for readback example.  */
    slavePortControlRegExp.slavePortCtrlReg = 1;   /*  Sets bit one to one  */
    cacheSizeSelRegExp.cacheSizeSelReg = 2;        /*  Sets bit two to one  */

    /**
     *  Code below is an example of a periodic readback.
     */

    int test_period = 3;
    int iteration = 0;
    while (iteration < test_period)
    {
        IpFma_R5fCoreCtrlRegs r5fCoreCtrlRegsActual;
        r5fCoreCtrlRegsStatus = IpFma_R5f_GetCoreCtrlRegs(&r5fCoreCtrlRegsActual);
        IpFma_R5f_PrintRegReadStatus(r5fCoreCtrlRegsStatus, "R5F Core Control");

        IpFma_R5fMpuRegionRegs r5fMpuRegionRegsActual[CSL_ARM_R5F_MPU_REGIONS_MAX];
        r5fCoreMpuRegionRegsStatus = IpFma_R5f_GetMpuRegs(r5fMpuRegionRegsActual);
        IpFma_R5f_PrintRegReadStatus(r5fCoreMpuRegionRegsStatus, "MPU region");

        IpFma_R5fAtcmRegionReg atcmRegionRegActual;
        atcmRegionRegStatus = IpFma_R5f_GetAtcmRegionReg(&atcmRegionRegActual);
        IpFma_R5f_PrintRegReadStatus(atcmRegionRegStatus, "R5F ATCM region");

        IpFma_R5fBtcmRegionReg btcmRegionRegActual;
        btcmRegionRegStatus = IpFma_R5f_GetBtcmRegionReg(&btcmRegionRegActual);
        IpFma_R5f_PrintRegReadStatus(btcmRegionRegStatus, "R5F BTCM region");

        IpFma_R5fSlavePortCtrlReg slavePortControlRegActual;
        slavePortControlRegStatus = IpFma_R5f_GetSlavePortCtrlReg(&slavePortControlRegActual);
        IpFma_R5f_PrintRegReadStatus(slavePortControlRegStatus, "R5F slave port control");

        IpFma_R5fCacheSizeSelReg cacheSizeSelRegActual;
        cacheSizeSelRegStatus = IpFma_R5f_GetCacheSizeSelReg(&cacheSizeSelRegActual);
        IpFma_R5f_PrintRegReadStatus(cacheSizeSelRegStatus, "R5F cache size selection");

        IpFma_R5fPeriphIfRegionRegs periphIfRegionRegsActual;
        periphIfRegionRegsStatus = IpFma_R5f_GetPeriphIfRegionRegs(&periphIfRegionRegsActual);
        IpFma_R5f_PrintRegReadStatus(periphIfRegionRegsStatus, "R5F peripheral interface region");

        IpFma_R5fThreadProcessIdsRegs threadProcessIdsRegsActual;
        threadProcessIdsRegsStatus = IpFma_R5f_GetThreadProcessIdsRegs(&threadProcessIdsRegsActual);
        IpFma_R5f_PrintRegReadStatus(threadProcessIdsRegsStatus, "R5F thread and process ids");


        /*  Comparison of initially read and current register values  */
        IpFma_Status r5fCoreCtrlRegsCompareStatus = IpFma_R5f_CompareCoreCtrlRegs(&r5fCoreCtrlRegsExp, &r5fCoreCtrlRegsActual);
        R5fApp_PrintRegCompareStatus(r5fCoreCtrlRegsCompareStatus, "R5F Core Control");

        IpFma_Status r5fMpuRegionRegsCompareStatus = IpFma_R5f_CompareMpuRegs(r5fMpuRegionRegsExp, r5fMpuRegionRegsActual);
        R5fApp_PrintRegCompareStatus(r5fMpuRegionRegsCompareStatus, "MPU region");

        IpFma_Status atcmRegionRegCompareStatus = IpFma_R5f_CompareAtcmRegionReg(&atcmRegionRegExp, &atcmRegionRegActual);
        R5fApp_PrintRegCompareStatus(atcmRegionRegCompareStatus, "R5F ATCM region");

        IpFma_Status btcmRegionRegCompareStatus = IpFma_R5f_CompareBtcmRegionReg(&btcmRegionRegExp, &btcmRegionRegActual);
        R5fApp_PrintRegCompareStatus(btcmRegionRegCompareStatus, "R5F BTCM region");

        /*  These should fail  */
        UART_printf("\n\nThese should fail as the registers value has been modified on purpose!");
        IpFma_Status slavePortControlRegCompareStatus = IpFma_R5f_CompareSlavePortCtrlReg(&slavePortControlRegExp, &slavePortControlRegActual);
        R5fApp_PrintRegCompareStatus(slavePortControlRegCompareStatus, "R5F slave port control");

        IpFma_Status cacheSizeSelRegCompareStatus = IpFma_R5f_CompareCacheSizeSelReg(&cacheSizeSelRegExp, &cacheSizeSelRegActual);
        R5fApp_PrintRegCompareStatus(cacheSizeSelRegCompareStatus, "R5F cache size selection");
        UART_printf("\n");
        IpFma_Status periphIfRegionRegsCompareStatus = IpFma_R5f_ComparePeriphIfRegionRegs(&periphIfRegionRegsExp, &periphIfRegionRegsActual);
        R5fApp_PrintRegCompareStatus(periphIfRegionRegsCompareStatus, "R5F peripheral interface region");

        IpFma_Status threadProcessIdsRegsCompareStatus = IpFma_R5f_CompareThreadProcessIdsRegs(&threadProcessIdsRegsExp, &threadProcessIdsRegsActual);
        R5fApp_PrintRegCompareStatus(threadProcessIdsRegsCompareStatus, "R5F thread and process ids");

        Board_delay(DELAY_MS);
        iteration++;
    }

    UART_printf("\nAll tests have passed.");
}

static int32_t R5fApp_Init(void)
{
    UART_printf("\n ... Init \n\n");

    int32_t status = BOARD_FAIL;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}

/**
 *  \brief Simple utility function that prints whether the registers were equal or not based
 *         on the comparison results.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerName     [IN] Name of the register.
 *  \param regCompareStatus [IN] Value that the register is expected contain.
 *
 *  \return None.
 */
static void R5fApp_PrintRegCompareStatus(IpFma_Status regCompareStatus, const char* registerName)
{
    if (IPFMA_OK == regCompareStatus)
    {
        UART_printf("\nSuccess! The registers *%s* have not been modified.", registerName);
    }
    else
    {
        UART_printf("\nError! The registers *%s* have been modified.", registerName);
    }
}
