/*
 *  Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
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
  *  \file  sciclient_extended_ut_main.c
  *
  *  \brief Implementation of Sciclient Extended Unit Test application
  *
  */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <ti/csl/csl_psilcfg.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_priv.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_rm_priv.h>
#include <ti/drv/sciclient/examples/common/sci_app_common.h>
#include <ti/drv/sciclient/examples/sciclient_extended_unit_testapp/sciclient_extended_ut_tests.h>
#include <ti/drv/sciclient/sciserver.h>
#include <ti/csl/soc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#if defined (SOC_J721E)
#define TISCI_DEV_NAVSS0_MODSS_INTAGG     TISCI_DEV_NAVSS0_MODSS_INTAGGR_0
#define TISCI_DEV_NAVSS0_INTR             TISCI_DEV_NAVSS0_INTR_ROUTER_0
#define TISCI_DEV_NAVSS0_MAILBOX          TISCI_DEV_NAVSS0_MAILBOX_0
#define TISCI_DEV_MCU_NAVSS0_INTR         TISCI_DEV_MCU_NAVSS0_INTR_0
#define TISCI_DEV_NAVSS0_MODSS_INTAGG1    TISCI_DEV_NAVSS0_MODSS_INTAGGR_1
#define TISCI_DEV_NAVSS0_UDMASS_INTAGG    TISCI_DEV_NAVSS0_UDMASS_INTAGGR_0
#define SCICLIENT_APP_MCU_SRAM_FWL_ID     CSL_STD_FW_MCU_MSRAM_1MB0_SLV_ID
#elif defined (SOC_J7200)
#define TISCI_DEV_NAVSS0_MODSS_INTAGG     TISCI_DEV_NAVSS0_MODSS_INTA_0
#define TISCI_DEV_NAVSS0_INTR             TISCI_DEV_NAVSS0_INTR_ROUTER_0
#define TISCI_DEV_NAVSS0_MAILBOX          TISCI_DEV_NAVSS0_MAILBOX_0
#define TISCI_DEV_MCU_NAVSS0_INTR         TISCI_DEV_MCU_NAVSS0_INTR_0
#define TISCI_DEV_NAVSS0_MODSS_INTAGG1    TISCI_DEV_NAVSS0_MODSS_INTA_1
#define TISCI_DEV_NAVSS0_UDMASS_INTAGG    TISCI_DEV_NAVSS0_UDMASS_INTA_0
#define SCICLIENT_APP_MCU_SRAM_FWL_ID     CSL_STD_FW_MCU_MSRAM_1MB0_RAM_ID
#elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
#define TISCI_DEV_NAVSS0_MODSS_INTAGG     TISCI_DEV_NAVSS0_MODSS_INTA_0
#define TISCI_DEV_NAVSS0_INTR             TISCI_DEV_NAVSS0_INTR_0
#define TISCI_DEV_NAVSS0_MAILBOX          TISCI_DEV_NAVSS0_MAILBOX1_0
#define TISCI_DEV_MCU_NAVSS0_INTR         TISCI_DEV_MCU_NAVSS0_INTR_ROUTER_0
#define TISCI_DEV_NAVSS0_MODSS_INTAGG1    TISCI_DEV_NAVSS0_MODSS_INTA_1
#define TISCI_DEV_NAVSS0_UDMASS_INTAGG    TISCI_DEV_NAVSS0_UDMASS_INTA_0
#define SCICLIENT_APP_MCU_SRAM_FWL_ID     CSL_STD_FW_MCU_MSRAM_1MB0_RAM_ID
#endif
#define SCICLIENT_APP_MCU_1_0_PRIVID  96
#define SCICLIENT_APP_C7x_1_PRIVID    21

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern Sciclient_ServiceHandle_t gSciclientHandle;
extern CSL_SecProxyCfg *pSciclient_secProxyCfg;
extern const char gcSciclientDirectExtBootX509MagicWord[8];
extern uint32_t gSciclient_writeInProgress;

/* For SafeRTOS on R5F with FFI Support, task stack should be aligned to the stack size */
/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 32KB
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
#if defined(SAFERTOS) && defined (BUILD_MCU)
static uint8_t  gSciclientAppTskStackMain[32*1024] __attribute__((aligned(32*1024))) = { 0 };
#else
static uint8_t  gSciclientAppTskStackMain[32*1024] __attribute__((aligned(8192)));
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

struct SciApp_RangeOfLines {
    uint16_t src_start;
    uint16_t src_end;
    uint16_t dst_start;
    uint16_t dst_end;
};

typedef struct {
    uint32_t comp_type;
    uint32_t boot_core;
    uint32_t comp_opts;
    uint64_t dest_addr;
    uint32_t comp_size;
} SciApp_extBootX509Comp;

typedef struct {
    uint8_t                magic_word[8];
    uint32_t               num_comps;
    SciApp_extBootX509Comp comps[8];
} SciApp_extBootX509Table;

typedef struct {
    uint16_t type;
    uint16_t offset;
    uint16_t size;
    uint8_t  devgrp;
    uint8_t  reserved;
} __attribute__((__packed__)) SciApp_extBootBoardCfgDesc;

typedef struct {
    uint8_t                    num_elems;
    uint8_t                    sw_rev;
    SciApp_extBootBoardCfgDesc descs[4];
} __attribute__((__packed__)) SciApp_boardCfgDescTable;

/* ========================================================================== */
/*                         Function Declarations                              */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                        Internal Function Declarations                      */
/* ========================================================================== */

static void mainTask(void* arg0, void* arg1);

static int32_t SciclientApp_sciclientTest(void);
static int32_t SciclientApp_rmTest(void);
static int32_t SciclientApp_firewallTest(void);
static int32_t SciclientApp_genericMsgsTest(void);
static int32_t SciclientApp_rmIrqTest(void);
static int32_t SciclientApp_procbootTest(void);
#if !defined(BUILD_MCU1_1)
static int32_t SciclientApp_pmTest(void);
#endif
#if defined (BUILD_MCU1_0)
static int32_t SciclientApp_boardcfgTest(void);
static int32_t SciclientApp_directTest(void);
#endif
static int32_t SciclientApp_dkekTest(void);
static int32_t SciclientApp_romTest(void);
static int32_t SciclientApp_secureproxyTest(void);
static int32_t SciclientApp_osalTest(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    TaskP_Handle task;
    TaskP_Params taskParams;

    uint32_t retVal = CSL_PASS;

    /*  This should be called before any other OS calls (like Task creation, OS_start, etc..) */
    OS_init();

    memset(gSciclientAppTskStackMain, 0xFF, sizeof(gSciclientAppTskStackMain));
    TaskP_Params_init(&taskParams);
    taskParams.priority     = 2;
    taskParams.stack        = gSciclientAppTskStackMain;
    taskParams.stacksize    = sizeof(gSciclientAppTskStackMain);
    task = TaskP_create(&mainTask, &taskParams);
    if(task == NULL)
    {
        OS_stop();
    }

    OS_start();

    return retVal;
}

uint32_t SciApp_getNumTests(void)
{
    return SCICLIENT_NUM_TESTCASES;
}

int32_t SciApp_testMain(SciApp_TestParams_t *testParams)
{
    switch (testParams->testcaseId)
    {
        case 1:
            testParams->testResult = SciclientApp_sciclientTest();
            break;
        case 2:
            testParams->testResult =  SciclientApp_rmTest();
            break;
        case 3:
            testParams->testResult =  SciclientApp_firewallTest();
            break;
        case 4:
            testParams->testResult =  SciclientApp_genericMsgsTest();
            break;
        case 5:
            testParams->testResult = SciclientApp_rmIrqTest();
            break;
        case 6:
            testParams->testResult = SciclientApp_procbootTest();
            break;
#if !defined(BUILD_MCU1_1)
        case 7:
            testParams->testResult = SciclientApp_pmTest();
            break;
#endif
#if defined (BUILD_MCU1_0)
        case 8:
            testParams->testResult = SciclientApp_boardcfgTest();
            break;
        case 9:
            testParams->testResult = SciclientApp_directTest();
            break;
#endif
        case 10:
            testParams->testResult = SciclientApp_secureproxyTest();
            break;
        case 11:
            testParams->testResult = SciclientApp_dkekTest();
            break;
        case 12:
            testParams->testResult = SciclientApp_osalTest();
            break;
        case 13:
            testParams->testResult = SciclientApp_romTest();
            break;
        default:
            break;
    }
    return 0;
}

/* ========================================================================== */
/*                          Internal Function Definitions                     */
/* ========================================================================== */

static void mainTask(void* arg0, void* arg1)
{
    /*To suppress unused variable warning*/
    (void)arg0;
    (void)arg1;

    volatile uint32_t loopForever = 1U;

    SciApp_parser();

    while(loopForever);
}

static int32_t SciclientApp_prepareHeaderNegTest(void)
{
    int32_t  status                  = CSL_PASS;
    int32_t  sciclientTestStatus     = CSL_PASS;
    Sciclient_ReqPrm_t SciApp_ReqPrm = {};
    uint8_t localSeqId;
    uint32_t contextId               = 0;
    struct tisci_header *header;
    uint8_t payload;

    status = Sciclient_servicePrepareHeader(NULL, NULL, SCICLIENT_CONTEXT_R5_NONSEC_0, NULL);
    if (status == CSL_EBADARGS)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_servicePrepareHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_servicePrepareHeader: Negative Arg Test Failed.\n");
    }

    status = Sciclient_servicePrepareHeader(&SciApp_ReqPrm, NULL, SCICLIENT_CONTEXT_MAX_NUM, NULL);
    if (status == CSL_EBADARGS)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_servicePrepareHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_servicePrepareHeader: Negative Arg Test Failed.\n");
    }
    SciApp_ReqPrm.forwardStatus = SCISERVER_FORWARD_MSG;
    SciApp_ReqPrm.pReqPayload   = &payload;
    status = Sciclient_servicePrepareHeader(&SciApp_ReqPrm, &localSeqId, contextId, &header);
    if(status == CSL_PASS)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_servicePrepareHeader: Positive Arg Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_servicePrepareHeader: Positive Arg Test Passed.\n");
    }

    return sciclientTestStatus;
}

static int32_t SciclientApp_contextNegTest(void)
{
    int32_t            status              = CSL_PASS;
    int32_t            sciclientTestStatus = CSL_PASS;
    uint32_t           IntrNum1            = 1000;
    uint32_t           contextId           = SCICLIENT_CONTEXT_NONSEC;
    uint32_t           IntrNum2            = gSciclientMap[contextId].respIntrNum;
    uint16_t           messagetype[10]     = {TISCI_MSG_BOOT_NOTIFICATION,
                                              TISCI_MSG_BOARD_CONFIG,
                                              TISCI_MSG_BOARD_CONFIG_SECURITY,
                                              TISCI_MSG_BOARD_CONFIG_RM,
                                              TISCI_MSG_BOARD_CONFIG_PM,
                                              TISCI_MSG_SA2UL_SET_DKEK,
                                              TISCI_MSG_SA2UL_RELEASE_DKEK,
                                              TISCI_MSG_SA2UL_GET_DKEK
                                             };
    int8_t             num;
    Sciclient_ReqPrm_t reqPrm;

    /* Passing different interrupt numbers to check proxy map context id for 'gSciclientMap' */
    status = Sciclient_contextIdFromIntrNum(IntrNum1);
    if (status != CSL_PASS)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_contextIdFromIntrNum initial condition Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_contextIdFromIntrNum initial condition Test Failed.\n");
    }

    SciApp_printf("The interrupt number:IntrNum2 is %d\n",IntrNum2);
    status = Sciclient_contextIdFromIntrNum(IntrNum2);
    if (status != CSL_EFAIL)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_contextIdFromIntrNum: IntrNum2 Arg Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_contextIdFromIntrNum: IntrNum2 Arg Test Failed.\n");
    }

    /* Passing different message types to determine the which context to be used. */
    for(num = 0; num < 8; num++)
    {
        reqPrm.messageType = messagetype[num];
        status = Sciclient_getCurrentContext((const Sciclient_ReqPrm_t *)&reqPrm);
        if (status == SCICLIENT_CONTEXT_SEC)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_getCurrentContext: Secure Arg Test Passed.\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_getCurrentContext: Secure Arg Test Failed.\n");
        }
    }

    gSciclientHandle.isSecureMode = 1;
    reqPrm.messageType = 0U;
    status = Sciclient_getCurrentContext((const Sciclient_ReqPrm_t *)&reqPrm);
    if (status == SCICLIENT_CONTEXT_SEC)
    {
        sciclientTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_getCurrentContext: Secure Arg Test Passed.\n");
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_getCurrentContext: Secure Arg Test Failed.\n");
    }

    return sciclientTestStatus;
}

static int32_t SciclientApp_initTest(void)
{
    int32_t status              = CSL_PASS;
    int32_t sciclientInitStatus = CSL_PASS;
    Sciclient_ServiceHandle_t gSciclientHandleTemp;
    Sciclient_ConfigPrms_t SciApp_Config =
    {
       2,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    Sciclient_ConfigPrms_t SciApp_ConfigIntrMode =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       2 /* isSecure = 0 un secured for all cores */
    };

    Sciclient_deinit();
    status = Sciclient_init(&SciApp_Config);
    if (status == CSL_EBADARGS)
    {
        sciclientInitStatus += CSL_PASS;
        SciApp_printf("Sciclient_init: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientInitStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init: Negative Arg Test Failed.\n");
    }

    Sciclient_deinit();
    status = Sciclient_init(&SciApp_ConfigIntrMode);
    if (status == CSL_EBADARGS)
    {
        sciclientInitStatus += CSL_PASS;
        SciApp_printf("Sciclient_init: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientInitStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init: Negative Arg Test Failed.\n");
    }

    Sciclient_deinit();
    status = Sciclient_init(NULL);
    if (status == CSL_PASS)
    {
        sciclientInitStatus += CSL_PASS;
        SciApp_printf("Sciclient_init: NULL Arg Test Passed.\n");
    }
    else
    {
        sciclientInitStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init: NULL Arg Test Failed.\n");
    }

    Sciclient_deinit();
    gSciclientHandleTemp = gSciclientHandle;
    gSciclientHandle.initCount=1;
    status = Sciclient_init(&SciApp_Config);
    gSciclientHandle = gSciclientHandleTemp;
    if (status == CSL_PASS)
    {
        sciclientInitStatus += CSL_PASS;
        SciApp_printf("Sciclient_init: initCount Arg Test Passed.\n");
    }
    else
    {
        sciclientInitStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init: initCount Arg Test Failed.\n");
    }

    status = Sciclient_deinit();
    if(status == CSL_PASS)
    {
        sciclientInitStatus += CSL_PASS;
    }
    else
    {
        sciclientInitStatus += CSL_EFAIL;
    }

    return sciclientInitStatus;
}

static int32_t SciclientApp_sciclientMcdcTest(void)
{
    int32_t  status               = CSL_PASS;
    int32_t  sciclientInitStatus  = CSL_PASS;
    int32_t  sciclientTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_POLLED,
        NULL,
        0 /* isSecure = 0 un secured for all cores */
    };
    Sciclient_ReqPrm_t reqPrms;
    Sciclient_RespPrm_t respPrms;
    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        /* setting up PM and RM boardcfg parameters */
#if defined(BUILD_MCU1_0)
        Sciclient_BoardCfgPrms_t  pmBoardCfgParams = {
                .boardConfigLow  = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
                .boardConfigHigh = 0U,
                .boardConfigSize = SCICLIENT_BOARDCFG_PM_SIZE_IN_BYTES,
                .devGrp          = DEVGRP_ALL
        };
        Sciclient_BoardCfgPrms_t rmBoardCfgParams =  {
                .boardConfigLow  = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
                .boardConfigHigh = 0,
                .boardConfigSize = SCICLIENT_BOARDCFG_RM_SIZE_IN_BYTES,
                .devGrp          = DEVGRP_ALL
        };
        status = Sciclient_boardCfgPm(&pmBoardCfgParams);
        if(status == CSL_PASS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgPm(): Execution successful\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgPm(): Execution failed\n");
        }

        status = Sciclient_boardCfgRm(&rmBoardCfgParams);
        if(status == CSL_PASS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgRm(): Execution successful\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgRm(): Execution failed\n");
        }

        status = Sciclient_configPrmsInit(&config);
        if(status == CSL_PASS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_configPrmsInit test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_configPrmsInit test Failed\n");
        }
#endif

        /* passing invalid arguments */
        while (gSciclientHandle.initCount != 0)
        {
            status = Sciclient_deinit();
        }
        config.opModeFlag = 2U;
        status = Sciclient_init(&config);
        if(status == CSL_EBADARGS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_init Negative test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_init Negative test Failed\n");
        }

        while (gSciclientHandle.initCount != 0)
        {
            status = Sciclient_deinit();
        }
        config.opModeFlag = SCICLIENT_SERVICE_OPERATION_MODE_POLLED;
        config.isSecureMode = 2U;
        status = Sciclient_init(&config);
        if(status == CSL_EBADARGS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_init Negative test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_init Negative test Failed\n");
        }

        /* passing NULL argument */
        while (gSciclientHandle.initCount != 0)
        {
            status = Sciclient_deinit();
        }
        status = Sciclient_serviceSecureProxy(&reqPrms, NULL);
        if(status == CSL_EBADARGS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_serviceGetPayloadSize Negative test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_serviceGetPayloadSize Negative test Failed\n");
        }

        reqPrms.pReqPayload = NULL;
        reqPrms.reqPayloadSize = sizeof(reqPrms.pReqPayload);
        status = Sciclient_serviceSecureProxy(&reqPrms, &respPrms);
        if(status == CSL_EBADARGS)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_serviceGetPayloadSize Negative test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_serviceGetPayloadSize Negative test Failed\n");
        }

        status = Sciclient_configPrmsInit(NULL);
        if(status == CSL_EFAIL)
        {
            sciclientTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_configPrmsInit Negative test Passed\n");
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_configPrmsInit Negative test Failed\n");
        }
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
    }

    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            sciclientTestStatus += CSL_PASS;
        }
        else
        {
            sciclientTestStatus += CSL_EFAIL;
        }
    }

    return sciclientTestStatus;
}

static int32_t SciclientApp_deinitTest(void)
{
    int32_t status               = CSL_PASS;
    Sciclient_ServiceHandle_t gSciclientHandleTemp = gSciclientHandle;
    gSciclientHandle.initCount   = 1U;
    gSciclientHandle.opModeFlag  = SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT;
    gSciclientHandle.respIntr[0] = NULL;
    gSciclientHandle.respIntr[1] = NULL;

    status = Sciclient_deinit();
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient deinit NULL test for interrupt has passed.\n");
        status = CSL_PASS;
    }
    else
    {
        SciApp_printf("Sciclient deinit NULL test for interrupt has failed.\n");
        status = CSL_EFAIL;
    }

    gSciclientHandle = gSciclientHandleTemp;

    return status;
}

static int32_t SciclientApp_getPayloadSizeTest(void)
{
    int32_t status = CSL_PASS;
    Sciclient_ReqPrm_t  pReqPrm;
    Sciclient_RespPrm_t pRespPrm;
    pReqPrm.messageType = TISCI_MSG_BOARD_CONFIG_RM;

    status = Sciclient_serviceSecureProxy(&pReqPrm, &pRespPrm);
    if(status != CSL_PASS)
    {
        status = CSL_PASS;
        SciApp_printf("SciclientApp_getPayloadSize Negative Test passed.\n");
    }
    else
    {
        status = CSL_EFAIL;
        SciApp_printf("SciclientApp_getPayloadSize Negative Test failed.\n");
    }

    return status;
}

static int32_t SciclientApp_getThreadIdsTest(void)
{
    int status = CSL_PASS;
    Sciclient_ReqPrm_t pReqPrm;
    pReqPrm.messageType   = TISCI_MSG_BOARD_CONFIG_RM;
    pReqPrm.forwardStatus = SCISERVER_FORWARD_MSG;
    uint32_t contextId, txThread, rxThread;
    status = Sciclient_serviceGetThreadIds(&pReqPrm, &contextId, &txThread, &rxThread);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_serviceGetThreadIds Positive Test passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_serviceGetThreadIds Positive Test failed.\n");
    }

    return status;
}

static int32_t SciclientApp_sciclientTest(void)
{
    int32_t  status               = CSL_PASS;
    int32_t  sciclientInitStatus  = CSL_PASS;
    int32_t  sciclientTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has four sub-tests:\n");
        sciclientTestStatus += SciclientApp_prepareHeaderNegTest();
        sciclientTestStatus += SciclientApp_contextNegTest();
        sciclientTestStatus += SciclientApp_initTest();
        sciclientTestStatus += SciclientApp_sciclientMcdcTest();
        sciclientTestStatus += SciclientApp_deinitTest();
        sciclientTestStatus += SciclientApp_getPayloadSizeTest();
        sciclientTestStatus += SciclientApp_getThreadIdsTest();
    }
    else
    {
        sciclientTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientTestStatus;
}

