/*
 *  Copyright (c) Texas Instruments Incorporated 2020
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

/*!
 *  \file enet_rm.c
 *
 *  \brief This file contains the implementation of the Enet Resource Manager.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EnetTrace id for this module, must be unique within Enet LLD */
#define ENETTRACE_MOD_ID 0x004

#include <ti/drv/enet/enet_cfg.h>
#include <ti/drv/enet/include/core/enet_utils.h>
#include <ti/drv/enet/include/core/enet_soc.h>
#include <ti/drv/enet/include/core/enet_rm.h>
#include <ti/drv/enet/priv/core/enet_trace_priv.h>
#include <ti/drv/enet/priv/core/enet_rm_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define ENET_COREKEY_CONVERT_MAGIC_NUM  (0x38ACB7E6U)
#define ENET_COREID_2_COREKEY(coreId)   (((coreId) * 100U) + ENET_COREKEY_CONVERT_MAGIC_NUM)
#define ENET_COREKEY_2_COREID(coreKey)  (((coreKey) - ENET_COREKEY_CONVERT_MAGIC_NUM) / 100U)

typedef enum  EnetRm_ResourceType_tag
{
    ENET_RM_RESOURCE_TXCH,
    ENET_RM_RESOURCE_RXFLOW,
    ENET_RM_RESOURCE_MACADDRESS,
    ENET_RM_RESOURCE_HWPUSHINSTANCES,
    ENET_RM_RESOURCE_LAST = ENET_RM_RESOURCE_HWPUSHINSTANCES,
} EnetRm_ResourceType_e;

#define ENET_RM_RESOURCE_COUNT                      (ENET_RM_RESOURCE_LAST + 1)
#define ENET_RM_RESOURCE_ID_ANY                     (~0U)
#define ENET_RM_RESOURCE_INTERNAL_FLAG              (1U << 31U)
#define ENET_RM_GET_INTERNAL_ALLOC_COREID(coreId)   ((coreId) | ENET_RM_RESOURCE_INTERNAL_FLAG)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static EnetQ_Node *EnetRm_allocResourceNode(EnetRm_CoreResTbl_t *resTbl,
                                            uint32_t coreId,
                                            EnetRm_NodeInfo nodeInfo);

static EnetRm_ResEntry_t *EnetRm_allocResource(EnetRm_CoreResTbl_t *resTbl,
                                                uint32_t coreId,
                                                EnetRm_NodeInfo nodeInfo);

static EnetRm_ResEntry_t *EnetRm_findResource(EnetRm_ResEntry_t *resTable,
                                                   uint32_t numTableEntries,
                                                   uint32_t coreId,
                                                   uint32_t resId);

static void EnetRm_initResourceNode(EnetRm_ResEntry_t *res,
                                    uint32_t resId);

static void EnetRm_initResourceQ(EnetQ *resQ,
                                 EnetRm_ResEntry_t *resNodes,
                                 uint32_t resNodeLen,
                                 uint32_t startResIdx,
                                 uint32_t numResource);

static void EnetRm_initTxResourceQ(EnetQ *resQ,
                                   EnetRm_ResEntry_t *resNodes,
                                   uint32_t resNodeLen,
                                   uint32_t numResource,
                                   const EnetRm_TxCh *txCh);

static void EnetRm_initCoreResourceQ(EnetRm_CoreResInfo_t *coreResTbl,
                                     uint32_t coreId,
                                     EnetRm_ResEntry_t *resNodes,
                                     uint32_t resNodeLen,
                                     uint32_t startResIdx,
                                     uint32_t numResource);

static EnetQ *EnetRm_getFreeQ(EnetRm_CoreResTbl_t *resTbl,
                              uint32_t coreId);

static int32_t EnetRm_freeResource(EnetRm_CoreResTbl_t *resTbl,
                                   EnetRm_ResEntry_t *resList,
                                   uint32_t resCount,
                                   uint32_t coreId,
                                   uint32_t resId);

static void EnetRm_freeCoreResource(EnetRm_CoreResTbl_t *resTbl,
                                    EnetRm_ResEntry_t *resList,
                                    uint32_t numTableEntries,
                                    uint32_t coreId);

static void EnetRm_initTxObj(EnetRm_TxChObj *txObj,
                             const EnetRm_Cfg *rmCfg);

static void EnetRm_initRxObj(EnetRm_RxFlowIdxObj *rxObj,
                             const EnetRm_Cfg *rmCfg,
                             uint32_t chIdx);

static void EnetRm_initHwPushObj(EnetRm_HwPushObj *hwPushObj,
                                 const EnetRm_Cfg *rmCfg);

static void EnetRm_initMacObj(EnetRm_MacAddressObj *macObj,
                              const EnetRm_Cfg *rmCfg);

static void EnetRm_initAttachObj(EnetRm_CoreAttachInfo *coreAttachObj);

static int32_t EnetRm_mapMacAddr2ResourceIdx(EnetRm_MacAddressPool *macPool,
                                             uint8_t macAddr[],
                                             uint32_t *resIndex);

static int32_t EnetRm_validateResPartInfo(const EnetRm_Cfg *rmCfg,
                                          uint32_t maxTxResCnt,
                                          uint32_t maxRxResCnt,
                                          uint32_t maxMacResCnt);

static bool EnetRm_isCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                  uint32_t coreId,
                                  uint32_t *attachIndex);

static int32_t EnetRm_validateCoreKey(EnetRm_Handle hRm,
                                      uint32_t coreKey);

static void EnetRm_addCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                   uint32_t coreId);

static void EnetRm_delCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                   uint32_t coreId);

static EnetRm_ResourceInfo *EnetRm_getCoreResourceInfo(EnetRm_Cfg *cfg,
                                                       uint32_t coreId);

static int32_t EnetRm_validateRxFlow(EnetRm_Handle hRm,
                                     uint32_t coreKey,
                                     uint32_t chIdx,
                                     uint32_t flowIdx);

static int32_t EnetRm_allocInternalRxFlowIdx(EnetRm_RxFlowIdxObj *rxObj,
                                             uint32_t coreId,
                                             uint32_t *flowIdx,
                                             bool internalAllocation);

static int32_t EnetRm_allocRxFlowIdx(EnetRm_Handle hRm,
                                     uint32_t coreKey,
                                     uint32_t chIdx,
                                     uint32_t *startIdx,
                                     uint32_t *flowIdx);

static int32_t EnetRm_allocTxChNum(EnetRm_Handle hRm,
                                   uint32_t coreKey,
                                   uint32_t *txPSILThreadId,
                                   uint32_t chNum);

static int32_t EnetRm_allocHwPushInst(EnetRm_Handle hRm,
                                      uint32_t coreKey,
                                      uint32_t *hwPushInst);

static int32_t EnetRm_allocMacAddr(EnetRm_Handle hRm,
                                   uint32_t coreKey,
                                   uint8_t macAddr[]);

static int32_t EnetRm_freeInternalRxFlowIdx(EnetRm_RxFlowIdxObj *rxObj,
                                            uint32_t coreId,
                                            uint32_t flowIdx,
                                            bool internalAllocation);

static int32_t EnetRm_freeRxFlowIdx(EnetRm_Handle hRm,
                                    uint32_t coreKey,
                                    uint32_t chIdx,
                                    uint32_t flowIdx);

