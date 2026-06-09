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
 * \file  cpsw_macport_iet.c
 *
 * \brief This file contains the implementation of the CPSW IET functionality.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EnetTrace id for this module, must be unique within Enet LLD */
#define ENETTRACE_MOD_ID 0x209

#include <stdint.h>
#include <stdarg.h>
#include <csl_cpswitch.h>
#include <enet_cfg.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_soc.h>
#include <include/mod/cpsw_macport.h>
#include <priv/core/enet_trace_priv.h>
#include <priv/mod/cpsw_macport_priv.h>
#include <include/per/cpsw_clks.h>

#if ENET_CFG_IS_ON(CPSW_MACPORT_IET)

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! \brief CPSW IET Verify timeout for 1 gig mode is 10ms */
#define CPSW_MACPORT_GIGABIT_IET_VERIFY_TIMEOUT_NS      (10*1000*1000)

/*! \brief CPSW Rx FIFO Blocks to enable IET. */
#define CPSW_MACPORT_RX_MAX_BLKS_IET                    (7U)

/*! \brief CPSW Tx FIFO Blocks to enable IET. */
#define CPSW_MACPORT_TX_MAX_BLKS_IET                    (13U)

/*! \brief CPSW default Rx FIFO Blocks for fifo_oneram */
#define CPSW_MACPORT_RX_MAX_BLKS_DEFAULT                (4U)

/*! \brief CPSW default Tx FIFO Blocks for fifo_oneram */
#define CPSW_MACPORT_TX_MAX_BLKS_DEFAULT                (16U)

/*! Time interval step in nsecs for 1 Gbps link */
#define CPSW_MACPORT_IET_TIME_STEP_1G                   (8U)

/*! Time interval step in nsecs for 100 Mbps link */
#define CPSW_MACPORT_IET_TIME_STEP_100M                 (40U)

/*! Time interval step in nsecs for a 10 Mbps link */
#define CPSW_MACPORT_IET_TIME_STEP_10M                  (400U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void CpswMacPort_enableIET(CSL_Xge_cpswRegs *regs,
                                  uint32_t portNum);

static void CpswMacPort_disableIET(CSL_Xge_cpswRegs *regs,
                                   uint32_t portNum);

static void CpswMacPort_isPreemptionEnabled(CSL_Xge_cpswRegs *regs,
                                            uint32_t portNum,
                                            bool *enabled);

static void CpswMacPort_isPreemptionActive(CSL_Xge_cpswRegs *regs,
                                           uint32_t portNum,
                                           bool *active);

static int32_t CpswMacPort_enableIetVerify(CSL_Xge_cpswRegs *regs,
                                           uint32_t portNum,
                                           Enet_Speed speed);

static void CpswMacPort_disableIetVerify(CSL_Xge_cpswRegs *regs,
                                         uint32_t portNum);

static bool CpswMacPort_isPremptVerifyEnabled(CSL_Xge_cpswRegs *regs,
                                              uint32_t portNum);

static void CpswMacPort_getPremptVerifyStatus(CSL_Xge_cpswRegs *regs,
                                              uint32_t portNum,
                                              EnetMacPort_PreemptVerifyStatus *verifyStatus);

static int32_t CpswMacPort_setPreemptQueue(CSL_Xge_cpswRegs *regs,
                                           uint32_t portNum,
                                           EnetMacPort_QueuePreemptCfg *queuePreemptCfg);

static void CpswMacPort_getQueuePreemptStatus(CSL_Xge_cpswRegs *regs,
                                              uint32_t portNum,
                                              EnetMacPort_QueuePreemptCfg *queuePreemptCfg);

static bool isPreemptFragWorkaroundNeeded(Enet_Type enetType,
                                             Enet_MacPort macPort);

#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

#if ENET_CFG_IS_ON(CPSW_MACPORT_IET)
static bool isPreemptFragWorkaroundNeeded(Enet_Type enetType, Enet_MacPort macPort)
{
    #if ENET_CFG_IS_ON(CPSW_MACPORT_IET_PREEMPT_FRAG_WORKAROUND)
        uint32_t portMask = (1U << macPort);
        #if defined(SOC_J721E) || defined(SOC_J784S4)
            return ((enetType == ENET_CPSW_9G && macPort >= ENET_MAC_PORT_3) &&
                    (portMask & ENET_CFG_IET_PREEMPT_FRAG_ERRATA_PORT_MASK));
        #elif defined(SOC_J7200) || defined(SOC_J721S2)
            return ((enetType == ENET_CPSW_5G && macPort >= ENET_MAC_PORT_2) &&
                    (portMask & ENET_CFG_IET_PREEMPT_FRAG_ERRATA_PORT_MASK));
        #else
            return false;
        #endif
    #else
        return false;
    #endif
}

