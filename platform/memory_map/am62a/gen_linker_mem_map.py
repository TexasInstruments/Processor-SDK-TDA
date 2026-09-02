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
# Here CPU name is mpu1, c7x_1, mcu1_0
#
#
from ti_psdk_rtos_tools import *
import math, os, sys, re
from gen_dts_file import Org_dts_file
from gen_edgeai_dtso import Edgeai_dts_file
from gen_c7x_1_syscfg import C7x_1_Syscfg
from gen_mcu1_0_syscfg import Mcu1_0_Syscfg
from gen_qnx_bsp_carveout import QNX_BSP_UPDATE

def roundUp(x,y):
        return int(math.ceil(x / y)) * y

KB = 1024;
MB = KB*KB;
GB = KB*MB;

#
# Notes,
# - recommend to keep all memory segment sizes in units of KB at least
#

#
# On AM62A, there is a total of 2GB
# but here a memory map of just 1GB is considered
#
# Linux Kernel address starts at 0x8000_0000 of size 512MB
# RTOS carveout address starts at 0xA000_0000 of size 384MB
# Linux user space address starts at 0xB800_0000 of size 128MB
#

ddr_mem_addr_1  = 0x0099800000;
ddr_mem_size_1  = 80*MB

ddr_mem_addr_2 = 0xA0000000;
ddr_mem_size_2 = 515*MB

#
# Other constant sizes
#
linux_ddr_ipc_size = 1*MB;
linux_ddr_resource_table_size = 1*KB;
linux_ddr_ipc_tracebuf_size = 1*MB - linux_ddr_resource_table_size;
linux_ddr_lpm_data_size = 640*KB

#
# MSMC memory allocation for various CPUs
#
#dmsc_msmc_size   = 64*KB;
#mpu1_msmc_size   = 128*KB;

#
# C7x L1, L2 memory allocation
#
# L2 main - 1MB SRAM
# L2 aux  - 256KB SRAM
c7x_1_l2_main_addr  = 0x7e000000;
c7x_1_l2_main_size  = 1*MB;
c7x_1_l2_aux_addr  = 0x7f000000;
c7x_1_l2_aux_size  = 240*KB;
c7x_1_l2_aux_as_l1_addr  = c7x_1_l2_aux_addr + c7x_1_l2_aux_size;
c7x_1_l2_aux_as_l1_size  = 16*KB;

#
# DDR memory allocation for various CPUs
#
c7x_1_ddr_ipc_addr = ddr_mem_addr_1;
c7x_1_ddr_resource_table_addr = c7x_1_ddr_ipc_addr + linux_ddr_ipc_size;
c7x_1_ddr_ipc_tracebuf_addr = c7x_1_ddr_resource_table_addr + linux_ddr_resource_table_size;
c7x_1_ddr_boot_addr = c7x_1_ddr_ipc_tracebuf_addr + linux_ddr_ipc_tracebuf_size;
c7x_1_ddr_boot_addr = roundUp(c7x_1_ddr_boot_addr, 1*MB);
c7x_1_ddr_boot_size = 1*KB;
c7x_1_ddr_vecs_addr = c7x_1_ddr_ipc_tracebuf_addr + 2*MB;
c7x_1_ddr_vecs_addr = roundUp(c7x_1_ddr_vecs_addr, 2*MB);
c7x_1_ddr_vecs_size = 16*KB;
c7x_1_ddr_addr = c7x_1_ddr_vecs_addr + c7x_1_ddr_vecs_size;
c7x_1_ddr_addr = roundUp(c7x_1_ddr_addr, 64*KB);
c7x_1_ddr_size = 32*MB - (c7x_1_ddr_addr-c7x_1_ddr_ipc_addr);
# Reduce 16Mb from the code/data segment to align with linux Sdk
c7x_1_ddr_data_seg_size = (c7x_1_ddr_size ) - 16*MB;

mcu_r5f_ddr_ipc_addr = c7x_1_ddr_addr + c7x_1_ddr_size ;
mcu_r5f_ddr_resource_table_addr = mcu_r5f_ddr_ipc_addr + linux_ddr_ipc_size;
mcu_r5f_ddr_ipc_tracebuf_addr = mcu_r5f_ddr_resource_table_addr + linux_ddr_resource_table_size
mcu_r5f_ddr_addr = mcu_r5f_ddr_ipc_tracebuf_addr + linux_ddr_ipc_tracebuf_size;
mcu_r5f_ddr_size = 16*MB - (mcu_r5f_ddr_addr-mcu_r5f_ddr_ipc_addr);

