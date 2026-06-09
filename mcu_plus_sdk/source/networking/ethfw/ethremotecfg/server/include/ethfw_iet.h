/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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
 * \file ethfw_iet.h
 *
 * \brief This file contains the type definitions, helper macros and functions
 *        required for Ethernet Firmware port mirroring support.
 */

#ifndef ETHFW_IET_H_
#define ETHFW_IET_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <enet.h>
#include <include/mod/cpsw_ale.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_SERVER
 * \defgroup ETHFW_IET_TYPES Support
 * @{
 */


/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! No of Iteration to run to verify IET capability before timeout */
#define ETHFW_NUM_IET_VERIFY_ATTEMPTS               (20U)

/*! Number of FIFO queues per macPort */
#define CPSW_MACPORT_FIFO                       (8)
/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief Ethernet Firmware IET config parameters
*/
typedef struct EthFwIET_Config_s
{

    /*! Minimum Fragment size */
    uint32_t minFragSize;

    /*! iet verification enabled/disabled */
    bool mac_verify_enable;

    /*! Traffic mode for each of the FIFO queues */
    uint32_t queueMode[CPSW_MACPORT_FIFO];

} EthFwIET_Config;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Notify Link Up down Event.
 *
 * Callback function to notify about mac port link up/down.
 *
 * \param macPort    macPort on which link changes is detected
 * \param isLinkUp   link is up or down
 */
void  EthFwIET_notifyLinkChange(const Enet_MacPort macPort, 
                                const bool isLinkUp);


/*!
 * \brief API to updated the global object with params
 
 * \param ietCfg    Params passed from application layer
 * \param hEnet     Ethernet handle
 * \param coreId    caller core id
 */
void EthFwIET_init(const EthFwIET_Config *ietCfg, 
                   Enet_Handle hEnet, 
                   uint32_t coreId);                                

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_IET_H_ */
