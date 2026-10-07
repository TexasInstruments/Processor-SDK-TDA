
ifeq ($(SOC),$(filter $(SOC), am65xx j7200 j721e j721s2 j784s4 j742s2 am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_timer_mgr.h csl_timer_mgr.h src/ip/timer_mgr/src_files_timer_mgr.mk src/ip/timer_mgr/V0
SRCDIR += src/ip/timer_mgr/V0/priv
INCDIR += src/ip/timer_mgr/V0
SRCS_COMMON += csl_timer_mgr.c
endif
