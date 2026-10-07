
ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px dra75x tda2ex dra72x am571x am572x am574x tda3xx am335x am437x dra78x am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_mcspi.h csl_mcspi.h src/ip/mcspi/src_files_mcspi.mk src/ip/mcspi/V0
SRCS_COMMON += mcspi.c
SRCDIR += src/ip/mcspi/V0/priv
INCDIR += . src/ip/mcspi/V0
endif
