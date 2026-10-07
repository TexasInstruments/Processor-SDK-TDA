
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px tpr12 awr294x am275x am283x))
  PACKAGE_SRCS_COMMON += csl_cpgmac_sl.h src/ip/emac/src_files_emac.mk src/ip/emac/V5
  SRCDIR += src/ip/emac/V5/priv
  INCDIR += . src/ip/emac/V5
  SRCS_COMMON += csl_cpgmac_sl.c
endif
