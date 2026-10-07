
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62a am62px am62x am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_psc.h csl_psc.h src/ip/psc/src_files_psc.mk src/ip/psc/V2
SRCDIR += src/ip/psc/V2/priv
INCDIR += src/ip/psc/V2
SRCS_COMMON += csl_psc.c
endif
