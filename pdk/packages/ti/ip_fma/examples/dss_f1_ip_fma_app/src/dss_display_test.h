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
 *  \file dss_display_test.h
 *
 *  \brief DSS display Test Header file.
 */

#ifndef DSS_DISPLAY_TEST_H_
#define DSS_DISPLAY_TEST_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/dss/dss.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of iterations the application will run. */
#define DISP_APP_RUN_COUNT              ((uint32_t)2000U)

/* Maximum number of frames per display handle. */
#define DISP_APP_MAX_FRAMES_PER_HANDLE    (2U)

/* LCD display width in pixels. */
#define DISP_APP_LCD_WIDTH              (1920U)

/* LCD display height in pixels. */
#define DISP_APP_LCD_HEIGHT             (1080U)

/* Enable the below macro to have prints on the IO Console */
#undef CIO_CONSOLE

#ifndef CIO_CONSOLE

/* Log output mapped to standard printf for selected SoCs. */
#if defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
#define DSS_log                printf
#else
#define DSS_log                UART_printf
#endif

#else
#define DSS_log                printf
#endif

/* Print buffer character limit for prints- UART or CCS Console */
#define DISP_APP_PRINT_BUFFER_SIZE                   ((uint32_t)4000)

/**< LUT mode: CLUT (indexed colors) or Inverse Gamma correction */
typedef enum
{
    CLUT_MODE,
    GAMMA_MODE
} DispApp_Mode;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Display application test parameters.
 *  The test case execution happens based on values of this structure
 */
