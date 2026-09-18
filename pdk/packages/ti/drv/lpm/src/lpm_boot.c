/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file     lpm_boot.c
 *
 *  \brief    This file implements booting of main domain core applications
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/lpm/include/lpm_boot.h>
#include <ti/boot/sbl/src/rprc/sbl_rprc_parse.h>
#include <ti/boot/sbl/src/ospi/sbl_ospi.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

#define TSICR_POST		(0x1 << 2)

#define MCU_TIMER0_BASE CSL_MCU_TIMER0_CFG_BASE
#define MCU_TIMER1_BASE CSL_MCU_TIMER1_CFG_BASE
#define MCU_TIMER2_BASE CSL_MCU_TIMER2_CFG_BASE
#define MCU_TIMER3_BASE CSL_MCU_TIMER3_CFG_BASE

#define MCU_PROFILING_TIMER_BASE MCU_TIMER2_BASE

/* Clocks */
#define CLK_12M_RC_FREQ           (12500000ULL)
#define CLK_200M_RC_DEFAULT_FREQ (200000000ULL)  /* Timer 2 clock frequency set by default to 200MHz */

#define MCU_TIMER_PTV       2
#define MCU_TIMER_CLOCK     (CLK_200M_RC_DEFAULT_FREQ / (2 << MCU_TIMER_PTV))
#define MCU_TIMER_LOAD_VAL	      0

#define writel(x,y)               (*((uint64_t *)(y))=(x))
#define readl(x)                  (*((uint64_t *)(x)))

#define LPM_SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0 (0x01U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Function Declarations                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Offset into app image that is being processed */
static uint32_t xipMemBase = 0x50000000;
static OSPI_v0_HwAttrs ospi_cfg;
static void *boardHandle = NULL;

sblEntryPoint_t k3xx_evmEntry;
Lpm_SblIncomingBootData gLpmSblInBootData __attribute__ ((section (".sblbootbuff")));

/* ========================================================================== */
/*                            External Variables                              */
/* ========================================================================== */

/* SBL functions required for standard boot */
extern uint32_t SBL_IsSysfwEnc(uint8_t *x509_cert_ptr);
extern int32_t  SBL_FileRead(void  *buff, void *fileptr, uint32_t size);
extern void     SBL_FileSeek(void *fileptr, uint32_t location);
extern void     SBL_DCacheClean(void *addr, uint32_t size);
/* Function Pointer used while reading data from the storage. */
extern int32_t  (*fp_readData)(void *dstAddr, void *srcAddr, uint32_t length);
extern void     (*fp_seek)(void *srcAddr, uint32_t location);

/* SBL structures required for standard boot */
/* FATFS Handle */
extern FATFS_Handle  sbl_fatfsHandle;
extern FATFS_HwAttrs FATFS_initCfg[_VOLUMES];

/* Functions required for secure boot */
extern int32_t  SBL_loadMMCSDBootFile(FIL * fp);
extern uint32_t SBL_authentication(void *addr);
extern uint32_t SBL_startSK(void);
extern int32_t  SBL_MemRead(void *buff, void *srcAddr, uint32_t size);
extern void     SBL_MemSeek(void *srcAddr, uint32_t location);
extern int32_t  SBL_loadOSPIBootData(void);

/* Structured needed for performing SBL logging */
extern sblProfileInfo_t *sblProfileLogAddr;
extern uint32_t         *sblProfileLogIndxAddr;
extern uint32_t         *sblProfileLogOvrFlwAddr;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*        This function uses the Global Timebase Counter (GTC)
 *        Important: the GTC clock is different than the ARM PMU clock.
 *                Function assumes input clock has already been set.
 *                Make sure to check freq. used for SoC!
 *  Note:
 *        For Jacinto 7 SoCs this register requires a SECURE WRITE so it only works
 *        if A53/A72 is in EL3 state.
 */
static void Lpm_initGTC(void)
{
    CSL_REG32_WR(CSL_GTC0_GTC_CFG1_BASE + CSL_GTC_CFG1_CNTCR, 0x1);
    CSL_REG32_WR(CSL_GTC0_GTC_CFG1_BASE + CSL_GTC_CFG1_CNTFID0, 200000000);
}

