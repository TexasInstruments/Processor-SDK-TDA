/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#ifndef APP_HWA_H_
#define APP_HWA_H_

#include <stdint.h>

/**
 *  Remote service name to free/reallocate DMPAC SL2
 */
#define APP_VHWA_SERVICE_NAME  "com.ti.vhwa_remote_service"

/**
 *  Remote service name for VPAC configuration (firewall, etc.)
 */
#define APP_VHWA_VPAC_SERVICE_NAME  "com.ti.vhwa_vpac_remote_service"

/**
 *  Remote service command to free SL2 for SDE
 */
#define APP_DMPAC_SDE_SL2_FREE        (0x0001)
/**
 *  Remote service command to free SL2 for DOF
 */
#define APP_DMPAC_DOF_SL2_FREE        (0x0002)
/**
 *  Remote service command to reallocate SL2 for SDE
 */
#define APP_DMPAC_SDE_SL2_REALLOC     (0x0003)
/**
 *  Remote service command to reallocate SL2 for DOF
 */
#define APP_DMPAC_DOF_SL2_REALLOC     (0x0004)
/**
 *  Remote service command to set VPAC to 720 MHz and DMPAC to 480 MHz
 */
#define APP_VPAC_720_DMPAC_480        (0x0005)
/**
 *  Remote service command to set VPAC to 650 MHz and DMPAC to 520 MHz
 */
#define APP_VPAC_650_DMPAC_520        (0x0006)

/**
 *  Remote service command to set enable VPAC firewall
 */
#define APP_VPAC_ENABLE_FIREWALL        (0x0007)

/**
 *  Remote service command to set enable DMPAC firewall
 */
#define APP_DMPAC_ENABLE_FIREWALL        (0x0008)

/**
 *  Remote service command to disable VPAC firewall
 */
#define APP_VPAC_DISABLE_FIREWALL        (0x0009)

/**
 *  Remote service command to disable DMPAC firewall
 */
#define APP_DMPAC_DISABLE_FIREWALL       (0x000A)

/**
 *  Remote service command to Run LBIST on VPAC0
 */
#define APP_VPAC_ACQUIRE_LOCK         (0x000B)

/**
 *  Remote service command to release lock on VPAC. It expects VPAC instance id as an argument.
 *  APP_VHWA_VPAC0_INST: VPAC0
 *  APP_VHWA_VPAC1_INST: VPAC1
 *  Any other number returns error.
 */
#define APP_VPAC_RELEASE_LOCK         (0x000C)

/**
 *  Remote service command to acquire lock on DMPAC.
 */
#define APP_DMPAC_ACQUIRE_LOCK        (0x000D)

/**
 *  Remote service command to release lock on DMPAC.
 */
#define APP_DMPAC_RELEASE_LOCK         (0x000E)

/**
 *  Identical to SDE_DEFAULT_DIS_BUFF_DEPTH
 */
#define APP_SDE_DEFAULT_DIS_BUFF_DEPTH          (2U)
/**
 *  Identical to SDE_SR_192
 */
#define APP_SDE_SR_192                          (2U)
/**
 *  Identical to FVID2_CCSF_BITS12_UNPACKED16
 */
#define APP_FVID2_CCSF_BITS12_UNPACKED16        ((uint32_t) 0x11U)


/**
 *  Identical to DOF_DEFAULT_REF_BUFF_DEPTH
 */
#define APP_DOF_DEFAULT_REF_BUFF_DEPTH          (2U)

/**
 *  Identical to DOF_DEFAULT_CUR_BUFF_DEPTH
 */
#define APP_DOF_DEFAULT_CUR_BUFF_DEPTH          (2U)

/**
 *  Identical to DOF_DEFAULT_FV_BUFF_DEPTH
 */
#define APP_DOF_DEFAULT_FV_BUFF_DEPTH           (4U)

/**
 *  VPAC Instance id to be used with APP_VPAC_ACQUIRE_LOCK and APP_VPAC_RELEASE_LOCK commands
 */
#define APP_VHWA_VPAC0_INST                     (0x2001)
/**
 *  VPAC Instance id to be used with APP_VPAC_ACQUIRE_LOCK and APP_VPAC_RELEASE_LOCK commands
 */
