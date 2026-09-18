/**
 *  \file   sbl_slave_core_boot.c
 *
 *  \brief  This file contain functions related to slave core boot-up.
 *
 */

/*
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

 #include <stdint.h>
 #include <string.h>
 #include <ti/csl/csl_types.h>
 #include <ti/csl/cslr_device.h>
 #include <ti/csl/hw_types.h>
 #include <ti/csl/arch/csl_arch.h>
 #include <ti/drv/uart/UART_stdio.h>

#include "sbl_soc.h"
#include "sbl_log.h"
#include "sbl_soc_cfg.h"
#include "sbl_profile.h"
#include "sbl_err_trap.h"
#include "sbl_sci_client.h"
#include "sbl_slave_core_boot.h"

#if defined(BOOT_OSPI)
#include "sbl_ospi.h"
#endif

#if defined(BOOT_MMCSD)
#include "sbl_mmcsd.h"
#endif

#if defined(BOOT_EMMC_UDA) || defined(BOOT_EMMC_BOOT0)
#include "sbl_emmc.h"
#endif

#if defined(BOOT_UART)
#include "sbl_uart.h"
#endif

#if defined(BOOT_HYPERFLASH)
#include "sbl_hyperflash.h"
#endif
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define SBL_DISABLE_MCU_LOCKSTEP         (0U)
#define SBL_ENABLE_MCU_LOCKSTEP          (1U)
#define SBL_BIT_MASK_128_BYTE_ALIGNED    (0xFFFFFF80U)
#define SBL_BIT_MASK_1KB_ALIGNED         (0xFFFFFC00U)
#define SBL_BIT_MASK_2MB_ALIGNED         (0xFFE00000U)

int32_t c7xCoreID[]={DSP1_C7X_ID,DSP2_C7X_ID,DSP3_C7X_ID,DSP4_C7X_ID};

/* List of device that shares DSP power domain*/
int32_t c7xLpscSharedDevid[4][2] =
{
    {
#if defined(SOC_J7200)
        SBL_INVALID_ID,
        SBL_INVALID_ID
#elif defined(SOC_J721S2) || defined(SOC_J721E)
        TISCI_DEV_RTI16,
        SBL_INVALID_ID
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        TISCI_DEV_RTI16,
        TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_CORE0
#endif
    },
    {
#if defined(SOC_J7200)
        SBL_INVALID_ID,
        SBL_INVALID_ID
#elif defined(SOC_J721S2) || defined(SOC_J721E)
        TISCI_DEV_RTI16,
        SBL_INVALID_ID
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        TISCI_DEV_RTI17,
        TISCI_DEV_COMPUTE_CLUSTER0_C71SS1_CORE0
#endif
    },
    {
#if defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J721E)
        SBL_INVALID_ID,
        SBL_INVALID_ID
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        TISCI_DEV_RTI18,
        TISCI_DEV_COMPUTE_CLUSTER0_C71SS2_CORE0
#endif
    },
    {
#if defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J721E)
        SBL_INVALID_ID,
        SBL_INVALID_ID
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        TISCI_DEV_RTI19,
        TISCI_DEV_COMPUTE_CLUSTER0_C71SS3_CORE0
#endif
    }
};

