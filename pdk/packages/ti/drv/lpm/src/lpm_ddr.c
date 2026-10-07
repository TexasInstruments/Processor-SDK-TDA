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
 * \file lpm_ddr.c
 *
 * \brief DDR self-refresh functions used in lpm_stub during low power entry.
 * Puts DDR into self-refresh and manages frequency switching before retention.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/soc.h>
#if defined(SOC_J7200)
#include <ti/csl/soc/j7200/src/cslr_wkup_ctrl_mmr.h>
#include <ti/csl/soc/j7200/src/cslr_main_pll_mmr.h>
#include <ti/board/src/j7200_evm/include/board_pll.h>
#include <ti/board/src/j7200_evm/include/board_ddr.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_wkup_ctrl_mmr.h>
#include <ti/csl/soc/j784s4/src/cslr_main_pll_mmr.h>
#include <ti/board/src/j784s4_evm/include/board_pll.h>
#include <ti/board/src/j784s4_evm/include/board_ddr.h>
#elif defined(SOC_J742S2)
#include <ti/csl/soc/j784s4/src/cslr_wkup_ctrl_mmr.h>
#include <ti/csl/soc/j784s4/src/cslr_main_pll_mmr.h>
#include <ti/board/src/j742s2_evm/include/board_pll.h>
#include <ti/board/src/j742s2_evm/include/board_ddr.h>
#elif defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_wkup_ctrl_mmr.h>
#include <ti/board/src/j721s2_evm/include/board_pll.h>
#include <ti/board/src/j721s2_evm/include/board_ddr.h>
#endif
#include <ti/drv/lpm/src/lpm_ddr.h>
#include <ti/drv/lpm/src/lpm_uart_baremetal.h>
#include <ti/drv/lpm/src/lpm_stub_utils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* DDR low power interface Registers */
#define LPM_DDR_LPI_THRESH_REGS_COUNT           (0x7U)

#define LPM_DDR_LPI_SHIFT                       (0x8U)
#define LPM_DDR_LPI_THRESH_MASK                 (0xFU)

#define LPM_DDR_DENALI_CTL_139_LAST_THRESH      (16U)
#define LPM_DDR_DENALI_CTL_LAST_THRESH          (24U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

