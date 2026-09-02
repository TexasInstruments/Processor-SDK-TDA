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

-stack  0x2000      /* SOFTWARE STACK SIZE */
-heap   0x1000      /* HEAP AREA SIZE      */
--symbol_map _Hwi_intcVectorTable=Hwi_intcVectorTable

SECTIONS
{
    .hwi_vect: {. = align(32); } > DDR_C66x_2 ALIGN(0x400)
    .text:csl_entry:{}           > DDR_C66x_2
    .text:_c_int00          load > DDR_C66x_2 ALIGN(0x400)
    .text:                       > DDR_C66x_2
    .stack:                      > DDR_C66x_2
    GROUP:                       > DDR_C66x_2
    {
        .bss: {}
        RUN_START(__BSS_START)
        RUN_END(__BSS_END)
        .neardata:
        .rodata:
    }
    .cio:                        > DDR_C66x_2
    .const:                      > DDR_C66x_2
    .data:                       > DDR_C66x_2
    .switch:                     > DDR_C66x_2
    .sysmem:                     > DDR_C66x_2
    .far:                        > DDR_C66x_2
    .args:                       > DDR_C66x_2
    .ppinfo:                     > DDR_C66x_2
    .ppdata:                     > DDR_C66x_2
    .ti.decompress:              > DDR_C66x_2
    .ti.handler_table:           > DDR_C66x_2

    /* COFF sections */
    .pinit:                      > DDR_C66x_2
    .cinit:                      > DDR_C66x_2

    /* EABI sections */
    .binit:                      > DDR_C66x_2
    .init_array:                 > DDR_C66x_2
    .fardata:                    > DDR_C66x_2
    .c6xabi.exidx:               > DDR_C66x_2
    .c6xabi.extab:               > DDR_C66x_2

    .csl_vect:                   > DDR_C66x_2
  
    ipc_data_buffer:             > DDR_C66x_2 type=NOLOAD
    .resource_table: 
    { 
        __RESOURCE_TABLE = .;
    }                            > DDR_C66x_2_RESOURCE_TABLE

    .tracebuf : {} align(1024)   > DDR_C66x_2

    .bss:taskStackSection                 > DDR_C66x_2
    .bss:ddr_local_mem      (NOLOAD) : {} > DDR_C66X_2_LOCAL_HEAP
    .bss:ddr_scratch_mem    (NOLOAD) : {} > DDR_C66X_2_SCRATCH
    .bss:app_log_mem        (NOLOAD) : {} > APP_LOG_MEM
    .bss:app_fileio_mem     (NOLOAD) : {} > APP_FILEIO_MEM
    .bss:tiovx_obj_desc_mem (NOLOAD) : {} > TIOVX_OBJ_DESC_MEM
    .bss:ipc_vring_mem      (NOLOAD) : {} > IPC_VRING_MEM

    .bss:l2mem              (NOLOAD)(NOINIT) : {} > L2RAM_C66x_2
}

