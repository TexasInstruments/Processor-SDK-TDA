
ifeq ($(SOC),$(filter $(SOC), am65xx))
PACKAGE_SRCS_COMMON += cslr_vtm.h csl_vtm.h src/ip/vtm/src_files_vtm.mk src/ip/vtm/V0
INCDIR += src/ip/vtm/V0
endif

ifeq ($(SOC),$(filter $(SOC), j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_vtm.h csl_vtm.h src/ip/vtm/src_files_vtm.mk src/ip/vtm/V1
SRCS_COMMON += csl_vtm.c csl_vtm_pvt_sensor.c
SRCDIR += src/ip/vtm/V1/priv
INCDIR += src/ip/vtm/V1
endif

