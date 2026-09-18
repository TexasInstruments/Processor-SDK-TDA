/******************************************************************************
 * Copyright (c) 2024 Texas Instruments Incorporated - http://www.ti.com
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
 *
 *****************************************************************************/
#include <string.h>
#include "board_ddr.h"
#include "board_ddrRegInit.h"

#if defined(BOARD_ENABLE_DDR_REG_VERIFY)
#include "board_ddrRegVerify.h"
#endif

Board_DdrObject_t gBoardDdrObject[BOARD_DDR_INSTANCE_MAX];

/**
 * \brief   Set DDR PLL clock value
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRSetPLLClock(uint32_t ddrInstance, uint64_t frequency)
{
    Board_STATUS status = BOARD_SOK;

    switch (ddrInstance)
    {
         case BOARD_DDR_INSTANCE_0:
            status = Board_PLLInit(TISCI_DEV_DDR0,
                                   TISCI_DEV_DDR0_DDRSS_DDR_PLL_CLK,
                                   frequency);
            break;

         case BOARD_DDR_INSTANCE_1:
            status = Board_PLLInit(TISCI_DEV_DDR1,
                                   TISCI_DEV_DDR1_DDRSS_DDR_PLL_CLK,
                                   frequency);
            break;

         default:
            status = BOARD_INVALID_PARAM;
            break;

    }

    if(status != BOARD_SOK)
    {
        BOARD_DEBUG_LOG("Failed to Set the DDR PLL Clock Frequency\n");
    }

    return status;
}

/**
 * \brief   Controls the DDR PLL clock change sequence during inits
 *
 * \return  None
 */
static void Board_DDRChangeFreqAck(uint32_t ddrInstance)
{
    uint32_t reqType;
    uint32_t regVal;
    volatile uint32_t counter;
    volatile uint32_t temp = 0;

    temp = temp;  /* To suppress compiler warning */
    BOARD_DEBUG_LOG("--->>> LPDDR4 Initialization is in progress ... <<<---\n");

    for(counter = 0; counter < DDRSS_PLL_FHS_CNT; counter++)
    {
        /* wait for freq change request */
        regVal = HW_RD_REG32((BOARD_DDR_FSP_CLKCHNG_REQ_ADDR + (0x10 * ddrInstance))) & 0x80;
        BOARD_DEBUG_LOG("Reg Value: %d \n", regVal);

        while(regVal == 0x0)
        {
            regVal = HW_RD_REG32((BOARD_DDR_FSP_CLKCHNG_REQ_ADDR + (0x10 * ddrInstance))) & 0x80;
            BOARD_DEBUG_LOG("Reg Value: %d \n", regVal);
        }

        reqType = HW_RD_REG32((BOARD_DDR_FSP_CLKCHNG_REQ_ADDR + (0x10 * ddrInstance))) & 0x03;
        BOARD_DEBUG_LOG("Frequency Change type %d request from Controller \n", reqType);

        if(reqType == 1)
        {
            Board_DDRSetPLLClock(ddrInstance, DDRSS_PLL_FREQUENCY_1);
        }
        else if(reqType == 2)
        {
            Board_DDRSetPLLClock(ddrInstance, DDRSS_PLL_FREQUENCY_2);
        }
        else if(reqType == 0)
        {
            Board_DDRSetPLLClock(ddrInstance, DDRSS_PLL_FREQUENCY_0);
        }
        else
        {
            BOARD_DEBUG_LOG("Invalid Request Type\n");
        }

        /* Acknowledge frequency change request */
        HW_WR_REG32((BOARD_DDR_FSP_CLKCHNG_ACK_ADDR + (0x10 * ddrInstance)), 0x1);

        while((HW_RD_REG32((BOARD_DDR_FSP_CLKCHNG_REQ_ADDR + (0x10 * ddrInstance))) & 0x80) == 0x80);

        /* Clear frequency change request acknowledge */
        HW_WR_REG32((BOARD_DDR_FSP_CLKCHNG_ACK_ADDR + (0x10 * ddrInstance)), 0x0);
    }

    BOARD_DEBUG_LOG("--->>> Frequency Change request handshake is completed... <<<---\n");
}

