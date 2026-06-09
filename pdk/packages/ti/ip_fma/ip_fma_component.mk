# File: ip_fma_component.mk
#       This file is component include make file of ip fma .
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

############################
# ip fma package
# List of components included under ip fma lib
# The components included here are built and will be part of ip fma lib
############################
ip_fma_LIB_LIST =

############################
# ip fma examples
# List of examples under ip fma (+= is used at each example definition)
# All the tests mentioned in list are built when test target is called
# List below all examples for allowed values
############################
ip_fma_EXAMPLE_LIST =


# IP FMA LIBRARY

ip_fma_SOCLIST         = j721s2 j784s4
ip_fma_BOARDLIST       = j721s2_evm j784s4_evm
ip_fma_j784s4_CORELIST = mcu1_0 mcu1_1 mcu2_0 mcu2_1
ip_fma_j721s2_CORELIST = mcu1_0 mcu1_1 mcu2_0 mcu2_1

ip_fma_COMP_LIST = ip_fma
ip_fma_RELPATH   = ti/ip_fma
ip_fma_PATH      = $(IP_FMA_COMP_PATH)
ip_fma_LIBNAME   = ip_fma
ip_fma_LIBPATH   = $(IP_FMA_COMP_PATH)/lib
ip_fma_MAKEFILE  = -fmakefile
export ip_fma_MAKEFILE
export ip_fma_LIBNAME
export ip_fma_LIBPATH
ip_fma_SOC_DEPENDENCY = yes
ip_fma_CORE_DEPENDENCY = yes
export ip_fma_COMP_LIST
export ip_fma_BOARD_DEPENDENCY
export ip_fma_CORE_DEPENDENCY
ip_fma_PKG_LIST = ip_fma
ip_fma_INCLUDE  = $(ip_fma_PATH)/inc
export ip_fma_SOCLIST
export ip_fma_$(SOC)_CORELIST
ip_fma_LIB_LIST += ip_fma

export ip_fma_LIB_LIST


# RL7 IP FMA APP
ip_fma_rl7_SOCLIST           = j784s4 j721s2
ip_fma_rl7_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_rl7_CORELIST 	     = mcu1_0

define RL7_TESTAPP_RULE

rl7_ip_fma_app_$(1)_COMP_LIST = rl7_ip_fma_app_$(1)
rl7_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/rl7_ip_fma_app
rl7_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/rl7_ip_fma_app
rl7_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export rl7_ip_fma_app_$(1)_MAKEFILE
rl7_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
rl7_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export rl7_ip_fma_app_$(1)_COMP_LIST
export rl7_ip_fma_app_$(1)_SOC_DEPENDENCY
export rl7_ip_fma_app_$(1)_CORE_DEPENDENCY
rl7_ip_fma_app_$(1)_PKG_LIST = rl7_ip_fma_app_$(1)
rl7_ip_fma_app_$(1)_INCLUDE  = $(rl7_ip_fma_app_$(1)_PATH)
rl7_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_rl7_BOARDLIST)
export rl7_ip_fma_app_$(1)_BOARDLIST
rl7_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_rl7_CORELIST)
export rl7_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += rl7_ip_fma_app_$(1)
rl7_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export rl7_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

RL7_MACRO_LIST := $(foreach curos, baremetal, $(call RL7_TESTAPP_RULE,$(curos)))

$(eval ${RL7_MACRO_LIST})


# C7X MMA SAFETY EXAMPLE APP

C7X_RTOS_LIST       = $(DEFAULT_RTOS_LIST)

c7x_mma_SOCLIST   = j721e j721s2 j784s4 j742s2
c7x_mma_BOARDLIST = j721e_evm j721s2_evm j784s4_evm j742s2_evm
c7x_mma_CORELIST  = c7x_1

define C7X_MMA_TESTAPP_RULE
export c7x_mma_error_injection_testapp_$(1)_COMP_LIST = c7x_mma_error_injection_testapp_$(1)
c7x_mma_error_injection_testapp_$(1)_RELPATH = ti/ip_fma/examples/c7x_mma_error_injection
c7x_mma_error_injection_testapp_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/c7x_mma_error_injection
export c7x_mma_error_injection_testapp_$(1)_BOARD_DEPENDENCY = yes
export c7x_mma_error_injection_testapp_$(1)_CORE_DEPENDENCY = yes
export c7x_mma_error_injection_testapp_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export c7x_mma_error_injection_testapp_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
c7x_mma_error_injection_testapp_$(1)_PKG_LIST = c7x_mma_error_injection_testapp_$(1)
c7x_mma_error_injection_testapp_$(1)_INCLUDE = $(c7x_mma_error_injection_testapp_$(1)_PATH)
export c7x_mma_error_injection_testapp_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(c7x_mma_BOARDLIST))
export c7x_mma_error_injection_testapp_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(c7x_mma_CORELIST))
export c7x_mma_error_injection_testapp_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += c7x_mma_error_injection_testapp_$(1)
endef

C7X_MMA_TESTAPP_MACRO_LIST := $(foreach curos, $(C7X_RTOS_LIST), $(call C7X_MMA_TESTAPP_RULE,$(curos)))

$(eval ${C7X_MMA_TESTAPP_MACRO_LIST})

# C7X CPU REGISTER READBACK EXAMPLE APP

C7X_CPU_RTOS_LIST = $(DEFAULT_RTOS_LIST)

c7x_cpu_register_SOCLIST   = j721e j721s2 j784s4 j742s2
c7x_cpu_register_BOARDLIST = j721e_evm j721s2_evm j784s4_evm j742s2_evm
c7x_cpu_register_CORELIST  = c7x_1

define C7X_CPU_REGISTER_READBACK_TESTAPP_RULE
export c7x_cpu_register_readback_testapp_$(1)_COMP_LIST = c7x_cpu_register_readback_testapp_$(1)
c7x_cpu_register_readback_testapp_$(1)_RELPATH = ti/ip_fma/examples/c7x_cpu_register_readback
c7x_cpu_register_readback_testapp_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/c7x_cpu_register_readback
export c7x_cpu_register_readback_testapp_$(1)_BOARD_DEPENDENCY = yes
export c7x_cpu_register_readback_testapp_$(1)_CORE_DEPENDENCY = yes
export c7x_cpu_register_readback_testapp_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export c7x_cpu_register_readback_testapp_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
c7x_cpu_register_readback_testapp_$(1)_PKG_LIST = c7x_cpu_register_readback_testapp_$(1)
c7x_cpu_register_readback_testapp_$(1)_INCLUDE = $(c7x_cpu_register_readback_testapp_$(1)_PATH)
export c7x_cpu_register_readback_testapp_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(c7x_cpu_register_BOARDLIST))
export c7x_cpu_register_readback_testapp_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(c7x_cpu_register_CORELIST))
export c7x_cpu_register_readback_testapp_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += c7x_cpu_register_readback_testapp_$(1)
endef

C7X_CPU_REGISTER_READBACK_TESTAPP_MACRO_LIST := $(foreach curos, $(C7X_CPU_RTOS_LIST), $(call C7X_CPU_REGISTER_READBACK_TESTAPP_RULE,$(curos)))

$(eval ${C7X_CPU_REGISTER_READBACK_TESTAPP_MACRO_LIST})

# C7X SE ERROR INJECTION EXAMPLE APP

C7X_SE_RTOS_LIST = $(DEFAULT_RTOS_LIST)

c7x_se_SOCLIST   = j721e j721s2 j784s4 j742s2
c7x_se_BOARDLIST = j721e_evm j721s2_evm j784s4_evm j742s2_evm
c7x_se_CORELIST  = c7x_1