static int32_t EnetRm_freeTxChNum(EnetRm_Handle hRm,
                                  uint32_t coreKey,
                                  uint32_t txChNum);

static int32_t EnetRm_freeHwPushInst(EnetRm_Handle hRm,
                                     uint32_t coreKey,
                                     uint32_t hwPushInst);

static int32_t EnetRm_freeMacAddr(EnetRm_Handle hRm,
                                  uint32_t coreKey,
                                  uint8_t macAddr[]);

static int32_t EnetRm_validateCoreIoctlPrivilege(EnetRm_Obj *hRm,
                                                 uint32_t cmd,
                                                 uint32_t coreId);

static int32_t EnetRm_attachCore(EnetRm_Handle hRm,
                                 uint32_t coreId,
                                 uint32_t *coreKey);

static int32_t EnetRm_detachCore(EnetRm_Handle hRm,
                                 uint32_t coreKey);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
/* Public RM IOCTL validation data. */
static Enet_IoctlValidate gEnetRm_ioctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_ALLOC_MAC_ADDR,
                          sizeof(uint32_t),
                          sizeof(EnetRm_AllocMacAddrOutArgs)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_FREE_MAC_ADDR,
                          sizeof(EnetRm_FreeMacAddrInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_ALLOC_RX_FLOW,
                          sizeof(EnetRm_AllocRxFlowInArgs),
                          sizeof(EnetRm_AllocRxFlow)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_FREE_RX_FLOW,
                          sizeof(EnetRm_FreeRxFlowInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_ALLOC_TX_CH_PEERID,
                          sizeof(EnetRm_AllocTxChInArgs),
                          sizeof(uint32_t)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_FREE_TX_CH_PEERID,
                          sizeof(EnetRm_FreeTxChInArgs),
                          0U),
    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_ALLOC_HW_PUSH_INST,
                          sizeof(uint32_t),
                          sizeof(EnetRm_AllocHwPushOutArgs)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_FREE_HW_PUSH_INST,
                          sizeof(EnetRm_FreeHwPushInArgs),
                          0U),
};

/* Private RM IOCTL validation data. */
static Enet_IoctlValidate gEnetRm_privIoctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_VALIDATE_PERMISSION,
                          sizeof(EnetRm_ValidatePermissionInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_ATTACH,
                          sizeof(uint32_t),
                          sizeof(uint32_t)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_DETACH,
                          sizeof(uint32_t),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_INTERNAL_ALLOC_RX_FLOW,
                          sizeof(EnetRm_AllocInternalRxFlowInArgs),
                          sizeof(EnetRm_AllocRxFlow)),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_INTERNAL_FREE_RX_FLOW,
                          sizeof(EnetRm_FreeInternalRxFlowInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_RM_IOCTL_VALIDATE_RX_FLOW,
                          sizeof(EnetRm_ValidateRxFlowInArgs),
                          0U),
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EnetRm_open(EnetMod_Handle hMod,
                    Enet_Type enetType,
                    uint32_t instId,
                    const void *cfg,
                    uint32_t cfgSize)
{
    EnetRm_Handle hRm = (EnetRm_Handle)hMod;
    const EnetRm_Cfg *rmCfg = (const EnetRm_Cfg *)cfg;
    uint32_t i;
    int32_t status = ENET_SOK;

    Enet_devAssert(cfgSize == sizeof(EnetRm_Cfg),
                   "Invalid RM config params size %u (expected %u)",
                   cfgSize, sizeof(EnetRm_Cfg));

    if (rmCfg == NULL)
    {
        status = ENET_EBADARGS;
        ENETTRACE_ERR(status, "Invalid NULL RM config");
    }

    if (status == ENET_SOK)
    {
        status = EnetRm_validateResPartInfo(rmCfg,
                                            ENET_ARRAYSIZE(hRm->txObj.txRes),
                                            ENET_ARRAYSIZE(hRm->rxObj[0U].rxRes),
                                            ENET_ARRAYSIZE(hRm->macObj.macRes));
        ENETTRACE_ERR_IF(status != ENET_SOK, status, "Resource partition validation failed");
    }

    if (ENET_SOK == status)
    {
        hRm->cfg = *rmCfg;
        EnetRm_initTxObj(&hRm->txObj, rmCfg);
        EnetRm_initMacObj(&hRm->macObj, rmCfg);
        EnetRm_initAttachObj(&hRm->coreAttachObj);
        EnetRm_initHwPushObj(&hRm->hwPushObj, rmCfg);

        hRm->numRxCh = rmCfg->numRxCh;
        for (i = 0U; i < hRm->numRxCh; i++)
        {
            EnetRm_initRxObj(&hRm->rxObj[i], rmCfg, i);
        }
    }

    return status;
}

int32_t EnetRm_rejoin(EnetMod_Handle hMod,
                      Enet_Type enetType,
                      uint32_t instId)
{
    return ENET_ENOTSUPPORTED;
}

void EnetRm_close(EnetMod_Handle hMod)
{
    EnetRm_Handle hRm = (EnetRm_Handle)hMod;
    EnetRm_ResPrms *resInfo = &hRm->cfg.resPartInfo;
    EnetQ *pQ;
    uint32_t txFreeResCnt = 0U;
    uint32_t rxFreeResCnt[ENET_RM_NUM_RXCHAN_MAX];
    uint32_t macFreeResCnt = 0U;
    uint32_t i;
    uint32_t j;

    for (j = 0U; j < hRm->numRxCh; j++)
    {
        rxFreeResCnt[j] = 0U;
    }

    /* Assert all internally allocated flows are freed */
    for (j = 0U; j < hRm->numRxCh; j++)
    {
        Enet_assert((hRm->rxObj[j].internalAllocCoreId == ENET_RM_INVALIDCORE) &&
                    (hRm->rxObj[j].internalAllocCount == 0U));
    }
    Enet_assert(resInfo->numCores <= ENET_ARRAYSIZE(resInfo->coreResInfo));

    for (i = 0U; i < resInfo->numCores; i++)
    {
        EnetRm_detachCore(hRm, ENET_COREID_2_COREKEY(resInfo->coreResInfo[i].coreId));

        pQ = EnetRm_getFreeQ(&hRm->txObj.txResTbl, resInfo->coreResInfo[i].coreId);
        if (NULL != pQ)
        {
            txFreeResCnt += EnetQueue_getQCount(pQ);
        }

        for (j = 0U; j < hRm->numRxCh; j++)
        {
            pQ = EnetRm_getFreeQ(&hRm->rxObj[j].rxResTbl, resInfo->coreResInfo[i].coreId);
            if (NULL != pQ)
            {
                rxFreeResCnt[j] += EnetQueue_getQCount(pQ);
            }
        }

        pQ = EnetRm_getFreeQ(&hRm->macObj.macTbl, resInfo->coreResInfo[i].coreId);
        if (NULL != pQ)
        {
            macFreeResCnt += EnetQueue_getQCount(pQ);
        }
    }

    Enet_assert(txFreeResCnt == hRm->txObj.resCnt);
    Enet_assert(macFreeResCnt == hRm->macObj.resCnt);

    for (j = 0U; j < hRm->numRxCh; j++)
    {
        Enet_assert(rxFreeResCnt[j] == hRm->rxObj[j].resCnt);
    }
}

