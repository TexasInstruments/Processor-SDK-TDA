/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include <stdint.h>
#include <string.h>
#include <CacheP.h>

#if (defined(MCU_PLUS_SDK) && !defined(QNX))
#include <drivers/udma.h>
#include <drivers/udma/udma_priv.h>
#else
#include <udma.h>
#endif

#include <utils/udma/include/app_udma.h>
#include <utils/mem/include/app_mem.h>
#include <utils/console_io/include/app_log.h>
#include <utils/rtos/include/app_rtos.h>

#if defined(C7X_FAMILY)
#include <c7x.h>

#if defined(MCU_PLUS_SDK)
#include <drivers/hw_include/csl_clec.h>
#include <drivers/hw_include/cslr_soc.h>
#elif defined(PDK)
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/arch/c7x/cslr_C7X_CPU.h>
#endif

#define DRU_LOCAL_EVENT_START_J784S4   (664U)
#define DRU_LOCAL_EVENT_START_DEFAULT  (128U)

#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Default heap ID to use */
#define APP_UDMA_HEAP_ID                (APP_MEM_HEAP_DDR)
/** \brief Alignment for pointers other than UDMA memories */
#define APP_UDMA_ALIGNMENT              (8U)


/*
 * Ring parameters
 */
/** \brief Number of ring entries - we can prime this much memcpy operations */
#define APP_UDMA_RING_ENTRIES           (1U)
/** \brief Size (in bytes) of each ring entry (Size of pointer - 64-bit) */
#define APP_UDMA_RING_ENTRY_SIZE        (sizeof(uint64_t))
/** \brief Total ring memory */
#define APP_UDMA_RING_MEM_SIZE          (APP_UDMA_RING_ENTRIES * \
                                         APP_UDMA_RING_ENTRY_SIZE)
/** \brief This ensures every channel memory is aligned */
#define APP_UDMA_RING_MEM_SIZE_ALIGN    ((APP_UDMA_RING_MEM_SIZE + UDMA_CACHELINE_ALIGNMENT) & \
                                        ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/**
 *  \brief UDMA TR packet descriptor memory.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  one Type_15 TR (CSL_UdmapTR15) + one TR response of 4 bytes -
 *  Since CSL_UdmapCppi5TRPD is less than CSL_UdmapTR15, size is just two times
 *  CSL_UdmapTR15 for alignment.
 */
#define APP_UDMA_TRPD_SIZE              ((sizeof(CSL_UdmapTR15) * 2U) + 4U)
/** \brief This ensures every channel memory is aligned */
#define APP_UDMA_TRPD_SIZE_ALIGN        ((APP_UDMA_TRPD_SIZE + UDMA_CACHELINE_ALIGNMENT) & \
                                        ~(UDMA_CACHELINE_ALIGNMENT - 1U))
/**
 *  \brief Max transfer for 1D transfer as ICNT is just 16-bit.
 *  Assuming 32K and not 64K-1 (0xFFFF) to make it round to multiple of
 *  1024 transfers
 */
#define APP_UDMA_1D_TX_MAX              (32U * 1024U)

/** \brief Minmum number of bytes that will be used for alignment, MUST >= max CPU cache line size */
#define APP_MEM_ALIGN_MIN_BYTES     (128u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/** \brief Information related one logical DMA CH
 *      - User should not set or modify any of the fields in this structure
 *      - This fields are initialized when using the DMA APIs and are used
 *        by the API internally
 */
typedef struct
{
    app_udma_create_prms_t  create_prms;
    /**< Create parameter */
    Udma_DrvHandle          drv_handle;
    /**< DMA driver handle */
#if defined(MCU_PLUS_SDK)
    Udma_ChObject       drv_ch_obj;
    /**< DMA driver channel object */
    Udma_EventObject    cq_event_obj;
    /**< DMA driver CQ event object */
    Udma_EventObject    tdcq_event_obj;
    /**< DMA driver TDCQ event object */
    Udma_EventObjectInt    tr_event_obj;
    /**< DMA driver TR event object */

    Udma_ChHandle           drv_ch_handle;
    /**< DMA channel handle */
    Udma_EventHandle        cq_event_handle;
    /**< DMA CQ event handle */
    Udma_EventHandle        tdcq_event_handle;
    /**< DMA TDCQ event handle */
    Udma_EventHandle        tr_event_handle;
    /**< DMA TR event handle */
#elif defined(PDK)
    struct Udma_ChObj       drv_ch_obj;
    /**< DMA driver channel object */
    struct Udma_EventObj    cq_event_obj;
    /**< DMA driver CQ event object */
    struct Udma_EventObj    tdcq_event_obj;
    /**< DMA driver TDCQ event object */
    struct Udma_EventObj    tr_event_obj;
    /**< DMA driver TR event object */
    Udma_ChHandle           drv_ch_handle;
    /**< DMA channel handle */
    Udma_EventHandle        cq_event_handle;
    /**< DMA CQ event handle */
    Udma_EventHandle        tdcq_event_handle;
    /**< DMA TDCQ event handle */
    Udma_EventHandle        tr_event_handle;
    /**< DMA TR event handle */
#endif
    void                   *fq_ring_mem;
    /**< Allocated free queue ring memory */
    void                   *cq_ring_mem;
    /**< Allocated completion queue ring memory */
    void                   *tdcq_ring_mem;
    /**< Allocated TD completion ring memory */

    void                   *trpd_mem;
    /**< Allocated TRPD memory including TR */
    uint64_t                trpd_mem_phy;
    /**< Allocated TRPD memory physical pointer */

    app_rtos_semaphore_handle_t       transfer_done_sem;
    /**< Semaphore to indicate transfer completion */
    app_rtos_semaphore_handle_t       lock_sem;
    /**< Semaphore for mutual exclusion */
    uint32_t                init_done;
    /**< Flag to indicate init is done for the structure */
    uint64_t                wait_word;
    /**< Flag to indicate wait word for C7x wait event from CLEC */
    volatile uint64_t     *swTriggerPointer;
    /**< Pointer for non-ring based DRU UDMA processing */
} app_udma_ch_obj_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if (defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J722S)) && defined(C7X_FAMILY)
extern uint32_t CSL_clecGetC7xRtmapCpuId(void);
static int32_t appUdmaGetClecConfigEvent(CSL_CLEC_EVTRegs *pRegs,
                            uint32_t evtNum,
                            CSL_ClecEventConfig *evtCfg);
static void appUdmaGetUtcInfo(uint32_t *pUtcId, uint32_t *pDru_local_event_start);
static int32_t appUdmaGetEventNum(uint32_t druChannelId);
#endif

static int32_t appUdmaCreateCh(app_udma_ch_obj_t *ch_obj);
static int32_t appUdmaCopy2DLocal(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers,
    uint32_t fill_data);
static int32_t appUdmaTransfer(app_udma_ch_obj_t *ch_obj);
static void appUdmaEventDmaCb(
    Udma_EventHandle eventHandle,
    uint32_t eventType,
    void *appData);
static void appUdmaEventTdCb(
    Udma_EventHandle eventHandle,
    uint32_t eventType,
    void *appData);
