
ifeq ($(SOC),$(filter $(SOC), tda3xx dra78x am65xx j7200 j721e j721s2 j784s4 j742s2 tpr12 awr294x am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_crc.h csl_crc.h src/ip/crc/src_files_crc.mk src/ip/crc/V0
SRCDIR += src/ip/crc/V0/priv
INCDIR += src/ip/crc/V0
SRCS_COMMON += crc.c
endif