define C7X_SE_ERROR_INJECTION_TESTAPP_RULE
export c7x_se_error_injection_testapp_$(1)_COMP_LIST = c7x_se_error_injection_testapp_$(1)
c7x_se_error_injection_testapp_$(1)_RELPATH = ti/ip_fma/examples/c7x_se_error_injection
c7x_se_error_injection_testapp_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/c7x_se_error_injection
export c7x_se_error_injection_testapp_$(1)_BOARD_DEPENDENCY = yes
export c7x_se_error_injection_testapp_$(1)_CORE_DEPENDENCY = yes
export c7x_se_error_injection_testapp_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export c7x_se_error_injection_testapp_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
c7x_se_error_injection_testapp_$(1)_PKG_LIST = c7x_se_error_injection_testapp_$(1)
c7x_se_error_injection_testapp_$(1)_INCLUDE = $(c7x_se_error_injection_testapp_$(1)_PATH)
export c7x_se_error_injection_testapp_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(c7x_se_BOARDLIST))
export c7x_se_error_injection_testapp_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(c7x_se_CORELIST))
export c7x_se_error_injection_testapp_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += c7x_se_error_injection_testapp_$(1)
endef

C7X_SE_ERROR_INJECTION_TESTAPP_MACRO_LIST := $(foreach curos, $(C7X_SE_RTOS_LIST), $(call C7X_SE_ERROR_INJECTION_TESTAPP_RULE,$(curos)))

$(eval ${C7X_SE_ERROR_INJECTION_TESTAPP_MACRO_LIST})

# C7X ILLEGAL INSTRUCTION EXAMPLE APP

C7X_ILLEGAL_INSTRUCTION_RTOS_LIST = $(DEFAULT_RTOS_LIST)

c7x_ILLEGAL_INSTRUCTION_SOCLIST   = j721e j721s2 j784s4 j742s2
c7x_ILLEGAL_INSTRUCTION_BOARDLIST = j721e_evm j721s2_evm j784s4_evm j742s2_evm
c7x_ILLEGAL_INSTRUCTION_CORELIST  = c7x_1

define C7X_ILLEGAL_INSTRUCTION_TESTAPP_RULE
export c7x_illegal_instruction_testapp_$(1)_COMP_LIST = c7x_illegal_instruction_testapp_$(1)
c7x_illegal_instruction_testapp_$(1)_RELPATH = ti/ip_fma/examples/c7x_illegal_instruction
c7x_illegal_instruction_testapp_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/c7x_illegal_instruction
export c7x_illegal_instruction_testapp_$(1)_BOARD_DEPENDENCY = yes
export c7x_illegal_instruction_testapp_$(1)_CORE_DEPENDENCY = yes
export c7x_illegal_instruction_testapp_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export c7x_illegal_instruction_testapp_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
c7x_illegal_instruction_testapp_$(1)_PKG_LIST = c7x_illegal_instruction_testapp_$(1)
c7x_illegal_instruction_testapp_$(1)_INCLUDE = $(c7x_illegal_instruction_testapp_$(1)_PATH)
export c7x_illegal_instruction_testapp_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(c7x_ILLEGAL_INSTRUCTION_BOARDLIST))
export c7x_illegal_instruction_testapp_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(c7x_ILLEGAL_INSTRUCTION_CORELIST))
export c7x_illegal_instruction_testapp_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += c7x_illegal_instruction_testapp_$(1)
endef

C7X_ILLEGAL_INSTRUCTION_TESTAPP_MACRO_LIST := $(foreach curos, $(C7X_ILLEGAL_INSTRUCTION_RTOS_LIST), $(call C7X_ILLEGAL_INSTRUCTION_TESTAPP_RULE,$(curos)))

$(eval ${C7X_ILLEGAL_INSTRUCTION_TESTAPP_MACRO_LIST})

# C7X MMU SAFETY EXAMPLE APP

C7X_RTOS_LIST       = $(DEFAULT_RTOS_LIST)

c7x_mmu_SOCLIST   = j721e j721s2 j784s4 j742s2
c7x_mmu_BOARDLIST = j721e_evm j721s2_evm j784s4_evm j742s2_evm
c7x_mmu_CORELIST  = c7x_1


define C7X_MMU_TESTAPP_RULE
export c7x_mmu_error_injection_testapp_$(1)_COMP_LIST = c7x_mmu_error_injection_testapp_$(1)
c7x_mmu_error_injection_testapp_$(1)_RELPATH = ti/ip_fma/examples/c7x_mmu_error_injection
c7x_mmu_error_injection_testapp_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/c7x_mmu_error_injection
export c7x_mmu_error_injection_testapp_$(1)_BOARD_DEPENDENCY = yes
export c7x_mmu_error_injection_testapp_$(1)_CORE_DEPENDENCY = yes
export c7x_mmu_error_injection_testapp_$(1)_XDC_CONFIGURO = $(if $(findstring tirtos, $(1)), yes, no)
export c7x_mmu_error_injection_testapp_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
c7x_mmu_error_injection_testapp_$(1)_PKG_LIST = c7x_mmu_error_injection_testapp_$(1)
c7x_mmu_error_injection_testapp_$(1)_INCLUDE = $(c7x_mmu_error_injection_testapp_$(1)_PATH)
export c7x_mmu_error_injection_testapp_$(1)_BOARDLIST = $(filter $(DEFAULT_BOARDLIST_$(1)), $(c7x_mmu_BOARDLIST))
export c7x_mmu_error_injection_testapp_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(c7x_mmu_CORELIST))
export c7x_mmu_error_injection_testapp_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += c7x_mmu_error_injection_testapp_$(1)
endef

C7X_MMU_TESTAPP_MACRO_LIST := $(foreach curos, $(C7X_RTOS_LIST), $(call C7X_MMU_TESTAPP_RULE,$(curos)))

$(eval ${C7X_MMU_TESTAPP_MACRO_LIST})

# CSIRX IP FMA APP

ip_fma_csirx_SOCLIST         = j784s4 j721s2
ip_fma_csirx_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_csirx_CORELIST        = mcu2_0

define CSIRX_TESTAPP_RULE

csirx_ip_fma_app_$(1)_COMP_LIST = csirx_ip_fma_app_$(1)
csirx_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/csirx_ip_fma_app
csirx_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/csirx_ip_fma_app
csirx_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export csirx_ip_fma_app_$(1)_MAKEFILE
csirx_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
csirx_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
csirx_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export csirx_ip_fma_app_$(1)_COMP_LIST
export csirx_ip_fma_app_$(1)_SOC_DEPENDENCY
export csirx_ip_fma_app_$(1)_CORE_DEPENDENCY
export csirx_ip_fma_app_$(1)_BOARD_DEPENDENCY
csirx_ip_fma_app_$(1)_PKG_LIST = csirx_ip_fma_app_$(1)
csirx_ip_fma_app_$(1)_INCLUDE  = $(csirx_ip_fma_app_$(1)_PATH)
csirx_ip_fma_app_$(1)_INCLUDE  += \
                                       $(PDK_INSTALL_PATH)/ti/csl \
                                       $(PDK_INSTALL_PATH)/ti/drv/fvid2 \
                                       $(PDK_INSTALL_PATH)/ti/drv/csirx
csirx_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_csirx_BOARDLIST)
export csirx_ip_fma_app_$(1)_BOARDLIST
csirx_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_csirx_CORELIST)
export csirx_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += csirx_ip_fma_app_$(1)
csirx_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export csirx_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

CSIRX_MACRO_LIST := $(foreach curos, baremetal, $(call CSIRX_TESTAPP_RULE,$(curos)))

$(eval ${CSIRX_MACRO_LIST})

# DRU IP FMA APP
ip_fma_dru_SOCLIST         = j784s4 j721s2
ip_fma_dru_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_dru_CORELIST        = mcu2_0

define DRU_TESTAPP_RULE

dru_ip_fma_app_$(1)_COMP_LIST = dru_ip_fma_app_$(1)
dru_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/dru_ip_fma_app
dru_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/dru_ip_fma_app
dru_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export dru_ip_fma_app_$(1)_MAKEFILE
dru_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dru_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
dru_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export dru_ip_fma_app_$(1)_COMP_LIST
export dru_ip_fma_app_$(1)_SOC_DEPENDENCY
export dru_ip_fma_app_$(1)_CORE_DEPENDENCY
export dru_ip_fma_app_$(1)_BOARD_DEPENDENCY
dru_ip_fma_app_$(1)_PKG_LIST = dru_ip_fma_app_$(1)
dru_ip_fma_app_$(1)_INCLUDE  = $(dru_ip_fma_app_$(1)_PATH)
dru_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dru_BOARDLIST)
export dru_ip_fma_app_$(1)_BOARDLIST
dru_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dru_CORELIST)
export dru_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dru_ip_fma_app_$(1)
dru_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dru_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DRU_MACRO_LIST := $(foreach curos, freertos, $(call DRU_TESTAPP_RULE,$(curos)))