int32_t CpswMacPort_ioctlIet(EnetMod_Handle hMod,
                             uint32_t cmd,
                             Enet_IoctlPrms *prms)
{
    CpswMacPort_Handle hPort = (CpswMacPort_Handle)hMod;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    Enet_MacPort macPort = hPort->macPort;
    uint32_t portNum = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portNum);

    switch(cmd)
    {
        case ENET_MACPORT_IOCTL_ENABLE_PREEMPTION:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_enableIET(regs, portNum);
        }
        break;

        case ENET_MACPORT_IOCTL_DISABLE_PREEMPTION:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_disableIET(regs, portNum);
        }
        break;

        case ENET_MACPORT_IOCTL_GET_PREEMPTION_ENABLE_STATUS:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            bool *enabled = (bool *)prms->outArgs;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_isPreemptionEnabled(regs, portNum, enabled);
        }
        break;

        case ENET_MACPORT_IOCTL_GET_PREEMPTION_ACTIVE_STATUS:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            bool *active = (bool *)prms->outArgs;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);
            CpswMacPort_isPreemptionActive(regs, portNum, active);
        }
        break;

        case ENET_MACPORT_IOCTL_ENABLE_PREEMPT_VERIFICATION:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            Enet_Speed speed = hPort->linkCfg.speed;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);
            CpswMacPort_enableIetVerify(regs, portNum, speed);
        }
        break;

        case ENET_MACPORT_IOCTL_DISABLE_PREEMPT_VERIFICATION:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_disableIetVerify(regs, portNum);
        }
        break;

        case ENET_MACPORT_IOCTL_GET_PREEMPT_VERIFY_STATUS:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            EnetMacPort_PreemptVerifyStatus *verifyStatus = (EnetMacPort_PreemptVerifyStatus *)prms->outArgs;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);
            if (CpswMacPort_isPremptVerifyEnabled(regs, portNum))
            {
                CpswMacPort_getPremptVerifyStatus(regs, portNum, verifyStatus);
            }
            else
            {
                *verifyStatus = ENET_MAC_VERIFYSTATUS_DISABLED;
            }
        }
        break;

        case ENET_MACPORT_IOCTL_SET_PREEMPT_MIN_FRAG_SIZE:
        {
            EnetMacPort_SetPreemptMinFragSizeInArgs *inArgs = (EnetMacPort_SetPreemptMinFragSizeInArgs *)prms->inArgs;
            CSL_CPSW_IET_CONTROL ietControl;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
            uint8_t fragSize = inArgs->preemptMinFragSize;

            if (isPreemptFragWorkaroundNeeded(hPort->enetType, inArgs->macPort) && fragSize == 0U)
            {
                /* Workaround : refer HW errata i2208 
                Setting the Mininum fragment size to 128 as setting it 64 results in Express Packet Drops at 2.5G speed*/
                fragSize = 1;
                ENETTRACE_WARN("Errata i2208: RX min fragment size cannot be less than 128");
            }
            ietControl.macAddFragSize = fragSize;
            CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
        }
        break;

        case ENET_MACPORT_IOCTL_GET_PREEMPT_MIN_FRAG_SIZE:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            uint8_t *preemptMinFragSize = (uint8_t *)prms->outArgs;
            CSL_CPSW_IET_CONTROL ietControl;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
            *preemptMinFragSize = ietControl.macAddFragSize;
        }
        break;

        case ENET_MACPORT_IOCTL_SET_PREEMPT_QUEUE:
        {
            EnetMacPort_SetPreemptQueueInArgs *inArgs = (EnetMacPort_SetPreemptQueueInArgs *)prms->inArgs;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_setPreemptQueue(regs, portNum, &inArgs->queuePreemptCfg);
        }
        break;

        case ENET_MACPORT_IOCTL_GET_QUEUE_PREEMPT_STATUS:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            EnetMacPort_QueuePreemptCfg *queuePreemptCfg = (EnetMacPort_QueuePreemptCfg *)prms->outArgs;

            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CpswMacPort_getQueuePreemptStatus(regs, portNum, queuePreemptCfg);
        }
        break;

        case ENET_MACPORT_IOCTL_IET_HOLD_PREEMPT_TRAFFIC:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            CSL_CPSW_IET_CONTROL ietControl;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
            ietControl.macHold = 1U;
            CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
        }
        break;

        case ENET_MACPORT_IOCTL_IET_RELEASE_PREEMPT_TRAFFIC:
        {
            EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
            CSL_CPSW_IET_CONTROL ietControl;
            Enet_devAssert(macPort == inArgs->macPort,
                           "MAC %u: Port mismatch %u\n", portNum, inArgs->macPort);

            CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
            ietControl.macHold = 0U;
            CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
        }
        break;

        default:
        {
            status = ENET_EINVALIDPARAMS;
            ENETTRACE_ERR(status, "Invalid IOCTL cmd 0x%08x", cmd);
        }
        break;
    }

    return status;
}

