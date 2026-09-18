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
 *  \file     lpm_mcu_only_recovery.c
 *
 *  \brief    This file implements main/mcu domain-related functions for switching the
 *            SoCs state from ACTIVE to MCU ONLY mode and from MCU ONLY to ACTIVE mode.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/csl/soc.h>
#if defined(SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_cfg.h>
#elif defined(SOC_J7200)
#include <ti/board/src/j7200_evm/include/board_cfg.h>
#elif defined(SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_cfg.h>
#endif
#include <ti/drv/lpm/lpm.h>
#include <ti/drv/lpm/src/lpm_uart_drv.h>
#include <ti/drv/lpm/src/lpm_mcu_only_recovery.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Write this value to the debug loop variable in CCS to break execution and
 * to pause execution at the corresponding debug loop.
 */
#define LPM_MCU_ONLY_DEBUG_CCS_HALT                  (0xDEADBEEFU)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**
 * \brief Holds the RM board configuration and its resource assignment table.
 *
 * The board configuration is sent for DEVGRP_01 (MAIN domain) during
 * the MCU Only recovery sequence.
 */
struct Lpm_mcuOnlylocalRmBoardcfg
{
    struct tisci_boardcfg_rm              rm_boardcfg;
    struct tisci_boardcfg_rm_resasg_entry resasg_entries[TISCI_RESASG_ENTRIES_MAX];
};

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#ifdef LPM_DEBUG
volatile uint32_t loopSwResetMainDomain = 0;
volatile uint32_t ccs_halt = 0x0;
#endif

/* Board configs required to recover back main domain from mcu_only mode */
__attribute((section(".sysfw_data_cfg_board")))     struct tisci_boardcfg gLpm_mcuOnlyBoardCfg;
__attribute((section(".sysfw_data_cfg_board_rm")))  struct Lpm_mcuOnlylocalRmBoardcfg gLpm_mcuOnlyBoardCfgRm;
__attribute((section(".sysfw_data_cfg_board_sec"))) struct tisci_boardcfg_sec gLpm_mcuOnlyBoardCfgSec;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 * \brief Enable the WKUPMCU2MAIN and MAIN2WKUPMCU inter-domain bridges via
 *        Sciclient so that the MCU domain can communicate with the MAIN domain.
 *        These devices are in the MCU devgrp and can be enabled before the MAIN
 *        domain board configuration is sent.
 *
 * \return CSL_PASS on success, CSL_EFAIL if either bridge fails to enable.
 */
static int32_t Lpm_mcuOnlyEnableMcu2MainBridges(void);

/**
 * \brief Perform read/write sanity checks on DDR, MSMC, and the MAIN MCAN0
 *        revision register to verify that MAIN domain peripherals are
 *        accessible after the domain has been brought back.
 */
static void Lpm_mcuOnlyAccessMainPeripherals(void);

/**
 * \brief Send the full board configuration (common, PM, RM, security) to the
 *        System Controller for DEVGRP_01 (MAIN domain), then run Board_init
 *        to re-initialize MAIN domain clocks, DDR, and UART.
 */
static void Lpm_mcuOnlySendMainDomainBoardCfgs(void);

/**
 * \brief Orchestrate the full MAIN domain bring-up sequence: enable the
 *        inter-domain bridges, send board configuration, and verify peripheral
 *        access.  Called after the PMIC has been transitioned back to ACTIVE
 *        state and MAIN domain isolation has been released.
 */
static void Lpm_mcuOnlyBringBackMainDomain(void);

/**
 * \brief Issue a software reset to the MAIN domain (DEVGRP_01) using the
 *        TISCI_MSG_SYS_RESET Sciclient message. This powers down and resets
 *        all MAIN domain modules before entering MCU Only mode.
 *        See TISCI user guide: TISCI_MSG_SYS_RESET.
 */
