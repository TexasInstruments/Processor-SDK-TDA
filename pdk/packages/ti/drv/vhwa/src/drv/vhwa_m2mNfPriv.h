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
 *
 */

/**
 *  \file vhwa_m2mNfPriv.h
 *
 *  \brief NF Private header file
 *
 */

#ifndef VHWA_M2M_NF_PRIV_H_
#define VHWA_M2M_NF_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <vhwa_vpac_abstraction_layer.h>


#include "src/csl/include/csl_hts.h"
#include "src/csl/include/csl_lse.h"
#include "src/csl/include/csl_vpactop.h"
#include "src/csl/include/csl_nf.h"

#include "include/vhwa_m2mNf.h"

#include <include/vhwa_common.h>
#include "src/drv/vhwa_cfg.h"
#include "src/drv/vhwa_utils.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Max Number of handles supported by NF M2M Driver */

/** \brief Maximum loop count - used while dequeuing UDMA */
#define VHWA_M2M_NF_MAX_WAIT_LOOP_CNT          (2000u)

/** \brief Maximim number of DMA Channels required for NF */
#define VHWA_M2M_NF_MAX_DMA_CH                 (2U)

/** \brief Maximum supported Queue Length per instance */
#define VHWA_NF_QUEUE_LEN_PER_INST             (15U)

/** \brief Total Transfer Record Descriptors */
#define VHWA_M2M_NF_MAX_WIDTH_HEIGHT           (8191u)

/** \brief Number of ring entries - we can prime this much memcpy operations */
#define VHWA_M2M_NF_UDMA_RING_ENTRIES      (16U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define VHWA_M2M_NF_UDMA_RING_ENTRY_SIZE   (sizeof(uint64_t))
/** \brief Total ring memory */
#define VHWA_M2M_NF_UDMA_RING_MEM_SIZE     (VHWA_M2M_NF_UDMA_RING_ENTRIES * \
                                         VHWA_M2M_NF_UDMA_RING_ENTRY_SIZE)


/** \brief Total Transfer Record Descriptors per channel */
#define VHWA_M2M_NF_UDMA_NUM_TR_DESC             (1U)

/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define VHWA_M2M_NF_UDMA_TRPD_SIZE               (sizeof(CSL_UdmapTR15) + \
    (sizeof(CSL_UdmapTR15) * VHWA_M2M_NF_UDMA_NUM_TR_DESC) + \
    (VHWA_M2M_NF_UDMA_NUM_TR_DESC * 4U)+ \
    (UDMA_CACHELINE_ALIGNMENT - (VHWA_M2M_NF_UDMA_NUM_TR_DESC * 4U)))

/**
 * @brief Bit position for the NF Watchdog Timer error in the VPAC INTD error interrupt register.
 */
#define VHWA_VPAC_INTD_NF_WDTIMER_ERR_BIT                   (CSL_VPAC_INTD_CFG_ENABLE_REG_LEVEL_VPAC_OUT_0_3_ENABLE_LEVEL_VPAC_OUT_0_EN_WATCHDOGTIMER_ERR_6_SHIFT)

/**
 * \brief Interrupt mask for NF error events and frame done.
 *
 * This mask combines the interrupt enable bits for:
 * - NF frame done
 * - NF SL2 read error
 * - NF SL2 write error
 *
 * Used to enable or check for these specific interrupt events in the INTD configuration.
 */
#define VHWA_VPAC_INTD_NF_ERR_MASK                       ( \
    CSL_VPAC_INTD_CFG_ENABLE_REG_LEVEL_VPAC_OUT_0_2_ENABLE_LEVEL_VPAC_OUT_0_EN_NF_FRAME_DONE_MASK | \
    CSL_VPAC_INTD_CFG_ENABLE_REG_LEVEL_VPAC_OUT_0_2_ENABLE_LEVEL_VPAC_OUT_0_EN_NF_SL2_RD_ERR_MASK | \
    CSL_VPAC_INTD_CFG_ENABLE_REG_LEVEL_VPAC_OUT_0_2_ENABLE_LEVEL_VPAC_OUT_0_EN_NF_SL2_WR_ERR_MASK \
)

