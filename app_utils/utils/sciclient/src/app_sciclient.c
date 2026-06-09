/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include <utils/console_io/include/app_log.h>
#include <stdio.h>

#if defined(PDK)
#include <sciclient/sciclient.h>
#elif defined(MCU_PLUS_SDK)
#include <sciclient.h>
#include <SystemP.h>
#include <utils/ipc/include/app_ipc.h>
#endif

#if defined (SOC_AM62A)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0
};
#endif

#if defined (SOC_J722S)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_WKUP_R5FSS0_0,
    CSL_CORE_ID_MAIN_R5FSS0_0,
    CSL_CORE_ID_C75SS0_0,
    CSL_CORE_ID_C75SS1_0,
    CSL_CORE_ID_MCU_R5FSS0_0
};
#endif

#if defined (SOC_TDA54)
static uint32_t g_app_to_ipc_cpu_id[APP_IPC_CPU_MAX] =
{
    CSL_CORE_ID_A720_0,
    CSL_CORE_ID_MCU0,
    CSL_CORE_ID_MCU1,
    CSL_CORE_ID_MCU2,
    CSL_CORE_ID_MCU3,
    CSL_CORE_ID_MCU4,
    CSL_CORE_ID_RMCU0_0,
    CSL_CORE_ID_RMCU0_1,
    CSL_CORE_ID_RMCU1_0,
    CSL_CORE_ID_RMCU1_1,
    CSL_CORE_ID_RMCU2_0,
    CSL_CORE_ID_RMCU2_1,
    CSL_CORE_ID_DSP0,
    CSL_CORE_ID_DSP1,
    CSL_CORE_ID_DSP2,
    CSL_CORE_ID_DSP3
};
#endif

int32_t appSciclientDmscGetVersion(char *version_str, uint32_t version_str_size)
{
    int32_t retVal = 0;

    struct tisci_msg_version_req request;
    const Sciclient_ReqPrm_t      reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (uint8_t *)&request,
        sizeof(request),
#if defined(PDK)
        SCICLIENT_SERVICE_WAIT_FOREVER
#elif defined(MCU_PLUS_SDK)
        SystemP_WAIT_FOREVER
#endif
    };

    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t           respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof (response)
    };

    retVal = Sciclient_service(&reqPrm, &respPrm);
    if (0 == retVal)
    {
        if (respPrm.flags == TISCI_MSG_FLAG_ACK)
        {
            if(version_str == NULL)
            {
                appLogPrintf("SCICLIENT: DMSC FW version [%s]\n", (char *) response.str);
                appLogPrintf("SCICLIENT: DMSC FW revision 0x%x  \n", response.version);
                appLogPrintf("SCICLIENT: DMSC FW ABI revision %d.%d\n",
                    response.abi_major, response.abi_minor);
            }
            else
            {
                snprintf(version_str, version_str_size, "version %s, revision 0x%x, ABI %d.%d",
                    (char *) response.str,
                    response.version,
                    response.abi_major, response.abi_minor
                    );
            }
        }
        else
        {
            retVal = -1;
        }
    }
    if(retVal!=0)
    {
        appLogPrintf("SCICLIENT: ERROR: DMSC Firmware Get Version failed !!!\n");
    }

    return (retVal);
}

int32_t appSciclientInit(uint32_t self_cpu_id)
{
    int32_t retVal = 0;

    appLogPrintf("SCICLIENT: Init ... !!!\n");

#if defined(PDK)
    Sciclient_ConfigPrms_t  sciClientCfg;
    Sciclient_configPrmsInit(&sciClientCfg);

    retVal = Sciclient_init(&sciClientCfg);
#elif defined(MCU_PLUS_SDK)
    retVal = Sciclient_init(g_app_to_ipc_cpu_id[self_cpu_id]);
    DebugP_assertNoLog(SystemP_SUCCESS == retVal);
#endif

    if(retVal!=0)
    {
        appLogPrintf("SCICLIENT: ERROR: Sciclient init failed !!!\n");
    }
    if(retVal==0)
    {
        appSciclientDmscGetVersion(NULL, 0);
    }

    appLogPrintf("SCICLIENT: Init ... Done !!!\n");

    return retVal;
}

int32_t appSciclientDeInit()
{
    int32_t retVal = 0;

    retVal = Sciclient_deinit();
    if(retVal!=0)
    {
        appLogPrintf("SCICLIENT: ERROR: Sciclient deinit failed !!!\n");
    }

    return retVal;
}
