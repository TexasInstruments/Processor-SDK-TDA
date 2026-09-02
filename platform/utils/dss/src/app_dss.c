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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <utils/dss/include/app_dss.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <dss.h>

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

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t appDssInit(app_dss_init_params_t *dssParams)
{
    int32_t retVal = 0; uint32_t i;
    Dss_InitParams dssInitParams;

    memset(&dssInitParams, 0, sizeof(Dss_InitParams));
    Dss_initParamsInit(&dssInitParams);

    dssInitParams.socParams.rmInfo.isCommRegAvailable[APP_DSS_COMM_REG_ID_0] = UTRUE;
    dssInitParams.socParams.rmInfo.isCommRegAvailable[APP_DSS_COMM_REG_ID_1] = UFALSE;
#if !defined(SOC_J722S)
    dssInitParams.socParams.rmInfo.isCommRegAvailable[APP_DSS_COMM_REG_ID_2] = UFALSE;
    dssInitParams.socParams.rmInfo.isCommRegAvailable[APP_DSS_COMM_REG_ID_3] = UFALSE;
#endif

    for(i=0U; i< APP_DSS_VID_PIPE_ID_MAX; i++)
    {
        dssInitParams.socParams.rmInfo.isPipeAvailable[i] = dssParams->isPipeAvailable[i];
    }
    for(i=0U; i< APP_DSS_OVERLAY_ID_MAX; i++)
    {
        dssInitParams.socParams.rmInfo.isOverlayAvailable[i] = dssParams->isOverlayAvailable[i];
    }
    for(i=0U; i< APP_DSS_VP_ID_MAX; i++)
    {
        dssInitParams.socParams.rmInfo.isPortAvailable[i] = dssParams->isPortAvailable[i];
    }

#if !defined(SOC_J722S)
    dssInitParams.socParams.dpInitParams.isAvailable = dssParams->isDpAvailable;
    dssInitParams.socParams.dsiInitParams.isAvailable = dssParams->isDsiAvailable;
#if defined (SOC_J721S2)
    dssInitParams.socParams.dpInitParams.isHpdSupported = false;
#elif defined (SOC_J721E) || defined (SOC_J784S4) || defined (SOC_J742S2)
    dssInitParams.socParams.dpInitParams.isHpdSupported = true;
#endif
#endif

    retVal = Dss_init(&dssInitParams);

    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_init failed !!!\n");
    }

    return retVal;
}

int32_t appDssDeInit(void)
{
    int32_t retVal = 0;

    retVal = Dss_deInit();
    if(0 != retVal)
    {
        appLogPrintf("DSS: ERROR: Dss_deInit failed !!!\n");
    }

    return (retVal);
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */
