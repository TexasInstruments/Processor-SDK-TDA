# File: ipc_component.mk
#       This file is component include make file of IPC driver library.
# List of variables set in this file and their purpose:
# <mod>_RELPATH        - This is the relative path of the module, typically from
#                        top-level directory of the package
# <mod>_PATH           - This is the absolute path of the module. It derives from
#                        absolute path of the top-level directory (set in env.mk)
#                        and relative path set above
# <mod>_INCLUDE        - This is the path that has interface header files of the
#                        module. This can be multiple directories (space separated)
# <mod>_PKG_LIST       - Names of the modules (and sub-modules) that are a part
#                        part of this module, including itself.
# <mod>_BOARD_DEPENDENCY - "yes": means the code for this module depends on
#                             platform and the compiled obj/lib has to be kept
#                             under <platform> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no platform dependent code and hence
#                             the obj/libs are not kept under <platform> dir.
# <mod>_CORE_DEPENDENCY     - "yes": means the code for this module depends on
#                             core and the compiled obj/lib has to be kept
#                             under <core> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no core dependent code and hence
#                             the obj/libs are not kept under <core> dir.
# <mod>_APP_STAGE_FILES     - List of source files that belongs to the module
#                             <mod>, but that needs to be compiled at application
#                             build stage (in the context of the app). This is
#                             primarily for link time configurations or if the
#                             source file is dependent on options/defines that are
#                             application dependent. This can be left blank or
#                             not defined at all, in which case, it means there
#                             no source files in the module <mod> that are required
#                             to be compiled in the application build stage.
#
ifeq ($(ipc_component_make_include), )

############################
# ipc package
# List of components included under ipc lib
# The components included here are built and will be part of ipc lib
############################
ipc_LIB_LIST = ipc

drvipc_RTOS_LIST       = $(DEFAULT_RTOS_LIST)

drvipc_SOCLIST         = j721e j7200 j721s2 j784s4 j742s2
drvipc_BOARDLIST       = j721e_sim j721e_qt j721e_evm j7200_evm j721s2_evm j784s4_evm j742s2_evm
drvipc_j721e_CORELIST  = mcu1_0 mcu2_0 mcu3_0 mcu1_1 mcu2_1 mcu3_1 c66xdsp_1 c66xdsp_2 c7x_1
drvipc_j721e_LASTCORE := $(word $(words $(drvipc_j721e_CORELIST)), $(drvipc_j721e_CORELIST))
drvipc_j721e_BAREMETAL_CORELIST = mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1
drvipc_j721e_RTOS_CORELIST = $(drvipc_j721e_CORELIST)
drvipc_j7200_CORELIST  = mcu1_0 mcu2_0 mcu1_1 mcu2_1
drvipc_j7200_LASTCORE := $(word $(words $(drvipc_j7200_CORELIST)), $(drvipc_j7200_CORELIST))
drvipc_j7200_BAREMETAL_CORELIST = mcu1_1 mcu2_0 mcu2_1
drvipc_j7200_RTOS_CORELIST = $(drvipc_j7200_CORELIST)
drvipc_j721s2_CORELIST  = mcu1_0 mcu2_0 mcu3_0 mcu1_1 mcu2_1 mcu3_1 c7x_1 c7x_2
drvipc_j721s2_LASTCORE := $(word $(words $(drvipc_j721s2_CORELIST)), $(drvipc_j721s2_CORELIST))
drvipc_j721s2_BAREMETAL_CORELIST = mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1
drvipc_j721s2_RTOS_CORELIST = $(drvipc_j721s2_CORELIST)
drvipc_j784s4_CORELIST  = mcu1_0 mcu2_0 mcu3_0 mcu4_0 mcu1_1 mcu2_1 mcu3_1 mcu4_1 c7x_1 c7x_2 c7x_3 c7x_4
drvipc_j784s4_LASTCORE := $(word $(words $(drvipc_j784s4_CORELIST)), $(drvipc_j784s4_CORELIST))
drvipc_j784s4_BAREMETAL_CORELIST = mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 mcu4_0 mcu4_1
drvipc_j784s4_RTOS_CORELIST = $(drvipc_j784s4_CORELIST)
drvipc_j742s2_CORELIST  = mcu1_0 mcu2_0 mcu3_0 mcu4_0 mcu1_1 mcu2_1 mcu3_1 mcu4_1 c7x_1 c7x_2 c7x_3
drvipc_j742s2_LASTCORE := $(word $(words $(drvipc_j742s2_CORELIST)), $(drvipc_j742s2_CORELIST))
drvipc_j742s2_BAREMETAL_CORELIST = mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 mcu4_0 mcu4_1
drvipc_j742s2_RTOS_CORELIST = $(drvipc_j742s2_CORELIST)
drvipc_DISABLE_PARALLEL_MAKE = yes

############################
# ipc examples
# List of examples under ipc (+= is used at each example definition)
# All the tests mentioned in list are built when test target is called
# List below all examples for allowed values
############################
ipc_EXAMPLE_LIST =

# IPC dependent examples which should be built after all examples are completed
ipc_DUP_EXAMPLE_LIST =

#
# IPC Modules
#

# IPC library
ipc_COMP_LIST = ipc
ipc_RELPATH = ti/drv/ipc
ipc_PATH = $(PDK_IPC_COMP_PATH)
ipc_LIBNAME = ipc
ipc_LIBPATH = $(PDK_IPC_COMP_PATH)/lib
ipc_MAKEFILE = -fsrc/makefile
export ipc_MAKEFILE
export ipc_LIBNAME
export ipc_LIBPATH
ipc_BOARD_DEPENDENCY = no
ipc_CORE_DEPENDENCY = yes
export ipc_COMP_LIST
export ipc_BOARD_DEPENDENCY
export ipc_CORE_DEPENDENCY
ipc_PKG_LIST = ipc
ipc_INCLUDE = $(ipc_PATH)
ipc_SOCLIST = $(drvipc_SOCLIST)
export ipc_SOCLIST
ipc_$(SOC)_CORELIST = $(drvipc_$(SOC)_CORELIST)
export ipc_$(SOC)_CORELIST

