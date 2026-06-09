#!/usr/bin/env python3
#
# Copyright (c) 2023-2026 Texas Instruments Incorporated
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
# Here CPU name is mpu1, c7x_1, c7x_2, mcu1_0, mcu2_0, mcu3_0
#
#
from ti_psdk_rtos_tools import *

KB = 1024;
MB = KB*KB;
GB = KB*MB;

#
# Notes,
# - recommend to keep all memory segment sizes in units of KB at least
#

#
# On J721E/J7ES/TDA4VM, there are 2 DDR chipsets
#
# lower DDR address starts at 0x0000_8000_0000
# higher DDR address starts at 0x0008_8000_0000
#
# As the address is non-contiguous it requires MMU to remap the address
# Currently the upper 2GB is accessed by either ARM (user space)
# C7x DSP heap/scratch space, as 32-bit cores like R5F cannot access it
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

ddr_mem_addr_hi_phys = 0x880000000;
ddr_mem_addr_hi = 0x100000000;
ddr_mem_size_hi = 512*MB;

#
# Other constant sizes
#
linux_ddr_ipc_size = 1*MB;
linux_ddr_resource_table_size = 1*KB;
linux_ddr_ipc_trace_size = 1*MB - 1*KB;

#
# C7x L1, L2 memory allocation
#
# L2 main - 2MB SRAM
# L2 aux  - 256KB SRAM
c7x_1_l2_main_addr  = 0x7e000000;
c7x_1_l2_main_size  = 2*MB;
c7x_1_l2_aux_addr  = 0x7f000000;
c7x_1_l2_aux_size  = 240*KB;
c7x_1_l2_aux_as_l1_addr  = c7x_1_l2_aux_addr + c7x_1_l2_aux_size;
c7x_1_l2_aux_as_l1_size  = 16*KB;

#
# C7x L1, L2 memory allocation
#
# L2 main - 2MB SRAM
# L2 aux  - 256KB SRAM
c7x_2_l2_main_addr  = 0x7e200000;
c7x_2_l2_main_size  = 2*MB;
c7x_2_l2_aux_addr  = 0x7f800000;
c7x_2_l2_aux_size  = 240*KB;
c7x_2_l2_aux_as_l1_addr  = c7x_2_l2_aux_addr + c7x_2_l2_aux_size;
c7x_2_l2_aux_as_l1_size  = 16*KB;

# Main OCRAM memory allocation
# NOTE: The main OCRAM memory is currently not required by any application on J722S.
# If future applications require OCRAM memory allocation, define it here.

#
# DDR memory allocation for various CPUs
#
mcu1_0_ddr_ipc_addr = ddr_mem_addr;
mcu1_0_ddr_resource_table_addr = mcu1_0_ddr_ipc_addr + linux_ddr_ipc_size;
mcu1_0_ddr_ipc_tracebuf_addr = mcu1_0_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu1_0_ddr_addr = mcu1_0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu1_0_ddr_size = 16*MB - (mcu1_0_ddr_addr-mcu1_0_ddr_ipc_addr);

mcu2_0_ddr_ipc_addr = mcu1_0_ddr_addr + mcu1_0_ddr_size + 16*MB;
mcu2_0_ddr_resource_table_addr = mcu2_0_ddr_ipc_addr + linux_ddr_ipc_size;
mcu2_0_ddr_ipc_tracebuf_addr = mcu2_0_ddr_resource_table_addr + linux_ddr_resource_table_size;
mcu2_0_ddr_addr = mcu2_0_ddr_ipc_tracebuf_addr + linux_ddr_ipc_trace_size;
mcu2_0_ddr_size = 32*MB - (mcu2_0_ddr_addr-mcu2_0_ddr_ipc_addr);

#
# DDR memory allocation for various shared memories
#

# Hardcoding this value, as this cannot be different from IPC echo test value
ipc_vring_mem_addr      = 0xA5000000;
ipc_vring_mem_size      = 16*MB;

tiovx_log_rt_mem_addr   = ipc_vring_mem_addr + ipc_vring_mem_size;
tiovx_log_rt_mem_size   = 16*MB;
tiovx_obj_desc_mem_addr = tiovx_log_rt_mem_addr + tiovx_log_rt_mem_size;
tiovx_obj_desc_mem_size = 8*MB;

app_fileio_mem_addr     = tiovx_obj_desc_mem_addr + tiovx_obj_desc_mem_size;
app_fileio_mem_size     = 4*MB;
app_log_mem_addr        = app_fileio_mem_addr + app_fileio_mem_size;
app_log_mem_size        = 256*KB;

c7x_1_ddr_ipc_addr = 0xAD000000;
c7x_1_ddr_resource_table_addr = c7x_1_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_1_ddr_ipc_trace_addr = c7x_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_1_ddr_boot_addr = c7x_1_ddr_resource_table_addr + 1*MB;
c7x_1_ddr_boot_size = 1*KB;
c7x_1_ddr_vecs_addr = c7x_1_ddr_resource_table_addr + 3*MB;
c7x_1_ddr_vecs_size = 16*KB;
c7x_1_ddr_secure_vecs_addr = c7x_1_ddr_resource_table_addr + 5*MB;
c7x_1_ddr_secure_vecs_size = 16*KB;
c7x_1_ddr_addr = c7x_1_ddr_secure_vecs_addr + c7x_1_ddr_secure_vecs_size;
c7x_1_ddr_size = 64*MB - (c7x_1_ddr_addr-c7x_1_ddr_ipc_addr);

