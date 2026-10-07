
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_ospi.h csl_ospi.h src/ip/ospi/src_files_ospi.mk src/ip/ospi/V0
SRCS_COMMON += csl_ospi.c
SRCDIR += src/ip/ospi/V0/priv
INCDIR += . src/ip/ospi/V0
endif
