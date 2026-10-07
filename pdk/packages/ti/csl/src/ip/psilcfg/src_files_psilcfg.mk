
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_psilcfg.h csl_psilcfg.h src/ip/psilcfg/src_files_psilcfg.mk src/ip/psilcfg/V0
SRCDIR += src/ip/psilcfg/V0/priv
INCDIR += src/ip/psilcfg/V0
SRCS_COMMON += csl_psilcfg.c
endif
