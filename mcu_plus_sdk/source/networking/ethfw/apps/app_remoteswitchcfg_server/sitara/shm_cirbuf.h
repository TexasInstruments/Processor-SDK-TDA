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

#ifndef SHM_CIRBUF_H
#define SHM_CIRBUF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define SHDMEM_MAGIC                    (0xABCDABCD)
#define ShmCirBuf_assert(cond, ...)     assert(cond)

typedef enum ShdMemStatus_t
{
    SHDMEM_STATUS_OK                  = 0,
    SHDMEM_STATUS_ERR_EMPTY           = -1,
    SHDMEM_STATUS_ERR_FULL            = -2,
    SHDMEM_STATUS_ERR_HEAD_CURRUPT    = -3, /*! Head refers to write Index */
    SHDMEM_STATUS_ERR_TAIL_CURRUPT    = -4, /*! Tail refers to Read Index */
    SHDMEM_STATUS_ERR_ELEM_CURRUPT    = -5,
    SHDMEM_STATUS_ERR_NO_INIT         = -6,
    SHDMEM_STATUS_ERR_HANDLER_CORRUPT = -7,
    SHDMEM_STATUS_ERR_BAD_PKT         = -8,
    SHDMEM_STATUS_ERR_READ_NO_DATA    = -9,
} ShdMemStatus;

typedef void* shm_handle;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief  This API creates shared-memory object.
 *
 * \param  pStartAdd      base address of the shared memory
 * \param  bufSize        total buffer size including overhead
 * \return shm_handle     handle of the shared-memory object
 *
 */
shm_handle shm_create(const uint32_t startAdd, const uint32_t bufSize);

/**
 * \brief  This API returns int16_t* data pointer for read AVB buffer.
 *
 * \param  hBuff          handle of shared memory object
 * \param  pData          Pointer to hold the buffer location
 * \param  dataLen        Size of requested buffer
 *
 */
ShdMemStatus shm_read(shm_handle hBuff, uint8_t* pData, uint16_t* pDataLen);

/**
 * \brief  This API returns int16_t* data pointer for read AVB buffer.
 *
 * \param  hBuff          handle of shared memory object
 * \param  pData          Pointer to hold the buffer location
 * \param  dataLen        Size of requested buffer
 *
 */
ShdMemStatus shm_readBufPtr(shm_handle hShmMem, uint8_t** pData, uint16_t* pDataLen);

/**
 * \brief  This API returns overhead of the shared-memory-object.
 *
 * \param  none
 *
 */
uint32_t shm_metadata_overhead();

/**
 * \brief  This API returns int16_t* data pointer for write AVB buffer.
 *
 * \param  hBuff          handle of shared memory object
 * \param  pData          Pointer to hold the buffer location
 * \param  dataLen        Size of requested buffer
 *
 */
ShdMemStatus shm_writeBufPtr(shm_handle hBuff, int16_t** pData, uint32_t dataLen);

/**
 * \brief  This API writes AVB data to be consumed by remote core.
 *
 * \param  hBuff          handle of shared memory object
 * \param  pData          Pointer buffer to be written
 * \param  dataLen        Size of buffer
 *
 */
ShdMemStatus shm_write(shm_handle hBuff, void* pData, uint32_t dataLen);


#ifdef __cplusplus
}
#endif

#endif // SHM_CIRBUF_H