typedef struct
{
    uint32_t numTestPipes;
    /**< Number of pipes in test params */
    uint32_t bpp;
    /**< Number of bytes per pixel */
    uint32_t instId[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Driver instance id */
    uint32_t pipeId[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Pipe id */
    uint32_t pipeNodeId[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Pipe Node id */
    uint32_t pipeType[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Video pipe type */
    uint32_t inDataFmt[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Data format */
    uint32_t inWidth[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Input buffer resolution width in pixels */
    uint32_t inHeight[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Input buffer resolution height in lines */
    uint32_t pitch[CSL_DSS_VID_PIPE_ID_MAX][FVID2_MAX_PLANES];
    /**< Pitch of input buffer */
    uint32_t inScanFmt[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Scan format */
    uint32_t outWidth[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Output buffer resolution width in pixels */
    uint32_t outHeight[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Output buffer resolution height in lines */
    uint32_t scEnable[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Scaler enable */
    uint32_t globalAlpha[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Global Alpha value */
    uint32_t preMultiplyAlpha[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Pre-multiply Alpha value */
    uint32_t posx[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Input buffer position x. */
    uint32_t posy[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Input buffer position y. */
    uint32_t invalidPipeId[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Pipe id */
    uint32_t lutRefreshEnable;
    /**< Enable refresh LUT */
} DispApp_TestParams;

/**
 *  \brief Driver instance information.
 */
typedef struct
{
    uint32_t instId;
    /**< Instance ID */
    Dss_DispCreateParams createParams;
    /**< Create time parameters */
    Dss_DispCreateStatus createStatus;
    /**< Create status returned by driver during Fvid2_create() */
    Dss_DispParams dispParams;
    /**< DSS display parameters */
    Dss_DispPipeMflagParams mflagParams;
    /**< DSS mflag parameters */
    Fvid2_Handle drvHandle;
    /**< FVID2 display driver handle */
    Fvid2_CbParams cbParams;
    /**< Callback parameters */
    Fvid2_Frame frames[DISP_APP_MAX_FRAMES_PER_HANDLE];
    /**< FVID2 Frames that will be used for display */
    SemaphoreP_Handle syncSem;
    /**< Semaphore for ISR */
} DispApp_InstObj;

/**
 *  \brief Test application data structure.
 */
typedef struct
{
    DispApp_InstObj instObj[CSL_DSS_VID_PIPE_ID_MAX];
    /**< Display driver instance objects */
    Fvid2_Handle dctrlHandle;
    /**< DCTRL handle */
    Dss_InitParams initParams;
    /**< DSS Initialization Parameters */
    Dss_DctrlPathInfo dctrlPathInfo;
    /**< DSS Path Information */
    Dss_DctrlVpParams vpParams;
    /**< VP Params */
    Dss_DctrlVpParams syncVpParams;
    /**< VP Params for synchronised VP */
    Dss_DctrlOverlayParams overlayParams;
    /**< Overlay Params */
    Dss_DctrlOverlayLayerParams layerParams;
    /**< Layer Params */
    Dss_DctrlVpErrorStats errorStats;
    /**< Error Stats */
    Dss_DctrlAdvVpParams advVpParams;
    /**< Advance VP Params */
    Dss_DctrlAdvVpParams syncAdvVpParams;
    /**< Advance VP Params for Synchronised VP */
    Dss_DctrlGlobalDssParams globalDssParams;
    /**< LUT mode: CLUT (indexed colors) or Inverse Gamma correction */
    DispApp_Mode lutMode;
} DispApp_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Formats and logs a message.
 *
 * \param format Printf-style format string.
 * \param ... Arguments for the format string.
 */
void App_print(const char *format, ...);

/* ========================================================================== */
/*                              Global Variables                              */
/* ========================================================================== */

/**
 *  Pointer to the active display test configuration.
 *  Points to either CLUT or Gamma test parameter set used by the application.
 */
static const DispApp_TestParams *gDispAppTestParams;

/**< Configuration for CLUT mode (Color Look-Up Table) */
static const DispApp_TestParams gClutConfig = 
{
    /* Number of Pipes */
    1U,
    /* bpp */
    1U,
    /* Instance Id */
    {
        DSS_DISP_INST_VID1,
    },
    /* Pipe Id */
    {
        CSL_DSS_VID_PIPE_ID_VID1,
    },
    /* Pipe Node Id */
    {
        DSS_DCTRL_NODE_VID1,
    },
    /* Pipe Type */
    {
        CSL_DSS_VID_PIPE_TYPE_VID,
    },
    /* Data format */
    {
        FVID2_DF_BITMAP8,     
    },
    /* Input buffer width */
    {
        480U,
    },
    /* Input buffer height */
    {
        360U,
    },
    /* Pitch */
    {
        { 480U, 0U, 0U, 0U, 0U, 0U },
    },
    /* Scan format */
    {
        FVID2_SF_PROGRESSIVE,
    },
    /* Output buffer width */
    {
        480U,
    },
    /* Output buffer height */
    {
        360U,
    },
    /* Scaler enable */
    {
        UFALSE,
    },
    /* Global Alpha */
    {
        0xFFU,
    },
    /* Pre-multiply alpha */
    {
        UFALSE,
    },
    /* X Position */
    {
        0U,
    },
    /* Y position */
    {
        0U,
    },
    /* Invalid Pipe Id */
    {
        CSL_DSS_VID_PIPE_ID_VIDL1,
        CSL_DSS_VID_PIPE_ID_VID2,
        CSL_DSS_VID_PIPE_ID_VIDL2
    },
    /* Enable refresh LUT */
    UTRUE
};

/**< Configuration for Gamma mode */
static const DispApp_TestParams gGammaConfig = 
{
    /* Number of Pipes */
    1U,
    /* bpp */
    4U,
    /* Instance Id */
    {
        DSS_DISP_INST_VID1,
    },
    /* Pipe Id */
    {
        CSL_DSS_VID_PIPE_ID_VID1,
    },
    /* Pipe Node Id */
    {
        DSS_DCTRL_NODE_VID1,
    },
    /* Pipe Type */
    {
        CSL_DSS_VID_PIPE_TYPE_VID,
    },
    /* Data format */
    {
        FVID2_DF_BGRA32_8888,     
    },
    /* Input buffer width */
    {
        480U,
    },
    /* Input buffer height */
    {
        360U,
    },
    /* Pitch */
    {
        { 480U*4U, 0U, 0U, 0U, 0U, 0U },
    },
    /* Scan format */
    {
        FVID2_SF_PROGRESSIVE,
    },
    /* Output buffer width */
    {
        480U,
    },
    /* Output buffer height */
    {
        360U,
    },
    /* Scaler enable */
    {
        UFALSE,
    },
    /* Global Alpha */
    {
        0xFFU,
    },
    /* Pre-multiply alpha */
    {
        UFALSE,
    },
    /* X Position */
    {
        0U,
    },
    /* Y position */
    {
        0U,
    },
    /* Invalid Pipe Id */
    {
        CSL_DSS_VID_PIPE_ID_VIDL1,
        CSL_DSS_VID_PIPE_ID_VID2,
        CSL_DSS_VID_PIPE_ID_VIDL2
    },
    /* Enable refresh LUT */
    UTRUE
};

/* ========================================================================== */
/*      Internal Function Declarations (Needed for other static inlines)      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DSS_DISPLAY_TEST_H_ */

/* @} */