# IPC baremetal library
ipc_baremetal_COMP_LIST = ipc_baremetal
ipc_baremetal_RELPATH = ti/drv/ipc
ipc_baremetal_PATH = $(PDK_IPC_COMP_PATH)
ipc_baremetal_LIBNAME = ipc_baremetal
ipc_baremetal_LIBPATH = $(PDK_IPC_COMP_PATH)/lib
ipc_baremetal_OBJPATH = $(ipc_baremetal_RELPATH)/ipc_baremetal
export ipc_baremetal_OBJPATH
ipc_baremetal_MAKEFILE = -fsrc/makefile_baremetal
export ipc_baremetal_MAKEFILE
export ipc_baremetal_LIBNAME
export ipc_baremetal_LIBPATH
ipc_baremetal_BOARD_DEPENDENCY = no
ipc_baremetal_CORE_DEPENDENCY = yes
export ipc_baremetal_COMP_LIST
export ipc_baremetal_BOARD_DEPENDENCY
export ipc_baremetal_CORE_DEPENDENCY
ipc_baremetal_PKG_LIST = ipc_baremetal
ipc_baremetal_INCLUDE = $(ipc_baremetal_PATH)
ipc_baremetal_SOCLIST = $(drvipc_SOCLIST)
export ipc_baremetal_SOCLIST
ipc_baremetal_$(SOC)_CORELIST = $(drvipc_$(SOC)_CORELIST)
export ipc_baremetal_$(SOC)_CORELIST
ipc_LIB_LIST += ipc_baremetal

#
# IPC Examples
#

#############################################
# IPC Echo tests involving Linux
#############################################

# Test Configuration: Linux on A72, all other cores running FreeRTOS
define IPC_ECHO_TEST_RULE
export ipc_echo_test_$(1)_COMP_LIST = ipc_echo_test_$(1)
ipc_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/linux
ipc_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/linux
export ipc_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_echo_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_echo_test_$(1)_PKG_LIST = ipc_echo_test_$(1)
ipc_echo_test_$(1)_INCLUDE = $(ipc_echo_test_$(1)_PATH)
export ipc_echo_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_echo_test_$(1)_$(SOC)_CORELIST = $(filter-out mcu1_0, $(drvipc_$(SOC)_RTOS_CORELIST))
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_echo_test_$(1)
endif
endif

endef

IPC_ECHO_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_ECHO_TEST_RULE,$(curos)))
$(eval ${IPC_ECHO_TEST_MACRO_LIST})

# Test Configuration: Linux on A72, all other cores running FreeRTOS with all R5F cores having reset vectors in BTCM
define IPC_ECHO_TESTB_RULE
export ipc_echo_testb_$(1)_COMP_LIST = ipc_echo_testb_$(1)
ipc_echo_testb_$(1)_RELPATH = ti/drv/ipc/examples/linux/echo_test_btcm
ipc_echo_testb_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/linux/echo_test_btcm
export ipc_echo_testb_$(1)_BOARD_DEPENDENCY = yes
export ipc_echo_testb_$(1)_CORE_DEPENDENCY = yes
export ipc_echo_testb_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_echo_testb_$(1)_MAKEFILE = -fmakefile.btcm BUILD_OS_TYPE=$(1)
ipc_echo_testb_$(1)_PKG_LIST = ipc_echo_testb_$(1)
ipc_echo_testb_$(1)_INCLUDE = $(ipc_echo_testb_$(1)_PATH)
export ipc_echo_testb_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_echo_testb_$(1)_$(SOC)_CORELIST = mcu1_0
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_echo_testb_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_echo_testb_$(1)
endif
endif

endef

IPC_ECHO_TESTB_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_ECHO_TESTB_RULE,$(curos)))
$(eval ${IPC_ECHO_TESTB_MACRO_LIST})


#############################################
# IPC Echo tests for RTOS
#############################################

# Test Configuration: mcu1_0 and mcu2_0 cores running FreeRTOS
define IPC_RTOS_SANITY_TEST_RULE

export ipc_rtos_sanity_test_$(1)_COMP_LIST = ipc_rtos_sanity_test_$(1)
ipc_rtos_sanity_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_sanity_test
ipc_rtos_sanity_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_sanity_test
export ipc_rtos_sanity_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_sanity_test_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_sanity_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_sanity_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_rtos_sanity_test_$(1)_PKG_LIST = ipc_rtos_sanity_test_$(1)
ipc_rtos_sanity_test_$(1)_INCLUDE = $(ipc_rtos_sanity_test_$(1)_PATH)
export ipc_rtos_sanity_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_sanity_test_$(1)_$(SOC)_CORELIST = mcu1_0 mcu2_0
export ipc_rtos_sanity_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_rtos_sanity_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_rtos_sanity_test_$(1)
endif
endif
endef
IPC_RTOS_SANITY_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_RTOS_SANITY_TEST_RULE,$(curos)))
$(eval ${IPC_RTOS_SANITY_TEST_MACRO_LIST})

drvipc_endpt_test_boardlist = j784s4_evm
# Test Configuration: mcu1_0 and mcu2_0 cores running FreeRTOS
define IPC_INVALID_ENDPT_TEST_RULE