int32_t EnetRm_ioctl(EnetMod_Handle hMod,
                     uint32_t cmd,
                     Enet_IoctlPrms *prms)
{
    EnetRm_Handle hRm = (EnetRm_Handle)hMod;
    int32_t status = ENET_SOK;

#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate Enet RM IOCTL parameters */
    if (ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_GENERIC)
    {
        if (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PUBLIC)
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gEnetRm_ioctlValidate,
                                        ENET_ARRAYSIZE(gEnetRm_ioctlValidate));
        }
        else
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gEnetRm_privIoctlValidate,
                                        ENET_ARRAYSIZE(gEnetRm_privIoctlValidate));
        }

        ENETTRACE_ERR_IF(status != ENET_SOK, status, "IOCTL 0x%08x params are not valid", cmd);
    }
#endif

    if (status == ENET_SOK)
    {
        switch (cmd)
        {
            case ENET_RM_IOCTL_ALLOC_MAC_ADDR:
            {
                uint32_t coreKey = *((uint32_t *)prms->inArgs);
                EnetRm_AllocMacAddrOutArgs *outArgs = (EnetRm_AllocMacAddrOutArgs *)prms->outArgs;

                status = EnetRm_allocMacAddr(hRm, coreKey, &outArgs->macAddr[0U]);
            }
            break;

            case ENET_RM_IOCTL_FREE_MAC_ADDR:
            {
                EnetRm_FreeMacAddrInArgs *inArgs = (EnetRm_FreeMacAddrInArgs *)prms->inArgs;

                status = EnetRm_freeMacAddr(hRm, inArgs->coreKey, &inArgs->macAddr[0U]);
            }
            break;

            case ENET_RM_IOCTL_ALLOC_RX_FLOW:
            {
                EnetRm_AllocRxFlowInArgs *inArgs = (EnetRm_AllocRxFlowInArgs *)prms->inArgs;
                EnetRm_AllocRxFlow *outArgs = (EnetRm_AllocRxFlow *)prms->outArgs;

                status = EnetRm_allocRxFlowIdx(hRm,
                                               inArgs->coreKey,
                                               inArgs->chIdx,
                                               &outArgs->startIdx,
                                               &outArgs->flowIdx);
            }
            break;

            case ENET_RM_IOCTL_FREE_RX_FLOW:
            {
                EnetRm_FreeRxFlowInArgs *inArgs = (EnetRm_FreeRxFlowInArgs *)prms->inArgs;

                status = EnetRm_freeRxFlowIdx(hRm, inArgs->coreKey, inArgs->chIdx, inArgs->flowIdx);
            }
            break;

            case ENET_RM_IOCTL_ALLOC_HW_PUSH_INST:
            {
                uint32_t coreKey = *((uint32_t *)prms->inArgs);
                EnetRm_AllocHwPushOutArgs *outArgs = (EnetRm_AllocHwPushOutArgs *)prms->outArgs;

                status = EnetRm_allocHwPushInst(hRm, coreKey, &outArgs->hwPushNum);
            }
            break;

            case ENET_RM_IOCTL_FREE_HW_PUSH_INST:
            {
                uint32_t coreKey = *((uint32_t *)prms->inArgs);
                EnetRm_FreeHwPushInArgs *inArgs = (EnetRm_FreeHwPushInArgs *)prms->inArgs;

                status = EnetRm_freeHwPushInst(hRm, coreKey, inArgs->hwPushNum);
            }
            break;

            case ENET_RM_IOCTL_ALLOC_TX_CH_PEERID:
            {
                EnetRm_AllocTxChInArgs *inArgs = (EnetRm_AllocTxChInArgs *)prms->inArgs;
                uint32_t *txPSILThreadId = (uint32_t *)prms->outArgs;

                status = EnetRm_allocTxChNum(hRm, inArgs->coreKey, txPSILThreadId, inArgs->chIdx);
            }
            break;

            case ENET_RM_IOCTL_FREE_TX_CH_PEERID:
            {
                EnetRm_FreeTxChInArgs *inArgs = (EnetRm_FreeTxChInArgs *)prms->inArgs;

                status = EnetRm_freeTxChNum(hRm, inArgs->coreKey, inArgs->txChNum);
            }
            break;

            case ENET_RM_IOCTL_VALIDATE_PERMISSION:
            {
                const EnetRm_ValidatePermissionInArgs *inArgs = (const EnetRm_ValidatePermissionInArgs *)prms->inArgs;

                status = EnetRm_validateCoreIoctlPrivilege(hRm, inArgs->cmd, inArgs->coreId);
            }
            break;

            case ENET_RM_IOCTL_ATTACH:
            {
                uint32_t *pCoreId  = (uint32_t *)prms->inArgs;
                uint32_t *pCoreKey = (uint32_t *)prms->outArgs;

                status = EnetRm_attachCore(hRm, *pCoreId, pCoreKey);
            }
            break;

            case ENET_RM_IOCTL_DETACH:
            {
                uint32_t *pCoreKey = (uint32_t *)prms->inArgs;

                status = EnetRm_detachCore(hRm, *pCoreKey);
            }
            break;

            case ENET_RM_IOCTL_INTERNAL_ALLOC_RX_FLOW:
            {
                EnetRm_AllocInternalRxFlowInArgs *inArgs = (EnetRm_AllocInternalRxFlowInArgs *)prms->inArgs;
                EnetRm_AllocRxFlow *outArgs = (EnetRm_AllocRxFlow *)prms->outArgs;
                EnetRm_RxFlowIdxObj *rxObj = &hRm->rxObj[inArgs->chIdx];

                Enet_assert((rxObj->internalAllocCoreId == ENET_RM_INVALIDCORE) ||
                            (rxObj->internalAllocCoreId == inArgs->coreId));

                status = EnetRm_allocInternalRxFlowIdx(rxObj,
                                                       inArgs->coreId,
                                                       &outArgs->flowIdx,
                                                       BTRUE);
                if (ENET_SOK == status)
                {
                    outArgs->startIdx = hRm->cfg.rxStartFlowIdx[inArgs->chIdx];
                    Enet_assert(outArgs->flowIdx < hRm->cfg.rxFlowIdxCnt[inArgs->chIdx]);
                    rxObj->internalAllocCoreId = inArgs->coreId;
                    rxObj->internalAllocCount++;
                }
            }
            break;

            case ENET_RM_IOCTL_INTERNAL_FREE_RX_FLOW:
            {
                EnetRm_FreeInternalRxFlowInArgs *inArgs = (EnetRm_FreeInternalRxFlowInArgs *)prms->inArgs;
                EnetRm_RxFlowIdxObj *rxObj = &hRm->rxObj[inArgs->chIdx];

                status = EnetRm_freeInternalRxFlowIdx(rxObj,
                                                      inArgs->coreId,
                                                      inArgs->flowIdx,
                                                      BTRUE);
                if (status == ENET_SOK)
                {
                    Enet_assert(rxObj->internalAllocCount > 0);
                    rxObj->internalAllocCount--;
                    if (rxObj->internalAllocCount == 0)
                    {
                        rxObj->internalAllocCoreId = ENET_RM_INVALIDCORE;
                    }
                }
            }
            break;

            case ENET_RM_IOCTL_VALIDATE_RX_FLOW:
            {
                EnetRm_ValidateRxFlowInArgs *inArgs = (EnetRm_ValidateRxFlowInArgs *)prms->inArgs;

                status = EnetRm_validateRxFlow(hRm, inArgs->coreKey, inArgs->chIdx, inArgs->flowIdx);
            }
            break;

            default:
            {
                status = ENET_EINVALIDPARAMS;
            }
        }
    }

    return status;
}