/* Don't forget to update parameter OPP of the AVS   */
/* setup function in SBL_SocLateInit if the CPU freq */
/* are changed to a higher or lower operating  point */
static const sblSlaveCoreInfo_t sbl_slave_core_info[] =
{
    /* MPU1_CPU0 info */
    {
    SBL_PROC_ID_MPU1_CPU0,
    SBL_DEV_ID_MPU1_CPU0,
    SBL_CLK_ID_MPU1_CPU0,
    SBL_MPU1_CPU0_FREQ_HZ,
    },
    /* MPU1_CPU1 info */
    {
    SBL_PROC_ID_MPU1_CPU1,
    SBL_DEV_ID_MPU1_CPU1,
    SBL_CLK_ID_MPU1_CPU1,
    SBL_MPU1_CPU1_FREQ_HZ,
    },
    /* MPU1_CPU2 info */
    {
    SBL_PROC_ID_MPU1_CPU2,
    SBL_DEV_ID_MPU1_CPU2,
    SBL_CLK_ID_MPU1_CPU2,
    SBL_MPU1_CPU2_FREQ_HZ,
    },
    /* MPU1_CPU3 info */
    {
    SBL_PROC_ID_MPU1_CPU3,
    SBL_DEV_ID_MPU1_CPU3,
    SBL_CLK_ID_MPU1_CPU3,
    SBL_MPU1_CPU3_FREQ_HZ,
    },
    /* MPU2_CPU0 info */
    {
    SBL_PROC_ID_MPU2_CPU0,
    SBL_DEV_ID_MPU2_CPU0,
    SBL_CLK_ID_MPU2_CPU0,
    SBL_MPU2_CPU0_FREQ_HZ,
    },
    /* MPU2_CPU1 info */
    {
    SBL_PROC_ID_MPU2_CPU1,
    SBL_DEV_ID_MPU2_CPU1,
    SBL_CLK_ID_MPU2_CPU1,
    SBL_MPU2_CPU1_FREQ_HZ,
    },
    /* MPU2_CPU2 info */
    {
    SBL_PROC_ID_MPU2_CPU2,
    SBL_DEV_ID_MPU2_CPU2,
    SBL_CLK_ID_MPU2_CPU2,
    SBL_MPU2_CPU2_FREQ_HZ,
    },
    /* MPU2_CPU3 info */
    {
    SBL_PROC_ID_MPU2_CPU3,
    SBL_DEV_ID_MPU2_CPU3,
    SBL_CLK_ID_MPU2_CPU3,
    SBL_MPU2_CPU3_FREQ_HZ,
    },
    /* MCU1_CPU0 info */
    {
    SBL_PROC_ID_MCU1_CPU0,
    SBL_DEV_ID_MCU1_CPU0,
    SBL_CLK_ID_MCU1_CPU0,
    SBL_MCU1_CPU0_FREQ_HZ,
    },
    /* MCU1_CPU1 info */
    {
    SBL_PROC_ID_MCU1_CPU1,
    SBL_DEV_ID_MCU1_CPU1,
    SBL_CLK_ID_MCU1_CPU1,
    SBL_MCU1_CPU1_FREQ_HZ,
    },
    /* MCU2_CPU0 info */
    {
    SBL_PROC_ID_MCU2_CPU0,
    SBL_DEV_ID_MCU2_CPU0,
    SBL_CLK_ID_MCU2_CPU0,
    SBL_MCU2_CPU0_FREQ_HZ,
    },
    /* MCU2_CPU1 info */
    {
    SBL_PROC_ID_MCU2_CPU1,
    SBL_DEV_ID_MCU2_CPU1,
    SBL_CLK_ID_MCU2_CPU1,
    SBL_MCU2_CPU1_FREQ_HZ,
    },
    /* MCU3_CPU0 info */
    {
    SBL_PROC_ID_MCU3_CPU0,
    SBL_DEV_ID_MCU3_CPU0,
    SBL_CLK_ID_MCU3_CPU0,
    SBL_MCU3_CPU0_FREQ_HZ,
    },
    /* MCU3_CPU1 info */
    {
    SBL_PROC_ID_MCU3_CPU1,
    SBL_DEV_ID_MCU3_CPU1,
    SBL_CLK_ID_MCU3_CPU1,
    SBL_MCU3_CPU1_FREQ_HZ,
    },
    /* MCU4_CPU0 info */
    {
    SBL_PROC_ID_MCU4_CPU0,
    SBL_DEV_ID_MCU4_CPU0,
    SBL_CLK_ID_MCU4_CPU0,
    SBL_MCU4_CPU0_FREQ_HZ,
    },
    /* MCU4_CPU1 info */
    {
    SBL_PROC_ID_MCU4_CPU1,
    SBL_DEV_ID_MCU4_CPU1,
    SBL_CLK_ID_MCU4_CPU1,
    SBL_MCU4_CPU1_FREQ_HZ,
    },
    /* DSP1_C66X info */
    {
    SBL_PROC_ID_DSP1_C66X,
    SBL_DEV_ID_DSP1_C66X,
    SBL_CLK_ID_DSP1_C66X,
    SBL_DSP1_C66X_FREQ_HZ,
    },
    /* DSP2_C66X info */
    {
    SBL_PROC_ID_DSP2_C66X,
    SBL_DEV_ID_DSP2_C66X,
    SBL_CLK_ID_DSP2_C66X,
    SBL_DSP2_C66X_FREQ_HZ,
    },
    /* DSP1_C7X info */
    {
    SBL_PROC_ID_DSP1_C7X,
    SBL_DEV_ID_DSP1_C7X,
    SBL_CLK_ID_DSP1_C7X,
    SBL_DSP1_C7X_FREQ_HZ,
    },
    /* DSP2_C7X info */
    {
    SBL_PROC_ID_DSP2_C7X,
    SBL_DEV_ID_DSP2_C7X,
    SBL_CLK_ID_DSP2_C7X,
    SBL_DSP2_C7X_FREQ_HZ,
    },
    /* DSP3_C7X info */
    {
    SBL_PROC_ID_DSP3_C7X,
    SBL_DEV_ID_DSP3_C7X,
    SBL_CLK_ID_DSP3_C7X,
    SBL_DSP3_C7X_FREQ_HZ,
    },
    /* DSP4_C7X info */
    {
    SBL_PROC_ID_DSP4_C7X,
    SBL_DEV_ID_DSP4_C7X,
    SBL_CLK_ID_DSP4_C7X,
    SBL_DSP4_C7X_FREQ_HZ,
    },
    /* DSP1_C7X HOST_EMU (invalid) */
    {
    SBL_INVALID_ID,
    SBL_INVALID_ID,
    SBL_INVALID_ID,
    SBL_INVALID_ID,
    },
    /* HSM M4F */
    {
    SBL_PROC_ID_HSM_M4,
    SBL_DEV_ID_HSM_M4,
    SBL_CLK_ID_HSM_M4,
    SBL_HSM_M4_FREQ_HZ
    }
};

static const uint32_t SblAtcmAddr[] =
{
SBL_MCU_ATCM_BASE,
SBL_MCU1_CPU1_ATCM_BASE_ADDR_SOC,
SBL_MCU2_CPU0_ATCM_BASE_ADDR_SOC,
SBL_MCU2_CPU1_ATCM_BASE_ADDR_SOC,
SBL_MCU3_CPU0_ATCM_BASE_ADDR_SOC,
SBL_MCU3_CPU1_ATCM_BASE_ADDR_SOC,
SBL_MCU4_CPU0_ATCM_BASE_ADDR_SOC,
SBL_MCU4_CPU1_ATCM_BASE_ADDR_SOC
};

static const uint32_t SblBtcmAddr[] =
{
SBL_MCU_BTCM_BASE,
SBL_MCU1_CPU1_BTCM_BASE_ADDR_SOC,
SBL_MCU2_CPU0_BTCM_BASE_ADDR_SOC,
SBL_MCU2_CPU1_BTCM_BASE_ADDR_SOC,
SBL_MCU3_CPU0_BTCM_BASE_ADDR_SOC,
SBL_MCU3_CPU1_BTCM_BASE_ADDR_SOC,
SBL_MCU4_CPU0_BTCM_BASE_ADDR_SOC,
SBL_MCU4_CPU1_BTCM_BASE_ADDR_SOC
};

/* ========================================================================== */
/*                           Internal Functions                               */
/* ========================================================================== */

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
__attribute__((weak)) uint32_t SBL_getBoardEcuType(void){
    return 0U; /** BOARD_ECU_NONE */
}
#endif

