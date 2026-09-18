# This distribution contains contributions or derivatives under copyright
# as follows:
#
# Copyright (c) 2026, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - Neither the name of Texas Instruments nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sim

##################### 
# Set reset vectors #
#####################
#MCU_SEC_MMR_BASE = (0x45950000)
mem_MCU_SEC_MMR_0_cfg0   = '/TDA5_System/TDA5_SoC/MCU_Domain/Periphs/MCU_SEC_MMR_0/cfg0'
DM_CORE_INTVECT          =  0x0040
HSM_CORE_INTVECT         =  0x0140
RT_CLSTR0_CORE0_CORE_CFG =  0x1080
RT_CLSTR0_CORE0_INTVEC   =  0x108C
RT_CLSTR0_CORE1_CORE_CFG =  0x10A0
RT_CLSTR0_CORE1_INTVEC   =  0x10AC
RT_CLSTR1_CORE0_CORE_CFG =  0x1180
RT_CLSTR1_CORE0_INTVEC   =  0x118C
RT_CLSTR1_CORE1_CORE_CFG =  0x11A0
RT_CLSTR1_CORE1_INTVEC   =  0x11AC
RT_CLSTR2_CORE0_CORE_CFG =  0x1280
RT_CLSTR2_CORE0_INTVEC   =  0x128C
RT_CLSTR2_CORE1_CORE_CFG =  0x12A0
RT_CLSTR2_CORE1_INTVEC   =  0x12AC
#MAIN_SEC_MMR_BASE = (0x45944000)
mem_MAIN_SEC_MMR_cfg0 = '/TDA5_System/TDA5_SoC/Main_Domain/Peripherals/MAIN_SEC_MMR_0/cfg0'
MM_CORE0_CORE_CFG     =   0x0020
MM_CORE0_INTVECT      =   0x0040
MM_CORE1_CORE_CFG     =   0x0120
MM_CORE1_INTVECT      =   0x0140
MM_CORE2_CORE_CFG     =   0x0220
MM_CORE2_INTVECT      =   0x0240
MM_CORE3_CORE_CFG     =   0x0320
MM_CORE3_INTVECT      =   0x0340
MM_CORE4_CORE_CFG     =   0x0420
MM_CORE4_INTVECT      =   0x0440
#DMSC_BOOT_BASE = (0x45A00000)
mem_CCC_DMSC_BOOT = '/TDA5_System/TDA5_SoC/Main_Domain/CCC/MSMC_Wrap/DMSC_0/BOOT'
MM_A720_CORE0_INTVECT =   0x1010
MM_A720_CORE1_INTVECT =   0x1018
MM_A720_CORE2_INTVECT =   0x1020
MM_A720_CORE3_INTVECT =   0x1028
MM_A720_CORE4_INTVECT =   0x1030
MM_A720_CORE5_INTVECT =   0x1038
MM_A720_CORE6_INTVECT =   0x1040
MM_A720_CORE7_INTVECT =   0x1048
AW0_C76_INTVECT       =   0x5010
AW1_C76_INTVECT       =   0x6010
AW2_C76_INTVECT       =   0x7010
AW3_C76_INTVECT       =   0x8010

################################
# CONFIGURE Cores out of reset #
################################
MDCTL_OFFSET = 0xA00
PDCTL_OFFSET = 0x300
PTCMD_OFFSET = 0x120
PSC_MDCTL_NEXT_MASK         = (0x0000001F)
PSC_MDCTL_NEXT_SWRSTDISABLE = (0x00000000)
PSC_MDCTL_NEXT_SYNCRST      = (0x00000001)
PSC_MDCTL_NEXT_DISABLE      = (0x00000002)
PSC_MDCTL_NEXT_ENABLE       = (0x00000003)
PSC_MDCTL_NEXT_AUTOSLEEP    = (0x00000004)
PSC_MDCTL_NEXT_AUTOWAKE     = (0x00000005)
PSC_PDCTL_NEXT_MASK         = (0x00000001)
PSC_PDCTL_NEXT_SHIFT        = (0x00000000)
PSC_PDCTL_NEXT_ON           = (0x00000001)