/**
 * \brief   Function to handle the configuration requests from DDR lib
 *
 * \return  None
 */
static void Board_DDRInfoHandler(const LPDDR4_PrivateData *pd, LPDDR4_InfoType infotype)
{
    Board_DdrHandle ddrHandle;

    if (infotype == LPDDR4_DRV_SOC_PLL_UPDATE)
    {
        ddrHandle = (Board_DdrHandle)pd->ddr_instance;
        Board_DDRChangeFreqAck(ddrHandle->ddrInst);
    }
}

/**
 * \brief   DDR probe function
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRProbe(Board_DdrHandle ddrHandle)
{
    uint32_t status = 0U;
    uint16_t configsize = 0U;

    status = LPDDR4_Probe(&ddrHandle->boardDdrCfg, &configsize);

    if ((status != 0) || (configsize != sizeof(LPDDR4_PrivateData)) ||
        (configsize > BOARD_DDR_SRAM_MAX))
    {
        BOARD_DEBUG_LOG("Board_DDRProbe: FAIL\n");
        return BOARD_FAIL;
    }
    else
    {
        BOARD_DEBUG_LOG("Board_DDRProbe: PASS\n");
    }

    return BOARD_SOK;
}

/**
 * \brief   DDR driver initialization function
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRInitDrv(Board_DdrHandle ddrHandle)
{
    uint32_t status = 0U;
    LPDDR4_Config *pBoardDdrCfg;
    LPDDR4_PrivateData *pBoardDdrPd;

    if(ddrHandle == NULL)
    {
        BOARD_DEBUG_LOG("Board_DDRInitDrv: FAIL\n");
        return BOARD_FAIL;
    }

    pBoardDdrCfg = &ddrHandle->boardDdrCfg;
    pBoardDdrPd  = &ddrHandle->boardDdrPd;

    if ((sizeof(ddrHandle->boardDdrPd) != sizeof(LPDDR4_PrivateData)) ||
        (sizeof(ddrHandle->boardDdrPd) > BOARD_DDR_SRAM_MAX))
    {
        BOARD_DEBUG_LOG("Board_DDRInitDrv: FAIL\n");
        return BOARD_FAIL;
    }

    pBoardDdrCfg->ctlBase = (struct LPDDR4_CtlRegs_s *)ddrHandle->ddrCtlAddr;
    pBoardDdrCfg->infoHandler = (LPDDR4_InfoCallback) Board_DDRInfoHandler;

    status = LPDDR4_Init(pBoardDdrPd, pBoardDdrCfg);

    if ((status > 0U) ||
        (pBoardDdrPd->ctlBase != (struct LPDDR4_CtlRegs_s *)pBoardDdrCfg->ctlBase) ||
        (pBoardDdrPd->ctlInterruptHandler != pBoardDdrCfg->ctlInterruptHandler) ||
        (pBoardDdrPd->phyIndepInterruptHandler != pBoardDdrCfg->phyIndepInterruptHandler))
    {
        BOARD_DEBUG_LOG("Board_DDRInitDrv: FAIL\n");
        return BOARD_FAIL;
    }
    else
    {
        BOARD_DEBUG_LOG("Board_DDRInitDrv: PASS\n");
    }

    return BOARD_SOK;
}

/**
 * \brief   DDR registers initialization function
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRHWRegInit(Board_DdrHandle ddrHandle)
{
    uint32_t status = 0U;

    status = LPDDR4_WriteCtlConfig(&ddrHandle->boardDdrPd,
                                   ddrHandle->ddrCtlReg,
                                   DDRSS_ctlRegNum,
                                   (uint16_t)DDRSS_CTL_REG_INIT_COUNT);
    if (!status)
    {
        status = LPDDR4_WritePhyIndepConfig(&ddrHandle->boardDdrPd,
                                            ddrHandle->ddrPhyIndepReg,
                                            DDRSS_phyIndepRegNum,
                                            (uint16_t)DDRSS_PHY_INDEP_REG_INIT_COUNT);
    }

    if (!status)
    {
        status = LPDDR4_WritePhyConfig(&ddrHandle->boardDdrPd,
                                       ddrHandle->ddrPhyReg,
                                       DDRSS_phyRegNum,
                                       (uint16_t)DDRSS_PHY_REG_INIT_COUNT);
    }

    if (status)
    {
        BOARD_DEBUG_LOG(" ERROR: Board_DDRHWRegInit failed!!\n");
        return BOARD_FAIL;
    }

    return BOARD_SOK;
}

#if defined(BOARD_ENABLE_DDR_REG_VERIFY)
/**
 * \brief   Verifies the DDR register values configured during init
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRHWRegVerify(Board_DdrHandle ddrHandle)
{
    uint32_t status = 0U;

    status = LPDDR4_DeferredRegVerify(&ddrHandle->boardDdrPd,
                                      LPDDR4_CTL_REGS,
                                      ddrHandle->ddrCtlRegRef,
                                      DDRSS_ctlRegNumRef,
                                      (uint16_t)DDRSS_CTL_REG_VERIFY_COUNT);
    if (status)
    {
        BOARD_DEBUG_LOG(" ERROR: DDR Control Register Check Failed at Index %d!!\n", index);
        return BOARD_DDR_CTL_REG_CHECK_FAIL;

    }

    status = LPDDR4_DeferredRegVerify(&ddrHandle->boardDdrPd,
                                      LPDDR4_PHY_INDEP_REGS,
                                      ddrHandle->ddrPhyIndepRegRef,
                                      DDRSS_phyIndepRegNumRef,
                                      (uint16_t)DDRSS_PHY_INDEP_REG_VERIFY_COUNT);
    if (status)
    {
        BOARD_DEBUG_LOG(" ERROR: DDR PHY INDEP Register Check Failed at Index %d!!\n", index);
        return BOARD_DDR_PHYINDEP_REG_CHECK_FAIL;

    }

    status = LPDDR4_DeferredRegVerify(&ddrHandle->boardDdrPd,
                                      LPDDR4_PHY_REGS,
                                      ddrHandle->ddrPhyRegRef,
                                      DDRSS_phyRegNumRef,
                                      (uint16_t)DDRSS_PHY_REG_VERIFY_COUNT);

    if (status)
    {
        BOARD_DEBUG_LOG(" ERROR: DDR PHY Register Check Failed at Index %d!!\n", index);
        return BOARD_DDR_PHY_REG_CHECK_FAIL;
    }

    if (status)
    {
        BOARD_DEBUG_LOG(" ERROR: Board_DDRHWRegVerify failed!!\n");
        return BOARD_FAIL;
    }

    return BOARD_SOK;
}
#endif //#if defined(BOARD_ENABLE_DDR_REG_VERIFY)

/**
 * \brief   DDR start function
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 */
static Board_STATUS Board_DDRStart(Board_DdrHandle ddrHandle)
{
    uint32_t status = 0U;
    uint32_t regval = 0U;
    uint32_t offset = 0U;
    LPDDR4_PrivateData *pBoardDdrPd;

    if(ddrHandle == NULL)
    {
        BOARD_DEBUG_LOG("Board_DDRStart: FAIL\n");
        return BOARD_FAIL;
    }

    pBoardDdrPd = &ddrHandle->boardDdrPd;
    offset      = BOARD_DDR_CTL_REG_OFFSET;

    status = LPDDR4_ReadReg(pBoardDdrPd, LPDDR4_CTL_REGS, offset, &regval);
    if ((status > 0U) || ((regval & 0x1U) != 0U))
    {
        BOARD_DEBUG_LOG("Board_DDRStart: FAIL\n");
        return BOARD_FAIL;
    }

    status = LPDDR4_Start(pBoardDdrPd);
    if (status > 0U)
    {
        BOARD_DEBUG_LOG("Board_DDRStart: FAIL\n");
        return BOARD_FAIL;
    }

    status = LPDDR4_ReadReg(pBoardDdrPd, LPDDR4_CTL_REGS, offset, &regval);
    if ((status > 0U) || ((regval & 0x1U) != 1U))
    {
        BOARD_DEBUG_LOG("Board_DDRStart: FAIL\n");
        return BOARD_FAIL;
    }
    else
    {
        BOARD_DEBUG_LOG("LPDDR4_Start: PASS\n");
    }

    return BOARD_SOK;
}

