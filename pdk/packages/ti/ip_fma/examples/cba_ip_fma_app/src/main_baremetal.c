/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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

/**
 *  \file main_baremetal.c
 *
 *  \brief Example application that does periodic readback of selected 
 *         static CBA registers and also some QOS and ERR register tests.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/soc.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/hw_types.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/board.h>

#include "ip_fma_common.h"
#include "ip_fma_cba.h"
#include <sciclient.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define  CBASS_QOS_TEST_ID          (0U)
#define  CBASS_ERROR_TEST_ID        (1U)
#define  CBASS_TOTAL_NUM_TESTS      (2U)

/* Number of times CBA register check will be done */
#define CBA_PERIODIC_REGCHECK_NUM   (3U)

/* Firewall open and close message type IDs*/
#define  CBA_APP_TIFS_FWL_OPEN_MESSAGE      (0x902CU)
#define  CBA_APP_TIFS_FWL_CLOSE_MESSAGE     (0x902DU)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initializes the application by setting the configuration.
 *
 *  This is a initialization function that sets the board configuration.
 *  It configures the board by enabling UART module needed to print log
 *  messages to the user via UART console.
 *
 *  \return  BOARD_SOK in case of success or appropriate error code.
 */
static int32_t CbaApp_Init();

/**
 * \brief   Function that invokes CBASS QOS module and Error module
 *          test functions.
 * 
 *  This is a test function which tests QOS and Error modules by setting
 *  up some of their registers using QOS and Error set and get API calls.
 *
 * \param   testId Test ID
 * 
 * \retval  CSL_PASS in case of success or appropriate error code.
 */
static int32_t CbaApp_Test(uint32_t testId);

extern int32_t CbaApp_CbassQosTest(void);
extern int32_t CbaApp_CbassErrTest(void);

/**
 *  \brief   Sends a request to TIFS (TI Foundational Security) to open a Firewall region.
 *
 *  This function constructs and sends a TISCI (Texas Instruments System Control Interface)
 *  message to the TIFS firmware to open a firewall region for access. It uses the Sciclient
 *  service API to communicate with the system controller.
 *
 *  The function prepares the request and response structures, sets the message type to
 *  `CBA_APP_TIFS_FWL_OPEN_MESSAGE`, and waits for an acknowledgment from TIFS.
 *
 *  \param   None
 *
 *  \return  IPFMA_OK on success, or IPFMA_E_IO if the request fails or is not acknowledged.
 */
static int32_t CbaApp_TifsReqFwlOpen(void);

/**
 *  \brief   Sends a request to TIFS to close a Firewall region.
 *
 *  This function constructs and sends a TISCI message to the TIFS firmware to close a
 *  previously opened firewall region. It uses the Sciclient service API to communicate
 *  with the system controller.
 *
 *  The function prepares the request and response structures, sets the message type to
 *  `CBA_APP_TIFS_FWL_CLOSE_MESSAGE`, and waits for an acknowledgment from TIFS.
 *
 *  \param   None
 *
 *  \return  IPFMA_OK on success, or IPFMA_E_IO if the request fails or is not acknowledged.
 */
