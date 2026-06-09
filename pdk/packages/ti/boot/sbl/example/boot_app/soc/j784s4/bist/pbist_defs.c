/*
 *  Copyright (c) Texas Instruments Incorporated 2024-26
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
 *  \file pbist_defs.c
 *
 *  \brief PBIST SOC-specific structures and functions
 */

/* ========================================================================== */
/*                             Include Files */
/* ========================================================================== */

#include <stdint.h>
#include <sdl_types.h>
#include <soc.h>
#if defined(SOC_J742S2)
#include <sdlr_soc.h>
#include <src/ip/pbist/V0/sdlr_pbist.h>
#include <src/ip/pbist/V0/sdl_ip_pbist.h>
#include <pbist/soc/j784s4/sdl_pbist.h>
#endif
#include <src/ip/sdl_ip_pbist.h>
#include <src/sdl/sdl_pbist.h>
#include <ti/csl/csl_clec.h>
#include <ti/csl/csl_cbass.h>
#include <ti/csl/csl_rat.h>
#include <ti/csl/src/ip/vpac/V3/cslr_viss.h>
#include <ti/csl/src/ip/vpac/V3/cslr_vpac.h>
#include <ti/csl/cslr_cp_ace.h>
#include <ti/csl/csl_dss.h>

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

/* SCI Client */
#include <ti/drv/sciclient/sciclient.h>

#include "pbist_utils.h"
#include "pbist_defs.h"

/* ========================================================================== */
/*                           Macros & Typedefs */
/* ========================================================================== */

#define MSMC_NUM_AUX_DEVICES              2

#define A72_NUM_AUX_DEVICES               1

#define MAIN_INFRA0_NUM_AUX_DEVICES       5

#define MAIN_R5F1_NUM_AUX_DEVICES         4

#define MAIN_INFRA1_NUM_AUX_DEVICES       18

#define HC_NUM_AUX_DEVICES                10

#define CODEC_NUM_AUX_DEVICES             1

#define DMPAC_NUM_AUX_DEVICES             2

#define GPU_NUM_AUX_DEVICES               3

#define DSS_NUM_AUX_DEVICES               9

#define C7X0_NUM_AUX_DEVICES              1

#define C7X1_NUM_AUX_DEVICES              1

#define C7X2_NUM_AUX_DEVICES              1

#define C7X3_NUM_AUX_DEVICES              1

#define ANA_NUM_AUX_DEVICES               2

/* This macro defined by default to indicate only a select memory section can
 * be run on the MCU memory PBIST instances to ensure test application running
 * on MCU is not corrupted. */
#define PBIST_MCU_SELECTED_MEMORY

/* HW POST run status definitions */
#define PBIST_POST_COMPLETED_SUCCESS_STR      "SDL_PBIST_POST_COMPLETED_SUCCESS"
#define PBIST_POST_COMPLETED_FAILURE_STR      "SDL_PBIST_POST_COMPLETED_FAILURE"
#define PBIST_POST_ATTEMPTED_TIMEOUT_STR      "SDL_PBIST_POST_TIMEOUT"
#define PBIST_POST_NOT_RUN_STR                "SDL_PBIST_POST_NOT_RUN"

#define PBIST_RAT_CFG_BASE SDL_MCU_R5FSS0_RAT_CFG_BASE

/* ========================================================================== */
/*                 Internal Function Declarations */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Local function prototypes */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations */
/* ========================================================================== */

/*
    InitRestore functions : Initialize or Restore based on init flag
    init : TRUE  --> Initialize
    init : FALSE --> Restore
*/
int32_t PBIST_VPAC0AuxInitRestore(bool init);
int32_t PBIST_VPAC1AuxInitRestore(bool init);
int32_t PBIST_HCAuxInitRestore(bool init);
int32_t PBIST_MainInfra0AuxInitRestore(bool init);
int32_t PBIST_MainInfra1AuxInitRestore(bool init);
int32_t PBIST_NAVSSAuxInitRestore(bool init);
int32_t PBIST_Codec0AuxInitRestore(bool init);
int32_t PBIST_Codec1AuxInitRestore(bool init);
int32_t PBIST_GPUAuxInitRestore(bool init);
int32_t PBIST_DSSAuxInitRestore(bool init);
int32_t PBIST_A720_0AuxInitRestore(bool init);
int32_t PBIST_A720_1AuxInitRestore(bool init);
int32_t PBIST_A721_0AuxInitRestore(bool init);
int32_t PBIST_A721_1AuxInitRestore(bool init);
int32_t PBIST_MainR50AuxInitRestore(bool init);
int32_t PBIST_MainR51AuxInitRestore(bool init);
int32_t PBIST_MainR52AuxInitRestore(bool init);
/* ========================================================================== */
/*                            Global Variables */
/* ========================================================================== */
uint32_t PBIST_MainInfra0AuxDevList[MAIN_INFRA0_NUM_AUX_DEVICES] =
{
    TISCI_DEV_MMCSD1,
    TISCI_DEV_CPSW1,
    TISCI_DEV_DEBUGSS_WRAP0,
    TISCI_DEV_STM0,
    TISCI_DEV_DEBUGSUSPENDRTR0,
};

uint32_t PBIST_DMPACAuxDevList[DMPAC_NUM_AUX_DEVICES] =
{
    TISCI_DEV_DMPAC0_CTSET_0,
    TISCI_DEV_DMPAC0_INTD_0,
};

uint32_t PBIST_MainInfra1AuxDevList[MAIN_INFRA1_NUM_AUX_DEVICES] =
{
    TISCI_DEV_MCAN0,
    TISCI_DEV_MCAN1,
    TISCI_DEV_MCAN2,
    TISCI_DEV_MCAN3,
    TISCI_DEV_MCAN4,
    TISCI_DEV_MCAN5,
    TISCI_DEV_MCAN6,
    TISCI_DEV_MCAN7,
    TISCI_DEV_MCAN8,
    TISCI_DEV_MCAN9,
    TISCI_DEV_MCAN10,
    TISCI_DEV_MCAN11,
    TISCI_DEV_MCAN12,
    TISCI_DEV_MCAN13,
    TISCI_DEV_MCAN14,
    TISCI_DEV_MCAN15,
    TISCI_DEV_MCAN16,
    TISCI_DEV_MCAN17
};

uint32_t PBIST_HCAuxDevList[HC_NUM_AUX_DEVICES] =
{
    TISCI_DEV_UFS0,
    TISCI_DEV_PCIE0,
    TISCI_DEV_PCIE1,
    TISCI_DEV_PCIE2,
    TISCI_DEV_PCIE3,
    TISCI_DEV_USB0,
    TISCI_DEV_MMCSD0,
    TISCI_DEV_SA2_UL0,
    TISCI_DEV_VUSR_DUAL0,
    TISCI_DEV_CPSW_9XUSS_J7AM0,
};

uint32_t PBIST_CodecAuxDevList[CODEC_NUM_AUX_DEVICES] =
{
    TISCI_DEV_CODEC0
};

uint32_t PBIST_GPUAuxDevList[GPU_NUM_AUX_DEVICES] =
{
    TISCI_DEV_J7AEP_GPU_BXS464_WRAP0,
    TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0,
    TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPUCORE_0,
};

uint32_t PBIST_DSSAuxDevList[DSS_NUM_AUX_DEVICES] =
{
    TISCI_DEV_DSS0,
    TISCI_DEV_DSS_EDP0,
    TISCI_DEV_DSS_DSI0,
    TISCI_DEV_DSS_DSI1,
    TISCI_DEV_CSI_TX_IF0,
    TISCI_DEV_CSI_TX_IF1,
    TISCI_DEV_CSI_RX_IF0,
    TISCI_DEV_CSI_RX_IF1,
    TISCI_DEV_CSI_RX_IF2,
};

uint32_t PBIST_C7X0_AuxDevList[C7X0_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS0,
};

uint32_t PBIST_C7X1_AuxDevList[C7X1_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS1,
};

uint32_t PBIST_C7X2_AuxDevList[C7X2_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS2,
};

uint32_t PBIST_C7X3_AuxDevList[C7X3_NUM_AUX_DEVICES] =
{
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS3,
};

uint32_t PBIST_MSMCAuxDevList[MSMC_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_DIVH2_DIVH_0,
    TISCI_DEV_COMPUTE_CLUSTER0_DIVP_TFT_0,
};

uint32_t PBIST_A720AuxDevList[A72_NUM_AUX_DEVICES] =
{
    TISCI_DEV_A72SS0,
};

