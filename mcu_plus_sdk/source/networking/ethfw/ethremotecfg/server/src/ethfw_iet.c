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

/*!
 * \file ethfw_iet.c
 *
 * \brief This file contains the implementation of port mirroring helper functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x10A

#include <stdint.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <ethremotecfg/server/include/ethfw_iet.h>
#include <utils/ethfw_common/include/ethfw_trace.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*!
 * \brief IET params passed from application.
 */

typedef struct EthFwIET_Params_s
{

    /*! Minimum Fragment size */
    uint32_t minFragSize;

    /*! iet verification enabled/disabled */
    bool mac_verify_enable;

    /*! Traffic mode for each of the FIFO queues */
    uint32_t queueMode[CPSW_MACPORT_FIFO];

    /*! Ethernet handle*/
    Enet_Handle hEnet;

    /*! Caller core id */
    uint32_t coreId;

} EthFwIET_Params;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*! Global object to store IET params */

EthFwIET_Params gEthFwIETObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static int32_t EthFwIET_handleLinkUp(Enet_Handle hEnet,
                              uint32_t coreId, 
                              Enet_MacPort macPort, 
                              uint32_t minFragSize, 
                              bool mac_verify_enable , 
                              uint32_t *queueMode);

static int32_t EthFwIET_handleLinkDown(Enet_Handle hEnet,
                                uint32_t coreId, 
                                Enet_MacPort macPort);

static EnetMacPort_PreemptVerifyStatus EthFwIET_doIetVerification(Enet_Handle hEnet,
                                                           uint32_t coreId,
                                                           Enet_MacPort macPort);



void  EthFwIET_notifyLinkChange(const Enet_MacPort macPort, const bool isLinkUp)
{
   int32_t status = ETHFW_SOK;
   if(isLinkUp == BTRUE)
   {
        status = EthFwIET_handleLinkUp(gEthFwIETObj.hEnet, gEthFwIETObj.coreId, macPort, gEthFwIETObj.minFragSize, gEthFwIETObj.mac_verify_enable , gEthFwIETObj.queueMode);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to handle IET Link Up");

   }
   else
   {
        status = EthFwIET_handleLinkDown(gEthFwIETObj.hEnet, gEthFwIETObj.coreId, macPort);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to handle IET Link Down");
        
   }
}


static int32_t EthFwIET_handleLinkUp(Enet_Handle hEnet,uint32_t coreId, Enet_MacPort macPort, uint32_t minFragSize, bool mac_verify_enable , uint32_t *queueMode)
{
    int32_t status = ETHFW_SOK;
    EnetMacPort_PreemptVerifyStatus verifyStatus;
    EnetMacPort_SetPreemptMinFragSizeInArgs fragSizeInArgs;
    EnetMacPort_SetPreemptQueueInArgs queuePreemptInArgs;
    Enet_IoctlPrms prms;
    EnetMacPort_GenericInArgs fpe;
    uint32_t i;

    /*
    If mac_verify_enable is false then without verification, we enable the iet assuming other side supports iet
    */
    if(mac_verify_enable == BTRUE)
    {
       verifyStatus =  EthFwIET_doIetVerification(hEnet,coreId, macPort);
    }
    else
    {
        /* Disable preemption verification */
        ENET_IOCTL_SET_IN_ARGS(&prms, &fpe);
        status = Enet_ioctl(hEnet, coreId, ENET_MACPORT_IOCTL_DISABLE_PREEMPT_VERIFICATION , &prms);
        if (status != ENET_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to disable macport IET verification");
        }
        verifyStatus = ENET_MAC_VERIFYSTATUS_DISABLED;
    }
    
    if((verifyStatus == ENET_MAC_VERIFYSTATUS_DISABLED) || ((verifyStatus == ENET_MAC_VERIFYSTATUS_SUCCEEDED)))
    {
        /* Enable preemption */
        fpe.macPort = macPort;
        ENET_IOCTL_SET_IN_ARGS(&prms, &fpe);
        status = Enet_ioctl(hEnet,coreId, ENET_MACPORT_IOCTL_ENABLE_PREEMPTION, &prms);

        fragSizeInArgs.macPort = macPort;
        fragSizeInArgs.preemptMinFragSize = minFragSize;
        ENET_IOCTL_SET_IN_ARGS(&prms, &fragSizeInArgs);
        status = Enet_ioctl(hEnet,coreId, ENET_MACPORT_IOCTL_SET_PREEMPT_MIN_FRAG_SIZE, &prms);
        queuePreemptInArgs.macPort = macPort;
        for(i = 0U; i < CPSW_MACPORT_FIFO; i++)
        {
            if (queueMode[i] == 0)
            {
                queuePreemptInArgs.queuePreemptCfg.preemptMode[i] = ENET_MAC_QUEUE_PREEMPT_MODE_EXPRESS;
            }
            else
            {
                queuePreemptInArgs.queuePreemptCfg.preemptMode[i] = ENET_MAC_QUEUE_PREEMPT_MODE_PREEMPT;
            }
        }
        ENET_IOCTL_SET_IN_ARGS(&prms, &queuePreemptInArgs);
        status = Enet_ioctl(hEnet,coreId, ENET_MACPORT_IOCTL_SET_PREEMPT_QUEUE, &prms);    
    }

    return status;
}