static int32_t SciclientApp_rmPsilNegTest(void)
{
    int32_t  status              = CSL_PASS;
    int32_t  rmPsilTestStatus    = CSL_PASS;
    const struct tisci_msg_rm_psil_pair_req SciApp_RmPsilPairReq     = {0};
    const struct tisci_msg_rm_psil_unpair_req SciApp_RmPsilUnpairReq = {0};
    const struct tisci_msg_rm_psil_read_req SciApp_RmPsilReadReq     = {0};
    const struct tisci_msg_rm_psil_write_req SciApp_RmPsilWriteReq   = {0};
    struct tisci_msg_rm_psil_read_resp SciApp_RmPsilReadResp;

    /* Passing a zero request parameter, sciclient_service function will return pass but response flag will be NAK */
    status = Sciclient_rmPsilPair(&SciApp_RmPsilPairReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmPsilTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilPair: Negative Arg Test Passed.\n");
    }
    else
    {
        rmPsilTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilPair: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilUnpair(&SciApp_RmPsilUnpairReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmPsilTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilUnpair: Negative Arg Test Passed.\n");
    }
    else
    {
        rmPsilTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilUnpair: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilRead(&SciApp_RmPsilReadReq, &SciApp_RmPsilReadResp,
                                    SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmPsilTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilRead: Negative Arg Test Passed.\n");
    }
    else
    {
        rmPsilTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilRead: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilWrite(&SciApp_RmPsilWriteReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmPsilTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilWrite: Negative Arg Test Passed.\n");
    }
    else
    {
        rmPsilTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilWrite: Negative Arg Test Failed.\n");
    }

    return rmPsilTestStatus;
}

static int32_t SciclientApp_rmRingCfgNegTest(void)
{
    int32_t  status              = CSL_PASS;
    int32_t  rmRingCfgTestStatus = CSL_PASS;
    const struct tisci_msg_rm_ring_cfg_req SciApp_RmRingCfgReq = {0};
    struct tisci_msg_rm_ring_cfg_resp SciApp_RmRingCfgResp;

    /* Passing a zero request parameter, sciclient_service function will return pass but response flag will be NAK */
    status = Sciclient_rmRingCfg(&SciApp_RmRingCfgReq, &SciApp_RmRingCfgResp,
                                    SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmRingCfgTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmRingCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmRingCfgTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmRingCfg: Negative Arg Test Failed.\n");
    }

    return rmRingCfgTestStatus;
}

static int32_t SciclientApp_rmRingMonCfgNegTest(void)
{
    int32_t  status                 = CSL_PASS;
    int32_t  rmRingMonCfgTestStatus = CSL_PASS;
    const struct tisci_msg_rm_ring_mon_cfg_req SciApp_RmRingMonCfgReq = {0};
    struct tisci_msg_rm_ring_mon_cfg_resp SciApp_RmRingMonCfgResp;

    /* Passing a zero request parameter, sciclient_service function will return pass but response flag will be NAK */
    status = Sciclient_rmRingMonCfg(&SciApp_RmRingMonCfgReq, &SciApp_RmRingMonCfgResp,
                                    SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmRingMonCfgTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmRingMonCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmRingMonCfgTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmRingMonCfg: Negative Arg Test Failed.\n");
    }

    return rmRingMonCfgTestStatus;
}

static int32_t SciclientApp_rmUdmapNegTest(void)
{
    int32_t  status              = CSL_PASS;
    int32_t  rmUdmapTestStatus   = CSL_PASS;
    const struct tisci_msg_rm_udmap_gcfg_cfg_req SciApp_RmUdmapGcfgReq       = {0};
    const struct tisci_msg_rm_udmap_tx_ch_cfg_req SciApp_RmUdmapTxChCfgReq   = {0};
    const struct tisci_msg_rm_udmap_rx_ch_cfg_req SciApp_RMUdmapRxChCfgReq   = {0};
    const struct tisci_msg_rm_udmap_flow_cfg_req SciApp_RmUdmapFlowCfgReq    = {0};
    const struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req SciApp_RmUdmapFlowSizeThreshCfgReq = {0};
    struct tisci_msg_rm_udmap_gcfg_cfg_resp SciApp_RmUdmapGcfgResp;
    struct tisci_msg_rm_udmap_tx_ch_cfg_resp SciApp_RmUdmapTxChCfgResp;
    struct tisci_msg_rm_udmap_rx_ch_cfg_resp SciApp_RMUdmapRxChCfgResp;
    struct tisci_msg_rm_udmap_flow_cfg_resp SciApp_RmUdmapFlowCfgResp;
    struct tisci_msg_rm_udmap_flow_size_thresh_cfg_resp SciApp_RmUdmapFlowSizeThreshCfgResp;

    /* Passing a zero request parameter, sciclient_service function will return pass but response flag will be NAK */
    status = Sciclient_rmUdmapTxChCfg(&SciApp_RmUdmapTxChCfgReq, &SciApp_RmUdmapTxChCfgResp,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmUdmapTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmUdmapTxChCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmUdmapTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmUdmapTxChCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapRxChCfg(&SciApp_RMUdmapRxChCfgReq, &SciApp_RMUdmapRxChCfgResp,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmUdmapTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmUdmapRxChCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmUdmapTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmUdmapRxChCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapFlowCfg(&SciApp_RmUdmapFlowCfgReq, &SciApp_RmUdmapFlowCfgResp,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmUdmapTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmUdmapFlowCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmUdmapTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmUdmapFlowCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapFlowSizeThreshCfg(&SciApp_RmUdmapFlowSizeThreshCfgReq,
                                                &SciApp_RmUdmapFlowSizeThreshCfgResp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmUdmapTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmUdmapFlowSizeThreshCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmUdmapTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmUdmapFlowSizeThreshCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapGcfgCfg(&SciApp_RmUdmapGcfgReq, &SciApp_RmUdmapGcfgResp,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmUdmapTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmUdmapGcfgCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmUdmapTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmUdmapGcfgCfg: Negative Arg Test Failed.\n");
    }

    return rmUdmapTestStatus;
}

static int32_t SciclientApp_rmSetProxyNegTest(void)
{
    int32_t  status               = CSL_PASS;
    int32_t  rmSetProxyTestStatus = CSL_PASS;
    const struct tisci_msg_rm_proxy_cfg_req SciApp_RmSetProxyCfgReq = {0};

    /* Passing a zero request parameter, sciclient_service function will return pass but response flag will be NAK */
    status = Sciclient_rmSetProxyCfg(&SciApp_RmSetProxyCfgReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmSetProxyTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmSetProxyCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmSetProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmSetProxyCfg: Negative Arg Test Failed.\n");
    }

    return rmSetProxyTestStatus;
}

static int32_t SciclientApp_rmNegTest(void)
{
      int32_t  status               = CSL_PASS;
      int32_t  rmTestStatus         = CSL_PASS;
      uint16_t intNum               = 0U;
      uint16_t invalidDevId         = 450U;
      uint16_t iaOutput             = 0U;
      uint16_t dstInput             = 0U;
      struct tisci_msg_rm_get_resource_range_req  negReq;
      struct tisci_msg_rm_get_resource_range_resp resp;

    negReq.type = 2000U;
    negReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&negReq, &resp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmGetResourceRange: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmGetResourceRange: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmIrqSetRaw(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmIrqSetRaw: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmIrqSetRaw: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmIrqReleaseRaw(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmIrqReleaseRaw: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmIrqReleaseRaw: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmIrqTranslateIaOutput(invalidDevId, iaOutput,
                                                TISCI_DEV_R5FSS0_CORE0, &intNum);
    if (status == CSL_EBADARGS)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmIrqTranslateIaOutput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmIrqTranslateIaOutput: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmIrqTranslateIrqInput(invalidDevId, dstInput,
                                                TISCI_DEV_R5FSS0_CORE0, &intNum);
    if (status == CSL_EBADARGS)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmIrqTranslateIrqInput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmIrqTranslateIrqInput: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilPair(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmPsilPair: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmPsilPair: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilUnpair(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmPsilUnpair: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmPsilUnpair: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilRead(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmPsilRead: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmPsilRead: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmPsilWrite(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmPsilWrite: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmPsilWrite: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmRingCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmRingCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmRingCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmRingMonCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmRingMonCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmRingMonCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapTxChCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapTxChCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapTxChCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapRxChCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapRxChCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapRxChCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapFlowCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapFlowCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapFlowCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapFlowSizeThreshCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapFlowSizeThreshCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapFlowSizeThreshCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmUdmapGcfgCfg(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapGcfgCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapGcfgCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_rmSetProxyCfg(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmSetProxyCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmSetProxyCfg: Negative Arg Test Failed.\n");
    }

    return rmTestStatus;
}

static void SciclientApp_getResourceRange(uint16_t src_id, uint16_t dst_dev_id, uint16_t dst_host_id, struct SciApp_RangeOfLines *range)
{
    /* Get the range of lines between the given src_id and dst_id */
    struct tisci_msg_rm_get_resource_range_req resourceRangeReq={0};
    struct tisci_msg_rm_get_resource_range_resp resourceRangeRes={0};
    uint16_t dst_start;
    uint16_t dst_end;

    resourceRangeReq.type = src_id;
    resourceRangeReq.secondary_host = dst_host_id;
    Sciclient_rmGetResourceRange(&resourceRangeReq, &resourceRangeRes, SCICLIENT_SERVICE_WAIT_FOREVER);
    (*range).src_start = resourceRangeRes.range_start;
    (*range).src_end   = resourceRangeRes.range_start + resourceRangeRes.range_num - 1;

    Sciclient_rmTranslateIntOutput(src_id, (*range).src_start, dst_dev_id, &dst_start);
    Sciclient_rmTranslateIntOutput(src_id, (*range).src_end, dst_dev_id, &dst_end);
    (*range).dst_start = dst_start;
    (*range).dst_end = dst_end;
}

static int32_t SciclientApp_rmTranslateIntOutputTest(void)
{
    int32_t  status                         = CSL_PASS;
    int32_t  rmTranslateIntOutputTestStatus = CSL_PASS;
    uint16_t dstInput;
    struct SciApp_RangeOfLines range;

    SciclientApp_getResourceRange(TISCI_DEV_NAVSS0_INTR, TISCI_DEV_R5FSS0_CORE0, TISCI_HOST_ID_MAIN_0_R5_0, &range);

    /* Passing valid dst_dev_id to translate the specified IA output to the destination processor IRQ input */
    status = Sciclient_rmTranslateIntOutput(TISCI_DEV_NAVSS0_INTR, (range).src_start, TISCI_DEV_R5FSS0_CORE0, &dstInput);
    if (status == CSL_PASS)
    {
        rmTranslateIntOutputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Valid Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIntOutputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Valid Arg Test Failed.\n");
    }

    /* Passing invalid dst_dev_id */
    status = Sciclient_rmTranslateIntOutput(TISCI_DEV_NAVSS0_INTR, (range).src_start, TISCI_DEV_GPIO0, &dstInput);
    if (status != CSL_PASS)
    {
        rmTranslateIntOutputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIntOutputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Failed.\n");
    }

    /* Passing valid src_id, dst_dev_id and invalid src_output */
    status = Sciclient_rmTranslateIntOutput(TISCI_DEV_NAVSS0_INTR, (range).src_end + 1, TISCI_DEV_R5FSS0_CORE0, &dstInput);
    if (status != CSL_PASS)
    {
        rmTranslateIntOutputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIntOutputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Failed.\n");
    }

    /* Passing invalid src_id and invalid dst_dev_id */
    status = Sciclient_rmTranslateIntOutput(TISCI_DEV_R5FSS0_CORE0, (range).src_start, TISCI_DEV_R5FSS0_CORE1, &dstInput);
    if (status != CSL_PASS)
    {
        rmTranslateIntOutputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIntOutputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Failed.\n");
    }

    /* Passing valid IA src_id and invalid dst_dev_id */
    status = Sciclient_rmTranslateIntOutput(TISCI_DEV_NAVSS0_MODSS_INTAGG, (range).src_start, TISCI_DEV_R5FSS0_CORE1, &dstInput);
    if (status != CSL_PASS)
    {
        rmTranslateIntOutputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIntOutputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIntOutput: Negative Arg Test Failed.\n");
    }

    return rmTranslateIntOutputTestStatus;
}

static int32_t SciclientApp_rmTranslateIrqInputTest(void)
{
    int32_t  status                         = CSL_PASS;
    int32_t  rmTranslateIrqInputTestStatus  = CSL_PASS;
    struct SciApp_RangeOfLines range;
    uint16_t srcOutput;

    SciclientApp_getResourceRange(TISCI_DEV_NAVSS0_INTR, TISCI_DEV_R5FSS0_CORE0, TISCI_HOST_ID_MAIN_0_R5_0, &range);

    /* Passing valid dst_dev_id to translate the specified destination processor IRQ input to the IA output */
    status = Sciclient_rmTranslateIrqInput(TISCI_DEV_R5FSS0_CORE0, (range).dst_start, TISCI_DEV_NAVSS0_INTR, &srcOutput);
    if (status == CSL_PASS)
    {
        rmTranslateIrqInputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Valid Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIrqInputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Valid Arg Test Failed.\n");
    }

    /* Passing invalid dst_dev_id */
    status = Sciclient_rmTranslateIrqInput(TISCI_DEV_GPIO0, (range).dst_start, TISCI_DEV_NAVSS0_INTR, &srcOutput);
    if (status != CSL_PASS)
    {
        rmTranslateIrqInputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIrqInputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Failed.\n");
    }

    /* Passing invalid src_id and invalid dst_dev_id */
    status = Sciclient_rmTranslateIrqInput(TISCI_DEV_R5FSS0_CORE0, (range).dst_start, TISCI_DEV_R5FSS0_CORE1, &srcOutput);
    if (status != CSL_PASS)
    {
        rmTranslateIrqInputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIrqInputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Failed.\n");
    }

    /* Passing valid src_id, dst_dev_id and invalid dst_input */
    status = Sciclient_rmTranslateIrqInput(TISCI_DEV_R5FSS0_CORE0, (range).dst_end + 1, TISCI_DEV_NAVSS0_INTR, &srcOutput);
    if (status != CSL_PASS)
    {
        rmTranslateIrqInputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIrqInputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Failed.\n");
    }

    /* Passing valid IA src_id, dst_dev_id and invalid dst_input */
    status = Sciclient_rmTranslateIrqInput(TISCI_DEV_NAVSS0_INTR, (range).dst_end + 1, TISCI_DEV_NAVSS0_MODSS_INTAGG, &srcOutput);
    if (status != CSL_PASS)
    {
        rmTranslateIrqInputTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Passed.\n");
    }
    else
    {
        rmTranslateIrqInputTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmTranslateIrqInput: Negative Arg Test Failed.\n");
    }

    return rmTranslateIrqInputTestStatus;
}

#if !defined(BUILD_MCU1_1)
/* This function covers the positive testcases for sciclient_rm.c file */
static int32_t SciclientApp_rmUdmapRingPsilProxyPosTest(void)
{
    int32_t  rmUdmapRingPsilProxyPositiveTestStatus                                = 0;
    int32_t  status                                                                = 0;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReq               = {0};
    struct tisci_msg_rm_udmap_gcfg_cfg_req rmUdmapGcfgReq                          = {0};
    struct tisci_msg_rm_psil_pair_req rmPsilPairReq                                = {0};
    struct tisci_msg_rm_psil_read_req rmPsilReadReq                                = {0};
    struct tisci_msg_rm_psil_write_req rmPsilWriteReq                              = {0};
    struct tisci_msg_rm_ring_cfg_req rmRingCfgReq                                  = {0};
    struct tisci_msg_rm_ring_mon_cfg_req rmRingMonCfgReq                           = {0};
    struct tisci_msg_rm_udmap_tx_ch_cfg_req rmUdmapTxChCfgReq                      = {0};
    struct tisci_msg_rm_udmap_rx_ch_cfg_req rmUdmapRxChCfgReq                      = {0};
    struct tisci_msg_rm_udmap_flow_cfg_req rmUdmapFlowCfgReq                       = {0};
    struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req rmUdmapFlowSizeThreshCfgReq = {0};
    struct tisci_msg_rm_proxy_cfg_req rmProxyCfgReq                                = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeResp;
    struct tisci_msg_rm_udmap_gcfg_cfg_resp rmUdmapGcfgResp;
    struct tisci_msg_rm_psil_unpair_req rmPsilUnpairReq;
    struct tisci_msg_rm_psil_read_resp rmPsilReadResp;
    struct tisci_msg_rm_ring_cfg_resp rmRingCfgResp;
    struct tisci_msg_rm_ring_mon_cfg_resp rmRingMonCfgResp;
    struct tisci_msg_rm_udmap_tx_ch_cfg_resp rmUdmapTxChCfgResp;
    struct tisci_msg_rm_udmap_rx_ch_cfg_resp rmUdmapRxChCfgResp;
    struct tisci_msg_rm_udmap_flow_cfg_resp rmUdmapFlowCfgResp;
    struct tisci_msg_rm_udmap_flow_size_thresh_cfg_resp rmUdmapFlowSizeThreshCfgResp;

    rmGetResourceRangeReq.type           = TISCI_DEV_NAVSS0_INTR;
    rmGetResourceRangeReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmGetResourceRange Test Passed.\n");
    }
    else
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmGetResourceRange Test Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_RINGACC0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_RA_UDMAP_TX;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmRingCfgReq.nav_id         = TISCI_DEV_MCU_NAVSS0_RINGACC0;
        rmRingCfgReq.index          = rmGetResourceRangeResp.range_start;
        status                      = Sciclient_rmRingCfg(&rmRingCfgReq, &rmRingCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmRingCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmRingCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_RINGACC0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_RA_MONITORS;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmRingMonCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_RINGACC0;
        rmRingMonCfgReq.index         = rmGetResourceRangeResp.range_start;
        status                        = Sciclient_rmRingMonCfg(&rmRingMonCfgReq, &rmRingMonCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmRingMonCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmRingMonCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_UDMAP_TX_CHAN;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapTxChCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
        rmUdmapTxChCfgReq.index         = rmGetResourceRangeResp.range_start;
        status                          = Sciclient_rmUdmapTxChCfg(&rmUdmapTxChCfgReq, &rmUdmapTxChCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmUdmapTxChCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmUdmapTxChCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_UDMAP_RX_CHAN;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRxChCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
        rmUdmapRxChCfgReq.index         = rmGetResourceRangeResp.range_start;
        status                   = Sciclient_rmUdmapRxChCfg(&rmUdmapRxChCfgReq, &rmUdmapRxChCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmUdmapRxChCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmUdmapRxChCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_UDMAP_TX_CHAN;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapFlowCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
        rmUdmapFlowCfgReq.flow_index    = rmGetResourceRangeResp.range_start;
        status                          = Sciclient_rmUdmapFlowCfg(&rmUdmapFlowCfgReq, &rmUdmapFlowCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmUdmapFlowCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmUdmapFlowCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type              = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    rmGetResourceRangeReq.subtype           = TISCI_RESASG_SUBTYPE_UDMAP_TX_CHAN;
    rmGetResourceRangeReq.secondary_host    = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapFlowSizeThreshCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
        rmUdmapFlowSizeThreshCfgReq.flow_index    = rmGetResourceRangeResp.range_start;
        status                                    = Sciclient_rmUdmapFlowSizeThreshCfg(&rmUdmapFlowSizeThreshCfgReq, &rmUdmapFlowSizeThreshCfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmUdmapFlowSizeThreshCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmUdmapFlowSizeThreshCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_UDMAP_TX_CHAN;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmPsilPairReq.nav_id        = TISCI_DEV_MCU_NAVSS0;
        rmPsilPairReq.src_thread    = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILS_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
        rmPsilPairReq.dst_thread    = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILD_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
        status                      = Sciclient_rmPsilPair(&rmPsilPairReq, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmPsilPair Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmPsilPair Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    rmPsilUnpairReq.nav_id     = TISCI_DEV_MCU_NAVSS0;
    rmPsilUnpairReq.src_thread = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILS_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
    rmPsilUnpairReq.dst_thread = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILD_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
    status                     = Sciclient_rmPsilUnpair(&rmPsilUnpairReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilUnpair Test Passed.\n");
    }
    else
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilUnpair Test Failed.\n");
    }

    rmPsilReadReq.nav_id        = TISCI_DEV_MCU_NAVSS0;
    rmPsilReadReq.thread        = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILS_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
    rmPsilReadReq.taddr         = CSL_PSILCFG_REG_ENABLE;
    status                      = Sciclient_rmPsilRead(&rmPsilReadReq, &rmPsilReadResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilRead Test Passed.\n");
    }
    else
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilRead Test Failed.\n");
    }

    rmPsilWriteReq.nav_id       = TISCI_DEV_MCU_NAVSS0;
    rmPsilWriteReq.thread       = CSL_PSILCFG_NAVSS_MCU_UDMAP0_STRM_PSILS_THREAD_OFFSET + rmGetResourceRangeResp.range_start;
    rmPsilWriteReq.taddr        = CSL_PSILCFG_REG_ENABLE;
    rmPsilWriteReq.data         = 1;
    status                      = Sciclient_rmPsilWrite(&rmPsilWriteReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmPsilWrite Test Passed.\n");
    }
    else
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmPsilWrite Test Failed.\n");
    }

    rmUdmapGcfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_UDMAP_0;
    status                       = Sciclient_rmUdmapGcfgCfg(&rmUdmapGcfgReq, &rmUdmapGcfgResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_rmUdmapGcfgCfg Test Passed.\n");
    }
    else
    {
        rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_rmUdmapGcfgCfg Test Failed.\n");
    }

    rmGetResourceRangeReq.type           = TISCI_DEV_MCU_NAVSS0_PROXY0;
    rmGetResourceRangeReq.subtype        = TISCI_RESASG_SUBTYPE_PROXY_PROXIES;
    rmGetResourceRangeReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;
    status = Sciclient_rmGetResourceRange(&rmGetResourceRangeReq, &rmGetResourceRangeResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmProxyCfgReq.nav_id        = TISCI_DEV_MCU_NAVSS0_PROXY0;
        rmProxyCfgReq.index         = rmGetResourceRangeResp.range_start;
        status                      = Sciclient_rmSetProxyCfg(&rmProxyCfgReq, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_rmSetProxyCfg Test Passed.\n");
        }
        else
        {
            rmUdmapRingPsilProxyPositiveTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_rmSetProxyCfg Test Failed.\n");
        }
    }
    else
    {
        SciApp_printf ("Sciclient_rmGetResourceRange() Failed.\n");
    }

    return rmUdmapRingPsilProxyPositiveTestStatus;
}
#endif

/* Testcase for UnmappedVintRouteCreate() and UnmappedVintRouteDelete()*/
static int32_t SciclientApp_rmUnmappedVintRouteCreateTest(void)
{
    int32_t  status                                                        = CSL_PASS;
    uint16_t intNum                                                        = 0U;
    int32_t  rmUnmappedVintRouteCreateTestStatus                           = CSL_PASS;
    const struct tisci_msg_rm_irq_set_resp Sciclient_Resp                  = {0};
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqVint   = {0};
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqGlobal = {0};
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqIrq    = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespVint;
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespGlobal;
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespIrq;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseResp;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseNegResp;
    struct tisci_msg_rm_irq_set_resp rmIrqSetNegResp;

    rmGetResourceRangeReqVint.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    rmGetResourceRangeReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    rmGetResourceRangeReqVint.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqVint,
                                            &rmGetResourceRangeRespVint,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for vint\n");
    }
    else
    {
        rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for vint\n");
    }

    rmGetResourceRangeReqGlobal.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    rmGetResourceRangeReqGlobal.subtype        = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    rmGetResourceRangeReqGlobal.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqGlobal,
                                            &rmGetResourceRangeRespGlobal,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for globalevent\n");
    }
    else
    {
        rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for globalevent\n");
    }

    rmGetResourceRangeReqIrq.type           = TISCI_DEV_MCU_NAVSS0_INTR;
    rmGetResourceRangeReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    rmGetResourceRangeReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqIrq,
                                           &rmGetResourceRangeRespIrq,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(rmGetResourceRangeReqIrq.type,
                                                  rmGetResourceRangeRespIrq.range_start,
                                                  TISCI_DEV_MCU_R5FSS0_CORE0,
                                                  &intNum);
        if(status == CSL_PASS)
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() execution is successful and host interrupt number is %d\n", intNum);
            const struct tisci_msg_rm_irq_set_req Sciclient_Req =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                         TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = rmGetResourceRangeRespGlobal.range_start,
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = rmGetResourceRangeRespVint.range_start,
                .vint_status_bit_index = 0U,
                .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
            };
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_PASS)
            {
                rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Passed.\n");
            }
            else
            {
                rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Failed.\n");
            }
        }
        else
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

    if(rmUnmappedVintRouteCreateTestStatus == CSL_PASS)
    {
        /* Deleting the interrupt Route()*/
        const struct tisci_msg_rm_irq_release_req rmIrqReleaseReq =
        {
            .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                     TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
            .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
            .src_index             = 0U,
            .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
            .dst_host_irq          = intNum,
            .global_event          = rmGetResourceRangeRespGlobal.range_start,
            .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
            .vint                  = rmGetResourceRangeRespVint.range_start,
            .vint_status_bit_index = 0U,
            .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
        };
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseReq, &rmIrqReleaseResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Valid Arg Test Passed.\n");
        }
        else
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Valid Arg Test Failed.\n");
        }

        /* Deleting the interrupt route even when it is not programmed */
        const struct tisci_msg_rm_irq_release_req rmIrqReleaseNegReq =
        {
            .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                     TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
            .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
            .src_index             = 0U,
            .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
            .dst_host_irq          = intNum,
            .global_event          = rmGetResourceRangeRespGlobal.range_start,
            .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
            .vint                  = rmGetResourceRangeRespVint.range_start,
            .vint_status_bit_index = 0U,
            .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
        };
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseNegReq, &rmIrqReleaseNegResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Valid Arg Test Passed.\n");
        }
        else
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Valid Arg Test Failed.\n");
        }

        /* Negative testcase for Sciclient_rmProgramInterruptRoute() */
        const struct tisci_msg_rm_irq_set_req rmIrqSetNegReq =
        {
            .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                     TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
            .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
            .src_index             = 0U,
            .dst_id                = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
            .dst_host_irq          = 0U,
            .global_event          = rmGetResourceRangeRespGlobal.range_start,
            .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
            .vint                  = rmGetResourceRangeRespVint.range_start,
            .vint_status_bit_index = 0U,
            .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
        };
        status = Sciclient_rmProgramInterruptRoute(&rmIrqSetNegReq, &rmIrqSetNegResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Passed.\n");
        }
        else
        {
            rmUnmappedVintRouteCreateTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Failed.\n");
        }
    }

    return rmUnmappedVintRouteCreateTestStatus;
}

static int32_t SciclientApp_rmIaValidateEvtTest(void)
{
    int32_t  status                         = CSL_PASS;
    int32_t  rmIaValidateEvtTestStatus      = CSL_PASS;
    uint16_t intNum                         = 0U;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqVint;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResVint;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqGlobal;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResGlobal;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqIrq;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResIrq;
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;

    Sciclient_ReqVint.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    Sciclient_ReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    Sciclient_ReqVint.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqVint, &Sciclient_ResVint, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for vint\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for vint\n");
    }

    Sciclient_ReqGlobal.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    Sciclient_ReqGlobal.subtype        = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    Sciclient_ReqGlobal.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqGlobal, &Sciclient_ResGlobal, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for globalevent\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for globalevent\n");
    }

    Sciclient_ReqIrq.type           = TISCI_DEV_MCU_NAVSS0_INTR;
    Sciclient_ReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    Sciclient_ReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqIrq, &Sciclient_ResIrq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(Sciclient_ReqIrq.type, Sciclient_ResIrq.range_start, TISCI_DEV_MCU_R5FSS0_CORE0, &intNum);
        if(status == CSL_PASS)
        {
            const struct tisci_msg_rm_irq_set_req Sciclient_Req =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                            TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                            TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = Sciclient_ResGlobal.range_start + Sciclient_ResGlobal.range_num + 16400U, /* Invalid global event value for TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0 */
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = Sciclient_ResVint.range_start,
                .vint_status_bit_index = 0U,
                .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
            };
            /* Passing invalid global event value to cover Sciclient_rmIaEvtRomMapped function from Sciclient_rmIaValidateEvt */
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_EFAIL)
            {
                rmIaValidateEvtTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Passed.\n");
            }
            else
            {
                rmIaValidateEvtTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Failed.\n");
            }
            const struct tisci_msg_rm_irq_set_req Sciclient_Req1 =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                            TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                            TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = Sciclient_ResGlobal.range_start,
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = Sciclient_ResVint.range_start,
                .vint_status_bit_index = 0U,
                .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
            };
            /* Passing these parameters can cover Sciclient_rmIaValidateEvt badargs condition */
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req1, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_EFAIL)
            {
                rmIaValidateEvtTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Arg Test Passed.\n");
            }
            else
            {
                rmIaValidateEvtTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Arg Test Failed.\n");
            }
        }
        else
        {
            rmIaValidateEvtTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
    }
    else
    {
        rmIaValidateEvtTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

    return rmIaValidateEvtTestStatus;
}

static int32_t SciclientApp_rmIrInpRomMappedTest(void)
{
    int32_t  status                                                     = CSL_PASS;
    int32_t  rmIrInpRomMappedTestStatus                                 = CSL_PASS;
    uint16_t intNum                                                     = 0U;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqIrq = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespIrq;
    #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;
    #endif

    rmGetResourceRangeReqIrq.type           = TISCI_DEV_MAIN2MCU_LVL_INTRTR0;
    rmGetResourceRangeReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    rmGetResourceRangeReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqIrq,
                                            &rmGetResourceRangeRespIrq,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(rmGetResourceRangeReqIrq.type,
                                                    rmGetResourceRangeRespIrq.range_start,
                                                    TISCI_DEV_MCU_R5FSS0_CORE0,
                                                    &intNum);
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed\n");
    }

    #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
    struct tisci_msg_rm_irq_set_req Sciclient_ReqIr =
    {
        .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .src_id                = TISCI_DEV_MMCSD0,
        .src_index             = 0U,
        .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
        .dst_host_irq          = intNum,
        .vint_status_bit_index = 0U,
        .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
    };

    /* Updating output control register value to match with input line to IR in order to cover Sciclient_rmIrInpRomMapped function */
    CSL_REG32_WR_OFF(0x00A10004U, 0, 28);
    /* Sciclient_rmIrInpRomMapped function will pass by passing valid parameters */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_ReqIr,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrInpRomMappedTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Passed.\n");
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Failed.\n");
    }

    struct tisci_msg_rm_irq_set_req Sciclient_RomUsageReq =
    {
        .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .src_id                = TISCI_DEV_USB0,
        .src_index             = 0U,
        .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
        .dst_host_irq          = intNum,
        .vint_status_bit_index = 0U,
        .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
    };

    /* Updating output control register value to match with input line to IR in order to cover Sciclient_rmIrInpRomMapped function */
    CSL_REG32_WR_OFF(0x00A10004U, 0, 157);
    /* inp value is greater than inp_start + inp_length so rom_mapped will not be true in Sciclient_rmIrInpRomMapped */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_RomUsageReq,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrInpRomMappedTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Passed.\n");
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Failed.\n");
    }

    Sciclient_RomUsageReq.src_index = 20U;
    /* Passing s_idx value such that cur_if->lbase < s_idx */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_RomUsageReq,
                                               &Sciclient_Resp,
                                               SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrInpRomMappedTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Passed.\n");
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Failed.\n");
    }

    struct tisci_msg_rm_irq_set_req Sciclient_RomUsageReqFail =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                        TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                        TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID |
                        TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .ia_id        = TISCI_DEV_NAVSS0_MODSS_INTAGG
    };

    /* Updating output control register value to match with input line to IR in order to cover Sciclient_rmIrInpRomMapped function */
    CSL_REG32_WR_OFF(0x310E0004U, 0, 320);
    /* Sciclient_rmIrInpRomMapped fails because rom_usage value is NULL */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_RomUsageReqFail,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrInpRomMappedTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Passed.\n");
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Failed.\n");
    }
    #endif

    return rmIrInpRomMappedTestStatus;
}