uint32_t PBIST_A721AuxDevList[A72_NUM_AUX_DEVICES] =
{
    TISCI_DEV_A72SS1,
};

uint32_t PBIST_ANA0AuxDevList[ANA_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS0,
    TISCI_DEV_COMPUTE_CLUSTER0_DRU4,
};

uint32_t PBIST_ANA1AuxDevList[ANA_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS1,
    TISCI_DEV_COMPUTE_CLUSTER0_DRU5,
};

uint32_t PBIST_ANA2AuxDevList[ANA_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS2,
    TISCI_DEV_COMPUTE_CLUSTER0_DRU6,
};

uint32_t PBIST_ANA3AuxDevList[ANA_NUM_AUX_DEVICES] =
{
    TISCI_DEV_COMPUTE_CLUSTER0_C71SS3,
    TISCI_DEV_COMPUTE_CLUSTER0_DRU7,
};

PBIST_TestHandle_t PBIST_TestHandleArray[PBIST_MAX_INSTANCE+1] =
    {
        /* HW POST - Used to check the status of HW POST MCU PBIST */
        {
            .testName              = "PBIST HWPOST MCU",
            .pbistInst             = SDL_PBIST_INST_MCUR5F0,
            .numPostPbistToCheck   = 1u,
        },
        /* Main R5F 0 */
        {
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST2, /* PBIST device id */
            .testName               = "Main R5F 0 PBIST",
            .pbistInst              = SDL_PBIST_INST_MAINR5F0,
            .numPostPbistToCheck    = 0u,
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = true,
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "Main R5F0 core0", /* Primary core */
            .secCoreName            = "Main R5F0 core1", /* Secondary core */
            .tisciProcId            = SCICLIENT_PROC_ID_R5FSS0_CORE0, /* Main R5F core 0 Proc Id */
            .tisciSecProcId         = SCICLIENT_PROC_ID_R5FSS0_CORE1, /* Main R5F core 1 Proc Id */
            .tisciDeviceId          = TISCI_DEV_R5FSS0_CORE0,   /* Main R5F core 0 Device Id */
            .tisciSecDeviceId       = TISCI_DEV_R5FSS0_CORE1,/* Main R5F core 1 Device id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u, /* No Aux devices */
            .auxInitRestoreFunction = PBIST_MainR50AuxInitRestore, /* Auxilliary init function */
        },
        /* Codec_0 */
        {
            .testName               = "Codec PBIST",
            .pbistInst              = SDL_PBIST_INST_CODEC,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST11, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "Encoder", /* Primary core */
            .tisciProcId            = 0x0u, /* No ProcId for Encoder */
            .tisciDeviceId          = TISCI_DEV_CODEC0, /* Encoder Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = CODEC_NUM_AUX_DEVICES, /* Aux devices */
            .auxDeviceIdsP          = &PBIST_CodecAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_Codec0AuxInitRestore, /* Auxilliary init function */
        },
        /* Main Infra 1 */
        {
            .testName               = "Main Infra1 PBIST",
            .pbistInst              = SDL_PBIST_INST_MAININFRA1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST1, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* No coreName */
            .tisciProcId            = 0x0u, /* No Proc Id needed for Main Intrastructure */
            .tisciDeviceId          = 0x0u, /* No Device Id needed for Main infrastructure */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = MAIN_INFRA1_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_MainInfra1AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_MainInfra1AuxInitRestore, /* Auxilliary init function */
        },
        /* VPAC */
        {
            .testName               = "VPAC PBIST",
            .pbistInst              = SDL_PBIST_INST_VPAC_0,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST8, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "VPAC core", /* Primary core */
            .tisciProcId            = 0x0u, /* No ProcId for VPAC */
            .tisciDeviceId          = TISCI_DEV_VPAC0, /* VPAC core Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u, /* No Aux devices */
            .auxInitRestoreFunction = PBIST_VPAC0AuxInitRestore, /* Auxilliary init function */
        },

        /* DSS EDP DSI */
        {
            .testName               = "DSS EDP PBIST",
            .pbistInst              = SDL_PBIST_INST_DSS,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST5, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,        
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "DSS", /* Primary core */
            .tisciProcId            = 0x0u,  /* No ProcId for DSS */
            .tisciDeviceId          = TISCI_DEV_DSS_DSI0, /* DSS Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = DSS_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_DSSAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_DSSAuxInitRestore, /* Auxilliary init function */
        },
        /* DMPAC */
        {
            .testName               = "DMPAC PBIST",
            .pbistInst              = SDL_PBIST_INST_DMPAC,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST7, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = true,                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "DMPAC core", /* Primary core */
            .secCoreName            = "DMPAC SDE core",     /* Secondary core */
            .tisciProcId            = 0x0u,  /* No ProcId for DMPAC */
            .tisciSecProcId         = 0x0u, /* No Sec ProcId for DMPAC */
            .tisciDeviceId          = TISCI_DEV_DMPAC0, /* DMPAC Core Device Id */
            .tisciSecDeviceId       = TISCI_DEV_DMPAC0_SDE_0, /* DMPAC_SDE Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u,/* No Aux devices */
            .auxInitRestoreFunction = NULL,
        },
        /* NAVSS */
        {
            .testName               = "NAVSS PBIST",
            .pbistInst              = SDL_PBIST_INST_NAVSS,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST4, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "NAVSS", /* Primary core */
            .tisciProcId            = 0x0u, /* No ProcId for NAVSS */
            .tisciDeviceId          = TISCI_DEV_NAVSS0, /* NAVSS Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u,  /* No Aux devices */
            .auxInitRestoreFunction = PBIST_NAVSSAuxInitRestore, /* Auxilliary init function */
        },
        /* Main Infra 0 */
        {
            .testName               = "Main Infra0 PBIST",
            .pbistInst              = SDL_PBIST_INST_MAININFRA0,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false, 
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* No coreName */
            .tisciProcId            = 0x0u, /* No Proc Id needed for Main Intrastructure */
            .tisciDeviceId          = 0x0u, /* No Device Id needed for Main infrastructure */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = MAIN_INFRA0_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_MainInfra0AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_MainInfra0AuxInitRestore, /* Auxilliary init function */
        },
        /* GPU */
        {
            .testName               = "GPU PBIST",
            .pbistInst              = SDL_PBIST_INST_GPU,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "GPU", /* Primary core */
            .tisciProcId            = 0x0u,  /* No ProcId for GPU */
            .tisciDeviceId          = TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPUCORE_0, /* GPU Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = GPU_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_GPUAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_GPUAuxInitRestore, /* Auxilliary init function */
        },
        /* HC */
        {
            .testName               = "HC PBIST",
            .pbistInst              = SDL_PBIST_INST_HC,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST3, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false, 
            .coreName               = "", /* No coreName */
            .tisciProcId            = 0x0u,                      /* No ProcId for HC */
            .tisciDeviceId          = 0x0u,                      /* No Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = HC_NUM_AUX_DEVICES,  /* Aux devices */
            .auxDeviceIdsP          = &PBIST_HCAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_HCAuxInitRestore, /* Auxilliary init function */
        },

        /* VPAC 1 */
        {
            .testName               = "VPAC_1 PBIST",
            .pbistInst              = SDL_PBIST_INST_VPAC_1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST13, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "VPAC", /* Primary core */
            .tisciProcId            = 0x0u, /* No ProcId for VPAC1 */
            .tisciDeviceId          = TISCI_DEV_VPAC1, /* VPAC1 Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u, /* No Aux devices */
            .auxInitRestoreFunction = PBIST_VPAC1AuxInitRestore,/* Auxilliary init function */
        },
        /* Main R5F 2 */
        {
            .testName                  = "Main R5F 2 PBIST",
            .pbistInst                 = SDL_PBIST_INST_MAINR5F2,
            .tisciPBISTDeviceId        = TISCI_DEV_PBIST14, /* PBIST device id */
            .procRstNeeded             = true,
            .secondaryCoreNeeded       = true,                
            .thirdCoreNeeded           = false,
            .fourthCoreNeeded          = false,
            .coreName                  = "Main R5F0 core0", /* Primary core */
            .secCoreName               = "Main R5F0 core1",     /* Secondary core */
            .tisciProcId               = SCICLIENT_PROC_ID_R5FSS2_CORE0, /* Main R5F core 0 Proc Id */
            .tisciSecProcId            = SCICLIENT_PROC_ID_R5FSS2_CORE1, /* Main R5F core 1 Proc Id */
            .tisciDeviceId             = TISCI_DEV_R5FSS2_CORE0,   /* Main R5F core 0 Device Id */
            .tisciSecDeviceId          = TISCI_DEV_R5FSS2_CORE1,/* Main R5F core 1 Device id */
            .coreCustPwrSeqNeeded      = false,
            .numAuxDevices             = 0u, /* No Aux devices */
            .auxInitRestoreFunction    = PBIST_MainR52AuxInitRestore, /* Auxilliary init function */
	    },
        /* Codec_1 */
        {
            .testName                 = "Codec 1 PBIST",
            .pbistInst                = SDL_PBIST_INST_CODEC_1,
            .numPostPbistToCheck      = 0u,
            .tisciPBISTDeviceId       = TISCI_DEV_PBIST15, /* PBIST device id */
            .procRstNeeded            = false,
            .secondaryCoreNeeded      = false,                
            .thirdCoreNeeded          = false,
            .fourthCoreNeeded         = false,
            .coreName                 = "CODEC", /* Primary core */
            .tisciProcId              = 0x0u,    /* No ProcId for Encoder */
            .tisciDeviceId            = TISCI_DEV_CODEC1, /* Encoder Device Id */
            .coreCustPwrSeqNeeded     = false,
            .numAuxDevices            = CODEC_NUM_AUX_DEVICES, /* Number of device id's */
            .auxDeviceIdsP            = &PBIST_CodecAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction   = PBIST_Codec1AuxInitRestore, /* Auxilliary init function */
        },
        /* A72_0 0 */
        {
            .testName                 = "A72_0_0 PBIST",
            .pbistInst                = PBIST_INSTANCE_A72_0_0,
            .numPostPbistToCheck      = 0u,
            .tisciPBISTDeviceId       = TISCI_DEV_COMPUTE_CLUSTER0_ARM0_DFT_EMBED_PBIST_0, /* Device Id for A72 PBIST */
            .procRstNeeded            = true,
            .secondaryCoreNeeded      = true,                
            .thirdCoreNeeded          = true,
            .fourthCoreNeeded         = true,
            .coreName                 = "A72 core 0", /* Primary core */
            .secCoreName              = "A72 core 1", /* Secondary core */
            .thCoreName               = "A72 core 2",
            .foCoreName               = "A72 core 3",
            .tisciProcId              = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE0_0,  /* A72 core 0 Proc Id */
            .tisciSecProcId           = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE1_0,  /* A72 core 1 Proc Id */
            .tisciThProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE2_0,  /* A72 core 2 Proc Id */
            .tisciFoProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE3_0,  /* A72 core 3 Proc Id */
            .tisciDeviceId            = TISCI_DEV_A72SS0_CORE0,  /* A72 core 0 Device Id */
            .tisciSecDeviceId         = TISCI_DEV_A72SS0_CORE1,  /* A72 core 1 Device Id */
            .tisciThDeviceId          = TISCI_DEV_A72SS0_CORE2,  /* A72 core 2 Device Id */
            .tisciFoDeviceId          = TISCI_DEV_A72SS0_CORE3,  /* A72 core 3 Device Id */
            .coreCustPwrSeqNeeded     = true,                    /* A72 needs custom powerdown sequence steps */
            .numAuxDevices            = A72_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP            = &PBIST_A720AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction   = PBIST_A720_0AuxInitRestore, /* Auxilliary init function */
        },
        /* A72_0_1 */
        {
            .testName                 = "A72_0_1 PBIST",
            .pbistInst                = SDL_PBIST_INST_A72_0_1,
            .numPostPbistToCheck      = 0u,
            .tisciPBISTDeviceId       = TISCI_DEV_COMPUTE_CLUSTER0_ARM0_DFT_EMBED_PBIST_1, /* Device Id for A72 PBIST */
            .procRstNeeded            = true,
            .secondaryCoreNeeded      = true,               
            .thirdCoreNeeded          = true,
            .fourthCoreNeeded         = true,
            .coreName                 = "A72 core 0", /* Primary core */
            .secCoreName              = "A72 core 1", /* Secondary core */
            .thCoreName               = "A72 core 2",
            .foCoreName               = "A72 core 3",
            .tisciProcId              = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE0_0,  /* A72 core 0 Proc Id */
            .tisciSecProcId           = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE1_0,  /* A72 core 1 Proc Id */
            .tisciThProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE2_0,  /* A72 core 2 Proc Id */
            .tisciFoProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS0_CORE3_0,  /* A72 core 3 Proc Id */
            .tisciDeviceId            = TISCI_DEV_A72SS0_CORE0,  /* A72 core 0 Device Id */
            .tisciSecDeviceId         = TISCI_DEV_A72SS0_CORE1,  /* A72 core 1 Device Id */
            .tisciThDeviceId          = TISCI_DEV_A72SS0_CORE2,  /* A72 core 2 Device Id */
            .tisciFoDeviceId          = TISCI_DEV_A72SS0_CORE3,  /* A72 core 3 Device Id */
            .coreCustPwrSeqNeeded     = true,                    /* A72 needs custom powerdown sequence steps */
            .numAuxDevices            = A72_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP            = &PBIST_A720AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction   = PBIST_A720_1AuxInitRestore, /* Auxilliary init function */
        },
        /* A72_1_0 */
        {
            .testName                 = "A72_1_0 PBIST",
            .pbistInst                = SDL_PBIST_INST_A72_1_0,
            .numPostPbistToCheck      = 0u,
            .tisciPBISTDeviceId        = TISCI_DEV_COMPUTE_CLUSTER0_ARM1_DFT_EMBED_PBIST_0, /* Device Id for A72 PBIST */
            .procRstNeeded            = true,
            .secondaryCoreNeeded      = true,                
            .thirdCoreNeeded          = true,
            .fourthCoreNeeded         = true,
            .coreName                 = "A721 core 0", /* Primary core */
            .secCoreName              = "A721 core 1", /* Secondary core */
            .thCoreName               = "A721 core 2",
            .foCoreName               = "A721 core 3",
            .tisciProcId              = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE0_0,  /* A72 core 0 Proc Id */
            .tisciSecProcId           = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE1_0,  /* A72 core 1 Proc Id */
            .tisciThProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE2_0,  /* A72 core 2 Proc Id */
            .tisciFoProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE3_0,  /* A72 core 3 Proc Id */
            .tisciDeviceId            = TISCI_DEV_A72SS1_CORE0,  /* A72 core 0 Device Id */
            .tisciSecDeviceId         = TISCI_DEV_A72SS1_CORE1,  /* A72 core 1 Device Id */
            .tisciThDeviceId          = TISCI_DEV_A72SS1_CORE2,  /* A72 core 2 Device Id */
            .tisciFoDeviceId          = TISCI_DEV_A72SS1_CORE3,  /* A72 core 3 Device Id */
            .coreCustPwrSeqNeeded     = true,                    /* A72 needs custom powerdown sequence steps */
            .numAuxDevices            = A72_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP            = &PBIST_A721AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction   = PBIST_A721_0AuxInitRestore, /* Auxilliary init function */
        },
        /* A72_1_1 */
        {
            .testName               = "A72_1_1 PBIST",
            .pbistInst              = SDL_PBIST_INST_A72_1_1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_ARM1_DFT_EMBED_PBIST_1, /* Device Id for A72 PBIST */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = true,                
            .thirdCoreNeeded        = true,
            .fourthCoreNeeded       = true,
            .coreName               = "A721 core 0", /* Primary core */
            .secCoreName            = "A721 core 1", /* Secondary core */
            .thCoreName             = "A721 core 2",
            .foCoreName             = "A721 core 3",
            .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE0_0,  /* A72 core 0 Proc Id */
            .tisciSecProcId         = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE1_0,  /* A72 core 1 Proc Id */
            .tisciThProcId          = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE2_0,  /* A72 core 2 Proc Id */
            .tisciFoProcId          = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_A72SS1_CORE3_0,  /* A72 core 3 Proc Id */
            .tisciDeviceId          = TISCI_DEV_A72SS1_CORE0,  /* A72 core 0 Device Id */
            .tisciSecDeviceId       = TISCI_DEV_A72SS1_CORE1,  /* A72 core 1 Device Id */
            .tisciThDeviceId        = TISCI_DEV_A72SS1_CORE2,  /* A72 core 2 Device Id */
            .tisciFoDeviceId        = TISCI_DEV_A72SS1_CORE3,  /* A72 core 3 Device Id */
            .coreCustPwrSeqNeeded   = true,                    /* A72 needs custom powerdown sequence steps */
            .numAuxDevices          = A72_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_A721AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = PBIST_A721_1AuxInitRestore, /* Auxilliary init function */
        },
        /* C7X_0 */
        {
            .testName               = "C7X_0 PBIST",
            .pbistInst              = SDL_PBIST_INST_C7X_0,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_C71X_0_PBIST_VD, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,              
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "C7x 0", /* Primary core */
            .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_C71SS0_CORE0_0,  /* C6x core0 Proc Id */
            .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_CORE0,          /* C6x Core 0 Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = C7X0_NUM_AUX_DEVICES,/* Aux devices */
            .auxDeviceIdsP          = &PBIST_C7X0_AuxDevList[0],
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* C7X_1 */
        {
            .testName               = "C7X_1 PBIST",
            .pbistInst              = SDL_PBIST_INST_C7X_1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_C71X_1_PBIST_VD, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,              
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "C7x 1", /* Primary core */
            .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_C71SS1_CORE0_0,  /* C7x core0 Proc Id */
            .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS1_CORE0,          /* C7x Core 0 Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = C7X1_NUM_AUX_DEVICES,/* Aux devices */
            .auxDeviceIdsP          = &PBIST_C7X1_AuxDevList[0],
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* C7X_2 */
        {
            .testName               = "C7X_2 PBIST",
            .pbistInst              = SDL_PBIST_INST_C7X_2,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AC71_6_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,              
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "C7x 2", /* Primary core */
            .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_C71SS2_CORE0_0,  /* C6x core0 Proc Id */
            .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS2_CORE0,          /* C6x Core 0 Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = C7X2_NUM_AUX_DEVICES,/* Aux devices */
            .auxDeviceIdsP          = &PBIST_C7X2_AuxDevList[0],
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* C7X_3*/
        {
            .testName               = "C7X_3 PBIST",
            .pbistInst              = SDL_PBIST_INST_C7X_3,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AC71_7_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = false,              
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "C7x 3", /* Primary core */
            .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER_J7AHP0_C71SS3_CORE0_0,  /* C6x core0 Proc Id */
            .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS3_CORE0,          /* C6x Core 0 Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = C7X3_NUM_AUX_DEVICES,/* Aux devices */
            .auxDeviceIdsP          = &PBIST_C7X3_AuxDevList[0],
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* ANA_0 */
        {
            .testName               = "ANA_0 PBIST",
            .pbistInst              = SDL_PBIST_INST_ANA_0,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AW4_MSMC_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "ANA_0", /* Primary core */
            .tisciProcId            = 0x0u,  /* No ProcId for ANA */
            .tisciDeviceId          = 0x0u,  /* ANA Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = ANA_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_ANA0AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* ANA_1 */
        {
            .testName               = "ANA_1 PBIST",
            .pbistInst              = SDL_PBIST_INST_ANA_1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AW5_MSMC_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "ANA_1", /* Primary core */
            .tisciProcId            = 0x0u, /* No ProcId for ANA */
            .tisciDeviceId          = 0x0u, /* ANA Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = ANA_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_ANA1AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* ANA_2 */
        {
            .testName               = "ANA_2 PBIST",
            .pbistInst              = SDL_PBIST_INST_ANA_2,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AW6_MSMC_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "ANA_2", /* Primary core */
            .tisciProcId            = 0x0u,  /* No ProcId for ANA */
            .tisciDeviceId          = 0x0u,  /* ANA Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = ANA_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_ANA2AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* ANA_3 */
        {
            .testName               = "ANA_3 PBIST",
            .pbistInst              = SDL_PBIST_INST_ANA_3,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_AW7_MSMC_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "ANA_3", /* Primary core */
            .tisciProcId            = 0x0u,  /* No ProcId for ANA */
            .tisciDeviceId          = 0x0u, /* ANA Device Id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = ANA_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_ANA3AuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL, /* Auxilliary init function not needed */
        },
        /* MSMC */
        {
            .testName               = "MSMC PBIST",
            .pbistInst              = SDL_PBIST_INST_MSMC,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_COMPUTE_CLUSTER0_MSMC_DFT_EMBED_PBIST_0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* Primary core */
            .tisciProcId            = 0x0u,
            .tisciDeviceId          = 0x0u,
            .coreCustPwrSeqNeeded   = false, 
            .numAuxDevices          = MSMC_NUM_AUX_DEVICES, /* Number of Aux devices */
            .auxDeviceIdsP          = &PBIST_MSMCAuxDevList[0], /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL,
        },
        /* Main R5F 1 */
        {
            .testName               = "Main R5F 1 PBIST",
            .pbistInst              = SDL_PBIST_INST_MAINR5F1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_PBIST10, /* PBIST device id */
            .procRstNeeded          = true,
            .secondaryCoreNeeded    = true,                      
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "Main R5F1 core0", /* Primary core */
            .secCoreName            = "Main R5F1 core1",          /* Secondary core */
            .tisciProcId            = SCICLIENT_PROC_ID_R5FSS1_CORE0,    /* Main R5F core 0 Proc Id */
            .tisciSecProcId         = SCICLIENT_PROC_ID_R5FSS1_CORE1, /* Main R5F core 1 Proc Id */
            .tisciDeviceId          = TISCI_DEV_R5FSS1_CORE0,          /* Main R5F core 0 Device id */
            .tisciSecDeviceId       = TISCI_DEV_R5FSS1_CORE1, /* Main R5F core 1 Device id */
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0u,  /* No Aux devices */
            .auxInitRestoreFunction = PBIST_MainR51AuxInitRestore, /* Auxilliary init function */
        },
        /* MCU PSROM*/
        {
            .testName               = "PBIST MCU PSROM",
            .pbistInst              = SDL_PBIST_INST_MCUR5F0,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_MCU_PBIST0, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* Primary core */
            .tisciProcId            = 0x0u,
            .tisciDeviceId          = 0x0u,
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0x0, /* No Aux devices */
            .auxDeviceIdsP          = 0x0,
            .auxInitRestoreFunction = NULL,
        },
        /* MCU_1*/
        {
            .testName               = "PBIST MCU_1",
            .pbistInst              = SDL_PBIST_INST_MCUR5F1,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_MCU_PBIST1, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* Primary core */
            .tisciProcId            = 0x0u,
            .tisciDeviceId          = 0x0u,
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0x0, /* No Aux devices */
            .auxDeviceIdsP          = 0x0, /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL,
        },
        /* MCU PULSAR*/
        {
            .testName               = "PBIST MCU PULSAR",
            .pbistInst              = SDL_PBIST_INST_MCU_PULSAR,
            .numPostPbistToCheck    = 0u,
            .tisciPBISTDeviceId     = TISCI_DEV_MCU_PBIST2, /* PBIST device id */
            .procRstNeeded          = false,
            .secondaryCoreNeeded    = false,                
            .thirdCoreNeeded        = false,
            .fourthCoreNeeded       = false,
            .coreName               = "", /* Primary core */
            .tisciProcId            = 0x0u,
            .tisciDeviceId          = 0x0u,
            .coreCustPwrSeqNeeded   = false,
            .numAuxDevices          = 0x0, /* Number of Aux devices */
            .auxDeviceIdsP          = 0x0, /* Array of Aux device ids */
            .auxInitRestoreFunction = NULL,
        },



};

/* ========================================================================== */
/*                            External Variables */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions */
/* ========================================================================== */

/*
    InitRestore functions : Initialize or Restore based on init flag
    init : TRUE  --> Initialize
    init : FALSE --> Restore
*/
int32_t PBIST_A720_0AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    bool result;

    CSL_RatTranslationCfgInfo translationCfg;
    uint32_t *localP = (uint32_t *)PBIST_REGION2_LOCAL_BASE;

    /* Add RAT configuration to access address > 32bit address range */
    translationCfg.translatedAddress = CSL_COMPUTE_CLUSTER0_CC_BASE;
    translationCfg.sizeInBytes = PBIST_REG_REGION_SIZE;
    translationCfg.baseAddress = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Set up RAT translation */
    result = CSL_ratConfigRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                            PBIST_RAT_REGION2_INDEX, &translationCfg);
    if (result == false) {
        UART_printf("   CSL_ratConfigRegionTranslation...FAILED \n");
        testResult = -1;
    }

    if (testResult == 0)
    {
        if (init)
        {
            *((uint32_t *)(((uint32_t)localP) + 0x100)) = 0x1;
        }
        else
        {
            *((uint32_t *)(((uint32_t)localP) + 0x100)) = 0x0;
        }
    }

    if (testResult == 0)
    {
        /* Disable RAT translation */
        result = CSL_ratDisableRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                                 PBIST_RAT_REGION2_INDEX);
        if (result == false) {
            UART_printf("   CSL_ratDisableRegionTranslation...FAILED \n");
            testResult = -1;
        }
    }

    return testResult;
}

int32_t PBIST_A720_1AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    bool result;

    CSL_RatTranslationCfgInfo translationCfg;
    uint32_t *localP = (uint32_t *)PBIST_REGION2_LOCAL_BASE;

    /* Add RAT configuration to access address > 32bit address range */
    translationCfg.translatedAddress = CSL_COMPUTE_CLUSTER0_CC_BASE;
    translationCfg.sizeInBytes = PBIST_REG_REGION_SIZE;
    translationCfg.baseAddress = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Set up RAT translation */
    result = CSL_ratConfigRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                            PBIST_RAT_REGION2_INDEX, &translationCfg);
    if (result == false) {
        UART_printf("   CSL_ratConfigRegionTranslation...FAILED \n");
        testResult = -1;
    }

    if (testResult == 0)
    {
        if (init)
        {
            *((uint32_t *)(((uint32_t)localP) + 0x100)) = 0x2;
        }
        else
        {
            *((uint32_t *)(((uint32_t)localP) + 0x100)) = 0x0;
        }
    }

    if (testResult == 0)
    {
        /* Disable RAT translation */
        result = CSL_ratDisableRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                                 PBIST_RAT_REGION2_INDEX);
        if (result == false) {
            UART_printf("   CSL_ratDisableRegionTranslation...FAILED \n");
            testResult = -1;
        }
    }

    return testResult;
}

int32_t PBIST_A721_0AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    bool result;

    CSL_RatTranslationCfgInfo translationCfg;
    uint32_t *localP = (uint32_t *)PBIST_REGION2_LOCAL_BASE;

    /* Add RAT configuration to access address > 32bit address range */
    translationCfg.translatedAddress = CSL_COMPUTE_CLUSTER0_CC_BASE;
    translationCfg.sizeInBytes = PBIST_REG_REGION_SIZE;
    translationCfg.baseAddress = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Set up RAT translation */
    result = CSL_ratConfigRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                            PBIST_RAT_REGION2_INDEX, &translationCfg);
    if (result == false) {
        UART_printf("   CSL_ratConfigRegionTranslation...FAILED \n");
        testResult = -1;
    }

    if (testResult == 0)
    {
        if (init)
        {
            *((uint32_t *)(((uint32_t)localP) + 0x104)) = 0x1;
        }
        else
        {
            *((uint32_t *)(((uint32_t)localP) + 0x104)) = 0x0;
        }
    }

    if (testResult == 0)
    {
        /* Disable RAT translation */
        result = CSL_ratDisableRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                                 PBIST_RAT_REGION2_INDEX);
        if (result == false) {
            UART_printf("   CSL_ratDisableRegionTranslation...FAILED \n");
            testResult = -1;
        }
    }

    return testResult;
}