static EnetQ_Node *EnetRm_allocResourceNode(EnetRm_CoreResTbl_t *resTbl,
                                            uint32_t coreId,
                                            EnetRm_NodeInfo nodeInfo)
{
    EnetQ_Node *resource = NULL;
    uint32_t i;
    EnetRm_ResEntry_t *resEntry;
    uint32_t numResCount = 0U;
    uint32_t status = ENET_SOK;

    Enet_assert(resTbl->numCores <= ENET_ARRAYSIZE(resTbl->coreResTbl));

    /* Invalid resource node allocation */
    if (nodeInfo.nodeType == ENET_RM_NODE_NONE)
    {
        status = ENET_EFAIL;
        resource = NULL;
        ENETTRACE_ERR(status, "Invalid resource node allocation");
        
    }

    if (status == ENET_SOK)
    {
        for (i = 0U; i < resTbl->numCores; i++)
        {
            if (coreId == resTbl->coreResTbl[i].coreId)
            {
                break;
            }
        }

        if (i < resTbl->numCores)
        {
            /* If the nodeId is not defined, dequeue the first resource allocated
            * for this core */
            if (nodeInfo.nodeId == ENET_RM_UNDEFINED_NODEID)
            {
                resource = EnetQueue_deq(&resTbl->coreResTbl[i].resQ);
            }
            else
            {
                /* Search for the specified nodeId in the resQ, return if present.*/
                numResCount = EnetQueue_getQCount(&resTbl->coreResTbl[i].resQ);
                while (numResCount != 0U)
                {
                    resource = EnetQueue_deq(&resTbl->coreResTbl[i].resQ);
                    resEntry = container_of(resource, EnetRm_ResEntry_t, node);
                    if(resEntry->id != nodeInfo.nodeId)
                    {
                        EnetQueue_enq(&resTbl->coreResTbl[i].resQ, resource);
                        resource = NULL;
                    }
                    else
                    {
                        break;
                    }
                    numResCount--;
                }
                if (resource == NULL)
                {
                    status = ENET_EFAIL;
                    ENETTRACE_ERR(status, "EnetRm_allocResourceNode failed, nodeId not found");
                }
            }
        }
        else
        {
            resource = NULL;
        }
    }

    return resource;
}

static EnetRm_ResEntry_t *EnetRm_allocResource(EnetRm_CoreResTbl_t *resTbl,
                                                uint32_t coreId,
                                                EnetRm_NodeInfo nodeInfo)
{
    EnetQ_Node *resNode;
    EnetRm_ResEntry_t *resource;

    resNode = EnetRm_allocResourceNode(resTbl, coreId, nodeInfo);
    if (resNode != NULL)
    {
        resource = container_of(resNode, EnetRm_ResEntry_t, node);
        memset(&resource->node, 0, sizeof(resource->node));
        Enet_assert(resource->ownerCoreId == ENET_RM_INVALIDCORE);
        resource->ownerCoreId = coreId;
    }
    else
    {
        resource = NULL;
    }

    return resource;
}

static EnetRm_ResEntry_t *EnetRm_findResource(EnetRm_ResEntry_t *resTable,
                                                   uint32_t numTableEntries,
                                                   uint32_t coreId,
                                                   uint32_t resId)
{
    EnetRm_ResEntry_t *resource;
    uint32_t i;

    for (i = 0U; i < numTableEntries; i++)
    {
        if (resTable[i].ownerCoreId == coreId)
        {
            if ((resTable[i].id == resId) ||
                (resId == ENET_RM_RESOURCE_ID_ANY))
            {
                break;
            }
        }
    }

    if (i < numTableEntries)
    {
        resource = &resTable[i];
    }
    else
    {
        resource = NULL;
    }

    return resource;
}

static void EnetRm_initNodeInfo(EnetRm_NodeInfo *nodeInfo)
{
    nodeInfo->nodeType = ENET_RM_NODE_NONE;
    nodeInfo->nodeId   = ENET_RM_UNDEFINED_NODEID;
}

static void EnetRm_initResourceNode(EnetRm_ResEntry_t *res,
                                    uint32_t resId)
{
    memset(res, 0, sizeof(*res));
    res->ownerCoreId = ENET_RM_INVALIDCORE;
    res->id          = resId;
}

static void EnetRm_initResourceQ(EnetQ *resQ,
                                 EnetRm_ResEntry_t *resNodes,
                                 uint32_t resNodeLen,
                                 uint32_t startResIdx,
                                 uint32_t numResource)
{
    uint32_t i;

    EnetQueue_initQ(resQ);

    for (i = 0U; i < numResource; i++)
    {
        Enet_assert(i < resNodeLen);
        EnetRm_initResourceNode(&resNodes[i], startResIdx + i);
        EnetQueue_enq(resQ, &resNodes[i].node);
    }
}

static void EnetRm_initTxResourceQ(EnetQ *resQ,
                                   EnetRm_ResEntry_t *resNodes,
                                   uint32_t resNodeLen,
                                   uint32_t numResource,
                                   const EnetRm_TxCh *txCh)
{
    uint32_t i;
    uint32_t chNum;

    EnetQueue_initQ(resQ);

    for (i = 0U; i < numResource; i++)
    {
        Enet_assert(i < resNodeLen);
        chNum = txCh[i];
        EnetRm_initResourceNode(&resNodes[chNum], chNum);
        EnetQueue_enq(resQ, &resNodes[chNum].node);
    }
}

static void EnetRm_initCoreResourceQ(EnetRm_CoreResInfo_t *coreResTbl,
                                     uint32_t coreId,
                                     EnetRm_ResEntry_t *resNodes,
                                     uint32_t resNodeLen,
                                     uint32_t startResIdx,
                                     uint32_t numResource)
{
    coreResTbl->coreId = coreId;
    EnetRm_initResourceQ(&coreResTbl->resQ, resNodes, resNodeLen, startResIdx, numResource);
}

static EnetQ *EnetRm_getFreeQ(EnetRm_CoreResTbl_t *resTbl,
                              uint32_t coreId)
{
    uint32_t i;
    EnetQ *queue;

    Enet_assert(resTbl->numCores <= ENET_ARRAYSIZE(resTbl->coreResTbl));
    for (i = 0U; i < resTbl->numCores; i++)
    {
        if ((coreId == resTbl->coreResTbl[i].coreId) ||
            (coreId == ENET_RM_GET_INTERNAL_ALLOC_COREID(resTbl->coreResTbl[i].coreId)))
        {
            break;
        }
    }

    if (i < resTbl->numCores)
    {
        queue = &resTbl->coreResTbl[i].resQ;
    }
    else
    {
        queue = NULL;
    }

    return queue;
}

