/*
 * Copyright (c) 2020, Texas Instruments Incorporated
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

/**
 *
 * \file  s28hs512t.h
 *
 * \brief This file contains S28HS512T NOR device definitions
 *
 *****************************************************************************/
#ifndef S28HS512T_H_
#define S28HS512T_H_

#include <ti/drv/spi/SPI.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 **                       Macro Definitions
 **************************************************************************/

/** FLASH device specific items (note: sizes are in bytes) */
#define NOR_BLOCK_SIZE               (256U * 1024U)
/* The Smallest Hybrid sector size is 4-KB */
#define NOR_SECTOR_SIZE              (4U * 1024U)
/* The Hybrid sector size of 64-KB */
#define NOR_HYBRID_SECTOR_SIZE       NOR_BLOCK_SIZE
#define NOR_SIZE                     (64U * 1024U * 1024U)
/** Physical set of thirty two 4-KB sectors at top or bottom of the address space */
#define NOR_NUM_4KSECTORS            (32U)
#define NOR_NUM_SECTORS              NOR_NUM_4KSECTORS
#define NOR_NUM_BLOCKS               (NOR_SIZE / NOR_BLOCK_SIZE)
#define NOR_PAGE_SIZE                (256U)
#define NOR_NUM_PAGES_PER_SECTOR     (NOR_SECTOR_SIZE / NOR_PAGE_SIZE)
#define NOR_NUM_PAGES_PER_BLOCK      (NOR_BLOCK_SIZE / NOR_PAGE_SIZE)

#define NOR_4K_SECT_BOT_END_OFFSET   (0x1FFFF)

/** Flash device commands */ 
#define NOR_BE_SECTOR_NUM            (-1U)

#define NOR_CMD_BULK_ERASE           (0xC7U)
#define NOR_CMD_WRREG                (0x71U)
#define NOR_CMD_WREN                 (0x06U)
#define NOR_CMD_WRDIS                (0x04U)
#define NOR_CMD_RDSR                 (0x05U)
#define NOR_CMD_RDSR2                (0x07U)
#define NOR_CMD_RDREG                (0x65U)
#define NOR_CMD_RDCR                 (0x35U)
#define NOR_CMD_RDID                 (0x9FU)
#define NOR_CMD_SRSTE                (0x66U)
#define NOR_CMD_SFRST                (0x99U)

#define NOR_CMD_BLOCK_ERASE          (0xDCU)
#define NOR_CMD_SECTOR_ERASE         (0x21U)
#define NOR_CMD_READ                 (0x03U)
#define NOR_CMD_FAST_READ            (0x0BU)
#define NOR_CMD_OCTAL_READ           (0xECU)
#define NOR_CMD_OCTAL_DDR_READ       (0xEEU)
#define NOR_CMD_PAGE_PROG            (0x02U)
#define NOR_CMD_OCTAL_PROG           (0x12U)
#define NOR_CMD_WRITE_VCR            (0x71U)
#define NOR_CMD_READ_VCR             (0x85U)

#define NOR_VREG_OFFSET              (0x80U)
#define NOR_NVREG_OFFSET             (0x0U)

#define NOR_STS1_NVREG_ADDR          (0x0U)
#define NOR_STS2_NVREG_ADDR          (0x1U)
#define NOR_CFG1_NVREG_ADDR          (0x2U)
#define NOR_CFG2_NVREG_ADDR          (0x3U)
#define NOR_CFG3_NVREG_ADDR          (0x4U)
#define NOR_CFG4_NVREG_ADDR          (0x5U)
#define NOR_CFG5_NVREG_ADDR          (0x6U)

#define NOR_STS1_VREG_ADDR           (0x800000U)
#define NOR_STS2_VREG_ADDR           (0x800001U)
#define NOR_CFG1_VREG_ADDR           (0x800002U)
#define NOR_CFG2_VREG_ADDR           (0x800003U)
#define NOR_CFG3_VREG_ADDR           (0x800004U)
#define NOR_CFG4_VREG_ADDR           (0x800005U)
#define NOR_CFG5_VREG_ADDR           (0x800006U)

/** Read ID command definitions */
#define NOR_RDID_NUM_BYTES           (0x3U)
#define NOR_MANF_ID                  (0x34U)   /* Manufacturer ID */
#define NOR_DEVICE_ID                (0x5B1A)  /* Device ID */

/** \brief OSPI controller address configuration.
 *  NOR_OSPI_ADDR_BYTES          : Number of address bytes the OSPI controller sends.
 *                                 S28HS512T supports 4-byte addressing for PHY DDR.
 *  NOR_ADDR_BYTES_PROGRAMMABLE  : 1U — device supports switching to 4-byte addressing,
 *                                 enabling PHY DDR mode on the controller.
 *                                 Set to 0U for devices with fixed 2/3-byte addressing.
 */
#define NOR_OSPI_ADDR_BYTES          (4U)
#define NOR_ADDR_BYTES_PROGRAMMABLE  (1U)

/** Status Register, Write-in-Progress bit */
#define NOR_SR_WIP			         (1U << 0U)

/** Status Register, program enabled bit */
#define NOR_SR_WRPGEN			     (1U << 1U)

/** Config Register, TBPARM bit */
#define NOR_CR_TBPARM                (1U << 2U)

/** Dummy cycles for Read operation */
#define NOR_SINGLE_READ_DUMMY_CYCLE           (0U)
#define NOR_SINGLE_CMD_READ_DUMMY_CYCLE       (1U)
#define NOR_OCTAL_SDR_CMD_READ_DUMMY_CYCLE    (3U)
#define NOR_OCTAL_DDR_CMD_READ_DUMMY_CYCLE    (4U)
#define NOR_OCTAL_READ_DUMMY_CYCLE            (24U)
#define NOR_OCTAL_READ_DUMMY_CYCLE_LC         (0xBU)
#define NOR_OCTAL_READ_DUMMY_CYCLE_INDAC      (20U)
#define NOR_OCTAL_READ_DUMMY_CYCLE_LC_INDAC   (0x8U)

#define NOR_RDID_CMD_LENGTH_SINGLE            (1U)
#define NOR_RDID_CMD_LENGTH_OCTAL             (5U)

#define NOR_XSPI_READ_DELAY_MAX               (15U)

/** In Micro seconds */
#define NOR_PAGE_PROG_TIMEOUT		(400U)
#define NOR_SECTOR_ERASE_TIMEOUT	(600U * 1000U)
#define NOR_WRR_WRITE_TIMEOUT		(600U * 1000U)
#define NOR_BULK_ERASE_TIMEOUT	    (110U * 1000U * 1000U)

#ifdef __cplusplus
}
#endif

#endif /* S25Fl512S_H_ */

/* Nothing past this point */
