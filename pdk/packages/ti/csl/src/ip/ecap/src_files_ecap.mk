
ifeq ($(SOC),$(filter $(SOC), am574x am572x am571x omapl137 am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62lx))
  ifeq ($(SOC),$(filter $(SOC), am574x am572x am571x omapl137))
    PACKAGE_SRCS_COMMON += cslr_pwmss.h
  endif
PACKAGE_SRCS_COMMON += csl_ecap.h cslr_ecap.h src/ip/ecap/src_files_ecap.mk
PACKAGE_SRCS_COMMON += src/ip/ecap/V0/ecap.h src/ip/ecap/V0/priv
PACKAGE_SRCS_COMMON += src/ip/ecap/V0
SRCDIR += src/ip/ecap/V0/priv
INCDIR += . src/ip/ecap/V0
SRCS_COMMON += ecap.c
endif

ifeq ($(SOC),$(filter $(SOC), tpr12 awr294x))
PACKAGE_SRCS_COMMON += csl_ecap.h cslr_ecap.h src/ip/ecap/src_files_ecap.mk src/ip/ecap/V1
endif

ifeq ($(SOC),$(filter $(SOC), am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_ecap.h src/ip/ecap/src_files_ecap.mk src/ip/ecap/V2
INCDIR += . src/ip/ecap/V2
endif