int32_t PBIST_A721_1AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    bool result;

    CSL_RatTranslationCfgInfo translationCfg;
    uint32_t *localP = (uint32_t *)PBIST_REGION2_LOCAL_BASE;

    /* Add RAT configuration to access address > 32bit address range */
    translationCfg.translatedAddress = CSL_COMPUTE_CLUSTER0_CC_BASE;
    translationCfg.sizeInBytes = PBIST_REG_REGION_SIZE;
    translationCfg.baseAddress = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Set up RAT translation */
    result = CSL_ratConfigRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                            PBIST_RAT_REGION2_INDEX, &translationCfg);
    if (result == false) {
        UART_printf("   CSL_ratConfigRegionTranslation...FAILED \n");
        testResult = -1;
    }

    if (testResult == 0)
    {
        if (init)
        {
            *((uint32_t *)(((uint32_t)localP) + 0x104)) = 0x2;
        }
        else
        {
            *((uint32_t *)(((uint32_t)localP) + 0x104)) = 0x0;
        }
    }

    if (testResult == 0)
    {
        /* Disable RAT translation */
        result = CSL_ratDisableRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                                 PBIST_RAT_REGION2_INDEX);
        if (result == false) {
            UART_printf("   CSL_ratDisableRegionTranslation...FAILED \n");
            testResult = -1;
        }
    }

    return testResult;
}

