/*
 * Copyright (c) 2026, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  \file ipc_invalid_endpt_test.h
 *
 *  \brief Define the macros and functions for common IPC test
 *
 */

#ifndef IPC_TEST_H_
#define IPC_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drv/ipc/ipc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Valid only for J784S4 / J742S2 */
#define TEST_VRING_BASE_ADDRESS      0xAC000000U

#define IPC_RPMESSAGE_MSG_BUFFER_SIZE  (496U + 32U)
#define IPC_RPMESSAGE_OBJ_SIZE  256U

#define TEST_NUM_BUF_RPMSG_CREATE       (2U)
#define TEST_RPMSG_DATA_SIZE            ((TEST_NUM_BUF_RPMSG_CREATE)*256U*IPC_RPMESSAGE_MSG_BUFFER_SIZE + IPC_RPMESSAGE_OBJ_SIZE)
#define TEST_VQ_BUF_SIZE                 2048U
#define TEST_CTRL_TASK_PRIORITY          10U
#define TEST_CTRL_TASK_STACKSIZE         0x2000U
#define TEST_INVALID_DEST_ENDPT          464U
#define TEST_NUM_REMOTE_CORES            1U

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t ipc_invalid_endpt_test(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t gSelfCoreId;
extern uint32_t gRemoteCores[];

#ifdef __cplusplus
}
#endif

#endif /* IPC_TEST_H_ */