static void appUdmaTrpdInit(app_udma_ch_obj_t *ch_obj, uint32_t copy_mode);
static void appUdmaTrpdSet1D(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_1d_prms_t *prms_1d);
static void appUdmaTrpdSet2D(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t fill_data);
static void appUdmaTrpdSetND(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_nd_prms_t *prms_nd);

static void appUdmaCacheInv(const void * addr, int32_t size);
static void appUdmaCacheWb(const void *addr, int32_t size);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief Default channel object to be used when NULL is passed */
static app_udma_ch_handle_t gAppUdmaDefaultChHandle;

static app_udma_ch_handle_t gAppUdmaNDChHandle[APP_UDMA_ND_CHANNELS_MAX] = {NULL};

static int32_t gAppUdmaNDChRequest[APP_UDMA_ND_CHANNELS_MAX];

int32_t appUdmaCopyInit(void)
{
    int32_t                 retVal = UDMA_SOK;
    app_udma_create_prms_t  prms;
    uint32_t i;

    appLogPrintf("UDMA Copy: Init ... !!!\n");

    appUdmaCreatePrms_Init(&prms);
    gAppUdmaDefaultChHandle = appUdmaCopyCreate(&prms);
    if(NULL == gAppUdmaDefaultChHandle)
    {
        appLogPrintf("UDMA : ERROR: Default channel object create failed!!!\n");
        retVal = UDMA_EFAIL;
    }

    for(i=0; i<APP_UDMA_ND_CHANNELS_MAX; i++)
    {
        gAppUdmaNDChHandle[i] = NULL;
        gAppUdmaNDChRequest[i] = 0;
    }

    appLogPrintf("UDMA Copy: Init ... Done !!!\n");

    return (retVal);
}

int32_t appUdmaCopyDeinit(void)
{
    int32_t     retVal = UDMA_SOK;

    retVal = appUdmaCopyDelete(gAppUdmaDefaultChHandle);
    if(UDMA_SOK != retVal)
    {
        appLogPrintf("UDMA : ERROR: Default channel object delete failed!!!\n");
    }

    return (retVal);
}

app_udma_ch_handle_t appUdmaCopyNDGetHandle(uint32_t ch_idx)
{
    app_udma_ch_handle_t ch_handle = NULL;

    if(ch_idx<APP_UDMA_ND_CHANNELS_MAX)
    {
        if(gAppUdmaNDChHandle[ch_idx]==NULL)
        {
            app_udma_create_prms_t udmaCreatePrms;

            appUdmaCreatePrms_Init(&udmaCreatePrms);
            udmaCreatePrms.enable_intr = 0;
            udmaCreatePrms.use_dru     = 0;
            if(ch_idx>=(APP_UDMA_ND_CHANNELS_MAX/2))
            {
                udmaCreatePrms.use_dru     = 1;
            }
            udmaCreatePrms.use_nd_copy = 1;
            gAppUdmaNDChHandle[ch_idx] = appUdmaCopyCreate(&udmaCreatePrms);
            #if 0
            if(udmaCreatePrms.use_dru == 0)
            {
                appLogPrintf(" UDMA: Creating %d ch at UDMA\n", ch_idx);
            }
            else
            {
                appLogPrintf(" UDMA: Creating %d ch at DRU\n", ch_idx);
            }
            #endif
        }
        ch_handle = gAppUdmaNDChHandle[ch_idx];
        gAppUdmaNDChRequest[ch_idx]++;
    }
    return ch_handle;
}

int32_t appUdmaCopyNDReleaseHandle(uint32_t ch_idx)
{
    int32_t  retVal = UDMA_SOK;

    if(ch_idx < APP_UDMA_ND_CHANNELS_MAX)
    {
        gAppUdmaNDChRequest[ch_idx]--;

        if(gAppUdmaNDChRequest[ch_idx] < 0)
        {
            appLogPrintf("UDMA : ERROR: Channel %d not allocated !!!\n", ch_idx);
            gAppUdmaNDChRequest[ch_idx] = 0;
            return UDMA_EFAIL;
        }

        if(gAppUdmaNDChRequest[ch_idx] == 0)
        {
            retVal = appUdmaCopyDelete(gAppUdmaNDChHandle[ch_idx]);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf("UDMA : ERROR: Unable to delete channel %d handle!!!\n", ch_idx);
            }
            else
            {
                gAppUdmaNDChHandle[ch_idx] = NULL;
            }
        }
    }
    else
    {
        appLogPrintf("UDMA : ERROR: Invalid Channel %d !!!\n", ch_idx);
    }

    return retVal;
}