static void Lpm_mcuOnlySwResetMainDomain(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_mcuOnlyToActiveSwitch(void)
{
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Starting PMIC STATE CHANGE: MCU ONLY -> ACTIVE...\n");
    Lpm_pmicStateChangeMCUOnlyToActive();
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "PMIC STATE CHANGE: MCU ONLY -> ACTIVE...Done\n");

    /* Disabling MAIN domain deep sleep isolation */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Disabling MAIN domain deep sleep isolation...\n");
    CSL_REG32_WR((CSL_WKUP_CTRL_MMR0_CFG0_BASE + CSL_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL),
                 LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_DIS);

    /* Enable the MAIN domain, till now we have only changed the PMIC state
     * lets enable the modules in the MAIN domain.
     */
    Lpm_mcuOnlyBringBackMainDomain();
}

uint32_t Lpm_mcuOnlyActiveToMcuSwitch(void)
{
    /* Issue a SW reset to the MAIN domain */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Issuing a SW reset to the MAIN domain...\n");
    Lpm_mcuOnlySwResetMainDomain();

    /* Enable MAIN domain isolation, this is done to prevent MAIN domain signals from reaching the MCU domain */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Enabling MAIN domain deep sleep isolation...\n");
    CSL_REG32_WR((CSL_WKUP_CTRL_MMR0_CFG0_BASE + CSL_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL),
                 LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_EN);

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "PMIC STATE CHANGE: ACTIVE -> MCU ONLY...\n");

    /* Change PMIC state from ACTIVE to MCU ONLY */
    Lpm_pmicStateChangeActiveToMCUOnly();
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "PMIC STATE CHANGE: ACTIVE -> MCU ONLY...Done\n");

    return 0;
}

void Lpm_mcuOnlyDisableMaxOutrgAlert(void)
{
    /*
     * Disable the VTM max-outrange alert for temperature sensors 1 through 4
     * in the MAIN domain.
     *
     * This must be called before entering MCU-only mode to
     * prevent spurious thermal alerts from firing while the MAIN domain is
     * powered down.
     */

    uint32_t vtmRegVal;

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Disabling MAXT_OUTRG_EN interrupt for TMPSENS1:4 in MAIN domain!\n");

    /* Disable MAXT_OUTRG_EN interrupt for TMPSENS1 */
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_1);
    vtmRegVal &= (~LPM_MCU_ONLY_MAXT_OUTRG_EN_MASK);
    HW_WR_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_1, vtmRegVal);
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_1);

    /* Disable MAXT_OUTRG_EN interrupt for TMPSENS2 */
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_2);
    vtmRegVal &= (~LPM_MCU_ONLY_MAXT_OUTRG_EN_MASK);
    HW_WR_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_2, vtmRegVal);
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_2);

    /* Disable MAXT_OUTRG_EN interrupt for TMPSENS3 */
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_3);
    vtmRegVal &= (~LPM_MCU_ONLY_MAXT_OUTRG_EN_MASK);
    HW_WR_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_3, vtmRegVal);
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_3);

    /* Disable MAXT_OUTRG_EN interrupt for TMPSENS4 */
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_4);
    vtmRegVal &= (~LPM_MCU_ONLY_MAXT_OUTRG_EN_MASK);
    HW_WR_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_4, vtmRegVal);
    vtmRegVal = HW_RD_REG32(LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_4);
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static int32_t Lpm_mcuOnlyEnableMcu2MainBridges(void)
{
    /* Enable both inter-domain bridges, these are needed to establish communication between MCU and
     * MAIN domain. As they are in the MCU devgrp and hence we can enable then w/o passing the
     * board configuration for the MAIN devgrp.
     */

    int32_t status;

    status = Sciclient_pmSetModuleState(TISCI_DEV_WKUPMCU2MAIN_VD,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        TISCI_MSG_FLAG_AOP | TISCI_MSG_FLAG_DEVICE_RESET_ISO,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        status = Sciclient_pmSetModuleState(TISCI_DEV_MAIN2WKUPMCU_VD,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                            TISCI_MSG_FLAG_AOP | TISCI_MSG_FLAG_DEVICE_RESET_ISO,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    }

    return status;
}

static void Lpm_mcuOnlyAccessMainPeripherals(void)
{
    uint32_t readValDDR;
    uint32_t readValMSMC;
    uint32_t readValMCAN;
    uint32_t writeValMSMC = LPM_MCU_ONLY_MSMC_TEST_VAL;
    uint32_t writeValDDR = LPM_MCU_ONLY_DDR_TEST_VAL;

    /* Test if DDR is accessible */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Writing to DDR...\n");
    HW_WR_REG32(LPM_MCU_ONLY_DDR_TEST_ADDRESS, writeValDDR);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Reading from DDR...\n");
    readValDDR = HW_RD_REG32(LPM_MCU_ONLY_DDR_TEST_ADDRESS);
    if(writeValDDR == readValDDR)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Read value matches the value written for DDR!\n");
    }

    /* Test if MSMC is accessible */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Writing to MSMC...\n");
    HW_WR_REG32(LPM_MCU_ONLY_MSMC_TEST_ADDRESS, writeValMSMC);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Reading from MSMC...\n");
    readValMSMC = HW_RD_REG32(LPM_MCU_ONLY_MSMC_TEST_ADDRESS);
    if(writeValMSMC == readValMSMC)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Read value matches the value written for MSMC!\n");
    }

    /* Test if MAIN_MCAN peripheral is accessible */
    readValMCAN = HW_RD_REG32(LPM_MCU_ONLY_MCAN_REV_REG);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Reading MAIN MCAN0 Revision register...\n");
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "MAIN MCAN0 MCANSS_PID = 0x%x\n", readValMCAN);
}

