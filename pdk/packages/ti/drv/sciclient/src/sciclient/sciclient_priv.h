/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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
 *
 */

/**
 *  \ingroup DRV_SCICLIENT_MODULE
 *  \defgroup SCICLIENT_PRIV_APIS Sciclient Private APIs
 *
 *  @{
 */

/**
 *  \file sciclient_priv.h
 *
 *  \brief This file contains the handle structure used internally by sciclient.
 */
#ifndef SCICLIENT_PRIV_H_
#define SCICLIENT_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/hw_types.h>
#include <ti/drv/sciclient/src/sec_proxy/csl_sec_proxy.h>
#include <ti/osal/osal.h>
#include <ti/osal/SemaphoreP.h>
#include <ti/osal/CacheP.h>
#include <ti/osal/RegisterIntr.h>

#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_romMessages.h>
#include <ti/drv/sciclient/soc/sciclient_soc_priv.h>

#if defined (SOC_AM65XX)
#include <ti/drv/sciclient/soc/V0/sciclient_defaultBoardcfg.h>
#endif

#if defined (SOC_J721E)
#include <ti/drv/sciclient/soc/V1/sciclient_defaultBoardcfg.h>
#endif

#if defined (SOC_J7200)
#include <ti/drv/sciclient/soc/V2/sciclient_defaultBoardcfg.h>
#endif

#if defined (SOC_AM64X)
#include <ti/drv/sciclient/soc/V3/sciclient_defaultBoardcfg.h>
#endif

#if defined (SOC_J721S2)
#include <ti/drv/sciclient/soc/V4/sciclient_defaultBoardcfg.h>
#endif

#if defined (SOC_J784S4) || defined (SOC_J742S2)
#include <ti/drv/sciclient/soc/V6/sciclient_defaultBoardcfg.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *    \brief Maximum number of messages waiting to be read.
 *           Cannot be greater than 256.
 */
#define SCICLIENT_MAX_QUEUE_SIZE            (7U)

/* Current context is SECURE */
#define SCICLIENT_SECURE_CONTEXT            (0U)

/* Current context is NON-SECURE */
#define SCICLIENT_NON_SECURE_CONTEXT        (1U)

#if defined (SCICLIENT_MERGED)
/* Sciclient merged library is applicable only to R5 cores.
 * Hence define the macros in SCI header for R5 core id.
 */
#define SCICLIENT_CORE_INVALID ((uint32_t)0xFFFFFFFFU)
#define SCICLIENT_CORE_MCU1_0 ((uint32_t)0U)
#define SCICLIENT_CORE_MCU1_1 ((uint32_t)1U)
#define SCICLIENT_CORE_MCU2_0 ((uint32_t)2U)
#define SCICLIENT_CORE_MCU2_1 ((uint32_t)3U)
#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
#define SCICLIENT_CORE_MCU3_0 ((uint32_t)4U)
#define SCICLIENT_CORE_MCU3_1 ((uint32_t)5U)
#if defined (SOC_J784S4) || defined (SOC_J742S2)
#define SCICLIENT_CORE_MCU4_0 ((uint32_t)6U)
#define SCICLIENT_CORE_MCU4_1 ((uint32_t)7U)
#endif /* #if defined (SOC_J784S4) || defined (SOC_J742S2) */
#endif /* #if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2) */
#endif /* #if defined (SCICLIENT_MERGED) */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Map structure used by #Sciclient_init function.
 */
