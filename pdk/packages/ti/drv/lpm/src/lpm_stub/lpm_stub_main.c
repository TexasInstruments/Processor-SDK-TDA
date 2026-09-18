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
 * \file lpm_stub_main.c
 *
 * \brief Entry point for LPM suspend-to-RAM support.
 * Contains the main retention entry function, cache clean, and IO isolation.
 *
 * IO isolation strategy differs by SoC:
 *   J7200:          The CANUART IO daisy-chain is connected to the PMIC.
 *                   CANUART_IOs are a subset of the MAIN domain IOs, so
 *                   isolate the MAIN domain to relay the wakeup signal.
 *
 *   J784S4/J742S2:  The MCU_GENERAL IO daisy-chain is connected to the PMIC.
 *                   Isolate the MCU_GENERAL IOs to relay the wakeup signal.
 *
 *   J721S2:         The HYPERNET IO daisy-chain is connected to the PMIC via
 *                   the PMCTRL_IO_1. Isolate the MAIN domain to relay the
 *                   wakeup signal.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/soc.h>
#if defined(SOC_J7200)
#include <ti/csl/soc/j7200/src/cslr_wkup_ctrl_mmr.h>
#include <ti/csl/soc/j7200/src/csl_soc_firewalls.h>
#include <ti/board/src/j7200_evm/include/board_cfg.h>
#include <ti/board/src/j7200_evm/include/board_internal.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_wkup_ctrl_mmr.h>
#include <ti/board/src/j784s4_evm/include/board_cfg.h>
#include <ti/board/src/j784s4_evm/include/board_internal.h>
#elif defined(SOC_J742S2)
#include <ti/csl/soc/j784s4/src/cslr_wkup_ctrl_mmr.h>
#include <ti/board/src/j742s2_evm/include/board_cfg.h>
#include <ti/board/src/j742s2_evm/include/board_internal.h>
#elif defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_wkup_ctrl_mmr.h>
#include <ti/board/src/j721s2_evm/include/board_cfg.h>
#include <ti/board/src/j721s2_evm/include/board_internal.h>
#endif
#include <ti/drv/lpm/src/lpm_ddr.h>
#include <ti/drv/lpm/src/lpm_stub_utils.h>
#include <ti/drv/lpm/include/lpm_pmic.h>
#include <ti/drv/lpm/src/lpm_uart_baremetal.h>
#include <ti/drv/lpm/src/lpm_io_isolation.h>
#include <ti/drv/lpm/include/lpm_stub.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined(SOC_J784S4) || defined(SOC_J742S2)
/* On J784S4 and J742S2, the MCU_GENERAL IOs are isolated.
 * These IOs include the bootstrap/bootmode IOs as well used by BootROM
 * to detect the boot device.
 * Hence, bypass IO isolation on all bootmode IOs.
 *
 * On J7200, the MAIN domain IOs specifically the CANUART_IO group is isolated
 * these do not contain bootmode IOs, So we dont need to bypass they in this case.
 */
static const uint32_t gLpmStubBootstrapPadOffsets[LPM_STUB_NUM_BOOTSTRAP_IOS] = {
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG3,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG4,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG7,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG8,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG72,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG95,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG96,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG97,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG40,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG41,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG42,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG48,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG49,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG50,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG62,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG63,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG60,
    CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG61,
};
#endif

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/** \brief Clean and invalidate the entire R5F data cache before putting DDR in self-refresh.
 */
static void Lpm_stubCleanAllDCache(void);

/**
 * \brief Write a wake-controller magic word, enable the load, and poll the
 *        status register until the hardware confirms the word has been latched.
 *
 * \param ctrlReg         Register offset of the wake control register.
 * \param loadEnableMask  Bit mask to set in ctrlReg to trigger the latch.
 * \param statReg         Register offset of the status register to poll.
 */
static void Lpm_stubLatchWakeCtrl(uint32_t ctrlReg, uint32_t loadEnableMask, uint32_t statReg);

#if defined(SOC_J7200)