struct Lpm_DdrInstanceConfig {
    uint32_t ddrBase;
    /**< [IN] Base address of the DDR controller */
    uint32_t freqChngReq;
    /**< [IN] Address of the FSP change request register */
    uint32_t freqChngAck;
    /**< [IN] Address of the FSP change request acknowledge register */
    uint32_t clkChngReq;
    /**< [IN] Address of the CLK change request register */
    uint32_t clkChngAck;
    /**< [IN] Address of the CLK change request acknowledge register */
    uint32_t pllCtrl;
    /**< [IN] Address of the PLL control register for the DDR instance */
    uint32_t pllHsdiv0;
    /**< [IN] Address of the HSDIV control register for the DDR instance */
    uint32_t pllFreq[3];
    /**< [IN] Set of PLL frequencies for the DDR instance */
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined(SOC_J784S4)
static const struct Lpm_DdrInstanceConfig gLpmDdr[] =
{
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS0_DDRSS0_CTLCFG_VBUSP_DDRSS0_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ0,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK0,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ0,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK0,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS1_DDRSS1_CTLCFG_VBUSP_DDRSS1_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ1,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK1,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ1,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK1,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS2_DDRSS2_CTLCFG_VBUSP_DDRSS2_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ2,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK2,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ2,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK2,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL27_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL27_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS3_DDRSS3_CTLCFG_VBUSP_DDRSS3_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ3,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK3,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ3,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK3,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL28_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL28_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    { },
};

#elif defined (SOC_J742S2)
static const struct Lpm_DdrInstanceConfig gLpmDdr[] =
{
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS0_DDRSS0_CTLCFG_VBUSP_DDRSS0_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ0,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK0,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ0,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK0,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS1_DDRSS1_CTLCFG_VBUSP_DDRSS1_CTLCFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ1,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK1,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ1,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK1,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    { },
};
#elif defined(SOC_J7200)
static const struct Lpm_DdrInstanceConfig gLpmDdr[] =
{
    {
        .ddrBase = CSL_STD_FW_COMPUTE_CLUSTER0__VBUSP_DDRSS0_DDRSS0_CTLCFG_CTL_CFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK,
        .pllCtrl = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_CTRL,
        .pllHsdiv0 = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0,
        .pllFreq = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_800_MHZ, LPM_DDR_FSP_800_MHZ }
    },
    { },
};
#elif defined(SOC_J721S2)
static const struct Lpm_DdrInstanceConfig gLpmDdr[] =
{
    {
        .ddrBase    = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS0_DDRSS0_CTLCFG_DDR0_0_CTL_CFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ0,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK0,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ0,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK0,
        .pllCtrl    = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_CTRL,
        .pllHsdiv0  = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0,
        .pllFreq    = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    {
        .ddrBase    = CSL_STD_FW_COMPUTE_CLUSTER0_CFG_WRAP_0__VBUSP_DDRSS1_DDRSS1_CTLCFG_DDR1_1_CTL_CFG_START,
        .freqChngReq = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQ1,
        .freqChngAck = CSL_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACK1,
        .clkChngReq = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQ1,
        .clkChngAck = CSL_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACK1,
        .pllCtrl    = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_CTRL,
        .pllHsdiv0  = CSL_PLL0_CFG_BASE + CSL_MAIN_PLL_MMR_CFG_PLL26_HSDIV_CTRL0,
        .pllFreq    = { LPM_DDR_FSP_19_2_MHZ, LPM_DDR_FSP_1066_5_MHZ, LPM_DDR_FSP_1066_5_MHZ }
    },
    { },
};

#else
#error "Unsupported SoC for LPM"
#endif

/*
 * ERRATA i2166: LPI wakeup threshold register offsets used to compute the
 * maximum threshold value that must be programmed into PHY_LP_WAKEUP.
 *
 * The Denali controller packs four independent 4-bit LPI wakeup threshold
 * fields into each register at bit positions [27:24], [19:16], [11:8], [3:0].
 * Registers CTL_133 through CTL_139 collectively cover all LPI modes
 * (SR, SRPD, PD, CTRL_IDLE, TIMER, MCCLK_GATE) across all three frequency
 * set points (F0, F1, F2).
 */
static const uint32_t gLpmLpiThreshRegs[LPM_DDR_LPI_THRESH_REGS_COUNT] = {
    CSL_EMIF_CTLCFG_DENALI_CTL_133,
    CSL_EMIF_CTLCFG_DENALI_CTL_134,
    CSL_EMIF_CTLCFG_DENALI_CTL_135,
    CSL_EMIF_CTLCFG_DENALI_CTL_136,
    CSL_EMIF_CTLCFG_DENALI_CTL_137,
    CSL_EMIF_CTLCFG_DENALI_CTL_138,
    CSL_EMIF_CTLCFG_DENALI_CTL_139,
};

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 * \brief Configure PLL12 (DDR PLL) HSDIV0 for the DDR frequency set point (FSP) at
 *        \a index in the instance's pllFreq table. For low frequencies the
 *        PLL is put into bypass; for higher frequencies the HSDIV divider is
 *        set to the appropriate ratio to derive the target data rate.
 */
static void Lpm_ddrConfigurePll(const struct Lpm_DdrInstanceConfig *ddrInstance, uint32_t index);

/**
 * \brief Spin until the field at bit \a shift of the MAIN_CTRL_MMR register
 *        at \a address reads back the value \a setval.  Used to poll
 *        handshake bits during FSP frequency-change sequences.
 */
static void Lpm_ddrWaitForSet(uint32_t address, uint32_t setval, uint32_t shift);

/**
 * \brief Write \a setval to the field described by \a mask and \a shift in
 *        the MAIN_CTRL_MMR register at \a address, then call Lpm_ddrWaitForSet
 *        to confirm the hardware has latched the value before returning.
 */
static void Lpm_ddrVerifiedSet(uint32_t address, uint32_t setval, uint32_t mask, uint32_t shift);

/**
 * \brief Drive the full DDR4 FSP (Frequency Set Point) change handshake for
 *        one DDR instance. Assert the change request, wait for the
 *        controller to request a PLL switch, reconfigure PLL12 (DDR PLL), acknowledge
 *        the clock change, wait for the controller's completion ACK, then
 *        de-asserts the request.
 */
static void Lpm_ddrInitiateFspFreqChange(const struct Lpm_DdrInstanceConfig *ddrInstance, const uint32_t fsp);

/**
 * \brief Write the unlock sequence to PLL12 (DDR PLL) LOCKKEY0/LOCKKEY1 to allow
 *        subsequent writes to PLL12 (DDR PLL) control and HSDIV registers.
 */
static void Lpm_ddrUnlockPll(void);

/**
 * \brief Workaround for ERRATA i2166: program PHY_LP_WAKEUP
 *        (DENALI_PHY_1318[15:8]) on one DDR controller to a value strictly
 *        greater than every LPI wakeup threshold across all three frequency
 *        set points (F0/F1/F2), preventing the PHY from entering the Deep
 *        Sleep low-power state.
 *
 * \param ddr  Pointer to the DDR instance configuration.
 */
static void Lpm_ddrApplyLpiWakeupErrata(const struct Lpm_DdrInstanceConfig *ddr);

/**
 * \brief Switch one DDR controller to FSP0 in preparation for self-refresh entry.
 *
 * FSP (Frequency Set Point) is a pre-trained DDR operating frequency.
 * FSP0 is a low-frequency bypass mode used only during retention entry/exit.
 * This function:
 *  1. Reads the current FSP from the PI and writes it to the CTL so both
 *     agree on the active frequency before the switch.
 *
 *  2. Transfers mode register (MR) write ownership to the PHY/PI, preventing
 *     the CTL from issuing conflicting MR writes during the frequency change.
 *
 *  3. Disables periodic WDQLVL (Write Data-eye Leveling) training for FSP0.
 *     WDQLVL re-calibrates write DQ timing and is only needed for the normal
 *     Operating frequencies (FSP1/FSP2), not for the brief retention bypass.
 *
 *  4. Marks which FSPs have trained data so the controller knows which
 *     frequencies are valid to switch back to on wakeup.
 *
 *  5. Triggers the FSP frequency-change handshake to move to FSP0.
 *
 * \param ddr  Pointer to the DDR instance configuration.
 */
static void Lpm_ddrSwitchToFsp0(const struct Lpm_DdrInstanceConfig *ddr);

/**
 * \brief Command one DDR controller into SRPD Long self-refresh mode and verify
 *        that both chip-selects report the expected LP state before returning.
 *
 * \param ddr   Pointer to the DDR instance configuration.
 */
static void Lpm_ddrEnterSelfRefreshOnController(const struct Lpm_DdrInstanceConfig *ddr);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_ddrEnterSelfRefresh(void)
{
    /* Unlock MCU_CTRL_MMR0 partition 0:
     * Gives write access to MCU domain peripheral control registers
     * used during the DDR self-refresh entry sequence.
     */
    Lpm_stubCtrlMmrUnlock(CSL_MCU_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION0);

    /* Unlock WKUP_CTRL_MMR0 partition 2:
     * Gives write access to PLL clock-source select registers, including
     * MAIN_PLL12_CLKSEL — the input mux for PLL12 (the DDR PLL).
     * This is required by Lpm_ddrConfigurePll when it switches PLL12 (DDR PLL)
     * between bypass mode (FSP0 low-frequency) and active PLL mode (FSP1/FSP2).
     */
    Lpm_stubCtrlMmrUnlock(CSL_WKUP_CTRL_MMR0_CFG0_BASE, BOARD_MMR_PARTITION2);

    /* Unlock DDR PLL CTRL and HSDIV registers directly via the PLL MMR */
    Lpm_ddrUnlockPll();

    /* ERRATA i2166: prevent the DDR PHY from entering Deep Sleep so it cannot
     * exit Deep Sleep before the PHY PLL is disabled, which would otherwise
     * misalign internal clocks and cause timing failures.
     */
    for (uint32_t i = 0U; gLpmDdr[i].ddrBase > 0U; i++)
    {
        Lpm_ddrApplyLpiWakeupErrata(&gLpmDdr[i]);
    }

    /* Switch each DDR controller to FSP0 (low-frequency mode) before self-refresh. */
    for (uint32_t i = 0U; gLpmDdr[i].ddrBase > 0U; i++)
    {
        Lpm_ddrSwitchToFsp0(&gLpmDdr[i]);
    }

    /* Put each DDR controller into self-refresh to preserve DRAM contents during suspend. */
    for (uint32_t i = 0U; gLpmDdr[i].ddrBase > 0U; i++)
    {
        Lpm_ddrEnterSelfRefreshOnController(&gLpmDdr[i]);
    }
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/*
 * CTL_133..CTL_138: four 4-bit threshold fields per register at bits
 * 27:24, 19:16, 11:8, and 3:0. CTL_139 has only 3 threshold fields, at
 * 19:16, 11:8, and 3:0.
 *
 * CTL Register Map:
 * ┌─────────────────┬──────────────────────────────────────────────┐
 * │ Register        │ Fields (bits)                                │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_133         │ [27:24] LPI_SR_LONG_MCCLK_GATE_WAKEUP_F0     │
 * │                 │ [19:16] LPI_SR_LONG_WAKEUP_F0                │
 * │                 │ [11:8]  LPI_SR_SHORT_WAKEUP_F0               │
 * │                 │ [3:0]   LPI_CTRL_IDLE_WAKEUP_F0              │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_134         │ [27:24] LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F0   │
 * │                 │ [19:16] LPI_SRPD_LONG_WAKEUP_F0              │
 * │                 │ [11:8]  LPI_SRPD_SHORT_WAKEUP_F0             │
 * │                 │ [3:0]   LPI_PD_WAKEUP_F0                     │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_135         │ [27:24] LPI_SR_LONG_WAKEUP_F1                │
 * │                 │ [19:16] LPI_SR_SHORT_WAKEUP_F1               │
 * │                 │ [11:8]  LPI_CTRL_IDLE_WAKEUP_F1              │
 * │                 │ [3:0]   LPI_TIMER_WAKEUP_F0                  │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_136         │ [27:24] LPI_SRPD_LONG_WAKEUP_F1              │
 * │                 │ [19:16] LPI_SRPD_SHORT_WAKEUP_F1             │
 * │                 │ [11:8]  LPI_PD_WAKEUP_F1                     │
 * │                 │ [3:0]   LPI_SR_LONG_MCCLK_GATE_WAKEUP_F1     │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_137         │ [27:24] LPI_SR_SHORT_WAKEUP_F2               │
 * │                 │ [19:16] LPI_CTRL_IDLE_WAKEUP_F2              │
 * │                 │ [11:8]  LPI_TIMER_WAKEUP_F1                  │
 * │                 │ [3:0]   LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F1   │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_138         │ [27:24] LPI_SRPD_SHORT_WAKEUP_F2             │
 * │                 │ [19:16] LPI_PD_WAKEUP_F2                     │
 * │                 │ [11:8]  LPI_SR_LONG_MCCLK_GATE_WAKEUP_F2     │
 * │                 │ [3:0]   LPI_SR_LONG_WAKEUP_F2                │
 * ├─────────────────┼──────────────────────────────────────────────┤
 * │ CTL_139         │ [29:24] LPI_WAKEUP_EN                        │
 * │                 │ [19:16] LPI_TIMER_WAKEUP_F2                  │
 * │                 │ [11:8]  LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_F2   │
 * │                 │ [3:0]   LPI_SRPD_LONG_WAKEUP_F2              │
 * └─────────────────┴──────────────────────────────────────────────┘
 */
static void Lpm_ddrApplyLpiWakeupErrata(const struct Lpm_DdrInstanceConfig *ddr)
{
    uint32_t max_thresh = 0U;
    uint32_t reg_idx, shift, regval, field, last_threshold;

    for (reg_idx = 0U; reg_idx < LPM_DDR_LPI_THRESH_REGS_COUNT; reg_idx++)
    {
        regval = CSL_REG32_RD_OFF(ddr->ddrBase, gLpmLpiThreshRegs[reg_idx]);

        /* CTL_139 only has three thresholds; at 19:16, 11:8, and 3:0 respectively */
        if (gLpmLpiThreshRegs[reg_idx] == CSL_EMIF_CTLCFG_DENALI_CTL_139)
        {
            last_threshold = LPM_DDR_DENALI_CTL_139_LAST_THRESH;
        }
        else
        {
            last_threshold = LPM_DDR_DENALI_CTL_LAST_THRESH;
        }

        for (shift = 0U; shift <= last_threshold; shift += LPM_DDR_LPI_SHIFT)
        {
            field = (regval >> shift) & LPM_DDR_LPI_THRESH_MASK;
            if (field > max_thresh)
            {
                max_thresh = field;
            }
        }
    }

    /* PHY_LP_WAKEUP must be strictly greater than max_thresh */
    regval = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PHY_1318);
    regval = (regval & ~(LPM_DDR_LPI_THRESH_MASK << LPM_DDR_LPI_SHIFT)) | ((max_thresh + 1U) << LPM_DDR_LPI_SHIFT);
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PHY_1318, regval);
}