export ipc_invalid_endpt_test_$(1)_COMP_LIST = ipc_invalid_endpt_test_$(1)
ipc_invalid_endpt_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_invalid_endpt_test
ipc_invalid_endpt_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_invalid_endpt_test
export ipc_invalid_endpt_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_invalid_endpt_test_$(1)_CORE_DEPENDENCY = yes
export ipc_invalid_endpt_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_invalid_endpt_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_invalid_endpt_test_$(1)_PKG_LIST = ipc_invalid_endpt_test_$(1)
ipc_invalid_endpt_test_$(1)_INCLUDE = $(ipc_invalid_endpt_test_$(1)_PATH)
export ipc_invalid_endpt_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_endpt_test_boardlist))
export ipc_invalid_endpt_test_$(1)_$(SOC)_CORELIST = mcu1_0 mcu2_0
export ipc_invalid_endpt_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_invalid_endpt_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_invalid_endpt_test_$(1)
endif
endif
endef
IPC_INVALID_ENDPT_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_INVALID_ENDPT_TEST_RULE,$(curos)))
$(eval ${IPC_INVALID_ENDPT_TEST_MACRO_LIST})

# Test Configuration: mcu1_0 and c7x_1 cores running FreeRTOS
define IPC_C7X_SANITY_TEST_RULE

export ipc_c7x_sanity_test_$(1)_COMP_LIST = ipc_c7x_sanity_test_$(1)
ipc_c7x_sanity_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_c7x_sanity_test
ipc_c7x_sanity_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_c7x_sanity_test
export ipc_c7x_sanity_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_c7x_sanity_test_$(1)_CORE_DEPENDENCY = yes
export ipc_c7x_sanity_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_c7x_sanity_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_c7x_sanity_test_$(1)_PKG_LIST = ipc_c7x_sanity_test_$(1)
ipc_c7x_sanity_test_$(1)_INCLUDE = $(ipc_c7x_sanity_test_$(1)_PATH)
export ipc_c7x_sanity_test_$(1)_BOARDLIST = j784s4_evm j721s2_evm j721e_evm j742s2_evm
export ipc_c7x_sanity_test_$(1)_$(SOC)_CORELIST = mcu1_0 c7x_1
export ipc_c7x_sanity_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_c7x_sanity_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_c7x_sanity_test_$(1)
endif
endif
endef
IPC_C7X_SANITY_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_C7X_SANITY_TEST_RULE,$(curos)))
$(eval ${IPC_C7X_SANITY_TEST_MACRO_LIST})

# Test Configuration: FreeRTOS on mcu1_0 and baremetal on mcu2_0
ipc_baremetal_sanity_test_COMP_LIST = ipc_baremetal_sanity_test
ipc_baremetal_sanity_test_RELPATH = ti/drv/ipc/examples/baremetal/ipc_baremetal_sanity_test
ipc_baremetal_sanity_test_PATH = $(PDK_IPC_COMP_PATH)/examples/baremetal/ipc_baremetal_sanity_test
ipc_baremetal_sanity_test_BOARD_DEPENDENCY = yes
ipc_baremetal_sanity_test_CORE_DEPENDENCY = yes
ipc_baremetal_sanity_test_XDC_CONFIGURO = no
export ipc_baremetal_sanity_test_COMP_LIST
export ipc_baremetal_sanity_test_BOARD_DEPENDENCY
export ipc_baremetal_sanity_test_CORE_DEPENDENCY
export ipc_baremetal_sanity_test_XDC_CONFIGURO
ipc_baremetal_sanity_test_PKG_LIST = ipc_baremetal_sanity_test
ipc_baremetal_sanity_test_INCLUDE = $(ipc_baremetal_sanity_test_PATH)
ipc_baremetal_sanity_test_BOARDLIST = $(drvipc_BOARDLIST)
export ipc_baremetal_sanity_test_BOARDLIST
ipc_baremetal_sanity_test_$(SOC)_CORELIST = mcu2_0
export ipc_baremetal_sanity_test_$(SOC)_CORELIST
ipc_EXAMPLE_LIST += ipc_baremetal_sanity_test
export ipc_baremetal_sanity_test_SBL_APPIMAGEGEN = yes

# Test Configuration: A72/A53 core not included, all cores running FreeRTOS
define IPC_RTOS_ECHO_TEST_RULE

export ipc_rtos_echo_test_$(1)_COMP_LIST = ipc_rtos_echo_test_$(1)
ipc_rtos_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_echo_test
ipc_rtos_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test
export ipc_rtos_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_echo_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_rtos_echo_test_$(1)_PKG_LIST = ipc_rtos_echo_test_$(1)
ipc_rtos_echo_test_$(1)_INCLUDE = $(ipc_rtos_echo_test_$(1)_PATH)
export ipc_rtos_echo_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_echo_test_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))
export ipc_rtos_echo_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_rtos_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_rtos_echo_test_$(1)
endif
endif
endef
IPC_RTOS_ECHO_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_RTOS_ECHO_TEST_RULE,$(curos)))
$(eval ${IPC_RTOS_ECHO_TEST_MACRO_LIST})

# Test Configuration: A72/A53 core not included, all cores running FreeRTOS with all R5F cores having reset vectors in BTCM
define IPC_RTOS_ECHO_TESTB_RULE
export ipc_rtos_echo_testb_$(1)_COMP_LIST = ipc_rtos_echo_testb_$(1)
ipc_rtos_echo_testb_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_echo_test/ipc_rtos_echo_testb
ipc_rtos_echo_testb_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test/ipc_rtos_echo_testb
export ipc_rtos_echo_testb_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_echo_testb_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_echo_testb_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_echo_testb_$(1)_MAKEFILE =  -fmakefile.btcm BUILD_OS_TYPE=$(1)
ipc_rtos_echo_testb_$(1)_PKG_LIST = ipc_rtos_echo_testb_$(1)
ipc_rtos_echo_testb_$(1)_INCLUDE = $(ipc_rtos_echo_testb_$(1)_PATH)
export ipc_rtos_echo_testb_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_echo_testb_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))
export ipc_rtos_echo_testb_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_rtos_echo_testb_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_rtos_echo_testb_$(1)
endif
endif
endef
IPC_RTOS_ECHO_TESTB_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_RTOS_ECHO_TESTB_RULE,$(curos)))
$(eval ${IPC_RTOS_ECHO_TESTB_MACRO_LIST})