$(eval ${DRU_MACRO_LIST})


# DDR IP FMA APP

ip_fma_ddr_SOCLIST         = j784s4 j721s2
ip_fma_ddr_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_ddr_CORELIST        = mcu1_0

define DDR_TESTAPP_RULE
ddr_ip_fma_app_$(1)_COMP_LIST = ddr_ip_fma_app_$(1)
ddr_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/ddr_ip_fma_app
ddr_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/ddr_ip_fma_app
ddr_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export ddr_ip_fma_app_$(1)_MAKEFILE
ddr_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
ddr_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
ddr_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export ddr_ip_fma_app_$(1)_COMP_LIST
export ddr_ip_fma_app_$(1)_SOC_DEPENDENCY
export ddr_ip_fma_app_$(1)_CORE_DEPENDENCY
export ddr_ip_fma_app_$(1)_BOARD_DEPENDENCY
ddr_ip_fma_app_$(1)_PKG_LIST = ddr_ip_fma_app_$(1)
ddr_ip_fma_app_$(1)_INCLUDE  = $(ddr_ip_fma_app_$(1)_PATH)
ddr_ip_fma_app_$(1)_INCLUDE  += \
                                        $(PDK_INSTALL_PATH)/ti/csl \
								        $(PDK_INSTALL_PATH)/ti/drv/fvid2 \
								        $(PDK_INSTALL_PATH)/ti/drv/ddr
ddr_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_ddr_BOARDLIST)
export ddr_ip_fma_app_$(1)_BOARDLIST
ddr_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_ddr_CORELIST)
export ddr_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += ddr_ip_fma_app_$(1)
ddr_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export ddr_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DDR_MACRO_LIST := $(foreach curos, baremetal, $(call DDR_TESTAPP_RULE,$(curos)))

$(eval $(DDR_MACRO_LIST))


# DDR T7 IP FMA APP
ip_fma_ddr_t7_SOCLIST           = j784s4 j721s2
ip_fma_ddr_t7_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_ddr_t7_CORELIST 	     	= mcu1_0

define DDR_T7_TESTAPP_RULE

ddr_t7_ip_fma_app_$(1)_COMP_LIST = ddr_t7_ip_fma_app_$(1)
ddr_t7_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/ddr_t7_ip_fma_app
ddr_t7_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/ddr_t7_ip_fma_app
ddr_t7_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export ddr_t7_ip_fma_app_$(1)_MAKEFILE
ddr_t7_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
ddr_t7_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
ddr_t7_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export ddr_t7_ip_fma_app_$(1)_COMP_LIST
export ddr_t7_ip_fma_app_$(1)_SOC_DEPENDENCY
export ddr_t7_ip_fma_app_$(1)_CORE_DEPENDENCY
export ddr_t7_ip_fma_app_$(1)_BOARD_DEPENDENCY
ddr_t7_ip_fma_app_$(1)_PKG_LIST = ddr_t7_ip_fma_app_$(1)
ddr_t7_ip_fma_app_$(1)_INCLUDE  = $(ddr_t7_ip_fma_app_$(1)_PATH)
ddr_t7_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_ddr_t7_BOARDLIST)
export ddr_t7_ip_fma_app_$(1)_BOARDLIST
ddr_t7_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_ddr_t7_CORELIST)
export ddr_t7_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += ddr_t7_ip_fma_app_$(1)
ddr_t7_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export ddr_t7_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DDR_T7_MACRO_LIST := $(foreach curos, baremetal, $(call DDR_T7_TESTAPP_RULE,$(curos)))

$(eval ${DDR_T7_MACRO_LIST})


# DDR 17 IP FMA APP
ip_fma_ddr_17_SOCLIST           = j784s4 j721s2
ip_fma_ddr_17_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_ddr_17_CORELIST 	     	= mcu1_0

define DDR_17_TESTAPP_RULE

ddr_17_ip_fma_app_$(1)_COMP_LIST = ddr_17_ip_fma_app_$(1)
ddr_17_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/ddr_17_ip_fma_app
ddr_17_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/ddr_17_ip_fma_app
ddr_17_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export ddr_17_ip_fma_app_$(1)_MAKEFILE
ddr_17_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
ddr_17_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
ddr_17_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export ddr_17_ip_fma_app_$(1)_COMP_LIST
export ddr_17_ip_fma_app_$(1)_SOC_DEPENDENCY
export ddr_17_ip_fma_app_$(1)_CORE_DEPENDENCY
export ddr_17_ip_fma_app_$(1)_BOARD_DEPENDENCY
ddr_17_ip_fma_app_$(1)_PKG_LIST = ddr_17_ip_fma_app_$(1)
ddr_17_ip_fma_app_$(1)_INCLUDE  = $(ddr_17_ip_fma_app_$(1)_PATH)
ddr_17_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_ddr_17_BOARDLIST)
export ddr_17_ip_fma_app_$(1)_BOARDLIST
ddr_17_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_ddr_17_CORELIST)
export ddr_17_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += ddr_17_ip_fma_app_$(1)
ddr_17_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export ddr_17_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DDR_17_MACRO_LIST := $(foreach curos, baremetal, $(call DDR_17_TESTAPP_RULE,$(curos)))

$(eval ${DDR_17_MACRO_LIST})


# CLK IP FMA APP

ip_fma_clk_SOCLIST           = j784s4 j721s2
ip_fma_clk_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_clk_CORELIST          = mcu1_0

define CLK_TESTAPP_RULE

clk_ip_fma_app_$(1)_COMP_LIST  = clk_ip_fma_app_$(1)
clk_ip_fma_app_$(1)_RELPATH    = ti/ip_fma/examples/clk_ip_fma_app
clk_ip_fma_app_$(1)_PATH       = $(IP_FMA_COMP_PATH)/examples/clk_ip_fma_app
clk_ip_fma_app_$(1)_MAKEFILE   = -fmakefile BUILD_OS_TYPE=$(1)
export clk_ip_fma_app_$(1)_MAKEFILE
clk_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
clk_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
clk_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export clk_ip_fma_app_$(1)_COMP_LIST
export clk_ip_fma_app_$(1)_SOC_DEPENDENCY
export clk_ip_fma_app_$(1)_CORE_DEPENDENCY
export clk_ip_fma_app_$(1)_BOARD_DEPENDENCY
clk_ip_fma_app_$(1)_PKG_LIST  = clk_ip_fma_app_$(1)
clk_ip_fma_app_$(1)_INCLUDE   = $(clk_ip_fma_app_$(1)_PATH)
clk_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_clk_BOARDLIST)
export clk_ip_fma_app_$(1)_BOARDLIST
clk_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_clk_CORELIST)
export clk_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += clk_ip_fma_app_$(1)
clk_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export clk_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

CLK_MACRO_LIST := $(foreach curos, baremetal, $(call CLK_TESTAPP_RULE,$(curos)))

$(eval ${CLK_MACRO_LIST})


# MSMC IP FMA APP
ip_fma_msmc_SOCLIST         = j784s4 j721s2
ip_fma_msmc_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_msmc_CORELIST        = mcu1_0 mcu1_1

define MSMC_TESTAPP_RULE

