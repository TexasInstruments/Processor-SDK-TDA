
ifeq ($(SOC),$(filter $(SOC), am65xx am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_udmap.h csl_udmap.h src/ip/udmap/src_files_udmap.mk src/ip/udmap/V0
SRCDIR += src/ip/udmap/V0/priv
INCDIR += src/ip/udmap/V0
SRCS_COMMON += csl_udmap.c
endif
