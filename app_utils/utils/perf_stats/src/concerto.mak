ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64))

    include $(PRELUDE)

    TARGET      := app_utils_perf_stats
    TARGETTYPE  := library

    CSOURCES    += app_perf_stats_api_x86.c

    include $(FINALE)

endif

ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 AM62A J722S TDA54))

    include $(PRELUDE)
    TARGET       := app_utils_perf_stats
    TARGETTYPE   := library

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), FREERTOS SAFERTOS THREADX))
        CSOURCES := app_perf_stats_freertos.c
        ifeq ($(SOC),j742s2)
            # Reuse j784s4 since a dedicated folder was not created when j742s2 was added
            IDIRS += $(PDK_PATH)/packages/ti/osal/soc/j784s4
        else
            IDIRS += $(PDK_PATH)/packages/ti/osal/soc/$(SOC)
        endif
        IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/FreeRTOS-LTS/FreeRTOS-Kernel/include
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R52P M55))
            ifeq ($(SOC),j742s2)
                # Reuse j784s4 since a dedicated folder was not created when j742s2 was added
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/r5f
            else
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/r5f
            endif
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/r5f
        endif
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C71 C7120 C7504 C7524 C7604))
            ifeq ($(SOC),j742s2)
                # Reuse j784s4 since a dedicated folder was not created when j742s2 was added
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/j784s4/c7x
            else
                IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/c7x
            endif
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/c7x
            IDIRS += $(PDK_PATH)/packages/ti/csl/arch/c7x
        endif
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C66))
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/config/$(SOC)/c66
            IDIRS += $(PDK_PATH)/packages/ti/kernel/freertos/portable/TI_CGT/c66
        endif
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        CSOURCES := app_perf_stats_hlos.c
    endif

    CSOURCES += app_perf_stats_api.c

    ifeq ($(SOC),am62a)
        ifeq ($(TARGET_CPU),C7504)
            DEFS+=CPU_c7504
        endif
    endif

    ifeq ($(SOC),tda54)
        ifeq ($(TARGET_CPU),C7604)
            DEFS+=CPU_c7604
        endif
    endif

    ifeq ($(SOC),j722s)
        ifeq ($(TARGET_CPU),C7524)
            DEFS+=CPU_c7524
        endif
    endif

    ifeq ($(VDK), yes)
        DEFS+=VDK
    endif

    include $(FINALE)

endif
