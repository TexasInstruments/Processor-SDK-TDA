/**
* @file device.h
* @brief J722S device specific definitions
*
* Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*
*  Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the
*  distribution.
*
*  Neither the name of Texas Instruments Incorporated nor the names of
*  its contributors may be used to endorse or promote products derived
*  from this software without specific prior written permission.
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
*/

#ifndef __DEVICE_J722S_H
#define __DEVICE_J722S_H

#include <stdlib.h>
#include <stdint.h>              // The library uses C99 exact-width integer types

#ifdef __cplusplus
extern "C" {
#endif

/*
* Note: This code is intended for use with J722S devices only.
*
* The JTAGID (WKUP_CTRL_MMR0_JTAGID) is part of the Wakeup MMR and is required for converting
* binary trace data to CSV/TDF format using Code Composer Studio (CCS). For AEN device, the JTAGID value is 0x0bba001f.
*
* To convert binary trace output to CSV, use the following commands from the CCS emulation analysis tools directory:
*   1. Convert binary to TDF:
*      ./bin2tdf -bin /<user-path>/CPT2_ddr_throu.bin -rcvr ETB -output ddr_throughput.tdf -procid stm -devicekey 0x0bba001f -dcmfile /<user-path>/CPT2_ddr_throu.dcm
*   2. Convert TDF to CSV:
*      ./td -bin ddr_throughput.tdf -rcvr ETB -output throughput.csv -procid stm -devicekey 0x0bba001f -timestamp abs
*/

// CPTracer2 probe Offset address   (As, ProbeBase: AggBase+0x20000 + Prob#x0x1000)
#define CPT2_PROBEn_OFFSET(n)       (0x00020000 + (n)*0x0001000) // CPT2 probe N offset in Aggregator space

// Debug Cell sub-components offset address
#define TBR_OFFSET                  0x00005000 // TBR offset in Debug Cell space
#define ATBREP_OFFSET               0x00004000 // ATB Replicator offset in Debug Cell space

////////////////////////////////////////////////////////////////////
// Device Specific Definitions
////////////////////////////////////////////////////////////////////

////////// GTC definitions ///////////////////////////
// GTC base address in system memory view (GTC0_GTC_CFG1 in TRM)
#define GTC_BADDR       (0xa90000UL)   // Global time counter GTC base address, this address is 32-bit so no need to be re-mapped for R5
#define GTC_TIMER_MAPPED_SIZE       (512)

///////// Debug Cell definitions ////////////////////
// Debug Cells are present in Main Domain only
typedef enum
{
    eSOC_DBGCELL_ID = 0,                 /*!< 0 - SOC Debug Cell */
} eDebugCell_ID;

// SoC Debug Cell Base address in system memory view
#define SOC_DBGCELL_BADDR      (0x073C020000) // DEBUGSS0_DEBUG_CELL_ROM_SLV. TBR Size: 65536 bytes

#define DBGCELL_MAPPED_SIZE       (0x10000) // 64KB

// Debug Cell base address in 48-bit system memory address space. This needs to be mapped to 32-bit address for R5 view.
#define _DBGCELL_BaseAddress_raw(dc_id) ((dc_id==eSOC_DBGCELL_ID)? SOC_DBGCELL_BADDR: SOC_DBGCELL_BADDR)

#define _DBGCELL_BaseAddress(dc_id) _DBGCELL_BaseAddress_raw(dc_id)

// ETB base address
#define _ETB_BaseAddress(dc_id)		(_DBGCELL_BaseAddress(dc_id)+TBR_OFFSET)      // DEBUGSS0_TBR_VBUSP_WRAP_TBR_CFG_TBR_CFG

// ATB Replicator base address
#define _ATBREP_BaseAddress(dc_id)	(_DBGCELL_BaseAddress(dc_id)+ATBREP_OFFSET)   // DEBUGSS0_ATB_REPLICATOR_CFG_CXATBREPLICATOR_CFG


////////// CPTracer2 definitions /////////////////////
// CPT2 Aggregators IDs
typedef enum
{
    eSOC_CPT2_ID  = 0,                 /*!< 0 - Main Clock Domain CPT2 Aggregator */
} eCPT2Aggr_ID ;

// Main Memory Map Area: DEBUGSS_WRAP0_EXT_APB0
// CPTracer2 Aggregator Base address in system memory (SOC) view
#define SOC_CPT2_BADDR          (0x073E100000)      // CPT2_AGGR0_MMR

#define CPT2_AGGR_MAPPED_SIZE      (0x40000) // 256KB

#define _CPT2_BaseAddress_raw(cpt2_id) ((cpt2_id==eSOC_CPT2_ID) ? SOC_CPT2_BADDR : SOC_CPT2_BADDR)

#define _CPT2_BaseAddress(cpt2_id)  _CPT2_BaseAddress_raw(cpt2_id)

// CPTracer2 probe base address
#define _CPT2Probe_BaseAddress(cpt2_id, pb_index)	(_CPT2_BaseAddress(cpt2_id) + CPT2_PROBEn_OFFSET(pb_index))

#define NUM_CPT2_PROBES    16 // Number of CPTracer2 probes

/*! \par cpt2pb_id_t
    CPTracer2 probe IDs
*/
typedef enum {
    /* Main CPTracer2 probes (Master Id: 224 + Probe Port Index) */
    eCpTracer2_Probe_0,        /**< 0 - "A53SS0 Read Initiator (Read transactions originating from A53SS0)" */
	eCpTracer2_Probe_1,        /**< 1 - "A53SS0 Write Initiator (Write transactions originating from A53SS0)" */
	eCpTracer2_Probe_2,        /**< 2 - "A53SS0 ACP Target (A53SS0 ACP interface)" */
	eCpTracer2_Probe_3,        /**< 3 - "DDR RT Target (DDR Real-Time interface)" */
	eCpTracer2_Probe_4,        /**< 4 - "DDR NRT Target (DDR Non-Real-Time interface)" */
	eCpTracer2_Probe_5,        /**< 5 - "GPMC Target Port" */
	eCpTracer2_Probe_6,        /**< 6 - "FSS Target Port" */
	eCpTracer2_Probe_7,        /**< 7 - "DSS and DMSS CSI Initiator" */
	eCpTracer2_Probe_8,        /**< 8 - "McASP Target" */
    eCpTracer2_Probe_9,        /**< 9 - "Device Manager Target" */
    eCpTracer2_Probe_10,       /**< 10 - "OCSRAM (MAIN) Target" */
    eCpTracer2_Probe_11,       /**< 11 - "Video Encoder Initiator Port" */
    eCpTracer2_Probe_12,       /**< 12 - "GPU Read Initiator (Read transactions issued by GPU)" */
    eCpTracer2_Probe_13,       /**< 13 - "GPU Write Initiator (Write transactions issued by GPU)" */
	eCpTracer2_Probe_14,       /**< 14 - "DDR RT Target (2nd Probe)" */
    eCpTracer2_Probe_15,       /**< 15 - "DDR NRT Target (2nd Probe)" */

} cpt2pb_id_t;

// DDR probe
#define eCpTracer2_DDR_Probe eCpTracer2_Probe_4

// CPTracer2 data type definitions
typedef struct {
    cpt2pb_id_t         probe_id; /* CPT2 probe ID */
    uint8_t             probe_port_index; /* Port number in CPT2 Aggregator */
    eDebugCell_ID       dbgcell_id; /* Debug Cell ID */
    uint64_t            dbgcell_baddr_raw; /* raw Debug Cell base address in 48-bit system memory address space */
    uint64_t            dbgcell_baddr; /* Debug Cell base address in the CPU view. For R5, this would be the RAT mapped 32-bit address; For A53, this would be the same as dbgcell_baddr_raw */
    eCPT2Aggr_ID        aggr_id; /* CPT2 Aggregator ID */
    uint64_t            aggr_baddr_raw; /* CPT2 Aggregator base address in 48-bit system memory address space */
    uint64_t            aggr_baddr; /* CPT2 Aggregator base address in the CPU view. For R5, this would be the RAT mapped 32-bit address; For A53, this would be the same as aggr_baddr_raw */
    uint8_t             master_id; /* Master ID for the CPT2 probe; this needs to match with the definition in Keystone3 platform database file in emupack */
} cpt2_access_t;

static cpt2_access_t g_cpt2_table[NUM_CPT2_PROBES] __attribute__((unused)) =
{
    { eCpTracer2_Probe_0, 0, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 224 },
    { eCpTracer2_Probe_1, 1, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 225 },
    { eCpTracer2_Probe_2, 2, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 226 },
    { eCpTracer2_Probe_3, 3, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 227 },
    { eCpTracer2_Probe_4, 4, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 228 },
    { eCpTracer2_Probe_5, 5, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 229 },
    { eCpTracer2_Probe_6, 6, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 230 },
    { eCpTracer2_Probe_7, 7, eSOC_DBGCELL_ID,  _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 231 },
    { eCpTracer2_Probe_8, 9, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 232 },
    { eCpTracer2_Probe_9, 10, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 233 },
    { eCpTracer2_Probe_10, 17, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 234 },
    { eCpTracer2_Probe_11, 19, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 235 },
    { eCpTracer2_Probe_12, 20, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 236 },
    { eCpTracer2_Probe_13, 21, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 237 },
    { eCpTracer2_Probe_14, 23, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 238 },
    { eCpTracer2_Probe_15, 27, eSOC_DBGCELL_ID, _DBGCELL_BaseAddress_raw(eSOC_DBGCELL_ID), _DBGCELL_BaseAddress(eSOC_DBGCELL_ID), eSOC_CPT2_ID, _CPT2_BaseAddress_raw(eSOC_CPT2_ID), _CPT2_BaseAddress(eSOC_CPT2_ID), 240 },

};

#ifdef __cplusplus
}
#endif

