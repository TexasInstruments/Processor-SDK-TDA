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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "string.h"
#include <kernel/dpl/CacheP.h>
#include "shm_cirbuf.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

typedef struct ShdMemObj_t
{
	uint32_t 	header;
	uint32_t	readAddr;
	uint32_t	writeAddr;
	uint32_t	startAddr;
	uint32_t    endAddr;
	uint32_t    dataAvail;
	uint32_t 	readErrCnt;
	uint32_t 	writeErrCnt;
	uint32_t	readErrorEmpty;
	uint32_t	writeErrorFull;
	uint32_t 	readError;
	uint32_t 	writeError;
	uint32_t    totalBufSize;
	uint32_t	tail;
} ShdMemObj;

#define CACHE_LINE_SIZE		128

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/*! None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*! None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint32_t shm_metadata_overhead()
{
	//return sizeof(ShdMemObj);
	return CACHE_LINE_SIZE;
}

shm_handle shm_create(const uint32_t startAdd, const uint32_t bufSize)
{
	ShdMemObj*	obj = (ShdMemObj*)startAdd;
	if(obj->header != SHDMEM_MAGIC)
	{
		uint32_t  dataAddr	= startAdd + CACHE_LINE_SIZE; //sizeof(ShdMemObj);
		uint32_t  dataSize 	= bufSize - CACHE_LINE_SIZE; //sizeof(ShdMemObj);
		obj->header 		= SHDMEM_MAGIC;
		obj->readAddr 		= dataAddr;
		obj->writeAddr 		= dataAddr;
		obj->startAddr		= dataAddr;
		obj->endAddr 		= dataAddr + dataSize;
		obj->readErrCnt 	= 0;
		obj->writeErrCnt	= 0;
		obj->readErrorEmpty	= 0;
		obj->writeErrorFull	= 0;
		obj->tail 			= SHDMEM_MAGIC;
		obj->totalBufSize 	= dataSize;
		obj->dataAvail  	= 0;
	}

	return (shm_handle)obj;
}

ShdMemStatus shm_read(shm_handle hShmMem, uint8_t* pData, uint16_t* pDataLen)
{
	ShdMemStatus	status 		= SHDMEM_STATUS_OK;
	ShdMemObj*		obj    		= (ShdMemObj*)hShmMem;
	uint32_t 		rqtSize 	= *pDataLen;
	uint32_t		readAddr 	= obj->readAddr;
	uint32_t		wrtAddr 	= obj->writeAddr;
	uint32_t		endAddr 	= obj->endAddr;
	uint32_t		dataSize 	= obj->totalBufSize;
	uint32_t 		dataAvail	= 0;

	if(wrtAddr >= readAddr)
	{
		dataAvail = wrtAddr - readAddr;
	}
	else
	{
		dataAvail = dataSize - readAddr - wrtAddr;
	}

	if((obj->header == SHDMEM_MAGIC) &&
	   (dataAvail >= rqtSize))
	{
		if((readAddr + rqtSize) <= endAddr)
		{
			memcpy((void*)pData, (void*)readAddr, rqtSize);
			readAddr += rqtSize;
			*pDataLen = rqtSize;

			if(readAddr == endAddr)
			{
				readAddr = obj->startAddr;
			}

			obj->readAddr = readAddr;
			CacheP_wb((void *)&(obj->readAddr), sizeof(readAddr), CacheP_TYPE_ALL);
		}
		else
		{
			obj->readErrorEmpty++;
		}
	}
	else
	{
		*pDataLen = 0;
		obj->readErrCnt++;
	}

	return status;
}

ShdMemStatus shm_readBufPtr(shm_handle hShmMem, uint8_t** pData, uint16_t* pDataLen)
{
	ShdMemStatus	status 		= SHDMEM_STATUS_OK;
	ShdMemObj*		obj    		= (ShdMemObj*)hShmMem;
	uint32_t 		rqtSize 	= *pDataLen;
	uint32_t		readAddr 	= obj->readAddr;
	uint32_t		wrtAddr 	= obj->writeAddr;
	uint32_t		endAddr 	= obj->endAddr;
	uint32_t		dataSize 	= obj->totalBufSize;
	uint32_t 		dataAvail	= 0;

	if(wrtAddr >= readAddr)
	{
		dataAvail = wrtAddr - readAddr;
	}
	else
	{
		dataAvail = dataSize - readAddr - wrtAddr;
	}

	if((obj->header == SHDMEM_MAGIC) &&
	   (dataAvail >= rqtSize))
	{
		if((readAddr + rqtSize) <= endAddr)
		{
			*pData = (uint8_t*)readAddr;
			readAddr += rqtSize;
			*pDataLen = rqtSize;

			if(readAddr == endAddr)
			{
				readAddr = obj->startAddr;
			}

			obj->readAddr = readAddr;
			CacheP_wb((void *)&(obj->readAddr), sizeof(readAddr), CacheP_TYPE_ALL);
		}
		else
		{
			obj->readErrorEmpty++;
		}
	}
	else
	{
		*pDataLen = 0;
		obj->readErrCnt++;
	}

	return status;
}

ShdMemStatus shm_write(shm_handle hShmMem, void* pData, uint32_t dataLen)
{
	ShdMemStatus	status 	= SHDMEM_STATUS_OK;
	ShdMemObj*		obj    	= (ShdMemObj*)hShmMem;
	uint32_t		wrtAddr = obj->writeAddr;
	uint32_t		endAddr = obj->endAddr;

	if(obj->header == SHDMEM_MAGIC)
	{
		if(wrtAddr+dataLen <= endAddr)
		{
			memcpy((void*)wrtAddr, pData, dataLen);
			CacheP_wb((void *)wrtAddr, dataLen, CacheP_TYPE_ALL);
			wrtAddr += dataLen;

			if(wrtAddr == endAddr)
			{
				wrtAddr = obj->startAddr;
			}

			obj->writeAddr = wrtAddr;
			CacheP_wb((void *)&(obj->writeAddr), sizeof(wrtAddr), CacheP_TYPE_ALL);

		} else {
			obj->writeErrorFull++;
		}
	}

	return status;
}

ShdMemStatus shm_writeBufPtr(shm_handle hShmMem, int16_t** pData, uint32_t dataLen)
{
	ShdMemStatus	status 	= SHDMEM_STATUS_OK;
	ShdMemObj*		obj    	= (ShdMemObj*)hShmMem;
	uint32_t		wrtAddr = obj->writeAddr;
	uint32_t		endAddr = obj->endAddr;

	if(obj->header == SHDMEM_MAGIC)
	{
		if(wrtAddr+dataLen <= endAddr)
		{
			*pData = (int16_t*)wrtAddr;
			wrtAddr += dataLen;

			if(wrtAddr == endAddr)
			{
				wrtAddr = obj->startAddr;
			}

			obj->writeAddr = wrtAddr;
			CacheP_wb((void *)&(obj->writeAddr), sizeof(wrtAddr), CacheP_TYPE_ALL);

		} else {
			obj->writeErrorFull++;
		}
	}

	return status;
}

