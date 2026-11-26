/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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
 *  \file  mailbox.c
 *
 *  \brief Mailbox Device Abstraction Layer APIs
 *
 *   This file contains the device abstraction layer APIs for the
 *   mailbox module. These are used for IPC communication.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/drv/ipc/src/mailbox/csl_mailbox.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint32_t Mailbox_getMessage(uintptr_t baseAddr,
                            uint32_t queueId,
                            uint32_t *msgPtr)
{
    uint32_t msgCount;
    uint32_t retVal;

    msgCount = Mailbox_getMessageCount(baseAddr, queueId);

    if (msgCount > 0U)
    {
        /* Read message */
        *msgPtr = CSL_REG32_RD(baseAddr + CSL_MAILBOX_MESSAGE(queueId));
        retVal  = MAILBOX_MESSAGE_VALID;
    }
    else
    {
        /* Queue empty */
        retVal = MAILBOX_MESSAGE_INVALID;
    }

    return retVal;
}

uint32_t Mailbox_sendMessage(uintptr_t baseAddr,
                             uint32_t queueId,
                             uint32_t msg)
{
    uint32_t fifoFull;
    uint32_t retVal;

    /* Read the FIFO Status */
    fifoFull = CSL_REG32_RD(baseAddr + CSL_MAILBOX_FIFO_STATUS(queueId));
    if (fifoFull == 0U)
    {
        /* FIFO not full write msg */
        Mailbox_writeMessage(baseAddr, queueId, msg);
        retVal = MAILBOX_MESSAGE_VALID;
    }
    else
    {
        retVal = MAILBOX_MESSAGE_INVALID;
    }

    return retVal;
}

void Mailbox_enableNewMsgInt(uintptr_t baseAddr,
                             uint32_t userId,
                             uint32_t queueId)
{
    uint32_t bitMask = (uint32_t)0x1 << (queueId * 2U);
    /* Set the NewMsgEnable field - Writing zero has no effect */
    CSL_REG32_WR(baseAddr + CSL_MAILBOX_USER_IRQ_ENABLE_SET(userId), bitMask);
}

void Mailbox_disableNewMsgInt(uintptr_t baseAddr,
                              uint32_t userId,
                              uint32_t queueId)
{
    uint32_t bitMask = (uint32_t)0x1 << (queueId * 2U);
    /* Set the NewMsgEnable field - Writing zero has no effect */
    CSL_REG32_WR(baseAddr + CSL_MAILBOX_USER_IRQ_ENABLE_CLR(userId), bitMask);
}

void Mailbox_clrNewMsgStatus(uintptr_t baseAddr,
                             uint32_t userId,
                             uint32_t queueId)
{
    uint32_t bitMask = (uint32_t)0x1 << (queueId * 2U);
    /* Set the NewMsgEnable field - Writing zero has no effect */
    CSL_REG32_WR(baseAddr + CSL_MAILBOX_USER_IRQ_STATUS_CLR(userId), bitMask);
}

uint32_t Mailbox_getRawNewMsgStatus(uintptr_t baseAddr,
                                    uint32_t userId,
                                    uint32_t queueId)
{
    uint32_t regVal;
    uint32_t bitShift = queueId * 2U;

    /* Read the IRQSTATUSRAW */
    regVal = CSL_REG32_RD(baseAddr + CSL_MAILBOX_USER_IRQ_STATUS_RAW(userId));

    /* Mask & read the NewMsgStatus for given queueId */
    regVal &= ((uint32_t)0x1 << bitShift);

    return (regVal >> bitShift);
}

uint32_t Mailbox_getMessageCount(uintptr_t baseAddr,
                                 uint32_t queueId)
{
    /* Return message count */
    return (CSL_REG32_RD(baseAddr + CSL_MAILBOX_MSG_STATUS(queueId)));
}

void Mailbox_writeMessage(uintptr_t baseAddr,
                          uint32_t queueId,
                          uint32_t msg)
{
    /* Write mailbox message */
    CSL_REG32_WR(baseAddr + CSL_MAILBOX_MESSAGE(queueId), msg);
}
