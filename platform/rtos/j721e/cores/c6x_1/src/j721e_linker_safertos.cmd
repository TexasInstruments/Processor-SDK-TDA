/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#define DEFAULT_SECTION_ALIGNMENT   ( 1K )

-stack  0x2000      /* SOFTWARE STACK SIZE */
-heap   0x1000      /* HEAP AREA SIZE      */
-u InterruptVectorTable

#define DDR_C66x_1_START 0xA8200400

SECTIONS
{
   .interrupt_vectors : {. = align(1K); }  > DDR_C66x_1_BOOT 
   .priv_code           palign( DEFAULT_SECTION_ALIGNMENT ),
                        LOAD_START( lnkKernelFuncStartAddr ),
                        LOAD_END( lnkKernelFuncEndAddr ) :
    {
        *(.KERNEL_FUNCTION)
    } > DDR_C66x_1

    .text:_c_int00      load >  DDR_C66x_1 ALIGN(0x10000)
    .text:              load >  DDR_C66x_1 palign( DEFAULT_SECTION_ALIGNMENT ),
                                     LOAD_START( lnkStartFlashAddress )

    .unpriv_rodata      palign( DEFAULT_SECTION_ALIGNMENT ) :
    {
        *(.const)
        *(.switch)
    } > DDR_C66x_1

    .cinit              LOAD_END( lnkEndFlashAddress )          : {} > DDR_C66x_1

    .KERNEL_DATA        palign( DEFAULT_SECTION_ALIGNMENT ),
                        LOAD_START( lnkKernelDataStartAddr ),
                        LOAD_END( lnkKernelDataEndAddr )        : {} > DDR_C66x_1_START

    GROUP               palign( DEFAULT_SECTION_ALIGNMENT ),
                        LOAD_START( lnkUnprivilegedDataStartAddr ),
                        LOAD_END( lnkUnprivilegedDataEndAddr )
    {
        .rodata:
        .bss: {}
        RUN_START(__BSS_START)
        RUN_END(__BSS_END)
        .neardata:
        .cio:
        .data:
        .sysmem:
        .fardata:
        .far:
    } > DDR_C66x_1

    .stack              palign( DEFAULT_SECTION_ALIGNMENT ),
                        LOAD_START( lnkStacksStartAddr )        : {} > DDR_C66x_1 
  
    ipc_data_buffer:             > DDR_C66x_1 type=NOLOAD
    .resource_table: 
    { 
        __RESOURCE_TABLE = .;
    }                            > DDR_C66x_1_RESOURCE_TABLE

    .tracebuf : {} align(1024)   > DDR_C66x_1

    .bss:taskStackSection                 > DDR_C66x_1
    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_C66X_1_LOCAL_HEAP
    .bss:ddr_scratch_mem    (NOLOAD) : {} > DDR_C66X_1_SCRATCH
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    .bss:l2mem              (NOLOAD)(NOINIT) : {} > L2RAM_C66x_1
}

