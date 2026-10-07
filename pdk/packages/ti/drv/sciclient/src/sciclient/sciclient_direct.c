/*
 * Copyright (c) 2020-2025, Texas Instruments Incorporated
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
 *  \file sciclient_direct.c
 *
 *  \brief File containing the SCICLIENT driver APIs.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ti/osal/osal.h>
#include <ti/drv/sciclient/soc/sysfw/include/tisci/tisci_protocol.h>
#include <boardcfg/boardcfg.h>
#include <startup.h>
#include <pm.h>
#include <rm.h>
#include <fw_caps.h>
/* Sciclient APIs are kept in the end of the include list to make sure the
 * RM and PM HAL typedefs are used.
 */
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_priv.h>
#include <ti/drv/sciclient/sciserver.h>
#include <ti/drv/sciclient/src/version/sciserver_version.h>
#include <ti/drv/sciclient/src/version/rmpmhal_version.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_trace_internal.h>

#if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
extern Sciclient_LpmData gSciclientLpmData;
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_MAGIC_WORD_LEN
 * Length of magic word for extended boot info table.
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_MIN_COMPONENTS
 * Minimum number of components that should be present in combined image.
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_MAX_COMPONENTS
 * Max number of components in extended boot info table.
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_ROM_INFO_BASE
 * Base address where ROM loads the extended boot info.
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SYSFW_DATA
 * Component type corresponding to SYSFW data blob.
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SBL_DATA
 * Component type corresponding to SBL data blob.
 */
#define SCICLIENT_DIRECT_EXTBOOT_X509_MAGIC_WORD_LEN             (8)
#define SCICLIENT_DIRECT_EXTBOOT_X509_MIN_COMPONENTS             (3)
#define SCICLIENT_DIRECT_EXTBOOT_X509_MAX_COMPONENTS             (8)
#define SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SYSFW_DATA        (0x12)
#define SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SBL_DATA          (0x11)

/**
 * \def SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_INDEX
 * Index of boardcfg in boardcfg descriptor table
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_SECURITY_INDEX
 * Index of security boardcfg in boardcfg descriptor table
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_PM_INDEX
 * Index of PM boardcfg in boardcfg descriptor table
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_RM_INDEX
 * Index of RM boardcfg in boardcfg descriptor table
 *
 * \def SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_NUM_DESCS
 * Number of boardcfg data in SYSFW data blob.
 */
#define SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_INDEX                  (0)
#define SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_SECURITY_INDEX         (1)
#define SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_PM_INDEX               (2)
#define SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_RM_INDEX               (3)
#define SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_NUM_DESCS              (4)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Component information populated by ROM. The combined image has
 *        multiple components.
 *        ROM populates this information for each component.
 *
 * \param comp_type Type of component
 * \param boot_core Core that loads or runs image
 * \param comp_opts Component options. Not used for now.
 * \param dest_addr Image destination address
 * \param comp_size Size of the component in bytes
 */
typedef struct {
    uint32_t comp_type;
    uint32_t boot_core;
    uint32_t comp_opts;
    uint64_t dest_addr;
    uint32_t comp_size;
} Sciclient_DirectExtBootX509Comp;

/**
 * \brief Component information table populated by ROM.
 *
 * \param magic_word Magic word for extended boot
 * \param num_comps Number of components present in extended boot info table
 * \param comps Information for each component
 */
typedef struct {
    uint8_t                             magic_word[
        SCICLIENT_DIRECT_EXTBOOT_X509_MAGIC_WORD_LEN];
    uint32_t                            num_comps;
    Sciclient_DirectExtBootX509Comp     comps[
        SCICLIENT_DIRECT_EXTBOOT_X509_MAX_COMPONENTS];
} Sciclient_DirectExtBootX509Table;

/**
 * \brief Describes the board config data
 *
 * \param type Type of board config data. This should map to corresponding TISCI
 *      message type. For example, a PM boardcfg should have a descriptor
 *      type TISCI_MSG_BOARD_CONFIG_PM.
 * \param offset Offset for board config data from beginning of SYSFW data
 *               binary blob
 * \param size Size of board config data
 * \param devgrp Device group to be used by SYSFW for automatic board
 *               config processing
 * \param reserved Reserved field. Not to be used.
 */
typedef struct {
    uint16_t type;
    uint16_t offset;
    uint16_t size;
    uint8_t  devgrp;
    uint8_t  reserved;
} __attribute__((__packed__)) Sciclient_DirectExtBootBoardCfgDesc ;

/**
 * \brief Boardcfg descriptor table provided by system integrator
 *
 * \param num_elems Number of elements in board config table.
 * \param sw_rev SW revision populated by system integrator.
 * \param descs Array of board config descriptors
 */
typedef struct {
    uint8_t                                 num_elems;
    uint8_t                                 sw_rev;
    Sciclient_DirectExtBootBoardCfgDesc     descs[
        SCICLIENT_DIRECT_EXTBOOT_BOARDCFG_NUM_DESCS];
} __attribute__((__packed__)) Sciclient_DirectBoardCfgDescTable;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t board_config_pm_handler(uint32_t *msg_recv);
__attribute__((optnone)) static uint16_t boardcfgRmFindCertSize(uint32_t *msg);
__attribute__((optnone)) static uint16_t boardcfgPmFindCertSize(uint32_t *msg);
static int32_t boardcfg_RmAdjustReq(uint32_t *msg, uint16_t adjSize);
static int32_t boardcfg_PmAdjustReq(uint32_t *msg, uint16_t adjSize);
static int32_t Sciclient_queryFwCapsHandler(const uint32_t reqFlags, void *tx_msg);
#if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
static int32_t Sciclient_getNextSysMode(void *tx_msg);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**
 * \def gcSciclientDirectExtBootX509MagicWord
 * Magic word that specifies whether combined image is being used.
 */
const char gcSciclientDirectExtBootX509MagicWord[
    SCICLIENT_DIRECT_EXTBOOT_X509_MAGIC_WORD_LEN] =
    { 'E', 'X', 'T', 'B', 'O', 'O', 'T', (char)0};

extern uint32_t gSecContextId;
extern uint32_t gNonSecContextId;
extern Sciclient_ServiceHandle_t gSciclientHandle;

/* Number of active cores. When reached 0, ready to shutdown */
static int32_t coreRefCnt = 0;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

#if defined(SCICLIENT_MERGED)
int32_t Sciclient_serviceDirect(const Sciclient_ReqPrm_t *pReqPrm,
                                Sciclient_RespPrm_t      *pRespPrm)
#else
int32_t Sciclient_service (const Sciclient_ReqPrm_t *pReqPrm,
                           Sciclient_RespPrm_t      *pRespPrm)