uint32_t SBL_getAtcmSize(cpu_core_id_t coreId)
{
    uint32_t atcmSize;
    switch (coreId)
    {
        case MCU1_CPU0_ID:
            atcmSize = SBL_MCU1_CPU0_ATCM_SIZE;
            break;
        case MCU1_CPU1_ID:
            atcmSize = SBL_MCU1_CPU1_ATCM_SIZE;
            break;
        case MCU2_CPU0_ID:
            atcmSize = SBL_MCU2_CPU0_ATCM_SIZE;
            break;
        case MCU2_CPU1_ID:
            atcmSize = SBL_MCU2_CPU1_ATCM_SIZE;
            break;
        case MCU3_CPU0_ID:
            atcmSize = SBL_MCU3_CPU0_ATCM_SIZE;
            break;
        case MCU3_CPU1_ID:
            atcmSize = SBL_MCU3_CPU1_ATCM_SIZE;
            break;
        case MCU4_CPU0_ID:
            atcmSize = SBL_MCU4_CPU0_ATCM_SIZE;
            break;
        case MCU4_CPU1_ID:
            atcmSize = SBL_MCU4_CPU1_ATCM_SIZE;
            break;
        default:
            atcmSize = 0;
            break;
    }
    return atcmSize;
}

uint32_t SBL_getBtcmSize(cpu_core_id_t coreId)
{
    uint32_t btcmSize;
    switch (coreId)
    {
        case MCU1_CPU0_ID:
            btcmSize = SBL_MCU1_CPU0_BTCM_SIZE;
            break;
        case MCU1_CPU1_ID:
            btcmSize = SBL_MCU1_CPU1_BTCM_SIZE;
            break;
        case MCU2_CPU0_ID:
            btcmSize = SBL_MCU2_CPU0_BTCM_SIZE;
            break;
        case MCU2_CPU1_ID:
            btcmSize = SBL_MCU2_CPU1_BTCM_SIZE;
            break;
        case MCU3_CPU0_ID:
            btcmSize = SBL_MCU3_CPU0_BTCM_SIZE;
            break;
        case MCU3_CPU1_ID:
            btcmSize = SBL_MCU3_CPU1_BTCM_SIZE;
            break;
        case MCU4_CPU0_ID:
            btcmSize = SBL_MCU4_CPU0_BTCM_SIZE;
            break;
        case MCU4_CPU1_ID:
            btcmSize = SBL_MCU4_CPU1_BTCM_SIZE;
            break;
        default:
            btcmSize = 0;
            break;
    }
    return btcmSize;

}

static void SBL_RequestCore(cpu_core_id_t core_id)
{
#if !defined(SBL_SKIP_BRD_CFG_BOARD) && !defined(SBL_SKIP_SYSFW_INIT)
    int32_t proc_id = sbl_slave_core_info[core_id].tisci_proc_id;
    int32_t status = CSL_EFAIL;

    if (proc_id != SBL_INVALID_ID)
    {
        SBL_log(SBL_LOG_MAX, "Calling Sciclient_procBootRequestProcessor, ProcId 0x%x... \n", proc_id);

        status = Sciclient_procBootRequestProcessor(proc_id, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            SBL_log(SBL_LOG_ERR, "Sciclient_procBootRequestProcessor, ProcId 0x%x...FAILED \n", proc_id);
            SblErrLoop(__FILE__, __LINE__);
        }
    }
#endif

    return;
}

static void SBL_RequestAllCores(void)
{
#if !defined(SBL_SKIP_BRD_CFG_BOARD) && !defined(SBL_SKIP_SYSFW_INIT)
    cpu_core_id_t core_id;
    uint32_t num_cores = sizeof(sbl_slave_core_info)/ sizeof(sblSlaveCoreInfo_t);

    for (core_id = 0; core_id < num_cores; core_id++)
    {
        SBL_RequestCore(core_id);
    }

#endif

    return;
}

static void SBL_ReleaseCore (cpu_core_id_t core_id, uint32_t reqFlag)
{
#if !defined(SBL_SKIP_BRD_CFG_BOARD) && !defined(SBL_SKIP_SYSFW_INIT)
    int32_t proc_id = sbl_slave_core_info[core_id].tisci_proc_id;
    int32_t status = CSL_EFAIL;

    if(proc_id != SBL_INVALID_ID)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_procBootReleaseProcessor, ProcId 0x%x...\n", proc_id);
        status = Sciclient_procBootReleaseProcessor(proc_id, reqFlag, SCICLIENT_SERVICE_WAIT_FOREVER);

        if (status != CSL_PASS)
        {
            SBL_log(SBL_LOG_ERR, "Sciclient_procBootReleaseProcessor, ProcId 0x%x...FAILED \n", proc_id);
            SblErrLoop(__FILE__, __LINE__);
        }
    }
#endif

    return;
}

static void SBL_ReleaseAllCores(void)
{
#if !defined(SBL_SKIP_BRD_CFG_BOARD) && !defined(SBL_SKIP_SYSFW_INIT)
    cpu_core_id_t core_id;
    uint32_t num_cores = sizeof(sbl_slave_core_info)/sizeof(sblSlaveCoreInfo_t);

    for (core_id = 0; core_id < num_cores; core_id++)
    {
        SBL_ReleaseCore(core_id, TISCI_MSG_FLAG_AOP);
    }

#endif

    return;
}