# Test Configuration: Graceful shutdown test on MCU1_0, MCU2_0 and MCU3_0. MCU2_0 sends shutdown to mcu3_0 and mcu3_0 sends ack back
define IPC_GRACEFUL_SHUTDOWN_TEST_RULE
export ipc_graceful_shutdown_test_$(1)_COMP_LIST = ipc_graceful_shutdown_test_$(1)
ipc_graceful_shutdown_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_graceful_shutdown_test
ipc_graceful_shutdown_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_graceful_shutdown_test
export ipc_graceful_shutdown_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_graceful_shutdown_test_$(1)_CORE_DEPENDENCY = yes
export ipc_graceful_shutdown_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_graceful_shutdown_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_graceful_shutdown_test_$(1)_PKG_LIST = ipc_graceful_shutdown_test_$(1)
ipc_graceful_shutdown_test_$(1)_INCLUDE = $(ipc_graceful_shutdown_test_$(1)_PATH)
export ipc_graceful_shutdown_test_$(1)_BOARDLIST = j784s4_evm j721s2_evm j721e_evm j7200_evm j742s2_evm
export ipc_graceful_shutdown_test_$(1)_$(SOC)_CORELIST = mcu1_0 mcu2_0 mcu2_1
export ipc_graceful_shutdown_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_graceful_shutdown_test_$(1)
endif
endef
IPC_GRACEFUL_SHUTDOWN_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_GRACEFUL_SHUTDOWN_TEST_RULE,$(curos)))
$(eval ${IPC_GRACEFUL_SHUTDOWN_TEST_MACRO_LIST})

# Test Configuration: A72/A53 core not included, baremetal on R5 cores other than mcu1_0, FreeRTOS on mcu1_0 and DSP cores(C66x,C7x)
ipc_baremetal_echo_test_COMP_LIST = ipc_baremetal_echo_test
ipc_baremetal_echo_test_RELPATH = ti/drv/ipc/examples/baremetal
ipc_baremetal_echo_test_PATH = $(PDK_IPC_COMP_PATH)/examples/baremetal
ipc_baremetal_echo_test_BOARD_DEPENDENCY = yes
ipc_baremetal_echo_test_CORE_DEPENDENCY = yes
ipc_baremetal_echo_test_XDC_CONFIGURO = no
export ipc_baremetal_echo_test_COMP_LIST
export ipc_baremetal_echo_test_BOARD_DEPENDENCY
export ipc_baremetal_echo_test_CORE_DEPENDENCY
export ipc_baremetal_echo_test_XDC_CONFIGURO
ipc_baremetal_echo_test_PKG_LIST = ipc_baremetal_echo_test
ipc_baremetal_echo_test_INCLUDE = $(ipc_baremetal_echo_test_PATH)
ipc_baremetal_echo_test_BOARDLIST = $(drvipc_BOARDLIST)
export ipc_baremetal_echo_test_BOARDLIST
ipc_baremetal_echo_test_$(SOC)_CORELIST = $(drvipc_$(SOC)_BAREMETAL_CORELIST)
export ipc_baremetal_echo_test_$(SOC)_CORELIST
ipc_EXAMPLE_LIST += ipc_baremetal_echo_test
export ipc_baremetal_echo_test_SBL_APPIMAGEGEN = yes

# Test Configuration: Set of negative tests to validate IPC APIs, run indpendently on each core
define IPC_NEGATIVE_TEST_RULE
export ipc_negative_test_$(1)_COMP_LIST = ipc_negative_test_$(1)
ipc_negative_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_negative_test
ipc_negative_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_negative_test
export ipc_negative_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_negative_test_$(1)_CORE_DEPENDENCY = yes
export ipc_negative_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_negative_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_negative_test_$(1)_PKG_LIST = ipc_negative_test_$(1)
ipc_negative_test_$(1)_INCLUDE = $(ipc_negative_test_$(1)_PATH)
export ipc_negative_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_negative_test_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))
export ipc_negative_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_negative_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_negative_test_$(1)
endif
endif
endef
IPC_NEGATIVE_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_NEGATIVE_TEST_RULE,$(curos)))
$(eval ${IPC_NEGATIVE_TEST_MACRO_LIST})

# Test Configuration: Set of tests to measure IPC communication performance
define IPC_PERF_TEST_RULE
export ipc_perf_test_$(1)_COMP_LIST = ipc_perf_test_$(1)
ipc_perf_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_perf_test
ipc_perf_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_perf_test
export ipc_perf_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_perf_test_$(1)_CORE_DEPENDENCY = yes
export ipc_perf_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_perf_test_$(1)_MAKEFILE =  -fmakefile BUILD_OS_TYPE=$(1)
ipc_perf_test_$(1)_PKG_LIST = ipc_perf_test_$(1)
ipc_perf_test_$(1)_INCLUDE = $(ipc_perf_test_$(1)_PATH)
export ipc_perf_test_$(1)_BOARDLIST = j721e_sim j721e_qt j721e_evm j7200_evm j721s2_evm j784s4_evm
export ipc_perf_test_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))
export ipc_perf_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_perf_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_perf_test_$(1)
endif
endif
endef
IPC_PERF_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_PERF_TEST_RULE,$(curos)))
$(eval ${IPC_PERF_TEST_MACRO_LIST})

