/*
 *
 * Copyright (c) 2024-2025 Texas Instruments Incorporated
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
 * \file ethfw_tsn.h
 *
 * \brief This file contains the private type definitions, helper macros and functions
 *        required for Ethernet Firmware TSN and gPTP support.
 */

#ifndef ETHFW_TSN_PRIV_H_
#define ETHFW_TSN_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! Interface name size */
#define ETHFW_TSN_IFNAMSIZ                                          (16U)

/*! Number of MAC ports supported by the gPTP stack */
#if defined(SOC_J721E) || defined(SOC_J784S4)
#define ETHFW_TSN_CFG_NUM_MAC_PORTS                                 (8U)
#elif defined(SOC_AM62DX) || defined(SOC_AM62PX)
#define ETHFW_TSN_CFG_NUM_MAC_PORTS                                 (2U)
#else
#define ETHFW_TSN_CFG_NUM_MAC_PORTS                                 (4U)
#endif

/*! Number of Uniconf files */
#define ETHFW_TSN_UC_CONF_FILE_NUM                                  (0U)
/*! Uniconf file path */
#define ETHFW_TSN_INTERFACE_CONFFILE_PATH                           (NULL)
/*! Uniconf database file path */
#define ETHFW_TSN_UC_DBFILE_PATH                                    (NULL)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*! Callback for setting gPTP config parameters from the application */
typedef void (*EthFwTsn_configPtpCb)(void *arg);

/*!
 * \brief PPS config parameters
 */
typedef struct EthFwTsn_PpsConfig_s
{
    /*! TSR input interrupt num */
    uint32_t tsrIn;

    /*! TSR output interrupt num */
    uint32_t tsrOut;

    /*! Frequency of PPS signal in Hz */
    uint64_t ppsFreqHz;

    /*! CPTS GenF instance to be used to generate PPS signal */
    uint32_t genfIdx;

} EthFwTsn_PpsConfig;

/*!
 * \brief Callback structure for gPTP config
 */
typedef struct EthFwTsn_gPTPConfigArg_s
{
    /*! gPTP instance number */
    uint8_t inst;

} EthFwTsn_gPTPConfigArg;

/*!
 * \brief Ethernet Firmware TSN config parameters
*/
typedef struct EthFwTsn_Config_s
{
    /*! Enet instance type */
    uint32_t enetType;

    /*! Enet instance id */
    uint32_t instId;

    /*! Callback for setting gPTP config parameters from the application */
    EthFwTsn_configPtpCb configPtpCb;

    /*! Argument to be passed to gPTP config callback function */
    void *configPtpCbArg;

    /*! PPS config params when enabled */
    EthFwTsn_PpsConfig ppsConfig;

} EthFwTsn_Config;

/*!
 * \brief Ethernet Firmware TSN net devices info
 */
typedef struct EthFwTsn_NetDevInfo_s
{
    /*! TSN stack netdevs */
    char netDevs[ETHFW_TSN_CFG_NUM_MAC_PORTS][ETHFW_TSN_IFNAMSIZ];

    /*! gPTP stack netdevs */
    char *gPtpNetDevs[ETHFW_TSN_CFG_NUM_MAC_PORTS + 1];

    /*! Number of active netdevs */
    uint32_t numNetDevs;
} EthFwTsn_NetDevInfo;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* TSN Module Task ID */
typedef enum
{
    ETHFWTSN_UNICONF_TASK_IDX,
    ETHFWTSN_GPTP_TASK_IDX,
#if (AVTP_ENABLED)
    ETHFWTSN_AVTPD_TASK_IDX,
    ETHFWTSN_AAF_AUTOAMP_APP_TX_CLASSA_TASK_IDX,
    ETHFWTSN_AUTOAMP_APP_RX_TASK_IDX,
#endif
#if defined(ETHFW_EST_DEMO_SUPPORT)
    ETHFWTSN_EST_TASK_IDX,
#endif
    ETHFWTSN_MAX_TASK_IDX
} EthFwTsn_TaskIdx;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize and enable EthFw gPTP stack.
 *
 * Initializes and enable the EthFw gPTP stack with the provided configuration parameters.
 *
 * \param hostMacAddr   Host port MAC address used in packets sent by TSN stack.
 *                      If NULL pointer is passed, TSN stack will allocate a MAC address
 *                      from Enet LLD MAC address pool for every port in the portMask.
 *                      If valid MAC address is passed, TSN stack will use the same MAC address
 *                      for packets sent on any port in portMask.
 * \param portMask      Mask of ports used for PTP. The mask is built by or-ing
 *                      ENET_MACPORT_MASK(macPort).
 *
 * \retval ENET_SOK if gPTP initialization was successful
 * \retval Negative error code if initialization failed
 */
int32_t EthFwTsn_initTimeSyncPtp(const uint8_t *hostMacAddr,
                                 uint32_t portMask);

/*!
 * \brief Initilize unibase and log buffer task configurations.
 *
 * EthFwTsn_initTimeSyncPtp should be called after this function to initilize
 * and start Uniconf, gPTP tasks.
 */
void EthFwTsn_init(EthFwTsn_Config *tsnCfg);

/*!
 * \brief De-initializes all the opened gPTP related tasks.
 *
 * Closes unibase, kills all the tasks, semaphores and mutexes.
 */
void EthFwTsn_deInit(void);

/*!
 * \brief start any TSN module
 *
 * This API takes the TSN module Idx as input and starts that task module
 * Returns ETHFW_SOK on successful start else ETHFW_EFAIL.
 */
int32_t EthFwTsn_startModule(uint32_t moduleIdx);

/*!
 * \brief Restart TSN module
 *
 * This API is called whenever a TSN module is stopped and restarted again.
 * Existing EthFwTsn_startModule cannot be used since restarting TSN module
 * requires a DB initializaion API call which is handled by this API.
 * This API takes the TSN module Idx as input and starts that task module.
 * Returns ETHFW_SOK on successful start else ETHFW_EFAIL.
 */
int EthFwTsn_restartTsnModule(int moduleIdx);

/*!
 * \brief Stop any TSN module
 *
 * This API takes the TSN module Idx as input and stops task module
 */
void EthFwTsn_stopModule(uint32_t moduleIdx);

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_TSN_PRIV_H_ */
