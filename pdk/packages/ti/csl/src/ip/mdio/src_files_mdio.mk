
ifeq ($(SOC),$(filter $(SOC), k2h k2k k2l k2e k2g))

PACKAGE_SRCS_COMMON += csl_mdioAux.h  csl_mdio_def.h  csl_mdio.h  cslr_mdio.h
PACKAGE_SRCS_COMMON += src/ip/mdio/V0/csl_mdioAux.h src/ip/mdio/V0/csl_mdio.h
PACKAGE_SRCS_COMMON += src/ip/mdio/V0/cslr_mdio.h
PACKAGE_SRCS_COMMON += src/ip/mdio/src_files_mdio.mk src/ip/mdio/V0
SRCS_COMMON += cslr_mdio_resetread.c
SRCDIR += src/ip/mdio/V0
INCDIR += . src/ip/mdio/V0

endif

ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px dra75x tda2ex dra72x am571x am572x am574x tda3xx dra78x))
PACKAGE_SRCS_COMMON += cslr_mdio.h src/ip/mdio/src_files_mdio.mk src/ip/mdio/V2
endif

ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px tpr12 awr294x am275x am283x))
PACKAGE_SRCS_COMMON += cslr_mdio.h csl_mdio.h csl_mdioAux.h csl_mdio_def.h src/ip/mdio/src_files_mdio.mk src/ip/mdio/V4/cslr_mdio.h src/ip/mdio/V5
SRCDIR += src/ip/mdio/V5/priv
INCDIR += src/ip/mdio/V5
SRCS_COMMON += csl_mdio.c
endif
