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
 *  \file ipc_virtioPrivate.h
 *
 *  \brief symmetric virtio interface.
 */

#ifndef IPC_VIRTIOPRIVATE_H_
#define IPC_VIRTIOPRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/drv/ipc/include/ipc_types.h>
#include <ti/osal/osal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define VIRTIOIPC_RPMSG   7

/**
 *  \brief Enumeration for Virtio direction
 */
typedef enum VIRTIO_DIR_e
{
    VIRTIO_TX,
    VIRTIO_RX
} VIRTIO_DIR;

typedef struct Virtio_Object_s* Virtio_Handle;

/**
 *  @brief   Signature of a callback function that can be registered
 *           with the Virtio
 */
typedef void (*Virtio_callback)(uint32_t* arg);

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


/**
 *  \brief      Notify a peer processor of new buffers in a Virtio
 *
 *  After one or more call to Virtio_addUsedBuf(), the peer
 *  processor is notified of the change by calling Virtio_kick().
 *
 *  \param  vq [IN]        Virtio handle
 *
 *  \ref         Virtio_addBuf
 */
void Virtio_kick(Virtio_Handle vq);

/**
 *  \brief      Sets the callback for a RX Virtio.
 *
 *  Installs a callback function into a Virtio.  The callback
 *  function will be executed in HWI context.
 *
 *  \param  vqid     [IN]   Virtio Id
 *  \param  callback [IN]   Address of the callback function
 *  \param  priv     [IN]   Private data to be passed to the callback
 *                          function
 *
 *  \returns    Returns 0 on success.
 *
 */
int32_t Virtio_setCallback(uint32_t vqid, Virtio_callback callback, uint32_t* priv);

/**
 *  \brief      Get the handle to the Virtio identified by its vqId.
 *
 *  Returns the handle for a Virtio.
 *
 *  \param  vqid [IN]   Virtio Id
 *  \param  dir  [IN]   direction of virtio tx or rx
 *
 *  \returns    Returns a Virtio Handle or NULL if no Virtio
 *              exists with that vqId.
 *
 */
Virtio_Handle Virtio_getHandle(uint32_t vqid, VIRTIO_DIR dir);

/**
 *  \brief      Get the procId of the peer processor.
 *
 *  Returns the handle for a Virtio.
 *
 *  \param  vq [IN] Virtio handle
 *
 *  \returns    Returns a procId
 *
 *  \sa         VirtQueue_getProcId
 */
uint32_t Virtio_getProcId(Virtio_Handle vq);

/**
 *  \brief      Lookup Virtio to use with a peer processor.
 *
 *  All Virtio are created by the call to VirtioIPC_init().
 *  VirtioIPC_getVirtQueues() is called to get the Ids of the
 *  TX and RX Virtio based on the type, procId and rank.
 *  The type will one of the value from VirtioIPC_vqdev_types.
 *  ProcId determines the processor to comminacate with.  Rank
 *  selects between the primary (0) and any addtional (>0) Virtio
 *  created between the local processor and the one specified by procId.
 *  Rank values are determined by the order that the Virtio are
 *  defined within a Shared Page.
 *
 *  \param   type     [IN]   The type of Virtio Device
 *  \param   procId   [IN]   procId of the peer processor
 *  \param   rank     [IN]   rank - 0 for primary Virtio
 *  \param   tx_vqId  [Out]  Id of the transmitting Virtio
 *  \param   rx_vqId  [OUT]  Id of the receiving Virtio
 *
 *  \returns    Returns TRUE on success
 *
 *  \sa         VirtioIPC_getVirtQueues
 */
Bool VirtioIPC_getVirtQueues(uint32_t type, uint32_t procId, uint32_t rank,
        uint32_t *tx_vqId, uint32_t *rx_vqId);

