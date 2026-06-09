/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#ifndef __APP_ETHFW_STATS_H__
#define __APP_ETHFW_STATS_H__

#include <stdint.h>
#include <stdio.h>

/*!
 * \defgroup apps_ethfw_stats Ethernet Firmware statistics reporting APIs
 *
 * \brief These APIs allows user to get Ethernet firmware related information
 *
 * \ingroup app_ethfw_stats
 *
 * @{
 */

/*! \brief Number of statistics provided by CPSW IP */
#define APP_ETHFW_STATS_BLOCK_ELEM_NUM             (128U)

/*! \brief EthFw statistics service name */
#define APP_ETHFW_STATS_SERVICE_NAME  "com.ti.ethfw_stats"

/*! \brief Maximum number of Ethernet ports */
#define APP_ETHFW_PORT_NUM_MAX                     (9U)

/*! \brief Command to reset bandwidth data of all Ethernet ports*/
#define APP_ETHFW_STATS_CMD_RESET_BANDWIDTH        (0x0001)

/*! \brief Command to get bandwidth data for all Ethernet ports */
#define APP_ETHFW_STATS_CMD_GET_BANDWIDTH          (0x0002)

/*!
 * \brief CPSW nG port Bandwidth
 *
 * Structure with Tx & Rx Bandwidth information.
 */
typedef struct
{
    /*! Status of ethernet port: enabled/disabled (true/false)  */
    bool isportenabled[APP_ETHFW_PORT_NUM_MAX];

    /*! Tx bandwidth of Ethernet ports */
    uint64_t tx_bandwidth[APP_ETHFW_PORT_NUM_MAX];

    /*! Rx bandwidth of Ethernet ports */
    uint64_t rx_bandwidth[APP_ETHFW_PORT_NUM_MAX];
} app_ethfw_port_bandwidth_t;

/*! @} */

#endif