/**
 * \brief Configures DDR ECC
 *
 * Invokes EMIF CSL APIs to configure ECC and Primes the memory
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
/* Refer EMIF ECC Configuration Section in TRM */
Board_STATUS emif_ConfigureECC(Board_DdrHandle ddrHandle)
{
    Board_STATUS   status    = BOARD_SOK;
    int32_t        cslResult = CSL_PASS;
    CSL_EmifConfig emifCfg;

    BOARD_DEBUG_LOG("\r\n Configuring ECC");

    memset(&emifCfg, 0, sizeof(emifCfg));

    emifCfg.bEnableMemoryECC = true;
    emifCfg.bReadModifyWriteEnable = true;
    emifCfg.bECCCheck = true;
    emifCfg.bWriteAlloc = true;
    emifCfg.ECCThreshold = 1U;
    emifCfg.pMemEccCfg.startAddr[0] = BOARD_DDR_START_ADDR-BOARD_DDR_START_ADDR;
    emifCfg.pMemEccCfg.endAddr[0] = BOARD_DDR_ECC_END_ADDR-BOARD_DDR_START_ADDR;

    cslResult = CSL_emifConfig((CSL_emif_sscfgRegs *)ddrHandle->eccAddr,
                               &emifCfg);
    if ( cslResult == BOARD_SOK )
    {
        /* Clears ECC errors */
        CSL_emifClearAllECCErrors((CSL_emif_sscfgRegs *)ddrHandle->eccAddr);
    }

    return status;
}