#define VHWA_VPAC_NF_WEIGHT_LUT_SIZE          (320U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint8_t                *trMem[VHWA_M2M_NF_MAX_COMP];
    /**< Start address of the TR memory */
} Vhwa_M2mNfChParams;

typedef struct
{
    uint64_t    sl2StartAddr;
    /**< SL2 Start Address allocated to NF driver */
    uint32_t    sl2MemSize;
    /**< SL2 Memory Size allocated to NF driver */

    uint64_t    sl2Addr[VHWA_M2M_NF_MAX_DMA_CH];
    /**< SL2 Write address for all 4 channels */

    uint32_t    sl2NumLines[VHWA_M2M_NF_MAX_DMA_CH];
    /**< SL2 circular buffer size in terms of lines */
    uint32_t    sl2Pitch[VHWA_M2M_NF_MAX_DMA_CH];
    /**< SL2 pitch */
} Vhwa_M2mNfSl2Prms;


/**
 * \brief Holds all status register groups for NF, including INTD, NF, and LSE status registers.
 */
typedef struct {
    CSL_NfHtsStatusRead    nfHtsStatus;
} NfStatusRegisterGroup;

typedef struct
{
    uint32_t ENABLE_REG_LEVEL_VPAC_OUT[1][3];
    uint32_t ENABLE_CLR_REG_LEVEL_VPAC_OUT[1][3];
} CSL_NfIntdConfigReg;

/**
 * @brief Structure to hold the values of registers written by the SOC for the NF (Noise Filter) module.
 *
 * This structure is used to store the register values that have been written by the System on Chip (SOC)
 * for the VPAC NF hardware block. It is typically used for read-back operations to verify or debug
 * the configuration of the NF module.
 */
typedef struct
{
    CSL_vpac_cntlRegs           vpacCntlRegs;
    CSL_vpac_nfRegs             nfRegs;
    CSL_NfIntdConfigReg         vpacNfIntd;
    CSL_NfHtsConfigReg          nfHtsRegs;
    CSL_lseRegs                 nfLseRegs;
} VhwaVpacNfSocReadBack;

/* Config registers params for Safety Mechanism VPAC NF */
typedef struct
{
    VhwaVpacNfSocReadBack         *configGoldenRegPtr;
    /** Holds the register values written by the SoC for read-back verification. */

    VhwaVpacNfSocReadBack         *configRegReadbackPtr;
    /**< Pointer to the register values read back from the SoC for verification. */

    Nf_ConfigRegCompareCbFxn cbFxn;
    /**< Callback function to be called on register memory compare events. */

    void *appData;
    /**< Application-specific data pointer, passed to the callback */
} Nf_ConfigRegValidateParams;


