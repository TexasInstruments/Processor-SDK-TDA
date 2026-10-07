
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am77x tpr12 awr294x am263x am64x am62x am62a am275x am283x))
  PACKAGE_SRCS_COMMON += arch/r5/src/csl_mcu_armss_ccmr5.c
  PACKAGE_SRCS_COMMON += arch/r5/cslr_mcu_armss_ccmr5.h arch/r5/csl_mcu_armss_ccmr5.h
  PACKAGE_SRCS_COMMON += arch/r5/src_files_r5_ccmr5.mk
  ifeq ($(ARCH),armv7r)
    SRCDIR += ./arch/r5/src
    INCDIR += ./arch/r5
    SRCS_COMMON += csl_mcu_armss_ccmr5.c
  endif

endif