/**
 * \brief Isolate MAIN domain CANUART IOs so the io_daisy_chain can relay the
 *        wakeup signal to the PMIC when activity is detected on those pads.
 */
static void Lpm_stubEnableMainIOIsolation(void);

/** \brief Configure H_PMIC_WAKE0 as output for PMIC wake signalling.
 */
static void Lpm_stubConfigurePmicWake0Pad(void);

/** \brief Enable IO isolation bypass, global wakeup, and deep sleep via DMSC PWRCTRL.
 */
static void Lpm_stubPrepareMainDomainForIsolation(void);

/** \brief Toggle WUCLK high then low, waiting for hardware acknowledgement at each edge.
 */
static void Lpm_stubToggleWuclk(void);

#elif defined(SOC_J784S4) || defined(SOC_J742S2)

/**
 * \brief Isolate MCU_GENERAL IOs so the MCU domain io_daisy_chain can relay the
 *        wakeup signal to the PMIC when activity is detected on those pads.
 */
static void Lpm_stubEnableMcuIOIsolation(void);

/** \brief Bypass IO isolation on the WKUP_I2C pads (PADCONFIG64/65) so PMIC
 *         commands continue to work during the isolation sequence.
 */
static void Lpm_stubBypassI2cPads(void);

/**
 * \brief Bypass IO isolation on all bootmode pads so the BootROM can still detect
 *        the boot device when the MCU resumes from retention.
 */
static void Lpm_stubBypassBootModeIOs(void);

/** \brief Bypass IO isolation on critical pads (WKUP_I2C, bootmode, debug UART).
 *
 *         Bypass IO isolation on the WKUP_I2C pads, so PMIC commands still work.
 *         All bootmode pads, so BootROM can detect boot device on resume.
 *         and optionally the MCU UART pads for debug logging.
 */
static void Lpm_stubBypassCriticalPads(void);

/** \brief Assert MCU domain IO isolation and wait for the hardware confirmation.
 */
static void Lpm_stubTriggerMcuDomainIsolation(void);

#elif defined(SOC_J721S2)

/*
 * On J721S2 the HYPERNET IO daisy-chain carries the wakeup signal to the PMIC.
 * Isolate the HYPERNET IO daisy-chain and enable IO wakeup for them.
 */
static void Lpm_stubEnableHypernetIOIsolation(void);

/** \brief Drive FW_CTRL_OUT0, force MAIN domain deep sleep, assert IO isolation,
 *         and poll PMCTRL_IO_1 until the hardware confirms isolation.
 */
static void Lpm_stubTriggerHypernetIsolation(void);

