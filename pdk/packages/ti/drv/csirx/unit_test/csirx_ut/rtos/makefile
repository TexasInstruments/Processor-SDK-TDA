#
# This file is the makefile for building CSIRX unit test app for TI RTOS
#
include $(PDK_INSTALL_PATH)/ti/build/Rules.make

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

# List all the components required by the application
COMP_LIST_COMMON = fvid2 csirx

ifeq ($(BUILD_OS_TYPE),freertos)
    CFLAGS_OS_DEFINES = -DFREERTOS
    COMP_LIST_COMMON += $(PDK_COMMON_FREERTOS_COMP)
    INCLUDE_EXTERNAL_INTERFACES += freertos
endif

APP_NAME = csirx_unit_testapp_$(BUILD_OS_TYPE)
#
# This file is common makefile for building CSIRX unit test app for both TI-RTOS/baremetal
#
SRCDIR = . ../src ../src/soc
INCDIR = . ../src ../testInput $(PDK_CSIRX_COMP_PATH)/src $(PDK_CSIRX_COMP_PATH)/soc/V0



SRCS_COMMON = main_rtos.c

#XDC_CFG_UPDATE_$(CORE) = csirx_ut.cfg

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . ../src ../testInput ../../csirx_ut_component.mk
SRCS_COMMON += csirx_test_parser.c csirx_test_common.c csirx_utils_mem.c csirx_sensorCfg.c
SRCS_COMMON += csirx_test_misc.c csirx_test_code.c csirx_test_coverage.c csirx_test_csl_coverage.c csirx_test_fvid2_coverage.c
#SRCS_COMMON += utils_mem.c utils_prf.c

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(CSIRX_CFLAGS) $(FVID2_CFLAGS) $(CFLAGS_OS_DEFINES)

# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-name> =
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