#Test Configuration: IPC extended test for IPC API coverage; runs on mcu1_0
define IPC_EXTENDED_TEST_RULE
export ipc_extended_test_$(1)_COMP_LIST = ipc_extended_test_$(1)
ipc_extended_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_extended_test
ipc_extended_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_extended_test
export ipc_extended_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_extended_test_$(1)_CORE_DEPENDENCY = yes
export ipc_extended_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_extended_test_$(1)_MAKEFILE =  -fmakefile BUILD_OS_TYPE=$(1)
ipc_extended_test_$(1)_PKG_LIST = ipc_extended_test_$(1)
ipc_extended_test_$(1)_INCLUDE = $(ipc_extended_test_$(1)_PATH)
export ipc_extended_test_$(1)_BOARDLIST = j784s4_evm j721s2_evm j721e_evm j7200_evm j742s2_evm
ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
export ipc_extended_test_$(1)_$(SOC)_CORELIST = mcu1_0 c7x_1
else
ifeq ($(SOC),$(filter $(SOC), j7200))
export ipc_extended_test_$(1)_$(SOC)_CORELIST = mcu1_0
endif
endif
export ipc_extended_test_$(1)_SBL_APPIMAGEGEN = yes
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_extended_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_extended_test_$(1)
endif
endif
endef
IPC_EXTENDED_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_EXTENDED_TEST_RULE,$(curos)))
$(eval ${IPC_EXTENDED_TEST_MACRO_LIST})

#############################################
# IPC Echo tests involving QNX
#############################################

# Test Configuration: QNX on A72, all other cores running FreeRTOS
define IPC_QNX_ECHO_TEST_RULE
export ipc_qnx_echo_test_$(1)_COMP_LIST = ipc_qnx_echo_test_$(1)
ipc_qnx_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/qnx
ipc_qnx_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/qnx
export ipc_qnx_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_qnx_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_qnx_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_qnx_echo_test_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
ipc_qnx_echo_test_$(1)_PKG_LIST = ipc_qnx_echo_test_$(1)
ipc_qnx_echo_test_$(1)_INCLUDE = $(ipc_qnx_echo_test_$(1)_PATH)
export ipc_qnx_echo_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_qnx_echo_test_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))

ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_qnx_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_qnx_echo_test_$(1)
endif
endif

endef

IPC_QNX_ECHO_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_QNX_ECHO_TEST_RULE,$(curos)))
$(eval ${IPC_QNX_ECHO_TEST_MACRO_LIST})

# Test Configuration: QNX on A72, all other cores running FreeRTOS with all R5F cores having reset vectors in BTCM
define IPC_QNX_ECHO_TESTB_RULE
export ipc_qnx_echo_testb_$(1)_COMP_LIST = ipc_qnx_echo_testb_$(1)
ipc_qnx_echo_testb_$(1)_RELPATH = ti/drv/ipc/examples/qnx/echo_test_btcm
ipc_qnx_echo_testb_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/qnx/echo_test_btcm
export ipc_qnx_echo_testb_$(1)_BOARD_DEPENDENCY = yes
export ipc_qnx_echo_testb_$(1)_CORE_DEPENDENCY = yes
export ipc_qnx_echo_testb_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_qnx_echo_testb_$(1)_MAKEFILE = -fmakefile.btcm BUILD_OS_TYPE=$(1)
ipc_qnx_echo_testb_$(1)_PKG_LIST = ipc_qnx_echo_testb_$(1)
ipc_qnx_echo_testb_$(1)_INCLUDE = $(ipc_qnx_echo_testb_$(1)_PATH)
export ipc_qnx_echo_testb_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_qnx_echo_testb_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(drvipc_$(SOC)_RTOS_CORELIST))
ifneq ($(1),$(filter $(1), safertos))
ipc_EXAMPLE_LIST += ipc_qnx_echo_testb_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_EXAMPLE_LIST += ipc_qnx_echo_testb_$(1)
endif
endif

endef

IPC_QNX_ECHO_TESTB_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_QNX_ECHO_TESTB_RULE,$(curos)))
$(eval ${IPC_QNX_ECHO_TESTB_MACRO_LIST})


##########################################################
# IPC Mutlicore tests: Multi core images for IPC tests
##########################################################

# Test Configuration: Dual core echo test for RTOS, uses ipc_rtos_sanity_test
define IPC_RTOS_DUALCORE_ECHO_TEST_RULE

export ipc_rtos_dualcore_echo_test_$(1)_COMP_LIST = ipc_rtos_dualcore_echo_test_$(1)
ipc_rtos_dualcore_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_sanity_test
ipc_rtos_dualcore_echo_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_rtos_echo_test_$(1)/bin
ipc_rtos_dualcore_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_sanity_test
export ipc_rtos_dualcore_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_dualcore_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_dualcore_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_dualcore_echo_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_sanity_test/ipc_rtos_dualcore_echo_test.mk BUILD_OS_TYPE=$(1)
export ipc_rtos_dualcore_echo_test_$(1)_DEPENDS_ON=ipc_rtos_sanity_test_$(1)
ipc_rtos_dualcore_echo_test_$(1)_PKG_LIST = ipc_rtos_dualcore_echo_test_$(1)
ipc_rtos_dualcore_echo_test_$(1)_INCLUDE = $(ipc_rtos_dualcore_echo_test_$(1)_PATH)
export ipc_rtos_dualcore_echo_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_dualcore_echo_test_$(1)_$(SOC)_CORELIST := mcu2_0
export ipc_rtos_dualcore_echo_test_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_rtos_dualcore_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_rtos_dualcore_echo_test_$(1)
endif
endif
endef
ipc_rtos_dualcore_echo_test_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_RTOS_DUALCORE_ECHO_TEST_RULE,$(curos)))
$(eval ${ipc_rtos_dualcore_echo_test_MACRO_LIST})

