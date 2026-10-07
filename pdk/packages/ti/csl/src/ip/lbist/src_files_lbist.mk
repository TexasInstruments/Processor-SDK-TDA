
ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_lbist.h csl_lbist.h src/ip/lbist/src_files_lbist.mk src/ip/lbist/V0
SRCDIR += src/ip/lbist/V0/priv
INCDIR += src/ip/lbist/V0
SRCS_COMMON += csl_lbist.c
endif