msmc_ip_fma_app_$(1)_COMP_LIST = msmc_ip_fma_app_$(1)
msmc_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/msmc_ip_fma_app
msmc_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/msmc_ip_fma_app
msmc_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export msmc_ip_fma_app_$(1)_MAKEFILE
msmc_ip_fma_app_$(1)_SOC_DEPENDENCY   = yes
msmc_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
msmc_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export msmc_ip_fma_app_$(1)_COMP_LIST
export msmc_ip_fma_app_$(1)_SOC_DEPENDENCY
export msmc_ip_fma_app_$(1)_CORE_DEPENDENCY
export msmc_ip_fma_app_$(1)_BOARD_DEPENDENCY
msmc_ip_fma_app_$(1)_PKG_LIST  = msmc_ip_fma_app_$(1)
msmc_ip_fma_app_$(1)_INCLUDE   = $(msmc_ip_fma_app_$(1)_PATH)
msmc_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_msmc_BOARDLIST)
export msmc_ip_fma_app_$(1)_BOARDLIST
msmc_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_msmc_CORELIST)
export msmc_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += msmc_ip_fma_app_$(1)
msmc_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export msmc_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

MSMC_MACRO_LIST := $(foreach curos, baremetal, $(call MSMC_TESTAPP_RULE,$(curos)))

$(eval ${MSMC_MACRO_LIST})


# MSMC INT IP FMA APP
ip_fma_msmc_int_SOCLIST         = j784s4 j721s2
ip_fma_msmc_int_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_msmc_int_CORELIST        = mcu1_0

define MSMC_INT_TESTAPP_RULE

msmc_int_ip_fma_app_$(1)_COMP_LIST = msmc_int_ip_fma_app_$(1)
msmc_int_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/msmc_int_ip_fma_app
msmc_int_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/msmc_int_ip_fma_app
msmc_int_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export msmc_int_ip_fma_app_$(1)_MAKEFILE
msmc_int_ip_fma_app_$(1)_SOC_DEPENDENCY   = yes
msmc_int_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
msmc_int_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export msmc_int_ip_fma_app_$(1)_COMP_LIST
export msmc_int_ip_fma_app_$(1)_SOC_DEPENDENCY
export msmc_int_ip_fma_app_$(1)_CORE_DEPENDENCY
export msmc_int_ip_fma_app_$(1)_BOARD_DEPENDENCY
msmc_int_ip_fma_app_$(1)_PKG_LIST  = msmc_int_ip_fma_app_$(1)
msmc_int_ip_fma_app_$(1)_INCLUDE   = $(msmc_int_ip_fma_app_$(1)_PATH)
msmc_int_ip_fma_app_$(1)_INCLUDE  += $(PDK_INSTALL_PATH)/ti/csl
msmc_int_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_msmc_int_BOARDLIST)
export msmc_int_ip_fma_app_$(1)_BOARDLIST
msmc_int_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_msmc_int_CORELIST)
export msmc_int_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += msmc_int_ip_fma_app_$(1)
msmc_int_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export msmc_int_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

MSMC_INT_MACRO_LIST := $(foreach curos, baremetal, $(call MSMC_INT_TESTAPP_RULE,$(curos)))

$(eval ${MSMC_INT_MACRO_LIST})


# DSS IP FMA APP
ip_fma_dss_SOCLIST          = j784s4 j721s2
ip_fma_dss_BOARDLIST        = j784s4_evm j721s2_evm
ip_fma_dss_CORELIST	 		= mcu2_0

define DSS_TESTAPP_RULE

dss_ip_fma_app_$(1)_COMP_LIST = dss_ip_fma_app_$(1)
dss_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/dss_ip_fma_app
dss_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/dss_ip_fma_app
dss_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export dss_ip_fma_app_$(1)_MAKEFILE
dss_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dss_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
dss_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export dss_ip_fma_app_$(1)_COMP_LIST
export dss_ip_fma_app_$(1)_SOC_DEPENDENCY
export dss_ip_fma_app_$(1)_CORE_DEPENDENCY
export dss_ip_fma_app_$(1)_BOARD_DEPENDENCY
dss_ip_fma_app_$(1)_PKG_LIST = dss_ip_fma_app_$(1)
dss_ip_fma_app_$(1)_INCLUDE  = $(dss_ip_fma_app_$(1)_PATH)
dss_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dss_BOARDLIST)
export dss_ip_fma_app_$(1)_BOARDLIST
dss_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dss_CORELIST)
export dss_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dss_ip_fma_app_$(1)
dss_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dss_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DSS_MACRO_LIST := $(foreach curos, baremetal, $(call DSS_TESTAPP_RULE,$(curos)))

$(eval ${DSS_MACRO_LIST})

# DSS-F1 IP FMA APP

dss_f1_lut_refresh_SOCLIST   = j721s2 j784s4
dss_f1_lut_refresh_BOARDLIST = j721s2_evm j784s4_evm
dss_f1_lut_refresh_CORELIST  = mcu2_0

define DSS_F1_LUT_REFRESH_TESTAPP_RULE

export dssf1_ip_fma_app_$(1)_COMP_LIST = dssf1_ip_fma_app_$(1)
dssf1_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/dss_f1_ip_fma_app
dssf1_ip_fma_app_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/dss_f1_ip_fma_app
export dssf1_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
export dssf1_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dssf1_ip_fma_app_$(1)_CORE_DEPENDENCY = yes
export dssf1_ip_fma_app_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
dssf1_ip_fma_app_$(1)_PKG_LIST = dssf1_ip_fma_app_$(1)
dssf1_ip_fma_app_$(1)_INCLUDE = $(dssf1_ip_fma_app_$(1)_PATH)
export dssf1_ip_fma_app_$(1)_BOARDLIST = $(dss_f1_lut_refresh_BOARDLIST)
export dssf1_ip_fma_app_$(1)_$(SOC)_CORELIST = $(dss_f1_lut_refresh_CORELIST)
export dssf1_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += dssf1_ip_fma_app_$(1)
endef

DSS_F1_LUT_REFRESH_MACRO_LIST := $(foreach curos, baremetal, $(call DSS_F1_LUT_REFRESH_TESTAPP_RULE,$(curos)))

$(eval ${DSS_F1_LUT_REFRESH_MACRO_LIST})


# SAUL5 IP FMA APP
ip_fma_saul5_SOCLIST           = j784s4 j721s2
ip_fma_saul5_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_saul5_CORELIST 	       = mcu1_0

define SAUL5_TESTAPP_RULE

saul5_ip_fma_app_$(1)_COMP_LIST = saul5_ip_fma_app_$(1)
saul5_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/saul5_ip_fma_app
saul5_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/saul5_ip_fma_app
saul5_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export saul5_ip_fma_app_$(1)_MAKEFILE
saul5_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
saul5_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export saul5_ip_fma_app_$(1)_COMP_LIST
export saul5_ip_fma_app_$(1)_SOC_DEPENDENCY
export saul5_ip_fma_app_$(1)_CORE_DEPENDENCY
saul5_ip_fma_app_$(1)_PKG_LIST = saul5_ip_fma_app_$(1)
saul5_ip_fma_app_$(1)_INCLUDE  = $(saul5_ip_fma_app_$(1)_PATH)
saul5_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_saul5_BOARDLIST)
export saul5_ip_fma_app_$(1)_BOARDLIST
saul5_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_saul5_CORELIST)
export saul5_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += saul5_ip_fma_app_$(1)
saul5_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export saul5_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

SAUL5_MACRO_LIST := $(foreach curos, baremetal, $(call SAUL5_TESTAPP_RULE,$(curos)))

$(eval ${SAUL5_MACRO_LIST})


# SAUL6 IP FMA APP
ip_fma_saul6_SOCLIST           = j784s4 j721s2
ip_fma_saul6_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_saul6_CORELIST 	       = mcu1_0

define SAUL6_TESTAPP_RULE

saul6_ip_fma_app_$(1)_COMP_LIST = saul6_ip_fma_app_$(1)
saul6_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/saul6_ip_fma_app
saul6_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/saul6_ip_fma_app
saul6_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export saul6_ip_fma_app_$(1)_MAKEFILE
saul6_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
saul6_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export saul6_ip_fma_app_$(1)_COMP_LIST
export saul6_ip_fma_app_$(1)_SOC_DEPENDENCY
export saul6_ip_fma_app_$(1)_CORE_DEPENDENCY
saul6_ip_fma_app_$(1)_PKG_LIST = saul6_ip_fma_app_$(1)
saul6_ip_fma_app_$(1)_INCLUDE  = $(saul6_ip_fma_app_$(1)_PATH)
saul6_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_saul6_BOARDLIST)
export saul6_ip_fma_app_$(1)_BOARDLIST
saul6_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_saul6_CORELIST)
export saul6_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += saul6_ip_fma_app_$(1)
saul6_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export saul6_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

