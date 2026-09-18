/*
 * Copyright (c) 2021-2024, Texas Instruments Incorporated
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
 *
 */

#include <ti/board/src/flash/nand/ospi/nand_ospi.h>
#include <ti/board/src/flash/nor/ospi/nor_spi_phy_tune.h>
#include <ti/drv/spi/soc/SPI_soc.h>
#include <ti/csl/soc.h>
#include <ti/osal/CacheP.h>
#include <ti/csl/src/ip/fss/V0/csl_fss.h>

#define BOARD_FLASH_FULL_PAGE_SIZE    (NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)
static uint8_t nandFullPageBuf[BOARD_FLASH_FULL_PAGE_SIZE] __attribute__((aligned(128)));
static bool dmaEnable = BFALSE;
static OSPI_Transaction transaction;

static NAND_HANDLE Nand_ospiOpen(uint32_t nandIntf, uint32_t portNum, void *params);
static void Nand_ospiClose(NAND_HANDLE handle);
static NAND_STATUS Nand_ospiRead(NAND_HANDLE handle, uint32_t addr,
                               uint32_t len, uint8_t *buf);
static NAND_STATUS Nand_ospiWrite(NAND_HANDLE handle, uint32_t addr,
                                uint32_t len, uint8_t *buf);
static NAND_STATUS Nand_ospiErase(NAND_HANDLE handle, int32_t eraseCnt);

static NAND_STATUS Nand_ospiCmdWrite(OSPI_Handle handle, uint8_t *cmdBuf,
                                   uint32_t cmdLen, uint32_t dataLen);

static NAND_STATUS Nand_ospiWaitReady(OSPI_Handle handle, uint32_t timeOut);

static NAND_STATUS Nand_ospiControl(NAND_HANDLE handle, uint32_t command, void *args);

/* NAND function table for NAND OSPI interface implementation */
const NAND_FxnTable Nand_ospiFxnTable =
{
    &Nand_ospiOpen,
    &Nand_ospiClose,
    &Nand_ospiRead,
    &Nand_ospiWrite,
    &Nand_ospiErase,
    &Nand_ospiControl
};

NAND_Info Nand_ospiInfo =
{
    0,                          /* hwHandle */
    0,                          /* manufacturerId */
    0,                          /* deviceId */
    0,                          /* busWidth */
    NAND_NUM_BLOCKS,            /* blockCnt */
    NAND_NUM_PAGES_PER_BLOCK,   /* pageCnt */
    NAND_PAGE_SIZE,             /* pageSize */
    0,                          /* baseAddr */
    NAND_SECTOR_SIZE            /* sectorSize */
};

static bool gPhyEnable;
static bool gIsPhyTuned;
static bool gDtrEnable;

static NAND_STATUS NAND_ospiCmdRead(OSPI_Handle handle, uint8_t *cmdBuf,
                            uint32_t cmdLen, uint8_t *rxBuf, uint32_t rxLen)
{
    OSPI_Transaction transaction;
    uint32_t        transferType = SPI_TRANSACTION_TYPE_READ;
    bool            ret;

    /* Update the mode and transfer type with the required values */
    OSPI_control(handle, OSPI_V0_CMD_SET_CFG_MODE, NULL);
    OSPI_control(handle, OSPI_V0_CMD_XFER_MODE_RW, (void *)&transferType);
    OSPI_control(handle, OSPI_V0_CMD_EXT_RD_CMD_LEN, (void *)&cmdLen);

    transaction.txBuf = (void *)cmdBuf;
    transaction.rxBuf = (void *)rxBuf;
    transaction.count = cmdLen + rxLen;

    ret = OSPI_transfer(handle, &transaction);
    if (BTRUE == ret)
    {
        return NAND_PASS;
    }
	else
    {
        return NAND_FAIL;
    }
}

static NAND_STATUS Nand_ospiReadStatusReg(OSPI_Handle handle, uint8_t regAddr, uint8_t *regData)
{
    NAND_STATUS     status;
    uint8_t         cmd[3];
    uint32_t        cmdDummyCycles = 0U;
    OSPI_v0_HwAttrs const *hwAttrs= (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    if(CSL_ospiGetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)))
    {
        cmd[0] = NAND_CMD_RDSR;
        cmd[1] = regAddr;  /* Address Bytes */
        cmd[2] = 0x00;

        cmdDummyCycles = 7U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);

        status = NAND_ospiCmdRead(handle, cmd, 3, regData, 1);

        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }
    else
    {
        cmd[0] = NAND_CMD_RDSR;
        cmd[1] = regAddr;  /* Address Bytes */

        cmdDummyCycles = 0U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);

        status = NAND_ospiCmdRead(handle, cmd, 2, regData, 1);

        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }

    return status;
}

static NAND_STATUS Nand_ospiWriteStatusReg(OSPI_Handle handle, uint8_t regAddr, uint8_t *regData)
{
    NAND_STATUS     status;
    uint8_t         cmd[3];

    cmd[0] = NAND_CMD_WRITE_STATUS;
    cmd[1] = regAddr;
    cmd[2] = *regData;

    status = Nand_ospiCmdWrite(handle, cmd, 2, 1);

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(handle, NAND_WRR_WRITE_TIMEOUT))
    {
        status = NAND_FAIL;
    }

    return status;
}

