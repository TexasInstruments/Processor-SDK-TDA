
ifeq ($(SOC),$(filter $(SOC), am64x am62x am62a am62px am275x am283x))
PACKAGE_SRCS_COMMON += cslr_pktdma.h csl_pktdma.h src/ip/udmap/V0/csl_udmap_cppi5.h src/ip/pktdma/V0 src/ip/pktdma/src_files_pktdma.mk
SRCDIR += src/ip/pktdma/V0/priv
INCDIR += src/ip/pktdma/V0
SRCS_COMMON += csl_pktdma.c
endif
