
ifeq ($(SOC),$(filter $(SOC), tda3xx dra78x am65xx tpr12 awr294x))
PACKAGE_SRCS_COMMON += cslr_dcc.h csl_dcc.h src/ip/dcc/src_files_dcc.mk src/ip/dcc/V0
SRCDIR += src/ip/dcc/V0/priv
INCDIR += src/ip/dcc/V0
SRCS_COMMON += dcc.c
endif

ifeq ($(SOC),$(filter $(SOC), j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_dcc.h csl_dcc.h src/ip/dcc/src_files_dcc.mk src/ip/dcc/V1
SRCDIR += src/ip/dcc/V1/priv
INCDIR += src/ip/dcc/V1
SRCS_COMMON += dcc.c
endif