#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_stubEnterRetention(uint32_t mode)
{
    Lpm_uartDebugPrintf("Lpm_stubEnterRetention: Enter retention with mode = %x\n", mode);

    /* Make sure that nothing remains in cache before going to retention */
    Lpm_stubCleanAllDCache();

    Lpm_ddrEnterSelfRefresh();
    Lpm_uartDebugFullPrintf("Lpm_stubEnterRetention: DDR retention done\n");

    if (mode == LPM_PMIC_IO_ONLY_PLUS_DDR)
    {
        #if defined(SOC_J7200)
        Lpm_stubEnableMainIOIsolation();
        #elif defined(SOC_J721S2)
        Lpm_stubEnableHypernetIOIsolation();
        #elif defined(SOC_J784S4) || defined(SOC_J742S2)
        Lpm_stubEnableMcuIOIsolation();
        #endif

        Lpm_uartDebugPrintf("Lpm_stubEnableMcuIOIsolation: Isolation enabled! \n");
    }

    Lpm_pmicSetupForLpm(mode);
    Lpm_uartDebugPrintf("Lpm_stubEnterRetention: Done! Going to wait now \n");

    while(1)
    {
        /* Wait for PMIC to powerdown the SoC */
    }
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void Lpm_stubCleanAllDCache(void)
{
    uint32_t set, way;

    for(way = 0; way < LPM_STUB_R5F_CACHE_NUM_WAYS ; way++)
    {
        for(set = 0; set < LPM_STUB_R5F_CACHE_NUM_SETS; set++)
        {
            uint32_t val = (set << LPM_STUB_R5F_CP15_CACHE_SET_SHIFT) | (way << LPM_STUB_R5F_CP15_CACHE_WAY_SHIFT);
            asm("    dmb");

            /* DCCSW (CP15 c7/c10 op2=2): Data Cache Clean by Set/Way — writes any dirty
             * lines in this set/way back to main memory without invalidating them.
             */
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 2\t\n": "=r"(val));

            /* DCISW (CP15 c7/c6 op2=2): Data Cache Invalidate by Set/Way.
             * Marks the cache line invalid so the next access fetches fresh data.
             * Combined with the clean above this implements a clean-and-invalidate
             * of every line in the D-cache before entering DDR self-refresh.
             */
            __asm__ __volatile__ ("mcr p15, 0, %0, c7, c6, 2\t\n": "=r"(val));
            asm("    dsb");
        }
    }
}

#if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)

/* Write a magic word to a wake-daisy-chain control register, enable the load,
 * and poll the corresponding status register until the hardware confirms the
 * word has been latched.
 *
 * Used by J7200 CANUART, J784S4 MCU_GEN, and J721S2 HYPERNET wake
 * controller sequences which are structurally identical.
 */
static void Lpm_stubLatchWakeCtrl(uint32_t ctrlReg, uint32_t loadEnableMask, uint32_t statReg)
{
    volatile uint32_t timeout = LPM_STUB_TIMEOUT_MS;
    uint32_t regVal;

    /* Write the magic word to arm the wake-daisy-chain controller */
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, ctrlReg,
                     LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_IO_MAGIC_WORD);

    /* Set load enable to latch the magic word */
    regVal  = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, ctrlReg);
    regVal |= loadEnableMask;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, ctrlReg, regVal);

    /* Poll the status register until the hardware confirms the latch */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, statReg);
    while ((timeout > 0U) && (regVal != LPM_STUB_WAKE_CTRL_STAT_LATCHED))
    {
        timeout--;
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, statReg);
    }
    if (0U == timeout)
    {
        Lpm_uartDebugFullPrintf("FAIL: Timed out waiting for wake-ctrl magic word to latch\n");
        Lpm_stubHangAbort();
    }
}

#endif
#if defined(SOC_J7200)

static void Lpm_stubConfigurePmicWake0Pad(void)
{
    /* Configure H_PMIC_WAKE0 as output so the MCU can drive the
     * PMIC wake line during Main domain IO isolation entry.
     */

    uint32_t regVal;

    /* Unlock WKUP_CTRL_MMR0 partition 6: gives write access to the WKUP domain
     * PADCONFIGxx registers and MCU_GEN_WAKE_CTRL.
     */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION6);

    /* Unlock WKUP_CTRL_MMR0 partition 7: gives write access to the upper range
     * of WKUP domain PADCONFIGxx registers. Unlocked together with partition 6
     * to cover the full set of WKUP IO pad configuration registers.
     */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION7);

    /* Configure PADCONFIG_73 (H_PMIC_WAKE0) with TX_ENABLE and DS_OUTPUT_ENABLE */
    regVal = CSL_REG32_RD_OFF(CSL_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG73);
    regVal &= ~(LPM_CTRL_MMR_CFG0_PADCONFIG_DSOUT_DIS_MASK |
                LPM_CTRL_MMR_CFG0_PADCONFIG_TX_DIS_MASK    |
                LPM_PMIC_WAKE0n_MUXMODE);
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG73, regVal);
}

