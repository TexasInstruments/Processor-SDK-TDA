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
 *  \defgroup DRV_LDC_MODULE LDC Module
 *            LDC Module
 *
 */
/**
 *  \ingroup  DRV_LDC_MODULE
 *  \defgroup DRV_LDC_MODULE_INTERFACE LDC Interface
 *            Interface file for LDC M2M FVID2 driver
 *
 *  @{
 */
/**
 *  \file vhwa_m2mLdc.h
 *
 *  \brief  Defines the structures / control operations that could be used to
 *              configure / control LDC M2M driver
 *
 *          Typical Application for the LDC M2M driver is as shown below.
 *
 *          Vhwa_m2mLdcInit
 *               ||
 *          Vhwa_m2mLdcAllocSl2
 *               ||
 *          FVID2_Create
 *               ||
 *          IOCTL_VHWA_M2M_LDC_SET_CONFIG (sets LDC configuration)
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


#ifndef VHWA_M2M_LDC_H_
#define VHWA_M2M_LDC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "include/ldc_cfg.h"
#include <include/vhwa_common.h>
#include <soc/vhwa_soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor VhwaM2mLdc_InstanceId
 *  \name VHWA LDC Instance ID 
 *
 *  VHWA VPAC LDC Instance ID's
 *
 *  @{
 */
#if defined (SOC_J784S4) || defined (SOC_J742S2)
    /** \brief VPAC 0 LDC Instance 0  */
    #define VHWA_M2M_VPAC_0_LDC_DRV_INST_ID_0             (0U)
    /** \brief VPAC 1 LDC Instance 0  */
    #define VHWA_M2M_VPAC_1_LDC_DRV_INST_ID_0             (1U)
    /** \brief Total number of LDC instances */
    #define VHWA_M2M_LDC_DRV_NUM_INST                     (VHWA_M2M_VPAC_1_LDC_DRV_INST_ID_0 + 1U)
#else
    /** \brief VPAC 0 LDC Instance 0  */
    #define VHWA_M2M_VPAC_0_LDC_DRV_INST_ID_0             (0U)
    /** \brief Total number of LDC instances */
    #define VHWA_M2M_LDC_DRV_NUM_INST                     (VHWA_M2M_VPAC_0_LDC_DRV_INST_ID_0 + 1U)
#endif
/* @} */

/**< Default Instance ID for the LDC M2m driver [For backward compatibility] */
#define VHWA_M2M_LDC_DRV_INST_ID            (VHWA_M2M_VPAC_0_LDC_DRV_INST_ID_0)

/** \brief Max Number of handles supported for each instance of LDC M2M Driver */
#define VHWA_M2M_LDC_MAX_HANDLES            (8u)

/**
 *  \anchor VhwaM2MLdc_Ioctls
 *  \name   Ioctls for the LDC memory to memory driver
 *  \brief  Input/Output control MACRO's for LDC memory to memory module
 *
 *  @{
 */
/**
 * \brief IOCTL for setting LDC configuration.
 *        This IOCTL takes LDC Configuration #Ldc_Config as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_SET_PARAMS       (VHWA_IOCTL_M2M_LDC_IOCTL_BASE)

/**
 * \brief IOCTL for getting module's performance numbers
 *        for the last frame submitted.
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_GET_PERFORMANCE  (IOCTL_VHWA_M2M_LDC_SET_PARAMS + 1U)

/**
 * \brief IOCTL for getting LSE PSA Signature
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_GET_PSA_SIGN     (                                  \
    IOCTL_VHWA_M2M_LDC_GET_PERFORMANCE + 1U)

/**
 * \brief IOCTL for enabling error events and
 *        registering callbacks for the same.
 *        This IOCTL pointer to #Ldc_ErrEventParams as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB  (IOCTL_VHWA_M2M_LDC_GET_PSA_SIGN + \
    1U)

/**
 * \brief IOCTL for enabling and setting parameters for
 *        LDC Read bandwidth limiter. This ioctl sets the configurations
 *        directly in the registers, so it gets used for all open
 *        handles. This IOCTL takes pointer to #Ldc_RdBwLimitConfig as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT  (                                  \
    IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB + 1U)

/**
 * \brief IOCTL for enabling and setting HTS limiter.
 *        This is used to slowed down the HTS by introducing clock cycles
 *        between internal start signals.
 *        This IOCTL takes pointer to #Vhwa_HtsLimiter as input
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_SET_HTS_LIMIT    (                                  \
    IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT + 1U)

/**
 * \brief IOCTL to sync start each module
 *        This IOCTL doesn't configure any register and only enable pipeline
 *        to start processing
 *
 * \return  FVID2_SOK on success, else error code.
 */
