/*
 *  Copyright (C) 2026 Texas Instruments Incorporated
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

#ifndef TI_DRVIERS_OPEN_CLOSE_H_
#define TI_DRVIERS_OPEN_CLOSE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DSS
 */
#include "Dss/v0/dss.h"

#define CONFIG_DSS0                        0
#define CONFIG_DSS_NUM_INSTANCES           1
#define CONFIG_DSS_NUM_FRAMES_PER_PIPELINE 2

/* DSS VP Params */
extern Dss_DctrlVpParams gDssVpParams;

/* DSS VP Advance Params */
extern Dss_DctrlAdvVpParams gDssAdvVpParams;

/* DSS Overlay Params */
extern Dss_DctrlOverlayParams gDssOverlayParams;

/* DSS Overlay Layer Params */
extern Dss_DctrlOverlayLayerParams gDssOverlayLayerParams;

/* DSS Pipeline Configuration Params */
extern Dss_ConfigPipelineParams gDssConfigPipelineParams;

/* DSS driver object */
extern Dss_Object gDssObjects[CONFIG_DSS_NUM_INSTANCES];

#ifdef __cplusplus
}
#endif

#endif /* TI_DRVIERS_OPEN_CLOSE_H_ */