static int32_t EnetRm_freeResource(EnetRm_CoreResTbl_t *resTbl,
                                   EnetRm_ResEntry_t *resList,
                                   uint32_t resCount,
                                   uint32_t coreId,
                                   uint32_t resId)
{
    EnetRm_ResEntry_t *resource;
    EnetQ *freeQ;
    int32_t status;

    freeQ = EnetRm_getFreeQ(resTbl, coreId);
    if (freeQ == NULL)
    {
        status = ENET_EFAIL;
    }
    else
    {
        status = ENET_SOK;
    }

    if (ENET_SOK == status)
    {
        resource = EnetRm_findResource(resList, resCount, coreId, resId);
        if (resource != NULL)
        {
            Enet_assert(resource->ownerCoreId == coreId);

            /* Do not use argument resId to init resource as it may be RSC_ANY
             * which is an invalid id
             */
            EnetRm_initResourceNode(resource, resource->id);
            EnetQueue_enq(freeQ, &resource->node);
            status = ENET_SOK;
        }
        else
        {
            status = ENET_EFAIL;
        }
    }

    return status;
}

static void EnetRm_freeCoreResource(EnetRm_CoreResTbl_t *resTbl,
                                    EnetRm_ResEntry_t *resList,
                                    uint32_t numTableEntries,
                                    uint32_t coreId)
{
    int32_t status;

    do
    {
        status = EnetRm_freeResource(resTbl, resList, numTableEntries, coreId, ENET_RM_RESOURCE_ID_ANY);
    }
    while (status == ENET_SOK);
}

static void EnetRm_initTxObj(EnetRm_TxChObj *txObj,
                             const EnetRm_Cfg *rmCfg)
{
    const uint32_t resTableSize = ENET_ARRAYSIZE(txObj->txRes);
    uint32_t allocResCnt;
    uint32_t txChPeerId;
    uint32_t coreCnt;
    uint32_t i;
    bool isAbsTxChAllocated = rmCfg->resPartInfo.isStaticTxChanAllocated;

    txChPeerId = EnetSoc_getTxChPeerId(rmCfg->enetType, rmCfg->instId, 0U);

    txObj->txPSILThreadIdOffset     = txChPeerId;
    txObj->txResTbl.numCores = rmCfg->resPartInfo.numCores;

    /* Initialize all TX resources to prevent uninitialized access */
    if (isAbsTxChAllocated)
    {
        for (i = 0U; i < resTableSize; i++)
        {
            EnetRm_initResourceNode(&txObj->txRes[i], i);
        }
    }

    allocResCnt = 0U;
    for (i = 0U; i < rmCfg->resPartInfo.numCores; i++)
    {
        coreCnt = rmCfg->resPartInfo.coreResInfo[i].numTxCh;

        Enet_assert((allocResCnt + coreCnt) <= txChPeerId);
        Enet_assert((allocResCnt + coreCnt) <= resTableSize);

        /* If application has allocated absolute tx channels */
        if (isAbsTxChAllocated)
        {
            txObj->txResTbl.coreResTbl[i].coreId = rmCfg->resPartInfo.coreResInfo[i].coreId;
            EnetRm_initTxResourceQ(&txObj->txResTbl.coreResTbl[i].resQ,
                                   txObj->txRes,
                                   (resTableSize - allocResCnt),
                                   coreCnt,
                                   rmCfg->resPartInfo.coreResInfo[i].txCh);
        }
        else
        {
            EnetRm_initCoreResourceQ(&txObj->txResTbl.coreResTbl[i],
                                     rmCfg->resPartInfo.coreResInfo[i].coreId,
                                     &txObj->txRes[allocResCnt],
                                     (resTableSize - allocResCnt),
                                     allocResCnt,
                                     coreCnt);
        }
        allocResCnt += coreCnt;
    }

    txObj->resCnt = allocResCnt;
}

static void EnetRm_initRxObj(EnetRm_RxFlowIdxObj *rxObj,
                             const EnetRm_Cfg *rmCfg,
                             uint32_t chIdx)
{
    const uint32_t resTableSize = ENET_ARRAYSIZE(rxObj->rxRes);
    uint32_t allocResCnt;
    uint32_t socRxFlowCnt;
    uint32_t coreCnt;
    uint32_t i;

    socRxFlowCnt = EnetSoc_getRxFlowCount(rmCfg->enetType, rmCfg->instId);

    rxObj->rxResTbl.numCores = rmCfg->resPartInfo.numCores;

    allocResCnt = 0U;
    for (i = 0U; i < rmCfg->resPartInfo.numCores; i++)
    {
        coreCnt = rmCfg->resPartInfo.coreResInfo[i].numRxFlows;

        Enet_assert((allocResCnt + coreCnt) <= socRxFlowCnt);
        Enet_assert((allocResCnt + coreCnt) <= resTableSize);

        EnetRm_initCoreResourceQ(&rxObj->rxResTbl.coreResTbl[i],
                                 rmCfg->resPartInfo.coreResInfo[i].coreId,
                                 &rxObj->rxRes[allocResCnt],
                                 resTableSize - allocResCnt,
                                 allocResCnt,
                                 coreCnt);
        allocResCnt += coreCnt;
    }

    rxObj->resCnt = allocResCnt;
    rxObj->internalAllocCount  = 0U;
    rxObj->internalAllocCoreId = ENET_RM_INVALIDCORE;
}

static void EnetRm_initHwPushObj(EnetRm_HwPushObj *hwPushObj,
                                 const EnetRm_Cfg *rmCfg)
{
    const uint32_t resTableSize = ENET_ARRAYSIZE(hwPushObj->hwPushRes);
    uint32_t allocResCnt;
    uint32_t socHwPushCnt;
    uint32_t coreCnt;
    uint32_t i;
    
    socHwPushCnt = EnetSoc_getHwPushCount(rmCfg->enetType, rmCfg->instId);

    hwPushObj->hwPushResTbl.numCores = rmCfg->resPartInfo.numCores;

    allocResCnt = 0U;
    for (i = 0U; i < rmCfg->resPartInfo.numCores; i++)
    {
        coreCnt = rmCfg->resPartInfo.coreResInfo[i].numHwPush;

        Enet_assert((allocResCnt + coreCnt) <= socHwPushCnt);
        Enet_assert((allocResCnt + coreCnt) <= resTableSize);

        EnetRm_initCoreResourceQ(&hwPushObj->hwPushResTbl.coreResTbl[i],
                                 rmCfg->resPartInfo.coreResInfo[i].coreId,
                                 &hwPushObj->hwPushRes[allocResCnt],
                                 resTableSize - allocResCnt,
                                 allocResCnt,
                                 coreCnt);
        allocResCnt += coreCnt;
    }

    hwPushObj->resCnt = allocResCnt;
}

static void EnetRm_initMacObj(EnetRm_MacAddressObj *macObj,
                              const EnetRm_Cfg *rmCfg)
{
    const uint32_t resTableSize = ENET_ARRAYSIZE(macObj->macRes);
    uint32_t allocResCnt;
    uint32_t coreCnt;
    uint32_t i;

    macObj->macTbl.numCores = rmCfg->resPartInfo.numCores;

    allocResCnt = 0U;
    for (i = 0U; i < rmCfg->resPartInfo.numCores; i++)
    {
        coreCnt = rmCfg->resPartInfo.coreResInfo[i].numMacAddress;

        Enet_assert((allocResCnt + coreCnt) < resTableSize);
        Enet_assert((allocResCnt + coreCnt) <= rmCfg->macList.numMacAddress);

        EnetRm_initCoreResourceQ(&macObj->macTbl.coreResTbl[i],
                                 rmCfg->resPartInfo.coreResInfo[i].coreId,
                                 &macObj->macRes[allocResCnt],
                                 (resTableSize - allocResCnt),
                                 allocResCnt,
                                 coreCnt);
        allocResCnt += coreCnt;
    }

    macObj->resCnt = allocResCnt;
}