#define IOCTL_VHWA_M2M_LDC_SYNC_START    (                                  \
                                    IOCTL_VHWA_M2M_LDC_SET_HTS_LIMIT + 1U)

/**
 * \brief IOCTL for enabling Watchdog timer error events and registering
 *         callbacks for the same.
 */
#define VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB      \
                                                    (IOCTL_VHWA_M2M_LDC_SYNC_START + 1U)

/** \brief IOCTL to get PSA signature for all enabeld outputs 
 *  \return  FVID2_SOK on success, else error code.
*/
#define VHWA_M2M_IOCTL_LDC_GET_PSA_SIGN         \
                                        (VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB + 1U)


/**
 * \brief IOCTL for enabling/disabling periodic reconfiguration of LDC registers.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the periodic reconfiguration of LDC registers.
 */
/** \brief IOCTL for enabling/disabling periodic reconfiguration of LDC regs */
#define VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG        \
                                    (VHWA_M2M_IOCTL_LDC_GET_PSA_SIGN + 1U)

/**
 * \brief IOCTL for enabling/disabling status register validation.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the status register validation feature.
 */
/** \brief IOCTL for validating the status register */
#define VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE        \
                                    (VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG + 1U)

/**
 * \brief IOCTL command to retrieve the validation status of the status register.
 *
 * This IOCTL can be used to trigger the after frame process completion and fetch
 * the register validation current state or outcome.
 *
 * Usage:
 * - Enable status register validation using VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE
 * - Submit a frame processing request
 * - Call this IOCTL to validate status registers and get the result
 * - The IOCTL will accumulate the validation status in the instance object
 * - After validation, the enableStatusRegValidate flag is reset for the current frame
 *
 * Important Notes:
 * - This IOCTL MUST be called after frame processing if status validation is enabled
 * - Failure to call this IOCTL may result in continuous invocation of status update functions
 */
/** \brief IOCTL for getting the status of status register validation */
#define VHWA_M2M_IOCTL_LDC_VALIDATE_REG    \
                                    (VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE + 1U)

/**
 * \brief IOCTL for enabling/disabling configuration register validation.
 *
 * This control command takes a single pointer to a uint32_t as its argument.
 * The value pointed to by the uint32_t should be set to enable (1) or disable (0)
 * the configuration register validation feature.
 */
/** \brief IOCTL for validating the configuration register */
#define VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK        \
                                    (VHWA_M2M_IOCTL_LDC_VALIDATE_REG + 1U)

/**
 * \brief IOCTL command to get the size required for configuration register readback.
 *
 * This IOCTL returns the size (in bytes) needed to store the configuration register readback data.
 */
/** \brief IOCTL for getting the size of configuration register readback */
#define VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE    \
                                    (VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK + 1U)

/**
 * \brief IOCTL for setting the configuration register readback and golden pointers, and callback.
 *
 * This IOCTL allows the application to provide pointers to the configuration register readback buffer,
 * the golden reference buffer, and a callback function. The driver will use these pointers to store
 * the configuration register readback data and compare it with the golden reference after processing.
 * The argument should be a pointer to a Vhwa_M2mLdcConfigRegMemParams structure, where:
 *   - readback_ptr: Pointer to the buffer for configuration register readback data (must not be NULL).
 *   - golden_ptr: Pointer to the buffer containing the golden reference data (must not be NULL).
 *   - cbFxn: Callback function to be called after comparison (must not be NULL).
 * The buffer size for readback_ptr should be at least as large as returned by VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE.
 */