# Test Configuration: Dual core echo test for RTOS, uses ipc_c7x_sanity_test
define IPC_C7X_DUALCORE_ECHO_TEST_RULE

export ipc_c7x_dualcore_echo_test_$(1)_COMP_LIST = ipc_c7x_dualcore_echo_test_$(1)
ipc_c7x_dualcore_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_c7x_sanity_test
ipc_c7x_dualcore_echo_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_rtos_echo_test_$(1)/bin
ipc_c7x_dualcore_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_c7x_sanity_test
export ipc_c7x_dualcore_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_c7x_dualcore_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_c7x_dualcore_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_c7x_dualcore_echo_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_c7x_sanity_test/ipc_c7x_dualcore_echo_test.mk BUILD_OS_TYPE=$(1)
export ipc_c7x_dualcore_echo_test_$(1)_DEPENDS_ON=ipc_c7x_sanity_test_$(1)
ipc_c7x_dualcore_echo_test_$(1)_PKG_LIST = ipc_c7x_dualcore_echo_test_$(1)
ipc_c7x_dualcore_echo_test_$(1)_INCLUDE = $(ipc_c7x_dualcore_echo_test_$(1)_PATH)
export ipc_c7x_dualcore_echo_test_$(1)_BOARDLIST = j784s4_evm j721s2_evm j721e_evm j742s2_evm
export ipc_c7x_dualcore_echo_test_$(1)_$(SOC)_CORELIST := c7x_1
export ipc_c7x_dualcore_echo_test_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_c7x_dualcore_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_c7x_dualcore_echo_test_$(1)
endif
endif
endef
ipc_c7x_dualcore_echo_test_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_C7X_DUALCORE_ECHO_TEST_RULE,$(curos)))
$(eval ${ipc_c7x_dualcore_echo_test_MACRO_LIST})

define IPC_INVALID_ENDPT_DUALCORE_TEST_RULE

export ipc_invalid_endpt_dualcore_test_$(1)_COMP_LIST = ipc_invalid_endpt_dualcore_test_$(1)
ipc_invalid_endpt_dualcore_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_invalid_endpt_test
ipc_invalid_endpt_dualcore_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_invalid_endpt_test_$(1)/bin
ipc_invalid_endpt_dualcore_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_invalid_endpt_test
export ipc_invalid_endpt_dualcore_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_invalid_endpt_dualcore_test_$(1)_CORE_DEPENDENCY = yes
export ipc_invalid_endpt_dualcore_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_invalid_endpt_dualcore_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_invalid_endpt_test/ipc_invalid_endpt_dualcore_test.mk BUILD_OS_TYPE=$(1)
export ipc_invalid_endpt_dualcore_test_$(1)_DEPENDS_ON=ipc_invalid_endpt_test_$(1)
ipc_invalid_endpt_dualcore_test_$(1)_PKG_LIST = ipc_invalid_endpt_dualcore_test_$(1)
ipc_invalid_endpt_dualcore_test_$(1)_INCLUDE = $(ipc_invalid_endpt_dualcore_test_$(1)_PATH)
export ipc_invalid_endpt_dualcore_test_$(1)_BOARDLIST = j784s4_evm 
export ipc_invalid_endpt_dualcore_test_$(1)_$(SOC)_CORELIST := mcu2_0
export ipc_invalid_endpt_dualcore_test_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_invalid_endpt_dualcore_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_invalid_endpt_dualcore_test_$(1)
endif
endif
endef
ipc_invalid_endpt_dualcore_test_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_INVALID_ENDPT_DUALCORE_TEST_RULE,$(curos)))
$(eval ${ipc_invalid_endpt_dualcore_test_MACRO_LIST})

# Test Configuration: Dual core echo test for baremetal, uses ipc_baremetal_sanity_test
export ipc_baremetal_dualcore_echo_test_COMP_LIST = ipc_baremetal_dualcore_echo_test
ipc_baremetal_dualcore_echo_test_RELPATH = ti/drv/ipc/examples/baremetal/ipc_baremetal_sanity_test
ipc_baremetal_dualcore_echo_test_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_baremetal_echo_test/bin
ipc_baremetal_dualcore_echo_test_PATH = $(PDK_IPC_COMP_PATH)/examples/baremetal/ipc_baremetal_sanity_test
export ipc_baremetal_dualcore_echo_test_BOARD_DEPENDENCY = yes
export ipc_baremetal_dualcore_echo_test_CORE_DEPENDENCY = yes
export ipc_baremetal_dualcore_echo_test_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_baremetal_dualcore_echo_test_MAKEFILE = -f$(PDK_IPC_COMP_PATH)/examples/baremetal/ipc_baremetal_sanity_test/ipc_baremetal_dualcore_echo_test.mk
export ipc_baremetal_dualcore_echo_test_DEPENDS_ON = ipc_baremetal_sanity_test ipc_rtos_sanity_test_freertos
ipc_baremetal_dualcore_echo_test_PKG_LIST = ipc_baremetal_dualcore_echo_test
ipc_baremetal_dualcore_echo_test_INCLUDE = $(ipc_baremetal_dualcore_echo_test_PATH)
export ipc_baremetal_dualcore_echo_test_BOARDLIST = $(drvipc_BOARDLIST)
export ipc_baremetal_dualcore_echo_test_$(SOC)_CORELIST:= mcu2_0
export ipc_baremetal_dualcore_echo_test_SBL_APPIMAGEGEN = no
ipc_DUP_EXAMPLE_LIST += ipc_baremetal_dualcore_echo_test

