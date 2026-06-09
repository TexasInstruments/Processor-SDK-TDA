/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

/* All firmware associated functions will be maintained here */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "tapfirmware.h"

/* Global Variables for Virtual Addresses*/
extern IcQ_Handle IcQ_globalQTable_Handle;
extern Bufpool_Handle BufpoolTable_Handle;

/**** START OF QUEUE RELATED FUNCTION DEFINITIONS ****/

bool IcQueue_isQValid(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);

	if(IC_QUEUE_INIT_DONE == hIcQueue->magic)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool IcQueue_isQFull(IcQ_Handle hIcQueue)
{
	uint32_t head;

	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	/* Check for wrap-around. This approach wastes one slot in the
	* queue but does not require producer-consumer synchronization
	*/ 
	head = hIcQueue->head + 1;

	if(head == hIcQueue->maxSize)
	{
		head = 0;
	}

	return (head == hIcQueue->tail);
}

bool IcQueue_isQEmpty(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	return (!IcQueue_isQFull(hIcQueue) && (hIcQueue->head == hIcQueue->tail));
}

/* Used when an element is enqueued */
static void IcQueue_moveHead(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	if(IcQueue_isQFull(hIcQueue))
	{
		if(++(hIcQueue->tail) == hIcQueue->maxSize)
		{
		    hIcQueue->tail = 0;
		}
	}

	if(++(hIcQueue->head) == hIcQueue->maxSize)
	{
		hIcQueue->head = 0;
	}
}

/* Used when an element is dequeued */
static void icQueue_moveTail(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	if(++(hIcQueue->tail) == hIcQueue->maxSize)
	{
		hIcQueue->tail = 0;
	}
}

/* Ignoring IcQueue_initQ function */

void IcQueue_freeQ(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	free(hIcQueue);
}

void IcQueue_resetQ(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	hIcQueue->head = 0;
	hIcQueue->tail = 0;
}

uint32_t IcQueue_getQCount(IcQ_Handle hIcQueue)
{
	uint32_t count;

	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	count = hIcQueue->maxSize;

	if(!IcQueue_isQFull(hIcQueue))
	{
		if(hIcQueue->head >= hIcQueue->tail)
		{
			count = (hIcQueue->head - hIcQueue->tail);
		}
		else
		{
	    		count = (hIcQueue->maxSize + hIcQueue->head - hIcQueue->tail);
		}
	}
	return count;
}

uint32_t IcQueue_getQMaxSize(IcQ_Handle hIcQueue)
{
	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	return hIcQueue->maxSize;
}

int32_t IcQueue_enq(IcQ_Handle hIcQueue,
                    IcQ_Node *pNode)
{
	int32_t retVal = ICQ_RETURN_FAILURE;

	assert(hIcQueue && pNode);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	if(!IcQueue_isQFull(hIcQueue))
	{
		hIcQueue->nodeArray[hIcQueue->head].DataBuffer   = pNode->DataBuffer;
		hIcQueue->nodeArray[hIcQueue->head].dataBufferLen = pNode->dataBufferLen;
		hIcQueue->nodeArray[hIcQueue->head].packetId      = pNode->packetId;
		IcQueue_moveHead(hIcQueue);
		retVal = ICQ_RETURN_SUCCESS;
	}
	else
	{
		retVal = ICQ_ERR_QFULL;
	}
	return retVal;
}

IcQ_Node* IcQueue_deq(IcQ_Handle hIcQueue)
{
	IcQ_Node* pNode = NULL;

	assert(hIcQueue != NULL);
	assert(hIcQueue->magic == IC_QUEUE_INIT_DONE);

	if(!IcQueue_isQEmpty(hIcQueue))
	{
		pNode = &hIcQueue->nodeArray[hIcQueue->tail];
		icQueue_moveTail(hIcQueue);
	}

	return pNode;
}

/**** END OF QUEUE RELATED FUNCTION DEFINITIONS ****/

/**** START OF BUFPOOL RELATED FUNCTION DEFINITIONS ****/
int32_t Bufpool_init(Bufpool_Handle hBufpool,
                     uint32_t poolId,
                     uint32_t maxSize)
{
	int32_t bufIdx  = 0;
	int32_t retVal = BUFPOOL_OK;

	assert(hBufpool && (maxSize<=NUM_BUFFERS));
	hBufpool->poolId    = poolId;
	hBufpool->lastAlloc = 0;
	hBufpool->numBufGet = 0;
	hBufpool->numBufFree   = 0;
	hBufpool->numBufGetErr = 0;
	hBufpool->maxSize   = maxSize;
	hBufpool->magic     = BUFPOOL_INIT_DONE;
	printf("Bufpool Init: Values Initialized\n");
	/* Clear all the buffers */
	/* Using custom alternative for memset */
	int pad_size = sizeof(hBufpool->buf_array[0].pad)/sizeof(hBufpool->buf_array[0].pad[0]);
	for(int i=0;i<maxSize;i++) {
		for(int j=0;j<ETH_FRAME_SIZE;j++) {
			hBufpool->buf_array[i].payload[j] = 0;
		}
		hBufpool->buf_array[i].payloadLen = 0;
		hBufpool->buf_array[i].refCount = 0;
		hBufpool->buf_array[i].isUsed = 0;
		hBufpool->buf_array[i].poolId = poolId;
		for(int j=0;j<pad_size;j++) {
			hBufpool->buf_array[i].pad[j] = 0;
		}
	}
	printf("Bufpool Init: Cleared Buffers\n");
	return retVal;
}

Bufpool_Buf* Bufpool_getBuf(Bufpool_Handle hBufpool)
{
	int32_t index;

	assert(hBufpool->magic == BUFPOOL_INIT_DONE);

	/* Start search at the last allocated node */

	/* Search from last alloc to end of pool */
	for(index=hBufpool->lastAlloc; index<hBufpool->maxSize; index++)
	{
		if(0 == hBufpool->buf_array[index].isUsed)
		{
			/* Mark the buffer used and return its address */
			hBufpool->buf_array[index].isUsed = 1;
			hBufpool->buf_array[index].refCount = 1;

			hBufpool->lastAlloc = index;
			hBufpool->numBufGet++;

			return &(hBufpool->buf_array[index]);
		}
	}

	/* Wrap around: search from 0 to last alloc */
	if(index == hBufpool->maxSize)
	{
		/* Search from 0 to last alloc */
		for(index=0; index<hBufpool->lastAlloc; index++)
		{
			if(0 == hBufpool->buf_array[index].isUsed)
			{
				/* Mark the buffer used and return its address */
				hBufpool->buf_array[index].isUsed = 1;
				hBufpool->buf_array[index].refCount = 1;

				hBufpool->lastAlloc = index;
				hBufpool->numBufGet++;

				return &(hBufpool->buf_array[index]);
			}
		}
	}

	/* No free buffers, search failed */    
	hBufpool->numBufGetErr++;
	return NULL;

}
int32_t Bufpool_freeBuf(Bufpool_Buf* hBuf)
{
	int32_t retVal = BUFPOOL_OK;

	assert(hBuf && (hBuf->refCount >=0));

	if(hBuf->refCount > 0)
	{
		--(hBuf->refCount);

		if(0 == hBuf->refCount)
		{
			/* Mark the buffer free */
			hBuf->isUsed = 0;
		}
	}

	/*
	* Here we don't have a direct bufpool handle so
	* refer to the pool using the global table and
	* the poolId available in hBuf
	*/
	BufpoolTable_Handle[hBuf->poolId].numBufFree++;
	return retVal;

}

/**** END OF BUFPOOL RELATED FUNCTION DEFINITIONS ****/
