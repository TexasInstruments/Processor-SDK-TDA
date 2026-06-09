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
 * \file test_ethRemoteCfg.c
 *
 * \brief EthFw UT functions for testing ethremoteCfg.
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
#include <ti/drv/ipc/ipc.h>
#include <ti/drv/enet/examples/utils/include/enet_apputils.h>

/* EthFw header files */
#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <ethremotecfg/protocol/ethremotecfg.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_utils.h>
#include <utils/board/include/ethfw_board_utils.h>

#include <ethremotecfg/protocol/ethremotecfg.h>
#include <ethremotecfg/server/include/ethfw.h>

#if defined(ETHFW_GPTP_SUPPORT)
/* Timesync header files */
#include <tsn_buildconf/jacinto_buildconf.h>
#include <tsn_gptp/gptpconf/gptpgcfg.h>
#include <tsn_gptp/gptpconf/xl4-extmod-xl4gptp.h>
#include <ethremotecfg/server/include/ethfw_tsn.h>
#endif

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

EthFw_Config *gTestEthFwCfg;
EthFw_Handle gTestEthFw;

#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
Enet_Type gEnetType = ENET_CPSW_9G;
uint32_t gInstId = 0U;
#elif defined(SOC_J7200)
Enet_Type gEnetType = ENET_CPSW_5G;
uint32_t gInstId = 0U;
#else
Enet_Type gEnetType = ENET_NULL;
uint32_t gInstId = 0U;
#endif

#if defined(ETHFW_GPTP_SUPPORT)
/* Ethernet ports where gPTP support is enabled, it must be composed of
 * ports in non MAC-only mode */
static Enet_MacPort gEthAppSwitchPorts[]=
{
#if defined(SOC_J721E)
    ENET_MAC_PORT_3,
#endif

#if defined(SOC_J7200)
    ENET_MAC_PORT_3,
#endif

#if defined(SOC_J784S4)
    ENET_MAC_PORT_3,
#endif

#if defined(SOC_J742S2)
    ENET_MAC_PORT_3,
#endif
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwUT_testEthFwInit(void)
{
    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);

    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testEthFwInit2(void)
{
    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);

    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testEthFwGetVersion(void)
{
    EthFw_Version ver;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);

    EthFw_getVersion(gTestEthFw, &ver);
    EthFwTrace_print("\nETHFW Version   : %d.%02d.%02d\n", ver.major, ver.minor, ver.rev);
    EthFwTrace_print("ETHFW Build Date: %s %s, %s\n", ver.month, ver.date, ver.year);
    EthFwTrace_print("ETHFW Build Time: %s:%s:%s\n", ver.hour, ver.min, ver.sec);
    EthFwTrace_print("ETHFW Commit SHA: %s\n\n", ver.commitHash);

    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testSrcPortMirroring(void)
{
    EthFwPortMirroring_Cfg portMirCfg = 
    {
        .mirroringMode =  
        {
            .srcPortMirCfg =
            {
                .srcPortNumMask = 1 << CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_3),
                .toPortNum  = CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_1)
            },
        },
        .mirroringType = SRC_PORT_MIRRORING
    };
    gTestEthFwCfg->portMirCfg = &portMirCfg;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);
    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testDstPortMirroring(void)
{
    EthFwPortMirroring_Cfg portMirCfg = 
    {
        .mirroringMode =  
        {
            .dstPortMirCfg = 
            {
                .dstPortNum = 0U,
                .toPortNum  = CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_1)
            },
        },
        .mirroringType = DST_PORT_MIRRORING
    };
    gTestEthFwCfg->portMirCfg = &portMirCfg;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);
    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testTblEntryPortMirroring(void)
{
    EthFwPortMirroring_Cfg portMirCfg = 
    {
        .mirroringMode =  
        {
            .tblEntryPortMirCfg =
            {
                .matchParams =
                {
                    .entryType = CPSW_ALE_TABLE_ENTRY_TYPE_ADDR,
                    .dstMacAddrInfo =
                    {
                        .addr =
                        {
                            /* Test MAC address */
                            .addr = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
                            .vlanId = 0U
                        },
                        .portNum = 0U
                    }
                },
                .toPortNum  = CPSW_ALE_MACPORT_TO_ALEPORT(ENET_MAC_PORT_1)
            },
        },
        .mirroringType = TBL_ENTRY_PORT_MIRRORING
    };
    gTestEthFwCfg->portMirCfg = &portMirCfg;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);
    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testDisablePortMirroring(void)
{
    EthFwPortMirroring_Cfg portMirCfg = 
    {
        .mirroringType = DISABLE_PORT_MIRRORING
    };
    gTestEthFwCfg->portMirCfg = &portMirCfg;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);
    EthFw_deinit(gTestEthFw);
}

void EthFwUT_testInvalidParamPortMirroring(void)
{
    /* EthFw_deinit will assert as EthFw handle returned is NULL
     * Hence failing this test case by default */
    TEST_FAIL();
    // gTestEthFwCfg->portMirCfg = NULL;
    // gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    // TEST_ASSERT_NULL(gTestEthFw);
    // EthFw_deinit(gTestEthFw);
}