static int32_t CbaApp_TifsReqFwlClose(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int32_t main(void)
{
#if defined (FAULT_INJECTION_ENABLED)
    uint32_t testId;
#endif
    int32_t  retVal = -1;
    int32_t  status = IPFMA_OK;

    int32_t status_fwl_open = IPFMA_OK;
    int32_t status_fwl_close = IPFMA_OK;

    /* Init Board */
    retVal = CbaApp_Init();
    UART_printf("Application Init Done\n");


    /* You need to open firewall in order to read firewall registers */
    status_fwl_open = CbaApp_TifsReqFwlOpen();

    if (status_fwl_open == IPFMA_OK)
    {
        UART_printf("Firewall open successful\r\n");
    }

    /* Read all registers to get expected values for comparison */
    IpFma_CbaErrRegs cbaErrRegs;
    IpFma_CbaFwRegs cbaFwRegs;
    IpFma_CbaFwchRegs cbaFwchRegs;
    IpFma_CbaIscRegs cbaIscRegs;
    IpFma_CbaMapRegs cbaMapRegs;

    status = IpFma_Cba_GetErrRegs(CSL_MCU_CBASS0_ERR_BASE, &cbaErrRegs);
    
    if (IPFMA_OK == status)
    {
        status = IpFma_Cba_GetFwRegs(CSL_MCU_CBASS0_FW_BASE, &cbaFwRegs);

        if (IPFMA_OK == status)
        {        
            status = IpFma_Cba_GetFwchRegs(CSL_CH_FW_MCU_NAVSS0_PROXY0_PROXY0__CFG__BUF__CFG__CFG_MMR_BASE, &cbaFwchRegs);

            if (IPFMA_OK == status)
            {
                status = IpFma_Cba_GetIscRegs(CSL_MCU_CBASS0_ISC_BASE, &cbaIscRegs);

                if (IPFMA_OK == status)
                {
                        status = IpFma_Cba_GetMapRegs(CSL_MCU_CBASS0_QOS_BASE, &cbaMapRegs);
                }
            }
        }
    }

    if (BOARD_SOK == retVal && IPFMA_OK == status)
    {
        int8_t i = 0;
        while (i < CBA_PERIODIC_REGCHECK_NUM)
        {

#if defined (FAULT_INJECTION_ENABLED)
            if (i == CBA_PERIODIC_REGCHECK_NUM - 1)
            {
                UART_printf("\n\n---------------------------------------------------\n");
                UART_printf("CBASS Tests:");
                for (testId = ((uint32_t)(0U)); testId < CBASS_TOTAL_NUM_TESTS; testId++)
                {
                    retVal = CbaApp_Test(testId);
                    if (retVal != CSL_PASS)
                    {
                        break;
                    }
                }
                UART_printf("---------------------------------------------------\n");
            }
#endif
            //////////////////////////////////////////////////////////////////////
            // Register readback //
            //////////////////////////////////////////////////////////////////////

            UART_printf("\n\n***************************************************\n");
            UART_printf("Periodic Register Readback\n");


            /* ERR Register Group */
            IpFma_CbaErrRegs cbaErrRegsActualValues;
            status = IpFma_Cba_GetErrRegs(CSL_MCU_CBASS0_ERR_BASE, &cbaErrRegsActualValues);

            if (IPFMA_OK == status)
            {
                /* Compare expected and actual values periodically */
                UART_printf("\n\nComparing expected-actual ERROR register values...\n");
                status = IpFma_Cba_CompareErrRegs(&cbaErrRegs, &cbaErrRegsActualValues);
                if (IPFMA_OK == status)
                {
                    UART_printf(".. values MATCH!\n");
                }
                else
                {
                    UART_printf(".. values MISMATCH!\n");
                }

            }

            /* FW Register Group */
            IpFma_CbaFwRegs cbaFwRegsActualValues;
            UART_printf("\n\nComparing expected-actual FW register values...\n");
            status = IpFma_Cba_GetFwRegs(CSL_MCU_CBASS0_FW_BASE, &cbaFwRegsActualValues);

            if (IPFMA_OK == status)
            {
                status = IpFma_Cba_CompareFwRegs(&cbaFwRegs, &cbaFwRegsActualValues);
                if (IPFMA_OK == status)
                {
                    UART_printf(".. values MATCH!\n");
                }
                else
                {
                    UART_printf(".. values MISMATCH!\n");
                }
            }

            /* FWCH Register Group */
            IpFma_CbaFwchRegs cbaFwchRegsActualValues;
            UART_printf("\n\nComparing expected-actual FWCH register values...\n");
            status = IpFma_Cba_GetFwchRegs(CSL_CH_FW_MCU_NAVSS0_PROXY0_PROXY0__CFG__BUF__CFG__CFG_MMR_BASE, &cbaFwchRegsActualValues);
            if (IPFMA_OK == status)
            {
                status = IpFma_Cba_CompareFwchRegs(&cbaFwchRegs, &cbaFwchRegsActualValues);
                if (IPFMA_OK == status)
                {
                    UART_printf(".. values MATCH!\n");
                }
                else
                {
                    UART_printf(".. values MISMATCH!\n");
                }
            }

            /* ISC Register Group */
            IpFma_CbaIscRegs cbaIscRegsActualValues;
            UART_printf("\n\nComparing expected-actual ISC register values...\n");
            status = IpFma_Cba_GetIscRegs(CSL_MCU_CBASS0_ISC_BASE, &cbaIscRegsActualValues);
            
            if (IPFMA_OK == status)
            {
                status = IpFma_Cba_CompareIscRegs(&cbaIscRegs, &cbaIscRegsActualValues);
                if (IPFMA_OK == status)
                {
                    UART_printf(".. values MATCH!\n");
                }
                else
                {
                    UART_printf(".. values MISMATCH!\n");
                }
            }

            /* MAP Register Group */
            IpFma_CbaMapRegs cbaMapRegsActualValues;
            UART_printf("\n\nComparing expected-actual QoS/MAP register values...\n");
            status = IpFma_Cba_GetMapRegs(CSL_MCU_CBASS0_QOS_BASE, &cbaMapRegsActualValues);
            
            if (IPFMA_OK == status)
            {
                status = IpFma_Cba_CompareMapRegs(&cbaMapRegs, &cbaMapRegsActualValues);
                if (IPFMA_OK == status)
                {
                    UART_printf(".. values MATCH!\n");
                }
                else
                {
                    UART_printf(".. values MISMATCH!\n");
                }
            }

            i++;
        }
    }

    if (CSL_PASS == retVal && IPFMA_OK == status)
    {
        UART_printf("\r\n All tests have passed. \r\n");
    }
    else
    {
        UART_printf("\r\n Some tests have failed. \r\n");
    }


    if (status_fwl_open == IPFMA_OK)
    {
        status_fwl_close = CbaApp_TifsReqFwlClose();
    }

    if (status_fwl_close == IPFMA_OK)
    {
        UART_printf("Firewall close successful\r\n");
    }

    UART_printf("\r\n Application DONE! \r\n");
    return retVal;
}

static int32_t CbaApp_Init()
{
    int32_t status = -1;
    Board_initCfg boardCfg;

    boardCfg = BOARD_INIT_PINMUX_CONFIG | BOARD_INIT_UART_STDIO;
    status   = Board_init(boardCfg);

    return status;
}

static int32_t CbaApp_Test(uint32_t testId)
{
    int32_t    retVal = 0;

    switch (testId)
    {
        /* CBASS QoS test */
        case CBASS_QOS_TEST_ID:

            retVal = CbaApp_CbassQosTest();
            UART_printf("\nCBASS QoS Module Test");
            if (retVal == CSL_PASS)
            {
                UART_printf(" Passed.\r\n");
            }
            else
            {
                UART_printf(" Failed.\r\n");
            }
            break;

        /* CBASS Error module test */
        case CBASS_ERROR_TEST_ID:

            retVal = CbaApp_CbassErrTest();
            UART_printf("\nCBASS Error Module Test");
            if (retVal == CSL_PASS)
            {
                UART_printf(" Passed.\r\n");
            }
            else
            {
                UART_printf(" Failed.\r\n");
            }
            break;

        default:
            UART_printf("\n[Error] Invalid CBASS test ID.\r\n");
            break;
    }

    return (retVal);
}

static int32_t CbaApp_TifsReqFwlOpen(void)
{
    int32_t status = IPFMA_OK;

    struct tisci_fwl_req request = {0};
    struct tisci_fwl_resp response = {0};

    Sciclient_ReqPrm_t  reqParam = {0};
    Sciclient_RespPrm_t respParam = {0};

    reqParam.messageType    = (uint16_t) CBA_APP_TIFS_FWL_OPEN_MESSAGE;
    reqParam.flags          = (uint32_t) TISCI_MSG_FLAG_AOP;
    reqParam.pReqPayload    = (const uint8_t *) &request;
    reqParam.reqPayloadSize = (uint32_t) sizeof (request);
    reqParam.timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER;

    respParam.flags           = (uint32_t) 0;   /* Populated by the API */
    respParam.pRespPayload    = (uint8_t *) &response;
    respParam.respPayloadSize = (uint32_t)  sizeof (response);

    status = Sciclient_service(&reqParam, &respParam);
    if ((status == IPFMA_OK)  && (respParam.flags == TISCI_MSG_FLAG_ACK))
    {
        status = IPFMA_OK;
    }
    else
    {
        status = IPFMA_E_IO;
    }
    return status;
}

static int32_t CbaApp_TifsReqFwlClose(void)
{
    int32_t status = IPFMA_OK;

    struct tisci_fwl_req request;
    struct tisci_fwl_resp response;

    Sciclient_ReqPrm_t  reqParam = {0};
    Sciclient_RespPrm_t respParam = {0};

    reqParam.messageType    = (uint16_t) CBA_APP_TIFS_FWL_CLOSE_MESSAGE;
    reqParam.flags          = (uint32_t) TISCI_MSG_FLAG_AOP;
    reqParam.pReqPayload    = (const uint8_t *) &request;
    reqParam.reqPayloadSize = (uint32_t) sizeof (request);
    reqParam.timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER;

    respParam.flags           = (uint32_t) 0;   /* Populated by the API */
    respParam.pRespPayload    = (uint8_t *) &response;
    respParam.respPayloadSize = (uint32_t)  sizeof (response);

   status = Sciclient_service(&reqParam, &respParam);
   if((status == IPFMA_OK) && (respParam.flags == TISCI_MSG_FLAG_ACK))
   {
        status = IPFMA_OK;
   }
   else
   {
        status = IPFMA_E_IO;
   }
   return status;
}
