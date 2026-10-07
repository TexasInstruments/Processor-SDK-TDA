
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_gtc.h src/ip/gtc/src_files_gtc.mk src/ip/gtc/V0
INCDIR += src/ip/gtc/V0
endif