static void SBL_ConfigMcuLockStep(uint8_t enableLockStep, const sblSlaveCoreInfo_t *sblCoreInfoPtr)
{
    int32_t status = CSL_EFAIL;
    struct tisci_msg_proc_get_status_resp cpuStatus;
    struct tisci_msg_proc_set_config_req  proc_set_config_req;

    SBL_log(SBL_LOG_MAX, "Calling Sciclient_procBootGetProcessorState, ProcId 0x%x... \n", sblCoreInfoPtr->tisci_proc_id);
    status = Sciclient_procBootGetProcessorState(sblCoreInfoPtr->tisci_proc_id, &cpuStatus, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status != CSL_PASS)
    {
        SBL_log(SBL_LOG_ERR, "Sciclient_procBootGetProcessorState...FAILED \n");
        SblErrLoop(__FILE__, __LINE__);
    }

    proc_set_config_req.processor_id = cpuStatus.processor_id;
    proc_set_config_req.bootvector_lo = cpuStatus.bootvector_lo;
    proc_set_config_req.bootvector_hi = cpuStatus.bootvector_hi;
    proc_set_config_req.config_flags_1_set = 0;
    proc_set_config_req.config_flags_1_clear = 0;

    if (enableLockStep)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_procBootSetProcessorCfg, ProcId 0x%x, enabling Lockstep mode...\n", cpuStatus.processor_id);
        proc_set_config_req.config_flags_1_set |= TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
    }
    else
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_procBootSetProcessorCfg, ProcId 0x%x, enabling split mode...\n", cpuStatus.processor_id);
        proc_set_config_req.config_flags_1_clear |= TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_LOCKSTEP;
    }

    status =  Sciclient_procBootSetProcessorCfg(&proc_set_config_req,  SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status != CSL_PASS)
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_procBootSetProcessorCfg lockstep...NOT DONE \n");
    }

    return;
}

int32_t SBL_BootImage(sblEntryPoint_t *pEntry)
{
    int32_t retval = 0;
    cpu_core_id_t core_id;

    /* Initialize the entry point array to 0. */
    for (core_id = MPU1_CPU0_ID; core_id < NUM_CORES; core_id ++)
        pEntry->CpuEntryPoint[core_id] = SBL_INVALID_ENTRY_ADDR;

    /* Request SYSFW for control of all cores */
    SBL_RequestAllCores();

#if defined(BOOT_MMCSD)
    /* MMCSD Boot Mode Image Copy function. */
    if (SBL_MMCBootImage(pEntry) != E_PASS)
#elif defined(BOOT_EMMC_UDA) || defined(BOOT_EMMC_BOOT0) 
    if (SBL_eMMCBootImage(pEntry) != E_PASS)
#elif defined(BOOT_OSPI)
    if (SBL_OSPIBootImage(pEntry) != E_PASS)
#elif defined(BOOT_UART)
    if (SBL_UARTBootImage(pEntry) != E_PASS)
#elif defined(BOOT_HYPERFLASH)
    if (SBL_HYPERFLASHBootImage(pEntry) != E_PASS)
#endif
    {
        retval = E_FAIL;
    }

    /* Release control of all cores */
    SBL_ReleaseAllCores();

    return retval;
}

/**
 * \brief        SBL_SetupCoreMem function sets up the CPUs internal memory
 *
 * \param[in]    core_id - CPU ID
 * \param[in]    pAppEntry - Core info struct
 *
 * \return   none
 */
