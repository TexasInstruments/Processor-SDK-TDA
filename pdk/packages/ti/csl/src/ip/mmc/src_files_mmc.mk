PACKAGE_SRCS_COMMON += cslr_mmc.h cslr_mmchs.h csl_mmcsd.h src/ip/mmc/src_files_mmc.mk

ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px dra75x tda2ex dra72x am571x am572x am574x tda3xx am335x am437x dra78x))
PACKAGE_SRCS_COMMON +=src/ip/mmc/V0
SRCS_COMMON += mmcsd.c
SRCDIR += src/ip/mmc/V0/priv
INCDIR += . src/ip/mmc/V0
endif

ifeq ($(SOC),$(filter $(SOC), omapl137 omapl138))
PACKAGE_SRCS_COMMON +=src/ip/mmc/V1
SRCS_COMMON += mmcsd.c
SRCDIR += src/ip/mmc/V1/priv
INCDIR += . src/ip/mmc/V1
endif

ifeq ($(SOC),$(filter $(SOC), am65xx))
PACKAGE_SRCS_COMMON += src/ip/mmc/V3 src/ip/mmc/V2
SRCS_COMMON += mmcsd.c mmcsdphy.c low_level_hw.c
SRCDIR += src/ip/mmc/V3/priv
INCDIR += . src/ip/mmc/V3 src/ip/mmc/V3/priv
endif

ifeq ($(SOC),$(filter $(SOC),j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += src/ip/mmc/V4
SRCS_COMMON += mmcsd.c mmcsdphy.c low_level_hw.c
SRCDIR += src/ip/mmc/V4/priv
INCDIR += . src/ip/mmc/V4 src/ip/mmc/V4/priv
endif

ifeq ($(SOC),$(filter $(SOC), am275x am283x))
PACKAGE_SRCS_COMMON += src/ip/mmc/V5
INCDIR += . src/ip/mmc/V5
endif