static void EnetRm_initAttachObj(EnetRm_CoreAttachInfo *coreAttachObj)
{
    coreAttachObj->numCores = 0U;
}

static int32_t EnetRm_mapMacAddr2ResourceIdx(EnetRm_MacAddressPool *macPool,
                                             uint8_t macAddr[],
                                             uint32_t *resIndex)
{
    uint32_t i;
    int32_t status = ENET_EFAIL;

    for (i = 0U; i < macPool->numMacAddress; i++)
    {
        if (memcmp(&macPool->macAddress[i][0], macAddr, ENET_MAC_ADDR_LEN) == 0)
        {
            break;
        }
    }

    if (i < macPool->numMacAddress)
    {
        *resIndex = i;
        status = ENET_SOK;
    }

    return status;
}

static int32_t EnetRm_validateResPartInfo(const EnetRm_Cfg *rmCfg,
                                          uint32_t maxTxResCnt,
                                          uint32_t maxRxResCnt,
                                          uint32_t maxMacResCnt)
{
    const EnetRm_ResPrms *resPartInfo = &rmCfg->resPartInfo;
    uint32_t txChCnt = 0U;
    uint32_t rxFlowCnt = 0U;
    uint32_t macAddrCnt = 0U;
    uint32_t socTxChCnt;
    uint32_t i;
    int32_t status = ENET_SOK;

    socTxChCnt = EnetSoc_getTxChCount(rmCfg->enetType, rmCfg->instId);

    if (resPartInfo->numCores >
        ENET_ARRAYSIZE(resPartInfo->coreResInfo))
    {
        status = ENET_EINVALIDPARAMS;
    }

    if (ENET_SOK == status)
    {
        for (i = 0U; i < resPartInfo->numCores; i++)
        {
            txChCnt    += resPartInfo->coreResInfo[i].numTxCh;
            rxFlowCnt  += resPartInfo->coreResInfo[i].numRxFlows;
            macAddrCnt += resPartInfo->coreResInfo[i].numMacAddress;
        }

        if ((txChCnt > socTxChCnt) ||
            (txChCnt > maxTxResCnt))
        {
            status = ENET_EINVALIDPARAMS;
        }

        if ((rxFlowCnt > rmCfg->rxFlowIdxCnt[0U]) ||
            (rxFlowCnt > maxRxResCnt))
        {
            status = ENET_EINVALIDPARAMS;
        }

        if ((macAddrCnt > rmCfg->macList.numMacAddress) ||
            (macAddrCnt > maxMacResCnt) ||
            (macAddrCnt > ENET_ARRAYSIZE(rmCfg->macList.macAddress)) ||
            (rmCfg->macList.numMacAddress > ENET_ARRAYSIZE(rmCfg->macList.macAddress)))
        {
            status = ENET_EINVALIDPARAMS;
        }
    }

    return status;
}

static bool EnetRm_isCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                  uint32_t coreId,
                                  uint32_t *attachIndex)
{
    bool coreAttached;
    uint32_t i;

    /* Initialize to avoid compiler warning.*/
    if (NULL != attachIndex)
    {
        *attachIndex = 0U;
    }

    for (i = 0U; i < coreAttachInfo->numCores; i++)
    {
        if (coreAttachInfo->attachedCores[i] == coreId)
        {
            break;
        }
    }

    if (i < coreAttachInfo->numCores)
    {
        coreAttached = BTRUE;
        if (attachIndex != NULL)
        {
            *attachIndex = i;
        }
    }
    else
    {
        coreAttached = BFALSE;
    }

    return coreAttached;
}

static int32_t EnetRm_validateCoreKey(EnetRm_Handle hRm,
                                      uint32_t coreKey)
{
    uint32_t coreId;
    int32_t status = ENET_SOK;

    coreId = ENET_COREKEY_2_COREID(coreKey);

    if (EnetRm_isCoreAttached(&hRm->coreAttachObj, coreId, (uint32_t *)NULL_PTR) == BFALSE)
    {
        status = ENET_EINVALIDPARAMS;
    }

    return status;
}

static void EnetRm_addCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                   uint32_t coreId)
{
    bool coreAttached;

    coreAttached = EnetRm_isCoreAttached(coreAttachInfo, coreId, (uint32_t *)NULL_PTR);

    Enet_assert(coreAttached == BFALSE);
    Enet_assert(coreAttachInfo->numCores < ENET_ARRAYSIZE(coreAttachInfo->attachedCores));

    coreAttachInfo->attachedCores[coreAttachInfo->numCores] = coreId;
    coreAttachInfo->numCores++;
}

static void EnetRm_delCoreAttached(EnetRm_CoreAttachInfo *coreAttachInfo,
                                   uint32_t coreId)
{
    bool coreAttached;
    uint32_t attachIndex;

    coreAttached = EnetRm_isCoreAttached(coreAttachInfo, coreId, &attachIndex);
    Enet_assert(coreAttached == BTRUE);
    Enet_assert(attachIndex < ENET_ARRAYSIZE(coreAttachInfo->attachedCores));
    Enet_assert(coreAttachInfo->numCores > 0);
    /* move the last entry to the detached core slot */
    coreAttachInfo->numCores--;
    coreAttachInfo->attachedCores[attachIndex] =
        coreAttachInfo->attachedCores[coreAttachInfo->numCores];
}

static EnetRm_ResourceInfo *EnetRm_getCoreResourceInfo(EnetRm_Cfg *cfg,
                                                       uint32_t coreId)
{
    EnetRm_ResourceInfo *resInfo;
    uint32_t i;

    Enet_assert(cfg->resPartInfo.numCores <= ENET_ARRAYSIZE(cfg->resPartInfo.coreResInfo));
    for (i = 0U; i < cfg->resPartInfo.numCores; i++)
    {
        if (cfg->resPartInfo.coreResInfo[i].coreId == coreId)
        {
            break;
        }
    }

    if (i < cfg->resPartInfo.numCores)
    {
        resInfo = &cfg->resPartInfo.coreResInfo[i];
    }
    else
    {
        resInfo = NULL;
    }

    return resInfo;
}

static int32_t EnetRm_validateRxFlow(EnetRm_Handle hRm,
                                     uint32_t coreKey,
                                     uint32_t chIdx,
                                     uint32_t flowIdx)
{
    EnetRm_RxFlowIdxObj *rxObj = &hRm->rxObj[chIdx];
    int32_t status = ENET_SOK;

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (ENET_SOK == status)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);
        EnetRm_ResEntry_t *res;

        res = EnetRm_findResource(rxObj->rxRes,
                                  ENET_ARRAYSIZE(rxObj->rxRes),
                                  coreId,
                                  flowIdx);

        status = (res != NULL) ? ENET_SOK : ENET_EINVALIDPARAMS;
    }

    return status;
}

static int32_t EnetRm_allocInternalRxFlowIdx(EnetRm_RxFlowIdxObj *rxObj,
                                             uint32_t coreId,
                                             uint32_t *flowIdx,
                                             bool internalAllocation)
{
    EnetRm_ResEntry_t *rxRes;
    int32_t status;
    EnetRm_NodeInfo nodeInfo;

    EnetRm_initNodeInfo(&nodeInfo);
    nodeInfo.nodeType = ENET_RM_NODE_RX;
    rxRes = EnetRm_allocResource(&rxObj->rxResTbl, coreId, nodeInfo);
    if (rxRes != NULL)
    {
        Enet_assert(rxRes->ownerCoreId == coreId);
        if (internalAllocation)
        {
            rxRes->ownerCoreId = ENET_RM_GET_INTERNAL_ALLOC_COREID(rxRes->ownerCoreId);
        }

        Enet_assert(rxRes->id < rxObj->resCnt);
        *flowIdx = rxRes->id;
        status = ENET_SOK;
    }
    else
    {
        status = ENET_EALLOC;
    }

    return status;
}