#if defined(MPU1_HLOS_BOOT_ENABLED) || defined(MPU1_HLOS_BOOT_ONLY_ENABLED)
/* Function to clean the MCU R5 cache for a given start address and given memory size */
void Lpm_mcuDCacheClean(void *addr, uint32_t size)
{
    /* Invalidate by MVA */
    CSL_armR5CacheWbInv((const void *)addr, uint32_to_int32(size), (bool)TRUE);
}
#endif

#    if defined(PROFILE_OSPI_READS_ENABLED)
uint64 total_memcpy_time = 0;
uint64 total_memcpy_size = 0;
#    endif

/* read of block of data from buffer */
static int32_t Lpm_xipReadMem(void    *buff,
                              void    *srcOffsetAddr,
                              uint32_t size)
{
#    if defined(PROFILE_OSPI_READS_ENABLED)
    uint64 memcpy_start;
    uint64 memcpy_end;
#    endif

    uint32_t imgOffset;
    int32_t  retVal = 0;

    imgOffset = *((uint32_t*)srcOffsetAddr);

#    if defined(PROFILE_OSPI_READS_ENABLED)
    memcpy_start = get_usec_timestamp();
#    endif

    memcpy(buff, (void*)((uint8_t*)xipMemBase + imgOffset), size);

#    if defined(PROFILE_OSPI_READS_ENABLED)
    memcpy_end = get_usec_timestamp();

    total_memcpy_time += memcpy_end - memcpy_start;
    total_memcpy_size += size;
#    endif

    /* Advance srcOffsetAddr for the next section to be read */
    *((uint32_t *) srcOffsetAddr) += size;

    return retVal;
}

/* move the buffer pointer */
static void Lpm_xipSeekMem(void *srcAddr, uint32_t location)
{
    *((uint32_t *) srcAddr) = location;
}

static int32_t Lpm_ospiBootImageLate(sblEntryPoint_t *pEntry, uint32_t imageOffset)
{
    int32_t retVal = E_FAIL;

#    ifdef SECURE_BOOT
    uint32_t authenticated = 0;
    uint32_t srcAddr = 0;
    uint32_t imgOffset = 0;
#    else
    uint32_t offset = 0;
#    endif

#    ifndef SECURE_BOOT
    /* Load the MAIN domain remotecore images included in the appimage */
    offset = imageOffset;

    #if defined(ENABLE_DMA)
    fp_readData = &SBL_OSPI_ReadSectors;
    fp_seek     = &SBL_OSPI_seek;
    #else
    fp_readData = &Lpm_xipReadMem;
    fp_seek     = &Lpm_xipSeekMem;
    #endif

    retVal = SBL_MulticoreImageParse((void *) &offset,
                                      imageOffset,
                                      pEntry,
                                      SBL_SKIP_BOOT_AFTER_COPY);
#    else
    retVal = SBL_loadOSPIBootData();

    if (retVal == E_PASS)
    {
        /* authentiate it */
        authenticated = SBL_authentication(sblInBootData.sbl_boot_buff);
        if (authenticated == 0)
        {
            /* fails authentiation */
            SBL_log(SBL_LOG_ERR, "\n OSPI Boot - fail authentication\n");

            retVal = E_FAIL;
        }
        else
        {
            /* need to skip the TOC headers */
            imgOffset = ((uint32_t*)sblInBootData.sbl_boot_buff)[0];
            srcAddr = (uint32_t)(sblInBootData.sbl_boot_buff) + imgOffset;
            retVal = SBL_MulticoreImageParse((void *)srcAddr, 0, pEntry,
                                             SBL_SKIP_BOOT_AFTER_COPY);
        }
    }
    else
    {
        retVal = E_FAIL;
        SBL_log(SBL_LOG_ERR, "\n OSPI Boot - problem processing image \n");
    }
#    endif

    if (retVal != E_PASS)
        Lpm_uartDrvPrintf("Error parsing Main Domain appimage\n");

    return retVal;
}

/* Boot all the OSPI images defined in the array */

