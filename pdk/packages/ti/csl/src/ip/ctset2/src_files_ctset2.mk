
ifeq ($(SOC),$(filter $(SOC), j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_ctset2.h src/ip/ctset2/src_files_ctset2.mk src/ip/ctset2/V0
INCDIR += src/ip/ctset2/V0
endif

ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4 j742s2))
PACKAGE_SRCS_COMMON += src/ip/ctset2/V1
INCDIR += src/ip/ctset2/V1
endif
