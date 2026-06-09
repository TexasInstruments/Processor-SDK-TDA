/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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
 * \file test_connection.c
 *
 * \brief EthFw UT functions for testing connection.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x802

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Enet LLD header files */
#include <ti/drv/enet/enet.h>
#include <ti/drv/enet/include/per/cpsw.h>

/* EthFw header files */
#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <ethremotecfg/protocol/ethremotecfg.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_utils.h>

#include <unit_test/unity/include/unity.h>
#include "ethfw_test_cases.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
CpswProxy_Handle gTestProxy;
CpswProxy_Config *gTestProxyConfig;
/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwUT_attachCmd1(void)
{
    uint32_t txMtu[ENET_PRI_NUM];
    uint32_t hostPortRxMtu;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t features;
    int32_t status;

    /* Send attach first. */
    status = CpswProxy_attach(gTestProxy,
                              gTestProxyConfig->virtPort,
                              &hostPortRxMtu,
                              txMtu,
                              &numTxCh,
                              &numRxFlow,
                              &features);

    if (status != CPSWPROXY_SOK)
        goto end;

    /* Send detach request. */
    status = CpswProxy_detach(gTestProxy);
    TEST_ASSERT_FALSE_MESSAGE((status != CPSWPROXY_SOK), "Failed to detach from Ethernet device.");

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_attachCmd2(void)
{
    uint32_t txMtu[ENET_PRI_NUM];
    uint32_t hostPortRxMtu;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t features;
    int32_t status;

    /* Send attach first. */
    status = CpswProxy_attach(gTestProxy,
                              gTestProxyConfig->virtPort,
                              &hostPortRxMtu,
                              txMtu,
                              &numTxCh,
                              &numRxFlow,
                              &features);

    if (status != CPSWPROXY_SOK)
        goto end;

    /* Send detach request. */
    status = CpswProxy_detach(gTestProxy);
    TEST_ASSERT_FALSE_MESSAGE((status != CPSWPROXY_SOK), "Failed to detach from Ethernet device.");

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_detachCmd1(void)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t txMtu[ENET_PRI_NUM];
    uint32_t hostPortRxMtu;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t features;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* For detach test,we need to attach first. */
    status = CpswProxy_attach(gTestProxy,
                              gTestProxyConfig->virtPort,
                              &hostPortRxMtu,
                              txMtu,
                              &numTxCh,
                              &numRxFlow,
                              &features);

    if (status != CPSWPROXY_SOK)
        goto end;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DETACH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_detachCmd2(void)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t txMtu[ENET_PRI_NUM];
    uint32_t hostPortRxMtu;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t features;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* For detach test,we need to attach first. */
    status = CpswProxy_attach(gTestProxy,
                              gTestProxyConfig->virtPort,
                              &hostPortRxMtu,
                              txMtu,
                              &numTxCh,
                              &numRxFlow,
                              &features);

    if (status != CPSWPROXY_SOK)
        goto end;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DETACH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}


void EthFwUT_detachOnly(void)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* We will try to detach without attaching first */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DETACH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    
    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_testSwitchConnection(void *arg1, void * arg2)
{
    int32_t status;

    gTestProxy = (CpswProxy_Handle)arg1;
    TEST_ASSERT_NOT_NULL(gTestProxy);

    gTestProxyConfig = (CpswProxy_Config *)arg2;
    TEST_ASSERT_FALSE_MESSAGE((gTestProxyConfig->virtPort != ETHREMOTECFG_SWITCH_PORT_1), "Invalid virtual port.");

    UnityBegin("test_connection.c");

    /* ETHFW-ETHFW_UT_SWITCH_ATTACH_TEST1_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_attachCmd1,  ETHFW_UT_SWITCH_ATTACH_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_ATTACH_TEST2_ID: Test ATTACH twice with valid parameters. */
    RUN_TEST(EthFwUT_attachCmd2,  ETHFW_UT_SWITCH_ATTACH_TEST2_ID);

    /* ETHFW-ETHFW_UT_SWITCH_DETACH_TEST1_ID: Test DETACH with valid parameters. */
    RUN_TEST(EthFwUT_detachCmd1,  ETHFW_UT_SWITCH_DETACH_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_DETACH_TEST2_ID: Test DETACH with valid parameters. */
    RUN_TEST(EthFwUT_detachCmd2,  ETHFW_UT_SWITCH_DETACH_TEST2_ID);

    /* ETHFW-ETHFW_UT_SWITCH_DETACH_ONLY_TEST_ID: Test DETACH without ATTACH with invalid parameters. */
    RUN_TEST(EthFwUT_detachOnly,  ETHFW_UT_SWITCH_DETACH_ONLY_TEST_ID);

    UnityEnd();
}

void EthFwUT_testMacConnection(void *arg1, void * arg2)
{
    int32_t status;

    gTestProxy = (CpswProxy_Handle)arg1;
    TEST_ASSERT_NOT_NULL(gTestProxy);

    gTestProxyConfig = (CpswProxy_Config *)arg2;
    TEST_ASSERT_FALSE_MESSAGE((gTestProxyConfig->virtPort != ETHREMOTECFG_MAC_PORT_4), "Invalid virtual port.");

    UnityBegin("test_connection.c");

    /* ETHFW-ETHFW_UT_MAC_ATTACH_TEST1_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_attachCmd1,  ETHFW_UT_MAC_ATTACH_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_ATTACH_TEST2_ID: Test ATTACH twice with valid parameters. */
    RUN_TEST(EthFwUT_attachCmd2,  ETHFW_UT_MAC_ATTACH_TEST2_ID);

    /* ETHFW-ETHFW_UT_MAC_DETACH_TEST1_ID: Test DETACH with valid parameters. */
    RUN_TEST(EthFwUT_detachCmd1,  ETHFW_UT_MAC_DETACH_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_DETACH_TEST2_ID: Test DETACH with valid parameters. */
    RUN_TEST(EthFwUT_detachCmd2,  ETHFW_UT_MAC_DETACH_TEST2_ID);

    /* ETHFW-ETHFW_UT_MAC_DETACH_ONLY_TEST_ID: Test DETACH without ATTACH with invalid parameters. */
    RUN_TEST(EthFwUT_detachOnly,  ETHFW_UT_MAC_DETACH_ONLY_TEST_ID);


    UnityEnd();
}