static int32_t Lpm_ospiBootStageImage(sblEntryPoint_t *pEntry, uint32_t address)
{
    int32_t status = E_FAIL;

    if (((uint32_t)NULL != address) && ((sblEntryPoint_t *)NULL != pEntry))
    {
        if (address != MAIN_DOMAIN_HLOS)
        {
#    if !defined(MPU1_HLOS_BOOT_ONLY_ENABLED)
            /* non-HLOS image */
            status = Lpm_ospiBootImageLate(&k3xx_evmEntry, address);
#    else
            status = E_PASS;
#    endif
        }
#    if defined(MPU1_HLOS_BOOT_ENABLED) || defined(MPU1_HLOS_BOOT_ONLY_ENABLED)
        else
        {
            /* Load the HLOS appimages */
            status = Lpm_ospiBootImageLate(&k3xx_evmEntry, OSPI_OFFSET_A72IMG1);
            if (status != E_PASS)
            {
                Lpm_uartDrvPrintf("Error parsing A72 appimage #1 for HLOS boot\n");
            }
            else
            {
                status = Lpm_ospiBootImageLate(&k3xx_evmEntry, OSPI_OFFSET_A72IMG2);
                if (status != E_PASS)
                {
                    Lpm_uartDrvPrintf("Error parsing A72 appimage #2 for HLOS boot\n");
                }
#        if defined(LINUX_OS)
                else
                {
                    status = Lpm_ospiBootImageLate(&k3xx_evmEntry, OSPI_OFFSET_A72IMG3);
                    if (status != E_PASS)
                        Lpm_uartDrvPrintf("Error parsing A72 appimage #3 for HLOS boot\n");
                }
#        endif
            }

            if (status == E_PASS)
            {
                /* Set the A72 entry point at the ATF address */
                pEntry->CpuEntryPoint[MPU1_CPU0_ID] = ATF_START_RAM_ADDR;
            }
        } /* if (address == MAIN_DOMAIN_HLOS) */
#    endif /* #if defined(MPU1_HLOS_BOOT_ENABLED)*/
    } /* if ((NULL != address) && (NULL != pEntry)) */

    return status;
}

static int32_t Lpm_ospiLeaveConfigSPI()
{
    int32_t retVal = E_PASS;
    Board_flashHandle h;
    Board_FlashInfo *flashInfo;

#if defined(UART_PRINT_DEBUG)
    Lpm_uartDrvPrintf("Entered OSPILeaveConfigSPI function...\n");
#endif

    /* Get default OSPI cfg */
    OSPI_socGetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    ospi_cfg.funcClk = OSPI_MODULE_CLK_133M;
    /* Configure the flash for SPI mode */
    ospi_cfg.xferLines = OSPI_XFER_LINES_SINGLE;
    /* Put controller in DAC mode so flash ID can be read directly */
    ospi_cfg.dacEnable = true;
    /* Disable PHY in legacy SPI mode (1-1-1) */
    ospi_cfg.phyEnable = false;
    ospi_cfg.dtrEnable = false;
    ospi_cfg.xipEnable = false;

    /* Set the default SPI init configurations */
    OSPI_socSetInitCfg(BOARD_OSPI_DOMAIN, BOARD_OSPI_NOR_INSTANCE, &ospi_cfg);

    h = Board_flashOpen(BOARD_FLASH_ID_MT35XU512ABA1G12,
                            BOARD_OSPI_NOR_INSTANCE, NULL);

    if (h)
    {
        Lpm_uartDrvPrintf("OSPI flash left configured in Legacy SPI mode.\n");
        flashInfo = (Board_FlashInfo *)h;
        Lpm_uartDrvPrintf("\n OSPI NOR device ID: 0x%x, manufacturer ID: 0x%x \n",
                flashInfo->device_id, flashInfo->manufacturer_id);
        Board_flashClose(h);
    }
    else
    {
        Lpm_uartDrvPrintf("Board_flashOpen failed in SPI mode!!\n");
        retVal = E_FAIL;
    }

    return(retVal);
}