/* define the unlock and lock values */
#define KICK0_UNLOCK_VAL 0x68EF3490
#define KICK1_UNLOCK_VAL 0xD172BC5A
#define KICK_LOCK_VAL    0x00000000

int32_t PBIST_VPAC0AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    CSL_viss_topRegs *vissTopRegsP;
    uint32_t mask = 0;
    mask |= 0x1 << 12; /* MAIN vpac 0 */

    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK1, KICK1_UNLOCK_VAL);

    /* Configure MAIN_CLKGATE_CTRL1 register using SDL register access APIs */
    uint32_t clkgateCtrl1RegAddr = SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_MAIN_CLKGATE_CTRL1;
    uint32_t clkgateCtrl1Tmp = SDL_REG32_RD(clkgateCtrl1RegAddr);

    vissTopRegsP = (CSL_viss_topRegs *)CSL_VPAC0_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE;
    if (init)
    {
        /* Initialize: Set the specified bits in MAIN_CLKGATE_CTRL1 */

        /* Notice: If the mask was set before initialization, this sequence will clear the mask on restore. */
        clkgateCtrl1Tmp |= mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);

        SDL_REG32_WR(CSL_VPAC0_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE + CSL_VISS_TOP_TEST_CNTL, 0x1);
        SDL_REG32_WR(CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE + CSL_VPAC_CNTL_VPAC_TEST_CTRL, 0x00000003);
        vissTopRegsP->VISS_CNTL = CSL_VISS_TOP_VISS_CNTL_NSF4V_EN_MASK
                                  | CSL_VISS_TOP_VISS_CNTL_GLBCE_EN_MASK;
    }
    else
    {
        /* Restore: Clear the bits that were set during init in MAIN_CLKGATE_CTRL1 */
        clkgateCtrl1Tmp &= ~mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);

        SDL_REG32_WR(CSL_VPAC0_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE + CSL_VISS_TOP_TEST_CNTL, 0x0);
        SDL_REG32_WR(CSL_VPAC0_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE + CSL_VPAC_CNTL_VPAC_TEST_CTRL, 0x00000000);
        vissTopRegsP->VISS_CNTL &= (~(CSL_VISS_TOP_VISS_CNTL_NSF4V_EN_MASK
                                  | CSL_VISS_TOP_VISS_CNTL_GLBCE_EN_MASK));
    }

    return testResult;
}