static int32_t EthFwIET_handleLinkDown(Enet_Handle hEnet,uint32_t coreId, Enet_MacPort macPort)
{
    EnetMacPort_GenericInArgs fpe;
    Enet_IoctlPrms prms; 
    int32_t status = ETHFW_SOK;
    fpe.macPort = macPort;
    ENET_IOCTL_SET_IN_ARGS(&prms, &fpe);
    status = Enet_ioctl(hEnet,coreId, ENET_MACPORT_IOCTL_DISABLE_PREEMPTION, &prms);
    return status;
}

/*
 * Function to Poll the Verify status and restart verification.
 * Application needs to call this after every link-up/link-down.
 * Verify timeout is set based on the link-speed from ENET handle.
 */
static EnetMacPort_PreemptVerifyStatus EthFwIET_doIetVerification(Enet_Handle hEnet,
                               uint32_t coreId,
                               Enet_MacPort macPort)
{
    uint32_t try = ETHFW_NUM_IET_VERIFY_ATTEMPTS;
    int32_t status = ENET_SOK;
    EnetMacPort_GenericInArgs fpe;
    Enet_IoctlPrms prms;
    EnetMacPort_PreemptVerifyStatus verifyStatus = ENET_MAC_VERIFYSTATUS_FAILED;

    fpe.macPort = macPort;
    do{
        ENET_IOCTL_SET_IN_ARGS(&prms, &fpe);
        status = Enet_ioctl(hEnet, coreId, ENET_MACPORT_IOCTL_ENABLE_PREEMPT_VERIFICATION , &prms);
        if (status != ENET_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to start IET verification\n");
            break;
        }
        /*
         * Since both side might
         * take variable setup/config time, need to Wait for
         * additional time. Chose 50 msec through trials
         */
        EthFwOsal_sleepTaskinMsecs(50U);
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &fpe, &verifyStatus);
        status = Enet_ioctl(hEnet, coreId, ENET_MACPORT_IOCTL_GET_PREEMPT_VERIFY_STATUS , &prms);
        if (status != ENET_SOK)
        {
            ETHFWTRACE_INFO("Failed to read IET verify status");
            break;
        }
        if(verifyStatus == ENET_MAC_VERIFYSTATUS_SUCCEEDED)
        {
            ETHFWTRACE_INFO("IET verify Success");
            break;
        }
        else if(verifyStatus == ENET_MAC_VERIFYSTATUS_FAILED )
        {
            ETHFWTRACE_INFO("IET verify failed, trying again");
        }
        else if(verifyStatus == ENET_MAC_VERIFYSTATUS_RXRESPOND_ERROR )
        {
            ETHFWTRACE_INFO("IET MAC respond error");
            break;
        }
        else if(verifyStatus == ENET_MAC_VERIFYSTATUS_RXVERIFY_ERROR )
        {
            ETHFWTRACE_INFO("IET MAC verify error");
            break;
        }
        try--;
    } while(try > 0);

    if(try == 0)
    {
        ETHFWTRACE_WARN("IET verify timeout");
    }
    return verifyStatus;
}


void EthFwIET_init(const EthFwIET_Config *ietCfg, Enet_Handle hEnet, uint32_t coreId){
    
    gEthFwIETObj.coreId = coreId;
    gEthFwIETObj.hEnet = hEnet;
    memcpy(gEthFwIETObj.queueMode, ietCfg->queueMode, CPSW_MACPORT_FIFO);
    gEthFwIETObj.minFragSize = ietCfg->minFragSize;
    gEthFwIETObj.mac_verify_enable = ietCfg->mac_verify_enable;
}
