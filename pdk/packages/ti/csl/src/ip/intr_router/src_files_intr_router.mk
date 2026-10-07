
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_intr_router.h csl_intr_router.h src/ip/intr_router/src_files_intr_router.mk src/ip/intr_router/V0
SRCDIR += src/ip/intr_router/V0/priv
INCDIR += src/ip/intr_router/V0
SRCS_COMMON += csl_intr_router.c
endif