int32_t PBIST_VPAC1AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    CSL_viss_topRegs *vissTopRegsP;
    uint32_t mask = 0;
    mask |= 0x1 << 13; /* MAIN vpac 1 */

    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK1, KICK1_UNLOCK_VAL);

    /* Configure MAIN_CLKGATE_CTRL1 register using SDL register access APIs */
    uint32_t clkgateCtrl1RegAddr = SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_MAIN_CLKGATE_CTRL1;
    uint32_t clkgateCtrl1Tmp = SDL_REG32_RD(clkgateCtrl1RegAddr);

    vissTopRegsP = (CSL_viss_topRegs *)CSL_VPAC1_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE;
    if (init)
    {
        /* Initialize: Set the specified bits in MAIN_CLKGATE_CTRL1 */

        /* Notice: If the mask was set before initialization, this sequence will clear the mask on restore. */
        clkgateCtrl1Tmp |= mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);

        SDL_REG32_WR(CSL_VPAC1_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE + CSL_VISS_TOP_TEST_CNTL, 0x1);
        SDL_REG32_WR(CSL_VPAC1_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE + CSL_VPAC_CNTL_VPAC_TEST_CTRL, 0x00000003);
        vissTopRegsP->VISS_CNTL = CSL_VISS_TOP_VISS_CNTL_NSF4V_EN_MASK
                                  | CSL_VISS_TOP_VISS_CNTL_GLBCE_EN_MASK;
    }
    else
    {
        /* Restore: Clear the bits that were set during init in MAIN_CLKGATE_CTRL1 */
        clkgateCtrl1Tmp &= ~mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);

        SDL_REG32_WR(CSL_VPAC1_PAR_VPAC_VISS0_S_VBUSP_MMR_CFG_VISS_TOP_BASE + CSL_VISS_TOP_TEST_CNTL, 0x0);
        SDL_REG32_WR(CSL_VPAC1_VPAC_REGS_VPAC_REGS_CFG_IP_MMRS_BASE + CSL_VPAC_CNTL_VPAC_TEST_CTRL, 0x00000000);
        vissTopRegsP->VISS_CNTL &= (~(CSL_VISS_TOP_VISS_CNTL_NSF4V_EN_MASK
                                  | CSL_VISS_TOP_VISS_CNTL_GLBCE_EN_MASK));
    }

    return testResult;
}

