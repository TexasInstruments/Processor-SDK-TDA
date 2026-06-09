/*
 *   Copyright (c) Texas Instruments Incorporated 2019
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
 *  \defgroup DRV_NF_MODULE NF Module
 *            NF Module
 *
 */
/**
 *  \ingroup  DRV_NF_MODULE
 *  \defgroup DRV_NF_MODULE_INTERFACE NF Interface
 *            Interface file for NF M2M FVID2 driver
 *
 *  @{
 */
/**
 *  \file vhwa_m2mNf.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *              configure / control NF M2M driver
 *
 *          Typical Application for the NF M2M driver is as shown below.
 *
 *          Vhwa_m2mNfInit
 *               ||
 *          Vhwa_m2mNfAllocSl2
 *               ||
 *          FVID2_Create
 *               ||
 *          IOCTL_VHWA_M2M_NF_SET_CONFIG (sets NF configuration)
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


#ifndef VHWA_M2M_NF_H_
#define VHWA_M2M_NF_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "include/nf_cfg.h"
#include <include/vhwa_common.h>
#include <soc/vhwa_soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor VhwaM2mNf_InstanceId
 *  \name VHWA NF Instance ID 
 *
 *  VHWA VPAC NF Instance ID's
 *
 *  @{
 */
#if defined (SOC_J784S4) || defined (SOC_J742S2)
    /** \brief VPAC 0 NF Instance 0  */
    #define VHWA_M2M_VPAC_0_NF_DRV_INST_ID_0             (0U)
    /** \brief VPAC 1 NF Instance 0  */
    #define VHWA_M2M_VPAC_1_NF_DRV_INST_ID_0             (1U)
    /** \brief Total number of NF instances */
    #define VHWA_M2M_NF_DRV_NUM_INST                     (VHWA_M2M_VPAC_1_NF_DRV_INST_ID_0 + 1U)
#else
    /** \brief VPAC 0 NF Instance 0  */
    #define VHWA_M2M_VPAC_0_NF_DRV_INST_ID_0             (0U)
    /** \brief Total number of NF instances */
    #define VHWA_M2M_NF_DRV_NUM_INST                     (VHWA_M2M_VPAC_0_NF_DRV_INST_ID_0 + 1U)
#endif
/* @} */

/**< Default Instance ID for the NF M2m driver  [For backward compatibility] */
#define VHWA_M2M_NF_DRV_INST_ID            (VHWA_M2M_VPAC_0_NF_DRV_INST_ID_0)

/** \brief Max Number of handles supported for each instance of NF M2M Driver */
#define VHWA_M2M_NF_MAX_HANDLES            (4u)

/** \brief Minimum supported depth value for read buffer in VHWA memory*/
#define VHWA_M2M_NF_MIN_RD_BUFFER_DEPTH    (6U)
/** \brief Minimum supported depth value for write buffer in VHWA memory */
#define VHWA_M2M_NF_MIN_WR_BUFFER_DEPTH    (2U)

/** \brief Maximum possible NF iteration for a simgle request (Luma + Chroma) */
#define VHWA_M2M_NF_MAX_COMP               (2u)

/** \brief Default width - Used to initialize configuration parameters */
#define VHWA_NF_DEF_WIDTH                      (1920U)
/** \brief Default Height - Used to initialize configuration parameters */
#define VHWA_NF_DEF_HEIGHT                     (1080U)


/**
 *  \anchor VhwaM2MNf_Ioctls
 *  \name   Ioctls for the NF memory to memory driver
 *  \brief  Input/Output control MACRO's for NF memory to memory module
 *
 *  @{
 */
/**
 * \brief IOCTL for setting NF configuration.
 *        This IOCTL takes NF Configuration #Nf_Config as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_SET_PARAMS       (VHWA_IOCTL_M2M_NF_IOCTL_BASE)

/**
 * \brief IOCTL for setting the filter coefficients
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF  (IOCTL_VHWA_M2M_NF_SET_PARAMS + 1U)

/**
 * \brief IOCTL for getting module's performance numbers
 *        for the last frame submitted.
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_GET_PERFORMANCE  (                                  \
            IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF + 1U)

/**
 * \brief IOCTL for getting LSE PSA Signature
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_GET_PSA_SIGN     (                                  \
    IOCTL_VHWA_M2M_NF_GET_PERFORMANCE + 1U)

/**
 * \brief IOCTL for enabling error events and
 *        registering callbacks for the same.
 *        This IOCTL pointer to #Nf_ErrEventParams as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB  (IOCTL_VHWA_M2M_NF_GET_PSA_SIGN + \
    1U)

/**
 * \brief IOCTL for enabling and setting HTS limiter.
 *        This is used to slowed down the HTS by introducing clock cycles
 *        between internal start signals.
 *        This IOCTL takes pointer to #Vhwa_HtsLimiter as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT    (                                  \
                            IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB + 1U)

/**
 * \brief IOCTL to sync start each module
 *        This IOCTL doesn't configure any register and only enable pipeline
 *        to start processing
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_NF_SYNC_START    (                                  \
                            IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT + 1U)

/**
 * \brief IOCTL for enabling Watchdog timer error events and registering
 *         callbacks for the same.
 */