typedef struct
{
    uint32_t                 isUsed;
    /**< Flag to indicate if this object is free or not */

    Vhwa_M2mNfCreatePrms    createPrms;
    /** Create Arguments */

    Fvid2_DrvCbParams       cbPrms;
    /** Callback parameters, copied locally */

    Vhwa_M2mNfChParams     chPrms[VHWA_M2M_NF_MAX_DMA_CH];
    /**< Pointer to TR Response Descr Memory */

    CSL_LseConfig           lseCfg[VHWA_M2M_NF_MAX_COMP];
    /**< LSE Configuration for this handle */
    Vhwa_M2mNfConfig        nfCfg[VHWA_M2M_NF_MAX_COMP];
    /**< NF Configuration for this handle */

    CSL_HtsSchConfig        htsCfg[VHWA_M2M_NF_MAX_COMP];
    /**< HTS Configuration for this handle */

    uint32_t                hIdx;
    /**< The index of this handle object in the array,
         used in allocating UDMA memory */

     uint32_t               numIter;
    /**< When input is YUV420, it requires two passes of NF,
         one for luma and one for chroma, This variable stores
         number of passes required
     */

    uint32_t                curIterCnt;
    /**< Current iteration count used to process Luma and chroma components */

    volatile uint64_t       perfNum[VHWA_M2M_NF_MAX_COMP];
    /**< Performance number for the last submitted request,
         Used only if GetTimeStamp function pointer is set in
         Create Args */

    Vhwa_M2mNfPsaSign       psa;
    /**< PSA Signature of the last submission */

    Fvid2UtilsLinkListObj   doneQObj;
    /**< Done Queue, queue containing queue objects,
     *   for which NF processing is completed */
    Fvid2UtilsLinkListObj  *doneQ;
    /**< Pointer to Done Queue */

    uint32_t                numPendReq;
    /**< Count of pending requests */

    Nf_ErrEventParams       eePrms;
    /**< Parameters for the Error Events */
    Nf_WdTimerErrEventParams wdTimerEePrms;
     /**< Parameters for the Watchdog timer Error Events */
    uint32_t                enableReconfigReinitReg;
    /**< Flag to control the periodic reconfiguration of registers for NF */

    uint32_t                enableStatusRegValidate;
    /*
     * If this flag is enabled, the user must take care of invoking the
     * VHWA_M2M_IOCTL_NF_VALIDATE_REG after the frame process request,
     * to accumulate the status values and reset the flag for current frame
     * after validation. Otherwise, there is a risk of continuous invocation
     * of updateStatusRegGroup.
     */

    NfStatusRegisterGroup   nfStatusRegs;
    /**< Holds the latest NF, HTS status register values for this handle */
    uint32_t                enableConfigRegValidate;
    /**< Flag to enable config register validation */
    /*   1. The goldenReg, readBackReg, memory comparison CB pointers in the Vhwa_M2mDofHandleObj
    *    structure must be populated using the control commands
    *    VHWA_M2M_IOCTL_DOF_SET_CONFIG_REG_PTRS before submitting
    *    the frame processing request.
    *
    *    2. If this flag is enabled, the user must ensure that memory allocation
    *    is performed first for both goldenReg and readBackReg. The required
    *    size for the readback memory can be obtained using VHWA_M2M_IOCTL_DOF_GET_READBACK_SIZE.
    */

    Nf_ConfigRegValidateParams          configRegMemPrms;
    /**< Parameters for the config Reg memory compare */

} Vhwa_M2mNfHandleObj;

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
    Vhwa_M2mNfHandleObj    *hObj;
    /**< Handle Object reference for this Queue object */
    Fvid2_FrameList         inFrmList;
    /**< FVID2 frame list for store list of input frames. */
    Fvid2_FrameList         outFrmList;
    /**< FVID2 frame list for store list of output frames. */
} Vhwa_M2mNfQueueObj;

typedef struct
{
    uint32_t                initDone;
    /**< Flag to know if NF is initialized or not,
         Essentially init is called or not */
    uint32_t                isSl2AllocDone;
    /**< Flag to know if SL2 memory is allocated or not.
     *   Essentially if Vhwa_m2mNfAllocSl2 is called and is successfully
     *   executed or not.
     *   Driver open is allowed only if this flag is set to UTRUE */
    Vhwa_M2mNfInitPrms      initPrms;
    /**< Init paramters, copied here */

    uint32_t                isRegistered;
    /**< Flag to indicate if this instance object is registered to FVID */

    VhwaAl_Udma_Ch_Struct       utcChObj[VHWA_M2M_NF_MAX_DMA_CH];
    /**< UDMA Channel object for input channels */
    Udma_ChHandle           utcChHndl[VHWA_M2M_NF_MAX_DMA_CH];
    /**< UDMA Channel handles for input channels */
    uint32_t                complRingNum[VHWA_M2M_NF_MAX_DMA_CH];
    /**< UDMA Completion ring number for input channels */
    Udma_RingHandle         fqRingHndl[VHWA_M2M_NF_MAX_DMA_CH];
    /**< Forward Queue's Ring Handle */
    Udma_RingHandle         cqRingHndl[VHWA_M2M_NF_MAX_DMA_CH];
    /**< Completion Queue's Ring Handle */
    uint32_t                utcCh[VHWA_M2M_NF_MAX_DMA_CH];

    VhwaAl_SemaphoreP_Struct       lock;
    /**< Semaphore to protect instance object's variables. */
    uint32_t                       hwaLockIdx;

    uint32_t                openCnt;
    /**< Keeps track of number of opens,
         used to initialize common configuration only once
         for example, enabling NF at the top level or
            initializing ring accelerator */

    uint32_t                pipeline;
    /**< HTS Pipeline number,
         Same pipeline is used for all handles,
         Allocated on the first open
         Refer to #CSL_HtsPipeline for valid values. */

    VhwaAl_HwiP_Struct             intrHndl;
    /**< Handle to Hardware Interrupt */

    Vhwa_M2mNfSl2Prms       sl2Prms;
    /**< Sl2 prameters for this handle Memory */

    Vhwa_M2mNfQueueObj     *actQObj;
    /**< Pointer points to queue object, whose request is submitted to
     * the HW and not yet completed */
    Vhwa_M2mNfHandleObj    *lastHndlObj;
    /**< Pointer to the handle object, whose request was submitted
     *   and completed */

    Nf_SocInfo              socInfo;
    /**< Soc Information, like base address, interrupt number,
     *   INTD irq numbers etc. */
    uint32_t                irqNum;
    /**< Core IRQ Number to be used for NF,
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
    Vhwa_M2mNfQueueObj      nfQObj[VHWA_M2M_NF_UDMA_RING_ENTRIES];
    /**< Nf queue objects */

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
} Vhwa_M2mNfInstObj;


