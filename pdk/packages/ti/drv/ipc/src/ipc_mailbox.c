/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2024
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
 *  \file ipc_mailbox.c
 *
 *  \brief Implementation of ipc Mailbox interaction
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/ipc/ipc.h>
#include <ti/drv/ipc/soc/ipc_soc.h>
#include <ti/drv/ipc/src/ipc_mailbox.h>
#include <ti/drv/ipc/src/mailbox/csl_mailbox.h>

#ifndef IPC_EXCLUDE_POLLED_RX
#include <ti/osal/TaskP.h>
#endif

#include <string.h>

#include "ipc_osal.h"
#include "ipc_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Fix Me, need not be 256, can be max remote proc / max proc, i.e. 1 as
    structure which uses this is instantiated max proc times. */
#define  IPC_MBOX_MAXFIFO_CNT    16U
/* Fix Me, need not be 256, can be max remote proc / max proc */
#define  IPC_MBOX_MAXDATA        8U

#define IPC_RP_MBOX_SPL_MSG_COUNT    (2U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct Ipc_MailboxFifo_s
{
    int32_t                 cfgNdx;
    Mailbox_hwiCallback     func;
    uint32_t                arg;
    uint32_t                queueId;
} Ipc_MailboxFifo;

/* mboxData */
typedef struct Ipc_MailboxData_s
{
    uintptr_t         baseAddr;
    uint32_t          fifoCnt;
    Ipc_MailboxFifo   fifoTable[IPC_MBOX_MAXFIFO_CNT];
    uint32_t          noMsgCnt;
    uint32_t          intCnt;
    uint32_t 	      userId;

} Ipc_MailboxData;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Mailbox Internal Callback
 */
static void Ipc_mailboxInternalCallback(uintptr_t arg);

/**
 *  \brief Enable remote processor interrupt
 */
static void Ipc_mailboxEnable(uintptr_t baseAddr, uint32_t userId, uint32_t queueId);

/**
 *  \brief Disables remote processor interrupt
 */
static void Ipc_mailboxDisable(uintptr_t baseAddr, uint32_t userId, uint32_t queueId);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint32_t               g_ipc_mBoxCnt = 0U;
static Ipc_MailboxData g_ipc_mBoxData[IPC_MBOX_MAXDATA];
/**
 * \brief Maps mBoxData allocated to a given Remote Processor
 */
uintptr_t       gIpcRProcIdToMBoxDataMap[IPC_MAX_PROCS];

extern Ipc_Object gIpcObject;

/* ========================================================================== */
/*                             Local Functions                                */
/* ========================================================================== */

#ifndef IPC_EXCLUDE_POLLED_RX
#define IPC_POLL_TIMER  100

uint32_t     g_pollTaskExit = FALSE;
TaskP_Handle g_pollTask     = NULL;

void Mailbox_Poll_Task(void* arg0, void* arg1)
{
    uint32_t                n    = 0U;
    uint32_t                cnt  = 0U;
    uint32_t                msg[4];
    Ipc_MailboxData        *mbox = NULL;
    Ipc_MailboxFifo        *fifo = NULL;

    while(FALSE == g_pollTaskExit)
    {
        for(n = 0; n < g_ipc_mBoxCnt; n++)
        {
             mbox = &g_ipc_mBoxData[n];
             for(cnt = 0; cnt < mbox->fifoCnt; cnt++)
             {
                 fifo = &mbox->fifoTable[cnt];
                 if(Mailbox_getMessageCount(mbox->baseAddr, fifo->queueId) > 0)
                 {
                    /* Get the message from Mailbox fifo */
                    Mailbox_getMessage(mbox->baseAddr, fifo->queueId, msg);

                    /* Clear new message status of Mailbox */
                    Mailbox_clrNewMsgStatus(mbox->baseAddr, mbox->userId, fifo->queueId);

#ifdef  DEBUG_PRINT
                    SystemP_printf("Mailbox polling : BaseAddr 0x%x queue %d value %d arg(procId) %d\n",
                        mbox->baseAddr,  fifo->queueId, msg[0], fifo->arg);
#endif
                    /* Call the function with arg */
                    (mbox->fifoTable[cnt].func)(msg, fifo->arg);
                 }
             }
        }
        /* Temporarily we use Task_yield() */
#ifdef QNX_OS
        TaskP_sleep(IPC_POLL_TIMER);
#else
        TaskP_yield();
#endif
    }
}

