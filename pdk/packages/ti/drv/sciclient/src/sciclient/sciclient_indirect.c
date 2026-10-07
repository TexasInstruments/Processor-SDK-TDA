/*
 * Copyright (c) 2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 *  \file sciclient_indirect.c
 *
 *  \brief File containing the SCICLIENT driver APIs.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_priv.h>
#include <ti/drv/sciclient/sciserver.h>

#ifdef QNX_OS
#include <ti/drv/sciclient/src/sciclient/sciclient_qnx.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t gSecContextId;
extern uint32_t gNonSecContextId;
extern Sciclient_ServiceHandle_t gSciclientHandle;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

#if defined(SCICLIENT_MERGED)
int32_t Sciclient_serviceIndirect(const Sciclient_ReqPrm_t *pReqPrm,
                                  Sciclient_RespPrm_t      *pRespPrm)
#else
int32_t Sciclient_service (const Sciclient_ReqPrm_t *pReqPrm,
                           Sciclient_RespPrm_t      *pRespPrm)
#endif
{
#ifdef QNX_OS
    /* Inserting a resource manager layer, to serialize communication, between
     * multiple processes running on multiple cores,
     */
    int     fd;
    int32_t status       = CSL_PASS;
    char    *devname = "/dev/tisci";
    iov_t   siov[1], riov[1];
    int rc = EOK;

    /* Open up resource manager */
    fd = open(devname, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open(%s): %s\n", devname, strerror(errno));
        status = CSL_EFAIL;
        return status;
    }

    tisci_msg_t tisci_msg = {0};
    off64_t reqPhyPayloadPtr = 0;
    void   *reqPayload = NULL;
    off64_t responsePayloadPhyPtr = 0;
    void   *respPayload = NULL;

    /* Fill out request */
    if((pReqPrm->pReqPayload != 0) && (pReqPrm->reqPayloadSize != 0)){
        reqPayload = mmap64(NULL, pReqPrm->reqPayloadSize, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED|MAP_PHYS|MAP_ANON, NOFD, 0);
         if(reqPayload == MAP_FAILED) {
            fprintf(stderr, "%s:%d mma64 failed errno/%d\n",__FUNCTION__, __LINE__, errno);
            status = CSL_EFAIL;
            goto exit;
        }
        else {
            mem_offset64(reqPayload, NOFD, pReqPrm->reqPayloadSize, &reqPhyPayloadPtr, 0);
            if(reqPhyPayloadPtr == 0) {
                fprintf(stderr, "%s:%d mem_offset64 failed errno/%d\n",__FUNCTION__, __LINE__, errno);
                status = CSL_EFAIL;
                goto exit;
            }
        }
        memcpy((void *)reqPayload, (void *)pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
    }
    tisci_msg.reqPrm.messageType = pReqPrm->messageType;
    tisci_msg.reqPrm.flags       = pReqPrm->flags;
    tisci_msg.reqPrm.pReqPayload = (uint8_t *) reqPhyPayloadPtr;
    tisci_msg.reqPrm.reqPayloadSize = pReqPrm->reqPayloadSize;
    tisci_msg.reqPrm.timeout = pReqPrm->timeout;


    /* Define expected response */
    if((pRespPrm->pRespPayload != 0) && (pRespPrm->respPayloadSize != 0)){
        respPayload = mmap64(NULL, pRespPrm->respPayloadSize, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED|MAP_PHYS|MAP_ANON, NOFD, 0);
        if(respPayload == MAP_FAILED) {
            fprintf(stderr, "%s:%d mma64 failed errno/%d\n",__FUNCTION__, __LINE__, errno);
            status = CSL_EFAIL;
            goto exit;
        }
        else {
            mem_offset64(respPayload, NOFD, pRespPrm->respPayloadSize, &responsePayloadPhyPtr, 0);
            if(responsePayloadPhyPtr == 0) {
                fprintf(stderr, "%s:%d mem_offset64 failed errno/%d\n",__FUNCTION__, __LINE__, errno);
                status = CSL_EFAIL;
                goto exit;
            }
        }
        memcpy((void *)respPayload, (void *)pRespPrm->pRespPayload, pRespPrm->respPayloadSize);
    }
    tisci_msg.respPrm.flags = pRespPrm->flags;
    tisci_msg.respPrm.pRespPayload = (uint8_t *) responsePayloadPhyPtr;
    tisci_msg.respPrm.respPayloadSize =  pRespPrm->respPayloadSize;

    /* Get message ready to send */
    SETIOV(&siov[0], &tisci_msg, sizeof(tisci_msg_t));
    SETIOV(&riov[0], &tisci_msg, sizeof(tisci_msg_t));

    /*  Send message to /dev/tisci */
    if ((rc = devctlv(fd, DCMD_TISCI_MESSAGE, 1, 1, siov, riov, NULL)) != EOK) {
        fprintf(stderr, "devctl failed:%d errno/%d rc/%d\n", status, errno, rc);
        status = CSL_EFAIL;
        goto exit;
    }

    memcpy((void *)pRespPrm->pRespPayload, (void *)respPayload, pRespPrm->respPayloadSize);

    /* Flags parameter not getting set sometimes, ensure the response to user has correct flags value  */
    pRespPrm->flags = tisci_msg.respPrm.flags;
    QNX_DEBUG_PRINT("%s: tisci_msg.respPrm.flags = 0x%08x\n",__func__,tisci_msg.respPrm.flags);
    QNX_DEBUG_PRINT("%s: pRespPrm->flags = 0x%08x\n",__func__,pRespPrm->flags);

exit:
    if (reqPayload) munmap(reqPayload, pReqPrm->reqPayloadSize);
    if (respPayload) munmap(respPayload, pRespPrm->respPayloadSize);

    /* Close fd */
    if(fd > 0) {
        close(fd);
    }

    return status;
}


int32_t Sciclient_service_rsmgr(const Sciclient_ReqPrm_t *pReqPrm,
                                Sciclient_RespPrm_t      *pRespPrm)
{
#endif
    uint8_t *fwdStatus = (uint8_t *)&pReqPrm->forwardStatus;
    /*
     * Never forward messages for this build. Set this flag explicitly to
     * prevent accidental setting by the application.
     */
    *fwdStatus = SCISERVER_NO_FORWARD_MSG;

    return Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);
}


#if defined(SCICLIENT_MERGED)
uint32_t Sciclient_getCurrentContextIndirect(const Sciclient_ReqPrm_t *pReqPrm)
#else
uint32_t Sciclient_getCurrentContext(const Sciclient_ReqPrm_t *pReqPrm)
#endif
{
    uint32_t retVal = SCICLIENT_CONTEXT_MAX_NUM;

    if (NULL != pReqPrm)
    {
        switch(pReqPrm->messageType) {
            case TISCI_MSG_BOOT_NOTIFICATION:
            case TISCI_MSG_BOARD_CONFIG:
            case TISCI_MSG_BOARD_CONFIG_RM:
            case TISCI_MSG_BOARD_CONFIG_SECURITY:
            case TISCI_MSG_BOARD_CONFIG_PM:
            case TISCI_MSG_SA2UL_SET_DKEK:
            case TISCI_MSG_SA2UL_RELEASE_DKEK:
            case TISCI_MSG_SA2UL_GET_DKEK:
            {
                retVal = gSecContextId;
                break;
            }
            default:
            {
                /* For all other message type use non-secure context */
                if(1U == gSciclientHandle.isSecureMode)
                {
                    retVal = gSecContextId;
                }
                else
                {
                    retVal = gNonSecContextId;
                }
                break;
            }
        }
    }

    return retVal;
}

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

/* None */

