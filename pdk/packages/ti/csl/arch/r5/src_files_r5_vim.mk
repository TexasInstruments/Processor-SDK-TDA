
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am77x tpr12 awr294x am263x am64x am62x am62a am275x am283x))
  PACKAGE_SRCS_COMMON += arch/r5/cslr_vim.h
  PACKAGE_SRCS_COMMON += arch/r5/csl_vim.h
  PACKAGE_SRCS_COMMON += arch/r5/src/csl_vim.c
  PACKAGE_SRCS_COMMON += arch/r5/src_files_r5_vim.mk

  ifeq ($(ARCH),armv7r)
    SRCDIR += ./arch/r5/src
    INCDIR += ./arch/r5
    SRCS_COMMON += csl_vim.c
  endif

endif
