
PACKAGE_SRCS_COMMON += cslr_ale.h
ifeq ($(SOC),$(filter $(SOC), k2g))
PACKAGE_SRCS_COMMON += src/ip/ale/V1/cslr_ale.h
SRCDIR += src/ip/ale/V1
INCDIR += . src/ip/ale/V1
else
ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px dra75x tda2ex dra72x am571x am572x tda3xx dra78x am574x))
PACKAGE_SRCS_COMMON += src/ip/ale/V2/cslr_ale.h
SRCDIR += src/ip/ale/V2
INCDIR += . src/ip/ale/V2
else
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px tpr12 awr294x am275x am283x))
PACKAGE_SRCS_COMMON += src/ip/ale/V4/cslr_ale.h
SRCDIR += src/ip/ale/V4
INCDIR += . src/ip/ale/V4
else
PACKAGE_SRCS_COMMON += src/ip/ale/V0/cslr_ale.h
SRCDIR += src/ip/ale/V0
INCDIR += . src/ip/ale/V0
endif
endif
endif