static void CpswMacPort_enableIET(CSL_Xge_cpswRegs *regs, uint32_t portNum)
{
    CSL_CPSW_PORT_CONTROL portControl;
    CSL_CPSW_IET_CONTROL ietControl;

    CSL_CPSW_setPortMaxBlksReg(regs, portNum, CPSW_MACPORT_RX_MAX_BLKS_IET, CPSW_MACPORT_TX_MAX_BLKS_IET);
    /* Set IET enable in Port Control reg pn_iet_port_en */
    CSL_CPSW_getPortControlReg(regs, portNum, &portControl);
    portControl.ietPortEnable = 1U;
    CSL_CPSW_setPortControlReg(regs, portNum, &portControl);

    /* Enable mac preemption in port IET control reg */
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    ietControl.macPremptEnable = 1U;
    CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
}

static void CpswMacPort_disableIET(CSL_Xge_cpswRegs *regs, uint32_t portNum)
{
    CSL_CPSW_PORT_CONTROL portControl;
    CSL_CPSW_IET_CONTROL ietControl;

    /* Set the Port Max Blks to default values */
    CSL_CPSW_setPortMaxBlksReg(regs, portNum, CPSW_MACPORT_RX_MAX_BLKS_DEFAULT, CPSW_MACPORT_TX_MAX_BLKS_DEFAULT);
    /* Disable IET enable in Port Control reg pn_iet_port_en */
    CSL_CPSW_getPortControlReg(regs, portNum, &portControl);
    portControl.ietPortEnable = 0U;
    CSL_CPSW_setPortControlReg(regs, portNum, &portControl);

    /* Disable mac preemption in port IET control reg */
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    ietControl.macPremptEnable = 0U;
    CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
}

static void CpswMacPort_isPreemptionEnabled(CSL_Xge_cpswRegs *regs, uint32_t portNum, bool *enabled)
{
    CSL_CPSW_IET_CONTROL ietControl;
    CSL_CPSW_CONTROL cpswControl;

    /* Check if the enable bits are set */
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    CSL_CPSW_getCpswControlReg(regs, &cpswControl);
    *enabled = ietControl.macPremptEnable & cpswControl.ietEnable;
}

static void CpswMacPort_isPreemptionActive(CSL_Xge_cpswRegs *regs, uint32_t portNum, bool *active)
{
    CSL_CPSW_IET_CONTROL ietControl;
    bool enabled = BFALSE;
    *active = BFALSE;
    CSL_CPSW_IET_STATUS ietStatus;

    /*
     * Packets will be sent to the prempt MAC only
     * when pn_mac_penable is set, and when mac_verified (from Enet_Pn_IET_Status)
     * or pn_mac_disableverify is set, and when pn_iet_port_en is set.
     */
    CpswMacPort_isPreemptionEnabled(regs, portNum, &enabled);
    if(enabled)
    {
        CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
        if(ietControl.macDisableVerify == 1U)
        {
            *active = BTRUE;
        }
        else
        {
            CSL_CPSW_PortIetStatus(regs, portNum, &ietStatus);
            *active = ietStatus.macVerified;
        }
    }

}

static int32_t CpswMacPort_enableIetVerify(CSL_Xge_cpswRegs *regs,
                                           uint32_t portNum,
                                           Enet_Speed speed)
{
    uint32_t timeIntvlStep;
    uint32_t verifyTimeoutNs;
    uint32_t timeoutCnt;
    CSL_CPSW_IET_CONTROL ietControl;
    int32_t status = ENET_SOK;

    switch (speed)
    {
        case ENET_SPEED_10MBIT:
            timeIntvlStep = CPSW_MACPORT_IET_TIME_STEP_10M;
            verifyTimeoutNs = 100 * CPSW_MACPORT_GIGABIT_IET_VERIFY_TIMEOUT_NS;
            break;
        case ENET_SPEED_100MBIT:
            timeIntvlStep = CPSW_MACPORT_IET_TIME_STEP_100M;
            verifyTimeoutNs = 10 * CPSW_MACPORT_GIGABIT_IET_VERIFY_TIMEOUT_NS;
            break;
        case ENET_SPEED_1GBIT:
        default:
            timeIntvlStep = CPSW_MACPORT_IET_TIME_STEP_1G;
            verifyTimeoutNs = CPSW_MACPORT_GIGABIT_IET_VERIFY_TIMEOUT_NS;
            break;
    }

    timeoutCnt = ENET_DIV_ROUNDUP(verifyTimeoutNs, timeIntvlStep);
    CSL_CPSW_setPortIetVerifyTimeout(regs, portNum, timeoutCnt);

    /*
     * Toggle macLinkFail bit to restart verification process.
     * i.e Set macLinkFail to 1 and make it to 0 to reset and start,
     * It needs to be set only at least for one clock cycle.
     */
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    ietControl.macDisableVerify = 0U;
    ietControl.macPremptEnable = 1U;
    ietControl.macLinkFail = 1U;
    CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    ietControl.macLinkFail = 0U;
    CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
    return status;
}

