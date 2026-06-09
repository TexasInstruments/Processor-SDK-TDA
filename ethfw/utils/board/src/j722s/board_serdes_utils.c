/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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

/*!
 * \file  board_serdes_utils.c
 *
 * \brief This file contains the implementation of the J722S SK-EVM board
 *        configuration functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <stdbool.h>
#include <drivers/hw_include/cslr_soc.h>
#include <drivers/sciclient.h>
#include <mod/cpsw_macport.h>
#include <utils/board/include/ethfw_board_utils.h>

#include <j722s_csl_serdes3.h>
#include <csl_serdes.h>
#include <sgmii/V5/csl_cpsgmii.h>
#include <serdes_cd/V1/csl_serdes3_ethernet.h>
#include <cslr_cpsgmii.h>
#include <kernel/dpl/DebugP.h>

#define CSL_PASS                        ( (int32_t) (0))
#define CSL_EFAIL                       (-(int32_t) (1))
#define CSL_EBADARGS                    (-(int32_t) (2))
#define CSL_EINVALID_PARAMS             (-(int32_t) (3))
#define CSL_ETIMEOUT                    (-(int32_t) (4))
#define CSL_EOUT_OF_RANGE               (-(int32_t) (5))
#define CSL_EUNSUPPORTED_CMD            (-(int32_t) (6))
#define CSL_EUNSUPPORTED_OPS            (-(int32_t) (7))
#define CSL_EALLOC                      (-(int32_t) (8))

void BoardUtils_setDeviceState(uint32_t moduleId,
                               uint32_t requiredState,
                               uint32_t appFlags)
{
    uint32_t status;
    uint32_t moduleState      = 0U;
    uint32_t resetState       = 0U;
    uint32_t contextLossState = 0U;
    uint32_t turnOn;
    uint32_t turnOff;

    status = Sciclient_pmGetModuleState(moduleId,
                                        &moduleState,
                                        &resetState,
                                        &contextLossState,
                                        SystemP_WAIT_FOREVER);

    turnOn = (moduleState == TISCI_MSG_VALUE_DEVICE_HW_STATE_OFF) &&
             (requiredState == TISCI_MSG_VALUE_DEVICE_SW_STATE_ON);
    turnOff = (moduleState == TISCI_MSG_VALUE_DEVICE_HW_STATE_ON) &&
              (requiredState == TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF);

    if (turnOn || turnOff)
    {
        status = Sciclient_pmSetModuleState(moduleId,
                                            requiredState,
                                            (appFlags |
                                             TISCI_MSG_FLAG_AOP |
                                             TISCI_MSG_FLAG_DEVICE_RESET_ISO),
                                             SystemP_WAIT_FOREVER);
        if (requiredState == TISCI_MSG_VALUE_DEVICE_SW_STATE_ON)
        {
            /* Reset if changed state to enabled */
            status = Sciclient_pmSetModuleRst(moduleId,
                                              0x0U /*resetBit*/,
                                              SystemP_WAIT_FOREVER);
            (void)status;
        }
    }
}

int32_t BoardUtils_enableSerDes(uint32_t serdesInstance)
{
    uint32_t moduleId = TISCI_DEV_SERDES_10G0;
    uint32_t appFlags = 0U;
    uint32_t coreRefClkId = TISCI_DEV_SERDES_10G0_CORE_REF_CLK;
    uint32_t coreRefClkPar = TISCI_DEV_SERDES_10G0_CORE_REF_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK;
    uint64_t coreRefClkHz = 100000000;
    uint64_t currClkFreqHz;
    int32_t status = CSL_PASS;

    if(serdesInstance == CSL_TORRENT_SERDES0)
    {
        moduleId = TISCI_DEV_SERDES_10G0;
    }
    else
    {
        moduleId = TISCI_DEV_SERDES_10G1;
    }
    /* Ensure the parent clock is at the desired frequency */
    status = PMLIBClkRateGet(moduleId, coreRefClkId, &currClkFreqHz);
    if ((status == CSL_PASS) &&
        (currClkFreqHz != coreRefClkHz))
    {
        status = PMLIBClkRateSet(moduleId, coreRefClkId, coreRefClkHz);
        if (status != CSL_PASS)
        {
            return  status;
        }
    }
    else
    {
        return  status;
    }

    /* Reparent to MAIN_PLL3_HSDIV4 or MAIN_PLL2_HSDIV4 depending on the req ref clock frequency */
    status = Sciclient_pmSetModuleClkParent(moduleId, coreRefClkId, coreRefClkPar, SystemP_WAIT_FOREVER);

    BoardUtils_setDeviceState(moduleId, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, appFlags);
    return  status;
}