static NAND_STATUS Nand_ospiReadId(OSPI_Handle handle)
{
    NAND_STATUS     retVal;
    uint8_t         idCode[NAND_RDID_NUM_BYTES];
    uint8_t         cmd = NAND_CMD_RDID;
    uint32_t        manfID, devID;
    uint32_t        cmdDummyCycles;
    OSPI_v0_HwAttrs const *hwAttrs= (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    if (OSPI_XFER_LINES_OCTAL == hwAttrs->xferLines)
    {
        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }

    retVal = NAND_ospiCmdRead(handle, &cmd, 1, idCode, NAND_RDID_NUM_BYTES);

    if (OSPI_XFER_LINES_OCTAL == hwAttrs->xferLines)
    {
        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }

    if (NAND_PASS == retVal)
    {
        manfID = (uint32_t)idCode[0];
        devID = ((uint32_t)idCode[1] << 8) | ((uint32_t)idCode[2]);
        if ((NAND_MANF_ID == manfID) && (NAND_DEVICE_ID == devID))
        {
            Nand_ospiInfo.manufacturerId = manfID;
            Nand_ospiInfo.deviceId = devID;
        }
        else
        {
            retVal = NAND_FAIL;
        }
    }

    return (retVal);
}

static NAND_STATUS Nand_ospiEnableOctalDDR(OSPI_Handle handle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint8_t         cmdWren = NAND_CMD_WREN;
    uint32_t        data[3];

    /* Send Write Enable command */
    if(Nand_ospiCmdWrite(handle, &cmdWren, 1, 0))
    {
        retVal = NAND_FAIL;
    }

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(handle, NAND_WRR_WRITE_TIMEOUT))
    {
        retVal = NAND_FAIL;
    }

    /* Enable double transfer rate mode */
    if (NAND_PASS == retVal)
    {
        /* send write VCR command to reg addr 0x0 to set to DDR mode */
        data[0] = (NAND_CMD_WRITE_VCR << 24)        | /* write volatile config reg cmd */
                  (0 << 23)                         | /* read data disable */
                  (7 << 20)                         | /* read 8 data bytes */
                  (1 << 19)                         | /* enable cmd adddr */
                  (2 << 16)                         | /* 3 address bytes */
                  (1 << 15);                          /* write data enable */
        data[1] = 0U;     /* Non-volatile config register address */
        data[2] = 0xE7U; /* set to Octal SPI DDR with DS in Nonvolatile Config Reg 0x0 */
        OSPI_control(handle, OSPI_V0_CMD_ENABLE_DDR, (void *)data);
    }

    return retVal;
}

static NAND_STATUS Nand_ospiEnableOctalSDR(OSPI_Handle handle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint8_t         cmdWren = NAND_CMD_WREN;
    uint32_t        data[3];

    /* Send Write Enable command */
    if(Nand_ospiCmdWrite(handle, &cmdWren, 1, 0))
    {
        retVal = NAND_FAIL;
    }

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(handle, NAND_WRR_WRITE_TIMEOUT))
    {
        retVal = NAND_FAIL;
    }

    /* Enable single transfer rate mode */
    if (NAND_PASS == retVal)
    {
        /* send write VCR command to reg addr 0x0 to set to SDR mode */
        data[0] = (NAND_CMD_WRITE_VCR << 24)        | /* write volatile config reg cmd */
                  (0 << 23)                         | /* read data disable */
                  (7 << 20)                         | /* read 8 data bytes */
                  (1 << 19)                         | /* enable cmd adddr */
                  (2 << 16)                         | /* 3 address bytes */
                  (1 << 15);                          /* write data enable */
        data[1] = 0U;    /* Non-volatile config register address */
        data[2] = 0xDFU; /* set to Octal SPI SDR without DS in Nonvolatile Config Reg 0x0 */
        OSPI_control(handle, OSPI_V0_CMD_ENABLE_SDR, (void *)data);

    }

    return retVal;
}

static NAND_STATUS Nand_ospiEnableSingleSDR(OSPI_Handle handle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint8_t         cmdWren = NAND_CMD_WREN;
    uint32_t        data[3];

    /* Send Write Enable command */
    if(Nand_ospiCmdWrite(handle, &cmdWren, 1, 0))
    {
        retVal = NAND_FAIL;
    }

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(handle, NAND_WRR_WRITE_TIMEOUT))
    {
        retVal = NAND_FAIL;
    }

    /* Enable single transfer rate mode */
    if (NAND_PASS == retVal)
    {
        /* send write VCR command to reg addr 0x0 to set to SDR mode */
        data[0] = (NAND_CMD_WRITE_VCR << 24)        | /* write volatile config reg cmd */
                  (0 << 23)                         | /* read data disable */
                  (7 << 20)                         | /* read 8 data bytes */
                  (1 << 19)                         | /* enable cmd adddr */
                  (2 << 16)                         | /* 3 address bytes */
                  (1 << 15);                          /* write data enable */
        data[1] = 0U;    /* Non-volatile config register address */
        data[2] = 0xFFU; /* set to Single SPI SDR with DS in Nonvolatile Config Reg 0x0 */
        OSPI_control(handle, OSPI_V0_CMD_ENABLE_SDR, (void *)data);

    }

    return retVal;
}

static NAND_STATUS Nand_ospiResetMemory(OSPI_Handle handle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint8_t         cmd;

    /* Send Reset Enable command */
    cmd = NAND_CMD_RSTEN;
    retVal = Nand_ospiCmdWrite(handle, &cmd, 1, 0);

    if (NAND_PASS == retVal)
    {
        /* Send Reset Device Memory command */
        cmd = NAND_CMD_RST_MEM;
        retVal = Nand_ospiCmdWrite(handle, &cmd, 1, 0);
    }

    return (retVal);
}