#endif 

/* 
* If we want to filter multiple RouteIDs, we can use the Route ID mask. 
* Say, If you wanna track all the transactions from C7x core, you can use the mask: 0xFF0
* If you wanna track all the transactions from only one of C7x core only, you can use the mask: 0xFFC
*/

// ;;;;;;;;;;;;;;;;;;;;;;;;;;
// TDA4AEN list of RouteIDs
// ;;;;;;;;;;;;;;;;;;;;;;;;;;
// A53-CORE0-nc-write 0 (0x00)
// A53-CORE1-nc-write 1 (0x01)
// A53-CORE0-nc-write 2 (0x02)
// A53-CORE1-nc=write 3 (0x03)
// A53-L2Cache-write 4 (0x04)
// A53-CORE0-nc-read 16 (0x10)
// A53-CORE1-nc-read 17 (0x11)
// A53-CORE0-nc-read 18 (0x12)
// A53-CORE1-nc=read 19 (0x13)
// A53-L2Cache-read 20 (0x14)
// A53-ACP-read 21 (0x15)

// C7x-CORE0-DMC 32 (0x20)
// C7x-CORE0-PMC 33 (0x21)
// C7x-CORE0-MMU 34 (0x22)
// C7x-CORE0-SE0 35 (0x23)
// C7x-CORE0-SE1 36 (0x24)
// C7x-CORE0-DRU-read 37 (0x25)
// C7x-CORE0-DRU-write 38 (0x26)
// C7x-CORE1-DMC 48 (0x30)
// C7x-CORE1-PMC 49 (0x31)
// C7x-CORE1-MMU 50 (0x32)
// C7x-CORE1-SE0 51 (0x33)
// C7x-CORE1-SE1 52 (0x34)
// C7x-CORE1-DRU-read 53 (0x35)
// C7x-CORE1-DRU-write 54 (0x36)