#define VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB      \
                                                    (IOCTL_VHWA_M2M_NF_SYNC_START + 1U)

/**
 * \brief IOCTL for enabling/disabling periodic reconfiguration of NF registers.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the periodic reconfiguration of NF registers.
 */
/** \brief IOCTL for enabling/disabling periodic reconfiguration of NF regs */
#define VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG        \
                                    (VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB + 1U)

/**
 * \brief IOCTL for enabling/disabling status register validation.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the status register validation feature.
 */
/** \brief IOCTL for validating the status register */
#define VHWA_M2M_IOCTL_NF_ENABLE_STATUS_REG_VALIDATE        \
                                    (VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG + 1U)

/**
 * \brief IOCTL command to retrieve the validation status of the status register.
 *
 * This IOCTL can be used to trigger the status check after frame process completion and fetch 
 * the register validation current state or outcome.
 *
 */
/** \brief IOCTL for getting the status of status register validation */
#define VHWA_M2M_IOCTL_NF_VALIDATE_REG    \
                                    (VHWA_M2M_IOCTL_NF_ENABLE_STATUS_REG_VALIDATE + 1U)

/**
 * \brief IOCTL for enabling/disabling configuration register validation.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the configuration register validation feature.
 */
/** \brief IOCTL for validating the configuration register */
#define VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK        \
                                    (VHWA_M2M_IOCTL_NF_VALIDATE_REG + 1U)

/**
 * \brief IOCTL command to get the size required for configuration register readback.
 *
 * This IOCTL returns the size (in bytes) needed to store the configuration register readback data.
 */
/** \brief IOCTL for getting the size of configuration register readback */
#define VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE    \
                                    (VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK + 1U)

/**
 * \brief IOCTL for setting the configuration register readback and golden pointers, and callback.
 *
 * This IOCTL allows the application to provide pointers to the configuration register readback buffer,
 * the golden reference buffer, and a callback function. The driver will use these pointers to store
 * the configuration register readback data and compare it with the golden reference after processing.
 * The argument should be a pointer to a Nf_ConfigRegMemParams structure, where:
 *   - readback_ptr: Pointer to the buffer for configuration register readback data (must not be NULL).
 *   - golden_ref_ptr: Pointer to the buffer containing the golden reference data (must not be NULL).
 *   - cbFxn: Callback function to be called after comparison (must not be NULL).
 * The buffer size for readback_ptr should be at least as large as returned by VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE.
 */
/** \brief IOCTL for setting configuration register readback pointer */
#define VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS    \
                                    (VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE + 1U)


/** @} */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  struct Vhwa_m2mNfConfig
 *  \brief Connfiguration parameters of NF.
 */
typedef struct
{
    Fvid2_Format        inFmt;
    /**< Input Frame format, describing input frames storage format
         Following parametes are used from this
         width = width of the input frame
         height = height of the input frame
         pitch = line offset for the input data, used based on the data format
         dataFormat = Input DataFormat
         sfcc = Storage format */
    Fvid2_Format        outFmt;
    /**< Input Frame format, describing input frames storage format
         Following parametes are used from this
         width = width of the output frame,
                 must be same or half of, input frame width
         height = height of the input frame
                  must be same or half of, input frame height
         pitch = line offset for the input data, used based on the data format
         dataFormat = DataFormat, must be same as input data format
         sfcc = Storage format */
    Nf_Config           nfCfg;
    /**< Noise Filter configuration */
} Vhwa_M2mNfConfig;

/**
 *  struct Vhwa_M2mNfCreatePrms
 *  \brief Create Parameters for NF M2M Driver
 */
typedef struct
{
    Vhwa_GetTimeStamp  getTimeStamp;
    /**< Function pointer to get timestamp */

    uint32_t                    enablePsa;
    /**< Flag to enable/Disble LSE PSA Module */
} Vhwa_M2mNfCreatePrms;

/**
 *  struct Vhwa_M2mNfInitPrms
 *  \brief Init Parameters for NF M2M Driver
 */
typedef struct
{
    Udma_DrvHandle              udmaDrvHndl;
    /**< UDMA driver handle, used for configuring UTC */
    Vhwa_IrqInfo                irqInfo;
    /**< IRQ Information */
} Vhwa_M2mNfInitPrms;