# Test Configuration: Multicore echo test for RTOS, uses ipc_rtos_echo_test
define IPC_RTOS_MULTICORE_ECHO_TEST_RULE

export ipc_rtos_multicore_echo_test_$(1)_COMP_LIST = ipc_rtos_multicore_echo_test_$(1)
ipc_rtos_multicore_echo_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_echo_test
ipc_rtos_multicore_echo_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_rtos_echo_test_$(1)/bin
ipc_rtos_multicore_echo_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test
export ipc_rtos_multicore_echo_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_multicore_echo_test_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_multicore_echo_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_multicore_echo_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test/ipc_rtos_multicore_echo_test.mk BUILD_OS_TYPE=$(1)
export ipc_rtos_multicore_echo_test_$(1)_DEPENDS_ON=ipc_rtos_echo_test_$(1)
ipc_rtos_multicore_echo_test_$(1)_PKG_LIST = ipc_rtos_multicore_echo_test_$(1)
ipc_rtos_multicore_echo_test_$(1)_INCLUDE = $(ipc_rtos_multicore_echo_test_$(1)_PATH)
export ipc_rtos_multicore_echo_test_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_multicore_echo_test_$(1)_$(SOC)_CORELIST := $(drvipc_$(SOC)_LASTCORE)
export ipc_rtos_multicore_echo_test_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_rtos_multicore_echo_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_rtos_multicore_echo_test_$(1)
endif
endif
endef
IPC_RTOS_MULTICORE_ECHO_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST) safertos, $(call IPC_RTOS_MULTICORE_ECHO_TEST_RULE,$(curos)))
$(eval ${IPC_RTOS_MULTICORE_ECHO_TEST_MACRO_LIST})

# Test Configuration: Multicore echo test for baremetal, uses ipc_baremetal_echo_test
export ipc_baremetal_multicore_echo_test_COMP_LIST = ipc_baremetal_multicore_echo_test
ipc_baremetal_multicore_echo_test_RELPATH = ti/drv/ipc/examples/baremetal
ipc_baremetal_multicore_echo_test_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_baremetal_echo_test/bin
ipc_baremetal_multicore_echo_test_PATH = $(PDK_IPC_COMP_PATH)/examples/baremetal
export ipc_baremetal_multicore_echo_test_BOARD_DEPENDENCY = yes
export ipc_baremetal_multicore_echo_test_CORE_DEPENDENCY = yes
export ipc_baremetal_multicore_echo_test_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_baremetal_multicore_echo_test_MAKEFILE = -f$(PDK_IPC_COMP_PATH)/examples/baremetal/ipc_baremetal_multicore_echo_test.mk
export ipc_baremetal_multicore_echo_test_DEPENDS_ON = ipc_baremetal_echo_test ipc_rtos_echo_test_freertos
ipc_baremetal_multicore_echo_test_PKG_LIST = ipc_baremetal_multicore_echo_test
ipc_baremetal_multicore_echo_test_INCLUDE = $(ipc_baremetal_multicore_echo_test_PATH)
export ipc_baremetal_multicore_echo_test_BOARDLIST = $(drvipc_BOARDLIST)
export ipc_baremetal_multicore_echo_test_$(SOC)_CORELIST:= $(drvipc_$(SOC)_LASTCORE)
export ipc_baremetal_multicore_echo_test_SBL_APPIMAGEGEN = no
ipc_DUP_EXAMPLE_LIST += ipc_baremetal_multicore_echo_test

# Test Configuration: Multicore echo test for echo_testb, uses ipc_rtos_echo_testb
define IPC_RTOS_MULTICORE_ECHO_TESTB_RULE
export ipc_rtos_multicore_echo_testb_$(1)_COMP_LIST = ipc_rtos_multicore_echo_testb_$(1)
ipc_rtos_multicore_echo_testb_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_rtos_echo_test/ipc_rtos_echo_testb
ipc_rtos_multicore_echo_testb_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_rtos_echo_testb_$(1)/bin
ipc_rtos_multicore_echo_testb_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test/ipc_rtos_echo_testb
export ipc_rtos_multicore_echo_testb_$(1)_BOARD_DEPENDENCY = yes
export ipc_rtos_multicore_echo_testb_$(1)_CORE_DEPENDENCY = yes
export ipc_rtos_multicore_echo_testb_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_rtos_multicore_echo_testb_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_rtos_echo_test/ipc_rtos_echo_testb/ipc_rtos_multicore_echo_testb.mk BUILD_OS_TYPE=$(1)
export ipc_rtos_multicore_echo_testb_$(1)_DEPENDS_ON=ipc_rtos_echo_testb_$(1)
ipc_rtos_multicore_echo_testb_$(1)_PKG_LIST = ipc_rtos_multicore_echo_testb_$(1)
ipc_rtos_multicore_echo_testb_$(1)_INCLUDE = $(ipc_rtos_multicore_echo_testb_$(1)_PATH)
export ipc_rtos_multicore_echo_testb_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(drvipc_BOARDLIST))
export ipc_rtos_multicore_echo_testb_$(1)_$(SOC)_CORELIST := $(drvipc_$(SOC)_LASTCORE)
export ipc_rtos_multicore_echo_testb_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_rtos_multicore_echo_testb_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_rtos_multicore_echo_testb_$(1)
endif
endif
endef
IPC_RTOS_MULTICORE_ECHO_TESTB_MACRO_LIST := $(foreach curos, $(filter-out tirtos, $(drvipc_RTOS_LIST)), $(call IPC_RTOS_MULTICORE_ECHO_TESTB_RULE,$(curos)))
$(eval ${IPC_RTOS_MULTICORE_ECHO_TESTB_MACRO_LIST})

