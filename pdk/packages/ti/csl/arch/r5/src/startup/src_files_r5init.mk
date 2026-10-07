ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am77x tpr12 awr294x am263x am64x am62x am62a am275x am283x))
  PACKAGE_SRCS_COMMON += r5/src/startup
  ifeq ($(ARCH),armv7r)
    SRCDIR += r5/src/startup
    INCDIR += r5/src/startup
    INCDIR += r5
    SRCS_ASM_COMMON += boot.asm r5_startup.asm resetvecs.asm handlers.asm
    SRCS_COMMON += startup.c default_r5f_mpu.c
  endif

endif