/**
 *  struct Vhwa_m2mNfSl2AllocPrms
 *  \brief Used for allocating SL2 memory
 */
typedef struct
{
    /** These parameters are used by the driver to allocate memory, used
     *  by all handles
     *  Ensure to allocate max of all handles requirement.
     */
    uint32_t        maxImgWidth;
    /**< Input Inage Width  */
    uint32_t        inCcsf;
    /**< Input storage format  */
    uint32_t        inBuffDepth;
    /**< Input buffer depth in SL2  */
    uint32_t        outBuffDepth;
    /**< Output buffer depth in SL2  */
    uint32_t        outCcsf;
    /**< Input storage format  */
    uint32_t        enable16KBAlignment;
    /**< Enable 16KB alignment for SL2 allocation
     *   TRUE: Allocate with 16KB address and size alignment (required for firewall)
     *   FALSE: Use default alignment without firewall support */
} Vhwa_M2mNfSl2AllocPrms;

/**
 *  struct Vhwa_m2mNfPerf
 *  \brief Structure for storing the performance numbers
 */
typedef struct
{
    uint32_t perf[VHWA_M2M_NF_MAX_COMP];
    /**< Time taken to process image */
    uint32_t swOvhd[VHWA_M2M_NF_MAX_COMP];
    /**< Software overhead - Time take by drvier for configuration before
         initiating scaling operation */
} Vhwa_M2mNfPerf;

/**
 *  struct Vhwa_m2mNfPsaSign
 *  \brief Structure for storing PSA signatures
 */
typedef struct
{
    uint32_t psaSign[VHWA_M2M_NF_MAX_COMP];
    /**< LSE PSA signature for all output channels */
} Vhwa_M2mNfPsaSign;

/**
 *  struct Vhwa_M2mNfConfigRegMemParams
 *  \brief Structure for configuration register readback and golden reference comparison for NF.
 */
typedef struct
{
    void *golden_ref_ptr;
    /**< Pointer to the buffer containing the golden reference data (must not be NULL) */

    void *readback_ptr;
    /**< Pointer to the buffer for configuration register readback data (must not be NULL) */

    Nf_ConfigRegCompareCbFxn cbFxn;
    /**< Callback function to be called after comparison (must not be NULL).
         The callback receives the handle and the comparison result. */

    uint32_t readback_mem_size;
    /**< Size of the readback buffer in bytes */

    void *appData;
    /**< Application-specific data pointer, passed to the callback */
} Vhwa_M2mNfConfigRegMemParams;

/**
 *  struct Vhwa_M2mNfSl2Info
 *  \brief Structure to hold NF SL2 memory information
 */
typedef struct
{
    uint64_t    sl2Addr;
    /**< Start address of allocated SL2 memory */
    uint32_t    sl2Size;
    /**< Size of allocated SL2 memory in bytes */
} Vhwa_M2mNfSl2Info;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initializes NF Init Parameters
 *
 *  \param nfInitPrms       Pointer to NF initialization structure
 *                          #Vhwa_M2mNfInitPrms. This parameter should not be 0.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t Vhwa_m2mNfInit(Vhwa_M2mNfInitPrms *initPrms);

/**
 *  \brief DeInitializes NF Init Parameters
 *
 */
void Vhwa_m2mNfDeInit(void);

/**
 *  \brief Function to allocate Sl2 memory for the output buffers.
 *
 *  \param sl2allocPrms Pointer to a #Vhwa_M2mNfSl2AllocPrms structure
 *                      containing the SL2 allocation parameters
 *
 *  \return FVID2_SOK if successful, else suitable error code
 */
int32_t Vhwa_m2mNfAllocSl2(const Vhwa_M2mNfSl2AllocPrms *sl2AllocPrms);

/**
 *  \brief Function to free allocated SL2.
 *
 */
void Vhwa_m2mNfFreeSl2(void);

/**
 * \brief API for getting NF SL2 allocation information.
 *        This IOCTL retrieves the start address and size of allocated SL2 memory.
 *        Takes pointer to Vhwa_M2mNfSl2Info as output parameter.
 *
 * \param sl2Info [OUT] Pointer to structure to store SL2 info
 *
 * \return  FVID2_SOK if successful
 *          FVID2_EBADARGS if sl2Info is NULL
 *          FVID2_EFAIL if SL2 memory is not allocated
 */
int32_t Vhwa_m2mNfGetSl2Info(Vhwa_M2mNfSl2Info *sl2Info);

/**
 *  \brief This function should be used to initialize variable of type
 *         #Vhwa_M2mNfInitPrms.
 *
 *  \param ldcCfg   Pointer to structure of type Vhwa_M2mNfInitPrms
 *  \return         None
 */
static inline void Vhwa_m2mNfInitPrmsInit(Vhwa_M2mNfInitPrms *initPrms);