static void Lpm_ddrSwitchToFsp0(const struct Lpm_DdrInstanceConfig *ddr)
{
    uint32_t fspop, fspwr;
    uint32_t regVal;

    Lpm_uartDebugFullPrintf("gLpmDdr[i].ddrBase = %x\n", ddr->ddrBase);

    /* Copy the DRAM's current FSP state from the PI into the CTL so both agree on which
     * frequency mode the DRAM is in before we start switching to FSP0.
     *
     * The PI programmed the DDR's MR13 mode register during training; the
     * CTL was not automatically informed of this, so we read the FSP_OP
     * (operating FSP) and FSP_WR (write FSP) fields here and write them
     * into the CTL's FSP_OP_CURRENT and FSP_WR_CURRENT fields.
     */
    fspop = (CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_259) & LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_OP_MASK) >>
             LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_OP_SHIFT;
    fspwr = (CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_259) & LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_WR_MASK) >>
             LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_WR_SHIFT;

    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_192);
    regVal |= ((fspwr << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_WR_CURRENT_SHIFT) |
               (fspop << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_OP_CURRENT_SHIFT));
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_192, regVal);

    /* Do not allow CTL to update mode registers MR12 and MR14, only PHY or PI should be allowed */
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_191);
    regVal |= LPM_DDR_EMIF_CTLCFG_DENALI_CTL_191_FSP_PHY_UPDATE_MRW_MASK;
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_191, regVal);

    /* Now, disable the PI from setting the VREF value and updating the MR12 and MR14 registers
     * after DFS is issued by the memory controller.
     * After this point only the PHY is allowed to update the MR13 and MR14 registers.
     */
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_64);
    regVal &= ~LPM_DDR_EMIF_CTLCFG_DENALI_PI_64_PI_MC_DFS_PI_SET_VREF_ENABLE_MASK;
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_64, regVal);

    /* Disable periodic WDQLVL (Write Data-eye Leveling) training for FSP0.
     * WDQLVL re-calibrates write DQ timing margins and is only required for
     * the DDR's normal operating frequencies (FSP1/FSP2).
     *
     * FSP0 is used solely during retention entry/exit, so re-calibration is unnecessary.
     */
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_212);
    regVal &= ~(LPM_DDR_EMIF_CTLCFG_DENALI_PI_212_PI_DISABLE_WDQLVL_INITIALIZATION_SUPPORT <<
                LPM_DDR_EMIF_CTLCFG_DENALI_PI_212_PI_WDQLVL_EN_F0_SHIFT);
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_212, regVal);

    /* PI_FREQ_MAP defines supported working frequencies */
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_PI_12);
    if (regVal & LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP1_MASK)
    {
        /* Define FSP0 and FSP1 as trained */
        regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_190);
        regVal |= (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F1_MASK |
                   LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F0_MASK);
        CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_190, regVal);
    }
    if (regVal & LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP2_MASK)
    {
        /* Define FSP0 and FSP2 as trained */
        regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_190);
        regVal |= (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F2_MASK |
                   LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F0_MASK);
        CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_190, regVal);
    }

    Lpm_ddrInitiateFspFreqChange(ddr, LPM_DDR_FSP0);
}