// GPU0_M0_WR 64 (0x40)
// GPU0_M0_RD 65 (0x41)
// JPG_e5010_WR 66 (0x42)
// JPG_e5010_RD 67 (0x43)
// DMSS-BCDMA_RD 128 (0x80)
// DMSS-BCDMA_WR 129 (0x81)
// DMSS-PKDMA 130 (0x82)
// DSS_MAIN0 160 (0xA0)
// DSS_MAIN1 161 (0xA1)
// MMCSD0_RD 256 (0x100)
// MMCSD0_WR 257 (0x101)
// DEBUGSS0_RD 258 (0x102)
// DEBUGSS0_WR 259 (0x103)
// PCIE0_RD 267 (0x10B)
// PCIE0_WR 268 (0x10C)
// MMCSD1_RD 269 (0x10D)
// MMCSD1_WR 270 (0x10E)
// MMCSD2_RD 271 (0x10F)
// MMCSD2_WR 272 (0x110)
// SAUL 277 (0x115)
// GIC_WR  302 (0x12E)
// GIC_RD  303 (0x12F)
// USB0_RD 304 (0x130)
// USB0_WR 305 (0x131)
// USB1_RD 306 (0x132)
// USB1_WR 307 (0x133)
// DMSS-CSI_RD 308 (0x134)
// DMSS-CSI_WR 309 (0x135)
// WAVE321_PRI_RD 310 (0x136)
// WAVE321_PRI_WR 311 (0x137)
// WAVE321_SEC_RD 312 (0x138)
// WAVE321_SEC_WR 313 (0x139)
// VPAC_LDC 314 (0x13A)
// VPAC_DATA0a 316 (0x13C)
// VPAC_DATA0b 317 (0x13D)
// VPAC_DATA1a 318 (0x13E)
// VPAC_DATA1b 319 (0x13F)
// USB2_RD 321 (0x141)
// USB2_WR 322 (0x142)
// LED 334 (0x14E)

// TIFS-M4-IBUS 448 (0x1C0)
// TIFS-M4-DBUS 449 (0x1C1)
// TIFS-M4-SBUS 450 (0x1C2)
// HSM-M4-IBUS 448 (0x1C0)
// HSM-M4-DBUS 449 (0x1C1)
// HSM-M4-SBUS 450 (0x1C2)

// PDMA2_MCASP_WR 3001 (0xBB9)
// PDMA2_MCASP_RD 3002 (0xBBA)
// PDMA0_SPI_WR 3010 (0xBC2)
// PDMA0_SPI_RD 3011 (0xBC3)
// PDMA1_UART_WR 3012 (0xBC4)
// PDMA1_UART_RD 3013 (0xBC5)

// MAIN_R5FSS0_CORE0_MEM_WR 4084 (0xFF4)
// MAIN_R5FSS0_CORE0_MEM_RD 4085 (0xFF5)
// MAIN_R5FSS0_CORE0_PER0 4086 (0xFF6)
// DM_R5FSS0_CORE0_MEM_WR 4088 (0xFF8)
// DM_R5FSS0_CORE0_MEM_RD 4089 (0xFF9)
// DM_R5FSS0_CORE0_PER0 4090 (0xFFA)
// SA3_PKDMA 4091 (0xFFB)
// MCU_R5FSS0_CORE0_MEM_WR 4092 (0xFFC)
// MCU_R5FSS0_CORE0_MEM_RD 4093 (0xFFD)
// MCU_R5FSS0_CORE0_PER0 4094 (0xFFE)
