/**
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
 */

/**
 *  \file vhwa_m2mSdePriv.h
 *
 *  \brief SDE Private header file
 *
 */

#ifndef VHWA_M2M_SDE_PRIV_H_
#define VHWA_M2M_SDE_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <vhwa_dmpac_abstraction_layer.h>


#include "src/csl/include/csl_dmpac_hts.h"
#include "src/csl/include/csl_lse.h"
#include "src/csl/include/csl_dmpactop.h"
#include "src/csl/include/csl_sde.h"
#include <soc/vhwa_soc.h>

#include "include/vhwa_m2mSde.h"

#include <include/vhwa_common.h>
#include "src/drv/vhwa_cfg.h"
#include "src/drv/vhwa_utils.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Number of FOCO channel required */
#define VHWA_M2M_SDE_FOCO_CH                    (2U)

/** \brief TX DMA Channels required for SDE */
#define VHWA_M2M_SDE_TX_DMA_CH                  (VHWA_M2M_SDE_MAX_IN_BUFFER)
/** \brief RX DMA Channels required for SDE */
#define VHWA_M2M_SDE_RX_DMA_CH                  (VHWA_M2M_SDE_MAX_OUT_BUFFER)
/** \brief Maximim number of DMA Channels required for SDE */
#define VHWA_M2M_SDE_MAX_DMA_CH                 (VHWA_M2M_SDE_TX_DMA_CH +   \
                                                 VHWA_M2M_SDE_RX_DMA_CH)

/** \brief Maximim number of SDE Buffer needed */
#define VHWA_M2M_SDE_MAX_SL2_BUFF               (VHWA_M2M_SDE_MAX_BUFFER + \
                                                 VHWA_M2M_SDE_FOCO_CH)

/**
 * @def CSL_LSE_SDE_MAX_INPUT_CH
 * @brief Defines the maximum number of input channels supported by the LSE SDE module.
 */
#define CSL_LSE_SDE_MAX_INPUT_CH                (1U)

/**
 * @def CSL_LSE_SDE_MAX_OUTPUT_CH
 * @brief Defines the maximum number of output channels supported by the LSE SDE module.
 */
#define CSL_LSE_SDE_MAX_OUTPUT_CH               (4U)

#define VHWA_SDE_FLOW_ID                        (0x3fffU)

/** \brief Maximum loop count - used while dequeuing UDMA */
#define VHWA_SDE_MAX_WAIT_LOOP_CNT          (5000u)

/** \brief Number of ring entries - we can prime this much memcpy operations */
#define VHWA_M2M_SDE_UDMA_RING_ENTRIES      (16U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define VHWA_M2M_SDE_UDMA_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define VHWA_M2M_SDE_UDMA_RING_MEM_SIZE     (VHWA_M2M_SDE_UDMA_RING_ENTRIES * \
                                         VHWA_M2M_SDE_UDMA_RING_ENTRY_SIZE)


/** Minimum Supported number of out buffer
 *  This value is fixed and shouldn't be changed
 */
#define VHWA_M2M_SDE_MIN_OUT_BUFF_DEPTH          (2U)
/** Maximum Supported number of out buffer
 *  Change this value to increse supported maximum number out buffer
 */
#define VHWA_M2M_SDE_MAX_OUT_BUFF_DEPTH          (5U)

/** \brief Maximum possible TX TR descriptors per channel */
#define VHWA_M2M_SDE_UDMA_NUM_TX_TR_DESC         (1U)

/** \brief Maximum possible RX TR descriptors per channel
 *         1 = first block row
 *         (VHWA_M2M_SDE_MAX_OUT_BUFF_DEPTH - 1)*2
 *         1 = TR to transfer rest of middle rows
 *         2 = Last block row
 */
#define VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC  (1u +                                  \
                            ((VHWA_M2M_SDE_MAX_OUT_BUFF_DEPTH - 1u) * 2u)         \
                            + 1u + 2u)

