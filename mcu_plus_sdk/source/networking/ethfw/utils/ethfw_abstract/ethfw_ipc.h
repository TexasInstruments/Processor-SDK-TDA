/*
 *  Copyright (c) Texas Instruments Incorporated 2024-2025
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
 * \file  ethfw_ipc.h
 *
 * \brief Ethernet Firmware IPC interface.
 */

#ifndef ETHFW_IPC_H_
#define ETHFW_IPC_H_
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ethfw_al.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup ETHFW_IPC_ABSTRACT Ethernet Firmware IPC Abstraction APIs
 *
 * This section contains Ethernet Firmware IPC APIs. This is a common header
 * file for both Jacinto and Sitara which holds macros and declarations for IPC
 * related APIs.
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_IPC_ABSTRACT
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! Invalid RPMessage endpoint value */
#define ETHFW_IPC_INVALID_RPMSG_ENDPOINT           (~1U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief RPMessage create params
 *
 * Common RPMessage params for Jacinto and Sitara
 */
typedef struct EthFwIpc_RpmsgCreateParams_s
{
    /*! Requested Endpoint - Any or next available */
    uint32_t  requestedEndpt;
    /*! Maximum number of buffers to allocate for queuing received messages. */
    uint32_t  numBufs;
    /*!  Buffer pointer to store RPMessage Object */
    void*     buf;
    /*! Buffer Size. Recommended Size is (512*256 + 256) */
    uint32_t  bufSize;
} EthFwIpc_RpmsgCreateParams;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/*!
 * \brief ETHFW IPC RPMessage Handle
 */
typedef void *EthFwIpc_RpmsgHandle;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief ETHFW IPC initialization
 *
 * For Jacinto based caller, this API initialize IPC modile. This is the very first
 * API to be invoked to initialize internal data structure and utilities required.
 * For Sitara, it sets global pool for RPMessage Objects to zero. It also initializes
 * the local remotecore table, which holds details of the remote cores which will
 * participate in IPC communication with ETHFW.
 *
 * \param selfId remotecore ID of ETHFW
 * \param numProc number of cores participating in IPC communication with ETHFW
 * \param procArray Array of cores participating in IPC communication with ETHFW
 * \param func For Jacinto this function pointer is used to print debug/info message, if not NULL.
 *             For Sitara this would be a don't care as this is not supported for MCU+SDK.
 *
 * \return Status of the function
 *
 */
int32_t EthFwIpc_init(uint32_t selfId,
                      uint16_t numProc,
                      uint32_t procArray[IPC_MAX_PROCS],
                      void (*func)(const char *str));

/*!
 * \brief ETHFW IPC VirtIO initialization
 *
 * For Jacinto based caller, this API initializes VRings for Tx and Rx for all the participating
 * remote cores in IPC communication. Incase of Sitara, this is an empty function as this is already
 * handled by sysconfig auto-generated code.
 *
 * \param numProc number of cores participating in IPC communication with ETHFW
 * \param vqObj base address of the vq object
 * \param vringAddr base address of the shared vring of all the cores
 * \param vringBufSize buffer size of the shared vring
 *
 * \return Status of the function
 *
 */
int32_t EthFwIpc_initVirtIO(uint16_t numProc,
                            void *vqObj,
                            void *vringAddr,
                            uint32_t vringBufSize);

/*!
 * \brief ETHFW IPC Register Control Endpoint Callback
 *
 * For MCU+ SDK environment, this function registers the control endpoint callback
 * which is necessary for receiving announce endpoints.
 * For Jacinto, this is an empty function and does nothing.
 *
 */
void EthFwIpc_enableControlEndPt();

/*!
 * \brief ETHFW IPC Reset Object
 *
 * For MCU+ SDK environment, this function resets gEthFwIpcObj.
 * For Jacinto, this is an empty function and does nothing.
 *
 */
void EthFwIpc_resetObj();

/*!
 * \brief ETHFW IPC RPMessage Module initialization
 *
 * For Jacinto based caller, this API initializes RPMessage module and must be called before calling
 * any other RPMessage functions. Incase of Sitara, this function registers for control endpoint callback
 * which is necessary for receiving announce endpoints.
 *
 * \param rpmsgBuff Buffer pointer to store RPMessage Object
 * \param taskBuff Buffer used by stack for control task
 * \param selfCoreId remotecore Id of ETHFW
 *
 * \return Status of the function
 *
 */
int32_t EthFwIpc_initRpmsg(void *rpmsgBuff, void *taskBuff, uint32_t selfCoreId);

/*!
 * \brief ETHFW IPC Remote Ready
 *
 * API to check if remote core (Linux) has booted and is ready
 *
 * \param coreId remotecoreId of the remotecore user wants to check if ready. This arguments is currently
 *               a don't care because MCU+SDK doesn't support for any other core other than Linux.
 * \param timeout In case of Sitara user can specify timeout for this check function, but for Jacinto this
 *                API waits forever till Linux is ready.
 *
 * \return Status of the function
 */
int32_t EthFwIpc_isRemoteReady(uint32_t coreId, uint32_t timeout);