/**
 * \brief DDR4 Configuration function
 *
 * Invokes DDR CSL APIs to configure the DDR timing parameters and ECC configuration
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
static Board_STATUS Board_DDRConfig(Board_DdrHandle ddrHandle, Bool eccEnable)
{
    Board_STATUS status = BOARD_SOK;

    status = Board_DDRProbe(ddrHandle);
    if(status != BOARD_SOK)
    {
        return status;
    }

    status = Board_DDRInitDrv(ddrHandle);
    if(status != BOARD_SOK)
    {
        return status;
    }

    status = Board_DDRHWRegInit(ddrHandle);
    if(status != BOARD_SOK)
    {
        return status;
    }

    status = Board_DDRStart(ddrHandle);
    if(status != BOARD_SOK)
    {
        return status;
    }

    return status;
}

/**
 * \brief Opens and initializes a DDR instances
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
static Board_DdrHandle Board_DDROpen(uint32_t ddrInstance)
{
    Board_DdrHandle ddrHandle = NULL;

    if(ddrInstance < BOARD_DDR_INSTANCE_MAX)
    {
        if(gBoardDdrObject[ddrInstance].isOpen == FALSE)
        {
            ddrHandle                          = &gBoardDdrObject[ddrInstance];
            ddrHandle->isOpen                  = TRUE;
            ddrHandle->boardDdrPd.ddr_instance = ddrHandle;
            ddrHandle->ddrInst                 = ddrInstance;

#if defined(BOARD_ENABLE_DDR_REG_VERIFY)
            ddrHandle->ddrCtlRegRef            = DDRSS_ctlRegRef;
            ddrHandle->ddrPhyIndepRegRef       = DDRSS_phyIndepRegRef;
            ddrHandle->ddrPhyRegRef            = DDRSS_phyRegRef;
#endif

            switch (ddrInstance)
            {
                 case BOARD_DDR_INSTANCE_0:
                    ddrHandle->ddrCtlAddr     = (void *)BOARD_DDR0_CTL_CFG_BASE;
                    ddrHandle->ddrCtlReg      = DDRSS0_ctlReg;
                    ddrHandle->ddrPhyIndepReg = DDRSS0_phyIndepReg;
                    ddrHandle->ddrPhyReg      = DDRSS0_phyReg;
                    ddrHandle->eccAddr        = (void *)CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS0_SSCFG_BASE;
                    break;
                 case BOARD_DDR_INSTANCE_1:
                    ddrHandle->ddrCtlAddr     = (void *)BOARD_DDR1_CTL_CFG_BASE;
                    ddrHandle->ddrCtlReg      = DDRSS1_ctlReg;
                    ddrHandle->ddrPhyIndepReg = DDRSS1_phyIndepReg;
                    ddrHandle->ddrPhyReg      = DDRSS1_phyReg;
                    ddrHandle->eccAddr        = (void *)CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS1_SSCFG_BASE;
                    break;
            }
        }
    }

    return ddrHandle;
}

/**
 * \brief Closes DDR instances
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
static Board_STATUS Board_DDRClose(Board_DdrHandle ddrHandle)
{
    Board_STATUS status = BOARD_FAIL;

    if(ddrHandle != NULL)
    {
        ddrHandle->isOpen = FALSE;
        status = BOARD_SOK;
    }

    return status;
}

#if defined(BOARD_ENABLE_DDR_REG_VERIFY)
/**
 * \brief DDR4 register value verification function
 *
 * Reads the DDR registers configured and compares with expected values.
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
Board_STATUS Board_DDRRegVerify(void)
{
    Board_STATUS status = BOARD_SOK;
    uint32_t ddrInstance;
    Board_DdrHandle ddrHandle;

    for(ddrInstance = 0; ddrInstance < BOARD_DDR_INSTANCE_MAX; ddrInstance++)
    {
        /* Check the registers for a DDR instance only if it is set active in EMIF tool generated config */
        if(MULTI_DDR_CFG_EMIFS_ACTIVE & (1 << ddrInstance))
        {
            ddrHandle = Board_DDROpen(ddrInstance);
            if(ddrHandle == NULL)
            {
                BOARD_DEBUG_LOG("Board_DDROpen: FAIL\n");
                return BOARD_FAIL;
            }

            status = Board_DDRProbe(ddrHandle);
            if(status != BOARD_SOK)
            {
                return status;
            }

            status = Board_DDRInitDrv(ddrHandle);
            if(status != BOARD_SOK)
            {
                return status;
            }

            status = Board_DDRHWRegVerify(ddrHandle);
            if(status != BOARD_SOK)
            {
                BOARD_DEBUG_LOG("Board_DDRHWRegVerify: FAIL\n");
                return status;
            }

            Board_DDRClose(ddrHandle);
        }
    }

    return status;
}
#endif //#if defined(BOARD_ENABLE_DDR_REG_VERIFY)

