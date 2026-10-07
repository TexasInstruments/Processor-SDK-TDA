
ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px dra75x tda2ex dra72x am571x am572x am574x tda3xx am335x am437x dra78x j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_elm.h csl_elm.h src/ip/elm/src_files_elm.mk src/ip/elm/V0
SRCS_COMMON += elm.c
SRCDIR += src/ip/elm/V0/priv
INCDIR += . src/ip/elm/V0
endif