/*!
 * \brief ETHFW IPC Late initialization for a remote core
 *
 * IPC late init for a remote core. This API is used once Linux is ready.
 *
 * \param coreId remotecoreId of the remotecore user wants to check if ready. This arguments is currently
 *               a don't care because MCU+SDK doesn't support for any other core other than Linux.
 *
 * \return Status of the function
 */
int32_t EthFwIpc_lateInit(uint32_t coreId);

/*!
 * \brief ETHFW IPC RPMessage params initialization
 *
 * Initialize an RPMessage_Params structure to default values.
 *
 * \param params Address of the \ref EthFwIpc_RpmsgCreateParams structure to be initialized.
 */
void EthFwIpc_initRpmsgParams(EthFwIpc_RpmsgCreateParams *params);

/*!
 * \brief ETHFW IPC RPMessage create endpoint instance.
 *
 * Create RPMessage endpoint instance.
 *
 * \param params Address of the \ref EthFwIpc_RpmsgCreateParams after updating.
 *
 * \return \ref EthFwIpc_RpmsgHandle on success or a NULL if could not allocate object
 */
EthFwIpc_RpmsgHandle EthFwIpc_createRpmsg(EthFwIpc_RpmsgCreateParams *params);

/*!
 * \brief ETHFW IPC RPMessage send message
 *
 * Sends data to a remote processor.
 *
 * \param handle \ref EthFwIpc_RpmsgHandle created during \ref EthFwIpc_createRpmsg for that endpoint.
 * \param dstProcId Destination ProcId.
 * \param dstEndPt  Destination Endpoint.
 * \param srcEndPt  Source Endpoint.
 * \param data      Data payload to be copied and sent.
 * \param len       Amount of data to be copied.
 *
 * \return Status of the function
 */
int32_t EthFwIpc_sendRpmsg(EthFwIpc_RpmsgHandle handle,
                           uint32_t dstProcId,
                           uint32_t dstEndPt,
                           uint32_t srcEndPt,
                           void *data,
                           uint32_t len);

/*!
 * \brief ETHFW IPC RPMessage receive message
 *
 * Receives a message from an endpoint instance
 *
 * \param handle \ref EthFwIpc_RpmsgHandle created during \ref EthFwIpc_createRpmsg for that endpoint.
 * \param data       Pointer to the client's data buffer.
 * \param len        Amount of data received.
 * \param remoteEndPt  Endpoint of source (for replies).
 * \param remoteProcId ProcId of source (for replies).
 * \param timeout    Maximum duration to wait for a message in microseconds.
 *
 * \return Status of the function
 */
int32_t EthFwIpc_recvRpmsg(EthFwIpc_RpmsgHandle handle,
                           void *data,
                           uint16_t *len,
                           uint32_t *remoteEndPt,
                           uint32_t *remoteProcId,
                           uint32_t timeout);

/*!
 * \brief ETHFW IPC RPMessage get remote endpoint
 *
 * Wait for an endpoint to become available on another processor.
 *
 *  \param currProcId    Remote processor ID
 *  \param name      Name of the endpoint
 *  \param remoteProcId    Remote processor ID
 *  \param remoteEndPt     Remote endpoint ID
 *  \param timeout   Timeout value (in system ticks)
 *
 * \return Status of the function
 */
int32_t EthFwIpc_getRemoteEndPt(uint32_t currProcId,
                                const char* name,
                                uint32_t *remoteProcId,
                                uint32_t *remoteEndPt,
                                uint32_t timeout);

/*!
 * \brief ETHFW IPC RPMessage announce to a all cores
 *
 * Announce local endpoint to all cores participating in the IPC communication
 *
 *  \param localEp local endpoint which needs to be announced
 *  \param name    Service name associated with the local endpoint
 *
 * \return Status of the function
 */
int32_t EthFwIpc_announceAll(uint32_t localEp,
                             const char* name);

/*!
 * \brief ETHFW IPC RPMessage announce to a given remotecore
 *
 * Announce local endpoint to a given remote core
 *
 *  \param remoteProcId    Remote processor ID
 *  \param localEp         local endpoint which needs to be announced
 *  \param name    Service name associated with the local endpoint
 *
 * \return Status of the function
 */
int32_t EthFwIpc_announce(uint32_t remoteProcId,
                          uint32_t localEp,
                          const char* name);


/*!
 * \brief ETHFW IPC RPMessage unblock
 *
 * Unblocks an RPMessage_recv()
 *
 * \param handle \ref EthFwIpc_RpmsgHandle created during \ref EthFwIpc_createRpmsg for that endpoint.
 */
void EthFwIpc_unblockRpmsg(EthFwIpc_RpmsgHandle handle);

/*!
 * \brief ETHFW IPC RPMessage delete endpoint
 *
 *  This function deletes a created endpoint instance. If the
 *  message queue is non-empty, any messages remaining in the queue
 *  will be lost.
 *
\param handle \ref EthFwIpc_RpmsgHandle created during \ref EthFwIpc_createRpmsg for that endpoint.
 *
 * \return Status of the function
 */
int32_t EthFwIpc_deleteRpmsg(EthFwIpc_RpmsgHandle handle);


#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_IPC_H_ */