#endif
{
    Sciclient_printf("Entering Sciclient_service function\n");
    int32_t ret = CSL_PASS;
    uint32_t msgType;
    uint32_t message[20] = {0};
    struct tisci_header *hdr;
    uint16_t adjSize = 0;
    uint8_t *fwdStatus = (uint8_t *)&pReqPrm->forwardStatus;
    uint8_t localSeqId;
    uint32_t contextId;
    uint32_t txThread __attribute__((unused));
    uint32_t rxThread __attribute__((unused));

    if ((pReqPrm == NULL) || (pRespPrm == NULL))
    {
        ret = CSL_EBADARGS;
    }

    /*
     * Below functions serve only to check the context and format the header for
     * local processing. If the message is to be forwarded, then these functions
     * will be called again to obtain the necessary transfer parameters.
     */
    if (CSL_PASS == ret)
    {
        msgType = pReqPrm->messageType;
        Sciclient_printf("Request message type = 0x%X\n", msgType);
        ret = Sciclient_serviceGetThreadIds (pReqPrm, &contextId, &txThread,
                                         &rxThread);

        if(CSL_PASS != ret)
        {
            Sciclient_printf("ERROR:: Sciclient_service: Failed to get thread IDs\n");
        }
    }

    if (CSL_PASS == ret)
    {
        ret = Sciclient_servicePrepareHeader(pReqPrm, &localSeqId,
                 contextId, &hdr);

        if (CSL_PASS != ret)
        {
            Sciclient_printf("ERROR:: Sciclient_service: Failed to prepare header\n");
        }
    }
    if (CSL_PASS == ret)
    {
        switch (msgType) {
            case TISCI_MSG_BOARD_CONFIG_PM:
            case TISCI_MSG_SET_CLOCK:
            case TISCI_MSG_GET_CLOCK:
            case TISCI_MSG_SET_CLOCK_PARENT:
            case TISCI_MSG_GET_CLOCK_PARENT:
            case TISCI_MSG_GET_NUM_CLOCK_PARENTS:
            case TISCI_MSG_SET_FREQ:
            case TISCI_MSG_QUERY_FREQ:
            case TISCI_MSG_GET_FREQ:
            case TISCI_MSG_GET_DEVICE:
            case TISCI_MSG_SYS_RESET:
            case TISCI_MSG_PREPARE_SLEEP:
            case TISCI_MSG_ENTER_SLEEP:
            #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
            case TISCI_MSG_LPM_WAKE_REASON:
            case TISCI_MSG_GET_SUSPEND_INITIATOR:
            #endif
                if (msgType == TISCI_MSG_BOARD_CONFIG_PM)
                {
                    /* If PM boardcfg has a certificate, find the length */
                    adjSize = boardcfgPmFindCertSize((uint32_t *)pReqPrm->pReqPayload);

                    Sciclient_printf("TISCI_MSG_BOARD_CONFIG_PM : PM boardcfg sent to TIFS before local processing\n");
                    /* Send to TIFS */
                    ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                    if(CSL_PASS != ret)
                    {
                        Sciclient_printf("ERROR:: Sciclient_service: TIFS failed to process PM boardcfg\n");
                    }

                    if ((ret == CSL_PASS) &&
                       ((pRespPrm->flags & TISCI_MSG_FLAG_ACK) == TISCI_MSG_FLAG_ACK))
                    {
                        /*
                         * Invalidate the cache since TIFS will overwrite the memory
                         * to replace the certificate.
                         *
                         * If the certificate is consumed, need to adjust the size
                         * of the boardcfg request.
                         *
                         * If the certificate is not consumed (duplicate PM boardcfg
                         * send), then the pointer to the boardcfg structure needs
                         * to be advanced.
                         */
                        ret = boardcfg_PmAdjustReq((uint32_t *)pReqPrm->pReqPayload, adjSize);

                        if(CSL_PASS != ret)
                        {
                            Sciclient_printf("ERROR:: Sciclient_service: Failed to adjust PM boardcfg ");
                            Sciclient_printf("processed by DM for size adjustment\n");
                        }
                    }
                }
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                Sciclient_printf("This request is releated to Power Management and will be processed by DM core\n");
                ret = Sciclient_ProcessPmMessage(pReqPrm->flags, message);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: DM failed to process PM message\n");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                break;
            /*
             * MCU_R5's device state request message needs to be processed by TIFS
             * All other device state request messages will be processed by DM.
             */
            case TISCI_MSG_SET_DEVICE:
            case TISCI_MSG_SET_DEVICE_RESETS:
                {
                    /*
                     * For TISCI_MSG_SET_DEVICE and TISCI_MSG_SET_DEVICE_RESETS request messages
                     * have id value at the same memory location in pReqPrm->pReqPayload.
                     * typecasting pReqPrm->pReqPayload to anyone of its request structure
                     * can gives us the correct id value.
                     */
                    struct tisci_msg_set_device_req *req =
                        (struct tisci_msg_set_device_req *) pReqPrm->pReqPayload;
                    uint32_t id = req->id;
                    if ((id == SCICLIENT_DEV_MCU_R5FSS0_CORE0) || (id == SCICLIENT_DEV_MCU_R5FSS0_CORE1))
                    {
                        Sciclient_printf("This request is related to MCU R5F Power Management.");
                        Sciclient_printf("Therefore, it will be forwarded to TIFS\n");
                        ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                        if(CSL_PASS != ret)
                        {
                            Sciclient_printf("ERROR:: Sciclient_service: TIFS failed to process PM message for MCU R5F\n");
                        }
                    }
                    else
                    {
                        memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                        Sciclient_printf("This request is releated to Power Management and will be processed by DM core\n");
                        ret = Sciclient_ProcessPmMessage(pReqPrm->flags, message);

                        if(CSL_PASS != ret)
                        {
                            Sciclient_printf("ERROR:: Sciclient_service: DM failed to process PM message of set device/reset\n");
                        }

                        if (pRespPrm->pRespPayload != NULL)
                        {
                            memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                        }
                        hdr = (struct tisci_header *) &message;
                        pRespPrm->flags = hdr->flags;
                    }
                }
                break;
            /* RM messages processed solely by RM within DM on MCU R5F */
            case TISCI_MSG_RM_GET_RESOURCE_RANGE:
            case TISCI_MSG_RM_UDMAP_FLOW_CFG:
            case TISCI_MSG_RM_UDMAP_FLOW_SIZE_THRESH_CFG:
            case TISCI_MSG_RM_UDMAP_FLOW_DELEGATE:
            case TISCI_MSG_RM_UDMAP_GCFG_CFG:
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                ret = Sciclient_ProcessRmMessage(message);
                Sciclient_printf("RM messages processed solely by RM within DM on MCU R5F\n");

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: DM failed to process RM message\n");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                break;
            /*
             * RM messages processed by RM within DM on MCU R5F and Secure
             * RM within TIFS on M3
             */
            case TISCI_MSG_RM_IRQ_SET:
            case TISCI_MSG_RM_IRQ_RELEASE:
            case TISCI_MSG_RM_RING_CFG:
            case TISCI_MSG_RM_RING_MON_CFG:
            case TISCI_MSG_RM_UDMAP_TX_CH_CFG:
            case TISCI_MSG_RM_UDMAP_RX_CH_CFG:
            case TISCI_MSG_RM_PROXY_CFG:
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                Sciclient_printf("This request is releated to Resource Management.");
                Sciclient_printf("Therefore Request will be processed by DM first ");
                Sciclient_printf("and then TIFS will process Secure RM\n");
                ret = Sciclient_ProcessRmMessage(message);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: DM failed to process RM message on MCU R5F\n");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                if((pRespPrm->flags & TISCI_MSG_FLAG_ACK) == TISCI_MSG_FLAG_ACK)
                {
                    /*
                     * This message is forwarded to DMSC for continued
                     * processing of secure RM configuration.
                     */
                    *fwdStatus = SCISERVER_FORWARD_MSG;
                    ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                    if(CSL_PASS != ret)
                    {
                        Sciclient_printf("ERROR:: Sciclient_service: TIFS failed to process RM message on M core\n");
                    }
                }

                break;
            /* RM boardcfg must be sent to TIFS before local processing */
            case TISCI_MSG_BOARD_CONFIG_RM:

                /* If RM boardcfg has a certificate, find the length */
                adjSize = boardcfgRmFindCertSize((uint32_t *)pReqPrm->pReqPayload);

                Sciclient_printf("TISCI_MSG_BOARD_CONFIG_RM : RM boardcfg sent to TIFS before local processing\n");
                /* Send to TIFS */
                ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: TIFS failed to process RM boardcfg\n");
                }

                if ((ret == CSL_PASS) &&
                        ((pRespPrm->flags & TISCI_MSG_FLAG_ACK) == TISCI_MSG_FLAG_ACK))
                {
                    /*
                     * Invalidate the cache since TIFS will overwrite the memory
                     * to replace the certificate.
                     *
                     * If the certificate is consumed, need to adjust the size
                     * of the boardcfg request.
                     *
                     * If the certificate is not consumed (duplicate RM boardcfg
                     * send), then the pointer to the boardcfg structure needs
                     * to be advanced.
                     */
                    ret = boardcfg_RmAdjustReq((uint32_t *)pReqPrm->pReqPayload, adjSize);

                    if(CSL_PASS != ret)
                    {
                        Sciclient_printf("ERROR:: Sciclient_service: Failed to adjust RM boardcfg by RM ");
                        Sciclient_printf("processed by DM for size adjustment\n");
                    }
                }
                if ((ret == CSL_PASS) &&
                        ((pRespPrm->flags & TISCI_MSG_FLAG_ACK) == TISCI_MSG_FLAG_ACK))
                {
                    memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                    Sciclient_printf("TISCI_MSG_BOARD_CONFIG_RM : RM boardcfg sending to RM processed by DM ");
                    Sciclient_printf("for local processing\n");
                    ret = Sciclient_ProcessRmMessage(message);

                    if(CSL_PASS != ret)
                    {
                        Sciclient_printf("ERROR:: Sciclient_service: Failed to process RM boardcfg by RM ");
                        Sciclient_printf("processed by DM for local processing\n");
                    }

                    if (pRespPrm->pRespPayload != NULL)
                    {
                        memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                    }
                    hdr = (struct tisci_header *) &message;
                    pRespPrm->flags = hdr->flags;
                }
                else
                {
                    ret = CSL_EFAIL;
                }
                break;
            /* RM messages processed by Secure RM within TIFS on M3 */
            case TISCI_MSG_RM_PSIL_PAIR:
            case TISCI_MSG_RM_PSIL_UNPAIR:
            case TISCI_MSG_RM_PSIL_READ:
            case TISCI_MSG_RM_PSIL_WRITE:
            {
                /*
                 * These RM messages are entirely processed on DMSC. When
                 * called on mcu1_0 directly, these are treated as native calls
                 * to DMSC. If these requests are made from other CPUs, the
                 * sciserver will take care of setting the forward status prior
                 * to calling this function.
                 * When running on mcu1_0 we still use forwarding to retain
                 * the host id information of the non-secure/secure mode of mcu1_0.
                 * Setting forwarding ensures that the mcu1_0 uses the DM2DMSC
                 * secure proxy threads.
                 * NOTE: Forwarding always leads to forced polling.
                 */
                *fwdStatus = SCISERVER_FORWARD_MSG;
                Sciclient_printf("RM messages processed by Secure RM within TIFS\n");
                ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: Failed to process RM message ");
                    Sciclient_printf("processed by Secure RM within TIFS\n");
                }

                break;
            }
            case TISCI_MSG_QUERY_FW_CAPS:
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                Sciclient_printf("Firmware Query is processed internally by DM driver\n");
                ret = Sciclient_queryFwCapsHandler(pReqPrm->flags,message);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: Failed to process Firmware Query ");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                break;
            case TISCI_MSG_DM_VERSION:
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                Sciclient_printf("DM Version is processed internally by DM driver\n");
                ret = Sciclient_processDMVersionMessage(message);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: Failed to process DM Version\n");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                break;
            case TISCI_MSG_LPM_GET_NEXT_SYS_MODE:
                #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
                memcpy(message, pReqPrm->pReqPayload, pReqPrm->reqPayloadSize);
                Sciclient_printf("Next Sys Mode is processed internally by DM driver\n");
                ret = Sciclient_getNextSysMode(message);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: Failed to process Next Sys Mode\n");
                }

                if (pRespPrm->pRespPayload != NULL)
                {
                    memcpy(pRespPrm->pRespPayload, message, pRespPrm->respPayloadSize);
                }
                hdr = (struct tisci_header *) &message;
                pRespPrm->flags = hdr->flags;
                #else
                ret = CSL_EFAIL;
                #endif
                break;
            default:
            {
                /*
                 * All baseport and security messages are entirely processed on
                 * DMSC. When called on mcu1_0 directly, these are treated as
                 * native calls to DMSC. If these requests are made from other
                 * CPUs, the sciserver will take care of setting the forward
                 * status prior to calling this function.
                 * The MCU1_0 will always be secure when trying to send the message
                 * to the TIFS directly to avoid self blocking.
                 */
                Sciclient_printf("This is either baseport or security message and forwarded to TIFS\n");
                ret = Sciclient_serviceSecureProxy(pReqPrm, pRespPrm);

                if(CSL_PASS != ret)
                {
                    Sciclient_printf("ERROR:: Sciclient_service: Failed to process baseport or ");
                    Sciclient_printf("security message forwarded to TIFS\n");
                }
                break;
            }
        }
    }

    /*
     * Reset the forward status. This prevents possible accidental reuse of
     * stack memory with flag previously set and not properly cleared by the
     * application.
     */
    *fwdStatus = SCISERVER_NO_FORWARD_MSG;

    if(CSL_PASS == ret)
    {
        Sciclient_printf("Exiting Sciclient_service function with Status - PASS\n");
    }
    else
    {
        Sciclient_printf("ERROR:: Exiting Sciclient_service function with Status - FAIL\n");
    }

    return ret;
}

