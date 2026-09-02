#!/usr/bin/env python3
#
# Copyright (c) 2018-2026 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

#
# This script is used to generate 'MEMORY' section for multiple CPUs
# in different CPU specific linker command files.
#
# This helps to define the memory map in one file vs having to manually
# keep the system memory map consistant across multiple CPUs
# in different linker commnd files.
#
# Make sure PyTI_PSDK_RTOS module is installed before running this script.
# See vision_apps/tools/PyTI_PSDK_RTOS/README.txt to install PyTI_PSDK_RTOS module.
#
# Edit this file to change the memory map
#
# Run this script by doing below,
# ./gen_linker_mem_map.py
#
# This will generate linker command file at below folders
# ./<cpu name>/linker_mem_map.cmd
#
# Here CPU name is mpu1, c7x_1, c7x_2, c7x_3, c7x_4, dmcu0, mcu1, mcu3, rmcu0_0
#
#
from ti_psdk_rtos_tools import *

KB = 1024;
MB = KB*KB;
GB = KB*MB;

SHARED_MEM_SIZE = 512*MB;

#
# Notes,
# - recommend to keep all memory segment sizes in units of KB at least
#

#
# TODO: On TDA54 EVM, there are 2 DDR chipsets
#
# lower DDR address starts at 0x0000_8000_0000
# higher DDR address starts at 0x0008_8000_0000
#
# As the address is non-contiguous it requires MMU to remap the address
# Currently the upper 2GB is accessed by either ARM (user space)
# C7x DSP heap/scratch space, as 32-bit cores like M55 cannot access it
#
# The upper DDR address is mapped contiguously from lower DDR address
# but remapped to actual physical address using MMU
#
# physical lower DDR address range 0x0000_8000_0000
# virtual lower DDR address range 0x0000_8000_0000
#
# physical lower DDR address range 0x0008_8000_0000
# virtual lower DDR address range 0x0001_0000_0000
#

ddr_mem_addr  = 0xa0000000;
ddr_mem_size  = 1*GB + 448*MB; # Last 64MB is used by Linux

ddr_mem_addr_hi_phys = 0x2080000000;
ddr_mem_addr_hi = 0x100000000;
ddr_mem_size_hi = 256*MB;

msmc_mem_addr = 0x68000000;
msmc_size = 24*MB
#Assumption of main OCRAM virtual address
main_ocram_mem_addr = 0x60000000; # Note: uses RAT to translate to proper address
main_ocram_mem_addr_phys = 0x23800000;

#TODO:
#codec_carveout_size = 2*GB;

#TODO:
uboot_reloc_mem_addr = 0xDC000000;
uboot_reloc_mem_size = 64*MB;
#
# Other constant sizes
#
linux_ddr_ipc_size = 1*MB;
linux_ddr_resource_table_size = 1*KB;
linux_ddr_ipc_trace_size = 1*MB - 1*KB;

#TODO:
# C7x L1, L2 memory allocation
# L1 - 32KB $, 16KB SRAM
# L2 - 64KB $, 448KB SRAM
c7x_1_l2_addr  = 0x64000000;
c7x_1_l2_size  = 3*MB;
c7x_1_msmc_mem_addr = 0x68000000;
c7x_1_msmc_size = 6*MB;
#c7x_1_l1_addr    = 0x64E00000;
#c7x_1_l1_size    = 16*KB;

#TODO:
# C7x L1, L2 memory allocation
# L1 - 32KB $, 16KB SRAM
# L2 - 64KB $, 448KB SRAM
c7x_2_l2_addr  = 0x65000000;
c7x_2_l2_size  = 3*MB;
c7x_2_msmc_mem_addr = c7x_1_msmc_mem_addr + c7x_1_msmc_size;
c7x_2_msmc_size = 6*MB;
#c7x_2_l1_addr    = 0x65E00000;
#c7x_2_l1_size    = 16*KB;

#TODO:
# C7x L1, L2 memory allocation
# L1 - 32KB $, 16KB SRAM
# L2 - 64KB $, 448KB SRAM
#c7x_3_l1_addr    = 0x66E00000;
#c7x_3_l1_size    = 16*KB;
c7x_3_l2_addr  = 0x66000000;
c7x_3_l2_size  = 3*MB;
c7x_3_msmc_mem_addr = c7x_2_msmc_mem_addr + c7x_2_msmc_size;
c7x_3_msmc_size = 6*MB;

#TODO:
# C7x L1, L2 memory allocation
# L1 - 32KB $, 16KB SRAM
# L2 - 64KB $, 448KB SRAM
#c7x_4_l1_addr    = 0x67E00000;
#c7x_4_l1_size    = 16*KB;
c7x_4_l2_addr  = 0x67000000;
c7x_4_l2_size  = 3*MB;
c7x_4_msmc_mem_addr = c7x_3_msmc_mem_addr + c7x_3_msmc_size;
c7x_4_msmc_size = 6*MB;

#TODO:
# Main OCRAM memory allocation
#
mcu2_main_ocram_addr       = main_ocram_mem_addr;
mcu2_main_ocram_addr_phys  = main_ocram_mem_addr_phys;
mcu2_main_ocram_size       = 2*MB;

mcu3_main_ocram_addr       = mcu2_main_ocram_addr + mcu2_main_ocram_size;
mcu3_main_ocram_addr_phys  = mcu2_main_ocram_addr_phys + mcu2_main_ocram_size;
mcu3_main_ocram_size       = 2*MB;

mcu4_main_ocram_addr       = mcu3_main_ocram_addr + mcu3_main_ocram_size;
mcu4_main_ocram_addr_phys  = mcu3_main_ocram_addr_phys + mcu3_main_ocram_size;
mcu4_main_ocram_size       = 2*MB;

#
# DDR memory allocation for various CPUs
#
dmcu0_ddr_addr = ddr_mem_addr;
dmcu0_ddr_size = 16*MB;

mcu0_ddr_ipc_addr = dmcu0_ddr_addr + dmcu0_ddr_size;
mcu0_ddr_resource_table_addr = mcu0_ddr_ipc_addr + linux_ddr_ipc_size;
mcu0_ddr_ipc_tracebuf_addr = mcu0_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu0_ddr_addr = mcu0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu0_ddr_size = 32*MB - (mcu0_ddr_addr-mcu0_ddr_ipc_addr);

mcu1_ddr_ipc_addr = mcu0_ddr_addr + mcu0_ddr_size;
mcu1_ddr_resource_table_addr = mcu1_ddr_ipc_addr + linux_ddr_ipc_size;
mcu1_ddr_ipc_tracebuf_addr = mcu1_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu1_ddr_addr = mcu1_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu1_ddr_size = 32*MB - (mcu1_ddr_addr-mcu1_ddr_ipc_addr);

mcu2_ddr_ipc_addr = mcu1_ddr_addr + mcu1_ddr_size;
mcu2_ddr_resource_table_addr = mcu2_ddr_ipc_addr + linux_ddr_ipc_size;
mcu2_ddr_ipc_tracebuf_addr = mcu2_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu2_ddr_addr = mcu2_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu2_ddr_size = 32*MB - (mcu2_ddr_addr-mcu2_ddr_ipc_addr);

mcu3_ddr_ipc_addr = mcu2_ddr_addr + mcu2_ddr_size;
mcu3_ddr_resource_table_addr = mcu3_ddr_ipc_addr + linux_ddr_ipc_size;
mcu3_ddr_ipc_tracebuf_addr = mcu3_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu3_ddr_addr = mcu3_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu3_ddr_size = 32*MB - (mcu3_ddr_addr-mcu3_ddr_ipc_addr);

mcu4_ddr_ipc_addr = mcu3_ddr_addr + mcu3_ddr_size;
mcu4_ddr_resource_table_addr = mcu4_ddr_ipc_addr + linux_ddr_ipc_size;
mcu4_ddr_ipc_tracebuf_addr = mcu4_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu4_ddr_addr = mcu4_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu4_ddr_size = 32*MB - (mcu4_ddr_addr-mcu4_ddr_ipc_addr);

rmcu0_0_ddr_ipc_addr = mcu4_ddr_addr + mcu4_ddr_size;
rmcu0_0_ddr_resource_table_addr = rmcu0_0_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu0_0_ddr_ipc_tracebuf_addr = rmcu0_0_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu0_0_ddr_addr = rmcu0_0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu0_0_ddr_size = 16*MB - (rmcu0_0_ddr_addr-rmcu0_0_ddr_ipc_addr);

rmcu0_1_ddr_ipc_addr = rmcu0_0_ddr_addr + rmcu0_0_ddr_size;
rmcu0_1_ddr_resource_table_addr = rmcu0_1_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu0_1_ddr_ipc_tracebuf_addr = rmcu0_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu0_1_ddr_addr = rmcu0_1_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu0_1_ddr_size = 16*MB - (rmcu0_1_ddr_addr-rmcu0_1_ddr_ipc_addr);

rmcu1_0_ddr_ipc_addr = rmcu0_1_ddr_addr + rmcu0_1_ddr_size;
rmcu1_0_ddr_resource_table_addr = rmcu1_0_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu1_0_ddr_ipc_tracebuf_addr = rmcu1_0_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu1_0_ddr_addr = rmcu1_0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu1_0_ddr_size = 16*MB - (rmcu1_0_ddr_addr-rmcu1_0_ddr_ipc_addr);

rmcu1_1_ddr_ipc_addr = rmcu1_0_ddr_addr + rmcu1_0_ddr_size;
rmcu1_1_ddr_resource_table_addr = rmcu1_1_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu1_1_ddr_ipc_tracebuf_addr = rmcu1_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu1_1_ddr_addr = rmcu1_1_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu1_1_ddr_size = 16*MB - (rmcu1_1_ddr_addr-rmcu1_1_ddr_ipc_addr);

rmcu2_0_ddr_ipc_addr = rmcu1_1_ddr_addr + rmcu1_1_ddr_size;
rmcu2_0_ddr_resource_table_addr = rmcu2_0_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu2_0_ddr_ipc_tracebuf_addr = rmcu2_0_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu2_0_ddr_addr = rmcu2_0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu2_0_ddr_size = 16*MB - (rmcu2_0_ddr_addr-rmcu2_0_ddr_ipc_addr);

rmcu2_1_ddr_ipc_addr = rmcu2_0_ddr_addr + rmcu2_0_ddr_size;
rmcu2_1_ddr_resource_table_addr = rmcu2_1_ddr_ipc_addr + linux_ddr_ipc_size;
rmcu2_1_ddr_ipc_tracebuf_addr = rmcu2_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
rmcu2_1_ddr_addr = rmcu2_1_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
rmcu2_1_ddr_size = 16*MB - (rmcu2_1_ddr_addr-rmcu2_1_ddr_ipc_addr);

c7x_1_ddr_ipc_addr = rmcu2_1_ddr_addr + rmcu2_1_ddr_size;
c7x_1_ddr_resource_table_addr = c7x_1_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_1_ddr_ipc_trace_addr = c7x_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_1_ddr_boot_addr = c7x_1_ddr_resource_table_addr + 1*MB;
c7x_1_ddr_boot_size = 1*KB;
c7x_1_ddr_vecs_addr = c7x_1_ddr_resource_table_addr + 3*MB;
c7x_1_ddr_vecs_size = 16*KB;
c7x_1_ddr_secure_vecs_addr = c7x_1_ddr_resource_table_addr + 5*MB;
c7x_1_ddr_secure_vecs_size = 16*KB;
c7x_1_ddr_addr = c7x_1_ddr_resource_table_addr + 7*MB;
c7x_1_ddr_size = 24*MB;

c7x_2_ddr_ipc_addr = c7x_1_ddr_addr + c7x_1_ddr_size;
c7x_2_ddr_resource_table_addr = c7x_2_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_2_ddr_ipc_trace_addr = c7x_2_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_2_ddr_boot_addr = c7x_2_ddr_resource_table_addr + 1*MB;
c7x_2_ddr_boot_size = 1*KB;
c7x_2_ddr_vecs_addr = c7x_2_ddr_resource_table_addr + 3*MB;
c7x_2_ddr_vecs_size = 16*KB;
c7x_2_ddr_secure_vecs_addr = c7x_2_ddr_resource_table_addr + 5*MB;
c7x_2_ddr_secure_vecs_size = 16*KB;
c7x_2_ddr_addr = c7x_2_ddr_resource_table_addr + 7*MB;
c7x_2_ddr_size = 24*MB;

c7x_3_ddr_ipc_addr = c7x_2_ddr_addr + c7x_2_ddr_size;
c7x_3_ddr_resource_table_addr = c7x_3_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_3_ddr_ipc_trace_addr = c7x_3_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_3_ddr_boot_addr = c7x_3_ddr_resource_table_addr + 1*MB;
c7x_3_ddr_boot_size = 1*KB;
c7x_3_ddr_vecs_addr = c7x_3_ddr_resource_table_addr + 3*MB;
c7x_3_ddr_vecs_size = 16*KB;
c7x_3_ddr_secure_vecs_addr = c7x_3_ddr_resource_table_addr + 5*MB;
c7x_3_ddr_secure_vecs_size = 16*KB;
c7x_3_ddr_addr = c7x_3_ddr_resource_table_addr + 7*MB;
c7x_3_ddr_size = 24*MB;

c7x_4_ddr_ipc_addr = c7x_3_ddr_addr + c7x_3_ddr_size;
c7x_4_ddr_resource_table_addr = c7x_4_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_4_ddr_ipc_trace_addr = c7x_4_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_4_ddr_boot_addr = c7x_4_ddr_resource_table_addr + 1*MB;
c7x_4_ddr_boot_size = 1*KB;
c7x_4_ddr_vecs_addr = c7x_4_ddr_resource_table_addr + 3*MB;
c7x_4_ddr_vecs_size = 16*KB;
c7x_4_ddr_secure_vecs_addr = c7x_4_ddr_resource_table_addr + 5*MB;
c7x_4_ddr_secure_vecs_size = 16*KB;
c7x_4_ddr_addr = c7x_4_ddr_resource_table_addr + 7*MB;
c7x_4_ddr_size = 24*MB;

mcu0_ddr_local_heap_addr  = c7x_4_ddr_addr + c7x_4_ddr_size;
mcu0_ddr_local_heap_size  = 4*MB;
mcu1_ddr_local_heap_addr  = mcu0_ddr_local_heap_addr + mcu0_ddr_local_heap_size;
mcu1_ddr_local_heap_size  = 4*MB;
mcu2_ddr_local_heap_addr  = mcu1_ddr_local_heap_addr + mcu1_ddr_local_heap_size;
mcu2_ddr_local_heap_size  = 4*MB;
mcu2_ddr_viss_heap_addr   = mcu2_ddr_local_heap_addr + mcu2_ddr_local_heap_size;
mcu2_ddr_viss_heap_size   = 2*MB;
mcu3_ddr_local_heap_addr  = mcu2_ddr_viss_heap_addr + mcu2_ddr_viss_heap_size;
mcu3_ddr_local_heap_size  = 4*MB;
mcu3_ddr_viss_heap_addr   = mcu3_ddr_local_heap_addr + mcu3_ddr_local_heap_size;
mcu3_ddr_viss_heap_size   = 2*MB;
mcu4_ddr_local_heap_addr  = mcu3_ddr_viss_heap_addr + mcu3_ddr_viss_heap_size;
mcu4_ddr_local_heap_size  = 4*MB;
mcu4_ddr_viss_heap_addr   = mcu4_ddr_local_heap_addr + mcu4_ddr_local_heap_size;
mcu4_ddr_viss_heap_size   = 2*MB;
rmcu0_0_ddr_local_heap_addr  = mcu4_ddr_viss_heap_addr + mcu4_ddr_viss_heap_size;
rmcu0_0_ddr_local_heap_size  = 3*MB;
rmcu0_1_ddr_local_heap_addr  = rmcu0_0_ddr_local_heap_addr + rmcu0_0_ddr_local_heap_size;
rmcu0_1_ddr_local_heap_size  = 3*MB;
rmcu1_0_ddr_local_heap_addr  = rmcu0_1_ddr_local_heap_addr + rmcu0_1_ddr_local_heap_size;
rmcu1_0_ddr_local_heap_size  = 3*MB;
rmcu1_1_ddr_local_heap_addr  = rmcu1_0_ddr_local_heap_addr + rmcu1_0_ddr_local_heap_size;
rmcu1_1_ddr_local_heap_size  = 3*MB;
rmcu2_0_ddr_local_heap_addr  = rmcu1_1_ddr_local_heap_addr + rmcu1_1_ddr_local_heap_size;
rmcu2_0_ddr_local_heap_size  = 3*MB;
rmcu2_1_ddr_local_heap_addr  = rmcu2_0_ddr_local_heap_addr + rmcu2_0_ddr_local_heap_size;
rmcu2_1_ddr_local_heap_size  = 3*MB;