c7x_2_ddr_ipc_addr = c7x_1_ddr_addr  + c7x_1_ddr_size;
c7x_2_ddr_resource_table_addr = c7x_2_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_2_ddr_ipc_trace_addr = c7x_2_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_2_ddr_boot_addr = c7x_2_ddr_resource_table_addr + 1*MB;
c7x_2_ddr_boot_size = 1*KB;
c7x_2_ddr_vecs_addr = c7x_2_ddr_resource_table_addr + 3*MB;
c7x_2_ddr_vecs_size = 16*KB;
c7x_2_ddr_secure_vecs_addr = c7x_2_ddr_resource_table_addr + 5*MB;
c7x_2_ddr_secure_vecs_size = 16*KB;
c7x_2_ddr_addr = c7x_2_ddr_secure_vecs_addr + c7x_2_ddr_secure_vecs_size;
c7x_2_ddr_size = 64*MB - (c7x_2_ddr_addr-c7x_2_ddr_ipc_addr);

mcu1_0_ddr_local_heap_addr  = c7x_2_ddr_addr + c7x_2_ddr_size;
mcu1_0_ddr_local_heap_size  = 8*MB;
mcu2_0_ddr_local_heap_addr  = mcu1_0_ddr_local_heap_addr + mcu1_0_ddr_local_heap_size;
mcu2_0_ddr_local_heap_size  = 32*MB;
mcu2_0_ddr_viss_heap_addr  = mcu2_0_ddr_local_heap_addr + mcu2_0_ddr_local_heap_size;
mcu2_0_ddr_viss_heap_size  = 4*MB;

# Hardcoding this value to align the start address with region size
ddr_intercore_eth_desc_addr = 0xB8000000;
ddr_intercore_eth_desc_size = 2*MB;

ddr_intercore_eth_data_addr = ddr_intercore_eth_desc_addr + ddr_intercore_eth_desc_size;
ddr_intercore_eth_data_size = 14*MB;

# Shared memory for DMA Buf FD carveout
ddr_shared_mem_addr     = 0xC0000000; # This will be the virtual address used for R5F's / C7X's
ddr_shared_mem_size     = 512*MB;

# C7x_1 Non-Cache sections
c7x_1_ddr_local_heap_non_cacheable_addr       = ddr_mem_addr_hi;
c7x_1_ddr_local_heap_non_cacheable_addr_phys  = ddr_mem_addr_hi_phys;
c7x_1_ddr_local_heap_non_cacheable_size       = 64*MB;
c7x_1_ddr_scratch_non_cacheable_addr          = c7x_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_1_ddr_scratch_non_cacheable_addr_phys     = c7x_1_ddr_local_heap_non_cacheable_addr_phys + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_1_ddr_scratch_non_cacheable_size          = 64*MB;

# C7x_1 Cache sections
c7x_1_ddr_local_heap_addr      = c7x_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_1_ddr_local_heap_addr_phys = c7x_1_ddr_scratch_non_cacheable_addr_phys + c7x_1_ddr_scratch_non_cacheable_size;
c7x_1_ddr_local_heap_size      = 64*MB;
c7x_1_ddr_scratch_addr         = c7x_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_1_ddr_scratch_addr_phys    = c7x_1_ddr_local_heap_addr_phys + c7x_1_ddr_local_heap_size;
c7x_1_ddr_scratch_size         = 64*MB;

# C7x_2 Non-Cache sections
c7x_2_ddr_local_heap_non_cacheable_addr       = c7x_1_ddr_scratch_addr + c7x_1_ddr_scratch_size;
c7x_2_ddr_local_heap_non_cacheable_addr_phys  = c7x_1_ddr_scratch_addr_phys + c7x_1_ddr_scratch_size;
c7x_2_ddr_local_heap_non_cacheable_size       = 64*MB;
c7x_2_ddr_scratch_non_cacheable_addr          = c7x_2_ddr_local_heap_non_cacheable_addr + c7x_2_ddr_local_heap_non_cacheable_size;
c7x_2_ddr_scratch_non_cacheable_addr_phys     = c7x_2_ddr_local_heap_non_cacheable_addr_phys + c7x_2_ddr_local_heap_non_cacheable_size;
c7x_2_ddr_scratch_non_cacheable_size          = 64*MB;

# C7x_2 Cache sections
c7x_2_ddr_local_heap_addr      = c7x_2_ddr_scratch_non_cacheable_addr + c7x_2_ddr_scratch_non_cacheable_size;
c7x_2_ddr_local_heap_addr_phys = c7x_2_ddr_scratch_non_cacheable_addr_phys + c7x_2_ddr_scratch_non_cacheable_size;
c7x_2_ddr_local_heap_size      = 64*MB;
c7x_2_ddr_scratch_addr         = c7x_2_ddr_local_heap_addr + c7x_2_ddr_local_heap_size;
c7x_2_ddr_scratch_addr_phys    = c7x_2_ddr_local_heap_addr_phys + c7x_2_ddr_local_heap_size;
c7x_2_ddr_scratch_size         = 64*MB;