/**
 *  \brief UDMA RX TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define VHWA_M2M_SDE_UDMA_TX_TRPD_SIZE               (sizeof(CSL_UdmapTR15) +   \
    (sizeof(CSL_UdmapTR15) * VHWA_M2M_SDE_UDMA_NUM_TX_TR_DESC) +                \
    (VHWA_M2M_SDE_UDMA_NUM_TX_TR_DESC * 4U) +                                   \
    (UDMA_CACHELINE_ALIGNMENT - (VHWA_M2M_SDE_UDMA_NUM_TX_TR_DESC * 4U)))

/**
 *  \brief UDMA RX TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define VHWA_M2M_SDE_UDMA_RX_TRPD_SIZE               (sizeof(CSL_UdmapTR15) +   \
    (sizeof(CSL_UdmapTR15) * VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC) +                \
    (VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC * 4U) +                                   \
    (UDMA_CACHELINE_ALIGNMENT - (VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC * 4U)))

/**
  * SL2 Parameters/Configuration required by SDE
  */
/** Output block width except last bloack in each row */
#define SDE_OUTPUT_BLOCK_WIDTH_IN_PIX       (64u)
#define SDE_OUTPUT_BLOCK_WIDTH_IN_BYTE      (SDE_OUTPUT_BLOCK_WIDTH_IN_PIX * 2u)
#define SDE_OUTPUT_BLOCK_HEIGHT             (10U)
#define SDE_PER_OUT_BUFF_SIZE                                                   \
                        (SDE_OUTPUT_BLOCK_WIDTH_IN_BYTE*SDE_OUTPUT_BLOCK_HEIGHT)

/** Buffer count required for Base and Reference image in SL2
  * Each Buffer should be of size 8*ImageStrige in SL2
  */
#define SDE_SL2_REF_BASE_IMG_BUFF_COUNT     (3U)
#define SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT    (8U)

#define SDE_SL2_REF_BASE_IMG_CIR_BUFF_SIZE                                      \
         (SDE_SL2_REF_BASE_IMG_BUFF_COUNT * SDE_SL2_REF_BASE_IMG_BUFF_HEIGHT)

#define SDE_PROCESS_BLOCK_WIDTH             (64U)
#define SDE_OUT_FIRST_LAST_BLK_HEIGHT       (10U)
#define SDE_OUT_MIDDLE_BLK_HEIGHT           (8U)

/*& FOCO Buffer Paramters */
#define SDE_FOCO_SL2_BUFF_DEPTH             (2U)

/**
 * @brief Bit position for the SDE Watchdog Timer error in the DMPAC INTD error interrupt register.
 */
#define VHWA_DMPAC_INTD_SDE_WDTIMER_ERR_BIT                 (CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_2_ENABLE_LEVEL_DMPAC_OUT_0_EN_WATCHDOGTIMER_ERR_1_SHIFT)
/**
 * @brief Bit position for the FOCO Watchdog Timer error in the DMPAC INTD error interrupt register.
 */
#define VHWA_DMPAC_INTD_SDE_FOCO_WDTIMER_ERR_BIT            (CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_2_ENABLE_LEVEL_DMPAC_OUT_0_EN_WATCHDOGTIMER_ERR_8_SHIFT)

/**
 *  \brief SDE Error Interrupt Mask
 *
 *  This mask combines all relevant interrupt enable bits for SDE error and completion events.
 *  It includes block done, frame done, read error, write error, FOCO frame done, and FOCO SL2
 *  read/write error events. Used to enable or check all SDE-related interrupts in the INTD.
 */
#define VHWA_DMPAC_INTD_SDE_ERR_MASK                        ( \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_SDE_BLK_DONE_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_SDE_FRAME_DONE_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_SDE_READ_ERROR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_SDE_WRITE_ERROR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_1_FR_DONE_EVT_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_1_SL2_RD_ERR_MASK | \
    CSL_DMPAC_INTD_CFG_ENABLE_REG_LEVEL_DMPAC_OUT_0_0_ENABLE_LEVEL_DMPAC_OUT_0_EN_FOCO_1_SL2_WR_ERR_MASK \
)

/**
 * @def VHWA_DMPAC_SDE_STATUS_IDLE
 * @brief Status flag indicating that the SDE (Stereo Depth Engine) is in idle mode.
 *
 * The 10th bit (bit 10) of the status register is set to 1 by default after reset,
 * which denotes that the SDE is in idle mode.
 */