# Hardcoding this value, as this cannot be different from IPC echo test value ??
# To Align Shared Memory to 1 GB
ipc_vring_mem_addr      = rmcu2_1_ddr_local_heap_addr + rmcu2_1_ddr_local_heap_size;
ipc_vring_mem_size      = 48*MB;

app_log_mem_addr        = ipc_vring_mem_addr + ipc_vring_mem_size;
app_log_mem_size        = 256*KB;

tiovx_obj_desc_mem_addr = app_log_mem_addr + app_log_mem_size;
tiovx_obj_desc_mem_size = 4*MB;

app_fileio_mem_addr     =  tiovx_obj_desc_mem_addr + tiovx_obj_desc_mem_size;
app_fileio_mem_size     = 4*MB;

# Reduced tiovx log rt log mem from 12 MB to 10 MB
tiovx_log_rt_mem_addr   = app_fileio_mem_addr + app_fileio_mem_size;
tiovx_log_rt_mem_size   = 14*MB - app_fileio_mem_size;

#TODO:
# DDR memory allocation for various shared memories
# ddr_intercore_eth_desc_addr = rmcu0_1_ddr_local_heap_addr + rmcu0_1_ddr_local_heap_size;
# ddr_intercore_eth_desc_size = 8*MB;
# ddr_intercore_eth_data_addr = ddr_intercore_eth_desc_addr + ddr_intercore_eth_desc_size;
# ddr_intercore_eth_data_size = 24*MB;

# TODO: Shared Mem should be in high mem need RAT for that
# Shared memory for DMA Buf FD carveout
# ddr_shared_mem_addr     = tiovx_log_rt_mem_addr + tiovx_log_rt_mem_size;
ddr_shared_mem_addr     = 0xC0000000;
ddr_shared_mem_addr_phys  = 0xC0000000;
ddr_shared_mem_size     = SHARED_MEM_SIZE - uboot_reloc_mem_size;

assert ddr_shared_mem_addr & (SHARED_MEM_SIZE - 1) == 0

# C7x 1 Persistent DDR
c7x_1_ddr_local_heap_non_cacheable_addr  = 0xE0000000;
c7x_1_ddr_local_heap_non_cacheable_addr_phys  = 0xE0000000;
c7x_1_ddr_local_heap_non_cacheable_size  = 32*MB;
c7x_1_ddr_local_heap_addr = c7x_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_1_ddr_local_heap_addr_phys = c7x_1_ddr_local_heap_non_cacheable_addr_phys + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_1_ddr_local_heap_size = 32*MB;
total_c7x_1_local_ddr = c7x_1_ddr_local_heap_non_cacheable_size + c7x_1_ddr_local_heap_size;

# C7x 1 Scratch DDR
c7x_1_ddr_scratch_non_cacheable_addr      = c7x_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_1_ddr_scratch_non_cacheable_addr_phys = c7x_1_ddr_local_heap_addr_phys + c7x_1_ddr_local_heap_size;
c7x_1_ddr_scratch_non_cacheable_size      = 32*MB;
c7x_1_ddr_scratch_addr = c7x_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_1_ddr_scratch_addr_phys = c7x_1_ddr_scratch_non_cacheable_addr_phys + c7x_1_ddr_scratch_non_cacheable_size;
c7x_1_ddr_scratch_size = 32*MB;
total_c7x_1_scratch_ddr = c7x_1_ddr_scratch_non_cacheable_size + c7x_1_ddr_scratch_size

total_c7x_1_ddr = total_c7x_1_local_ddr + total_c7x_1_scratch_ddr

# C7x 2 Persistent DDR
c7x_2_ddr_local_heap_non_cacheable_addr  = 0xE0000000;
c7x_2_ddr_local_heap_non_cacheable_addr_phys  = c7x_1_ddr_scratch_addr_phys + c7x_1_ddr_scratch_size;
c7x_2_ddr_local_heap_non_cacheable_size  = 32*MB;
c7x_2_ddr_local_heap_addr = c7x_2_ddr_local_heap_non_cacheable_addr + c7x_2_ddr_local_heap_non_cacheable_size;
c7x_2_ddr_local_heap_addr_phys = c7x_2_ddr_local_heap_non_cacheable_addr_phys + c7x_2_ddr_local_heap_non_cacheable_size;
c7x_2_ddr_local_heap_size = 32*MB;
total_c7x_2_local_ddr = c7x_2_ddr_local_heap_non_cacheable_size + c7x_2_ddr_local_heap_size

# C7x 2 Scratch DDR
c7x_2_ddr_scratch_non_cacheable_addr     = c7x_2_ddr_local_heap_addr + c7x_2_ddr_local_heap_size;
c7x_2_ddr_scratch_non_cacheable_addr_phys     = c7x_2_ddr_local_heap_addr_phys + c7x_2_ddr_local_heap_size;
c7x_2_ddr_scratch_non_cacheable_size     = 32*MB;
c7x_2_ddr_scratch_addr = c7x_2_ddr_scratch_non_cacheable_addr + c7x_2_ddr_scratch_non_cacheable_size;
c7x_2_ddr_scratch_addr_phys = c7x_2_ddr_scratch_non_cacheable_addr_phys + c7x_2_ddr_scratch_non_cacheable_size;
c7x_2_ddr_scratch_size = 32*MB;
total_c7x_2_scratch_ddr = c7x_2_ddr_scratch_non_cacheable_size + c7x_2_ddr_scratch_size

total_c7x_2_ddr = total_c7x_2_local_ddr + total_c7x_2_scratch_ddr

# C7x 3 Persistent DDR
c7x_3_ddr_local_heap_non_cacheable_addr  = 0xE0000000;
c7x_3_ddr_local_heap_non_cacheable_addr_phys  = c7x_2_ddr_scratch_addr_phys + c7x_2_ddr_scratch_size;
c7x_3_ddr_local_heap_non_cacheable_size  = 32*MB;
c7x_3_ddr_local_heap_addr = c7x_3_ddr_local_heap_non_cacheable_addr + c7x_3_ddr_local_heap_non_cacheable_size;
c7x_3_ddr_local_heap_addr_phys = c7x_3_ddr_local_heap_non_cacheable_addr_phys + c7x_3_ddr_local_heap_non_cacheable_size;
c7x_3_ddr_local_heap_size = 32*MB;
total_c7x_3_local_ddr = c7x_3_ddr_local_heap_non_cacheable_size + c7x_3_ddr_local_heap_size
# C7x 3 Scratch DDR
c7x_3_ddr_scratch_non_cacheable_addr     = c7x_3_ddr_local_heap_addr + c7x_3_ddr_local_heap_size;
c7x_3_ddr_scratch_non_cacheable_addr_phys     = c7x_3_ddr_local_heap_addr_phys + c7x_3_ddr_local_heap_size;
c7x_3_ddr_scratch_non_cacheable_size     = 32*MB;
c7x_3_ddr_scratch_addr = c7x_3_ddr_scratch_non_cacheable_addr + c7x_3_ddr_scratch_non_cacheable_size;
c7x_3_ddr_scratch_addr_phys = c7x_3_ddr_scratch_non_cacheable_addr_phys + c7x_3_ddr_scratch_non_cacheable_size;
c7x_3_ddr_scratch_size = 32*MB;
total_c7x_3_scratch_ddr = c7x_3_ddr_scratch_non_cacheable_size + c7x_3_ddr_scratch_size

total_c7x_3_ddr = total_c7x_3_local_ddr + total_c7x_3_scratch_ddr

# C7x 4 Persistent DDR
c7x_4_ddr_local_heap_non_cacheable_addr  = 0xE0000000;
c7x_4_ddr_local_heap_non_cacheable_addr_phys  = c7x_3_ddr_scratch_addr_phys + c7x_3_ddr_scratch_size;
c7x_4_ddr_local_heap_non_cacheable_size  = 32*MB;
c7x_4_ddr_local_heap_addr = c7x_4_ddr_local_heap_non_cacheable_addr + c7x_4_ddr_local_heap_non_cacheable_size;
c7x_4_ddr_local_heap_addr_phys = c7x_4_ddr_local_heap_non_cacheable_addr_phys + c7x_4_ddr_local_heap_non_cacheable_size;
c7x_4_ddr_local_heap_size = 32*MB;
total_c7x_4_local_ddr = c7x_4_ddr_local_heap_non_cacheable_size + c7x_4_ddr_local_heap_size
# C7x 4 Scratch DDR
c7x_4_ddr_scratch_non_cacheable_addr     = c7x_4_ddr_local_heap_addr + c7x_4_ddr_local_heap_size;
c7x_4_ddr_scratch_non_cacheable_addr_phys     = c7x_4_ddr_local_heap_addr_phys + c7x_4_ddr_local_heap_size;
c7x_4_ddr_scratch_non_cacheable_size     = 32*MB;
c7x_4_ddr_scratch_addr = c7x_4_ddr_scratch_non_cacheable_addr + c7x_4_ddr_scratch_non_cacheable_size;
c7x_4_ddr_scratch_addr_phys = c7x_4_ddr_scratch_non_cacheable_addr_phys + c7x_4_ddr_scratch_non_cacheable_size;
c7x_4_ddr_scratch_size = 32*MB;
total_c7x_4_scratch_ddr = c7x_4_ddr_scratch_non_cacheable_size + c7x_4_ddr_scratch_size

total_c7x_4_ddr = total_c7x_4_local_ddr + total_c7x_4_scratch_ddr

total_c7x_ddr = total_c7x_1_ddr + total_c7x_2_ddr + total_c7x_3_ddr + total_c7x_4_ddr

# Virtual address for C7x 1 corresponding to physical addresses for C7x2, C7x3, C7x4
c7x_1_2_ddr_local_heap_non_cacheable_addr = c7x_1_ddr_scratch_addr + c7x_1_ddr_scratch_size;
c7x_1_3_ddr_local_heap_non_cacheable_addr = c7x_1_2_ddr_local_heap_non_cacheable_addr + c7x_2_ddr_local_heap_non_cacheable_size;
c7x_1_4_ddr_local_heap_non_cacheable_addr = c7x_1_3_ddr_local_heap_non_cacheable_addr + c7x_3_ddr_local_heap_non_cacheable_size;

c7x_1_2_ddr_local_heap_addr = c7x_1_4_ddr_local_heap_non_cacheable_addr + c7x_4_ddr_local_heap_non_cacheable_size;
c7x_1_3_ddr_local_heap_addr = c7x_1_2_ddr_local_heap_addr + c7x_2_ddr_local_heap_size;
c7x_1_4_ddr_local_heap_addr = c7x_1_3_ddr_local_heap_addr + c7x_3_ddr_local_heap_size;

c7x_1_2_ddr_scratch_non_cacheable_addr = c7x_1_4_ddr_local_heap_addr + c7x_4_ddr_local_heap_size;
c7x_1_3_ddr_scratch_non_cacheable_addr = c7x_1_2_ddr_scratch_non_cacheable_addr + c7x_2_ddr_scratch_non_cacheable_size;
c7x_1_4_ddr_scratch_non_cacheable_addr = c7x_1_3_ddr_scratch_non_cacheable_addr + c7x_3_ddr_scratch_non_cacheable_size;

c7x_1_2_ddr_scratch_addr = c7x_1_4_ddr_scratch_non_cacheable_addr + c7x_4_ddr_scratch_non_cacheable_size;
c7x_1_3_ddr_scratch_addr = c7x_1_2_ddr_scratch_addr + c7x_2_ddr_scratch_size;
c7x_1_4_ddr_scratch_addr = c7x_1_3_ddr_scratch_addr + c7x_3_ddr_scratch_size;

# Virtual address for C7x 2 corresponding to physical addresses for C7x1, C7x3, C7x4
c7x_2_1_ddr_local_heap_non_cacheable_addr = c7x_2_ddr_scratch_addr + c7x_2_ddr_scratch_size;
c7x_2_3_ddr_local_heap_non_cacheable_addr = c7x_2_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_2_4_ddr_local_heap_non_cacheable_addr = c7x_2_3_ddr_local_heap_non_cacheable_addr + c7x_3_ddr_local_heap_non_cacheable_size;

c7x_2_1_ddr_local_heap_addr = c7x_2_4_ddr_local_heap_non_cacheable_addr + c7x_4_ddr_local_heap_non_cacheable_size;
c7x_2_3_ddr_local_heap_addr = c7x_2_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_2_4_ddr_local_heap_addr = c7x_2_3_ddr_local_heap_addr + c7x_3_ddr_local_heap_size;

c7x_2_1_ddr_scratch_non_cacheable_addr = c7x_2_4_ddr_local_heap_addr + c7x_4_ddr_local_heap_size;
c7x_2_3_ddr_scratch_non_cacheable_addr = c7x_2_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_2_4_ddr_scratch_non_cacheable_addr = c7x_2_3_ddr_scratch_non_cacheable_addr + c7x_3_ddr_scratch_non_cacheable_size;

c7x_2_1_ddr_scratch_addr = c7x_2_4_ddr_scratch_non_cacheable_addr + c7x_4_ddr_scratch_non_cacheable_size;
c7x_2_3_ddr_scratch_addr = c7x_2_1_ddr_scratch_addr + c7x_1_ddr_scratch_size;
c7x_2_4_ddr_scratch_addr = c7x_2_3_ddr_scratch_addr + c7x_3_ddr_scratch_size;

# Virtual address for C7x 3 corresponding to physical addresses for C7x1, C7x2, C7x4
c7x_3_1_ddr_local_heap_non_cacheable_addr = c7x_3_ddr_scratch_addr + c7x_3_ddr_scratch_size;
c7x_3_2_ddr_local_heap_non_cacheable_addr = c7x_3_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_3_4_ddr_local_heap_non_cacheable_addr = c7x_3_2_ddr_local_heap_non_cacheable_addr + c7x_2_ddr_local_heap_non_cacheable_size;

c7x_3_1_ddr_local_heap_addr = c7x_3_4_ddr_local_heap_non_cacheable_addr + c7x_4_ddr_local_heap_non_cacheable_size;
c7x_3_2_ddr_local_heap_addr = c7x_3_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_3_4_ddr_local_heap_addr = c7x_3_2_ddr_local_heap_addr + c7x_2_ddr_local_heap_size;

c7x_3_1_ddr_scratch_non_cacheable_addr = c7x_3_4_ddr_local_heap_addr + c7x_4_ddr_local_heap_size;
c7x_3_2_ddr_scratch_non_cacheable_addr = c7x_3_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_3_4_ddr_scratch_non_cacheable_addr = c7x_3_2_ddr_scratch_non_cacheable_addr + c7x_2_ddr_scratch_non_cacheable_size;

c7x_3_1_ddr_scratch_addr = c7x_3_4_ddr_scratch_non_cacheable_addr + c7x_4_ddr_scratch_non_cacheable_size;
c7x_3_2_ddr_scratch_addr = c7x_3_1_ddr_scratch_addr + c7x_1_ddr_scratch_size;
c7x_3_4_ddr_scratch_addr = c7x_3_2_ddr_scratch_addr + c7x_2_ddr_scratch_size;

# Virtual address for C7x 4 corresponding to physical addresses for C7x1, C7x2, C7x3
c7x_4_1_ddr_local_heap_non_cacheable_addr = c7x_4_ddr_scratch_addr + c7x_4_ddr_scratch_size;
c7x_4_2_ddr_local_heap_non_cacheable_addr = c7x_4_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_4_3_ddr_local_heap_non_cacheable_addr = c7x_4_2_ddr_local_heap_non_cacheable_addr + c7x_2_ddr_local_heap_non_cacheable_size;