app_udma_ch_handle_t appUdmaCopyCreate(const app_udma_create_prms_t *prms)
{
    int32_t             retVal = UDMA_SOK;
    app_rtos_semaphore_params_t   semPrms;
    app_udma_ch_obj_t  *ch_obj = NULL;

    if(NULL == prms)
    {
        appLogPrintf("UDMA : ERROR: NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        ch_obj = appMemAlloc(APP_UDMA_HEAP_ID, sizeof(app_udma_ch_obj_t), APP_UDMA_ALIGNMENT);
        if(NULL == ch_obj)
        {
            appLogPrintf("UDMA : ERROR: Channel object alloc failed !!!\n");
            retVal = UDMA_EFAIL;
        }
    }

    if(UDMA_SOK == retVal)
    {
        (void)memset(ch_obj, 0U, sizeof(app_udma_ch_obj_t));
        (void)memcpy(&ch_obj->create_prms, prms, sizeof(app_udma_create_prms_t));

        /* get driver handle */
        ch_obj->drv_handle = appUdmaGetObj();

        /* Alloc semaphore */
        appRtosSemaphoreParamsInit(&semPrms);

        semPrms.initValue = 0U;
        ch_obj->transfer_done_sem = appRtosSemaphoreCreate(semPrms);
        semPrms.initValue = 1U;
        ch_obj->lock_sem = appRtosSemaphoreCreate(semPrms);
        if((NULL == ch_obj->transfer_done_sem) || (NULL == ch_obj->lock_sem))
        {
            appLogPrintf("UDMA : ERROR: Semaphore alloc failed !!!\n");
            retVal = UDMA_EFAIL;
        }

        if(UDMA_SOK == retVal)
        {
            /* Allocate ring and descriptor memory */
            ch_obj->fq_ring_mem = appMemAlloc(APP_UDMA_HEAP_ID, APP_UDMA_RING_MEM_SIZE_ALIGN, UDMA_CACHELINE_ALIGNMENT);
            ch_obj->cq_ring_mem = appMemAlloc(APP_UDMA_HEAP_ID, APP_UDMA_RING_MEM_SIZE_ALIGN, UDMA_CACHELINE_ALIGNMENT);
            ch_obj->tdcq_ring_mem = appMemAlloc(APP_UDMA_HEAP_ID, APP_UDMA_RING_MEM_SIZE_ALIGN, UDMA_CACHELINE_ALIGNMENT);
            ch_obj->trpd_mem = appMemAlloc(APP_UDMA_HEAP_ID, APP_UDMA_TRPD_SIZE_ALIGN, UDMA_CACHELINE_ALIGNMENT);
            if((NULL == ch_obj->fq_ring_mem) ||
               (NULL == ch_obj->cq_ring_mem) ||
               (NULL == ch_obj->tdcq_ring_mem) ||
               (NULL == ch_obj->trpd_mem))
            {
                appLogPrintf("UDMA : ERROR: Descriptor/Ring mem alloc failed !!!\n");
                retVal = UDMA_EFAIL;
            }
            else
            {
                ch_obj->trpd_mem_phy =
                    appMemGetVirt2PhyBufPtr(
                        (uint64_t) ch_obj->trpd_mem, APP_UDMA_HEAP_ID);
            }
        }

        if(UDMA_SOK == retVal)
        {
            retVal = appUdmaCreateCh(ch_obj);

            if(retVal == UDMA_SOK)
            {
                appUdmaTrpdInit(ch_obj, 0);
            }
        }

        if(UDMA_SOK != retVal)
        {
            /* Free-up memory */
            appUdmaCopyDelete(ch_obj);
            ch_obj = NULL;
        }
        else
        {
            ch_obj->init_done = UTRUE;
        }
    }

    return (ch_obj);
}

int32_t appUdmaCopyDelete(app_udma_ch_handle_t ch_handle)
{
    int32_t             retVal = UDMA_SOK, tempRetVal, i;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;
    uint64_t            pDesc;

    if(NULL == ch_obj)
    {
        appLogPrintf("UDMA : ERROR: NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        if(NULL != ch_obj->drv_ch_handle)
        {
            retVal += Udma_chDisable(ch_obj->drv_ch_handle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);

            /* Flush any pending request from the free queue */
            if (1U == ch_obj->create_prms.use_ring)
            {
                while(1)
                {
                    tempRetVal = Udma_ringFlushRaw(
                                     Udma_chGetFqRingHandle(ch_obj->drv_ch_handle), &pDesc);
                    if(UDMA_ETIMEOUT == tempRetVal)
                    {
                        break;
                    }
                }
            }
        }
        if(NULL != ch_obj->fq_ring_mem)
        {
            appMemFree(APP_UDMA_HEAP_ID, ch_obj->fq_ring_mem, APP_UDMA_RING_MEM_SIZE_ALIGN);
            ch_obj->fq_ring_mem = NULL;
        }
        if(NULL != ch_obj->cq_event_handle)
        {
            retVal += Udma_eventUnRegister(ch_obj->cq_event_handle);
            ch_obj->cq_event_handle = NULL;
        }
        if(NULL != ch_obj->tdcq_event_handle)
        {
            retVal += Udma_eventUnRegister(ch_obj->tdcq_event_handle);
            ch_obj->tdcq_event_handle = NULL;
        }
        if(NULL != ch_obj->tr_event_handle)
        {
            retVal += Udma_eventUnRegister(ch_obj->tr_event_handle);
            ch_obj->tr_event_handle = NULL;
        }
        if(NULL != ch_obj->drv_ch_handle)
        {
            retVal += Udma_chClose(ch_obj->drv_ch_handle);
            ch_obj->drv_ch_handle = NULL;
        }
        if(NULL != ch_obj->cq_ring_mem)
        {
            appMemFree(APP_UDMA_HEAP_ID, ch_obj->cq_ring_mem, APP_UDMA_RING_MEM_SIZE_ALIGN);
            ch_obj->cq_ring_mem = NULL;
        }
        if(NULL != ch_obj->tdcq_ring_mem)
        {
            appMemFree(APP_UDMA_HEAP_ID, ch_obj->tdcq_ring_mem, APP_UDMA_RING_MEM_SIZE_ALIGN);
            ch_obj->tdcq_ring_mem = NULL;
        }
        if(NULL != ch_obj->trpd_mem)
        {
            appMemFree(APP_UDMA_HEAP_ID, ch_obj->trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);
            ch_obj->trpd_mem = NULL;
            ch_obj->trpd_mem_phy = 0U;
        }
        if(ch_obj->transfer_done_sem != NULL)
        {
            appRtosSemaphoreDelete(&ch_obj->transfer_done_sem);
            ch_obj->transfer_done_sem = NULL;
        }
        if(ch_obj->lock_sem != NULL)
        {
            appRtosSemaphoreDelete(&ch_obj->lock_sem);
            ch_obj->lock_sem = NULL;
        }
        ch_obj->drv_handle = NULL;
        ch_obj->init_done = UFALSE;
        appMemFree(APP_UDMA_HEAP_ID, ch_obj, sizeof(app_udma_ch_obj_t));
        for (i = 0; i < APP_UDMA_ND_CHANNELS_MAX; i++)
        {
            if (ch_obj == gAppUdmaNDChHandle[i])
            {
                gAppUdmaNDChHandle[i] = NULL;
                break;
            }
        }
        ch_obj = NULL;
    }

    return (retVal);
}

int32_t appUdmaCopy1D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_1d_prms_t *prms_1d)
{
    int32_t             retVal = UDMA_SOK;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

    if(NULL == prms_1d)
    {
        appLogPrintf("UDMA : ERROR: NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        /* Use default object when NULL is passed */
        if(NULL == ch_obj)
        {
            ch_obj = (app_udma_ch_obj_t *)gAppUdmaDefaultChHandle;
        }

        if(prms_1d->length <= APP_UDMA_1D_TX_MAX)
        {
            appRtosSemaphorePend(ch_obj->lock_sem, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

            appUdmaTrpdSet1D(ch_obj, prms_1d);
            retVal = appUdmaTransfer(ch_obj);

            appRtosSemaphorePost(ch_obj->lock_sem);
        }
        /* More than ICNT supported transfer - do split transfer */
        /* Caution: Semaphore lock is called inside 2D function - not called here */
        else
        {
            /* Perform 32KxN 2D transfer + reminder reminder */
            uint32_t                width, height, reminder, size;
            app_udma_copy_2d_prms_t prms_2d;

            width       = APP_UDMA_1D_TX_MAX;
            height      = prms_1d->length / APP_UDMA_1D_TX_MAX;
            size        = (width * height);
            reminder    = prms_1d->length - size;

            appUdmaCopy2DPrms_Init(&prms_2d);
            prms_2d.width        = (uint16_t) width;
            prms_2d.height       = (uint16_t) height;
            prms_2d.dest_pitch   = width;
            prms_2d.src_pitch    = width;
            prms_2d.dest_addr    = prms_1d->dest_addr;
            prms_2d.src_addr     = prms_1d->src_addr;
            retVal = appUdmaCopy2D(NULL, &prms_2d, 1U);

            if(reminder > 0U)
            {
                prms_2d.width        = (uint16_t) reminder;
                prms_2d.height       = (uint16_t) 1U;
                prms_2d.dest_pitch   = reminder;    /* Don't care */
                prms_2d.src_pitch    = reminder;    /* Don't care */
                prms_2d.dest_addr    = prms_1d->dest_addr + size;
                prms_2d.src_addr     = prms_1d->src_addr + size;
                retVal += appUdmaCopy2D(NULL, &prms_2d, 1U);
            }
        }
    }

    return (retVal);
}

int32_t appUdmaCopy2D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers)
{
    int32_t             retVal;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

    retVal = appUdmaCopy2DLocal(ch_obj, prms_2d, num_transfers, UFALSE);
    return (retVal);
}

int32_t appUdmaFill2D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers)
{
    int32_t             retVal;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

    retVal = appUdmaCopy2DLocal(ch_obj, prms_2d, num_transfers, UTRUE);
    return (retVal);
}

int32_t appUdmaCopyNDInit(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_nd_prms_t *prms_nd)
{
    int32_t             retVal = UDMA_SOK;

    if(NULL == ch_handle)
    {
        appLogPrintf("UDMA : ERROR: ch_handle NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }

    if(NULL == prms_nd)
    {
        appLogPrintf("UDMA : ERROR: ch_obj NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }


    if(UDMA_SOK == retVal)
    {
        app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

        /* Set the user provided transfer params */
        appUdmaTrpdSetND(ch_obj, prms_nd);

        if (0U == ch_obj->create_prms.use_ring)
        {
#if defined(PDK) || (defined(MCU_PLUS_SDK) && defined(BUILD_C7X))
        Udma_chDruSubmitTr(ch_obj->drv_ch_handle, (CSL_UdmapTR *)((uint8_t *)ch_obj->trpd_mem + sizeof(CSL_UdmapTR15)));
#endif
        }
        else
        {
            /* Submit request */
            retVal = Udma_ringQueueRaw(
                     Udma_chGetFqRingHandle(ch_obj->drv_ch_handle), ch_obj->trpd_mem_phy);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf("UDMA : ERROR: Channel queue failed!!\n");
            }
        }
    }

    return (retVal);
}

int32_t appUdmaCopyNDTrigger(
    app_udma_ch_handle_t ch_handle)
{
    int32_t             retVal = UDMA_SOK;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

    if (0U == ch_obj->create_prms.use_ring)
    {
        CSL_druChSetGlobalTrigger0Raw(ch_obj->swTriggerPointer);
    }
    else
    {
        retVal = Udma_chSetSwTrigger(ch_obj->drv_ch_handle, CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL0);
        if(UDMA_SOK != retVal)
        {
            appLogPrintf("UDMA : ERROR: SW trigger failed!!\n");
        }
    }

    return (retVal);
}

#if defined(C7X_FAMILY)

int32_t appUdmaCopyNDWait(
    app_udma_ch_handle_t ch_handle)
{
    int32_t     retVal = UDMA_SOK;

    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;

    if ((uint32_t)UTRUE == ch_obj->create_prms.use_dru)
    {
        volatile uint64_t eflRegisterVal;
        uint64_t wait_word;

        wait_word = ch_obj->wait_word;
        eflRegisterVal = __get_indexed(__EFR,0);
        while((eflRegisterVal & wait_word ) != wait_word )
        {
            eflRegisterVal = __get_indexed(__EFR,0);
        }
        __set_indexed(__EFCLR,0, wait_word);
    }
    else
    {
        volatile uint64_t   intrStatusReg;

        Udma_EventPrms *eventPrms = &ch_obj->tr_event_obj.eventPrms;

        while(1U)
        {
            intrStatusReg = CSL_REG64_RD(eventPrms->intrStatusReg);
            if(intrStatusReg & eventPrms->intrMask)
            {
                /* Clear interrupt */
                CSL_REG64_WR(eventPrms->intrClearReg, eventPrms->intrMask);
                /* Work-around for K3_OPEN_SI-213 */
                /* We need to ensure that status clear is written to IA
                   before re-submitting on the same channel */
#if defined (_TMS320C6X)
                _mfence();
                _mfence();
#endif
                break;
            }
            appRtosTaskYield();
        }
    }

    return (retVal);
}

#else

int32_t appUdmaCopyNDWait(
    app_udma_ch_handle_t ch_handle)
{
    int32_t     retVal = UDMA_SOK;

    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;
    volatile uint64_t   intrStatusReg;

    Udma_EventPrms *eventPrms = &ch_obj->tr_event_obj.eventPrms;

    while(1U)
    {
        intrStatusReg = CSL_REG64_RD(eventPrms->intrStatusReg);
        if(intrStatusReg & eventPrms->intrMask)
        {
            /* Clear interrupt */
            CSL_REG64_WR(eventPrms->intrClearReg, eventPrms->intrMask);
            /* Work-around for K3_OPEN_SI-213 */
            /* We need to ensure that status clear is written to IA
               before re-submitting on the same channel */
#if defined (_TMS320C6X)
            _mfence();
            _mfence();
#endif
            break;
        }
        appRtosTaskYield();
    }

    return (retVal);
}

#endif

int32_t appUdmaCopyNDDeinit(
    app_udma_ch_handle_t ch_handle)
{
    int32_t     retVal = UDMA_SOK;
    app_udma_ch_obj_t  *ch_obj = (app_udma_ch_obj_t *)ch_handle;
    uint32_t   *pTrResp, trRespStatus;
    uint8_t    *trpd_mem;
    uint64_t    pDesc = 0;

    if (1U == ch_obj->create_prms.use_ring)
    {
        while (1U)
        {
            /* Wait till response is received in completion queue */
            retVal =
                Udma_ringDequeueRaw(Udma_chGetCqRingHandle(ch_obj->drv_ch_handle), &pDesc);
            if(UDMA_SOK == retVal)
            {
                break;
            }
            appRtosTaskYield();
        }

        /*
            * Sanity check
            */
        /* Check returned descriptor pointer */
        if(pDesc != ch_obj->trpd_mem_phy)
        {
            appLogPrintf("UDMA : ERROR: TR descriptor pointer returned doesn't "
                    "match the submitted address!!\n");
            retVal = UDMA_EFAIL;
        }

        if(UDMA_SOK == retVal)
        {
            /* Invalidate cache */
            trpd_mem  = ch_obj->trpd_mem;
            appUdmaCacheInv(trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);

            /* check TR response status */
            pTrResp = (uint32_t *) (trpd_mem + (sizeof(CSL_UdmapTR15) * 2U));
            trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
            if(trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
            {
                appLogPrintf("UDMA : ERROR: TR Response not completed!!\n");
                retVal = UDMA_EFAIL;
            }
        }
    }

    return (retVal);
}

#if (defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J722S)) && defined(C7X_FAMILY)

static int32_t appUdmaGetClecConfigEvent(CSL_CLEC_EVTRegs *pRegs,
                            uint32_t evtNum,
                            CSL_ClecEventConfig *evtCfg)
{
    int32_t     retVal = CSL_PASS;
    uint32_t    regVal;

    //HACK: Without this log this function returns error in release mode
    appLogPrintf("UDMA : LOG: %s\n", __func__);
    if((NULL == pRegs) ||
        (evtNum >= CSL_CLEC_MAX_EVT_IN))
    {
        retVal = CSL_EFAIL;
    }
    else
    {
        /* Perform read/modify/write so that the default interrupt mode (bit 24)
         * is in power on reset value and should not be changed by CSL
         */
        regVal = CSL_REG32_RD(&pRegs->CFG[evtNum].MRR);
        evtCfg->secureClaimEnable = CSL_REG32_FEXT(&regVal, CLEC_EVT_CFG_MRR_S         );
        evtCfg->evtSendEnable     = CSL_REG32_FEXT(&regVal, CLEC_EVT_CFG_MRR_ESE       );
        evtCfg->rtMap             = CSL_REG32_FEXT(&regVal, CLEC_EVT_CFG_MRR_RTMAP     );
        evtCfg->extEvtNum         = CSL_REG32_FEXT(&regVal, CLEC_EVT_CFG_MRR_EXT_EVTNUM);
        evtCfg->c7xEvtNum         = CSL_REG32_FEXT(&regVal, CLEC_EVT_CFG_MRR_C7X_EVTNUM);
    }

    return (retVal);
}

static void appUdmaGetUtcInfo(uint32_t *pUtcId, uint32_t *pDru_local_event_start)
{
  uint32_t utcId  = UDMA_UTC_ID_MSMC_DRU0;
  uint32_t dru_local_event_start = DRU_LOCAL_EVENT_START_DEFAULT;

#if defined(SOC_J784S4) || defined(SOC_J742S2)
  uint64_t dnum;
  uint8_t corePacNum;

  /* Get the bits from bit 7 to bit 15, which represents the core pac number */
  dnum = __DNUM;    // This register is used to identify the current core
  corePacNum = CSL_REG64_FEXT(&dnum, C7X_CPU_DNUM_COREPACNUM);
  switch (corePacNum)
  {
    case CSL_C7X_CPU_COREPACK_NUM_C7X1:
      utcId = UDMA_UTC_ID_C7X_MSMC_DRU4;
      dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96*0);
      break;
    case CSL_C7X_CPU_COREPACK_NUM_C7X2:
      utcId = UDMA_UTC_ID_C7X_MSMC_DRU5;
      dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96*1) ;
      break;
    case CSL_C7X_CPU_COREPACK_NUM_C7X3:
      utcId = UDMA_UTC_ID_C7X_MSMC_DRU6;
      dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96*2);
      break;
    case CSL_C7X_CPU_COREPACK_NUM_C7X4:
      utcId = UDMA_UTC_ID_C7X_MSMC_DRU7;
      dru_local_event_start = DRU_LOCAL_EVENT_START_J784S4 + (96*3);
      break;
  }
#endif
  if(pUtcId) *pUtcId = utcId ;
  if(pDru_local_event_start) *pDru_local_event_start = dru_local_event_start ;
  return ;
}

static int32_t appUdmaGetEventNum(uint32_t druChannelId)
{
    int32_t eventId;

    uint32_t dru_local_event_start;
    CSL_ClecEventConfig   cfgClec;

#ifdef SOC_J722S
    uint32_t clusterId;
    CSL_CLEC_EVTRegs *clecBaseAddr = NULL;

    clusterId = CSL_clecGetC7xClusterId();

    if (clusterId == CSL_C75_CPU_CLUSTER_NUM_C75_1) {
        clecBaseAddr = (CSL_CLEC_EVTRegs *) CSL_C7X256V0_CLEC_BASE;
    }
#if (CSL_C7X256V_MAIN_CNT == 2U)
    else if (clusterId == CSL_C75_CPU_CLUSTER_NUM_C75_2) {
        clecBaseAddr = (CSL_CLEC_EVTRegs *) CSL_C7X256V1_CLEC_BASE;
    }
#endif
    else {
        clecBaseAddr = NULL;
    }
#else
    int32_t thisCore = CSL_clecGetC7xRtmapCpuId();

    CSL_CLEC_EVTRegs     *clecBaseAddr = (CSL_CLEC_EVTRegs*)CSL_COMPUTE_CLUSTER0_CLEC_REGS_BASE;
#endif

    appUdmaGetUtcInfo( NULL, &dru_local_event_start) ;

    appUdmaGetClecConfigEvent(clecBaseAddr, dru_local_event_start + druChannelId, &cfgClec);
#ifndef SOC_J722S
    if((cfgClec.rtMap !=  thisCore) && (cfgClec.rtMap != CSL_CLEC_RTMAP_CPU_ALL)){
        appLogPrintf("This core (%d) is different than CLEC RTMAP CPU (%d) programming for channel %d\n",
            thisCore, cfgClec.rtMap, druChannelId);
        return UDMA_EFAIL;
    }
    else
#endif
    {
        eventId = cfgClec.c7xEvtNum;
    }

    return eventId;
}
#endif

static int32_t appUdmaCreateCh(app_udma_ch_obj_t *ch_obj)
{
    int32_t             retVal = UDMA_SOK;
    uint32_t            chType;
    Udma_ChPrms         chPrms;
    Udma_ChTxPrms       txPrms;
    Udma_ChRxPrms       rxPrms;
    Udma_ChUtcPrms      utcPrms;

    /* Init channel parameters */
    if ((uint32_t)UTRUE == ch_obj->create_prms.use_dru)
    {
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.utcId = UDMA_UTC_ID_MSMC_DRU0;

        #if (defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J722S)) && defined(C7X_FAMILY)
        appUdmaGetUtcInfo( &chPrms.utcId, NULL);
        ch_obj->create_prms.use_ring = 0U;
        #endif
    }
    else
    {
        chType = UDMA_CH_TYPE_TR_BLK_COPY;
        UdmaChPrms_init(&chPrms, chType);
    }

    if (0U == ch_obj->create_prms.use_ring)
    {
        chPrms.fqRingPrms.ringMem      = NULL;
        chPrms.cqRingPrms.ringMem     = NULL;
        chPrms.tdCqRingPrms.ringMem = NULL;
        chPrms.fqRingPrms.elemCnt     = 0U;
        chPrms.cqRingPrms.elemCnt     = 0U;
        chPrms.tdCqRingPrms.elemCnt  = 0U;
    }
    else
    {
        chPrms.fqRingPrms.ringMem   = ch_obj->fq_ring_mem;
        chPrms.cqRingPrms.ringMem   = ch_obj->cq_ring_mem;
        chPrms.tdCqRingPrms.ringMem = ch_obj->tdcq_ring_mem;
        chPrms.fqRingPrms.ringMemSize   = APP_UDMA_RING_MEM_SIZE;
        chPrms.cqRingPrms.ringMemSize   = APP_UDMA_RING_MEM_SIZE;
        chPrms.tdCqRingPrms.ringMemSize = APP_UDMA_RING_MEM_SIZE;
        chPrms.fqRingPrms.elemCnt   = APP_UDMA_RING_ENTRIES;
        chPrms.cqRingPrms.elemCnt   = APP_UDMA_RING_ENTRIES;
        chPrms.tdCqRingPrms.elemCnt = APP_UDMA_RING_ENTRIES;
    }

    /* Open channel for block copy */
    retVal = Udma_chOpen(ch_obj->drv_handle, &ch_obj->drv_ch_obj, chType, &chPrms);
    if(UDMA_SOK != retVal)
    {
        appLogPrintf("UDMA : ERROR: UDMA channel open failed!!\n");
    }
    else
    {
        ch_obj->drv_ch_handle = &ch_obj->drv_ch_obj;
    }

    if ((uint32_t)UTRUE == ch_obj->create_prms.use_dru)
    {
        if(UDMA_SOK == retVal)
        {
            UdmaChUtcPrms_init(&utcPrms);
            if (0U == ch_obj->create_prms.use_ring)
            {
                utcPrms.druOwner = CSL_DRU_OWNER_DIRECT_TR;
                utcPrms.druQueueId = 0;
            }
            retVal = Udma_chConfigUtc(ch_obj->drv_ch_handle, &utcPrms);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf(" UDMA: UTC channel config failed!!\n");
            }
        }
    }
    else
    {
        if(UDMA_SOK == retVal)
        {
            /* Config TX channel */
            UdmaChTxPrms_init(&txPrms, chType);
            utcPrms.busOrderId = 5;
            utcPrms.addrType = 0;
            utcPrms.busPriority = 5;
            retVal = Udma_chConfigTx(ch_obj->drv_ch_handle, &txPrms);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf("UDMA : ERROR: UDMA TX channel config failed!!\n");
            }
        }

        if(UDMA_SOK == retVal)
        {
            /* Config RX channel - which is implicitly paired to TX channel in
             * block copy mode */
            UdmaChRxPrms_init(&rxPrms, chType);
            utcPrms.busOrderId = 5;
            utcPrms.addrType = 0;
            utcPrms.busPriority = 5;
            retVal = Udma_chConfigRx(ch_obj->drv_ch_handle, &rxPrms);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf("UDMA : ERROR: UDMA RX channel config failed!!\n");
            }
        }
    }
    if((UDMA_SOK == retVal) && (UTRUE == ch_obj->create_prms.enable_intr))
    {

        /* Register ring completion callback */
        Udma_EventPrms eventPrms;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_DMA_COMPLETION;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = ch_obj->drv_ch_handle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(ch_obj->drv_handle);
        eventPrms.eventCb           = &appUdmaEventDmaCb;
        eventPrms.appData           = ch_obj;
        retVal = Udma_eventRegister(
                     ch_obj->drv_handle, &ch_obj->cq_event_obj, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            appLogPrintf("UDMA : ERROR: UDMA CQ event register failed!!\n");
        }
        else
        {
            ch_obj->cq_event_handle = &ch_obj->cq_event_obj;
        }
    }

    if((UDMA_SOK == retVal) && (UTRUE == ch_obj->create_prms.enable_intr))
    {
        /* Register teardown ring completion callback */
        Udma_EventPrms eventPrms;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_TEARDOWN_PACKET;
        eventPrms.eventMode         = UDMA_EVENT_MODE_SHARED;
        eventPrms.chHandle          = ch_obj->drv_ch_handle;
        eventPrms.masterEventHandle = Udma_eventGetGlobalHandle(ch_obj->drv_handle);
        eventPrms.eventCb           = &appUdmaEventTdCb;
        eventPrms.appData           = ch_obj;
        retVal = Udma_eventRegister(
                     ch_obj->drv_handle, &ch_obj->tdcq_event_obj, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            appLogPrintf("UDMA : ERROR: UDMA Teardown CQ event register failed!!\n");
        }
        else
        {
            ch_obj->tdcq_event_handle = &ch_obj->tdcq_event_obj;
        }
    }

