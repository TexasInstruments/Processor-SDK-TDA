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

/* All firmware defines will be maintained here */
#ifndef TAPFIRMWARE_H_
#define TAPFIRMWARE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h> /* mmap */

/* define USER-SPACE BUFFER SIZE */
#define TAP_BUFFER_SIZE 16384
/* RX and TX Queue Init Wait time */
#define SLEEP_RXQ_TXQ 100000

/**** START OF QUEUE RELATED DEFINITIONS ****/
#define NUM_Q_NODES (2048U)
/* General Success and Failure returns */
#define ICQ_RETURN_SUCCESS      (0)
#define ICQ_RETURN_FAILURE      (-1)

/* Specific error codes */    
#define ICQ_ERR_BASE            (-0x1000)
#define ICQ_ERR_QFULL           (ICQ_ERR_BASE - 1)     
#define ICQ_ERR_QEMPTY          (ICQ_ERR_BASE - 2)
/* Queue Init Magic Number */
#define IC_QUEUE_INIT_DONE              (0xABCDABCDU)
/**** END OF QUEUE RELATED DEFINITIONS ****/

/**** START OF BUFPOOL RELATED DEFINITIONS ****/
#define NUM_BUFFERS (1280U)
/* Bufpool check values */
#define BUFPOOL_OK          (0)       
#define BUFPOOL_ERROR       (-1)
#define BUFPOOL_INIT_DONE               (0xABCDABCDU)
/**** END OF BUFPOOL RELATED DEFINITIONS ****/

/* Maximum Ethernet Payload Size. */
#ifdef _INCLUDE_JUMBOFRAME_SUPPORT
#define ETH_MAX_PAYLOAD     10236
#else
#define ETH_MAX_PAYLOAD     1514
#endif
#define VLAN_TAG_SIZE 4
#define ETH_FRAME_SIZE (ETH_MAX_PAYLOAD + VLAN_TAG_SIZE)

/**** START OF QUEUE RELATED STRUCTS ****/
typedef struct IcQ_Node_s
{
	/* Data buffer pointer treated as an integer */
	/* and contains the physical address of the buffer */
	uint32_t DataBuffer;
	
	/*! Data buffer length */
	uint32_t dataBufferLen;

	/*! Packet ID */
	uint32_t packetId;

	/*! Reserved */
	uint32_t reserved;

} IcQ_Node;

typedef struct IcQ_s
{
	/*! Queue ID */
	uint32_t queueId;

	/*! Pointer to the head descriptor */
	uint32_t head;

	/*! Pointer to the tail descriptor */
	uint32_t tail;

	/*! Magic number used to check queue initialization */
	uint32_t magic;

	/*! Maximum no of nodes in the queue (fixed) */
	uint32_t maxSize;

	/*! Contiguous array of queue nodes */
	IcQ_Node nodeArray[NUM_Q_NODES];
}IcQ;

typedef struct IcQ_s* IcQ_Handle;

/**** END OF QUEUE RELATED STRUCTS ****/

/**** START OF BUFPOOL RELATED STRUCTS ****/

typedef struct Bufpool_Buf_s
{
	/*! Data buffer */
	uint8_t payload[ETH_FRAME_SIZE];

	/*! Valid payload length */
	uint16_t payloadLen;

	/*! Reference count: 0 means the buffer is free */
	int16_t refCount;

	/*! Is buffer in use (1) or free (0) */
	int16_t isUsed;

	/*! Pool ID to which this buffer belongs */
	uint32_t poolId;

	/*! Padding to keep the buffer size 128 byte aligned */
	uint8_t pad[8];

}Bufpool_Buf;

typedef struct Bufpool_Pool_s
{
	/*! Pool ID */
	uint32_t poolId;

	/*! Pointer to the last freed buffer */ 
	int32_t lastFreed;

	/*! Pointer to the last allocated buffer */
	int32_t lastAlloc;

	/*! Magic number used to check pool initialization */
	uint32_t magic;

	/*! Maximum no of buffers in the pool (fixed) */
	uint32_t maxSize;

	/*! Stats */
	uint32_t numBufGet;
	uint32_t numBufFree;
	uint32_t numBufGetErr;

	/*! Contiguous array of Bufpool_Buf objects */
	Bufpool_Buf buf_array[NUM_BUFFERS];

}Bufpool_Pool;

typedef struct Bufpool_Pool_s* Bufpool_Handle;

/**** START OF QUEUE FUNCTION DECLARATIONS ****/
int32_t IcQueue_initQ(IcQ_Handle hIcQueue,
                      uint32_t queueId,
                      uint32_t maxSize);
void IcQueue_freeQ(IcQ_Handle hIcQueue);
void IcQueue_resetQ(IcQ_Handle hIcQueue);
int32_t IcQueue_enq(IcQ_Handle hIcQueue,
                    IcQ_Node *pNode);
IcQ_Node* IcQueue_deq(IcQ_Handle hIcQueue);
bool IcQueue_isQEmpty(IcQ_Handle hIcQueue);
bool IcQueue_isQFull(IcQ_Handle hIcQueue);
uint32_t IcQueue_getQMaxSize(IcQ_Handle hIcQueue);
uint32_t IcQueue_getQCount(IcQ_Handle hIcQueue);
bool IcQueue_isQValid(IcQ_Handle hIcQueue);

/**** END OF BUFPOOL FUNCTION DECLARATIONS ****/

/**** START OF BUFPOOL FUNCTION DECLARATIONS ****/

int32_t Bufpool_init(Bufpool_Handle hBufpool,
                     uint32_t poolId,
                     uint32_t maxSize);
Bufpool_Buf* Bufpool_getBuf(Bufpool_Handle hBufpool);
int32_t Bufpool_freeBuf(Bufpool_Buf* hBuf);

/**** END OF BUFPOOL FUNCTION DECLARATIONS ****/

#endif /* TAPFIRMWARE_H_ */