/**
 *  \brief      Initializes the Virtio IPC module
 *
 *  VirtioIPC_init() will create Virtio from all the Shared Pages
 *  found in the Shared Page Table.  The numProcessors argument
 *  must contain the number of rows in the table.  It should
 *  be the same as the number of processors declared in MultiProc.
 *
 *  \param  tableAddress   [IN]     Address of the Shared Page Table
 *  \param  numProcessors  [IN]     The number of rows in the table
 *  \param  vqBuf          [IN]     Buffer to hold the virtio array
 *  \param  vqBufSize      [IN]     Buffer Size for virtio array
 *
 *  \returns    Returns the number of shared pages found for
 *              the local processor.
 *
 *  \sa         VirtioIPC_init
 */
int32_t VirtioIPC_init(Ipc_VirtIoParams* vqParams);

/**
 *  \brief      Create Virtio from a Shared Page
 *
 *  Two Virtio will be created according to the configuration
 *  found in the Shared Page.
 *
 *  \param  shared_page  [IN]      Address of the Shared Page
 *
 *  \returns    Returns zero on success
 *
 *  \sa         VirtioIPC_initFromSharedPage
 */
/* Not Needed */
/* int32_t VirtioIPC_initFromSharedPage(void *shared_page, void** vqBuf); */

/**
 *  \brief      Check if a Virtio is ready.
 *
 *  When FALSE a VirtQueue has not been initialized.  When TRUE it is
 *  safe to check it for messages.
 *
 *  \param  vq  [IN]      VirtQueue handle
 *
 *  \returns    Returns TRUE or FALSE
 *
 *  \sa         Virtio_isReady
 */
uint8_t Virtio_isReady(Virtio_Handle vq);

/**
 *  \brief Ipc_allocVirtio
 */
void* Ipc_allocVirtio(void);

/*
 *  ============================================================================
 *  Host Only Functions:
 *  ============================================================================
 */

/**
 *  \brief      Add available buffer to virtio's available buffer list.
 *              Only used by Host.
 *
 *  \param  vq  [IN]      the Virtio.
 *  \param  buf [IN]     the buffer to be processed by the slave.
 *  \param  head[IN]     used for indexing the descriptor list
 *
 *  \return     Remaining capacity of queue or a negative error.
 *
 *  \sa         Virtio_getUsedBuf
 */
void Virtio_addAvailBuf(Virtio_Handle vq, void *buf, uint16_t head);

/**
 *  \brief      Get the next used buffer.
 *              Only used by Host.
 *
 *  \param  vq [IN]       the Virtio.
 *
 *  \return     Returns NULL or the processed buffer.
 *
 *  \sa         Virtio_addAvailBuf
 */
void *Virtio_getUsedBuf(Virtio_Handle vq, uint16_t *token);

/*
 *  ============================================================================
 *  Slave Only Functions:
 *  ============================================================================
 */

/**
 *  \brief      Get the next available buffer.
 *              Only used by Slave.
 *
 *  \param  vq   [IN]     the Virtio.
 *  \param  buf  [OUT]    Pointer to location of available buffer;
 *
 *  \return     Returns a token used to identify the available buffer, to be
 *              passed back into Virtio_addUsedBuf();
 *              token is negative if failure to find an available buffer.
 *
 *  \sa         Virtio_addUsedBuf
 */
int16_t Virtio_getAvailBuf(Virtio_Handle vq, void **buf, int32_t *len);

/**
 *  \brief      Add used buffer to virtio's used buffer list.
 *              Only used by Slave.
 *
 *  \param  vq    [IN]    the Virtio.
 *  \param  head [IN]    token/index of the buffer to be added to vring used list.
 *
 *  \return     Remaining capacity of queue or a negative error.
 *
 *  \sa         Virtio_getAvailBuf
 */
int32_t Virtio_addUsedBuf(Virtio_Handle vq, int16_t head, int32_t len);

/**
 * \brief    Checks if remote is A72 with Linux
 *
 * \param  procId  [IN] Remote proc Id
 *
 * \return  1 if remote core is A72 & valid resource table
 *          0 otherwise
 *
 */
uint8_t Virtio_isRemoteLinux(uint16_t procId);

/**
 *  \brief Virtio_enableCallback
 */
uint8_t Virtio_enableCallback(Virtio_Handle vq);

/**
 *  \brief Virtio_disableCallback
 */
void Virtio_disableCallback(Virtio_Handle vq);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IPC_VIRTIOPRIVATE_H_ */