#if defined(C7X_FAMILY)
    if ((uint32_t)UFALSE == ch_obj->create_prms.use_dru)
    {
#endif
    if((UDMA_SOK == retVal) && (ch_obj->create_prms.use_nd_copy != 0))
    {
        /* Register TR event */
        Udma_EventPrms eventPrms;
        UdmaEventPrms_init(&eventPrms);
        eventPrms.eventType         = UDMA_EVENT_TYPE_TR;
        eventPrms.eventMode         = UDMA_EVENT_MODE_EXCLUSIVE;
        eventPrms.chHandle          = ch_obj->drv_ch_handle;
        eventPrms.masterEventHandle = NULL;
        eventPrms.eventCb           = NULL;
        eventPrms.appData           = NULL;
        retVal = Udma_eventRegister(ch_obj->drv_handle, &ch_obj->tr_event_obj, &eventPrms);
        if(UDMA_SOK != retVal)
        {
            appLogPrintf("UDMA : ERROR: TR event register failed!!\n");
        }
        else
        {
            ch_obj->tr_event_handle = &ch_obj->tr_event_obj;
        }
    }
#if defined(C7X_FAMILY)
    }
#endif

    if(UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(ch_obj->drv_ch_handle);
        if(UDMA_SOK != retVal)
        {
            appLogPrintf("UDMA : ERROR: UDMA channel enable failed!!\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        uint32_t druChannelId;

        druChannelId = Udma_chGetNum(ch_obj->drv_ch_handle);

        #if defined(MCU_PLUS_SDK) && defined(C7X_FAMILY)
        int32_t eventId;

        ch_obj->swTriggerPointer = Udma_chGetSwTriggerRegister(ch_obj->drv_ch_handle);
        eventId = appUdmaGetEventNum(druChannelId);
        ch_obj->wait_word = ((uint64_t)1U << eventId);
        #elif (defined(SOC_J784S4) || defined(SOC_J742S2)) && defined(__C7120__)
        int32_t eventId;

        ch_obj->swTriggerPointer = Udma_druGetTriggerRegAddr(ch_obj->drv_ch_handle);

        eventId = appUdmaGetEventNum(druChannelId);

        ch_obj->wait_word = ((uint64_t)1U << eventId);
        #else
        #if defined (SOC_J721S2)
        if (UDMA_CORE_ID_C7X_2==Udma_getCoreId())
        {
            ch_obj->wait_word =  ((uint64_t)1U << (16 + druChannelId) );
        }
        else
        #endif
        {
            ch_obj->wait_word =  ((uint64_t)1U << (32 + druChannelId) );
        }
        #endif
    }

    return (retVal);
}

static int32_t appUdmaCopy2DLocal(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers,
    uint32_t fill_data)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t    i;

    if(NULL == prms_2d)
    {
        appLogPrintf("UDMA : ERROR: NULL Pointer!!!\n");
        retVal = UDMA_EFAIL;
    }

    if(UDMA_SOK == retVal)
    {
        /* Use default object when NULL is passed */
        if(NULL == ch_obj)
        {
            ch_obj = (app_udma_ch_obj_t *)gAppUdmaDefaultChHandle;
        }

        appRtosSemaphorePend(ch_obj->lock_sem, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

        for(i = 0U; i < num_transfers; i++)
        {
            appUdmaTrpdSet2D(ch_obj, prms_2d, fill_data);
            retVal = appUdmaTransfer(ch_obj);
            if(UDMA_SOK != retVal)
            {
                break;
            }
            prms_2d++;
        }

        appRtosSemaphorePost(ch_obj->lock_sem);
    }

    return (retVal);
}

static int32_t appUdmaTransfer(app_udma_ch_obj_t *ch_obj)
{
    int32_t     retVal = UDMA_SOK;
    uint32_t   *pTrResp, trRespStatus;
    uint8_t    *trpd_mem;
    uint64_t    pDesc = 0;

    /* Submit request */
    retVal = Udma_ringQueueRaw(
                 Udma_chGetFqRingHandle(ch_obj->drv_ch_handle), ch_obj->trpd_mem_phy);
    if(UDMA_SOK != retVal)
    {
        appLogPrintf("UDMA : ERROR: Channel queue failed!!\n");
    }

    if(UDMA_SOK == retVal)
    {
        if(UTRUE == ch_obj->create_prms.enable_intr)
        {
            /* Wait for return descriptor in completion ring
             * This marks the entire transfer completion */
            appRtosSemaphorePend(ch_obj->transfer_done_sem, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

            /* Response received in completion queue */
            retVal =
                Udma_ringDequeueRaw(Udma_chGetCqRingHandle(ch_obj->drv_ch_handle), &pDesc);
            if(UDMA_SOK != retVal)
            {
                appLogPrintf("UDMA : ERROR: No descriptor after callback!!\n");
                retVal = UDMA_EFAIL;
            }
        }
        else
        {
            while (1U)
            {
                /* Wait till response is received in completion queue */
                retVal =
                    Udma_ringDequeueRaw(Udma_chGetCqRingHandle(ch_obj->drv_ch_handle), &pDesc);
                if(UDMA_SOK == retVal)
                {
                    break;
                }
                appRtosTaskYield();
            }
        }

        if(UDMA_SOK == retVal)
        {
            /*
             * Sanity check
             */
            /* Check returned descriptor pointer */
            if(pDesc != ch_obj->trpd_mem_phy)
            {
                appLogPrintf("UDMA : ERROR: TR descriptor pointer returned doesn't "
                       "match the submitted address!!\n");
                retVal = UDMA_EFAIL;
            }
        }

        if(UDMA_SOK == retVal)
        {
            /* Invalidate cache */
            trpd_mem  = ch_obj->trpd_mem;
            appUdmaCacheInv(trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);

            /* check TR response status */
            pTrResp = (uint32_t *) (trpd_mem + (sizeof(CSL_UdmapTR15) * 2U));
            trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);
            if(trRespStatus != CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE)
            {
                appLogPrintf("UDMA : ERROR: TR Response not completed!!\n");
                retVal = UDMA_EFAIL;
            }
        }
    }

    return (retVal);
}

static void appUdmaEventDmaCb(
    Udma_EventHandle eventHandle,
    uint32_t eventType,
    void *appData)
{
    app_udma_ch_obj_t *ch_obj = (app_udma_ch_obj_t *) appData;

    if(ch_obj != NULL)
    {
        if(UDMA_EVENT_TYPE_DMA_COMPLETION == eventType)
        {
            appRtosSemaphorePost(ch_obj->transfer_done_sem);
        }
    }

    return;
}

static void appUdmaEventTdCb(
    Udma_EventHandle eventHandle,
    uint32_t eventType,
    void *appData)
{
#if defined(PDK)
    int32_t             retVal;
    CSL_UdmapTdResponse tdResp;
    app_udma_ch_obj_t     *ch_obj = (app_udma_ch_obj_t *) appData;

    if(ch_obj != NULL)
    {
        if(UDMA_EVENT_TYPE_TEARDOWN_PACKET == eventType)
        {
            /* Response received in Teardown completion queue */
            retVal = Udma_chDequeueTdResponse(ch_obj->drv_ch_handle, &tdResp);
            if(UDMA_SOK != retVal)
            {
                /* [Error] No TD response after callback!! */
            }
        }
    }
#endif
    return;
}

static void appUdmaTrpdInit(app_udma_ch_obj_t *ch_obj, uint32_t copy_mode)
{
    uint32_t            cqRingNum;
    CSL_UdmapTR15      *pTr;
    void *pTrpd;

    pTrpd = ch_obj->trpd_mem;
    pTr = (CSL_UdmapTR15 *)((uint8_t *)ch_obj->trpd_mem + sizeof(CSL_UdmapTR15));
    if (0 == ch_obj->create_prms.use_ring)
    {
        cqRingNum = 0;
    }
    else
    {
        cqRingNum = Udma_chGetCqRingNum(ch_obj->drv_ch_handle);
    }

    /* Make TRPD */
    UdmaUtils_makeTrpd(pTrpd, UDMA_TR_TYPE_15, 1U, cqRingNum);

    /* Setup TR Flags */
    if ((uint32_t)UTRUE == ch_obj->create_prms.use_dru)
    {
        pTr->flags  =
            CSL_FMK(UDMAP_TR_FLAGS_TYPE,
                CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE);
    }
    else
    {
        pTr->flags  =
            CSL_FMK(UDMAP_TR_FLAGS_TYPE,
                CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING_INDIRECTION);
    }
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_STATIC, 0U);
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EOL, 0U);

    if(copy_mode == 0)
    {
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_COMPLETION);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);
    }
    else if(copy_mode == 1)
    {
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT1_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL0);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT1_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EOL, CSL_UDMAP_TR_FLAGS_EOL_ICNT0);
    }
    else if(copy_mode == 2)
    {
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT2_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL0);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT2_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EOL, CSL_UDMAP_TR_FLAGS_EOL_ICNT0_ICNT1);
    }
    else if(copy_mode == 3)
    {
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EVENT_SIZE, CSL_UDMAP_TR_FLAGS_EVENT_SIZE_ICNT3_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0, CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL0);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER0_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ICNT3_DEC);
        pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EOL, CSL_UDMAP_TR_FLAGS_EOL_ICNT0_ICNT1_ICNT2);
    }

    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1, CSL_UDMAP_TR_FLAGS_TRIGGER_NONE);
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_TRIGGER1_TYPE, CSL_UDMAP_TR_FLAGS_TRIGGER_TYPE_ALL);

    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_CMD_ID, 0x25U);
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_SA_INDIRECT, 0U);
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_DA_INDIRECT, 0U);
    pTr->flags |= CSL_FMK(UDMAP_TR_FLAGS_EOP, 1U);

    pTr->icnt0    = 0U;
    pTr->icnt1    = 0U;
    pTr->icnt2    = 0U;
    pTr->icnt3    = 0U;
    pTr->dim1     = 0U;
    pTr->dim2     = 0U;
    pTr->dim3     = 0U;
    pTr->addr     = 0U;
    pTr->fmtflags = 0x00000000U;        /* Linear addressing, 1 byte per elem.
                                           Replace with CSL-FL API */
    pTr->dicnt0   = 0U;
    pTr->dicnt1   = 0U;
    pTr->dicnt2   = 0U;
    pTr->dicnt3   = 0U;
    pTr->ddim1    = 0U;
    pTr->ddim2    = 0U;
    pTr->ddim3    = 0U;
    pTr->daddr    = 0U;

    return;
}

