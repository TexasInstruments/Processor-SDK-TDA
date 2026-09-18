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
 *  \ingroup DRV_LPM_MODULE
 *  \defgroup DRV_LPM_BOOT_MODULE LPM Driver Boot API
 *            This is LPM driver boot related configuration parameters and
 *            API
 *
 *  @{
 */

/**
 *  \file lpm_boot.h
 *
 *  \brief LPM boot related parameters and API.
 */

#ifndef LPM_BOOT_H_
#define LPM_BOOT_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>

#include <ti/boot/sbl/soc/sbl_soc.h>
#include <ti/boot/sbl/soc/k3/sbl_slave_core_boot.h>
#include <ti/boot/sbl/soc/k3/sbl_profile.h>
#include <ti/boot/sbl/soc/k3/sbl_soc_cfg.h>
#include <ti/boot/sbl/soc/k3/sbl_qos.h>
#include <ti/boot/sbl/soc/k3/sbl_log.h>
#include <ti/boot/sbl/soc/k3/sbl_qos.h>

#include <ti/csl/cslr_gtc.h>

#include <ti/drv/spi/soc/SPI_soc.h>
#include <ti/board/board.h>
#include <ti/board/board_cfg.h>
#include <ti/board/src/flash/include/board_flash.h>
#if defined(SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_control.h>
#include <ti/drv/lpm/soc/j721e/boot_core_defs.h>
#endif
#if defined(SOC_J7200)
#include <ti/board/src/j7200_evm/include/board_control.h>
#include <ti/drv/lpm/soc/j7200/boot_core_defs.h>
#endif
#if defined(SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_control.h>
#include <ti/drv/lpm/soc/j784s4/boot_core_defs.h>
#endif

#include <ti/drv/lpm/src/lpm_uart_drv.h>

#include <ti/osal/osal.h>
#include <ti/osal/TaskP.h>

/* PM Lib */
#include <ti/drv/pm/include/pm_types.h>
#include <ti/drv/pm/include/dmsc/pmlib_sysconfig.h>
#include <ti/drv/pm/include/dmsc/pmlib_clkrate.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/**
 *  \brief This structure contains boot paramters required by the SBL
 */
typedef struct Lpm_SblIncomingBootData_S
{
    uint8_t     sblBootBuff[SBL_MAX_BOOT_BUFF_SIZE+1];
    /**< LPM SBL boot buffer to hold boot information */
    uint32_t    sblBootSize;
    /**< LPM SBL boot size. */
    uint32_t    sblBootBuffIdx;
    /**< LPM SBL boot index. */
} Lpm_SblIncomingBootData;

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Boots up all main domain cores 
 * 
 * Boots R5Fs and DSPs with the RTOS/Baremtal firmwares present in Flash
 * Boots A72 core with Linux
 * 
 *
 *  \return Implementation specific return codes. Negative values indicate
 *          unsuccessful operations.
 */
int32_t Lpm_bootApp(void);

/**
 *  \brief Intializes the SBL and the Flash Driver
 */
void Lpm_bootAppInit(void);

/**
 *  \brief De-Intializes the SBL and the Flash Driver
 */
void Lpm_bootAppDeInit(void);

#endif /* LPM_BOOT_H_ */