static NAND_STATUS Nand_ospiSetDummyCycle(OSPI_Handle handle, uint32_t dummyCycle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint8_t         cmdWren = NAND_CMD_WREN;
    uint32_t        data[3];

    /* Send Write Enable command */
    if(Nand_ospiCmdWrite(handle, &cmdWren, 1, 0))
    {
        retVal = NAND_FAIL;
    }

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(handle, NAND_WRR_WRITE_TIMEOUT))
    {
        retVal = NAND_FAIL;
    }

    /* send the dummy cycle value to reg addr 0x1 */
    if (NAND_PASS == retVal)
    {
        data[0] = (NAND_CMD_WRITE_VCR << 24)        | /* write volatile config reg cmd */
                  (0 << 23)                         | /* read data disable */
                  (7 << 20)                         | /* read 8 data bytes */
                  (1 << 19)                         | /* enable cmd adddr */
                  (2 << 16)                         | /* 3 address bytes */
                  (1 << 15);                          /* write data enable */
        data[1] = 0x00000001U;                        /* Dummy cycle config register address */
        data[2] = dummyCycle;                         /* Dummy cycle # */
        OSPI_control(handle, OSPI_V0_CMD_CFG_DUMMY_CYCLE, (void *)data);
    }

    return retVal;
}

static NAND_STATUS Nand_ospiDisableWriteProtection(OSPI_Handle handle)
{
    NAND_STATUS     retVal = NAND_PASS;
    uint32_t        data[3];
    uint32_t        retry = 10U;
    volatile uint32_t delay = 1000U;
    uint32_t        idleFlag = UFALSE;
    OSPI_v0_HwAttrs const *hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;
    const CSL_ospi_flash_cfgRegs *pRegs = (const CSL_ospi_flash_cfgRegs *)hwAttrs->baseAddr;

    data[0] = (NAND_CMD_WRITE_STATUS << 24)   | /* write volatile config reg cmd */
            (0 << 23)                         | /* read data disable */
            (0 << 20)                         | /* read 0 data bytes */
            (1 << 19)                         | /* enable cmd adddr */
            (0 << 16)                         | /* 1 address bytes */
            (1 << 15)                         | /* write data enable */
            (0 << 12);                          /* write 1 data bytes */
    data[1] = 0x000000A0U;                      /* status register 1 address */
    data[2] = 0x00000000U;                      /* Value to disable all protection */

    CSL_ospiFlashStig((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), data[0], data[1], data[2]);

    while (UFALSE == idleFlag)
    {
        idleFlag = CSL_ospiIsIdle(pRegs);
    }

    /* Start to execute flash read/write command */
    CSL_ospiFlashExecCmd(pRegs);

    while (0U != retry)
    {
        /* Check the command execution status */
        if (UTRUE == CSL_ospiFlashExecCmdComplete(pRegs))
        {
            break;
        }
        while(delay--);
        delay = 1000U;
        retry--;
    }

    if (0U == retry)
    {
        retVal = NAND_FAIL;
    }

    idleFlag = UFALSE;
    while (UFALSE == idleFlag)
    {
        idleFlag = CSL_ospiIsIdle(pRegs);
    }

    return retVal;
}

static void Nand_ospiSetOpcode(OSPI_Handle handle)
{
    uint32_t               data[6];
    uint32_t               rdDummyCycles = 0U;
    uint32_t               cmdDummyCycles = 0U;
    uint32_t               readCmd;
    uint32_t               progCmd;
    uint32_t               rx_lines;
    uint32_t               addr_lines;
    OSPI_v0_HwAttrs const *hwAttrs = (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    rx_lines    = hwAttrs->xferLines;
    addr_lines  = hwAttrs->numAddrLines;
    if (OSPI_XFER_LINES_OCTAL == rx_lines)
    {
        if (hwAttrs->dacEnable)
        {
            rdDummyCycles = NAND_OCTAL_READ_DUMMY_CYCLE;
        }
        else
        {
            rdDummyCycles = NAND_OCTAL_READ_DUMMY_CYCLE_INDAC;
        }

        if (BTRUE == gDtrEnable)
        {
            cmdDummyCycles  = NAND_OCTAL_DDR_CMD_READ_DUMMY_CYCLE;
            readCmd         = NAND_CMD_FAST_READ_DDR;
            progCmd         = NAND_CMD_PAGE_PROG;
        }
        else if(addr_lines == OSPI_XFER_LINES_OCTAL)
        {
            /* Set to SDR 1-8-8 mode */
            cmdDummyCycles  = NAND_OCTAL_SDR_CMD_READ_DUMMY_CYCLE;
            readCmd         = NAND_CMD_OCTAL_IO_FAST_RD;
            progCmd         = NAND_CMD_EXT_OCTAL_FAST_PROG;
        }
        else
        {
            /* Set to SDR 1-1-8 mode */
            cmdDummyCycles  = NAND_OCTAL_SDR_CMD_READ_DUMMY_CYCLE;
            readCmd         = NAND_CMD_OCTAL_DDR_O_FAST_RD;
            progCmd         = NAND_CMD_EXT_OCTAL_FAST_PROG;
        }
    }
    else
    {
        /* Set to legacy SPI mode 1-1-1 if not Octal mode */
        rdDummyCycles   = NAND_SINGLE_READ_DUMMY_CYCLE;
        cmdDummyCycles  = NAND_SINGLE_CMD_READ_DUMMY_CYCLE;
        readCmd         = NAND_CMD_READ;
        progCmd         = NAND_CMD_PAGE_PROG;
    }

    data[0] = readCmd;
    data[1] = progCmd;
    data[2] = NAND_CMD_RDSR;
    data[3] = NAND_SR3_ADDR;

    /* Update the read opCode, rx lines and read dummy cycles */
    OSPI_control(handle, OSPI_V0_CMD_RD_DUMMY_CLKS, (void *)&rdDummyCycles);
    OSPI_control(handle, OSPI_V0_CMD_SET_XFER_LINES, (void *)&rx_lines);
    OSPI_control(handle, OSPI_V0_CMD_XFER_OPCODE, (void *)data);
    OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);

    /* Set device size cofigurations */
    CSL_ospiSetDevSize((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr),
                        1,
                        256,
                        16);

    if (BTRUE == gDtrEnable)
    {
        /* Set the opcodes for dual opcode mode */
        data[0] = 0x00U;
        data[1] = 0xFAU;
        data[2] = readCmd;
        data[3] = progCmd;
        data[4] = 0xF9U;
        data[5] = 0x06U;
        OSPI_control(handle, OSPI_V0_CMD_XFER_OPCODE_EXT, (void *)data);
    }
    else
    {
        CSL_ospiSetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
    }

    return;
}

