
ifeq ($(SOC),$(filter $(SOC), tda3xx dra78x tda2px))
PACKAGE_SRCS_COMMON += cslr_mcan.h csl_mcan.h src/ip/mcan/mcan.h src/ip/mcan/src_files_mcan.mk src/ip/mcan/V0
SRCDIR += src/ip/mcan/V0
INCDIR += src/ip/mcan/V0
SRCS_COMMON += mcan.c
endif

ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
  ifneq ($(CORE),$(filter $(CORE), c7x-hostemu))
    PACKAGE_SRCS_COMMON += cslr_mcan.h csl_mcan.h src/ip/mcan/mcan.h src/ip/mcan/src_files_mcan.mk src/ip/mcan/V1
    SRCDIR += src/ip/mcan/V1
    INCDIR += src/ip/mcan/V1/V1_0
    SRCS_COMMON += mcan.c
  endif
endif

ifeq ($(SOC),$(filter $(SOC), tpr12 awr294x))
  PACKAGE_SRCS_COMMON += cslr_mcan.h csl_mcan.h src/ip/mcan/mcan.h src/ip/mcan/src_files_mcan.mk src/ip/mcan/V1
  SRCDIR += src/ip/mcan/V1
  INCDIR += src/ip/mcan/V1/V1_1
  SRCS_COMMON += mcan.c
endif