# Test Configuration: Multicore IPC performance test, uses ipc_perf_test
define IPC_MULTICORE_PERF_TEST_RULE
export ipc_multicore_perf_test_$(1)_COMP_LIST = ipc_multicore_perf_test_$(1)
ipc_multicore_perf_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_perf_test
ipc_multicore_perf_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_perf_test_$(1)/bin
ipc_multicore_perf_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_perf_test
export ipc_multicore_perf_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_multicore_perf_test_$(1)_CORE_DEPENDENCY = yes
export ipc_multicore_perf_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_multicore_perf_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_perf_test/ipc_multicore_perf_test.mk BUILD_OS_TYPE=$(1)
export ipc_multicore_perf_test_$(1)_DEPENDS_ON=ipc_perf_test_$(1)
ipc_multicore_perf_test_$(1)_PKG_LIST = ipc_multicore_perf_test_$(1)
ipc_multicore_perf_test_$(1)_INCLUDE = $(ipc_multicore_perf_test_$(1)_PATH)
export ipc_multicore_perf_test_$(1)_BOARDLIST = j721e_sim j721e_qt j721e_evm j7200_evm j721s2_evm j784s4_evm
export ipc_multicore_perf_test_$(1)_$(SOC)_CORELIST := $(drvipc_$(SOC)_LASTCORE)
export ipc_multicore_perf_test_SBL_APPIMAGEGEN = no
ifneq ($(SOC),$(filter $(SOC), j721s2))
#Temp disable multicore image gen for J721S2 since SBL_CORE_ID_c7x_2 is not yet supported
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_multicore_perf_test_$(1)
else
ifneq ($(wildcard $(SAFERTOS_KERNEL_INSTALL_PATH)),)
ipc_DUP_EXAMPLE_LIST += ipc_multicore_perf_test_$(1)
endif
endif
endif
endef
IPC_MULTICORE_PERF_TEST_MACRO_LIST := $(foreach curos, $(filter-out tirtos, $(drvipc_RTOS_LIST)), $(call IPC_MULTICORE_PERF_TEST_RULE,$(curos)))
$(eval ${IPC_MULTICORE_PERF_TEST_MACRO_LIST})
 
# Test Configuration: Multicore IPC graceful shutdown test, uses ipc_graceful_shutdown_test
define IPC_GRACEFUL_SHUTDOWN_MULTICORE_TEST_RULE

export ipc_graceful_shutdown_multicore_test_$(1)_COMP_LIST = ipc_graceful_shutdown_multicore_test_$(1)
ipc_graceful_shutdown_multicore_test_$(1)_RELPATH = ti/drv/ipc/examples/rtos/ipc_graceful_shutdown_test
ipc_graceful_shutdown_multicore_test_$(1)_BINPATH = $(PDK_INSTALL_PATH)/ti/binary/ipc_graceful_shutdown_test_$(1)/bin
ipc_graceful_shutdown_multicore_test_$(1)_PATH = $(PDK_IPC_COMP_PATH)/examples/rtos/ipc_graceful_shutdown_test
export ipc_graceful_shutdown_multicore_test_$(1)_BOARD_DEPENDENCY = yes
export ipc_graceful_shutdown_multicore_test_$(1)_CORE_DEPENDENCY = yes
export ipc_graceful_shutdown_multicore_test_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export ipc_graceful_shutdown_multicore_test_$(1)_MAKEFILE =  -f$(PDK_IPC_COMP_PATH)/examples/rtos/ipc_graceful_shutdown_test/ipc_graceful_shutdown_multicore_test.mk BUILD_OS_TYPE=$(1)
export ipc_graceful_shutdown_multicore_test_$(1)_DEPENDS_ON=ipc_graceful_shutdown_test_$(1)
ipc_graceful_shutdown_multicore_test_$(1)_PKG_LIST = ipc_graceful_shutdown_multicore_test_$(1)
ipc_graceful_shutdown_multicore_test_$(1)_INCLUDE = $(ipc_graceful_shutdown_multicore_test_$(1)_PATH)
export ipc_graceful_shutdown_multicore_test_$(1)_BOARDLIST = j784s4_evm j721s2_evm j721e_evm j7200_evm j742s2_evm
export ipc_graceful_shutdown_multicore_test_$(1)_$(SOC)_CORELIST := mcu2_1
export ipc_graceful_shutdown_multicore_test_SBL_APPIMAGEGEN = no
ifneq ($(1),$(filter $(1), safertos))
ipc_DUP_EXAMPLE_LIST += ipc_graceful_shutdown_multicore_test_$(1)
endif
endef
IPC_GRACEFUL_SHUTDOWN_MULTICORE_TEST_MACRO_LIST := $(foreach curos, $(drvipc_RTOS_LIST), $(call IPC_GRACEFUL_SHUTDOWN_MULTICORE_TEST_RULE,$(curos)))
$(eval ${IPC_GRACEFUL_SHUTDOWN_MULTICORE_TEST_MACRO_LIST})


export ipc_LIB_LIST
export ipc_EXAMPLE_LIST
export ipc_DUP_EXAMPLE_LIST
export drvipc_LIB_LIST = $(ipc_LIB_LIST)
export drvipc_EXAMPLE_LIST  = $(ipc_EXAMPLE_LIST)
export drvipc_DUP_EXAMPLE_LIST  = $(ipc_DUP_EXAMPLE_LIST)

IPC_CFLAGS =

# Enable asserts and prints
IPC_CFLAGS += -DIPC_CFG_ASSERT_ENABLE
#IPC_CFLAGS += -DIPC_CFG_USE_STD_ASSERT

export IPC_CFLAGS

ipc_component_make_include := 1
endif
