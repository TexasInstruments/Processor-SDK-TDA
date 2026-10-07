
ifeq ($(SOC),$(filter $(SOC), j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_tog.h csl_tog.h src/ip/tog/src_files_tog.mk src/ip/tog/V0
SRCDIR += src/ip/tog/V0/priv
INCDIR += . src/ip/tog/V0
SRCS_COMMON += csl_slv_tog.c
endif

ifeq ($(SOC),$(filter $(SOC), j7200 j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
SRCS_COMMON += csl_mst_tog.c
endif