void EthFwUT_dummyOpen(void *args)
{

}

void EthFwUT_dummyClose(void *args)
{
    
}

#if defined(ETHFW_MONITOR_SUPPORT)
void EthFwUT_testMonitor(void)
{
    gTestEthFwCfg->monitorCfg.openLwipDmaCb = EthFwUT_dummyOpen;
    gTestEthFwCfg->monitorCfg.closeLwipDmaCb = EthFwUT_dummyClose;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);

    EthFw_deinit(gTestEthFw);
}
#endif

#if defined(ETHFW_GPTP_SUPPORT)
static void EthFwUT_configPtpCb(void *arg)
{
    int32_t useHwPhase = 1;

    /* Apply phase adjustment directly to the HW */
    gptpgcfg_set_item(0, XL4_EXTMOD_XL4GPTP_USE_HW_PHASE_ADJUSTMENT, 
                      YDBI_CONFIG, &useHwPhase, sizeof(useHwPhase));
}

static void EthFwUT_initPtp(void)
{
    uint32_t portMask = 0U;
    uint8_t hostMacAddr[ENET_MAC_ADDR_LEN] = {0x70, 0xFF, 0x1D, 0x00, 0x40, 0x1E};
    uint8_t i;
    int32_t status;

    /* MAC port used for PTP */
    for (i = 0U; i < ENET_ARRAYSIZE(gEthAppSwitchPorts); i++)
    {
        portMask |= ENET_MACPORT_MASK(gEthAppSwitchPorts[i]);
    }

    status = EthFwTsn_initTimeSyncPtp(&hostMacAddr[0U], portMask);
    TEST_ASSERT_FALSE(status != CPSWPROXY_SOK);
}

void EthFwUT_testgPTP(void)
{
    gTestEthFwCfg->configPtpCb    = EthFwUT_configPtpCb;

    gTestEthFw = EthFw_init(gEnetType, gInstId, gTestEthFwCfg);
    TEST_ASSERT_NOT_NULL(gTestEthFw);

    EthFwUT_initPtp();

    EthFw_deinit(gTestEthFw);
}
#endif

void EthFwUT_testEthRemoteCfg(void *arg1)
{
    int32_t status;

    gTestEthFwCfg = (EthFw_Config *)arg1;
    TEST_ASSERT_NOT_NULL(gTestEthFwCfg);
    
 
    UnityBegin("test_ethRemoteCfg.c");

    /* ETHFW-ETHFW_UT_SERVER_INIT_TEST_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_testEthFwInit,  ETHFW_UT_SERVER_INIT_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_INIT2_TEST_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_testEthFwInit2,  ETHFW_UT_SERVER_INIT2_TEST_ID);

#if defined(ETHFW_MONITOR_SUPPORT)
    /* ETHFW-ETHFW_UT_SERVER_MONITOR_TEST_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_testMonitor,  ETHFW_UT_SERVER_MONITOR_TEST_ID);
#endif 

// #if defined(ETHFW_GPTP_SUPPORT)
//     /* ETHFW-ETHFW_UT_SERVER_GPTP_TEST_ID: Test ATTACH with valid parameters. */
//     RUN_TEST(EthFwUT_testgPTP,  ETHFW_UT_SERVER_GPTP_TEST_ID);
// #endif

    /* ETHFW-ETHFW_UT_SERVER_VERSION_TEST_ID: Test ATTACH with valid parameters. */
    RUN_TEST(EthFwUT_testEthFwGetVersion,  ETHFW_UT_SERVER_VERSION_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_SRC_PORT_MIRROR_TEST_ID: Test src port mirroring. */
    RUN_TEST(EthFwUT_testSrcPortMirroring,  ETHFW_UT_SERVER_SRC_PORT_MIRROR_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_DST_PORT_MIRROR_TEST_ID: Test dst port mirroring. */
    RUN_TEST(EthFwUT_testDstPortMirroring,  ETHFW_UT_SERVER_DST_PORT_MIRROR_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_TBL_ENTRY_PORT_MIRROR_TEST_ID: Test table entry port mirroring. */
    RUN_TEST(EthFwUT_testTblEntryPortMirroring,  ETHFW_UT_SERVER_TBL_ENTRY_PORT_MIRROR_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_DISABLE_PORT_MIRROR_TEST_ID: Test disable port mirroring. */
    RUN_TEST(EthFwUT_testDisablePortMirroring,  ETHFW_UT_SERVER_DISABLE_PORT_MIRROR_TEST_ID);

    /* ETHFW-ETHFW_UT_SERVER_INVALID_PARAM_PORT_MIRROR_TEST_ID: Test port mirroring with invalid input params. */
    RUN_TEST(EthFwUT_testInvalidParamPortMirroring,  ETHFW_UT_SERVER_INVALID_PARAM_PORT_MIRROR_TEST_ID);

    UnityEnd();
}