dm_r5f_ddr_ipc_addr = mcu_r5f_ddr_addr + mcu_r5f_ddr_size;
dm_r5f_ddr_resource_table_addr = dm_r5f_ddr_ipc_addr + linux_ddr_ipc_size;
dm_r5f_ddr_ipc_tracebuf_addr = dm_r5f_ddr_resource_table_addr + linux_ddr_resource_table_size
linux_ddr_lpm_data_addr = dm_r5f_ddr_ipc_tracebuf_addr + linux_ddr_ipc_tracebuf_size
dm_r5f_ddr_addr = linux_ddr_lpm_data_addr + linux_ddr_lpm_data_size;
dm_r5f_ddr_size = 31*MB - (dm_r5f_ddr_addr-dm_r5f_ddr_ipc_addr);

tifs_lpm_ctx_addr = dm_r5f_ddr_addr + dm_r5f_ddr_size;
tifs_lpm_ctx_size = 512*KB;
atf_addr = tifs_lpm_ctx_addr + tifs_lpm_ctx_size;
atf_size = 512*KB;
optee_addr = atf_addr + atf_size;
optee_size = 24*MB;

#
# DDR memory allocation for various shared memories
#
carveout_size = 0
# Keeping 16MB additional for VRING start, so that IPC Shared memory starts
# exactly at 0xAA000000 offset. This gap of 16MB is not currently used and
# can be used for Linux..
ipc_vring_mem_addr      = ddr_mem_addr_2;
ipc_vring_mem_size      = 16*MB;
carveout_size += ipc_vring_mem_size

app_log_mem_addr        = ipc_vring_mem_addr + ipc_vring_mem_size;
app_log_mem_size        = 256*KB;
carveout_size += app_log_mem_size

tiovx_obj_desc_mem_addr = app_log_mem_addr + app_log_mem_size;
tiovx_obj_desc_mem_size = 16*MB - app_log_mem_size;
carveout_size += tiovx_obj_desc_mem_size

app_fileio_mem_addr        = tiovx_obj_desc_mem_addr + tiovx_obj_desc_mem_size;
app_fileio_mem_size        = 4*MB;
carveout_size += app_fileio_mem_size

tiovx_log_rt_mem_addr   = app_fileio_mem_addr + app_fileio_mem_size;
tiovx_log_rt_mem_size   = 16*MB - app_fileio_mem_size;
carveout_size += tiovx_log_rt_mem_size

# Shared memory for Buffers/ION allocator
ddr_shared_mem_addr     = tiovx_log_rt_mem_addr + tiovx_log_rt_mem_size;
ddr_shared_mem_size     = 172*MB;
carveout_size += ddr_shared_mem_size

ddr_viss_config_heap_addr = ddr_shared_mem_addr + ddr_shared_mem_size;
ddr_viss_config_heap_size = 4*MB;
carveout_size += ddr_viss_config_heap_size

mcu_r5f_ddr_local_heap_addr  = ddr_viss_config_heap_addr + ddr_viss_config_heap_size;
mcu_r5f_ddr_local_heap_size  = 16*MB;
carveout_size += mcu_r5f_ddr_local_heap_size

dm_r5f_ddr_local_heap_addr  = mcu_r5f_ddr_local_heap_addr + mcu_r5f_ddr_local_heap_size;
dm_r5f_ddr_local_heap_size  = 16*MB;
carveout_size += dm_r5f_ddr_local_heap_size

c7x_1_ddr_local_heap_non_cacheable_addr  = dm_r5f_ddr_local_heap_addr + dm_r5f_ddr_local_heap_size;
c7x_1_ddr_local_heap_non_cacheable_size  = 16*MB;
carveout_size += c7x_1_ddr_local_heap_non_cacheable_size

c7x_1_ddr_scratch_non_cacheable_addr     = c7x_1_ddr_local_heap_non_cacheable_addr + c7x_1_ddr_local_heap_non_cacheable_size;
c7x_1_ddr_scratch_non_cacheable_size     = 16*MB;
carveout_size += c7x_1_ddr_scratch_non_cacheable_size

c7x_1_ddr_local_heap_addr  = c7x_1_ddr_scratch_non_cacheable_addr + c7x_1_ddr_scratch_non_cacheable_size;
c7x_1_ddr_local_heap_size  = 112*MB;
carveout_size += c7x_1_ddr_local_heap_size