SAUL6_MACRO_LIST := $(foreach curos, baremetal, $(call SAUL6_TESTAPP_RULE,$(curos)))

$(eval ${SAUL6_MACRO_LIST})


# CBA T3 IP FMA APP
ip_fma_cbat3_SOCLIST         = j784s4 j721s2
ip_fma_cbat3_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_cbat3_CORELIST        = mcu1_0

define CBAT3_TESTAPP_RULE

cbat3_ip_fma_app_$(1)_COMP_LIST = cbat3_ip_fma_app_$(1)
cbat3_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/cbat3_ip_fma_app
cbat3_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/cbat3_ip_fma_app
cbat3_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export cbat3_ip_fma_app_$(1)_MAKEFILE
cbat3_ip_fma_app_$(1)_SOC_DEPENDENCY   = yes
cbat3_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
cbat3_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export cbat3_ip_fma_app_$(1)_COMP_LIST
export cbat3_ip_fma_app_$(1)_SOC_DEPENDENCY
export cbat3_ip_fma_app_$(1)_CORE_DEPENDENCY
export cbat3_ip_fma_app_$(1)_BOARD_DEPENDENCY
cbat3_ip_fma_app_$(1)_PKG_LIST  = cbat3_ip_fma_app_$(1)
cbat3_ip_fma_app_$(1)_INCLUDE   = $(cbat3_ip_fma_app_$(1)_PATH)
cbat3_ip_fma_app_$(1)_INCLUDE  += $(PDK_INSTALL_PATH)/ti/csl
cbat3_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_cbat3_BOARDLIST)
export cbat3_ip_fma_app_$(1)_BOARDLIST
cbat3_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_cbat3_CORELIST)
export cbat3_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += cbat3_ip_fma_app_$(1)
cbat3_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export cbat3_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

CBAT3_MACRO_LIST := $(foreach curos, baremetal, $(call CBAT3_TESTAPP_RULE,$(curos)))

$(eval ${CBAT3_MACRO_LIST})


# CBA T4 IP FMA APP
ip_fma_cbat4_SOCLIST         = j784s4 j721s2
ip_fma_cbat4_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_cbat4_CORELIST        = mcu1_0

define CBAT4_TESTAPP_RULE

cbat4_ip_fma_app_$(1)_COMP_LIST = cbat4_ip_fma_app_$(1)
cbat4_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/cbat4_ip_fma_app
cbat4_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/cbat4_ip_fma_app
cbat4_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export cbat4_ip_fma_app_$(1)_MAKEFILE
cbat4_ip_fma_app_$(1)_SOC_DEPENDENCY   = yes
cbat4_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
cbat4_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export cbat4_ip_fma_app_$(1)_COMP_LIST
export cbat4_ip_fma_app_$(1)_SOC_DEPENDENCY
export cbat4_ip_fma_app_$(1)_CORE_DEPENDENCY
export cbat4_ip_fma_app_$(1)_BOARD_DEPENDENCY
cbat4_ip_fma_app_$(1)_PKG_LIST  = cbat4_ip_fma_app_$(1)
cbat4_ip_fma_app_$(1)_INCLUDE   = $(cbat4_ip_fma_app_$(1)_PATH)
cbat4_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_cbat4_BOARDLIST)
export cbat4_ip_fma_app_$(1)_BOARDLIST
cbat4_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_cbat4_CORELIST)
export cbat4_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += cbat4_ip_fma_app_$(1)
cbat4_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export cbat4_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

CBAT4_MACRO_LIST := $(foreach curos, baremetal, $(call CBAT4_TESTAPP_RULE,$(curos)))

$(eval ${CBAT4_MACRO_LIST})


# CBA IP FMA APP

ip_fma_cba_SOCLIST         = j784s4 j721s2
ip_fma_cba_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_cba_CORELIST        = mcu1_0

define CBA_TESTAPP_RULE

cba_ip_fma_app_$(1)_COMP_LIST = cba_ip_fma_app_$(1)
cba_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/cba_ip_fma_app
cba_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/cba_ip_fma_app
cba_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export cba_ip_fma_app_$(1)_MAKEFILE
cba_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
cba_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
cba_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export cba_ip_fma_app_$(1)_COMP_LIST
export cba_ip_fma_app_$(1)_SOC_DEPENDENCY
export cba_ip_fma_app_$(1)_CORE_DEPENDENCY
export cba_ip_fma_app_$(1)_BOARD_DEPENDENCY
cba_ip_fma_app_$(1)_PKG_LIST = cba_ip_fma_app_$(1)
cba_ip_fma_app_$(1)_INCLUDE  = $(cba_ip_fma_app_$(1)_PATH)
cba_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_cba_BOARDLIST)
export cba_ip_fma_app_$(1)_BOARDLIST
cba_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_cba_CORELIST)
export cba_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += cba_ip_fma_app_$(1)
cba_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export cba_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

CBA_MACRO_LIST := $(foreach curos, baremetal, $(call CBA_TESTAPP_RULE,$(curos)))

$(eval ${CBA_MACRO_LIST})

# PSC IP FMA APP

ip_fma_psc_SOCLIST           = j784s4 j721s2
ip_fma_psc_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_psc_CORELIST          = mcu2_0 mcu1_0

define PSC_TESTAPP_RULE

psc_ip_fma_app_$(1)_COMP_LIST  = psc_ip_fma_app_$(1)
psc_ip_fma_app_$(1)_RELPATH    = ti/ip_fma/examples/psc_ip_fma_app
psc_ip_fma_app_$(1)_PATH       = $(IP_FMA_COMP_PATH)/examples/psc_ip_fma_app
psc_ip_fma_app_$(1)_MAKEFILE   = -fmakefile BUILD_OS_TYPE=$(1)
export psc_ip_fma_app_$(1)_MAKEFILE
psc_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
psc_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
psc_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export psc_ip_fma_app_$(1)_COMP_LIST
export psc_ip_fma_app_$(1)_SOC_DEPENDENCY
export psc_ip_fma_app_$(1)_CORE_DEPENDENCY
export psc_ip_fma_app_$(1)_BOARD_DEPENDENCY
psc_ip_fma_app_$(1)_PKG_LIST  = psc_ip_fma_app_$(1)
psc_ip_fma_app_$(1)_INCLUDE   = $(psc_ip_fma_app_$(1)_PATH)
psc_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_psc_BOARDLIST)
export psc_ip_fma_app_$(1)_BOARDLIST
psc_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_psc_CORELIST)
export psc_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += psc_ip_fma_app_$(1)
psc_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export psc_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

PSC_MACRO_LIST := $(foreach curos, baremetal, $(call PSC_TESTAPP_RULE,$(curos)))

$(eval ${PSC_MACRO_LIST})


# R5F IP FMA APP
ip_fma_r5f_SOCLIST           = j784s4 j721s2
ip_fma_r5f_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_r5f_CORELIST 	     = mcu1_0

# R5F CPU3 IP FMA APP
ip_fma_r5f_cpu3_SOCLIST           = j784s4 j721s2
ip_fma_r5f_cpu3_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_r5f_cpu3_CORELIST 	      = mcu1_0

define R5F_CPU3_TESTAPP_RULE