static void Lpm_ddrEnterSelfRefreshOnController(const struct Lpm_DdrInstanceConfig *ddr)
{
    uint32_t regVal;
    volatile uint32_t timeout;

    const uint32_t lpStateMask = (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CS0_MASK << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CSx_SHIFT) |
                                  LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CS1_MASK;

    /* Expected LP state for one chip-select: valid bit set + SRPD_LONG state */
    const uint32_t lpStatePerCS  = (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_MASK <<
                                    LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_CS0_SHIFT) |
                                    LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CSx_SRPD_LONG_WITH_MEM_AND_CTL_CLK_GATING_MASK;

    /* Both CS0 and CS1 must report the same expected LP state */
    const uint32_t lpStateExpected = (lpStatePerCS << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_CS1_SHIFT) |
                                      lpStatePerCS;

    /* Disable auto entry / exit for all low power states when a read/write command enters the command queue */
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_141);
    regVal &= ~((LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_ENTRY_EXIT_EN_MASK << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_EXIT_EN_SHIFT) |
                (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_ENTRY_EXIT_EN_MASK << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_ENTRY_EN_SHIFT));
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_141, regVal);

    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_132) &
                             ~(LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_MASK << LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_SHIFT);

    /* Set low power mode to enter self refresh power down (SRPD) with memory and controller clock gating */
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_132,
                     (regVal | (LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_SRPD_LONG_WITH_MEM_AND_CTL_CLK_GATING_MASK <<
                                LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_SHIFT)));

    timeout = LPM_STUB_TIMEOUT_MS;
    while((timeout > 0U) &&
          ((CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_293) &
           LPM_DDR_EMIF_CTLCFG_DENALI_CTL_293_INT_STATUS_LP_CMD_COMPLETE_MASK) == 0))
    {
        timeout--;
    }
    if (0U == timeout)
    {
        Lpm_uartDebugFullPrintf("FAIL: Timed out waiting for DDR LP cmd complete\n");
        Lpm_stubHangAbort();
    }

    /* Clear the lp_cmd complete interrupt by writing to INT_ACK field in DENALI_CTL_295[10] */
    CSL_REG32_WR_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_295,
                     LPM_DDR_EMIF_CTLCFG_DENALI_CTL_295_INT_ACK_LP_CMD_COMPLETE_MASK);

    /* Poll until LP_STATE confirms both chip-selects have entered SRPD Long
     * (Self-Refresh Power Down) with memory and controller clocks gated.
     * This is the final hardware confirmation that the DDR controller has
     * safely reached the retention state before the MCU yields execution.
     */
    timeout = LPM_STUB_TIMEOUT_MS;
    regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_141);
    regVal &= lpStateMask;
    while((timeout > 0U) && (regVal != lpStateExpected))
    {
        timeout--;
        regVal = CSL_REG32_RD_OFF(ddr->ddrBase, CSL_EMIF_CTLCFG_DENALI_CTL_141);
        regVal &= lpStateMask;
    }
    if (0U == timeout)
    {
        Lpm_uartDebugFullPrintf("FAIL: Timed out waiting for DDR SRPD Long state\n");
        Lpm_stubHangAbort();
    }
}