#define VHWA_DMPAC_SDE_STATUS_IDLE               (0x00000400U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
typedef struct
{
    uint32_t sicnt1[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];
    uint32_t sicnt2[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];
    uint32_t sicnt3[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];

    uint32_t dicnt2[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];
    uint32_t dicnt3[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];

    uint32_t sl2AddrOff[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];
    /**< SL2 Buffer address offset for each TR */

    uint32_t bufAddrOff[VHWA_M2M_SDE_UDMA_NUM_RX_TR_DESC];
    /**< DDR buffer address offset for each TR */

    uint32_t numBlkInRow;
    /**< Number of TR in a Row -> Used for calculation and testing only TODO remove*/
    uint32_t numVerBlk;
    /**< Number of TR required for this region -> Used for calculation and testing only TODO remove*/
    uint32_t numTr;
} Vhwa_M2mSdeOutChPrms;

typedef struct
{
    uint8_t *trMem[VHWA_M2M_SDE_MAX_DMA_CH];

    Vhwa_M2mSdeOutChPrms outPrams;
    /**< Region parameters for this channel */
} Vhwa_M2mSdeChParams;

typedef struct
{
    uint64_t    sl2RwCostBuffAddr;
    uint64_t    sl2Addr[VHWA_M2M_SDE_MAX_SL2_BUFF];
    /**< SL2 Write address for all 4 channels */
    uint32_t    sl2Pitch[VHWA_M2M_SDE_MAX_SL2_BUFF];
    uint32_t    sl2OutBuffDepth;
} Vhwa_M2mSdeSl2Prms;

typedef struct
{
    CSL_SdeStatusRead       sdeStatus;
    CSL_SdeLseStatusRead    sdeFocoLseStatus;
    CSL_SdeHtsStatusRead    sdeHtsStatus;
} SdeStatusRegisterGroup;

typedef struct
{
    uint32_t ENABLE_REG_LEVEL_DMPAC_OUT[1][3];
    uint32_t ENABLE_CLR_REG_LEVEL_DMPAC_OUT[1][3];
} CSL_SdeIntdConfigReg;

/**
 * @brief Structure to hold the values of registers written by the SOC for the SDE module.
 */
typedef struct
{
    CSL_dmpacRegs             dmpacCntlRegs;
    CSL_SdeIntdConfigReg      dmpacSdeIntd;
    CSL_dmpac_foco_coreRegs   dmpacFocoRegs;
    CSL_dmpac_sdeRegs         sdeRegs;
    CSL_lseRegs               lseRegs;
    CSL_SdeHtsConfigReg       sdeHts;
} VhwaDmpacSdeSocReadBack;

typedef struct
{
    VhwaDmpacSdeSocReadBack         *configGoldenRegPtr;
    VhwaDmpacSdeSocReadBack         *configRegReadbackPtr;
    Sde_ConfigRegCompareCbFxn      cbFxn;
    void *appData;
} Sde_ConfigRegValidateParams;