int32_t PMLIBClkRateGet(    uint32_t modId,
                            uint32_t clkId,
                            uint64_t *clkRate)
{
    int32_t retval   = CSL_EFAIL;
    
    retval = Sciclient_pmGetModuleClkFreq(  modId,
                                            clkId,
                                            clkRate,
                                            SystemP_WAIT_FOREVER);

    return retval;
}

int32_t PMLIBClkRateSet(uint32_t modId,
                        uint32_t clkId,
                        uint64_t clkRate)
{
    uint32_t i = 0U;
    int32_t status   = CSL_EFAIL;
    int32_t finalStatus = CSL_EFAIL;
    uint64_t respClkRate = 0;
    uint32_t numParents = 0U;
    uint32_t moduleClockParentChanged = 0U;
    uint32_t clockStatus = 0U;
    uint32_t origParent = 0U;
    uint32_t foundParent = 0U;

    /* Check if the clock is enabled or not */
    status = Sciclient_pmModuleGetClkStatus(modId,
                                            clkId,
                                            &clockStatus,
                                            SystemP_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        /* Get the number of parents for the clock */
        status = Sciclient_pmGetModuleClkNumParent(modId,
                                                clkId,
                                                &numParents,
                                                SystemP_WAIT_FOREVER);
        if ((status == CSL_PASS) && (numParents > 1U))
        {
            status = Sciclient_pmGetModuleClkParent(modId, clkId, &origParent,
                                       SystemP_WAIT_FOREVER);
        }
    }
    if (status == CSL_PASS)
    {
        /* Disabling the clock */
        status = Sciclient_pmModuleClkRequest(
                                            modId,
                                            clkId,
                                            TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ,
                                            0U,
                                            SystemP_WAIT_FOREVER);
    }
    if (status == CSL_PASS)
    {
        foundParent = 0U;
        /* Try to loop and change parents of the clock */
        for(i=0U;i<numParents;i++)
        {
            if (numParents > 1U)
            {
                /* Setting the new parent */
                status = Sciclient_pmSetModuleClkParent(
                                            modId,
                                            clkId,
                                            clkId+i+1U,
                                            SystemP_WAIT_FOREVER);
                /* Check if the clock can be set to desirable freq. */
                if (status == CSL_PASS)
                {
                    moduleClockParentChanged = 1U;
                }
            }
            if (status == CSL_PASS)
            {
                status = Sciclient_pmQueryModuleClkFreq(modId,
                                                        clkId,
                                                        clkRate,
                                                        &respClkRate,
                                                        SystemP_WAIT_FOREVER);
            }
            if ((status == CSL_PASS) && (respClkRate == clkRate))
            {
                foundParent = 1U;
                break;
            }
        }
    }
    if ((status == CSL_PASS) && (numParents == 0U))
    {
        status = Sciclient_pmQueryModuleClkFreq(modId,
                                                clkId,
                                                clkRate,
                                                &respClkRate,
                                                SystemP_WAIT_FOREVER);
        if ((status == CSL_PASS) && (respClkRate == clkRate))
        {
            foundParent = 1U;
        }
    }
    if (foundParent == 1U)
    {
        /* Set the clock at the desirable frequency*/
        status = Sciclient_pmSetModuleClkFreq(
                                modId,
                                clkId,
                                clkRate,
                                TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
                                SystemP_WAIT_FOREVER);
    }
    else
    {
        status = CSL_EFAIL;
    }
    if ((status == CSL_PASS) &&
        (clockStatus == (uint32_t) TISCI_MSG_VALUE_CLOCK_SW_STATE_UNREQ))
    {
        /* Restore the clock again to original state */
        status = Sciclient_pmModuleClkRequest(
                                            modId,
                                            clkId,
                                            clockStatus,
                                            0U,
                                            SystemP_WAIT_FOREVER);
    }
    finalStatus = status;
    if ((status != CSL_PASS) && (moduleClockParentChanged == 1U))
    {
        /* Setting the original parent if failure */
        (void) Sciclient_pmSetModuleClkParent(
                                    modId,
                                    clkId,
                                    origParent,
                                    SystemP_WAIT_FOREVER);
    }
    return finalStatus;
}