r5f_cpu3_ip_fma_app_$(1)_COMP_LIST = r5f_cpu3_ip_fma_app_$(1)
r5f_cpu3_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/r5f/r5f_cpu3_ip_fma_app
r5f_cpu3_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/r5f/r5f_cpu3_ip_fma_app
r5f_cpu3_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export r5f_cpu3_ip_fma_app_$(1)_MAKEFILE
r5f_cpu3_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
r5f_cpu3_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export r5f_cpu3_ip_fma_app_$(1)_COMP_LIST
export r5f_cpu3_ip_fma_app_$(1)_SOC_DEPENDENCY
export r5f_cpu3_ip_fma_app_$(1)_CORE_DEPENDENCY
r5f_cpu3_ip_fma_app_$(1)_PKG_LIST = r5f_cpu3_ip_fma_app_$(1)
r5f_cpu3_ip_fma_app_$(1)_INCLUDE  = $(r5f_cpu3_ip_fma_app_$(1)_PATH) $(IP_FMA_COMP_PATH)/examples/r5f/common/inc
r5f_cpu3_ip_fma_app_$(1)_APP_STAGE_FILES  = $(IP_FMA_COMP_PATH)/examples/r5f/common/src/ip_fma_r5f_app_utils.c
r5f_cpu3_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_r5f_cpu3_BOARDLIST)
export r5f_cpu3_ip_fma_app_$(1)_BOARDLIST
r5f_cpu3_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_r5f_cpu3_CORELIST)
export r5f_cpu3_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += r5f_cpu3_ip_fma_app_$(1)
r5f_cpu3_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export r5f_cpu3_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

R5F_CPU3_MACRO_LIST := $(foreach curos, baremetal, $(call R5F_CPU3_TESTAPP_RULE,$(curos)))

$(eval ${R5F_CPU3_MACRO_LIST})


# R5F CPU5 IP FMA APP
ip_fma_r5f_cpu5_SOCLIST           = j784s4 j721s2
ip_fma_r5f_cpu5_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_r5f_cpu5_CORELIST 	     = mcu1_0

define R5F_CPU5_TESTAPP_RULE

r5f_cpu5_ip_fma_app_$(1)_COMP_LIST = r5f_cpu5_ip_fma_app_$(1)
r5f_cpu5_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/r5f/r5f_cpu5_ip_fma_app
r5f_cpu5_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/r5f/r5f_cpu5_ip_fma_app
r5f_cpu5_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export r5f_cpu5_ip_fma_app_$(1)_MAKEFILE
r5f_cpu5_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
r5f_cpu5_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export r5f_cpu5_ip_fma_app_$(1)_COMP_LIST
export r5f_cpu5_ip_fma_app_$(1)_SOC_DEPENDENCY
export r5f_cpu5_ip_fma_app_$(1)_CORE_DEPENDENCY
r5f_cpu5_ip_fma_app_$(1)_PKG_LIST = r5f_cpu5_ip_fma_app_$(1)
r5f_cpu5_ip_fma_app_$(1)_INCLUDE  = $(r5f_cpu5_ip_fma_app_$(1)_PATH)
r5f_cpu5_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_r5f_cpu5_BOARDLIST)
export r5f_cpu5_ip_fma_app_$(1)_BOARDLIST
r5f_cpu5_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_r5f_cpu5_CORELIST)
export r5f_cpu5_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += r5f_cpu5_ip_fma_app_$(1)
r5f_cpu5_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export r5f_cpu5_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

R5F_CPU5_MACRO_LIST := $(foreach curos, baremetal, $(call R5F_CPU5_TESTAPP_RULE,$(curos)))

$(eval ${R5F_CPU5_MACRO_LIST})


# R5F CPU6 IP FMA APP
ip_fma_r5f_cpu6_SOCLIST           = j784s4 j721s2
ip_fma_r5f_cpu6_BOARDLIST         = j784s4_evm j721s2_evm
ip_fma_r5f_cpu6_CORELIST 	      = mcu1_0

define R5F_CPU6_TESTAPP_RULE

r5f_cpu6_ip_fma_app_$(1)_COMP_LIST = r5f_cpu6_ip_fma_app_$(1)
r5f_cpu6_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/r5f/r5f_cpu6_ip_fma_app
r5f_cpu6_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/r5f/r5f_cpu6_ip_fma_app
r5f_cpu6_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export r5f_cpu6_ip_fma_app_$(1)_MAKEFILE
r5f_cpu6_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
r5f_cpu6_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export r5f_cpu6_ip_fma_app_$(1)_COMP_LIST
export r5f_cpu6_ip_fma_app_$(1)_SOC_DEPENDENCY
export r5f_cpu6_ip_fma_app_$(1)_CORE_DEPENDENCY
r5f_cpu6_ip_fma_app_$(1)_PKG_LIST = r5f_cpu6_ip_fma_app_$(1)
r5f_cpu6_ip_fma_app_$(1)_INCLUDE  = $(r5f_cpu6_ip_fma_app_$(1)_PATH) $(IP_FMA_COMP_PATH)/examples/r5f/common/inc
r5f_cpu6_ip_fma_app_$(1)_APP_STAGE_FILES  = $(IP_FMA_COMP_PATH)/examples/r5f/common/src/ip_fma_r5f_app_utils.c
r5f_cpu6_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_r5f_cpu6_BOARDLIST)
export r5f_cpu6_ip_fma_app_$(1)_BOARDLIST
r5f_cpu6_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_r5f_cpu6_CORELIST)
export r5f_cpu6_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += r5f_cpu6_ip_fma_app_$(1)
r5f_cpu6_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export r5f_cpu6_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

R5F_CPU6_MACRO_LIST := $(foreach curos, baremetal, $(call R5F_CPU6_TESTAPP_RULE,$(curos)))

$(eval ${R5F_CPU6_MACRO_LIST})


# SMS IP FMA APP
ip_fma_sms_SOCLIST          = j784s4 j721s2
ip_fma_sms_BOARDLIST        = j784s4_evm j721s2_evm
ip_fma_sms_CORELIST	 	    = mcu1_0

define SMS_TESTAPP_RULE

sms_ip_fma_app_$(1)_COMP_LIST = sms_ip_fma_app_$(1)
sms_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/sms_ip_fma_app
sms_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/sms_ip_fma_app
sms_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export sms_ip_fma_app_$(1)_MAKEFILE
sms_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
sms_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
sms_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export sms_ip_fma_app_$(1)_COMP_LIST
export sms_ip_fma_app_$(1)_SOC_DEPENDENCY
export sms_ip_fma_app_$(1)_CORE_DEPENDENCY
export sms_ip_fma_app_$(1)_BOARD_DEPENDENCY
sms_ip_fma_app_$(1)_PKG_LIST = sms_ip_fma_app_$(1)
sms_ip_fma_app_$(1)_INCLUDE  = $(sms_ip_fma_app_$(1)_PATH)
sms_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_sms_BOARDLIST)
export sms_ip_fma_app_$(1)_BOARDLIST
sms_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_sms_CORELIST)
export sms_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += sms_ip_fma_app_$(1)
sms_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export sms_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

SMS_MACRO_LIST := $(foreach curos, baremetal, $(call SMS_TESTAPP_RULE,$(curos)))

$(eval ${SMS_MACRO_LIST})


# ADC IP FMA APP

ip_fma_adc_SOCLIST         = j784s4 j721s2
ip_fma_adc_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_adc_CORELIST        = mcu1_0

define ADC_TESTAPP_RULE

adc_ip_fma_app_$(1)_COMP_LIST = adc_ip_fma_app_$(1)
adc_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/adc_ip_fma_app
adc_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/adc_ip_fma_app
adc_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export adc_ip_fma_app_$(1)_MAKEFILE
adc_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
adc_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
adc_ip_fma_app_$(1)_BOARD_DEPENDENCY  = yes
export adc_ip_fma_app_$(1)_COMP_LIST
export adc_ip_fma_app_$(1)_SOC_DEPENDENCY
export adc_ip_fma_app_$(1)_CORE_DEPENDENCY
export adc_ip_fma_app_$(1)_BOARD_DEPENDENCY
adc_ip_fma_app_$(1)_PKG_LIST = adc_ip_fma_app_$(1)
adc_ip_fma_app_$(1)_INCLUDE  = $(adc_ip_fma_app_$(1)_PATH)
adc_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_adc_BOARDLIST)
export adc_ip_fma_app_$(1)_BOARDLIST
adc_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_adc_CORELIST)
export adc_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += adc_ip_fma_app_$(1)
adc_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export adc_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