static void appUdmaTrpdSet1D(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_1d_prms_t *prms_1d)
{
    CSL_UdmapTR15 *pTr;
    uint32_t      *pTrResp;

    pTr = (CSL_UdmapTR15 *)((uint8_t *)ch_obj->trpd_mem + sizeof(CSL_UdmapTR15));

    /* Set SRC params */
    pTr->icnt0    = prms_1d->length;
    pTr->icnt1    = 1U;
    pTr->icnt2    = 1U;
    pTr->icnt3    = 1U;
    pTr->dim1     = pTr->icnt0;
    pTr->dim2     = pTr->icnt0 * pTr->icnt1;
    pTr->dim3     = pTr->icnt0 * pTr->icnt1 * pTr->icnt2;
    pTr->addr     = prms_1d->src_addr;

    /* Set DEST params */
    pTr->dicnt0   = pTr->icnt0;
    pTr->dicnt1   = pTr->icnt1;
    pTr->dicnt2   = pTr->icnt2;
    pTr->dicnt3   = pTr->icnt3;
    pTr->ddim1    = pTr->dicnt0;
    pTr->ddim2    = pTr->dicnt0 * pTr->dicnt1;
    pTr->ddim3    = pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2;
    pTr->daddr    = prms_1d->dest_addr;

    /* Clear TR response memory */
    pTrResp = (uint32_t *) ((uint8_t *)ch_obj->trpd_mem + (sizeof(CSL_UdmapTR15) * 2U));
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback TRPD memory */
    appUdmaCacheWb(ch_obj->trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);

    return;
}