void ecc_ufs_config(bool init)
{
    uint32_t retval;

    if (init)
    {

        /* Initialize: Set bit 0 in register at offset 0x34 */

        /* Notice: If these below registers was set before initialization, this sequence will clear these registers on restore. */

        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) | 0x00000001;
        *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) = retval;
        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) & 0x00000001;
        while (retval == 0)
        {
            retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) & 0x00000001;
        }

        /* Set bits [7:3] in register at offset 0xC8 */
        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0xC8)) | 0x000000F8;
        *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0xC8)) = retval;
    }
    else
    {
        /* De-initialize: Clear bits [7:3] from register at offset 0xC8 */
        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0xC8)) & (~0x000000F8);
        *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0xC8)) = retval;

        /* Clear bit 0 from register at offset 0x34 */
        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) & (~0x00000001);
        *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) = retval;

        /* Wait for bit 0 to be confirmed cleared */
        retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) & 0x00000001;
        while (retval != 0)
        {
            retval = *((uint32_t *)(SDL_UFS0_P2A_WRAP_CFG_VBP_UFSHCI_BASE + 0x34)) & 0x00000001;
        }
    }
}

int32_t PBIST_HCAuxInitRestore(bool init)
{
    int32_t testResult = 0;

    CSL_Cp_aceRegs *SA2ULRegsP;

    SA2ULRegsP = (CSL_Cp_aceRegs *)CSL_SA2_UL0_BASE;
    uint32_t mask = 0;
    mask |= 0x1 << 21; /* vusr */
    mask |= 0x1 << 16; /* ufs */
    mask |= 0x1 << 11; /* pcie3 */
    mask |= 0x1 << 10; /* pcie2 */
    mask |= 0x1 <<  9; /* pcie1 */
    mask |= 0x1 <<  8; /* pcie0 */
    mask |= 0x1 <<  4; /* usb0 */
    mask |= 0x1 <<  0; /* emmc0 (8 bit) */

    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1, KICK1_UNLOCK_VAL);

    /* Configure PBIST_EN register using SDL register access APIs */
    uint32_t pbistEnRegAddr = SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_PBIST_EN;
    uint32_t pbistEnTmp = SDL_REG32_RD(pbistEnRegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in PBIST_EN */
        pbistEnTmp |= mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);

        ecc_ufs_config(TRUE);

        /* Notice: If these ENGINE_ENABLE masks was set before initialization, this sequence will clear these ENGINE_ENABLE masks on restore. */
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_ENCSS_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_AUTHSS_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_TRNG_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_PKA_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CTX_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_IN_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_OUT_EN_MASK;
        SA2ULRegsP->UPDATES.ENGINE_ENABLE |= CSL_CP_ACE_UPDATES_ENGINE_ENABLE_PKA_EN_MASK;
    }
    else
    {
        /* Restore: Clear the bits that were set during init in PBIST_EN */
        pbistEnTmp &= ~mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);

        ecc_ufs_config(FALSE);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_ENCSS_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_AUTHSS_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_TRNG_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_PKA_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CTX_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_IN_EN_MASK);
        SA2ULRegsP->UPDATES.ENGINE_ENABLE &= (~CSL_CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_OUT_EN_MASK);
    }

    return testResult;
}

int32_t PBIST_MainInfra0AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    uint32_t mask = 0;
    mask |= 0x1 <<  1; // emmc1 (4 bit)

    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1, KICK1_UNLOCK_VAL);

    /* Configure PBIST_EN register using SDL register access APIs */
    uint32_t pbistEnRegAddr = SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_PBIST_EN;
    uint32_t pbistEnTmp = SDL_REG32_RD(pbistEnRegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits */
        pbistEnTmp |= mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }
    else
    {
        /* Restore: Clear the bits that were set during init */
        pbistEnTmp &= ~mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }
    return testResult;
}

int32_t PBIST_NAVSSAuxInitRestore(bool init)
{
    int32_t testResult = 0;
    uint32_t mask = 0;
    mask |= 0x1 <<  0; /* MAIN navss modss */
    mask |= 0x1 <<  1; /* MAIN navss udma */
    mask |= 0x1 <<  3; /* MAIN navss virtss */

    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK1, KICK1_UNLOCK_VAL);

    /* Configure MAIN_CLKGATE_CTRL1 register using SDL register access APIs */
    uint32_t clkgateCtrl1RegAddr = SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_MAIN_CLKGATE_CTRL1;
    uint32_t clkgateCtrl1Tmp = SDL_REG32_RD(clkgateCtrl1RegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in MAIN_CLKGATE_CTRL1 */

        /* Notice: If the mask was set before initialization, this sequence will clear the mask on restore. */
        clkgateCtrl1Tmp |= mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);
        clkgateCtrl1Tmp |= mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);
    }
    else
    {
        /* Restore: Clear the bits that were set during init in MAIN_CLKGATE_CTRL1 */
        clkgateCtrl1Tmp &= ~mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);
    }

    return testResult;
}

int32_t PBIST_MainInfra1AuxInitRestore(bool init)
{
    int32_t testResult = 0;

    uint32_t mask = 0;
    mask |= 0x1 << 20; /* MAIN PDMA MCAN */
    mask |= 0x1 << 18; /* MAIN PDMA SPI G1 */
    mask |= 0x1 << 17; /* MAIN PDMA SPI G0 */

    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK1, KICK1_UNLOCK_VAL);

    /* Configure MAIN_CLKGATE_CTRL1 register using SDL register access APIs */
    uint32_t clkgateCtrl1RegAddr = SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_MAIN_CLKGATE_CTRL1;
    uint32_t clkgateCtrl1Tmp = SDL_REG32_RD(clkgateCtrl1RegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in MAIN_CLKGATE_CTRL1 */

        /* Notice: If the mask was set before initialization, this sequence will clear the mask on restore. */
        clkgateCtrl1Tmp |= mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);
    }
    else
    {
        /* Restore: Clear the bits that were set during init in MAIN_CLKGATE_CTRL1 */
        clkgateCtrl1Tmp &= ~mask;
        SDL_REG32_WR(clkgateCtrl1RegAddr, clkgateCtrl1Tmp);
    }

    return testResult;
}

int32_t PBIST_Codec0AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    uint32_t mask = 0;
    mask |= 0x1 << 18; /* encoder 0 */

    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1, KICK1_UNLOCK_VAL);

    /* Configure PBIST_EN register using SDL register access APIs */
    uint32_t pbistEnRegAddr = SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_PBIST_EN;
    uint32_t pbistEnTmp = SDL_REG32_RD(pbistEnRegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in PBIST_EN */
        pbistEnTmp |= mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }
    else
    {
        /* Restore: Clear the bits that were set during init in PBIST_EN */
        pbistEnTmp &= ~mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }

    return testResult;
}

int32_t PBIST_Codec1AuxInitRestore(bool init)
{
    int32_t testResult = 0;
    uint32_t mask = 0;
    mask |= 0x1 << 19; /* encoder 1 */

    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1, KICK1_UNLOCK_VAL);

    /* Configure PBIST_EN register using SDL register access APIs */
    uint32_t pbistEnRegAddr = SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_PBIST_EN;
    uint32_t pbistEnTmp = SDL_REG32_RD(pbistEnRegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in PBIST_EN */
        pbistEnTmp |= mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }
    else
    {
        /* Restore: Clear the bits that were set during init in PBIST_EN */
        pbistEnTmp &= ~mask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);
    }

    return testResult;
}