/**
 *  \brief This function should be used to initialize variable of type
 *          #Vhwa_M2mNfCreatePrms.
 *
 *  \param createArgs   Pointer to structure of type Vhwa_M2mNfCreatePrms
 *  \return             None
 */
static inline void Vhwa_M2mNfCreatePrmsInit(Vhwa_M2mNfCreatePrms *createArgs);

/**
 *  \brief This function should be used to initialize variable of type
 *          #Vhwa_M2mNfSl2AllocPrms.
 *
 *  \param sl2Prms      Pointer to structure of type Vhwa_M2mNfSl2AllocPrms
 *  \return             None
 */
static inline void Vhwa_M2mNfSl2AllocPrmsInit(Vhwa_M2mNfSl2AllocPrms *sl2Prms);

/**
 *  \brief This function should be used to initialize variable of type
 *          #Vhwa_M2mNfConfig.
 *
 *  \param nfCfg        Pointer to structure of type Vhwa_M2mNfConfig
 *  \return             None
 */
static inline void Vhwa_M2mNfConfigInit(Vhwa_M2mNfConfig *nfCfg);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static inline void Vhwa_m2mNfInitPrmsInit(Vhwa_M2mNfInitPrms *initPrms)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != initPrms)
    /* LDRA_JUSTIFY_END */
    {
        Nf_getIrqInfo(&initPrms->irqInfo);
        initPrms->udmaDrvHndl = NULL;
    }
}


static inline void Vhwa_M2mNfSl2AllocPrmsInit(Vhwa_M2mNfSl2AllocPrms *sl2Prms)
{
    if (NULL != sl2Prms)
    {
        sl2Prms->maxImgWidth = VHWA_NF_DEF_WIDTH;
        sl2Prms->inCcsf = FVID2_CCSF_BITS16_PACKED;
        sl2Prms->inBuffDepth = VHWA_M2M_NF_MIN_RD_BUFFER_DEPTH;
        sl2Prms->outBuffDepth = VHWA_M2M_NF_MIN_WR_BUFFER_DEPTH;
        sl2Prms->outCcsf = FVID2_CCSF_BITS16_PACKED;
        #if defined(VHWA_VPAC_IP_REV_VPAC) || defined(VHWA_VPAC_IP_REV_VPAC3)
        sl2Prms->enable16KBAlignment = (uint32_t)UTRUE;  /* 16KB alignment enabled by default (required for firewall support) */
        #else
        sl2Prms->enable16KBAlignment = (uint32_t)UFALSE;  /* VPAC3L does not support firewall */
        #endif
    }
}

static inline void Vhwa_M2mNfCreatePrmsInit(Vhwa_M2mNfCreatePrms *createArgs)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale:
    The negative test framework and test apps cannot reach this portion.
    CreateArgs is a member of the struct pointed at by nf_generic_obj. It is ensured that nf_generic_obj is not null. Therefore, no test configuration will result in createArgs to be NULL.
    Effect on this Unit:
    If createArgs is NULL, the initialization is skipped and the configuration structure remains uninitialized, which may lead to undefined behavior.
    <justification end> */      
    if (NULL != createArgs)
    /* LDRA_JUSTIFY_END */
    {
        createArgs->getTimeStamp = NULL;
        createArgs->enablePsa = (uint32_t)UFALSE;
    }
}

static inline void Vhwa_M2mNfConfigInit(Vhwa_M2mNfConfig *nfCfg)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale:
    The negative test framework and test apps cannot reach this portion.
    nfCfg is a member of the struct pointed at by nf_generic_obj. It is ensured that nf_generic_obj is not null. Therefore, no test configuration will result in nfCfg to be NULL.
    Effect on this Unit:
    If nfCfg is NULL, the initialization is skipped and the configuration structure remains uninitialized, which may lead to undefined behavior.
    <justification end> */  
    if (NULL != nfCfg)
    /* LDRA_JUSTIFY_END */
    {
        nfCfg->inFmt.width = VHWA_NF_DEF_WIDTH;
        nfCfg->inFmt.height = VHWA_NF_DEF_HEIGHT;
        nfCfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
        nfCfg->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        nfCfg->inFmt.pitch[0] = (VHWA_NF_DEF_WIDTH*3u)/2u;

        nfCfg->outFmt.width = VHWA_NF_DEF_WIDTH;
        nfCfg->outFmt.height = VHWA_NF_DEF_HEIGHT;
        nfCfg->outFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
        nfCfg->outFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        nfCfg->outFmt.pitch[0] = (VHWA_NF_DEF_WIDTH*3u)/2u;

        Nf_ConfigInit(&nfCfg->nfCfg);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* VHWA_M2M_NF_H_ */

/** @} */