void SBL_SetupCoreMem(uint32_t core_id)
{
    int32_t status = CSL_EFAIL;
    uint8_t runLockStep = 0;
    uint8_t mcuModeConfigured = 0;
    uint32_t atcmSize;
    uint32_t btcmSize;
    struct tisci_msg_proc_get_status_resp cpuStatus;
    struct tisci_msg_proc_set_config_req  proc_set_config_req;
    const sblSlaveCoreInfo_t *sblSlaveCoreInfoPtr;

    /* Remap virtual core-ids if needed */
    switch (core_id)
    {
        case MCU1_SMP_ID:
            runLockStep = 1;
            core_id = MCU1_CPU0_ID;
            break;
        case MCU2_SMP_ID:
            runLockStep = 1;
            core_id = MCU2_CPU0_ID;
            break;
        case MCU3_SMP_ID:
            runLockStep = 1;
            core_id = MCU3_CPU0_ID;
            break;
        case MCU4_SMP_ID:
            runLockStep = 1;
            core_id = MCU4_CPU0_ID;
            break;
        default:
            break;
    }

    sblSlaveCoreInfoPtr = &(sbl_slave_core_info[core_id]);

    if(runLockStep)
    {
        SBL_log(SBL_LOG_MAX, "Detected lockstep for core_id %d, proc_id 0x%x... \n", core_id, sblSlaveCoreInfoPtr->tisci_proc_id);
        SBL_ConfigMcuLockStep(SBL_ENABLE_MCU_LOCKSTEP, sblSlaveCoreInfoPtr);
        mcuModeConfigured = 1;
    }

    switch (core_id)
    {

        case DSP1_C66X_ID:
            break;
        case DSP2_C66X_ID:
            break;
        case DSP1_C7X_ID:
            break;
        case DSP2_C7X_ID:
            break;
        case DSP3_C7X_ID:
            break;
        case DSP4_C7X_ID:
            break;

        case MCU1_CPU1_ID:
        case MCU2_CPU1_ID:
        case MCU3_CPU1_ID:
        case MCU4_CPU1_ID:
            SBL_log(SBL_LOG_MAX, "Switching core id %d, proc_id 0x%x to split mode... \n", core_id-1, sbl_slave_core_info[core_id-1].tisci_proc_id);
            /* Image for second MCU core present, disable lock step for the cluster */
            SBL_ConfigMcuLockStep(SBL_DISABLE_MCU_LOCKSTEP, &(sbl_slave_core_info[core_id-1]));
            mcuModeConfigured = 1;
            /* DOnt break, fall through for enabling TCMs */
        case MCU1_CPU0_ID:
        case MCU2_CPU0_ID:
        case MCU3_CPU0_ID:
        case MCU4_CPU0_ID:
            if (!mcuModeConfigured)
            {
                SBL_log(SBL_LOG_MAX, "Switching core id %d, proc_id 0x%x to split mode... \n", core_id, sbl_slave_core_info[core_id].tisci_proc_id);
                /* Non-SMP image used, disable lock step for the cluster */
                SBL_ConfigMcuLockStep(SBL_DISABLE_MCU_LOCKSTEP, &(sbl_slave_core_info[core_id]));
            }

            /* Ensure Power is OFF for each core before configuring TCMs */
            /* SBL running on MCU0, don't fool around with its power */
            if (core_id != MCU1_CPU0_ID)
            {
                if (runLockStep)
                {
                    SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState Off, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id + 1);
                    Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id + 1, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                }
                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState Off, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id);
                Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
            }

            SBL_log(SBL_LOG_MAX, "Calling Sciclient_procBootGetProcessorState, ProcId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_proc_id);
            status = Sciclient_procBootGetProcessorState(sblSlaveCoreInfoPtr->tisci_proc_id, &cpuStatus, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
                SBL_log(SBL_LOG_ERR, "Sciclient_procBootGetProcessorState...FAILED \n");
                SblErrLoop(__FILE__, __LINE__);
            }

            proc_set_config_req.processor_id = cpuStatus.processor_id;
            proc_set_config_req.bootvector_lo = cpuStatus.bootvector_lo;
            proc_set_config_req.bootvector_hi = cpuStatus.bootvector_hi;
            proc_set_config_req.config_flags_1_set = 0;
            proc_set_config_req.config_flags_1_clear = 0;
            SBL_log(SBL_LOG_MAX, "Enabling MCU TCMs after reset for core %d\n", core_id);
            proc_set_config_req.config_flags_1_set |= TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_ATCM_EN;
            proc_set_config_req.config_flags_1_set |= (TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_BTCM_EN |
                                                       TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_TCM_RSTBASE);

#if defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
            /* Only need to set mem_init disable bit for MCU1_0 or MCU2_0 (for each cluster) */
            if ((core_id == MCU1_CPU0_ID) || (core_id == MCU2_CPU0_ID) || (core_id == MCU3_CPU0_ID) || (core_id == MCU4_CPU0_ID))
            {
                SBL_log(SBL_LOG_MAX, "Disabling HW-based memory init of MCU TCMs for core %d\n", core_id);
                proc_set_config_req.config_flags_1_set |= TISCI_MSG_VAL_PROC_BOOT_CFG_FLAG_R5_MEM_INIT_DIS;
            }
#endif

            SBL_log(SBL_LOG_MAX, "Sciclient_procBootSetProcessorCfg update TCM enable/disable settings...\n");
            status =  Sciclient_procBootSetProcessorCfg(&proc_set_config_req,  SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
                SBL_log(SBL_LOG_ERR, "Sciclient_procBootSetProcessorCfg...FAILED \n");
                SblErrLoop(__FILE__, __LINE__);
            }

            /* For lockstep R5 pairs, this section will naturally only set HALT bit for MCU2_CPU0_ID or MCU3_CPU0_ID */
            if (core_id != MCU1_CPU0_ID)
            {
                SBL_log(SBL_LOG_MAX, "Setting HALT for ProcId 0x%x...\n", sblSlaveCoreInfoPtr->tisci_proc_id);
                status =  Sciclient_procBootSetSequenceCtrl(sblSlaveCoreInfoPtr->tisci_proc_id, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, 0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                if (status != CSL_PASS)
                {
                    SBL_log(SBL_LOG_ERR, "Sciclient_procBootSetSequenceCtrl...FAILED \n");
                    SblErrLoop(__FILE__, __LINE__);
                }
            }

            /* SBL running on MCU0, don't fool around with its power & TCMs */
            if (core_id != MCU1_CPU0_ID)
            {
                if (runLockStep)
                {
                    /* For J721E, Hardware is not optimized to use the ATCM of both the cores
                       in lockstep mode */
#if !defined(SOC_J721E)
                    /* When operating in lockstep, size of atcm becomes sum of atcm of both the cores in the cluster */
                    atcmSize =  SBL_getAtcmSize(core_id) + SBL_getAtcmSize(core_id+1);
                    btcmSize =  SBL_getBtcmSize(core_id) + SBL_getBtcmSize(core_id+1);
#else
                    atcmSize =  SBL_getAtcmSize(core_id);
                    btcmSize =  SBL_getBtcmSize(core_id);
#endif
                }
                else
                {
                    atcmSize =  SBL_getAtcmSize(core_id);
                    btcmSize =  SBL_getBtcmSize(core_id);
                }

                if (runLockStep)
                {
                    /* If in lock-step mode, need to bring Core 1 out of reset, before Core 0, in order to init TCMs */
                    SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id + 1);
                    Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id + 1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                }
                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id);
                Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);

                /* Initialize the TCMs - TCMs of MCU running SBL are already initialized by ROM & SBL */
                SBL_log(SBL_LOG_MAX, "Clearing core_id %d (lock-step) ATCM @ 0x%x\n", core_id, SblAtcmAddr[core_id - MCU1_CPU0_ID]);
                memset(((void *)(SblAtcmAddr[core_id - MCU1_CPU0_ID])), 0xFF, atcmSize);

                SBL_log(SBL_LOG_MAX, "Clearing core_id %d (lock-step) BTCM @ 0x%x\n", core_id, SblBtcmAddr[core_id - MCU1_CPU0_ID]);
                memset(((void *)(SblBtcmAddr[core_id - MCU1_CPU0_ID])), 0xFF, btcmSize);
            }
            break;
        case MPU1_SMP_ID:
        case MPU1_CPU0_ID:
        case MPU1_CPU1_ID:
        case MPU1_CPU2_ID:
        case MPU1_CPU3_ID:
            SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", SBL_DEV_ID_MPU_CLUSTER0);
            Sciclient_pmSetModuleState(SBL_DEV_ID_MPU_CLUSTER0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            break;
        case MPU2_SMP_ID:
        case MPU2_CPU0_ID:
        case MPU2_CPU1_ID:
        case MPU2_CPU2_ID:
        case MPU2_CPU3_ID:
            SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", SBL_DEV_ID_MPU_CLUSTER1);
            Sciclient_pmSetModuleState(SBL_DEV_ID_MPU_CLUSTER1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            break;
        case MPU_SMP_ID:
            /* Enable SMP on all MPU clusters. Enable SMP only if cluster is present */
            if (SBL_DEV_ID_MPU_CLUSTER0 != SBL_INVALID_ID)
            {
                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", SBL_DEV_ID_MPU_CLUSTER0);
                Sciclient_pmSetModuleState(SBL_DEV_ID_MPU_CLUSTER0, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            }
            if (SBL_DEV_ID_MPU_CLUSTER1 != SBL_INVALID_ID)
            {
                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", SBL_DEV_ID_MPU_CLUSTER1);
                Sciclient_pmSetModuleState(SBL_DEV_ID_MPU_CLUSTER1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            }
            break;
        default:
            /* No special memory setup needed */
            break;
    }

    return;
}

/**
 * \brief    SBL_SlaveCoreBoot function sets the entry point, sets up clocks
 *           and enable to core to start executing from entry point.
 *
 * \param    core_id = Selects a core on the SOC, refer to cpu_core_id_t enum
 *           freqHz = Speed of core at boot up, 0 indicates use SBL default freqs.
 *           pAppEntry = SBL entry point struct
 *           requestCoresFlag = Specify whether cores should be requested/released
 *               from within SBL_SlaveCoreBoot. Accepts the values SBL_REQUEST_CORE
 *               and SBL_DONT_REQUEST_CORE.
 *
 **/
void SBL_SlaveCoreBoot(cpu_core_id_t core_id, uint32_t freqHz, sblEntryPoint_t *pAppEntry, uint32_t requestCoresFlag)
{
    int32_t status = CSL_EFAIL;
    struct tisci_msg_proc_set_config_req  proc_set_config_req;
    const sblSlaveCoreInfo_t *sblSlaveCoreInfoPtr = &(sbl_slave_core_info[core_id]);
    uint64_t slave_clk_freq_hz = BOARD_INVALID_CORE_CLK_FREQ;

#if defined(SBL_SKIP_MCU_RESET) && (defined(SBL_SKIP_BRD_CFG_BOARD) || defined(SBL_SKIP_BRD_CFG_PM) || defined(SBL_SKIP_SYSFW_INIT))
    /* Skip copy if R5 app entry point is already 0 */
    if ((core_id == MCU1_CPU0_ID) &&
       (pAppEntry->CpuEntryPoint[core_id]) &&
       (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR))
    {
        SBL_log(SBL_LOG_MAX, "Copying first 128 bytes from app to MCU ATCM @ 0x%x for core %d\n", SblAtcmAddr[core_id - MCU1_CPU0_ID], core_id);
        memcpy(((void *)(SblAtcmAddr[core_id - MCU1_CPU0_ID])), (void *)(pAppEntry->CpuEntryPoint[core_id]), 128);
        return;
    }

    /* Finished processing images for all cores, start MCU_0 */
    if ((core_id == MCU1_CPU1_ID) &&
        (pAppEntry->CpuEntryPoint[core_id] >=  SBL_INVALID_ENTRY_ADDR))
    {
            /* Display profile logs */
            SBL_printProfileLog();

            SBL_log(SBL_LOG_MAX, "Starting app, branching to 0x0 \n");
            /* Branch to start of ATCM */
            ((void(*)(void))0x0)();
    }
#endif

    /* Check if entry point is aligned correctly */
    if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR)
    {
        if ((core_id == MCU1_CPU0_ID) || (core_id == MCU1_CPU1_ID) ||
            (core_id == MCU2_CPU0_ID) || (core_id == MCU2_CPU1_ID) ||
            (core_id == MCU3_CPU0_ID) || (core_id == MCU3_CPU1_ID) ||
            (core_id == MCU4_CPU0_ID) || (core_id == MCU4_CPU1_ID))
        {
            if ((pAppEntry->CpuEntryPoint[core_id] & SBL_BIT_MASK_128_BYTE_ALIGNED) != pAppEntry->CpuEntryPoint[core_id])
            {
                SBL_log(SBL_LOG_ERR, "Entry Point of R5F application should be aligned to 128 bytes.\n");
                SblErrLoop(__FILE__, __LINE__);
            }
        }
        else if ((core_id == DSP1_C7X_ID) || (core_id == DSP2_C7X_ID) ||
                 (core_id == DSP3_C7X_ID) || (core_id == DSP4_C7X_ID))
        {
            if ((pAppEntry->CpuEntryPoint[core_id] & SBL_BIT_MASK_2MB_ALIGNED) != pAppEntry->CpuEntryPoint[core_id])
            {
                SBL_log(SBL_LOG_ERR, "Entry Point of C7x application should be aligned to 2MB.\n");
                SblErrLoop(__FILE__, __LINE__);
            }
        }
        else if ((core_id == DSP1_C66X_ID) || (core_id == DSP2_C66X_ID))
        {
            if ((pAppEntry->CpuEntryPoint[core_id] & SBL_BIT_MASK_1KB_ALIGNED) != pAppEntry->CpuEntryPoint[core_id])
            {
                SBL_log(SBL_LOG_ERR, "Entry Point of C66x application should be aligned to 1KB.\n");
                SblErrLoop(__FILE__, __LINE__);
            }
        }
    }

    /* Request core */
    if (requestCoresFlag == SBL_REQUEST_CORE)
    {
        SBL_RequestCore(core_id);
    }

    /* Set entry point as boot vector */
    proc_set_config_req.processor_id = sblSlaveCoreInfoPtr->tisci_proc_id;
    proc_set_config_req.bootvector_lo = pAppEntry->CpuEntryPoint[core_id];
    proc_set_config_req.bootvector_hi = 0x0;
    proc_set_config_req.config_flags_1_set = 0;
    proc_set_config_req.config_flags_1_clear = 0;

    if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR) /* Set entry point only is valid */
    {
        SBL_log(SBL_LOG_MAX, "Sciclient_procBootSetProcessorCfg, ProcId 0x%x, EntryPoint 0x%x...\n", proc_set_config_req.processor_id, proc_set_config_req.bootvector_lo);
        status =  Sciclient_procBootSetProcessorCfg(&proc_set_config_req,  SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            SBL_log(SBL_LOG_ERR, "Sciclient_procBootSetProcessorCfg...FAILED \n");
            SblErrLoop(__FILE__, __LINE__);
        }

        if (sblSlaveCoreInfoPtr->tisci_dev_id != SBL_INVALID_ID)
        {
            #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
            slave_clk_freq_hz = Board_ecuGetCpuClock(sblSlaveCoreInfoPtr->tisci_dev_id, SBL_getBoardEcuType());
            #endif

            /* Setting default clock frequency or if slave_clk_freq_hz frequency is still BOARD_INVALID_CORE_CLK_FREQ, fallback to default */
            if (slave_clk_freq_hz == BOARD_INVALID_CORE_CLK_FREQ){
                slave_clk_freq_hz = sblSlaveCoreInfoPtr->slave_clk_freq_hz;
            }

            SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleClkFreq, DevId 0x%x @ %dHz... \n", sblSlaveCoreInfoPtr->tisci_dev_id, slave_clk_freq_hz);
            Sciclient_pmSetModuleClkFreq(sblSlaveCoreInfoPtr->tisci_dev_id,
                                         sblSlaveCoreInfoPtr->tisci_clk_id,
                                         slave_clk_freq_hz,
                                         TISCI_MSG_FLAG_AOP,
                                         SCICLIENT_SERVICE_WAIT_FOREVER);
        }
    }
    else
    {
        SBL_log(SBL_LOG_MAX, "Skipping Sciclient_procBootSetProcessorCfg for ProcId 0x%x, EntryPoint 0x%x...\n", proc_set_config_req.processor_id, proc_set_config_req.bootvector_lo);
    }
    /* Power down and then power up each core*/
    switch (core_id)
    {
        case MCU1_CPU1_ID:
            /* Display profile logs */
            SBL_printProfileLog();

            if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR)
            {
                /* Skip copy if R5 app entry point is already 0 */
                if (pAppEntry->CpuEntryPoint[core_id])
                {
                    SBL_log(SBL_LOG_MAX, "Copying first 128 bytes from app to MCU ATCM @ 0x%x for core %d\n", SblAtcmAddr[core_id - MCU1_CPU0_ID], core_id);
                    memcpy(((void *)(SblAtcmAddr[core_id - MCU1_CPU0_ID])), (void *)(pAppEntry->CpuEntryPoint[core_id]), 128);
                }
            }

#ifdef SBL_SKIP_MCU_RESET
            if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR)
            {
                /* Un-halt MCU1_1 core */
                Sciclient_procBootSetSequenceCtrl(SBL_PROC_ID_MCU1_CPU1, 0, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                Sciclient_pmSetModuleState(SBL_DEV_ID_MCU1_CPU1, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                Sciclient_pmSetModuleState(SBL_DEV_ID_MCU1_CPU1, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            }

            /* Release the CPU and branch to app */
            if (requestCoresFlag == SBL_REQUEST_CORE)
            {
                SBL_ReleaseCore(core_id, TISCI_MSG_FLAG_AOP);
            }

            /* Release MCU1_0 before jumping to application */
            SBL_ReleaseCore(core_id - 1, TISCI_MSG_FLAG_AOP);

            /* Start the App - Branch to start of ATCM (0x0) */
            ((void(*)(void))0x0)();
#else
            /* Request MCU1_0 */
            if (requestCoresFlag == SBL_REQUEST_CORE)
            {
                SBL_RequestCore(core_id - 1);
            }

            /**
             * Reset sequence for cluster running SBL
             *
             *   The reset sequence for the cluster running SBL has to be done differently from
             *   that of other clusters. We first need to provide a sequence of commands to SYSFW
             *   which then holds these commands in a queue and they are all executed at once.
             *   This ensures that we don't mess up the state of current host CPU.
             *   After providing whole sequence of commands host CPU will go into WFI.
             *   Then TIFS will detect the WFI and execute the commands from queue.
             *   More detail is described in comments below, but a high-level overview of the
             *   reset sequence is as follows:
             *
             *   1. Processor Boot Wait (hold all subsequent commands from current host in a queue i.e. points 2 to 8)
             *   2. MCU1_1 Enter Reset
             *   3. MCU1_0 Enter Reset
             *   4. Un-halt MCU1_1     - This is done to ensure that MCU1_1 does not go to halt once TIFS brings it out of reset
             *   5. MCU1_0 Leave Reset
             *   6. MCU1_1 Leave Reset (if an application is requested to run there)
             *   7. Release control of MCU1_0 - Tell SYSFW that MCU1_0 is not going to boot control this CPU anymore
             *   8. Release control of MCU1_1 - Tell SYSFW that MCU1_0 is not going to boot control this CPU anymore
             *   9. Execute WFI
             */

            /**
             * Processor Boot Wait
             *
             *   DMSC will block until a WFI is issued, thus allowing the following commands
             *   to be queued so this cluster may be reset by DMSC (queue length is defined in
             *   "drv/sciclient/soc/sysfw/include/<soc>/tisci_sec_proxy.h"). If these commands
             *   were to be issued and executed prior to WFI, the cluster would enter reset and
             *   SBL would quite sensibly not be able to tell DMSC to take itself out of reset.
             */
            /* SBL_log(SBL_LOG_MAX, "Sciclient_procBootWaitProcessorState, ProcId 0x%x... \n", SBL_PROC_ID_MCU1_CPU0); */
            status = Sciclient_procBootWaitProcessorState(SBL_PROC_ID_MCU1_CPU0, 1, 1, 0, 3, 0, 0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
                SBL_log(SBL_LOG_ERR, "Sciclient_procBootWaitProcessorState...FAILED \n");
                SblErrLoop(__FILE__, __LINE__);
            }

            /**
             * Both cores enter reset
             *
             *   It is necessary to reset MCU1_1 before MCU1_0, so as to maintain the specification that
             *   MCU1_1 may never ben in a higher functional state than MCU1_0.
             */
            Sciclient_pmSetModuleRst_flags(SBL_DEV_ID_MCU1_CPU1, 1, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            Sciclient_pmSetModuleRst_flags(SBL_DEV_ID_MCU1_CPU0, 1, 0, SCICLIENT_SERVICE_WAIT_FOREVER);

            /**
             * Un-halt MCU1_1 (MCU1_0 is not halted)
             */
            Sciclient_procBootSetSequenceCtrl(SBL_PROC_ID_MCU1_CPU1, 0, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, 0, SCICLIENT_SERVICE_WAIT_FOREVER);

            /**
             * MCU1_0 and (optionally) MCU1_1 leave reset
             *
             *   Ensuring that MCU1_1 is never in a higher functional state than MCU1_0, both cores
             *   shall leave reset. Only take MCU1_1 out of reset if an application will be running
             *   on it.
             */
            Sciclient_pmSetModuleRst_flags(SBL_DEV_ID_MCU1_CPU0, 0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR)
            {
                Sciclient_pmSetModuleRst_flags(SBL_DEV_ID_MCU1_CPU1, 0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            }

            /* Notifying SYSFW that the SBL is relinquishing the MCU cluster running the SBL */
            /* This is done at the end as the PM set module state relies on the fact the SBL is the owner of MCU1_0 and MCU1_1 */
            if (requestCoresFlag == SBL_REQUEST_CORE)
            {
                Sciclient_procBootReleaseProcessor(SBL_PROC_ID_MCU1_CPU0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
                Sciclient_procBootReleaseProcessor(SBL_PROC_ID_MCU1_CPU1, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
            }

            /* Execute a WFI */
            asm volatile ("    wfi");
#endif
            break;

        case MCU1_CPU0_ID:
            /* Skip copy if R5 app entry point is already 0 */
#if defined(SOC_J721E) || defined(SOC_J7200) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SBL_SKIP_MCU_RESET)
            if (pAppEntry->CpuEntryPoint[core_id])
            {
                SBL_log(SBL_LOG_MAX, "Copying first 128 bytes from app to MCU ATCM @ 0x%x for core %d\n", SblAtcmAddr[core_id - MCU1_CPU0_ID], core_id);
                memcpy(((void *)(SblAtcmAddr[core_id - MCU1_CPU0_ID])), (void *)(proc_set_config_req.bootvector_lo), 128);
            }
#endif
            break;
        case MCU2_CPU0_ID:
        case MCU2_CPU1_ID:
        case MCU3_CPU0_ID:
        case MCU3_CPU1_ID:
        case MCU4_CPU0_ID:
        case MCU4_CPU1_ID:
            if (pAppEntry->CpuEntryPoint[core_id] <  SBL_INVALID_ENTRY_ADDR)
            {
                /* Skip copy if R5 app entry point is already 0 */
                if (pAppEntry->CpuEntryPoint[core_id])
                {
                    SBL_log(SBL_LOG_MAX, "Copying first 128 bytes from app to MCU ATCM @ 0x%x for core %d\n", SblAtcmAddr[core_id - MCU1_CPU0_ID], core_id);
                    memcpy(((void *)(SblAtcmAddr[core_id - MCU1_CPU0_ID])), (void *)(proc_set_config_req.bootvector_lo), 128);
                }
                SBL_log(SBL_LOG_MAX, "Clearing HALT for ProcId 0x%x...\n", sblSlaveCoreInfoPtr->tisci_proc_id);
                status =  Sciclient_procBootSetSequenceCtrl(sblSlaveCoreInfoPtr->tisci_proc_id, 0, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                if (status != CSL_PASS)
                {
                    SBL_log(SBL_LOG_ERR, "Sciclient_procBootSetSequenceCtrl...FAILED \n");
                    SblErrLoop(__FILE__, __LINE__);
                }
            }

            /* Release core */
            if (requestCoresFlag == SBL_REQUEST_CORE)
            {
                SBL_ReleaseCore(core_id, TISCI_MSG_FLAG_AOP);
            }

            break;
       case HSM_CPU_ID:
            /* Release core */
            if (status == CSL_PASS) {
                status = Sciclient_procBootSetSequenceCtrl(sblSlaveCoreInfoPtr->tisci_proc_id, 0, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_HSM_M4_RESET, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                if (status == CSL_PASS)
                {
                    if (requestCoresFlag == SBL_REQUEST_CORE)
                    {
                        SBL_ReleaseCore(core_id, TISCI_MSG_FLAG_AOP);
                    }
                }

            }
            break;
        default:
            if (status == CSL_PASS) {
                /* DSP LPSC's is shared among other devices.               */
                /* Inorder to turnoff DSP cores, we need to make sure all  */
                /* devices connected to DSP power domain is turned off     */

                int32_t coreIdIndex = 0;
                int32_t coreIdCount=sizeof(c7xCoreID)/sizeof(c7xCoreID[0]);
                for (coreIdIndex = 0 ; coreIdIndex < coreIdCount; coreIdIndex++)
                {
                    if (core_id == c7xCoreID[coreIdIndex])
                    {
                        int32_t loopCount = 0;
                        int32_t devIndex =0;

                        loopCount=sizeof(c7xLpscSharedDevid[coreIdIndex])/sizeof(c7xLpscSharedDevid[coreIdIndex][0]);

                        for (devIndex = 0; devIndex < loopCount; devIndex++)
                        {
                            int32_t devId = c7xLpscSharedDevid[coreIdIndex][devIndex];
                            if (devId == SBL_INVALID_ID)
                            {
                                break;
                            }
                            SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState Off, DevId 0x%x... \n", devId);
                            Sciclient_pmSetModuleState(devId, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                        }
                    }
                }

                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState Off, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id);
                Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
                SBL_log(SBL_LOG_MAX, "Sciclient_pmSetModuleState On, DevId 0x%x... \n", sblSlaveCoreInfoPtr->tisci_dev_id);
                Sciclient_pmSetModuleState(sblSlaveCoreInfoPtr->tisci_dev_id, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);

                /* Release core */
                if (requestCoresFlag == SBL_REQUEST_CORE)
                {
                    SBL_ReleaseCore(core_id, TISCI_MSG_FLAG_AOP);
                }
            }

            break;
    }
}