typedef struct
{
    uint32_t                isUsed;
    /**< Flag to indicate if this object is free or not */

    uint32_t                isFocoUsed;
    /**< Flag to indicate if FOCO is used */

    Vhwa_M2mSdeCreateArgs   createArgs;
    /** Create Arguments */

    Vhwa_M2mSdeChParams     chPrms;
    /** SDE channel parameters */

    Dmpac_FocoConfig        focoCfg;
    /**< Foco configuration for Base and reference image */

    CSL_LseConfig           lseCfg;
    /**< LSE Configuration for this handle */
    Vhwa_M2mSdePrms         sdePrms;
    /**< SDE Configuration for this handle */

    CSL_SdeBufParams        sdeBuffPrms;
    /**< SDE Buffer parameters for this handle */

    Vhwa_M2mSdeSl2Prms      sl2Prms;
    /**< Sl2 prameters for this handle Memory */

    CSL_DmpacHtsSchConfig        htsCfg;
    /**< HTS Configuration for this handle */
    CSL_DmpacHtsSchConfig        focoHtsCfg;
    /**< FOCO HTS Configuration for this handle */

    Fvid2_DrvCbParams       cbPrms;
    /** Callback parameters, copied locally */

    uint32_t                hIdx;
    /**< The index of this handle object in the array,
         used in allocating UDMA memory */

    uint64_t                perfNum;
    /**< Performance number for the last submitted request,
         Used only if GetTimeStamp function pointer is set in
         Create Args */

    Fvid2UtilsLinkListObj   doneQObj;
    /**< Done Queue, queue containing queue objects,
     *   for which SDE processing is completed */
    Fvid2UtilsLinkListObj  *doneQ;
    /**< Pointer to Done Queue */

    uint32_t                numPendReq;
    /**< Count of pending requests */

    Sde_ErrEventParams      eePrms;
    /**< Parameters for the Error Events */
    Sde_WdTimerErrEventParams wdTimerEePrms;
     /**< Parameters for the Watchdog timer Error Events */

    uint32_t                enableStatusRegValidate;
    /**< Flag to enable status register validation. */
    /* One-shot enable      - 1U
    *  Continuous enable    - 2U
    *  Disabled             - 0U
    */
    /*
     * If this flag is enabled, the user must take care of invoking the
     * VHWA_M2M_IOCTL_SDE_VALIDATE_STATUS_REG after the frame process request,
     * to accumulate the status values and reset the flag for current frame
     * after validation. Otherwise, there is a risk of continuous invocation
     * of vhwaM2mSdeUpdateStatusRegGroup.
     */

    SdeStatusRegisterGroup  statusRegs;
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
    * 1. The goldenReg, readBackReg, and memory comparison callback pointers in the Vhwa_M2mSdeHandleObj
    *    structure must be populated using the control command
    *    VHWA_M2M_IOCTL_SDE_SET_CONFIG_REG_PTRS before submitting
    *    the frame processing request.
    * 2. If this flag is enabled, the user must ensure that memory allocation
    *    is performed first for both goldenReg and readBackReg. The required
    *    size for the readback memory can be obtained using VHWA_M2M_IOCTL_SDE_GET_READBACK_SIZE.
    */

    Sde_ConfigRegValidateParams          configRegMemPrms;
    /**< Parameters for the config Reg memory compare */

    uint32_t                             hist[DMPAC_SDE_MAX_HISTOGRAM_SIZE];
    /**< Locally storing histogram */
} Vhwa_M2mSdeHandleObj;

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
    Vhwa_M2mSdeHandleObj    *hObj;
    /**< Handle Object reference for this Queue object */
    Fvid2_FrameList         inFrmList;
    /**< FVID2 frame list for store list of input frames. */
    Fvid2_FrameList         outFrmList;
    /**< FVID2 frame list for store list of output frames. */
} Vhwa_M2mSdeQueueObj;

typedef struct
{
    uint32_t                initDone;
    /**< Flag to know if SDE is initialized or not,
         Essentially init is called or not */
    uint32_t                isSl2AllocDone;
    /**< Flag to know if SL2 memory is allocated or not.
     *   Essentially if Vhwa_m2mSdeAllocSl2 is called and is successfully
     *   executed or not.
     *   Driver open is allowed only if this flag is set to UTRUE */
    Vhwa_M2mSdeInitParams   initPrms;
    /**< Init paramters, copied here */

    uint32_t                isRegistered;
    /**< Flag to indicate if this instance object is registered to FVID */

    VhwaAl_Udma_Ch_Struct       utcChObj[VHWA_M2M_SDE_MAX_DMA_CH];
    /**< UDMA Channel object for input channels */
    Udma_ChHandle           utcChHndl[VHWA_M2M_SDE_MAX_DMA_CH];
    /**< UDMA Channel handles for input channels */
    uint32_t                complRingNum[VHWA_M2M_SDE_MAX_DMA_CH];
    /**< UDMA Completion ring number for input channels */
    Udma_RingHandle         fqRingHndl[VHWA_M2M_SDE_MAX_DMA_CH];
    /**< Forward Queue's Ring Handle */
    Udma_RingHandle         cqRingHndl[VHWA_M2M_SDE_MAX_DMA_CH];
    /**< Completion Queue's Ring Handle */
    uint32_t                utcCh[VHWA_M2M_SDE_MAX_DMA_CH];

    VhwaAl_SemaphoreP_Struct       lock;
    /**< Semaphore to protect instance object's variables. */
    uint32_t                openCnt;
    /**< Keeps track of number of opens,
         used to initialize common configuration only once
         for example, enabling SDE at the top level or
            initializing ring accelerator */

    uint32_t                pipeline;
    /**< HTS Pipeline number,
         Same pipeline is used for all handles,
         Allocated on the first open
         Refer to #CSL_HtsPipeline for valid values. */

    VhwaAl_HwiP_Struct             intrHndl;
    /**< Handle to Hardware Interrupt */

    Vhwa_M2mSdeSl2AllocPrms sl2AllocPrms;
    /**< Sl2 Alloc Parameters */

    Vhwa_M2mSdeQueueObj    *actQObj;
    /**< Pointer points to queue object, whose request is submitted to
     * the HW and not yet completed */
    Vhwa_M2mSdeHandleObj   *lastHndlObj;
    /**< Pointer to the handle object, whose request was submitted
     *   and completed */

    Sde_SocInfo             socInfo;
    /**< Soc Information, like base address, interrupt number,
     *   INTD irq numbers etc. */
    uint32_t                irqNum;
    /**< Core IRQ Number to be used for SDE,
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
    Vhwa_M2mSdeQueueObj     sdeQObj[VHWA_M2M_SDE_UDMA_RING_ENTRIES];
    /**< SDE queue objects */

    uint64_t                sl2StartAddr;
    /**< start address of the buffer for SDE in SL2 */
    uint32_t                sl2Size;
    /**< Size of SL2 memory allocated to SDE */

    uint32_t                totalReqCnt;
    /**< Keeps track of total number of Request submitte */
    uint32_t                totalIntrCnt;
    /**< Keeps track of total number of interrupts,
      *  including error interrupts */
    uint32_t                errIntrInvalidStatus;
    /**< Keeps track of Error interrupt,
      * when pipeline completion status is 0 */
    uint32_t                errIntrRepeateIntr;
    /**< Keeps track of Error interrupt,
      * when pipeline completion status is repeated */
    uint32_t                errInvalidHtsPipelineStatus;
    /**< Keeps track of Invalid HTS Pipline Status Error */
    uint32_t errNoActQObj;
    uint32_t errNoLastHndlObj;

    uint32_t                isStalled;
    /**< Flag to indicate if this IP instance is in a stalled state due to
     *   error interrupt or watchdog timer timeout. When set to TRUE,
     *   new process requests will be rejected with FVID2_EFAIL status.
     *   This prevents cascading stalls in multi-node OpenVX graphs. */
} Vhwa_M2mSdeInstObj;


