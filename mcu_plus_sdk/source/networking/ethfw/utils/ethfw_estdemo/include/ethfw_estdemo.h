/*
 *

 * Copyright (c) 2024-2026 Texas Instruments Incorporated
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
 * \file ethfw_estdemo.f
 *
 * \brief Header file of the EthFw EST demo app
 */

#ifndef __ETHFW_ESTDEMO_H__
#define __ETHFW_ESTDEMO_H__

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <tsn_buildconf/jacinto_buildconf.h>
#include <tsn_combase/combase.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define AVTP_VLAN_ID                    (110U)
#define AVTP_AAF_SUBTYPE                (2U)

/* Default interface configured for the application, please change it
 * if you want to test it for the port other than mac port 3 (tilld3)
 * This value holds the ondex of the netdevs defined for TSN
*/
#if defined(SOC_J721E) || defined(SOC_J7200)
#define DEFAULT_INTERFACE_INDEX         (1U)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define DEFAULT_INTERFACE_INDEX         (0U)
#elif defined(SOC_AM62PX) || defined(SOC_AM62DX) || defined(SOC_J722S)
#define DEFAULT_INTERFACE_INDEX         (0U)
#endif

#define MAX_KEY_SIZE                    (256U)
#define MAX_LOG_LEN                     (256U)
#define MAX_VAL_SIZE                    (64U)

#define ESTDEMO_MAX_STREAMS             (8U)
#define ESTDEMO_NUM_OF_STREAMS          (7U)
#define ESTDEMO_TASK_PRIORITY           (1U)
#define ESTDEMO_PRIORITY_MAX            (8U)

#define ESTDEMO_VLAN_TPID               (0x8100U)
#define ESTDEMO_VLAN_PCP_OFFSET         (13U)
#define ESTDEMO_VLAN_PCP_MASK           (0x7U)
#define ESTDEMO_VLAN_DEI_OFFSET         (12U)
#define ESTDEMO_VLAN_DEI_MASK           (0x1U)
#define ESTDEMO_VLAN_VID_MASK           (0xFFFU)
#define ESTDEMO_VLAN_TCI(pcp, dei, vid) ( (((pcp) & ESTDEMO_VLAN_PCP_MASK) << ESTDEMO_VLAN_PCP_OFFSET)| \
                                           (((dei) & ESTDEMO_VLAN_DEI_MASK)<< ESTDEMO_VLAN_DEI_OFFSET) | \
                                           ((vid) & ESTDEMO_VLAN_VID_MASK) )

UB_ABIT32_FIELD(cmsh_sv, 23, 0x1)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EstDemoCommonParam_s
{
    /*! Name of network interface */
    char *netdev;
    /*! index is priority, value of each index is TC, -1: not used. */
    int8_t priority2TcMapping[ESTDEMO_PRIORITY_MAX];
    uint8_t nTCs;                 /*! Num of traffic classes */
    uint8_t nQueues;              /*! Num of HW queue */
} EstDemoCommonParam;

typedef struct EstDemoBitrateCtrl_s
{
    uint32_t maxCapacity; /*!  Maximum bytes of the bucket */
    uint64_t bitRate;     /*!  bit per second. */
    uint32_t tokens;      /*!  Number of available tokens in bytes */
    uint64_t lastTs;      /*!  Timestamp of previous sent packet */
} EstDemoBitrateCtrl;

struct EstDemoCommonStreamParam
{
    uint8_t subtype; /*! Subtype of AVTP header */
    uint8_t bf0;
    uint8_t seqn;    /*! Sequence number of AVTP header */
    uint8_t bf1;
} __attribute__ ((packed));

typedef struct EstDemoCommonStreamHdr_s
{
    union
    {
        struct EstDemoCommonStreamParam hh;
        uint32_t bf;
    };
    ub_streamid_t streamId;   /*! Stream ID  */
    uint32_t headerTimestamp; /*! Timestamp of AVTP packet */
    uint32_t fsd2;
    uint16_t pdLength;        /*! packet data length */
    uint16_t fsd3;
} __attribute__ ((packed)) EstDemoCommonStreamHdr;

