/*
 *  Copyright (c) Texas Instruments Incorporated 2025-26
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
 *  \file lbist_defs.c
 *
 *  \brief LBIST SOC-specific structures and functions
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <sdl_types.h>
#include <soc.h>
#include <src/sdl/sdl_lbist.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/sciclient/sciclient.h>

#include "lbist_utils.h"
#include "lbist_defs.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define A72_NUM_AUX_DEVICES            1

/* HW POST run status definitions */
#define LBIST_POST_COMPLETED_SUCCESS_STR      "SDL_LBIST_POST_COMPLETED_SUCCESS"
#define LBIST_POST_COMPLETED_FAILURE_STR      "SDL_LBIST_POST_COMPLETED_FAILURE"
#define LBIST_POST_ATTEMPTED_TIMEOUT_STR      "SDL_LBIST_POST_TIMEOUT"
#define LBIST_POST_NOT_RUN_STR                "SDL_LBIST_POST_NOT_RUN"

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

char * LBIST_getPostStatusString(SDL_LBIST_postStatus postStatus);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint32_t LBIST_A72AuxDevList[A72_NUM_AUX_DEVICES] =
{
	TISCI_DEV_A72SS0,
};

LBIST_TestHandle_t LBIST_TestHandleArray[LBIST_MAX_CORE_INDEX+1] =
{
    /* HW POST - SMS - Checks MISR results only */ 
    {
        .coreName               = "HWPOST - SMS",
        .numAuxDevices          = 0u,                       /* No Aux devices */
        .hwPostCoreCheck        = true,
        .hwPostCoreNum          = LBIST_POST_CORE_SMS,
    },
    /* HW POST - MCU - Checks MISR results only */
    {
        .coreName               = "HWPOST - MCU",
        .numAuxDevices          = 0u,                       /* No Aux devices */
        .hwPostCoreCheck        = true,
        .hwPostCoreNum          = LBIST_POST_CORE_MCU,
    },                  /* LBIST event handler */

    /* Main R5F 0 */
    {
        .coreName               = "Main R5F0-0",
        .instance               = SDL_LBIST_INST_MAINR5F0,
        .secondaryCoreNeeded    = true,             /* Secondary core needed */
        .wfiCheckNeeded         = false,            /* wfi check no needed */
        .secCoreName            = "Main R5F1-0",    /* Secondary core */
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = SCICLIENT_PROC_ID_R5FSS0_CORE0, /* Main R5F core 0 Proc Id */
        .tisciSecProcId         = SCICLIENT_PROC_ID_R5FSS0_CORE1, /* Main R5F core 1 Proc Id */
        .tisciDeviceId          = TISCI_DEV_R5FSS0_CORE0,   /* Main R5F core 0 Device Id */
        .tisciSecDeviceId       = TISCI_DEV_R5FSS0_CORE1,   /* Main R5F core 1 Device Id */
        .numAuxDevices          = 0u,                       /* No Aux devices */
    },
    /* Main R5F 1 */
    {
        .coreName               = "Main R5F1-0",
        .instance               = SDL_LBIST_INST_MAINR5F1,
        .secondaryCoreNeeded    = true,            /* Secondary core needed */
        .wfiCheckNeeded         = false,           /* wfi check not needed */
        .secCoreName            = "Main R5F1-1",   /* Secondary core */
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = SCICLIENT_PROC_ID_R5FSS1_CORE0, /* Main R5F core 0 Proc Id */
        .tisciSecProcId         = SCICLIENT_PROC_ID_R5FSS1_CORE1, /* Main R5F core 1 Proc Id */
        .tisciDeviceId          = TISCI_DEV_R5FSS1_CORE0,  /* Main R5F core 0 Device id */
        .tisciSecDeviceId       = TISCI_DEV_R5FSS1_CORE1,  /* Main R5F core 1 Device id */
        .numAuxDevices          = 0u,                      /* No Aux devices */
    },
    /* C7x0 */
    {
        .coreName               = "C7x0 ",
        .instance               = SDL_LBIST_INST_C7X0,
        .secondaryCoreNeeded    = false,  /* Secondary core not needed */
        .wfiCheckNeeded         = false,  /* wfi check not needed */
        .secCoreName            = "None",
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER0_C71SS0_0,  /* C7x0 Proc Id */
        .tisciSecProcId         = 0u,
        .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_0,          /* C7x0 Device Id */
        .tisciSecDeviceId       = 0u,
        .numAuxDevices          = 0u,                        /* No Aux devices */
    },
    /* C7x1 */
    {
        .coreName               = "C7x1 ",
        .instance               = SDL_LBIST_INST_C7X1,
        .secondaryCoreNeeded    = false,  /* Secondary core not needed */
        .wfiCheckNeeded         = false,  /* wfi check not needed */
        .secCoreName            = "None",
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = SCICLIENT_PROC_ID_COMPUTE_CLUSTER0_C71SS1_0,  /* C7x1 Proc Id */
        .tisciSecProcId         = 0u,
        .tisciDeviceId          = TISCI_DEV_COMPUTE_CLUSTER0_C71SS1_0,          /* C7x1 Device Id */
        .tisciSecDeviceId       = 0u,
        .numAuxDevices          = 0u,                        /* No Aux devices */
    },
    /* VPAC0 */
    {
        .coreName               = "VPAC0",
        .instance               = SDL_LBIST_INST_VPAC0,
        .secondaryCoreNeeded    = false,           /* Secondary core not needed */
        .wfiCheckNeeded         = false,           /* wfi check not needed */
        .secCoreName            = "None",
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = 0,  /* No proc id */
        .tisciSecProcId         = 0,  /* No Proc Id */
        .tisciDeviceId          = TISCI_DEV_VPAC0,                       /* VPAC Device Id */
        .tisciSecDeviceId       = 0,
        .numAuxDevices          = 0u,                                    /* No Aux devices */
    },

    /* DMPAC */
    {
        .coreName               = "DMPAC",
        .instance               = SDL_LBIST_INST_DMPAC,
        .secondaryCoreNeeded    = true,           /* Secondary core needed */
        .wfiCheckNeeded         = false,           /* wfi check not needed */
        .secCoreName            = "None",
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = 0,  /* No proc id */
        .tisciSecProcId         = 0,  /* No Proc Id */
        .tisciDeviceId          = TISCI_DEV_DMPAC0,                       /* DMPAC Device Id */
        .tisciSecDeviceId       = TISCI_DEV_DMPAC0_SDE_0,                 /*DMPAC secondary Device Id*/
        .numAuxDevices          = 0u,                                     /* No Aux devices */
    },

    /* A72_0*/
    {
        .coreName               = "A72 core 0",
        .instance               = SDL_LBIST_INST_A72,
        .secondaryCoreNeeded    = true,           /* Secondary core needed */
        .wfiCheckNeeded         = false,          /* wfi check not needed */
        .secCoreName            = "A72 core 1",   /* Secondary core */
        .cpuStatusFlagMask      = TISCI_MSG_VAL_PROC_BOOT_STATUS_FLAG_R5_WFI, /* Expected boot status value for wfi */
        .tisciProcId            = SCICLIENT_PROC_ID_A72SS0_CORE0, /* A72 core 0 Proc Id */
        .tisciSecProcId         = SCICLIENT_PROC_ID_A72SS0_CORE1, /* A72 core 1 Proc Id */
        .tisciDeviceId          = TISCI_DEV_A72SS0_CORE0,  /* A72 core 0 Device Id */
        .tisciSecDeviceId       = TISCI_DEV_A72SS0_CORE1,  /* A72 core 1 Device Id */
        .numAuxDevices          = A72_NUM_AUX_DEVICES,     /* Number of Aux devices */
        .auxDeviceIdsP          = &LBIST_A72AuxDevList[0], /* Array of Aux device ids */
    },


};

/* ========================================================================== */
/*                            External Variables                              */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

char * LBIST_getPostStatusString(SDL_LBIST_postStatus postStatus)
{
    if (postStatus == SDL_LBIST_POST_NOT_RUN)
    {
        return LBIST_POST_NOT_RUN_STR;
    }
    else if (postStatus == SDL_LBIST_POST_TIMEOUT)
    {
        return LBIST_POST_ATTEMPTED_TIMEOUT_STR;
    }
    else if (postStatus == SDL_LBIST_POST_COMPLETED_FAILURE)
    {
        return LBIST_POST_COMPLETED_FAILURE_STR;
    }
    else if (postStatus == SDL_LBIST_POST_COMPLETED_SUCCESS)
    {
        return LBIST_POST_COMPLETED_SUCCESS_STR;
    }
    return NULL;
}

void LBIST_printPostStatus(SDL_LBIST_postResult *result)
{
    UART_printf("    HW POST MCU Status : %s\n", (LBIST_getPostStatusString(result->mcuPostStatus)) ? : "Invalid");
    UART_printf("    HW POST DMSC Status : %s\n", (LBIST_getPostStatusString(result->smsPostStatus)) ? : "Invalid");

    return;
}