/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 * \brief   Function to initialize and allocate UTC/UDMA(External) Channels
 *
 **/
int32_t Vhwa_m2mNfUdmaInit(Vhwa_M2mNfInstObj *instObj,
                           Vhwa_M2mNfInitPrms *initPrms);
int32_t Vhwa_m2mNfUdmaDeInit(const Vhwa_M2mNfInstObj *instObj);
void Vhwa_m2mNfSetTrDesc(const Vhwa_M2mNfInstObj *instObj,
                         Vhwa_M2mNfHandleObj *hObj);
int32_t Vhwa_m2mNfStartCh(const Vhwa_M2mNfInstObj *instObj);
int32_t Vhwa_m2mNfStopCh(const Vhwa_M2mNfInstObj *instObj);
int32_t Vhwa_m2mNfSubmitRing(Vhwa_M2mNfInstObj *instObj,
                             Vhwa_M2mNfHandleObj *hObj,
                             uint32_t itrCnt);
int32_t Vhwa_m2mNfAllocUdmaMem(Vhwa_M2mNfHandleObj *hObj);
void Vhwa_m2mNfSetTRAddress(Vhwa_M2mNfHandleObj *hObj,
                            const Fvid2_FrameList *inFrmList,
                            const Fvid2_FrameList *outFrmList);

int32_t Vhwa_m2mNfPopRings(Vhwa_M2mNfInstObj *instObj,
                           const Vhwa_M2mNfHandleObj *hObj,
                           uint32_t itrCnt);

void Vhwa_m2mNfUnregisterIsr(Vhwa_M2mNfInstObj *instObj);
int32_t Vhwa_m2mNfRegisterIsr(Vhwa_M2mNfInstObj *instObj);

int32_t Vhwa_m2mNfSetConfigInHW(const Vhwa_M2mNfInstObj *instObj,
                                const Vhwa_M2mNfQueueObj *qObj,
                                uint32_t itrCnt);

int32_t Vhwa_m2mNfSetFrameSize(Vhwa_M2mNfInstObj *instObj,
                               const Vhwa_M2mNfQueueObj *qObj,
                               uint32_t itrCnt);

int32_t Vhwa_m2mNfSubmitRequest(Vhwa_M2mNfInstObj *instObj,
                                Vhwa_M2mNfQueueObj *qObj,
                                uint32_t itrCnt);

Vhwa_M2mNfHandleObj *Vhwa_m2mNfGetHandleObj(uint32_t cnt);

int32_t Vhwa_m2mNfConfigRegReadback(const Vhwa_M2mNfHandleObj *hObj,
                                    const Vhwa_M2mNfInstObj *instObj);

int32_t vhwaM2mNfUpdateConfigRegGroup(VhwaVpacNfSocReadBack *reg_buf_ptr,
                                        const Vhwa_M2mNfInstObj *instObj,
                                        const Vhwa_M2mNfHandleObj *hObj);


#ifdef __cplusplus
}
#endif

#endif  /* VHWA_M2M_NF_PRIV_H_ */