c7x_1_ddr_scratch_addr     = c7x_1_ddr_local_heap_addr + c7x_1_ddr_local_heap_size;
c7x_1_ddr_scratch_size     = 112*MB;
carveout_size += c7x_1_ddr_scratch_size

assert carveout_size <= ddr_mem_size_2

#
# Create memory section based on addr and size defined above, including
# any CPU specific internal memories
#

# r5f local memory sections
mcu_r5f_tcma_vecs            = MemSection("R5F_TCMA_VECS" , "X"   , 0x00000000, (KB >> 4));
mcu_r5f_tcma                 = MemSection("R5F_TCMA" , "X"   , 0x00000040, (32*KB) - (KB >> 4));
mcu_r5f_tcma_trace_buff      = MemSection("WKUP_SRAM_TRACE_BUFF" , "RWIX"   , 0x41880000, KB << 1);

mcu_r5f_rmpm_hal_trace_buff  = MemSection("DDR_DM_RMPM_TRACE", "RWIX", 0x9CAA0000, (KB << 4) + (KB << 2));

mcu_r5f_tcmb0_vecs            = MemSection("R5F_TCMB0_VECS", "RWIX", 0x41010000, (KB >> 4));
mcu_r5f_tcmb0                 = MemSection("R5F_TCMB0", "RWIX", 0x41010040, (32*KB) - (KB >> 4));

# C7x L1/L2/L3 memory sections
c7x_1_l3   = MemSection("L2RAM_C7x_1_MAIN", "RWIX", c7x_1_l2_main_addr  , c7x_1_l2_main_size  , "L3 for C7x_1");
c7x_1_l2   = MemSection("L2RAM_C7x_1_AUX", "RWIX", c7x_1_l2_aux_addr  , c7x_1_l2_aux_size  , "L2 for C7x_1");
c7x_1_l1   = MemSection("L2RAM_C7x_1_AUX_AS_L1", "RWIX", c7x_1_l2_aux_as_l1_addr  , c7x_1_l2_aux_as_l1_size  , "L1 for C7x_1");

