# currently required to be set to yes
BUILD_IGNORE_LIB_ORDER?=yes

# Build for SoC
BUILD_TARGET_MODE?=yes
# Build for x86 PC
BUILD_EMULATION_MODE?=no
# valid values: X86 x86_64 all
BUILD_EMULATION_ARCH?=x86_64

# valid values: release debug all
PROFILE?=release

# Applied to target mode only
BUILD_LINUX_MPU?=yes
# Applied to target mode only - by default kept as no so that all users don't have to change
BUILD_QNX_MPU?=no

# RTOS selection for R5F - FREERTOS, SAFERTOS, THREADX
RTOS?=FREERTOS

# SOC selection - supported values: j721e, j721s2, j784s4, j742s2, am62a, j722s
export SOC?=replace_me_soc_name

# TISDK_IMAGE selection - supported values: default, adas, edgeai
export TISDK_IMAGE?=replace_me_tisdk_image_name

# Flag to differentiate between EVM/VDK builds
export VDK?=no

# PDK board to build for, valid values: j721e_sim j721e_evm j721s2_evm j784s4_evm j742s2_evm am62a_evm j722s_evm
BUILD_PDK_BOARD=$(SOC)_evm

# Default RTOS SDK
ifeq ($(SOC), $(filter $(SOC), am62a j722s))
    RTOS_SDK ?= mcu_plus_sdk
else ifeq ($(SOC),tda54)
    RTOS_SDK ?= mcu_sdk
else
    RTOS_SDK ?= pdk
endif

# Macro to enable LDRA build
export LDRA_COVERAGE_ENABLED?=no
export LDRA_COVERAGE_ENABLED_IMAGING?=no

# Macro to enable building of CTools library
export CTOOLS_BUILD_ENABLED ?= no

# Concerto Banner Suppression of build log to reduce the build log verboseness
export NO_BANNER=1

# Macro to enable new TIDL structure for CICD
ifneq ($(SOC), $(filter $(SOC), am62a))
    export ENABLE_NEW_TIDL_STRUCTURE ?= yes
else
    export ENABLE_NEW_TIDL_STRUCTURE ?= no
endif

ifeq ($(SOC),j721e)
    TARGET_SOC=J721E
    SOC_DEF=SOC_J721E
    VPAC_VERSION=VPAC1
    C7X_TARGET=C71
    C7X_VERSION=C7100
    MPU_CPU=A72
    SOC_FAMILY=SOC_FAMILY_J7
else ifeq ($(SOC),j721s2)
    TARGET_SOC=J721S2
    SOC_DEF=SOC_J721S2
    VPAC_VERSION=VPAC3
    C7X_TARGET=C7120
    C7X_VERSION=C7120
    MPU_CPU=A72
    SOC_FAMILY=SOC_FAMILY_J7
else ifeq ($(SOC),j784s4)
    TARGET_SOC=J784S4
    SOC_DEF=SOC_J784S4
    VPAC_VERSION=VPAC3
    C7X_TARGET=C7120
    C7X_VERSION=C7120
    MPU_CPU=A72
    SOC_FAMILY=SOC_FAMILY_J7
else ifeq ($(SOC),j742s2)
    TARGET_SOC=J742S2
    SOC_DEF=SOC_J742S2
    VPAC_VERSION=VPAC3
    C7X_TARGET=C7120
    C7X_VERSION=C7120
    MPU_CPU=A72
    SOC_FAMILY=SOC_FAMILY_J7
else ifeq ($(SOC),am62a)
    TARGET_SOC=AM62A
    SOC_DEF+=SOC_AM62A
    ifeq ($(RTOS_SDK),mcu_plus_sdk)
        SOC_DEF += SOC_AM62AX
    endif
    VPAC_VERSION=VPAC3L
    C7X_TARGET=C7504
    C7X_VERSION=C7504
    MPU_CPU=A53
    SOC_FAMILY=SOC_FAMILY_AM
else ifeq ($(SOC),j722s)
    TARGET_SOC=J722S
    SOC_DEF+=SOC_J722S
    VPAC_VERSION=VPAC3L
    C7X_TARGET=C7524
    C7X_VERSION=C7524
    MPU_CPU=A53
    SOC_FAMILY=SOC_FAMILY_J7
else ifeq ($(SOC),tda54)
    TARGET_SOC=TDA54
    SOC_DEF+=SOC_TDA54
    # Temporary: Using VPAC3 c models for TDA54, should be VPAC4 once the models are up
    VPAC_VERSION=VPAC3
    C7X_TARGET=C7604
    C7X_VERSION=C7604
    MPU_CPU=A720
    SOC_FAMILY=SOC_FAMILY_TDA5
else
    $(error SOC env variable should be set to one of (j721e, j721s2, j784s4, j742s2, j722s, am62a, tda54))
endif
