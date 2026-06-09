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
 *  \brief DSS registers periodic readback example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/drv/dss/dss.h>

#include "ti/ip_fma/inc/ip_fma_dss.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DELAY_MS                ((uint32_t)1000U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t DssApp_Init(void);
static void DssApp_Run(void);
static void DssApp_PrintRegReadStatus(IpFma_Status status, const char* registerName);
static void DssApp_PrintRegCompareStatus(IpFma_Status status, const char* registerName);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    int32_t status = BOARD_FAIL;
    status = DssApp_Init();
    if (status != FVID2_SOK)
    {
        UART_printf("\n[DSS APP]: Board Init failed \n");
    }
    else
    {
        DssApp_Run();
    }

    return 0;
}

/**
 *  \brief Application main
 *
 *  This is an example of how to read dss registers.
 */
static void DssApp_Run(void)
{
    IpFma_DssCommonMRegs commonMRegsExp;
    IpFma_DssCommonSRegs commonSRegsExp;
    IpFma_DssVideoPipeRegs video1PipeRegsExp;
    IpFma_DssVideoPipeRegs video2PipeRegsExp;
    IpFma_DssVideoPipeLayerRegs videoPipeLayerRegsExp;
    IpFma_DssOverlayRegs overlayRegsExp;
    IpFma_DssVideoPortRegs videoPort1RegsExp;
    IpFma_DssVideoPortRegs videoPort2RegsExp;
    IpFma_DssWriteBackPipeRegs writeBackPipeRegsExp;
    IpFma_Status overallCompareStatus = IPFMA_OK;

    /* Common M Regs */
    IpFma_Status commonMRegsReadStatus;
    commonMRegsReadStatus = IpFma_Dss_GetCommonMRegs(&commonMRegsExp, CSL_DSS_COMM_REG_ID_0);
    DssApp_PrintRegReadStatus(commonMRegsReadStatus, "DSS COMMON M REGION");

    /* Common S Regs */
    IpFma_Status commonSRegsReadStatus;
    commonSRegsReadStatus = IpFma_Dss_GetCommonSRegs(&commonSRegsExp, CSL_DSS_COMM_REG_ID_1);
    DssApp_PrintRegReadStatus(commonSRegsReadStatus, "DSS COMMON S REGION");

    /* Video 1 pipe regs */
    IpFma_Status video1PipeRegsReadStatus;
    video1PipeRegsReadStatus = IpFma_Dss_GetVideoPipeRegs(&video1PipeRegsExp, CSL_DSS_VID_PIPE_ID_VID1);
    DssApp_PrintRegReadStatus(video1PipeRegsReadStatus, "DSS VIDEO 1 PIPE REGION");

    /* Video 2 pipe regs */
    IpFma_Status video2PipeRegsReadStatus;
    video2PipeRegsReadStatus = IpFma_Dss_GetVideoPipeRegs(&video2PipeRegsExp, CSL_DSS_VID_PIPE_ID_VID2);
    DssApp_PrintRegReadStatus(video2PipeRegsReadStatus, "DSS VIDEO 2 PIPE REGION");

    /* Video pipe layer regs */
    IpFma_Status videoPipeLayerRegsReadStatus;
    videoPipeLayerRegsReadStatus = IpFma_Dss_GetVideoPipeLayerRegs(&videoPipeLayerRegsExp, CSL_DSS_VID_PIPE_ID_VIDL1);
    DssApp_PrintRegReadStatus(videoPipeLayerRegsReadStatus, "DSS VIDEO PIPE LAYER REGION");

    /* Overlay regs */
    IpFma_Status overlayRegsReadStatus;
    overlayRegsReadStatus = IpFma_Dss_GetOverlayRegs(&overlayRegsExp, CSL_DSS_OVERLAY_ID_1);
    DssApp_PrintRegReadStatus(overlayRegsReadStatus, "DSS OVERLAY REGION");

    /* Video port 1 regs */
    IpFma_Status videoPort1RegsReadStatus;
    videoPort1RegsReadStatus = IpFma_Dss_GetVideoPortRegs(&videoPort1RegsExp, CSL_DSS_VP_ID_1);
    DssApp_PrintRegReadStatus(videoPort1RegsReadStatus, "VIDEO PORT 1 REGION");

    /* Video port 2 regs */
    IpFma_Status videoPort2RegsReadStatus;
    videoPort2RegsReadStatus = IpFma_Dss_GetVideoPortRegs(&videoPort2RegsExp, CSL_DSS_VP_ID_2);
    DssApp_PrintRegReadStatus(videoPort2RegsReadStatus, "VIDEO PORT 2 REGION");

    /* Write back pipe regs */
    IpFma_Status writeBackPipeRegsReadStatus;
    writeBackPipeRegsReadStatus = IpFma_Dss_GetWriteBackPipeRegs(&writeBackPipeRegsExp, CSL_DSS_WB_PIPE_ID_1);
    DssApp_PrintRegReadStatus(writeBackPipeRegsReadStatus, "WRITE BACK PIPE REGION");

    /*  These registers initial values are modified only to demonstrate register difference for readback example.  */
    overlayRegsExp.dssVirtualVp = 45;   /* 0010 1101  = 0x2d hex */

    uint32_t test_period = 3;
    uint32_t iteration = 0;
    while (iteration < test_period)
    {
        IpFma_DssCommonMRegs commonMRegsActual;
        IpFma_DssCommonSRegs commonSRegsActual;
        IpFma_DssVideoPipeRegs video1PipeRegsActual;
        IpFma_DssVideoPipeRegs video2PipeRegsActual;
        IpFma_DssVideoPipeLayerRegs videoPipeLayerRegsActual;
        IpFma_DssOverlayRegs overlayRegsActual;
        IpFma_DssVideoPortRegs videoPort1RegsActual;
        IpFma_DssVideoPortRegs videoPort2RegsActual;
        IpFma_DssWriteBackPipeRegs writeBackPipeRegsActual;

        UART_printf("\n");
        commonMRegsReadStatus = IpFma_Dss_GetCommonMRegs(&commonMRegsActual, CSL_DSS_COMM_REG_ID_0);
        DssApp_PrintRegReadStatus(commonMRegsReadStatus, "DSS COMMON M REGION");
        commonSRegsReadStatus = IpFma_Dss_GetCommonSRegs(&commonSRegsActual, CSL_DSS_COMM_REG_ID_1);
        DssApp_PrintRegReadStatus(commonSRegsReadStatus, "DSS COMMON S REGION");
        video1PipeRegsReadStatus = IpFma_Dss_GetVideoPipeRegs(&video1PipeRegsActual, CSL_DSS_VID_PIPE_ID_VID1);
        DssApp_PrintRegReadStatus(video1PipeRegsReadStatus, "DSS VIDEO 1 PIPE REGION");
        video2PipeRegsReadStatus = IpFma_Dss_GetVideoPipeRegs(&video2PipeRegsActual, CSL_DSS_VID_PIPE_ID_VID2);
        DssApp_PrintRegReadStatus(video2PipeRegsReadStatus, "DSS VIDEO 2 PIPE REGION");
        videoPipeLayerRegsReadStatus = IpFma_Dss_GetVideoPipeLayerRegs(&videoPipeLayerRegsActual, CSL_DSS_VID_PIPE_ID_VIDL1);
        DssApp_PrintRegReadStatus(videoPipeLayerRegsReadStatus, "DSS VIDEO PIPE LAYER REGION");
        overlayRegsReadStatus = IpFma_Dss_GetOverlayRegs(&overlayRegsActual, CSL_DSS_OVERLAY_ID_1);
        DssApp_PrintRegReadStatus(overlayRegsReadStatus, "DSS OVERLAY REGION");
        videoPort1RegsReadStatus = IpFma_Dss_GetVideoPortRegs(&videoPort1RegsActual, CSL_DSS_VP_ID_1);
        DssApp_PrintRegReadStatus(videoPort1RegsReadStatus, "VIDEO PORT 1 REGION");
        videoPort2RegsReadStatus = IpFma_Dss_GetVideoPortRegs(&videoPort2RegsActual, CSL_DSS_VP_ID_2);
        DssApp_PrintRegReadStatus(videoPort2RegsReadStatus, "VIDEO PORT 2 REGION");
        writeBackPipeRegsReadStatus = IpFma_Dss_GetWriteBackPipeRegs(&writeBackPipeRegsActual, CSL_DSS_WB_PIPE_ID_1);
        DssApp_PrintRegReadStatus(writeBackPipeRegsReadStatus, "WRITE BACK PIPE REGION");

        /* Comparison of initially read and current register values */
        IpFma_Status commonMRegsCompareStatus = IpFma_Dss_CompareCommonMRegs(&commonMRegsExp, &commonMRegsActual);
        overallCompareStatus |= commonMRegsCompareStatus;
        DssApp_PrintRegCompareStatus(commonMRegsCompareStatus, "COMMON M REGION");
        IpFma_Status commonSRegsCompareStatus = IpFma_Dss_CompareCommonSRegs(&commonSRegsExp, &commonSRegsActual);
        DssApp_PrintRegCompareStatus(commonSRegsCompareStatus, "COMMON S REGION");
        overallCompareStatus |= commonSRegsCompareStatus;
        IpFma_Status video1PipeRegsCompareStatus = IpFma_Dss_CompareVideoPipeRegs(&video1PipeRegsExp, &video1PipeRegsActual);
        DssApp_PrintRegCompareStatus(video1PipeRegsCompareStatus, "VIDEO PIPE 1 REGION");
        overallCompareStatus |= video1PipeRegsCompareStatus;
        IpFma_Status video2PipeRegsCompareStatus = IpFma_Dss_CompareVideoPipeRegs(&video2PipeRegsExp, &video2PipeRegsActual);
        DssApp_PrintRegCompareStatus(video2PipeRegsCompareStatus, "VIDEO PIPE 2 REGION");
        overallCompareStatus |= video2PipeRegsCompareStatus;
        IpFma_Status videoPipeLayerRegsCompareStatus = IpFma_Dss_CompareVideoPipeLayerRegs(&videoPipeLayerRegsExp, &videoPipeLayerRegsActual);
        DssApp_PrintRegCompareStatus(videoPipeLayerRegsCompareStatus, "VIDEO PIPE LAYER REGION");
        overallCompareStatus |= videoPipeLayerRegsCompareStatus;
        /*  These should fail  */
        UART_printf("\n\nThese should fail as the registers expected value has been modified on purpose!");
        IpFma_Status overlayRegsCompareStatus = IpFma_Dss_CompareOverlayRegs(&overlayRegsExp, &overlayRegsActual);

        DssApp_PrintRegCompareStatus(overlayRegsCompareStatus, "OVERLAY REGION");
        if (IPFMA_OK == overlayRegsCompareStatus)
        {
            /* it is expected that overlayRegsCompareStatus is IPFMA_E_MISMATCH */
            /* if it is not, update the overall */
            overallCompareStatus = IPFMA_E_MISMATCH;
        }
        IpFma_Status videoPort1RegsCompareStatus = IpFma_Dss_CompareVideoPortRegs(&videoPort1RegsExp, &videoPort1RegsActual);
        DssApp_PrintRegCompareStatus(videoPort1RegsCompareStatus, "VIDEO PORT 1 REGION");
        overallCompareStatus |= videoPort1RegsCompareStatus;
        IpFma_Status videoPort2RegsCompareStatus = IpFma_Dss_CompareVideoPortRegs(&videoPort2RegsExp, &videoPort2RegsActual);
        DssApp_PrintRegCompareStatus(videoPort2RegsCompareStatus, "VIDEO PORT 2 REGION");
        overallCompareStatus |= videoPort2RegsCompareStatus;
        IpFma_Status writeBackPipeRegsCompareStatus = IpFma_Dss_CompareWriteBackPipeRegs(&writeBackPipeRegsExp, &writeBackPipeRegsActual);
        DssApp_PrintRegCompareStatus(writeBackPipeRegsCompareStatus, "WRITE BACK PIPE REGION");
        overallCompareStatus |= writeBackPipeRegsCompareStatus;

        Board_delay(DELAY_MS);
        iteration++;
    }

    if (IPFMA_OK == overallCompareStatus)
    {
        UART_printf("\nAll tests have passed!!\n");
    }
    else
    {
        UART_printf("\nSome tests have failed!!\n");
    }
}