void Sciclient_TisciMsgSetAckResp(struct tisci_header *hdr)
{
    Sciclient_printf("Requested service is executed sucessfully by DM\n");
    hdr->flags |= TISCI_MSG_FLAG_ACK;
}

void Sciclient_TisciMsgSetNakResp(struct tisci_header *hdr)
{
    Sciclient_printf("Requested service in DM results in failure\n");
    hdr->flags &= (~TISCI_MSG_FLAG_ACK);
}

#if defined(SCICLIENT_MERGED)
uint32_t Sciclient_getCurrentContextDirect(const Sciclient_ReqPrm_t *pReqPrm)
#else
uint32_t Sciclient_getCurrentContext(const Sciclient_ReqPrm_t *pReqPrm)
#endif
{
    uint32_t retVal = SCICLIENT_CONTEXT_MAX_NUM;

    if (NULL != pReqPrm)
    {
        switch(pReqPrm->messageType) {
            case TISCI_MSG_SET_CLOCK:
            case TISCI_MSG_GET_CLOCK:
            case TISCI_MSG_SET_CLOCK_PARENT:
            case TISCI_MSG_GET_CLOCK_PARENT:
            case TISCI_MSG_GET_NUM_CLOCK_PARENTS:
            case TISCI_MSG_SET_FREQ:
            case TISCI_MSG_QUERY_FREQ:
            case TISCI_MSG_GET_FREQ:
            case TISCI_MSG_GET_DEVICE:
            case TISCI_MSG_SYS_RESET:
            case TISCI_MSG_PREPARE_SLEEP:
            case TISCI_MSG_ENTER_SLEEP:
            case TISCI_MSG_LPM_WAKE_REASON:
            case TISCI_MSG_GET_SUSPEND_INITIATOR:
            case TISCI_MSG_RM_GET_RESOURCE_RANGE:
            case TISCI_MSG_RM_UDMAP_FLOW_CFG:
            case TISCI_MSG_RM_UDMAP_FLOW_SIZE_THRESH_CFG:
            case TISCI_MSG_RM_UDMAP_FLOW_DELEGATE:
            case TISCI_MSG_RM_UDMAP_GCFG_CFG:
            case TISCI_MSG_RM_IRQ_SET:
            case TISCI_MSG_RM_IRQ_RELEASE:
            case TISCI_MSG_RM_RING_CFG:
            case TISCI_MSG_RM_RING_MON_CFG:
            case TISCI_MSG_RM_UDMAP_TX_CH_CFG:
            case TISCI_MSG_RM_UDMAP_RX_CH_CFG:
            case TISCI_MSG_RM_PROXY_CFG:
            case TISCI_MSG_RM_PSIL_PAIR:
            case TISCI_MSG_RM_PSIL_UNPAIR:
            case TISCI_MSG_RM_PSIL_READ:
            case TISCI_MSG_RM_PSIL_WRITE:
            case TISCI_MSG_QUERY_FW_CAPS:
            case TISCI_MSG_DM_VERSION:
            case TISCI_MSG_LPM_GET_NEXT_SYS_MODE:
            {
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
            case TISCI_MSG_SET_DEVICE:
            case TISCI_MSG_SET_DEVICE_RESETS:
            {
                if (NULL != pReqPrm->pReqPayload)
                {
                    struct tisci_msg_set_device_req *req =
                    (struct tisci_msg_set_device_req *) pReqPrm->pReqPayload;
                    uint32_t id = req->id;
                    if ((id == SCICLIENT_DEV_MCU_R5FSS0_CORE0) || (id == SCICLIENT_DEV_MCU_R5FSS0_CORE1))
                    {
                        retVal = gSecContextId;
                    }
                    else
                    {
                        if(1U == gSciclientHandle.isSecureMode)
                        {
                            retVal = gSecContextId;
                        }
                        else
                        {
                            retVal = gNonSecContextId;
                        }
                    }
                }
                break;
            }
            default:
            {
                retVal = gSecContextId;
                break;
            }
        }
    }

    return retVal;
}

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static int32_t board_config_pm_handler(uint32_t *msg_recv)
{
    int32_t ret;
    struct tisci_msg_board_config_pm_req *req =
        (struct tisci_msg_board_config_pm_req *) msg_recv;

    ret = boardcfg_pm_receive_and_validate(req->hdr.host,
                           req->tisci_boardcfg_pmp_low,
                           req->tisci_boardcfg_pmp_high,
                           req->tisci_boardcfg_pm_size,
                           req->tisci_boardcfg_pm_devgrp);

    if (ret == CSL_PASS) {
        ret = pm_init();
    }

    #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
    Sciclient_LpmData *lpmLocal;
    bool lpmBcfgValid = is_lpm_boardcfg_valid();
    if (lpmBcfgValid == true)
    {
        lpmLocal = (Sciclient_LpmData *) boardcfg_pm_extract_lpm_cfg();
        if (lpmLocal == NULL)
        {
            ret = CSL_EFAIL;
        }

        if (ret == CSL_PASS)
        {
            gSciclientLpmData.suspend_initiator = lpmLocal->suspend_initiator;
            gSciclientLpmData.lpm_mode = lpmLocal->lpm_mode;
        }
    }
    #endif

    return ret;
}

#if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
static int32_t Sciclient_getNextSysMode(void *tx_msg)
{
    int32_t ret = CSL_PASS;
    uint32_t flags = ((struct tisci_header *) tx_msg)->flags;
    struct tisci_msg_lpm_get_next_sys_mode_resp *resp = (struct tisci_msg_lpm_get_next_sys_mode_resp *) tx_msg;
    resp->hdr.flags = 0U;

    if (is_lpm_boardcfg_valid())
    {
        resp->mode = gSciclientLpmData.lpm_mode;
    }
    else
    {
        /* lpm boardcfg is invalid */
        ret = CSL_EFAIL;
    }

    if ((flags & TISCI_MSG_FLAG_AOP) != 0UL)
    {
        if (ret == CSL_PASS)
        {
            Sciclient_TisciMsgSetAckResp((struct tisci_header *) tx_msg);
        }
        else
        {
            Sciclient_TisciMsgSetNakResp((struct tisci_header *) tx_msg);
        }
    }

    return ret;
}
#endif

static int32_t Sciclient_queryFwCapsHandler(const uint32_t reqFlags __attribute__((unused)), void *tx_msg)
{
    int32_t ret = CSL_PASS;
    uint32_t flags = ((struct tisci_header *) tx_msg)->flags;
    bool lpmBcfgValid = false;

    ret = query_fw_caps_handler((uint32_t*)tx_msg);
	struct tisci_query_fw_caps_resp *resp = (struct tisci_query_fw_caps_resp *)((uint32_t*)tx_msg);

    /* If the lpm_config is not valid inside the pm-boardcfg,
     * then remove the LPM_BOARDCFG_MANAGED capability as it is invalid now
     */
    if (ret == CSL_PASS)
    {
        lpmBcfgValid = is_lpm_boardcfg_valid();
        if (lpmBcfgValid == false)
        {
            resp->fw_caps &= ~TISCI_MSG_FLAG_FW_CAP_LPM_BOARDCFG_MANAGED;
        }
    }

    if ((flags & TISCI_MSG_FLAG_AOP) != 0UL) {
        if (ret == CSL_PASS) {
            Sciclient_TisciMsgSetAckResp((struct tisci_header *) tx_msg);
        } else {
            Sciclient_TisciMsgSetNakResp((struct tisci_header *) tx_msg);
        }
    }

    return ret;
}

int32_t Sciclient_ProcessPmMessage(const uint32_t reqFlags  __attribute__((unused)), void *tx_msg)
{
    Sciclient_printf("Entering Sciclient_ProcessPmMessage function\n");
    int32_t ret = CSL_PASS;
    bool msg_inval = (bool)false;
    uint32_t msgType = ((struct tisci_header *) tx_msg)->type;
    uint32_t flags = ((struct tisci_header *) tx_msg)->flags;
    switch (msgType)
    {
        case TISCI_MSG_BOARD_CONFIG_PM         :
            Sciclient_printf("case: TISCI_MSG_BOARD_CONFIG_PM\n");
            ret = board_config_pm_handler((uint32_t*)tx_msg); break;
        case TISCI_MSG_SET_CLOCK               :
            {
                struct tisci_msg_set_clock_req *req =(struct tisci_msg_set_clock_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_SET_CLOCK : Requested Device id = %u Clock ID = %u State = %d \n",
                                 req->device, req->clk, req->state);
                ret = set_clock_handler((uint32_t*)tx_msg); break;
            }
        case TISCI_MSG_GET_CLOCK               :
            {
                struct tisci_msg_get_clock_req *req = (struct tisci_msg_get_clock_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_CLOCK : Requested Device id = %u Clock ID = %u\n", req->device, req->clk);
                ret = get_clock_handler((uint32_t*)tx_msg);
                struct tisci_msg_get_clock_resp *resp = (struct tisci_msg_get_clock_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_CLOCK : Current Clock state is %d\n", resp->current_state);
                break;
            }
        case TISCI_MSG_SET_CLOCK_PARENT        :
            {
                struct tisci_msg_set_clock_parent_req *req = (struct tisci_msg_set_clock_parent_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_SET_CLOCK_PARENT : Requested Device id = %u Clock ID = %u Parent Clock ID = %u\n",
                                 req->device, req->clk, req->parent);
                ret = set_clock_parent_handler((uint32_t*)tx_msg); break;
            }
        case TISCI_MSG_GET_CLOCK_PARENT        :
            {
                struct tisci_msg_get_clock_parent_req *req = (struct tisci_msg_get_clock_parent_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_CLOCK_PARENT : Requested Device id = %u Clock ID = %u\n", req->device, req->clk);
                ret = get_clock_parent_handler((uint32_t*)tx_msg);
                struct tisci_msg_get_clock_parent_resp *resp = (struct tisci_msg_get_clock_parent_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_CLOCK_PARENT : Clock Parent = %u \n", resp->parent);
                break;
            }
        case TISCI_MSG_GET_NUM_CLOCK_PARENTS   :
            {
                struct tisci_msg_get_num_clock_parents_req *req = (struct tisci_msg_get_num_clock_parents_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_NUM_CLOCK_PARENTS : Requested Device id = %u Clock ID = %u\n", req->device, req->clk);
                ret = get_num_clock_parents_handler((uint32_t*)tx_msg);
                struct tisci_msg_get_num_clock_parents_resp *resp = (struct tisci_msg_get_num_clock_parents_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_NUM_CLOCK_PARENTS : Total Number of parents = %u \n", resp->num_parents);
                break;
            }
        case TISCI_MSG_SET_FREQ                :
            {
                struct tisci_msg_set_freq_req *req = (struct tisci_msg_set_freq_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_SET_FREQ : Requested Device id = %u Clock ID = %u "\
                                 "Target Frequency = %luHz Min_Frequency = %luHz Max_Frequency = %luHz\n",
                                 req->device, req->clk, req->target_freq_hz, req->min_freq_hz, req->max_freq_hz);
                ret = set_freq_handler((uint32_t*)tx_msg); break;
            }
        case TISCI_MSG_QUERY_FREQ              :
            {
                struct tisci_msg_query_freq_req *req = (struct tisci_msg_query_freq_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_QUERY_FREQ : Requested Device id = %u Clock ID = %u "\
                                 "Target Frequency = %luHz Min_Frequency = %luHz Max_Frequency = %luHz\n",
                                 req->device, req->clk, req->target_freq_hz, req->min_freq_hz, req->max_freq_hz);
                ret = query_freq_handler((uint32_t*)tx_msg);
                struct tisci_msg_query_freq_resp *resp = (struct tisci_msg_query_freq_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_QUERY_FREQ : Allowed Frequency = %lu\n", resp->freq_hz);
                break;
            }
        case TISCI_MSG_GET_FREQ                :
            {
                struct tisci_msg_get_freq_req *req = (struct tisci_msg_get_freq_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_FREQ : Requested Device id = %u Clock ID = %u\n", req->device, req->clk);
                ret = get_freq_handler((uint32_t*)tx_msg);
                struct tisci_msg_get_freq_resp *resp = (struct tisci_msg_get_freq_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_FREQ : Current Frequency = %lu\n", resp->freq_hz);
                break;
            }
        case TISCI_MSG_SET_DEVICE              :
            {
                struct tisci_msg_set_device_req *req =
                    (struct tisci_msg_set_device_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_SET_DEVICE : Requested Device id = %u State = %d\n", req->id, req->state);
                uint32_t id = req->id;
                uint8_t state = req->state;
                if (id == TISCI_DEV_BOARD0)
                {
                    if (state == (uint8_t)TISCI_MSG_VALUE_DEVICE_SW_STATE_ON) {
                        coreRefCnt++;
                    }
                    else if (state == (uint8_t)TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF) {
                        coreRefCnt--;
                        /*
                         * When no core is active, shutdown PMIC.
                         * The <= catches call to shutdown before powering up a core.
                         */
                        if (coreRefCnt <= 0) {
                            Osal_delay(1000U); /* time for ATF go in WFI */
                            ret = Sciclient_pmicShutdown();
                        }
                    }
                    else {
                        ret = CSL_EFAIL;
                    }
                }
                else
                {
                    ret = set_device_handler((uint32_t*)tx_msg);
                }
            }
            break;
        case TISCI_MSG_GET_DEVICE              :
            {
                struct tisci_msg_get_device_req *req = (struct tisci_msg_get_device_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_DEVICE : Requested Device id = %u \n", req->id);
                ret = get_device_handler((uint32_t*)tx_msg);
                struct tisci_msg_get_device_resp *resp = (struct tisci_msg_get_device_resp *) tx_msg;
                Sciclient_printf("TISCI_MSG_GET_DEVICE : Current Device state = %d \n", resp->current_state);
                break;
            }
        case TISCI_MSG_SET_DEVICE_RESETS       :
            Sciclient_printf("TISCI_MSG_SET_DEVICE_RESETS : Requested Device id = %u \n",
                                    ((struct tisci_msg_set_device_resets_req *)tx_msg)->id);
            ret = set_device_resets_handler((uint32_t*)tx_msg); break;
        case TISCI_MSG_SYS_RESET               :
            {
                struct tisci_msg_sys_reset_req *req = (struct tisci_msg_sys_reset_req *) tx_msg;
                Sciclient_printf("TISCI_MSG_SYS_RESET : Requested Domain = %d\n", req->domain);
                ret = sys_reset_handler((uint32_t*)tx_msg);
                break;
            }
        case TISCI_MSG_PREPARE_SLEEP            :
            Sciclient_printf("TISCI_MSG_PREPARE_SLEEP : Prepare the device to enter into low power mode \n");
            ret = Sciclient_prepareSleep((uint32_t*)tx_msg);
            break;
        case TISCI_MSG_ENTER_SLEEP              :
            {
                #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
                struct tisci_msg_enter_sleep_req *req = (struct tisci_msg_enter_sleep_req *) tx_msg;
                if (gSciclientLpmData.suspend_initiator == (req->hdr.host))
                {
                    /* Keep WKUP_I2C0 ON before starting the LPM sequence.
                     * The sequence requires talking to the PMIC over WKUP_I2C0
                     * to trigger IO and DDR retention.
                     */
                    ret = Sciclient_s2rEnableWkupI2c();
                    if (ret != CSL_PASS)
                    {
                        Sciclient_printf("TISCI_MSG_ENTER_SLEEP : Failed to enable WKUP_I2C0, aborting LPM entry \n");
                        ret = CSL_EFAIL;
                    }
                    else
                    {
                        Sciclient_printf("TISCI_MSG_ENTER_SLEEP : Device enters into low power mode \n");
                        ret = Sciclient_enterSleep((uint32_t*)tx_msg);
                    }
                }
                else
                {
                    Sciclient_printf("TISCI_MSG_ENTER_SLEEP : Device did not enter low power mode, request came from invalid initiator \n");
                    ret = CSL_EFAIL;
                    msg_inval = (bool)true;
                }
                #else
                ret = CSL_EFAIL;
                #endif
                break;
            }
        case TISCI_MSG_LPM_WAKE_REASON:
            #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
            ret = Sciclient_getWakeReason((uint32_t*)tx_msg);
            #else
            ret = CSL_EFAIL;
            #endif
            break;
        case TISCI_MSG_GET_SUSPEND_INITIATOR:
            #if defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J721S2)
            ret = Sciclient_getSuspendMaster((uint32_t*)tx_msg);
            #else
            ret = CSL_EFAIL;
            #endif
            break;
        default:
            ret = CSL_EFAIL;
            msg_inval = (bool)true;
            break;
    }
    if ((flags & TISCI_MSG_FLAG_AOP) != 0UL) {
        if (ret == CSL_PASS) {
            Sciclient_TisciMsgSetAckResp((struct tisci_header *) tx_msg);
        } else {
            Sciclient_TisciMsgSetNakResp((struct tisci_header *) tx_msg);
        }
    }
    /*
     * Avoid overwriting status of calling message handler if the message
     * handler has a failure status.
     */
    if ((ret != CSL_PASS) && !msg_inval) {
        /*
         * NACK will be sent in message if error so send CSL_PASS back
         * in return status
         */
        ret = CSL_PASS;
    }

    if(CSL_PASS != ret)
    {
        Sciclient_printf("ERROR:: Exiting Sciclient_ProcessPmMessage function with status FAIL\n");
    }
    else
    {
        Sciclient_printf("Exiting Sciclient_ProcessPmMessage function with status PASS\n");
    }

    return ret;
}

__attribute__((optnone)) static uint16_t boardcfgRmFindCertSize(uint32_t *msg_recv)
{
    uint16_t cert_len = 0U;
    uint16_t certSize = 0U;
    uint8_t *cert_len_ptr = (uint8_t *)&cert_len;
    uint8_t *x509_cert_ptr;

    struct tisci_msg_board_config_rm_req *req =
        (struct tisci_msg_board_config_rm_req *) msg_recv;

   x509_cert_ptr = (uint8_t *)req->tisci_boardcfg_rmp_low;


    if (*x509_cert_ptr != 0x30U)
    {
        /* The data does not contain a certificate - return */
        certSize = 0U;
    }
    else
    {
        cert_len = *(x509_cert_ptr + 1);

        /* If you need more than 2 bytes to store the cert length  */
        /* it means that the cert length is greater than 64 Kbytes */
        /* and we do not support it                                */
        if ((cert_len > 0x80U) &&
          (cert_len != 0x82U))
          {
            certSize = 0U;
          }
          else
          {
            if (cert_len == 0x82U)
            {
              *cert_len_ptr = *(x509_cert_ptr + 3);
              *(cert_len_ptr + 1) = *(x509_cert_ptr + 2);

              /* add current offset from start of x509 cert */
              certSize = cert_len + 3U;
            }
            else
            {
              /* add current offset from start of x509 cert  */
              /* if cert len was obtained from 2nd byte i.e. */
              /* cert size is 127 bytes or less              */
              certSize= cert_len + 1U;
            }

            /* certSize now contains the offset of the last byte */
            /* of the cert from the ccert_start. To get the size */
            /* of certificate, add 1                             */
            certSize =  certSize + 1U;
          }
    }
    return (certSize);
}

__attribute__((optnone)) static uint16_t boardcfgPmFindCertSize(uint32_t *msg_recv)
{
    uint16_t cert_len = 0U;
    uint16_t certSize = 0U;
    uint8_t *cert_len_ptr = (uint8_t *)&cert_len;
    uint8_t *x509_cert_ptr;

    struct tisci_msg_board_config_pm_req *req =
        (struct tisci_msg_board_config_pm_req *) msg_recv;

    x509_cert_ptr = (uint8_t *)req->tisci_boardcfg_pmp_low;

    if (*x509_cert_ptr != 0x30U)
    {
        /* The data does not contain a certificate - return */
        certSize = 0U;
    }
    else
    {
        cert_len = *(x509_cert_ptr + 1);

        /* If you need more than 2 bytes to store the cert length  */
        /* it means that the cert length is greater than 64 Kbytes */
        /* and we do not support it                                */
        if ((cert_len > 0x80U) && (cert_len != 0x82U))
        {
            certSize = 0U;
        }
        else
        {
            if (cert_len == 0x82U)
            {
                *cert_len_ptr = *(x509_cert_ptr + 3);
                *(cert_len_ptr + 1) = *(x509_cert_ptr + 2);

                /* add current offset from start of x509 cert */
                certSize = cert_len + 3U;
            }
            else
            {
                /* add current offset from start of x509 cert  */
                /* if cert len was obtained from 2nd byte i.e. */
                /* cert size is 127 bytes or less              */
                certSize= cert_len + 1U;
            }

            /* certSize now contains the offset of the last byte */
            /* of the cert from the ccert_start. To get the size */
            /* of certificate, add 1                             */
            certSize =  certSize + 1U;
        }
    }
    return (certSize);
}

static int32_t boardcfg_PmAdjustReq(uint32_t *msg, uint16_t adjSize)
{
    int32_t r = CSL_PASS;
    uint16_t newSize = 0U;
    struct tisci_msg_board_config_pm_req *req =
        (struct tisci_msg_board_config_pm_req *) msg;

    /* If there was no certificate to begin with, do not adjust anything */
    if (adjSize == 0U)
    {
        r = CSL_PASS;
    }
    else
    {
        /* Invalidate the cache */
        CacheP_Inv((const void*) req->tisci_boardcfg_pmp_low,
                   (req->tisci_boardcfg_pm_size));

        /*
         * See if there is still a certificate that needs to be compensated for (in
         * case TIFS did not process this upon multiple requests for PM board cfg).
         *
         * If there is no certificate, then we adjust the size of the PM boardcfg
         * request. If there is a certificate, it should match the size we retrieved
         * earlier. Advance the base pointer. If the size does not match, then this
         * is an error.
         */
        newSize = boardcfgPmFindCertSize(msg);

        if (newSize == 0U)
        {
            req->tisci_boardcfg_pm_size -= adjSize;
        }
        else if (newSize == adjSize)
        {
            req->tisci_boardcfg_pm_size -= adjSize;
            req->tisci_boardcfg_pmp_low += adjSize;
        }
        else
        {
            r = CSL_EFAIL;
        }
    }

    return r;
}

static int32_t boardcfg_RmAdjustReq(uint32_t *msg, uint16_t adjSize)
{
    int32_t r = CSL_PASS;
    uint16_t newSize = 0U;
    struct tisci_msg_board_config_rm_req *req =
        (struct tisci_msg_board_config_rm_req *) msg;

    /* If there was no certificate to begin with, do not adjust anything */
    if (adjSize == 0U)
    {
        r = CSL_PASS;
    }
    else{
            /* Invalidate the cache */
            CacheP_Inv((const void*) req->tisci_boardcfg_rmp_low,
                    req->tisci_boardcfg_rm_size);

            /*
             * See if there is still a certificate that needs to be compensated for (in
             * case TIFS did not process this upon multiple requests for RM board cfg).
             *
             * If there is no certificate, then we adjust the size of the RM boardcfg
             * request. If there is a certificate, it should match the size we retrieved
             * earlier. Advance the base pointer. If the size does not match, then this
             * is an error.
             */
            newSize = boardcfgRmFindCertSize(msg);

            if (newSize == 0U){
                req->tisci_boardcfg_rm_size -= adjSize;
            }
            else if (newSize == adjSize){
                req->tisci_boardcfg_rm_size -= adjSize;
                req->tisci_boardcfg_rmp_low += adjSize;
            }
            else{
                r = CSL_EFAIL;
            }
    }

    return r;
}

static int32_t tisci_msg_board_config_rm_handler(uint32_t *msg_recv)
{
    int32_t r = CSL_PASS;
    struct tisci_msg_board_config_rm_req *req =
        (struct tisci_msg_board_config_rm_req *) msg_recv;

    r = boardcfg_rm_receive_and_validate(req->hdr.host,
                         req->tisci_boardcfg_rmp_low,
                         req->tisci_boardcfg_rmp_high,
                         req->tisci_boardcfg_rm_size,
                         req->tisci_boardcfg_rm_devgrp);

    if (r == CSL_PASS) {
        r = rm_init();
    }

    return r;
}

int32_t Sciclient_ProcessRmMessage(void *tx_msg)
{
    Sciclient_printf("Entering Sciclient_ProcessRmMessage function\n");
    int32_t r = CSL_PASS;
    bool msg_inval = (bool)false;
    uint32_t msgType = ((struct tisci_header *) tx_msg)->type;

    switch (msgType) {
        case TISCI_MSG_BOARD_CONFIG_RM:
            r = tisci_msg_board_config_rm_handler((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_BOARD_CONFIG_RM\n");
            break;
        case TISCI_MSG_RM_GET_RESOURCE_RANGE:
            r = rm_core_get_resource_range((uint32_t *)tx_msg, (uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_GET_RESOURCE_RANGE\n");
            break;
        case TISCI_MSG_RM_IRQ_SET:
            r = rm_irq_set((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_IRQ_SET\n");
            break;
        case TISCI_MSG_RM_IRQ_RELEASE:
            r = rm_irq_release((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_IRQ_RELEASE\n");
            break;
        case TISCI_MSG_RM_RING_CFG:
            r = rm_ra_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_RING_CFG\n");
            break;
        case TISCI_MSG_RM_RING_MON_CFG:
            r = rm_ra_mon_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_RING_MON_CFG\n");
            break;
        case TISCI_MSG_RM_UDMAP_TX_CH_CFG:
            r = rm_udmap_tx_ch_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_TX_CH_CFG\n");
            break;
        case TISCI_MSG_RM_UDMAP_RX_CH_CFG:
            r = rm_udmap_rx_ch_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_RX_CH_CFG\n");
            break;
        case TISCI_MSG_RM_UDMAP_FLOW_CFG:
            r = rm_udmap_flow_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_FLOW_CFG\n");
            break;
        case TISCI_MSG_RM_UDMAP_FLOW_SIZE_THRESH_CFG:
            r = rm_udmap_flow_size_thresh_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_FLOW_SIZE_THRESH_CFG\n");
            break;
        case TISCI_MSG_RM_UDMAP_FLOW_DELEGATE:
            r = rm_udmap_flow_delegate((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_FLOW_DELEGATE\n");
            break;
        case TISCI_MSG_RM_UDMAP_GCFG_CFG:
            r = rm_udmap_gcfg_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_UDMAP_GCFG_CFG\n");
            break;
        case TISCI_MSG_RM_PROXY_CFG:
            r = rm_proxy_cfg((uint32_t *)tx_msg);
            Sciclient_printf("case: TISCI_MSG_RM_PROXY_CFG\n");
            break;
        default:
            r = CSL_EFAIL;
            msg_inval = (bool)true;
            break;
    }

    if ((((struct tisci_header *) tx_msg)->flags & TISCI_MSG_FLAG_AOP) != 0U) {
        if (r != CSL_PASS) {
            Sciclient_TisciMsgSetNakResp((struct tisci_header *)tx_msg);
        } else {
            Sciclient_TisciMsgSetAckResp((struct tisci_header *)tx_msg);
        }
    }

    /*
     * Avoid overwriting status of calling message handler if the message
     * handler has a failure status.
     */
    if ((r != CSL_PASS) && !msg_inval) {
        /*
         * NACK will be sent in message if error so send CSL_PASS back
         * in return status
         */
        r = CSL_PASS;
    }

    if(CSL_PASS != r)
    {
        Sciclient_printf("ERROR:: Exiting Sciclient_ProcessRmMessage function with status FAIL\n");
    }
    else
    {
        Sciclient_printf("Exiting Sciclient_ProcessRmMessage function with status PASS\n");
    }
    return r;
}

int32_t Sciclient_processDMVersionMessage(void *tx_msg)
{
    int32_t ret = CSL_PASS;

    if (tx_msg == NULL)
    {
        ret = CSL_EBADARGS;
    }

    if (ret == CSL_PASS)
    {
        struct tisci_msg_dm_version_resp *resp_prms = ((struct tisci_msg_dm_version_resp *)(tx_msg));
        const char rm_pm_hal_version[] = RMPMHAL_SCMVERSION;
        const char sciserver_version[] = SCISERVER_DMVERSION;
        resp_prms->version = RMPMHAL_MAJORVERSION;
        resp_prms->sub_version = RMPMHAL_SUBVERSION;
        resp_prms->patch_version = RMPMHAL_PATCHVERSION;
        resp_prms->abi_major = RMPMHAL_ABIMAJOR;
        resp_prms->abi_minor = RMPMHAL_ABIMINOR;
        memset(resp_prms->rm_pm_hal_version, 0, 12UL);
        memset(resp_prms->sciserver_version, 0, 26UL);

        memcpy(resp_prms->rm_pm_hal_version, rm_pm_hal_version, (((strlen(rm_pm_hal_version)+1UL)>12UL)?11UL:(strlen(rm_pm_hal_version)+1UL)));
        resp_prms->rm_pm_hal_version[11] = '\0';
        if (strcmp(resp_prms->rm_pm_hal_version, rm_pm_hal_version) != 0) {
            ret = CSL_EFAIL;
        }

        if (ret == CSL_PASS)
        {
            memcpy(resp_prms->sciserver_version, sciserver_version, (((strlen(sciserver_version)+1UL)>26UL)?25UL:(strlen(sciserver_version)+1UL)));
            resp_prms->sciserver_version[25] = '\0';
            if (strcmp(resp_prms->sciserver_version, sciserver_version) != 0) {
                ret = CSL_EFAIL;
            }
        }

        if ((((struct tisci_header *) tx_msg)->flags & TISCI_MSG_FLAG_AOP) != 0U) {
            if (ret != CSL_PASS) {
                Sciclient_TisciMsgSetNakResp((struct tisci_header *)tx_msg);
            } else {
                Sciclient_TisciMsgSetAckResp((struct tisci_header *)tx_msg);
            }
        }
    }

    return ret;
}

int32_t Sciclient_boardCfgPrepHeader (
    uint8_t * pCommonHeader, uint8_t * pBoardCfgHeader,
    const Sciclient_BoardCfgPrms_t * pInPmPrms,
    const Sciclient_BoardCfgPrms_t * pInRmPrms)
{
    int32_t ret = CSL_PASS;
    if ((pCommonHeader == NULL) || (pBoardCfgHeader == NULL) ||
        (pInPmPrms == NULL) || (pInRmPrms == NULL))
    {
        ret = CSL_EBADARGS;
    }
    /* Populate the common header which will be loaded by ROM in case of
     * combined boot image format.
     */
    if (CSL_PASS == ret)
    {
        Sciclient_DirectExtBootX509Table  *pX509Table =
            (Sciclient_DirectExtBootX509Table *) pCommonHeader;
        memcpy(pX509Table->magic_word, gcSciclientDirectExtBootX509MagicWord,
               sizeof(gcSciclientDirectExtBootX509MagicWord));
        pX509Table->num_comps = 1;
        pX509Table->comps[0].comp_type =
            SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SBL_DATA;
        pX509Table->comps[0].boot_core = 0x10U;
        pX509Table->comps[0].comp_opts = 0U;
        pX509Table->comps[0].dest_addr = (uint64_t) pBoardCfgHeader;
        pX509Table->comps[0].comp_size =
            sizeof(Sciclient_DirectBoardCfgDescTable) +
            SCICLIENT_BOARDCFG_PM_SIZE_IN_BYTES +
            SCICLIENT_BOARDCFG_RM_SIZE_IN_BYTES;
    }
    /* Populate the Board config structure */
    if (CSL_PASS == ret)
    {
        Sciclient_DirectBoardCfgDescTable * pBoardCfgDesc =
            (Sciclient_DirectBoardCfgDescTable *) pBoardCfgHeader;
        pBoardCfgDesc->num_elems = 2U;
        pBoardCfgDesc->sw_rev  = 0U; /* Not Used for RM and PM */
        pBoardCfgDesc->descs[0].type = TISCI_MSG_BOARD_CONFIG_PM;
        pBoardCfgDesc->descs[0].offset = (uint16_t) ((uint32_t) pInPmPrms->boardConfigLow -
                                         (uint32_t) pBoardCfgHeader);
        pBoardCfgDesc->descs[0].size = pInPmPrms->boardConfigSize;
        pBoardCfgDesc->descs[0].devgrp = pInPmPrms->devGrp;
        pBoardCfgDesc->descs[0].reserved = 0x0;
        pBoardCfgDesc->descs[1].type = TISCI_MSG_BOARD_CONFIG_RM;
        pBoardCfgDesc->descs[1].offset =
            (uint16_t) ((uint32_t) pInRmPrms->boardConfigLow -
            (uint32_t) pBoardCfgHeader);
        pBoardCfgDesc->descs[1].size = pInRmPrms->boardConfigSize;
        pBoardCfgDesc->descs[1].devgrp = pInRmPrms->devGrp;
        pBoardCfgDesc->descs[1].reserved = 0x0;
    }
    return ret;
}

int32_t Sciclient_boardCfgParseHeader (
    uint8_t * pCommonHeader,
    Sciclient_BoardCfgPrms_t * pInPmPrms,
    Sciclient_BoardCfgPrms_t * pInRmPrms)
{
    int32_t ret = CSL_PASS;
    Sciclient_DirectExtBootX509Table  *pX509Table =
            (Sciclient_DirectExtBootX509Table *) pCommonHeader;
    if ((pCommonHeader == NULL) || (pInPmPrms == NULL) || (pInRmPrms == NULL))
    {
        ret = CSL_EBADARGS;
    }
    /* Populate the common header which will be loaded by ROM in case of
     * combined boot image format.
     */
    if (CSL_PASS == ret)
    {
        if (memcmp(pX509Table->magic_word,
            gcSciclientDirectExtBootX509MagicWord,
            sizeof(gcSciclientDirectExtBootX509MagicWord)) == 0)
        {
            ret = CSL_PASS;
        }
        else
        {
            ret = CSL_EFAIL;
        }
    }
    if (CSL_PASS == ret)
    {
        uint32_t i = 0U;
        uint32_t foundRm = 0U, foundPm = 0U;
        uint32_t j = 0U;
        for (j = 0U; j < pX509Table->num_comps; j++)
        {
            uint32_t addr = (uint32_t) (pX509Table->comps[j].dest_addr
                 & (uint64_t) 0xFFFFFFFFU);
            if (pX509Table->comps[j].comp_type !=
                (uint32_t)SCICLIENT_DIRECT_EXTBOOT_X509_COMPTYPE_SBL_DATA)
            {
                continue;
            }
            Sciclient_DirectBoardCfgDescTable *pBoardCfgDesc =
                (Sciclient_DirectBoardCfgDescTable *) addr;
            for (i = 0U; i < pBoardCfgDesc->num_elems; i++)
            {
                if (pBoardCfgDesc->descs[i].type == TISCI_MSG_BOARD_CONFIG_PM)
                {
                    pInPmPrms->boardConfigLow = pBoardCfgDesc->descs[i].offset +
                        (uint32_t) pBoardCfgDesc;
                    pInPmPrms->boardConfigSize = pBoardCfgDesc->descs[i].size;
                    pInPmPrms->devGrp = pBoardCfgDesc->descs[i].devgrp;
                    pInPmPrms->boardConfigHigh = 0U;
                    foundPm = 1U;
                }
                if (pBoardCfgDesc->descs[i].type == TISCI_MSG_BOARD_CONFIG_RM)
                {
                    pInRmPrms->boardConfigLow = pBoardCfgDesc->descs[i].offset +
                        (uint32_t) pBoardCfgDesc;
                    pInRmPrms->boardConfigSize = pBoardCfgDesc->descs[i].size;
                    pInRmPrms->devGrp = pBoardCfgDesc->descs[i].devgrp;
                    pInRmPrms->boardConfigHigh = 0U;
                    foundRm = 1U;
                }
            }
        }
        if ((foundPm == 1U) && (foundRm == 1U))
        {
            ret = CSL_PASS;
        }
        else
        {
            ret = CSL_EFAIL;
        }
    }
    return ret;
}