static void appUdmaTrpdSet2D(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t fill_data)
{
    uint32_t           *pTrResp;
    CSL_UdmapTR15      *pTr;

    pTr = (CSL_UdmapTR15 *)((uint8_t *)ch_obj->trpd_mem + sizeof(CSL_UdmapTR15));

    /* Set SRC params */
    pTr->icnt0  = prms_2d->width;
    pTr->icnt1  = prms_2d->height;
    pTr->icnt2  = 1U;
    pTr->icnt3  = 1U;
    pTr->dim2   = pTr->icnt0 * pTr->icnt1;
    pTr->dim3   = pTr->icnt0 * pTr->icnt1 * pTr->icnt2;
    pTr->addr   = prms_2d->src_addr;
    if(UTRUE == fill_data)
    {
        pTr->dim1 = 0U;
    }
    else
    {
        pTr->dim1 = prms_2d->src_pitch;
    }

    /* Set DEST params */
    pTr->dicnt0 = pTr->icnt0;
    pTr->dicnt1 = pTr->icnt1;
    pTr->dicnt2 = pTr->icnt2;
    pTr->dicnt3 = pTr->icnt3;
    pTr->ddim1  = prms_2d->dest_pitch;
    pTr->ddim2  = pTr->dicnt0 * pTr->dicnt1;
    pTr->ddim3  = pTr->dicnt0 * pTr->dicnt1 * pTr->dicnt2;
    pTr->daddr  = prms_2d->dest_addr;

    /* Clear TR response memory */
    pTrResp = (uint32_t *) ((uint8_t *)ch_obj->trpd_mem + (sizeof(CSL_UdmapTR15) * 2U));
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback TRPD memory */
    appUdmaCacheWb(ch_obj->trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);

    return;
}

