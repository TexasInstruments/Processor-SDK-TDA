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

#ifndef APP_DSS_DEFAULT_PRIV_H
#define APP_DSS_DEFAULT_PRIV_H

#include <utils/dss/include/app_dss.h>
#include <utils/dss/include/app_dctrl.h>
#include <utils/dss/include/app_dss_defaults.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <dss.h>

typedef struct {

    uint32_t enableM2m;
    uint32_t nodeOverlayId;
    uint32_t overlayId;
    uint32_t pipeId;
    uint32_t vpId;
    uint32_t nodeVpId;
} app_dss_default_m2m_obj_t;

typedef struct {

    app_dss_default_prm_t initPrm;
    uint32_t overlayId;
    uint32_t vpId;
    uint32_t nodeOverlayId;
    uint32_t nodeVpId;
    uint32_t nodeDpiId;
    uint32_t videoIfWidth;

    app_dss_default_m2m_obj_t m2m;
} app_dss_default_obj_t;

typedef struct {

    app_dss_default_obj_t display[2];

    uint32_t vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_MAX];

    app_dss_default_m2m_obj_t m2m;
} app_dss_dual_display_default_obj_t;

void appDssConfigurePm(app_dss_default_prm_t *prm);
void appDssConfigureBoard(app_dss_default_prm_t *prm);
void appDssConfigureDP(void);

int32_t appDctrlDefaultInit(app_dss_default_obj_t *obj);
int32_t appDctrlDefaultDeInit(void);
void appDssConfigureUB941AndUB925(app_dss_default_prm_t *prm);

int32_t appDctrlDualDisplayDefaultInit(app_dss_dual_display_default_obj_t *obj);
int32_t appDctrlDualDisplayDefaultDeInit(void);




#endif /* APP_DSS_DEFAULT_PRIV_H */