typedef struct
{
    uint32_t context;
    /**< context(sec/non-sec) **/

    uint32_t hostId;
    /**< CPU ID of the A53/A72/R5F/DSP */
#if !defined (SOC_AM64X)
    uint32_t reqHighPrioThreadId;
    /**< Thread ID of the high priority thread(write) allowed for the CPU */
#endif
    uint32_t reqLowPrioThreadId;
    /**< Thread ID of the low priority thread(write) allowed for the CPU */

#if !defined (SOC_AM64X)
    uint32_t notificationRespThreadId;
    /**< Thread ID of the thread(write) for sending a notification to the
     *   firmware
     */
#endif
    uint32_t respThreadId;
    /**< Thread ID of the response thread(read) available for the CPU */

#if !defined (SOC_AM64X)
    uint32_t notificationThreadId;
    /**< Thread ID of the notification thread(read) available for the CPU */
#endif
    uint32_t respIntrNum;
    /**< Response Interrupt Number. */
#if defined (BUILD_C7X)
    uint32_t c7xEvtIn;
    /**< Thread ID of the notification thread(read) available for the CPU */
#endif
} Sciclient_MapStruct_t;

/**
 *  \brief Handle for #Sciclient_service function
 */
typedef struct
{
    SemaphoreP_Handle     semHandles[SCICLIENT_MAX_QUEUE_SIZE];
    /**< Semaphore is posted when there is a interrupt for the response.
     *   Index is the currSeqId
     */
    SemaphoreP_Status     semStatus[SCICLIENT_MAX_QUEUE_SIZE];
    /**< Status returned for the SemaphoreP_pend corresponding to a
     *   particular seqId.
     */
    uint32_t              currSeqId;
    /**< Sequence ID of the current request **/
    HwiP_Handle           notificationIntr;
    /**<  Interrupt for notification **/
    HwiP_Handle           respIntr[2];
    /**<  Interrupt for response message. Have 2 for secure and non-secure context **/
    uint32_t              opModeFlag;
    /**< Operation mode for the Sciclient Service API. Refer to
     * \ref Sciclient_ServiceOperationMode for valid values.
     */
    uint8_t               initCount;
    /**< Count to keep track of the number of inits/de-inits done. Actual
     *   initialization done
     *   only when initCount=0, and de-init done only when initCount=1
     */
    uint32_t              isSecureMode;
    /**< Variable to check whether Core context is secure/non-secure. This has
     * to be given by the user via configParams. Default value is 0.
     */
#if (defined(SCICLIENT_MERGED) || defined (BUILD_MCU1_0)) && (defined(SOC_J721E) || defined(SOC_J7200) || defined(SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2))
    uint32_t              pmBoardConfigComplete;
    /**< Status flag indicating PM Board config went through successfully */
    uint32_t              rmBoardConfigComplete;
    /**< Status flag indicating RM Board config went through successfully */
#endif
#if defined(_TMS320C6X)
    uint32_t              c66xRatRegion;
    /**< rat configuration region to de-init the same */
#endif
} Sciclient_ServiceHandle_t;


/**
 *  \anchor Sciclient_proxyMapStruct
 *  \name Sciclient map structure
 *  @{
 *  Map structure for R5F,A53,GPU and ICSSG \n
 *  in different contexts.
 */
extern const Sciclient_MapStruct_t gSciclientMap[SCICLIENT_CONTEXT_MAX_NUM];
/* @} */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**<
 *  \brief   API to send the board configuration messsage to the firmware .
 *          Valid(not NULL) pointer to Sciclient_BoardCfgPrms_t will use the
 *          provided values for tisci_msg_board_config_req, otherwise
 *          default values are used .
 *
 *  \param pInPrms   [IN] Pointer to #Sciclient_BoardCfgPrms_t .
 *
 *  \return  status    Status of the message.
 */
int32_t Sciclient_boardCfg(const Sciclient_BoardCfgPrms_t * pInPrms);

/**
 *  \brief   API to identify which mode the CPU is operating in. This utility
 *           function would read CPU related registers to know which mode
 *           (secure or non secure) the CPU is in and then would determine the
 *           context to be used. If more than one context is required for a
 *           a given code, users of SCICLENT would need to modify this function
 *           and recompile.
 *
 *  \param   pReqPrm Sciclient request parameters
 *
 *  \return  retVal  SCICLENT Context of the CPU
 */