# SEC_PSC0_REG_BASE = 0x5e620000
mem_SEC_PSC_mReg = '/TDA5_System/TDA5_SoC/MCU_Domain/Security/SEC_PSC_0/mReg'
LPSC_SEC_AUTOHSM_IDX = 4
# MCU_PSC0_REG_BASE = 0x5e600000
mem_MCU_PSC_0_mReg = '/TDA5_System/TDA5_SoC/MCU_Domain/Periphs/MCU_PSC_0/mReg'
LPSC_MCU_DM_M55_IDX = 2
# MCU_PSC1_REG_BASE = 0x5e610000
mem_MCU_PSC_1_mReg = '/TDA5_System/TDA5_SoC/MCU_Domain/Periphs/MCU_PSC_1/mReg'
LPSC_MCU_RT_CLSTR0_CORE0_IDX = 12
LPSC_MCU_RT_CLSTR0_CORE1_IDX = 13
LPSC_MCU_RT_CLSTR1_CORE0_IDX = 15
LPSC_MCU_RT_CLSTR1_CORE1_IDX = 16
LPSC_MCU_RT_CLSTR2_CORE0_IDX = 18
LPSC_MCU_RT_CLSTR2_CORE1_IDX = 19
# MAIN_PSC0_ADDRESS_BASE = 0x00400000
mem_MAIN_PSC_0_mReg = '/TDA5_System/TDA5_SoC/Main_Domain/Peripherals/MAIN_PSC_0/mReg'
LPSC_MAIN_A720_CORE0_IDX = 51
LPSC_MAIN_A720_CORE1_IDX = 53
LPSC_MAIN_A720_CORE2_IDX = 55
LPSC_MAIN_A720_CORE3_IDX = 57
LPSC_MAIN_A720_CORE4_IDX = 59
LPSC_MAIN_A720_CORE5_IDX = 61
LPSC_MAIN_A720_CORE6_IDX = 63
LPSC_MAIN_A720_CORE7_IDX = 65
LPSC_MAIN_AW0_C7_IDX = 82
LPSC_MAIN_AW1_C7_IDX = 88
LPSC_MAIN_AW2_C7_IDX = 94
LPSC_MAIN_AW3_C7_IDX = 100
# MAIN_PSC1_ADDRESS_BASE = 0x00410000
mem_MAIN_PSC_1_mReg = '/TDA5_System/TDA5_SoC/Main_Domain/Peripherals/MAIN_PSC_1/mReg'
LPSC_MAIN_M55_CLSTR0_IDX = 2
LPSC_MAIN_M55_CLSTR1_IDX = 7
LPSC_MAIN_M55_CLSTR2_IDX = 8
LPSC_MAIN_M55_CLSTR3_IDX = 9
LPSC_MAIN_M55_CLSTR4_IDX = 10

# MAIN_CTRL_MMR_0_BASE = (0x00100000)
mem_MAIN_CTRL_MMR_cfg0 = '/TDA5_System/TDA5_SoC/Main_Domain/Peripherals/MAIN_CTRL_MMR_0/cfg0'
mem_MAIN_DEVSTAT_offset = 0x30

##############
# PSC States #
##############