c7x_4_1_ddr_local_heap_addr = c7x_4_3_ddr_local_heap_non_cacheable_addr + c7x_3_ddr_local_heap_non_cacheable_size;
c7x_4_2_ddr_local_heap_addr = c7x_4_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_4_3_ddr_local_heap_addr = c7x_4_2_ddr_local_heap_addr + c7x_2_ddr_local_heap_size;

c7x_4_1_ddr_scratch_non_cacheable_addr = c7x_4_3_ddr_local_heap_addr + c7x_3_ddr_local_heap_size;
c7x_4_2_ddr_scratch_non_cacheable_addr = c7x_4_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_4_3_ddr_scratch_non_cacheable_addr = c7x_4_2_ddr_scratch_non_cacheable_addr + c7x_2_ddr_scratch_non_cacheable_size;

c7x_4_1_ddr_scratch_addr = c7x_4_3_ddr_scratch_non_cacheable_addr + c7x_3_ddr_scratch_non_cacheable_size;
c7x_4_2_ddr_scratch_addr = c7x_4_1_ddr_scratch_addr + c7x_1_ddr_scratch_size;
c7x_4_3_ddr_scratch_addr = c7x_4_2_ddr_scratch_addr + c7x_2_ddr_scratch_size;

#
# Create memory section based on addr and size defined above, including
# any CPU specific internal memories
#

# C7x L1/L2 memory sections
c7x_1_l2   = MemSection("L2RAM_C7x_1", "RWIX", c7x_1_l2_addr  , c7x_1_l2_size  , "L2 for C7x_1");
# c7x_1_l1   = MemSection("L1RAM_C7x_1", "RWIX", c7x_1_l1_addr  , c7x_1_l1_size  , "L1 for C7x_1");
c7x_1_msmc  = MemSection("MSMC_C7x_1", "RWIX", c7x_1_msmc_mem_addr  , c7x_1_msmc_size  , "MSMC for C7x_1");

c7x_2_l2   = MemSection("L2RAM_C7x_2", "RWIX", c7x_2_l2_addr  , c7x_2_l2_size  , "L2 for C7x_2");
# c7x_2_l1   = MemSection("L1RAM_C7x_2", "RWIX", c7x_2_l1_addr  , c7x_2_l1_size  , "L1 for C7x_2");
c7x_2_msmc  = MemSection("MSMC_C7x_2", "RWIX", c7x_2_msmc_mem_addr  , c7x_2_msmc_size  , "MSMC for C7x_2");

c7x_3_l2   = MemSection("L2RAM_C7x_3", "RWIX", c7x_3_l2_addr  , c7x_3_l2_size  , "L2 for C7x_3");
# c7x_3_l1   = MemSection("L1RAM_C7x_3", "RWIX", c7x_3_l1_addr  , c7x_3_l1_size  , "L1 for C7x_3");
c7x_3_msmc  = MemSection("MSMC_C7x_3", "RWIX", c7x_3_msmc_mem_addr  , c7x_3_msmc_size  , "MSMC for C7x_3");

c7x_4_l2   = MemSection("L2RAM_C7x_4", "RWIX", c7x_4_l2_addr  , c7x_4_l2_size  , "L2 for C7x_4");
# c7x_4_l1   = MemSection("L1RAM_C7x_4", "RWIX", c7x_4_l1_addr  , c7x_4_l1_size  , "L1 for C7x_4");
c7x_4_msmc  = MemSection("MSMC_C7x_4", "RWIX", c7x_4_msmc_mem_addr  , c7x_4_msmc_size  , "MSMC for C7x_4");

uboot_reloc_mem = MemSection("UBOOT_RELOC_MEM", "", uboot_reloc_mem_addr  , uboot_reloc_mem_size  , "Uboot DDR relocation memory");

# Main OCRAM memory sections
mcu2_main_ocram        = MemSection("MAIN_OCRAM_MCU2", "RWIX", mcu2_main_ocram_addr  , mcu2_main_ocram_size  , "Main OCRAM for MCU2");
mcu3_main_ocram        = MemSection("MAIN_OCRAM_MCU3", "RWIX", mcu3_main_ocram_addr  , mcu3_main_ocram_size  , "Main OCRAM for MCU3");
mcu4_main_ocram        = MemSection("MAIN_OCRAM_MCU4", "RWIX", mcu4_main_ocram_addr  , mcu4_main_ocram_size  , "Main OCRAM for MCU4");

mcu2_main_ocram_phys   = MemSection("MAIN_OCRAM_MCU2_PHYS", "RWIX", mcu2_main_ocram_addr_phys  , mcu2_main_ocram_size  , "Main OCRAM Physical Address for MCU2");
mcu3_main_ocram_phys   = MemSection("MAIN_OCRAM_MCU3_PHYS", "RWIX", mcu3_main_ocram_addr_phys  , mcu3_main_ocram_size  , "Main OCRAM Physical Address for MCU3");
mcu4_main_ocram_phys   = MemSection("MAIN_OCRAM_MCU4_PHYS", "RWIX", mcu4_main_ocram_addr_phys  , mcu4_main_ocram_size  , "Main OCRAM Physical Address for MCU4");

# CPU code/data memory sections in DDR
dmcu0_ddr                 = MemSection("DDR_DMCU0", "RWIX", dmcu0_ddr_addr, dmcu0_ddr_size, "DDR for DMCU0 for code/data");
dmcu0_ddr_total           = MemSection("DDR_DMCU0_DTS", "", 0, 0, "DDR for DMCU0 for all sections, used for reserving memory in DTS file");
dmcu0_ddr_total.concat(dmcu0_ddr);
dmcu0_ddr_total.setDtsName("vision_apps_dmcu0_memory_region", "vision-apps-m55-memory");