/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

int32_t Vhwa_m2mSdeUdmaInit(Vhwa_M2mSdeInstObj *instObj,
    Vhwa_M2mSdeInitParams *initPrms);
int32_t Vhwa_m2mSdeUdmaDeInit(const Vhwa_M2mSdeInstObj *instObj);

void Vhwa_m2mSdeSetTrDesc(const Vhwa_M2mSdeInstObj *instObj,
                          Vhwa_M2mSdeHandleObj *hObj);
int32_t Vhwa_m2mSdeStartCh(const Vhwa_M2mSdeInstObj *instObj);
int32_t Vhwa_m2mSdeStopCh(const Vhwa_M2mSdeInstObj *instObj);
int32_t Vhwa_m2mSdeSubmitRing(Vhwa_M2mSdeInstObj *instObj,
                              const Vhwa_M2mSdeHandleObj *hObj);
int32_t Vhwa_m2mSdeAllocUdmaMem(Vhwa_M2mSdeHandleObj *hObj);
void Vhwa_m2mSdeSetAddress(Vhwa_M2mSdeHandleObj *hObj,
                           const Fvid2_FrameList *inFrmList,
                           const Fvid2_FrameList *outFrmList);

int32_t Vhwa_m2mSdePopRings(Vhwa_M2mSdeInstObj *instObj,
                                    Vhwa_M2mSdeHandleObj *hObj);

void Vhwa_m2mSdeUnregisterIsr(Vhwa_M2mSdeInstObj *instObj);
int32_t Vhwa_m2mSdeRegisterIsr(Vhwa_M2mSdeInstObj *instObj);

Vhwa_M2mSdeHandleObj *Vhwa_m2mSdeGetHandleObj(uint32_t cnt);

#if !defined(SOC_J722S)
int32_t Vhwa_m2mSdeConfigRegReadback(const Vhwa_M2mSdeHandleObj *hObj, const Vhwa_M2mSdeInstObj *instObj);

int32_t vhwaM2mSdeUpdateConfigRegGroup(VhwaDmpacSdeSocReadBack *RegVal, const Vhwa_M2mSdeInstObj *instObj, const Vhwa_M2mSdeHandleObj *hObj);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* VHWA_M2M_SDE_PRIV_H_ */