uint32_t Sciclient_getCurrentContext(const Sciclient_ReqPrm_t *pReqPrm);

/**
 *  \brief   Gives the address for status register for a particular thread.
 *
 *  \param   thread    Index of the thread.
 *
 *  \return  address   address of the thread status
 */
#ifdef QNX_OS
uintptr_t Sciclient_threadStatusReg(uint32_t thread);
#else
uint32_t Sciclient_threadStatusReg(uint32_t thread);
#endif

/**
 *  \brief   Read a 32 bit word from the thread.
 *
 *  \param   thread    Index of the thread to be read from.
 *  \param   idx       Index of the word to be read from the thread.
 *
 *  \return  word      Value read back.
 */
uint32_t Sciclient_readThread32(uint32_t thread, uint8_t idx);

/**
 *  \brief   Read the current thread count.
 *
 *  \param   thread    Index of the thread to be read from.
 *
 *  \return  word      Count read back.
 */
uint32_t Sciclient_readThreadCount(uint32_t thread);

/**
 *  \brief   Validate thread has no errors and has space to accept the next
 *           message.
 *
 *  \param   thread    Index of the thread.
 *
 *  \return  status    Status of the message.
 */
int32_t Sciclient_verifyThread(uint32_t thread);

/**
 *  \brief   Check if there are credits to write to the thread.
 *
 *  \param   thread    Index of the thread.
 *  \param   timeout   Wait for timeout if operation is complete.
 *
 *  \return  status    Status of the message.
 */
int32_t Sciclient_waitThread(uint32_t thread, uint32_t timeout);

/**
 *  \brief   API to send the message to the thread.
 *
 *  \param   thread                 Index of the thread.
 *  \param   pSecHeader             Pointer to the security header extension.
 *  \param   secHeaderSizeWords     Secure Header size in Words.
 *  \param   pHeader                Pointer to the header structure.
 *  \param   pPayload               Pointer to the payload structure.
 *  \param   payloadSize            Size of the payload.
 *  \param   maxMsgSizeBytes        Maximum Message size in words.
 *
 *  \return  status    Status of the message.
 */
void Sciclient_sendMessage(uint32_t        thread,
                           const uint8_t  *pSecHeader,
                           const uint8_t   secHeaderSizeWords,
                           const uint8_t  *pHeader,
                           const uint8_t  *pPayload,
                           uint32_t        payloadSize,
                           const uint32_t  maxMsgSizeBytes);

/**
 *  \brief   This utility function would find the proxy map context id for
 *           'gSciclientMap' corresponding to a particular interrupt number.
 *
 *  \param   intrNum    Interrupt number.
 *
 *  \return  retVal     Context Id for the interrupt number.
 */
int32_t Sciclient_contextIdFromIntrNum(uint32_t intrNum);

/**
 *  \brief   API to flush/remove all outstanding messages on a thread .
 *
 *  \param   thread             Index of the thread.
 *  \param   maxMsgSizeBytes    Maximum message size in Bytes.
 *
 *  \return None
 */
void Sciclient_flush(uint32_t thread, uint32_t maxMsgSizeBytes);

/**
 *  \brief   API to flush/reset all message data on a tx thread .
 *
 *  \param   thread             Index of the thread.
 *  \param   maxMsgSizeBytes    Maximum message size in Bytes.
 *
 *  \return None
 */
void Sciclient_txFlush(uint32_t thread, uint32_t maxMsgSizeBytes);