static int32_t EnetRm_allocRxFlowIdx(EnetRm_Handle hRm,
                                     uint32_t coreKey,
                                     uint32_t chIdx,
                                     uint32_t *startIdx,
                                     uint32_t *flowIdx)
{
    int32_t status;

    status = EnetRm_validateCoreKey(hRm, coreKey);

    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        status = EnetRm_allocInternalRxFlowIdx(&hRm->rxObj[chIdx], coreId, flowIdx, BFALSE);
        if (ENET_SOK == status)
        {
            *startIdx = hRm->cfg.rxStartFlowIdx[chIdx];
            Enet_assert(*flowIdx < hRm->cfg.rxFlowIdxCnt[chIdx]);
        }
    }

    return status;
}

static int32_t EnetRm_allocTxChNum(EnetRm_Handle hRm,
                                   uint32_t coreKey,
                                   uint32_t *txPSILThreadId,
                                   uint32_t chNum)
{
    uint32_t socTxChCnt;
    int32_t status;
    EnetRm_NodeInfo nodeInfo;

    *txPSILThreadId = ENET_RM_TXCHNUM_INVALID;
    EnetRm_initNodeInfo(&nodeInfo);

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (status == ENET_SOK)
    {
        EnetRm_ResEntry_t *txRes;
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        nodeInfo.nodeType = ENET_RM_NODE_TX;
        if (chNum != ENET_RM_TX_CH_ANY)
        {
            nodeInfo.nodeId   = chNum;
        }
        txRes = EnetRm_allocResource(&hRm->txObj.txResTbl, coreId, nodeInfo);
        if (txRes != NULL)
        {
            socTxChCnt = EnetSoc_getTxChCount(hRm->cfg.enetType, hRm->cfg.instId);
            Enet_assert(txRes->ownerCoreId == coreId);
            /* If application has allocated absolute tx channels, then this error check should not be there
             * As application might allocate a tx channel with id > the resource count
             * For example - Application allocated 1 tx channel of priority 7, then in that case
             * id will be 7 and resCnt will be 1 (1 channel allocated) */
            if (hRm->cfg.resPartInfo.isStaticTxChanAllocated == BFALSE)
            {
                Enet_assert(txRes->id < hRm->txObj.resCnt);
            } 
            Enet_assert(txRes->id < socTxChCnt);
            *txPSILThreadId = txRes->id + hRm->txObj.txPSILThreadIdOffset;
            status = ENET_SOK;
        }
        else
        {
            status = ENET_EALLOC;
        }
    }

    return status;
}

static int32_t EnetRm_allocMacAddr(EnetRm_Handle hRm,
                                   uint32_t coreKey,
                                   uint8_t macAddr[])
{
    int32_t status;
    EnetRm_NodeInfo nodeInfo;

    EnetRm_initNodeInfo(&nodeInfo);
    status = EnetRm_validateCoreKey(hRm, coreKey);

    if (status == ENET_SOK)
    {
        EnetRm_ResEntry_t *macRes;
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        nodeInfo.nodeType = ENET_RM_NODE_MAC;
        macRes = EnetRm_allocResource(&hRm->macObj.macTbl, coreId, nodeInfo);
        if (macRes != NULL)
        {
            Enet_assert(macRes->ownerCoreId == coreId);
            Enet_assert(macRes->id < hRm->cfg.macList.numMacAddress);
            Enet_assert(macRes->id < hRm->macObj.resCnt);
            Enet_assert(macRes->id < ENET_ARRAYSIZE(hRm->cfg.macList.macAddress));
            memcpy(macAddr, &hRm->cfg.macList.macAddress[macRes->id][0], ENET_MAC_ADDR_LEN);
            status = ENET_SOK;
        }
        else
        {
            status = ENET_EALLOC;
        }
    }

    return status;
}

static int32_t EnetRm_allocHwPushInst(EnetRm_Handle hRm,
                                      uint32_t coreKey,
                                      uint32_t *hwPushNum)
{
    int32_t status;
    EnetRm_NodeInfo nodeInfo;

    EnetRm_initNodeInfo(&nodeInfo);
    status = EnetRm_validateCoreKey(hRm, coreKey);

    if (status == ENET_SOK)
    {
        EnetRm_ResEntry_t *hwPushRes;
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        nodeInfo.nodeType = ENET_RM_NODE_HW_PUSH;
        hwPushRes = EnetRm_allocResource(&hRm->hwPushObj.hwPushResTbl, coreId, nodeInfo);
        if (hwPushRes != NULL)
        {
            Enet_assert(hwPushRes->ownerCoreId == coreId);
            Enet_assert(hwPushRes->id < hRm->hwPushObj.resCnt);
            
            *hwPushNum = ++(hwPushRes->id);
            status = ENET_SOK;
        }
        else
        {
            status = ENET_EALLOC;
        }
    }

    return status;
}


static int32_t EnetRm_freeInternalRxFlowIdx(EnetRm_RxFlowIdxObj *rxObj,
                                            uint32_t coreId,
                                            uint32_t flowIdx,
                                            bool internalAllocation)
{
    uint32_t matchCoreId;
    int32_t status = ENET_SOK;

    if (internalAllocation)
    {
        matchCoreId = ENET_RM_GET_INTERNAL_ALLOC_COREID(coreId);
    }
    else
    {
        matchCoreId = coreId;
    }

    Enet_assert(rxObj->resCnt <= ENET_ARRAYSIZE(rxObj->rxRes));
    status = EnetRm_freeResource(&rxObj->rxResTbl,
                                 rxObj->rxRes,
                                 rxObj->resCnt,
                                 matchCoreId,
                                 flowIdx);
    return status;
}

static int32_t EnetRm_freeRxFlowIdx(EnetRm_Handle hRm,
                                    uint32_t coreKey,
                                    uint32_t chIdx,
                                    uint32_t flowIdx)
{
    int32_t status;

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        EnetRm_freeInternalRxFlowIdx(&hRm->rxObj[chIdx], coreId, flowIdx, BFALSE);
    }

    return status;
}

static int32_t EnetRm_freeTxChNum(EnetRm_Handle hRm,
                                  uint32_t coreKey,
                                  uint32_t txChNum)
{
    EnetRm_TxChObj *txChObj = &hRm->txObj;
    uint32_t txChOffset = txChNum - txChObj->txPSILThreadIdOffset;
    int32_t status = ENET_SOK;
    uint32_t numTableEntries = hRm->txObj.resCnt;

    /* If application has allocated absolute tx channels, then numTableEntries should be ENET_CFG_RM_TX_CH_MAX
     * As application might allocate a tx channel with id > the resource count, hence in that case
     * we might not be going to the full resTable to find the resource
     * For example - Application allocated 1 tx channel of priority 7, then in that case
     * restTable[7] will have the correct entry and numTableEnries will be 1 */
    if (hRm->cfg.resPartInfo.isStaticTxChanAllocated == BTRUE)
    {
        numTableEntries = ENET_CFG_RM_TX_CH_MAX;
    }

    status = EnetRm_validateCoreKey(hRm, coreKey);

    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        Enet_assert(hRm->txObj.resCnt <= ENET_ARRAYSIZE(hRm->txObj.txRes));

        status = EnetRm_freeResource(&hRm->txObj.txResTbl,
                                     hRm->txObj.txRes,
                                     numTableEntries,
                                     coreId,
                                     txChOffset);
    }

    return status;
}

