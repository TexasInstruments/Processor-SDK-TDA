/*
 *  Copyright (c) Texas Instruments Incorporated 2018
 *  All rights reserved.
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
 *  \file ipc_virtio.c
 *
 *  \brief Implementation of ipc rtos virtio.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/ipc/ipc.h>
#include <string.h>

#include "ipc_virtioPrivate.h"
#include "ipc_vring.h"
#include "ipc_mailbox.h"
#include "ipc_osal.h"
#include "ipc_utils.h"
#include "ipc_priv.h"

/* 512k for each vring */
#define IPC_VRING_ALIGNMENT    (0x1000)
#define IPC_VRING_BUF_CNT      (256)

typedef struct Ipc_VirtioInfo_s
{
    uint32_t  daTx;       /* Buffer for TX to remote core */
    uint32_t  daRx;       /* Buffer for RX from remote core */
    uint32_t  primeBuf;   /* Address for prime buffer */
    uint32_t  align;      /* Data alignment */
    uint32_t  num;        /* Number of buffer */
    uint32_t  selfId;     /* Self procId */
    uint32_t  remoteId;   /* Remote procId */
    uint32_t  txNotifyId; /* Notify Id for TX */
    uint32_t  rxNotifyId; /* Notify Id for RX */
} Ipc_VirtioInfo;


/* TranslationEntry */
typedef struct Ipc_TranslationEntry_s
{
    uint32_t  pa;  /* physical address */
    uintptr_t va;  /* virtual address  */
    uint32_t  len; /* Length of buffer */
}Ipc_TranslationEntry;

#define   IPC_TABLE_MAX_CNT   64U

/* IPC PhyAddr to VirtAddr translation table */
typedef struct Ipc_TranslationTable_s
{
    uint32_t              count;
    /**< Number of entryies in the translation table */

    Ipc_TranslationEntry  entry[IPC_TABLE_MAX_CNT];
    /**< Entries in the table, there should be 3-entries for
     * each remote cores */

}Ipc_TranslationTable;

typedef struct Ipc_NotifyIdEntry_s
{
    int32_t    txNotifyId;
    int32_t    rxNotifyId;
} Ipc_NotifyIdEntry;

static int32_t  virtio_cnt = 0;
static uint32_t g_vqItemCnt = 0;

/**
 *  \brief Used for defining the size of the virtqueue registry
 */
#define MAX_VIRTQUEUES          (IPC_MAX_PROCS * 2U)
#define MAX_VQ_PAIRS            ((MAX_VIRTQUEUES+1U)/2U)


#define DIV_ROUND_UP(n,d)   (((n) + (d) - 1) / (d))
#define RP_MSG_BUF_SIZE     (512)

/**
 *  \brief The alignment to use between consumer and producer parts of vring.
 *         Note: this is part of the "wire" protocol. If you change this,
 *         you need to update your BIOS image as well
 */
#define RP_MSG_VRING_ALIGN  (4096)

/* With 256 buffers, the vring will occupy 3 pages
 *
 * RP_MSG_RING_SIZE = ((DIV_ROUND_UP(vring_size(RP_MSG_NUM_BUFS,
 *                    RP_MSG_VRING_ALIGN), PAGE_SIZE)) * PAGE_SIZE)
 *
 * The total IPC space needed to communicate with a remote processor
 *
 * RPMSG_IPC_MEM = (RP_MSG_BUFS_SPACE + 2 * RP_MSG_RING_SIZE)
 */

typedef struct Virtio_Object_s
{
    uint16_t          id;
    /**< Id for this VirtQueue_Object */

    Virtio_callback   callback;
    /**< The function to call when buffers are consumed (can be NULL) */

    uint32_t*         callback_priv;
    /**< The private data to pass to the callback (can be NULL) */

    struct VRing      vring;
    /**< Shared state */

    uint16_t          last_avail_idx;
    /**< Last available index; updated by VirtQueue_getAvailBuf */

    uint16_t          last_used_idx;
    /**< Last available index; updated by VirtQueue_addUsedBuf */

    uint16_t          procId;
    /** Will eventually be used to kick remote processor */

    uint8_t          status;
    /**< Address of the vring status variable in shared memory.
     For TX vq, the address is for this proc's variable.
     For RX vq, the address is for the peer proc's variable.*/

    VIRTIO_DIR        direction;
    /**< Indicates TX or RX direction */

    void*             rscTable;
    /**< Address of resource table. This address is provided by
    application. If the remote core is not A72 or Linux is not
    running, it should be NULL */

    uint32_t        timeoutCnt;
    /**< Timeout count used for mailbox send. If the mailbox FIFO
     * is full, then how long it should try.
     * 0xFFFFFFFF  --> ForEver
     * */

} Virtio_Object;

