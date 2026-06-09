#
# This file is the makefile for building ip fma library.
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

MODULE_NAME = ip_fma

SRCDIR = src/
INCDIR = inc/

INCLUDE_EXTERNAL_INTERFACES = pdk csl csirx fvid2

COMP_LIST_COMMON = csirx fvid2

SRCS_COMMON += src/ip_fma_common.c
SRCS_COMMON += src/ip_fma_csirx.c
SRCS_COMMON += src/ip_fma_dru.c
SRCS_COMMON += src/ip_fma_ddr.c
SRCS_COMMON += src/ip_fma_ecc_aggr.c
SRCS_COMMON += src/ip_fma_clk.c
SRCS_COMMON += src/ip_fma_clec.c
SRCS_COMMON += src/ip_fma_cfg_wrap_cc.c
SRCS_COMMON += src/ip_fma_rl7.c
SRCS_COMMON += src/ip_fma_dss.c
SRCS_COMMON += src/ip_fma_r5f.c
SRCS_COMMON += src/ip_fma_sa3ul.c
SRCS_COMMON += src/ip_fma_cba.c
SRCS_COMMON += src/ip_fma_psc.c
SRCS_COMMON += src/ip_fma_sms.c

ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4))
PACKAGE_SRCS_COMMON += inc/ip_fma_csirx.h src/ip_fma_csirx.c
PACKAGE_SRCS_COMMON += inc/ip_fma_dru.h src/ip_fma_dru.c
PACKAGE_SRCS_COMMON += inc/ip_fma_ddr.h src/ip_fma_ddr.c
PACKAGE_SRCS_COMMON += inc/ip_fma_ecc_aggr.h src/ip_fma_ecc_aggr.c
PACKAGE_SRCS_COMMON += inc/ip_fma_clk.h src/ip_fma_clk.c
PACKAGE_SRCS_COMMON += inc/ip_fma_clec.h src/ip_fma_clec.c
PACKAGE_SRCS_COMMON += inc/ip_fma_cfg_wrap_cc.h src/ip_fma_cfg_wrap_cc.c
PACKAGE_SRCS_COMMON += inc/ip_fma_cba.h src/ip_fma_cba.c
PACKAGE_SRCS_COMMON += inc/ip_fma_psc.h src/ip_fma_psc.c
PACKAGE_SRCS_COMMON += inc/ip_fma_common.h src/ip_fma_common.c
PACKAGE_SRCS_COMMON += inc/ip_fma_rl7.h src/ip_fma_rl7.c
PACKAGE_SRCS_COMMON += inc/ip_fma_dss.h src/ip_fma_dss.c
PACKAGE_SRCS_COMMON += inc/ip_fma_r5f.h src/ip_fma_r5f.c
PACKAGE_SRCS_COMMON += inc/ip_fma_sa3ul.h src/ip_fma_sa3ul.c
PACKAGE_SRCS_COMMON += inc/ip_fma_sms.h src/ip_fma_sms.c
PACKAGE_SRCS_COMMON += makefile ip_fma_component.mk
endif

# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-nameutils> =
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
ifeq ($(MAKERULEDIR), )
#Makerule path not defined, define this and assume relative path from ROOTDIR
  MAKERULEDIR := $(ROOTDIR)/ti/build/makerules
  export MAKERULEDIR
endif
include $(MAKERULEDIR)/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
