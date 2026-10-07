
ifeq ($(SOC),$(filter $(SOC), tda2xx tda2px tda2ex tda3xx dra72x dra75x dra78x am571x am572x am574x am437x am335x k2g omapl137 omapl138))
PACKAGE_SRCS_COMMON += cslr_pwmss.h cslr_epwm.h csl_epwm.h src/ip/epwm/src_files_epwm.mk src/ip/epwm/V0
PACKAGE_SRCS_COMMON += src/ip/epwm/hw_pwmss_submodule_offsets.h
SRCDIR += src/ip/epwm/V0/priv
INCDIR += src/ip/epwm/V0
SRCS_COMMON += csl_epwm.c
endif

ifeq ($(SOC),$(filter $(SOC), k2g omapl137 omapl138))
PACKAGE_SRCS_COMMON += src/ip/epwm/V0_1
endif

ifeq ($(SOC),$(filter $(SOC), j7200 j721e j721s2 j784s4 j742s2 am65xx))
PACKAGE_SRCS_COMMON += src/ip/epwm/V0_1
PACKAGE_SRCS_COMMON += cslr_epwm.h csl_epwm.h src/ip/epwm/src_files_epwm.mk src/ip/epwm/V0
PACKAGE_SRCS_COMMON += src/ip/epwm/hw_pwmss_submodule_offsets.h
SRCDIR += src/ip/epwm/V0/priv src/ip/epwm/V0_1/priv
SRCS_COMMON += csl_epwm.c csl_hrpwm.c
endif

ifeq ($(SOC),$(filter $(SOC), am64x am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += src/ip/epwm/V0_1/hw_pwmss_submodule_offsets.h
PACKAGE_SRCS_COMMON += src/ip/epwm/V0_2
PACKAGE_SRCS_COMMON += cslr_epwm.h csl_epwm.h src/ip/epwm/src_files_epwm.mk src/ip/epwm/V0
PACKAGE_SRCS_COMMON += src/ip/epwm/hw_pwmss_submodule_offsets.h
SRCDIR += src/ip/epwm/V0/priv
SRCS_COMMON += csl_epwm.c
endif
