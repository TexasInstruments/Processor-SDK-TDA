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
 * \file test_resources.c
 *
 * \brief EthFw UT functions for testing resources.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x803

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

static void EthFwUT_testAttach(void)
{
    uint32_t txMtu[ENET_PRI_NUM];
    uint32_t hostPortRxMtu;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t features;
    int32_t status;

    status = CpswProxy_attach(gTestProxy,
                              gTestProxyConfig->virtPort,
                              &hostPortRxMtu,
                              txMtu,
                              &numTxCh,
                              &numRxFlow,
                              &features);
    TEST_ASSERT_FALSE_MESSAGE((status != CPSWPROXY_SOK), "Failed to attach to Ethernet device.");
}

static void EthFwUT_TestDetach(void)
{
    int32_t status;

    status = CpswProxy_detach(gTestProxy);
    TEST_ASSERT_FALSE_MESSAGE((status != CPSWPROXY_SOK), "Failed to detach from Ethernet device.");
}

void EthFwUT_allocTxCmdTest1(void)
{
    EthRemoteCfg_AllocTxReq req;
    EthRemoteCfg_AllocTxRes res;
    uint32_t relChPriority = 0U;
    int32_t status;

    req.chRelPriority = relChPriority;
    memset(&res, 0, sizeof(EthRemoteCfg_AllocTxRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto end;

    /* Free the allocated channel */
    status = CpswProxy_freeTxCh(gTestProxy,
                                res.txPsilDstId);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_allocTxCmdNegTest(void)
{
    EthRemoteCfg_AllocTxReq req;
    EthRemoteCfg_AllocTxRes res;
    int32_t status;

    /* Send invalid priority */
    req.chRelPriority = 0xFFU;
    memset(&res, 0, sizeof(EthRemoteCfg_AllocTxRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_freeTxCmdTest1(void)
{
    EthRemoteCfg_FreeTxReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t txPSILId;
    uint32_t relChPriority = 0U;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* For freeing a TX channel, we need to allocate one first */
    status = CpswProxy_allocTxCh(gTestProxy,
                                 &txPSILId,
                                 relChPriority);
    if (status != CPSWPROXY_SOK)
        goto end;

    req.txPsilDstId = txPSILId;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_FREE_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_freeTxCmdNegTest(void)
{
    EthRemoteCfg_FreeTxReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    /* Send invalid txPsilDstId */
    req.txPsilDstId = 0xFFU;
    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_FREE_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_allocRxCmdTest1(void)
{
    EthRemoteCfg_AllocRxReq req;
    EthRemoteCfg_AllocRxRes res;
    int32_t status;

    req.flowIdx = 0U;
    memset(&res, 0, sizeof(EthRemoteCfg_AllocRxRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto end;

    /* Free the allocated flow */
    status = CpswProxy_freeRxFlow(gTestProxy,
                                  res.rxFlowIdxBase,
                                  res.rxFlowIdxOffset);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_freeRxCmdTest1(void)
{
    EthRemoteCfg_FreeRxReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    uint32_t flowIdx = 0U;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* For freeing a RX flow, we need to allocate one RX flow first */
    status = CpswProxy_allocRxFlow(gTestProxy,
                                   &rxStartFlowId,
                                   &rxFlowIdOffset,
                                   flowIdx);
    if (status != CPSWPROXY_SOK)
        goto end;

    req.rxFlowIdxBase = rxStartFlowId;
    req.rxFlowIdxOffset = rxFlowIdOffset;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_FREE_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_allocRxCmdNegTest(void)
{
    EthRemoteCfg_AllocRxReq req;
    EthRemoteCfg_AllocRxRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_AllocRxRes));

    /* Send invalid priority */
    req.flowIdx = 0xFFU;

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_freeRxCmdNegTest(void)
{
    EthRemoteCfg_FreeRxReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    /* Send invalid priority */
    req.rxFlowIdxBase = 0xFFU;
    req.rxFlowIdxOffset = 0xFFFU;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_FREE_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_allocMacCmdTest1(void)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_AllocMacRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_AllocMacRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto end;

    /* Free the allocated MAC */
    status = CpswProxy_freeMac(gTestProxy,
                               res.macAddr);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_freeMacCmdTest1(void)
{
    EthRemoteCfg_FreeMacReq req;
    EthRemoteCfg_StatusRes res;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* To free a MAC, we need to allocate one first */
    status = CpswProxy_allocMac(gTestProxy,
                                macAddr);
    if (status != CPSWPROXY_SOK)
        goto end;

    memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_FREE_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_registerMacCmdTest(void)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    uint32_t flowIdx = 0U;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    status = CpswProxy_allocRxFlow(gTestProxy,
                                   &rxStartFlowId,
                                   &rxFlowIdOffset,
                                   flowIdx);
    if (status != CPSWPROXY_SOK)
        goto end;

    status = CpswProxy_allocMac(gTestProxy,
                                macAddr);
    if (status != CPSWPROXY_SOK)
        goto err_alloc;

    req.flowIdxBase   = rxStartFlowId;
    req.flowIdxOffset = rxFlowIdOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_REGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
    goto err_regmac;

    /* Deregister the MAC */
    status = CpswProxy_unregisterDstMacRxFlow(gTestProxy,
                                              rxStartFlowId,
                                              rxFlowIdOffset,
                                              macAddr);

err_regmac:
    /* Free the MAC */
    status = CpswProxy_freeMac(gTestProxy,
                               macAddr);
err_alloc:
    /* Free the allocated RX flow */
    status = CpswProxy_freeRxFlow(gTestProxy,
                                  rxStartFlowId,
                                  rxFlowIdOffset);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_registerMacCmdNegTest(void)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN] = {0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6};
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send invalid values */
    req.flowIdxBase   = 0xFF;
    req.flowIdxOffset = 0xFFF;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_REGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_unregisterMacCmdTest(void)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    uint32_t flowIdx = 0U;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN];
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    status = CpswProxy_allocRxFlow(gTestProxy,
                                   &rxStartFlowId,
                                   &rxFlowIdOffset,
                                   flowIdx);
    if (status != CPSWPROXY_SOK)
    goto end;

    status = CpswProxy_allocMac(gTestProxy,
                                macAddr);
    if (status != CPSWPROXY_SOK)
        goto err_alloc;

    status = CpswProxy_registerDstMacRxFlow(gTestProxy,
                                            rxStartFlowId,
                                            rxFlowIdOffset,
                                            macAddr);
    if (status != CPSWPROXY_SOK)
        goto err_regmac;

    req.flowIdxBase   = rxStartFlowId;
    req.flowIdxOffset = rxFlowIdOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DEREGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

err_regmac:
    /* Free the MAC */
    status = CpswProxy_freeMac(gTestProxy,
                               macAddr);
err_alloc:
    /* Free the allocated RX flow */
    status = CpswProxy_freeRxFlow(gTestProxy,
                                  rxStartFlowId,
                                  rxFlowIdOffset);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_unregisterMacCmdNegTest(void)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN] = {0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6};
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send invalid values */
    req.flowIdxBase   = 0xFF;
    req.flowIdxOffset = 0xFFF;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DEREGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_registerDefaultRxFlowCmdTest(void)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    uint32_t flowIdx = 0U;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    status = CpswProxy_allocRxFlow(gTestProxy,
                                   &rxStartFlowId,
                                   &rxFlowIdOffset,
                                   flowIdx);
    if (status != CPSWPROXY_SOK)
        goto end;

    req.flowIdxBase   = rxStartFlowId;
    req.flowIdxOffset = rxFlowIdOffset;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto err_setdef;

    /* Unregister RX flow */
    status = CpswProxy_unregisterDefaultRxFlow(gTestProxy,
                                               rxStartFlowId,
                                               rxFlowIdOffset);

err_setdef:
    /* Free the allocated RX flow */
    status = CpswProxy_freeRxFlow(gTestProxy,
                                  rxStartFlowId,
                                  rxFlowIdOffset);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_registerDefaultRxFlowCmdNegTest(void)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send invalid values */
    req.flowIdxBase   = 0xFF;
    req.flowIdxOffset = 0xFFF;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_unregisterDefaultRxFlowCmdTest(void)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    uint32_t flowIdx = 0U;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    status = CpswProxy_allocRxFlow(gTestProxy,
                                   &rxStartFlowId,
                                   &rxFlowIdOffset,
                                   flowIdx);
    if (status != CPSWPROXY_SOK)
        goto end;

    status = CpswProxy_registerDefaultRxFlow(gTestProxy,
                                             rxStartFlowId,
                                             rxFlowIdOffset);
    if (status != CPSWPROXY_SOK)
        goto err_setdef;

    req.flowIdxBase   = rxStartFlowId;
    req.flowIdxOffset = rxFlowIdOffset;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

err_setdef:
    /* Free the allocated RX flow */
    status = CpswProxy_freeRxFlow(gTestProxy,
                                  rxStartFlowId,
                                  rxFlowIdOffset);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_unregisterDefaultRxFlowCmdNegTest(void)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send invalid values */
    req.flowIdxBase   = 0xFF;
    req.flowIdxOffset = 0xFFF;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));

    /* Detach the client. */
    EthFwUT_TestDetach();

    if (status != CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_registerIPV4AddrCmdTest(void)
{
    EthRemoteCfg_IPv4AddrRegisterReq req;
    EthRemoteCfg_StatusRes res;
    uint8_t macAddr[ETHREMOTECFG_MACADDRLEN] = {0x70, 0xFF, 0x1D, 0x00, 0x40, 0x1E};
    uint8_t ipv4Addr[ETHREMOTECFG_IPV4ADDRLEN] = {192, 168, 0, 10};
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));
    memcpy(req.ipAddr, ipv4Addr, ETHREMOTECFG_IPV4ADDRLEN);
    memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_REGISTER_IPv4,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto end;

    /* unregister the ipv4 address */
    status = CpswProxy_unregisterIPV4Addr(gTestProxy,
                                          ipv4Addr);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_hwPushCmdTest(void)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_AllocHwPushRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_AllocHwPushRes));

    /* Attach the client. */
    EthFwUT_testAttach();

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(gTestProxy, ETHREMOTECFG_CMD_ALLOC_CPTS_HW_PUSH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    if (status != CPSWPROXY_SOK)
        goto end;

    /* Free the allocated HW push */
    status = CpswProxy_freeHwPushInst(gTestProxy, res.hwPushNum);

    /* Detach the client. */
    EthFwUT_TestDetach();

end:
    if (status == CPSWPROXY_SOK)
        TEST_PASS();
    else
        TEST_FAIL();
}

void EthFwUT_testSwitchResources(void *arg1, void *arg2)
{
    gTestProxy = (CpswProxy_Handle)arg1;
    TEST_ASSERT_NOT_NULL(gTestProxy);

    gTestProxyConfig = (CpswProxy_Config *)arg2;
    TEST_ASSERT_FALSE_MESSAGE((gTestProxyConfig->virtPort != ETHREMOTECFG_SWITCH_PORT_1), "Invalid virtual port.");

    UnityBegin("test_resources.c");

    /* ETHFW-ETHFW_UT_SWITCH_ALLOC_TX_TEST1_ID: Test ALLOC_TX with valid parameters. */
    RUN_TEST(EthFwUT_allocTxCmdTest1, ETHFW_UT_SWITCH_ALLOC_TX_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_ALLOC_TX_NEGTEST_ID: Test ALLOC_TX with invalid priority. */
    RUN_TEST(EthFwUT_allocTxCmdNegTest,  ETHFW_UT_SWITCH_ALLOC_TX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_FREE_TX_TEST1_ID: Test FREE_TX with valid parameters. */
    RUN_TEST(EthFwUT_freeTxCmdTest1,  ETHFW_UT_SWITCH_FREE_TX_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_FREE_TX_NEGTEST_ID: Test FREE_TX with invalid txPsilDstId. */
    RUN_TEST(EthFwUT_freeTxCmdNegTest,  ETHFW_UT_SWITCH_FREE_TX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_ALLOC_RX_TEST1_ID: Test ALLOC_RX with valid parameters. */
    RUN_TEST(EthFwUT_allocRxCmdTest1,  ETHFW_UT_SWITCH_ALLOC_RX_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_FREE_RX_TEST1_ID: Test FREE_RX with valid parameters. */
    RUN_TEST(EthFwUT_freeRxCmdTest1,  ETHFW_UT_SWITCH_FREE_RX_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_ALLOC_RX_NEGTEST_ID: Test ALLOC_RX with invalid priority. */
    RUN_TEST(EthFwUT_allocRxCmdNegTest,  ETHFW_UT_SWITCH_ALLOC_RX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_FREE_RX_NEGTEST_ID: Test FREE_RX with invalid priority. */
    RUN_TEST(EthFwUT_freeRxCmdNegTest,  ETHFW_UT_SWITCH_FREE_RX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_ALLOC_MAC_TEST1_ID: Test ALLOC_MAC with valid parameters. */
    RUN_TEST(EthFwUT_allocMacCmdTest1,  ETHFW_UT_SWITCH_ALLOC_MAC_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_FREE_MAC_TEST1_ID: Test FREE_MAC with valid parameters. */
    RUN_TEST(EthFwUT_freeMacCmdTest1,  ETHFW_UT_SWITCH_FREE_MAC_TEST1_ID);

    /* ETHFW-ETHFW_UT_SWITCH_REGISTER_MAC_TEST_ID: Test REGISTER_MAC with valid parameters. */
    RUN_TEST(EthFwUT_registerMacCmdTest,  ETHFW_UT_SWITCH_REGISTER_MAC_TEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_REGISTER_MAC_NEGTEST_ID: Test REGISTER_MAC with invalid RX flow params. */
    RUN_TEST(EthFwUT_registerMacCmdNegTest,  ETHFW_UT_SWITCH_REGISTER_MAC_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_UNREGISTER_MAC_TEST_ID: Test UNREGISTER_MAC with valid parameters. */
    RUN_TEST(EthFwUT_unregisterMacCmdTest,  ETHFW_UT_SWITCH_UNREGISTER_MAC_TEST_ID);

    /* ETHFW-ETHFW_UT_SWITCH_UNREGISTER_MAC_NEGTEST_ID: Test UNREGISTER_MAC with invalid RX flow params.. */
    RUN_TEST(EthFwUT_unregisterMacCmdNegTest,  ETHFW_UT_SWITCH_UNREGISTER_MAC_NEGTEST_ID);

    // RUN_TEST(EthFwUT_registerDefaultRxFlowCmdTest,  0);

    // RUN_TEST(EthFwUT_registerDefaultRxFlowCmdNegTest,  0);
    
    // RUN_TEST(EthFwUT_unregisterDefaultRxFlowCmdTest,  0);

    // RUN_TEST(EthFwUT_unregisterDefaultRxFlowCmdNegTest,  0);

    /* ETHFW-ETHFW_UT_SWITCH_REGISTER_IPV4_TEST_ID: Test REGISTER_IPV4 with valid parameters. */
    RUN_TEST(EthFwUT_registerIPV4AddrCmdTest,  ETHFW_UT_SWITCH_REGISTER_IPV4_TEST_ID);

    /* ETHFW-ETHFW_UT_HW_PUSH_TEST_ID: Test HW Push with valid parameters. */
    RUN_TEST(EthFwUT_hwPushCmdTest,  ETHFW_UT_HW_PUSH_TEST_ID);
    
    UnityEnd();
}

void EthFwUT_testMacResources(void *arg1, void *arg2)
{
    gTestProxy = (CpswProxy_Handle)arg1;
    TEST_ASSERT_NOT_NULL(gTestProxy);

    gTestProxyConfig = (CpswProxy_Config *)arg2;
    TEST_ASSERT_FALSE_MESSAGE((gTestProxyConfig->virtPort != ETHREMOTECFG_MAC_PORT_4), "Invalid virtual port.");

    UnityBegin("test_resources.c");

    /* ETHFW-ETHFW_UT_MAC_ALLOC_TX_TEST1_ID: Test ALLOC_TX with valid parameters. */
    RUN_TEST(EthFwUT_allocTxCmdTest1, ETHFW_UT_MAC_ALLOC_TX_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_FREE_TX_TEST1_ID: Test FREE_TX with valid parameters. */
    RUN_TEST(EthFwUT_freeTxCmdTest1,  ETHFW_UT_MAC_FREE_TX_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_ALLOC_RX_TEST1_ID: Test ALLOC_RX with valid parameters. */
    RUN_TEST(EthFwUT_allocRxCmdTest1,  ETHFW_UT_MAC_ALLOC_RX_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_FREE_RX_TEST1_ID: Test FREE_RX with valid parameters. */
    RUN_TEST(EthFwUT_freeRxCmdTest1,  ETHFW_UT_MAC_FREE_RX_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_ALLOC_MAC_TEST1_ID: Test ALLOC_MAC with valid parameters. */
    RUN_TEST(EthFwUT_allocMacCmdTest1,  ETHFW_UT_MAC_ALLOC_MAC_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_FREE_MAC_TEST1_ID: Test FREE_MAC with valid parameters. */
    RUN_TEST(EthFwUT_freeMacCmdTest1,  ETHFW_UT_MAC_FREE_MAC_TEST1_ID);

    /* ETHFW-ETHFW_UT_MAC_REGISTER_MAC_TEST_ID: Test REGISTER_MAC with valid parameters. */
    RUN_TEST(EthFwUT_registerMacCmdTest,  ETHFW_UT_MAC_REGISTER_MAC_TEST_ID);

    /* ETHFW-ETHFW_UT_MAC_UNREGISTER_MAC_TEST_ID: Test UNREGISTER_MAC with valid parameters. */
    RUN_TEST(EthFwUT_unregisterMacCmdTest,  ETHFW_UT_MAC_UNREGISTER_MAC_TEST_ID);

    /* ETHFW-ETHFW_UT_MAC_ALLOC_TX_NEGTEST_ID: Test ALLOC_TX with invalid priority. */
    RUN_TEST(EthFwUT_allocTxCmdNegTest,  ETHFW_UT_MAC_ALLOC_TX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_MAC_FREE_TX_NEGTEST_ID: Test FREE_TX with invalid txPsilDstId. */
    RUN_TEST(EthFwUT_freeTxCmdNegTest,  ETHFW_UT_MAC_FREE_TX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_MAC_ALLOC_RX_NEGTEST_ID: Test ALLOC_RX with invalid priority. */
    RUN_TEST(EthFwUT_allocRxCmdNegTest,  ETHFW_UT_MAC_ALLOC_RX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_MAC_FREE_RX_NEGTEST_ID: Test FREE_RX with invalid priority. */
    RUN_TEST(EthFwUT_freeRxCmdNegTest,  ETHFW_UT_MAC_FREE_RX_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_MAC_REGISTER_MAC_NEGTEST_ID: Test REGISTER_MAC with invalid RX flow params. */
    RUN_TEST(EthFwUT_registerMacCmdNegTest,  ETHFW_UT_MAC_REGISTER_MAC_NEGTEST_ID);

    /* ETHFW-ETHFW_UT_MAC_UNREGISTER_MAC_NEGTEST_ID: Test UNREGISTER_MAC with invalid RX flow params.. */
    RUN_TEST(EthFwUT_unregisterMacCmdNegTest,  ETHFW_UT_MAC_UNREGISTER_MAC_NEGTEST_ID);
    
    UnityEnd();
}