static void Lpm_ddrConfigurePll(const struct Lpm_DdrInstanceConfig *ddrInstance, uint32_t index)
{
    uint32_t pll_freq = ddrInstance->pllFreq[index];
    uint32_t val, val1;

    if (pll_freq <= LPM_DDR_PLL_FREQ_27_5_MHZ)
    {
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) | LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK);
    }
    else if (pll_freq <= LPM_DDR_PLL_FREQ_133_MHZ)
    {
        /* Compute the integer division ratio: 2133 MHz / target_freq.
         * The HSDIV field encodes (divisor - 1) — hardware adds 1 before dividing
         * the VCO output, so subtract 1 before writing.
         */
        val1 = LPM_DDR_PLL_FREQ_2133_MHZ / pll_freq;
        val = CSL_REG32_RD(ddrInstance->pllHsdiv0) & ~CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_MASK;
        CSL_REG32_WR(ddrInstance->pllHsdiv0, val | (val1 - 1U));
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) & ~(LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK));
    }
    else if (pll_freq <= LPM_DDR_PLL_FREQ_266_5_MHZ)
    {
        val = CSL_REG32_RD(ddrInstance->pllHsdiv0) & ~CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_MASK;
        CSL_REG32_WR(ddrInstance->pllHsdiv0, val | LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY8);
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) & ~(LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK));
    }
    else if (pll_freq <= LPM_DDR_PLL_FREQ_533_MHZ)
    {
        val = CSL_REG32_RD(ddrInstance->pllHsdiv0) & ~CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_MASK;
        CSL_REG32_WR(ddrInstance->pllHsdiv0, val | LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY4);
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) & ~(LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK));
    }
    else if (pll_freq <= LPM_DDR_FSP_800_MHZ)
    {
        val = CSL_REG32_RD(ddrInstance->pllHsdiv0) & ~CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_MASK;
        CSL_REG32_WR(ddrInstance->pllHsdiv0, val | LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY4);
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) & ~(LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK));
    }
    else if (pll_freq <= LPM_DDR_FSP_1066_5_MHZ)
    {
        val = CSL_REG32_RD(ddrInstance->pllHsdiv0) & ~CSL_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_MASK;
        CSL_REG32_WR(ddrInstance->pllHsdiv0, val | LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY2);
        CSL_REG32_WR(ddrInstance->pllCtrl, CSL_REG32_RD(ddrInstance->pllCtrl) & ~(LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK));
    }
    else
    {
        Lpm_uartDebugFullPrintf("Unexpected frequency for DDR: 0x%x\n", pll_freq);
    }
}