int32_t PBIST_GPUAuxInitRestore(bool init)
{
    int32_t testResult = 0;
    bool result;
    int32_t i = 0;
    uint32_t timeoutCnt;

    CSL_RatTranslationCfgInfo translationCfg;

    uint32_t localP = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Add RAT configuration to access address > 32bit address range */
    translationCfg.translatedAddress = SDL_AEP_GPU_BXS464_WRAP0_CORE_MMRS_BASE;
    translationCfg.sizeInBytes = PBIST_REG_REGION2_SIZE;
    translationCfg.baseAddress = (uint32_t)PBIST_REGION2_LOCAL_BASE;

    /* Set up RAT translation */
    result = CSL_ratConfigRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                            PBIST_RAT_REGION2_INDEX, &translationCfg);
    if (result == false) {
        UART_printf("   CSL_ratConfigRegionTranslation...FAILED \n");
        testResult = -1;
    }

    if (testResult == 0)
    {
        if (init)
        {
            *((uint32_t *)(localP + 0xA100)) = 0x0;
            timeoutCnt = 100000U;
            while (timeoutCnt-- > 0)
            {
                if (*((uint32_t *)(localP + 0xA100)) ==  (uint32_t)0x0)
                {
                    break;
                }
            }
            if (timeoutCnt == 0)
            {
                UART_printf("timeout in GPUAuxInitRestore [1]\n");
            }

            /* Force all GPU clocks on */
            *((uint64_t *)(localP)) = 0x0015550015115555;

            /* Force the FW Processor clocks to ON to enable testing of mars memories */
            *((uint32_t *)(localP + 0x3090)) = 0x00000001;
            i = 1000;
            while(i--);
            *((uint32_t *)(localP + 0x3480)) = 0x00000001;
            timeoutCnt = 100000U;
            while (timeoutCnt-- > 0)
            {
                if((*(uint32_t *)(localP + 0x3488) & 0x400) == (uint32_t)0x400)
                {
                    break;
                }
            }
            if (timeoutCnt == 0)
            {
                UART_printf("timeout in GPUAuxInitRestore [2]\n");
            }
            *((uint32_t *)(localP + 0x3480)) = 0x80000001;
            i = 1000;
            while(i--);
            timeoutCnt = 100000U;
            while (timeoutCnt-- > 0)
            {
                if((*((uint32_t *)(((uint32_t)localP) + 0x3488)) & 0x200) == (uint32_t)0x200)
                {
                    break;
                }
            }
            if (timeoutCnt == 0)
            {
                UART_printf("timeout in GPUAuxInitResotre [3]\n");
            }
            *((uint32_t *)(localP + 0x3420)) = 0x000001FF;
            *((uint32_t *)(localP + 0x34B8)) = 0x002307F8;
            timeoutCnt = 100000U;
            while (timeoutCnt-- > 0)
            {
                if((*((uint32_t *)(localP + 0x34B0)) & 0x1700) == (uint32_t)0x0000)
                {
                    break;
                }
            }
            if (timeoutCnt == 0)
            {
                UART_printf("timeout in GPUAuxInitRestore [4]\n");
            }
            i = 1000;
            while(i--);
            *((uint32_t *)(((uint32_t)localP) + 0x3480)) = 0x00;
            i = 1000;
            while(i--);
        }
        else
        {
          /* TODO: Need to see how to revert */
        }
    }
    if (testResult == 0)
    {
        /* Disable RAT translation */
        result = CSL_ratDisableRegionTranslation((CSL_ratRegs *)PBIST_RAT_CFG_BASE,
                                                 PBIST_RAT_REGION2_INDEX);
        if (result == false) {
            UART_printf("   CSL_ratDisableRegionTranslation...FAILED \n");
            testResult = -1;
        }
    }

    return testResult;
}

int32_t PBIST_DSSAuxInitRestore(bool init)
{
    int32_t testResult = 0;
    uint32_t value;

    uint32_t *common_m = (uint32_t *)(CSL_DSS0_DISPC_0_COMMON_M_BASE);

    uint32_t *vp2 = (uint32_t *)(CSL_DSS0_VP2_BASE);

    uint32_t *ovr2 = (uint32_t *)(SDL_DSS0_OVR2_BASE);

    uint32_t *vid1 = (uint32_t *)(SDL_DSS0_VID1_BASE);
    uint32_t *vid2 = (uint32_t *)(SDL_DSS0_VIDL1_BASE);
    uint32_t *vid3 = (uint32_t *)(SDL_DSS0_VID2_BASE);
    uint32_t *vid4 = (uint32_t *)(SDL_DSS0_VIDL2_BASE);
    uint32_t pbistEnMask = 0;
    pbistEnMask |= 0x1 << 13; /* csi */
    pbistEnMask |= 0x1 << 12; /* dsi */

    uint32_t clkgateCtrl2Mask = 0;
    clkgateCtrl2Mask |= 0x1 << 25; /* MAIN CSI_TX1 */
    clkgateCtrl2Mask |= 0x1 << 24; /* MAIN CSI_TX0 */
    clkgateCtrl2Mask |= 0x1 << 16; /* MAIN CSI PSILSS */

    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1, KICK1_UNLOCK_VAL);

    /* Configure PBIST_EN register using SDL register access APIs */
    uint32_t pbistEnRegAddr = SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_PBIST_EN;
    uint32_t pbistEnTmp = SDL_REG32_RD(pbistEnRegAddr);

    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK0, KICK0_UNLOCK_VAL);
    SDL_REG32_WR(SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_LOCK6_KICK1, KICK1_UNLOCK_VAL);

    /* Configure MAIN_CLKGATE_CTRL2 register using SDL register access APIs */
    uint32_t clkgateCtrl2RegAddr = SDL_WKUP_CTRL_MMR0_CFG0_BASE + SDL_WKUP_CTRL_MMR_CFG0_MAIN_CLKGATE_CTRL2;
    uint32_t clkgateCtrl2Tmp = SDL_REG32_RD(clkgateCtrl2RegAddr);

    if (init)
    {
        /* Initialize: Set the specified bits in PBIST_EN */
        pbistEnTmp |= pbistEnMask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);

        /* Initialize: Set the specified bits in MAIN_CLKGATE_CTRL2 */

        /* Notice: If the mask was set before initialization, this sequence will clear the mask on restore. */
        clkgateCtrl2Tmp |= clkgateCtrl2Mask;
        SDL_REG32_WR(clkgateCtrl2RegAddr, clkgateCtrl2Tmp);

        CSL_REG32_WR(&((CSL_dss_vid1Regs*)(vid1))->FBDC_ATTRIBUTES, 1);
        CSL_REG32_WR(&((CSL_dss_vidl1Regs*)(vid2))->FBDC_ATTRIBUTES, 1);
        CSL_REG32_WR(&((CSL_dss_vid1Regs*)(vid3))->FBDC_ATTRIBUTES, 1);
        CSL_REG32_WR(&((CSL_dss_vidl1Regs*)(vid4))->FBDC_ATTRIBUTES, 1);

        CSL_REG32_FINS(&((CSL_dss_vidl1Regs*)(vid2))->ATTRIBUTES, DSS_VIDL1_ATTRIBUTES_ENABLE, 1);

        value = CSL_FMK(DSS_OVR1_ATTRIBUTES_ENABLE, 1) | CSL_FMK(DSS_OVR1_ATTRIBUTES_CHANNELIN, 1);
        CSL_REG32_WR(&((CSL_dss_ovr1Regs *)(ovr2))->ATTRIBUTES, value);

        {
            uint32_t division = 2400 / 100;

            uint32_t divfactor = 0x8000 | (division-1);

            SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0, KICK0_UNLOCK_VAL);
            SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + SDL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1, KICK1_UNLOCK_VAL);

            SDL_REG32_WR(SDL_CTRL_MMR0_CFG0_BASE + CSL_MAIN_CTRL_MMR_CFG0_DSS_DISPC0_CLKSEL1, 0x1);
            SDL_REG32_WR(CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL19_PID + CSL_MAIN_PLL_MMR_CFG_PLL0_HSDIV_CTRL0, divfactor);
        }

        CSL_REG32_FINS(&((CSL_dss_common_mRegs*)(common_m))->DISPC_CONNECTIONS, DSS_COMMON_M_DISPC_CONNECTIONS_DPI_0_CONN, 4);

        CSL_REG32_FINS(&((CSL_dss_vp1Regs*)(vp2))->CONTROL, DSS_VP1_CONTROL_ENABLE, 1);
    }
    else
    {
        /* Restore: Clear the bits that were set during init in PBIST_EN */
        pbistEnTmp &= ~pbistEnMask;
        SDL_REG32_WR(pbistEnRegAddr, pbistEnTmp);

        /* Restore: Clear the bits that were set during init in MAIN_CLKGATE_CTRL2 */
        clkgateCtrl2Tmp &= ~clkgateCtrl2Mask;
        SDL_REG32_WR(clkgateCtrl2RegAddr, clkgateCtrl2Tmp);

        /* Restore DSS configuration by clearing enables */
        CSL_REG32_WR(&((CSL_dss_vid1Regs*)(vid1))->FBDC_ATTRIBUTES, 0);
        CSL_REG32_WR(&((CSL_dss_vidl1Regs*)(vid2))->FBDC_ATTRIBUTES, 0);
        CSL_REG32_WR(&((CSL_dss_vid1Regs*)(vid3))->FBDC_ATTRIBUTES, 0);
        CSL_REG32_WR(&((CSL_dss_vidl1Regs*)(vid4))->FBDC_ATTRIBUTES, 0);

        CSL_REG32_FINS(&((CSL_dss_vidl1Regs*)(vid2))->ATTRIBUTES, DSS_VIDL1_ATTRIBUTES_ENABLE, 0);

        value = CSL_FMK(DSS_OVR1_ATTRIBUTES_ENABLE, 0) | CSL_FMK(DSS_OVR1_ATTRIBUTES_CHANNELIN, 0);
        CSL_REG32_WR(&((CSL_dss_ovr1Regs *)(ovr2))->ATTRIBUTES, value);

        CSL_REG32_FINS(&((CSL_dss_vp1Regs*)(vp2))->CONTROL, DSS_VP1_CONTROL_ENABLE, 0);
    }

    return testResult;
}
char * PBIST_getPostStatusString(SDL_PBIST_postStatus postStatus)
{
    if (postStatus == SDL_PBIST_POST_NOT_RUN)
    {
        return PBIST_POST_NOT_RUN_STR;
    }
    else if (postStatus == SDL_PBIST_POST_TIMEOUT)
    {
        return PBIST_POST_ATTEMPTED_TIMEOUT_STR;
    }
    else if (postStatus == SDL_PBIST_POST_COMPLETED_FAILURE)
    {
        return PBIST_POST_COMPLETED_FAILURE_STR;
    }
    else if (postStatus == SDL_PBIST_POST_COMPLETED_SUCCESS)
    {
        return PBIST_POST_COMPLETED_SUCCESS_STR;
    }
    return NULL;
}

