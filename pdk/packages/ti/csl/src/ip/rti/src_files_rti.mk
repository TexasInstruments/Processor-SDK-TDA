
ifeq ($(SOC),$(filter $(SOC), tda3xx dra78x am65xx j7200 j721e j721s2 j784s4 j742s2 tpr12 awr294x am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_rti.h csl_rti.h src/ip/rti/src_files_rti.mk src/ip/rti/V0/hw_rti.h src/ip/rti/V0/rti.h  src/ip/rti/V0/priv/rti.c  
SRCDIR += src/ip/rti/V0/priv
INCDIR += src/ip/rti/V0
SRCS_COMMON += rti.c
endif

ifeq ($(SOC),$(filter $(SOC), tpr12 awr294x))
PACKAGE_SRCS_COMMON += src/ip/rti/V0/rtiTmr.h  src/ip/rti/V0/priv/rtiTmr.c
SRCS_COMMON += rtiTmr.c
endif