static void appUdmaTrpdSetND(
    app_udma_ch_obj_t *ch_obj,
    const app_udma_copy_nd_prms_t *prms_nd)
{
    uint32_t           *pTrResp;
    CSL_UdmapTR15      *pTr;

    /* Initialize TRPD memory - one time init like header etc... */
    appUdmaTrpdInit(ch_obj, prms_nd->copy_mode);

    pTr = (CSL_UdmapTR15 *)((uint8_t *)ch_obj->trpd_mem + sizeof(CSL_UdmapTR15));

    /* Set SRC params */
    pTr->icnt0  = prms_nd->icnt0;
    pTr->icnt1  = prms_nd->icnt1;
    pTr->icnt2  = prms_nd->icnt2;
    pTr->icnt3  = prms_nd->icnt3;
    pTr->dim1   = prms_nd->dim1;
    pTr->dim2   = prms_nd->dim2;
    pTr->dim3   = prms_nd->dim3;
    pTr->addr   = prms_nd->src_addr;

    /* Set DEST params */
    pTr->dicnt0  = prms_nd->dicnt0;
    pTr->dicnt1  = prms_nd->dicnt1;
    pTr->dicnt2  = prms_nd->dicnt2;
    pTr->dicnt3  = prms_nd->dicnt3;
    pTr->ddim1   = prms_nd->ddim1;
    pTr->ddim2   = prms_nd->ddim2;
    pTr->ddim3   = prms_nd->ddim3;
    pTr->daddr   = prms_nd->dest_addr;

    if((prms_nd->eltype == 1) || (prms_nd->eltype == 0))
    {
        /* Indicate 1 byte per element for transferring 8bit data */
        pTr->fmtflags |= CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_1);
    }
    else if(prms_nd->eltype == 2)
    {
        /* Indicate 2 bytes per element for transferring 16bit data */
        pTr->fmtflags |= CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_2);
    }
    else if(prms_nd->eltype == 3)
    {
        /* Indicate 3 bytes per element for transferring 24bit data */
        pTr->fmtflags |= CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_3);
    }
    else if(prms_nd->eltype == 4)
    {
        /* Indicate 4 bytes per element for transferring 32bit data */
        pTr->fmtflags |= CSL_FMK(UDMAP_TR_FMTFLAGS_ELYPE, CSL_UDMAP_TR_FMTFLAGS_ELYPE_4);
    }

    /* Clear TR response memory */
    pTrResp = (uint32_t *) ((uint8_t *)ch_obj->trpd_mem + (sizeof(CSL_UdmapTR15) * 2U));
    *pTrResp = 0xFFFFFFFFU;

    /* Writeback TRPD memory */
    appUdmaCacheWb(ch_obj->trpd_mem, APP_UDMA_TRPD_SIZE_ALIGN);

    return;
}