static int32_t SciclientApp_rmIrqReleaseTest(void)
{
    int32_t rmIrqReleasestatus  = CSL_PASS;
    int32_t status              = CSL_PASS;

    status = Sciclient_rmIrqRelease(NULL,SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        rmIrqReleasestatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqRelease negative Test PASSED.\n");
    }
    else
    {
        rmIrqReleasestatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqRelease negative Test Failed.\n");
    }

    return rmIrqReleasestatus;
}

static int32_t SciclientApp_rmIrqGetResourceRangeTest(void)
{
    int status = CSL_PASS;
    status     = Sciclient_rmGetResourceRange(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        status = CSL_PASS;
        SciApp_printf("Sciclient_rmGetResourceRange negative test passed.\n");
    }
    else
    {
        status = CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange negative test failed.\n");
    }

    return status;
}

static int32_t SciclientApp_rmTest(void)
{
    int32_t  status                       = CSL_PASS;
    int32_t  sciclientInitStatus          = CSL_PASS;
    int32_t  sciclientRmTestStatus        = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has twelve sub-tests:\n");
        sciclientRmTestStatus += SciclientApp_rmPsilNegTest();
        sciclientRmTestStatus += SciclientApp_rmRingCfgNegTest();
        sciclientRmTestStatus += SciclientApp_rmRingMonCfgNegTest();
        sciclientRmTestStatus += SciclientApp_rmUdmapNegTest();
        sciclientRmTestStatus += SciclientApp_rmSetProxyNegTest();
        sciclientRmTestStatus += SciclientApp_rmNegTest();
        sciclientRmTestStatus += SciclientApp_rmTranslateIntOutputTest();
        sciclientRmTestStatus += SciclientApp_rmTranslateIrqInputTest();
        #if !defined(BUILD_MCU1_1)
        sciclientRmTestStatus += SciclientApp_rmUdmapRingPsilProxyPosTest();
        #endif
        sciclientRmTestStatus += SciclientApp_rmUnmappedVintRouteCreateTest();
        sciclientRmTestStatus += SciclientApp_rmIaValidateEvtTest();
        sciclientRmTestStatus += SciclientApp_rmIrInpRomMappedTest();
        sciclientRmTestStatus += SciclientApp_rmIrqReleaseTest();
        sciclientRmTestStatus += SciclientApp_rmIrqGetResourceRangeTest();
    }
    else
    {
        sciclientRmTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientRmTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientRmTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientRmTestStatus;
}

static int32_t SciclientApp_firewallNegTest(void)
{
    int32_t status                = CSL_PASS;
    int32_t firewallNegTestStatus = CSL_PASS;

    status = Sciclient_firewallChangeOwnerInfo(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        firewallNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Negative Arg Test Passed.\n");
    }
    else
    {
        firewallNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Negative Arg Test Failed.\n");
    }

    status = Sciclient_firewallSetRegion(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        firewallNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallSetRegion: Negative Arg Test Passed.\n");
    }
    else
    {
        firewallNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallSetRegion: Negative Arg Test Failed.\n");
    }

    status = Sciclient_firewallGetRegion(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        firewallNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallGetRegion: Negative Arg Test Passed.\n");
    }
    else
    {
        firewallNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallGetRegion: Negative Arg Test Failed.\n");
    }

    return firewallNegTestStatus;
}

static int32_t SciclientApp_firewallPosTest(void)
{
    int32_t status                       = CSL_PASS;
    int32_t firewallPositiveTestStatus   = CSL_PASS;
    struct tisci_msg_fwl_get_firewall_region_resp getFirewallRegionResp;
    #if defined(BUILD_C7X_1) && (defined(SOC_J721S2) || defined(SOC_J784S4))
    uint32_t firewallId;
    uint64_t startAddress;
    uint64_t endAddress;
    #endif
    struct tisci_msg_fwl_get_firewall_region_req  getFirewallRegionReq =
    {
        .fwl_id            = SCICLIENT_APP_MCU_SRAM_FWL_ID,
        .region            = 1,
        .n_permission_regs = 3
    };
    #if defined(BUILD_MCU1_0)
    struct tisci_msg_fwl_change_owner_info_resp fwlChangeOwnerInfoRespR5  = {0};
    struct tisci_msg_fwl_set_firewall_region_resp fwlSetRegionRespR5;
    #endif
    #if defined(BUILD_C7X_1) && (defined(SOC_J721S2) || defined(SOC_J784S4))
    struct tisci_msg_fwl_change_owner_info_resp fwlChangeOwnerInfoRespC7x  = {0};
    struct tisci_msg_fwl_set_firewall_region_resp fwlSetRegionRespC7x;
    #endif

    status = Sciclient_firewallGetRegion(&getFirewallRegionReq, &getFirewallRegionResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        firewallPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallGetRegion: Positive Arg Test Passed.\n");
    }
    else
    {
        firewallPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallGetRegion: Positive Arg Test Failed.\n");
    }

    #if defined(BUILD_MCU1_0)
    struct tisci_msg_fwl_change_owner_info_req fwlChangeOwnerInfoReqR5 =
    {
        .fwl_id      = (uint16_t) SCICLIENT_APP_MCU_SRAM_FWL_ID,
        .region      = (uint16_t) 1,
        .owner_index = (uint8_t)  TISCI_HOST_ID_MCU_0_R5_1
    };
    status = Sciclient_firewallChangeOwnerInfo(&fwlChangeOwnerInfoReqR5, &fwlChangeOwnerInfoRespR5, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        firewallPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Positive Arg Test Passed.\n");
    }
    else
    {
        firewallPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Positive Arg Test Failed.\n");
    }

    struct tisci_msg_fwl_set_firewall_region_req fwlSetRegionReqR5 = {
        .fwl_id            = (uint16_t) SCICLIENT_APP_MCU_SRAM_FWL_ID,
        .region            = (uint16_t) 1,
        .n_permission_regs = (uint32_t) 3,
        .control           = (uint32_t) 0x10A,
        .permissions[0]    = (uint32_t) (SCICLIENT_APP_MCU_1_0_PRIVID << 16) | 0,
        .permissions[1]    = (uint32_t) 0,
        .permissions[2]    = (uint32_t) 0,
        .start_address     = 0x41C00000,
        .end_address       = 0x41C00002
    };
    status = Sciclient_firewallSetRegion(&fwlSetRegionReqR5, &fwlSetRegionRespR5, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        firewallPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallSetRegion: Positive Arg Test Passed.\n");
    }
    else
    {
        firewallPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallSetRegion: Positive Arg Test Failed.\n");
    }
    #endif

    #if defined(BUILD_C7X_1)
    #if defined(SOC_J721S2)
    firewallId   = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_DSP4_CFG_DSP_ECCAGGR4_ID;
    startAddress = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_DSP4_CFG_DSP_ECCAGGR4_DSP0_ECC_AGGR_START;
    endAddress   = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_DSP4_CFG_DSP_ECCAGGR4_DSP0_ECC_AGGR_END;
    #endif
    #if defined(SOC_J784S4)
    firewallId   = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_AW4_CFG_DSP_ECCAGGR4_ID;
    startAddress = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_AW4_CFG_DSP_ECCAGGR4_DSP0_ECC_AGGR_START;
    endAddress   = CSL_STD_FW_COMPUTE_CLUSTER0_C71SS0_ECC_AGGR_0__VBUSP4_CFG_AW4_CFG_DSP_ECCAGGR4_DSP0_ECC_AGGR_END;
    #endif
    #if defined(SOC_J721S2) || defined(SOC_J784S4)
    struct tisci_msg_fwl_change_owner_info_req fwlChangeOwnerInfoReqC7x =
    {
        .fwl_id      = (uint16_t) firewallId,
        .region      = (uint16_t) 1,
        .owner_index = (uint8_t)  TISCI_HOST_ID_C7X_0_1
    };
    status = Sciclient_firewallChangeOwnerInfo(&fwlChangeOwnerInfoReqC7x, &fwlChangeOwnerInfoRespC7x, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        firewallPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Positive Arg Test Passed.\n");
    }
    else
    {
        firewallPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallChangeOwnerInfo: Positive Arg Test Failed.\n");
    }
    struct tisci_msg_fwl_set_firewall_region_req fwlSetRegionReqC7x = {
        .fwl_id            = (uint16_t) firewallId,
        .region            = (uint16_t) 1,
        .n_permission_regs = (uint32_t) 3,
        .control           = (uint32_t) 0,
        .permissions[0]    = (uint32_t) (SCICLIENT_APP_C7x_1_PRIVID << 16) | 0,
        .permissions[1]    = (uint32_t) 0,
        .permissions[2]    = (uint32_t) 0,
        .start_address     = startAddress,
        .end_address       = endAddress
    };
    status = Sciclient_firewallSetRegion(&fwlSetRegionReqC7x, &fwlSetRegionRespC7x, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        firewallPositiveTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_firewallSetRegion: Positive Arg Test Passed.\n");
    }
    else
    {
        firewallPositiveTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_firewallSetRegion: Positive Arg Test Failed.\n");
    }
    #endif
    #endif

    return firewallPositiveTestStatus;
}

static int32_t SciclientApp_firewallTest(void)
{
    int32_t  status                       = CSL_PASS;
    int32_t  sciclientInitStatus          = CSL_PASS;
    int32_t  sciclientFirewallTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has two sub-tests:\n");
        sciclientFirewallTestStatus += SciclientApp_firewallNegTest();
        sciclientFirewallTestStatus += SciclientApp_firewallPosTest();
    }
    else
    {
        sciclientFirewallTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientFirewallTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientFirewallTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientFirewallTestStatus;
}

static int32_t SciclientApp_genericMsgsTest(void)
{
    int32_t status                 = CSL_PASS;
    int32_t sciclientInitStatus    = CSL_PASS;
    int32_t msmcQueryTestStatus    = CSL_PASS;
    struct  tisci_query_msmc_resp resp;
    Sciclient_ConfigPrms_t  config =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
        NULL,
        0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        status = Sciclient_msmcQuery(NULL, &resp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
           msmcQueryTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_msmcQuery: Negative Arg Test Passed.\n");
        }
        else
        {
           msmcQueryTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_msmcQuery: Negative Arg Test Failed.\n");
        }
    }
    else
    {
        msmcQueryTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            msmcQueryTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_deinit PASSED.\n");
        }
        else
        {
            msmcQueryTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_deinit FAILED.\n");
        }
    }

    return msmcQueryTestStatus;
}

static int32_t SciclientApp_rmIrqVintDeleteNegTest(void)
{
    int32_t  status                         = CSL_PASS;
    int32_t  rmIrqVintDeleteTestStatus      = CSL_PASS;
    uint16_t intNum                         = 0U;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqVint;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResVint;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqGlobal;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResGlobal;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqIrq;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResIrq;
    struct tisci_msg_rm_irq_release_resp Sciclient_Resp;

    Sciclient_ReqVint.type           = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    Sciclient_ReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    Sciclient_ReqVint.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqVint, &Sciclient_ResVint, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for vint\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for vint\n");
    }

    Sciclient_ReqGlobal.type            = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    Sciclient_ReqGlobal.subtype         = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    Sciclient_ReqGlobal.secondary_host  = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqGlobal, &Sciclient_ResGlobal, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for globalevent\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for globalevent\n");
    }

    Sciclient_ReqIrq.type           = TISCI_DEV_NAVSS0_INTR;
    Sciclient_ReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    Sciclient_ReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqIrq, &Sciclient_ResIrq, SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(Sciclient_ReqIrq.type,
                                                    Sciclient_ResIrq.range_start,
                                                    TISCI_DEV_MCU_R5FSS0_CORE0,
                                                    &intNum);
        if(status == CSL_PASS)
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() execution is successful and host interrupt number is %d\n", intNum);
            const struct tisci_msg_rm_irq_release_req Sciclient_Req =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                            TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                            TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_NAVSS0_MAILBOX,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = Sciclient_ResGlobal.range_start,
                .ia_id                 = TISCI_DEV_NAVSS0_UDMASS_INTAGG,
                .vint                  = Sciclient_ResVint.range_start,
                .vint_status_bit_index = 0U
            };
            /* No events are present for TISCI_DEV_NAVSS0_UDMASS_INTAGG, so Sciclient_rmIrqVintDelete
                returns CSL_EBADARGS since no events are there to unmap */
            status = Sciclient_rmClearInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_EBADARGS)
            {
                rmIrqVintDeleteTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmClearInterruptRoute: rmIrqVintDelete Test Passed.\n");
            }
            else
            {
                rmIrqVintDeleteTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmClearInterruptRoute: rmIrqVintDelete Test Failed.\n");
            }
        }
        else
        {
            rmIrqVintDeleteTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
    }
    else
    {
        rmIrqVintDeleteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

    return rmIrqVintDeleteTestStatus;
}

static int32_t SciclientApp_rmIrqCfgIsUnmappedVintDirectEventNegTest(void)
{
    int32_t  status                     = CSL_PASS;
    int32_t  vintDirectEventTestStatus  = CSL_PASS;
    uint8_t  loopCounter                = 0U;
    uint32_t numConditions              = 6U;
    uint32_t validParams[7]             = {
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                             TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                             TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID |
                                             TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                             TISCI_MSG_VALUE_RM_IA_ID_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                             TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID)
                                           };
    struct tisci_msg_rm_irq_set_req   rmIrqReq  = {0};
    struct tisci_msg_rm_irq_set_resp  rmIrqResp = {0};

    /* To cover MC/DC for Sciclient_rmIrqCfgIsUnmappedVintDirectEvent() */
    for(loopCounter = 0U; loopCounter <= numConditions; loopCounter++)
    {
          rmIrqReq.valid_params = validParams[loopCounter];
          status += Sciclient_rmProgramInterruptRoute(&rmIrqReq, &rmIrqResp,
                                                     SCICLIENT_SERVICE_WAIT_FOREVER);
          if(status != CSL_PASS)
          {
              vintDirectEventTestStatus += CSL_PASS;
          }
          else
          {
              vintDirectEventTestStatus += CSL_EFAIL;
          }
    }

    if(vintDirectEventTestStatus == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmIrqCfgIsUnmappedVintDirectEvent() Test Passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmIrqCfgIsUnmappedVintDirectEvent() Test Failed.\n");
    }

    return vintDirectEventTestStatus;
}