/**
 *  \brief  This API allows communicating with the System firmware which can be
 *          called to perform various functions in the system.
 *          Core sciclient function for transmitting payload and recieving
 *          the response.
 *          The caller is expected to allocate memory for the input request
 *          parameter (Refer #Sciclient_ReqPrm_t). This involves setting the
 *          message type being communicated to the firmware, the response flags,
 *          populate the payload of the message based on the inputs in the
 *          files sciclient_fmwPmMessages.h,sciclient_fmwRmMessages.h,
 *          sciclient_fmwSecMessages.h and sciclient_fmwCommonMessages.h.
 *          Since the payload in considered a stream of bytes in this API,
 *          the caller should also populate the size of this stream in
 *          reqPayloadSize. The timeout is used to determine for what amount
 *          of iterations the API would wait for their operation to complete.
 *
 *          To make sure the response is captured correctly the caller should
 *          also allocate the space for #Sciclient_RespPrm_t parameters. The
 *          caller should populate the pointer to the pRespPayload and the size
 *          respPayloadSize. The API would populate the response flags to
 *          indicate any firmware specific errors and also populate the memory
 *          pointed by pRespPayload till the size given in respPayloadSize.
 *
 *
 * Requirement: DOX_REQ_TAG(PDK-2142), DOX_REQ_TAG(PDK-2141),
 *              DOX_REQ_TAG(PDK-2140), DOX_REQ_TAG(PDK-2139)
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *  \param pRespPrm       [OUT] Pointer to #Sciclient_RespPrm_t
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_serviceSecureProxy(const Sciclient_ReqPrm_t *pReqPrm,
                                     Sciclient_RespPrm_t      *pRespPrm);

/**
 *  \brief  Sciclient_serviceGetThreadIds Gets the threads used for message.
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *  \param contextId      [OUT] Context ID to be used.
 *  \param txThread       [OUT] Transmit Thread
 *  \param rxThread       [OUT] Recieve Thread
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_serviceGetThreadIds (const Sciclient_ReqPrm_t *pReqPrm,
                                       uint32_t *contextId,
                                       uint32_t *txThread,
                                       uint32_t *rxThread);

/**
 *  \brief  Sciclient_servicePrepareHeader Prepares the Message Header.
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *  \param contextId      [IN]  Context ID to be used.
 *  \param localSeqId     [OUT] Transmit Thread
 *  \param header         [OUT] Pointer to the header
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_servicePrepareHeader(const Sciclient_ReqPrm_t *pReqPrm,
                                       uint8_t *localSeqId,
                                       uint32_t contextId,
                                       struct tisci_header **header);

int32_t Sciclient_ProcessRmMessage(void *tx_msg);
int32_t Sciclient_ProcessPmMessage(const uint32_t reqFlags, void *tx_msg);
int32_t Sciclient_processDMVersionMessage(void *tx_msg);

#if defined (SCICLIENT_MERGED)

/**
 *  \brief  This API will be used internally in sciclient merged library to update
 *          each R5 core specific value in the scilcient driver's global variables.
 *
 * Requirement: DOX_REQ_TAG(PDK-17592)
 *
 *  \return CSL_PASS on success, else failure
 */
int32_t Sciclient_mergedInit(void);