uint32_t Board_serdesCfgSgmii(uint32_t serdesInstance)
{
    CSL_SerdesResult result;
    CSL_SerdesLaneEnableStatus laneRetVal = CSL_SERDES_LANE_ENABLE_NO_ERR;
    CSL_SerdesLaneEnableParams serdesLaneEnableParams  = {0};
    uint32_t serdesBaseAddr;

    if (serdesInstance == CSL_TORRENT_SERDES1)
    {
		  serdesBaseAddr = CSL_SERDES_10G1_BASE;
    }
    else
    {
		  serdesBaseAddr = CSL_SERDES_10G0_BASE;
    }

    memset(&serdesLaneEnableParams, 0, sizeof(serdesLaneEnableParams));

    /* SGMII Config */
    serdesLaneEnableParams.serdesInstance    = serdesInstance;
    serdesLaneEnableParams.baseAddr          = serdesBaseAddr;
    serdesLaneEnableParams.refClock          = CSL_SERDES_REF_CLOCK_100M;
    serdesLaneEnableParams.refClkSrc         = CSL_SERDES_REF_CLOCK_INT;
    serdesLaneEnableParams.linkRate          = CSL_SERDES_LINK_RATE_1p25G;
    serdesLaneEnableParams.numLanes          = 1U;
    serdesLaneEnableParams.laneMask          = 1U;
    serdesLaneEnableParams.SSC_mode          = CSL_SERDES_NO_SSC;
    serdesLaneEnableParams.phyType           = CSL_SERDES_PHY_TYPE_SGMII;
    serdesLaneEnableParams.operatingMode     = CSL_SERDES_FUNCTIONAL_MODE;
    serdesLaneEnableParams.phyInstanceNum    = 0U;
    serdesLaneEnableParams.pcieGenType       = CSL_SERDES_PCIE_GEN3;

    serdesLaneEnableParams.laneCtrlRate[0] = CSL_SERDES_LANE_FULL_RATE;
    serdesLaneEnableParams.loopbackMode[0] = CSL_SERDES_LOOPBACK_DISABLED;

    CSL_serdesPorReset(serdesLaneEnableParams.baseAddr);

    /* Select the IP type, IP instance num, Serdes Lane Number */
    CSL_serdesIPSelect(CSL_CTRL_MMR0_CFG0_BASE,
                       serdesLaneEnableParams.phyType,
                       serdesLaneEnableParams.phyInstanceNum,
                       serdesLaneEnableParams.serdesInstance,
                       0);

    result = CSL_serdesRefclkSel(CSL_CTRL_MMR0_CFG0_BASE,
                                 serdesLaneEnableParams.baseAddr,
                                 serdesLaneEnableParams.refClock,
                                 serdesLaneEnableParams.refClkSrc,
                                 serdesLaneEnableParams.serdesInstance,
                                 serdesLaneEnableParams.phyType);

    if (CSL_SERDES_NO_ERR != result)
    {
        return 1U;
    }
    /* Assert PHY reset and disable all lanes */
    CSL_serdesDisablePllAndLanes(serdesLaneEnableParams.baseAddr,
                                 serdesLaneEnableParams.numLanes,
                                 serdesLaneEnableParams.laneMask);

    /* Load the Serdes Config File */
    result = CSL_serdesEthernetInit(&serdesLaneEnableParams);
    /* Return error if input params are invalid */
    if (CSL_SERDES_NO_ERR != result)
    {
        return 1U;
    }

    /* Common Lane Enable API for lane enable, pll enable etc */
    laneRetVal = CSL_serdesLaneEnable(&serdesLaneEnableParams);
    if (0U != laneRetVal)
    {
        return 1U;
    }

    return 0U;
}