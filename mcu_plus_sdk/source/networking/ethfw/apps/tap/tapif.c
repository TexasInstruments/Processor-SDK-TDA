/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

/* Source code for user-space application */
/* that makes use of a TAP device to read */
/* from and write to Linux Network Stack */
/* The user-space application acts as a medium */
/* to facilitate inter-core transport of */
/* Layer 2 (Ethernet Frames) PDUs */
/* Two threads: tx_task and rx_task */
/* manage the process of sending L2 PDUs from */
/* Linux Network stack to R5 bridge and from */
/* R5 bridge to Linux Network stack via shared */
/* memory region */

/* Headers for memcpy, mmap, stdint, tap device */
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>

/* Header to include firmware APIs and other data */
#include "tapfirmware.h"

/* Global Variables for Queue and Bufpool Handles*/
IcQ_Handle IcQ_globalQTable_Handle;
Bufpool_Handle BufpoolTable_Handle;

/* Struct for passing arguments to pthread_create */
struct arg_struct {
	/* TX/RX Queue Handle */
	IcQ_Handle arg_Q_Handle;
	/* TX Bufpool Handle */
	Bufpool_Handle arg_txBufpoolHandle;
	/* Queue Table Handle */
	IcQ_Handle arg_IcQ_globalQTable_Handle;
	/* Bufpool Table Handle */	
	Bufpool_Handle arg_BufpoolTable_Handle;
	/* Tap device File descriptor */	
	int arg_tap_fd;
	/* TX/RX Queue Polling Interval */
	uint32_t arg_q_polling_interval;
	/* Bufpool base address */
	uint32_t arg_bufpool_base_addr;
	/* Maximum number of frames to process */
	/* in a single wakeup cycle */
	uint32_t arg_max_frames; /*max_tx or max_rx */
};

/* Function to convert virtual address to physical address */
size_t virt_to_phy(void *virt_addr, void *virt_base_addr, size_t phy_base_addr)
{
	return (size_t) ((size_t)(virt_addr) - (size_t) (virt_base_addr) + phy_base_addr);
}

/* Function to convert physical address to virtual address */
void * phy_to_virt(size_t phy_addr, size_t phy_base_addr, void *virt_base_addr)
{
	return (void *)((size_t) virt_base_addr + phy_addr - phy_base_addr);
}

/* Function to open the TAP device named devname */
/* to enable the user-space application to read from */
/* and write to the Linux Network Stack via the TAP */
/* device. */
static int tap_open(char *devname)
{
	struct ifreq ifr;
	int fd, err;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd == -1) {
		perror("open /dev/net/tun");
		exit(1);
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, devname, IFNAMSIZ);
	err = ioctl(fd, TUNSETIFF, (void *) &ifr);
	if (err == -1) {
		perror("ioctl TUNSETIFF");
		close(fd);
		exit(1);
	}
	return fd;
}