NAND_HANDLE Nand_ospiOpen(uint32_t nandIntf, uint32_t portNum, void *params)
{
    OSPI_Params      spiParams;      /* OSPI params structure */
    OSPI_Handle      hwHandle;       /* OSPI handle */
    NAND_HANDLE     nandHandle = 0;
    OSPI_v0_HwAttrs ospiCfg;
    NAND_STATUS     retVal;
    uint32_t        data;

    /* Get the OSPI SoC configurations */
    OSPI_socGetInitCfg(SPI_OSPI_DOMAIN_MCU, portNum, &ospiCfg);

    /* Apply device-specific address bytes and PHY mode from the device header.
     *
     * Errata i2383: the OSPI controller requires 4-byte addressing when
     * sampling in PHY DDR mode.
     *
     *   NAND_ADDR_BYTES_PROGRAMMABLE = 0 — fixed address bytes (e.g.
     *       W35N01JWTBAG: always 2-byte).  PHY DDR is incompatible since the
     *       controller needs 4-byte addressing for PHY DDR. TAP (no-PHY) DDR
     *       is the valid alternative: dtrEnable is preserved so the flash
     *       operates in 8D-8D-8D DDR mode; phyEnable is forced off so the
     *       PHY calibration path (which needs 4-byte) is not used.
     *
     *   NAND_ADDR_BYTES_PROGRAMMABLE = 1 — switchable address bytes (e.g. a
     *       future NAND supporting 4-byte). PHY DDR is fully supported when
     *       NAND_OSPI_ADDR_BYTES is 4. Both dtrEnable and phyEnable are
     *       preserved from the SOC config.
     *
     * For a new NAND device update NAND_OSPI_ADDR_BYTES and
     * NAND_ADDR_BYTES_PROGRAMMABLE in the corresponding device header file. */
    ospiCfg.numAddrBytes = NAND_OSPI_ADDR_BYTES;
    gDtrEnable           = ospiCfg.dtrEnable;
    if (NAND_ADDR_BYTES_PROGRAMMABLE == 0U)
    {
        /* Fixed 2-byte addressing: PHY DDR not compatible per errata i2383.
         * Force phyEnable off to use TAP DDR mode (DDR without PHY). */
        ospiCfg.phyEnable = BFALSE;
    }
    dmaEnable  = ospiCfg.dmaEnable;

    /* Reset the PHY tunning configuration data when enabled */
    data = *(uint32_t *)params;
    if (0U != data)
    {
        Nand_spiPhyTuneReset(gDtrEnable);
        gIsPhyTuned = BFALSE;
    }

    /* Save the PHY enable flag */
    gPhyEnable = ospiCfg.phyEnable;
    if (BTRUE == gPhyEnable)
    {
        /*
         * phyEnable is turned on only for DAC read,
         * it turned off for open/erase/write operation
         */
        ospiCfg.phyEnable = BFALSE;
    }
    /* Always write back config so numAddrBytes and dtrEnable overrides
     * are applied even when phyEnable was already BFALSE. */
    OSPI_socSetInitCfg(SPI_OSPI_DOMAIN_MCU, portNum, &ospiCfg);

    /* Use default OSPI config params if no params provided */
    OSPI_Params_init(&spiParams);
    hwHandle = (OSPI_Handle)OSPI_open(SPI_OSPI_DOMAIN_MCU, portNum, &spiParams);
    if (hwHandle)
    {
        retVal = NAND_PASS;
        if (NAND_PASS == retVal)
        {
            OSPI_v0_HwAttrs *hwAttrs = (OSPI_v0_HwAttrs *)hwHandle->hwAttrs;
            CSL_ospiSetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr), UFALSE);
            if (OSPI_XFER_LINES_OCTAL == ospiCfg.xferLines)
            {
                /* Disable write protection */
                Nand_ospiDisableWriteProtection(hwHandle);
                /* Set read dummy cycles to the flash device */
                Nand_ospiSetDummyCycle(hwHandle, 0x08);
                /* Enable DDR or SDR mode for Octal lines */
                if (BTRUE == gDtrEnable)
                {
                    Nand_ospiEnableOctalDDR(hwHandle);
                }
                else
                {
                    Nand_ospiEnableOctalSDR(hwHandle);
                }
            }
            else
            {
                /* Reset device memory for all the other lines */
                Nand_ospiResetMemory(hwHandle);
                /* Disable write protection */
                Nand_ospiDisableWriteProtection(hwHandle);
                /* Set read dummy cycles to the flash device */
                Nand_ospiSetDummyCycle(hwHandle, 0x08);
                /* Enable Single SDR mode */
                Nand_ospiEnableSingleSDR(hwHandle);
            }

            /* Set read/write opcode and read dummy cycles */
            Nand_ospiSetOpcode(hwHandle);

            /* Read the flash ID to ensure correct config */
            if (NAND_PASS == Nand_ospiReadId(hwHandle))
            {
                Nand_ospiInfo.hwHandle = (uintptr_t)hwHandle;
                nandHandle = (NAND_HANDLE)(&Nand_ospiInfo);
            }
        }

        if (0 == nandHandle)
        {
            OSPI_close(hwHandle);
        }
        else
        {
            memset((void *)nandFullPageBuf, 0xFF, BOARD_FLASH_FULL_PAGE_SIZE);
        }
    }

    return (nandHandle);
}