/**
 *  \brief Vring parameter
 */
typedef struct Vring_Params_s
{
    uint32_t  num;
    uintptr_t addr;
    uint32_t  align;
} Vring_Params;

static Virtio_Object *queueRegistry[MAX_VIRTQUEUES];

/* Application provides the memory for storing local Virtio */
static void*    vqBaseAddr = NULL;
static uint32_t vqSizeLeft = 0U;
static void*    gRscTable = NULL;

static Ipc_TranslationTable  vrTranslationTable =
{
    /* initialize number of translation entry explicitly to zero at build time.
    * this is needed for c7x where in RAM model static globals are not zero initialized by default.
    */
    .count = 0
};

void* Ipc_allocVirtio(void)
{
    void  *ptr = NULL;
    if(vqSizeLeft >= sizeof(Virtio_Object))
    {
        ptr = (void*)((uintptr_t)vqBaseAddr + ((uint32_t)virtio_cnt * sizeof(Virtio_Object)));
        virtio_cnt++;
        vqSizeLeft -= (uint32_t)sizeof(Virtio_Object);
    }

    return ptr;
}

/* ========================================================================== */
/*                          Function Declarations                               */
/* ========================================================================== */

#ifdef QNX_OS

/**
 * \brief    Add entry to the translation able
 **/
static void Ipc_addTranslationEntry(uint32_t phyAddr, uint32_t len);
#endif

/**
 * \brief Get virual address for given physical address
 *
 * \param pa physical address
 *        va virtual address
 *
 * \return #IPC_SOK or #IPC_EFAIL
 *
 **/
static int32_t Ipc_physToVirt(uint32_t pa, uintptr_t *va);

/**
 * \brief Get physical address from given virual address
 *
 * \param   pa physical address
 *          va virtual address
 *
 * \return #IPC_SOK or #IPC_EFAIL
 *
 **/
static int32_t Ipc_virtToPhys(uintptr_t va, uint32_t *pa);

/**
 *  \brief Virtio_isr
 */
static void Virtio_isr(uint32_t* msg, uint32_t priv);

/**
 *  \brief Setups vring buffers.  The buffers are "allocated" from
 *  a single block of memory beginning at addr.  It is assumed that
 *  addr has the appropriate alignment.
 */
static void Virtio_prime(Virtio_Object *vq, uintptr_t addr, uint32_t num);

 /**
  *  \brief Virtio_create
  */
static Virtio_Handle Virtio_create(uint32_t vqId, uint32_t procId,
     Virtio_callback callback, Vring_Params *params,
     VIRTIO_DIR direction, uint8_t status, uint32_t timeoutCnt);

/**
 * \brief Creates Virtio for TX and RX for given core pair.
 * Must be called multiple times with for each remote core.
 * It will result in a TX/RX pair of Virtio being created.
 * One assumption made is that all vrings will be accessed as Virtio.
 * The Virtio callback to the higher level transport is set to
 * Null.  It must be changed when higher level transport is initialized.
 *
 * \return #IPC_SOK or #IPC_EFAIL
 *
 */
static int32_t VirtioIPC_createVirtioCorePair(Ipc_VirtioInfo* vqInfo, uint32_t timeoutCnt);

/**
 *  \brief Ipc_updateVirtioInfo
 */
static void Ipc_updateVirtioInfo(uint32_t numProc, const void *baseAddr, uint32_t vrBufSize,
                      Ipc_VirtioInfo* info);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

#ifdef QNX_OS