/* TX task which polls the Linux Network Stack via the TAP */
/* device to check for any frames that have to be sent. */
/* In the presence of frames to be sent, the frame is copied */
/* to a buffer in the Bufpool and the address of the buffer */
/* is enqueued in the TX Queue */
static void tx_task(void *arguments)
{
	printf("Started TX Task\n");
	fflush(stdout);
	/* Fetching arguments for tx_task to use */
	struct arg_struct *args = (struct arg_struct *)arguments;
	/* Storing the values */
	IcQ_Handle txQHandle = args->arg_Q_Handle;
	Bufpool_Handle txBufpoolHandle = args->arg_txBufpoolHandle;
	IcQ_Handle IcQ_globalQTable_Handle = args->arg_IcQ_globalQTable_Handle;
	Bufpool_Handle BufpoolTable_Handle = args->arg_BufpoolTable_Handle;
	uint32_t q_polling_interval = args->arg_q_polling_interval;
	uint32_t bufpool_base_addr = args->arg_bufpool_base_addr;
	uint32_t max_tx = args->arg_max_frames;
	int tap_fd = args->arg_tap_fd;
	/* Variable to store output of read syscall */
	int nbytes = -1;
	/* Character buffer to store frame contents */
	unsigned char tap_buffer[TAP_BUFFER_SIZE];
	/* Variable to store physical address of Data Buffer */
	/* Variable to store number of frames transmitted in */
	/* current wake cycle */
	uint32_t phy_DataBuffer, tx_frame_count;
	/* Variable to store the length of frame that was read */
	/* from the Linux Network Stack */
	uint32_t frame_len;
	/* Variable to store return value of Queue Enqueue function */
	int32_t retVal;
	/* Pointer to Bufpool Buffer */
	Bufpool_Buf *pDriverBuf;
	/* Node that will be used to enqueue buffer containing frame */
	/* to be transmitted */
	IcQ_Node enq_node;
	while (1) {
		/* Set count of transmitted frames in current wake up cycle */
		/* to zero. */
		tx_frame_count = 0;
		while (tx_frame_count < max_tx) {
			/********** Tx Section **********/
			/* Read from TAP device to see if has data from network stack */
			nbytes = read(tap_fd, tap_buffer, sizeof(tap_buffer));
			frame_len = nbytes;
			if (frame_len > 0) {
				/* Request space in shared buffer pool */
				pDriverBuf = Bufpool_getBuf(txBufpoolHandle);
				/* Here, we receive a pointer in virtual address space */
				if (pDriverBuf == NULL)
				{
					break;
				}
				/* ASSUMPTION: Frame size is less than buffer size */
				assert(frame_len <= ETH_FRAME_SIZE);
				/* Copy frame to Virtual Address of Shared Buffer */
				/* payload is an array with each element of 8 bits */
				for (int index = 0; index < frame_len; index++) {
					pDriverBuf->payload[index] = tap_buffer[index];
				}
				/* Store payload information */                                          
				pDriverBuf->payloadLen = frame_len;
				
				/* Populate the queue node */
				/* NOTE: Here, we need to enqueue the true physical */
				/* address of the buffer and not the virtual address */
				/* Convert Data Buffer to physical address from virtual address */
				phy_DataBuffer = (uint32_t) virt_to_phy((void *)pDriverBuf,
									(void *) BufpoolTable_Handle,
									(uint32_t) bufpool_base_addr);

				/*  Add DataBuffer data and fill other details in q node */
				enq_node.DataBuffer = phy_DataBuffer;
				enq_node.dataBufferLen = frame_len;
				enq_node.packetId = tx_frame_count;
				
				/* Push it on the queue */
				retVal = IcQueue_enq(txQHandle, &enq_node);
				/* If retVal!=ICQ_RETURN_SUCCESS : Unable to Enqueue */
				assert(retVal == ICQ_RETURN_SUCCESS);
				/* Increment frame count */
				tx_frame_count++;
			} else {
				break;
			}
		}
		/* Sleep */
		usleep(q_polling_interval);
	}
	/* Clean up */
	free(args);
}

static void rx_task(void *arguments)
{
	printf("Started RX Task\n");
	fflush(stdout);
	/* Fetching arguments for tx_task to use */
	struct arg_struct *args = (struct arg_struct *)arguments;
	/* Storing the values */
	IcQ_Handle rxQHandle = args->arg_Q_Handle;
	IcQ_Handle IcQ_globalQTable_Handle = args->arg_IcQ_globalQTable_Handle;
	Bufpool_Handle BufpoolTable_Handle = args->arg_BufpoolTable_Handle;
	int tap_fd = args->arg_tap_fd;
	uint32_t q_polling_interval = args->arg_q_polling_interval;
	uint32_t bufpool_base_addr = args->arg_bufpool_base_addr;
	uint32_t max_rx = args->arg_max_frames;
	/* Variable to store output of read syscall */
	int nbytes = -1;
	/* Character buffer to store frame contents */
	unsigned char tap_buffer[TAP_BUFFER_SIZE];
	/* Variable to store length of received frame */
	uint32_t recv_frame_len;
	/* Variable to store return value of Bufpool_freeBuf function */
	int32_t retVal = -1;
	/* Variable to store number of frames received in current wake */
	/* cycle. */
	uint32_t  rx_frame_count = 0;
	/* Pointer to queue node that will be dequeued from RX Queue */
	IcQ_Node *p_deq_node = NULL;
	/* Pointer to Bufpool_Buf */
	Bufpool_Buf *pDriverBuf;
	while (1) {
		/* Set count of received frames in current wake up cycle */
		/* to zero. */
		rx_frame_count = 0;
		while (rx_frame_count < max_rx) {
			/********** Rx Section **********/
			/* Perform Dequeue to fetch descriptor from Queue */
			p_deq_node = IcQueue_deq(rxQHandle);
			/* If deq_node is not NULL and deq_node points to */
			/* non NULL Buffer Pool */
			if (p_deq_node != NULL) {
				/* p_deq_node is already virtual address */
				/* Convert physical address of buffer to virtual address */
				pDriverBuf = 
						(Bufpool_Buf *)phy_to_virt((size_t)p_deq_node->DataBuffer,
						(uint32_t) bufpool_base_addr, (void *)BufpoolTable_Handle);
				recv_frame_len = pDriverBuf->payloadLen;
				/* Copy buffer to user-space */
				for (int index = 0; index < recv_frame_len; index++) {
					tap_buffer[index] = pDriverBuf->payload[index];
				}
				/* Write to Linux Network Stack via TAP device */
				nbytes = write(tap_fd, tap_buffer, recv_frame_len);
				assert(nbytes==recv_frame_len && "Unable to write to network stack");
				/* Increment count of received frames */						
				rx_frame_count++;
				/* Release frame memory from shared buffer pool */
				retVal = Bufpool_freeBuf(pDriverBuf);
				/* If retVal!=BUFPOOL_OK : Unable to free memory */
				assert(retVal == BUFPOOL_OK);
			} else {
				break;
			}
		}
		/* Sleep */
		usleep(q_polling_interval);
	}
	/* Clean up */
	free(args);
}