void Nand_ospiClose(NAND_HANDLE handle)
{
    NAND_Info    *nandOspiInfo;
    OSPI_Handle   ospiHandle;
    OSPI_v0_HwAttrs const        *hwAttrs;
    const CSL_ospi_flash_cfgRegs *pRegs;

    if (handle)
    {
        nandOspiInfo = (NAND_Info *)handle;
        ospiHandle = (OSPI_Handle)nandOspiInfo->hwHandle;

        if (ospiHandle)
        {
            /*
            * Some fields in RD_DATA_CAPTURE_REG are modified by the Nand_spiPhyTune API.
            * These fields need to be reset here to avoid errors in subsequent tests.
            */
            hwAttrs = (OSPI_v0_HwAttrs const *)ospiHandle->hwAttrs;
            pRegs = (const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr);
            CSL_REG32_FINS(&pRegs->RD_DATA_CAPTURE_REG,
                            OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_SAMPLE_EDGE_SEL_FLD,
                            CSL_OSPI_CFG_PHY_SAMPLE_EDGE_DEFAULT);

            CSL_REG32_FINS(&pRegs->RD_DATA_CAPTURE_REG,
                            OSPI_FLASH_CFG_RD_DATA_CAPTURE_REG_DQS_ENABLE_FLD,
                            0);
            OSPI_close(ospiHandle);
        }
    }
}

static NAND_STATUS Nand_ospiCmdWrite(OSPI_Handle handle, uint8_t *cmdBuf,
                                        uint32_t cmdLen, uint32_t dataLen)
{
    OSPI_Transaction  transaction;
    uint32_t         transferType = SPI_TRANSACTION_TYPE_WRITE;
    bool             ret;

    /* Update the mode and transfer type with the required values */
    OSPI_control(handle, OSPI_V0_CMD_SET_CFG_MODE, NULL);
    OSPI_control(handle, OSPI_V0_CMD_XFER_MODE_RW, (void *)&transferType);

    transaction.txBuf = (void *)cmdBuf; /* Buffer includes command and write data */
    transaction.count = cmdLen + dataLen;
    transaction.rxBuf = NULL;
    transaction.arg = (void *)(uintptr_t)dataLen;

    ret = OSPI_transfer(handle, &transaction);
    if (BTRUE == ret)
    {
        return NAND_PASS;
    }
	else
    {
        return NAND_FAIL;
    }
}

static NAND_STATUS Nand_ospiWaitReady(OSPI_Handle handle, uint32_t timeOut)
{
    uint8_t         status;
    uint8_t         cmd[3];
    uint32_t        cmdDummyCycles = 0U;
    OSPI_v0_HwAttrs const *hwAttrs= (OSPI_v0_HwAttrs const *)handle->hwAttrs;

    if(CSL_ospiGetDualByteOpcodeMode((const CSL_ospi_flash_cfgRegs *)(hwAttrs->baseAddr)))
    {
        cmd[0] = NAND_CMD_RDSR;
        cmd[1] = NAND_SR3_ADDR;  /* Address Bytes */
        cmd[2] = 0x00;

        cmdDummyCycles = 7U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);

        do
        {
            if (NAND_ospiCmdRead(handle, cmd, 3, &status, 1))
            {
                return NAND_FAIL;
            }
            if (0 == (status & NAND_SR_WIP))
            {
                break;
            }

            timeOut--;
            if (!timeOut) {
                status = 0U;
                break;
            }

        } while (1);
        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }
    else
    {
        cmd[0] = NAND_CMD_RDSR;
        cmd[1] = NAND_SR3_ADDR;  /* Address Bytes */

        cmdDummyCycles = 0U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);

        do
        {
            if (NAND_ospiCmdRead(handle, cmd, 2, &status, 1))
            {
                return NAND_FAIL;
            }
            if (0 == (status & NAND_SR_WIP))
            {
                break;
            }

            timeOut--;
            if (!timeOut) {
                break;
            }

        } while (1);
        cmdDummyCycles = 8U;
        OSPI_control(handle, OSPI_V0_CMD_EXT_RD_DUMMY_CLKS, (void *)&cmdDummyCycles);
    }

    if (0 == (status & NAND_SR_WIP))
    {
        return NAND_PASS;
    }

    /* Timed out */
    return NAND_FAIL;
}

