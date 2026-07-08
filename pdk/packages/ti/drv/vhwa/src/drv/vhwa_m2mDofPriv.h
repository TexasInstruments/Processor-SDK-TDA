/**
 *   Copyright (c) Texas Instruments Incorporated 2018
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
 *  \file vhwa_m2mDofPriv.h
 *
 *  \brief DOF Private header file
 *
 */

#ifndef VHWA_M2M_DOF_PRIV_H_
#define VHWA_M2M_DOF_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <vhwa_dmpac_abstraction_layer.h>


#include "src/csl/include/csl_dmpac_hts.h"
#include "src/csl/include/csl_lse.h"
#include "src/csl/include/csl_dmpactop.h"
#include "src/csl/include/csl_dof.h"

#include "include/vhwa_m2mDof.h"

#include <include/vhwa_common.h>
#include "src/drv/vhwa_cfg.h"
#include "src/drv/vhwa_utils.h"
#include <soc/vhwa_soc.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Number of FOCO channel required */
#define VHWA_M2M_DOF_FOCO_CH                    (2U)

/** \brief Maximim number of DMA Channels required for DOF */
#define VHWA_M2M_DOF_MAX_DMA_CH                 (8U)

/**
 * @def CSL_LSE_DOF_MAX_INPUT_CH
 * @brief Defines the maximum number of input channels supported by the LSE DOF module.
 */
#define CSL_LSE_DOF_MAX_INPUT_CH                (1U)

/**
 * @def CSL_LSE_DOF_MAX_OUTPUT_CH
 * @brief Defines the maximum number of output channels supported by the LSE DOF module.
 */
#define CSL_LSE_DOF_MAX_OUTPUT_CH               (4U)

/**
 *  \anchor Vhwa_DofSl2BuffDepth
 *  \name   VHWA DOF SL2 buffer Depfth
 *  \brief  Sl2 Buffer depth macros for Temporal predictor, Pyramidal predictor,
 *          SOF and FOCO
 *
 *  @{
 */
#define DOF_REF_IMG_SL2_DEPTH            (2U)
#define DOF_CURR_IMG_SL2_DEPTH           (2U)
#define DOF_TEMPORAL_PRED_SL2_DEPTH      (2U)
#define DOF_SOF_SL2_DEPTH                (6U)
#define DOF_PYRAM_PRED_SL2_DEPTH         (2U)
#define DOF_FOCO_SL2_DEPTH               (2U)
#define DOF_OUTPUT_SL2_DEPTH             (4U)
 /** @} */

#define VHWA_DOF_FLOW_ID                        (0x3fffU)

/** \brief Maximum loop count - used while dequeuing UDMA */
#define VHWA_DOF_MAX_WAIT_LOOP_CNT          (5000u)

/** \brief Number of ring entries - we can prime this much memcpy operations */
#define VHWA_M2M_DOF_UDMA_RING_ENTRIES      (16U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define VHWA_M2M_DOF_UDMA_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define VHWA_M2M_DOF_UDMA_RING_MEM_SIZE     (VHWA_M2M_DOF_UDMA_RING_ENTRIES * \
                                         VHWA_M2M_DOF_UDMA_RING_ENTRY_SIZE)


/** \brief Total Transfer Record Descriptors per channel */
#define VHWA_M2M_DOF_UDMA_NUM_TR_DESC             (1U)

/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define VHWA_M2M_DOF_UDMA_TRPD_SIZE               (sizeof(CSL_UdmapTR15) + \
    (sizeof(CSL_UdmapTR15) * VHWA_M2M_DOF_UDMA_NUM_TR_DESC) + \
    (VHWA_M2M_DOF_UDMA_NUM_TR_DESC * 4U)+ \
    (UDMA_CACHELINE_ALIGNMENT - (VHWA_M2M_DOF_UDMA_NUM_TR_DESC * 4U)))

/**
 * @brief Bit position for the DOF Watchdog Timer error in the DMPAC INTD error interrupt register.
 */
#define VHWA_DMPAC_INTD_DOF_WDTIMER_ERR_BIT                 (CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_2_ENABLE_LEVEL_DMPAC_OUT_0_EN_WATCHDOGTIMER_ERR_0_SHIFT)
/**
 * @brief Bit position for the FOCO Watchdog Timer error in the DMPAC INTD error interrupt register.
 */