static void appUdmaCacheWb(const void *addr, int32_t size)
{
    uint32_t    isCacheCoherent = Udma_isCacheCoherent();

    if(isCacheCoherent != UTRUE)
    {
        #if defined(PDK)
            CacheP_wb((void*)addr, APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
        #elif defined(MCU_PLUS_SDK)
            CacheP_wb((void*)addr, APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES), CacheP_TYPE_L1D);
        #endif
    }

    return;
}

static void appUdmaCacheInv(const void * addr, int32_t size)
{
    uint32_t    isCacheCoherent = Udma_isCacheCoherent();

    if(isCacheCoherent != UTRUE)
    {
        #if defined(PDK)
            CacheP_Inv((void*)addr,APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES));
        #elif defined(MCU_PLUS_SDK)
            CacheP_inv((void*)addr, APP_MEM_ALIGN32(size, APP_MEM_ALIGN_MIN_BYTES), CacheP_TYPE_L1D);
        #endif
    }

    return;
}

void appUdmaCopyNDPrmsPrint(app_udma_copy_nd_prms_t *prm, char *name)
{
    appLogPrintf(" # %s ND TR PRMs,\n", name);
    appLogPrintf(" copy mode = %d\n", prm->copy_mode);
    appLogPrintf(" src       = %llx\n", prm->src_addr);
    appLogPrintf(" dst       = %llx\n", prm->dest_addr);
    appLogPrintf(" icnt0     = %u\n", (uint32_t)prm->icnt0);
    appLogPrintf(" icnt1     = %u\n", (uint32_t)prm->icnt1);
    appLogPrintf(" icnt2     = %u\n", (uint32_t)prm->icnt2);
    appLogPrintf(" icnt3     = %u\n", (uint32_t)prm->icnt3);
    appLogPrintf(" dicnt0    = %u\n", (uint32_t)prm->dicnt0);
    appLogPrintf(" dicnt1    = %u\n", (uint32_t)prm->dicnt1);
    appLogPrintf(" dicnt2    = %u\n", (uint32_t)prm->dicnt2);
    appLogPrintf(" dicnt3    = %u\n", (uint32_t)prm->dicnt3);
    appLogPrintf(" dim1      = %d\n", prm->dim1);
    appLogPrintf(" dim2      = %d\n", prm->dim2);
    appLogPrintf(" dim3      = %d\n", prm->dim3);
    appLogPrintf(" ddim1     = %d\n", prm->ddim1);
    appLogPrintf(" ddin2     = %d\n", prm->ddim2);
    appLogPrintf(" ddim3     = %d\n", prm->ddim3);
    appLogPrintf(" \n");
}