static NAND_STATUS Nand_ospiPageLoad(OSPI_Handle ospiHandle, uint32_t rdAddr)
{
    uint32_t         pageAddr = 0U;
    uint32_t         pageReadCmdLen = 4U;
    uint8_t          pageReadCmd[4];
    CSL_FssCfg             fssCfg;
    uint32_t         xipPrefetchEnable;

    /* Disable XIP Prefetch before programming flash memory */
    xipPrefetchEnable = UFALSE;
    fssCfg.pFsasRegs = (CSL_fss_fsas_genregsRegs *)CSL_MCU_FSS0_FSAS_CFG_BASE;
    CSL_fssOspiSetXipPrefetchEnable(&fssCfg, CSL_FSS_FSAS_INTERFACE_PATH_SELECT_OSPI0, xipPrefetchEnable);

    pageAddr = rdAddr / NAND_PAGE_SIZE;

    if (BTRUE == gDtrEnable)
    {
        /* Send the page read command */
        pageReadCmd[0] = NAND_CMD_PAGE_READ;
        pageReadCmd[1] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
        pageReadCmd[2] = (pageAddr >>  0) & 0xFF;
        pageReadCmdLen = 3;
        Nand_ospiCmdWrite(ospiHandle, pageReadCmd, pageReadCmdLen, 0);
    }
    else
    {
        /* Send the page read command */
        pageReadCmd[0] = NAND_CMD_PAGE_READ;
        pageReadCmd[1] = 0x00;                       /* Dummy Byte */
        pageReadCmd[2] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
        pageReadCmd[3] = (pageAddr >>  0) & 0xFF;
        pageReadCmdLen = 4;
        Nand_ospiCmdWrite(ospiHandle, pageReadCmd, pageReadCmdLen, 0);
    }

    /* Check BUSY bit of Flash */
    if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
    {
        return NAND_FAIL;
    }

    /* Enable back XIP prefetch */
    xipPrefetchEnable = UTRUE;
    fssCfg.pFsasRegs = (CSL_fss_fsas_genregsRegs *)CSL_MCU_FSS0_FSAS_CFG_BASE;
    CSL_fssOspiSetXipPrefetchEnable(&fssCfg, CSL_FSS_FSAS_INTERFACE_PATH_SELECT_OSPI0, xipPrefetchEnable);

    return NAND_PASS;
}

NAND_STATUS Nand_ospiRead(NAND_HANDLE handle, uint32_t addr, uint32_t len, uint8_t *buf)
{
    NAND_Info        *nandOspiInfo;
    OSPI_Handle      ospiHandle;
    NAND_STATUS      status;
    uint32_t         rdAddr;
    uint32_t         pageAddr;
    uint32_t         colmAddr;
    uint32_t         chunkLen;
    uint32_t         actual = 0U;
    uint32_t         pageReadCmdLen = 4U;
    bool             ret = BTRUE;
    uint32_t         transferType = SPI_TRANSACTION_TYPE_READ;
    OSPI_v0_HwAttrs *hwAttrs;
    uint8_t          pageReadCmd[4];
    uint8_t          bufMode;

    if (!handle)
    {
        return NAND_FAIL;
    }

    nandOspiInfo = (NAND_Info *)handle;
    if (!nandOspiInfo->hwHandle)
    {
        return NAND_FAIL;
    }
    ospiHandle = (OSPI_Handle)nandOspiInfo->hwHandle;
    hwAttrs = (OSPI_v0_HwAttrs *)ospiHandle->hwAttrs;

    /* Read the buffer mode configuration from flash register */
    status = Nand_ospiReadStatusReg(ospiHandle, NAND_SR2_ADDR, &bufMode);
    if(NAND_PASS != status)
    {
        return status;
    }

    if (BTRUE == gPhyEnable)
    {
        if(BFALSE == gIsPhyTuned)
        {
            if (NAND_FAIL == Nand_ospiPageLoad(ospiHandle, NAND_TUNING_DATA_OFFSET))
            {
                return NAND_FAIL;
            }
        }
        if (NAND_FAIL == Nand_spiPhyTune(ospiHandle, NAND_TUNING_DATA_OFFSET))
        {
           return NAND_FAIL;
        }
        else
        {
            gIsPhyTuned = BTRUE;
        }
    }
    /* Validate address input */
    if (NAND_SIZE < (addr + len))
    {
        return NAND_FAIL;
    }

    bufMode = (bufMode >> 3) & 0x1;
    if(1U == bufMode)
    {
        actual = 0U;
        for(rdAddr = addr; rdAddr < (addr+len);)
        {
            /*
             * In the buffered read mode NAND flash returns data from specified column address
             * till end of the data buffer filled by page read command. Multiple page read and
             * data read commands are needed for reading the data.
             */

            /* Split the page and column addresses */
            pageAddr = rdAddr / NAND_PAGE_SIZE;
            colmAddr = rdAddr % NAND_PAGE_SIZE;

            chunkLen =  NAND_PAGE_SIZE - colmAddr;

            /* Send Page Program command */
            chunkLen = (((len - actual) < chunkLen) ?
                        (len - actual) : chunkLen);

            Nand_ospiPageLoad(ospiHandle, rdAddr);

            /* Set transfer mode and read type */
            OSPI_control(ospiHandle, OSPI_V0_CMD_SET_XFER_MODE, NULL);
            OSPI_control(ospiHandle, OSPI_V0_CMD_XFER_MODE_RW, (void *)&transferType);

            transaction.arg   = (void *)(uintptr_t)colmAddr;
            transaction.txBuf = NULL;
            transaction.rxBuf = (void *)(buf + actual);
            transaction.count = chunkLen;

            ret = OSPI_transfer(ospiHandle, &transaction);
            if (BFALSE == ret)
            {
                return NAND_FAIL;
            }

            rdAddr += chunkLen;
            actual += chunkLen;
        }
    }
    else
    {
        /*
         * In the continuous read mode NAND flash returns data from start of the page
         * and cotinue to output the data from next page when it reaches end of buffer.
         * Whole memory area can be read with single data read command.
         * Column address will be ingored in continuous read mode.
         */

        /* Split the page and column addresses */
        pageAddr = addr / NAND_PAGE_SIZE;
        colmAddr = 0U; /* Column address will be dummy cycles in contnuous read mode */

        if (OSPI_XFER_LINES_OCTAL == hwAttrs->xferLines)
        {
            /* Send the page read command */
            pageReadCmd[0] = NAND_CMD_PAGE_READ;
            pageReadCmd[1] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
            pageReadCmd[2] = (pageAddr >>  0) & 0xFF;
            pageReadCmdLen = 3U;
            Nand_ospiCmdWrite(ospiHandle, pageReadCmd, pageReadCmdLen, 0);
        }
        else
        {
            /* Send the page read command */
            pageReadCmd[0] = NAND_CMD_PAGE_READ;
            pageReadCmd[1] = 0x00;                       /* Dummy Byte */
            pageReadCmd[2] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
            pageReadCmd[3] = (pageAddr >>  0) & 0xFF;
            pageReadCmdLen = 4U;
            Nand_ospiCmdWrite(ospiHandle, pageReadCmd, pageReadCmdLen, 0);
        }

        /* Check BUSY bit of Flash */
        if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
        {
            return NAND_FAIL;
        }

        /* Set transfer mode and read type */
        OSPI_control(ospiHandle, OSPI_V0_CMD_SET_XFER_MODE, NULL);
        OSPI_control(ospiHandle, OSPI_V0_CMD_XFER_MODE_RW, (void *)&transferType);

        transaction.arg   = (void *)(uintptr_t)colmAddr;
        transaction.txBuf = NULL;
        transaction.rxBuf = (void *)(buf);
        transaction.count = len;

        ret = OSPI_transfer(ospiHandle, &transaction);
        if (BFALSE == ret)
        {
            return NAND_FAIL;
        }
    }

    return NAND_PASS;
}