psc_configurations = {
    'MCU_SEC_PSC_LPSC_STATE': {
        'g_MCU_SEC_PSC_LPSC_STATE_00_31': 0xFFFFFFEF,
        'g_MCU_SEC_PSC_LPSC_STATE_32_63': 0xFFFFFFFF,
        'g_MCU_SEC_PSC_LPSC_STATE_64_95': 0xFFFFFFFF,
        'g_MCU_SEC_PSC_LPSC_STATE_96_127': 0xFFFFFFFF,
    },
    'MCU_PSC_0_LPSC_STATE': {
        'g_MCU_PSC_0_LPSC_STATE_00_31': 0xFFFFFFFB,
        'g_MCU_PSC_0_LPSC_STATE_32_63': 0xFFFFFFFF,
        'g_MCU_PSC_0_LPSC_STATE_64_95': 0xFFFFFFFF,
        'g_MCU_PSC_0_LPSC_STATE_96_127': 0xFFFFFFFF,
    },
    'MCU_PSC_1_LPSC_STATE': {
        'g_MCU_PSC_1_LPSC_STATE_00_31': 0xFFF24FFF,
        'g_MCU_PSC_1_LPSC_STATE_32_63': 0xFFFFFFFF,
        'g_MCU_PSC_1_LPSC_STATE_64_95': 0xFFFFFFFF,
        'g_MCU_PSC_1_LPSC_STATE_96_127': 0xFFFFFFFF,
    },
    'MAIN_PSC_0_LPSC_STATE': {
        'g_MAIN_PSC_0_LPSC_STATE_00_31': 0xFFFFFFFF,
        'g_MAIN_PSC_0_LPSC_STATE_32_63': 0x5557FFFF,
        'g_MAIN_PSC_0_LPSC_STATE_64_95': 0x0000007D,
        'g_MAIN_PSC_0_LPSC_STATE_96_127': 0xFFFFFF00,
    },
    'MAIN_PSC_1_LPSC_STATE': {
        'g_MAIN_PSC_1_LPSC_STATE_00_31': 0xFFFFF87B,
        'g_MAIN_PSC_1_LPSC_STATE_32_63': 0xFFFFFFFF,
        'g_MAIN_PSC_1_LPSC_STATE_64_95': 0x00000000,
        'g_MAIN_PSC_1_LPSC_STATE_96_127': 0x00000000,
    }
}

#####################
# UTILITY FUNCTIONS #
#####################

def hex_to_bit_indices(hex_value, start_bit=0):
    # Convert a hex value to a list of bit indices where bits are set to 1.
    indices = []
    for bit_pos in range(32):  # 32 bits per uint32_t
        if hex_value & (1 << bit_pos):
            indices.append(start_bit + bit_pos)
    return indices

def analyze_psc_instance(psc_values):
    # Analyze a single PSC instance and return the enabled indices array.
    all_indices = []
    for i, (_, hex_val) in enumerate(psc_values.items()):
        start_bit = i * 32
        indices = hex_to_bit_indices(hex_val, start_bit)
        all_indices.extend(indices)
    return sorted(all_indices)

def read_val_from_mem_reg(mem_name, offset):
    mem_probe = sim.MemoryProbe(mem_name, 4, offset)
    read_val = mem_probe.call_read_behavior()
    sim.wait_for((1,'ns'))
    return read_val

def write_mem_reg_with_val(mem_name, offset, val):
    mem_probe = sim.MemoryProbe(mem_name, 4, offset)
    mem_probe.call_write_behavior_with_value(val)
    sim.wait_for((1,'ns'))

def program_lpsc(psc_mem_name, lpsc_idx_list, next_state = PSC_MDCTL_NEXT_ENABLE):
    # Set next states
    for idx in lpsc_idx_list:
        curr_value = read_val_from_mem_reg(psc_mem_name, MDCTL_OFFSET + (idx*4))
        next_value = (curr_value & ~(PSC_MDCTL_NEXT_MASK)) | (PSC_MDCTL_NEXT_MASK & next_state)
        write_mem_reg_with_val(psc_mem_name, MDCTL_OFFSET+(idx*4), next_value)
    # Set the 'GO' bits to start transition to the next states
    write_mem_reg_with_val(psc_mem_name, PTCMD_OFFSET,   0xFFFFFFFF)
    write_mem_reg_with_val(psc_mem_name, PTCMD_OFFSET+4, 0xFFFFFFFF)

def enable_all_pd(psc_mem_name):
    for pd_num in range(64):
        # enable the PD
        curr_value = read_val_from_mem_reg(psc_mem_name, PDCTL_OFFSET + (pd_num*4))
        next_value = (curr_value & ~PSC_PDCTL_NEXT_MASK) | (PSC_PDCTL_NEXT_ON << PSC_PDCTL_NEXT_SHIFT)
        write_mem_reg_with_val(psc_mem_name, PDCTL_OFFSET + (pd_num*4), next_value)
