
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_rat.h csl_rat.h src/ip/rat/src_files_rat.mk src/ip/rat/V0
SRCDIR += src/ip/rat/V0/priv
INCDIR += src/ip/rat/V0
SRCS_COMMON += csl_rat.c
endif