void PBIST_printPostStatus(SDL_PBIST_postResult *result)
{
    UART_printf("    HW POST MCU Status : %s\n", (PBIST_getPostStatusString(result->mcuPostStatus)) ? : "Invalid");

    return;
}

int32_t PBIST_MainR50AuxInitRestore(bool init)
{
    int32_t testResult = SDL_PASS;
    struct tisci_msg_proc_get_status_resp cpuStatus;
    struct tisci_msg_proc_set_config_req proc_set_config_req;
    uint8_t procId = SCICLIENT_PROC_ID_R5FSS0_CORE0; /* Main R5FSS0 Core 0 */

    /* Step 1: Get current processor configuration without requesting control */
    testResult = Sciclient_procBootGetProcessorState(procId, &cpuStatus, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(testResult == CSL_PASS)
    {
        /* Step 2: Prepare configuration based on init flag */
        proc_set_config_req.processor_id = cpuStatus.processor_id;
        proc_set_config_req.bootvector_lo = cpuStatus.bootvector_lo;
        proc_set_config_req.bootvector_hi = cpuStatus.bootvector_hi;

        if (init)
        {
            /* Initialize: Change from lockstep to split mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) != 0U)
            {
                /* Currently in lockstep, change to split mode */
                proc_set_config_req.config_flags_1_set = 0U;
                proc_set_config_req.config_flags_1_clear = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
            }
            else
            {
                /* Already in split mode, no change needed */
            }
        }
        else
        {
            /* Restore: Change from split to lockstep mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) == 0U)
            {
                /* Currently in split mode, change to lockstep */
                proc_set_config_req.config_flags_1_set = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
                proc_set_config_req.config_flags_1_clear = 0U;
            }
            else
            {
                /* Already in lockstep mode, no change needed */
            }
        }
    }
    else
    {
        UART_printf("ERROR: Failed to get processor state for MainR50 (0x%02X)\n", procId);
    }
    if(testResult == SDL_PASS)
    {
        /* Step 3: Apply the new configuration */
        testResult = Sciclient_procBootSetProcessorCfg(&proc_set_config_req, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (testResult != CSL_PASS)
        {
            UART_printf("ERROR: Failed to set processor config for MainR50 (0x%02X)\n", procId);
        }
        else
        {
            UART_printf("MainR50: Mode change completed successfully\n");
        }
    }
    return testResult;
}

int32_t PBIST_MainR51AuxInitRestore(bool init)
{
    int32_t testResult = SDL_PASS;
    struct tisci_msg_proc_get_status_resp cpuStatus;
    struct tisci_msg_proc_set_config_req proc_set_config_req;
    uint8_t procId = SCICLIENT_PROC_ID_R5FSS1_CORE0; /* Main R5FSS1 Core 0 */

    /* Step 1: Get current processor configuration without requesting control */
    testResult = Sciclient_procBootGetProcessorState(procId, &cpuStatus, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(testResult == CSL_PASS)
    {
        /* Step 2: Prepare configuration based on init flag */
        proc_set_config_req.processor_id = cpuStatus.processor_id;
        proc_set_config_req.bootvector_lo = cpuStatus.bootvector_lo;
        proc_set_config_req.bootvector_hi = cpuStatus.bootvector_hi;

        if (init)
        {
            /* Initialize: Change from lockstep to split mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) != 0U)
            {
                /* Currently in lockstep, change to split mode */
                proc_set_config_req.config_flags_1_set = 0U;
                proc_set_config_req.config_flags_1_clear = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
            }
            else
            {
                /* Already in split mode, no change needed */
            }
        }
        else
        {
            /* Restore: Change from split to lockstep mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) == 0U)
            {
                /* Currently in split mode, change to lockstep */
                proc_set_config_req.config_flags_1_set = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
                proc_set_config_req.config_flags_1_clear = 0U;
            }
            else
            {
                /* Already in lockstep mode, no change needed */
            }
        }
    }
    else
    {
        UART_printf("ERROR: Failed to get processor state for MainR51 (0x%02X)\n", procId);
    }
    if(testResult == SDL_PASS)
    {
        /* Step 3: Apply the new configuration */
        testResult = Sciclient_procBootSetProcessorCfg(&proc_set_config_req, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (testResult != CSL_PASS)
        {
            UART_printf("ERROR: Failed to set processor config for MainR51 (0x%02X)\n", procId);
        }
        else
        {
            UART_printf("MainR51: Mode change completed successfully\n");
        }
    }
    return testResult;
}

int32_t PBIST_MainR52AuxInitRestore(bool init)
{
    int32_t testResult = SDL_PASS;
    struct tisci_msg_proc_get_status_resp cpuStatus;
    struct tisci_msg_proc_set_config_req proc_set_config_req;
    uint8_t procId = SCICLIENT_PROC_ID_R5FSS2_CORE0; /* Main R5FSS2 Core 0 */

    /* Step 1: Get current processor configuration without requesting control */
    testResult = Sciclient_procBootGetProcessorState(procId, &cpuStatus, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(testResult == CSL_PASS)
    {
        /* Step 2: Prepare configuration based on init flag */
        proc_set_config_req.processor_id = cpuStatus.processor_id;
        proc_set_config_req.bootvector_lo = cpuStatus.bootvector_lo;
        proc_set_config_req.bootvector_hi = cpuStatus.bootvector_hi;

        if (init)
        {
            /* Initialize: Change from lockstep to split mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) != 0U)
            {
                /* Currently in lockstep, change to split mode */
                proc_set_config_req.config_flags_1_set = 0U;
                proc_set_config_req.config_flags_1_clear = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
            }
            else
            {
                /* Already in split mode, no change needed */
            }
        }
        else
        {
            /* Restore: Change from split to lockstep mode */
            if ((cpuStatus.config_flags_1 & TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP) == 0U)
            {
                /* Currently in split mode, change to lockstep */
                proc_set_config_req.config_flags_1_set = TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
                proc_set_config_req.config_flags_1_clear = 0U;
            }
            else
            {
                /* Already in lockstep mode, no change needed */
            }
        }
    }
    else
    {
        UART_printf("ERROR: Failed to get processor state for MainR52 (0x%02X)\n", procId);
    }
    if(testResult == SDL_PASS)
    {
        /* Step 3: Apply the new configuration */
        testResult = Sciclient_procBootSetProcessorCfg(&proc_set_config_req, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (testResult != CSL_PASS)
        {
            UART_printf("ERROR: Failed to set processor config for MainR52 (0x%02X)\n", procId);
        }
        else
        {
            UART_printf("MainR52: Mode change completed successfully\n");
        }
    }
    return testResult;
}