total_c7x_1_local_ddr   = c7x_1_ddr_local_heap_non_cacheable_size + c7x_1_ddr_local_heap_size;
total_c7x_1_scratch_ddr = c7x_1_ddr_scratch_non_cacheable_size + c7x_1_ddr_scratch_size
total_c7x_1_ddr         = total_c7x_1_local_ddr + total_c7x_1_scratch_ddr

total_c7x_2_local_ddr   = c7x_2_ddr_local_heap_non_cacheable_size + c7x_2_ddr_local_heap_size
total_c7x_2_scratch_ddr = c7x_2_ddr_scratch_non_cacheable_size + c7x_2_ddr_scratch_size
total_c7x_2_ddr         = total_c7x_2_local_ddr + total_c7x_2_scratch_ddr

total_c7x_ddr = total_c7x_1_ddr + total_c7x_2_ddr

# Virtual address for C7x 1 corresponding to physical addresses for C7x2
c7x_1_2_ddr_local_heap_non_cacheable_addr = c7x_2_ddr_local_heap_non_cacheable_addr;
c7x_1_2_ddr_scratch_non_cacheable_addr = c7x_2_ddr_scratch_non_cacheable_addr;
c7x_1_2_ddr_local_heap_addr = c7x_2_ddr_local_heap_addr;
c7x_1_2_ddr_scratch_addr = c7x_2_ddr_scratch_addr;

# Virtual address for C7x 2 corresponding to physical addresses for C7x1
c7x_2_1_ddr_local_heap_non_cacheable_addr = c7x_1_ddr_local_heap_non_cacheable_addr;
c7x_2_1_ddr_scratch_non_cacheable_addr = c7x_1_ddr_scratch_non_cacheable_addr;
c7x_2_1_ddr_local_heap_addr = c7x_1_ddr_local_heap_addr;
c7x_2_1_ddr_scratch_addr = c7x_1_ddr_scratch_addr;

# Shared memory allocation for DMA Buffer File Descriptor (FD) carveout.
# This memory is located in high memory, used by OPENVX, and is both CACHEABLE and SHAREABLE.
ddr_shared_mem_addr_phys  = 0x900000000;

#
# Create memory section based on addr and size defined above, including
# any CPU specific internal memories
#

# r5f local memory sections
mcu_r5f_tcma_vecs  = MemSection("R5F_TCMA_VECS" , "X"   , 0x00000000, (KB >> 4));
mcu_r5f_tcma       = MemSection("R5F_TCMA" , "X"   , 0x00000040, (32*KB) - (KB >> 4));

r5f_tcmb0      = MemSection("R5F_TCMB0", "RWIX", 0x41010000, 32*KB);

mcu_r5f_tcmb0_vecs   = MemSection("R5F_TCMB0_VECS", "RWIX", 0x41010000, (KB >> 4));
mcu_r5f_tcmb0        = MemSection("R5F_TCMB0", "RWIX", 0x41010040, (32*KB) - (KB >> 4));

# C7x L1/L2 memory sections
c7x_1_l3   = MemSection("L2RAM_C7x_1_MAIN", "RWIX", c7x_1_l2_main_addr  , c7x_1_l2_main_size  , "L3 for C7x_1");
c7x_1_l2   = MemSection("L2RAM_C7x_1_AUX", "RWIX", c7x_1_l2_aux_addr  , c7x_1_l2_aux_size  , "L2 for C7x_1");
c7x_1_l1   = MemSection("L2RAM_C7x_1_AUX_AS_L1", "RWIX", c7x_1_l2_aux_as_l1_addr  , c7x_1_l2_aux_as_l1_size  , "L1 for C7x_1");

c7x_2_l3   = MemSection("L2RAM_C7x_2_MAIN", "RWIX", c7x_2_l2_main_addr  , c7x_2_l2_main_size  , "L3 for C7x_2");
c7x_2_l2   = MemSection("L2RAM_C7x_2_AUX", "RWIX", c7x_2_l2_aux_addr  , c7x_2_l2_aux_size  , "L2 for C7x_2");
c7x_2_l1   = MemSection("L2RAM_C7x_2_AUX_AS_L1", "RWIX", c7x_2_l2_aux_as_l1_addr  , c7x_2_l2_aux_as_l1_size  , "L1 for C7x_2");