static void Ipc_addTranslationEntry(uint32_t phyAddr, uint32_t len)
{
    if(vrTranslationTable.count < (IPC_TABLE_MAX_CNT-1U))
    {
       uintptr_t va = IpcUtils_getMemoryAddress(phyAddr, len);
       vrTranslationTable.entry[vrTranslationTable.count].pa  = phyAddr;
       vrTranslationTable.entry[vrTranslationTable.count].va  = va;
       vrTranslationTable.entry[vrTranslationTable.count].len = len;

#ifdef DEBUG_PRINT
        SystemP_printf("Ipc_addTranslationEntry : phy 0x%x, virt 0x%x, len %d\n",
                phyAddr, va, len);
#endif
       vrTranslationTable.count++;
    }
    else
    {
#ifdef DEBUG_PRINT
        SystemP_printf("Ipc_addTranslationEntry : failed, reached maximum\n");
#endif
    }
}
#endif

static int32_t Ipc_physToVirt(uint32_t pa, uintptr_t *va)
{
    uint32_t                n;
    uint32_t                offset;
    int32_t                 rtnVal = IPC_EFAIL;
    Ipc_TranslationEntry   *e;

    *va = 0U;

    if(vrTranslationTable.count == 0U)
    {
        *va = pa;
    }

    for (n = 0; n < vrTranslationTable.count; n++)
    {
        e = &vrTranslationTable.entry[n];
        if ((pa >= e->pa) && (pa < (e->pa + e->len)))
        {
            offset = pa - e->pa;
            *va = e->va + offset;
            rtnVal = IPC_SOK;
            break;
        }
    }

    return (rtnVal);
}

static int32_t Ipc_virtToPhys(uintptr_t va, uint32_t *pa)
{
    uint32_t              n;
    uint32_t              offset;
    int32_t               rtnVal = IPC_EFAIL;
    Ipc_TranslationEntry *e;

    *pa = 0U;

    if(vrTranslationTable.count == 0U)
    {
        *pa = va;
    }

    for (n = 0; n < vrTranslationTable.count; n++)
    {
        e = &vrTranslationTable.entry[n];
        if ((va >= e->va) && (va < (e->va + e->len)))
        {
            offset = (uint32_t)(va - e->va);
            *pa = (uint32_t)(e->pa + offset);
             rtnVal = IPC_SOK;
             break;
        }
    }

    return (rtnVal);
}


uint32_t Ipc_getVqObjMemoryRequired(void)
{
    /* Two virtio object for each core combination */
    return (sizeof(Virtio_Object) * IPC_MAX_PROCS * 2U);
}

uint32_t Ipc_getVqObjMemoryRequiredPerCore(void)
{
    /* Two VRing (TX/RX) for each core combination */
    return (sizeof(Virtio_Object) * 2U);
}

/**
 * \brief    Checks if remote is A72 with Linux
 *
 * \param  procId  [IN] Remote proc Id
 *
 * \return  1 if remote core is A72 & valid resource table
 *          0 otherwise
 *
 */
uint8_t Virtio_isRemoteLinux(uint16_t procId)
{
    uint8_t  retVal;
    if((procId == IPC_MPU1_0) && (gRscTable != NULL))
    {
        retVal = 1;
    }
    else
    {
        retVal = 0;
    }
    return retVal;
}

/**
 * \brief  Checks if remote is ready
 *
 * \return  1 if remote proc is is not A72 or resource table is null
 *            or linux vdev status is 0x7
 *          0 if linux vdev status is not 0x7
 *
 */
