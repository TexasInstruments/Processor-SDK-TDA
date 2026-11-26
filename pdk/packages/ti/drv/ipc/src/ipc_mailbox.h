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
 *  \file ipc_mailbox.h
 *
 *  \brief Declaration for Mailbox wrapper implementation.
 */

#ifndef IPC_MAILBOX_H_
#define IPC_MAILBOX_H_

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

#define  MAILBOX_CLUSTER_INVALID   (0xFFU)
#define  MAILBOX_USER_INVALID      (0xFFU)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Mailbox entry for all transmit and receive
 */
typedef struct Ipc_MailboxEntry_s
{
    uint32_t    cluster;
    uint32_t    user;
    uint32_t    fifo;
} Ipc_MailboxEntry;

typedef struct Ipc_MailboxInfo_s
{
    Ipc_MailboxEntry    rx;
    Ipc_MailboxEntry    tx;
}Ipc_MailboxInfo;

typedef void (*Mailbox_hwiCallback)(uint32_t* arg1, uint32_t arg2);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

extern uint32_t g_ipc_mBoxCnt;

/** \brief Initializes mailbox module */
int32_t Ipc_mailboxModuleStartup (void);

int32_t Ipc_mailboxSend(uint32_t selfId, uint32_t remoteProcId, uint32_t val,
                        uint32_t timeoutCnt);
int32_t Ipc_mailboxRegister(uint16_t selfId, uint16_t remoteProcId,
            Mailbox_hwiCallback func, uint32_t arg, uint32_t timeoutCnt);

/** \brief Mailbox Interrupts  */
void *Mailbox_plugInterrupt(Ipc_MbConfig *cfg, Ipc_OsalIsrFxn func, uintptr_t arg);

/**
*  \brief Clear interrupt and return the message.
*/
uint32_t Ipc_mailboxClear(uintptr_t baseAddr, uint32_t queueId);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IPC_MAILBOX_H_ */