static void Lpm_stubPrepareMainDomainForIsolation(void)
{
    /* Unlock DMSC PWRCTRL, enable IO isolation bypass, set global wakeup enable,
     * force Main domain IOs into deep sleep, and drive the io_daisy_chain signal
     */

    uint32_t regVal;

    /* Unlock DMSC PWRCTRL registers */
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK0,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK0_VAL);
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK1,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK1_VAL);

    /* Enable Isolation bypass and global wakeup enable flag */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    regVal |= (LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_GLOBAL_WUEN_MASK |
               LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISOBYPASS_MASK);

    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1, regVal);

    /* Force all main domain IOs into deep sleep mode */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL, regVal);

    /* Drive 1 from main_domain io_daisy chain  */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET);
    regVal |= LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET_PMCTRL1;
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET, regVal);
}

static void Lpm_stubToggleWuclk(void)
{
    /* Toggle WUCLK high then low, waiting for hardware acknowledgement at each edge,
     * to clock the io_daisy_chain and confirm the isolation request has propagated
     */

    uint32_t regVal;

    /* Toggle WUCLK for the io_daisy chain */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    regVal |= LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_MASK;
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1, regVal);

    /* Wait for CLK acknowledgement from io-daisy chain */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    while((regVal & LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_STATUS_MASK) == 0U)
    {
        regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                                  LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    }

    /* Clear the WUCLK_CTRL field */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    regVal &= ~(LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_MASK);
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1, regVal);

    /* Wait for CLK acknowledgement from io-daisy chain */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    while((regVal & LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_STATUS_MASK) != 0U)
    {
        regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                                  LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    }
}

static void Lpm_stubEnableMainIOIsolation(void)
{
    volatile uint32_t timeout = LPM_STUB_TIMEOUT_MS;
    uint32_t regVal;

    Lpm_stubConfigurePmicWake0Pad();
    Lpm_stubPrepareMainDomainForIsolation();
    Lpm_stubToggleWuclk();

    /* Enable io_isolation for main domain IOs now */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    regVal |= LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_CTRL_MASK;
    CSL_REG32_WR_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                     LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1, regVal);

    /* Ensure PMIC_WAKE0 is tri-stated, i.e controlled by IO daisy-chain */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL);
    regVal &= ~(LPM_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL_WAKE_EN_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL, regVal);

    /* Latch the CANUART IO daisy-chain wake controller */
    Lpm_stubLatchWakeCtrl(CSL_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL,
                          LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_MASK,
                          CSL_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_STAT1);

    /* Wait for io_isolation status to be set */
    regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                              LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    while ((timeout > 0U) &&
           ((regVal & LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_STATUS_0_MASK) !=
             LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_STATUS_0_MASK))
    {
        timeout--;
        regVal = CSL_REG32_RD_OFF(CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START,
                                  LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1);
    }
    if (0U == timeout)
    {
        Lpm_uartDebugFullPrintf("FAIL: Timedout while waiting for ISOLATION status\n");
        Lpm_stubHangAbort();
    }
}

#elif defined(SOC_J784S4) || defined(SOC_J742S2)

static void Lpm_stubBypassBootModeIOs(void)
{
    uint32_t i;
    uint32_t regVal;

    for (i = 0U; i < LPM_STUB_NUM_BOOTSTRAP_IOS; i++)
    {
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, gLpmStubBootstrapPadOffsets[i]);
        regVal |= LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK;
        CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, gLpmStubBootstrapPadOffsets[i], regVal);
    }
}

static void Lpm_stubBypassI2cPads(void)
{
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG64,
                     (LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK |
                      LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_MASK));
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG65,
                     (LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK |
                      LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_MASK));
}

static void Lpm_stubBypassCriticalPads(void)
{
    Lpm_stubBypassI2cPads();

    /* Bypass IO Isolation for all BOOTMODE pins as they are part of MCU_GENERAL IOs
     * this is necessary as they are used by the BootROM to detect the boot device during
     * boot/resume.
     */
    Lpm_stubBypassBootModeIOs();

    /* Bypass IO Isolation for MCU uart to enable debug logs in IO_DDR mode entry */
#ifdef LPM_DEBUG
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE,
                     CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG60,
                     LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE,
                     CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG61,
                     LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE,
                     CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG62,
                     LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE,
                     CSL_WKUP_CTRL_MMR_CFG0_PADCONFIG63,
                     LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK);
#endif
}

