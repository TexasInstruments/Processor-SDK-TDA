ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET     := app_utils_mem
    TARGETTYPE := library

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),FREERTOS SAFERTOS THREADX))
        CSOURCES := app_mem_rtos.c
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES := app_mem_linux.c
    endif

    ifeq ($(RTOS_SDK),pdk)
        IDIRS    += $(PDK_PATH)/packages/ti/drv/udma
    endif

    ifeq ($(TARGET_OS),QNX)
        ifeq ($(SOC), tda54)
            IDIRS += $(PSDK_QNX_PATH)/src/resmgrs/sharedmemallocator/usr/public
            IDIRS += $(PSDK_QNX_PATH)/src/resmgrs/sharedmemallocator/resmgr/public
            IDIRS += $(PSDK_QNX_PATH)/src/common/lld_source/source/kernel/dpl
            IDIRS += $(PSDK_QNX_PATH)/src/common/lld_source/source/
        else
            IDIRS    += $(PSDK_QNX_PATH)/qnx/sharedmemallocator/usr/public
            IDIRS    += $(PSDK_QNX_PATH)/qnx/sharedmemallocator/resmgr/public
        endif
        CSOURCES := app_mem_qnx.c
    endif

    ifeq ($(LDRA_COVERAGE_ENABLED), yes)
        include $(PSDK_PATH)/tiovx/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
    else
        DEFS += LDRA_UNTESTABLE_CODE
        DEFS += LDRA_R5F_UNTESTABLE_CODE
    endif

    include $(FINALE)

endif

ifeq ($(TARGET_CPU),x86_64)
    include $(PRELUDE)
    TARGET     := app_utils_mem
    TARGETTYPE := library

    CSOURCES   := app_mem_pc.c

    ifeq ($(RTOS_SDK),pdk)
        IDIRS    += $(PDK_PATH)/packages/ti/drv/udma
    endif

    ifeq ($(RTOS_SDK),mcu_sdk)
        IDIRS      += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
        IDIRS      += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
        IDIRS      += $(MCU_SDK_PATH)/source/compiler/hostemu-gcc-linux
        IDIRS      += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
        IDIRS      += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support
        IDIRS      += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
        IDIRS      += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/Hal_Cfg
        IDIRS      += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
    endif

    include $(FINALE)
endif