static int32_t SciclientApp_rmIrqCfgIsEventToVintMappingOnlyNegTest(void)
{
    int32_t  status                     = CSL_PASS;
    int32_t  vintMappingOnlyTestStatus  = CSL_PASS;
    uint8_t  loopCounter                = 0U;
    uint32_t numConditions              = 6U;
    uint32_t validParams[7]             = {
                                            (TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID |
                                             TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                            (TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_VINT_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                              TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                            (TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID  |
                                             TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID)
                                          };
    struct tisci_msg_rm_irq_set_req   rmIrqReq  = {0};
    struct tisci_msg_rm_irq_set_resp  rmIrqResp = {0};

    /* To cover statement and MC/DC coverage for Sciclient_rmIrqCfgIsEventToVintMappingOnly() */
    for(loopCounter = 0U; loopCounter <= numConditions; loopCounter++)
    {
          rmIrqReq.valid_params = validParams[loopCounter];
          status = Sciclient_rmProgramInterruptRoute(&rmIrqReq, &rmIrqResp,
                                                     SCICLIENT_SERVICE_WAIT_FOREVER);
          if(status != CSL_PASS)
          {
              vintMappingOnlyTestStatus += CSL_PASS;
          }
          else
          {
              vintMappingOnlyTestStatus += CSL_EFAIL;
          }
    }

    if(vintMappingOnlyTestStatus == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmIrqCfgIsEventToVintMappingOnly() Test Passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmIrqCfgIsEventToVintMappingOnly() Test Failed.\n");
    }

    return vintMappingOnlyTestStatus;
}

static int32_t SciclientApp_rmIrqCfgIsOesOnlyNegTest(void)
{
    int32_t  status                       = CSL_PASS;
    int32_t  rmIrqCfgIsOesOnlyTestStatus  = CSL_PASS;
    uint8_t  loopCounter                  = 0U;
    uint32_t numConditions                = 6U;
    uint32_t validParams[7]               = {
                                                (TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID  | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                                (0U),
                                                (TISCI_MSG_VALUE_RM_VINT_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                                (TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                                (TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                                (TISCI_MSG_VALUE_RM_DST_ID_VALID  | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
                                                (TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID)
                                            };
    struct tisci_msg_rm_irq_set_req   rmIrqReq  = {0};
    struct tisci_msg_rm_irq_set_resp  rmIrqResp = {0};

    /* To cover MC/DC conditions for Sciclient_rmIrqCfgIsOesOnly() */
    for(loopCounter = 0U; loopCounter <= numConditions; loopCounter++)
    {
      rmIrqReq.valid_params = validParams[loopCounter];
      status = Sciclient_rmProgramInterruptRoute(&rmIrqReq, &rmIrqResp,
                                                 SCICLIENT_SERVICE_WAIT_FOREVER);
      if(status != CSL_PASS)
      {
          rmIrqCfgIsOesOnlyTestStatus += CSL_PASS;
      }
      else
      {
          rmIrqCfgIsOesOnlyTestStatus += CSL_EFAIL;
      }
    }

    if(rmIrqCfgIsOesOnlyTestStatus == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmIrqCfgIsOesOnly() Test Passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmIrqCfgIsOesOnly() Test Failed.\n");
    }

    return rmIrqCfgIsOesOnlyTestStatus;
}

static int32_t SciclientApp_rmIrqCfgIsDirectEventTest(void)
{
    int32_t  status                          = CSL_PASS;
    int32_t  rmIrqCfgIsDirectEventTestStatus = CSL_PASS;
    uint32_t rmVintStatusBitIndexInvalid     = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                               TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID);
    uint32_t rmGlobalEventInvalid            = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                                TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t rmVintInvalid                   = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID |
                                                TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t rmIaIdInvalid                   = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                                TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t rmDstHostIrqInvalid             = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                                TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t rmDstIdInvalid                  = (TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                                TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t directEventPass                 = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                                TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                                TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID);
    uint32_t inValidParams[7]                = {directEventPass,
                                                rmVintStatusBitIndexInvalid,
                                                rmGlobalEventInvalid,
                                                rmVintInvalid,
                                                rmIaIdInvalid,
                                                rmDstHostIrqInvalid,
                                                rmDstIdInvalid};
    uint8_t  num;
    uint32_t numConditions                   = 6U;
    struct tisci_msg_rm_irq_set_resp sciclient_DirectEventResp;

    /* To cover MC/DC for Sciclient_rmIrqCfgIsDirectEvent() */
    for(num = 0; num <= numConditions; num++)
    {
        const struct tisci_msg_rm_irq_set_req sciclient_DirectEventReq =
        {
            .valid_params = inValidParams[num]
        };
        status = Sciclient_rmProgramInterruptRoute(&sciclient_DirectEventReq,
                                                    &sciclient_DirectEventResp,
                                                    SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            rmIrqCfgIsDirectEventTestStatus += CSL_PASS;
        }
        else
        {
            rmIrqCfgIsDirectEventTestStatus += CSL_EFAIL;
        }
    }

    if(rmIrqCfgIsDirectEventTestStatus == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmIrqCfgIsDirectEvent() Test Passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmIrqCfgIsDirectEvent() Test Failed.\n");
    }

    return rmIrqCfgIsDirectEventTestStatus;
}

static int32_t SciclientApp_rmIrqCfgIsDirectNonEventTest(void)
{
    int32_t status                                = CSL_PASS;
    int32_t rmIrqCfgIsDirectNonEventTestStatus    = CSL_PASS;
    int32_t rmIrqCfgIsDirectNonEventParms[7]      = {(TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID  | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
													 (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID),
													 (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_VINT_VALID),
													 (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID),
                                                      TISCI_MSG_VALUE_RM_DST_ID_VALID,
                                                      TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
                                                     (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID)
													};
    int8_t number;
    uint32_t numConditions                        = 6U;
    struct tisci_msg_rm_irq_release_req rmIrqReleaseReq;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseResp;

    /* To cover MC/DC for Sciclient_rmIrqCfgIsDirectNonEvent() */
    for(number = 0; number <= numConditions; number++)
    {
        rmIrqReleaseReq.valid_params = rmIrqCfgIsDirectNonEventParms[number];
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseReq, &rmIrqReleaseResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            rmIrqCfgIsDirectNonEventTestStatus += CSL_PASS;
        }
        else
        {
            rmIrqCfgIsDirectNonEventTestStatus += CSL_EFAIL;
        }
    }

    if(rmIrqCfgIsDirectNonEventTestStatus == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmIrqCfgIsDirectNonEvent() Test Passed.\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmIrqCfgIsDirectNonEvent() Test Failed.\n");
    }

    return rmIrqCfgIsDirectNonEventTestStatus;
}

static int32_t SciclientApp_rmIrqValidParamsNegTest(void)
{
    int32_t  rmIrqTestStatus      = CSL_PASS;

   /* For example, if a branch statement exists in the following way:
      if(A && B && C && D) ==> Inorder to cover 16(2^4) combinations for MC/DC, we have to just cover 5 combinations given below:
      (FFFF) ==> covers 8 combinations {The "&&" operation will stop it's search once it reaches a false condition, it doesn't
                                        care about what the other conditions evaluates to. So, on covering (FFFF) LDRA will
                                        assume that (FTFF),(FTTF),(FTTT) etc... are also covered.}
      (TFFF) ==> covers 4 combinations
      (TTFF) ==> covers 2 combinations
      (TTTF) ==> covers 1 combination
      (TTTT) ==> covers 1 combination

      The above logic is followed for all the below five sub-tests. These sub-tests have 6 conditions in their if statement,
      so only 7 combinations are needed to covered inorder to cover all 64(2^6) MC/DC combinations. */

    SciApp_printf("This test has five sub-tests:\n");
    rmIrqTestStatus += SciclientApp_rmIrqCfgIsUnmappedVintDirectEventNegTest();
    rmIrqTestStatus += SciclientApp_rmIrqCfgIsEventToVintMappingOnlyNegTest();
    rmIrqTestStatus += SciclientApp_rmIrqCfgIsOesOnlyNegTest();
    rmIrqTestStatus += SciclientApp_rmIrqCfgIsDirectEventTest();
    rmIrqTestStatus += SciclientApp_rmIrqCfgIsDirectNonEventTest();

    return rmIrqTestStatus;
}

static int32_t SciclientApp_rmClearInterruptRouteTest(void)
{
    int32_t status                            = CSL_PASS;
    int32_t rmClearInterruptRouteTestStatus   = CSL_PASS;
    int32_t rmClearInterruptRouteTestCases[4] = {0xffffff07,0x000000fc,0x0000000f,0x0000003f};
    int8_t number;
    struct tisci_msg_rm_irq_release_req rmIrqReleaseReq;
    struct tisci_msg_rm_irq_release_req rmIrqReleaseFailReq;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseFailResp;
    struct tisci_msg_rm_irq_release_req rmIrqReleaseValidParamsReq;
    rmIrqReleaseValidParamsReq.valid_params = TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID;

    /* Passing a NULL request and NULL response */
    status = Sciclient_rmClearInterruptRoute(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmClearInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmClearInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Passing a NULL response */
    status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseReq, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmClearInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmClearInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Covers negative test for Sciclient_rmIrqCfgIsOesOnly */
    status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseValidParamsReq, &rmIrqReleaseFailResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmClearInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmClearInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Covers the various event types */
    for(number = 0; number < 4; number++)
    {
        rmIrqReleaseFailReq.valid_params = rmClearInterruptRouteTestCases[number];
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseFailReq, &rmIrqReleaseFailResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EBADARGS)
        {
            rmClearInterruptRouteTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
        }
        else
        {
            rmClearInterruptRouteTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
        }
    }

    return rmClearInterruptRouteTestStatus;
}

static int32_t SciclientApp_rmProgramInterruptRouteTest(void)
{
    int32_t status                                           = CSL_PASS;
    int32_t rmProgramInterruptRouteTestStatus                = CSL_PASS;
    int32_t rmProgramInterruptRouteTestparams[5]             = {0xffffff07,0x000000fc,0x0000000f,0x00000010,0x0000003f};
    int8_t  number;
    struct tisci_msg_rm_irq_set_req rmIrqSetReq;
    struct tisci_msg_rm_irq_set_resp rmIrqSetResp;

    /* Passing a NULL request and a NULL response parameter */
    status = Sciclient_rmProgramInterruptRoute(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmProgramInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmProgramInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Passing a NULL response parameter */
    status = Sciclient_rmProgramInterruptRoute(&rmIrqSetReq, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmProgramInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmProgramInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Covers the various event types */
    for(number = 0; number < 5; number++)
    {
        rmIrqSetReq.valid_params = rmProgramInterruptRouteTestparams[number];
        status = Sciclient_rmProgramInterruptRoute(&rmIrqSetReq, &rmIrqSetResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            rmProgramInterruptRouteTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
        }
        else
        {
            rmProgramInterruptRouteTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
        }
    }

    /* Passing src_id as interrupt aggregator and remaining parameters as invalid covers Sciclient_rmIaIsIa() in sciclient_rmProgramInterruptRoute() */
    rmIrqSetReq.valid_params = (TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID);
    rmIrqSetReq.src_id       = TISCI_DEV_NAVSS0_MODSS_INTAGG1 ;
    status = Sciclient_rmProgramInterruptRoute(&rmIrqSetReq, &rmIrqSetResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status != CSL_PASS )
    {
        rmProgramInterruptRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmProgramInterruptRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmProgramInterruptRouteTestStatus;
}

static int32_t SciclientApp_rmIrqUnmappedVintRouteDeleteNegTest(void)
{
    int32_t  status                                 = CSL_PASS;
    int32_t  rmIrqUnmappedVintRouteDeleteTestStatus = CSL_PASS;
    struct tisci_msg_rm_irq_release_resp sciclient_UnmappedVintRouteDeleteResp;
    const struct tisci_msg_rm_irq_release_req sciclient_UnmappedVintRouteDeleteReqIa =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
        .src_id       = TISCI_DEV_NAVSS0_MODSS_INTAGG
    };
    const struct tisci_msg_rm_irq_release_req sciclient_UnmappedVintRouteDeleteReq   =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                        TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID,
        .src_id       = TISCI_DEV_NAVSS0_MAILBOX_0
    };

    /* By passing src_id as TISCI_DEV_NAVSS0_MODSS_INTAGG, Sciclient_rmIaVintGetInfo function will return pass */
    status = Sciclient_rmClearInterruptRoute(&sciclient_UnmappedVintRouteDeleteReqIa,
                                                &sciclient_UnmappedVintRouteDeleteResp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqUnmappedVintRouteDeleteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqUnmappedVintRouteDeleteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* By passing src_id as TISCI_DEV_NAVSS0_MAILBOX_0, Sciclient_rmIaVintGetInfo function will return fail */
    status = Sciclient_rmClearInterruptRoute(&sciclient_UnmappedVintRouteDeleteReq,
                                                &sciclient_UnmappedVintRouteDeleteResp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIrqUnmappedVintRouteDeleteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqUnmappedVintRouteDeleteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmIrqUnmappedVintRouteDeleteTestStatus;
}

static int32_t SciclientApp_rmIrqFindRouteNegTest(void)
{
    int32_t  status                                 = CSL_PASS;
    int32_t  rmIrqFindRouteTestStatus               = CSL_PASS;
    struct tisci_msg_rm_irq_set_req Sciclient_ReqSrcIr =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
        .src_id       = TISCI_DEV_NAVSS0_INTR
    };
    struct tisci_msg_rm_irq_set_req Sciclient_ReqIa =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                        TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                        TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID,
        .ia_id        = TISCI_DEV_NAVSS0_MODSS_INTAGG
    };
    struct tisci_msg_rm_irq_set_req Sciclient_ReqDstIr =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
        .src_id       = TISCI_DEV_NAVSS0_MAILBOX_0,
        .dst_id       = TISCI_DEV_NAVSS0_INTR
    };
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;

    /* Passing source_id as IR fails the source and destination node checks for Sciclient_rmIrqFindRoute function */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_ReqSrcIr,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIrqFindRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqFindRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Passing device ID of interrupt aggregator will retrieve source device's node for Sciclient_rmIrqFindRoute function */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_ReqIa,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqFindRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqFindRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    /* Passing dest_id as IR fails the source and destination node checks for Sciclient_rmIrqFindRoute function */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_ReqDstIr,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIrqFindRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqFindRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmIrqFindRouteTestStatus;
}

static int32_t SciclientApp_rmIaVintGetInfoNegTest(void)
{
    int32_t  status                                     = CSL_PASS;
    int32_t  rmIaVintGetInfoTestStatus                  = CSL_PASS;
    const struct tisci_msg_rm_irq_set_req Sciclient_Req =
    {
        .valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                          TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID,
        .ia_id          = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
        .vint           = 300U /* Invalid vint value for TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0 */
    };
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;

    /* Passing invalid vint value(greater than the vint value of ia_id) to cover the badargs condition for Sciclient_rmIaVintGetInfo */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIaVintGetInfoTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIaVintGetInfoTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmIaVintGetInfoTestStatus;
}

static int32_t SciclientApp_rmIrqIsVintRouteSetNegTest(void)
{
    int32_t  status                                     = CSL_PASS;
    int32_t  rmIrqIsVintRouteSetTestStatus              = CSL_PASS;
    const struct tisci_msg_rm_irq_set_req Sciclient_Req =
    {
        .valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                          TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                          TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID,
        .ia_id          = TISCI_DEV_NAVSS0_MODSS_INTAGG,
        .vint           = 200U /* Invalid vint value for TISCI_DEV_NAVSS0_MODSS_INTAGG */
    };
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;

    /* "IR input tied to the IA VINT is in use" check will fail by passing invalid vint value to Sciclient_rmIrqIsVintRouteSet */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqIsVintRouteSetTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrqIsVintRouteSetTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmIrqIsVintRouteSetTestStatus;
}

static int32_t SciclientApp_rmIrqGetRouteTest(void)
{
  int32_t  status                  = CSL_PASS;
  int32_t  rmIrqGetRouteTestStatus = CSL_PASS;
  uint16_t intNum                  = 0U;
  const struct tisci_msg_rm_irq_set_resp Sciclient_Resp                  = {0};
  struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqVint   = {0};
  struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqGlobal = {0};
  struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqIrq    = {0};
  struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespVint;
  struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespGlobal;
  struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespIrq;
  struct tisci_msg_rm_irq_release_resp rmIrqReleaseResp;
  struct tisci_msg_rm_irq_release_resp sciclient_rmIrqGetRouteResp;
  struct tisci_msg_rm_irq_release_req sciclient_rmIrqGetRouteReq =
  {
      .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
      .src_id       = TISCI_DEV_NAVSS0_INTR
  };

    /* Passing src_id as TISCI_DEV_NAVSS0_INTR, Sciclient_rmIrqGetRoute function will return CSL_EBADARGS */
    status = Sciclient_rmClearInterruptRoute(&sciclient_rmIrqGetRouteReq,
                                            &sciclient_rmIrqGetRouteResp,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIrqGetRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-1 Passed.\n");
    }
    else
    {
        rmIrqGetRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-1 Failed.\n");
    }

    /* Passing dst_id TISCI_DEV_NAVSS0_INTR */
    sciclient_rmIrqGetRouteReq.src_id = TISCI_DEV_R5FSS0_CORE0;
    sciclient_rmIrqGetRouteReq.dst_id = TISCI_DEV_NAVSS0_INTR;
    status = Sciclient_rmClearInterruptRoute(&sciclient_rmIrqGetRouteReq,
                                            &sciclient_rmIrqGetRouteResp,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EBADARGS)
    {
        rmIrqGetRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-2 Passed.\n");
    }
    else
    {
        rmIrqGetRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-2 Failed.\n");
    }

    /* Passing invalid Params */
    sciclient_rmIrqGetRouteReq.src_id       = TISCI_DEV_R5FSS0_CORE0;
    sciclient_rmIrqGetRouteReq.dst_id       = TISCI_DEV_R5FSS0_CORE1;
    sciclient_rmIrqGetRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID
                                            | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID
                                            | TISCI_MSG_VALUE_RM_IA_ID_VALID
                                            | TISCI_MSG_VALUE_RM_VINT_VALID;
    sciclient_rmIrqGetRouteReq.ia_id        = TISCI_DEV_NAVSS0_MODSS_INTAGG;

    status = Sciclient_rmClearInterruptRoute(&sciclient_rmIrqGetRouteReq,
                                            &sciclient_rmIrqGetRouteResp,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqGetRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-3 Passed.\n");
    }
    else
    {
        rmIrqGetRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-3 Failed.\n");
    }

    rmGetResourceRangeReqVint.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    rmGetResourceRangeReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    rmGetResourceRangeReqVint.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqVint,
                                            &rmGetResourceRangeRespVint,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqGetRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for vint\n");
    }
    else
    {
        rmIrqGetRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for vint\n");
    }

    rmGetResourceRangeReqGlobal.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    rmGetResourceRangeReqGlobal.subtype        = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    rmGetResourceRangeReqGlobal.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqGlobal,
                                            &rmGetResourceRangeRespGlobal,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqGetRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful for globalevent\n");
    }
    else
    {
        rmIrqGetRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for globalevent\n");
    }

    rmGetResourceRangeReqIrq.type           = TISCI_DEV_MCU_NAVSS0_INTR;
    rmGetResourceRangeReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    rmGetResourceRangeReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqIrq,
                                            &rmGetResourceRangeRespIrq,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(rmGetResourceRangeReqIrq.type,
                                                    rmGetResourceRangeRespIrq.range_start,
                                                    TISCI_DEV_MCU_R5FSS0_CORE0,
                                                    &intNum);
        if(status == CSL_PASS)
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() execution is successful and host interrupt number is %d\n", intNum);
            const struct tisci_msg_rm_irq_set_req Sciclient_Req =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                         TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = rmGetResourceRangeRespGlobal.range_start,
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = rmGetResourceRangeRespVint.range_start,
                .vint_status_bit_index = 0U,
                .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
            };
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_PASS)
            {
                rmIrqGetRouteTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Passed.\n");
            }
            else
            {
                rmIrqGetRouteTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: Valid Arg Test Failed.\n");
            }
        }
        else
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

  	const struct tisci_msg_rm_irq_release_req rmIrqReleaseReq =
  	{
  	    .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
  		                         TISCI_MSG_VALUE_RM_IA_ID_VALID  | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
  	    .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
  	    .src_index             = 0U,
  	    .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
  	    .dst_host_irq          = intNum + 1U,
  	    .global_event          = rmGetResourceRangeRespGlobal.range_start,
  	    .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
  	    .vint                  = rmGetResourceRangeRespVint.range_start,
  	    .vint_status_bit_index = 0U,
        .secondary_host        = TISCI_HOST_ID_MCU_0_R5_0
  	};
  	status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseReq, &rmIrqReleaseResp, SCICLIENT_SERVICE_WAIT_FOREVER);
  	if (status == CSL_EFAIL)
  	{
  	    rmIrqGetRouteTestStatus += CSL_PASS;
  	    SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-4 Passed.\n");
  	}
  	else
  	{
  	    rmIrqGetRouteTestStatus += CSL_EFAIL;
  	    SciApp_printf("Sciclient_rmIrqGetRoute: Negative Arg Test-4 Failed.\n");
  	}

    return rmIrqGetRouteTestStatus;
}

static int32_t SciclientApp_rmIrqFindRouteTest(void)
{
  int32_t  status                   = CSL_PASS;
  int32_t  rmIrqFindRouteTestStatus = CSL_PASS;
  struct tisci_msg_rm_irq_set_resp sciclient_rmIrqFindRouteResp;
  struct tisci_msg_rm_irq_set_req sciclient_rmIrqFindRouteReq =
  {
      .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID
                      | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID,
      .src_id       = TISCI_DEV_NAVSS0_INTR,
      .ia_id        = TISCI_DEV_NAVSS0_MODSS_INTAGG,
      .src_id       = TISCI_DEV_R5FSS0_CORE0,
      .dst_id       = TISCI_DEV_R5FSS0_CORE1,
      .dst_host_irq = 220U
  };

    /* Passing invalid dst_host_irq */
    status = Sciclient_rmProgramInterruptRoute(&sciclient_rmIrqFindRouteReq,
                                                &sciclient_rmIrqFindRouteResp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqFindRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqFindRoute: Negative Arg Test-1 Passed.\n");
    }
    else
    {
        rmIrqFindRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqFindRoute: Negative Arg Test-1 Failed.\n");
    }

    /* Passing invalid dst_host_irq */
    sciclient_rmIrqFindRouteReq.dst_host_irq = 260U;
    status = Sciclient_rmProgramInterruptRoute(&sciclient_rmIrqFindRouteReq,
                                                &sciclient_rmIrqFindRouteResp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqFindRouteTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmIrqFindRoute: Negative Arg Test-2 Passed.\n");
    }
    else
    {
        rmIrqFindRouteTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmIrqFindRoute: Negative Arg Test-2 Failed.\n");
    }

    return rmIrqFindRouteTestStatus;
}