/** \brief IOCTL for setting configuration register readback pointer */
#define VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS    \
                                    (VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE + 1U)

/** @} */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    Vhwa_GetTimeStamp  getTimeStamp;
    /**< Function pointer to get timestamp */

    uint32_t                    enablePsa;
    /**< Flag to enable/Disble LSE PSA Module */
} Vhwa_M2mLdcCreateArgs;

/**
 *  struct Vhwa_M2mLdcInitParams
 *  \brief Init Parameters for LDC M2M Driver
 */
typedef struct
{
    Udma_DrvHandle              udmaDrvHndl;
    /**< UDMA driver handle, used for configuring UTC */
    Vhwa_IrqInfo                irqInfo;
    /**< IRQ Information */
} Vhwa_M2mLdcInitParams;

/**
 *  struct Vhwa_m2mLdcSl2AllocPrms
 *  \brief Used for allocating SL2 memory
 */
typedef struct
{
    /** These parameters are used by the driver to allocate memory, used
     *  by all handles
     *  Ensure to allocate max of all handles requirement.
     */
    uint32_t                    enableOutput[LDC_MAX_OUTPUT];
    /**< Flag to indicate if the output is enabled,
     *   Memory is allocated for this output only if it is enabled,
     *   If memory is not allocated for a given output, it cannot be enabled
     *   later using IOCTL_VHWA_M2M_LDC_SET_CONFIG ioctl */
    uint32_t                    outCcsf[LDC_MAX_OUTPUT];
    /**< Output storage format for both the outputs */
    uint32_t                    maxNumBlocks[LDC_MAX_OUTPUT];
    /**< Maximum number of block to be allocated in SL2 memory,
         Minimum 2 blocks are required for each output */
    uint32_t                    maxBlockWidth;
    /**< Maximum output block width for both the outputs,
     *   Block size is same for all outputs */
    uint32_t                    maxBlockHeight;
    /**< Maximum output block height for both the outputs,
     *   Block size is same for all outputs */
    uint32_t                    enable16KBAlignment;
    /**< Enable 16KB alignment for SL2 allocation
     *   TRUE: Allocate with 16KB address and size alignment (required for firewall)
     *   FALSE: Use default alignment without firewall support */
} Vhwa_M2mLdcSl2AllocPrms;

/**
 *  struct Vhwa_m2mLdcPsaSign
 *  \brief Used for PsaSign
 */
typedef struct
{
    /**< Stores PsaSign value for both the outputs */
    uint32_t psaSign[LDC_MAX_OUTPUT];
} Vhwa_M2mLdcPsaSign;

/**
 * \brief Configuration register memory parameters for ISO 26262 safety mechanism (VPAC7).
 *        Interface for providing golden and readback buffer pointers for register validation.
 */
typedef struct
{
    void *golden_ptr;
    /**< Pointer to the buffer containing the golden reference data (must not be NULL) */

    void *readback_ptr;
    /**< Pointer to the buffer for configuration register readback data (must not be NULL) */

    Ldc_ConfigRegCompareCbFxn cbFxn;
    /**< Callback function to be called after comparison (must not be NULL) */

    uint32_t readback_mem_size;
    /**< Size of the readback buffer in bytes */

    void *appData;
    /**< Application-specific data pointer, passed to the callback */
} Vhwa_M2mLdcConfigRegMemParams;

/**
 *  struct Vhwa_M2mLdcSl2Info
 *  \brief Structure to hold LDC SL2 memory information
 */
typedef struct
{
    uint64_t    sl2Addr;
    /**< Start address of allocated SL2 memory */
    uint32_t    sl2Size;
    /**< Size of allocated SL2 memory in bytes */
} Vhwa_M2mLdcSl2Info;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initializes LDC Init Parameters
 *
 *  \param ldcInitPrms      LDC Init Parameters
 *                          This parameter should not be 0.
 *  \return                 Returns 0 on success else returns error value
 */
int32_t Vhwa_m2mLdcInit(Vhwa_M2mLdcInitParams *initPrms);

/**
 *  \brief DeInitializes LDC Init Parameters
 *
 */
