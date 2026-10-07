
ifeq ($(SOC),$(filter $(SOC), am65xx j721e j721s2 j784s4 j742s2 j7200 am64x am62x))
PACKAGE_SRCS_COMMON += cslr_cp_ace.h src/ip/sa/src_files_sa.mk src/ip/sa/V2/cslr_eip_29t2_ram.h src/ip/sa/V3
endif
ifeq ($(SOC),$(filter $(SOC), am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_cp_ace.h src/ip/sa/src_files_sa.mk src/ip/sa/V2/cslr_eip_29t2_ram.h src/ip/sa/V5
endif
ifeq ($(SOC),$(filter $(SOC), am263x))
PACKAGE_SRCS_COMMON += src/ip/sa/src_files_sa.mk src/ip/sa/V4/cslr_eip_29t2_ram.h
endif