#define APP_VHWA_VPAC1_INST                     (0x2002)
/**
 *  DMPAC Instance id to be used with APP_DMPAC_ACQUIRE_LOCK and APP_DMPAC_RELEASE_LOCK commands
 */
#define APP_VHWA_DMPAC_INST                     (0x2003)

/**
 *  struct App_m2mSdeSl2AllocPrms
 *  \brief Init Parameters required to allocate SDE driver.
 *         This should be identical to Vhwa_m2mSdeSl2AllocPrms.
 *         It is defined locally for application to use the same params
 */
typedef struct
{
    /** These parameters are used by the driver to allocater SL2 memory, used
     *  all driver handles.
     *  Ensure to allocate max of all handles requirement.
     */
    uint32_t        maxImgWidth;
    /**< Maximun image width */
    uint32_t        inCcsf;
    /**< CCSF for input image */
    uint32_t        searchRange;
    /**< Search range refer to \ref SDE_DispSearchRange */
    uint32_t        disBuffDepth;
    /**< Maximum disparity buffer depth */
} App_M2mSdeSl2AllocPrms;


/**
 *  struct App_m2mDofSl2AllocPrms
 *  \brief Init Parameters required to allocate DOF driver
 *         This should be identical to Vhwa_m2mDofSl2AllocPrms.
 *         It is defined locally for application to use the same params
 */
typedef struct
{
    /** These parameters are used by the driver to allocater SL2 memory, used
     *  all driver handles.
     *  Ensure to allocate max of all handles requirement.
     */
    uint32_t        maxImgWidth;
    /**< Maximun image width */
    uint32_t        inCcsf;
    /**< CCSF for input image */
    uint32_t        refBuffDepth;
    /**< Input buffer depth in SL2 for reference image, final depth will be
         calculated based on tob and bottom searc range.
         Minumum configurable depth is 2 */
    uint32_t        currBuffDepth;
    /**< Input buffer depth in SL2 for Current image
         Minumum configurable depth is 2 */
    uint32_t        fvBuffDepth;
    /**< Flow vector buffer depth in Sl2 */
    uint32_t        topSR;
    /**< Top search range */
    uint32_t        botSR;
    /**< Bottom search range */
} App_M2mDofSl2AllocPrms;


/**
 * \defgroup group_vision_apps_utils_hwa HW accelerator initialization APIs (RTOS only)
 *
 * \brief This section contains APIs for HW accelerator initialization
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/** \brief Performs initializations needed for Fvid2 must be called before
 *         capture, display and other HWA init
 *
 */
int32_t appFvid2Init(void);

/** \brief Performs initializations needed for CSI2RX driver
 *
 */
int32_t appCsi2RxInit(void);

/** \brief Performs initializations needed for CSI2TX driver
 *
 */
int32_t appCsi2TxInit(void);

/** \brief Performs initializations needed for VHWA drivers
 *
 */
int32_t appVhwaVpacInit(uint32_t vpacInst);
int32_t appVhwaDmpacInit(void);

/** \brief Performs de-initializations needed for CSI2RX driver
 *
 */
int32_t appCsi2RxDeInit(void);

/** \brief Performs de-initializations needed for CSI2TX driver
 *
 */
int32_t appCsi2TxDeInit(void);

/** \brief Performs de-initializations needed for Fvid2
 *
 */
int32_t appFvid2DeInit(void);


/** \brief Performs de-initializations needed for VHWA drivers
 *
 */
int32_t appVhwaVpacDeInit(void);
int32_t appVhwaDmpacDeInit(void);

/** \brief Performs registration of remote service for DMPAC (SL2, etc.)
 *
 */
int32_t appVhwaRemoteServiceInit(void);

/** \brief Performs unregistration of remote service for DMPAC
 *
 */
int32_t appVhwaRemoteServiceDeInit(void);

/** \brief Performs registration of remote service for VPAC (firewall, etc.)
 *
 */
int32_t appVhwaVpacRemoteServiceInit(void);

/** \brief Performs unregistration of remote service for VPAC
 *
 */
int32_t appVhwaVpacRemoteServiceDeInit(void);

/** \brief Performs registration of remote service for VISS node
 *
 */
int32_t appVissRemoteServiceInit(void);

/** \brief Performs unregistration of remote service for VISS node
 *
 */
int32_t appVissRemoteServiceDeInit(void);

/* @} */

#endif

