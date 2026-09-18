/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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
 * \file  w35n01jwtbag.h
 *
 * \brief This file contains W35N01JWTBAG NAND device definitions
 *
 *****************************************************************************/
#ifndef W35N01JWTBAG_H_
#define W35N01JWTBAG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 **                       Macro Definitions
 **************************************************************************/

/** \brief FLASH device specific items (note: sizes are in bytes) */
#define NAND_BLOCK_SIZE               (256U * 1024U)
#define NAND_SECTOR_SIZE              (0U)                          /* TO DO */
#define NAND_SIZE                     (128U * 1024U * 1024U)
#define NAND_NUM_SECTORS              (0U)                          /* TO DO */
#define NAND_NUM_BLOCKS               (NAND_SIZE / NAND_BLOCK_SIZE)
#define NAND_PAGE_SIZE                (4U * 1024U)
#define NAND_NUM_PAGES_PER_BLOCK      (NAND_BLOCK_SIZE / NAND_PAGE_SIZE)
#define NAND_SPARE_AREA_SIZE          (128U)

/** \brief Flash device commands */
#define NAND_CMD_RSTEN                (0x66U)
#define NAND_CMD_RST_MEM              (0x99U)
#define NAND_CMD_RDID                 (0x9FU)
#define NAND_CMD_RDSR                 (0x0FU)
#define NAND_CMD_WRITE_STATUS         (0x1FU)
#define NAND_CMD_VCR_READ             (0x85U)
#define NAND_CMD_WRITE_VCR            (0x81U)
#define NAND_CMD_WREN                 (0x06U)
#define NAND_CMD_WR_DISABLE           (0x04U)
#define NAND_CMD_BBM                  (0xA1U)
#define NAND_CMD_READ_BBM_LUT         (0xA5U)
#define NAND_CMD_LAST_ECC_ADD         (0xA9U)
#define NAND_CMD_BLOCK_ERASE          (0xD8U)
#define NAND_CMD_PAGE_READ            (0x13U)
#define NAND_CMD_READ                 (0x03U)
#define NAND_CMD_FAST_READ            (0x0BU)
#define NAND_CMD_OCTAL_DDR_O_FAST_RD  (0x8BU)
#define NAND_CMD_OCTAL_IO_FAST_RD     (0xCBU)
#define NAND_CMD_FAST_READ_DDR        (0x9DU)
#define NAND_CMD_PAGE_PROG            (0x84U)
#define NAND_CMD_PAGE_PROG_EXECUTE    (0x10U)
#define NAND_CMD_EXT_OCTAL_FAST_PROG  (0x82U)
#define NAND_CMD_OCTAL_FAST_PROG      (0xC4U)

/** Dummy cycles for Read operation */
#define NAND_OCTAL_READ_DUMMY_CYCLE             (0x08U)
#define NAND_OCTAL_READ_DUMMY_CYCLE_LC          (0x08U)
#define NAND_OCTAL_READ_DUMMY_CYCLE_INDAC       (0x08U)
#define NAND_OCTAL_READ_DUMMY_CYCLE_INDAC_LC    (0x08U)
#define NAND_SINGLE_READ_DUMMY_CYCLE            (0x08U)
#define NAND_SINGLE_READ_DUMMY_CYCLE_LC         (0x08U)
#define NAND_OCTAL_DDR_CMD_READ_DUMMY_CYCLE     (0x08U)
#define NAND_OCTAL_SDR_CMD_READ_DUMMY_CYCLE     (0x08U)
#define NAND_SINGLE_CMD_READ_DUMMY_CYCLE        (0x08U)

/** In Micro seconds */
#define NAND_PAGE_PROG_TIMEOUT		  (1000U)              /* TO DO */
#define NAND_SECTOR_ERASE_TIMEOUT	  (1000U)              /* TO DO */
#define NAND_WRR_WRITE_TIMEOUT		  (1000U)              /* TO DO */
#define NAND_BULK_ERASE_TIMEOUT       (1000U)              /* TO DO */

/* \brief Read ID command definitions */
#define NAND_RDID_NUM_BYTES           (0x03U)
#define NAND_MANF_ID                  (0xEFU)   /* Manufacturer ID */
#define NAND_DEVICE_ID                (0xDC21U) /* Device ID */

/** \brief OSPI controller address configuration.
 *  NAND_OSPI_ADDR_BYTES         : Number of address bytes the OSPI controller sends
 *                                 for memory operations. Fixed by array geometry:
 *                                 Column CA[12:0] + Page PA[15:0] both fit in 2 bytes.
 *  NAND_ADDR_BYTES_PROGRAMMABLE : Whether the device supports switching address bytes
 *                                 to 4 bytes. This device has fixed 2-byte addressing
 *                                 determined by array geometry - not programmable.
 *                                 Note: Nand_ospiSetOpcode() always configures the
 *                                 controller for 2-byte addressing so DDR and PHY DDR
 *                                 modes work correctly regardless of this setting.
 *                                 Set to 1U for devices that support address byte switching.
 */
#define NAND_OSPI_ADDR_BYTES          (2U)
#define NAND_ADDR_BYTES_PROGRAMMABLE  (0U)

/** Status Register, Write-in-Progress bit */
#define NAND_SR1_ADDR                 (0xA0U)
#define NAND_SR2_ADDR                 (0xB0U)
#define NAND_SR3_ADDR                 (0xC0U)

/** Status Register, Write-in-Progress bit */
#define NAND_SR_WIP			          (1U << 0U)

#ifdef __cplusplus
}
#endif

#endif /* W35N01JWTBAG_H_ */

/* Nothing past this point */