static int32_t SciclientApp_rmIrqVintRouteTest(void)
{
    /* Use different global_event, vint for different SoC's and cores */
    int32_t rmIrqTeststatus = CSL_PASS;
    int32_t status = CSL_PASS;
    struct tisci_msg_rm_irq_set_req VintMappingOnlyProgramRouteReq;
    struct tisci_msg_rm_irq_set_resp VintMappingOnlyProgramRouteResp;
    struct tisci_msg_rm_irq_set_req DirectEventprogramRouteReq;
    struct tisci_msg_rm_irq_set_resp DirectEventProgramRouteResp;
    struct tisci_msg_rm_irq_set_req sciclientRmIrRouteReq;
    struct tisci_msg_rm_irq_set_resp sciclientRmIrRouteResp;
    struct tisci_msg_rm_irq_release_req VintMappingOnlyDeleteRouteReq;
    struct tisci_msg_rm_irq_release_resp VintMappingOnlyDeleteRouteResp;
    struct tisci_msg_rm_irq_release_req DirectEventDeleteRouteReq;
    struct tisci_msg_rm_irq_release_resp DirectEventDeleteRouteResp;
    struct tisci_msg_rm_irq_release_req VintMappingOnlyDeleteRouteNegReq;
    struct tisci_msg_rm_irq_release_resp VintMappingOnlyDeleteRouteNegResp;
    struct tisci_msg_rm_irq_release_req DirectEventDeleteRouteNegReq;
    struct tisci_msg_rm_irq_release_resp DirectEventDeleteRouteNegResp;
    #if defined(SOC_J784S4)
    struct tisci_msg_rm_irq_set_resp rmIrqSetRespIrInpRomMappedFail;
    #endif
    #if !defined(SOC_J7200)
    struct tisci_msg_rm_irq_set_resp rmIrqSetRespIrInpIsFreeFail;
    struct tisci_msg_rm_irq_set_req rmIrqSetReqIrInpIsFreeFail =
    {
        .valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                        TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                        TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID |
                        TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .vint           = 0,
        .ia_id          = TISCI_DEV_CSI_RX_IF0,
        .secondary_host = TISCI_HOST_ID_MAIN_0_R5_0
    };
    #endif
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqGlobal = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespGlobal;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqVint = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespVint;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqSrcIdx = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespSrcIdx;

    rmGetResourceRangeReqGlobal.type           = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    rmGetResourceRangeReqGlobal.subtype        = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    rmGetResourceRangeReqGlobal.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqGlobal,
                                           &rmGetResourceRangeRespGlobal,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    rmGetResourceRangeReqVint.type           = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    rmGetResourceRangeReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    rmGetResourceRangeReqVint.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqVint,
                                           &rmGetResourceRangeRespVint,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    rmGetResourceRangeReqSrcIdx.type            = TISCI_DEV_NAVSS0_RINGACC_0;
    rmGetResourceRangeReqSrcIdx.subtype         = TISCI_RESASG_SUBTYPE_RA_UDMAP_TX;
    rmGetResourceRangeReqSrcIdx.secondary_host  = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqSrcIdx,
                                           &rmGetResourceRangeRespSrcIdx,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    /* Programs the interrupt Route for VintMappingOnly */
    VintMappingOnlyProgramRouteReq.valid_params = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                    TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    VintMappingOnlyProgramRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    VintMappingOnlyProgramRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    VintMappingOnlyProgramRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    VintMappingOnlyProgramRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    VintMappingOnlyProgramRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    VintMappingOnlyProgramRouteReq.vint_status_bit_index = 0;
    VintMappingOnlyProgramRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmIrqSet(&VintMappingOnlyProgramRouteReq, &VintMappingOnlyProgramRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for VintMappingOnly has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for VintMappingOnly has failed.\n");
    }

    /* Clears the interrupt Route set for VintMappingOnly */
    VintMappingOnlyDeleteRouteReq.valid_params = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                    TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    VintMappingOnlyDeleteRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    VintMappingOnlyDeleteRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    VintMappingOnlyDeleteRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    VintMappingOnlyDeleteRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    VintMappingOnlyDeleteRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    VintMappingOnlyDeleteRouteReq.vint_status_bit_index = 0;
    VintMappingOnlyDeleteRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmClearInterruptRoute(&VintMappingOnlyDeleteRouteReq, &VintMappingOnlyDeleteRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute for VintMappingOnly has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute for VintMappingOnly has failed.\n");
    }

    /* Programs the interrupt Route for DirectEvent */
    DirectEventprogramRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    DirectEventprogramRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    DirectEventprogramRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    DirectEventprogramRouteReq.dst_id = TISCI_DEV_R5FSS0_CORE0;
    DirectEventprogramRouteReq.dst_host_irq = 228;
    DirectEventprogramRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    DirectEventprogramRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    DirectEventprogramRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    DirectEventprogramRouteReq.vint_status_bit_index = 0;
    DirectEventprogramRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmProgramInterruptRoute(&DirectEventprogramRouteReq, &DirectEventProgramRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for DirectEvent has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for DirectEvent has failed.\n");
    }

    /* Clears the interrupt Route for DirectEvent */
    DirectEventDeleteRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    DirectEventDeleteRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    DirectEventDeleteRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    DirectEventDeleteRouteReq.dst_id = TISCI_DEV_R5FSS0_CORE0;
    DirectEventDeleteRouteReq.dst_host_irq = 228;
    DirectEventDeleteRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    DirectEventDeleteRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    DirectEventDeleteRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    DirectEventDeleteRouteReq.vint_status_bit_index = 0;
    DirectEventDeleteRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmClearInterruptRoute(&DirectEventDeleteRouteReq, &DirectEventDeleteRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute for DirectEvent has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute for DirectEvent has failed.\n");
    }

    /* Negative test for Sciclient_rmIrqVintDelete() and del_mapping=True */
    VintMappingOnlyDeleteRouteNegReq.valid_params = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                    TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    VintMappingOnlyDeleteRouteNegReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    VintMappingOnlyDeleteRouteNegReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    VintMappingOnlyDeleteRouteNegReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    VintMappingOnlyDeleteRouteNegReq.vint = rmGetResourceRangeRespVint.range_start;
    VintMappingOnlyDeleteRouteNegReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    VintMappingOnlyDeleteRouteNegReq.vint_status_bit_index = 0;
    VintMappingOnlyDeleteRouteNegReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmClearInterruptRoute(&VintMappingOnlyDeleteRouteNegReq, &VintMappingOnlyDeleteRouteNegResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute has failed.\n");
    }

    /* Negative test for Sciclient_rmIrqVintDelete() and del_whole_route=True */
    DirectEventDeleteRouteNegReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    DirectEventDeleteRouteNegReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    DirectEventDeleteRouteNegReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    DirectEventDeleteRouteNegReq.dst_id = TISCI_DEV_R5FSS0_CORE0;
    DirectEventDeleteRouteNegReq.dst_host_irq = 228;
    DirectEventDeleteRouteNegReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    DirectEventDeleteRouteNegReq.vint = rmGetResourceRangeRespVint.range_start;
    DirectEventDeleteRouteNegReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    DirectEventDeleteRouteNegReq.vint_status_bit_index = 0;
    DirectEventDeleteRouteNegReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmClearInterruptRoute(&DirectEventDeleteRouteNegReq, &DirectEventDeleteRouteNegResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute has failed.\n");
    }

    /* Test to fail sciclientRmIr() function in Sciclient_rmProgramInterruptRoute() */
    sciclientRmIrRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                            TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    sciclientRmIrRouteReq.ia_id = TISCI_DEV_NAVSS0_INTR;
    sciclientRmIrRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmProgramInterruptRoute(&sciclientRmIrRouteReq, &sciclientRmIrRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Negative test for sciclientRmIr() has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Negative test for sciclientRmIr() has failed.\n");
    }
    #if defined(SOC_J784S4)
    struct tisci_msg_rm_irq_set_req rmIrqSetReqIrInpRomMappedFail =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                        TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                        TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .ia_id        = TISCI_DEV_NAVSS0_MODSS_INTAGG,
        .secondary_host = TISCI_HOST_ID_MAIN_0_R5_0
    };

    /* Updating output control register value to match with input line to IR in order to cover Sciclient_rmIrInpRomMapped function */
    CSL_REG32_WR_OFF(0x310E0004U, 0, 320);
    /* Passing the required paramets to cover Sciclient_rmIrInpIsFree and Sciclient_rmIrqIsVintRouteSet */
    status = Sciclient_rmProgramInterruptRoute(&rmIrqSetReqIrInpRomMappedFail, &rmIrqSetRespIrInpRomMappedFail, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpIsFree and Sciclient_rmIrqIsVintRouteSetA rg Test Passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpIsFree and Sciclient_rmIrqIsVintRouteSetArg Arg Test Failed.\n");
    }
    #endif

    #if !defined(SOC_J7200)
    /* Passing the required paramets to cover Sciclient_rmIrInpIsFree */
    status = Sciclient_rmProgramInterruptRoute(&rmIrqSetReqIrInpIsFreeFail, &rmIrqSetRespIrInpIsFreeFail, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpIsFree Arg Test Passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpIsFree Arg Test Failed.\n");
    }
    #endif

    return rmIrqTeststatus;
}

static int32_t SciclientApp_rmIrqClearRouteNegTest(void)
{
    int32_t status = CSL_PASS;
    int32_t rmIrqTeststatus = CSL_PASS;
    struct tisci_msg_rm_irq_set_req VintMappingOnlyProgramRouteReq;
    struct tisci_msg_rm_irq_set_resp VintMappingOnlyProgramRouteResp;
    struct tisci_msg_rm_irq_release_req VintMappingOnlyDeleteRouteReq;
    struct tisci_msg_rm_irq_release_resp VintMappingOnlyDeleteRouteResp;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseNegResp1;
    struct tisci_msg_rm_irq_release_resp rmIrqReleaseNegResp2;
    struct tisci_msg_rm_irq_set_req DirectEventprogramRouteReq;
    struct tisci_msg_rm_irq_set_resp DirectEventProgramRouteResp;
    struct tisci_msg_rm_irq_release_req DirectEventDeleteRouteReq;
    struct tisci_msg_rm_irq_release_resp DirectEventDeleteRouteResp;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqGlobal = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespGlobal;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqVint = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespVint;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqSrcIdx = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespSrcIdx;

    rmGetResourceRangeReqGlobal.type           = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    rmGetResourceRangeReqGlobal.subtype        = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    rmGetResourceRangeReqGlobal.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqGlobal,
                                           &rmGetResourceRangeRespGlobal,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    rmGetResourceRangeReqVint.type           = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    rmGetResourceRangeReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    rmGetResourceRangeReqVint.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqVint,
                                           &rmGetResourceRangeRespVint,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    rmGetResourceRangeReqSrcIdx.type            = TISCI_DEV_NAVSS0_RINGACC_0;
    rmGetResourceRangeReqSrcIdx.subtype         = TISCI_RESASG_SUBTYPE_RA_UDMAP_TX;
    rmGetResourceRangeReqSrcIdx.secondary_host  = TISCI_HOST_ID_MAIN_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqSrcIdx,
                                           &rmGetResourceRangeRespSrcIdx,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

    /* Programs the interrupt Route for VintMappingOnly */
    VintMappingOnlyProgramRouteReq.valid_params = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                  TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    VintMappingOnlyProgramRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    VintMappingOnlyProgramRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    VintMappingOnlyProgramRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    VintMappingOnlyProgramRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    VintMappingOnlyProgramRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    VintMappingOnlyProgramRouteReq.vint_status_bit_index = 0;
    VintMappingOnlyProgramRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmProgramInterruptRoute(&VintMappingOnlyProgramRouteReq, &VintMappingOnlyProgramRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for VintMappingOnly has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for VintMappingOnly has failed.\n");
    }

    if(status == CSL_PASS)
    {
        const struct tisci_msg_rm_irq_release_req rmIrqReleaseNegReq1 =
        {
            .valid_params          = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                     TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
            .src_id                = TISCI_DEV_NAVSS0_RINGACC_0,
            .src_index             = rmGetResourceRangeRespSrcIdx.range_start,
            .global_event          = (uint16_t)1000000,
            .ia_id                 = TISCI_DEV_NAVSS0_UDMASS_INTAGG,
            .vint                  = rmGetResourceRangeRespVint.range_start,
            .vint_status_bit_index = 0U,
            .secondary_host        = TISCI_HOST_ID_MAIN_0_R5_0
        };
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseNegReq1, &rmIrqReleaseNegResp1, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            rmIrqTeststatus += CSL_PASS;
            SciApp_printf("Negative test for Sciclient_rmIrqVintDelete:VintMappingOnly has passed.\n");
        }
        else
        {
            rmIrqTeststatus += CSL_EFAIL;
            SciApp_printf("Negative test for Sciclient_rmIrqVintDelete:VintMappingOnly has failed.\n");
        }

        const struct tisci_msg_rm_irq_release_req rmIrqReleaseNegReq2 =
        {
            .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                     TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
            .src_id                = TISCI_DEV_NAVSS0_INTR,
            .dst_id                = TISCI_DEV_NAVSS0_INTR,
            .src_index             = 1536U,
            .global_event          = (uint16_t)1000000,
            .ia_id                 = TISCI_DEV_NAVSS0_UDMASS_INTAGG,
            .vint                  = rmGetResourceRangeRespVint.range_start,
            .vint_status_bit_index = 0U,
            .secondary_host        = TISCI_HOST_ID_MAIN_0_R5_0
        };
        status = Sciclient_rmClearInterruptRoute(&rmIrqReleaseNegReq2, &rmIrqReleaseNegResp2, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_EFAIL)
        {
            rmIrqTeststatus += CSL_PASS;
            SciApp_printf("Negative test for Sciclient_rmIrqVintDelete:Direct event has passed.\n");
        }
        else
        {
            rmIrqTeststatus += CSL_EFAIL;
            SciApp_printf("Negative test for Sciclient_rmIrqVintDelete:Direct event has failed.\n");
        }

        /* Clears the interrupt Route set for VintMappingOnly */
        VintMappingOnlyDeleteRouteReq.valid_params = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                                        TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
        VintMappingOnlyDeleteRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
        VintMappingOnlyDeleteRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
        VintMappingOnlyDeleteRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
        VintMappingOnlyDeleteRouteReq.vint = rmGetResourceRangeRespVint.range_start;
        VintMappingOnlyDeleteRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
        VintMappingOnlyDeleteRouteReq.vint_status_bit_index = 0;
        VintMappingOnlyDeleteRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
        status = Sciclient_rmClearInterruptRoute(&VintMappingOnlyDeleteRouteReq, &VintMappingOnlyDeleteRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            rmIrqTeststatus += CSL_PASS;
            SciApp_printf("Sciclient_rmClearInterruptRoute for VintMappingOnly has passed.\n");
        }
        else
        {
            rmIrqTeststatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmClearInterruptRoute for VintMappingOnly has failed.\n");
        }
    }

    /* Programs the interrupt Route for DirectEvent */
    DirectEventprogramRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                              TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    DirectEventprogramRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    DirectEventprogramRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    DirectEventprogramRouteReq.dst_id = TISCI_DEV_R5FSS0_CORE0;
    DirectEventprogramRouteReq.dst_host_irq = 228;
    DirectEventprogramRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    DirectEventprogramRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    DirectEventprogramRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    DirectEventprogramRouteReq.vint_status_bit_index = 0;
    DirectEventprogramRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmProgramInterruptRoute(&DirectEventprogramRouteReq, &DirectEventProgramRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for DirectEvent has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute for DirectEvent has failed.\n");
    }

    /* Negative test for clearing the interrupt Route for DirectEvent */
    DirectEventDeleteRouteReq.valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                             TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
    DirectEventDeleteRouteReq.src_id = TISCI_DEV_NAVSS0_RINGACC_0;
    DirectEventDeleteRouteReq.src_index = rmGetResourceRangeRespSrcIdx.range_start;
    DirectEventDeleteRouteReq.dst_id = TISCI_DEV_R5FSS0_CORE0;
    DirectEventDeleteRouteReq.dst_host_irq = 228;
    DirectEventDeleteRouteReq.ia_id = TISCI_DEV_NAVSS0_UDMASS_INTAGG;
    DirectEventDeleteRouteReq.vint = rmGetResourceRangeRespVint.range_start;
    DirectEventDeleteRouteReq.global_event = rmGetResourceRangeRespGlobal.range_start;
    DirectEventDeleteRouteReq.vint_status_bit_index = 73U;
    DirectEventDeleteRouteReq.secondary_host = TISCI_HOST_ID_MAIN_0_R5_0;
    status = Sciclient_rmClearInterruptRoute(&DirectEventDeleteRouteReq, &DirectEventDeleteRouteResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        rmIrqTeststatus += CSL_PASS;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute:DirectEvent has passed.\n");
    }
    else
    {
        rmIrqTeststatus += CSL_EFAIL;
        SciApp_printf("Negative test for Sciclient_rmClearInterruptRoute:DirectEvent has failed.\n");
    }

    return rmIrqTeststatus;
}

static int32_t SciclientApp_rmIrGetOutpTest(void)
{
    int32_t  status                = CSL_PASS;
    int32_t  rmIrGetOutpTestStatus = CSL_PASS;
    struct tisci_msg_rm_irq_release_resp sciclient_RmIrGetOutpResp;
    const struct tisci_msg_rm_irq_release_req sciclient_RmIrGetOutpReq =
    {
        .valid_params = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID,
        .src_id       = TISCI_DEV_NAVSS0_UDMASS_INTAGG
    };

    /* By passing src_id as TISCI_DEV_NAVSS0_UDMASS_INTAGG, in Sciclient_rmIrGetOutp inp = 0 condition will be true */
    status = Sciclient_rmClearInterruptRoute(&sciclient_RmIrGetOutpReq,
                                             &sciclient_RmIrGetOutpResp,
                                             SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrGetOutpTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Passed.\n");
    }
    else
    {
        rmIrGetOutpTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmClearInterruptRoute: Negative Arg Test Failed.\n");
    }

    return rmIrGetOutpTestStatus;
}

static int32_t SciclientApp_RmIrOutpRomMappedTest(void)
{
    int32_t  status                                                     = CSL_PASS;
    int32_t  rmIrInpRomMappedTestStatus                                 = CSL_PASS;
    uint16_t intNum                                                     = 0U;
    struct tisci_msg_rm_get_resource_range_req rmGetResourceRangeReqIrq = {0};
    struct tisci_msg_rm_get_resource_range_resp rmGetResourceRangeRespIrq;
    #if defined(SOC_J784S4)
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;
    #endif

    rmGetResourceRangeReqIrq.type           = TISCI_DEV_NAVSS0_INTR;
    rmGetResourceRangeReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    rmGetResourceRangeReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&rmGetResourceRangeReqIrq,
                                            &rmGetResourceRangeRespIrq,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(rmGetResourceRangeReqIrq.type,
                                                    rmGetResourceRangeRespIrq.range_start,
                                                    TISCI_DEV_MCU_R5FSS0_CORE0,
                                                    &intNum);
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed\n");
    }

    #if defined(SOC_J784S4)
    struct tisci_msg_rm_irq_set_req Sciclient_ReqIr =
    {
        .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
        .src_id                = TISCI_DEV_NAVSS0_CPTS_0,
        .src_index             = 0U,
        .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
        .dst_host_irq          = intNum,
        .vint_status_bit_index = 0U
    };

    /* Updating output control register value to match with input line to IR in order to cover Sciclient_rmIrInpRomMapped function */
    CSL_REG32_WR_OFF(0x310E0644U, 0, 1028);
    /* By passing these parameters Sciclient_rmIrInpRomMapped function will return false because of rom_usage is NULL */
    status = Sciclient_rmProgramInterruptRoute(&Sciclient_ReqIr,
                                                &Sciclient_Resp,
                                                SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        rmIrInpRomMappedTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Passed.\n");
    }
    else
    {
        rmIrInpRomMappedTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmProgramInterruptRoute: Sciclient_rmIrInpRomMapped Arg Test Failed.\n");
    }
    #endif

    return rmIrInpRomMappedTestStatus;
}

static int32_t SciclientApp_iaEvtRomMappedTest()
{
    int32_t  status                         = CSL_PASS;
    int32_t  rmIaValidateEvtTestStatus      = CSL_PASS;
    uint16_t intNum                         = 0U;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqVint;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResVint;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqGlobal;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResGlobal;
    struct tisci_msg_rm_get_resource_range_req Sciclient_ReqIrq;
    struct tisci_msg_rm_get_resource_range_resp Sciclient_ResIrq;
    #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp1;
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp2;
    #endif

    Sciclient_ReqVint.type           = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    Sciclient_ReqVint.subtype        = TISCI_RESASG_SUBTYPE_IA_VINT;
    Sciclient_ReqVint.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqVint, &Sciclient_ResVint, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() 123execution is successful for vint\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for vint\n");
    }

    Sciclient_ReqGlobal.type            = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0;
    Sciclient_ReqGlobal.subtype         = TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT;
    Sciclient_ReqGlobal.secondary_host  = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqGlobal, &Sciclient_ResGlobal, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() 12execution is successful for globalevent\n");
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is failed for globalevent\n");
    }

    Sciclient_ReqIrq.type           = TISCI_DEV_MCU_NAVSS0_INTR;
    Sciclient_ReqIrq.subtype        = TISCI_RESASG_SUBTYPE_IR_OUTPUT;
    Sciclient_ReqIrq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;
    status  = Sciclient_rmGetResourceRange(&Sciclient_ReqIrq, &Sciclient_ResIrq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(Sciclient_ReqIrq.type, Sciclient_ResIrq.range_start, TISCI_DEV_MCU_R5FSS0_CORE0, &intNum);
        #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
        if(status == CSL_PASS)
        {
            const struct tisci_msg_rm_irq_set_req Sciclient_Req1 =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                            TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                            TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID |
                                            TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = 16415,
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = Sciclient_ResVint.range_start,
                .vint_status_bit_index = 0U
            };
            CSL_REG32_WR(0x285600f8, 12);
            /* Passing these parameters can cover Sciclient_rmIaValidateEvt badargs condition */
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req1, &Sciclient_Resp1, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_EFAIL)
            {
                rmIaValidateEvtTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: 2Arg Test Passed.\n");
            }
            else
            {
                rmIaValidateEvtTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: 2 Arg Test Failed.\n");
            }

            const struct tisci_msg_rm_irq_set_req Sciclient_Req2 =
            {
                .valid_params          = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID |
                                         TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID |
                                         TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID | TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID |
                                         TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID,
                .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
                .src_index             = 0U,
                .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
                .dst_host_irq          = intNum,
                .global_event          = 16414,
                .ia_id                 = TISCI_DEV_MCU_NAVSS0_UDMASS_INTA_0,
                .vint                  = Sciclient_ResVint.range_start,
                .vint_status_bit_index = 0U
            };
            CSL_REG32_WR(0x285600f0, 12);
            /* Passing these parameters can cover Sciclient_rmIaValidateEvt badargs condition */
            status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req2, &Sciclient_Resp2, SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status == CSL_EFAIL)
            {
                rmIaValidateEvtTestStatus += CSL_PASS;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: 22Arg Test Passed.\n");
            }
            else
            {
                rmIaValidateEvtTestStatus += CSL_EFAIL;
                SciApp_printf("Sciclient_rmProgramInterruptRoute: 22 Arg Test Failed.\n");
            }
        }
        else
        {
            rmIaValidateEvtTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
        #endif
    }
    else
    {
        rmIaValidateEvtTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

    return rmIaValidateEvtTestStatus;
}

