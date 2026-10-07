/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file dru_periodic_readback.c
 *
 *  \brief DRU Readback Example powers up every DRU instance that is reachable
 *         from the core this application is built for, programs the DRU queue
 *         and channel static configuration registers, then performs periodic
 *         readback and comparison of those registers.
 *
 *         J722S implements four DRU instances - one in VPAC0, one in DMPAC0 and
 *         one local to each of the two C7x subsystems. The C7x local DRU MMRs
 *         are only reachable from the C7x that owns them, so the instance table
 *         below is selected by the build target.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <drivers/soc.h>
#include <drivers/hw_include/cslr.h>
#include <drivers/hw_include/dru/v2/csl_dru.h>

#include <ip_fma_dru.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Number of periodic readbacks per register group */
#define DRU_APP_PERIODIC_CHECK_NUM              ((uint32_t)10U)

/**< Interval between two periodic readbacks, in microseconds */
#define DRU_APP_PERIODIC_CHECK_DELAY_US         ((uint32_t)100000U)

/**< Reset bits to deassert every software controlled reset of a module.
 *   The TISCI convention is 1 to assert and 0 to deassert, which is the
 *   opposite of the PSC MDCTL register. */
#define DRU_APP_MODULE_RST_DEASSERT             ((uint32_t)0x0U)

/**< DRU queue static configuration programmed by this application */
#define DRU_APP_QUEUE_PRIORITY                  ((uint64_t)4U)
#define DRU_APP_QUEUE_QOS                       ((uint64_t)4U)
#define DRU_APP_QUEUE_ORDER_ID                  ((uint64_t)0U)
#define DRU_APP_QUEUE_CONSECUTIVE_TRANS         ((uint64_t)0xFU)
#define DRU_APP_QUEUE_REARB_WAIT                ((uint64_t)0xFU)

/**< DRU channel static configuration programmed by this application */
#define DRU_APP_CH_TYPE                         ((uint64_t)0U)
#define DRU_APP_CH_OWNER                        (CSL_DRU_OWNER_UDMAC_TR)
#define DRU_APP_CH_PAUSE_ON_ERR                 ((uint64_t)FALSE)
#define DRU_APP_CH_EVENT_NUM                    ((uint64_t)0U)
#define DRU_APP_CH_QUEUE_ID                     ((uint64_t)CSL_DRU_QUEUE_ID_3)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Description of one DRU instance covered by this application.
 */
typedef struct
{
    const char *name;
    /**< Name used in the application log */
    uint32_t    tisciDevId;
    /**< TISCI device the DRU MMRs belong to */
    uint32_t    isPowerManaged;
    /**< TRUE when the TISCI device has to be turned on by this application */
    uintptr_t   druBase;
    /**< Base address of the DRU MMR overlay */
    uint32_t    isConfigurable;
    /**< TRUE when writes to the static configuration registers take effect.
     *   FALSE for DMPAC0, whose MMRs read back correctly but silently discard
     *   writes, so it is covered by readback alone. */
} DruApp_DruInstance;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Turns on the TISCI device the DRU MMRs belong to.
 *
 * This function requests the device of the given DRU instance to the ON state
 * and reads the resulting module state back. The DRU owning devices span
 * several LPSC modules, for which the device manager does not support the
 * TISCI reset requests, so the power on sequence alone brings the DRU MMRs up.
 *
 * \param   pInst           DRU instance to power up.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_ModuleInit(const DruApp_DruInstance *pInst);