mcu0_tcma_vecs           = MemSection("MCU0_TCMA_VECS" , "X"   , 0x00000000, (KB << 2));
mcu0_tcma                = MemSection("MCU0_TCMA" , "X"   , 0x00001000, (64*KB) - (KB << 2) - 1);
mcu0_ddr_ipc             = MemSection("DDR_MCU0_IPC", "RWIX", mcu0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU0 for Linux IPC");
mcu0_ddr_ipc.setDtsName("vision_apps_mcu0_dma_memory_region", "vision-apps-m55-dma-memory");
mcu0_ddr_resource_table  = MemSection("DDR_MCU0_RESOURCE_TABLE", "RWIX", mcu0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU0 for Linux resource table");
mcu0_ddr_ipc_trace       = MemSection("DDR_MCU0_IPC_TRACE", "RWIX", mcu0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU0 for Linux IPC trace");
mcu0_ddr                 = MemSection("DDR_MCU0", "RWIX", mcu0_ddr_addr, mcu0_ddr_size, "DDR for MCU0 for code/data");
mcu0_ddr_local_heap      = MemSection("DDR_MCU0_LOCAL_HEAP", "RWIX", mcu0_ddr_local_heap_addr, mcu0_ddr_local_heap_size, "DDR for MCU0 for local heap");
mcu0_ddr_total           = MemSection("DDR_MCU0_DTS", "", 0, 0, "DDR for MCU0 for all sections, used for reserving memory in DTS file");
mcu0_ddr_total.concat(mcu0_ddr_resource_table);
mcu0_ddr_total.concat(mcu0_ddr_ipc_trace);
mcu0_ddr_total.concat(mcu0_ddr);
mcu0_ddr_total.setDtsName("vision_apps_mcu0_memory_region", "vision-apps-m55-memory");

mcu1_tcma_vecs           = MemSection("MCU1_TCMA_VECS" , "X"   , 0x00000000, (KB << 2));
mcu1_tcma                = MemSection("MCU1_TCMA" , "X"   , 0x00001000, (64*KB) - (KB << 2) - 1);
mcu1_ddr_ipc             = MemSection("DDR_MCU1_IPC", "RWIX", mcu1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU1 for Linux IPC");
mcu1_ddr_ipc.setDtsName("vision_apps_mcu1_dma_memory_region", "vision-apps-m55-dma-memory");
mcu1_ddr_resource_table  = MemSection("DDR_MCU1_RESOURCE_TABLE", "RWIX", mcu1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU1 for Linux resource table");
mcu1_ddr_ipc_trace       = MemSection("DDR_MCU1_IPC_TRACE", "RWIX", mcu1_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU1 for Linux IPC trace");
mcu1_ddr                 = MemSection("DDR_MCU1", "RWIX", mcu1_ddr_addr, mcu1_ddr_size, "DDR for MCU1 for code/data");
mcu1_ddr_total           = MemSection("DDR_MCU1_DTS", "", 0, 0, "DDR for MCU1 for all sections, used for reserving memory in DTS file");
mcu1_ddr_local_heap      = MemSection("DDR_MCU1_LOCAL_HEAP", "RWIX", mcu1_ddr_local_heap_addr, mcu1_ddr_local_heap_size, "DDR for MCU1 for local heap");
mcu1_ddr_total.concat(mcu1_ddr_resource_table);
mcu1_ddr_total.concat(mcu1_ddr_ipc_trace);
mcu1_ddr_total.concat(mcu1_ddr);
mcu1_ddr_total.setDtsName("vision_apps_mcu1_memory_region", "vision-apps-m55-memory");

mcu2_tcma_vecs           = MemSection("MCU2_TCMA_VECS" , "X"   , 0x00000000, (KB << 2));
mcu2_tcma                = MemSection("MCU2_TCMA" , "X"   , 0x00001000, (64*KB) - (KB << 2) - 1);
mcu2_ddr_ipc             = MemSection("DDR_MCU2_IPC", "RWIX", mcu2_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU2 for Linux IPC");
mcu2_ddr_ipc.setDtsName("vision_apps_mcu2_dma_memory_region", "vision-apps-m55-dma-memory");
mcu2_ddr_resource_table  = MemSection("DDR_MCU2_RESOURCE_TABLE", "RWIX", mcu2_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU2 for Linux resource table");
mcu2_ddr_ipc_trace       = MemSection("DDR_MCU2_IPC_TRACE", "RWIX", mcu2_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU2 for Linux IPC trace");
mcu2_ddr                 = MemSection("DDR_MCU2", "RWIX", mcu2_ddr_addr, mcu2_ddr_size, "DDR for MCU2 for code/data");
mcu2_ddr_total           = MemSection("DDR_MCU2_DTS", "", 0, 0, "DDR for MCU2 for all sections, used for reserving memory in DTS file");
mcu2_ddr_local_heap      = MemSection("DDR_MCU2_LOCAL_HEAP", "RWIX", mcu2_ddr_local_heap_addr, mcu2_ddr_local_heap_size, "DDR for MCU2 for local heap");
mcu2_ddr_viss_config_heap = MemSection("DDR_MCU2_VISS_CONFIG_HEAP", "RWIX", mcu2_ddr_viss_heap_addr, mcu2_ddr_viss_heap_size, "VISS configuration memory. MUST be write through cache policy.");
mcu2_ddr_total.concat(mcu2_ddr_resource_table);
mcu2_ddr_total.concat(mcu2_ddr_ipc_trace);
mcu2_ddr_total.concat(mcu2_ddr);
mcu2_ddr_total.setDtsName("vision_apps_mcu2_memory_region", "vision-apps-m55-memory");

mcu3_tcma_vecs           = MemSection("MCU3_TCMA_VECS" , "X"   , 0x00000000, (KB << 2));
mcu3_tcma                = MemSection("MCU3_TCMA" , "X"   , 0x00001000, (64*KB) - (KB << 2) - 1);
mcu3_ddr_ipc             = MemSection("DDR_MCU3_IPC", "RWIX", mcu3_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU3 for Linux IPC");
mcu3_ddr_ipc.setDtsName("vision_apps_mcu3_dma_memory_region", "vision-apps-m55-dma-memory");
mcu3_ddr_resource_table  = MemSection("DDR_MCU3_RESOURCE_TABLE", "RWIX", mcu3_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU3 for Linux resource table");
mcu3_ddr_ipc_trace       = MemSection("DDR_MCU3_IPC_TRACE", "RWIX", mcu3_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU3 for Linux IPC trace");
mcu3_ddr                 = MemSection("DDR_MCU3", "RWIX", mcu3_ddr_addr, mcu3_ddr_size, "DDR for MCU3 for code/data");
mcu3_ddr_local_heap      = MemSection("DDR_MCU3_LOCAL_HEAP", "RWIX", mcu3_ddr_local_heap_addr, mcu3_ddr_local_heap_size, "DDR for MCU3 for local heap");
mcu3_ddr_viss_config_heap = MemSection("DDR_MCU3_VISS_CONFIG_HEAP", "RWIX", mcu3_ddr_viss_heap_addr, mcu3_ddr_viss_heap_size, "VISS configuration memory. MUST be write through cache policy.");
mcu3_ddr_total           = MemSection("DDR_MCU3_DTS", "", 0, 0, "DDR for MCU3 for all sections, used for reserving memory in DTS file");
mcu3_ddr_total.concat(mcu3_ddr_resource_table);
mcu3_ddr_total.concat(mcu3_ddr_ipc_trace);
mcu3_ddr_total.concat(mcu3_ddr);
mcu3_ddr_total.setDtsName("vision_apps_mcu3_memory_region", "vision-apps-m55-memory");

mcu4_tcma_vecs           = MemSection("MCU4_TCMA_VECS" , "X"   , 0x00000000, (KB << 2));
mcu4_tcma                = MemSection("MCU4_TCMA" , "X"   , 0x00001000, (64*KB) - (KB << 2) - 1);
mcu4_ddr_ipc             = MemSection("DDR_MCU4_IPC", "RWIX", mcu4_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU4 for Linux IPC");
mcu4_ddr_ipc.setDtsName("vision_apps_mcu4_dma_memory_region", "vision-apps-m55-dma-memory");
mcu4_ddr_resource_table  = MemSection("DDR_MCU4_RESOURCE_TABLE", "RWIX", mcu4_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU4 for Linux resource table");
mcu4_ddr_ipc_trace       = MemSection("DDR_MCU4_IPC_TRACE", "RWIX", mcu4_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU4 for Linux IPC trace");
mcu4_ddr                 = MemSection("DDR_MCU4", "RWIX", mcu4_ddr_addr, mcu4_ddr_size, "DDR for MCU4 for code/data");
mcu4_ddr_local_heap      = MemSection("DDR_MCU4_LOCAL_HEAP", "RWIX", mcu4_ddr_local_heap_addr, mcu4_ddr_local_heap_size, "DDR for MCU4 for local heap");
mcu4_ddr_viss_config_heap = MemSection("DDR_MCU4_VISS_CONFIG_HEAP", "RWIX", mcu4_ddr_viss_heap_addr, mcu4_ddr_viss_heap_size, "VISS configuration memory. MUST be write through cache policy.");
mcu4_ddr_total           = MemSection("DDR_MCU4_DTS", "", 0, 0, "DDR for MCU4 for all sections, used for reserving memory in DTS file");
mcu4_ddr_total.concat(mcu4_ddr_resource_table);
mcu4_ddr_total.concat(mcu4_ddr_ipc_trace);
mcu4_ddr_total.concat(mcu4_ddr);
mcu4_ddr_total.setDtsName("vision_apps_mcu4_memory_region", "vision-apps-m55-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu0_0_tcm                 = MemSection("RMCU0_0_TCM", "RWIX", 0x67000000, (32*KB));
rmcu0_0_ddr_ipc             = MemSection("DDR_RMCU0_0_IPC", "RWIX", rmcu0_0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU0_0 for Linux IPC");
rmcu0_0_ddr_ipc.setDtsName("vision_apps_rmcu0_0_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu0_0_ddr_resource_table  = MemSection("DDR_RMCU0_0_RESOURCE_TABLE", "RWIX", rmcu0_0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU0_0 for Linux resource table");
rmcu0_0_ddr_ipc_trace       = MemSection("DDR_RMCU0_0_IPC_TRACE", "RWIX", rmcu0_0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU0_0 for Linux IPC trace");
rmcu0_0_ddr                 = MemSection("DDR_RMCU0_0", "RWIX", rmcu0_0_ddr_addr, rmcu0_0_ddr_size, "DDR for RMCU0_0 for code/data");
rmcu0_0_ddr_local_heap      = MemSection("DDR_RMCU0_0_LOCAL_HEAP", "RWIX", rmcu0_0_ddr_local_heap_addr, rmcu0_0_ddr_local_heap_size, "DDR for RMCU0_0 for local heap");
rmcu0_0_ddr_total           = MemSection("DDR_RMCU0_0_DTS", "", 0, 0, "DDR for RMCU0_0 for all sections, used for reserving memory in DTS file");
rmcu0_0_ddr_total.concat(rmcu0_0_ddr_resource_table);
rmcu0_0_ddr_total.concat(rmcu0_0_ddr_ipc_trace);
rmcu0_0_ddr_total.concat(rmcu0_0_ddr);
rmcu0_0_ddr_total.setDtsName("vision_apps_rmcu0_0_memory_region", "vision-apps-r52f-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu0_1_tcm                 = MemSection("RMCU0_1_TCM", "RWIX", 0x67010000, (32*KB));
rmcu0_1_ddr_ipc             = MemSection("DDR_RMCU0_1_IPC", "RWIX", rmcu0_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU0_1 for Linux IPC");
rmcu0_1_ddr_ipc.setDtsName("vision_apps_rmcu0_1_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu0_1_ddr_resource_table  = MemSection("DDR_RMCU0_1_RESOURCE_TABLE", "RWIX", rmcu0_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU0_1 for Linux resource table");
rmcu0_1_ddr_ipc_trace       = MemSection("DDR_RMCU0_1_IPC_TRACE", "RWIX", rmcu0_1_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU0_1 for Linux IPC trace");
rmcu0_1_ddr                 = MemSection("DDR_RMCU0_1", "RWIX", rmcu0_1_ddr_addr, rmcu0_1_ddr_size, "DDR for RMCU0_1 for code/data");
rmcu0_1_ddr_local_heap      = MemSection("DDR_RMCU0_1_LOCAL_HEAP", "RWIX", rmcu0_1_ddr_local_heap_addr, rmcu0_1_ddr_local_heap_size, "DDR for RMCU0_1 for local heap");
rmcu0_1_ddr_total           = MemSection("DDR_RMCU0_1_DTS", "", 0, 0, "DDR for RMCU0_1 for all sections, used for reserving memory in DTS file");
rmcu0_1_ddr_total.concat(rmcu0_1_ddr_resource_table);
rmcu0_1_ddr_total.concat(rmcu0_1_ddr_ipc_trace);
rmcu0_1_ddr_total.concat(rmcu0_1_ddr);
rmcu0_1_ddr_total.setDtsName("vision_apps_rmcu0_1_memory_region", "vision-apps-r52f-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu1_0_tcm                 = MemSection("RMCU1_0_TCM", "RWIX", 0x67020000, (32*KB));
rmcu1_0_ddr_ipc             = MemSection("DDR_RMCU1_0_IPC", "RWIX", rmcu1_0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU1_0 for Linux IPC");
rmcu1_0_ddr_ipc.setDtsName("vision_apps_rmcu1_0_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu1_0_ddr_resource_table  = MemSection("DDR_RMCU1_0_RESOURCE_TABLE", "RWIX", rmcu1_0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU1_0 for Linux resource table");
rmcu1_0_ddr_ipc_trace       = MemSection("DDR_RMCU1_0_IPC_TRACE", "RWIX", rmcu1_0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU1_0 for Linux IPC trace");
rmcu1_0_ddr                 = MemSection("DDR_RMCU1_0", "RWIX", rmcu1_0_ddr_addr, rmcu1_0_ddr_size, "DDR for RMCU1_0 for code/data");
rmcu1_0_ddr_local_heap      = MemSection("DDR_RMCU1_0_LOCAL_HEAP", "RWIX", rmcu1_0_ddr_local_heap_addr, rmcu1_0_ddr_local_heap_size, "DDR for RMCU1_0 for local heap");
rmcu1_0_ddr_total           = MemSection("DDR_RMCU1_0_DTS", "", 0, 0, "DDR for RMCU1_0 for all sections, used for reserving memory in DTS file");
rmcu1_0_ddr_total.concat(rmcu1_0_ddr_resource_table);
rmcu1_0_ddr_total.concat(rmcu1_0_ddr_ipc_trace);
rmcu1_0_ddr_total.concat(rmcu1_0_ddr);
rmcu1_0_ddr_total.setDtsName("vision_apps_rmcu1_0_memory_region", "vision-apps-r52f-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu1_1_tcm                 = MemSection("RMCU1_1_TCM", "RWIX", 0x67030000, (32*KB));
rmcu1_1_ddr_ipc             = MemSection("DDR_RMCU1_1_IPC", "RWIX", rmcu1_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU1_1 for Linux IPC");
rmcu1_1_ddr_ipc.setDtsName("vision_apps_rmcu1_1_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu1_1_ddr_resource_table  = MemSection("DDR_RMCU1_1_RESOURCE_TABLE", "RWIX", rmcu1_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU1_1 for Linux resource table");
rmcu1_1_ddr_ipc_trace       = MemSection("DDR_RMCU1_1_IPC_TRACE", "RWIX", rmcu1_1_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU1_1 for Linux IPC trace");
rmcu1_1_ddr                 = MemSection("DDR_RMCU1_1", "RWIX", rmcu1_1_ddr_addr, rmcu1_1_ddr_size, "DDR for RMCU1_1 for code/data");
rmcu1_1_ddr_local_heap      = MemSection("DDR_RMCU1_1_LOCAL_HEAP", "RWIX", rmcu1_1_ddr_local_heap_addr, rmcu1_1_ddr_local_heap_size, "DDR for RMCU1_1 for local heap");
rmcu1_1_ddr_total           = MemSection("DDR_RMCU1_1_DTS", "", 0, 0, "DDR for RMCU1_1 for all sections, used for reserving memory in DTS file");
rmcu1_1_ddr_total.concat(rmcu1_1_ddr_resource_table);
rmcu1_1_ddr_total.concat(rmcu1_1_ddr_ipc_trace);
rmcu1_1_ddr_total.concat(rmcu1_1_ddr);
rmcu1_1_ddr_total.setDtsName("vision_apps_rmcu1_1_memory_region", "vision-apps-r52f-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu2_0_tcm                 = MemSection("RMCU2_0_TCM", "RWIX", 0x67040000, (32*KB));
rmcu2_0_ddr_ipc             = MemSection("DDR_RMCU2_0_IPC", "RWIX", rmcu2_0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU2_0 for Linux IPC");
rmcu2_0_ddr_ipc.setDtsName("vision_apps_rmcu2_0_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu2_0_ddr_resource_table  = MemSection("DDR_RMCU2_0_RESOURCE_TABLE", "RWIX", rmcu2_0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU2_0 for Linux resource table");
rmcu2_0_ddr_ipc_trace       = MemSection("DDR_RMCU2_0_IPC_TRACE", "RWIX", rmcu2_0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU2_0 for Linux IPC trace");
rmcu2_0_ddr                 = MemSection("DDR_RMCU2_0", "RWIX", rmcu2_0_ddr_addr, rmcu2_0_ddr_size, "DDR for RMCU2_0 for code/data");
rmcu2_0_ddr_local_heap      = MemSection("DDR_RMCU2_0_LOCAL_HEAP", "RWIX", rmcu2_0_ddr_local_heap_addr, rmcu2_0_ddr_local_heap_size, "DDR for RMCU2_0 for local heap");
rmcu2_0_ddr_total           = MemSection("DDR_RMCU2_0_DTS", "", 0, 0, "DDR for RMCU2_0 for all sections, used for reserving memory in DTS file");
rmcu2_0_ddr_total.concat(rmcu2_0_ddr_resource_table);
rmcu2_0_ddr_total.concat(rmcu2_0_ddr_ipc_trace);
rmcu2_0_ddr_total.concat(rmcu2_0_ddr);
rmcu2_0_ddr_total.setDtsName("vision_apps_rmcu2_0_memory_region", "vision-apps-r52f-memory");

# HACK: R52 TCM should be used instead of L2SRAM of analytics cluster
rmcu2_1_tcm                 = MemSection("RMCU2_1_TCM", "RWIX", 0x67050000, (32*KB));
rmcu2_1_ddr_ipc             = MemSection("DDR_RMCU2_1_IPC", "RWIX", rmcu2_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for RMCU2_1 for Linux IPC");
rmcu2_1_ddr_ipc.setDtsName("vision_apps_rmcu2_1_dma_memory_region", "vision-apps-r52f-dma-memory");
rmcu2_1_ddr_resource_table  = MemSection("DDR_RMCU2_1_RESOURCE_TABLE", "RWIX", rmcu2_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for RMCU2_1 for Linux resource table");
rmcu2_1_ddr_ipc_trace       = MemSection("DDR_RMCU2_1_IPC_TRACE", "RWIX", rmcu2_1_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for RMCU2_1 for Linux IPC trace");
rmcu2_1_ddr                 = MemSection("DDR_RMCU2_1", "RWIX", rmcu2_1_ddr_addr, rmcu2_1_ddr_size, "DDR for RMCU2_1 for code/data");
rmcu2_1_ddr_local_heap      = MemSection("DDR_RMCU2_1_LOCAL_HEAP", "RWIX", rmcu2_1_ddr_local_heap_addr, rmcu2_1_ddr_local_heap_size, "DDR for RMCU2_1 for local heap");
rmcu2_1_ddr_total           = MemSection("DDR_RMCU2_1_DTS", "", 0, 0, "DDR for RMCU2_1 for all sections, used for reserving memory in DTS file");
rmcu2_1_ddr_total.concat(rmcu2_1_ddr_resource_table);
rmcu2_1_ddr_total.concat(rmcu2_1_ddr_ipc_trace);
rmcu2_1_ddr_total.concat(rmcu2_1_ddr);
rmcu2_1_ddr_total.setDtsName("vision_apps_rmcu2_1_memory_region", "vision-apps-r52f-memory");

c7x_1_ddr_ipc             = MemSection("DDR_C7x_1_IPC", "RWIX", c7x_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_1 for Linux IPC");
c7x_1_ddr_ipc.setDtsName("vision_apps_c71_0_dma_memory_region", "vision-apps-c71-dma-memory");
c7x_1_ddr_resource_table  = MemSection("DDR_C7x_1_RESOURCE_TABLE", "RWIX", c7x_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_1 for Linux resource table");
c7x_1_ddr_ipc_trace       = MemSection("DDR_C7x_1_IPC_TRACE", "RWIX", c7x_1_ddr_ipc_trace_addr, linux_ddr_ipc_trace_size, "DDR for C7x_1 for Linux IPC trace");
c7x_1_ddr_boot            = MemSection("DDR_C7x_1_BOOT", "RWIX", c7x_1_ddr_boot_addr, c7x_1_ddr_boot_size, "DDR for C7x_1 for boot section");
c7x_1_ddr_vecs            = MemSection("DDR_C7x_1_VECS", "RWIX", c7x_1_ddr_vecs_addr, c7x_1_ddr_vecs_size, "DDR for C7x_1 for vecs section");
c7x_1_ddr_secure_vecs     = MemSection("DDR_C7x_1_SECURE_VECS", "RWIX", c7x_1_ddr_secure_vecs_addr, c7x_1_ddr_secure_vecs_size, "DDR for C7x_1 for secure vecs section");
c7x_1_ddr                 = MemSection("DDR_C7x_1", "RWIX", c7x_1_ddr_addr, c7x_1_ddr_size, "DDR for C7x_1 code/data section");
c7x_1_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_1_ddr_local_heap_non_cacheable_addr_phys, c7x_1_ddr_local_heap_non_cacheable_size, "Physical address of non-cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_local_heap      = MemSection("DDR_C7X_1_LOCAL_HEAP", "RWIX", c7x_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_local_heap_phys = MemSection("DDR_C7X_1_LOCAL_HEAP_PHYS", "RWIX", c7x_1_ddr_local_heap_addr_phys, c7x_1_ddr_local_heap_size, "Physical address of cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_1_ddr_scratch_non_cacheable_addr_phys, c7x_1_ddr_scratch_non_cacheable_size, "Physical address of non-cacheable DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_scratch         = MemSection("DDR_C7X_1_SCRATCH", "RWIX", c7x_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_scratch_phys    = MemSection("DDR_C7X_1_SCRATCH_PHYS", "RWIX", c7x_1_ddr_scratch_addr_phys, c7x_1_ddr_scratch_size, "Physical address of cacheable DDR for c7x_1 for Scratch Memory");

c7x_1_2_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for local heap wrt c7x_1");
c7x_1_2_ddr_local_heap      = MemSection("DDR_C7X_1_2_LOCAL_HEAP", "RWIX", c7x_1_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_2 for local heap wrt c7x_1");
c7x_1_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for Scratch Memory wrt c7x_1");
c7x_1_2_ddr_scratch         = MemSection("DDR_C7X_1_2_SCRATCH", "RWIX", c7x_1_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_2 for Scratch Memory wrt c7x_1");

c7x_1_3_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_1_3_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_3_ddr_local_heap_non_cacheable_addr, c7x_3_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for local heap wrt c7x_1");
c7x_1_3_ddr_local_heap      = MemSection("DDR_C7X_1_3_LOCAL_HEAP", "RWIX", c7x_1_3_ddr_local_heap_addr, c7x_3_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_3 for local heap wrt c7x_1");
c7x_1_3_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_3_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_3_ddr_scratch_non_cacheable_addr, c7x_3_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for Scratch Memory wrt c7x_1");
c7x_1_3_ddr_scratch         = MemSection("DDR_C7X_1_3_SCRATCH", "RWIX", c7x_1_3_ddr_scratch_addr, c7x_3_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_3 for Scratch Memory wrt c7x_1");

c7x_1_4_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_1_4_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_4_ddr_local_heap_non_cacheable_addr, c7x_4_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for local heap wrt c7x_1");
c7x_1_4_ddr_local_heap      = MemSection("DDR_C7X_1_4_LOCAL_HEAP", "RWIX", c7x_1_4_ddr_local_heap_addr, c7x_4_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_4 for local heap wrt c7x_1");
c7x_1_4_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_4_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_4_ddr_scratch_non_cacheable_addr, c7x_4_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for Scratch Memory wrt c7x_1");
c7x_1_4_ddr_scratch         = MemSection("DDR_C7X_1_4_SCRATCH", "RWIX", c7x_1_4_ddr_scratch_addr, c7x_4_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_4 for Scratch Memory wrt c7x_1");

c7x_1_ddr_total           = MemSection("DDR_C7x_1_DTS", "", 0, 0, "DDR for C7x_1 for all sections, used for reserving memory in DTS file");
c7x_1_ddr_total.concat(c7x_1_ddr_resource_table);
c7x_1_ddr_total.concat(c7x_1_ddr_ipc_trace);
c7x_1_ddr_total.concat(c7x_1_ddr_boot);
c7x_1_ddr_total.concat(c7x_1_ddr_vecs);
c7x_1_ddr_total.concat(c7x_1_ddr_secure_vecs);
c7x_1_ddr_total.concat(c7x_1_ddr);
c7x_1_ddr_total.setDtsName("vision_apps_c71_0_memory_region", "vision-apps-c71_0-memory");

c7x_2_ddr_ipc             = MemSection("DDR_C7x_2_IPC", "RWIX", c7x_2_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_2 for Linux IPC");
c7x_2_ddr_ipc.setDtsName("vision_apps_c71_1_dma_memory_region", "vision-apps-c71_1-dma-memory");
c7x_2_ddr_resource_table  = MemSection("DDR_C7x_2_RESOURCE_TABLE", "RWIX", c7x_2_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_2 for Linux resource table");
c7x_2_ddr_ipc_trace       = MemSection("DDR_C7x_2_IPC_TRACE", "RWIX", c7x_2_ddr_ipc_trace_addr, linux_ddr_ipc_trace_size, "DDR for C7x_2 for Linux IPC trace");
c7x_2_ddr_boot            = MemSection("DDR_C7x_2_BOOT", "RWIX", c7x_2_ddr_boot_addr, c7x_2_ddr_boot_size, "DDR for C7x_2 for boot section");
c7x_2_ddr_vecs            = MemSection("DDR_C7x_2_VECS", "RWIX", c7x_2_ddr_vecs_addr, c7x_2_ddr_vecs_size, "DDR for C7x_2 for vecs section");
c7x_2_ddr_secure_vecs     = MemSection("DDR_C7x_2_SECURE_VECS", "RWIX", c7x_2_ddr_secure_vecs_addr, c7x_2_ddr_secure_vecs_size, "DDR for C7x_2 for secure vecs section");
c7x_2_ddr                 = MemSection("DDR_C7x_2", "RWIX", c7x_2_ddr_addr, c7x_2_ddr_size, "DDR for C7x_2 code/data section");
c7x_2_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_2_ddr_local_heap_non_cacheable_addr_phys, c7x_2_ddr_local_heap_non_cacheable_size, "Non-cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_local_heap      = MemSection("DDR_C7X_2_LOCAL_HEAP", "RWIX", c7x_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "Virtual address of Cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_local_heap_phys      = MemSection("DDR_C7X_2_LOCAL_HEAP_PHYS", "RWIX", c7x_2_ddr_local_heap_addr_phys, c7x_2_ddr_local_heap_size, "Physical address of Cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_2 for Scratch Memory");
c7x_2_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_2_ddr_scratch_non_cacheable_addr_phys, c7x_2_ddr_scratch_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_2 for Scratch Memory");
c7x_2_ddr_scratch         = MemSection("DDR_C7X_2_SCRATCH", "RWIX", c7x_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_2 for Scratch Memory");
c7x_2_ddr_scratch_phys    = MemSection("DDR_C7X_2_SCRATCH_PHYS", "RWIX", c7x_2_ddr_scratch_addr_phys, c7x_2_ddr_scratch_size, "Physical address of cacheable DDR for c7x_2 for Scratch Memory");

c7x_2_1_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for local heap wrt c7x_2");
c7x_2_1_ddr_local_heap      = MemSection("DDR_C7X_2_1_LOCAL_HEAP", "RWIX", c7x_2_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_1 for local heap wrt c7x_2");
c7x_2_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for Scratch Memory wrt c7x_2");
c7x_2_1_ddr_scratch         = MemSection("DDR_C7X_2_1_SCRATCH", "RWIX", c7x_2_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_1 for Scratch Memory wrt c7x_2");

c7x_2_3_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_2_3_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_3_ddr_local_heap_non_cacheable_addr, c7x_3_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for local heap wrt c7x_2");
c7x_2_3_ddr_local_heap      = MemSection("DDR_C7X_2_3_LOCAL_HEAP", "RWIX", c7x_2_3_ddr_local_heap_addr, c7x_3_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_3 for local heap wrt c7x_2");
c7x_2_3_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_3_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_3_ddr_scratch_non_cacheable_addr, c7x_3_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for Scratch Memory wrt c7x_2");
c7x_2_3_ddr_scratch         = MemSection("DDR_C7X_2_3_SCRATCH", "RWIX", c7x_2_3_ddr_scratch_addr, c7x_3_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_3 for Scratch Memory wrt c7x_2");

c7x_2_4_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_2_4_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_4_ddr_local_heap_non_cacheable_addr, c7x_4_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for local heap wrt c7x_2");
c7x_2_4_ddr_local_heap      = MemSection("DDR_C7X_2_4_LOCAL_HEAP", "RWIX", c7x_2_4_ddr_local_heap_addr, c7x_4_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_4 for local heap wrt c7x_2");
c7x_2_4_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_4_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_4_ddr_scratch_non_cacheable_addr, c7x_4_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for Scratch Memory wrt c7x_2");
c7x_2_4_ddr_scratch         = MemSection("DDR_C7X_2_4_SCRATCH", "RWIX", c7x_2_4_ddr_scratch_addr, c7x_4_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_4 for Scratch Memory wrt c7x_2");

c7x_2_ddr_total           = MemSection("DDR_C7x_2_DTS", "", 0, 0, "DDR for C7x_2 for all sections, used for reserving memory in DTS file");
c7x_2_ddr_total.concat(c7x_2_ddr_resource_table);
c7x_2_ddr_total.concat(c7x_2_ddr_ipc_trace);
c7x_2_ddr_total.concat(c7x_2_ddr_boot);
c7x_2_ddr_total.concat(c7x_2_ddr_vecs);
c7x_2_ddr_total.concat(c7x_2_ddr_secure_vecs);
c7x_2_ddr_total.concat(c7x_2_ddr);
c7x_2_ddr_total.setDtsName("vision_apps_c71_1_memory_region", "vision-apps-c71_1-memory");

c7x_3_ddr_ipc             = MemSection("DDR_C7x_3_IPC", "RWIX", c7x_3_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_3 for Linux IPC");
c7x_3_ddr_ipc.setDtsName("vision_apps_c71_2_dma_memory_region", "vision-apps-c71_2-dma-memory");
c7x_3_ddr_resource_table  = MemSection("DDR_C7x_3_RESOURCE_TABLE", "RWIX", c7x_3_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_3 for Linux resource table");
c7x_3_ddr_ipc_trace       = MemSection("DDR_C7x_3_IPC_TRACE", "RWIX", c7x_3_ddr_ipc_trace_addr, linux_ddr_ipc_trace_size, "DDR for C7x_3 for Linux IPC trace");
c7x_3_ddr_boot            = MemSection("DDR_C7x_3_BOOT", "RWIX", c7x_3_ddr_boot_addr, c7x_3_ddr_boot_size, "DDR for C7x_3 for boot section");
c7x_3_ddr_vecs            = MemSection("DDR_C7x_3_VECS", "RWIX", c7x_3_ddr_vecs_addr, c7x_3_ddr_vecs_size, "DDR for C7x_3 for vecs section");
c7x_3_ddr_secure_vecs     = MemSection("DDR_C7x_3_SECURE_VECS", "RWIX", c7x_3_ddr_secure_vecs_addr, c7x_3_ddr_secure_vecs_size, "DDR for C7x_3 for secure vecs section");
c7x_3_ddr                 = MemSection("DDR_C7x_3", "RWIX", c7x_3_ddr_addr, c7x_3_ddr_size, "DDR for C7x_3 code/data section");
c7x_3_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_3_ddr_local_heap_non_cacheable_addr, c7x_3_ddr_local_heap_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_3 for local heap");
c7x_3_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_3_ddr_local_heap_non_cacheable_addr_phys, c7x_3_ddr_local_heap_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_3 for local heap");
c7x_3_ddr_local_heap      = MemSection("DDR_C7X_3_LOCAL_HEAP", "RWIX", c7x_3_ddr_local_heap_addr, c7x_3_ddr_local_heap_size, "Virtual address of Cacheable DDR for c7x_3 for local heap");
c7x_3_ddr_local_heap_phys = MemSection("DDR_C7X_3_LOCAL_HEAP_PHYS", "RWIX", c7x_3_ddr_local_heap_addr_phys, c7x_3_ddr_local_heap_size, "Physical address of Cacheable DDR for c7x_3 for local heap");
c7x_3_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_3_SCRATCH_NON_CACHEABLE", "RWIX", c7x_3_ddr_scratch_non_cacheable_addr, c7x_3_ddr_scratch_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_3 for Scratch Memory");
c7x_3_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_3_ddr_scratch_non_cacheable_addr_phys, c7x_3_ddr_scratch_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_3 for Scratch Memory");
c7x_3_ddr_scratch         = MemSection("DDR_C7X_3_SCRATCH", "RWIX", c7x_3_ddr_scratch_addr, c7x_3_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_3 for Scratch Memory");
c7x_3_ddr_scratch_phys    = MemSection("DDR_C7X_3_SCRATCH_PHYS", "RWIX", c7x_3_ddr_scratch_addr_phys, c7x_3_ddr_scratch_size, "Physical address of cacheable DDR for c7x_3 for Scratch Memory");

c7x_3_1_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_3_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_3_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for local heap wrt c7x_3");
c7x_3_1_ddr_local_heap      = MemSection("DDR_C7X_3_1_LOCAL_HEAP", "RWIX", c7x_3_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_1 for local heap wrt c7x_3");
c7x_3_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_3_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_3_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for Scratch Memory wrt c7x_3");
c7x_3_1_ddr_scratch         = MemSection("DDR_C7X_3_1_SCRATCH", "RWIX", c7x_3_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_1 for Scratch Memory wrt c7x_3");

c7x_3_2_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_3_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_3_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for local heap wrt c7x_3");
c7x_3_2_ddr_local_heap      = MemSection("DDR_C7X_3_2_LOCAL_HEAP", "RWIX", c7x_3_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_2 for local heap wrt c7x_3");
c7x_3_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_3_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_3_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for Scratch Memory wrt c7x_3");
c7x_3_2_ddr_scratch         = MemSection("DDR_C7X_3_2_SCRATCH", "RWIX", c7x_3_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_2 for Scratch Memory wrt c7x_3");

c7x_3_4_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_3_4_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_3_4_ddr_local_heap_non_cacheable_addr, c7x_4_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for local heap wrt c7x_3");
c7x_3_4_ddr_local_heap      = MemSection("DDR_C7X_3_4_LOCAL_HEAP", "RWIX", c7x_3_4_ddr_local_heap_addr, c7x_4_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_4 for local heap wrt c7x_3");
c7x_3_4_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_3_4_SCRATCH_NON_CACHEABLE", "RWIX", c7x_3_4_ddr_scratch_non_cacheable_addr, c7x_4_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_4 for Scratch Memory wrt c7x_3");
c7x_3_4_ddr_scratch         = MemSection("DDR_C7X_3_4_SCRATCH", "RWIX", c7x_3_4_ddr_scratch_addr, c7x_4_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_4 for Scratch Memory wrt c7x_3");

c7x_3_ddr_total           = MemSection("DDR_C7x_3_DTS", "", 0, 0, "DDR for C7x_3 for all sections, used for reserving memory in DTS file");
c7x_3_ddr_total.concat(c7x_3_ddr_resource_table);
c7x_3_ddr_total.concat(c7x_3_ddr_ipc_trace);
c7x_3_ddr_total.concat(c7x_3_ddr_boot);
c7x_3_ddr_total.concat(c7x_3_ddr_vecs);
c7x_3_ddr_total.concat(c7x_3_ddr_secure_vecs);
# rmcu2_1_mmap.addMemSection( rmcu_tcma_vecs );
# rmcu2_1_mmap.addMemSection( rmcu_tcma      );
# rmcu2_1_mmap.addMemSection( rmcu_tcmb0          );
c7x_3_ddr_total.concat(c7x_3_ddr);
c7x_3_ddr_total.setDtsName("vision_apps_c71_2_memory_region", "vision-apps-c71_2-memory");

c7x_4_ddr_ipc             = MemSection("DDR_C7x_4_IPC", "RWIX", c7x_4_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_4 for Linux IPC");
c7x_4_ddr_ipc.setDtsName("vision_apps_c71_3_dma_memory_region", "vision-apps-c71_3-dma-memory");
c7x_4_ddr_resource_table  = MemSection("DDR_C7x_4_RESOURCE_TABLE", "RWIX", c7x_4_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_4 for Linux resource table");
c7x_4_ddr_ipc_trace       = MemSection("DDR_C7x_4_IPC_TRACE", "RWIX", c7x_4_ddr_ipc_trace_addr, linux_ddr_ipc_trace_size, "DDR for C7x_4 for Linux IPC trace");
c7x_4_ddr_boot            = MemSection("DDR_C7x_4_BOOT", "RWIX", c7x_4_ddr_boot_addr, c7x_4_ddr_boot_size, "DDR for C7x_4 for boot section");
c7x_4_ddr_vecs            = MemSection("DDR_C7x_4_VECS", "RWIX", c7x_4_ddr_vecs_addr, c7x_4_ddr_vecs_size, "DDR for C7x_4 for vecs section");
c7x_4_ddr_secure_vecs     = MemSection("DDR_C7x_4_SECURE_VECS", "RWIX", c7x_4_ddr_secure_vecs_addr, c7x_4_ddr_secure_vecs_size, "DDR for C7x_4 for secure vecs section");
c7x_4_ddr                 = MemSection("DDR_C7x_4", "RWIX", c7x_4_ddr_addr, c7x_4_ddr_size, "DDR for C7x_4 code/data section");
c7x_4_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_4_ddr_local_heap_non_cacheable_addr, c7x_4_ddr_local_heap_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_4 for local heap");
c7x_4_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_4_ddr_local_heap_non_cacheable_addr_phys, c7x_4_ddr_local_heap_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_4 for local heap physical addr");
c7x_4_ddr_local_heap      = MemSection("DDR_C7X_4_LOCAL_HEAP", "RWIX", c7x_4_ddr_local_heap_addr, c7x_4_ddr_local_heap_size, "Virtual address of Cacheable DDR for c7x_4 for local heap");
c7x_4_ddr_local_heap_phys      = MemSection("DDR_C7X_4_LOCAL_HEAP_PHYS", "RWIX", c7x_4_ddr_local_heap_addr_phys, c7x_4_ddr_local_heap_size, "Physical address of Cacheable DDR for c7x_4 for local heap");
c7x_4_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_4_SCRATCH_NON_CACHEABLE", "RWIX", c7x_4_ddr_scratch_non_cacheable_addr, c7x_4_ddr_scratch_non_cacheable_size, "Virtual address of Non-cacheable DDR for c7x_4 for Scratch Memory");
c7x_4_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_4_ddr_scratch_non_cacheable_addr_phys, c7x_4_ddr_scratch_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_4 for Scratch Memory");
c7x_4_ddr_scratch         = MemSection("DDR_C7X_4_SCRATCH", "RWIX", c7x_4_ddr_scratch_addr, c7x_4_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_4 for Scratch Memory");
c7x_4_ddr_scratch_phys    = MemSection("DDR_C7X_4_SCRATCH_PHYS", "RWIX", c7x_4_ddr_scratch_addr_phys, c7x_4_ddr_scratch_size, "Physical address of cacheable DDR for c7x_4 for Scratch Memory");

c7x_4_1_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_4_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for local heap wrt c7x_4");
c7x_4_1_ddr_local_heap      = MemSection("DDR_C7X_4_1_LOCAL_HEAP", "RWIX", c7x_4_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_1 for local heap wrt c7x_4");
c7x_4_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_4_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_4_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for Scratch Memory wrt c7x_4");
c7x_4_1_ddr_scratch         = MemSection("DDR_C7X_4_1_SCRATCH", "RWIX", c7x_4_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_1 for Scratch Memory wrt c7x_4");

c7x_4_2_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_4_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for local heap wrt c7x_4");
c7x_4_2_ddr_local_heap      = MemSection("DDR_C7X_4_2_LOCAL_HEAP", "RWIX", c7x_4_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_2 for local heap wrt c7x_4");
c7x_4_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_4_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_4_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for Scratch Memory wrt c7x_4");
c7x_4_2_ddr_scratch         = MemSection("DDR_C7X_4_2_SCRATCH", "RWIX", c7x_4_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_2 for Scratch Memory wrt c7x_4");

c7x_4_3_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_4_3_ddr_local_heap_non_cacheable_addr, c7x_3_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for local heap wrt c7x_4");
c7x_4_3_ddr_local_heap      = MemSection("DDR_C7X_4_3_LOCAL_HEAP", "RWIX", c7x_4_3_ddr_local_heap_addr, c7x_3_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_3 for local heap wrt c7x_4");
c7x_4_3_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_4_3_SCRATCH_NON_CACHEABLE", "RWIX", c7x_4_3_ddr_scratch_non_cacheable_addr, c7x_3_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_3 for Scratch Memory wrt c7x_4");
c7x_4_3_ddr_scratch         = MemSection("DDR_C7X_4_3_SCRATCH", "RWIX", c7x_4_3_ddr_scratch_addr, c7x_3_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_3 for Scratch Memory wrt c7x_4");

c7x_4_ddr_total           = MemSection("DDR_C7x_4_DTS", "", 0, 0, "DDR for C7x_4 for all sections, used for reserving memory in DTS file");
c7x_4_ddr_total.concat(c7x_4_ddr_resource_table);
c7x_4_ddr_total.concat(c7x_4_ddr_ipc_trace);
c7x_4_ddr_total.concat(c7x_4_ddr_boot);
c7x_4_ddr_total.concat(c7x_4_ddr_vecs);
c7x_4_ddr_total.concat(c7x_4_ddr_secure_vecs);
c7x_4_ddr_total.concat(c7x_4_ddr);
c7x_4_ddr_total.setDtsName("vision_apps_c71_3_memory_region", "vision-apps-c71_3-memory");

#TODO:
# Shared memory memory sections in DDR
app_log_mem            = MemSection("APP_LOG_MEM"        , "", app_log_mem_addr       , app_log_mem_size       , "Memory for remote core logging");
tiovx_obj_desc_mem     = MemSection("TIOVX_OBJ_DESC_MEM" , "", tiovx_obj_desc_mem_addr, tiovx_obj_desc_mem_size, "Memory for TI OpenVX shared memory. MUST be non-cached or cache-coherent");
app_fileio_mem        = MemSection("APP_FILEIO_MEM"        , "", app_fileio_mem_addr       , app_fileio_mem_size       , "Memory for remote core file operations");
tiovx_log_rt_mem     = MemSection("TIOVX_LOG_RT_MEM" , "", tiovx_log_rt_mem_addr, tiovx_log_rt_mem_size, "Memory for TI OpenVX shared memory for Run-time logging. MUST be non-cached or cache-coherent");

ipc_vring_mem      = MemSection("IPC_VRING_MEM"     , "", ipc_vring_mem_addr     , ipc_vring_mem_size     , "Memory for IPC Vring's. MUST be non-cached or cache-coherent");
ipc_vring_mem.setDtsName("vision_apps_rtos_ipc_memory_region", "vision-apps-rtos-ipc-memory-region");

vision_apps_ddr_total  = MemSection("DDR_VISION_APPS_DTS", "", 0, 0, "DDR for Vision Apps for all sections, used for reserving memory in DTS file");
vision_apps_ddr_total.concat(app_log_mem);
vision_apps_ddr_total.concat(app_fileio_mem);
vision_apps_ddr_total.concat(tiovx_obj_desc_mem);
vision_apps_ddr_total.concat(tiovx_log_rt_mem);
vision_apps_ddr_total.setDtsName("vision_apps_memory_region", "vision-apps-dma-memory");

vision_apps_core_heaps_lo = MemSection("DDR_VISION_APPS_CORE_HEAPS_LO_DTS", "", 0, 0, "Vision Apps Core Heaps in 32bit address range of DDR");
vision_apps_core_heaps_lo.concat(mcu0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu1_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu2_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu2_ddr_viss_config_heap);
vision_apps_core_heaps_lo.concat(mcu3_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu3_ddr_viss_config_heap);
vision_apps_core_heaps_lo.concat(mcu4_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu4_ddr_viss_config_heap);
vision_apps_core_heaps_lo.concat(rmcu0_0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(rmcu0_1_ddr_local_heap);
vision_apps_core_heaps_lo.concat(rmcu1_0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(rmcu1_1_ddr_local_heap);
vision_apps_core_heaps_lo.concat(rmcu2_0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(rmcu2_1_ddr_local_heap);
vision_apps_core_heaps_lo.setDtsName("vision_apps_core_heaps_lo", "vision-apps-core-heap-memory-lo");

#TODO:
c7x_ddr_high_mem_phy  = MemSection("DDR_C7X_HIGH_MEM", "RWIX", ddr_mem_addr_hi_phys, total_c7x_ddr, "High mem sections for c7x-1 thru c7x-4 for code/data, local heap and scratch");

c7x_ddr_heaps_hi = MemSection("DDR_C7X_HIGH_MEM_DTS", "", 0, 0, "C7X DDR memory sections in 40bit address range of DDR");
c7x_ddr_heaps_hi.concat(c7x_ddr_high_mem_phy);
c7x_ddr_heaps_hi.setDtsName("c7x_ddr_heaps_hi", "c7x-ddr-heaps-hi");
c7x_ddr_heaps_hi.splitOrigin(True)

#TODO:
# this region should NOT have the "no-map" flag since we want ION to map this memory and do cache ops on it as needed
ddr_shared_mem     = MemSection("DDR_SHARED_MEM"    , "", ddr_shared_mem_addr    , ddr_shared_mem_size    , "Memory for shared memory buffers in DDR");
ddr_shared_mem.setDtsName("vision_apps_shared_region", "vision_apps_shared-memories");
ddr_shared_mem.setCompatibility("dma-heap-carveout");
ddr_shared_mem.setNoMap(False);
ddr_shared_mem.setOriginTag(False);
ddr_shared_mem.splitOrigin(True)
ddr_shared_mem_phys  = MemSection("DDR_SHARED_MEM_PHYS"    , "", ddr_shared_mem_addr_phys  , ddr_shared_mem_size    , "Physical address of memory for shared memory buffers in DDR");
ddr_shared_mem_phys.setDtsName("vision_apps_shared_region", "vision_apps_shared-memories");
ddr_shared_mem_phys.setCompatibility("dma-heap-carveout");
ddr_shared_mem_phys.setNoMap(False);
ddr_shared_mem_phys.setOriginTag(False);
ddr_shared_mem_phys.splitOrigin(True)

# TODO: EthFW ?
# This region is for ethernet firmware, multi-core, multi-cast feature
# intercore_eth_desc_mem = MemSection("INTERCORE_ETH_DESC_MEM", "", ddr_intercore_eth_desc_addr, ddr_intercore_eth_desc_size, "Inter-core ethernet shared desc queues. MUST be non-cached or cache-coherent");
# intercore_eth_desc_mem.setDtsName("vision_apps_main_r5fss0_core0_shared_memory_queue_region", "vision-apps-r5f-virtual-eth-queues");
#
# intercore_eth_data_mem = MemSection("INTERCORE_ETH_DATA_MEM", "", ddr_intercore_eth_data_addr, ddr_intercore_eth_data_size, "Inter-core ethernet shared data buffers. MUST be non-cached or cache-coherent");
# intercore_eth_data_mem.setDtsName("vision_apps_main_r5fss0_core0_shared_memory_bufpool_region", "vision-apps-r5f-virtual-eth-buffers");

#
# Create CPU specific memory maps using memory sections created above
#

# dmcu0_mmap = MemoryMap("dmcu0");
# dmcu0_mmap.addMemSection( dmcu0_ddr           );
# dmcu0_mmap.addMemSection( app_log_mem          );
# dmcu0_mmap.addMemSection( tiovx_obj_desc_mem   );
# dmcu0_mmap.addMemSection( app_fileio_mem          );
# dmcu0_mmap.addMemSection( ipc_vring_mem        );
# dmcu0_mmap.addMemSection( ddr_shared_mem       );
# dmcu0_mmap.checkOverlap();

mcu0_mmap = MemoryMap("mcu0");
mcu0_mmap.addMemSection( mcu0_tcma_vecs     );
mcu0_mmap.addMemSection( mcu0_tcma     );
mcu0_mmap.addMemSection( mcu0_ddr_ipc     );
mcu0_mmap.addMemSection( mcu0_ddr_resource_table  );
mcu0_mmap.addMemSection( mcu0_ddr_ipc_trace  );
mcu0_mmap.addMemSection( mcu0_ddr         );
mcu0_mmap.addMemSection( app_log_mem        );
mcu0_mmap.addMemSection( tiovx_obj_desc_mem );
mcu0_mmap.addMemSection( app_fileio_mem        );
mcu0_mmap.addMemSection( ipc_vring_mem      );
mcu0_mmap.addMemSection( mcu0_ddr_local_heap  );
mcu0_mmap.addMemSection( ddr_shared_mem     );
# mcu0_mmap.addMemSection( intercore_eth_desc_mem );
# mcu0_mmap.addMemSection( intercore_eth_data_mem );
mcu0_mmap.checkOverlap();

mcu1_mmap = MemoryMap("mcu1");
mcu1_mmap.addMemSection( mcu1_tcma_vecs     );
mcu1_mmap.addMemSection( mcu1_tcma     );
mcu1_mmap.addMemSection( mcu1_ddr_ipc     );
mcu1_mmap.addMemSection( mcu1_ddr_resource_table  );
mcu1_mmap.addMemSection( mcu1_ddr_ipc_trace  );
mcu1_mmap.addMemSection( mcu1_ddr         );
mcu1_mmap.addMemSection( app_log_mem        );
mcu1_mmap.addMemSection( tiovx_obj_desc_mem );
mcu1_mmap.addMemSection( app_fileio_mem        );
mcu1_mmap.addMemSection( ipc_vring_mem      );
mcu1_mmap.addMemSection( mcu1_ddr_local_heap  );
mcu1_mmap.addMemSection( ddr_shared_mem     );
mcu1_mmap.checkOverlap();

mcu2_mmap = MemoryMap("mcu2");
mcu2_mmap.addMemSection( mcu2_tcma_vecs     );
mcu2_mmap.addMemSection( mcu2_tcma     );
mcu2_mmap.addMemSection( mcu2_ddr_ipc     );
mcu2_mmap.addMemSection( mcu2_ddr_resource_table  );
mcu2_mmap.addMemSection( mcu2_ddr_ipc_trace  );
mcu2_mmap.addMemSection( mcu2_ddr         );
mcu2_mmap.addMemSection( app_log_mem        );
mcu2_mmap.addMemSection( tiovx_obj_desc_mem );
mcu2_mmap.addMemSection( app_fileio_mem        );
mcu2_mmap.addMemSection( ipc_vring_mem      );
mcu2_mmap.addMemSection( mcu2_ddr_local_heap  );
mcu2_mmap.addMemSection( mcu2_ddr_viss_config_heap  );
mcu2_mmap.addMemSection( ddr_shared_mem     );
mcu2_mmap.addMemSection( mcu2_main_ocram );
mcu2_mmap.checkOverlap();

mcu3_mmap = MemoryMap("mcu3");
mcu3_mmap.addMemSection( mcu3_tcma_vecs     );
mcu3_mmap.addMemSection( mcu3_tcma     );
mcu3_mmap.addMemSection( mcu3_ddr_ipc     );
mcu3_mmap.addMemSection( mcu3_ddr_resource_table  );
mcu3_mmap.addMemSection( mcu3_ddr_ipc_trace  );
mcu3_mmap.addMemSection( mcu3_ddr         );
mcu3_mmap.addMemSection( app_log_mem        );
mcu3_mmap.addMemSection( tiovx_obj_desc_mem );
mcu3_mmap.addMemSection( app_fileio_mem        );
mcu3_mmap.addMemSection( ipc_vring_mem      );
mcu3_mmap.addMemSection( mcu3_ddr_local_heap  );
mcu3_mmap.addMemSection( mcu3_ddr_viss_config_heap  );
mcu3_mmap.addMemSection( ddr_shared_mem     );
mcu3_mmap.addMemSection( mcu3_main_ocram );
mcu3_mmap.checkOverlap();

mcu4_mmap = MemoryMap("mcu4");
mcu4_mmap.addMemSection( mcu4_tcma_vecs     );
mcu4_mmap.addMemSection( mcu4_tcma     );
mcu4_mmap.addMemSection( mcu4_ddr_ipc     );
mcu4_mmap.addMemSection( mcu4_ddr_resource_table  );
mcu4_mmap.addMemSection( mcu4_ddr_ipc_trace  );
mcu4_mmap.addMemSection( mcu4_ddr         );
mcu4_mmap.addMemSection( app_log_mem        );
mcu4_mmap.addMemSection( tiovx_obj_desc_mem );
mcu4_mmap.addMemSection( app_fileio_mem        );
mcu4_mmap.addMemSection( ipc_vring_mem      );
mcu4_mmap.addMemSection( mcu4_ddr_local_heap  );
mcu4_mmap.addMemSection( mcu4_ddr_viss_config_heap  );
mcu4_mmap.addMemSection( ddr_shared_mem     );
mcu4_mmap.addMemSection( mcu4_main_ocram );
mcu4_mmap.checkOverlap();

rmcu0_0_mmap = MemoryMap("rmcu0_0");
rmcu0_0_mmap.addMemSection( rmcu0_0_tcm         );
rmcu0_0_mmap.addMemSection( rmcu0_0_ddr_ipc     );
rmcu0_0_mmap.addMemSection( rmcu0_0_ddr_resource_table  );
rmcu0_0_mmap.addMemSection( rmcu0_0_ddr_ipc_trace  );
rmcu0_0_mmap.addMemSection( rmcu0_0_ddr         );
rmcu0_0_mmap.addMemSection( rmcu0_0_ddr_local_heap  );
rmcu0_0_mmap.addMemSection( app_log_mem        );
rmcu0_0_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu0_0_mmap.addMemSection( app_fileio_mem        );
rmcu0_0_mmap.addMemSection( ipc_vring_mem      );
rmcu0_0_mmap.addMemSection( ddr_shared_mem     );
rmcu0_0_mmap.checkOverlap();

rmcu0_1_mmap = MemoryMap("rmcu0_1");
rmcu0_1_mmap.addMemSection( rmcu0_1_tcm         );
rmcu0_1_mmap.addMemSection( rmcu0_1_ddr_ipc     );
rmcu0_1_mmap.addMemSection( rmcu0_1_ddr_resource_table  );
rmcu0_1_mmap.addMemSection( rmcu0_1_ddr_ipc_trace  );
rmcu0_1_mmap.addMemSection( rmcu0_1_ddr         );
rmcu0_1_mmap.addMemSection( rmcu0_1_ddr_local_heap  );
rmcu0_1_mmap.addMemSection( app_log_mem        );
rmcu0_1_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu0_1_mmap.addMemSection( app_fileio_mem        );
rmcu0_1_mmap.addMemSection( ipc_vring_mem      );
rmcu0_1_mmap.addMemSection( ddr_shared_mem     );
rmcu0_1_mmap.checkOverlap();

rmcu1_0_mmap = MemoryMap("rmcu1_0");
rmcu1_0_mmap.addMemSection( rmcu1_0_tcm         );
rmcu1_0_mmap.addMemSection( rmcu1_0_ddr_ipc     );
rmcu1_0_mmap.addMemSection( rmcu1_0_ddr_resource_table  );
rmcu1_0_mmap.addMemSection( rmcu1_0_ddr_ipc_trace  );
rmcu1_0_mmap.addMemSection( rmcu1_0_ddr         );
rmcu1_0_mmap.addMemSection( rmcu1_0_ddr_local_heap  );
rmcu1_0_mmap.addMemSection( app_log_mem        );
rmcu1_0_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu1_0_mmap.addMemSection( app_fileio_mem        );
rmcu1_0_mmap.addMemSection( ipc_vring_mem      );
rmcu1_0_mmap.addMemSection( ddr_shared_mem     );
rmcu1_0_mmap.checkOverlap();

rmcu1_1_mmap = MemoryMap("rmcu1_1");
rmcu1_1_mmap.addMemSection( rmcu1_1_tcm         );
rmcu1_1_mmap.addMemSection( rmcu1_1_ddr_ipc     );
rmcu1_1_mmap.addMemSection( rmcu1_1_ddr_resource_table  );
rmcu1_1_mmap.addMemSection( rmcu1_1_ddr_ipc_trace  );
rmcu1_1_mmap.addMemSection( rmcu1_1_ddr         );
rmcu1_1_mmap.addMemSection( rmcu1_1_ddr_local_heap  );
rmcu1_1_mmap.addMemSection( app_log_mem        );
rmcu1_1_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu1_1_mmap.addMemSection( app_fileio_mem        );
rmcu1_1_mmap.addMemSection( ipc_vring_mem      );
rmcu1_1_mmap.addMemSection( ddr_shared_mem     );
rmcu1_1_mmap.checkOverlap();

rmcu2_0_mmap = MemoryMap("rmcu2_0");
rmcu2_0_mmap.addMemSection( rmcu2_0_tcm         );
rmcu2_0_mmap.addMemSection( rmcu2_0_ddr_ipc     );
rmcu2_0_mmap.addMemSection( rmcu2_0_ddr_resource_table  );
rmcu2_0_mmap.addMemSection( rmcu2_0_ddr_ipc_trace  );
rmcu2_0_mmap.addMemSection( rmcu2_0_ddr         );
rmcu2_0_mmap.addMemSection( rmcu2_0_ddr_local_heap  );
rmcu2_0_mmap.addMemSection( app_log_mem        );
rmcu2_0_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu2_0_mmap.addMemSection( app_fileio_mem        );
rmcu2_0_mmap.addMemSection( ipc_vring_mem      );
rmcu2_0_mmap.addMemSection( ddr_shared_mem     );
rmcu2_0_mmap.checkOverlap();

rmcu2_1_mmap = MemoryMap("rmcu2_1");
rmcu2_1_mmap.addMemSection( rmcu2_1_tcm         );
rmcu2_1_mmap.addMemSection( rmcu2_1_ddr_ipc     );
rmcu2_1_mmap.addMemSection( rmcu2_1_ddr_resource_table  );
rmcu2_1_mmap.addMemSection( rmcu2_1_ddr_ipc_trace  );
rmcu2_1_mmap.addMemSection( rmcu2_1_ddr         );
rmcu2_1_mmap.addMemSection( rmcu2_1_ddr_local_heap  );
rmcu2_1_mmap.addMemSection( app_log_mem        );
rmcu2_1_mmap.addMemSection( tiovx_obj_desc_mem );
rmcu2_1_mmap.addMemSection( app_fileio_mem        );
rmcu2_1_mmap.addMemSection( ipc_vring_mem      );
rmcu2_1_mmap.addMemSection( ddr_shared_mem     );
rmcu2_1_mmap.checkOverlap();

c7x_1_mmap = MemoryMap("c7x_1");
c7x_1_mmap.addMemSection( c7x_1_l2           );
# c7x_1_mmap.addMemSection( c7x_1_l1           );
c7x_1_mmap.addMemSection( c7x_1_msmc         );
c7x_1_mmap.addMemSection( c7x_1_ddr_ipc      );
c7x_1_mmap.addMemSection( c7x_1_ddr_resource_table      );
c7x_1_mmap.addMemSection( c7x_1_ddr_ipc_trace      );
c7x_1_mmap.addMemSection( c7x_1_ddr_boot     );
c7x_1_mmap.addMemSection( c7x_1_ddr_vecs     );
c7x_1_mmap.addMemSection( c7x_1_ddr_secure_vecs     );
c7x_1_mmap.addMemSection( c7x_1_ddr          );
c7x_1_mmap.addMemSection( app_log_mem        );
c7x_1_mmap.addMemSection( tiovx_obj_desc_mem );
c7x_1_mmap.addMemSection( app_fileio_mem        );
c7x_1_mmap.addMemSection( ipc_vring_mem      );
c7x_1_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_ddr_local_heap  );
c7x_1_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_ddr_scratch  );
c7x_1_mmap.addMemSection( c7x_1_2_ddr_local_heap_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_2_ddr_local_heap  );
c7x_1_mmap.addMemSection( c7x_1_2_ddr_scratch_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_2_ddr_scratch  );
c7x_1_mmap.addMemSection( c7x_1_3_ddr_local_heap_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_3_ddr_local_heap  );
c7x_1_mmap.addMemSection( c7x_1_3_ddr_scratch_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_3_ddr_scratch  );
c7x_1_mmap.addMemSection( c7x_1_4_ddr_local_heap_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_4_ddr_local_heap  );
c7x_1_mmap.addMemSection( c7x_1_4_ddr_scratch_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_4_ddr_scratch  );
c7x_1_mmap.addMemSection( ddr_shared_mem     );
c7x_1_mmap.checkOverlap();

c7x_2_mmap = MemoryMap("c7x_2");
c7x_2_mmap.addMemSection( c7x_2_l2           );
# c7x_2_mmap.addMemSection( c7x_2_l1           );
c7x_2_mmap.addMemSection( c7x_2_msmc         );
c7x_2_mmap.addMemSection( c7x_2_ddr_ipc      );
c7x_2_mmap.addMemSection( c7x_2_ddr_resource_table      );
c7x_2_mmap.addMemSection( c7x_2_ddr_ipc_trace      );
c7x_2_mmap.addMemSection( c7x_2_ddr_boot     );
c7x_2_mmap.addMemSection( c7x_2_ddr_vecs     );
c7x_2_mmap.addMemSection( c7x_2_ddr_secure_vecs     );
c7x_2_mmap.addMemSection( c7x_2_ddr          );
c7x_2_mmap.addMemSection( app_log_mem        );
c7x_2_mmap.addMemSection( tiovx_obj_desc_mem );
c7x_2_mmap.addMemSection( app_fileio_mem        );
c7x_2_mmap.addMemSection( ipc_vring_mem      );
c7x_2_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_ddr_local_heap  );
c7x_2_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_ddr_scratch  );
c7x_2_mmap.addMemSection( c7x_2_1_ddr_local_heap_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_1_ddr_local_heap  );
c7x_2_mmap.addMemSection( c7x_2_1_ddr_scratch_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_1_ddr_scratch  );
c7x_2_mmap.addMemSection( c7x_2_3_ddr_local_heap_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_3_ddr_local_heap  );
c7x_2_mmap.addMemSection( c7x_2_3_ddr_scratch_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_3_ddr_scratch  );
c7x_2_mmap.addMemSection( c7x_2_4_ddr_local_heap_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_4_ddr_local_heap  );
c7x_2_mmap.addMemSection( c7x_2_4_ddr_scratch_non_cacheable  );
c7x_2_mmap.addMemSection( c7x_2_4_ddr_scratch  );
c7x_2_mmap.addMemSection( ddr_shared_mem     );
c7x_2_mmap.checkOverlap();
#
c7x_3_mmap = MemoryMap("c7x_3");
c7x_3_mmap.addMemSection( c7x_3_l2           );
# c7x_3_mmap.addMemSection( c7x_3_l1           );
c7x_3_mmap.addMemSection( c7x_3_msmc         );
c7x_3_mmap.addMemSection( c7x_3_ddr_ipc      );
c7x_3_mmap.addMemSection( c7x_3_ddr_resource_table      );
c7x_3_mmap.addMemSection( c7x_3_ddr_ipc_trace      );
c7x_3_mmap.addMemSection( c7x_3_ddr_boot     );
c7x_3_mmap.addMemSection( c7x_3_ddr_vecs     );
c7x_3_mmap.addMemSection( c7x_3_ddr_secure_vecs     );
c7x_3_mmap.addMemSection( c7x_3_ddr          );
c7x_3_mmap.addMemSection( app_log_mem        );
c7x_3_mmap.addMemSection( tiovx_obj_desc_mem );
c7x_3_mmap.addMemSection( app_fileio_mem        );
c7x_3_mmap.addMemSection( ipc_vring_mem      );
c7x_3_mmap.addMemSection( c7x_3_ddr_local_heap_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_ddr_local_heap  );
c7x_3_mmap.addMemSection( c7x_3_ddr_scratch_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_ddr_scratch  );
c7x_3_mmap.addMemSection( c7x_3_1_ddr_local_heap_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_1_ddr_local_heap  );
c7x_3_mmap.addMemSection( c7x_3_1_ddr_scratch_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_1_ddr_scratch  );
c7x_3_mmap.addMemSection( c7x_3_2_ddr_local_heap_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_2_ddr_local_heap  );
c7x_3_mmap.addMemSection( c7x_3_2_ddr_scratch_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_2_ddr_scratch  );
c7x_3_mmap.addMemSection( c7x_3_4_ddr_local_heap_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_4_ddr_local_heap  );
c7x_3_mmap.addMemSection( c7x_3_4_ddr_scratch_non_cacheable  );
c7x_3_mmap.addMemSection( c7x_3_4_ddr_scratch  );
c7x_3_mmap.addMemSection( ddr_shared_mem     );
c7x_3_mmap.checkOverlap();
#
c7x_4_mmap = MemoryMap("c7x_4");
c7x_4_mmap.addMemSection( c7x_4_l2           );
# c7x_4_mmap.addMemSection( c7x_4_l1           );
c7x_4_mmap.addMemSection( c7x_4_msmc         );
c7x_4_mmap.addMemSection( c7x_4_ddr_ipc      );
c7x_4_mmap.addMemSection( c7x_4_ddr_resource_table      );
c7x_4_mmap.addMemSection( c7x_4_ddr_ipc_trace      );
c7x_4_mmap.addMemSection( c7x_4_ddr_boot     );
c7x_4_mmap.addMemSection( c7x_4_ddr_vecs     );
c7x_4_mmap.addMemSection( c7x_4_ddr_secure_vecs     );
c7x_4_mmap.addMemSection( c7x_4_ddr          );
c7x_4_mmap.addMemSection( app_log_mem        );
c7x_4_mmap.addMemSection( tiovx_obj_desc_mem );
c7x_4_mmap.addMemSection( app_fileio_mem        );
c7x_4_mmap.addMemSection( ipc_vring_mem      );
c7x_4_mmap.addMemSection( c7x_4_ddr_local_heap_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_ddr_local_heap  );
c7x_4_mmap.addMemSection( c7x_4_ddr_scratch_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_ddr_scratch  );
c7x_4_mmap.addMemSection( c7x_4_1_ddr_local_heap_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_1_ddr_local_heap  );
c7x_4_mmap.addMemSection( c7x_4_1_ddr_scratch_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_1_ddr_scratch  );
c7x_4_mmap.addMemSection( c7x_4_2_ddr_local_heap_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_2_ddr_local_heap  );
c7x_4_mmap.addMemSection( c7x_4_2_ddr_scratch_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_2_ddr_scratch  );
c7x_4_mmap.addMemSection( c7x_4_3_ddr_local_heap_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_3_ddr_local_heap  );
c7x_4_mmap.addMemSection( c7x_4_3_ddr_scratch_non_cacheable  );
c7x_4_mmap.addMemSection( c7x_4_3_ddr_scratch  );
c7x_4_mmap.addMemSection( ddr_shared_mem     );
c7x_4_mmap.checkOverlap();

#TODO:
html_mmap = MemoryMap("System Memory Map for Linux+RTOS mode");
html_mmap.addMemSection( c7x_1_l2           );
# html_mmap.addMemSection( c7x_1_l1           );
html_mmap.addMemSection( c7x_2_l2           );
# html_mmap.addMemSection( c7x_2_l1           );
html_mmap.addMemSection( c7x_3_l2           );
# html_mmap.addMemSection( c7x_3_l1           );
html_mmap.addMemSection( c7x_4_l2           );
# html_mmap.addMemSection( c7x_4_l1           );
html_mmap.addMemSection( c7x_1_msmc         );
html_mmap.addMemSection( c7x_2_msmc         );
html_mmap.addMemSection( c7x_3_msmc         );
html_mmap.addMemSection( c7x_4_msmc         );
html_mmap.addMemSection( dmcu0_ddr         );
html_mmap.addMemSection( mcu0_ddr_ipc     );
html_mmap.addMemSection( mcu0_ddr_resource_table      );
html_mmap.addMemSection( mcu0_ddr_ipc_trace      );
html_mmap.addMemSection( mcu0_ddr         );
html_mmap.addMemSection( mcu0_ddr_local_heap );
html_mmap.addMemSection( mcu1_ddr_ipc     );
html_mmap.addMemSection( mcu1_ddr_resource_table      );
html_mmap.addMemSection( mcu1_ddr_ipc_trace      );
html_mmap.addMemSection( mcu1_ddr         );
html_mmap.addMemSection( mcu1_ddr_local_heap );
html_mmap.addMemSection( mcu2_ddr_ipc     );
html_mmap.addMemSection( mcu2_ddr_resource_table      );
html_mmap.addMemSection( mcu2_ddr_ipc_trace      );
html_mmap.addMemSection( mcu2_ddr         );
html_mmap.addMemSection( mcu2_ddr_local_heap );
html_mmap.addMemSection( mcu2_ddr_viss_config_heap );
html_mmap.addMemSection( mcu3_ddr_ipc     );
html_mmap.addMemSection( mcu3_ddr_resource_table      );
html_mmap.addMemSection( mcu3_ddr_ipc_trace      );
html_mmap.addMemSection( mcu3_ddr         );
html_mmap.addMemSection( mcu3_ddr_local_heap );
html_mmap.addMemSection( mcu3_ddr_viss_config_heap );
html_mmap.addMemSection( mcu4_ddr_ipc     );
html_mmap.addMemSection( mcu4_ddr_resource_table      );
html_mmap.addMemSection( mcu4_ddr_ipc_trace      );
html_mmap.addMemSection( mcu4_ddr         );
html_mmap.addMemSection( mcu4_ddr_local_heap );
html_mmap.addMemSection( mcu4_ddr_viss_config_heap );
html_mmap.addMemSection( rmcu0_0_ddr_ipc     );
html_mmap.addMemSection( rmcu0_0_ddr_resource_table      );
html_mmap.addMemSection( rmcu0_0_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu0_0_ddr         );
html_mmap.addMemSection( rmcu0_0_ddr_local_heap         );
html_mmap.addMemSection( rmcu0_1_ddr_ipc     );
html_mmap.addMemSection( rmcu0_1_ddr_resource_table      );
html_mmap.addMemSection( rmcu0_1_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu0_1_ddr         );
html_mmap.addMemSection( rmcu0_1_ddr_local_heap         );
html_mmap.addMemSection( rmcu1_0_ddr_ipc     );
html_mmap.addMemSection( rmcu1_0_ddr_resource_table      );
html_mmap.addMemSection( rmcu1_0_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu1_0_ddr         );
html_mmap.addMemSection( rmcu1_0_ddr_local_heap         );
html_mmap.addMemSection( rmcu1_1_ddr_ipc     );
html_mmap.addMemSection( rmcu1_1_ddr_resource_table      );
html_mmap.addMemSection( rmcu1_1_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu1_1_ddr         );
html_mmap.addMemSection( rmcu1_1_ddr_local_heap         );
html_mmap.addMemSection( rmcu2_0_ddr_ipc     );
html_mmap.addMemSection( rmcu2_0_ddr_resource_table      );
html_mmap.addMemSection( rmcu2_0_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu2_0_ddr_local_heap         );
html_mmap.addMemSection( rmcu2_0_ddr         );
html_mmap.addMemSection( rmcu2_1_ddr_ipc     );
html_mmap.addMemSection( rmcu2_1_ddr_resource_table      );
html_mmap.addMemSection( rmcu2_1_ddr_ipc_trace      );
html_mmap.addMemSection( rmcu2_1_ddr         );
html_mmap.addMemSection( rmcu2_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_ddr_ipc     );
html_mmap.addMemSection( c7x_1_ddr_resource_table     );
html_mmap.addMemSection( c7x_1_ddr_ipc_trace      );
html_mmap.addMemSection( c7x_1_ddr_boot    );
html_mmap.addMemSection( c7x_1_ddr_vecs    );
html_mmap.addMemSection( c7x_1_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_1_ddr         );
html_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_ddr_scratch );
html_mmap.addMemSection( c7x_1_2_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_1_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_2_ddr_scratch );
html_mmap.addMemSection( c7x_1_3_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_1_3_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_3_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_3_ddr_scratch );
html_mmap.addMemSection( c7x_1_4_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_1_4_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_4_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_4_ddr_scratch );
html_mmap.addMemSection( c7x_2_ddr_ipc     );
html_mmap.addMemSection( c7x_2_ddr_resource_table     );
html_mmap.addMemSection( c7x_2_ddr_ipc_trace      );
html_mmap.addMemSection( c7x_2_ddr_boot    );
html_mmap.addMemSection( c7x_2_ddr_vecs    );
html_mmap.addMemSection( c7x_2_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_2_ddr         );
html_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_ddr_scratch );
html_mmap.addMemSection( c7x_2_1_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_2_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_1_ddr_scratch );
html_mmap.addMemSection( c7x_2_3_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_2_3_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_3_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_3_ddr_scratch );
html_mmap.addMemSection( c7x_2_4_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_2_4_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_4_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_4_ddr_scratch );
html_mmap.addMemSection( c7x_3_ddr_ipc     );
html_mmap.addMemSection( c7x_3_ddr_resource_table     );
html_mmap.addMemSection( c7x_3_ddr_ipc_trace      );
html_mmap.addMemSection( c7x_3_ddr_boot    );
html_mmap.addMemSection( c7x_3_ddr_vecs    );
html_mmap.addMemSection( c7x_3_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_3_ddr         );
html_mmap.addMemSection( c7x_3_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_3_ddr_local_heap         );
html_mmap.addMemSection( c7x_3_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_3_ddr_scratch );
html_mmap.addMemSection( c7x_3_1_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_3_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_3_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_3_1_ddr_scratch );
html_mmap.addMemSection( c7x_3_2_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_3_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_3_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_3_2_ddr_scratch );
html_mmap.addMemSection( c7x_3_4_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_3_4_ddr_local_heap         );
html_mmap.addMemSection( c7x_3_4_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_3_4_ddr_scratch );
html_mmap.addMemSection( c7x_4_ddr_ipc     );
html_mmap.addMemSection( c7x_4_ddr_resource_table     );
html_mmap.addMemSection( c7x_4_ddr_ipc_trace     );
html_mmap.addMemSection( c7x_4_ddr_boot    );
html_mmap.addMemSection( c7x_4_ddr_vecs    );
html_mmap.addMemSection( c7x_4_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_4_ddr         );
html_mmap.addMemSection( c7x_4_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_4_ddr_local_heap         );
html_mmap.addMemSection( c7x_4_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_4_ddr_scratch );
html_mmap.addMemSection( c7x_4_1_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_4_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_4_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_4_1_ddr_scratch );
html_mmap.addMemSection( c7x_4_2_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_4_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_4_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_4_2_ddr_scratch );
html_mmap.addMemSection( c7x_4_3_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_4_3_ddr_local_heap         );
html_mmap.addMemSection( c7x_4_3_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_4_3_ddr_scratch );

html_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_1_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_1_ddr_scratch_phys );
html_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_2_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_2_ddr_scratch_phys );
html_mmap.addMemSection( c7x_3_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_3_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_3_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_3_ddr_scratch_phys );
html_mmap.addMemSection( c7x_4_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_4_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_4_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_4_ddr_scratch_phys );

html_mmap.addMemSection( uboot_reloc_mem    );
html_mmap.addMemSection( app_log_mem        );
html_mmap.addMemSection( tiovx_obj_desc_mem );
html_mmap.addMemSection( app_fileio_mem        );
html_mmap.addMemSection( ipc_vring_mem      );
html_mmap.addMemSection( ddr_shared_mem     );
html_mmap.addMemSection( tiovx_log_rt_mem );
html_mmap.addMemSection( mcu2_main_ocram );
html_mmap.addMemSection( mcu3_main_ocram );
html_mmap.addMemSection( mcu4_main_ocram );
#html_mmap.addMemSection( intercore_eth_desc_mem );
#html_mmap.addMemSection( intercore_eth_data_mem );
html_mmap.addMemSection( ddr_shared_mem_phys     );
html_mmap.checkOverlap();

c_header_mmap = MemoryMap("Memory Map for C header file");
c_header_mmap.addMemSection( c7x_1_l2           );
# c_header_mmap.addMemSection( c7x_1_l1           );
c_header_mmap.addMemSection( c7x_1_msmc         );
c_header_mmap.addMemSection( c7x_2_l2           );
# c_header_mmap.addMemSection( c7x_2_l1           );
c_header_mmap.addMemSection( c7x_2_msmc         );
c_header_mmap.addMemSection( c7x_3_l2           );
# c_header_mmap.addMemSection( c7x_3_l1           );
c_header_mmap.addMemSection( c7x_3_msmc         );
c_header_mmap.addMemSection( c7x_4_l2           );
# c_header_mmap.addMemSection( c7x_4_l1           );
c_header_mmap.addMemSection( c7x_4_msmc         );
c_header_mmap.addMemSection( mcu0_ddr_ipc     );
c_header_mmap.addMemSection( mcu1_ddr_ipc     );
c_header_mmap.addMemSection( mcu2_ddr_ipc     );
c_header_mmap.addMemSection( mcu3_ddr_ipc     );
c_header_mmap.addMemSection( mcu4_ddr_ipc     );
c_header_mmap.addMemSection( rmcu0_0_ddr_ipc     );
c_header_mmap.addMemSection( rmcu0_1_ddr_ipc     );
c_header_mmap.addMemSection( rmcu1_0_ddr_ipc     );
c_header_mmap.addMemSection( rmcu1_1_ddr_ipc     );
c_header_mmap.addMemSection( rmcu2_0_ddr_ipc     );
c_header_mmap.addMemSection( rmcu2_1_ddr_ipc     );
c_header_mmap.addMemSection( c7x_1_ddr_ipc     );
c_header_mmap.addMemSection( c7x_2_ddr_ipc     );
c_header_mmap.addMemSection( c7x_3_ddr_ipc     );
c_header_mmap.addMemSection( c7x_4_ddr_ipc     );
c_header_mmap.addMemSection( dmcu0_ddr_total     );
c_header_mmap.addMemSection( mcu0_ddr_total     );
c_header_mmap.addMemSection( mcu1_ddr_total     );
c_header_mmap.addMemSection( mcu2_ddr_total     );
c_header_mmap.addMemSection( mcu3_ddr_total     );
c_header_mmap.addMemSection( mcu4_ddr_total     );
c_header_mmap.addMemSection( rmcu0_0_ddr_total     );
c_header_mmap.addMemSection( rmcu0_1_ddr_total     );
c_header_mmap.addMemSection( rmcu1_0_ddr_total     );
c_header_mmap.addMemSection( rmcu1_1_ddr_total     );
c_header_mmap.addMemSection( rmcu2_0_ddr_total     );
c_header_mmap.addMemSection( rmcu2_1_ddr_total     );
c_header_mmap.addMemSection( c7x_1_ddr_total     );
c_header_mmap.addMemSection( c7x_2_ddr_total     );
c_header_mmap.addMemSection( c7x_3_ddr_total     );
c_header_mmap.addMemSection( c7x_4_ddr_total     );

c_header_mmap.addMemSection( mcu0_ddr_local_heap);
c_header_mmap.addMemSection( mcu1_ddr_local_heap);
c_header_mmap.addMemSection( mcu2_ddr_local_heap);
c_header_mmap.addMemSection( mcu2_ddr_viss_config_heap);
c_header_mmap.addMemSection( mcu3_ddr_local_heap);
c_header_mmap.addMemSection( mcu3_ddr_viss_config_heap);
c_header_mmap.addMemSection( mcu4_ddr_local_heap);
c_header_mmap.addMemSection( mcu4_ddr_viss_config_heap);
c_header_mmap.addMemSection( rmcu0_0_ddr_local_heap);
c_header_mmap.addMemSection( rmcu0_1_ddr_local_heap);
c_header_mmap.addMemSection( rmcu1_0_ddr_local_heap);
c_header_mmap.addMemSection( rmcu1_1_ddr_local_heap);
c_header_mmap.addMemSection( rmcu2_0_ddr_local_heap);
c_header_mmap.addMemSection( rmcu2_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap_phys);
c_header_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_1_ddr_scratch);
c_header_mmap.addMemSection( c7x_1_ddr_scratch_phys);
c_header_mmap.addMemSection( c7x_1_2_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_1_2_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_2_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_1_2_ddr_scratch);
c_header_mmap.addMemSection( c7x_1_3_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_1_3_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_3_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_1_3_ddr_scratch);
c_header_mmap.addMemSection( c7x_1_4_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_1_4_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_4_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_1_4_ddr_scratch);

c_header_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_2_ddr_local_heap);
c_header_mmap.addMemSection( c7x_2_ddr_local_heap_phys);
c_header_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_2_ddr_scratch);
c_header_mmap.addMemSection( c7x_2_ddr_scratch_phys);
c_header_mmap.addMemSection( c7x_2_1_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_2_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_2_1_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_2_1_ddr_scratch);
c_header_mmap.addMemSection( c7x_2_3_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_2_3_ddr_local_heap);
c_header_mmap.addMemSection( c7x_2_3_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_2_3_ddr_scratch);
c_header_mmap.addMemSection( c7x_2_4_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_2_4_ddr_local_heap);
c_header_mmap.addMemSection( c7x_2_4_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_2_4_ddr_scratch);

c_header_mmap.addMemSection( c7x_3_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_3_ddr_local_heap_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_3_ddr_local_heap);
c_header_mmap.addMemSection( c7x_3_ddr_local_heap_phys);
c_header_mmap.addMemSection( c7x_3_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_3_ddr_scratch_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_3_ddr_scratch);
c_header_mmap.addMemSection( c7x_3_ddr_scratch_phys);
c_header_mmap.addMemSection( c7x_3_1_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_3_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_3_1_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_3_1_ddr_scratch);
c_header_mmap.addMemSection( c7x_3_2_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_3_2_ddr_local_heap);
c_header_mmap.addMemSection( c7x_3_2_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_3_2_ddr_scratch);
c_header_mmap.addMemSection( c7x_3_4_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_3_4_ddr_local_heap);
c_header_mmap.addMemSection( c7x_3_4_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_3_4_ddr_scratch);

c_header_mmap.addMemSection( c7x_4_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_4_ddr_local_heap_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_4_ddr_local_heap);
c_header_mmap.addMemSection( c7x_4_ddr_local_heap_phys);
c_header_mmap.addMemSection( c7x_4_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_4_ddr_scratch_non_cacheable_phys);
c_header_mmap.addMemSection( c7x_4_ddr_scratch);
c_header_mmap.addMemSection( c7x_4_ddr_scratch_phys);
c_header_mmap.addMemSection( c7x_4_1_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_4_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_4_1_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_4_1_ddr_scratch);
c_header_mmap.addMemSection( c7x_4_2_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_4_2_ddr_local_heap);
c_header_mmap.addMemSection( c7x_4_2_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_4_2_ddr_scratch);
c_header_mmap.addMemSection( c7x_4_3_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_4_3_ddr_local_heap);
c_header_mmap.addMemSection( c7x_4_3_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_4_3_ddr_scratch);
c_header_mmap.addMemSection( tiovx_log_rt_mem );
c_header_mmap.addMemSection( app_log_mem        );
c_header_mmap.addMemSection( tiovx_obj_desc_mem );
c_header_mmap.addMemSection( app_fileio_mem        );
c_header_mmap.addMemSection( ipc_vring_mem      );
c_header_mmap.addMemSection( ddr_shared_mem     );
c_header_mmap.addMemSection( ddr_shared_mem_phys     );
c_header_mmap.addMemSection( uboot_reloc_mem     );
c_header_mmap.addMemSection( mcu2_main_ocram  );
c_header_mmap.addMemSection( mcu3_main_ocram  );
c_header_mmap.addMemSection( mcu4_main_ocram  );
c_header_mmap.addMemSection( mcu2_main_ocram_phys  );
c_header_mmap.addMemSection( mcu3_main_ocram_phys  );
c_header_mmap.addMemSection( mcu4_main_ocram_phys  );
# c_header_mmap.addMemSection( intercore_eth_desc_mem  );
# c_header_mmap.addMemSection( intercore_eth_data_mem  );
c_header_mmap.checkOverlap();

dts_mmap = MemoryMap("Memory Map for Linux kernel dts/dtsi file");
dts_mmap.addMemSection( dmcu0_ddr_total   );
dts_mmap.addMemSection( mcu0_ddr_ipc     );
dts_mmap.addMemSection( mcu0_ddr_total   );
dts_mmap.addMemSection( mcu1_ddr_ipc     );
dts_mmap.addMemSection( mcu1_ddr_total   );
dts_mmap.addMemSection( mcu2_ddr_ipc     );
dts_mmap.addMemSection( mcu2_ddr_total   );
dts_mmap.addMemSection( mcu3_ddr_ipc     );
dts_mmap.addMemSection( mcu3_ddr_total   );
dts_mmap.addMemSection( mcu4_ddr_ipc     );
dts_mmap.addMemSection( mcu4_ddr_total   );
dts_mmap.addMemSection( rmcu0_0_ddr_ipc     );
dts_mmap.addMemSection( rmcu0_0_ddr_total   );
dts_mmap.addMemSection( rmcu0_1_ddr_ipc     );
dts_mmap.addMemSection( rmcu0_1_ddr_total   );
dts_mmap.addMemSection( rmcu1_0_ddr_ipc     );
dts_mmap.addMemSection( rmcu1_0_ddr_total   );
dts_mmap.addMemSection( rmcu1_1_ddr_ipc     );
dts_mmap.addMemSection( rmcu1_1_ddr_total   );
dts_mmap.addMemSection( rmcu2_0_ddr_ipc     );
dts_mmap.addMemSection( rmcu2_0_ddr_total   );
dts_mmap.addMemSection( rmcu2_1_ddr_ipc     );
dts_mmap.addMemSection( rmcu2_1_ddr_total   );
dts_mmap.addMemSection( c7x_1_ddr_ipc      );
dts_mmap.addMemSection( c7x_1_ddr_total    );
dts_mmap.addMemSection( c7x_2_ddr_ipc      );
dts_mmap.addMemSection( c7x_2_ddr_total    );
dts_mmap.addMemSection( c7x_3_ddr_ipc      );
dts_mmap.addMemSection( c7x_3_ddr_total    );
dts_mmap.addMemSection( c7x_4_ddr_ipc      );
dts_mmap.addMemSection( c7x_4_ddr_total    );
dts_mmap.addMemSection( vision_apps_ddr_total );
dts_mmap.addMemSection( ipc_vring_mem      );
dts_mmap.addMemSection( vision_apps_core_heaps_lo );
dts_mmap.addMemSection( c7x_ddr_heaps_hi );
# Should be ddr_shared_mem_phys
dts_mmap.addMemSection( ddr_shared_mem );
# dts_mmap.addMemSection( intercore_eth_desc_mem );
# dts_mmap.addMemSection( intercore_eth_data_mem );
dts_mmap.checkOverlap();

#
# Generate linker command files containing "MEMORY" definitions
#
LinkerCmdFile(c7x_1_mmap , "../../rtos/tda54/cores/c7x_1/src/linker_mem_map.cmd" ).export();
LinkerCmdFile(c7x_2_mmap , "../../rtos/tda54/cores/c7x_2/src/linker_mem_map.cmd" ).export();
LinkerCmdFile(c7x_3_mmap , "../../rtos/tda54/cores/c7x_3/src/linker_mem_map.cmd" ).export();
LinkerCmdFile(c7x_4_mmap , "../../rtos/tda54/cores/c7x_4/src/linker_mem_map.cmd" ).export();
LinkerCmdFile(mcu0_mmap, "../../rtos/tda54/cores/mcu0/src/linker_mem_map.cmd").export();
LinkerCmdFile(mcu1_mmap, "../../rtos/tda54/cores/mcu1/src/linker_mem_map.cmd").export();
LinkerCmdFile(mcu2_mmap, "../../rtos/tda54/cores/mcu2/src/linker_mem_map.cmd").export();
LinkerCmdFile(mcu3_mmap, "../../rtos/tda54/cores/mcu3/src/linker_mem_map.cmd").export();
LinkerCmdFile(mcu4_mmap, "../../rtos/tda54/cores/mcu4/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu0_0_mmap, "../../rtos/tda54/cores/rmcu0_0/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu0_1_mmap, "../../rtos/tda54/cores/rmcu0_1/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu1_0_mmap, "../../rtos/tda54/cores/rmcu1_0/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu1_1_mmap, "../../rtos/tda54/cores/rmcu1_1/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu2_0_mmap, "../../rtos/tda54/cores/rmcu2_0/src/linker_mem_map.cmd").export();
LinkerCmdFile(rmcu2_1_mmap, "../../rtos/tda54/cores/rmcu2_1/src/linker_mem_map.cmd").export();

HtmlMmapTable(html_mmap, "./system_memory_map.html").export();

CHeaderFile(c_header_mmap, 0x2080000000, 0x100000000, "./app_mem_map.h").export();

DtsFile(dts_mmap, "./k3-tda54-rtos-memory-map.dtsi").export();