static int32_t SciclientApp_rmIrqVintMappingOnlyTest()
{
    /* Covers True condition for (!Sciclient_rmIrIsIr(iface->rid)) in  Sciclient_rmIrqCfgIsEventToVintMappingOnly() */
    int32_t status          = CSL_PASS;
    int32_t rmIrqTestStatus = CSL_PASS;
    const struct tisci_msg_rm_irq_set_req Sciclient_Req =
    {
        .valid_params          = TISCI_MSG_VALUE_RM_IA_ID_VALID | TISCI_MSG_VALUE_RM_VINT_VALID | TISCI_MSG_VALUE_RM_GLOBAL_EVENT_VALID |
                                 TISCI_MSG_VALUE_RM_VINT_STATUS_BIT_INDEX_VALID | TISCI_MSG_VALUE_RM_DST_ID_VALID,
        .src_id                = TISCI_DEV_MCU_NAVSS0_MCRC_0,
        .src_index             = 0U,
        .dst_id                = TISCI_DEV_MCU_R5FSS0_CORE0,
        .dst_host_irq          = 0U,
        .global_event          = 10,
        .ia_id                 = TISCI_DEV_MCU_NAVSS0_INTR,
        .vint                  = 0U,
        .vint_status_bit_index = 0U,
        .secondary_host        = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST
    };
    struct tisci_msg_rm_irq_set_resp Sciclient_Resp;

    status = Sciclient_rmProgramInterruptRoute(&Sciclient_Req, &Sciclient_Resp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        SciApp_printf("Sciclient_rmProgramInterruptRoute Negative testcase has passed.\n");
        rmIrqTestStatus += CSL_PASS;
    }
    else
    {
        SciApp_printf("Sciclient_rmProgramInterruptRoute Negative testcase has failed.\n");
        rmIrqTestStatus += CSL_EFAIL;
    }

    return rmIrqTestStatus;
}

#if defined(SOC_J7200) || defined(SOC_J721E)
static int32_t SciclientApp_rmIrqDirectNonEventTest(void)
{
    int32_t  status                = CSL_PASS;
    int32_t  intrRouteStatus       = CSL_PASS;
    uint16_t intNum                = 0;

    struct tisci_msg_rm_irq_set_req rmIrqReq;
    struct tisci_msg_rm_irq_set_resp rmIrqResp;
    struct tisci_msg_rm_get_resource_range_req req;
    struct tisci_msg_rm_get_resource_range_resp res;
    struct tisci_msg_rm_irq_release_req rmIrqReqRel;
    memset(&req, 0, sizeof(req));
    memset(&res, 0, sizeof(res));
    memset(&rmIrqReqRel,0,sizeof(rmIrqReqRel));

    req.type = TISCI_DEV_MAIN2MCU_LVL_INTRTR0;
    req.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;

    status  = Sciclient_rmGetResourceRange(&req,
                                            &res,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_rmGetResourceRange() execution is successful\n");
        status = Sciclient_rmIrqTranslateIrOutput(req.type,
                                                    res.range_start,
                                                    TISCI_DEV_MCU_R5FSS0_CORE0,
                                                    &intNum);
        if(status == CSL_PASS)
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() execution is successful and host interrupt number is %d\n", intNum);

            memset(&rmIrqReq, 0, sizeof(rmIrqReq));
            rmIrqReq.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
            rmIrqReq.valid_params  |= TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
            rmIrqReq.src_id         = TISCI_DEV_UART0;
            rmIrqReq.src_index      = 0;
            rmIrqReq.dst_id         = TISCI_DEV_MCU_R5FSS0_CORE0;
            rmIrqReq.dst_host_irq   = intNum;
            rmIrqReq.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;

            status                  = Sciclient_rmIrqSet(&rmIrqReq,
                                                            &rmIrqResp,
                                                            SCICLIENT_SERVICE_WAIT_FOREVER);
            intrRouteStatus       = status;
            if(status == CSL_PASS)
            {
                SciApp_printf("Sciclient_rmIrqSet() execution is successful\n");
            }
            else
            {
                SciApp_printf("Sciclient_rmIrqSet() has failed\n");
            }
        }
        else
        {
            SciApp_printf("Sciclient_rmIrqTranslateIrOutput() has failed\n");
        }
    }
    else
    {
        SciApp_printf("Sciclient_rmGetResourceRange() has failed\n");
    }

    if(intrRouteStatus == CSL_PASS)
    {
        rmIrqReqRel.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID | TISCI_MSG_VALUE_RM_SECONDARY_HOST_VALID;
        rmIrqReqRel.valid_params  |= TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
        rmIrqReqRel.src_id         = TISCI_DEV_UART0;
        rmIrqReqRel.src_index      = 0;
        rmIrqReqRel.dst_id         = TISCI_DEV_MCU_R5FSS0_CORE0;
        rmIrqReqRel.dst_host_irq   = intNum;
        rmIrqReqRel.secondary_host = TISCI_HOST_ID_MCU_0_R5_0;

        status = Sciclient_rmIrqRelease(&rmIrqReqRel, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            SciApp_printf("Sciclient_rmIrqRelease() execution is successful\n");
        }
        else
        {
            SciApp_printf("Sciclient_rmIrqRelease() has failed\n");
        }
    }

    return status;
}
#endif

static int32_t SciclientApp_rmIrqTest(void)
{
    int32_t  status                       = CSL_PASS;
    int32_t  sciclientInitStatus          = CSL_PASS;
    int32_t  sciclientRmIrqTestStatus     = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has eleven sub-tests:\n");
        sciclientRmIrqTestStatus += SciclientApp_rmIrqVintDeleteNegTest();
        sciclientRmIrqTestStatus += SciclientApp_RmIrOutpRomMappedTest();
        sciclientRmIrqTestStatus += SciclientApp_iaEvtRomMappedTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqValidParamsNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmClearInterruptRouteTest();
        sciclientRmIrqTestStatus += SciclientApp_rmProgramInterruptRouteTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqUnmappedVintRouteDeleteNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqFindRouteNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIaVintGetInfoNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqIsVintRouteSetNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqGetRouteTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqFindRouteTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrGetOutpTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqVintRouteTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqClearRouteNegTest();
        sciclientRmIrqTestStatus += SciclientApp_rmIrqVintMappingOnlyTest();
#if defined(SOC_J7200) || defined(SOC_J721E)
        sciclientRmIrqTestStatus += SciclientApp_rmIrqDirectNonEventTest();
#endif
    }
    else
    {
        sciclientRmIrqTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientRmIrqTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientRmIrqTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientRmIrqTestStatus;
}

static int32_t SciclientApp_procbootNegTest(void)
{
    int32_t  status               = CSL_PASS;
    int32_t  procbootTestStatus   = CSL_PASS;
    uint8_t  invalidProcID        = 0x81U;
    uint32_t ctrlFlagSet          = 1U;
    uint32_t ctrlFlagClr          = 2U;
    uint8_t  numMatchIterations   = 1U;
    uint8_t  delayPerIterations   = 10U;
    uint32_t statusFlagSetAllWait = 1U;
    uint32_t statusFlagSetAnyWait = 2U;
    uint32_t statusFlagClrAllWait = 1U;
    uint32_t statusFlagClrAnyWait = 2U;
    const struct tisci_msg_proc_set_config_req procSetConfigReq  = {0};
    const struct tisci_msg_proc_auth_boot_req procAuthBootReq    = {0};
    struct tisci_msg_proc_get_status_resp procGetStatusResp;

    status = Sciclient_procBootRequestProcessor(invalidProcID, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootRequestProcessor: Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootRequestProcessor: Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootReleaseProcessor(invalidProcID, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: TISCI_MSG_FLAG_AOP Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: TISCI_MSG_FLAG_AOP Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootReleaseProcessor(invalidProcID, TISCI_MSG_FLAG_RESERVED0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootHandoverProcessor(invalidProcID, TISCI_HOST_ID_MCU_0_R5_1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootHandoverProcessor: Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootHandoverProcessor: Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootSetSequenceCtrl(invalidProcID, ctrlFlagSet, ctrlFlagClr, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_AOP Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_AOP Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootSetSequenceCtrl(invalidProcID, ctrlFlagSet, ctrlFlagClr, TISCI_MSG_FLAG_RESERVED0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootSetSequenceCtrl(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, ctrlFlagSet, ctrlFlagClr, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_AOP Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootSetSequenceCtrl: TISCI_MSG_FLAG_AOP Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootSetProcessorCfg(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootSetProcessorCfg: Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootSetProcessorCfg: Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootAuthAndStart(NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootAuthAndStart: Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootAuthAndStart: Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootGetProcessorState(invalidProcID, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootWaitProcessorState(invalidProcID, numMatchIterations,
                                                    delayPerIterations,statusFlagSetAllWait,
                                                    statusFlagSetAnyWait, statusFlagClrAllWait,
                                                    statusFlagClrAnyWait, TISCI_MSG_FLAG_AOP,
                                                    SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootWaitProcessorState: TISCI_MSG_FLAG_AOP Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootWaitProcessorState: TISCI_MSG_FLAG_AOP Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootWaitProcessorState(invalidProcID, numMatchIterations,
                                                    delayPerIterations, statusFlagSetAllWait,
                                                    statusFlagSetAnyWait, statusFlagClrAllWait,
                                                    statusFlagClrAnyWait, TISCI_MSG_FLAG_RESERVED0,
                                                    SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootWaitProcessorState: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootWaitProcessorState: TISCI_MSG_FLAG_RESERVED0 Negative Arg Test FAILED \n");
    }

    status = Sciclient_procBootGetProcessorState(invalidProcID, &procGetStatusResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Negative Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Negative Arg Test Failed.\n");
    }

    status = Sciclient_procBootSetProcessorCfg(&procSetConfigReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootSetProcessorCfg: Negative Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootSetProcessorCfg: Negative Arg Test Failed.\n");
    }

    status = Sciclient_procBootAuthAndStart(&procAuthBootReq, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootAuthAndStart: Negative Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootAuthAndStart: Negative Arg Test Failed.\n");
    }

    return procbootTestStatus;
}

static int32_t SciclientApp_procbootPosTest(void)
{
    int32_t  status                      = CSL_PASS;
    int32_t  procbootTestStatus          = CSL_PASS;
    struct tisci_msg_proc_get_status_resp procGetStatusResp;

    status = Sciclient_procBootWaitProcessorState(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, 1, 1, 0, 3, 0, 0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Positive Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Positive Arg Test Failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootRequestProcessor: Positive Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootRequestProcessor: Positive Arg Test Failed.\n");
    }

    status = Sciclient_procBootGetProcessorState(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, &procGetStatusResp, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Positive Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootGetProcessorState: Positive Arg Test Failed.\n");
    }

    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: Positive Arg Test Passed.\n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: Positive Arg Test Failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_R5FSS0_CORE0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        status = Sciclient_procBootGetProcessorState(SCICLIENT_PROC_ID_R5FSS0_CORE0, &procGetStatusResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            SciApp_printf ("Sciclient_procBootGetProcessorState: PASSED \n");
            const struct tisci_msg_proc_set_config_req procSetConfigReq =
            {
                .processor_id         = procGetStatusResp.processor_id,
                .bootvector_lo        = procGetStatusResp.bootvector_lo,
                .bootvector_hi        = procGetStatusResp.bootvector_hi,
                .config_flags_1_clear = 0,
                .config_flags_1_set   = 0
            };
            status = Sciclient_procBootSetProcessorCfg(&procSetConfigReq, SCICLIENT_SERVICE_WAIT_FOREVER);
            if(status == CSL_PASS)
            {
                procbootTestStatus += CSL_PASS;
                SciApp_printf ("Sciclient_procBootSetProcessorCfg: Positive Arg Test PASSED \n");
            }
            else
            {
                procbootTestStatus += CSL_EFAIL;
                SciApp_printf ("Sciclient_procBootSetProcessorCfg: Positive Arg Test FAILED \n");
            }
        }
        else
        {
            procbootTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_procBootGetProcessorState: FAILED \n");
        }

        status = Sciclient_procBootSetSequenceCtrl(SCICLIENT_PROC_ID_R5FSS0_CORE0, TISCI_MSG_VAL_PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, 0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            procbootTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_procBootSetSequenceCtrl: Positive Arg Test PASSED \n");
        }
        else
        {
            procbootTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_procBootSetSequenceCtrl: Positive Arg Test FAILED \n");
        }

        status = Sciclient_procBootWaitProcessorState(SCICLIENT_PROC_ID_R5FSS0_CORE0, 1, 1, 0, 3, 0, 0, 0, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            procbootTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_procBootWaitProcessorState: Positive Arg Test PASSED \n");
        }
        else
        {
            procbootTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_procBootWaitProcessorState: Positive Arg Test FAILED \n");
        }

        status = Sciclient_procBootHandoverProcessor(SCICLIENT_PROC_ID_R5FSS0_CORE0, TISCI_HOST_ID_MAIN_0_R5_1, SCICLIENT_SERVICE_WAIT_FOREVER);
        if(status == CSL_PASS)
        {
            procbootTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_procBootHandoverProcessor: Positive Arg Test PASSED \n");
        }
        else
        {
            procbootTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_procBootHandoverProcessor: Positive Arg Test FAILED \n");
        }
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootHandoverProcessor: Positive Arg Test FAILED \n");
    }

    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, 0U, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        procbootTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: Positive Arg Test PASSED \n");
    }
    else
    {
        procbootTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_procBootReleaseProcessor: Positive Arg Test FAILED \n");
    }

    return procbootTestStatus;
}

static int32_t SciclientApp_procbootTest(void)
{
    int32_t  status                       = CSL_PASS;
    int32_t  sciclientInitStatus          = CSL_PASS;
    int32_t  sciclientProcbootTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has two sub-tests:\n");
        sciclientProcbootTestStatus += SciclientApp_procbootNegTest();
        sciclientProcbootTestStatus += SciclientApp_procbootPosTest();
    }
    else
    {
        sciclientProcbootTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientProcbootTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientProcbootTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientProcbootTestStatus;
}

#if defined(BUILD_MCU1_0) && (defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2))
static int32_t SciclientApp_lpmMessagePosTest(void)
{
    int32_t   status                   = CSL_PASS;
    int32_t   lpmMessagePosTestStatus  = CSL_PASS;

    /* GET_SUSPEND_INITIATOR Positive test */
    struct tisci_msg_get_suspend_initiator_req getReq = {};
    struct tisci_msg_get_suspend_initiator_resp getResp = {};

    Sciclient_RespPrm_t respPrm = {
        .pRespPayload   = (uint8_t *) &getResp,
        .respPayloadSize = (uint32_t) sizeof(getResp),
    };

    Sciclient_ReqPrm_t reqPrm = {
        .messageType    = (uint16_t) TISCI_MSG_GET_SUSPEND_INITIATOR,
        .flags          = (uint32_t) TISCI_MSG_FLAG_ACK,
        .pReqPayload    = (const uint8_t *) &getReq,
        .reqPayloadSize = (uint32_t) sizeof (getReq),
        .timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER
    };

    status = Sciclient_service(&reqPrm, &respPrm);

    if(status == CSL_PASS)
    {
        lpmMessagePosTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_getSuspendMaster Test Passed\n");
    }
    else
    {
        lpmMessagePosTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_getSuspendMaster Test Failed\n");
    }

    return lpmMessagePosTestStatus;
}

static int32_t SciclientApp_lpmMessageNegTest(void)
{
    int32_t   status                    = CSL_PASS;
    int32_t   lpmMessageNegTestStatus   = CSL_PASS;

    /* GET_SUSPEND_INITIATOR Negative test */
    status = Sciclient_getSuspendMaster(NULL);
    if(status == CSL_EFAIL)
    {
        lpmMessageNegTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_getSuspendMaster NULL Negative Test Passed\n");
    }
    else
    {
        lpmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_getSuspendMaster NULL Negative Test Failed\n");
    }

    return lpmMessageNegTestStatus;
}
#endif

#if !defined(BUILD_MCU1_1)
static int32_t SciclientApp_pmMessagePosTest(void)
{
    int32_t   status               = CSL_PASS;
    int32_t   pmMessageTestStatus  = CSL_PASS;
    uint64_t  reqFreq              = 0UL;
    uint64_t  respFreq             = 0UL;
    uint32_t  clockStatus          = 1U;
    uint32_t  parentStatus         = 0U;
    uint32_t  numParents           = 0U;
    uint32_t  moduleState          = 0U;
    uint32_t  resetState           = 0U;
    uint32_t  contextLossState     = 0U;
    uint32_t  reqFlag              = 0U;

    status = Sciclient_pmGetModuleClkFreq(TISCI_DEV_UART1,
                                          TISCI_DEV_UART1_FCLK_CLK,
                                          &respFreq,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);

    reqFreq = respFreq;
    status = Sciclient_pmQueryModuleClkFreq(TISCI_DEV_UART1,
                                            TISCI_DEV_UART1_FCLK_CLK,
                                            reqFreq,
                                            &respFreq,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmQueryModuleClkFreq Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmQueryModuleClkFreq Test Failed.\n");
    }

    status = Sciclient_pmSetModuleClkFreq(TISCI_DEV_UART1,
                                          TISCI_DEV_UART1_FCLK_CLK,
                                          respFreq,
                                          TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmSetModuleClkFreq Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmSetModuleClkFreq Test Failed.\n");
    }

    status = Sciclient_pmModuleGetClkStatus(TISCI_DEV_UART1,
                                            TISCI_DEV_UART1_FCLK_CLK,
                                            &clockStatus,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmModuleGetClkStatus Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmModuleGetClkStatus Test Failed.\n");
    }

    status = Sciclient_pmModuleClkRequest(TISCI_DEV_UART1,
                                          TISCI_DEV_UART1_FCLK_CLK,
                                          TISCI_MSG_VALUE_CLOCK_HW_STATE_READY,
                                          0U,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmModuleClkRequest Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmModuleClkRequest Test Failed.\n");
    }

    status = Sciclient_pmSetModuleClkParent(TISCI_DEV_MCSPI3,
                                            TISCI_DEV_MCSPI3_IO_CLKSPII_CLK,
                                            TISCI_DEV_MCSPI3_IO_CLKSPII_CLK_PARENT_SPI_MAIN_3_IO_CLKSPIO_CLK,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmSetModuleClkParent Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmSetModuleClkParent Test Failed.\n");
    }

    status = Sciclient_pmGetModuleClkParent(TISCI_DEV_MCSPI3,
                                            TISCI_DEV_MCSPI3_IO_CLKSPII_CLK,
                                            &parentStatus,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if ((status == CSL_PASS) && (parentStatus == TISCI_DEV_MCSPI3_IO_CLKSPII_CLK_PARENT_SPI_MAIN_3_IO_CLKSPIO_CLK))
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmGetModuleClkParent Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmGetModuleClkParent Test Failed.\n");
    }

    status = Sciclient_pmGetModuleClkNumParent(TISCI_DEV_MCSPI3,
                                               TISCI_DEV_MCSPI3_IO_CLKSPII_CLK,
                                               &numParents,
                                               SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmGetModuleClkNumParent Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmGetModuleClkNumParent Test Failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE0,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                            0U,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmMessageTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_pmSetModuleState: SCICLIENT_DEV_MCU_R5FSS0_CORE0 Test Passed.\n");
        }
        else
        {
            pmMessageTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_pmSetModuleState: SCICLIENT_DEV_MCU_R5FSS0_CORE0 Test Failed.\n");
        }
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_0 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_0 has failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                            0U,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmMessageTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_pmSetModuleState: SCICLIENT_DEV_MCU_R5FSS0_CORE1 Test Passed.\n");
        }
        else
        {
            pmMessageTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_pmSetModuleState: SCICLIENT_DEV_MCU_R5FSS0_CORE1 Test Failed.\n");
        }
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    status = Sciclient_pmSetModuleState(TISCI_DEV_MCU_UART0,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        0U,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmSetModuleState: TISCI_DEV_MCU_UART0 Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmSetModuleState: TISCI_DEV_MCU_UART0 Test Failed.\n");
    }

    status = Sciclient_pmSetModuleState(TISCI_DEV_BOARD0,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        TISCI_MSG_FLAG_AOP,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("TISCI_DEV_BOARD0 TISCI_MSG_VALUE_DEVICE_SW_STATE_ON Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("TISCI_DEV_BOARD0 TISCI_MSG_VALUE_DEVICE_SW_STATE_ON Test Failed.\n");
    }

    status = Sciclient_pmGetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE0,
                                        &moduleState,
                                        &resetState,
                                        &contextLossState,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf("SCICLIENT_DEV_MCU_R5FSS0_CORE0 States: \n");
        SciApp_printf("ModuleState: %d\n", moduleState);
        SciApp_printf("ResetState: %d\n", resetState);
        SciApp_printf("ContextLossState: %d\n", contextLossState);
        SciApp_printf("Sciclient_pmGetModuleState Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmGetModuleState Test Failed.\n");
    }

    /* Reset TISCI_DEV_LED0 module */
    status = Sciclient_pmSetModuleRst(TISCI_DEV_LED0,
                                      1U,
                                      SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleRst Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleRst Test Failed.\n");
    }

    /* SetModuleRst_flags for TISCI_DEV_LED0 */
    reqFlag = TISCI_MSG_FLAG_ACK;
    status = Sciclient_pmSetModuleRst_flags(TISCI_DEV_LED0, 0U, reqFlag, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleRst_flags Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleRst_flags Test Failed.\n");
    }

    /* Check whether TISCI_DEV_UART1 module is valid or not */
    status = Sciclient_pmIsModuleValid(TISCI_DEV_UART1);
    if (status == CSL_PASS)
    {
        pmMessageTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmIsModuleValid Test Passed.\n");
    }
    else
    {
        pmMessageTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmIsModuleValid Test Failed.\n");
    }

    return pmMessageTestStatus;
}

static int32_t SciclientApp_pmMessageNegTest(void)
{
    int32_t  status                 = CSL_PASS;
    int32_t  pmMessageNegTestStatus = CSL_PASS;
    uint64_t reqFreq                = 164UL;
    uint64_t respFreq               = 0UL;
    uint32_t clockStatus            = 1U;
    uint32_t parentStatus           = 0U;
    uint32_t numParents             = 0U;
    uint64_t freq                   = 0UL;
    uint32_t moduleState            = 0U;
    uint32_t resetState             = 0U;
    uint32_t contextLossState       = 0U;
    uint32_t invalidModuleId        = 440U;

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE0,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        1U,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            pmMessageNegTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetModuleState Negative Test Passed.\n");
        }
        else
        {
            pmMessageNegTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetModuleState Negative Test Failed.\n");
        }
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_0 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_0 has failed.\n");
    }

    status = Sciclient_pmGetModuleState(invalidModuleId,
                                        &moduleState,
                                        &resetState,
                                        &contextLossState,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmGetModuleState Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmGetModuleState Negative Test Failed.\n");
    }

    status = Sciclient_pmSetModuleRst(invalidModuleId,
                                      1U,
                                      SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleRst Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleRst Negative Test Failed.\n");
    }

    status = Sciclient_pmSetModuleRst_flags(invalidModuleId, 0U, 0U, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleRst_flags Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleRst_flags Negative Test Failed.\n");
    }

    status = Sciclient_pmModuleClkRequest(invalidModuleId,
                                          256U,
                                          TISCI_MSG_VALUE_CLOCK_HW_STATE_READY,
                                          0U,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmModuleClkRequest Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmModuleClkRequest Negative Test Failed.\n");
    }

    status = Sciclient_pmModuleGetClkStatus(invalidModuleId,
                                            256U,
                                            &clockStatus,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmModuleGetClkStatus Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmModuleGetClkStatus Negative Test Failed.\n");
    }

    /* Reset domain group DOMGRP_02 */
    status = Sciclient_pmDomainReset(DOMGRP_02, SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmDomainReset Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmDomainReset Negative Test Failed.\n");
    }

    status = Sciclient_pmSetModuleClkParent(invalidModuleId,
                                            256U,
                                            256U,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleClkParent Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleClkParent Negative Test Failed.\n");
    }

    status = Sciclient_pmGetModuleClkParent(invalidModuleId,
                                            256U,
                                            &parentStatus,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmGetModuleClkParent Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmGetModuleClkParent Negative Test Failed.\n");
    }

    status = Sciclient_pmGetModuleClkNumParent(invalidModuleId,
                                               256U,
                                               &numParents,
                                               SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmGetModuleClkNumParent Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmGetModuleClkNumParent Negative Test Failed.\n");
    }

    status = Sciclient_pmQueryModuleClkFreq(invalidModuleId,
                                            256U,
                                            reqFreq,
                                            &respFreq,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmQueryModuleClkFreq Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmQueryModuleClkFreq Negative Test Failed.\n");
    }

    status = Sciclient_pmSetModuleClkFreq(invalidModuleId,
                                          256U,
                                          reqFreq,
                                          TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmSetModuleClkFreq Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleClkFreq Negative Test Failed.\n");
    }

    status = Sciclient_pmGetModuleClkFreq(invalidModuleId,
                                          256U,
                                          &freq,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmGetModuleClkFreq Negative Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmGetModuleClkFreq Negative Test Failed.\n");
    }

    status = Sciclient_pmEnableWdt(SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmEnableWdt Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmEnableWdt Test Failed.\n");
    }

    status = Sciclient_pmDisableWakeup(SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_EFAIL)
    {
        pmMessageNegTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_pmDisableWakeup Test Passed.\n");
    }
    else
    {
        pmMessageNegTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmDisableWakeup Test Failed.\n");
    }

    return pmMessageNegTestStatus;
}

static int32_t SciclientApp_pmSetMsgProxyPosTest(void)
{
    int32_t  status                     = CSL_PASS;
    int32_t  pmSetMsgProxyTestStatus    = CSL_PASS;
    uint32_t reqFlag                    = 0U;
    uint32_t invalidState               = 3U;

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        /* Turning OFF CORE1 */
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF,
                                            reqFlag,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmSetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetMsgProxy TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF Test Passed.\n");
        }
        else
        {
            pmSetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetMsgProxy TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF Test Failed.\n");
        }
    }
    else
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
         /* Passing invalid state and reqFlag */
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                        invalidState,
                                        reqFlag,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmSetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetMsgProxy default case Test Passed.\n");
        }
        else
        {
            pmSetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetMsgProxy default case Test Failed.\n");
        }
    }
    else
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
         /* If CORE1 gets OFF with above tests, Turn it ON again */
        status = Sciclient_pmSetModuleState(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                            reqFlag,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmSetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("SCICLIENT_DEV_MCU_R5FSS0_CORE1 ON.\n");
        }
        else
        {
            pmSetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("SCICLIENT_DEV_MCU_R5FSS0_CORE1 ON Failed.\n");
        }
    }
    else
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    return pmSetMsgProxyTestStatus;
}

static int32_t SciclientApp_pmSetCpuResetMsgProxyTest(void)
{
    int32_t  status                             = CSL_PASS;
    int32_t  pmSetCpuResetMsgProxyTestStatus    = CSL_PASS;
    uint32_t resetBit                           = 0U;
    #if defined(BUILD_MCU1_0)
    uint32_t message[20]                     = {0};
    struct tisci_msg_set_device_resets_req request = {0};
    Sciclient_ReqPrm_t reqParam = {
        .messageType    = (uint16_t) TISCI_MSG_SET_DEVICE_RESETS,
        .flags          = (uint32_t) TISCI_MSG_FLAG_ACK,
        .pReqPayload    = (const uint8_t *) SCICLIENT_DEV_MCU_R5FSS0_CORE1,
        .reqPayloadSize = (uint32_t) sizeof (request),
        .timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER
    };
    #endif
    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    /* Taking CORE1 out of reset */
    if(status == CSL_PASS)
    {
        status = Sciclient_pmSetModuleRst(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                          resetBit,
                                          SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Test-1 Passed.\n");
        }
        else
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Test-1 Failed.\n");
        }
    }
    else
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        /* Taking CORE1 out of reset */
        /* Putting CORE1 in reset */
        resetBit = 1U;
        status = Sciclient_pmSetModuleRst(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                        resetBit,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Test-2 Passed.\n");
        }
        else
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Test-2 Failed.\n");
        }
    }
    else
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status == CSL_PASS)
    {
        /* Setting up invalid resetBit */
        resetBit = 2U;
        status = Sciclient_pmSetModuleRst(SCICLIENT_DEV_MCU_R5FSS0_CORE1,
                                        resetBit,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Negative Test Passed.\n");
        }
        else
        {
            pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetCpuResetMsgProxy Negative Test Failed.\n");
        }
    }
    else
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_1 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE1, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_1 has failed.\n");
    }

    #if defined(BUILD_MCU1_0)
    /* Passing TISCI_MSG_SET_DEVICE_RESETS messageType */
    resetBit = 0U;
    memcpy(message, &reqParam, sizeof(reqParam));
    message[3] = resetBit;
    status = Sciclient_ProcessPmMessage(reqParam.flags, message);
    if(status == CSL_PASS)
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_pmSetCpuResetMsgProxy test Passed\n");
    }
    else
    {
        pmSetCpuResetMsgProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_pmSetCpuResetMsgProxy test Failed\n");
    }
    #endif

    return pmSetCpuResetMsgProxyTestStatus;
}

