include $(PDK_INSTALL_PATH)/ti/build/Rules.make

BUILD_OS_TYPE=baremetal

ifeq ($(RB_ENABLE), TRUE)
 APP_NAME = csl_ddr_ecc_test_app_rb_enable
else
 APP_NAME = csl_ddr_ecc_test_app
endif

SRCDIR = .
INCDIR = .

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk csl_arch

# List all the components required by the application
ifeq ($(BOARD),$(filter $(BOARD), j721s2_evm j784s4_evm))
 COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP)
 ifeq ($(RB_ENABLE), TRUE)
  COMP_LIST_COMMON := $(filter-out csl,$(COMP_LIST_COMMON))
  COMP_LIST_COMMON += csl_ipfma
 endif
else
 COMP_LIST_COMMON += csl
 ifeq ($(CORE),$(filter $(CORE), mcu1_0))
  COMP_LIST_a15_0 = csl_init
 endif
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = .

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS)

ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4))
ifeq ($(CORE),mcu1_0)
SDL_INSTALL_PATH=$(PDK_INSTALL_PATH)/../../sdl
INCDIR += $(SDL_INSTALL_PATH)
INCDIR += $(SDL_INSTALL_PATH)/src/sdl
INCDIR += $(SDL_INSTALL_PATH)/include/soc/$(SOC)
INCDIR += $(SDL_INSTALL_PATH)/include
INCDIR += $(SDL_INSTALL_PATH)/osal
INCDIR += $(SDL_INSTALL_PATH)/test/osal
SRCDIR += $(SDL_INSTALL_PATH)/osal/src
SRCDIR += $(SDL_INSTALL_PATH)/test/osal/src
EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/src/sdl/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_api.$(LIBEXT)
EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/osal/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_osal.$(LIBEXT)
EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/src/ip/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_ip.$(LIBEXT)
EXTERNAL_LNKCMD_FILE_LOCAL = $(PDK_CSL_COMP_PATH)/example/ddr/ddr_ecc_test_app/linker_r5_$(SOC).lds
SRCS_COMMON = main.c ecc_ddr.c osal_interface.c ecc_ddr_prime_dma.c
endif
endif

# LNKFLAGS_LOCAL_a15_0 += --entry Entry
# LNKFLAGS_LOCAL_mpu1_0 += --entry Entry

ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4))
 SRCDIR += $(PDK_INSTALL_PATH)/ti/build/unit-test/Unity/src
 SRCDIR += $(PDK_INSTALL_PATH)/ti/build/unit-test/config
 INCDIR += $(PDK_INSTALL_PATH)/ti/build/unit-test/Unity/src
 INCDIR += $(PDK_INSTALL_PATH)/ti/build/unit-test/config
 SRCS_COMMON += unity_config.c unity.c
 CFLAGS_LOCAL_COMMON += -DUNITY_INCLUDE_CONFIG_H
endif

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