# CPU code/data memory sections in DDR
mcu_r5f_ddr_ipc             = MemSection("DDR_MCU_R5F_IPC", "RWIX", mcu_r5f_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for MCU R5F for Linux IPC");
mcu_r5f_ddr_ipc.setDtsName("mcu_r5fss0_core0_dma_memory_region", "r5f-dma-memory");
mcu_r5f_ddr_resource_table  = MemSection("DDR_MCU_R5F_RESOURCE_TABLE", "RWIX", mcu_r5f_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for MCU R5F for Linux resource table");
mcu_r5f_ddr_ipc_tracebuf    = MemSection("DDR_MCU_R5F_IPC_TRACEBUF", "RWIX", mcu_r5f_ddr_ipc_tracebuf_addr, linux_ddr_ipc_tracebuf_size, "DDR for MCU R5F for Linux IPC tracebuffer");
mcu_r5f_ddr                 = MemSection("DDR_MCU_R5F", "RWIX", mcu_r5f_ddr_addr, mcu_r5f_ddr_size, "DDR for MCU R5F for code/data");
mcu_r5f_ddr_local_heap      = MemSection("DDR_MCU_R5F_LOCAL_HEAP", "RWIX", mcu_r5f_ddr_local_heap_addr, mcu_r5f_ddr_local_heap_size, "DDR for MCU R5F for local heap");
mcu_r5f_ddr_total           = MemSection("DDR_MCU_R5F_DTS", "", 0, 0, "DDR for MCU R5F for all sections, used for reserving memory in DTS file");
mcu_r5f_ddr_total.concat(mcu_r5f_ddr_resource_table);
mcu_r5f_ddr_total.concat(mcu_r5f_ddr_ipc_tracebuf);
mcu_r5f_ddr_total.concat(mcu_r5f_ddr);
mcu_r5f_ddr_total.setDtsName("mcu_r5fss0_core0_memory_region", "r5f-dma-memory");

ddr_viss_config_heap        = MemSection("DDR_DM_R5F_VISS_CONFIG_HEAP", "RW", ddr_viss_config_heap_addr, ddr_viss_config_heap_size, "DDR for storing DMA buffers for VISS configuration");
dm_r5f_ddr_ipc             = MemSection("DDR_DM_R5F_IPC", "RWIX", dm_r5f_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for DM R5F for Linux IPC");
dm_r5f_ddr_ipc.setDtsName("wkup_r5fss0_core0_dma_memory_region", "r5f-dma-memory");
dm_r5f_ddr_resource_table  = MemSection("DDR_DM_R5F_RESOURCE_TABLE", "RWIX", dm_r5f_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for DM R5F for Linux resource table");
dm_r5f_ddr_ipc_tracebuf    = MemSection("DDR_DM_R5F_IPC_TRACEBUF", "RWIX", dm_r5f_ddr_ipc_tracebuf_addr, linux_ddr_ipc_tracebuf_size, "DDR for DM R5F for Linux IPC tracebuffer");
linux_ddr_lpm_data          = MemSection("DDR_LPM_DATA", "RWIX", linux_ddr_lpm_data_addr, linux_ddr_lpm_data_size, "DDR for LPM Data");
dm_r5f_ddr                 = MemSection("DDR_DM_R5F", "RWIX", dm_r5f_ddr_addr, dm_r5f_ddr_size, "DDR for DM R5F for code/data");
dm_r5f_ddr_local_heap      = MemSection("DDR_DM_R5F_LOCAL_HEAP", "RWIX", dm_r5f_ddr_local_heap_addr, dm_r5f_ddr_local_heap_size, "DDR for DM R5F for local heap");
dm_r5f_ddr_total           = MemSection("DDR_DM_R5F_DTS", "", 0, 0, "DDR for DM R5F for all sections, used for reserving memory in DTS file");
dm_r5f_ddr_total.concat(dm_r5f_ddr_resource_table);
dm_r5f_ddr_total.concat(dm_r5f_ddr_ipc_tracebuf);
dm_r5f_ddr_total.concat(dm_r5f_ddr);
dm_r5f_ddr_total.setDtsName("wkup_r5fss0_core0_memory_region", "r5f-dma-memory");

c7x_1_ddr_ipc             = MemSection("DDR_C7x_1_IPC", "RWIX", c7x_1_ddr_ipc_addr, linux_ddr_ipc_size, "DDR for C7x_1 for Linux IPC");
c7x_1_ddr_ipc.setDtsName("c7x_0_dma_memory_region", "c7x-dma-memory");
c7x_1_ddr_resource_table  = MemSection("DDR_C7x_1_RESOURCE_TABLE", "RWIX", c7x_1_ddr_resource_table_addr, linux_ddr_resource_table_size, "DDR for C7x_1 for Linux resource table");
c7x_1_ddr_ipc_tracebuf    = MemSection("DDR_C7X_1_IPC_TRACEBUF", "RWIX", c7x_1_ddr_ipc_tracebuf_addr, linux_ddr_ipc_tracebuf_size, "DDR for C7X_1 for Linux IPC tracebuffer");
c7x_1_ddr_boot            = MemSection("DDR_C7x_1_BOOT", "RWIX", c7x_1_ddr_boot_addr, c7x_1_ddr_boot_size, "DDR for C7x_1 for boot section");
c7x_1_ddr_vecs            = MemSection("DDR_C7x_1_VECS", "RWIX", c7x_1_ddr_vecs_addr, c7x_1_ddr_vecs_size, "DDR for C7x_1 for vecs section");
c7x_1_ddr                 = MemSection("DDR_C7x_1", "RWIX", c7x_1_ddr_addr, c7x_1_ddr_data_seg_size, "DDR for C7x_1 for code/data");
c7x_1_ddr_local_heap_non_cacheable      = MemSection("DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE", "RWIX", c7x_1_ddr_local_heap_non_cacheable_addr, c7x_1_ddr_local_heap_non_cacheable_size, "DDR for c7x_1 for non cacheable local heap");
c7x_1_ddr_scratch_non_cacheable         = MemSection("DDR_C7X_1_SCRATCH_NON_CACHEABLE", "RWIX", c7x_1_ddr_scratch_non_cacheable_addr, c7x_1_ddr_scratch_non_cacheable_size, "DDR for c7x_1 for non cacheable scratch Memory");
c7x_1_ddr_local_heap      = MemSection("DDR_C7X_1_LOCAL_HEAP", "RWIX", c7x_1_ddr_local_heap_addr, c7x_1_ddr_local_heap_size, "DDR for c7x_1 for local heap");
c7x_1_ddr_scratch         = MemSection("DDR_C7X_1_SCRATCH", "RWIX", c7x_1_ddr_scratch_addr, c7x_1_ddr_scratch_size, "DDR for c7x_1 for Scratch Memory");
c7x_1_ddr_total           = MemSection("DDR_C7x_1_DTS", "", 0, 0, "DDR for C7x_1 for all sections, used for reserving memory in DTS file");
c7x_1_ddr_total.concat(c7x_1_ddr_resource_table);
c7x_1_ddr_total.concat(c7x_1_ddr_ipc_tracebuf);
c7x_1_ddr_total.concat(c7x_1_ddr_boot);
c7x_1_ddr_total.concat(c7x_1_ddr_vecs);
c7x_1_ddr_total.concat(c7x_1_ddr);
c7x_1_ddr_total.setDtsName("c7x_0_memory_region", "c7x-memory");

tifs_lpm_mem       = MemSection("TIFS_LPM_CTX"        , "", tifs_lpm_ctx_addr       , tifs_lpm_ctx_size       , "TIFS LPM context save memory");
atf_mem            = MemSection("ATF_MEM"        , "", atf_addr       , atf_size       , "ARM Trusted Firmware");
optee_mem          = MemSection("OPTEE_MEM"        , "", optee_addr       , optee_size       , "Open Portable Trusted Execution Environment");

# Shared memory memory sections in DDR
app_log_mem            = MemSection("APP_LOG_MEM"        , "", app_log_mem_addr       , app_log_mem_size       , "Memory for remote core logging");
tiovx_obj_desc_mem     = MemSection("TIOVX_OBJ_DESC_MEM" , "", tiovx_obj_desc_mem_addr, tiovx_obj_desc_mem_size, "Memory for TI OpenVX shared memory. MUST be non-cached or cache-coherent");
app_fileio_mem        = MemSection("APP_FILEIO_MEM"        , "", app_fileio_mem_addr       , app_fileio_mem_size       , "Memory for remote core file operations");
tiovx_log_rt_mem     = MemSection("TIOVX_LOG_RT_MEM" , "", tiovx_log_rt_mem_addr, tiovx_log_rt_mem_size, "Memory for TI OpenVX shared memory for Run-time logging. MUST be non-cached or cache-coherent");

ipc_vring_mem      = MemSection("IPC_VRING_MEM"     , "", ipc_vring_mem_addr     , ipc_vring_mem_size     , "Memory for IPC Vring's. MUST be non-cached or cache-coherent");
ipc_vring_mem.setDtsName("rtos_ipc_memory_region", "ipc-memories");
ipc_vring_mem.setAlignment(True)
ipc_vring_mem.setPrintCompatibility(False)
ipc_vring_mem.setOriginTag(False);

edgeai_ddr_total  = MemSection("DDR_EDGEAI_DTS", "", 0                      , 0                      , "DDR for EdgeAI for all sections, used for reserving memory in DTS file");
edgeai_ddr_total.concat(app_log_mem);
edgeai_ddr_total.concat(tiovx_obj_desc_mem);
edgeai_ddr_total.concat(app_fileio_mem);
edgeai_ddr_total.concat(tiovx_log_rt_mem);
edgeai_ddr_total.setDtsName("edgeai_memory_region", "edgeai-dma-memory");

# this region should NOT have the "no-map" flag since we want ION to map this memory and do cache ops on it as needed
ddr_shared_mem     = MemSection("DDR_SHARED_MEM"    , "", ddr_shared_mem_addr    , ddr_shared_mem_size    , "Memory for shared memory buffers in DDR");
ddr_shared_mem.setDtsName("edgeai_shared_region", "edgeai_shared-memories");
ddr_shared_mem.setCompatibility("dma-heap-carveout");
ddr_shared_mem.setNoMap(False);
ddr_shared_mem.setOriginTag(False);

edgeai_core_heaps = MemSection("DDR_EDGEAI_CORE_HEAPS_DTS", "", 0, 0, "EdgeAI Core Heaps in 32bit address range of DDR");
edgeai_core_heaps.concat(ddr_viss_config_heap);
edgeai_core_heaps.concat(mcu_r5f_ddr_local_heap);
edgeai_core_heaps.concat(dm_r5f_ddr_local_heap);
edgeai_core_heaps.concat(c7x_1_ddr_local_heap);
edgeai_core_heaps.concat(c7x_1_ddr_scratch);
edgeai_core_heaps.setDtsName("edgeai_core_heaps", "edgeai-core-heap-memory");

#
# Create CPU specific memory maps using memory sections created above
#

mcu_r5f_mmap = MemoryMap("mcu_r5f");
mcu_r5f_mmap.addMemSection( mcu_r5f_tcma_vecs );
mcu_r5f_mmap.addMemSection( mcu_r5f_tcma      );
mcu_r5f_mmap.addMemSection( mcu_r5f_tcmb0_vecs   );
mcu_r5f_mmap.addMemSection( mcu_r5f_tcmb0        );
mcu_r5f_mmap.addMemSection( mcu_r5f_ddr_ipc       );
mcu_r5f_mmap.addMemSection( mcu_r5f_ddr_resource_table  );
mcu_r5f_mmap.addMemSection( mcu_r5f_ddr_ipc_tracebuf  );
mcu_r5f_mmap.addMemSection( mcu_r5f_ddr           );
mcu_r5f_mmap.addMemSection( app_log_mem          );
mcu_r5f_mmap.addMemSection( tiovx_obj_desc_mem   );
mcu_r5f_mmap.addMemSection( app_fileio_mem          );
mcu_r5f_mmap.addMemSection( ipc_vring_mem        );
mcu_r5f_mmap.addMemSection( mcu_r5f_ddr_local_heap  );
mcu_r5f_mmap.addMemSection( ddr_viss_config_heap  );
mcu_r5f_mmap.addMemSection( ddr_shared_mem       );
mcu_r5f_mmap.checkOverlap();

dm_r5f_mmap = MemoryMap("dm_r5f");
dm_r5f_mmap.addMemSection( mcu_r5f_tcma_vecs );
dm_r5f_mmap.addMemSection( mcu_r5f_tcma      );
dm_r5f_mmap.addMemSection( mcu_r5f_tcma_trace_buff      );
dm_r5f_mmap.addMemSection( mcu_r5f_rmpm_hal_trace_buff );
dm_r5f_mmap.addMemSection( mcu_r5f_tcmb0_vecs   );
dm_r5f_mmap.addMemSection( mcu_r5f_tcmb0        );
dm_r5f_mmap.addMemSection( dm_r5f_ddr_ipc       );
dm_r5f_mmap.addMemSection( dm_r5f_ddr_resource_table  );
dm_r5f_mmap.addMemSection( dm_r5f_ddr_ipc_tracebuf  );
dm_r5f_mmap.addMemSection( linux_ddr_lpm_data  );
dm_r5f_mmap.addMemSection( dm_r5f_ddr           );
dm_r5f_mmap.addMemSection( app_log_mem          );
dm_r5f_mmap.addMemSection( tiovx_obj_desc_mem   );
dm_r5f_mmap.addMemSection( app_fileio_mem          );
dm_r5f_mmap.addMemSection( ipc_vring_mem        );
dm_r5f_mmap.addMemSection( dm_r5f_ddr_local_heap  );
dm_r5f_mmap.addMemSection( ddr_shared_mem       );
dm_r5f_mmap.addMemSection( ddr_viss_config_heap       );
dm_r5f_mmap.checkOverlap();

c7x_1_mmap = MemoryMap("c7x_1");
c7x_1_mmap.addMemSection( c7x_1_l3           );
c7x_1_mmap.addMemSection( c7x_1_l2           );
c7x_1_mmap.addMemSection( c7x_1_l1           );
c7x_1_mmap.addMemSection( c7x_1_ddr_ipc      );
c7x_1_mmap.addMemSection( c7x_1_ddr_resource_table      );
c7x_1_mmap.addMemSection( c7x_1_ddr_ipc_tracebuf        );
c7x_1_mmap.addMemSection( c7x_1_ddr_boot     );
c7x_1_mmap.addMemSection( c7x_1_ddr_vecs     );
c7x_1_mmap.addMemSection( c7x_1_ddr          );
c7x_1_mmap.addMemSection( app_log_mem        );
c7x_1_mmap.addMemSection( tiovx_obj_desc_mem );
c7x_1_mmap.addMemSection( app_fileio_mem        );
c7x_1_mmap.addMemSection( ipc_vring_mem      );
c7x_1_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable  );
c7x_1_mmap.addMemSection( c7x_1_ddr_local_heap  );
c7x_1_mmap.addMemSection( c7x_1_ddr_scratch  );
c7x_1_mmap.addMemSection( ddr_shared_mem     );
c7x_1_mmap.checkOverlap();

html_mmap = MemoryMap("System Memory Map for Linux+RTOS mode");

html_mmap.addMemSection( c7x_1_l3           );
html_mmap.addMemSection( c7x_1_l2           );
html_mmap.addMemSection( c7x_1_l1           );
html_mmap.addMemSection( mcu_r5f_ddr_ipc     );
html_mmap.addMemSection( mcu_r5f_ddr_resource_table      );
html_mmap.addMemSection( mcu_r5f_ddr_ipc_tracebuf      );
html_mmap.addMemSection( mcu_r5f_ddr         );
html_mmap.addMemSection( mcu_r5f_ddr_local_heap );
html_mmap.addMemSection( ddr_viss_config_heap );
html_mmap.addMemSection( dm_r5f_ddr_ipc     );
html_mmap.addMemSection( dm_r5f_ddr_resource_table      );
html_mmap.addMemSection( dm_r5f_ddr_ipc_tracebuf      );
html_mmap.addMemSection( dm_r5f_ddr         );
html_mmap.addMemSection( dm_r5f_ddr_local_heap );
html_mmap.addMemSection( c7x_1_ddr_ipc     );
html_mmap.addMemSection( c7x_1_ddr_resource_table     );
html_mmap.addMemSection( c7x_1_ddr_ipc_tracebuf     );
html_mmap.addMemSection( c7x_1_ddr_boot    );
html_mmap.addMemSection( c7x_1_ddr_vecs    );
html_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable );
html_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable );
html_mmap.addMemSection( c7x_1_ddr_local_heap         );
html_mmap.addMemSection( c7x_1_ddr_scratch );
html_mmap.addMemSection( c7x_1_ddr         );
html_mmap.addMemSection( tifs_lpm_mem      );
html_mmap.addMemSection( atf_mem      );
html_mmap.addMemSection( optee_mem      );
html_mmap.addMemSection( app_log_mem        );
html_mmap.addMemSection( tiovx_obj_desc_mem );
html_mmap.addMemSection( app_fileio_mem        );
html_mmap.addMemSection( ipc_vring_mem      );
html_mmap.addMemSection( ddr_shared_mem     );
html_mmap.addMemSection( tiovx_log_rt_mem );
html_mmap.checkOverlap();

c_header_mmap = MemoryMap("Memory Map for C header file");
c_header_mmap.addMemSection( c7x_1_l3           );
c_header_mmap.addMemSection( c7x_1_l2           );
c_header_mmap.addMemSection( c7x_1_l1           );
c_header_mmap.addMemSection( mcu_r5f_ddr_ipc     );
c_header_mmap.addMemSection( dm_r5f_ddr_ipc     );
c_header_mmap.addMemSection( c7x_1_ddr_ipc     );
c_header_mmap.addMemSection( mcu_r5f_ddr_total     );
c_header_mmap.addMemSection( dm_r5f_ddr_total     );
c_header_mmap.addMemSection( c7x_1_ddr_total     );

c_header_mmap.addMemSection( mcu_r5f_ddr_local_heap);
c_header_mmap.addMemSection( ddr_viss_config_heap);
c_header_mmap.addMemSection( dm_r5f_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap_non_cacheable);
c_header_mmap.addMemSection( c7x_1_ddr_scratch_non_cacheable);
c_header_mmap.addMemSection( c7x_1_ddr_local_heap);
c_header_mmap.addMemSection( c7x_1_ddr_scratch);
c_header_mmap.addMemSection( tiovx_log_rt_mem );
c_header_mmap.addMemSection( app_log_mem        );
c_header_mmap.addMemSection( tiovx_obj_desc_mem );
c_header_mmap.addMemSection( app_fileio_mem        );
c_header_mmap.addMemSection( ipc_vring_mem      );
c_header_mmap.addMemSection( ddr_shared_mem     );
c_header_mmap.checkOverlap();

dts_mmap = MemoryMap("Memory Map for Linux kernel dts/dtsi file");
dts_mmap.addMemSection( mcu_r5f_ddr_ipc     );
dts_mmap.addMemSection( mcu_r5f_ddr_total   );
dts_mmap.addMemSection( dm_r5f_ddr_ipc     );
dts_mmap.addMemSection( dm_r5f_ddr_total   );
dts_mmap.addMemSection( c7x_1_ddr_ipc      );
dts_mmap.addMemSection( c7x_1_ddr_total    );
dts_mmap.addMemSection( ipc_vring_mem );
dts_mmap.checkOverlap();

edgeai_dts_mmap = MemoryMap("Memory Map for Linux kernel dts/dtsi file");
edgeai_dts_mmap.addMemSection( edgeai_ddr_total );
edgeai_dts_mmap.addMemSection( ddr_shared_mem     );
edgeai_dts_mmap.addMemSection( edgeai_core_heaps );
edgeai_dts_mmap.checkOverlap();

#Create Memory Sections required for example.syscfg under c7x_1
c7x_1_syscfg = MemoryMap("Memory Map for example.syscfg under c7x_1");
c7x_1_syscfg.addMemSection(c7x_1_l3);
c7x_1_syscfg.addMemSection(c7x_1_l2);
c7x_1_syscfg.addMemSection(c7x_1_ddr_total);
c7x_1_syscfg.addMemSection(ddr_shared_mem );
c7x_1_syscfg.addMemSection(c7x_1_ddr_local_heap);
c7x_1_syscfg.addMemSection(c7x_1_ddr_scratch );
c7x_1_syscfg.addMemSection(app_log_mem);
c7x_1_syscfg.addMemSection(app_fileio_mem );
c7x_1_syscfg.addMemSection(tiovx_obj_desc_mem);
c7x_1_syscfg.addMemSection(ipc_vring_mem);
c7x_1_syscfg.addMemSection(c7x_1_ddr_ipc);
c7x_1_syscfg.addMemSection(tiovx_log_rt_mem );
c7x_1_syscfg.addMemSection(c7x_1_ddr_local_heap_non_cacheable);
c7x_1_syscfg.addMemSection(c7x_1_ddr_scratch_non_cacheable);
c7x_1_syscfg.checkOverlap();

#Create Memory Sections required for example.syscfg under mcu1_0
mcu1_0_syscfg = MemoryMap("Memory Map for example.syscfg under mcu1_0");
mcu1_0_syscfg.addMemSection(dm_r5f_ddr_ipc);
mcu1_0_syscfg.addMemSection(ipc_vring_mem);
mcu1_0_syscfg.addMemSection(ddr_viss_config_heap);
mcu1_0_syscfg.checkOverlap();

#Create Memory Sections to update QNX BSP carveout 
qnx_bsp_carveout = MemoryMap("Carveout required for QNX BSP");
qnx_bsp_carveout.addMemSection(c7x_1_ddr_boot); #This section denotes the start address
qnx_bsp_carveout.addMemSection(c7x_1_ddr_scratch); #This section denotes the end segment in BSP Carveout 
qnx_bsp_carveout.checkOverlap();

#
# Generate linker command files containing "MEMORY" definitions
#
LinkerCmdFile(c7x_1_mmap , "../../rtos/am62a/cores/c7x_1/src/linker_mem_map.cmd" ).export();
LinkerCmdFile(dm_r5f_mmap, "../../rtos/am62a/cores/mcu1_0/src/linker_mem_map.cmd").export();

HtmlMmapTable(html_mmap, "./system_memory_map.html").export();

CHeaderFile(c_header_mmap, 0,0, "./app_mem_map.h").export();

#Update example.syscfg file for c7x_1
C7x_1_Syscfg(c7x_1_syscfg).export();

#Update example.syscfg file for mcu1_0
Mcu1_0_Syscfg(mcu1_0_syscfg).export();

#Update qnx_bsp_carveout under psdkqa/qnx
if os.path.isdir(os.path.join(os.path.dirname(__file__),'../../../psdkqa')):
	QNX_BSP_UPDATE(qnx_bsp_carveout).export();

def org_dts_check_path():
        ORG_DTS_PATH = os.environ.get("ORG_DTS_PATH")
        if not ORG_DTS_PATH :
                print("ERROR: ORG_DTS_PATH NOT SET, DTS CANNOT BE UPDATED")
                sys.exit(1)
        else:
                Org_dts_file(dts_mmap).export();

def edgeai_dts_check_path():
        EDGEAI_DTS_PATH = os.environ.get("EDGEAI_DTS_PATH")
        if not EDGEAI_DTS_PATH :
                print("ERROR: EDGEAI_DTS_PATH NOT SET, OVERLAY CANNOT BE UPDATED")
                sys.exit(1)
        else:
                Edgeai_dts_file(edgeai_dts_mmap).export();


org_dts_check_path();
edgeai_dts_check_path();
