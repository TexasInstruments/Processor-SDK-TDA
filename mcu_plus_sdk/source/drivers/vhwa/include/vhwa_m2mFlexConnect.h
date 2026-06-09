/*
 *   Copyright (c) Texas Instruments Incorporated 2021
 *   All rights reserved.
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
/**
 *  \ingroup  DRV_VHWA_MODULE
 *  \defgroup DRV_FlexConnect_MODULE Flex-connect Module
 *            VHWA Flex-connect Module
 *
 */
/**
 *  \ingroup  DRV_FlexConnect_MODULE
 *  \defgroup DRV_FlexConnect_MODULE_INTERFACE Flex-connect Interface
 *            Interface file for Flex-connect FVID2 driver
 *
 *  @{
 */
/**
 *  \file vhwa_m2mFlexConnect.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *              configure / control vhwa_m2mFlexConnect
 *
 *          Typical Application for the FC driver is as shown below.
 *
 *          Vhwa_FCDrvInit
 *               ||
 *          FVID2_Create
 *               ||
 *          Configure nodes
 *               ||
 *          FVID2_ProcessReq
 *               ||
 *          Wait for completion callback
 *               ||
 *          FVID2_GetProcessedReq
 *               ||
 *          FVID2_close
 *               ||
 *          FVID2_deInit
 */


#ifndef VHWA_FC_DRV_H_
#define VHWA_FC_DRV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "include/flexConnect_cfg.h"
#include "include/vhwa_m2mMsc.h"
#include "include/vhwa_m2mViss.h"
#include <include/vhwa_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Flexconnect Driver instance ID */

#if defined (SOC_J784S4) || defined (SOC_J742S2)
    /** \brief VPAC 0 FC Instance 0  */
    #define VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0             (0U)
    /** \brief VPAC 1 FC Instance 0  */
    #define VHWA_M2M_VPAC_1_FC_DRV_INST_ID_0             (1U)
    /** \brief Total number of FC instances */
    #define VHWA_M2M_FC_DRV_NUM_INST                     (VHWA_M2M_VPAC_1_FC_DRV_INST_ID_0 + 1U)
#else
    /** \brief VPAC 0 FC Instance 0  */
    #define VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0             (0U)
    /** \brief Total number of FC instances */
    #define VHWA_M2M_FC_DRV_NUM_INST                     (VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0 + 1U)
#endif
/* @} */

/**
 *  \brief Default FC Driver Instance ID [For backward compatibility]
 */
#define VHWA_M2M_FC_DRV_INST0                 (VHWA_M2M_VPAC_0_FC_DRV_INST_ID_0)

/** \brief Max Number of Flexconnect Driver handles */
#define VHWA_FC_DRV_MAX_HANDLES                 (4u)

/** \brief Max Number of Flexconnect Nodes */
#define VHWA_FC_MAX_NODE_INST                   (VHWA_FC_MAX_NODES)

/** \brief Max Number of Flexconnect edges */
#define VHWA_FC_MAX_EDGES                       (30u)

/**
 *  \anchor VhwaFcDrv_Ioctls
 *  \name   Ioctls for the Flex-Connect driver
 *  \brief  Input/Output control MACRO's for Flex-Connect driver module
 *
 *  @{
 */

/**
 * \brief IOCTL for setting VPAC Flex-Connect configuration
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_SET_GRAPH     (VHWA_IOCTL_FC_IOCTL_BASE)

/**
 * \brief IOCTL for setting VPAC Flex-Connect path configuration
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_SET_CONFIG    (IOCTL_VHWA_FC_SET_GRAPH + 1u)

/**
 * \brief IOCTL for deletting  VPAC Flex-Connect configuration
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_DELETE_GRAPH  (IOCTL_VHWA_FC_SET_CONFIG + 1u)

/**
 * \brief IOCTL for enabling error events and
 *        registering callbacks for the same.
 *        This IOCTL pointer to FC_ErrEventParams as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_REGISTER_ERR_CB (IOCTL_VHWA_FC_DELETE_GRAPH + 1u)

/**
 * \brief IOCTL for getting the FVID2 handles for VHWA  modules
 *        This IOCTL requires pointer to #Vhwa_M2mFcModuleHdls as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_GET_HANDLES (IOCTL_VHWA_FC_REGISTER_ERR_CB + 1u)

/**
 * \brief IOCTL for getting FC Processing attributes
 *        for the last frame submitted.
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_FC_GET_PERFORMANCE (IOCTL_VHWA_FC_GET_HANDLES + 1u)

/** @} */