# CPU code/data memory sections in DDR
mcu1_0_ddr_ipc             = MemSection("DDR_MCU1_0_IPC", "RWIX", mcu1_0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU1_0 for Linux IPC");
mcu1_0_ddr_ipc.setDtsName("vision_apps_mcu_r5fss0_core0_dma_memory_region", "vision-apps-r5f-dma-memory");
mcu1_0_ddr_resource_table  = MemSection("DDR_MCU1_0_RESOURCE_TABLE", "RWIX", mcu1_0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU1_0 for Linux resource table");
mcu1_0_ddr_ipc_trace       = MemSection("DDR_MCU1_0_IPC_TRACE", "RWIX", mcu1_0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU1_0 for Linux IPC trace");
mcu1_0_ddr                 = MemSection("DDR_MCU1_0", "RWIX", mcu1_0_ddr_addr, mcu1_0_ddr_size, "DDR for MCU1_0 for code/data");
mcu1_0_ddr_local_heap      = MemSection("DDR_MCU1_0_LOCAL_HEAP", "RWIX", mcu1_0_ddr_local_heap_addr, mcu1_0_ddr_local_heap_size, "DDR for MCU1_0 for local heap");
mcu1_0_ddr_total           = MemSection("DDR_MCU1_0_DTS", "", 0, 0, "DDR for MCU1_0 for all sections, used for reserving memory in DTS file");
mcu1_0_ddr_total.concat(mcu1_0_ddr_resource_table);
mcu1_0_ddr_total.concat(mcu1_0_ddr_ipc_trace);
mcu1_0_ddr_total.concat(mcu1_0_ddr);
mcu1_0_ddr_total.setDtsName("vision_apps_mcu_r5fss0_core0_memory_region", "vision-apps-r5f-memory");

mcu2_0_ddr_ipc             = MemSection("DDR_MCU2_0_IPC", "RWIX", mcu2_0_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU2_0 for Linux IPC");
mcu2_0_ddr_ipc.setDtsName("vision_apps_main_r5fss0_core0_dma_memory_region", "vision-apps-r5f-dma-memory");
mcu2_0_ddr_resource_table  = MemSection("DDR_MCU2_0_RESOURCE_TABLE", "RWIX", mcu2_0_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU2_0 for Linux resource table");
mcu2_0_ddr_ipc_trace       = MemSection("DDR_MCU2_0_IPC_TRACE", "RWIX", mcu2_0_ddr_ipc_tracebuf_addr, linux_ddr_ipc_trace_size, "DDR for MCU2_0 for Linux IPC trace");
mcu2_0_ddr                 = MemSection("DDR_MCU2_0", "RWIX", mcu2_0_ddr_addr, mcu2_0_ddr_size, "DDR for MCU2_0 for code/data");
mcu2_0_ddr_total           = MemSection("DDR_MCU2_0_DTS", "", 0, 0, "DDR for MCU2_0 for all sections, used for reserving memory in DTS file");
mcu2_0_ddr_local_heap      = MemSection("DDR_MCU2_0_LOCAL_HEAP", "RWIX", mcu2_0_ddr_local_heap_addr, mcu2_0_ddr_local_heap_size, "DDR for MCU2_0 for local heap");
mcu2_0_ddr_viss_config_heap = MemSection("DDR_MCU2_0_VISS_CONFIG_HEAP", "RWIX", mcu2_0_ddr_viss_heap_addr, mcu2_0_ddr_viss_heap_size, "VISS configuration memory. MUST be write through cache policy.");
mcu2_0_ddr_total.concat(mcu2_0_ddr_resource_table);
mcu2_0_ddr_total.concat(mcu2_0_ddr_ipc_trace);
mcu2_0_ddr_total.concat(mcu2_0_ddr);
mcu2_0_ddr_total.setDtsName("vision_apps_main_r5fss0_core0_memory_region", "vision-apps-r5f-memory");

c7x_1_ddr_ipc             = MemSection("DDR_C7x_1_IPC", "RWIX", c7x_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_1 for Linux IPC");
c7x_1_ddr_ipc.setDtsName("vision_apps_c71_0_dma_memory_region", "vision-apps-c71-dma-memory");
c7x_1_ddr_resource_table  = MemSection("DDR_C7x_1_RESOURCE_TABLE", "RWIX", c7x_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_1 for Linux resource table");
c7x_1_ddr_ipc_trace       = MemSection("DDR_C7x_1_IPC_TRACE", "RWIX", c7x_1_ddr_ipc_trace_addr, linux_ddr_ipc_trace_size, "DDR for C7x_1 for Linux IPC trace");
c7x_1_ddr_boot            = MemSection("DDR_C7x_1_BOOT", "RWIX", c7x_1_ddr_boot_addr, c7x_1_ddr_boot_size, "DDR for C7x_1 for boot section");
c7x_1_ddr_vecs            = MemSection("DDR_C7x_1_VECS", "RWIX", c7x_1_ddr_vecs_addr, c7x_1_ddr_vecs_size, "DDR for C7x_1 for vecs section");
c7x_1_ddr_secure_vecs     = MemSection("DDR_C7x_1_SECURE_VECS", "RWIX", c7x_1_ddr_secure_vecs_addr, c7x_1_ddr_secure_vecs_size, "DDR for C7x_1 for secure vecs section");
c7x_1_ddr                 = MemSection("DDR_C7x_1", "RWIX", c7x_1_ddr_addr, c7x_1_ddr_size, "DDR for C7x_1 for code/data");
c7x_1_ddr_local_heap_non_cacheable      = MemSection("DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "DDR for c7x_1 for non cacheable local heap");
c7x_1_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_1_ddr_local_heap_non_cacheable_addr_phys, c7x_1_ddr_local_heap_non_cacheable_size, "Physical address of non-cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_local_heap      = MemSection("DDR_C7X_1_LOCAL_HEAP", "RWIX", c7x_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "DDR for c7x_1 for local heap");
c7x_1_ddr_local_heap_phys = MemSection("DDR_C7X_1_LOCAL_HEAP_PHYS", "RWIX", c7x_1_ddr_local_heap_addr_phys, c7x_1_ddr_local_heap_size, "Physical address of cacheable DDR for c7x_1 for local heap");
c7x_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "DDR for c7x_1 for non cacheable scratch Memory");
c7x_1_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_1_ddr_scratch_non_cacheable_addr_phys, c7x_1_ddr_scratch_non_cacheable_size, "Physical address of non-cacheable DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_scratch         = MemSection("DDR_C7X_1_SCRATCH", "RWIX", c7x_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_scratch_phys    = MemSection("DDR_C7X_1_SCRATCH_PHYS", "RWIX", c7x_1_ddr_scratch_addr_phys, c7x_1_ddr_scratch_size, "Physical address of cacheable DDR for c7x_1 for Scratch Memory");

c7x_1_2_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for local heap wrt c7x_1");
c7x_1_2_ddr_local_heap      = MemSection("DDR_C7X_1_2_LOCAL_HEAP", "RWIX", c7x_1_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_2 for local heap wrt c7x_1");
c7x_1_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_2 for Scratch Memory wrt c7x_1");
c7x_1_2_ddr_scratch         = MemSection("DDR_C7X_1_2_SCRATCH", "RWIX", c7x_1_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_2 for Scratch Memory wrt c7x_1");

c7x_1_ddr_total           = MemSection("DDR_C7x_1_DTS", "", 0, 0, "DDR for C7x_1 for all sections, used for reserving memory in DTS file");
c7x_1_ddr_total.concat(c7x_1_ddr_resource_table);
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
c7x_2_ddr                 = MemSection("DDR_C7x_2", "RWIX", c7x_2_ddr_addr, c7x_2_ddr_size, "DDR for C7x_2 for code/data");
c7x_2_ddr_local_heap_non_cacheable      = MemSection("DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_ddr_local_heap_non_cacheable_addr, c7x_2_ddr_local_heap_non_cacheable_size, "DDR for c7x_2 for non cacheable local heap");
c7x_2_ddr_local_heap_non_cacheable_phys = MemSection("DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS", "RWIX", c7x_2_ddr_local_heap_non_cacheable_addr_phys, c7x_2_ddr_local_heap_non_cacheable_size, "Non-cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_local_heap      = MemSection("DDR_C7X_2_LOCAL_HEAP", "RWIX", c7x_2_ddr_local_heap_addr, c7x_2_ddr_local_heap_size, "DDR for c7x_2 for local heap");
c7x_2_ddr_local_heap_phys      = MemSection("DDR_C7X_2_LOCAL_HEAP_PHYS", "RWIX", c7x_2_ddr_local_heap_addr_phys, c7x_2_ddr_local_heap_size, "Physical address of Cacheable DDR for c7x_2 for local heap");
c7x_2_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_ddr_scratch_non_cacheable_addr, c7x_2_ddr_scratch_non_cacheable_size, "DDR for c7x_2 for non cacheable scratch Memory");
c7x_2_ddr_scratch_non_cacheable_phys    = MemSection("DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS", "RWIX", c7x_2_ddr_scratch_non_cacheable_addr_phys, c7x_2_ddr_scratch_non_cacheable_size, "Physical address of Non-cacheable DDR for c7x_2 for Scratch Memory");
c7x_2_ddr_scratch         = MemSection("DDR_C7X_2_SCRATCH", "RWIX", c7x_2_ddr_scratch_addr, c7x_2_ddr_scratch_size, "DDR for c7x_2 for Scratch Memory");
c7x_2_ddr_scratch_phys    = MemSection("DDR_C7X_2_SCRATCH_PHYS", "RWIX", c7x_2_ddr_scratch_addr_phys, c7x_2_ddr_scratch_size, "Physical address of cacheable DDR for c7x_2 for Scratch Memory");

c7x_2_1_ddr_local_heap_non_cacheable = MemSection("DDR_C7X_2_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_2_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for local heap wrt c7x_2");
c7x_2_1_ddr_local_heap      = MemSection("DDR_C7X_2_1_LOCAL_HEAP", "RWIX", c7x_2_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "Virtual address of cacheable DDR for c7x_1 for local heap wrt c7x_2");
c7x_2_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_2_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_2_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "Virtual address of non-cacheable DDR for c7x_1 for Scratch Memory wrt c7x_2");
c7x_2_1_ddr_scratch         = MemSection("DDR_C7X_2_1_SCRATCH", "RWIX", c7x_2_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "Virtual address of cacheable DDR for c7x_1 for Scratch Memory wrt c7x_2");

c7x_2_ddr_total           = MemSection("DDR_C7x_2_DTS", "", 0, 0, "DDR for C7x_2 for all sections, used for reserving memory in DTS file");
c7x_2_ddr_total.concat(c7x_2_ddr_resource_table);
c7x_2_ddr_total.concat(c7x_2_ddr_boot);
c7x_2_ddr_total.concat(c7x_2_ddr_vecs);
c7x_2_ddr_total.concat(c7x_2_ddr_secure_vecs);
c7x_2_ddr_total.concat(c7x_2_ddr);
c7x_2_ddr_total.setDtsName("vision_apps_c71_1_memory_region", "vision-apps-c71_1-memory");

# Shared memory memory sections in DDR
app_log_mem            = MemSection("APP_LOG_MEM"        , "", app_log_mem_addr       , app_log_mem_size       , "Memory for remote core logging");
tiovx_obj_desc_mem     = MemSection("TIOVX_OBJ_DESC_MEM" , "", tiovx_obj_desc_mem_addr, tiovx_obj_desc_mem_size, "Memory for TI OpenVX shared memory. MUST be non-cached or cache-coherent");
app_fileio_mem        = MemSection("APP_FILEIO_MEM"        , "", app_fileio_mem_addr       , app_fileio_mem_size       , "Memory for remote core file operations");
tiovx_log_rt_mem     = MemSection("TIOVX_LOG_RT_MEM" , "", tiovx_log_rt_mem_addr, tiovx_log_rt_mem_size, "Memory for TI OpenVX shared memory for Run-time logging. MUST be non-cached or cache-coherent");

ipc_vring_mem      = MemSection("IPC_VRING_MEM"     , "", ipc_vring_mem_addr     , ipc_vring_mem_size     , "Memory for IPC Vring's. MUST be non-cached or cache-coherent");
ipc_vring_mem.setDtsName("vision_apps_rtos_ipc_memory_region", "vision-apps-rtos-ipc-memory-region");

vision_apps_ddr_total  = MemSection("DDR_VISION_APPS_DTS", "", 0                      , 0                      , "DDR for Vision Apps for all sections, used for reserving memory in DTS file");
vision_apps_ddr_total.concat(app_log_mem);
vision_apps_ddr_total.concat(tiovx_obj_desc_mem);
vision_apps_ddr_total.concat(app_fileio_mem);
vision_apps_ddr_total.concat(tiovx_log_rt_mem);
vision_apps_ddr_total.setDtsName("vision_apps_memory_region", "vision-apps-dma-memory");

vision_apps_core_heaps_lo = MemSection("DDR_VISION_APPS_CORE_HEAPS_LO_DTS", "", 0, 0, "Vision Apps Core Heaps in 32bit address range of DDR");
vision_apps_core_heaps_lo.concat(mcu1_0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu2_0_ddr_local_heap);
vision_apps_core_heaps_lo.concat(mcu2_0_ddr_viss_config_heap);
vision_apps_core_heaps_lo.setDtsName("vision_apps_core_heaps_lo", "vision-apps-core-heap-memory-lo");

c7x_ddr_high_mem_phy  = MemSection("DDR_C7X_HIGH_MEM", "RWIX", ddr_mem_addr_hi_phys, total_c7x_ddr, "High mem sections for c7x-1 and c7x-2 for code/data, local heap and scratch");

c7x_ddr_heaps_hi = MemSection("DDR_C7X_HIGH_MEM_DTS", "", 0, 0, "C7X DDR memory sections in 40bit address range of DDR");
c7x_ddr_heaps_hi.concat(c7x_ddr_high_mem_phy);
c7x_ddr_heaps_hi.setDtsName("c7x_ddr_heaps_hi", "c7x-ddr-heaps-hi");
c7x_ddr_heaps_hi.splitOrigin(True)

# this region should NOT have the "no-map" flag since we want ION to map this memory and do cache ops on it as needed
ddr_shared_mem = MemSection("DDR_SHARED_MEM"    , "", ddr_shared_mem_addr    , ddr_shared_mem_size    , "Memory for shared memory buffers in DDR");
ddr_shared_mem.setDtsName("vision_apps_shared_region", "vision_apps_shared-memories");
ddr_shared_mem.setCompatibility("dma-heap-carveout");
ddr_shared_mem.setNoMap(False);
ddr_shared_mem.setOriginTag(False);
ddr_shared_mem.splitOrigin(True)

ddr_shared_mem_high  = MemSection("DDR_SHARED_MEM_PHYS"    , "", ddr_shared_mem_addr_phys  , ddr_shared_mem_size    , "memory for shared memory buffers in high DDR");
ddr_shared_mem_high.setDtsName("vision_apps_shared_region", "vision_apps_shared-memories");
ddr_shared_mem_high.setCompatibility("dma-heap-carveout");
ddr_shared_mem_high.setNoMap(False);
ddr_shared_mem_high.setOriginTag(False);
ddr_shared_mem_high.splitOrigin(True)

# This region is for ethernet firmware, multi-core, multi-cast feature
intercore_eth_desc_mem = MemSection("INTERCORE_ETH_DESC_MEM", "", ddr_intercore_eth_desc_addr, ddr_intercore_eth_desc_size, "Inter-core ethernet shared desc queues. MUST be non-cached or cache-coherent");
intercore_eth_desc_mem.setDtsName("vision_apps_main_r5fss0_core0_shared_memory_queue_region", "vision-apps-r5f-virtual-eth-queues");

intercore_eth_data_mem = MemSection("INTERCORE_ETH_DATA_MEM", "", ddr_intercore_eth_data_addr, ddr_intercore_eth_data_size, "Inter-core ethernet shared data buffers. MUST be non-cached or cache-coherent");
intercore_eth_data_mem.setDtsName("vision_apps_main_r5fss0_core0_shared_memory_bufpool_region", "vision-apps-r5f-virtual-eth-buffers");

#
# Create CPU specific memory maps using memory sections created above
#

mcu1_0_mmap = MemoryMap("mcu1_0");
mcu1_0_mmap.addMemSection( mcu_r5f_tcma_vecs );
mcu1_0_mmap.addMemSection( mcu_r5f_tcma      );
mcu1_0_mmap.addMemSection( mcu_r5f_tcmb0_vecs   );
mcu1_0_mmap.addMemSection( mcu_r5f_tcmb0        );
mcu1_0_mmap.addMemSection( mcu1_0_ddr_ipc       );
mcu1_0_mmap.addMemSection( mcu1_0_ddr_resource_table  );
mcu1_0_mmap.addMemSection( mcu1_0_ddr_ipc_trace  );
mcu1_0_mmap.addMemSection( mcu1_0_ddr           );
mcu1_0_mmap.addMemSection( app_log_mem          );
mcu1_0_mmap.addMemSection( tiovx_obj_desc_mem   );
mcu1_0_mmap.addMemSection( app_fileio_mem          );
mcu1_0_mmap.addMemSection( ipc_vring_mem        );
mcu1_0_mmap.addMemSection( mcu1_0_ddr_local_heap  );
mcu1_0_mmap.addMemSection( ddr_shared_mem       );
mcu1_0_mmap.checkOverlap();

mcu2_0_mmap = MemoryMap("mcu2_0");
mcu2_0_mmap.addMemSection( mcu_r5f_tcma_vecs );
mcu2_0_mmap.addMemSection( mcu_r5f_tcma      );
mcu2_0_mmap.addMemSection( r5f_tcmb0          );
mcu2_0_mmap.addMemSection( mcu2_0_ddr_ipc     );
mcu2_0_mmap.addMemSection( mcu2_0_ddr_resource_table  );
mcu2_0_mmap.addMemSection( mcu2_0_ddr_ipc_trace  );
mcu2_0_mmap.addMemSection( mcu2_0_ddr         );
mcu2_0_mmap.addMemSection( app_log_mem        );
mcu2_0_mmap.addMemSection( tiovx_obj_desc_mem );
mcu2_0_mmap.addMemSection( app_fileio_mem        );
mcu2_0_mmap.addMemSection( ipc_vring_mem      );
mcu2_0_mmap.addMemSection( mcu2_0_ddr_local_heap  );
mcu2_0_mmap.addMemSection( mcu2_0_ddr_viss_config_heap  );
mcu2_0_mmap.addMemSection( ddr_shared_mem     );
mcu2_0_mmap.addMemSection( intercore_eth_desc_mem );
mcu2_0_mmap.addMemSection( intercore_eth_data_mem );
mcu2_0_mmap.checkOverlap();

c7x_1_mmap = MemoryMap("c7x_1");
c7x_1_mmap.addMemSection( c7x_1_l3           );
c7x_1_mmap.addMemSection( c7x_1_l2           );
c7x_1_mmap.addMemSection( c7x_1_l1           );
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
c7x_1_mmap.addMemSection( ddr_shared_mem     );
c7x_1_mmap.checkOverlap();

c7x_2_mmap = MemoryMap("c7x_2");
c7x_2_mmap.addMemSection( c7x_2_l3           );
c7x_2_mmap.addMemSection( c7x_2_l2           );
c7x_2_mmap.addMemSection( c7x_2_l1           );
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
c7x_2_mmap.addMemSection( ddr_shared_mem     );
c7x_2_mmap.checkOverlap();

html_mmap = MemoryMap("System Memory Map for Linux+RTOS mode");
html_mmap.addMemSection( c7x_1_l3           );
html_mmap.addMemSection( c7x_1_l2           );
html_mmap.addMemSection( c7x_1_l1           );
html_mmap.addMemSection( c7x_2_l3           );
html_mmap.addMemSection( c7x_2_l2           );
html_mmap.addMemSection( c7x_2_l1           );
html_mmap.addMemSection( mcu1_0_ddr_ipc     );
html_mmap.addMemSection( mcu1_0_ddr_resource_table      );
html_mmap.addMemSection( mcu1_0_ddr_ipc_trace      );
html_mmap.addMemSection( mcu1_0_ddr         );
html_mmap.addMemSection( mcu1_0_ddr_local_heap );
html_mmap.addMemSection( mcu2_0_ddr_ipc     );
html_mmap.addMemSection( mcu2_0_ddr_resource_table      );
html_mmap.addMemSection( mcu2_0_ddr_ipc_trace      );
html_mmap.addMemSection( mcu2_0_ddr         );
html_mmap.addMemSection( mcu2_0_ddr_local_heap );
html_mmap.addMemSection( mcu2_0_ddr_viss_config_heap );
html_mmap.addMemSection( c7x_1_ddr_ipc     );
html_mmap.addMemSection( c7x_1_ddr_resource_table     );
html_mmap.addMemSection( c7x_1_ddr_ipc_trace      );
html_mmap.addMemSection( c7x_1_ddr_boot    );
html_mmap.addMemSection( c7x_1_ddr_vecs    );
html_mmap.addMemSection( c7x_1_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_1_ddr         );
html_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable );
html_mmap.addMemSection( c7x_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_ddr_scratch );
html_mmap.addMemSection( c7x_1_2_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_1_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_2_ddr_scratch );
html_mmap.addMemSection( c7x_2_ddr_ipc     );
html_mmap.addMemSection( c7x_2_ddr_resource_table     );
html_mmap.addMemSection( c7x_2_ddr_ipc_trace      );
html_mmap.addMemSection( c7x_2_ddr_boot    );
html_mmap.addMemSection( c7x_2_ddr_vecs    );
html_mmap.addMemSection( c7x_2_ddr_secure_vecs    );
html_mmap.addMemSection( c7x_2_ddr         );
html_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable );
html_mmap.addMemSection( c7x_2_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_ddr_scratch );
html_mmap.addMemSection( c7x_2_1_ddr_local_heap_non_cacheable         );
html_mmap.addMemSection( c7x_2_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_2_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_2_1_ddr_scratch );
html_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_1_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_1_ddr_scratch_phys );
html_mmap.addMemSection( c7x_2_ddr_local_heap_non_cacheable_phys         );
html_mmap.addMemSection( c7x_2_ddr_local_heap_phys         );
html_mmap.addMemSection( c7x_2_ddr_scratch_non_cacheable_phys );
html_mmap.addMemSection( c7x_2_ddr_scratch_phys );
html_mmap.addMemSection( app_log_mem        );
html_mmap.addMemSection( tiovx_obj_desc_mem );
html_mmap.addMemSection( app_fileio_mem        );
html_mmap.addMemSection( ipc_vring_mem      );
html_mmap.addMemSection( ddr_shared_mem     );
html_mmap.addMemSection( tiovx_log_rt_mem );
html_mmap.addMemSection( intercore_eth_desc_mem );
html_mmap.addMemSection( intercore_eth_data_mem );
#html_mmap.addMemSection( ddr_shared_mem_phys     );
html_mmap.checkOverlap();

c_header_mmap = MemoryMap("Memory Map for C header file");
c_header_mmap.addMemSection( c7x_1_l3           );
c_header_mmap.addMemSection( c7x_1_l2           );
c_header_mmap.addMemSection( c7x_1_l1           );
c_header_mmap.addMemSection( c7x_2_l3           );
c_header_mmap.addMemSection( c7x_2_l2           );
c_header_mmap.addMemSection( c7x_2_l1           );
c_header_mmap.addMemSection( mcu1_0_ddr_ipc     );
c_header_mmap.addMemSection( mcu2_0_ddr_ipc     );
c_header_mmap.addMemSection( c7x_1_ddr_ipc     );
c_header_mmap.addMemSection( c7x_2_ddr_ipc     );
c_header_mmap.addMemSection( mcu1_0_ddr_total     );
c_header_mmap.addMemSection( mcu2_0_ddr_total     );
c_header_mmap.addMemSection( c7x_1_ddr_total     );
c_header_mmap.addMemSection( c7x_2_ddr_total     );
c_header_mmap.addMemSection( mcu1_0_ddr_local_heap);
c_header_mmap.addMemSection( mcu2_0_ddr_local_heap);
c_header_mmap.addMemSection( mcu2_0_ddr_viss_config_heap);
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

c_header_mmap.addMemSection( tiovx_log_rt_mem );
c_header_mmap.addMemSection( app_log_mem        );
c_header_mmap.addMemSection( tiovx_obj_desc_mem );
c_header_mmap.addMemSection( app_fileio_mem        );
c_header_mmap.addMemSection( ipc_vring_mem      );
c_header_mmap.addMemSection( ddr_shared_mem     );
c_header_mmap.addMemSection( ddr_shared_mem_high     );
c_header_mmap.addMemSection( intercore_eth_desc_mem  );
c_header_mmap.addMemSection( intercore_eth_data_mem  );
c_header_mmap.checkOverlap();

dts_mmap = MemoryMap("Memory Map for Linux kernel dts/dtsi file");
dts_mmap.addMemSection( mcu1_0_ddr_ipc     );
dts_mmap.addMemSection( mcu1_0_ddr_total   );
dts_mmap.addMemSection( mcu2_0_ddr_ipc     );
dts_mmap.addMemSection( mcu2_0_ddr_total   );
dts_mmap.addMemSection( c7x_1_ddr_ipc      );
dts_mmap.addMemSection( c7x_1_ddr_total    );
dts_mmap.addMemSection( c7x_2_ddr_ipc      );
dts_mmap.addMemSection( c7x_2_ddr_total    );
dts_mmap.addMemSection( vision_apps_ddr_total );
dts_mmap.addMemSection( ipc_vring_mem      );
dts_mmap.addMemSection( vision_apps_core_heaps_lo );
dts_mmap.addMemSection( c7x_ddr_heaps_hi );
dts_mmap.addMemSection( ddr_shared_mem_high );
dts_mmap.addMemSection( intercore_eth_desc_mem );
dts_mmap.addMemSection( intercore_eth_data_mem );
dts_mmap.checkOverlap();

#
# Generate linker command files containing "MEMORY" definitions
#
LinkerCmdFile(c7x_1_mmap , "./c7x_1/linker_mem_map_freertos.cmd" ).export();
LinkerCmdFile(c7x_2_mmap , "./c7x_2/linker_mem_map_freertos.cmd" ).export();
LinkerCmdFile(mcu1_0_mmap, "./mcu1_0/linker_mem_map_freertos.cmd").export();
LinkerCmdFile(mcu2_0_mmap, "./mcu2_0/linker_mem_map_freertos.cmd").export();

HtmlMmapTable(html_mmap, "./system_memory_map.html").export();

CHeaderFile(c_header_mmap, 0x880000000, 0x100000000, "./app_mem_map.h").export();

DtsFile(dts_mmap, "./k3-j722s-rtos-memory-map.dtsi").export();