#if defined(LPM_BOOTLOG_OUTPUT_ENABLED)
static void LPM_printSblProfileLog(sblProfileInfo_t *sblProfileLog,
                                              uint32_t sblProfileLogIndx,
                                              uint32_t sblProfileLogOvrFlw)
{
    uint64_t mcu_clk_freq = SBL_MCU1_CPU0_FREQ_HZ;
    uint32_t i = 0, prev_cycle_cnt = 0, cycles_per_usec;
    uint32_t lastlogIndx;
    char sbl_test_str[256];

    Sciclient_pmGetModuleClkFreq(SBL_DEV_ID_MCU1_CPU0,
                                 SBL_CLK_ID_MCU1_CPU0,
                                 &mcu_clk_freq,
                                 SCICLIENT_SERVICE_WAIT_FOREVER);
    cycles_per_usec = (mcu_clk_freq / 1000000);

    Lpm_uartDrvPrintf("\r\nProfiling info ....\r\n");
    sprintf(sbl_test_str,"MCU @ %uHz.\r\n", ((uint32_t)mcu_clk_freq));
    Lpm_uartDrvPrintf(sbl_test_str);
    sprintf(sbl_test_str,"cycles per usec  = %u\r\n", cycles_per_usec);
    Lpm_uartDrvPrintf(sbl_test_str);

    lastlogIndx = sblProfileLogIndx;

    if (sblProfileLogOvrFlw)
    {
        i = sblProfileLogIndx;
        prev_cycle_cnt = sblProfileLog[i].cycle_cnt;
        lastlogIndx = MAX_PROFILE_LOG_ENTRIES;
        Lpm_uartDrvPrintf("Detected overflow, some profile entries might be lost.\r\n");
        Lpm_uartDrvPrintf("Rebuild with a larger vlaue of MAX_PROFILE_LOG_ENTRIES ??\r\n");
    }

    while((i % MAX_PROFILE_LOG_ENTRIES) < lastlogIndx)
    {
        uint32_t cycles_to_us;

        if (sblProfileLog[i].cycle_cnt < prev_cycle_cnt)
        {
            Lpm_uartDrvPrintf("**");
        }
        else
        {
            Lpm_uartDrvPrintf("  ");
        }
        cycles_to_us = sblProfileLog[i].cycle_cnt/cycles_per_usec;
        sprintf(sbl_test_str,"fxn:%32s\t", sblProfileLog[i].fxn);
        Lpm_uartDrvPrintf(sbl_test_str);
        sprintf(sbl_test_str,"line:%4u\t", sblProfileLog[i].line);
        Lpm_uartDrvPrintf(sbl_test_str);
        sprintf(sbl_test_str,"cycle:%10u\t", sblProfileLog[i].cycle_cnt);
        Lpm_uartDrvPrintf(sbl_test_str);
        sprintf(sbl_test_str,"timestamp:%10uus\r\n", cycles_to_us);
        Lpm_uartDrvPrintf(sbl_test_str);
        prev_cycle_cnt = sblProfileLog[i].cycle_cnt;
        i++;
    }
}
#endif

static void Lpm_mainDomainBootSetup(void)
{
#    if defined(BOOTAPP_MAINDOMAIN_BOARD_SETUP)
    int32_t retVal;
#        if defined(UART_PRINT_DEBUG)
    Lpm_uartDrvPrintf("Configuring Sciclient_board for MAIN domain\n");
#        endif

    Sciclient_BoardCfgPrms_t bootAppBoardCfgPrms = {
                                                    .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfg,
                                                    .boardConfigHigh = 0,
                                                    .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfg),
                                                    .devGrp = DEVGRP_01
                                                   };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgPmPrms = {
                                                      .boardConfigLow = (uint32_t)NULL,
                                                      .boardConfigHigh = 0,
                                                      .boardConfigSize = 0,
                                                      .devGrp = DEVGRP_01
                                                     };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgRmPrms = {
                                                      .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfgRm,
                                                      .boardConfigHigh = 0,
                                                      .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfgRm),
                                                      .devGrp = DEVGRP_01
                                                     };
    Sciclient_BoardCfgPrms_t bootAppBoardCfgSecPrms = {
                                                       .boardConfigLow = (uint32_t)&gLpm_mcuOnlyBoardCfgSec,
                                                       .boardConfigHigh = 0,
                                                       .boardConfigSize = sizeof(gLpm_mcuOnlyBoardCfgSec),
                                                       .devGrp = DEVGRP_01
                                                      };

    retVal = Sciclient_boardCfg(&bootAppBoardCfgPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf("Sciclient_boardCfg() failed.\n");
    }
    retVal = Sciclient_boardCfgPm(&bootAppBoardCfgPmPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf("Sciclient_boardCfgPm() failed.\n");
    }
    retVal = Sciclient_boardCfgRm(&bootAppBoardCfgRmPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf("Sciclient_boardCfgRm() failed.\n");
    }
    retVal = Sciclient_boardCfgSec(&bootAppBoardCfgSecPrms);
    if (retVal != CSL_PASS)
    {
         Lpm_uartDrvPrintf("Sciclient_boardCfgSec() failed.\n");
    }

    Board_init(BOARD_INIT_PLL_MAIN);
    Board_init(BOARD_INIT_MODULE_CLOCK_MAIN);
    Board_init(BOARD_INIT_DDR);

    SBL_SetQoS();