NAND_STATUS Nand_ospiWrite(NAND_HANDLE handle, uint32_t addr, uint32_t len, uint8_t *buf)
{
    NAND_Info        *nandOspiInfo;
    OSPI_Handle       ospiHandle;
    bool             ret;
    uint32_t         byteAddr;
    uint32_t         pageAddr;
    uint32_t         colmAddr;
    uint32_t         wrSize = len;
    uint32_t         chunkLen;
    uint32_t         actual;
    uint32_t         transferType = SPI_TRANSACTION_TYPE_WRITE;
    OSPI_v0_HwAttrs *hwAttrs;
    uint8_t          progExecuteCmd[4];
    uint32_t         progExecuteCmdLen = 4U;
    uint8_t          cmdWren = NAND_CMD_WREN;

    if (!handle)
    {
        return NAND_FAIL;
    }

    nandOspiInfo = (NAND_Info *)handle;
    if (!nandOspiInfo->hwHandle)
    {
        return NAND_FAIL;
    }

    /* Validate address input */
    if (NAND_SIZE < (addr + len))
    {
        return NAND_FAIL;
    }

    ospiHandle = (OSPI_Handle)nandOspiInfo->hwHandle;
    hwAttrs = (OSPI_v0_HwAttrs *)ospiHandle->hwAttrs;

    wrSize = NAND_PAGE_SIZE;
    byteAddr = addr & (wrSize - 1U);

    for (actual = 0U; actual < len; actual += chunkLen)
    {
        /* Send Write Enable command */
        if(Nand_ospiCmdWrite(ospiHandle, &cmdWren, 1, 0))
        {
            return NAND_FAIL;
        }

        /* Check BUSY bit of Flash */
        if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
        {
            return NAND_FAIL;
        }

        /* Set the transfer mode, write op code and tx lines */
        OSPI_control(ospiHandle, OSPI_V0_CMD_SET_XFER_MODE, NULL);
        OSPI_control(ospiHandle, OSPI_V0_CMD_XFER_MODE_RW, (void *)&transferType);

        /* Send Page Program command */
        chunkLen = (((len - actual) < (wrSize - byteAddr)) ?
                    (len - actual) : (wrSize - byteAddr));

        /* Split the page and column addresses */
        pageAddr = addr / NAND_PAGE_SIZE;
        colmAddr = addr % NAND_PAGE_SIZE;

        /* Fill the data buffer with 0xFF if the data size is less than page size
         * to avoid writing erased sections which need not be written for this page.
         */
        if(NAND_PAGE_SIZE > chunkLen)
        {
            memset((void *)nandFullPageBuf, 0xFF, BOARD_FLASH_FULL_PAGE_SIZE);
        }

        memcpy((void*)nandFullPageBuf, (const void*)&buf[actual], chunkLen);

        if(BTRUE == dmaEnable)
        {
            CacheP_wbInv((void *)nandFullPageBuf, BOARD_FLASH_FULL_PAGE_SIZE);
        }

        transaction.arg   = (void *)(uintptr_t)colmAddr;
        transaction.txBuf = (void *)(nandFullPageBuf);
        transaction.rxBuf = NULL;
        transaction.count = BOARD_FLASH_FULL_PAGE_SIZE;

        ret = OSPI_transfer(ospiHandle, &transaction);
        if (BFALSE == ret)
        {
            return NAND_FAIL;
        }

        /* Check BUSY bit of Flash */
        if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
        {
            return NAND_FAIL;
        }

        if (OSPI_XFER_LINES_OCTAL == hwAttrs->xferLines)
        {
            progExecuteCmd[0] = NAND_CMD_PAGE_PROG_EXECUTE;
            progExecuteCmd[1] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
            progExecuteCmd[2] = (pageAddr >>  0) & 0xFF;
            progExecuteCmdLen = 3U;
            Nand_ospiCmdWrite(ospiHandle, progExecuteCmd, progExecuteCmdLen, 0);
        }
        else
        {
            progExecuteCmd[0] = NAND_CMD_PAGE_PROG_EXECUTE;
            progExecuteCmd[1] = 0x00;                       /* Dummy Byte */
            progExecuteCmd[2] = (pageAddr >>  8) & 0xFF;    /* page address 2 bytes */
            progExecuteCmd[3] = (pageAddr >>  0) & 0xFF;
            progExecuteCmdLen = 4U;
            Nand_ospiCmdWrite(ospiHandle, progExecuteCmd, progExecuteCmdLen, 0);
        }
        addr += chunkLen;
        byteAddr = 0U;

        /* Wait till the write operation completes */
        if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
        {
            return NAND_FAIL;
        }
    }

    return NAND_PASS;
}