typedef struct EstDemoStreamParams_s
{
    ub_streamid_t sid; /*!  8 bytes stream id */
    /*! priority of traffic has on-to-one mapping with PCP(3 bits) */
    uint8_t priority;
    /*! traffic class of the stream */
    uint8_t tc;
    /*!  Sequence number of AVTP packet */
    uint8_t seqn;
    /*! Seceived timestamp of the received packet */
    uint64_t rxts;
    /*! Present timestamp in avtp header which
     * is captured when sending by application.
     */
    int64_t txts;
    /*! numof bytes received */
    uint64_t rxBytes;
    /*! The previous time it shows the bitrate */
    uint64_t prevTs;
    /*! bitRate of a stream. */
    uint64_t bitRate;
    /*!  Length of payload data in the Ethernet
     * packet for each stream.
     */
    uint32_t payloadLen;
    /*!  Num of broken packets. */
    uint32_t nBrokenPkt;
} EstDemoStreamParams;

typedef struct EstDemoTaskCfg_s
{
    /*! A string which indicate task name */
    char *name;
    /*! Priority of task */
    int8_t priority;
    /*! Start address of a buffer allocaing for task stack  */
    void *stackBuffer;
    /*! Size of buffer allocated for the stack */
    uint32_t stackBufferSize;
} EstDemoTaskCfg;

typedef struct EstDemoTaskCtx_s
{
    /*!  Handle of the task.*/
    CB_THREAD_T hTask;
    /*! true: Enable task running, false: disable task running. */
    bool enable;
    /*! Num of traffic classes */
    uint8_t nTCs;
    /*!  VLAN ID of the stream. */
    uint16_t vid;
    /*!  Num of streams for talker or listener. */
    uint16_t nStreams;
    /*! Buffer for sending or receiving ethernet packet */
    uint8_t buffer[sizeof(EthVlanFrameHeader)+ETH_PAYLOAD_LEN];
    /*! Info associated for each stream of this task */
    EstDemoStreamParams streams[ESTDEMO_MAX_STREAMS];
    /*! Handle of an object to control bitrate for the talker */
    EstDemoBitrateCtrl bitrateCtrl[ESTDEMO_MAX_STREAMS];
    /*! A semaphore to signal receiver thread for availability of packets */
    CB_SEM_T rxPacketSem;
    /*!  A semaphore to signal the parent thread when
     * child thread is terminated.
     */
    CB_SEM_T terminatedSem;
} EstDemoTaskCtx;

typedef struct EstDemoPacket_s
{
    /*! Start address of buffer for containing an ethernet packet */
    void *buffer;
    /*! Size of buffer */
    uint32_t bufferSize;
    /*! Meta info of the buffer */
    CB_SOCKADDR_LL_T recvAddr;
} EstDemoPacket;

/*! Callback for notifying packet receiption to higher layer application */
typedef void (*PacketHandlerCb)(void *ctx, EstDemoPacket *pkt);

typedef struct EstDemoAppCtx_s
{
    /*! Socket handle for entire application */
    CB_SOCKET_T est_sock;
    /*! Source address associated with the socket above. */
    ub_macaddr_t source_mac;
    /*! Address associated with the socket above. */
    CB_SOCKADDR_LL_T sockAddress;
    /*! A handle of talker of the app */
    EstDemoTaskCtx talker;
    /*! A handle of listener of the app */
    EstDemoTaskCtx listener;
    /*! An active network interface used for this app */
    char *netdev[MAX_NUMBER_ENET_DEVS];
    /*! How many network interfaces */
    int32_t netdevSize;
    /*! A delay offset for applying a schedule in microsecond unit */
    int64_t adminDelayOffset;
    /*! Default interface index of the network interface for the application */
    int32_t ifidx;
    /*! A pointer to a context object. */
    void *ectx;
    /*! Callback to notify application on packet reception */
    PacketHandlerCb packetHandlerCb;
} EstDemoAppCtx;

typedef struct EstDemoStreamCfgParams_s
{
    /*! Bitrate of the stream in Kbps */
    uint32_t bitRateKbps;
    /*! Length of payload */
    uint32_t payloadLen;
    /*! Traffic class ID */
    uint8_t tc;
    /*! Priority of traffic (PCP) */
    uint8_t priority;
} EstDemoStreamCfgParams;

typedef struct EstDemoStreamConfig_s
{
    /*! Config parameters of the streams to be run */
    EstDemoStreamCfgParams streamParams[ESTDEMO_MAX_STREAMS];
    /*! How many streams requested by user */
    int nStreams;
} EstDemoStreamConfig;


/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_ESTDEMO_H_ */
