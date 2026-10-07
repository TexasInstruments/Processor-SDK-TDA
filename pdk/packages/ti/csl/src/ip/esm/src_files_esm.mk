
ifeq ($(SOC),$(filter $(SOC), tda3xx dra78x tpr12 awr294x))
PACKAGE_SRCS_COMMON += cslr_esm.h csl_esm.h src/ip/esm/src_files_esm.mk src/ip/esm/V0
SRCDIR += src/ip/esm/V0/priv
INCDIR += src/ip/esm/V0
ifeq ($(SOC),$(filter $(SOC), tpr12 awr294x))
  INCDIR += src/ip/esm/V0/V0_1
endif
SRCS_COMMON += esm.c
endif

ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e))
PACKAGE_SRCS_COMMON += cslr_esm.h csl_esm.h src/ip/esm/src_files_esm.mk src/ip/esm/V1
SRCDIR += src/ip/esm/V1/priv
INCDIR += src/ip/esm/V1/V1_0
SRCS_COMMON += csl_esm.c
endif

ifeq ($(SOC),$(filter $(SOC), am64x am62x am62a am62px j721s2 j784s4 j742s2 am275x am283x))
PACKAGE_SRCS_COMMON += cslr_esm.h csl_esm.h src/ip/esm/src_files_esm.mk src/ip/esm/V1
SRCDIR += src/ip/esm/V1/priv
INCDIR += src/ip/esm/V1/V1_1
SRCS_COMMON += csl_esm.c
endif