NAND_STATUS Nand_ospiErase(NAND_HANDLE handle, int32_t erLoc)
{
    uint8_t         cmd[4];
    uint32_t        cmdLen;
    uint32_t        address = 0U;
    uint32_t        pageAddr = 0U;
    uint8_t         cmdWren  = NAND_CMD_WREN;
    NAND_Info       *nandOspiInfo;
    OSPI_Handle      ospiHandle;
    OSPI_v0_HwAttrs *hwAttrs;

    if (!handle)
    {
        return NAND_FAIL;
    }

    nandOspiInfo = (NAND_Info *)handle;
    if (!nandOspiInfo->hwHandle)
    {
        return NAND_FAIL;
    }
    ospiHandle = (OSPI_Handle)nandOspiInfo->hwHandle;
    hwAttrs = (OSPI_v0_HwAttrs *)ospiHandle->hwAttrs;

    if (NAND_NUM_BLOCKS <= erLoc)
    {
        return NAND_FAIL;
    }
    address = erLoc * NAND_BLOCK_SIZE;

    /* Get the page addresses */
    pageAddr = address / NAND_PAGE_SIZE;

    if (OSPI_XFER_LINES_OCTAL == hwAttrs->xferLines)
    {
        cmd[0]  = NAND_CMD_BLOCK_ERASE;
        cmd[1] = (pageAddr >> 8) & 0xFF; /* page address 2 bytes */
        cmd[2] = (pageAddr >> 0) & 0xFF;
        cmdLen = 3U;
    }
    else
    {
        cmd[0]  = NAND_CMD_BLOCK_ERASE;
        cmd[1]  = 0x00;                  /* Dummy Byte */
        cmd[2] = (pageAddr >> 8) & 0xFF; /* page address 2 bytes */
        cmd[3] = (pageAddr >> 0) & 0xFF;
        cmdLen = 4U;
    }

    if (Nand_ospiCmdWrite(ospiHandle, &cmdWren, 1, 0))
    {
    	return NAND_FAIL;
    }

    if (Nand_ospiWaitReady(ospiHandle, NAND_WRR_WRITE_TIMEOUT))
    {
    	return NAND_FAIL;
    }

    if (Nand_ospiCmdWrite(ospiHandle, cmd, cmdLen, 0))
    {
    	return NAND_FAIL;
    }

    if (Nand_ospiWaitReady(ospiHandle, NAND_BULK_ERASE_TIMEOUT))
    {
    	return NAND_FAIL;
    }

    return NAND_PASS;
}

NAND_STATUS Nand_ospiControl(NAND_HANDLE handle, uint32_t command, void *args)
{
    uint8_t        regData = 0U;
    NAND_Info      *nandOspiInfo;
    OSPI_Handle    ospiHandle;
    NAND_STATUS    status;

    if (!handle)
    {
        return NAND_FAIL;
    }

    nandOspiInfo = (NAND_Info *)handle;
    if (!nandOspiInfo->hwHandle)
    {
        return NAND_FAIL;
    }
    ospiHandle = (OSPI_Handle)nandOspiInfo->hwHandle;

    switch (command)
    {
        case NAND_FLASH_CTRL_ENABLE_OTP_ACCESS:
            status = Nand_ospiReadStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            if(!status)
            {
                regData |= 0x40;
                status = Nand_ospiWriteStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            }
            break;

        case NAND_FLASH_CTRL_DISABLE_OTP_ACCESS:
            status = Nand_ospiReadStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            if(!status)
            {
                regData &= ~0x40;
                status = Nand_ospiWriteStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            }
            break;

        case NAND_FLASH_CTRL_LOCK_OTP:
            status = Nand_ospiReadStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            if(!status)
            {
                regData |= 0x80;
                status = Nand_ospiWriteStatusReg(ospiHandle, NAND_SR2_ADDR, &regData);
            }
            break;

        default:
            status = NAND_FAIL;
            break;
    }

    return status;
}