void Vhwa_m2mLdcDeInit(void);

/**
 *  \brief Function to allocate Sl2 memory for the output buffers.
 *
 *  \param sl2allocPrms Pointer to a #Vhwa_M2mLdcSl2AllocPrms structure
 *                      containing the SL2 allocation parameters
 *
 *  \return FVID2_SOK if successful, else suitable error code
 */
int32_t Vhwa_m2mLdcAllocSl2(const Vhwa_M2mLdcSl2AllocPrms *sl2AllocPrms);

/**
 *  \brief Function to free allocated SL2.
 *
 */
void Vhwa_m2mLdcFreeSl2(void);


/**
 * \brief Function for getting LDC SL2 allocation information.
 *        This function retrieves the start address and size of allocated SL2 memory.
 *        Takes pointer to Vhwa_M2mLdcSl2Info as output parameter.
 *
 *  \param sl2Info [OUT] Pointer to structure to store SL2 info
 *
 *  \return FVID2_SOK if successful
 *          FVID2_EBADARGS if sl2Info is NULL
 *          FVID2_EFAIL if SL2 memory is not allocated
 */
int32_t Vhwa_m2mLdcGetSl2Info(Vhwa_M2mLdcSl2Info *sl2Info);

/**
 *  VhwaM2mLdcCfg_Init
 *  \brief This function should be used to initialize variable of type
 *          #Vhwa_M2mLdcInitParams.
 *
 *  \param ldcCfg   A pointer of type Vhwa_M2mLdcInitParams
 *  \return         None
 */
static inline void Vhwa_m2mLdcInitParamsInit(Vhwa_M2mLdcInitParams *initPrms);

/**
 *  Vhwa_M2mLdcCreateArgsInit
 *  \brief This function should be used to initialize variable of type
 *          #Vhwa_M2mLdcCreateArgs.
 *
 *  \param createArgs   A pointer of type Vhwa_M2mLdcCreateArgs
 *  \return             None
 */
static inline void Vhwa_M2mLdcCreateArgsInit(Vhwa_M2mLdcCreateArgs *createArgs);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static inline void Vhwa_m2mLdcInitParamsInit(Vhwa_M2mLdcInitParams *initPrms)
{
    if (NULL != initPrms)
    {
        Ldc_getIrqInfo(&initPrms->irqInfo);
        initPrms->udmaDrvHndl = NULL;
    }
}


static inline void Vhwa_M2mLdcSl2AllocPrmsInit(Vhwa_M2mLdcSl2AllocPrms *sl2Prms)
{
    uint32_t cnt;

    if (NULL != sl2Prms)
    {
        for (cnt = 0u; cnt < LDC_MAX_OUTPUT; cnt ++)
        {
            if (0u == cnt)
            {
                sl2Prms->enableOutput[cnt] = (uint32_t)UTRUE;
                sl2Prms->outCcsf[cnt] = FVID2_CCSF_BITS16_PACKED;
            }
            else
            {
                sl2Prms->enableOutput[cnt] = (uint32_t)UTRUE;
                sl2Prms->outCcsf[cnt] = FVID2_CCSF_BITS8_PACKED;
            }
            sl2Prms->maxNumBlocks[cnt] = 2U;
        }
        sl2Prms->maxBlockWidth = 64U;
        sl2Prms->maxBlockHeight = 64U;
        #if defined(VHWA_VPAC_IP_REV_VPAC) || defined(VHWA_VPAC_IP_REV_VPAC3)
        sl2Prms->enable16KBAlignment = (uint32_t)UTRUE;  /* 16KB alignment enabled by default (required for firewall support) */
        #else
        sl2Prms->enable16KBAlignment = (uint32_t)UFALSE;  /* VPAC3L does not support firewall */
        #endif
    }
}

static inline void Vhwa_M2mLdcCreateArgsInit(Vhwa_M2mLdcCreateArgs *createArgs)
{
    if (NULL != createArgs)
    {
        createArgs->getTimeStamp = NULL;
        createArgs->enablePsa = (uint32_t)UFALSE;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* VHWA_M2M_LDC_H_ */

/** @} */