#define VHWA_DMPAC_INTD_DOF_FOCO_WDTIMER_ERR_BIT            (CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_2_ENABLE_LEVEL_DMPAC_OUT_0_EN_WATCHDOGTIMER_ERR_7_SHIFT)

/**
 *  \brief DOF Error Interrupt Mask
 *
 *  This mask defines the set of interrupt enable bits for various
 *  DOF (Dense Optical Flow) error and completion events in the DMPAC INTD
 *  (Interrupt Distributor) configuration register. It includes frame done,
 *  row done, read/write errors, memory protection errors, and FOCO (Focus of
 *  Confidence) related events. Used to enable or check for these interrupt
 *  sources in the driver.
 */
#define VHWA_DMPAC_INTD_DOF_ERR_MASK                        ( \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_DOF_FRAME_DONE_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_DOF_ROW_DONE_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_DOF_READ_ERROR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_DOF_WRITE_ERROR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_DOF_MP0_RD_STATUS_ERROR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_0_FR_DONE_EVT_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_0_SL2_RD_ERR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_0_SL2_WR_ERR_MASK \
)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                isEnabled;
    /**< Flag to indicate if this channel is enabled or not */

    uint8_t                *trMem;
    /**< Start address of the TR memory */
    uint8_t                *trRespMem;
    /**< Address of the start of the TR response memory */

} Vhwa_M2mDofChParams;