ADC_MACRO_LIST := $(foreach curos, baremetal, $(call ADC_TESTAPP_RULE,$(curos)))

$(eval ${ADC_MACRO_LIST})

# DSS8_DISPLAY IP FMA APP

ip_fma_dss8_display_SOCLIST         = j784s4 j721s2
ip_fma_dss8_display_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_dss8_display_CORELIST        = mcu2_0

define DSS8_DISPLAY_TESTAPP_RULE

dss8_display_ip_fma_app_$(1)_COMP_LIST = dss8_display_ip_fma_app_$(1)
dss8_display_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/dss_syncloss_ip_fma_app/display_app
dss8_display_ip_fma_app_$(1)_PATH = $(IP_FMA_COMP_PATH)/examples/dss_syncloss_ip_fma_app/display_app
dss8_display_ip_fma_app_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
export dss8_display_ip_fma_app_$(1)_MAKEFILE
dss8_display_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dss8_display_ip_fma_app_$(1)_CORE_DEPENDENCY = yes
dss8_display_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dss8_display_ip_fma_app_$(1)_COMP_LIST
export dss8_display_ip_fma_app_$(1)_SOC_DEPENDENCY
export dss8_display_ip_fma_app_$(1)_CORE_DEPENDENCY
export dss8_display_ip_fma_app_$(1)_BOARD_DEPENDENCY
dss8_display_ip_fma_app_$(1)_PKG_LIST = dss8_display_ip_fma_app_$(1)
dss8_display_ip_fma_app_$(1)_INCLUDE = $(dss8_display_ip_fma_app_$(1)_PATH)
dss8_display_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dss8_display_BOARDLIST)
export dss8_display_ip_fma_app_$(1)_BOARDLIST
dss8_display_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dss8_display_CORELIST)
export dss8_display_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dss8_display_ip_fma_app_$(1)
dss8_display_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dss8_display_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DSS8_DISPLAY_MACRO_LIST := $(foreach curos, freertos, $(call DSS8_DISPLAY_TESTAPP_RULE,$(curos)))

$(eval ${DSS8_DISPLAY_MACRO_LIST})


# DSS8_DRU IP FMA APP

ip_fma_dss8_dru_SOCLIST         = j784s4 j721s2
ip_fma_dss8_dru_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_dss8_dru_CORELIST        = mcu2_1

define DSS8_DRU_TESTAPP_RULE

dss8_dru_ip_fma_app_$(1)_COMP_LIST = dss8_dru_ip_fma_app_$(1)
dss8_dru_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/dss_syncloss_ip_fma_app/dru_app
dss8_dru_ip_fma_app_$(1)_PATH = $(IP_FMA_COMP_PATH)/examples/dss_syncloss_ip_fma_app/dru_app
dss8_dru_ip_fma_app_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
export dss8_dru_ip_fma_app_$(1)_MAKEFILE
dss8_dru_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dss8_dru_ip_fma_app_$(1)_CORE_DEPENDENCY = yes
dss8_dru_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dss8_dru_ip_fma_app_$(1)_COMP_LIST
export dss8_dru_ip_fma_app_$(1)_SOC_DEPENDENCY
export dss8_dru_ip_fma_app_$(1)_CORE_DEPENDENCY
export dss8_dru_ip_fma_app_$(1)_BOARD_DEPENDENCY
dss8_dru_ip_fma_app_$(1)_PKG_LIST = dss8_dru_ip_fma_app_$(1)
dss8_dru_ip_fma_app_$(1)_INCLUDE = $(dss8_dru_ip_fma_app_$(1)_PATH)
dss8_dru_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dss8_dru_BOARDLIST)
export dss8_dru_ip_fma_app_$(1)_BOARDLIST
dss8_dru_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dss8_dru_CORELIST)
export dss8_dru_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dss8_dru_ip_fma_app_$(1)
dss8_dru_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dss8_dru_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DSS8_DRU_MACRO_LIST := $(foreach curos, freertos, $(call DSS8_DRU_TESTAPP_RULE,$(curos)))

$(eval ${DSS8_DRU_MACRO_LIST})


# DSS8_UDMA IP FMA APP

ip_fma_dss8_udma_SOCLIST         = j784s4 j721s2
ip_fma_dss8_udma_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_dss8_udma_CORELIST        = mcu3_0

define DSS8_UDMA_TESTAPP_RULE

dss8_udma_ip_fma_app_$(1)_COMP_LIST = dss8_udma_ip_fma_app_$(1)
dss8_udma_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/dss_syncloss_ip_fma_app/udma_app
dss8_udma_ip_fma_app_$(1)_PATH = $(IP_FMA_COMP_PATH)/examples/dss_syncloss_ip_fma_app/udma_app
dss8_udma_ip_fma_app_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
export dss8_udma_ip_fma_app_$(1)_MAKEFILE
dss8_udma_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dss8_udma_ip_fma_app_$(1)_CORE_DEPENDENCY = yes
dss8_udma_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dss8_udma_ip_fma_app_$(1)_COMP_LIST
export dss8_udma_ip_fma_app_$(1)_SOC_DEPENDENCY
export dss8_udma_ip_fma_app_$(1)_CORE_DEPENDENCY
export dss8_udma_ip_fma_app_$(1)_BOARD_DEPENDENCY
dss8_udma_ip_fma_app_$(1)_PKG_LIST = dss8_udma_ip_fma_app_$(1)
dss8_udma_ip_fma_app_$(1)_INCLUDE = $(dss8_udma_ip_fma_app_$(1)_PATH)
dss8_udma_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dss8_udma_BOARDLIST)
export dss8_udma_ip_fma_app_$(1)_BOARDLIST
dss8_udma_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dss8_udma_CORELIST)
export dss8_udma_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dss8_udma_ip_fma_app_$(1)
dss8_udma_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dss8_udma_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DSS8_UDMA_MACRO_LIST := $(foreach curos, freertos, $(call DSS8_UDMA_TESTAPP_RULE,$(curos)))

$(eval ${DSS8_UDMA_MACRO_LIST})


# UTC_ACTIVE_TR_EVENT_OVERFLOW_DETECTION_CHECK FMA APP

utc_legal_tr_check_SOCLIST   = j721s2 j784s4
utc_legal_tr_check_BOARDLIST = j721s2_evm j784s4_evm
utc_legal_tr_check_CORELIST  = mcu2_0

define UTC_LEGAL_TR_CHECK_TESTAPP_RULE

export utc_legal_tr_check_ip_fma_app_$(1)_COMP_LIST = utc_legal_tr_check_ip_fma_app_$(1)
utc_legal_tr_check_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/utc_legal_tr_check_ip_fma_app
utc_legal_tr_check_ip_fma_app_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/utc_legal_tr_check_ip_fma_app
export utc_legal_tr_check_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export utc_legal_tr_check_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
export utc_legal_tr_check_ip_fma_app_$(1)_XDC_CONFIGURO    = $(if $(findstring tirtos, $(1)), yes, no)
export utc_legal_tr_check_ip_fma_app_$(1)_MAKEFILE         = -f makefile BUILD_OS_TYPE=$(1)
utc_legal_tr_check_ip_fma_app_$(1)_PKG_LIST = utc_legal_tr_check_ip_fma_app_$(1)
utc_legal_tr_check_ip_fma_app_$(1)_INCLUDE  = $(utc_legal_tr_check_ip_fma_app_$(1)_PATH)
export utc_legal_tr_check_ip_fma_app_$(1)_BOARDLIST       = $(utc_legal_tr_check_BOARDLIST)
export utc_legal_tr_check_ip_fma_app_$(1)_$(SOC)_CORELIST = $(utc_legal_tr_check_CORELIST)
export utc_legal_tr_check_ip_fma_app_$(1)_BOARD_DEPENDENCY
export utc_legal_tr_check_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += utc_legal_tr_check_ip_fma_app_$(1)
endef

UTC_LEGAL_TR_CHECK_MACRO_LIST := $(call UTC_LEGAL_TR_CHECK_TESTAPP_RULE,baremetal)
$(eval ${UTC_LEGAL_TR_CHECK_MACRO_LIST})