static int32_t EnetRm_freeMacAddr(EnetRm_Handle hRm,
                                  uint32_t coreKey,
                                  uint8_t macAddr[])
{
    int32_t status = ENET_SOK;

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);
        uint32_t macIdx;

        status = EnetRm_mapMacAddr2ResourceIdx(&hRm->cfg.macList, macAddr, &macIdx);
        if (status == ENET_SOK)
        {
            Enet_assert(hRm->macObj.resCnt <= ENET_ARRAYSIZE(hRm->macObj.macRes));
            status = EnetRm_freeResource(&hRm->macObj.macTbl,
                                         hRm->macObj.macRes,
                                         hRm->macObj.resCnt,
                                         coreId,
                                         macIdx);
        }
    }

    return status;
}

static int32_t EnetRm_freeHwPushInst(EnetRm_Handle hRm,
                                    uint32_t coreKey,
                                    uint32_t hwPushInst)
{
    int32_t status = ENET_SOK;

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);

        Enet_assert(hRm->hwPushObj.resCnt <= ENET_ARRAYSIZE(hRm->hwPushObj.hwPushRes));

        status = EnetRm_freeResource(&hRm->hwPushObj.hwPushResTbl,
                                        hRm->hwPushObj.hwPushRes,
                                        hRm->hwPushObj.resCnt,
                                        coreId,
                                        hwPushInst);
    }

    return status;
}

static int32_t EnetRm_validateCoreIoctlPrivilege(EnetRm_Obj *hRm,
                                                 uint32_t cmd,
                                                 uint32_t coreId)
{
    bool ioctlPermitted;
    uint32_t i;
    int32_t status;

    for (i = 0U; i < hRm->cfg.ioctlPermissionInfo.numEntries; i++)
    {
        if (cmd == hRm->cfg.ioctlPermissionInfo.entry[i].cmd)
        {
            break;
        }
    }

    if (i < hRm->cfg.ioctlPermissionInfo.numEntries)
    {
        ioctlPermitted = ENET_IS_BIT_SET(hRm->cfg.ioctlPermissionInfo.entry[i].permittedCoreMask, coreId);
    }
    else
    {
        ioctlPermitted = ENET_IS_BIT_SET(hRm->cfg.ioctlPermissionInfo.defaultPermittedCoreMask, coreId);
    }

    status = ioctlPermitted ? ENET_SOK : ENET_EPERM;

    return status;
}

static int32_t EnetRm_attachCore(EnetRm_Handle hRm,
                                 uint32_t coreId,
                                 uint32_t *coreKey)
{
    bool coreAttached;
    int32_t status = ENET_SOK;

    coreAttached = EnetRm_isCoreAttached(&hRm->coreAttachObj, coreId, (uint32_t *)NULL_PTR);
    if (coreAttached == BFALSE)
    {
        EnetRm_addCoreAttached(&hRm->coreAttachObj, coreId);
        *coreKey = ENET_COREID_2_COREKEY(coreId);
    }
    else
    {
        status = ENET_EALREADYOPEN;
    }

    return status;
}

static int32_t EnetRm_detachCore(EnetRm_Handle hRm,
                                 uint32_t coreKey)
{
    uint32_t i;
    int32_t status;
    uint32_t numTableEntries = hRm->txObj.resCnt;

    status = EnetRm_validateCoreKey(hRm, coreKey);
    if (status == ENET_SOK)
    {
        uint32_t coreId = ENET_COREKEY_2_COREID(coreKey);
        bool coreAttached;

        coreAttached = EnetRm_isCoreAttached(&hRm->coreAttachObj, coreId, (uint32_t *)NULL_PTR);
        if (coreAttached == BTRUE)
        {
            EnetRm_ResourceInfo *resInfo = EnetRm_getCoreResourceInfo(&hRm->cfg, coreId);
            EnetQ *txFreeQ  = EnetRm_getFreeQ(&hRm->txObj.txResTbl, coreId);
            EnetQ *macFreeQ = EnetRm_getFreeQ(&hRm->macObj.macTbl, coreId);
            uint32_t internalRxAllocCnt;

            if (resInfo == NULL)
            {
                Enet_assert(BFALSE);
            }
            else
            {
                /* If application has allocated absolute tx channels, then numTableEntries should be ENET_CFG_RM_TX_CH_MAX
                 * As application might allocate a tx channel with id > the resource count, hence in that case
                 * we might not be going to the full resTable to find the resource
                 * For example - Application allocated 1 tx channel of priority 7, then in that case
                 * restTable[7] will have the correct entry and numTableEnries will be 1 */
                if (hRm->cfg.resPartInfo.isStaticTxChanAllocated == BTRUE)
                {
                    numTableEntries = ENET_CFG_RM_TX_CH_MAX;
                }

                Enet_assert(hRm->txObj.resCnt <= ENET_ARRAYSIZE(hRm->txObj.txRes));
                EnetRm_freeCoreResource(&hRm->txObj.txResTbl,
                                        hRm->txObj.txRes,
                                        numTableEntries,
                                        coreId);
                if (NULL != txFreeQ)
                {
                    Enet_assert(EnetQueue_getQCount(txFreeQ) == resInfo->numTxCh);
                }

                for (i = 0U; i < hRm->numRxCh; i++)
                {
                    EnetQ *rxFreeQ  = EnetRm_getFreeQ(&hRm->rxObj[i].rxResTbl, coreId);

                    Enet_assert(hRm->rxObj[i].resCnt <= ENET_ARRAYSIZE(hRm->rxObj[i].rxRes));
                    EnetRm_freeCoreResource(&hRm->rxObj[i].rxResTbl,
                                            hRm->rxObj[i].rxRes,
                                            hRm->rxObj[i].resCnt,
                                            coreId);
                    internalRxAllocCnt = 0;
                    if (coreId == hRm->rxObj[i].internalAllocCoreId)
                    {
                        internalRxAllocCnt = hRm->rxObj[i].internalAllocCount;
                    }

                    if (NULL != rxFreeQ)
                    {
                        Enet_assert((EnetQueue_getQCount(rxFreeQ) + internalRxAllocCnt) == resInfo->numRxFlows);
                    }
                }

                Enet_assert(hRm->macObj.resCnt <= ENET_ARRAYSIZE(hRm->macObj.macRes));
                EnetRm_freeCoreResource(&hRm->macObj.macTbl,
                                        hRm->macObj.macRes,
                                        hRm->macObj.resCnt,
                                        coreId);
                if (NULL != macFreeQ)
                {
                    Enet_assert(EnetQueue_getQCount(macFreeQ) == resInfo->numMacAddress);
                }

                EnetRm_delCoreAttached(&hRm->coreAttachObj, coreId);
            }
        }
        else
        {
            status = ENET_EINVALIDPARAMS;
        }
    }

    return status;
}