void main(int argc, char *argv[])
{
	/* Shell script tapif.sh calls the user-space application with */
	/* 14 arguments = 13 (parameters) + 1 (name of executable) */ 
	assert(argc == 14 && "Error: Insufficient number of command line arguments\n");
	char tap_device_name[40];
	strcpy(tap_device_name,argv[1]); /* TAP_DEVICE_NAME */
	uint32_t rx_q_id = atoi(argv[2]); /* ICQ_MCU2_0_TO_A72_ID */
	uint32_t tx_q_id = atoi(argv[3]); /* ICQ_A72_TO_MCU2_0_ID */
	uint32_t max_num_queues = atoi(argv[4]); /* ICQ_MAX_NUM_QUEUES */
	uint32_t q_base_addr = strtoul(argv[5],NULL,16); /* ICQ_BASE_ADDR */
	uint32_t q_len = strtoul(argv[6],NULL,16); /* ICQ_MEM_LEN */
	if (q_len == 0) {
		/* Not provided by user */
		printf("Queue Region Length has not been provided by user\n");
		fflush(stdout);
		q_len = (uint32_t)(max_num_queues*sizeof(IcQ));
		printf("Computing and using default value\n");
		fflush(stdout);
		printf("Computed Queue Region Length: 0x%x\n",q_len);
		fflush(stdout);
	}
	uint32_t q_polling_interval = atoi(argv[7]); /* Q_POLLING_INTERVAL_MICROSECONDS */
	uint32_t max_num_bufpools = atoi(argv[8]);/* BUFPOOL_MAX_NUM_POOLS */
	uint32_t bufpool_base_addr = strtoul(argv[9],NULL,16); /* BUFPOOL_BASE_ADDR */
	uint32_t bufpool_len = strtoul(argv[10],NULL,16); /* BUFPOOL_MEM_LEN */
	if (bufpool_len == 0) {
		/* Not provided by user */
		printf("Bufpool Region Length has not been provided by user\n");
		fflush(stdout);
		bufpool_len = (uint32_t)(max_num_bufpools*sizeof(Bufpool_Pool));
		printf("Computing and using default value\n");
		fflush(stdout);
		printf("Computed Bufpool Region Length: 0x%x\n",bufpool_len);
		fflush(stdout);
	}
	uint32_t tx_bufpool_id = atoi(argv[11]); /* BUFPOOL_A72_ID */
	uint32_t max_tx = atoi(argv[12]); /* MAX_NUM_PACKET_TRANSMITS_PER_CYCLE */
	uint32_t max_rx = atoi(argv[13]); /* MAX_NUM_PACKET_RECEIVES_PER_CYCLE */
	int bufpool_init_status, tap_fd, mem_fd;

	/* Create threads for rx_task and tx_task */
	pthread_t thread_tx, thread_rx;
	Bufpool_Handle txBufpoolHandle; /* Handle for TX Buffer Pool */
	IcQ_Handle txQ_Handle; /* Handle for TX Queue */
	IcQ_Handle rxQ_Handle; /* Handle for RX Queue */
	/*Create structs for passing args to tx and rx task threads */
	struct arg_struct *tx_args_struct =
								(struct arg_struct *)malloc(sizeof(struct arg_struct));
	struct arg_struct *rx_args_struct =
								(struct arg_struct *)malloc(sizeof(struct arg_struct));

	/* Open TAP device and get TAP device descriptor */
	tap_fd = tap_open(tap_device_name);
	if (tap_fd < 0) {
		perror("Allocating interface");
		assert(tap_fd >= 0);
	}
	printf("Opened TAP Device successfully\n");
	fflush(stdout);

	/* Try to open the memory and fetch its file descriptor */
	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		printf("Failed to open /dev/mem\n");
		fflush(stdout);
		assert(0 && "Failed to access shared memory");
	}

	/*Create a mapping between the physical addresses and virtual addresses */
	/* for the Queue Region using mmap*/
	IcQ_globalQTable_Handle =
				(IcQ_Handle)mmap(NULL, q_len, PROT_READ | PROT_WRITE,
				MAP_SHARED, mem_fd, q_base_addr);
	/* Check for failure in mapping */
	assert(IcQ_globalQTable_Handle != MAP_FAILED && "Queue Mapping Failed");
	printf("Queue Mapping Succeeded\n");
	fflush(stdout);

	/*Create a mapping between the physical addresses and virtual addresses */
	/* for the Buffer Region using mmap*/
	BufpoolTable_Handle = (Bufpool_Handle)mmap(NULL, bufpool_len,
								PROT_READ | PROT_WRITE, MAP_SHARED,
								mem_fd, bufpool_base_addr);
	/* Check for failure in mapping */
	assert(BufpoolTable_Handle != MAP_FAILED && "Bufpool Mapping Failed");
	printf("Bufpool Mapping Succeeded\n");
	fflush(stdout);

	/* Define txQ_Handle and rxQ_Handle */
	txQ_Handle =
		(IcQ_Handle)&(IcQ_globalQTable_Handle[tx_q_id]);
	rxQ_Handle =
		(IcQ_Handle)&(IcQ_globalQTable_Handle[rx_q_id]);
	assert(txQ_Handle != NULL && "txQ_Handle is Null");
	assert(rxQ_Handle != NULL && "rxQ_Handle is Null");
	
	printf("Assigned Queue Handles\n");
	fflush(stdout);	

	/* Wait for the queues to be initialized by the master core */
	/* Sleep while either txQ or rxQ is not ready */
	/* IcQueue_isQValid is located in ic_queue.c */
	while (!IcQueue_isQValid(txQ_Handle) ||
		!IcQueue_isQValid(rxQ_Handle)) {
		printf("Waiting for queue initialization\n");
		fflush(stdout);
		usleep(SLEEP_RXQ_TXQ);
	}
	printf("Queues have been initialized\n");
	fflush(stdout);
	
	/* Define txBufpool Handle */
	txBufpoolHandle =
		(Bufpool_Handle)&(BufpoolTable_Handle[tx_bufpool_id]);
	printf("Assigned Bufpool Handle\n");
	fflush(stdout);

	/* Initialize the bufpool */
	bufpool_init_status =
	Bufpool_init(txBufpoolHandle, tx_bufpool_id, NUM_BUFFERS);
	/* If bufpool_init_status!=BUFPOOL_OK : Failed to initialize bufpool */
	assert(bufpool_init_status == BUFPOOL_OK);
	printf("Initialized Bufpool\n");
	fflush(stdout);
	
	/* Store argument values in args_tx and args_rx structs */
	tx_args_struct->arg_Q_Handle = txQ_Handle;
	tx_args_struct->arg_txBufpoolHandle = txBufpoolHandle;
	tx_args_struct->arg_IcQ_globalQTable_Handle = IcQ_globalQTable_Handle;
	tx_args_struct->arg_BufpoolTable_Handle = BufpoolTable_Handle;
	tx_args_struct->arg_tap_fd = tap_fd;
	tx_args_struct->arg_q_polling_interval = q_polling_interval;
	tx_args_struct->arg_bufpool_base_addr = bufpool_base_addr;
	tx_args_struct->arg_max_frames = max_tx;
	rx_args_struct->arg_Q_Handle = rxQ_Handle;
	rx_args_struct->arg_txBufpoolHandle = NULL; /* Do not use this arg in rx */
	rx_args_struct->arg_IcQ_globalQTable_Handle = IcQ_globalQTable_Handle;
	rx_args_struct->arg_BufpoolTable_Handle = BufpoolTable_Handle;
	rx_args_struct->arg_tap_fd = tap_fd;
	rx_args_struct->arg_q_polling_interval = q_polling_interval;
	rx_args_struct->arg_bufpool_base_addr = bufpool_base_addr;
	rx_args_struct->arg_max_frames = max_rx;
	
	printf("Initialization complete\n");
	fflush(stdout);
	printf("-------------------------------------------------------------------------\n");
	fflush(stdout);
	printf("Starting TX and RX Tasks\n");
	fflush(stdout);
	/* Start receiver and transmitter tasks */
	pthread_create(&thread_tx, NULL, (void *) tx_task, (void *) tx_args_struct);
	pthread_create(&thread_rx, NULL, (void *) rx_task, (void *) rx_args_struct);
	pthread_join(thread_tx, NULL);
	pthread_join(thread_rx, NULL);
}