static void CpswMacPort_disableIetVerify(CSL_Xge_cpswRegs *regs, uint32_t portNum)
{
    /* This is forced mode with no IET verification. */
    CSL_CPSW_IET_CONTROL ietControl;
    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    ietControl.macDisableVerify = 1U;
    CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
}

static bool CpswMacPort_isPremptVerifyEnabled(CSL_Xge_cpswRegs *regs, uint32_t portNum)
{
    CSL_CPSW_IET_CONTROL ietControl;
    bool enabled = BFALSE;

    /* Check if preemption is enabled */
    CpswMacPort_isPreemptionEnabled(regs, portNum, &enabled);
    if(enabled)
    {
        /* Verify and response frames will be sent/allowed only
         * when macLinkFail is set to zero and macDisableVerify is set to zero.*/
        CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
        if( (ietControl.macDisableVerify == 1U) || (ietControl.macLinkFail == 1U) )
        {
            enabled = BFALSE;
        }
    }
    return enabled;
}

static void CpswMacPort_getPremptVerifyStatus(CSL_Xge_cpswRegs *regs,
                                              uint32_t portNum,
                                              EnetMacPort_PreemptVerifyStatus *verifyStatus)
{
    CSL_CPSW_IET_STATUS ietStatus;

    CSL_CPSW_PortIetStatus(regs, portNum, &ietStatus);

    if (ietStatus.macVerified == 1U)
    {
        *verifyStatus = ENET_MAC_VERIFYSTATUS_SUCCEEDED;
    }
    else if (ietStatus.macRxRespondErr == 1U)
    {
        *verifyStatus = ENET_MAC_VERIFYSTATUS_RXRESPOND_ERROR;
    }
    else if (ietStatus.macRxVerifyErr == 1U)
    {
        *verifyStatus = ENET_MAC_VERIFYSTATUS_RXVERIFY_ERROR;
    }
    else if (ietStatus.macVerifyFail == 1U)
    {
        *verifyStatus = ENET_MAC_VERIFYSTATUS_FAILED;
    }
    else
    {
        *verifyStatus = ENET_MAC_VERIFYSTATUS_UNKNOWN;
    }
}

static int32_t CpswMacPort_setPreemptQueue(CSL_Xge_cpswRegs *regs,
                                           uint32_t portNum,
                                           EnetMacPort_QueuePreemptCfg *queuePreemptCfg)
{
    int32_t status = ENET_SOK;
    uint32_t queueIdx;
    uint32_t queueMask = 0;
    CSL_CPSW_IET_CONTROL ietControl;

    for (queueIdx = 0U; queueIdx < ENET_PRI_NUM; queueIdx++)
    {
        if(queuePreemptCfg->preemptMode[queueIdx] == ENET_MAC_QUEUE_PREEMPT_MODE_PREEMPT)
        {
            queueMask |= (1U << queueIdx);
        }
        else if(queuePreemptCfg->preemptMode[queueIdx] == ENET_MAC_QUEUE_PREEMPT_MODE_EXPRESS)
        {
            continue;
        }
        else
        {
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
        ietControl.macPremptQueue = queueMask;
        CSL_CPSW_setPortIetControlReg(regs, portNum, &ietControl);
    }

    return status;

}

static void CpswMacPort_getQueuePreemptStatus(CSL_Xge_cpswRegs *regs,
                                              uint32_t portNum,
                                              EnetMacPort_QueuePreemptCfg *queuePreemptCfg)
{
    uint32_t queueMask = 0;
    uint32_t queueIdx;
    CSL_CPSW_IET_CONTROL ietControl;

    CSL_CPSW_getPortIetControlReg(regs, portNum, &ietControl);
    queueMask = ietControl.macPremptQueue;

    for (queueIdx = 0U; queueIdx < ENET_PRI_NUM; queueIdx++)
    {
        if( (queueMask & (1U << queueIdx)) != 0U)
        {
            queuePreemptCfg->preemptMode[queueIdx] = ENET_MAC_QUEUE_PREEMPT_MODE_PREEMPT;
        }
        else
        {
            queuePreemptCfg->preemptMode[queueIdx] = ENET_MAC_QUEUE_PREEMPT_MODE_EXPRESS;
        }
    }

}
#endif
