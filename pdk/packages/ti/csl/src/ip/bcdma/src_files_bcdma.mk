
ifeq ($(SOC),$(filter $(SOC), am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_bcdma.h csl_bcdma.h src/ip/bcdma/src_files_bcdma.mk src/ip/bcdma/V0 src/ip/bcdma/V0/priv src/ip/udmap/V0/csl_udmap_tr.h
SRCDIR += src/ip/bcdma/V0/priv
INCDIR += src/ip/bcdma/V0
SRCS_COMMON += csl_bcdma.c

ifeq ($(SOC),$(filter $(SOC), am64x))
INCDIR += src/ip/bcdma/V0/V0_1
endif

ifeq ($(SOC),$(filter $(SOC), am62px am275x am283x))
INCDIR += src/ip/bcdma/V0/V0_2
endif

ifeq ($(SOC),$(filter $(SOC), am62x am62a))
INCDIR += src/ip/bcdma/V0/V0_3
endif

endif