/**
 * \brief DDR4 Initialization function
 *
 * \return  BOARD_SOK in case of success or appropriate error code
 *
 */
Board_STATUS Board_DDRInit(Bool eccEnable)
{
    Board_STATUS status = BOARD_SOK;
    uint32_t multiDdrCfgVal0;
    uint32_t multiDdrCfgVal1;
    uint32_t ddrInstance;
    Board_DdrHandle ddrHandle;

    /* Unlock the PLL register access for DDR clock bypass */
    HW_WR_REG32(BOARD_PLL12_LOCK0, KICK0_UNLOCK);
    HW_WR_REG32(BOARD_PLL12_LOCK1, KICK1_UNLOCK);

    HW_WR_REG32(BOARD_DDR_LOCK0, KICK0_UNLOCK);
    HW_WR_REG32(BOARD_DDR_LOCK1, KICK1_UNLOCK);

    multiDdrCfgVal0 = (MULTI_DDR_CFG_INTRLV_GRAN      << 24) |
                      (MULTI_DDR_CFG_INTRLV_SIZE      << 16) |
                      (BOARD_MULTI_DDR_CFG_INTRLV_HEARTBEAT <<  0);

    multiDdrCfgVal1 = (MULTI_DDR_CFG_ECC_ENABLE      << 16) |
                      (MULTI_DDR_CFG_HYBRID_SELECT   <<  8) |
                      (MULTI_DDR_CFG_EMIFS_ACTIVE    <<  0);

    HW_WR_REG32(BOARD_MULTI_DDR_CFG0, multiDdrCfgVal0); 
    HW_WR_REG32(BOARD_MULTI_DDR_CFG1, multiDdrCfgVal1);
    HW_WR_REG32(BOARD_DDR_CFG_LOAD,   BOARD_DDR_CFG_LOAD_VALUE);

    /* Wait until DDR config load is complete */
    while(HW_RD_REG32(BOARD_DDR_CFG_LOAD & 0x1) == 0x0);

    /* Partition5 lockkey0 */
    HW_WR_REG32(BOARD_CTRL_MMR_PART5_LOCK0, KICK0_UNLOCK);
    /* Partition5 lockkey1 */
    HW_WR_REG32(BOARD_CTRL_MMR_PART5_LOCK1, KICK1_UNLOCK);

    for(ddrInstance = BOARD_DDR_INSTANCE_START; ddrInstance <= BOARD_DDR_INSTANCE_END; ddrInstance++)
    {
        /* Set to Boot Frequency(F0) while configuring the DDR */
        Board_DDRSetPLLClock(ddrInstance, DDRSS_PLL_FREQUENCY_0);
        ddrHandle = Board_DDROpen(ddrInstance);
        if(ddrHandle == NULL)
        {
            BOARD_DEBUG_LOG("Board_DDROpen: FAIL\n");
            return BOARD_FAIL;
        }

        status = Board_DDRConfig(ddrHandle, 0);
        if(status != BOARD_SOK)
        {
            BOARD_DEBUG_LOG("Board_DDRConfig: FAIL\n");
            return status;
        }

        if (eccEnable == TRUE)
        {
            status = emif_ConfigureECC(ddrHandle);
            if ( status != BOARD_SOK )
            {
                BOARD_DEBUG_LOG("\r\n CSL_emifConfig Failed");
                return status;
            }
        }
        Board_DDRClose(ddrHandle);
    }

    if (eccEnable == TRUE)
    {
#ifdef BOARD_DDR_ENABLE_DDR_MEM_PRIME
        status = BOARD_udmaPrimeDDR((void *)BOARD_DDR_START_ADDR, BOARD_DDR_ECC_END_ADDR-BOARD_DDR_START_ADDR+1U);
        if ( status != BOARD_SOK )
        {
            BOARD_DEBUG_LOG("\r\n UDMAPrime Failed");
            return status;
        }
#endif
    }

    /* Lock the register access */
    HW_WR_REG32(BOARD_PLL12_LOCK0, KICK_LOCK);
    HW_WR_REG32(BOARD_PLL12_LOCK1, KICK_LOCK);
    HW_WR_REG32(BOARD_DDR_LOCK0, KICK_LOCK);
    HW_WR_REG32(BOARD_DDR_LOCK1, KICK_LOCK);
    HW_WR_REG32(BOARD_CTRL_MMR_PART5_LOCK0, KICK_LOCK);
    HW_WR_REG32(BOARD_CTRL_MMR_PART5_LOCK1, KICK_LOCK);

    return status;
}