# UDMA1_UDMA2 IP FMA APP

ip_fma_udma1_udma2_SOCLIST         = j784s4 j721s2
ip_fma_udma1_udma2_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_udma1_udma2_CORELIST        = mcu1_0

define UDMA1_UDMA2_TESTAPP_RULE
udma1_udma2_ip_fma_app_$(1)_COMP_LIST = udma1_udma2_ip_fma_app_$(1)
udma1_udma2_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/udma1_udma2_ip_fma_app
udma1_udma2_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/udma1_udma2_ip_fma_app
udma1_udma2_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export udma1_udma2_ip_fma_app_$(1)_MAKEFILE
udma1_udma2_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
udma1_udma2_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
udma1_udma2_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export udma1_udma2_ip_fma_app_$(1)_COMP_LIST
export udma1_udma2_ip_fma_app_$(1)_SOC_DEPENDENCY
export udma1_udma2_ip_fma_app_$(1)_CORE_DEPENDENCY
export udma1_udma2_ip_fma_app_$(1)_BOARD_DEPENDENCY
udma1_udma2_ip_fma_app_$(1)_PKG_LIST = udma1_udma2_ip_fma_app_$(1)
udma1_udma2_ip_fma_app_$(1)_INCLUDE  = $(udma1_udma2_ip_fma_app_$(1)_PATH)
udma1_udma2_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_udma1_udma2_BOARDLIST)
export udma1_udma2_ip_fma_app_$(1)_BOARDLIST
udma1_udma2_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_udma1_udma2_CORELIST)
export udma1_udma2_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += udma1_udma2_ip_fma_app_$(1)
udma1_udma2_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export udma1_udma2_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

UDMA1_UDMA2_MACRO_LIST := $(foreach curos, baremetal, $(call UDMA1_UDMA2_TESTAPP_RULE,$(curos)))

$(eval $(UDMA1_UDMA2_MACRO_LIST))

# DRU11_DRU12 IP FMA APP

ip_fma_dru11_dru12_SOCLIST         = j784s4 j721s2
ip_fma_dru11_dru12_BOARDLIST       = j784s4_evm j721s2_evm
ip_fma_dru11_dru12_CORELIST        = mcu2_0  

define DRU11_DRU12_TESTAPP_RULE
dru11_dru12_ip_fma_app_$(1)_COMP_LIST = dru11_dru12_ip_fma_app_$(1)
dru11_dru12_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/dru11_dru12_ip_fma_app
dru11_dru12_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/dru11_dru12_ip_fma_app
dru11_dru12_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export dru11_dru12_ip_fma_app_$(1)_MAKEFILE
dru11_dru12_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
dru11_dru12_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
dru11_dru12_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dru11_dru12_ip_fma_app_$(1)_COMP_LIST
export dru11_dru12_ip_fma_app_$(1)_SOC_DEPENDENCY
export dru11_dru12_ip_fma_app_$(1)_CORE_DEPENDENCY
export dru11_dru12_ip_fma_app_$(1)_BOARD_DEPENDENCY
dru11_dru12_ip_fma_app_$(1)_PKG_LIST = dru11_dru12_ip_fma_app_$(1)
dru11_dru12_ip_fma_app_$(1)_INCLUDE  = $(dru11_dru12_ip_fma_app_$(1)_PATH)
dru11_dru12_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_dru11_dru12_BOARDLIST)
export dru11_dru12_ip_fma_app_$(1)_BOARDLIST
dru11_dru12_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_dru11_dru12_CORELIST)
export dru11_dru12_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += dru11_dru12_ip_fma_app_$(1)
dru11_dru12_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export dru11_dru12_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

DRU11_DRU12_MACRO_LIST := $(foreach curos, freertos, $(call DRU11_DRU12_TESTAPP_RULE,$(curos)))

$(eval $(DRU11_DRU12_MACRO_LIST))


# UDMA3 IP FMA APP

ip_fma_udma3_SOCLIST        = j784s4 j721s2
ip_fma_udma3_BOARDLIST      = j784s4_evm j721s2_evm
ip_fma_udma3_CORELIST       = mcu1_0

define UDMA3_TESTAPP_RULE
udma3_ip_fma_app_$(1)_COMP_LIST = udma3_ip_fma_app_$(1)
udma3_ip_fma_app_$(1)_RELPATH   = ti/ip_fma/examples/udma3_ip_fma_app
udma3_ip_fma_app_$(1)_PATH      = $(IP_FMA_COMP_PATH)/examples/udma3_ip_fma_app
udma3_ip_fma_app_$(1)_MAKEFILE  = -fmakefile BUILD_OS_TYPE=$(1)
export udma3_ip_fma_app_$(1)_MAKEFILE
udma3_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
udma3_ip_fma_app_$(1)_CORE_DEPENDENCY  = yes
udma3_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export udma3_ip_fma_app_$(1)_COMP_LIST
export udma3_ip_fma_app_$(1)_SOC_DEPENDENCY
export udma3_ip_fma_app_$(1)_CORE_DEPENDENCY
export udma3_ip_fma_app_$(1)_BOARD_DEPENDENCY
udma3_ip_fma_app_$(1)_PKG_LIST = udma3_ip_fma_app_$(1)
udma3_ip_fma_app_$(1)_INCLUDE  = $(udma3_ip_fma_app_$(1)_PATH)
udma3_ip_fma_app_$(1)_BOARDLIST = $(ip_fma_udma3_BOARDLIST)
export udma3_ip_fma_app_$(1)_BOARDLIST
udma3_ip_fma_app_$(1)_$(SOC)_CORELIST = $(ip_fma_udma3_CORELIST)
export udma3_ip_fma_app_$(1)_$(SOC)_CORELIST
ip_fma_EXAMPLE_LIST += udma3_ip_fma_app_$(1)
udma3_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
export udma3_ip_fma_app_$(1)_SBL_APPIMAGEGEN
endef

UDMA3_MACRO_LIST := $(foreach curos, freertos, $(call UDMA3_TESTAPP_RULE,$(curos)))

$(eval $(UDMA3_MACRO_LIST))


# DRU8 IP FMA APP
utc_ovrflw_SOCLIST   = j721s2 j784s4
utc_ovrflw_BOARDLIST = j721s2_evm j784s4_evm 
utc_ovrflw_CORELIST  = mcu2_0

define UTC_OVRFLW_TESTAPP_RULE

export dru8_ip_fma_app_$(1)_COMP_LIST = dru8_ip_fma_app_$(1)
dru8_ip_fma_app_$(1)_RELPATH = ti/ip_fma/examples/dru8_ip_fma_app
dru8_ip_fma_app_$(1)_PATH = $(PDK_INSTALL_PATH)/ti/ip_fma/examples/dru8_ip_fma_app
export dru8_ip_fma_app_$(1)_BOARD_DEPENDENCY = yes
export dru8_ip_fma_app_$(1)_CORE_DEPENDENCY = yes
export dru8_ip_fma_app_$(1)_SOC_DEPENDENCY = yes
export dru8_ip_fma_app_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
dru8_ip_fma_app_$(1)_PKG_LIST = dru8_ip_fma_app_$(1)
dru8_ip_fma_app_$(1)_INCLUDE = $(dru8_ip_fma_app_$(1)_PATH)
export dru8_ip_fma_app_$(1)_BOARDLIST = $(utc_ovrflw_BOARDLIST)
export dru8_ip_fma_app_$(1)_$(SOC)_CORELIST = $(utc_ovrflw_CORELIST)
export dru8_ip_fma_app_$(1)_SBL_APPIMAGEGEN = yes
ip_fma_EXAMPLE_LIST += dru8_ip_fma_app_$(1)
endef

UTC_OVRFLW_MACRO_LIST := $(foreach curos, baremetal, $(call UTC_OVRFLW_TESTAPP_RULE,$(curos)))
$(eval ${UTC_OVRFLW_MACRO_LIST})

export ip_fma_EXAMPLE_LIST

ip_fma_component_make_include := 1