static int32_t SciclientApp_processPmMessageTest(void)
{
    int32_t  status                      = CSL_PASS;
    int32_t  processPmMessageTestStatus  = CSL_PASS;
    uint32_t resetBit                    = 0U;
    uint32_t reqFlag                     = 0U;
    uint32_t invalidState                = 3U;

    /* Passing invalid state */
    status = Sciclient_pmSetModuleState(TISCI_DEV_BOARD0,
                                        invalidState,
                                        reqFlag,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        processPmMessageTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_ProcessPmMessage TISCI_MSG_SET_DEVICE Test Passed\n");
    }
    else
    {
        processPmMessageTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_ProcessPmMessage TISCI_MSG_SET_DEVICE Test Failed\n");
    }

    status = Sciclient_procBootRequestProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, SCICLIENT_SERVICE_WAIT_FOREVER);
    /* Passing invalid resetBit */
    if(status == CSL_PASS)
    {
        resetBit = 2U;
        status = Sciclient_pmSetModuleRst(SCICLIENT_DEV_MCU_R5FSS0_CORE0,
                                        resetBit,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            processPmMessageTestStatus += CSL_PASS;
            SciApp_printf ("sciclient_processPMmessage Negative Test Passed.\n");
        }
        else
        {
            processPmMessageTestStatus += CSL_EFAIL;
            SciApp_printf ("sciclient_processPMmessage Negative Test Failed.\n");
        }
    }
    else
    {
        processPmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor access request for MCU1_0 has failed.\n");
    }
    status = Sciclient_procBootReleaseProcessor(SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER);
    if(status != CSL_PASS)
    {
        processPmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Processor release request for MCU1_0 has failed.\n");
    }

    /* Incrementing coreRefCnt twice so that it will not shutdown even if
       TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF called */
    status = Sciclient_pmSetModuleState(TISCI_DEV_BOARD0,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        reqFlag,
                                        SCICLIENT_SERVICE_WAIT_FOREVER);
    if (status == CSL_PASS)
    {
        processPmMessageTestStatus += CSL_PASS;
        SciApp_printf ("coreRefCnt Incremented to 1\n");
        status = Sciclient_pmSetModuleState(TISCI_DEV_BOARD0,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                            reqFlag,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            processPmMessageTestStatus += CSL_PASS;
            SciApp_printf ("coreRefCnt Incremented to 2\n");
        }
        else
        {
            processPmMessageTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_pmSetModuleState Test Failed.\n");
        }

        status = Sciclient_pmSetModuleState(TISCI_DEV_BOARD0,
                                            TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF,
                                            reqFlag,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            processPmMessageTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_ProcessPmMessage TISCI_MSG_SET_DEVICE Test Passed\n");
        }
        else
        {
            processPmMessageTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_ProcessPmMessage TISCI_MSG_SET_DEVICE Test Failed.\n");
        }
    }
    else
    {
        processPmMessageTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_pmSetModuleState Test Failed.\n");
    }

    return processPmMessageTestStatus;
}

#if defined(BUILD_MCU1_0)
static int32_t SciclientApp_processRmMessageTest(void)
{
    int32_t  status                      = CSL_PASS;
    int32_t  processRmMessageTestStatus  = CSL_PASS;
    uint32_t message[20]                 = {0};
    struct tisci_msg_board_config_rm_req request = {
        .tisci_boardcfg_rmp_low   = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
        .tisci_boardcfg_rmp_high  = (uint32_t) 0x0U,
        .tisci_boardcfg_rm_size   = (uint16_t) SCICLIENT_BOARDCFG_RM_SIZE_IN_BYTES,
        .tisci_boardcfg_rm_devgrp = (uint8_t) DEVGRP_ALL
    };
    Sciclient_ReqPrm_t reqParam = {
        .messageType    = (uint16_t) TISCI_MSG_BOARD_CONFIG_RM,
        .flags          = (uint32_t) TISCI_MSG_FLAG_ACK,
        .pReqPayload    = (const uint8_t *) &request,
        .reqPayloadSize = (uint32_t) sizeof (request),
        .timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER
    };

    /* Passing TISCI_MSG_RM_UDMAP_FLOW_DELEGATE messageType */
    reqParam.messageType = TISCI_MSG_RM_UDMAP_FLOW_DELEGATE;
    memcpy(message, &reqParam, sizeof(reqParam));
    status = Sciclient_ProcessRmMessage(message);
    if(status == CSL_PASS)
    {
        processRmMessageTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_ProcessRmMessage TISCI_MSG_RM_UDMAP_FLOW_DELEGATE test Passed\n");
    }
    else
    {
        processRmMessageTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_ProcessRmMessage TISCI_MSG_RM_UDMAP_FLOW_DELEGATE test Failed\n");
    }

    return processRmMessageTestStatus;
}
#endif

static int32_t SciclientApp_pmTest(void)
{
    int32_t  status                 = CSL_PASS;
    int32_t  sciclientInitStatus    = CSL_PASS;
    int32_t  sciclientPmTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has six sub-tests:\n");
        sciclientPmTestStatus += SciclientApp_pmMessagePosTest();
        sciclientPmTestStatus += SciclientApp_pmMessageNegTest();
        sciclientPmTestStatus += SciclientApp_pmSetMsgProxyPosTest();
        sciclientPmTestStatus += SciclientApp_pmSetCpuResetMsgProxyTest();
        sciclientPmTestStatus += SciclientApp_processPmMessageTest();
        #if defined(BUILD_MCU1_0) && (defined(SOC_J7200) || defined(SOC_J784S4) || defined(SOC_J742S2))
        sciclientPmTestStatus += SciclientApp_lpmMessagePosTest();
        sciclientPmTestStatus += SciclientApp_lpmMessageNegTest();
        #endif
        #if defined(BUILD_MCU1_0)
        sciclientPmTestStatus += SciclientApp_processRmMessageTest();
        #endif
    }
    else
    {
        sciclientPmTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientPmTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientPmTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientPmTestStatus;
}
#endif

#if defined(BUILD_MCU1_0)
static int32_t SciclientApp_boardcfgTest(void)
{
    int32_t status                = CSL_PASS;
    int32_t sciclientInitStatus   = CSL_PASS;
    int32_t boardCfgTestStatus    = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_POLLED,
        NULL,
        1U,
        0U,
        UTRUE
    };
    Sciclient_BoardCfgPrms_t pmBoardCfgParams  = {0};
    pmBoardCfgParams.devGrp = DEVGRP_04;
    Sciclient_BoardCfgPrms_t boardCfgParams    = {0};
    Sciclient_BoardCfgPrms_t secBoardCfgParams = {0};

    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if (CSL_PASS == status)
    {
        SciApp_printf ("Sciclient_init Passed.\n");
        status = Sciclient_boardCfgPm(&pmBoardCfgParams);
        if(status != CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgPm NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgPm NULL Arg Test FAILED \n");
        }

        status = Sciclient_getDefaultBoardCfgInfo(NULL);
        if(status != CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_getDefaultBoardCfgInfo NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_getDefaultBoardCfgInfo NULL Arg Test FAILED \n");
        }

        /* Passing NULL parameter to cover branch in Sciclient_boardCfg */
        status = Sciclient_boardCfg(NULL);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfg NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfg NULL Arg Test FAILED \n");
        }

        /* Passing NULL parameter to cover branch in Sciclient_boardCfgRm */
        status = Sciclient_boardCfgRm(NULL);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgRm NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgRm NULL Arg Test FAILED \n");
        }

        /* Passing NULL parameter to cover branch in  Sciclient_boardCfgSec */
        status = Sciclient_boardCfgSec(NULL);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgSec NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgSec NULL Arg Test FAILED \n");
        }

        /* Passing NULL parameter to cover branch in Sciclient_boardCfgPm */
        status = Sciclient_boardCfgPm(NULL);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgPm NULL Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgPm NULL Arg Test FAILED \n");
        }

        /* Passing valid parameters to cover Sciclient_boardCfg */
        status = Sciclient_boardCfg(&boardCfgParams);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfg Positive Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfg Positive Arg Test FAILED \n");
        }

        /* Passing valid parameters to cover Sciclient_boardCfgSec */
        status = Sciclient_boardCfgSec(&secBoardCfgParams);
        if (status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_boardCfgSec Positive Arg Test PASSED \n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_boardCfgSec Positive Arg Test FAILED \n");
        }
    }
    else
    {
        boardCfgTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_init Failed.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            boardCfgTestStatus += CSL_PASS;
            SciApp_printf ("Sciclient_deinit Passed.\n");
        }
        else
        {
            boardCfgTestStatus += CSL_EFAIL;
            SciApp_printf ("Sciclient_deinit Failed.\n");
        }
    }

    return boardCfgTestStatus;
}
#endif

static int32_t SciclientApp_dkekTest(void)
{
    int32_t status                = CSL_PASS;
    int32_t sciclientInitStatus   = CSL_PASS;
    int32_t dkekTestStatus        = CSL_PASS;
#if defined(BUILD_MCU1_0)
    struct tisci_msg_sa2ul_set_dkek_req setDkekReq;
    struct tisci_msg_sa2ul_set_dkek_resp setDkekResp;
    struct tisci_msg_sa2ul_get_dkek_req getDkekReq;
    struct tisci_msg_sa2ul_get_dkek_resp getDkekResp;
    struct tisci_msg_sa2ul_release_dkek_req releaseDkekReq;
    struct tisci_msg_sa2ul_release_dkek_resp releaseDkekResp;
    const uint8_t label[]         = {0x21U, 0x42U};
	const uint8_t context[]       = {0x55U, 0x66U, 0x77U, 0x88U};
#endif

    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

     while (gSciclientHandle.initCount != 0)
     {
         status = Sciclient_deinit();
     }
     status = Sciclient_init(&config);
     sciclientInitStatus = status;

     if(status == CSL_PASS)
     {
        SciApp_printf("Sciclient_init PASSED.\n");
        status = Sciclient_setDKEK(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_setDKEK: Negative Arg Test Passed.\n");
        }
        else
        {
           dkekTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_setDKEK: Negative Arg Test Failed.\n");
        }

        status = Sciclient_releaseDKEK(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_releaseDKEK: Negative Arg Test Passed.\n");
        }
        else
        {
           dkekTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_releaseDKEK: Negative Arg Test Failed.\n");
        }

        status = Sciclient_getDKEK(NULL, NULL, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_EFAIL)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_getDKEK: Negative Arg Test Passed.\n");
        }
        else
        {
           dkekTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_getDKEK: Negative Arg Test Failed.\n");
        }
#if defined(BUILD_MCU1_0)
        /* Passing valid setDkekReq parameters to cover Sciclient_setDKEK */
        setDkekReq.sa2ul_instance  = 0;
        memcpy(&setDkekReq.kdf_label_and_context[0], label, sizeof(label));
        setDkekReq.kdf_label_len   = sizeof(label);
        memcpy(&setDkekReq.kdf_label_and_context[setDkekReq.kdf_label_len], context, sizeof(context));
        setDkekReq.kdf_context_len = sizeof(context);
        status = Sciclient_setDKEK(&setDkekReq, &setDkekResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_setDKEK: Positive Arg Test Passed.\n");
        }
        else
        {
            dkekTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_setDKEK: Positive Arg Test Failed.\n");
        }

        /* Passing valid getDkekReq parameters to cover Sciclient_getDKEK */
        getDkekReq.sa2ul_instance  = 0;
        memcpy(&getDkekReq.kdf_label_and_context[0], label, sizeof(label));
        getDkekReq.kdf_label_len   = sizeof(label);
        memcpy(&getDkekReq.kdf_label_and_context[getDkekReq.kdf_label_len], context, sizeof(context));
        getDkekReq.kdf_context_len = sizeof(context);
        status = Sciclient_getDKEK(&getDkekReq, &getDkekResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_getDKEK: Positive Arg Test Passed.\n");
        }
        else
        {
            dkekTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_getDKEK: Positive Arg Test Failed.\n");
        }

        /* Passing valid releaseDkekReq parameters to cover Sciclient_releaseDKEK */
        releaseDkekReq.sa2ul_instance = 0;
        status = Sciclient_releaseDKEK(&releaseDkekReq, &releaseDkekResp, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_releaseDKEK: Positive Arg Test Passed.\n");
        }
        else
        {
            dkekTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_releaseDKEK: Positive Arg Test Failed.\n");
        }
#endif
    }
    else
    {
        dkekTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            dkekTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_deinit PASSED.\n");
        }
        else
        {
            dkekTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_deinit FAILED.\n");
        }
    }

    return dkekTestStatus;
}

static int32_t SciclientApp_romTest(void)
{
    int32_t status                 = CSL_PASS;
    int32_t sciclientInitStatus    = CSL_PASS;
    int32_t romTestStatus          = CSL_PASS;
    #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
    uint32_t txThread              = SCICLIENT_ROM_R5_TX_NORMAL_THREAD;
    uint32_t rxThread              = SCICLIENT_ROM_R5_RX_NORMAL_THREAD;
    uint32_t pSciclient_firmware;
    uint32_t txThreadVal,rxThreadVal;
    #endif

    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

     while (gSciclientHandle.initCount != 0)
     {
         status = Sciclient_deinit();
     }
     status = Sciclient_init(&config);
     sciclientInitStatus = status;

     if(status == CSL_PASS)
     {
        SciApp_printf("Sciclient_init PASSED.\n");
        /* Passing a NULL parameter */
        status = Sciclient_loadFirmware(NULL);
        if (status == CSL_EFAIL )
        {
            romTestStatus += CSL_PASS;
            SciApp_printf("loadFirmwareTestStatus: Negative Arg Test Passed.\n");
        }
        else
        {
           romTestStatus += CSL_EFAIL;
           SciApp_printf("loadFirmwareTestStatus: Negative Arg Test Failed.\n");
        }
        #if defined(SOC_J784S4) && defined(BUILD_MCU1_0)
        /* Updating the threadStatusReg value to fail Sciclient_verifyThread() in Sciclient_loadFirmware() */
        txThreadVal = Sciclient_threadStatusReg(txThread);
        HW_WR_REG32(txThreadVal, 0x80000000U);
        status = Sciclient_loadFirmware(&pSciclient_firmware);
        if (status == CSL_EFAIL )
        {
            romTestStatus += CSL_PASS;
            SciApp_printf("loadFirmwareTestStatus: Negative Arg Test Passed.\n");
        }
        else
        {
           romTestStatus += CSL_EFAIL;
           SciApp_printf("loadFirmwareTestStatus: Negative Arg Test Failed.\n");
        }

        /* Updating the threadStatusReg value to fail Sciclient_verifyThread() in Sciclient_bootNotification() */
        rxThreadVal=Sciclient_threadStatusReg(rxThread);
        HW_WR_REG32(rxThreadVal, 0x80000000U);
        status = Sciclient_bootNotification();
        if (status == CSL_EFAIL)
        {
            romTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_bootNotification: Negative Arg Test Passed.\n");
        }
        else
        {
           romTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_bootNotification: Negative Arg Test Failed.\n");
        }
        #endif
    }
    else
    {
        romTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }
    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            romTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_deinit PASSED.\n");
        }
        else
        {
            romTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_deinit FAILED.\n");
        }
    }

    return romTestStatus;
}

