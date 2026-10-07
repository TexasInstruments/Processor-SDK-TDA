
ifeq ($(SOC),$(filter $(SOC), am65xx))
PACKAGE_SRCS_COMMON += cslr_usb.h src/ip/usb/src_files_usb.mk src/ip/usb/V4
INCDIR += . src/ip/usb/V4
endif

ifeq ($(SOC),$(filter $(SOC), j721e))
PACKAGE_SRCS_COMMON += cslr_usb.h src/ip/usb/src_files_usb.mk src/ip/usb/V5/V5_1
INCDIR += . src/ip/usb/V5/V5_1
endif

ifeq ($(SOC),$(filter $(SOC), j7200 am64x j721s2 j784s4 j742s2))
PACKAGE_SRCS_COMMON += cslr_usb.h src/ip/usb/src_files_usb.mk src/ip/usb/V5/V5_2
INCDIR += . src/ip/usb/V5/V5_2
endif

ifeq ($(SOC),$(filter $(SOC), am62x am62a am62px am62lx am275x am283x))
PACKAGE_SRCS_COMMON += cslr_usb.h src/ip/usb/src_files_usb.mk src/ip/usb/V6
INCDIR += . src/ip/usb/V6
endif
