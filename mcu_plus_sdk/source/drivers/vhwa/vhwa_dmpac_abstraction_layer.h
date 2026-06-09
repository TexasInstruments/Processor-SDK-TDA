#ifndef VHWA_DMPAC_ABSTRACTION_LAYER_H_
#define VHWA_DMPAC_ABSTRACTION_LAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================= */
/**
 *   @file  vhwa_dmpac_abstraction_layer.h
 *
 *   path  ti/drv/vhwa/vhwa_dmpac_abstraction_layer.h
 *
 *   @brief  SDK Abstraction layer definition
 *
 *  ============================================================
 *  Copyright (c) Texas Instruments Incorporated 2025
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    #define VHWA_PDK
#elif defined (SOC_J722S) || defined (SOC_AM62A) || defined (SOC_AM62AX)
    #define VHWA_MCU_PLUS_SDK
#endif

#if defined(VHWA_MCU_PLUS_SDK)
    #include     "kernel/dpl/SemaphoreP.h"
    #include     "drivers/fvid2/v0/fvid2.h"
    #include     "drivers/fvid2/v0/fvid2_dataTypes.h"
    #include     "drivers/fvid2/v0/fvid2_drvMgr.h"
    #include     "drivers/fvid2/v0/fvid2_api.h"
    #include     "drivers/udma/udma_priv.h"
    #include     "kernel/dpl/CacheP.h"
    #if defined(SOC_J722S)
    #include     "drivers/vhwa/soc/vhwa_dmpac_priv.h"
    #endif
#elif defined (VHWA_PDK)
    #include     "ti/osal/osal.h"
    #include     "ti/drv/fvid2/fvid2.h"
    #include     "ti/csl/csl_fvid2_dataTypes.h"
    #include     "ti/drv/fvid2/include/fvid2_drvMgr.h"
    #include     "ti/drv/fvid2/include/fvid2_api.h"
    #include     "ti/csl/csl_types.h"
    #include     "ti/drv/udma/udma.h"
    #include     "ti/drv/vhwa/soc/vhwa_dmpac_priv.h"
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#if defined(VHWA_MCU_PLUS_SDK)
    #define SYSTEMP_WAIT_FOREVER    SystemP_WAIT_FOREVER
    #define SYSTEMP_SUCCESS         SystemP_SUCCESS
    #define SYSTEMP_FAILURE         SystemP_FAILURE
    #define SYSTEMP_TIMEOUT         SystemP_TIMEOUT
    #define SYSTEMP_NO_WAIT         SystemP_NO_WAIT 
#elif defined (VHWA_PDK)
    #define SYSTEMP_WAIT_FOREVER    SemaphoreP_WAIT_FOREVER
    #define SYSTEMP_SUCCESS         SemaphoreP_OK
    #define SYSTEMP_FAILURE         SemaphoreP_FAILURE
    #define SYSTEMP_TIMEOUT         SemaphoreP_TIMEOUT
    #define SYSTEMP_NO_WAIT         SemaphoreP_NO_WAIT
    #define CacheP_TYPE_L1D         0U
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#if defined(VHWA_MCU_PLUS_SDK)
    typedef Udma_ChObjectInt            VhwaAl_Udma_Ch_Struct;
    typedef Udma_EventObjectInt         VhwaAl_Udma_Event_Struct;
    typedef SemaphoreP_Object           VhwaAl_SemaphoreP_Struct;
    typedef HwiP_Object                 VhwaAl_HwiP_Struct;
#elif defined (VHWA_PDK)
    typedef struct Udma_ChObj           VhwaAl_Udma_Ch_Struct;
    typedef struct Udma_EventObj        VhwaAl_Udma_Event_Struct;
    typedef SemaphoreP_Handle           VhwaAl_SemaphoreP_Struct;
    typedef HwiP_Handle                 VhwaAl_HwiP_Struct;
#endif

#ifdef __cplusplus
}
#endif


#endif  /* VHWA_DMPAC_ABSTRACTION_LAYER_H_ */