
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px tpr12 awr294x am275x am283x))
PACKAGE_SRCS_COMMON += cslr_cpts.h csl_cpts.h src/ip/cpts/src_files_cpts.mk src/ip/cpts/V0
SRCDIR += src/ip/cpts/V0/priv
INCDIR += src/ip/cpts/V0
SRCS_COMMON += csl_cpts.c
endif