void Mailbox_Poll_TaskExit()
{
    g_pollTaskExit = TRUE;
}

#if defined(BUILD_C7X) && !defined(MAILBOX_INTERRUPT_MODE)
static uint8_t g_pollTaskStack[IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;
#endif

void StartmailboxPollingTask()
{
    TaskP_Params param;

    TaskP_Params_init(&param);
    param.priority = 2;
#if defined(BUILD_C7X) && !defined(MAILBOX_INTERRUPT_MODE)
    param.stack = g_pollTaskStack;
#endif
    param.stacksize = IPC_TASK_STACKSIZE;
    g_pollTask = TaskP_create(&(void *) Mailbox_Poll_Task, &param);
}

#endif /* IPC_EXCLUDE_POLLED_RX */

/* ========================================================================== */
/*                             Functions                                      */
/* ========================================================================== */

int32_t Ipc_mailboxModuleStartup (void)
{
    int32_t  retVal = IPC_SOK;

    memset(gIpcRProcIdToMBoxDataMap, 0, sizeof(gIpcRProcIdToMBoxDataMap));

    return retVal;
}

/**
 *  \brief Enable remote processor interrupt
 */
static void Ipc_mailboxEnable(uintptr_t baseAddr, uint32_t userId, uint32_t queueId)
{
    Mailbox_enableNewMsgInt(baseAddr, userId, queueId);
}

/**
 *  \brief Disables remote processor interrupt
 */
static void Ipc_mailboxDisable(uintptr_t baseAddr, uint32_t userId, uint32_t queueId)
{
    Mailbox_disableNewMsgInt(baseAddr, userId, queueId);
}

/**
 *  \brief Clear interrupt and return the message.
 */
uint32_t Ipc_mailboxClear(uintptr_t baseAddr, uint32_t queueId)
{
    uint32_t retVal = 0;
    uint32_t msg[4];

    retVal = Mailbox_getMessage(baseAddr, queueId, msg);

    return retVal;
}

/**
 *  \brief Send interrupt to the remote processor
 */
int32_t Ipc_mailboxSend(uint32_t selfId, uint32_t remoteProcId, uint32_t val,
                        uint32_t timeoutCnt)
{
    int32_t   status = 0;
    uint32_t  clusterId;
    uint32_t  userId;
    uint32_t  queueId;
    uintptr_t baseAddr;
    uint32_t  cnt = timeoutCnt;

#ifndef IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE
    Ipc_Object      *pObj  = NULL;
    Ipc_OsalPrms    *pOsal = NULL;
    uintptr_t        key   = 0U;

    pObj = getIpcObjInst(0U);
    pOsal = &pObj->initPrms.osalPrms;
#endif /* IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE */

    Ipc_getMailboxInfoTx(selfId, remoteProcId,
        &clusterId, &userId, &queueId);

    if( (clusterId != MAILBOX_CLUSTER_INVALID) && (queueId < 16U) )
    {
        uint32_t    retVal;

        baseAddr = Ipc_getMailboxBaseAddr(clusterId);

        /* In case of baremetal, the HISR protection and Interrupt disable would
            same. This function is invoked by Virtio_kick() to let other cores
            know that there is new message available.
            Virtio_kick is called under HISR protection
            which means that we should not invoke protection again. As recursive
            interrupt disable is not recommended */
#ifndef IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE
        if (NULL != pOsal->disableAllIntr)
        {
            key = pOsal->disableAllIntr();
        }
#endif /* IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE */

        do
        {
            retVal = Mailbox_sendMessage(baseAddr, queueId, val);
            cnt--;
        } while( (cnt != 0U) && (retVal == MAILBOX_MESSAGE_INVALID));

        if(MAILBOX_MESSAGE_INVALID == retVal)
        {
#ifdef DEBUG_PRINT
            SystemP_printf("Ipc_mailboxSend : BaseAddr 0x%x queue %d value %d failed\n",
                baseAddr,  queueId, val);
#endif
            status = IPC_EFAIL;
        }

#if defined DEBUG_PRINT
        SystemP_printf("Ipc_mailboxSend : BaseAddr 0x%x queue %d value %d\n",
            baseAddr,  queueId, val);
#endif

#ifndef IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE
        if (NULL != pOsal->restoreAllIntr)
        {
            pOsal->restoreAllIntr(key);
        }
#endif /* IPC_ONE_CONTEXT_FOR_HISRGATE_HWIGATE */

    }

    return status;
}

int32_t Ipc_mailboxRegister(uint16_t selfId, uint16_t remoteProcId,
           Mailbox_hwiCallback func, uint32_t arg, uint32_t timeoutCnt)
{
    int32_t               retVal = IPC_SOK;
    uint32_t              clusterId;
    uint32_t              userId;
    uint32_t              queueId;
    uintptr_t             baseAddr;
    uint32_t              n;
    Ipc_MailboxData      *mbox = NULL;
    Ipc_Object           *pObj  = NULL;
    Ipc_OsalPrms         *pOsal = NULL;

    pObj = getIpcObjInst(0U);
    pOsal = &pObj->initPrms.osalPrms;

    Ipc_getMailboxInfoRx(selfId, remoteProcId,
        &clusterId, &userId, &queueId);

    if( (clusterId != MAILBOX_CLUSTER_INVALID) && (IPC_MAX_PROCS > remoteProcId))
    {
        baseAddr = Ipc_getMailboxBaseAddr(clusterId);
    }
    else
    {
#ifdef DEBUG_PRINT      
        SystemP_printf("Ipc_Mailbox_register : failed Invalid cluster..\n");
#endif        
        retVal = IPC_EFAIL;
    }

    if(retVal == IPC_SOK)
    {
        for(n = 0; n < g_ipc_mBoxCnt; n++)
        {
            if((baseAddr == g_ipc_mBoxData[n].baseAddr) && (userId == g_ipc_mBoxData[n].userId))
            {
              break;
            }
        }

        /* Get the MailBox Data */
        mbox = &g_ipc_mBoxData[n];

        if(n == g_ipc_mBoxCnt)
        {
            /* Could not find one, this is new entry */
            mbox->baseAddr = baseAddr;
            mbox->fifoCnt  = 0;
            mbox->userId   = userId;

            if (NULL != pOsal->registerIntr)
            {
                /* Do not clear the mailbox, other cores could have already sent messages */
                Ipc_MbConfig cfg;

                Mailbox_clrNewMsgStatus(baseAddr, userId, queueId);

                /* Get the Interrupt Configuration */
                Ipc_getMailboxIntrRouterCfg(selfId, clusterId, userId, &cfg, g_ipc_mBoxCnt);

                {
                    /* Release the resource first */
                    Ipc_sciclientIrqRelease(selfId, clusterId, userId, cfg.eventId);

                    uint32_t timeout_cnt = 10;
                    do
                    {
                        retVal = Ipc_sciclientIrqSet(selfId, clusterId, userId, cfg.eventId);
                        if(retVal != 0)
                        {
                            SystemP_printf("Failed to register irq through sciclient...%x\n", retVal);
                        }
                        timeout_cnt--;
                    }while((retVal != 0) && (timeout_cnt > 0U));

                    if(timeout_cnt == 0U)
                    {
                        retVal = IPC_EFAIL;
                    }
                }

                /* Add the fifo data for the remoteProcId. */
                mbox->fifoTable[mbox->fifoCnt].cfgNdx  = (int32_t)n;
                mbox->fifoTable[mbox->fifoCnt].func    = func;
                mbox->fifoTable[mbox->fifoCnt].arg     = arg;
                mbox->fifoTable[mbox->fifoCnt].queueId = queueId;
                gIpcRProcIdToMBoxDataMap[remoteProcId] = (uintptr_t)mbox;

                mbox->fifoCnt++;

                /* Register Mailbox interrupt now... */
                if (retVal == IPC_SOK)
                {
                    /* disable the mailbox interrupt (from previous runs) */
                    Ipc_mailboxDisable(baseAddr, userId, queueId);
                    pObj->interruptHandle = pOsal->registerIntr(
                            &cfg,Ipc_mailboxInternalCallback,
                            (uintptr_t)mbox);
                }
	        }

            g_ipc_mBoxCnt++;
        }
        else
        {
            /* Add the fifo data for the remoteProcId. */
            mbox->fifoTable[mbox->fifoCnt].cfgNdx  = (int32_t)n;
            mbox->fifoTable[mbox->fifoCnt].func    = func;
            mbox->fifoTable[mbox->fifoCnt].arg     = arg;
            mbox->fifoTable[mbox->fifoCnt].queueId = queueId;
            gIpcRProcIdToMBoxDataMap[remoteProcId] = (uintptr_t)mbox;

            mbox->fifoCnt++;
        }

#ifdef DEBUG_PRINT
        SystemP_printf("Ipc_MB(%d): Self %d Remote %d (c%d,u%d,q%d) arg %d,total %d\n",
                mbox->fifoCnt, selfId, remoteProcId, clusterId, userId, queueId, arg,
                g_ipc_mBoxCnt);
#endif
    }

    return retVal;
}

/*!
 *  ======== Ipc_mailboxInternalCallback ========
 */
static void Ipc_mailboxInternalCallback(uintptr_t arg)
{
    uint32_t          n, i, j;
    Ipc_MailboxData  *mbox;
    volatile uint32_t msg[4] = {0, 0, 0, 0};
    volatile uint32_t parsedMsg[4] = {0, 0, 0, 0};
    volatile uint32_t rpMboxMsgRecv = 0, rpMboxMsg = 0;
    volatile uint32_t numMessages;
    Ipc_MailboxFifo  *fifo;
    uint32_t specialMessages[IPC_RP_MBOX_SPL_MSG_COUNT] = {
        IPC_RP_MBOX_SHUTDOWN,
        IPC_RP_MBOX_SUSPEND_SYSTEM
    };
    bool specialMessageFound;

    mbox = (Ipc_MailboxData *)arg;
    if(mbox != NULL)
    {
        mbox->intCnt++;

        for(n = 0; n < mbox->fifoCnt; n++)
        {
            fifo = &mbox->fifoTable[n];

            if(0U != Mailbox_getRawNewMsgStatus(mbox->baseAddr, mbox->userId, fifo->queueId))
            {
                numMessages = Mailbox_getMessageCount(mbox->baseAddr, fifo->queueId);
                if(numMessages > 0U)
                {
                    /* Get the message from Mailbox fifo */
                    Mailbox_getMessage(mbox->baseAddr, fifo->queueId, (uint32_t *)msg);

                    /* Clear new message status of Mailbox */
                    Mailbox_clrNewMsgStatus(mbox->baseAddr, mbox->userId,
                                               fifo->queueId);

                    /* Process till we get the special RP Mbox message */
                    for(i=0; i<numMessages; i++)
                    {
                        specialMessageFound = false;
                        for(j=0; j<IPC_RP_MBOX_SPL_MSG_COUNT; j++)
                        {
                            if(msg[i] == specialMessages[j])
                            {
                                specialMessageFound = true;
                                break;
                            }
                        }

                        if(!specialMessageFound)
                        {
                            parsedMsg[i] = msg[i];
                        }
                        else
                        {
                            rpMboxMsgRecv = 1;
                            rpMboxMsg = msg[i];
                            break;
                        }
                    }

                    if((0U == rpMboxMsgRecv))
                    {
                        /* Call the function with arg */
                        (mbox->fifoTable[n].func)((uint32_t *)parsedMsg, fifo->arg);
                    }
                    else /* (1U == rpMboxMsgRecv) */
                    {
                        if(numMessages > 1U)
                        {
                            (mbox->fifoTable[n].func)((uint32_t *)parsedMsg, fifo->arg);
                        }
                        
                        if(NULL != gIpcObject.initPrms.rpMboxMsgFxn)
                        {
                            gIpcObject.initPrms.rpMboxMsgFxn(fifo->arg, rpMboxMsg);
                        }
                    }
                }
                else
                {
                    /* Clear new message status of Mailbox */
                    Mailbox_clrNewMsgStatus(mbox->baseAddr, mbox->userId, fifo->queueId);
                }
            }
        }
        if(n == mbox->fifoCnt)
        {
            mbox->noMsgCnt++;
        }
    }
}

void Ipc_mailboxEnableNewMsgInt(uint16_t selfId, uint16_t remoteProcId)
{
    uint32_t  clusterId;
    uint32_t  userId;
    uint32_t  queueId;
    uintptr_t baseAddr;

    Ipc_getMailboxInfoRx(selfId, remoteProcId, &clusterId, &userId,
                         &queueId);

    baseAddr = Ipc_getMailboxBaseAddr(clusterId);

    Ipc_mailboxEnable(baseAddr, userId, queueId);
}

void Ipc_mailboxDisableNewMsgInt(uint16_t selfId, uint16_t remoteProcId)
{
    uint32_t  clusterId;
    uint32_t  userId;
    uint32_t  queueId;
    uintptr_t baseAddr;

    if ((IPC_MAX_PROCS > selfId) && (IPC_MAX_PROCS > remoteProcId))
    {
        Ipc_getMailboxInfoRx(selfId, remoteProcId, &clusterId, &userId,
                                &queueId);

        baseAddr = Ipc_getMailboxBaseAddr(clusterId);

        Ipc_mailboxDisable(baseAddr, userId, queueId);
    }
}

void *Mailbox_plugInterrupt(Ipc_MbConfig *cfg, Ipc_OsalIsrFxn func, uintptr_t arg)
{
    OsalRegisterIntrParams_t    intrPrms;
    OsalInterruptRetCode_e      osalRetVal;
    HwiP_Handle                 hwiHandle = NULL;
    uint32_t                    coreIntrNum = 0U;

#ifdef DEBUG_PRINT
    SystemP_printf("Navss Rtr: input %d, output %d%d\n",
        cfg->inputIntrNum, cfg->outputIntrNum);
#endif
#ifdef QNX_OS
#ifdef DEBUG_PRINT
    SystemP_printf("Mailbox_plugInterrupt: Navss Rtr input %d, output %d\n",
        cfg->inputIntrNum, cfg->outputIntrNum);
#endif
#endif

    /*
     * CLEC needs to be configured for all modes - CSL and Sciclient
     **/
#if defined(BUILD_C7X)
    /* Pass the corePackEvent and the base (which was derived from the NAVSS IR o/p
     * range returned from BoardCfg) to route the corressponding CLEC i/p Event to
     * a C7x IRQ. The returned IRQ num is used to register Interrupt with OSAL. */
    coreIntrNum = Ipc_configClecRouter(cfg->eventId, cfg->eventIdBase);
#else
    coreIntrNum = cfg->eventId;
#endif

#ifdef QNX_OS
#if 1 //def DEBUG_PRINT
    SystemP_printf("Mailbox_plugInterrupt: interrupt Number %d, arg 0x%X\n",
        cfg->eventId, (uint32_t)arg);
#endif
#endif

    /* Register interrupts */
    Osal_RegisterInterrupt_initParams(&intrPrms);
    intrPrms.corepacConfig.arg              = arg;
    intrPrms.corepacConfig.isrRoutine       = func;
    intrPrms.corepacConfig.priority         = cfg->priority;

#if defined(BUILD_C66X)
    intrPrms.corepacConfig.corepacEventNum  = coreIntrNum;
    intrPrms.corepacConfig.intVecNum        = OSAL_REGINT_INTVEC_EVENT_COMBINER;
#else
    intrPrms.corepacConfig.intVecNum        = coreIntrNum;
    intrPrms.corepacConfig.corepacEventNum  = 0;
#endif
#ifdef QNX_OS
    intrPrms.corepacConfig.intAutoEnable  = 1;
#endif
    osalRetVal = Osal_RegisterInterrupt(&intrPrms, &hwiHandle);
    if(OSAL_INT_SUCCESS != osalRetVal)
    {
        SystemP_printf("Mailbox_plugInterrupt : Failed to register ISR...\n");
    }

    return (hwiHandle);
}