/**
 *  \brief  This API is rename of Sciclient_init API. This API is renamed because
 *          to have a wrapper API in which another APIs can also be called. It is
 *          not recommended to use this API. This API is only for internal use in
 *          sciclient merged library and done for maintaining the backward
 *          compatibility.
 *
 *  Requirement: DOX_REQ_TAG(PDK-17592)
 *
 *  \param pCfgPrms     [IN]  Pointer to #Sciclient_ConfigPrms_t
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_initInternal(const Sciclient_ConfigPrms_t *pCfgPrms);

/**
 *  \brief  This API will be used to directly communicate with the System Firmware
 *          when the Sciclient driver is running on the DM Core in Merged Mode.
 *
 *          The caller is expected to allocate memory for the input request
 *          parameter (Refer #Sciclient_ReqPrm_t). This involves setting the
 *          message type being communicated to the firmware, the response flags,
 *          populate the payload of the message based on the inputs in the
 *          files sciclient_fmwPmMessages.h,sciclient_fmwRmMessages.h,
 *          sciclient_fmwSecMessages.h and sciclient_fmwCommonMessages.h.
 *          Since the payload in considered a stream of bytes in this API,
 *          the caller should also populate the size of this stream in
 *          reqPayloadSize. The timeout is used to determine for what amount
 *          of iterations the API would wait for their operation to complete.
 *
 *          To make sure the response is captured correctly the caller should
 *          also allocate the space for #Sciclient_RespPrm_t parameters. The
 *          caller should populate the pointer to the pRespPayload and the size
 *          respPayloadSize. The API would populate the response flags to
 *          indicate any firmware specific errors and also populate the memory
 *          pointed by pRespPayload till the size given in respPayloadSize.
 *
 *
 * Requirement: DOX_REQ_TAG(PDK-17592)
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *  \param pRespPrm       [OUT] Pointer to #Sciclient_RespPrm_t
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_serviceDirect(const Sciclient_ReqPrm_t *pReqPrm,
                                Sciclient_RespPrm_t      *pRespPrm);

/**
 *  \brief  This API will be used to indirectly communicate with the System Firmware
 *          over the secure proxy Threads. This API is used when the Sciclient driver
 *          is running on the NON-DM Core in Merged Mode.
 *
 *          The caller is expected to allocate memory for the input request
 *          parameter (Refer #Sciclient_ReqPrm_t). This involves setting the
 *          message type being communicated to the firmware, the response flags,
 *          populate the payload of the message based on the inputs in the
 *          files sciclient_fmwPmMessages.h,sciclient_fmwRmMessages.h,
 *          sciclient_fmwSecMessages.h and sciclient_fmwCommonMessages.h.
 *          Since the payload in considered a stream of bytes in this API,
 *          the caller should also populate the size of this stream in
 *          reqPayloadSize. The timeout is used to determine for what amount
 *          of iterations the API would wait for their operation to complete.
 *
 *          To make sure the response is captured correctly the caller should
 *          also allocate the space for #Sciclient_RespPrm_t parameters. The
 *          caller should populate the pointer to the pRespPayload and the size
 *          respPayloadSize. The API would populate the response flags to
 *          indicate any firmware specific errors and also populate the memory
 *          pointed by pRespPayload till the size given in respPayloadSize.
 *
 *
 * Requirement: DOX_REQ_TAG(PDK-17592)
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *  \param pRespPrm       [OUT] Pointer to #Sciclient_RespPrm_t
 *
 *  \return CSL_PASS on success, else failure
 *
 */
int32_t Sciclient_serviceIndirect(const Sciclient_ReqPrm_t *pReqPrm,
                                  Sciclient_RespPrm_t      *pRespPrm);

/**
 *  \brief  This API fetches the R5 core ID on which current application is running.
 *
 *          This API is only for internal use and used by the SciClient driver.
 *
 * Requirement: DOX_REQ_TAG(PDK-17592)
 *
 *  \return gets R5 core ID on which current application is running.
 *
 */
uint32_t Sciclient_getR5CoreId(void);

/**
 *  \brief  This API identifies the current context when the Sciclient driver is
 *          running on the DM Core (MCU1_0) in Merged Mode.
 *
 *          This API is only for internal use by the SciClient merged library.
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *
 *  \return SCICLIENT context ID for the current CPU.
 *
 */
uint32_t Sciclient_getCurrentContextDirect(const Sciclient_ReqPrm_t *pReqPrm);

/**
 *  \brief  This API identifies the current context when the Sciclient driver is
 *          running on a NON-DM Core in Merged Mode.
 *
 *          This API is only for internal use by the SciClient merged library.
 *
 *  \param pReqPrm        [IN]  Pointer to #Sciclient_ReqPrm_t
 *
 *  \return SCICLIENT context ID for the current CPU.
 *
 */
uint32_t Sciclient_getCurrentContextIndirect(const Sciclient_ReqPrm_t *pReqPrm);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SCICLIENT_PRIV_H_*/

/* @} */
