
ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4))
  PACKAGE_SRCS_COMMON += src/ip/msmc/cslr_msmc.h src/ip/msmc/csl_msmcmaint.h src/ip/msmc/src_files_msmc.mk src/ip/msmc/priv/
endif