typedef struct
{
    uint64_t    sl2Addr[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< SL2 Write address for all 4 channels */

    uint32_t    sl2NumLines[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< SL2 circular buffer size in terms of lines */
    uint32_t    sl2RefCurPitch;
    /**< SL2 pitch for current and reference frames */
} Vhwa_M2mDofSl2Prms;

/**
 * \brief Holds all status register groups for DOF, including DMPAC INTD, DOF, and LSE status registers.
 */
typedef struct {
    CSL_DofStatusRead       dofStatus;
    CSL_DofLseStatusRead    focoLseStatus;
    CSL_DofHtsStatusRead    htsStatus;
} DofStatusRegisterGroup;

typedef struct
{
    uint32_t ENABLE_REG_LEVEL_DMPAC_OUT[1][3];
    uint32_t ENABLE_CLR_REG_LEVEL_DMPAC_OUT[1][3];
} CSL_DofIntdConfigReg;

/**
 * @brief Structure to hold the values of registers written by the SOC for the DOF (Dense Optical Flow) module.
 *
 * This structure is used to store the register values that have been written by the System on Chip (SOC)
 * for the DMPAC DOF hardware block. It is typically used for read-back operations to verify or debug
 * the configuration of the DOF module.
 */
typedef struct
{
    CSL_dmpacRegs             dmpacCntlRegs;
    CSL_DofIntdConfigReg      dmpacDofIntd;
    CSL_dmpac_foco_coreRegs   dmpacFocoRegs;
    CSL_dmpac_dofRegs         dofRegs;
    CSL_lseRegs               lseRegs;
    CSL_DofHtsConfigReg       dofHts;
} VhwaDmpacDofSocReadBack;

/* Config registers params for Safety Mechanism DMPAC7 */
typedef struct
{
    VhwaDmpacDofSocReadBack         *configGoldenRegPtr;
    /** Holds the register values written by the SoC for read-back verification. */

    VhwaDmpacDofSocReadBack         *configRegReadbackPtr;
    /**< Pointer to the register values read back from the SoC for verification. */

    Dof_ConfigRegCompareCbFxn cbFxn;
    /**< Callback function to be called on register memory compare events. */

    void *appData;
    /**< Application-specific data pointer, passed to the callback */
} Dof_ConfigRegValidateParams;
typedef struct
{
    uint32_t                 isUsed;
    /**< Flag to indicate if this object is free or not */

    uint32_t                isFocoUsed[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< Flag to indicate if FOCO is used */

    uint32_t                isTempPred;
    /**< FLAG to indicate if temporal predictor is enabled */

    uint32_t                isPyrPred[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< FLAG to indicate if Pyramidal predictor is enabled */

    uint32_t                isSof;
    /**< FLAG to indicate if Sparse Optical flow is enabled */

    Vhwa_M2mDofCreateArgs    createArgs;
    /** Create Arguments */
    Fvid2_DrvCbParams       cbPrms;
    /** Callback parameters, copied locally */
    Dof_ConfScoreParam       confScoreCfg;
    /**< DOF confidence score configuration parameters */
    CSL_DofBufParams         dofBuffPrams;
    /**< DOF Buffer configuration parameters */

    Vhwa_M2mDofChParams     chPrms[DOF_MAX_PYR_LVL_SUPPORTED]
                                    [VHWA_M2M_DOF_MAX_DMA_CH];
    /**< Pointer to TR Response Descr Memory */

    Vhwa_M2mDofSl2Prms      sl2Prms;
    /**< Sl2 prameters for this handle Memory */

    Dmpac_FocoConfig        focoCfg[DOF_MAX_PYR_LVL_SUPPORTED]
                                    [VHWA_M2M_DOF_FOCO_CH];
    /**< Foco configuration for Current and reference image */

    CSL_LseConfig           lseCfg[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< LSE Configuration for this handle */
    Vhwa_M2mDofPrms         dofPrms;
    /**< DOF Input Configuration for this handle */
    Dof_Config              dofCfg[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< DOF Core Configuration for this handle */
    CSL_DmpacHtsSchConfig        htsCfg[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< HTS Configuration for this handle */
    CSL_DmpacHtsSchConfig        focoHtsCfg[DOF_MAX_PYR_LVL_SUPPORTED];
    /**< FOCO HTS Configuration for this handle */

    uint32_t                hIdx;
    /**< The index of this handle object in the array,
         used in allocating UDMA memory */

    uint32_t                curPyrLvl;
    /**< Current pyramid level being processed */

    uint32_t                nextPyrLvl;
    /**< Next Pyramid level to be processed */

    volatile uint64_t       perfNum;
    /**< Performance number for the last submitted request,
         Used only if GetTimeStamp function pointer is set in
         Create Args */

    Fvid2UtilsLinkListObj   doneQObj;
    /**< Done Queue, queue containing queue objects,
     *   for which DOF processing is completed */
    Fvid2UtilsLinkListObj  *doneQ;
    /**< Pointer to Done Queue */

    uint32_t                numPendReq;
    /**< Count of pending requests */

    Dof_ErrEventParams      eePrms;
    /**< Parameters for the Error Events */
    Dof_WdTimerErrEventParams wdTimerEePrms;
     /**< Parameters for the Watchdog timer Error Events */

    uint32_t                enableStatusRegValidate;
    /**< Flag to enable status register validation. */
    /* One-shot enable      - 1U
    *  Continuous enable    - 2U
    *  Disabled             - 0U
    */
    /*
     * If this flag is enabled, the user must take care of invoking the
     * VHWA_M2M_IOCTL_DOF_VALIDATE_REG after the frame process request,
     * to accumulate the status values and reset the flag for current frame
     * after validation. Otherwise, there is a risk of continuous invocation
     * of vhwaM2mDofUpdateStatusRegGroup.
     */

    DofStatusRegisterGroup  statusRegs;
    /**< Holds the latest DOF, and LSE status register values for this handle */

    uint32_t                enableReconfigReinitReg;
    /**< Flag to enable reconfiguration and reinitialization of registers. */
    /* One-shot enable      - 1U
    *  Continuous enable    - 2U
    *  Disabled             - 0U
    */

    uint32_t                enableConfigRegValidate;
    /**< Flag to enable config register validation. */
    /* One-shot enable      - 1U
    *  Continuous enable    - 2U
    *  Disabled             - 0U
    */
    /*
    *
    * 1. The goldenReg, readBackReg, memory comparison CB pointers in the Vhwa_M2mDofHandleObj
    *    structure must be populated using the control commands
    *    VHWA_M2M_IOCTL_DOF_SET_CONFIG_REG_PTRS before submitting
    *    the frame processing request.
    * 2. If this flag is enabled, the user must ensure that memory allocation
    *    is performed first for both goldenReg and readBackReg. The required
    *    size for the readback memory can be obtained using VHWA_M2M_IOCTL_DOF_GET_READBACK_SIZE.
    */

    Dof_ConfigRegValidateParams          configRegMemPrms;
    /**< Parameters for the config Reg memory compare */

    uint32_t                             hist[DOF_MAX_HISTOGRAM_SIZE];
    /**< Locally storing histogram */
} Vhwa_M2mDofHandleObj;

/**
 *  \brief Structure defining the queue object used in queue/dequeue operation.
 *         This is used for enqueueing pending request as well as
 *         the completed requested to the drivers request queue
 *         and done queue resepctively. This stores pointer to handle
 *         object, for easy access to configurations and also input
 *         and output frame lists.
 */
typedef struct
{
    Fvid2Utils_QElem        qElem;
    /**< FVID2 Utils queue element used in node addition. */
    uint32_t                pyrLvl;
    /**< Pyramid level processed */
    Vhwa_M2mDofHandleObj   *hObj;
    /**< Handle Object reference for this Queue object */
    Fvid2_FrameList         inFrmList;
    /**< FVID2 frame list for store list of input frames. */
    Fvid2_FrameList         outFrmList;
    /**< FVID2 frame list for store list of output frames. */
} Vhwa_M2mDofQueueObj;

typedef struct
{
    uint32_t                initDone;
    /**< Flag to know if DOF is initialized or not,
         Essentially init is called or not */
    uint32_t                isSl2AllocDone;
    /**< Flag to know if SL2 memory is allocated or not.
     *   Essentially if Vhwa_m2mDofAllocSl2 is called and is successfully
     *   executed or not.
     *   Driver open is allowed only if this flag is set to UTRUE */
    Vhwa_M2mDofInitParams   initPrms;
    /**< Init paramters, copied here */

    uint32_t                isRegistered;
    /**< Flag to indicate if this instance object is registered to FVID */

    VhwaAl_Udma_Ch_Struct       utcChObj[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< UDMA Channel object for input channels */
    Udma_ChHandle           utcChHndl[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< UDMA Channel handles for input channels */
    uint32_t                complRingNum[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< UDMA Completion ring number for input channels */
    Udma_RingHandle         fqRingHndl[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< Forward Queue's Ring Handle */
    Udma_RingHandle         cqRingHndl[VHWA_M2M_DOF_MAX_DMA_CH];
    /**< Completion Queue's Ring Handle */
    uint32_t                utcCh[VHWA_M2M_DOF_MAX_DMA_CH];

    VhwaAl_SemaphoreP_Struct       lock;
    /**< Semaphore to protect instance object's variables. */

    uint32_t                openCnt;
    /**< Keeps track of number of opens,
         used to initialize common configuration only once
         for example, enabling DOF at the top level or
            initializing ring accelerator */

    uint32_t                pipeline;
    /**< HTS Pipeline number,
         Same pipeline is used for all handles,
         Allocated on the first open
         Refer to #CSL_HtsPipeline for valid values. */

    VhwaAl_HwiP_Struct             intrHndl;
    /**< Handle to Hardware Interrupt */

    Vhwa_M2mDofSl2AllocPrms sl2AllocPrms;
    /**< Sl2 Alloc Parameters */

    Vhwa_M2mDofQueueObj    *actQObj;
    /**< Pointer points to queue object, whose request is submitted to
     * the HW and not yet completed */
    Vhwa_M2mDofHandleObj   *lastHndlObj;
    /**< Pointer to the handle object, whose request was submitted
     *   and completed */

    Dof_SocInfo             socInfo;
    /**< Soc Information, like base address, interrupt number,
     *   INTD irq numbers etc. */
    uint32_t                irqNum;
    /**< Core IRQ Number to be used for DOF,
     *   Used internally for isr registration,
     *   Storing it locally for easy access */
    uint32_t                vhwaIrqNum;
    /**< VHWA IRQ Number,
     *   Refer to #VHWA_IrqNum for valid number.
     *   Used to enable the irq in INTD number,
     *   Storing it locally for easy access */

    Fvid2UtilsLinkListObj   freeQObj;
    /**< Free Queue, queue containing free queue objcts */
    Fvid2UtilsLinkListObj  *freeQ;
    /**< Pointer to Free Queue */
    Vhwa_M2mDofQueueObj     dofQObj[VHWA_M2M_DOF_UDMA_RING_ENTRIES];
    /**< Dof queue objects */

    uint64_t                startAddr;
    /**< start address of the buffer for Dof in SL2 */
    uint32_t                sl2Size;
    /**< Size of SL2 memory allocated to DOF */

    /* TWO-REGION FIREWALL LAYOUT
    * Region 0 (MAIN): Current image, FOCO preprocessed current, predictors, flow output
    *   Permissions: R5(213), FOCO(211), DOF(210)
    * Region 1 (REFERENCE): Reference image, FOCO preprocessed reference
    *   Permissions: R5(213), FOCO(211), DOF_RD(209)
    */
    uint64_t                mainRegionStartAddr;
    /**< Start address of main region (Region 0) in SL2 */
    uint32_t                mainRegionSl2Size;
    /**< Size of main region in bytes */

    uint64_t                refRegionStartAddr;
    /**< Start address of reference region (Region 1) in SL2 */
    uint32_t                refRegionSl2Size;
    /**< Size of reference region in bytes */

    uint32_t                    totalReqCnt;
    /**< Keeps track of total number of Request submitte */
    uint32_t                    totalIntrCnt;
    /**< Keeps track of total number of interrupts,
      *  including error interrupts */
    uint32_t                    errIntrInvalidStatus;
    /**< Keeps track of Error interrupt,
      * when pipeline completion status is 0 */
    uint32_t                    errIntrRepeateIntr;
    /**< Keeps track of Error interrupt,
      * when pipeline completion status is repeated */
    uint32_t                    errInvalidHtsPipelineStatus;
    /**< Keeps track of Invalid HTS Pipline Status Error */
    uint32_t errNoActQObj;
    uint32_t errNoLastHndlObj;

    uint32_t                   isStalled;
    /**< Flag to indicate if this IP instance is in a stalled state due to
     *   error interrupt or watchdog timer timeout. When set to TRUE,
     *   new process requests will be rejected with FVID2_EFAIL status.
     *   This prevents cascading stalls in multi-node OpenVX graphs. */

} Vhwa_M2mDofInstObj;


/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 * \brief   Function to initialize and allocate UTC/UDMA(External) Channels
 *
 **/
int32_t Vhwa_m2mDofUdmaInit(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofInitParams *initPrms);
int32_t Vhwa_m2mDofUdmaDeInit(const Vhwa_M2mDofInstObj *instObj);

void Vhwa_m2mDofSetTrDesc(const Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj);
int32_t Vhwa_m2mDofStartCh(const Vhwa_M2mDofInstObj *instObj);
int32_t Vhwa_m2mDofStopCh(const Vhwa_M2mDofInstObj *instObj);
int32_t Vhwa_m2mDofSubmitRing(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj);
int32_t Vhwa_m2mDofAllocUdmaMem(Vhwa_M2mDofHandleObj *hObj);
void vhwaM2mDofSetTRAddress(Vhwa_M2mDofHandleObj *hObj,
                            const Fvid2_FrameList *inFrmList,
                            uint64_t outAddr);

int32_t Vhwa_m2mDofPopRings(Vhwa_M2mDofInstObj *instObj,
    Vhwa_M2mDofHandleObj *hObj, uint32_t pyrLvl);

void Vhwa_m2mDofUnregisterIsr(Vhwa_M2mDofInstObj *instObj);
int32_t Vhwa_m2mDofRegisterIsr(Vhwa_M2mDofInstObj *instObj);

Vhwa_M2mDofHandleObj *Vhwa_m2mDofGetHandleObj(uint32_t cnt);

#if !defined(SOC_J722S)
int32_t Vhwa_m2mDofConfigRegReadback(const Vhwa_M2mDofHandleObj *hObj, const Vhwa_M2mDofInstObj *instObj);

int32_t vhwaM2mDofUpdateConfigRegGroup(VhwaDmpacDofSocReadBack *RegVal, const Vhwa_M2mDofInstObj *instObj, const Vhwa_M2mDofHandleObj *hObj);
#endif
#ifdef __cplusplus
}
#endif

#endif  /* VHWA_M2M_DOF_PRIV_H_ */