static void Lpm_mcuOnlySendMainDomainBoardCfgs(void)
{
    /*
     * Send the four Sciclient board configurations for the MAIN domain
     * (DEVGRP_01) in the order required by the System Controller firmware:
     *
     *  1. Common board config   — peripheral pinmux and SoC integration data.
     *  2. PM board config       — power-management clock and voltage domains.
     *  3. Security board config — firewall and privilege settings.
     *  4. RM board config       — resource-manager channel and ring assignments.
     *
     * All four configs must be sent before calling Board_init, which configures
     * MAIN domain clocks, DDR training, and the debug UART.
     *
     * The common board config is stored in the dedicated .sysfw_data_cfg_board
     * linker section so the System Controller can access it directly.
     * The RM config uses a local structure that embeds both the header and the
     * resource-assignment table to keep them contiguous in memory.
     */

    int32_t retVal;
    Sciclient_DefaultBoardCfgInfo_t boardCfgInfo;

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Configuring Sciclient_board for MAIN domain\n");

    /* Retrieve the PM and RM board config pointers built into the firmware image */
    retVal = Sciclient_getDefaultBoardCfgInfo(&boardCfgInfo);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Sciclient_getDefaultBoardCfgInfo() failed.\n");
    }

    /* Prepare the boardcfgs to be sent during main domain recovery */
    Sciclient_BoardCfgPrms_t mcuOnlyBoardCfgPrms = {
                                                    .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfg,
                                                    .boardConfigHigh = 0,
                                                    .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfg),
                                                    .devGrp = DEVGRP_01
                                                   };

    Sciclient_BoardCfgPrms_t mcuOnlyBoardCfgPmPrms = {
                                                      .boardConfigLow = (uint32_t)boardCfgInfo.boardCfgLowPm,
                                                      .boardConfigHigh = 0,
                                                      .boardConfigSize = boardCfgInfo.boardCfgLowPmSize,
                                                      .devGrp = DEVGRP_01
                                                     };

    Sciclient_BoardCfgPrms_t mcuOnlyBoardCfgRmPrms = {
                                                      .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfgRm,
                                                      .boardConfigHigh = 0,
                                                      .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfgRm),
                                                      .devGrp = DEVGRP_01
                                                     };

    Sciclient_BoardCfgPrms_t mcuOnlyBoardCfgSecPrms = {
                                                       .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfgSec,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfgSec),
                                                       .devGrp = DEVGRP_01
                                                      };