/**
 * \brief Reads one register of every DRU MMR region of an instance.
 *
 * This function reads the DRU capabilities register and the first register of
 * the queue and channel non-real time regions, so that an unreachable instance
 * is reported before its whole register group is walked.
 *
 * \param   pInst           DRU instance to probe.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_DruProbe(const DruApp_DruInstance *pInst);

/**
 * \brief Reads a single 64-bit DRU register.
 *
 * Nothing is printed unless the read fails.
 *
 * \param   regName         Name used in the application log.
 * \param   regAddr         Address of the register to read.
 * \param   pValue          Value read from the register.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_ReadOne(const char *regName,
                              uintptr_t regAddr,
                              uint64_t *pValue);

/**
 * \brief Reads a single 64-bit DRU register and checks it against \c expected.
 *
 * Nothing is printed unless the read fails or the value does not match, so that
 * a reported success means the value really did reach the hardware.
 *
 * \param   regName         Name used in the application log.
 * \param   regAddr         Address of the register to read.
 * \param   expected        Value the register is expected to hold.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_VerifyOne(const char *regName,
                                uintptr_t regAddr,
                                uint64_t expected);

/**
 * \brief Programs the static configuration of all DRU queues and channels.
 *
 * This function applies the queue configuration parameters (priority, orderId,
 * etc.) to all DRU queues and the channel configuration parameters (owner,
 * queue, event) to all DRU channels of the given instance, so that the
 * periodic readback has non-default values to compare against. The first queue
 * and channel register are read back afterwards, because a DRU whose MMRs are
 * readable but not writable accepts the writes without reporting an error.
 *
 * \param   pInst           DRU instance to configure.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_DruConfig(const DruApp_DruInstance *pInst);

/**
 * \brief Performs a register check for the DRU registers.
 *
 * This function reads the expected values from the DRU Queue and CHNRT registers,
 * periodically reads the actual register values, and compares them with the expected values.
 * It prints the comparison results to the console.
 *
 * \param   pInst           DRU instance to check.
 *
 * \retval  status          SystemP_SUCCESS on success
 *                          else otherwise..
 */