/** \brief VISS Source buffert start index for Input process frame list */
#define VHWA_FC_VISS_SRC_BUFF_IDX_START             (0U)
/** \brief MSC0 Source buffert start index for Input process frame list */
#define VHWA_FC_MSC0_SRC_BUFF_IDX_START             (3U)
/** \brief MSC1 Source buffert start index for Input process frame list */
#define VHWA_FC_MSC1_SRC_BUFF_IDX_START             (5U)

/** \brief Total Source buffert for Input process frame list */
#define VHWA_FC_TOTAL_SRC_BUFF                      (7U)

/** \brief VISS Output buffert start index for Output process frame list */
#define VHWA_FC_VISS_DST_BUFF_IDX_START             (0U)
/** \brief MSC0 Output buffert start index for Output process frame list */
#define VHWA_FC_MSC0_DST_BUFF_IDX_START             (8U)
/** \brief MSC1 Output buffert start index for Output process frame list */
#define VHWA_FC_MSC1_DST_BUFF_IDX_START             (18U)
/** \brief Total Output buffert for Output process frame list */
#define VHWA_FC_TOTAL_DST_BUFF                      (18U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Structure containing edge information. Edge is a connection
 *  between two nodes i.e. two modules. Video Hardware can be represented
 *  by a Flex-Connect, where each module is node and edge is present between two
 *  nodes if they are connected.
 */
typedef struct
{
    uint64_t startPort;
    /**< Starting node of the edge */
    uint64_t endPort;
    /**< End node of the edge */
} Vhwa_M2mFcEdgeInfo;

/**
 *  struct Vhwa_M2mFcGraphPathInfo
 *  \brief Structure contaning the VHWA FC path info.
 *         This contains array of edge connected in the FC
 */
typedef struct
{
    uint32_t numEdges;
    /**< Number edge in the edgeInfo array */
    Vhwa_M2mFcEdgeInfo edgeInfo[VHWA_FC_MAX_EDGES];
    /**< List of edges connecting VHWA Flex-Connect. Driver parses these
     *   edges and enables/disables input/output path in the appropriate VPAC
     *   module. This edge tells which module is connected to which module
     *   enabling output in edge start module and input in edge end module. */
} Vhwa_M2mFcGraphPathInfo;

/**
 *  struct Vhwa_FCDrvInitPrms
 *  \brief Structure containing parameters to initialize the Flex-Connect driver
 */
typedef struct
{
    Udma_DrvHandle              udmaDrvHndl;
    /**< UDMA driver handle, used for configuring UTC */

} Vhwa_M2mFcInitPrms;

/**
 *  struct Vhwa_M2mFcCreatePrms
 *  \brief Create Parameters for vhwa Flex-Connect
 */
typedef struct
{
    Vhwa_GetTimeStamp  getTimeStamp;
    /**< Function pointer to get timestamp */
    uint32_t                    enablePsa;
    /**< Flag to enable/Disble LSE PSA Module */
} Vhwa_M2mFcCreatePrms;

/**
 *  struct Vhwa_M2mFcSl2AllocPrms
 *  \brief Used for allocating SL2 memory
 */
typedef struct
{
    Vhwa_M2mMscSl2AllocPrms mscSl2Prms;
    Vhwa_M2mVissSl2Params   vissSl2Prms;
} Vhwa_M2mFcSl2AllocPrms;

/**
 *  struct Vhwa_M2mFcPerf
 *  \brief Structure for storing the performance numbers
 */
typedef struct
{
    uint32_t per;
    /**< Time taken to process FC */
} Vhwa_M2mFcPerf;

/**
 *  struct Vhwa_M2mFcModuleHdls
 *  \brief Sturcuture for getting the Module handles
 */