#ifdef LPM_DEBUG
    if(ccs_halt == LPM_MCU_ONLY_DEBUG_CCS_HALT)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Connect CCS and break out of ccs_halt loop!!\n");
	    while(ccs_halt)
        {
            /* Wait for user to change the state of halt variable before
             * sending boardconfigs to main domain.
             */
        }
    }
#endif

    retVal = Sciclient_boardCfg(&mcuOnlyBoardCfgPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Sciclient_boardCfg() failed.\n");
    }

    retVal = Sciclient_boardCfgPm(&mcuOnlyBoardCfgPmPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Sciclient_boardCfgPm() failed.\n");
    }

    retVal = Sciclient_boardCfgSec(&mcuOnlyBoardCfgSecPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Sciclient_boardCfgSec() failed.\n");
    }

    retVal = Sciclient_boardCfgRm(&mcuOnlyBoardCfgRmPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Sciclient_boardCfgRm() failed.\n");
    }

    /* Re-initialize the MAIN domain peripherals: PLL, clocks, DDR, and UART. */
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Doing board init...\n");
    Board_uartDeInit();
    Board_initCfg cfg = BOARD_INIT_PINMUX_CONFIG     |
                        BOARD_INIT_PLL_MAIN          |
                        BOARD_INIT_MODULE_CLOCK_MAIN |
                        BOARD_INIT_DDR               |
                        BOARD_INIT_UART_STDIO;
    Board_init(cfg);
}

static void Lpm_mcuOnlyBringBackMainDomain(void)
{
    int32_t status;

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Configure WKUPMCU2MAIN and MAIN2WKUPMCU Bridges...\n");

    /* Enable WKUPMCU2MAIN and MAIN2WKUPMCU bridges,
     * this needs to be done before sending the RM, PM, Sec and common board cfg
     * for DEVGRP01.
     */
    status = Lpm_mcuOnlyEnableMcu2MainBridges();

    /* Pass board config for MAIN domain i.e. DEVGRP01 */
    if(status == CSL_PASS)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "WKUPMCU2MAIN and MAIN2WKUPMCU Bridges configured successfully!\n");
        Lpm_mcuOnlySendMainDomainBoardCfgs();
    }

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Trying to access MAIN domain peripherals...\n");
    Lpm_mcuOnlyAccessMainPeripherals();
}

static void Lpm_mcuOnlySwResetMainDomain(void)
{
    struct tisci_msg_sys_reset_req request;
    struct tisci_msg_sys_reset_resp response = {0};

    Sciclient_ReqPrm_t reqParam;
    Sciclient_RespPrm_t respParam;
    int32_t retVal;

    memset(&request, 0, sizeof(request));
    request.domain = DEVGRP_01;

    reqParam.messageType        = (uint16_t) TISCI_MSG_SYS_RESET;
    reqParam.flags              = (uint32_t) TISCI_MSG_FLAG_AOP;
    reqParam.pReqPayload        = (const uint8_t *) &request;
    reqParam.reqPayloadSize     = (uint32_t) sizeof (request);
    reqParam.timeout            = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER;
    respParam.flags             = (uint32_t) 0;   /* Populated by the API */
    respParam.pRespPayload      = (uint8_t *) &response;
    respParam.respPayloadSize   = (uint32_t) sizeof (response);

#ifdef LPM_DEBUG
    /* For debug purpose */
    if(loopSwResetMainDomain == LPM_MCU_ONLY_DEBUG_CCS_HALT)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Connect CCS and change loopSwResetMainDomain to 0!\n");
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "After that the MAIN domain will be reset!\n");
    }

    while(loopSwResetMainDomain == LPM_MCU_ONLY_DEBUG_CCS_HALT)
    {
        /* Wait for user to change the state of halt variable before
         * warm resetting the main domain.
         */
    }
#endif

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Entering into SW Reset using Sciclient call\n");
    retVal = Sciclient_service(&reqParam, &respParam);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Performed SW Reset using Sciclient call\n");

    if ((respParam.flags & TISCI_MSG_FLAG_ACK) == 0)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "retVal = %d\nGOT NACK! resp flag = 0x%08x\n",
                          retVal, respParam.flags);
    }
}