static void Lpm_stubTriggerMcuDomainIsolation(void)
{
    /* Drive the MCU domain io_daisy_chain signal, enable deep sleep and global
     * wakeup for the MCU/WKUP domain, assert IO isolation, and wait for confirmation
     * from the daisy chain.
     */

    volatile uint32_t timeout = LPM_STUB_TIMEOUT_MS;
    uint32_t regVal;

    /* Drive 1 from mcu_domain io_daisy chain  */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_0_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET, regVal);

    /* Global DeepSleep enable only for MCU domain */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_WKUP_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL, regVal);

    /* Set global wuen only for WKUP domain */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0, regVal);

    /* Set global isoin for WKUP domain */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0, regVal);

    /* Wait for io_iso status to be 1 */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0);
    while ((timeout > 0U) &&
           ((regVal & LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_MASK) !=
            LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_MASK))
    {
        timeout--;
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0);
    }
    if (0U == timeout) {
        Lpm_uartDebugFullPrintf("FAIL: Timedout while waiting for ISOLATION status\n");
        Lpm_stubHangAbort();
    }
}

static void Lpm_stubEnableMcuIOIsolation(void)
{
    uint32_t regVal;

    /* Unlock the partition 7 to get access to WKUP_PADCONFIGS */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION7);

    /* UNLOCK partition 5 for PMCTRL_IO, FW_CTRL_OUT etc */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION5);

    /* Global IO_ISOBYPASS signal */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0, regVal);

    Lpm_stubBypassCriticalPads();
    Lpm_stubTriggerMcuDomainIsolation();

    /* Unlock partition 6 for MCU_GEN_WAKE_CTRL then latch the wake controller */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION6);
    Lpm_stubLatchWakeCtrl(CSL_WKUP_CTRL_MMR_CFG0_MCU_GEN_WAKE_CTRL,
                          LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_MASK,
                          CSL_WKUP_CTRL_MMR_CFG0_MCU_GEN_WAKE_STAT1);
}

#elif defined(SOC_J721S2)

static void Lpm_stubTriggerHypernetIsolation(void)
{
    volatile uint32_t timeout = LPM_STUB_TIMEOUT_MS;
    uint32_t regVal;

    /* Drive IO_PM_CTRL[1] from FW_CTRL_OUT0 to signal IO isolation to the MAIN domain */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_1_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET, regVal);

    /* Force MAIN domain IOs into deep sleep mode */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL, regVal);

    /* Enable global wakeup on PMCTRL_IO_1 */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1, regVal);

    /* Assert IO isolation on PMCTRL_IO_1 */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1, regVal);

    /* Poll IO_ISO_STATUS_0 until isolation is confirmed */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1);
    while ((timeout > 0U) &&
           ((regVal & LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_MASK) !=
            LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_MASK))
    {
        timeout--;
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1);
    }
    if (0U == timeout)
    {
        Lpm_uartDebugFullPrintf("FAIL: Timedout while waiting for ISOLATION status\n");
        Lpm_stubHangAbort();
    }
}

static void Lpm_stubEnableHypernetIOIsolation(void)
{
    uint32_t regVal;

    /* Unlock WKUP_CTRL_MMR0 partition 6 for PADCONFIGxx */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION6);

    /* Unlock WKUP_CTRL_MMR0 partition 5 for PMCTRL_IO, FW_CTRL_OUT etc */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION5);

    /* Set IO_ISOBYPASS on PMCTRL_IO_1 (Main domain IOs) */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1);
    regVal |= LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_1, regVal);

    Lpm_stubTriggerHypernetIsolation();

    /* Unlock partition 6 for HYPERNET_WAKE_CTRL then latch the wake controller */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION6);
    Lpm_stubLatchWakeCtrl(CSL_WKUP_CTRL_MMR_CFG0_HYPERNET_WAKE_CTRL,
                          LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_MASK,
                          CSL_WKUP_CTRL_MMR_CFG0_HYPERNET_WAKE_STAT1);
}
#endif
