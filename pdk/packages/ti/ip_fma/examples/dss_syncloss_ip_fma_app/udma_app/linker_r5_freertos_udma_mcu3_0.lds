/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

/*=========================*/
/*     Linker Settings     */
/*=========================*/

--retain="*(.bootCode)"
--retain="*(.startupCode)"
--retain="*(.startupData)"
--retain="*(.irqStack)"
--retain="*(.fiqStack)"
--retain="*(.abortStack)"
--retain="*(.undStack)"
--retain="*(.svcStack)"

--fill_value=0
--stack_size=0x4000
--heap_size=0x8000
--entry_point=_freertosresetvectors

-stack  0x4000  /* SOFTWARE STACK SIZE */
-heap   0x8000  /* HEAP AREA SIZE      */

/*-------------------------------------------*/
/*       Stack Sizes for various modes       */
/*-------------------------------------------*/
__IRQ_STACK_SIZE   = 0x1000;
__FIQ_STACK_SIZE   = 0x0100;
__ABORT_STACK_SIZE = 0x0100;
__UND_STACK_SIZE   = 0x0100;
__SVC_STACK_SIZE   = 0x0100;

/*--------------------------------------------------------------------------*/
/*                               Memory Map                                 */
/*--------------------------------------------------------------------------*/
MEMORY
{
    VECTORS (X)                 : ORIGIN = 0x00000000 LENGTH = 0x00000040

    MCU0_R5F_TCMA  (X)          : ORIGIN = 0x00000040 LENGTH = 0x00007FC0
    MCU0_R5F_TCMB0 (RWIX)       : ORIGIN = 0x41010000 LENGTH = 0x00008000

    MCU0_R5F1_ATCM (RWIX)       : ORIGIN = 0x41400000 LENGTH = 0x00008000
    MCU0_R5F1_BTCM (RWIX)       : ORIGIN = 0x41410000 LENGTH = 0x00008000

    OCMC_RAM_BOARD_CFG   (RWIX) : ORIGIN = 0x41C80000 LENGTH = 0x00002000
    OCMC_RAM_SCISERVER   (RWIX) : ORIGIN = 0x41C82000 LENGTH = 0x00060000
    OCMC_RAM             (RWIX) : ORIGIN = 0x41CE3100 LENGTH = 0x00008000  /* 32 KB: 0x41CE3100–0x41CEB0FF */
    OCMC_RAM_X509_HEADER (RWIX) : ORIGIN = 0x41CFDB00 LENGTH = 0x00000500

    MSMC3_ARM_FW  (RWIX)        : ORIGIN = 0x70000000 LENGTH = 0x00020000
    MSMC3         (RWIX)        : ORIGIN = 0x70020000 LENGTH = 0x003D0000
    MSMC3_DMSC_FW (RWIX)        : ORIGIN = 0x703F0000 LENGTH = 0x00010000

    DDR0 (RWIX)                 : ORIGIN = 0x80000000 LENGTH = 0x20000000  /* 512 MB: 0x80000000–0x9FFFFFFF */
}

/*--------------------------------------------------------------*/
/*                     Section Configuration                    */
/*--------------------------------------------------------------*/
SECTIONS
{
    .freertosrstvectors      : {} palign(8)      > VECTORS
    .bootCode        : {} palign(8)      > OCMC_RAM
    .startupCode     : {} palign(8)      > OCMC_RAM
    .startupData     : {} palign(8)      > OCMC_RAM, type = NOINIT
    .text            : {} palign(8)      > DDR0
    GROUP {
        .text.hwi    : palign(8)
        .text.cache  : palign(8)
        .text.mpu    : palign(8)
        .text.boot   : palign(8)
    }                                    > DDR0
    .const           : {} palign(8)      > DDR0
    .rodata          : {} palign(8)      > DDR0
    .cinit           : {} palign(8)      > DDR0
    .bss             : {} align(4)       > DDR0
    .far             : {} align(4)       > DDR0
    .data            : {} palign(128)    > DDR0
    .sysmem          : {}                > DDR0
    .data_buffer     : {} palign(128)    > DDR0
    .bss.devgroup    : {*(.bss.devgroup*)} align(4)       > DDR0
    .const.devgroup  : {*(.const.devgroup*)} align(4)     > DDR0
    .boardcfg_data   : {} align(4)       > DDR0

    /* USB or any other LLD buffer for benchmarking */
    .benchmark_buffer (NOLOAD) {} ALIGN (8) > DDR0

    .stack      : {} align(4)                               > DDR0  (HIGH)

    .irqStack   : {. = . + __IRQ_STACK_SIZE;} align(4)      > DDR0  (HIGH)
    RUN_START(__IRQ_STACK_START)
    RUN_END(__IRQ_STACK_END)

    .fiqStack   : {. = . + __FIQ_STACK_SIZE;} align(4)      > DDR0  (HIGH)
    RUN_START(__FIQ_STACK_START)
    RUN_END(__FIQ_STACK_END)

    .abortStack : {. = . + __ABORT_STACK_SIZE;} align(4)    > DDR0  (HIGH)
    RUN_START(__ABORT_STACK_START)
    RUN_END(__ABORT_STACK_END)

    .undStack   : {. = . + __UND_STACK_SIZE;} align(4)      > DDR0  (HIGH)
    RUN_START(__UND_STACK_START)
    RUN_END(__UND_STACK_END)

    .svcStack   : {. = . + __SVC_STACK_SIZE;} align(4)      > DDR0  (HIGH)
    RUN_START(__SVC_STACK_START)
    RUN_END(__SVC_STACK_END)
}