Bool Ipc_isRemoteReady(uint16_t procId)
{
    Bool            retVal = TRUE;
    Ipc_ResourceTable *rsc    = NULL;

    if(TRUE == Virtio_isRemoteLinux(procId))
    {
        rsc = (Ipc_ResourceTable*)gRscTable;
        CacheP_Inv(rsc, (int32_t)sizeof(Ipc_ResourceTable));
        if(rsc->rpmsg_vdev.status != 0x7U)
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

/**
 *  \brief Initializes the registry
 */
int32_t Ipc_initVirtIO(Ipc_VirtIoParams *vqParam)
{
    int32_t   retVal = IPC_SOK;

    if( (vqParam == NULL) ||
        (vqParam->vringBufSize < IPC_VRING_BUFFER_SIZE) ||
		(vqParam->vqBufSize == 0U))
    {
        SystemP_printf("Ipc_initVirtIO : Invalid Memory, Failed\n");
        retVal = IPC_EFAIL;
    }

    if(retVal == IPC_SOK)
    {
        /* Initialize the global queue registry */
        memset(queueRegistry, 0, sizeof(queueRegistry));

        virtio_cnt = 0;
        g_vqItemCnt = 0;
        g_ipc_mBoxCnt = 0;

        vqBaseAddr = (void*)vqParam->vqObjBaseAddr;
        vqSizeLeft = vqParam->vqBufSize;

        /* Initialize the shared page table and VRing */
        retVal = VirtioIPC_init(vqParam);
    }

    return retVal;
}

/**
 *  \brief Saves the resourcetable to be used for
 *  vring for remotecore A72 running Linux
 */
int32_t Ipc_loadResourceTable(void *rsctable)
{
    gRscTable = rsctable;
    return IPC_SOK;
}

void Ipc_resetCoreVirtIO(uint32_t remoteId)
{
    Virtio_Handle rxVqHandle = Virtio_getHandle(remoteId, VIRTIO_RX);
    Virtio_Handle txVqHandle = Virtio_getHandle(remoteId, VIRTIO_TX);

    txVqHandle->last_avail_idx = 0U;
    txVqHandle->last_used_idx = 0U;
    (txVqHandle->vring.used)->idx = 0U;
    (txVqHandle->vring.avail)->idx = 0U;
    rxVqHandle->last_avail_idx = 0U;
    rxVqHandle->last_used_idx = 0U;
    (rxVqHandle->vring.used)->idx = 0U;
    (rxVqHandle->vring.avail)->idx = 0U;
}

/*
 *  \brief Gets the trace buffer pointer from
 *  the resource table
 */
void * Ipc_getResourceTraceBufPtr(void)
{
    Ipc_ResourceTable *rsc = NULL;
    uint32_t i;
    uintptr_t offset;
    uint32_t type;
    Ipc_Trace *entry = NULL;
    void *tracePtr = NULL;

    if (gRscTable != NULL) {
        rsc = (Ipc_ResourceTable*)gRscTable;

        for (i = 0; i < rsc->base.num; i++) {
            offset = (uintptr_t)((uint8_t *)rsc + rsc->offset[i]);
            type = *(uint32_t *)offset;
            if (type == TYPE_TRACE) {
                entry = (Ipc_Trace *)offset;
                tracePtr = (void *)((uintptr_t)entry->da);
                break;
            }
        }
    }

    return tracePtr;
}

/**
 *  \brief Placeholder for MMU Address translation
 */
static inline void * mapPAtoVA(uint32_t pa)
{
    uintptr_t va;

    Ipc_physToVirt(pa, &va);

    return (void*)va;
}

/**
 *  \brief Placeholder for MMU Address translation
 */
static inline uint32_t mapVAtoPA(const void *va)
{
    uint32_t pa;

    Ipc_virtToPhys((uintptr_t)va, &pa);

    return pa;
}

/**
 *  \brief Check the vq status
 */
uint8_t Virtio_isReady(Virtio_Handle vq)
{
    return (uint8_t)(vq->status);
}

/**
 *  \brief Get the processor Id
 */
uint32_t Virtio_getProcId(Virtio_Handle vq)
{
    return vq->procId;
}

/**
 *  \brief Virtio kick
 */
void Virtio_kick(Virtio_Handle vq)
{
    uint32_t selfId = Ipc_mpGetSelfId();

#ifdef QNX_OS
    asm("   DSB ST");
#endif

    /* For now, simply interrupt remote processor */
    if (0U == (vq->vring.avail->flags & VRING_AVAIL_F_NO_INTERRUPT))
    {
        if (IPC_SOK != Ipc_mailboxSend(selfId, vq->procId, vq->id, vq->timeoutCnt))
        {
            SystemP_printf("Virtio_kick: Ipc_mailboxSend failed\n");
        }
    }
}

/**
 *  \brief Virtio_addUsedBuf
 */
int32_t Virtio_addUsedBuf(Virtio_Handle vq, int16_t head, int32_t len)
{
    struct vring_used_elem *used;

    /*
    * The virtqueue contains a ring of used buffers.  Get a pointer to the
    * next entry in that used ring.
    */
    used = &vq->vring.used->ring[vq->vring.used->idx % vq->vring.num];
    used->id = (uint32_t)head;
    used->len = (uint32_t)len;

#ifdef QNX_OS
    asm("   DMB ST");
#endif

    vq->vring.used->idx++;

    return (0);
}

/**
 *  \brief Virtio_addAvailBuf
 */
void Virtio_addAvailBuf(Virtio_Handle vq, void *buf, uint16_t head)
{
    uint16_t avail;

    avail = (uint16_t)(vq->vring.avail->idx % vq->vring.num);

    vq->vring.desc[head].addr   = mapVAtoPA(buf);
    vq->vring.desc[head].len    = RP_MSG_BUF_SIZE;
    vq->vring.desc[head].flags  = 2;
    vq->vring.avail->ring[avail] = head;

    /*
     * Descriptors and available array need to be set before we expose the
     * new available array entries.
     */
#ifdef QNX_OS
    asm("   DMB ST");
#endif

    vq->vring.avail->idx++;
}

/**
 *  \brief Virtio_getUsedBuf ========
 */
void *Virtio_getUsedBuf(Virtio_Handle vq, uint16_t *token)
{
    uint16_t head;
    void     *buf = NULL;

    /* There's nothing available? */
    if (vq->last_used_idx != vq->vring.used->idx)
    {
#ifdef QNX_OS
        asm("   DMB SY");
#endif

        head = (uint16_t)(vq->vring.used->ring[vq->last_used_idx % vq->vring.num].id);
        vq->last_used_idx++;

        buf = mapPAtoVA(vq->vring.desc[head].addr);
        *token = head;
    }

    return (buf);
}

/**
 *  \brief VirtQueue_getAvailBuf
 */
int16_t Virtio_getAvailBuf(Virtio_Handle vq, void **buf, int32_t *len)
{
    uint16_t head;
    int16_t  retVal;

    /* There's nothing available? */
    if (vq->last_avail_idx == vq->vring.avail->idx)
    {
        /* We need to know about added buffers */
        vq->vring.used->flags &= (uint16_t)~VRING_USED_F_NO_NOTIFY;
        retVal = -1;
    }
    else
    {
        /*
         * Grab the next descriptor number they're advertising, and increment
         * the index we've seen.
         */

#ifdef QNX_OS
        /* Only get avail array entries after they have been exposed. */
        asm("   DMB SY");
#endif

        head = vq->vring.avail->ring[vq->last_avail_idx % vq->vring.num];
        vq->last_avail_idx++;

        *buf = mapPAtoVA(vq->vring.desc[head].addr);
        *len = (int32_t)vq->vring.desc[head].len;
        retVal = (int16_t)head;
    }

    return (retVal);
}


/**
 *  \brief Virtio_disableCallback
 */
void Virtio_disableCallback(Virtio_Handle vq)
{
}


/**
 *  \brief Virtio_enableCallback
 */
uint8_t Virtio_enableCallback(Virtio_Handle vq)
{
    return (0U);
}

static void Virtio_isr(uint32_t* msg, uint32_t priv)
{
    Virtio_Object *vq;

    vq = queueRegistry[(2U*priv)+1U];
    if(vq && (vq->procId == priv) && vq->callback)
    {
        vq->callback(vq->callback_priv);
    }
}

/**
 *  \brief Virtio_create
 */
 static Virtio_Handle Virtio_create(uint32_t vqId, uint32_t procId,
    Virtio_callback callback, Vring_Params *params,
    VIRTIO_DIR direction, uint8_t status, uint32_t timeoutCnt)
{
    int32_t retVal;
    Virtio_Object *vq = NULL;

    if (procId < IPC_MAX_PROCS)
    {
        vq = (Virtio_Object *)Ipc_allocVirtio();
    }

    if (NULL != vq)
    {
        vq->callback       = callback;
        vq->id             = (uint16_t)vqId;
        vq->procId         = (uint16_t)procId;
        vq->status         = status;
        vq->last_avail_idx = 0;
        vq->last_used_idx  = 0;
        vq->direction      = direction;
        vq->timeoutCnt     = timeoutCnt;

        vring_init(&(vq->vring), params->num, (void*)params->addr, params->align);

        /* Each processor clears only its TX vq memory. */
        if (direction == VIRTIO_TX)
        {
            memset((void*)params->addr, 0, vring_size(params->num, params->align));
            /* Don't trigger a mailbox message every time remote rpoc */
            /* makes another buffer available.                        */
            vq->vring.used->flags |= VRING_USED_F_NO_NOTIFY;
            queueRegistry[2U*procId] = vq;
        }

        if (direction == VIRTIO_RX)
        {
            uint32_t selfId = Ipc_mpGetSelfId();
            queueRegistry[(2U*procId)+1U] = vq;
            retVal = Ipc_mailboxRegister((uint16_t)selfId, (uint16_t)procId, Virtio_isr, procId, timeoutCnt);
            if (retVal != IPC_SOK)
            {
                 SystemP_printf("Virtio_create : Failed to register mailbox\n");
            }
        }
    }

    return (vq);
}


/**
 *  \brief Virtio_getHandle
 *  processor id correponds to the first arguement i.e vqid
 */
Virtio_Handle Virtio_getHandle(uint32_t vqid, VIRTIO_DIR dir)
{
    uint32_t  vqIndex = (2U*vqid) + (uint32_t)dir;
    return queueRegistry[vqIndex];
}

/**
 *  \brief Virtio_setCallback
 *  procId correponds to the first arguement i.e vqid
 */
int32_t Virtio_setCallback(uint32_t vqid, Virtio_callback callback, uint32_t* priv)
{
    int32_t status = 1;
    Virtio_Object *vq = Virtio_getHandle(vqid, VIRTIO_RX);

    if (NULL != vq)
    {
        vq->callback = callback;
        vq->callback_priv = priv;
        status = 0;
    }

    return status;
}

static void Virtio_prime(Virtio_Object *vq, uintptr_t addr, uint32_t num)
{
    uint32_t i;
    uint8_t *buf;

    /* Add buffers to the vring's available ring. */
    buf = (uint8_t *)addr;
    for (i = 0; i < num; i++)
    {
        Virtio_addAvailBuf(vq, (void *)(uintptr_t)buf, (uint16_t)i);
        buf += RP_MSG_BUF_SIZE;
    }
}

/* This array serves as a way to look up virtQueues when creating higher
 * level transports, such as RPMessage.  So far, the convention is
 * that the first pair of vrings defined for a remote proc will be
 * used for the primary RPMessage transport.
 *
 * Since none of this info is derived, it's all in the shared page, the
 * search could operate directly on the shared page instead of using
 * this array.
 */
static struct VirtioIPC_Vqdev
{
    uint32_t virtioId;
    uint32_t rank;    /* zero for primary... */
    uint32_t tx_vqId;
    uint32_t rx_vqId;
    uint32_t procId;
} g_vqTable[MAX_VQ_PAIRS];


/**
 *  \brief VirtioIPC_getVirtQueues :
 *           Look up the VirtQueues by procId.
 *           Rank is 0 for primary VQs.
 */
Bool VirtioIPC_getVirtQueues(uint32_t type, uint32_t procId, uint32_t rank,
                             uint32_t *tx_vqId, uint32_t *rx_vqId)
{
    uint32_t i;
    Bool retVal = FALSE;

    for (i = 0; i < g_vqItemCnt; i++)
    {
        if ((type   == g_vqTable[i].virtioId) &&
            (rank   == g_vqTable[i].rank)     &&
            (procId == g_vqTable[i].procId))
        {
            *tx_vqId = g_vqTable[i].tx_vqId;
            *rx_vqId = g_vqTable[i].rx_vqId;
            retVal   = TRUE;
            break;
        }
    }

    return retVal;
}

static int32_t VirtioIPC_createVirtioCorePair(Ipc_VirtioInfo* vqInfo, uint32_t timeoutCnt)
{
    Vring_Params   params;
    Virtio_Handle  tx_vq, rx_vq;
    uint8_t        status = 0;
    int32_t        retVal = IPC_SOK;

    /* TX VQ */
    params.num   = vqInfo->num;
    params.addr  = (uintptr_t)mapPAtoVA(vqInfo->daTx);
    params.align = vqInfo->align;
    tx_vq = Virtio_create(vqInfo->txNotifyId, vqInfo->remoteId, NULL,
                  &params, VIRTIO_TX, status, timeoutCnt);
    if (tx_vq == NULL)
    {
        SystemP_printf("TX Virtio creation failure: %d\n", vqInfo->txNotifyId);
        retVal = IPC_EFAIL;
    }
    if(retVal == IPC_SOK)
    {
        /* Prime the transmit buffer */
        uint32_t *primeBuf = (uint32_t *)mapPAtoVA(vqInfo->primeBuf);
        Virtio_prime(tx_vq, (uintptr_t)primeBuf, vqInfo->num);

        /* RX VQ */
        params.num   = vqInfo->num;
        params.addr  = (uintptr_t)mapPAtoVA(vqInfo->daRx);
        params.align = vqInfo->align;
        rx_vq = Virtio_create(vqInfo->rxNotifyId, vqInfo->remoteId, NULL,
                  &params, VIRTIO_RX, status, timeoutCnt);
        if (rx_vq == NULL)
        {
            SystemP_printf("RX Virtio creation failure: %d\n", vqInfo->rxNotifyId);
            retVal = IPC_EFAIL;
        }
    }

    if(retVal == IPC_SOK)
    {
        /* Add to global registry */
        g_vqTable[g_vqItemCnt].virtioId = VIRTIO_ID_RPMSG;
        g_vqTable[g_vqItemCnt].rank     = 0;
        g_vqTable[g_vqItemCnt].tx_vqId  = vqInfo->txNotifyId;
        g_vqTable[g_vqItemCnt].rx_vqId  = vqInfo->rxNotifyId;
        g_vqTable[g_vqItemCnt].procId   = vqInfo->remoteId;
        g_vqItemCnt++;
    }

    return retVal;
}

Bool Ipc_isRemoteVirtioCreated(uint32_t remoteId)
{
    Bool      vqCreated = FALSE;
    uint32_t  idx     = 0;

    for(idx = 0; idx < (uint32_t)g_vqItemCnt; idx++)
    {
        if (remoteId == g_vqTable[idx].procId)
        {
            vqCreated = TRUE;
            break;
        }
    }
    return vqCreated;
}

static void Ipc_updateVirtioInfo(uint32_t numProc, const void *baseAddr, uint32_t vrBufSize,
                      Ipc_VirtioInfo* info)
{
    uint32_t cnt = 0, a , b, i;
#ifdef QNX_OS
    uint32_t revPrimeBuf;
#endif

    if(info->remoteId > info->selfId)
    {
        a = info->selfId;
        b = info->remoteId;
    }
    else
    {
        a = info->remoteId;
        b = info->selfId;
    }

    for(i = 0; i < a; i++)
    {
        cnt += (numProc - i - 1U);
    }
    cnt += (b - a - 1U);
    cnt *= 4U;

    if(info->remoteId > info->selfId)
    {
        info->daTx       = (uint32_t)((uint32_t)(uintptr_t)baseAddr + (cnt * vrBufSize));
        info->daRx       = (uint32_t)((uint32_t)(uintptr_t)baseAddr + ((cnt+2U) * vrBufSize));
        info->primeBuf   = (uint32_t)((uint32_t)(uintptr_t)baseAddr + ((cnt+1U) * vrBufSize));
#ifdef QNX_OS
        revPrimeBuf      = (uint32_t)((uintptr_t)baseAddr + (cnt+3) * vrBufSize);
#endif
        info->txNotifyId = 1;
        info->rxNotifyId = 0;
    }
    else
    {
        info->daTx       = (uint32_t)((uint32_t)(uintptr_t)baseAddr + ((cnt+2U) * vrBufSize));
        info->daRx       = (uint32_t)((uint32_t)(uintptr_t)baseAddr + (cnt * vrBufSize));
        info->primeBuf   = (uint32_t)((uint32_t)(uintptr_t)baseAddr + ((cnt+3U) * vrBufSize));
#ifdef QNX_OS
        revPrimeBuf      = (uint32_t)((uintptr_t)baseAddr + (cnt+1) * vrBufSize);
#endif
        info->txNotifyId = 0;
        info->rxNotifyId = 1;
    }

    /* If remote core is Linux, and vdev is ready
     * update the address
     */
    if(TRUE == Virtio_isRemoteLinux((uint16_t)(info->remoteId)))
    {
        Ipc_ResourceTable *rsc = (Ipc_ResourceTable*)gRscTable;
        info->daTx      = rsc->rpmsg_vring0.da;
        info->daRx      = rsc->rpmsg_vring1.da;
        info->primeBuf  = info->daTx + (2U*vrBufSize);
    }

#ifdef QNX_OS
    /* For QNX, mmap to local process space */
    Ipc_addTranslationEntry(info->daTx, vrBufSize);
    Ipc_addTranslationEntry(info->daRx, vrBufSize);
    Ipc_addTranslationEntry(info->primeBuf, vrBufSize);
    Ipc_addTranslationEntry(revPrimeBuf, vrBufSize);
#endif
#if defined DEBUG_PRINT
    SystemP_printf("Virtio: remote %d, daTx 0x%x, daRx 0x%x, prime 0x%x\n",
        info->remoteId, info->daTx, info->daRx, info->primeBuf);
#endif

}

int32_t VirtioIPC_init(Ipc_VirtIoParams *vqParams)
{
    int32_t          retVal = IPC_SOK;
    uint32_t         selfId;
    uint32_t         numProcessors = Ipc_mpGetNumProcessors();
    Ipc_VirtioInfo   vqInfo;
    uint32_t         procId;
    uint32_t         idx;

    selfId = Ipc_mpGetSelfId();

    vqInfo.align = IPC_VRING_ALIGNMENT;
    vqInfo.num   = IPC_VRING_BUF_CNT;
    vqInfo.selfId = selfId;

    for(idx = 0; idx < numProcessors; idx++)
    {
        procId = Ipc_mpGetRemoteProcId(idx);

        /* if virtio is already created for this core then ignore */
        if(TRUE == Ipc_isRemoteVirtioCreated(procId))
        {
            continue;
        }

        if(FALSE == Ipc_isRemoteReady((uint16_t)procId))
        {
            /* Linux on A72 is not ready, go to next core
             * Virtio is not created at this moment, it must be
             * created on demand.
             */
            continue;
        }
        vqInfo.remoteId   = procId;
        Ipc_updateVirtioInfo(IPC_MAX_PROCS, vqParams->vringBaseAddr, IPC_VRING_SIZE, &vqInfo);
        retVal = VirtioIPC_createVirtioCorePair(&vqInfo, vqParams->timeoutCnt);
        if(retVal != IPC_SOK)
        {
            SystemP_printf("VirtioIPC_init: Failed to create VirtIO for procId..\n",
                 procId);
            break;
        }
    }
    return retVal;
}

/**
 * \brief Creates Virtio late when Linux is ready
 *
 * \param procId  [IN] Id of remote processor
 *
 * \return #IPC_SOK or #IPC_EFAIL
 */
int32_t Ipc_lateVirtioCreate(uint16_t procId)
{
    int32_t             retVal = IPC_SOK;
    Ipc_ResourceTable  *rsc    = NULL;
    Ipc_VirtioInfo      vqInfo;
    uint32_t            vrBufSize = IPC_VRING_SIZE;

    if(TRUE == Ipc_isRemoteReady(procId))
    {
        /* Initialize the structure */
        memset(&vqInfo, 0, sizeof(Ipc_VirtioInfo));

        vqInfo.remoteId   = procId;
        vqInfo.selfId     = Ipc_mpGetSelfId();

        rsc = (Ipc_ResourceTable*)gRscTable;
        if(rsc != NULL)
        {
            vqInfo.align     = rsc->rpmsg_vring0.align;
            vqInfo.num       = rsc->rpmsg_vring0.num;
            vqInfo.daTx      = rsc->rpmsg_vring0.da;
            vqInfo.daRx      = rsc->rpmsg_vring1.da;
            vqInfo.primeBuf  = vqInfo.daTx + (2U*vrBufSize);

            retVal = VirtioIPC_createVirtioCorePair(&vqInfo, 100);
            if(retVal != IPC_SOK)
            {
                SystemP_printf("Ipc_lateVirtioCreate: Failed to create VirtIO for procId..\n",
                    procId);
            }
        }
    }

    return retVal;
}