static void Lpm_ddrWaitForSet(uint32_t address, uint32_t setval, uint32_t shift)
{
    uint32_t val;

    do
    {
        val = CSL_REG32_RD_OFF(CSL_CTRL_MMR0_CFG0_BASE, address);
    }
    while((val & (setval << shift)) != (setval << shift));
}

static void Lpm_ddrVerifiedSet(uint32_t address, uint32_t  setval, uint32_t  mask,
                            uint32_t  shift)
{
    uint32_t val;

    val = CSL_REG32_RD_OFF(CSL_CTRL_MMR0_CFG0_BASE, address) & ~(mask << shift);
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, address, val | (setval << shift));

    Lpm_ddrWaitForSet(address, setval, shift);
}

static void Lpm_ddrInitiateFspFreqChange(const struct Lpm_DdrInstanceConfig *ddrInstance, const uint32_t fsp)
{
    volatile uint32_t timeout;
    uint32_t regVal;

    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_LOCK5_KICK0, KICK0_UNLOCK);
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_LOCK5_KICK1, KICK1_UNLOCK);

    /* Set REQ_TYPE to fsp0 */
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->freqChngReq, fsp);
    for(timeout = 0; timeout < LPM_STUB_TIMEOUT_MS; timeout++)
    {
        /* Wait for the frequency change to take affect */
    }

    /*
     * Assert req: Initiate FSP frequency change.
     * DDR will respond as if it was its idea to change the frequency
     */
    regVal = CSL_REG32_RD_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->freqChngReq);
    regVal |= LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_MASK;
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->freqChngReq, regVal);

    /* Wait for DDR to assert a CLKCHNG_REQ */
    Lpm_ddrWaitForSet(ddrInstance->clkChngReq, LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_EN,
                      LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_SHIFT);

    /* Set the DDR PLL appropriately for the chosen FSP */
    Lpm_ddrConfigurePll(ddrInstance, fsp);

    /* Acknowledge the DDR PLL clock frequency change to the DDR Controller */
    Lpm_ddrVerifiedSet(ddrInstance->clkChngAck, LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_EN,
                       LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_MASK,
                       LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_SHIFT);

    /* Wait for the indication from the DDR Controller that the FSP change operation is complete */
    Lpm_ddrWaitForSet(ddrInstance->freqChngAck, LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_EN,
                      LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_SHIFT);

    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->clkChngAck,
                     LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_SHIFT);

    /* De-assert FSP change request */
    regVal = CSL_REG32_RD_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->freqChngReq);
    regVal &= ~(LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_MASK |
                LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_TYPE_MASK);
    CSL_REG32_WR_OFF(CSL_CTRL_MMR0_CFG0_BASE, ddrInstance->freqChngReq, regVal);
}

static void Lpm_ddrUnlockPll(void)
{
    CSL_REG32_WR_OFF(CSL_PLL0_CFG_BASE, CSL_MAIN_PLL_MMR_CFG_PLL12_LOCKKEY0, KICK0_UNLOCK);
    CSL_REG32_WR_OFF(CSL_PLL0_CFG_BASE, CSL_MAIN_PLL_MMR_CFG_PLL12_LOCKKEY1, KICK1_UNLOCK);
}