static int32_t SciclientApp_secureproxyTest(void)
{
    int32_t status                      = CSL_PASS;
    int32_t sciclientInitStatus         = CSL_PASS;
    int32_t secureProxyTestStatus       = CSL_PASS;
    int32_t contextId                   = SCICLIENT_CONTEXT_NONSEC;
    uint32_t thread                     = gSciclientMap[contextId].notificationThreadId;
    uint32_t threadAddr                 = 0U;
    uint32_t regVal                     = 0U;
    uint32_t gSciclient_maxMsgSizeBytes = 0U;
    uint32_t timeout                    = 2U;
    Sciclient_ConfigPrms_t config       =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
        NULL,
        0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");

        /* Setting up MSB in thread value to fail Sciclient_verifyThread() */
        gSciclient_maxMsgSizeBytes = CSL_secProxyGetMaxMsgSize(pSciclient_secProxyCfg) - CSL_SEC_PROXY_RSVD_MSG_BYTES;
        threadAddr = CSL_secProxyGetDataAddr(pSciclient_secProxyCfg, thread, 0U) + ((uintptr_t) gSciclient_maxMsgSizeBytes  - (uintptr_t) 4U);
        regVal = HW_RD_REG32(threadAddr);
        regVal = regVal | 0x8000000;
        HW_WR_REG32(threadAddr, regVal);
        status = Sciclient_verifyThread(thread);
        if (status == CSL_EFAIL)
        {
            secureProxyTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_verifyThread: Negative Arg Test Passed.\n");
        }
        else
        {
            secureProxyTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_verifyThread: Negative Arg Test Failed.\n");
        }

        /* Less timeout is given */
        status = Sciclient_waitThread(thread, timeout);
        if (status == CSL_ETIMEOUT)
        {
            secureProxyTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_waitThread: Negative Arg Test Passed.\n");
        }
        else
        {
            secureProxyTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_waitThread: Negative Arg Test Failed.\n");
        }

        /* 0 timeout is given */
        status = Sciclient_waitThread(thread, SCICLIENT_SERVICE_NO_WAIT);
        if (status == CSL_ETIMEOUT)
        {
            secureProxyTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_waitThread: Negative Arg Test Passed.\n");
        }
        else
        {
            secureProxyTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_waitThread: Negative Arg Test Failed.\n");
        }

        uint8_t pSecHeaderVar=0;
        /* Few NULL, 0U Arguments and appropriate gSciclient_maxMsgSizeBytes argument is passed
            to Sciclient_sendMessage */
        gSciclient_maxMsgSizeBytes = 8U;
        Sciclient_sendMessage(thread, NULL, 0U, NULL, NULL, 0U, gSciclient_maxMsgSizeBytes);
        Sciclient_sendMessage(thread,&pSecHeaderVar,1,NULL, NULL, 0U, gSciclient_maxMsgSizeBytes);
    }
    else
    {
        secureProxyTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
        status = Sciclient_deinit();
        if(status == CSL_PASS)
        {
            secureProxyTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_deinit PASSED.\n");
        }
        else
        {
            secureProxyTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_deinit FAILED.\n");
        }
    }

    return secureProxyTestStatus;
}

#if defined(BUILD_MCU1_0)
static int32_t SciclientApp_directNegTest1(void)
{
    int32_t status                = CSL_PASS;
    int32_t directTestStatus      = CSL_PASS;
    const uint32_t reqFlags       = 0U;
    struct tisci_header *negHdr;

    status = Sciclient_boardCfgPrepHeader(NULL, NULL, NULL, NULL);
    if (status == CSL_EBADARGS)
    {
        directTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        directTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Failed.\n");
    }

    status = Sciclient_boardCfgParseHeader(NULL, NULL, NULL);
    if (status == CSL_EBADARGS)
    {
        directTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        directTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Failed.\n");
    }

    status = Sciclient_ProcessPmMessage(reqFlags, &negHdr);
    if (status == CSL_EFAIL)
    {
        directTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_ProcessPmMessage: Negative Arg Test Passed.\n");
    }
    else
    {
        directTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_ProcessPmMessage: Negative Arg Test Failed.\n");
    }

    return directTestStatus;
}

/* This function covers the MC/DC coverage of sciclient_direct.c file */
static int32_t SciclientApp_directNegTest2(void)
{
    int32_t  status                    = CSL_PASS;
    int32_t  sciclientDirectTestStatus = CSL_PASS;
    uint32_t msg                       = TISCI_MSG_BOARD_CONFIG_PM;
    void     *messageType              = (uint32_t *)&msg;
    uint8_t  boardCfgHeader;
    uint8_t  commonHeader;
    Sciclient_BoardCfgPrms_t SciclientApp_PmRmPrms;
    Sciclient_ReqPrm_t SciclientApp_ReqPrm;

    /* Passing response parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_service(&SciclientApp_ReqPrm, NULL);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_service: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_service: Negative Arg Test Failed.\n");
    }

    /* Passing boardcfgheader parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_boardCfgPrepHeader(&commonHeader, NULL, &SciclientApp_PmRmPrms, &SciclientApp_PmRmPrms);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Failed.\n");
    }

    /* Passing pm parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_boardCfgPrepHeader(&commonHeader, &boardCfgHeader, NULL, &SciclientApp_PmRmPrms);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Failed.\n");
    }

    /* Passing rm parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_boardCfgPrepHeader(&commonHeader, &boardCfgHeader, &SciclientApp_PmRmPrms, NULL);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Negative Arg Test Failed.\n");
    }

    /* Passing valid parameters to Sciclient_boardCfgPrepHeader function */
    status = Sciclient_boardCfgPrepHeader(&commonHeader, &boardCfgHeader, &SciclientApp_PmRmPrms, &SciclientApp_PmRmPrms);
    if (status == CSL_PASS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Valid Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgPrepHeader: Valid Arg Test Failed.\n");
    }

    /* Passing pm parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_boardCfgParseHeader(&commonHeader, NULL, &SciclientApp_PmRmPrms);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Failed.\n");
    }

    /* Passing rm parameter as NULL to cover BADARGS condition for MCDC */
    status = Sciclient_boardCfgParseHeader(&commonHeader, &SciclientApp_PmRmPrms, NULL);
    if (status == CSL_EBADARGS)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_boardCfgParseHeader: Negative Arg Test Failed.\n");
    }

    /* Passing TISCI_MSG_BOARD_CONFIG_PM as message type will cover default switch condition in Sciclient_ProcessRmMessage */
    status = Sciclient_ProcessRmMessage(messageType);
    if (status == CSL_EFAIL)
    {
        sciclientDirectTestStatus += CSL_PASS;
        SciApp_printf ("Sciclient_ProcessRmMessage: Negative Arg Test Passed.\n");
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf ("Sciclient_ProcessRmMessage: Negative Arg Test Failed.\n");
    }

    return sciclientDirectTestStatus;
}

static int32_t SciclientApp_boardCfgParseHeaderTest(void)
{
    int32_t  status                         = CSL_PASS;
    int32_t  boardCfgParseHeaderTestStatus  = CSL_PASS;
    uint8_t  commonHeader                   = 0x0U;
    uint8_t *commonHeaderAddress;
    commonHeaderAddress                     = &commonHeader;
    uint8_t *boardCfgHeader                 = 0U;
    Sciclient_BoardCfgPrms_t rmBoardCfgParams =  {
      .boardConfigLow   = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
      .boardConfigHigh  = 0,
      .boardConfigSize  = SCICLIENT_BOARDCFG_RM_SIZE_IN_BYTES,
      .devGrp           = DEVGRP_ALL
    };
    Sciclient_BoardCfgPrms_t pmBoardCfgParams  = {
        .boardConfigLow = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
        .boardConfigHigh = 0U,
        .boardConfigSize = SCICLIENT_BOARDCFG_PM_SIZE_IN_BYTES,
        .devGrp = DEVGRP_ALL,
    };

    SciApp_printf("sciclient_boardCfgParseHeader Test :\n");
    /* Passing invalid commonHeader */
    status = Sciclient_boardCfgParseHeader(&commonHeader, &pmBoardCfgParams,
                                           &rmBoardCfgParams);
    if(status == CSL_EFAIL)
    {
        boardCfgParseHeaderTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_boardCfgParseHeader Negative test-1 Passed\n");
    }
    else
    {
        boardCfgParseHeaderTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_boardCfgParseHeader Negative test-1 Failed\n");
    }

    /* Passing invalid comp type */
    SciApp_extBootX509Table  *pX509Table = (SciApp_extBootX509Table *) commonHeaderAddress;
    memcpy(pX509Table->magic_word, gcSciclientDirectExtBootX509MagicWord,
           sizeof(gcSciclientDirectExtBootX509MagicWord));
     pX509Table->num_comps = 1;
     pX509Table->comps[0].comp_type = 0x11;
    status = Sciclient_boardCfgParseHeader((uint8_t *)commonHeaderAddress,
                                            &pmBoardCfgParams, &rmBoardCfgParams);
    if(status == CSL_EFAIL)
    {
        boardCfgParseHeaderTestStatus += CSL_PASS;
        SciApp_printf("Sciclient_boardCfgParseHeader Negative test-2 Passed\n");
    }
    else
    {
        boardCfgParseHeaderTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_boardCfgParseHeader Negative test-2 Failed\n");
    }

   /* Passing invalid commonHeaderAddress and valid comp type */
   pX509Table->comps[0].comp_type = 0x10;
   status = Sciclient_boardCfgParseHeader((uint8_t *)commonHeaderAddress,
                                           &pmBoardCfgParams, &rmBoardCfgParams);
   if(status == CSL_EFAIL)
   {
       boardCfgParseHeaderTestStatus += CSL_PASS;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-3 Passed\n");
   }
   else
   {
       boardCfgParseHeaderTestStatus += CSL_EFAIL;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-3 Failed\n");
   }

   /* Passing TISCI_MSG_BOARD_CONFIG_PM desc type */
   SciApp_boardCfgDescTable * pBoardCfgDesc = (SciApp_boardCfgDescTable *) boardCfgHeader;
   pBoardCfgDesc->num_elems       = 1U;
   pBoardCfgDesc->sw_rev          = 0U;
   pBoardCfgDesc->descs[0].type   = TISCI_MSG_BOARD_CONFIG_PM;
   pX509Table->comps[0].comp_type = 0x11;
   pX509Table->comps[0].dest_addr = (uint64_t) boardCfgHeader;
   status = Sciclient_boardCfgParseHeader((uint8_t *)commonHeaderAddress,
                                           &pmBoardCfgParams, &rmBoardCfgParams);
   if(status == CSL_EFAIL)
   {
       boardCfgParseHeaderTestStatus += CSL_PASS;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-4 Passed\n");
   }
   else
   {
       boardCfgParseHeaderTestStatus += CSL_EFAIL;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-4 Failed\n");
   }

   /* Passing TISCI_MSG_BOARD_CONFIG_RM desc type */
   pBoardCfgDesc->descs[0].type = TISCI_MSG_BOARD_CONFIG_RM;
   status = Sciclient_boardCfgParseHeader((uint8_t *)commonHeaderAddress,
                                           &pmBoardCfgParams, &rmBoardCfgParams);
   if(status == CSL_EFAIL)
   {
       boardCfgParseHeaderTestStatus += CSL_PASS;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-5 Passed\n");
   }
   else
   {
       boardCfgParseHeaderTestStatus += CSL_EFAIL;
       SciApp_printf("Sciclient_boardCfgParseHeader Negative test-5 Failed\n");
   }

   return boardCfgParseHeaderTestStatus;
}

static int32_t SciclientApp_boardcfgRmFindCertSizeTest(void)
{
    int32_t status                       = CSL_PASS;
    int32_t boardcfgRmFindCertSizeStatus = CSL_PASS;
    uint8_t cert[10]                     = {0x30U, 0x81, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04};
    uint8_t *tisciBoardCfgRmpLowAddr     = &cert[0];
    int32_t sciclientInitStatus          = CSL_PASS;
    Sciclient_ConfigPrms_t config      =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(sciclientInitStatus == CSL_PASS)
    {
        struct tisci_msg_board_config_rm_req request = {
            .tisci_boardcfg_rmp_low   = (uint32_t) tisciBoardCfgRmpLowAddr,
            .tisci_boardcfg_rmp_high  = (uint32_t) 0x0U,
            .tisci_boardcfg_rm_size   = (uint16_t) 10U,
            .tisci_boardcfg_rm_devgrp = (uint8_t) DEVGRP_ALL
        };
        Sciclient_ReqPrm_t reqParam = {
            .messageType    = (uint16_t) TISCI_MSG_BOARD_CONFIG_RM,
            .flags          = (uint32_t) TISCI_MSG_FLAG_ACK,
            .pReqPayload    = (const uint8_t *) &request,
            .reqPayloadSize = (uint32_t) sizeof (request),
            .timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER
        };
        Sciclient_RespPrm_t respParam = {
            .flags           = (uint32_t) 0,
            .pRespPayload    = (uint8_t *) 0,
            .respPayloadSize = (uint32_t) 0
        };

        SciApp_printf("boardcfgRmFindCertSize Test :\n");
        /* Passing certSize 0x30 */
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            boardcfgRmFindCertSizeStatus += CSL_PASS;
            SciApp_printf("boardcfgRmFindCertSize test-1 passed\n");
        }
        else
        {
            boardcfgRmFindCertSizeStatus += CSL_EFAIL;
            SciApp_printf("boardcfgRmFindCertSize test-1 failed\n");
        }

        /* Passing cert_len 0x82 */
        cert[1] = 0x82;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            boardcfgRmFindCertSizeStatus += CSL_PASS;
            SciApp_printf("boardcfgRmFindCertSize test-2 passed\n");
        }
        else
        {
            boardcfgRmFindCertSizeStatus += CSL_EFAIL;
            SciApp_printf("boardcfgRmFindCertSize test-2 failed\n");
        }

        /* Passing cert_len 0x70 */
        cert[0] = 0x30;
        cert[1] = 0x70;
        tisciBoardCfgRmpLowAddr = &cert[0];
        request.tisci_boardcfg_rmp_low   = (uint32_t) tisciBoardCfgRmpLowAddr;
        reqParam.pReqPayload    = (const uint8_t *) &request,
        reqParam.reqPayloadSize = (uint32_t) sizeof (request);
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            boardcfgRmFindCertSizeStatus += CSL_PASS;
            SciApp_printf("boardcfgRmFindCertSize test-3 passed\n");
        }
        else
        {
            boardcfgRmFindCertSizeStatus += CSL_EFAIL;
            SciApp_printf("boardcfgRmFindCertSize test-3 failed\n");
        }
    }
    else
    {
        boardcfgRmFindCertSizeStatus = CSL_EFAIL;
    }

    status = Sciclient_deinit();
    if(status == CSL_PASS)
    {
        boardcfgRmFindCertSizeStatus += CSL_PASS;
    }
    else
    {
        boardcfgRmFindCertSizeStatus += CSL_EFAIL;
    }

    return boardcfgRmFindCertSizeStatus;
}

static int32_t SciclientApp_sciServiceTest(void)
{
    int32_t  status               = CSL_PASS;
    int32_t  sciServiceTestStatus = CSL_PASS;
    int32_t  sciclientInitStatus  = CSL_PASS;
    struct tisci_msg_board_config_rm_req request = {
        .tisci_boardcfg_rmp_low   = SCICLIENT_ALLOWED_BOARDCFG_BASE_START,
        .tisci_boardcfg_rmp_high  = (uint32_t) 0x0U,
        .tisci_boardcfg_rm_size   = (uint16_t) SCICLIENT_BOARDCFG_RM_SIZE_IN_BYTES,
        .tisci_boardcfg_rm_devgrp = (uint8_t) DEVGRP_ALL
    };
    Sciclient_ReqPrm_t reqParam = {
        .messageType    = (uint16_t) TISCI_MSG_BOARD_CONFIG_RM,
        .flags          = (uint32_t) TISCI_MSG_FLAG_ACK,
        .pReqPayload    = (const uint8_t *) &request,
        .reqPayloadSize = (uint32_t) sizeof (request),
        .timeout        = (uint32_t) SCICLIENT_SERVICE_WAIT_FOREVER
    };
    Sciclient_RespPrm_t respParam = {
        .flags           = (uint32_t) 0,
        .pRespPayload    = (uint8_t *) 0,
        .respPayloadSize = (uint32_t) 0
    };
    struct tisci_msg_board_config_rm_resp resp;
    Sciclient_RespPrm_t queryFwCapsRespParam = {
        .flags           = (uint32_t) 0,
        .pRespPayload    = (uint8_t *) &resp,
        .respPayloadSize = (uint32_t) sizeof (request)
    };

    Sciclient_ConfigPrms_t config      =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
        status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(sciclientInitStatus == CSL_PASS)
    {
        SciApp_printf("sciclient_service Test :\n");

        /* Passing TISCI_MSG_QUERY_FW_CAPS message type */
        reqParam.messageType = TISCI_MSG_QUERY_FW_CAPS;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test failed\n");
        }

        /* Passing TISCI_MSG_QUERY_FW_CAPS message type with TISCI_MSG_FLAG_AOP as req flag */
        reqParam.flags       = TISCI_MSG_FLAG_AOP;
        reqParam.messageType = TISCI_MSG_QUERY_FW_CAPS;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test failed\n");
        }

        /* Passing TISCI_MSG_QUERY_FW_CAPS message type with TISCI_MSG_FLAG_AOP as req flag */
        reqParam.flags       = TISCI_MSG_FLAG_AOP;
        reqParam.messageType = TISCI_MSG_QUERY_FW_CAPS;
        status = Sciclient_service(&reqParam, &queryFwCapsRespParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test failed\n");
        }

        /* Passing TISCI_MSG_RM_UDMAP_FLOW_DELEGATE message type */
        respParam.pRespPayload = NULL;
        reqParam.messageType   = TISCI_MSG_RM_UDMAP_FLOW_DELEGATE;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_RM_UDMAP_FLOW_DELEGATE test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_RM_UDMAP_FLOW_DELEGATE test failed\n");
        }

        respParam.pRespPayload = NULL;
        reqParam.messageType = TISCI_MSG_RM_PROXY_CFG;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_RM_PROXY_CFG test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_RM_PROXY_CFG test failed\n");
        }

        respParam.pRespPayload = NULL;
        reqParam.messageType = TISCI_MSG_QUERY_FW_CAPS;
        status = Sciclient_service(&reqParam, &respParam);
        if(status == CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service TISCI_MSG_QUERY_FW_CAPS test failed\n");
        }

        status = Sciclient_service(NULL,&respParam);
        if(status != CSL_PASS)
        {
            sciServiceTestStatus += CSL_PASS;
            SciApp_printf("Sciclient_service input params test passed\n");
        }
        else
        {
            sciServiceTestStatus += CSL_EFAIL;
            SciApp_printf("Sciclient_service input params test failed\n");
        }
    }
    else
    {
        sciServiceTestStatus = CSL_EFAIL;
        SciApp_printf("sciclient_init() has failed.\n");
    }

    status = Sciclient_deinit();
    if(status == CSL_PASS)
    {
        sciServiceTestStatus += CSL_PASS;
    }
    else
    {
        sciServiceTestStatus += CSL_EFAIL;
    }

    return sciServiceTestStatus;
}

static int32_t SciclientApp_directTest(void)
{
    int32_t  status                     = CSL_PASS;
    int32_t  sciclientInitStatus        = CSL_PASS;
    int32_t  sciclientDirectTestStatus  = CSL_PASS;
    Sciclient_ConfigPrms_t config =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_init PASSED.\n");
        SciApp_printf("This test has five sub-tests:\n");
        sciclientDirectTestStatus += SciclientApp_directNegTest1();
        sciclientDirectTestStatus += SciclientApp_directNegTest2();
        sciclientDirectTestStatus += SciclientApp_boardCfgParseHeaderTest();
        sciclientDirectTestStatus += SciclientApp_boardcfgRmFindCertSizeTest();
        sciclientDirectTestStatus += SciclientApp_sciServiceTest();
    }
    else
    {
        sciclientDirectTestStatus += CSL_EFAIL;
        SciApp_printf("Sciclient_init FAILED.\n");
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientDirectTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientDirectTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientDirectTestStatus;
}
#endif

static int32_t SciclientApp_osalTest(void)
{
    int32_t  status                     = CSL_PASS;
    int32_t  sciclientInitStatus        = CSL_PASS;
    int32_t  sciclientOsalTestStatus    = CSL_PASS;
    uintptr_t key                       = 0U;
    Sciclient_ConfigPrms_t config       =
    {
       SCICLIENT_SERVICE_OPERATION_MODE_INTERRUPT,
       NULL,
       0 /* isSecure = 0 un secured for all cores */
    };

    while (gSciclientHandle.initCount != 0)
    {
       status = Sciclient_deinit();
    }
    status = Sciclient_init(&config);
    sciclientInitStatus = status;

    gSciclient_writeInProgress = 1;
    status    = Sciclient_osalAcquireSecureProxyAcess(&key,1);
    Sciclient_osalReleaseSecureProxyAcess(&key);

    if(status == CSL_PASS)
    {
        SciApp_printf("Sciclient_osalAcquireSecureProxyAcess() failed\n");
        sciclientOsalTestStatus += CSL_EFAIL;
    }
    else
    {
        SciApp_printf("Sciclient_osalAcquireSecureProxyAcess() passed\n");
        sciclientOsalTestStatus += CSL_PASS;
    }

    if(sciclientInitStatus == CSL_PASS)
    {
       status = Sciclient_deinit();
       if(status == CSL_PASS)
       {
           sciclientOsalTestStatus += CSL_PASS;
           SciApp_printf("Sciclient_deinit PASSED.\n");
       }
       else
       {
           sciclientOsalTestStatus += CSL_EFAIL;
           SciApp_printf("Sciclient_deinit FAILED.\n");
       }
    }

    return sciclientOsalTestStatus;
}

#if defined(BUILD_MPU) || defined (BUILD_C7X)
extern void Osal_initMmuDefault(void);
void InitMmu(void)
{
    Osal_initMmuDefault();
}
#endif
