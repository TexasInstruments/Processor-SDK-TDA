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

#ifndef APP_DSS_H_
#define APP_DSS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \defgroup group_vision_apps_utils_dss Display subsystem initialization APIs (RTOS only)
 *
 * \brief This section contains APIs for display initialization
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/**
 *  \anchor App_DssCommRegId
 *  \name   DSS Common Region Id
 *  \brief  Id for different Common Regions
 *
 *  @{
 */
/** \brief Common Region 0 i.e. M region */
#define APP_DSS_COMM_REG_ID_0                       ((uint32_t) 0x0U)
/** \brief Common Region 1 i.e. S0 region */
#define APP_DSS_COMM_REG_ID_1                       ((uint32_t) 0x1U)
/** \brief Common Region 2 i.e. S1 region */
#define APP_DSS_COMM_REG_ID_2                       ((uint32_t) 0x2U)
/** \brief Common Region 3 i.e. S2 region */
#define APP_DSS_COMM_REG_ID_3                       ((uint32_t) 0x3U)
/** \brief Common Region Max Id */
#define APP_DSS_COMM_REG_ID_MAX                     ((uint32_t) 0x4U)
/* @} */

/**
 *  \anchor App_DssVidPipeId
 *  \name   DSS Video Pipeline Id
 *  \brief  Id for different Video Pipelines
 *
 *  @{
 */
/** \brief Video Pipeline 1 */
#define APP_DSS_VID_PIPE_ID_VID1                    ((uint32_t) 0x0U)
/** \brief Video Lite Pipeline 1 */
#define APP_DSS_VID_PIPE_ID_VIDL1                   ((uint32_t) 0x1U)
/** \brief Video Pipeline 2 */
#define APP_DSS_VID_PIPE_ID_VID2                    ((uint32_t) 0x2U)
/** \brief Video Lite Pipeline 2 */
#define APP_DSS_VID_PIPE_ID_VIDL2                   ((uint32_t) 0x3U)
/** \brief Video Pipeline Max Id */
#define APP_DSS_VID_PIPE_ID_MAX                     ((uint32_t) 0x4U)
/* @} */

/**
 *  \anchor App_DssOverlayId
 *  \name   DSS Overlay Id
 *  \brief  Id for DSS Overlays
 *
 *  @{
 */
/** \brief Overlay 1 */
#define APP_DSS_OVERLAY_ID_1                        ((uint32_t) 0x0U)
/** \brief Overlay 2 */
#define APP_DSS_OVERLAY_ID_2                        ((uint32_t) 0x1U)
/** \brief Overlay 3 */
#define APP_DSS_OVERLAY_ID_3                        ((uint32_t) 0x2U)
/** \brief Overlay 4 */
#define APP_DSS_OVERLAY_ID_4                        ((uint32_t) 0x3U)
/** \brief Overlay Max Id */
#define APP_DSS_OVERLAY_ID_MAX                      ((uint32_t) 0x4U)
/* @} */

/**
 *  \anchor App_DssVpId
 *  \name   DSS Video Port Id
 *  \brief  Id for DSS Video Ports
 *
 *  @{
 */
/** \brief Video Port 1 */
#define APP_DSS_VP_ID_1                             ((uint32_t) 0x0U)
/** \brief Video Port 2 */
#define APP_DSS_VP_ID_2                             ((uint32_t) 0x1U)
/** \brief Video Port 3 */
#define APP_DSS_VP_ID_3                             ((uint32_t) 0x2U)
/** \brief Video Port 4 */
#define APP_DSS_VP_ID_4                             ((uint32_t) 0x3U)
/** \brief Video Port Max Id */
#define APP_DSS_VP_ID_MAX                           ((uint32_t) 0x4U)
/* @} */

/**
 *  \anchor App_DssCscRange
 *  \name   DSS CSC Range
 *  \brief  DSS Color Space Conversion range setting
 *
 *  @{
 */
/** \brief Limited range selected */
#define APP_DSS_CSC_RANGE_LIMITED                   ((uint32_t) 0x0U)
/** \brief Full range selected */
#define APP_DSS_CSC_RANGE_FULL                      ((uint32_t) 0x1U)
/* @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Structure containing DSS resources information. This enables
 *         display sharing between two different softwares.
 */
typedef struct
{
    uint32_t isPipeAvailable[APP_DSS_VID_PIPE_ID_MAX];
    /**< Ids for available video pipes */
    uint32_t isOverlayAvailable[APP_DSS_OVERLAY_ID_MAX];
    /**< Ids for available overlays */
    uint32_t isPortAvailable[APP_DSS_VP_ID_MAX];
    /**< Ids for available video ports */
    uint32_t isDpAvailable;
    /**< Is display port available */
    uint32_t isDsiAvailable;
    /**< Is DSI available */

} app_dss_init_params_t;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 *  \brief app_dss_init_params_t structure init function.
 *
 *  \param  initParams  [IN] Pointer to #app_dss_init_params_t structure.
 *
 *  \return None
 */
static inline void appDssInitParamsInit(app_dss_init_params_t *initParams);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief DSS initialization function.
 *
 *  \param  dssParams  [IN] Pointer to #app_dss_init_params_t structure.
 *
 *  \return 0 on success else failure
 */
int32_t appDssInit(app_dss_init_params_t *dssParams);

/**
 *  \brief DSS de-initialization function.
 */
int32_t appDssDeInit(void);


/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void appDssInitParamsInit(app_dss_init_params_t *initParams)
{
    uint32_t i;
    if(NULL != initParams)
    {
        for(i=0U; i<APP_DSS_VID_PIPE_ID_MAX ; i++)
        {
            initParams->isPipeAvailable[i] = false;
        }
        for(i=0U; i<APP_DSS_OVERLAY_ID_MAX ; i++)
        {
            initParams->isOverlayAvailable[i] = false;
        }
        for(i=0U; i<APP_DSS_VP_ID_MAX ; i++)
        {
            initParams->isPortAvailable[i] = false;
        }
        initParams->isDpAvailable = false;
        initParams->isDsiAvailable = false;
    }
}

/* @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef APP_DSS_H_ */
