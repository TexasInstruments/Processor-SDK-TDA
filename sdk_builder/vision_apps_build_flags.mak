PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder

# Inherit build flags from tiovx, imaging needed for building vision_apps
include $(PSDK_PATH)/tiovx/build_flags.mak
include $(PSDK_PATH)/imaging/build_flags.mak
include $(PSDK_BUILDER_PATH)/makerules/makefile_ecu.mak

ifndef $(VISION_APPS_BUILD_FLAGS_MAK)
    VISION_APPS_BUILD_FLAGS_MAK = 1

    ifeq ($(BUILD_QNX_MPU),yes)
        ifeq ($(SOC), $(filter $(SOC), j722s j721s2 j721e j784s4 j742s2))
            include $(PSDK_QNX_PATH)/qnx/psdk_qnx_build.mk
        else ifeq ($(SOC), $(filter $(SOC), tda54))
            include $(PSDK_QNX_PATH)/makerules/psdk_qnx_build.mk
        endif
    endif

    # Used to version control the tivision_apps.so and ipk files
    ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5)
        PSDK_VERSION?=12.0.1
    else ifeq ($(SOC_FAMILY), $(filter $(SOC_FAMILY), SOC_FAMILY_AM SOC_FAMILY_J7))
        PSDK_VERSION?=11.2.1
    endif

    # Build specific CPUs
    ifeq ($(SOC),j721e)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=no
        BUILD_CPU_MCU2_0?=yes
        BUILD_CPU_MCU2_1?=yes
        BUILD_CPU_MCU3_0?=yes
        BUILD_CPU_MCU3_1?=yes
        BUILD_CPU_C6x_1?=yes
        BUILD_CPU_C6x_2?=yes
        BUILD_CPU_C7x_1?=yes
    else ifeq ($(SOC),j721s2)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=no
        BUILD_CPU_MCU2_0?=yes
        BUILD_CPU_MCU2_1?=yes
        BUILD_CPU_MCU3_0?=yes
        BUILD_CPU_MCU3_1?=yes
        BUILD_CPU_C7x_1?=yes
        ifeq ($(ECU_BUILD), $(filter $(ECU_BUILD), fc)) # disable C7x-2 for FC ECU build
            BUILD_CPU_C7x_2?=no
        else
            BUILD_CPU_C7x_2?=yes
        endif
    else ifeq ($(SOC),j784s4)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=no
        BUILD_CPU_MCU2_0?=yes
        BUILD_CPU_MCU2_1?=yes
        BUILD_CPU_MCU3_0?=yes
        BUILD_CPU_MCU3_1?=yes
        BUILD_CPU_MCU4_0?=yes
        BUILD_CPU_MCU4_1?=yes
        BUILD_CPU_C7x_1?=yes
        ifeq ($(ECU_BUILD), $(filter $(ECU_BUILD), fc)) # disable C7x-2 for FC ECU build
            BUILD_CPU_C7x_2?=no
        else
            BUILD_CPU_C7x_2?=yes
        endif
        ifeq ($(ECU_BUILD), $(filter $(ECU_BUILD), srv fc avp4)) # disable C7x-3 and C7x-4 for SRV/FC/AVP4 ECU build
            BUILD_CPU_C7x_3?=no
            BUILD_CPU_C7x_4?=no
        else
            BUILD_CPU_C7x_3?=yes
            BUILD_CPU_C7x_4?=yes
        endif
    else ifeq ($(SOC),j742s2)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=no
        BUILD_CPU_MCU2_0?=yes
        BUILD_CPU_MCU2_1?=yes
        BUILD_CPU_MCU3_0?=yes
        BUILD_CPU_MCU3_1?=yes
        BUILD_CPU_MCU4_0?=yes
        BUILD_CPU_MCU4_1?=yes
        BUILD_CPU_C7x_1?=yes
        BUILD_CPU_C7x_2?=yes
        BUILD_CPU_C7x_3?=yes
    else ifeq ($(SOC),am62a)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=yes
        BUILD_CPU_C7x_1?=yes
        ifeq ($(BUILD_QNX_MPU),yes)
            BUILD_CPU_MCU2_0?=yes
        else
            BUILD_CPU_MCU2_0?=no
        endif
    else ifeq ($(SOC),j722s)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_MCU1_0?=no
        BUILD_CPU_MCU2_0?=yes
        BUILD_CPU_C7x_1?=yes
        ifeq ($(ECU_BUILD), $(filter $(ECU_BUILD), fc)) # disable C7x-2 for FC ECU build
            BUILD_CPU_C7x_2?=no
        else
            BUILD_CPU_C7x_2?=yes
        endif
    else ifeq ($(SOC),tda54)
        BUILD_CPU_MPU1?=yes
        BUILD_CPU_DMCU0?=no
        BUILD_CPU_MCU0?=yes
        BUILD_CPU_MCU1?=yes
        BUILD_CPU_MCU2?=yes
        BUILD_CPU_MCU3?=yes
        BUILD_CPU_MCU4?=yes
        BUILD_CPU_RMCU0_0?=yes
        BUILD_CPU_RMCU0_1?=no
        BUILD_CPU_RMCU1_0?=no
        BUILD_CPU_RMCU1_1?=no
        BUILD_CPU_RMCU2_0?=no
        BUILD_CPU_RMCU2_1?=no
        BUILD_CPU_C7x_1?=yes
        BUILD_CPU_C7x_2?=no
        BUILD_CPU_C7x_3?=no
        BUILD_CPU_C7x_4?=no
    endif

    # Since MCU R5F runs in locked step mode in vision apps, dont set these to 'yes'
    BUILD_CPU_MCU1_1?=no

    # when mcu2-1 build is enabled, mcu2-0 must also be built
    ifeq ($(BUILD_CPU_MCU2_1),yes)
    BUILD_CPU_MCU2_0=yes
    endif

    CORE_LIST = MPU1 MCU1_0 MCU2_0 MCU2_1 MCU3_0 MCU3_1 MCU4_0 MCU4_1 MCU0 MCU1 MCU2 MCU3 MCU4 RMCU0_0 RMCU0_1 \
                RMCU1_0 RMCU1_1 RMCU2_0 RMCU2_1 C6x_1 C6x_2 C7x_1 C7x_2 C7x_3 C7x_4
    ENABLED_IPC_CORES = $(foreach CORE,$(CORE_LIST),$(if $(filter yes,$(BUILD_CPU_$(CORE))),ENABLE_IPC_$(CORE)))

    # BUILD_EDGEAI and HS flags are now internal ONLY
    # Derived from SOC and TISDK_IMAGE flags
    ifeq ($(SOC)-$(TISDK_IMAGE), am62a-edgeai)
        BUILD_EDGEAI=yes
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), am62a-adas)
        BUILD_EDGEAI=no
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), j721e-edgeai)
        BUILD_EDGEAI=yes
        HS=0
    else ifeq ($(SOC)-$(TISDK_IMAGE), j721e-adas)
        BUILD_EDGEAI=no
        HS=0
    else ifeq ($(SOC)-$(TISDK_IMAGE), j721s2-edgeai)
        BUILD_EDGEAI=yes
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), j721s2-adas)
        BUILD_EDGEAI=no
        HS=0
    else ifeq ($(SOC)-$(TISDK_IMAGE), j784s4-edgeai)
        BUILD_EDGEAI=yes
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), j784s4-adas)
        BUILD_EDGEAI=no
        HS=0
    else ifeq ($(SOC)-$(TISDK_IMAGE), j742s2-adas)
        BUILD_EDGEAI=no
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), j722s-edgeai)
        BUILD_EDGEAI=yes
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), j722s-adas)
        BUILD_EDGEAI=no
        HS=1
    else ifeq ($(SOC)-$(TISDK_IMAGE), tda54-adas)
        BUILD_EDGEAI=no
        HS=1
    else
        BUILD_EDGEAI=no
        HS=0
    endif

    # Flag to select silicon revision: 2_0 for SR 2.0, 1_1 for SR 1.1, 1_0 for SR 1.0
    HS_SR?=1_1
    ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2))
        # Note: There is only SR 1.0 for J721S2 and J784S4 HS
        HS_SR=1_0
    endif

    ifneq ($(SOC_FAMILY), SOC_FAMILY_TDA5)
        BUILD_PTK?=yes
        BUILD_ENABLE_ETHFW?=yes
    endif

    # ETHFW is not supported on the below platforms
    ifneq (,$(filter $(SOC),j721s2 am62a j722s j742s2 tda54))
        BUILD_ENABLE_ETHFW=no
    endif

    ifeq ($(RTOS),SAFERTOS)
        ifeq ($(SOC),$(filter $(SOC), j722s j784s4 j742s2 tda54))
            BUILD_ENABLE_ETHFW=no
        endif
    endif

    ifeq ($(BUILD_EDGEAI),yes)
        BUILD_ENABLE_ETHFW=no
    endif

    ifeq ($(BUILD_QNX_MPU),yes)
        ifeq ($(QNX_IOPKT_BSP),0)
            BUILD_ENABLE_ETHFW=no
        endif
        ifeq ($(QNX_SDP_VERSION),800)
            BUILD_ENABLE_ETHFW=no
        endif
    endif

    # Need to export this variable so that the following xdc .cfg file can pick this up from the env:
    # ${PSDK_PATH}/vision_apps/platform/$(SOC)/rtos/mcu2_0/mcu2_0.cfg
    export BUILD_ENABLE_ETHFW

    ifeq ($(BUILD_ENABLE_ETHFW),yes)
        # Enable iperf server (TCP only) by default
        ETHFW_IPERF_SERVER_SUPPORT?=yes
        # gPTP stack support - currently supported in FreeRTOS build only
        ifeq ($(RTOS),FREERTOS)
            ETHFW_GPTP_SUPPORT?=yes
        endif

        # gPTP is supported only in ETHFW R5F core
        ifeq ($(ETHFW_GPTP_SUPPORT),yes)
            ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5)
                ETHFW_GPTP_BUILD_SUPPORT = $(BUILD_CPU_MCU0)
            else
                ETHFW_GPTP_BUILD_SUPPORT = $(BUILD_CPU_MCU2_0)
            endif
        endif

        # Ethfw Demo support
        ETHFW_DEMO_SUPPORT?=no

        # Enable EthFw Monitor - to monitor the EthFw status and handle any HW Errors if detected.
        ETHFW_MONITOR_SUPPORT?=yes

        # Ethfw EST Demo App support
        ETHFW_EST_DEMO_SUPPORT?=no

        # Proxy ARP handling support
        # To enable this feature, ETHFW_PROXY_ARP_SUPPORT must be set to "yes" in
        # ethfw_build_flags.mk. This feature is enabled by default.

        # Inter-core virtual ethernet support
        # Supported Values: yes | no
        ifneq (,$(filter yes,$(BUILD_CPU_MCU2_0) $(BUILD_CPU_MCU0)))
            ifeq ($(BUILD_QNX_MPU),yes)
                ETHFW_INTERCORE_ETH_SUPPORT?=no
            else
                ETHFW_INTERCORE_ETH_SUPPORT?=yes
            endif
        endif

        ifneq (,$(filter $(SOC), j784s4 j742s2))
            ETHFW_CPSW_VEPA_SUPPORT = yes
        endif

    endif

    # Build TI-RTOS fileio binaries
    BUILD_APP_RTOS_FILEIO?=no

    # Build RTOS + Linux binaries
    BUILD_APP_RTOS_LINUX?=$(BUILD_LINUX_MPU)

    # Build RTOS + QNX binaries
    BUILD_APP_RTOS_QNX?=$(BUILD_QNX_MPU)

    ifeq ($(RTOS_SDK),mcu_plus_sdk)
        ifeq ($(SOC),am62a)
            # mcu plus sdk DEVICE to build for, valid values: am62ax
            BUILD_MCU_PLUS_SDK_DEVICE=am62ax
        else ifeq ($(SOC),j722s)
            # mcu plus sdk DEVICE to build for, valid values: j722s
            BUILD_MCU_PLUS_SDK_DEVICE=j722s
        endif
    endif

    # If set to no, then MCU core firmware will be built with NO board dependencies
    # (such as I2C, board specific PINMUX, DSS, HDMI, I2C, ETHFW, CSIRX, CSITX).  Most of
    # the packaged vision_apps require these interfaces on the MCU for the EVM, but
    # when porting to a board other than an EVM, or using applications which control
    # these interfaces from the HLOS on A72 (such as EdgeAI kit), then this should be set
    # to 'no'.
    BUILD_MCU_BOARD_DEPENDENCIES?=yes

    ifeq ($(BUILD_EDGEAI), yes)
        BUILD_MCU_BOARD_DEPENDENCIES=no
        FIRMWARE_SUBFOLDER=vision_apps_eaik
        UENV_NAME=uEnv_$(SOC)_edgeai_apps.txt
    endif

    # Set to 'yes' to build tidl_rt and copy OSRT libs to fs_stage
    BUILD_ENABLE_TIDL_RT?=yes

    # Set to 'yes' to link all .out files against libtivision_apps.so instead of static libs
    LINK_SHARED_OBJ?=yes
    ifeq ($(SOC),$(filter $(SOC), j722s tda54))
        # temporarily setting this to "no" for the QNX J722S and TDA54 until the full BSP is received which
        # has the screen package
        ifeq ($(BUILD_QNX_MPU),yes)
            LINK_SHARED_OBJ=no
        endif
    endif

    # Build a specific CPU type's based on CPU flags status defined above
    ifneq (,$(filter yes,$(BUILD_CPU_MCU1_0) $(BUILD_CPU_MCU1_1) $(BUILD_CPU_MCU2_0) $(BUILD_CPU_MCU2_1) $(BUILD_CPU_MCU3_0) $(BUILD_CPU_MCU3_1) $(BUILD_CPU_MCU4_0) $(BUILD_CPU_MCU4_1)))
        BUILD_ISA_R5F=yes
    else
        BUILD_ISA_R5F=no
    endif
    ifneq (,$(filter yes,$(BUILD_CPU_MCU0) $(BUILD_CPU_MCU1) $(BUILD_CPU_MCU2) $(BUILD_CPU_MCU3) $(BUILD_CPU_MCU4)))
        BUILD_ISA_M55=yes
    else
        BUILD_ISA_M55=no
    endif
    ifneq (,$(filter yes,$(BUILD_CPU_RMCU0_0) $(BUILD_CPU_RMCU0_1) $(BUILD_CPU_RMCU1_0) $(BUILD_CPU_RMCU1_1) $(BUILD_CPU_RMCU2_0) $(BUILD_CPU_RMCU2_1)))
        BUILD_ISA_R52P=yes
    else
        BUILD_ISA_R52P=no
    endif
    ifneq (,$(filter yes,$(BUILD_CPU_C6x_1) $(BUILD_CPU_C6x_2)))
        BUILD_ISA_C6x=yes
    else
        BUILD_ISA_C6x=no
    endif
    ifneq (,$(filter yes,$(BUILD_CPU_C7x_1) $(BUILD_CPU_C7x_2) $(BUILD_CPU_C7x_3) $(BUILD_CPU_C7x_4)))
        BUILD_ISA_C7x=yes
    else
        BUILD_ISA_C7x=no
    endif
    ifneq (,$(filter yes,$(BUILD_CPU_MPU1)))
        BUILD_ISA_MPU=yes
    else
        BUILD_ISA_MPU=no
    endif

endif # ifndef $(VISION_APPS_BUILD_FLAGS_MAK)
