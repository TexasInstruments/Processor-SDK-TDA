ifneq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
    include $(PRELUDE)

    TARGET      := app_utils_misc
    TARGETTYPE  := library

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C71 C7120 C7504))
            ifneq ($(SOC),am62a)
                ASSEMBLY += app_c7x_init_asm.asm
                CSOURCES += app_c7x_init.c
                CFLAGS   += -DBUILD_C7X
            endif
        endif

        ifeq ($(TARGET_OS),FREERTOS)
            ifeq ($(RTOS_SDK), mcu_plus_sdk)
                IDIRS  += $(MCU_PLUS_SDK_PATH)/source/fs/freertos_fat/portable/nortos/
            else ifeq ($(RTOS_SDK), mcu_sdk)
                IDIRS  += $(MCU_SDK_PATH)/source/kernel/freertos/FreeRTOS-Kernel/include
            else
                IDIRS  += $(PDK_PATH)/packages/ti/kernel/freertos/FreeRTOS-LTS/FreeRTOS-Kernel/include/
            endif
        endif

        ifeq ($(TARGET_OS),$(filter $(TARGET_OS),FREERTOS SAFERTOS THREADX))
            CSOURCES  += app_cpu_hz.c
        endif

        ifeq ($(TARGET_OS),SAFERTOS)
            ifeq ($(TARGET_CPU),R5F)
                IDIRS  += ${SAFERTOS_KERNEL_INSTALL_PATH_r5f}/source_code_and_projects/SafeRTOS/config
                CFLAGS += -DBUILD_MCU
            endif
            ifeq ($(TARGET_CPU),C66)
                IDIRS  += ${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/config
            endif
            ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C71 C7120 C7504))
                IDIRS  += ${SAFERTOS_KERNEL_INSTALL_PATH_c7x}/source_code_and_projects/SafeRTOS/config
            endif
            ifeq ($(RTOS_SDK), pdk)
                IDIRS      += ${PDK_PATH}/packages/ti/osal/src/safertos
            endif
        endif

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C71 C7120))
            IDIRS     += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/c7x
            ifeq ($(SOC),j742s2)
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/c7x
            else
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/c7x
            endif
            IDIRS     += $(PDK_PATH)/packages/ti/csl/arch/c7x
        endif

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C7504))
            IDIRS     += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/c75x
            ifeq ($(SOC),j742s2)
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/c75x
            else
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/c75x
            endif
            IDIRS     += $(PDK_PATH)/packages/ti/csl/arch/c7x
        endif

        ifeq ($(TARGET_CPU),C7604)
            IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/freertos_cfg/c76
            IDIRS += $(MCU_SDK_PATH)/source/kernel/freertos/portable/ti-c7000/c76
            IDIRS += $(MCU_SDK_PATH)/source/arch/c76/include
        endif

        ifeq ($(TARGET_CPU),R52P)
            IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/freertos_cfg/r52
            IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/arch_cfg
            IDIRS += $(MCU_SDK_PATH)/source/arch/r52/include
            IDIRS += $(MCU_SDK_PATH)/source/kernel/freertos/portable/ti-arm-clang/r52
        endif

        ifeq ($(TARGET_CPU),M55)
            IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/freertos_cfg/m55
            IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/arch_cfg
            IDIRS += $(MCU_SDK_PATH)/source/kernel/freertos/FreeRTOS-Kernel/portable/GCC/ARM_CM55_NTZ/non_secure
        endif

        ifeq ($(TARGET_CPU),C66)
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/c66
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/c66
        endif

        ifeq ($(TARGET_CPU),R5F)
            IDIRS     += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/r5f
            ifeq ($(SOC),j742s2)
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/r5f
            else
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/r5f
            endif

            CSOURCES  += app_r5f_init.c

            ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 AM62A))
                CSOURCES += app_pinmux_$(SOC).c
            endif

            DEFS += $(BUILD_PDK_BOARD)

        endif
    endif

    CSOURCES += app_utils_task_init.c

    include $(FINALE)
endif
