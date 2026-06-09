/*
 *
 * Copyright (c) 2021-2024 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*!
 * \file ethfw_monitor.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware monitor support.
 */

#ifndef ETHFW_MONITOR_H_
#define ETHFW_MONITOR_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <utils/ethfw_common/include/ethfw_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_SERVER_MONITOR Monitor Support
 * @{
 *
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \anchor EthFw_StatsMonEvt
 * \name Statistics Monitor Events
 *
 * Event flags reported by Ethernet Firmware when specific statistics being monitored
 * had increased.
 *
 * @{
 */
/*! CPSW Rx Bottom of FIFO Drop - Frames received on a port overran the port's receive
 *  FIFO and were dropped. */
#define ETHFW_STATSMON_RXBOTTOMOFFIFODROP        ETHFW_BIT(0)
/*! CPSW Rx Top of FIFO Drop - Frames received on a port that had a start-of-frame (SOF)
 *  overrun on any destination port egress (when attempting to load a packet from the
 *  top of the ingress port receive FIFO into any other port's transmit FIFO). */
#define ETHFW_STATSMON_RXTOPOFFIFODROP           ETHFW_BIT(1)
/*! CPSW Transmit Priority 0-7 Drop - Frames on the port that overran the transmit FIFO
 *  priority 0-7 and were dropped. */
#define ETHFW_STATSMON_TXPRIDROP                 ETHFW_BIT(2)
/*! @} */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Statistics being monitored.
 */
typedef struct EthFwMon_Stats_s
{
    /*! RX bottom of FIFO drop counter */
    uint64_t rxBottomOfFifoDrop;

    /*! RX top of FIFO drop counter */
    uint64_t rxTopOfFifoDrop;

    /*! TX priority drop counters */
    uint64_t txPriDrop[ENET_PRI_NUM];
} EthFwMon_Stats;

/*! Callback for closing the Lwip DMA channels from the application */
typedef void (*EthFw_closeLwipDmaCb)(void *arg);

/*! Callback for opening the Lwip DMA channels from the application */
typedef void (*EthFw_openLwipDmaCb)(void *arg);

/*! Callback when a host port monitored statistics counter has increased */
typedef void (*EthFw_StatsMonHostPortEvtCb)(uint32_t evtMask,
                                            const EthFwMon_Stats *monStats,
                                            const CpswStats_HostPort_Ng *stats,
                                            void *arg);

/*! Callback when a MAC port monitored statistics counter has increased */
typedef void (*EthFw_StatsMonMacPortEvtCb)(Enet_MacPort macPort,
                                           uint32_t evtMask,
                                           const EthFwMon_Stats *monStats,
                                           const CpswStats_MacPort_Ng *stats,
                                           void *arg);

/*!
 * \brief Monitor and recovery configuration parameters.
 */
typedef struct EthFwMon_Cfg_s
{
    /*! Monitor period in milliseconds */
    uint32_t periodInMsecs;

    /*! Argument for opening or closing the Lwip DMA channel to be passed ot callback function */
    void *lwipDmaCbArg;

    /*! Callback for closing the Lwip DMA channels from the application */
    EthFw_closeLwipDmaCb closeLwipDmaCb;

    /*! Callback for closing the Lwip DMA channels from the application */
    EthFw_openLwipDmaCb openLwipDmaCb;

    /*! Callback to receive notifications when a host port monitored statistics has increased */
    EthFw_StatsMonHostPortEvtCb statsMonHostEvtCb;

    /*! Callback to receive notifications when a MAC port monitored statistics has increased */
    EthFw_StatsMonMacPortEvtCb statsMonMacEvtCb;

    /*! Argument passed in statistics monitor callbacks */
    void *statsMonCbArg;
} EthFwMon_Cfg;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Start EthFw Monitor.
 *
 * \returns \ref ETHFW_SOK if monitor task is successfully created.
 */
int32_t EthFwMon_startTask(void);

/*!
 * \brief Stop EthFw Monitor.
 */
void EthFwMon_stopTask(void);

/*!
 * \brief Initialize EthFw Monitor configuration parameters.
 *
 * \param monCfg    Configuration parameters to be initialized
 */
void EthFwMon_initCfg(EthFwMon_Cfg *monCfg);

/*!
 * \brief Initialize EthFw Monitor.
 *
 * Initializes EthFw Monitor. User should call \ref EthFwMon_Cfg
 * to initialize configuration parameters and make updates if needed
 * before calling this function.
 *
 * \param monCfg    Configuration parameters
 * \param enetType  EnetType
 * \param instId    instance Id
 * \param numPorts  Number of ports opened by EthFw
 *
 * \returns ETHFW_SOK if EthFw Monitor initialization was successful
 */
int32_t EthFwMon_init(const EthFwMon_Cfg *monCfg,
                      Enet_Type enetType,
                      uint32_t instId,
                      uint32_t numPorts);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_MONITOR_H_ */