typedef struct
{
    Fvid2_Handle                vissHandle;
    /**< FVID2 Handle for VISS opened and used by driver */
    Fvid2_Handle                msc0Handle;
    /**< FVID2 Handle for MSC0 opened and used by driver */
    Fvid2_Handle                msc1Handle;
    /**< FVID2 Handle for MSC1 opened and used by driver */
} Vhwa_M2mFcModuleHdls;

typedef struct
{
    uint32_t mscInstId;

    Ptr cmdArgs;
}Vhwa_M2mFcMscArgs;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initializes the VHWA Flex-Connect
 *
 *  \param fcInitPrms       Pointer to Flex-Connect structure
 *                          #Vhwa_M2mFcInitPrms. This parameter should not be 0.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t Vhwa_m2mFcInit(const Vhwa_M2mFcInitPrms *initPrms);

/**
 *  \brief DeInitializes vhwa Flex-Connect
 *
 */
void Vhwa_m2mFcDeInit(void);

/**
 *  \brief Function to allocate Sl2 memory for the buffers.
 *
 *  \param sl2allocPrms Pointer to a #Vhwa_M2mFcSl2AllocPrms structure
 *                      containing the SL2 allocation parameters
 *
 *  \return FVID2_SOK if successful, else suitable error code
 */
int32_t Vhwa_m2mFcAllocSl2(const Vhwa_M2mFcSl2AllocPrms *sl2AllocPrms);

/**
 *  \brief Function to free allocated SL2.
 *
 */
void Vhwa_m2mFCDrvFreeSl2(void);


/**
 *  \brief Vhwa_M2mFcInitPrms structure init function.
 *
 *  \param  initPrms  [IN]Pointer to #Vhwa_M2mFcInitPrms structure.
 *
 *  \return None
 */
static inline void Vhwa_m2mFcInitPrmsInit(Vhwa_M2mFcInitPrms *initPrms);

/**
 *  \brief Vhwa_M2mFcCreatePrms structure init function.
 *
 *  \param  initPrms  [IN]Pointer to #Vhwa_M2mFcCreatePrms structure.
 *
 *  \return None
 */
static inline void Vhwa_m2mFcDrvCreatePrmsInit(Vhwa_M2mFcCreatePrms *createPrms);

/**
 *  \brief Vhwa_M2mFcSl2AllocPrms structure init function.
 *
 *  \param  initPrms  [IN]Pointer to #Vhwa_M2mFcSl2AllocPrms structure.
 *
 *  \return None
 */
static inline void Vhwa_m2mFcDrvSl2AllocPrmsInit(Vhwa_M2mFcSl2AllocPrms *sl2AllocPrms);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static inline void Vhwa_m2mFcInitPrmsInit(Vhwa_M2mFcInitPrms *initPrms)
{
    if (NULL != initPrms)
    {
        initPrms->udmaDrvHndl = NULL;
    }
}

static inline void Vhwa_m2mFcDrvCreatePrmsInit(Vhwa_M2mFcCreatePrms *createPrms)
{
    if (NULL != createPrms)
    {
        createPrms->getTimeStamp = NULL;
    }
}

static inline void Vhwa_m2mFcDrvSl2AllocPrmsInit(
                                        Vhwa_M2mFcSl2AllocPrms *sl2AllocPrms)
{
    if (NULL != sl2AllocPrms)
    {
        Vhwa_m2mVissSl2ParamsInit(&sl2AllocPrms->vissSl2Prms);
        /* Update the Saturation output width of VISS to 0 */
        sl2AllocPrms->vissSl2Prms.maxOutWidth[VHWA_M2M_VISS_OUT_SATURA_8B_IDX] = 0U;

        Vhwa_M2mMscSl2AllocPrmsInit(&sl2AllocPrms->mscSl2Prms);

        /* Update MSC SL2 input buffer depth */
        uint32_t cnt, idx;

        for(cnt = 0; cnt < VHWA_M2M_MSC_MAX_INST; cnt++)
        {
            for(idx = 0; idx < VHWA_M2M_MSC_MAX_IN_CHANNEL; idx++)
            {
                sl2AllocPrms->mscSl2Prms.inBuffDepth[cnt][idx] = 8U;
            }
        }
    }

}


#ifdef __cplusplus
}
#endif

#endif /* VHWA_FC_DRV_H_ */

/** @} */