static int32_t DruApp_RegisterCheck(const DruApp_DruInstance *pInst);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static const DruApp_DruInstance gDruAppInstances[] =
{
#if defined (BUILD_C75X_1)
    {
        "C7X256V0",
        TISCI_DEV_C7X256V0,
        FALSE,
        CSL_C7X256V0_DRU_BASE,
        TRUE,
    },
#elif defined (BUILD_C75X_2)
    {
        "C7X256V1",
        TISCI_DEV_C7X256V1,
        FALSE,
        CSL_C7X256V1_DRU_BASE,
        TRUE,
    },
#else
    {
        "VPAC0",
        TISCI_DEV_VPAC0,
        TRUE,
        CSL_VPAC0_IVPAC_TOP_0_CFG_SLV_DRU_UTC_VPAC0_DRU_MMR_CFG_DRU_DRU_BASE,
        TRUE,
    },
    {
        "DMPAC0",
        TISCI_DEV_DMPAC0,
        TRUE,
        CSL_DMPAC0_DMPAC_TOP_CFG_SLV_DRU_UTC_DMPAC0_DRU_MMR_CFG_DRU_DRU_BASE,
        FALSE,
    },
#endif
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int32_t DruApp_ReadbackTest(void)
{
    int32_t                     retVal = SystemP_SUCCESS;
    int32_t                     instRetVal;
    uint32_t                    instCnt;
    const DruApp_DruInstance   *pInst;

    DebugP_log("DRU register readback application started ...\r\n");

    for(instCnt = 0U;
        instCnt < (sizeof(gDruAppInstances) / sizeof(gDruAppInstances[0U]));
        instCnt++)
    {
        pInst = &gDruAppInstances[instCnt];

        DebugP_log("DRU instance %s ..\r\n", pInst->name);

        instRetVal = SystemP_SUCCESS;

        if(TRUE == pInst->isPowerManaged)
        {
            instRetVal = DruApp_ModuleInit(pInst);
            if(SystemP_SUCCESS != instRetVal)
            {
                DebugP_logError("[Error] %s power and clock enable failed!!\r\n",
                                pInst->name);
            }
        }

        if(SystemP_SUCCESS == instRetVal)
        {
            instRetVal = DruApp_DruProbe(pInst);
            if(SystemP_SUCCESS != instRetVal)
            {
                DebugP_logError("[Error] %s DRU probe failed!!\r\n", pInst->name);
            }
        }

        if((SystemP_SUCCESS == instRetVal) && (TRUE == pInst->isConfigurable))
        {
            instRetVal = DruApp_DruConfig(pInst);
            if(SystemP_SUCCESS != instRetVal)
            {
                DebugP_logError("[Error] %s DRU config failed!!\r\n", pInst->name);
            }
        }

        if(SystemP_SUCCESS == instRetVal)
        {
            instRetVal = DruApp_RegisterCheck(pInst);
            if(SystemP_SUCCESS != instRetVal)
            {
                DebugP_logError("[Error] %s Dru register check failed!!\r\n",
                                pInst->name);
            }
        }

        if(SystemP_SUCCESS != instRetVal)
        {
            retVal = SystemP_FAILURE;
        }
    }

    if(SystemP_SUCCESS == retVal)
    {
        DebugP_log("All tests have passed!!\r\n");
    }

    return (retVal);
}

static int32_t DruApp_ModuleInit(const DruApp_DruInstance *pInst)
{
    int32_t     status = SystemP_FAILURE;
    uint32_t    moduleState = TISCI_MSG_VALUE_DEVICE_HW_STATE_OFF;
    uint32_t    resetState = 0U;
    uint32_t    contextLossState = 0U;

    // Turn on the module
    status = Sciclient_pmSetModuleState(pInst->tisciDevId,
                                        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                        TISCI_MSG_FLAG_AOP,
                                        SystemP_WAIT_FOREVER);
    if(SystemP_SUCCESS != status)
    {
        DebugP_log("  Sciclient_pmSetModuleState failed (%d)\r\n", status);
    }

    // Release the module from reset (0 deasserts, 1 asserts)
    if(SystemP_SUCCESS == status)
    {
        status = Sciclient_pmSetModuleRst(pInst->tisciDevId,
                                         DRU_APP_MODULE_RST_DEASSERT,
                                         SystemP_WAIT_FOREVER);
        if(SystemP_SUCCESS != status)
        {
            DebugP_log("  Sciclient_pmSetModuleRst failed (%d)\r\n", status);
        }
    }

    // Check if the module power-on was successful
    if(SystemP_SUCCESS == status)
    {
        status = Sciclient_pmGetModuleState(pInst->tisciDevId,
                                           &moduleState,
                                           &resetState,
                                           &contextLossState,
                                           SystemP_WAIT_FOREVER);
        if(SystemP_SUCCESS != status)
        {
            DebugP_log("  Sciclient_pmGetModuleState failed (%d)\r\n", status);
        }
    }

    if(SystemP_SUCCESS == status)
    {
        if(TISCI_MSG_VALUE_DEVICE_HW_STATE_ON == moduleState)
        {
            DebugP_log("  Module is ON\r\n");
        }
        else
        {
            DebugP_log("  TISCI device %u did not reach the ON state"
                       " (module state = %u, reset state = %u)\r\n",
                       pInst->tisciDevId, moduleState, resetState);
            status = SystemP_FAILURE;
        }
    }

    return (status);
}

static int32_t DruApp_DruProbe(const DruApp_DruInstance *pInst)
{
    int32_t             retVal = SystemP_FAILURE;
    const CSL_DRU_t    *pDruRegs = (const CSL_DRU_t *)pInst->druBase;
    uint64_t            value;

    DebugP_log("  DRU register probe ... ");

    retVal = DruApp_ReadOne("DRU CAPABILITIES",
                            (uintptr_t)&pDruRegs->DRURegs.DRU_CAPABILITIES,
                            &value);

    if(SystemP_SUCCESS == retVal)
    {
        retVal = DruApp_ReadOne("DRU QUEUE CFG0",
                                (uintptr_t)&pDruRegs->DRUQueues.CFG[0U],
                                &value);
    }

    if(SystemP_SUCCESS == retVal)
    {
        retVal = DruApp_ReadOne("DRU CHNRT CHST_SCHED0",
                                (uintptr_t)&pDruRegs->CHNRT[0U].CHST_SCHED,
                                &value);
    }

    if(SystemP_SUCCESS == retVal)
    {
        DebugP_log("Done !!\r\n");
    }

    return (retVal);
}

static int32_t DruApp_ReadOne(const char *regName,
                              uintptr_t regAddr,
                              uint64_t *pValue)
{
    int32_t         retVal = SystemP_FAILURE;
    IpFma_Status    status = IPFMA_E_PARAM;
    IpFma_RegDesc   registerDescriptor = { 0U, 0U, IPFMA_WIDTH_64 };

    status = IpFma_GetRegsValues(regAddr, &registerDescriptor, 1U);
    if(IPFMA_OK != status)
    {
        DebugP_log("\r\n  %s read at 0x%08x failed\r\n",
                   regName, (uint32_t)regAddr);
        retVal = SystemP_FAILURE;
    }
    else
    {
        *pValue = registerDescriptor.value;
        retVal = SystemP_SUCCESS;
    }

    return (retVal);
}

static int32_t DruApp_VerifyOne(const char *regName,
                                uintptr_t regAddr,
                                uint64_t expected)
{
    int32_t     retVal = SystemP_FAILURE;
    uint64_t    actual = 0U;

    retVal = DruApp_ReadOne(regName, regAddr, &actual);

    if((SystemP_SUCCESS == retVal) && (expected != actual))
    {
        DebugP_log("\r\n  %s at 0x%08x holds 0x%08x%08x, expected 0x%08x%08x\r\n",
                   regName,
                   (uint32_t)regAddr,
                   (uint32_t)(actual >> 32U),
                   (uint32_t)(actual & 0xFFFFFFFFU),
                   (uint32_t)(expected >> 32U),
                   (uint32_t)(expected & 0xFFFFFFFFU));
        retVal = SystemP_FAILURE;
    }

    return (retVal);
}

static int32_t DruApp_DruConfig(const DruApp_DruInstance *pInst)
{
    int32_t             retVal = SystemP_FAILURE;
    const CSL_DRU_t    *pDruRegs = (const CSL_DRU_t *)pInst->druBase;
    CSL_DruQueueConfig  queueCfg;
    CSL_DruChConfig     chCfg;
    uint32_t            queId, chId;
    uint64_t            expQueueCfg;
    uint64_t            expChstSched;

    queueCfg.priority         = DRU_APP_QUEUE_PRIORITY;
    queueCfg.qos              = DRU_APP_QUEUE_QOS;
    queueCfg.orderId          = DRU_APP_QUEUE_ORDER_ID;
    queueCfg.consecuitveTrans = DRU_APP_QUEUE_CONSECUTIVE_TRANS;
    queueCfg.rearbWait        = DRU_APP_QUEUE_REARB_WAIT;

    DebugP_log("  DRU queue and channel config ... ");

    for(queId = 0U; queId < DRU_NUM_QUEUES; queId++)
    {
        retVal = CSL_druQueueConfig(pDruRegs, queId, &queueCfg);
        if(SystemP_SUCCESS != retVal)
        {
            DebugP_logError("\r\n[Error] DRU queue config failed!!\r\n");
            break;
        }
    }

    if(SystemP_SUCCESS == retVal)
    {
        chCfg.type       = DRU_APP_CH_TYPE;
        chCfg.owner      = DRU_APP_CH_OWNER;
        chCfg.pauseOnErr = DRU_APP_CH_PAUSE_ON_ERR;
        chCfg.evtNum     = DRU_APP_CH_EVENT_NUM;
        chCfg.queueId    = DRU_APP_CH_QUEUE_ID;

        for(chId = 0U; chId < DRU_NUM_CHANNELS; chId++)
        {
            retVal = CSL_druChConfig(pDruRegs, chId, &chCfg);
            if(SystemP_SUCCESS != retVal)
            {
                DebugP_logError("\r\n[Error] DRU channel config failed!!\r\n");
                break;
            }
        }
    }

    if(SystemP_SUCCESS == retVal)
    {
        expQueueCfg  = CSL_FMK(DRU_QUEUE_CFG_PRI, DRU_APP_QUEUE_PRIORITY);
        expQueueCfg |= CSL_FMK(DRU_QUEUE_CFG_QOS, DRU_APP_QUEUE_QOS);
        expQueueCfg |= CSL_FMK(DRU_QUEUE_CFG_ORDERID, DRU_APP_QUEUE_ORDER_ID);
        expQueueCfg |= CSL_FMK(DRU_QUEUE_CFG_CONSECUTIVE_TRANS,
                               DRU_APP_QUEUE_CONSECUTIVE_TRANS);
        expQueueCfg |= CSL_FMK(DRU_QUEUE_CFG_REARB_WAIT,
                               DRU_APP_QUEUE_REARB_WAIT);

        retVal = DruApp_VerifyOne("DRU QUEUE CFG0",
                                  (uintptr_t)&pDruRegs->DRUQueues.CFG[0U],
                                  expQueueCfg);
    }

    if(SystemP_SUCCESS == retVal)
    {
        expChstSched = CSL_FMK(DRU_CHNRT_CHST_SCHED_QUEUE, DRU_APP_CH_QUEUE_ID);

        retVal = DruApp_VerifyOne("DRU CHNRT CHST_SCHED0",
                                  (uintptr_t)&pDruRegs->CHNRT[0U].CHST_SCHED,
                                  expChstSched);
    }

    if(SystemP_SUCCESS == retVal)
    {
        DebugP_log("Done !!\r\n");
    }

    return (retVal);
}

static int32_t DruApp_RegisterCheck(const DruApp_DruInstance *pInst)
{
    int32_t             retStatus = SystemP_SUCCESS;
    IpFma_Status        status = IPFMA_E_PARAM;
    const CSL_DRU_t    *pDruRegs = (const CSL_DRU_t *)pInst->druBase;
    uintptr_t           queueBase = (uintptr_t)&pDruRegs->DRUQueues;
    uintptr_t           chnrtBase = (uintptr_t)&pDruRegs->CHNRT[0U];
    uint32_t            checkCnt;
    IpFma_DruQueueRegs  druQueueRegsExpValues;
    IpFma_DruQueueRegs  druQueueRegsActualValues;
    IpFma_DruChnrtRegs  chnrtRegsExpValues;
    IpFma_DruChnrtRegs  chnrtRegsActualValues;

    DebugP_log("Register check starts ..\r\n");

    /* Get expected values from dru_queue registers */
    status = IpFma_Dru_GetQueueRegs(queueBase, &druQueueRegsExpValues);

    if(IPFMA_OK != status)
    {
        DebugP_logError("[Error] dru queue register read failed!!\r\n");
        retStatus = SystemP_FAILURE;
    }
    else
    {
        DebugP_log("Comparing expected-actual dru queue register values...\r\n");

        /******************************************************
                       Periodic readback
        *******************************************************/
        for(checkCnt = 0U; checkCnt < DRU_APP_PERIODIC_CHECK_NUM; checkCnt++)
        {
            /* Read the actual values from registers */
            status = IpFma_Dru_GetQueueRegs(queueBase, &druQueueRegsActualValues);

            if(IPFMA_OK != status)
            {
                DebugP_logError("[Error] dru queue register read failed!!\r\n");
                retStatus = SystemP_FAILURE;
                break;
            }

            /* Compare expected and actual values periodically */
            status = IpFma_Dru_CompareQueueRegs(&druQueueRegsExpValues,
                                                &druQueueRegsActualValues);
            if(IPFMA_OK == status)
            {
                DebugP_log("Expected - Actual dru queue register values match!\r\n");
            }
            else
            {
                DebugP_log("Expected - Actual dru queue values mismatch!\r\n");
                retStatus = SystemP_FAILURE;
            }

            ClockP_usleep(DRU_APP_PERIODIC_CHECK_DELAY_US);
        }
    }

    /* Get expected values from dru_chnrt registers */
    status = IpFma_Dru_GetChnrtRegs(chnrtBase, &chnrtRegsExpValues);

    if(IPFMA_OK != status)
    {
        DebugP_logError("[Error] dru chnrt register read failed!!\r\n");
        retStatus = SystemP_FAILURE;
    }
    else
    {
        DebugP_log("Comparing expected-actual dru chnrt register values...\r\n");

        for(checkCnt = 0U; checkCnt < DRU_APP_PERIODIC_CHECK_NUM; checkCnt++)
        {
            /* Read the actual values from registers */
            status = IpFma_Dru_GetChnrtRegs(chnrtBase, &chnrtRegsActualValues);

            if(IPFMA_OK != status)
            {
                DebugP_logError("[Error] dru chnrt register read failed!!\r\n");
                retStatus = SystemP_FAILURE;
                break;
            }

            /* Compare expected and actual values periodically */
            status = IpFma_Dru_CompareChnrtRegs(&chnrtRegsExpValues,
                                                &chnrtRegsActualValues);
            if(IPFMA_OK == status)
            {
                DebugP_log("Expected - Actual dru chnrt register values match!\r\n");
            }
            else
            {
                DebugP_log("Expected - Actual dru chnrt values mismatch!\r\n");
                retStatus = SystemP_FAILURE;
            }

            ClockP_usleep(DRU_APP_PERIODIC_CHECK_DELAY_US);
        }
    }

    DebugP_log("Register check done ...\r\n");

    return (retStatus);
}