/**
 *  \brief Initializes the application by setting the configuration.
 *
 *  This is a initialization function that sets the board configuration.
 *  It configures the board by enabling UART module needed to print log
 *  messages to the user via UART console. This is used so the tests can
 *  print results back to us.
 *
 *  \return  BOARD_SOK in case of success or appropriate error code.
 */
static int32_t DssApp_Init(void)
{
    /* This code initialises uart on the board */
    int32_t status = BOARD_SOK;
    Board_initCfg boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    if (status == BOARD_SOK)
    {
        /* This code initialises DSS on the board */
        Dss_InitParams dssParams;
        Dss_initParamsInit(&dssParams);
        status = Dss_init(&dssParams);
    }

    return status;
}

/**
 *  \brief Simple utility function that prints whether the register read was successfull or not.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerName  [IN] Name of the register.
 *  \param regReadStatus [IN] Value that the register is expected contain.
 *
 *  \return None.
 */
static void DssApp_PrintRegReadStatus(IpFma_Status regReadStatus, const char* registerName)
{
    if (IPFMA_OK == regReadStatus)
    {
        UART_printf("\nSuccess! The %s registers have been read successfully.", registerName);
    }
    else
    {
        UART_printf("\nError! The %s registers have not been read successfully.", registerName);
    }
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
static void DssApp_PrintRegCompareStatus(IpFma_Status regCompareStatus, const char* registerName)
{
    if (IPFMA_OK == regCompareStatus)
    {
        UART_printf("\nSuccess! The %s registers have not been modified.", registerName);
    }
    else
    {
        UART_printf("\nError! The %s registers have been modified.", registerName);
    }
}