#    endif /* defined(BOOTAPP_MAINDOMAIN_BOARD_SETUP) */

#    if defined(ENABLE_DMA)
    fp_readData = &SBL_OSPI_ReadSectors;
    fp_seek     = &SBL_OSPI_seek;
#    else
    fp_readData = &Lpm_xipReadMem;
    fp_seek     = &Lpm_xipSeekMem;
#    endif

    return;
}

/* Local functions common between OSPI and MMCSD functionality */
static int32_t Lpm_requestStageCores(uint8_t stageNum)
{
    uint32_t i;
    int32_t  status = CSL_EFAIL;
    uint8_t  stage  = stageNum;

    for (i = 0; i < MAX_CORES_PER_STAGE; i++)
    {
        if (sbl_late_slave_core_stages_info[stage][i].tisci_proc_id != SBL_INVALID_ID)
        {
#if defined(UART_PRINT_DEBUG)
            Lpm_uartDrvPrintf("Calling Sciclient_procBootRequestProcessor, ProcId 0x%x... \n",
                            sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#else
            SBL_log(SBL_LOG_MAX,
                    "Calling Sciclient_procBootRequestProcessor, ProcId 0x%x... \n",
                    sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#endif
            status = Sciclient_procBootRequestProcessor(sbl_late_slave_core_stages_info[stage][i].tisci_proc_id,
                                                        SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
#if defined(UART_PRINT_DEBUG)
                Lpm_uartDrvPrintf("Sciclient_procBootRequestProcessor, ProcId 0x%x...FAILED \n",
                                sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#else
                SBL_log(SBL_LOG_MAX,
                        "Sciclient_procBootRequestProcessor, ProcId 0x%x...FAILED \n",
                        sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#endif
                break;
            }
        }
    }

    return (status);
}

static int32_t Lpm_releaseStageCores(uint8_t stageNum)
{
    uint32_t i;
    int32_t  status   = CSL_EFAIL;
    uint8_t  stage  = stageNum;

    for (i = 0; i < MAX_CORES_PER_STAGE; i++)
    {
        if (sbl_late_slave_core_stages_info[stage][i].tisci_proc_id != SBL_INVALID_ID)
        {
#if defined(UART_PRINT_DEBUG)
            Lpm_uartDrvPrintf("Sciclient_procBootReleaseProcessor, ProcId 0x%x...\n",
                            sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#else
            SBL_log(SBL_LOG_MAX,
                    "Sciclient_procBootReleaseProcessor, ProcId 0x%x...\n",
                    sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#endif
            status = Sciclient_procBootReleaseProcessor(sbl_late_slave_core_stages_info[stage][i].tisci_proc_id,
                                                        TISCI_MSG_FLAG_AOP,
                                                        SCICLIENT_SERVICE_WAIT_FOREVER);
            if (status != CSL_PASS)
            {
#if defined(UART_PRINT_DEBUG)
                Lpm_uartDrvPrintf("Sciclient_procBootReleaseProcessor, ProcId 0x%x...FAILED \n",
                                sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#else
                SBL_log(SBL_LOG_MAX,
                        "Sciclient_procBootReleaseProcessor, ProcId 0x%x...FAILED \n",
                        sbl_late_slave_core_stages_info[stage][i].tisci_proc_id);
#endif
                break;
            }
        }
    }

    return (status);
}

void Lpm_bootAppInit(void)
{
    bool isNandBootEnabled = BFALSE;
#if defined(SOC_J721E)
    Board_STATUS status;

    /* Ethernet config: set proper board muxes for J712E Eth. firmware */
    /* Set IO Expander to use RMII on GESI board */
    status = Board_control(BOARD_CTRL_CMD_SET_RMII_DATA_MUX, NULL);
    if (status != BOARD_SOK)
    {
        Lpm_uartDrvPrintf("Board_control failed to configure RMII pins\n");
    }
    /* Enable CPSW9G MDIO mux */
    status = Board_control(BOARD_CTRL_CMD_SET_GESI_CPSW_MDIO_MUX, NULL);
    if (status != BOARD_SOK)
    {
        Lpm_uartDrvPrintf("Board_control failed to configure CPSW9G MDIO mux\n");
    }
#endif

    Lpm_mainDomainBootSetup();
    SBL_SPI_init();
    SBL_ospiInit(&boardHandle,isNandBootEnabled);
}

void Lpm_bootAppDeInit(void)
{
    SBL_ospiClose(&boardHandle);
    Lpm_ospiLeaveConfigSPI();
}

/* Main Boot task */
int32_t Lpm_bootApp()
{
    int32_t        retVal;
    cpu_core_id_t  core_id;
    cpu_core_id_t  booted_core_ids[DSP2_C7X_ID];
    uint8_t        i, j;
    cpu_core_id_t *boot_array;
    uint8_t        num_cores_to_boot;
    uint8_t        num_booted_cores = 0;
    uint64_t       time_boot_app_start;
#if defined(GATHER_STAGE_DETAILS)
    uint64_t       time_request_all_cores_start[NUM_BOOT_STAGES];
    uint64_t       time_request_all_cores_end[NUM_BOOT_STAGES];
    uint64_t       time_boot_image_late_start[NUM_BOOT_STAGES];
    uint64_t       time_boot_image_late_end[NUM_BOOT_STAGES];
    uint64_t       time_boot_stage_finish[NUM_BOOT_STAGES];
#endif
    uint64_t       time_boot_core_finish[DSP2_C7X_ID];

#if defined(PROFILE_OSPI_READS_ENABLED)
    uint64_t stage_memcpy_time[NUM_BOOT_STAGES];
    uint64_t stage_memcpy_size[NUM_BOOT_STAGES];
#endif

    time_boot_app_start = TimerP_getTimeInUsecs();

    /* Initialize GTC required by Cortex-Axx */
    Lpm_initGTC();

    /* Initialize the entry point array to 0. */
    for (core_id = MPU1_CPU0_ID; core_id < NUM_CORES; core_id ++)
        (&k3xx_evmEntry)->CpuEntryPoint[core_id] = SBL_INVALID_ENTRY_ADDR;

    for (j = 0; j < NUM_BOOT_STAGES; j++)
    {
#if defined(GATHER_STAGE_DETAILS)
        time_request_all_cores_start[j] = TimerP_getTimeInUsecs();
#endif

        retVal = Lpm_requestStageCores(j);

#if defined(GATHER_STAGE_DETAILS)
        time_request_all_cores_end[j] = TimerP_getTimeInUsecs();
#endif

        if (retVal != CSL_PASS)
        {
            Lpm_uartDrvPrintf("Failed to request all late cores in Stage %d\n\n",
                            j);
            Lpm_releaseStageCores(j);
        } else
        {
#if defined(UART_PRINT_DEBUG)
            Lpm_uartDrvPrintf("Loading BootImage\n");
#else
            SBL_log(SBL_LOG_MAX,
                   "Loading BootImage\n");
#endif
#if defined(GATHER_STAGE_DETAILS)
            time_boot_image_late_start[j] = TimerP_getTimeInUsecs();
#endif

#    if defined(PROFILE_OSPI_READS_ENABLED)
            /* Reset the global variable for memcpy time and size */
            total_memcpy_time = 0;
            total_memcpy_size = 0;
#    endif

            retVal = Lpm_ospiBootStageImage(&k3xx_evmEntry,
                                            ospi_main_domain_flash_rtos_images[j]);

#    if defined(PROFILE_OSPI_READS_ENABLED)
            /* Save global variable for memcpy time and size, which has been
             * incremented within OSPIBootStageImage function */
            stage_memcpy_time[j] = total_memcpy_time;
            stage_memcpy_size[j] = total_memcpy_size;
#    endif
#if defined(GATHER_STAGE_DETAILS)
            time_boot_image_late_end[j] = TimerP_getTimeInUsecs();
#endif
#if defined(UART_PRINT_DEBUG)
            Lpm_uartDrvPrintf("BootImage completed, status = %d\n",
                            retVal);
#else
            SBL_log(SBL_LOG_MAX,
                    "BootImage completed, status = %d\n",
                    retVal);
#endif

            if (retVal != CSL_PASS)
            {
                Lpm_uartDrvPrintf("Failure during image copy and parsing\n\n");
            } else
            {
                retVal = Lpm_releaseStageCores(j);
                if (retVal != CSL_PASS)
                {
                    Lpm_uartDrvPrintf("Failed to release all late cores\n\n");
                }
            }
        } /* if (retVal != CSL_PASS) */

        if (retVal == CSL_PASS)
        {
            /* Start the individual cores for the boot stage */
            num_cores_to_boot = num_cores_per_boot_stage[j];
            boot_array        = boot_array_stage[j];
            for (i = 0; i < num_cores_to_boot; i++)
            {
                core_id = boot_array[i];
                /* Try booting all cores other than the cluster running the SBL */
                if ((k3xx_evmEntry.CpuEntryPoint[core_id] != SBL_INVALID_ENTRY_ADDR) &&
                    ((core_id != MCU1_CPU1_ID) && (core_id != MCU1_CPU0_ID)))
                {
                    SBL_SlaveCoreBoot(core_id, (uint32_t)NULL, &k3xx_evmEntry, SBL_REQUEST_CORE);
                    TaskP_sleep(1*1000);
#if defined(UART_PRINT_DEBUG)
                    Lpm_uartDrvPrintf("SBL_SlaveCoreBoot completed for Core ID#%d, Entry point is 0x%x\n",
                                    core_id, k3xx_evmEntry.CpuEntryPoint[core_id]);
#endif
                    booted_core_ids[num_booted_cores] = core_id;
                    time_boot_core_finish[num_booted_cores] = TimerP_getTimeInUsecs();
                    num_booted_cores++;
                }
            }
#if defined(GATHER_STAGE_DETAILS)
            time_boot_stage_finish[j] = TimerP_getTimeInUsecs();
#endif
        } /* if (retVal == CSL_PASS) */        
        if(j == NUM_BOOT_STAGES-1)
        {
            Lpm_uartDrvPrintf("Sleeping for 20 seconds after the last stage\n");
            TaskP_sleep(20*1000);
        }
        else
        {
            Lpm_uartDrvPrintf("Sleeping for 10 seconds after each stage\n");
            TaskP_sleep(10*1000);
        }        
    } /* for (j = 0; j < NUM_BOOT_STAGES; j++) */

    /* Delay print out of boot log to avoid prints by other tasks */
    TaskP_sleep(4000);

    if (retVal == CSL_PASS)
    {
        /* Print boot log, including all gathered timestamps */
        Lpm_uartDrvPrintf("Boot App: Started at %d usec\n",
                        (uint32_t)time_boot_app_start);
#if defined(GATHER_STAGE_DETAILS)
        for (j = 0; j < NUM_BOOT_STAGES; j++)
        {
            Lpm_uartDrvPrintf("Boot App: Stage %d - Started requesting all cores at %d usec\n",
                            j, (uint32_t)time_request_all_cores_start[j]);
            Lpm_uartDrvPrintf("Boot App: Stage %d - Completed requesting all cores at %d usec\n",
                            j, (uint32_t)time_request_all_cores_end[j]);
            Lpm_uartDrvPrintf("Boot App: Stage %d - Started copying and image parsing at %d usec\n",
                            j, (uint32_t)time_boot_image_late_start[j]);
            Lpm_uartDrvPrintf("Boot App: Stage %d - Completed copying and image parsing at %d usec\n",
                            j, (uint32_t)time_boot_image_late_end[j]);
#    if defined(PROFILE_OSPI_READS_ENABLED)
            Lpm_uartDrvPrintf("Boot App: Stage %d - stage_memcpy_time = %ju, stage_memcpy_size = %ju\n",
                            j, stage_memcpy_time[j], stage_memcpy_size[j]);
#    endif
            Lpm_uartDrvPrintf("Boot App: Stage %d finished at %d usecs\n",
                            j,
                            (uint32_t)time_boot_stage_finish[j]);
        }
#endif
        Lpm_uartDrvPrintf("Boot App: Total Num booted cores = %d\n",
                        num_booted_cores);

        for (core_id = 0; core_id < num_booted_cores; core_id++)
        {
            Lpm_uartDrvPrintf("Boot App: Booted Core ID #%d at %d usecs\n",
                            booted_core_ids[core_id],
                            (uint32_t)time_boot_core_finish[core_id]);
        }
    } /* if (retVal == CSL_PASS) */
    else
    {
        Lpm_uartDrvPrintf("Boot App: Failure occurred in boot sequence\n");
    }

#if defined(LPM_BOOTLOG_OUTPUT_ENABLED)
    LPM_printSblProfileLog(sblProfileLogAddr,
                                      *sblProfileLogIndxAddr,
                                      *sblProfileLogOvrFlwAddr);
#endif

    return (retVal);
}
